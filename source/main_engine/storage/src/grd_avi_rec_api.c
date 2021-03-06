#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <malloc.h>

#include "sdk_struct.h"
#include "log.h"

#include "avi_typedef.h"
#include "grd_avi_rec_api.h"
#include "avi_common.h"
#include "storage.h"
#include "record.h"


#define DATA_SAVE_NUM 	(100)
#define ONE_INDEX_SIZE  (16)


char normal_data_array[REC_BUF_SIZE];
char event_data_array[REC_BUF_SIZE];



struct timeval g_rec_pre_time;
int a_chans = 1;    /* Audio channels, 0 for no audio */
int a_rate = 8000;  /* Rate in Hz */
int a_bits = 8;    /* bits per audio sample */

int samplesize;
int g_sd_tats;
sna_callback  cb;

#define CAL_TIME_REC 0
#define NO_WRITE_INDEX_FILE 1


#define PRINT_ERR_INFO(fmt, args...) printf(fmt, ##args)
#define PRINT_INFO(fmt, args...) printf(fmt, ##args)
#define PRINT_ERR() printf("<<< File: %s, Line: %d, Function: %s >>>\n", __FILE__, __LINE__, __FUNCTION__)

#define PRINT_ERR_MSG(fmt, args...) \
            printf("[File: %s, Line: %d, Function: %s ]  " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##args)

#define TIMEVAL_DURATION(start, end) (((long)(end)-(long)(start)))



int write_record_node_index(unsigned char recordtype, unsigned int stratTime, unsigned endTime)
{
    FILE *fpIdx = NULL;
	char fullname[64];
	int nbytes;
	unsigned int duration = TIMEVAL_DURATION(stratTime, endTime);

	snprintf(fullname, sizeof(fullname), "%s/%s/%s", GRD_SD_MOUNT_POINT, RECORD_VIDEO_PATH, VIDEO_NODE_FILE_NAME);
    fpIdx = fopen(fullname, "a+");
    if(fpIdx == NULL)
    {
        printf("open index file error!\n");
        return -1;
    }
    NodeIndex file_node;

    memset(&file_node, 0, sizeof(NodeIndex));
    file_node.recordtype = recordtype;
    file_node.uiStartTime = stratTime;
    file_node.uiDurations = duration;
printf("NodeIndex: recordtype  = %d\n", file_node.recordtype);
printf("NodeIndex: uiStartTime = %u\n", file_node.uiStartTime);
printf("NodeIndex: uiDurations = %u\n", file_node.uiDurations);

    nbytes = fwrite(&file_node, 1, sizeof(NodeIndex), fpIdx);
info("write NodeIndex to %s with %d bytes.\n", fullname, nbytes);
    fclose(fpIdx);

    return 0;
}


unsigned long get_UTC_Time()
{
	time_t t;
	t = time(&t);
	return (unsigned long)t;
}


int gm_system_cmd(char *cmd)
{
    int ret;
    if ( (ret=system(cmd)) != -1)   /* fork success */
        return ret;
}

int check_rename_file_size(char *file)
{
    assert( file != NULL );

    if(0 != access(file, F_OK) )
    {
        printf("%s[%d]......file not exist!\n", __FUNCTION__, __LINE__);
        return 0;
    }

    char cmd[32] = {0};
    sprintf(cmd, "du -m %s > /tmp/1.txt", file);
    gm_system_cmd(cmd);

    int fd = open("/tmp/1.txt", O_RDONLY);

    char buf[128] = {0};
    int size = read(fd, buf, 128);

    close(fd);
    int i = 0;

    char buf_size[8] = {0};

    while( buf[i]  != '\t' )
    {
        buf_size[i] = buf[i];
        i++;
    }

    memset(cmd, 0, 32);
    sprintf(cmd, "rm %s", "/tmp/1.txt");
    gm_system_cmd(cmd);

    return atoi(buf_size);
}

static int update_avi_header(AviFile *avi_file)
{
    int ret;
    int avi_len = ftell(avi_file->file) - 8;
    int movi_len = avi_file->data_offset - sizeof(AviHeader) + 4;

	//movi_len 为实际数据除过头部信息， data_offset 为缓冲区的偏移量，这个偏移量指的是整个avi 文件
    info("index offset = %d, avi_len = %d, movi_len = %d.%d\n", avi_file->data_offset, avi_len, movi_len, sizeof(AviHeader) );
	if(0 > g_sd_tats)	
	{
		error("update_avi_header:SD card Error!!\n");
		return -1;
	}

    fseek(avi_file->file, 4, SEEK_SET);
	//avi_len 为整个文件的长度
    ret = write_int32(avi_file->file, avi_len);
    if(ret != 1)
    {
        error("write avi len failed.\n");
        return -1;
    }
	fflush(avi_file->file);

    fseek(avi_file->file, sizeof(AviHeader) - 8, SEEK_SET);
    ret = write_int32(avi_file->file, movi_len);
    if(ret != 1)
    {
        error("write movi len failed.\n");
        return -1;
    }
	
	fflush(avi_file->file);
	//视频帧数个数
    fseek(avi_file->file, (int)&(((AviHeader *)0)->main_header.dwTotalFrames), SEEK_SET);
    ret = write_int32(avi_file->file, avi_file->video_count);
    if(ret != 1)
    {
        error("write total frames count failed.\n");
        return -1;
    }
	 //dwMicroSecPerFrame 视频帧间隔时间（以毫秒为单位）
    fseek(avi_file->file,
          (int) &(((AviHeader *) 0)->main_header.dwMicroSecPerFrame),
          SEEK_SET);
	 //视频帧数个数
    ret = write_int32(avi_file->file,
                      avi_file->duration * 1000 / avi_file->video_count);
    if( ret != 1 )
    {
        error("write MicroSecPerFrame failed.\n");
        return -1;
    }
	
	fflush(avi_file->file);
	#if 1  // change by bc 20160114
	// dwScale 这个流使用的时间尺度
    fseek(avi_file->file, (int) &(((AviHeader *) 0)->video_header.dwScale),
          SEEK_SET);
	//视频帧数个数
    ret = write_int32(avi_file->file,
                      avi_file->duration * 1000 / avi_file->video_count);
    if( ret != 1 )
    {
        error("write scale failed.\n");
        return -1;
    }
	#endif
	
	//dwLength 流的长度（单位与dwScale和dwRate的定义有关）
    fseek(avi_file->file, (int)&(((AviHeader *)0)->video_header.dwLength), SEEK_SET);
	//视频帧数个数
    ret = write_int32(avi_file->file, avi_file->video_count);
    if(ret != 1)
    {
        error("write video frames count failed.\n");
        return -1;
    }
	fflush(avi_file->file);

    samplesize = ((a_bits+7)/8)*a_chans;
    if(samplesize == 0)
        samplesize=1; /* avoid possible zero divisions */
	
    int audio_count = avi_file->audio_bytes / samplesize;
	//printf("audio_count=%d  audio_bytes=%d\n",audio_count,avi_file->audio_bytes);
	//音频帧数个数
    fseek(avi_file->file, (int)&(((AviHeader *)0)->audio_header.dwLength), SEEK_SET);
    ret = write_int32(avi_file->file, audio_count);
    if(ret != 1)
    {
        error("write audio block count failed.\n");
        return -1;
    }
	fflush(avi_file->file);
	warning("===++++++++++++++++++++WANGRONG======avi_file->audio_count : [%d]====== audio_count : [%d] \n", avi_file->audio_count,audio_count);

   fseek(avi_file->file, (int)&(((AviHeader *)0)->audio_header.dwRate), SEEK_SET);
   if(avi_file->duration < 1000) avi_file->duration = 1000;//单位是ms
   int dwRate = audio_count/(avi_file->duration/1000);
   ret = write_int32(avi_file->file, 8000);//dwRate
	if(ret != 1)
	{
		error("write audio dwRate failed.\n");
		return -1;
	}

	fflush(avi_file->file);

	warning("=========dwRate : [%d] \n", dwRate);
	return 0;
}


int avi_add_index(AviFile *avi_file)
{
    int ret;
	//static int first=0;
	if(0 > g_sd_tats)	
	{
		error("avi_add_index:SD card Error!!\n");
		return -1;
	}

    if(!avi_file)
    {
        error("parameter is error.\n");
        return -1;
    }
 
    info("index_count = %d. data offset = %d\n", avi_file->index_count, avi_file->data_offset);

    //add index
    ret = write_fourcc(avi_file->file, MAKE_FOURCC('i','d','x','1'));
    if(ret != 1)
    {
        error("write avi index fcc failed.\n");
        return -1;
    }
	
	//将剩余的索引协议文件
	 int idx_count = avi_file->pre_index_pos;// > DATA_SAVE_NUM) ? DATA_SAVE_NUM :avi_file->pre_index_pos;
	 info("-----------idx_count:%d  index_count:%d idxfile:%d pre_index_pos:%d \n",idx_count,avi_file->index_count,avi_file->idxfile,avi_file->pre_index_pos);
	 //写索引到临时文件 一次写16个索引
	 if(idx_count > 0)
	 {
	  ret = fwrite(avi_file->idx_array,	idx_count*ONE_INDEX_SIZE, 1, avi_file->idxfile);
	  if(ret != 1)
        {
            error("fwrite failed.\n");
            return -1;
        }
	  
	  fflush(avi_file->idxfile);
	 }
	  avi_file->pre_index_pos = 0 ; // 记录上一次写索引的偏移位置
	  
	//在索引部分添加所有的帧数
    ret = write_int32(avi_file->file, 16*avi_file->index_count);
    if(ret != 1)
    {
        error("write avi index size failed.\n");
        return -1;
    }
	
   // info("add index, count = %d.\n", avi_file->index_count);
	//将所有帧数的索引一次性写入录像的索引部分
	#if 0
    ret = fwrite(avi_file->idx_array, avi_file->index_count*16, 1, avi_file->file);
    if(ret != 1)
    {
       error("write avi index array failed.\n");
       return -1;
    }
	#else
    fseek(avi_file->idxfile, 0, SEEK_SET);
	char read_buf[1024] = {0}; //16*64
	int read_size = 0;
	int read_count = 0,left_count = avi_file->index_count;
	while(left_count)
	{
	if(64 > left_count)
	{
		read_size = left_count*16;
		left_count = 0;
	}
	else
	{
	 read_size	 = 1024;
	 left_count  -=  64;
	}
	///info("=====left_count:%d  read_buf:%p, read_size:%d,avi_file->idxfile:%d \n",left_count,read_buf, read_size,avi_file->idxfile);
	ret = fread(read_buf, read_size, 1, avi_file->idxfile);
	if(ret != 1)
    {
       error("write avi index array failed.\n");
       return -1;
    }

	ret = fwrite(read_buf, read_size, 1, avi_file->file);
    if(ret != 1)
    {
       error("write avi index array failed.\n");
       return -1;
    }
	}
	#endif
	fflush(avi_file->file);
    info("update_avi_header\n");
    //updata avi header
    ret = update_avi_header(avi_file);
    if(ret < 0)
    {
        error("call update_avi_header failed.\n");
        return -1;
    }

    return 0;
}

int avi_add_index_fixed(AviFile *avi_file)
{
	int ret;
 	if(0 > g_sd_tats)	
	{
		error("avi_add_index:SD card Error!!\n");
		return -1;
	}

	if(!avi_file)
	{
		error("parameter is error.\n");
		return -1;
	}
 
	info("index_count = %d. data offset = %d\n", avi_file->index_count, avi_file->data_offset);

	fseek(avi_file->file,0L,SEEK_END);
	
	//add index
	ret = write_fourcc(avi_file->file, MAKE_FOURCC('i','d','x','1'));
	if(ret != 1)
	{
		error("write avi index fcc failed.\n");
		return -1;
	}
	
 	//在索引部分添加所有的帧数
	ret = write_int32(avi_file->file, 16*avi_file->index_count);
	if(ret != 1)
	{
		error("write avi index size failed.\n");
		return -1;
	}
	
   // info("add index, count = %d.\n", avi_file->index_count);
	//将所有帧数的索引一次性写入录像的索引部分 
	fseek(avi_file->idxfile, 0, SEEK_SET);
	char read_buf[1024] = {0}; //16*64
	int read_size = 0;
	int read_count = 0,left_count = avi_file->index_count;
	while(left_count)
	{
	if(64 > left_count)
	{
		read_size = left_count*16;
		left_count = 0;
	}
	else
	{
	 read_size	 = 1024;
	 left_count  -=  64;
	}
	///info("=====left_count:%d  read_buf:%p, read_size:%d,avi_file->idxfile:%d \n",left_count,read_buf, read_size,avi_file->idxfile);
	ret = fread(read_buf, read_size, 1, avi_file->idxfile);
	if(ret != 1)
	{
	   error("write avi index array failed.\n");
	   return -1;
	}

	ret = fwrite(read_buf, read_size, 1, avi_file->file);
	if(ret != 1)
	{
	   error("write avi index array failed.\n");
	   return -1;
	}
	}
 	fflush(avi_file->file);
	info("update_avi_header\n");
	//updata avi header
	ret = update_avi_header(avi_file);
	if(ret < 0)
	{
		error("call update_avi_header failed.\n");
		return -1;
	}

	return 0;
}


int avi_write_header(AviFile *avi_file, AviInitParam *param)
{
    int bitrate, framerate, width, height, ms_per_frame;
    int ret;

    if( (!avi_file) || (!param))
    {
        error("parameter error.\n");
        return -1;
    }
	if(0 > g_sd_tats)	
	{
		error("avi_write_header:SD card Error!!\n");
		return -1;
	}

    //init param
    bitrate = param->bps;
    framerate = param->fps;
    //ms_per_frame = 1000000./framerate + 0.5;
    ms_per_frame = 1000000./framerate;

	///info("RRRR param->fps:%d  framerate:%d ms_per_frame:%d \n",param->fps,framerate,ms_per_frame);
    width = param->width;
    height = param->height;

    switch(param->codec_type)
    {
    case ENC_TYPE_H264:
        param->type = AVI_TYPE_H264;
        break;
    case ENC_TYPE_MPEG:
        param->type = AVI_TYPE_MPEG4;
        break;
    case ENC_TYPE_MJPEG:
        param->type = AVI_TYPE_MJPEG;
        break;
    }
    switch(param->type)
    {
    case AVI_TYPE_H264:
    case AVI_TYPE_MPEG4:
    case AVI_TYPE_MJPEG:
        param->fcctype = MAKE_FOURCC('v','i','d','s');
        break;
    default:
        break;
    }

    AviHeader avi_header;
    memset(&avi_header, 0, sizeof(AviHeader));

    //write RIFF_AVI
    avi_header.fcc_riff = MAKE_FOURCC('R','I','F','F');
    avi_header.avi_len = 0; //should be updated
    avi_header.fcc_avi = MAKE_FOURCC('A','V','I',' ');

    //write LIST_hdrl
    avi_header.fcc_list_hdrl = MAKE_FOURCC('L','I','S','T');
    avi_header.hdrl_len = (int)&(((AviHeader *)0)->fcc_list_movi) - (int)&(((AviHeader *)0)->fcc_hdrl);
    avi_header.fcc_hdrl = MAKE_FOURCC('h','d','r','l');

    //write AviMainHeader
    avi_header.main_header.fcc = MAKE_FOURCC('a','v','i','h');
    avi_header.main_header.cb = 56; //sizeof(AviMainHeader) - 8 = 64 -8 =56
    avi_header.main_header.dwMicroSecPerFrame = ms_per_frame;
	///info("main_header.dwMicroSecPerFrame:%d ms_per_frame:%d \n",avi_header.main_header.dwMicroSecPerFrame ,ms_per_frame);
    avi_header.main_header.dwMaxBytesPerSec = bitrate / 8;
    avi_header.main_header.dwPaddingGranularity = 0;
    avi_header.main_header.dwFlags = AVIF_HASINDEX | AVIF_WASCAPTUREFILE;
    avi_header.main_header.dwTotalFrames = avi_file->video_count;     //<==need update, framecount, main_header_start+24
    avi_header.main_header.dwInitialFrames = 0;
    avi_header.main_header.dwStreams = 2;
    avi_header.main_header.dwSuggestedBufferSize = PB_BUF_SIZE; // width*height*3; //1000000
    avi_header.main_header.dwWidth = width;
    avi_header.main_header.dwHeight = height;

    /************ video start *******************/
    //write LIST_strl
    avi_header.fcc_list_strl_video = MAKE_FOURCC('L','I','S','T');
    avi_header.size_strl_video = (int)&(((AviHeader *)0)->fcc_list_strl_audio) - (int)&(((AviHeader *)0)->fcc_strl_video);
    avi_header.fcc_strl_video = MAKE_FOURCC('s','t','r','l');

    //write AviStreamHeader
    avi_header.video_header.fcc = MAKE_FOURCC('s','t','r','h');
    avi_header.video_header.cb = 56; //sizeof(AviStreamHeader)-8 = 64-8 =56
    avi_header.video_header.fccType = param->fcctype;  //vids
    avi_header.video_header.fccHandler = param->type; //MAKE_FOURCC('H','2','6','4');
    avi_header.video_header.dwFlags = 0x0;
    avi_header.video_header.wPriority = 0;
    avi_header.video_header.wLanguage = 0;
    avi_header.video_header.dwInitialFrames = 0;
    avi_header.video_header.dwScale = ms_per_frame;
	//info("video_header.dwScale:%d ms_per_frame:%d \n",avi_header.video_header.dwScale ,ms_per_frame);
    avi_header.video_header.dwRate = 1000000;
    avi_header.video_header.dwStart = 0;
    avi_header.video_header.dwLength = avi_file->video_count;  //<==need update, framecount, stream_header_start+52
    avi_header.video_header.dwSuggestedBufferSize = PB_BUF_SIZE; // width*height*3; //1024*1024
    avi_header.video_header.dwQuality = -1;
    avi_header.video_header.dwSampleSize = width*height;
    avi_header.video_header.rcFrame.left = 0;
    avi_header.video_header.rcFrame.top = 0;
    avi_header.video_header.rcFrame.right = width;
    avi_header.video_header.rcFrame.bottom = height;

    //write strf+size+AviStreamFormat
    avi_header.fcc_strf_video = MAKE_FOURCC('s','t','r','f');
    avi_header.size_video_format = 40; //sizeof(BitMapInfoHeader)
    avi_header.bmiHeader.biSize = 40;
    avi_header.bmiHeader.biWidth = width;
    avi_header.bmiHeader.biHeight = height;
    avi_header.bmiHeader.biPlanes = 1;
    avi_header.bmiHeader.biBitCount = 24;
    avi_header.bmiHeader.biCompression = param->type; //MAKE_FOURCC('H','2','6','4'); H264,h264
    avi_header.bmiHeader.biSizeImage = width*height; //width*height
    avi_header.bmiHeader.biXPelsPerMeter = 0;
    avi_header.bmiHeader.biYPelsPerMeter = 0;
    avi_header.bmiHeader.biClrUsed = 0;
    avi_header.bmiHeader.biClrImportant = 0;
    /************ video end *******************/

    /************ audio start *******************/
    //write LIST_strl
    avi_header.fcc_list_strl_audio = MAKE_FOURCC('L','I','S','T');
    avi_header.size_strl_audio = (int)&(((AviHeader *)0)->fcc_list_movi) - (int)&(((AviHeader *)0)->fcc_strl_audio);
	avi_header.fcc_strl_audio = MAKE_FOURCC('s','t','r','l');

    //write AviStreamHeader
    avi_header.audio_header.fcc = MAKE_FOURCC('s','t','r','h');
    avi_header.audio_header.cb = 56; //sizeof(AviStreamHeader)-8 = 64-8 =56
    avi_header.audio_header.fccType = MAKE_FOURCC('a','u','d','s');  //auds
    if(param->a_enc_type == 2) avi_header.audio_header.fccHandler = 0x00000001; // AAC
    else if(param->a_enc_type == 3) avi_header.audio_header.fccHandler = 0; // PCM
    else avi_header.audio_header.fccHandler = MAKE_FOURCC('G','7','1','1');
    //avi_header.audio_header.fccHandler = MAKE_FOURCC('\0','\0','\0','\0');
    avi_header.audio_header.dwFlags = 0x0;
    avi_header.audio_header.wPriority = 0;
    avi_header.audio_header.wLanguage = 0;
    avi_header.audio_header.dwInitialFrames = 0;

    if (param->a_enc_type == 3) a_bits = 16; //PCM
    else a_bits = 8;

    samplesize = ((a_bits+7)/8)*a_chans;
    if(samplesize == 0)
        samplesize=1; /* avoid possible zero divisions */

    if(param->a_enc_type == 2) //AAC
    {
        avi_header.audio_header.dwScale = 1024; //for AAC
    }
    else
    {
        avi_header.audio_header.dwScale = samplesize; // a_bits = 16, 则2 ; a_bits = 8, 则1
    }
    avi_header.audio_header.dwRate = samplesize * a_rate; // 2 * 8000
    avi_header.audio_header.dwStart = 0;
    /*
      dwLength 指定这个流的长度。
      单位由流的头信息中的dwRate 和dwScale 来 确定（即其单位是dwRate/dwScale）。
      （对于视频流，dwLength就是流包含的总 帧数；
      对于音频流，dwLength就是包含的block的数量，block是音频解码器能处理的原子单位）。
      dwLength/(dwRate/dwScale)，即dwLength * dwScale / dwRate，可以得到流的总时长。
    */
    avi_header.audio_header.dwLength = avi_file->audio_bytes / samplesize;
    avi_header.audio_header.dwSuggestedBufferSize = 0; //1024*1024
    avi_header.audio_header.dwQuality = -1;
    avi_header.audio_header.dwSampleSize = samplesize;
    avi_header.audio_header.rcFrame.left = 0;
    avi_header.audio_header.rcFrame.top = 0;
    avi_header.audio_header.rcFrame.right = 0;
    avi_header.audio_header.rcFrame.bottom = 0;

    //write strf+size+AviStreamFormat
    avi_header.fcc_strf_audio = MAKE_FOURCC('s','t','r','f');
    avi_header.size_audio_format = sizeof(AudioFormateX);
	//printf("avi_header.size_audio_format=%d\n",avi_header.size_audio_format);
    //G.711音频编码分a-law和u-law，其中a-law 的tAuds.formatTag=0x06,u-law 的tAuds.formatTag=0x07
    if(param->a_enc_type == 0) //a-law
        avi_header.audio_format.wFormatTag = 0x06;
    else if(param->a_enc_type == 1)
        avi_header.audio_format.wFormatTag = 0x07;
    else if(param->a_enc_type == 2) //AAC
        avi_header.audio_format.wFormatTag = 0xff;
    else if(param->a_enc_type == 3) //PCM
        avi_header.audio_format.wFormatTag = 0x01;
    else
        avi_header.audio_format.wFormatTag = 0x06;

    avi_header.audio_format.nChannels = a_chans;  //声道数,1
    avi_header.audio_format.nSamplesPerSec = a_rate; //采样率,8000
    if(param->a_enc_type == 2) //AAC
    {
        avi_header.audio_format.nAvgBytesPerSec = 4000; //bitrate(per_channel)  = 32000;
        avi_header.audio_format.nBlockAlign = 1024;  //for AAC
        avi_header.audio_format.wBitsPerSample = a_bits; //8
        avi_header.audio_format.cbSize = 0;
    }
    else
    {
        avi_header.audio_format.nAvgBytesPerSec = a_rate * samplesize; //WAVE声音中每秒的数据量
        avi_header.audio_format.nBlockAlign = samplesize;  //数据块的对齐标志
        avi_header.audio_format.wBitsPerSample = a_bits; //8, if pcm: 32
       	avi_header.audio_format.cbSize = 1;
    }
    /************ audio end *******************/

    //write LIST_movi
    avi_header.fcc_list_movi = MAKE_FOURCC('L','I','S','T');
    avi_header.movi_len = 0;
    avi_header.fcc_movi = MAKE_FOURCC('m','o','v','i');

    avi_file->data_offset = sizeof(AviHeader);
	
	if(0 > g_sd_tats)	
	{
		error("avi_write_header:SD card Error!!\n");
		return -1;
	}
	
	
    //write avi header
    ret = fwrite(&avi_header, sizeof(AviHeader), 1, avi_file->file);
    if(ret != 1)
    {
        error("write avi header failed.\n");
        return -1;
    }

    fflush(avi_file->file);
    return 0;
}


int init_rec_param( AviInitParam *avi_init_parm )
{
    memset((void *)avi_init_parm, 0, sizeof(AviInitParam));
    avi_init_parm->ch_num =  0 ; //ch_num;

    int compression_type = 0;

    avi_init_parm->bps =2048 * 1024;
    avi_init_parm->fps = 25;      // grd_pic_current_cfg->stEncMainStream.dwFrameRate;
    avi_init_parm->width = 1280;
    avi_init_parm->height = 720;




    //audio parameter
    avi_init_parm->a_chans = 1;
    avi_init_parm->a_rate = 8000;
    avi_init_parm->a_bits = 8;
    avi_init_parm->a_enc_type = 0; // 0 a-law; 1 u-law; 2 AAC; 3 PCM

    if (avi_init_parm->a_enc_type == 3) avi_init_parm->a_bits = 16; //PCM
    else avi_init_parm->a_bits = 8; // others

    compression_type = 96;// grd_pic_current_cfg->stEncMainStream.enEncodeType;

    switch(compression_type)
    {
    case GRD_ENC_TYPE_H264:
    {
        avi_init_parm->codec_type = ENC_TYPE_H264;
        info("compression type is h264.\n");
        break;
    }
    case GRD_ENC_TYPE_MPEG:
    {
        avi_init_parm->codec_type = ENC_TYPE_MPEG;
        info("compression type is mpeg4.\n");
        break;
    }
    //case 1002:
    case GRD_ENC_TYPE_MJPEG:
    {
        avi_init_parm->codec_type = ENC_TYPE_MJPEG;
        info("compression type is mjpeg.\n");
        break;
    }
    default:
        error("compression type error, compression type = %d.\n", compression_type);
        break;
    }


    avi_init_parm->mode = RECORD_FIXED_DURATION;
    avi_init_parm->size_m = 50;
    info(" init avi_init_parm sucess!!! \n");
    return 0; //init success
}



int avi_normal_record_open( AviFile *avi_file)
{
    int ret = 0;
    char date[10];
    char date_path[128];
    uint64_t all_size;
    char time_start[20];
    AviInitParam *param  = &(avi_file->init_param);
	
    info("avi_record_open start !!!!!!!!!!!!!!! \n");
    if( (!avi_file) || (!param))
    {
        error("parameter error.\n");
        return -1;
    }

    //init avi_file
    avi_file->data_offset = 0;
    avi_file->video_count = 0;
    avi_file->audio_bytes = 0;
    avi_file->index_count = 0;
    avi_file->idx_array_count = 0;

    //调整大小，从MB到Byte
    if( RECORD_IS_FIXED_SIZE(param->mode) )
    {
        all_size = param->size_m * 1024 * 1024; // M to Byte
 
        info("fps = %d, rec size = %d Mb, idx_array_count = %d\n", param->fps, param->size_m, avi_file->idx_array_count);
    }
    else if( RECORD_IS_FIXED_DURATION(param->mode) )
    {
        param->size_m = param->bps / 8 /1024 * param->duration  * 3/2/1024 + 300;     //add 300M for snapshot
		//估算多少帧数据,录像长度（秒） x 30 fps
		avi_file->idx_array_count = ((param->duration) * 30*2); //这里只是一个预估值
        info("fps = %d, rec duration = %d seconds, idx_array_count = %d, size_m=%d\n",
             param->fps, param->duration, avi_file->idx_array_count, param->size_m);
    }
    else
    {
        error("record mode is error. mode = %d\n", param->mode);
        return -1;
    }

    time_t lt;
	time(&lt);
	struct tm *ptm = localtime(&lt);//获取当前时间

    memset(date_path, 0, sizeof(date_path));	
	sprintf(date_path, "%s/%04d%02d%02d", GRD_SD_MOUNT_POINT, ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday); // str = /mnt/sd_card/20120914
	//检测有无SD卡
	if(0 > g_sd_tats)	
	{
		error("avi_record_open:SD card Error!!\n");
		return -1;
	}	
    if( access( date_path, F_OK ) != 0 ) 
    {
        ret = mkdir(date_path, 0777);
        if(ret < 0)
        {
            error("mkdir %s failed\n", date_path);
            return -1;
        }
    }


    //create avi file
    memset(avi_file->filename, 0, sizeof(avi_file->filename));
	sprintf(avi_file->filename, "%s/%llu", date_path, avi_file->file_info.start);  // filename: ch0

    if(avi_file->file)
    {
        fclose(avi_file->file);
        avi_file->file = NULL;
    }
	if(0 > g_sd_tats)	
	{
		error("avi_record_open:SD card Error!!\n");
		return -1;
	}	
    avi_file->file = fopen(avi_file->filename, "wb+"); //改为rb+ ?
    if(NULL == avi_file->file)
    {
        error("fopen %s failed., errno = %d, %s\n", avi_file->filename, errno, strerror(errno));
		return -1;
    }
	
	if( avi_file->idxfile )
	{
		fclose(avi_file->idxfile);
		avi_file->idxfile = NULL;
	}
	
	// 临时文件索引
	memset(avi_file->idxfilename, 0, sizeof(avi_file->idxfilename));
	sprintf(avi_file->idxfilename, "%s/%llu", GRD_SD_MOUNT_POINT, avi_file->file_info.start);
	avi_file->idxfile = fopen(avi_file->idxfilename, "wb+");
	if(NULL == avi_file->idxfile)
	{
	   return -1;
	}
	
	warning("normal avi_file->filename=%s avi_file->idxfilename=%s\n",avi_file->filename,avi_file->idxfilename);
    memset(avi_file->idx_array, 0, INDEX_ARRAY_MAX_NUMBER);
   
   	//write avi header 写信息部分( AVI 头部信息)
    ret = avi_write_header(avi_file, param);
    if(ret < 0)
    {
        error("call avi_write_header failed.\n");
        return -1;
    }

    //写完信息部和初始化玩参数之后关闭文件指针
    if(avi_file->file)
    {
        fclose(avi_file->file);
        avi_file->file = NULL;
    }
	//new_flag : 表示第一次打开，第一次写AVI 文件
    avi_file->new_flag = 1;  
		

	if(1 == avi_file->normal_buf_flag)
		memset(normal_data_array, 0, sizeof(normal_data_array));
    avi_file->buf_offset = 0;
    avi_file->buf_index = 0;
    info("OPEN FILE SUCCESS!!!! \n");
    return 0;
}

int avi_event_record_open( AviFile *avi_file)
{
    int ret = 0;
    char date[10];
    char date_path[128];
    uint64_t all_size;
    char time_start[20];
    AviInitParam *param  = &(avi_file->init_param);
	
	

    info("avi_record_open start !!!!!!!!!!!!!!! \n");
    if( (!avi_file) || (!param))
    {
        error("parameter error.\n");
        return -1;
    }
	usleep(10000);
    //init avi_file
    avi_file->data_offset = 0;
    avi_file->video_count = 0;
	avi_file->audio_count = 0;
    avi_file->audio_bytes = 0;
    avi_file->index_count = 0;
    avi_file->idx_array_count = 0;

    //调整大小，从MB到Byte
    if( RECORD_IS_FIXED_SIZE(param->mode) )
    {
        all_size = param->size_m * 1024 * 1024; // M to Byte
 
        info("fps = %d, rec size = %d Mb, idx_array_count = %d\n", param->fps, param->size_m, avi_file->idx_array_count);
    }
    else if( RECORD_IS_FIXED_DURATION(param->mode) )
    {

        //param->size_m = 50 * param->duration *10 / 600 ;
        param->size_m = param->bps / 8 /1024 * param->duration  * 3/2/1024 + 300;     //add 300M for snapshot


        
		//估算多少帧数据,录像长度（秒） x 30 fps
		avi_file->idx_array_count = ((param->duration) * 60*2); //仅仅是一个预估值
        info("fps = %d, rec duration = %d seconds, idx_array_count = %d, size_m=%d\n",
             param->fps, param->duration, avi_file->idx_array_count, param->size_m);
    }
    else
    {
        error("record mode is error. mode = %d\n", param->mode);
        return -1;
    }
    memset(time_start, 0, sizeof(time_start));
    get_nowtime_str(time_start);
    memset(date, 0, sizeof(date));
    avi_substring(date, time_start, 0, 0+8);
    memset(date_path, 0, sizeof(date_path));
    info("avi_record_open 00000 \n");
	sprintf(date_path, "%s/%s", GRD_SD_MOUNT_POINT, date); // str = /mnt/sd_card/20120914
	//检测有无SD卡
	if(0 > g_sd_tats)	
	{
		error("avi_record_open:SD card Error!!\n");
		return -1;
	}	
    if( access( date_path, F_OK ) != 0 ) 
    {
        ret = mkdir(date_path, 0777);
        if(ret < 0)
        {
            error("mkdir %s failed\n", date_path);
            return -1;
        }
    }

    //create avi file
    memset(avi_file->filename, 0, sizeof(avi_file->filename));
	sprintf(avi_file->filename, "%s/%llu", date_path, avi_file->file_info.start);  // filename: ch0

   
		
    avi_file->file = fopen(avi_file->filename, "wb+"); //改为rb+ ?
    if(NULL == avi_file->file)
    {
        error("fopen %s failed., errno = %d, %s\n", avi_file->filename, errno, strerror(errno));
		return -1;
    }

    info("avi_record_open 111111111111 avi_file->idxfile:%d \n",avi_file->idxfile);
    if( avi_file->idxfile )
    {
        fclose(avi_file->idxfile);
        avi_file->idxfile = NULL;
    }
	// 临时文件索引
    sprintf(avi_file->idxfilename, "%s/%llu", GRD_SD_MOUNT_POINT, avi_file->file_info.start);
    info("avi_record_open 3333333  avi_file->idxfilename:%s \n",avi_file->idxfilename);
    avi_file->idxfile = fopen(avi_file->idxfilename, "wb+");
    if(NULL == avi_file->idxfile)
    {
       error("NULL == avi_file->idxfile !!!!\n");
       return -1;
    }
    memset(avi_file->idx_array, 0, INDEX_ARRAY_MAX_NUMBER);
    if(0 > g_sd_tats)	
	{
		error("avi_record_open:SD card Error!!\n");
		return -1;
	}
	warning("==========befor==avi_write_header============\n");
	sync();
   	//write avi header 写信息部分( AVI 头部信息)
    ret = avi_write_header(avi_file, param);
    if(ret < 0)
    {
        error("call avi_write_header failed.\n");
        return -1;
    }

    //写完信息部和初始化玩参数之后关闭文件指针
    if(avi_file->file)
    {
        fclose(avi_file->file);
        avi_file->file = NULL;
    }
	//new_flag : 表示第一次打开，第一次写AVI 文件
    avi_file->new_flag = 1;  
	sync();warning("==========end==avi_write_header============\n");sync();

	if(1 == avi_file->event_buf_flag)
		memset(event_data_array, 0, sizeof(event_data_array));
    avi_file->buf_offset = 0;
    avi_file->buf_index = 0;
    info("ENENT OPEN FILE SUCCESS!!!! \n");
    return 0;
}


//record_duration: 录像时长 单位是秒
int _record_event_start(AviFile *avi_file, int record_type,int record_duration, int frame_rate)
{
    int ret = 0, i = 0;
    if(!avi_file)
    {
        assert(0);
        return -1;
    }
	
    init_rec_param(&avi_file->init_param); //浠庨厤缃腑寰楀埌璁剧疆鍙傛暟
    avi_file->init_param.duration = record_duration;
	avi_file->record_type =record_type;
	avi_file->init_param.fps = frame_rate;
	
    info(" _record_start avi_init_param->duration:%d \n",avi_file->init_param.duration);
	
    if( avi_file->init_param.duration > 0 )
    {
        
        ret = avi_event_record_open(avi_file);
        if(-1 == ret )
            return -1;
  
    }
    return ret;
}
int _record_normal_start(AviFile *avi_file, int record_type,int record_duration, int frame_rate)
{
    int ret = 0, i = 0;
    if(!avi_file)
    {
        assert(0);
        return -1;
    }
	
    init_rec_param(&avi_file->init_param); //浠庨厤缃腑寰楀埌璁剧疆鍙傛暟
    avi_file->init_param.duration = record_duration;
	avi_file->record_type =record_type;
	avi_file->init_param.fps = frame_rate;
	
    info(" _record_start avi_init_param->duration:%d frame_rate:%d \n",avi_file->init_param.duration,frame_rate);
	
    if( avi_file->init_param.duration > 0 )
    {
        
        ret = avi_normal_record_open(avi_file);
        if(-1 == ret )
            return -1;
  
    }
    return ret;
}

 
static int avi_write_frame_data( AviFile *avi_file, char *data, int length_tmp, int intra, struct timeval *pTimeval)
{
    int ret = 0;
	int length = length_tmp;
    int is_align = length % 2;// 2字节对齐
	if(0 > g_sd_tats)	
	{
		error("avi_record_open:SD card Error!!\n");
		return -1;
	}
    if(!avi_file)
    {
        error("parameter error.\n");
        return -1;
    }
 
	//new_flag = 1 : 表示 第一次写AVI 文件; 第一帧数据必须是 I 帧
    if(avi_file->new_flag)  
    {
        if(intra == 1 || intra == 3) //第一帧数据，重命名文件
        {
            avi_file->file = fopen(avi_file->filename, "rb+");
            if(NULL == avi_file->file)
            {
                error("fopen %s failed.\n", avi_file->filename);
                return -1;
            }
            avi_file->data_offset = sizeof(AviHeader) ;//帧头在第一次打开的时候就已经写好了
            fseek(avi_file->file, avi_file->data_offset, SEEK_SET);
            avi_file->new_flag = 0;	
			printf(">>>>>>>>>>avi_file->new_flag.\n");
        }
        else
        {
            info("It is not key frame.\n");
            return 1; //第一帧不是I帧
        }
    }

    //write data
    if((avi_file->buf_offset + 8 + length + is_align + 16)> REC_BUF_SIZE) //偏移量
    {
        error("REC_BUF_SIZE is too small. avi_file->buf_offset = %d \n", avi_file->buf_offset);
        //return -1;
        goto write_data;
    }
	//写 00dc : 表示视频帧， 01wb: 表示音频帧
    if(intra == 1 || intra == 0 || intra == 3 )
    {
    	if(1 == avi_file->event_buf_flag)
			avi_set_32( &(event_data_array[avi_file->buf_offset]), MAKE_FOURCC('0','0','d','c'));//	 4 字节
		if(1 == avi_file->normal_buf_flag)
			avi_set_32( &(normal_data_array[avi_file->buf_offset]), MAKE_FOURCC('0','0','d','c'));//	 4 字节
    }
    else if(intra == 2)
    {	
    	if(1 == avi_file->event_buf_flag)
        	avi_set_32( &(event_data_array[avi_file->buf_offset]), MAKE_FOURCC('0','1','w','b'));
		if(1 == avi_file->normal_buf_flag)
			avi_set_32( &(normal_data_array[avi_file->buf_offset]), MAKE_FOURCC('0','1','w','b'));
		
    }
    else
    {
        error("frame type error.\n");
        return -1;
    }
	// 写数据长度
	if(1 == avi_file->event_buf_flag)
    	avi_set_32( &(event_data_array[avi_file->buf_offset + 4]), length + is_align);
	if(1 == avi_file->normal_buf_flag)
    	avi_set_32( &(normal_data_array[avi_file->buf_offset + 4]), length + is_align);
	avi_file->buf_offset += 8 ;

	//写数据流
	if(1 == avi_file->event_buf_flag)
		memcpy(&event_data_array[avi_file->buf_offset],data,length);
	if(1 == avi_file->normal_buf_flag)
		memcpy(&normal_data_array[avi_file->buf_offset],data,length);
		
    avi_file->buf_offset += length + is_align;

    //下面用到16个字节
    #if 1
	if(1 == avi_file->event_buf_flag)
	{
	    avi_set_32( &(event_data_array[avi_file->buf_offset]), MAKE_FOURCC('J','U','N','K'));
	    avi_set_32( &(event_data_array[avi_file->buf_offset + 4]), 8);
	    avi_set_32( &(event_data_array[avi_file->buf_offset + 8]), pTimeval->tv_sec);
	    avi_set_32( &(event_data_array[avi_file->buf_offset + 12]), pTimeval->tv_usec);
	}
	if(1 == avi_file->normal_buf_flag)
	{
		  avi_set_32( &(normal_data_array[avi_file->buf_offset]), MAKE_FOURCC('J','U','N','K'));
		  avi_set_32( &(normal_data_array[avi_file->buf_offset + 4]), 8);
		  avi_set_32( &(normal_data_array[avi_file->buf_offset + 8]), pTimeval->tv_sec);
		  avi_set_32( &(normal_data_array[avi_file->buf_offset + 12]), pTimeval->tv_usec);
	}
	
    avi_file->buf_offset += 16;
	#endif
	//当前计数器，计算够15帧，一次性写入
    avi_file->buf_index ++;

    if(intra == 1 || intra == 0 || intra == 3)
    {
        avi_set_32( &avi_file->idx_array[4*avi_file->pre_index_pos+0], MAKE_FOURCC('0','0','d','c'));
        avi_set_32( &avi_file->idx_array[4*avi_file->pre_index_pos+1], (intra) ? 0x11:0x10);
        avi_file->video_count++;
		//临时增加的
		avi_set_32( &avi_file->idx_array[4*avi_file->pre_index_pos+2], avi_file->data_offset /*- sizeof(AviHeader)+4*/);
		avi_set_32( &avi_file->idx_array[4*avi_file->pre_index_pos+3], length + is_align);
    }
    else 
    {
        avi_set_32( &avi_file->idx_array[4*avi_file->pre_index_pos+0], MAKE_FOURCC('0','1','w','b'));
        avi_set_32( &avi_file->idx_array[4*avi_file->pre_index_pos+1], 0x00);
        avi_file->audio_bytes += length+is_align;
		avi_file->audio_count++;
		
		//临时增加的
		avi_set_32( &avi_file->idx_array[4*avi_file->pre_index_pos+2], avi_file->data_offset /*- sizeof(AviHeader)+4*/);
		avi_set_32( &avi_file->idx_array[4*avi_file->pre_index_pos+3], length + is_align);

		
    }
    avi_file->data_offset += (8+length+is_align+16); //下一帧数据的偏移地址
	//所有的帧数
	avi_file->index_count++;
	avi_file->pre_index_pos++;
	//缓存15帧 然后一次性写入
    if( avi_file->buf_index == DATA_SAVE_NUM )
	{
write_data: 
		avi_file->buf_index = 0;
		long idxOffset;
		int idx_count = avi_file->pre_index_pos > DATA_SAVE_NUM?DATA_SAVE_NUM :avi_file->pre_index_pos;
		//写索引到临时文件 一次写16个索引
		ret = fwrite(avi_file->idx_array,	idx_count*ONE_INDEX_SIZE, 1, avi_file->idxfile);
		if(ret != 1)
		{
		error("fwrite failed.\n");
		return -1;
		}

		fflush(avi_file->idxfile);

		avi_file->pre_index_pos = 0 ; // 记录上一次写索引的偏移位置
		info("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXavi_file->buf_offset====%dB %dK,,pTimeval->tv_sec==%llu\n",avi_file->buf_offset,avi_file->buf_offset>>10,pTimeval->tv_sec);
		//此处的data_array_pos  已经记录间接记录数据量的大小了
		if(1 == avi_file->event_buf_flag)
			ret = fwrite(event_data_array, avi_file->buf_offset, 1, avi_file->file);
		if(1 == avi_file->normal_buf_flag)
			ret = fwrite(normal_data_array, avi_file->buf_offset, 1, avi_file->file);

		if(ret != 1)
		{
			error("fwrite failed.\n");
			return -1;
		}
		
		fflush(avi_file->file);
		if(1 == avi_file->event_buf_flag)
			memset(event_data_array,0x0,REC_BUF_SIZE);;
		if(1 == avi_file->normal_buf_flag)
			memset(normal_data_array,0x0,REC_BUF_SIZE);
		avi_file->buf_offset = 0;
	}
 
    return 0;
}

/*
intra  带便帧类型 0:P帧  1:I帧  2:音频帧  3:mjpeg
*/
inline int avi_record_write(AviFile *avi_file, char *data, int length, int intra, struct timeval *pTimeval)
{
    int ret = 0;
    ret = avi_write_frame_data(avi_file,data, length, intra, pTimeval);
    if(ret < 0)
    {
        //重新OPEN，重置flag
        avi_file->new_flag = 1;
		if(1 == avi_file->event_buf_flag)
        	memset(event_data_array, 0, sizeof(event_data_array));
		if(1 == avi_file->normal_buf_flag)
        	memset(normal_data_array, 0, sizeof(normal_data_array));
        avi_file->buf_offset = 0;
        avi_file->buf_index = 0;
		
		return -1;
    }

    return ret;
}

 
static int avi_write_frame_data_extend( AviFile *avi_file, char *data, int length_tmp, int intra, long long pTimeval)
{
    int ret = 0;
	int length = length_tmp;
    int is_align = length % 2;// 2字节对齐
	if(0 > g_sd_tats)	
	{
		error("avi_record_open:SD card Error!!\n");
		return -1;
	}
    if(!avi_file)
    {
        error("parameter error.\n");
        return -1;
    }
 
	//new_flag = 1 : 表示 第一次写AVI 文件; 第一帧数据必须是 I 帧
    if(avi_file->new_flag)  
    {
        if(intra == 1 || intra == 3) //第一帧数据，重命名文件
        {
            avi_file->file = fopen(avi_file->filename, "rb+");
            if(NULL == avi_file->file)
            {
                error("fopen %s failed.\n", avi_file->filename);
                return -1;
            }
            avi_file->data_offset = sizeof(AviHeader) ;//帧头在第一次打开的时候就已经写好了
            fseek(avi_file->file, avi_file->data_offset, SEEK_SET);
            avi_file->new_flag = 0;	
			printf(">>>>>>>>>>avi_file->new_flag.\n");
        }
        else
        {
            info("It is not key frame.\n");
            return 1; //第一帧不是I帧
        }
    }

    //write data
    if((avi_file->buf_offset + 8 + length + is_align + 16)> REC_BUF_SIZE) //偏移量
    {
        error("REC_BUF_SIZE is too small. avi_file->buf_offset = %d \n", avi_file->buf_offset);
        //return -1;
        goto write_data;
    }
	//写 00dc : 表示视频帧， 01wb: 表示音频帧
    if(intra == 1 || intra == 0 || intra == 3 )
    {
    	if(1 == avi_file->event_buf_flag)
			avi_set_32( &(event_data_array[avi_file->buf_offset]), MAKE_FOURCC('0','0','d','c'));//	 4 字节
		if(1 == avi_file->normal_buf_flag)
			avi_set_32( &(normal_data_array[avi_file->buf_offset]), MAKE_FOURCC('0','0','d','c'));//	 4 字节
    }
    else if(intra == 2)
    {	
    	if(1 == avi_file->event_buf_flag)
        	avi_set_32( &(event_data_array[avi_file->buf_offset]), MAKE_FOURCC('0','1','w','b'));
		if(1 == avi_file->normal_buf_flag)
			avi_set_32( &(normal_data_array[avi_file->buf_offset]), MAKE_FOURCC('0','1','w','b'));
		
    }
    else
    {
        error("frame type error.\n");
        return -1;
    }
	// 写数据长度
	if(1 == avi_file->event_buf_flag)
    	avi_set_32( &(event_data_array[avi_file->buf_offset + 4]), length + is_align);
	if(1 == avi_file->normal_buf_flag)
    	avi_set_32( &(normal_data_array[avi_file->buf_offset + 4]), length + is_align);
	avi_file->buf_offset += 8 ;

	//写数据流
	if(1 == avi_file->event_buf_flag)
		memcpy(&event_data_array[avi_file->buf_offset],data,length);
	if(1 == avi_file->normal_buf_flag)
		memcpy(&normal_data_array[avi_file->buf_offset],data,length);
		
    avi_file->buf_offset += length + is_align;

    //下面用到16个字节
    #if 1
	if(1 == avi_file->event_buf_flag)
	{
	    avi_set_32( &(event_data_array[avi_file->buf_offset]), MAKE_FOURCC('J','U','N','K'));
	    avi_set_32( &(event_data_array[avi_file->buf_offset + 4]), 8);
	    avi_set_32( &(event_data_array[avi_file->buf_offset + 8]), pTimeval);
	    avi_set_32( &(event_data_array[avi_file->buf_offset + 12]), pTimeval);
	}
	if(1 == avi_file->normal_buf_flag)
	{
		  avi_set_32( &(normal_data_array[avi_file->buf_offset]), MAKE_FOURCC('J','U','N','K'));
		  avi_set_32( &(normal_data_array[avi_file->buf_offset + 4]), 8);
		  avi_set_32( &(normal_data_array[avi_file->buf_offset + 8]), pTimeval);
		  avi_set_32( &(normal_data_array[avi_file->buf_offset + 12]), pTimeval);
	}
	
    avi_file->buf_offset += 16;
	#endif
	//当前计数器，计算够15帧，一次性写入
    avi_file->buf_index ++;

    if(intra == 1 || intra == 0 || intra == 3)
    {
        avi_set_32( &avi_file->idx_array[4*avi_file->pre_index_pos+0], MAKE_FOURCC('0','0','d','c'));
        avi_set_32( &avi_file->idx_array[4*avi_file->pre_index_pos+1], (intra) ? 0x11:0x10);
        avi_file->video_count++;
		//临时增加的
		avi_set_32( &avi_file->idx_array[4*avi_file->pre_index_pos+2], avi_file->data_offset /*- sizeof(AviHeader)+4*/);
		avi_set_32( &avi_file->idx_array[4*avi_file->pre_index_pos+3], length + is_align);
    }
    else 
    {
        avi_set_32( &avi_file->idx_array[4*avi_file->pre_index_pos+0], MAKE_FOURCC('0','1','w','b'));
        avi_set_32( &avi_file->idx_array[4*avi_file->pre_index_pos+1], 0x00);
        avi_file->audio_bytes += length+is_align;
		avi_file->audio_count++;
		
		//临时增加的
		avi_set_32( &avi_file->idx_array[4*avi_file->pre_index_pos+2], avi_file->data_offset /*- sizeof(AviHeader)+4*/);
		avi_set_32( &avi_file->idx_array[4*avi_file->pre_index_pos+3], length + is_align);

		
    }

	avi_file->data_offset += (8+length+is_align+16); //下一帧数据的偏移地址
	//所有的帧数
	avi_file->index_count++;
	avi_file->pre_index_pos++;
	//缓存15帧 然后一次性写入
    if( avi_file->buf_index == DATA_SAVE_NUM )
    {
 write_data: 
		avi_file->buf_index = 0;
		long idxOffset;
		int idx_count = avi_file->pre_index_pos > DATA_SAVE_NUM?DATA_SAVE_NUM :avi_file->pre_index_pos;
		//写索引到临时文件 一次写16个索引
		ret = fwrite(avi_file->idx_array,	idx_count*ONE_INDEX_SIZE, 1, avi_file->idxfile);
		if(ret != 1)
		{
			error("fwrite failed.\n");
			return -1;
		}


		fflush(avi_file->idxfile);
		avi_file->pre_index_pos = 0 ; // 记录上一次写索引的偏移位置
		//info("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXavi_file->buf_offset==== %dK,,pTimeval->tv_sec==%llu\n",avi_file->buf_offset>>10,pTimeval);
		//此处的data_array_pos  已经记录间接记录数据量的大小了
		if(1 == avi_file->event_buf_flag)
			ret = fwrite(event_data_array, avi_file->buf_offset, 1, avi_file->file);
		if(1 == avi_file->normal_buf_flag)
			ret = fwrite(normal_data_array, avi_file->buf_offset, 1, avi_file->file);
		if(ret != 1)
		{
			error("fwrite failed.\n");
			return -1;
		}
		
		fflush(avi_file->file);
		if(1 == avi_file->event_buf_flag)
			memset(event_data_array,0x0,REC_BUF_SIZE);;
		if(1 == avi_file->normal_buf_flag)
			memset(normal_data_array,0x0,REC_BUF_SIZE);
		avi_file->buf_offset = 0;
    }
 
    return 0;
}

/*
intra  带便帧类型 0:P帧  1:I帧  2:音频帧  3:mjpeg
*/
inline int avi_record_write_extend(AviFile *avi_file, char *data, int length, int intra, long long pTimeval)
{
    int ret = 0;
    ret = avi_write_frame_data_extend(avi_file,data, length, intra, pTimeval);
    if(ret < 0)
    {
        //重新OPEN，重置flag
        avi_file->new_flag = 1;
		if(1 == avi_file->event_buf_flag)
        	memset(event_data_array, 0, sizeof(event_data_array));
		if(1 == avi_file->normal_buf_flag)
        	memset(normal_data_array, 0, sizeof(normal_data_array));
        avi_file->buf_offset = 0;
        avi_file->buf_index = 0;
		
		return -1;
    }

    return ret;
}

int avi_record_event_close(AviFile *avi_file)
{
    int ret;
    char oldname[128];
    char time_stop[30];
	char newname[128] = {0};

    if(!avi_file)
    {
        error("parameter is error.\n");
        return -1;
    }
	if(0 > g_sd_tats)	
	{
		error("avi_record_event_close:SD card Error!!\n");
		return -1;
	}


	
    if(avi_file->new_flag == 0)
    {
        //get stop time to rename file name.
        memset(time_stop, 0, sizeof(time_stop));

      
        get_nowtime_str(time_stop);
	
        //将buff里面最后的数据写入AVI文件中
        if(avi_file->buf_offset != 0)
        {
            printf("%s[%d]......avi_file->buf_offset=%d, avi_file->pre_index_pos=%d, %d\n", __FUNCTION__, __LINE__,
                   avi_file->buf_offset, avi_file->pre_index_pos, avi_file->index_count);
			
			if(1 == avi_file->event_buf_flag)
            	ret = fwrite(event_data_array, avi_file->buf_offset, 1, avi_file->file);
			
			if(1 == avi_file->normal_buf_flag)
				ret = fwrite(normal_data_array, avi_file->buf_offset, 1, avi_file->file);
            if(ret != 1)
            {
                error("fwrite failed.\n");
                return -1;
            }

            avi_file->buf_index = 0;
            avi_file->buf_offset = 0;
        }
    
        ret = avi_add_index(avi_file);
        if(ret < 0)
        {
            error("call avi_add_index failed.\n");
            return -1;
        }

        memset(oldname, 0, sizeof(oldname));
#if 1
		char firstname[36];
		char *lastname = NULL;
		char newname[72];
		memset(firstname, 0, 23);
		memset(newname, 0,40);

		strncpy(firstname, avi_file->filename, 22);//:/tmp/sd_card/20150817/
		lastname = strstr(avi_file->filename, "ch");//lastname: 20150817034755

		sprintf(newname , "%sal%s",firstname, lastname + 2);

#endif
		strncpy(oldname, avi_file->filename, sizeof(oldname));   // :/tmp/sd_card/20150817/ch0_20150817034755

		sprintf(avi_file->filename, "%s_%llu.avi", newname,  //oldname: /tmp/sd_card/20150817/ch0_20150817034755_20120101011101.avi
                avi_file->file_info.stop);


		
        ret = rename(oldname, avi_file->filename);
        if(ret)
        {
            error("rename %s to %s failed.\n", oldname, avi_file->filename);
            return -1;
        }
        info("3. Record file is %s\n", avi_file->filename);

         
    }

    avi_file->pre_index_pos = 0;
    //close & free
    if(avi_file->file)
    {
        fclose(avi_file->file);
        avi_file->file = NULL;
    }

	 if(avi_file->idxfile)
    {
        fclose(avi_file->idxfile);
        avi_file->idxfile = NULL;
    }

    char cmd[64];
    sprintf(cmd, "rm -f %s", avi_file->idxfilename);
	system(cmd);
	
	avi_file->event_buf_flag = 0;
	info("4. leaving avi_record_event_close ...\n");
    return 0;
}

int avi_record_nornal_close(AviFile *avi_file)
{
    int ret;
    char oldname[128];
	int record_duration;

    if(!avi_file)
    {
        error("parameter is error.\n");
        return -1;
    }
	if(0 > g_sd_tats)	
	{
		error("avi_record_close:SD card Error!!\n");
		return -1;
	}


    if(avi_file->new_flag == 0)
    {	
        //将buff里面最后的数据写入AVI文件中
        if(avi_file->buf_offset != 0)
        {
            printf("%s[%d]......avi_file->buf_offset=%d, avi_file->pre_index_pos=%d, %d\n", __FUNCTION__, __LINE__,
                   avi_file->buf_offset, avi_file->pre_index_pos, avi_file->index_count);
			
			if(1 == avi_file->event_buf_flag)
            	ret = fwrite(event_data_array, avi_file->buf_offset, 1, avi_file->file);
			
			if(1 == avi_file->normal_buf_flag)
				ret = fwrite(normal_data_array, avi_file->buf_offset, 1, avi_file->file);
            if(ret != 1)
            {
                error("fwrite failed.\n");
                return -1;
            }
            avi_file->buf_index = 0;
            avi_file->buf_offset = 0;
        }
  
        ret = avi_add_index(avi_file);
        if(ret < 0)
        {
            error("call avi_add_index failed.\n");
            return -1;
        }
		
		if(avi_file->idxfile)
		{
			fclose(avi_file->idxfile);
			avi_file->idxfile = NULL;
		}
		
		ret = remove(avi_file->idxfilename);
		if(ret != 0)
		{
			info("remove index error :avi_file->idxfilename =%s\n",avi_file->idxfilename);
			return -1;
		}
		else
		{
		   
		    info("remove index success :avi_file->idxfilename =%s\n",avi_file->idxfilename);
		}

		record_duration = avi_file->duration/1000;//TIMEVAL_DURATION(avi_file->file_info.start, avi_file->file_info.stop);
        memset(oldname, 0, sizeof(oldname));
        strncpy(oldname, avi_file->filename, sizeof(oldname));   //ch0_20120101010101
        sprintf(avi_file->filename, "%s-%d.avi", oldname, record_duration);
		
        ret = rename(oldname, avi_file->filename);
        if(ret)
        {
            error("rename %s to %s failed.\n", oldname, avi_file->filename);
            return -1;
        }
        info("3. Record file is %s\n", avi_file->filename);

    }

    avi_file->pre_index_pos = 0;
    //close & free
    if(avi_file->file)
    {
        fclose(avi_file->file);
        avi_file->file = NULL;
    }

	avi_file->normal_buf_flag = 0;

    return 0;
}
/************/
int record_set_start_time(AviFile *avi_file,int start_second)
{
	unsigned long ultimesec = get_UTC_Time();
	avi_file->file_info.start = ultimesec;
    return 0;
}

int record_set_end_time(AviFile *avi_file,int end_second)
{
	unsigned long ultimesec = get_UTC_Time();
	avi_file->file_info.stop = ultimesec;

    return 0;
}
/*

 * _record_event_stop
 
 *   在事件录像结束时，调用单独的关闭接口，重新命名事件录像，以区分常规录像

 */

int _record_event_stop(AviFile *avi_file)
{
    int ret = 0;
    ret = avi_record_event_close(avi_file);
    //write_manage_info(avi_file->record_type, avi_file->file_info.ch_num,
    //                  avi_file->file_info.start, avi_file->file_info.stop);

    return ret;
}


int _record_normal_stop(AviFile *avi_file)
{
    int ret = 0;
    ret = avi_record_nornal_close(avi_file);
    //write_manage_info(avi_file->record_type, avi_file->file_info.ch_num,
    //                  avi_file->file_info.start, avi_file->file_info.stop);

	//write_record_node_index(0, avi_file->file_info.start, avi_file->file_info.stop);
    return ret;
}

