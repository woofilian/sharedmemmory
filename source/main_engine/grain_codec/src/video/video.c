 /******************************************************************************

                  版权所有 (C), 2012-2022

 ******************************************************************************
  文 件 名   : video.c
  版 本 号   : v1.0
  功能描述   : 视频数据的初始化采集
  函数列表   :
              video_enc_init
              video_get_chipinfo
              video_sys_init
  修改历史   :
    修改内容   : 创建文件

******************************************************************************/

//video_capture

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include "gmlib.h"
#include "sdk_struct.h"
#include "video.h"
#include "log.h"
#include <sys/ioctl.h>

#include "ioctl_isp328.h"
#include "shm_common.h"
#include "shm_write.h"
#include "shm_read.h"


//#define VIDEO_TEST 1

#define TRUE	1
#define FALSE    0


/***********************/
static gm_system_t g_grain_system;
av_info_t	g_av_info;
static int  g_video_capture = 0;
static int  g_osd_capture = 0;
static int  g_motion_capture = 0;


int g_timemode = 0;
int g_osd_show = 0;
int g_channelname_show = 0;
int g_record_show = 0;
int g_osd_time_zone=0;
int time_zone_change=0;
char  username[64];
int  VGA_flag = 0;
extern SendstreamCallback	mVideoStreamCB ;  //视频回调
video_info_t  *pvideo_info;
/************osd**************/
//#define LOGO_YUV422_PATTERNS     "/mnt/nfs/col_YUV422.bin"//"/mnt/nfs/logo_32x32_yuv422.bin"//
#define LOGO_YUV422_PATTERNS     "/mnt/nfs/raw2.yuv"//"/mnt/nfs/logo_32x32_yuv422.bin"//

#define LOGO_WIDTH              64
#define LOGO_HEIGHT             64

/************osg**************/
#define OSG_LOGO_RGB1555_PATTERNS     "/mnt/nfs/osg_16x72_rgb1555.bin"//"/mnt/nfs/col_RGB.bin" 
#define OSG_LOGO_WIDTH               16
#define OSG_LOGO_HEIGHT              72

float temperatureC;
float humidityRH;

static int ir_cut_delay_flag;
static int ircut_change_time;

/**********************/
static char *video_type_str[] =
{
    "SDK_VIDEO_FORMAT_H264 ",
    "SDK_VIDEO_FORMAT_MPEG4",
    "SDK_VIDEO_FORMAT_MJPEG"
};


static int video_get_chipinfo(void)
{
    FILE *fp;
    char buffer[256];
    int i;
    int chipid;
    char *match;

    fp = fopen("/proc/pmu/chipver","r");
    i = fread(buffer,1,sizeof(buffer),fp);
    fclose(fp);
    if (i == 0)
        return 0;
    buffer[i] = '\0';
    match = strstr(buffer, "81");
    if (match == NULL)
        return 0;
    sscanf(match,"%X",&chipid);
    return chipid;
}

int video_sys_init()
{
    int chipid;
    int i;
    gm_init(); //gmlib initial
    gm_get_sysinfo(&g_grain_system);
    chipid = video_get_chipinfo();

    info("video_sys_init chip ID:%x \n",chipid);
    memset(&g_av_info,0,sizeof(av_info_t));
    g_av_info.max_audio_stream_num = 1;
    g_av_info.max_video_stream_num = 2;
    g_av_info.video_groupfd = gm_new_groupfd();
    ///g_av_info.audio_groupfd = gm_new_groupfd();

    return 0;
}

int video_sys_uninit()
{
    gm_release();
}

//mult-channel only support h264  多路码流的时候只支持H264模式
int video_enc_init(int ch, sdk_av_enc_t *pav_enc)
{
    int width = 0, height = 0;

    if(ch < 0 || ch > MAX_VIDEO_STREAM_NUM || (!pav_enc))
    {
        error("ch > MAX_VIDEO_STREAM_NUM || (!pav_enc) \n");
        return -1;
    }
    int enc_type =  pav_enc->video_type ;//SDK_VIDEO_FORMAT_E
    if(enc_type != SDK_VIDEO_FORMAT_H264 && ch > 0 )
    {
        error("enc_type != SDK_VIDEO_FORMAT_H264 && ch > 0 \n");
        return -1;
    }
    pvideo_info = &g_av_info.video_info[ch];
    //if (pvideo_info->capture_object == NULL)
    //{
        DECLARE_ATTR(cap_attr, gm_cap_attr_t);
        DECLARE_ATTR(dnr_attr, gm_3dnr_attr_t); // 降噪

    pvideo_info->capture_object = gm_new_obj(GM_CAP_OBJECT); // new capture object
    cap_attr.cap_vch = 0;  //默认为0

    //GM813x capture path 0(liveview), 1(substream), 2(substream), 3(mainstream)
    //GM8139/GM8287 capture path 0(liveview), 1(CVBS), 2(can scaling), 3(can't scaling down)

    cap_attr.path = (ch == 0) ? 2:0;
    cap_attr.enable_mv_data = (ch == 0) ? 1:0; //第 0 通道移动侦测打开

    gm_set_attr(pvideo_info->capture_object, &cap_attr); // set capture attribute

    pvideo_info->video_object = gm_new_obj(GM_ENCODER_OBJECT); // new encoder object
    pvideo_info->video_type = enc_type;

    resolution_2_pic(0,pav_enc->resolution, &pvideo_info->video_width, &pvideo_info->video_height);
	printf("path=%d | res=%d x %d", cap_attr.path, pvideo_info->video_width, pvideo_info->video_height);

	pvideo_info->frame_rate = pav_enc->frame_rate;
    if(!pvideo_info->video_buf )
    {
        pvideo_info->video_buf_len = pvideo_info->video_width*pvideo_info->video_height*3/2;
        pvideo_info->video_buf     = malloc(pvideo_info->video_buf_len);
        memset(pvideo_info->video_buf, 0, pvideo_info->video_buf_len);

    }
	dnr_attr.enabled = 1;            
	gm_set_attr(pvideo_info->capture_object, &dnr_attr);
	
    DECLARE_ATTR(h264e_attr, gm_h264e_attr_t);
    switch (enc_type)
    {
    case SDK_VIDEO_FORMAT_H264:
        h264e_attr.dim.width = pvideo_info->video_width;
        h264e_attr.dim.height = pvideo_info->video_height;
        h264e_attr.frame_info.framerate = pav_enc->frame_rate;//帧率
        h264e_attr.ratectl.mode = pav_enc->bitrate_type + 1;  //GM_CBR
        h264e_attr.ratectl.gop = pav_enc->gop;
        h264e_attr.ratectl.bitrate = pav_enc->bitrate;
        h264e_attr.ratectl.bitrate_max = pav_enc->bitrate;
        h264e_attr.b_frame_num = 0;  // B-frames per GOP (H.264 high profile)
        h264e_attr.enable_mv_data = 0;  // disable H.264 motion data output
        h264e_attr.ratectl.init_quant = 25;
        h264e_attr.ratectl.min_quant = 14;
        h264e_attr.ratectl.max_quant = 51;
		h264e_attr.profile_setting.profile = 100;
        //error("111111111111111111 \n");
        gm_set_attr(pvideo_info->video_object, &h264e_attr);
        /// memcpy(&param->enc[rec_track].codec.h264e_attr, &h264e_attr, sizeof(gm_h264e_attr_t));//最终设置参数的位置
        break;
#if 0
    case SDK_VIDEO_FORMAT_MPEG4:
        DECLARE_ATTR(mpeg4e_attr, gm_mpeg4e_attr_t);
        mpeg4e_attr.dim.width = width;
        mpeg4e_attr.dim.height = height;
        mpeg4e_attr.frame_info.framerate = framerate;
        mpeg4e_attr.ratectl.mode = mode;
        mpeg4e_attr.ratectl.gop = 60;
        mpeg4e_attr.ratectl.bitrate = bitrate;
        mpeg4e_attr.ratectl.bitrate_max = bitrate;
        gm_set_attr(param->enc[rec_track].obj, &mpeg4e_attr);
        memcpy(&param->enc[rec_track].codec.mpeg4e_attr, &mpeg4e_attr, sizeof(gm_mpeg4e_attr_t));
        break;
    case SDK_VIDEO_FORMAT_MJPEG:
        DECLARE_ATTR(mjpege_attr, gm_mjpege_attr_t);
        mjpege_attr.dim.width = width;
        mjpege_attr.dim.height = height;
        mjpege_attr.frame_info.framerate = framerate;
        mjpege_attr.quality = 30;
        gm_set_attr(param->enc[rec_track].obj, &mjpege_attr);
        memcpy(&param->enc[rec_track].codec.mjpege_attr, &mjpege_attr, sizeof(gm_mjpege_attr_t));
        break;
#endif
    default:
        warning("Not support enc_type %d \n", enc_type);
        break;
    }
    // bind channel recording
    //抓取通道  和编码通道绑定
    warning("gm_bind ----------------------?>Jensen\n");
    pvideo_info->video_fd = gm_bind(g_av_info.video_groupfd, pvideo_info->capture_object, pvideo_info->video_object);
    return 0;
}

int video_enc_uninit(int ch)
{
    video_info_t  *pvideo_info = &g_av_info.video_info[ch];

    gm_unbind(pvideo_info->video_fd);
    gm_apply(g_av_info.video_groupfd);

    gm_delete_obj(pvideo_info->video_object);
    gm_delete_obj(pvideo_info->capture_object);

    if(pvideo_info->video_buf)
        free(pvideo_info->video_buf);
    pvideo_info->video_buf = NULL;
    pvideo_info->video_buf_len = 0;
    return 0;

}
//获取绑定句柄句柄
int get_video_bind_handle(int ch,int sub_ch)
{
    video_info_t  *pvideo_info = &g_av_info.video_info[0];
    //warning("g_av_info.video_info[0].video_fd  [%d]  pvideo_info->video_fd %d\n",g_av_info.video_info[0].video_fd,pvideo_info->video_fd);
    return pvideo_info->video_fd;

}
void *get_video_object(int ch,int sub_ch)
{
    video_info_t  *pvideo_info = &g_av_info.video_info[0];
    return pvideo_info->video_object;
}

int get_sys_info(gm_system_t *grain_system)
{
    if(!grain_system)
        return -1;

    memset(grain_system,0,sizeof(gm_system_t));
    memcpy(grain_system,&g_grain_system,sizeof(gm_system_t));
    return  0;
}

static int video_capture_start()
{
    int ret = -1;
    ret = gm_apply(g_av_info.video_groupfd); // active setting
    if(ret < 0 )
    {
        info("video_capture_start failed!!! \n");
    }

}
/***********************************/
#ifdef VIDEO_TEST
h264_write_fd[3] = { -1};
static int init_write_fd()
{
    int i;
    char filename[50];
    for(i = 0 ; i < 2 ; i++)
    {
        sprintf(filename, "/mnt/nfs/stream%d.264", i);
        h264_write_fd[i] = open(filename,  O_WRONLY | O_CREAT, 0644);
    }
    return 0;
}


static int write_video_data(int ch,const char *video_data,int data_len)
{
    if(ch > 2)
        return -1;

    int ret =  write(h264_write_fd[ch],video_data,data_len);
    if(ret < 0)
    {
        error("write_video_data !!! ret = %d \n",ret);
    }
    sync();

}
#endif
static int g_frame_no[MAX_VIDEO_STREAM_NUM] = {0};
inline unsigned int get_time_stamp(struct timeval tv)
{
    return (tv.tv_sec*1000 + tv.tv_usec/1000);
}

static inline int write_video_streaming(int ch, gm_enc_bitstream_t *bs)
{
#if 1
    struct timeval tv;
    video_info_t  *pvideo_info = &g_av_info.video_info[ch];

    sdk_stream_info_t stream_info;
    memset(&stream_info,0,sizeof(sdk_stream_info_t));
    stream_info.ch = ch ;
    stream_info.sub_ch = 0;

    stream_info.frame_head.frame_type = (bs->keyframe == 1)? (SDK_VIDEO_FRAME_I):(SDK_VIDEO_FRAME_P);
    stream_info.frame_head.frame_size = bs->bs_len;
    stream_info.frame_head.frame_no = g_frame_no[ch]++;
    gettimeofday(&tv, NULL);
    stream_info.frame_head.sec = tv.tv_sec;				   //帧时间（秒）
    stream_info.frame_head.usec = tv.tv_usec;              //帧时间（微秒）
    stream_info.frame_head.pts =  bs->timestamp;		   //这个时间戳不准
     ////stream_info.frame_head.pts =  tv.tv_sec *1000 + tv.tv_usec/1000;	
	//printf(" write_video_streaming stream_info.frame_head.sec   %d ,stream_info.frame_head.usec %d ,stream_info.frame_head.pts %d\n",stream_info.frame_head.sec,stream_info.frame_head.usec,stream_info.frame_head.pts);
    stream_info.frame_addr = (const char *)(bs->bs_buf);
    stream_info.frame_head.video_info.encode_type = 0;
    stream_info.frame_head.video_info.standard = 0;

    //stream_info.frame_head.video_info.resolution= g_video_info[ch_type].resolution;
    stream_info.frame_head.video_info.width = pvideo_info->video_width;
    stream_info.frame_head.video_info.height= pvideo_info->video_height;
    stream_info.frame_head.video_info.frame_rate = pvideo_info->frame_rate;//把相关的信息写给流库


    if(mVideoStreamCB)
        mVideoStreamCB((void *)&stream_info);
    return 0;
#endif

}

void COMM_GetSystemUpMSecs(unsigned long long *pullTimeStamp)
{
	struct timespec structTimeSpec;
	clock_gettime(CLOCK_MONOTONIC,&structTimeSpec);
	*pullTimeStamp = (unsigned long long)structTimeSpec.tv_sec*1000 + (unsigned long long)structTimeSpec.tv_nsec/1000000;	
}


typedef struct tagShmVideoProfileInfo
{
    unsigned char cProfileNO;
    unsigned int iShmSize;
    unsigned int iMaxFrameSize;
}ShmVideoProfileInfo;

int VE_VideoShmInit(int iShmProfileID)
{	
    ShmVideoHeaderInfo structShmVideoHeaderInfo;	
    ShmVideoProfileInfo structShmVideoProfileInfo;	
    int ret;	
    int iRes;
    int iUnionRes;
    
    memset(&structShmVideoProfileInfo, -1, sizeof(structShmVideoProfileInfo));	
    memset(&structShmVideoHeaderInfo, -1, sizeof(structShmVideoHeaderInfo));	
    
    structShmVideoHeaderInfo.ucMediaType = SHM_ENUM_MEDIA_TYPE_VIDEO; //video flag	
    structShmVideoHeaderInfo.ucCodec = SHM_ENUM_VIDEO_CODEC_TYPE_H264;

    if(iShmProfileID==SHM_ENUM_VIDEO_STREAM0_PROFILE)  
	    //iUnionRes = (1280+ (720<<16 ));
	    iRes = SDK_VIDEO_RESOLUTION_720p;
    else if(iShmProfileID==SHM_ENUM_VIDEO_STREAM1_PROFILE)		
        //iUnionRes = (640 + (480<<16 ));
        iRes = SDK_VIDEO_RESOLUTION_VGA;
    else if(iShmProfileID==SHM_ENUM_VIDEO_STREAM2_PROFILE)		
        //iUnionRes = ENUM_VIDEO_UNION_RES_QVGA;
        iRes = SDK_VIDEO_RESOLUTION_QVGA;
    //comm_res_union2int(iUnionRes, &iRes);
    structShmVideoHeaderInfo.ucResolution = (unsigned char)iRes;	
    structShmVideoHeaderInfo.ucQuality = 3;	
    
    structShmVideoProfileInfo.cProfileNO = iShmProfileID;
    
	if(iShmProfileID==SHM_ENUM_VIDEO_STREAM0_PROFILE)  
	    structShmVideoProfileInfo.iShmSize = SHM_VIDEO_STREAM_720P_SIZE;	
    else if(iShmProfileID==SHM_ENUM_VIDEO_STREAM1_PROFILE)		
        structShmVideoProfileInfo.iShmSize = SHM_VIDEO_STREAM_VGA_SIZE;
    //else if(iShmProfileID==SHM_ENUM_VIDEO_STREAM2_PROFILE)		
    //    structShmVideoProfileInfo.iShmSize = SHM_VIDEO_STREAM_QVGA_SIZE;
    
    structShmVideoProfileInfo.iMaxFrameSize = SHM_VIDEO_STREAM_720P_MAX_FRAME_SIZE;
    
    ret=SHM_VideoInitWrite(structShmVideoProfileInfo.cProfileNO, 
        structShmVideoProfileInfo.iShmSize,         
        structShmVideoProfileInfo.iMaxFrameSize,        
        &structShmVideoHeaderInfo); 
    if (ret < 0)    
    {       
        printf("video0 share memory init failed\n");        
        return -1;  
    }

    return 0;
}


/***************************************************************************
input data to the video share memory
*/
int VE_VideoEngineInput(int iStreamID, unsigned char ucIsKeyFrame, char* pBuf, int iLen, unsigned int uitv_ms)
{
    ShmVideoExtraData structShmVideoExtras;
	//static unsigned char sucPreAudioEnable = 0;
	//static unsigned char sucAudioChangeCount = 0;
	
	memset(&structShmVideoExtras, -1, sizeof(structShmVideoExtras));

    if(iStreamID == 0)  
	    structShmVideoExtras.nbitsRes = SDK_VIDEO_RESOLUTION_720p;//ENUM_VIDEO_BITS_RES_720P;	
    //else if(iStreamID == 1)		
        //structShmVideoExtras.nbitsRes = ENUM_VIDEO_BITS_RES_VGA;
    //else if(iStreamID == 2)		
    //    structShmVideoExtras.nbitsRes = ENUM_VIDEO_BITS_RES_QVGA;


	//if (sucPreAudioEnable != structShmVideoExtras.ucAudioEnable)
	//	sucAudioChangeCount++;

	//sucPreAudioEnable = structShmVideoExtras.ucAudioEnable;
	//structShmVideoExtras.ucAudioChangedCount = sucAudioChangeCount;			
	structShmVideoExtras.ucKeyFrameFlag = (ucIsKeyFrame==1)?1:0;

	unsigned long long ullcurtime;
	COMM_GetSystemUpMSecs(&ullcurtime);
    structShmVideoExtras.ullTimeStamp = ullcurtime;
	
	static unsigned int cc=0;
	cc++;
	//if(cc % 2000 == 0) printf("%d     ^^^video timestamp: %llu\n", iStreamID, structShmVideoExtras.ullTimeStamp);

	if(iLen>256*1024)
	{
		int iii=0;
		for(iii=0; iii<100; iii++)
		printf("      overflowed!!!! frame is too large ucIsKeyFrame=%d, len=%d\n", ucIsKeyFrame, iLen);
	}

    SHM_VideoWriteFrame(iStreamID, pBuf, iLen, &structShmVideoExtras);

    //VE_FrameStat(iShmProfileID, iLen, ucIsKeyFrame);		
    return 0;
}


static void *video_capture_thread(void *arg)
{
    int ret = 0;
    int i = 0 ;
    video_info_t  *pvideo_infos = NULL;
    gm_enc_multi_bitstream_t *pmulti_bs = NULL;
    char *bitstream_data[MAX_VIDEO_STREAM_NUM];
    gm_pollfd_t poll_fds[MAX_VIDEO_STREAM_NUM];
    gm_enc_multi_bitstream_t multi_bs[MAX_VIDEO_STREAM_NUM];

    memset(poll_fds, 0, sizeof(poll_fds));
    /*** Capture mainstream stream */
    poll_fds[0].bindfd = g_av_info.video_info[0].video_fd;
    poll_fds[0].event = GM_POLL_READ;
    /*** Capture another path substream */
    //poll_fds[1].bindfd = g_av_info.video_info[1].video_fd;
    //poll_fds[1].event = GM_POLL_READ;
#if 1//JFG_API	
	int streamVID = SHM_ENUM_VIDEO_STREAM0_PROFILE;
	if(VE_VideoShmInit(streamVID) < 0)
	{
		printf("****** Error: VideoShmInit failed......\n");
	}
#endif	
    g_video_capture = 1;
    warning ("g_av_info.video_info[0].video_fd   is  >>>>>>>[%d]\n",g_av_info.video_info[0].video_fd);
    while(g_video_capture)
    {
        //warning(" g_av_info.video_info[0].video_fd  is  [%d]\n", g_av_info.video_info[0].video_fd);
        /** poll bitstream until 500ms timeout */
        ret = gm_poll(poll_fds, MAX_VIDEO_STREAM_NUM, 500);
        if (ret == GM_TIMEOUT)
        {
            info("Poll timeout!! \n");
            continue;
        }
        memset(multi_bs, 0, sizeof(multi_bs));  //clear all mutli bs
        for (i = 0; i < MAX_VIDEO_STREAM_NUM; i++)
        {
            if (poll_fds[i].revent.event != GM_POLL_READ)
                continue;
            if (poll_fds[i].revent.bs_len > g_av_info.video_info[i].video_buf_len)
            {
                printf("bitstream buffer length is not enough! %d, %d\n",
                       poll_fds[i].revent.bs_len, g_av_info.video_info[i].video_buf_len);
                continue;
            }
            multi_bs[i].bindfd = poll_fds[i].bindfd;
            multi_bs[i].bs.bs_buf = g_av_info.video_info[i].video_buf;  // set buffer point(指定输出缓冲指针位置)
            multi_bs[i].bs.bs_buf_len = g_av_info.video_info[i].video_buf_len;  // set buffer length(指定缓冲长度)
            multi_bs[i].bs.mv_buf = 0;  // not to recevie MV data
            multi_bs[i].bs.mv_buf_len = 0;  // not to recevie MV data
        }

        if( (ret = gm_recv_multi_bitstreams(&multi_bs[0],MAX_VIDEO_STREAM_NUM)) < 0 )
        {
            // <=-1:fail, 0:success
            info("Error to receive bitstream. ret(%d)\n", ret);
            continue;  // while(1) {
        }

        for (i = 0; i < MAX_VIDEO_STREAM_NUM; i++)
        {
            pvideo_infos = &g_av_info.video_info[i];

            if (multi_bs[i].retval == GM_SUCCESS)
            {
                if (multi_bs[i].bs.newbs_flag & GM_FLAG_NEW_BITRATE)
                {
                    info("<CH%d, newbsflag=0x%x detect bitrate change>\n", i,
                         multi_bs[i].bs.newbs_flag);
                }
                if (multi_bs[i].bs.newbs_flag & GM_FLAG_NEW_FRAME_RATE)
                {
                    info("<CH%d, newbsflag=0x%x detect framerate change>\n", i,
                         multi_bs[i].bs.newbs_flag);
                }
                if (multi_bs[i].bs.newbs_flag & GM_FLAG_NEW_GOP)
                {
                    info("<CH%d, newbsflag=0x%x detect GOP change>\n", i,
                         multi_bs[i].bs.newbs_flag);
                }
#if 1//JFG_API	
				//printf("i=%d, multi_bs[i].bs.keyframe=%d, multi_bs[i].bs.bs_len=%d, multi_bs[i].bs.timestamp=%d\n",
				//i, multi_bs[i].bs.keyframe, multi_bs[i].bs.bs_len, multi_bs[i].bs.timestamp);
				if(multi_bs[i].bs.bs_len > 0)
				{
					VE_VideoEngineInput(streamVID, multi_bs[i].bs.keyframe, \
									multi_bs[i].bs.bs_buf, multi_bs[i].bs.bs_len, multi_bs[i].bs.timestamp);
				}
#endif
				
                write_video_streaming(i, &multi_bs[i].bs);

            }
            //print_enc_average(i, j, bs[i][j].bs.bs_len, &prev);
        }

    }
    info("thread exit......\n");
	
	SHM_VideoRemoveID(streamVID);
}




static pthread_t video_capture_id = 0;
///pthread_t encode_thread_id = 0;
int video_server_start()
{
    pthread_attr_t attr;
    int ret ;
#ifdef VIDEO_TEST
    init_write_fd();
#endif
    video_capture_start();
    /* Record Thread */
    if (video_capture_id == (pthread_t)NULL)
    {
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);  //分离的线程
        ret = pthread_create(&video_capture_id, &attr, &video_capture_thread, NULL);
        pthread_attr_destroy(&attr);
    }
    else
    {
        info("video_capture_thread is already runing..... \n");
    }

    return 0;
}

//停止取流 释放资源
int video_server_stop()
{
    g_video_capture = 0 ;
    return 0;
}

//强制I帧
int video_force_i_frame(int ch)
{
    int ret = -1;
    if(ch < 0 || ch > MAX_VIDEO_STREAM_NUM)
    {
        error("ch out of rang!!!  \n");
        return -1;
    }
    video_info_t  *pvideo_info = &g_av_info.video_info[ch];
    if(pvideo_info->video_fd < 0)
    {
        error("video_fd < 0   ch:%d \n",ch);
        return -1;
    }
    if(gm_request_keyframe(pvideo_info->video_fd) < 0)
    {
        error("gm_request_keyframe faile \n");
        return -1;
    }
}

int video_set_enc_attr(int ch,sdk_av_enc_t *pav_enc)
{
    int ret = -1;
    if(ch < 0 || ch > MAX_VIDEO_STREAM_NUM || (!pav_enc))
    {
        error("ch > MAX_VIDEO_STREAM_NUM || (!pav_enc) \n");
        return -1;
    }

    int enc_type =  pav_enc->video_type ;//SDK_VIDEO_FORMAT_E
    if(enc_type != SDK_VIDEO_FORMAT_H264 && ch > 0 )
    {
        error("enc_type != SDK_VIDEO_FORMAT_H264 && ch > 0 \n");
        return -1;
    }
    video_info_t  *pvideo_info = &g_av_info.video_info[ch];
    resolution_2_pic(0,pav_enc->resolution, &pvideo_info->video_width, &pvideo_info->video_height);
    pvideo_info->frame_rate = pav_enc->frame_rate;
	//现在这里简单地写一下  全局同步帧率改变同事就跟着改变
	g_av_info.video_info[ch].frame_rate=pav_enc->frame_rate;

	//if(1 == ch && 1 == pav_enc->is_using)
	//{
	//	VGA_flag=1;//区分 VGA 还是720P
	//}

    DECLARE_ATTR(h264e_attr, gm_h264e_attr_t);
    //暂时只支持h264
    switch (enc_type)
    {
    case SDK_VIDEO_FORMAT_H264:
        h264e_attr.dim.width = pvideo_info->video_width;
        h264e_attr.dim.height = pvideo_info->video_height;
        h264e_attr.frame_info.framerate = pav_enc->frame_rate;
        h264e_attr.ratectl.mode = pav_enc->bitrate_type + 1;  //GM_CBR
        h264e_attr.ratectl.gop = pav_enc->gop;
        h264e_attr.ratectl.bitrate = pav_enc->bitrate;
        h264e_attr.ratectl.bitrate_max = pav_enc->bitrate;
        h264e_attr.b_frame_num = 0;  // B-frames per GOP (H.264 high profile)
        h264e_attr.enable_mv_data = 0;  // disable H.264 motion data output
        h264e_attr.ratectl.init_quant = 25; //暂时没加 后面看CMS传值信息
        h264e_attr.ratectl.min_quant = 14;
        h264e_attr.ratectl.max_quant = 51;
        ///gm_set_attr(pvideo_info->video_object, &h264e_attr);
        /// memcpy(&param->enc[rec_track].codec.h264e_attr, &h264e_attr, sizeof(gm_h264e_attr_t));
        break;
#if 0
    case SDK_VIDEO_FORMAT_MPEG4:
        DECLARE_ATTR(mpeg4e_attr, gm_mpeg4e_attr_t);
        mpeg4e_attr.dim.width = width;
        mpeg4e_attr.dim.height = height;
        mpeg4e_attr.frame_info.framerate = framerate;
        mpeg4e_attr.ratectl.mode = mode;
        mpeg4e_attr.ratectl.gop = 60;
        mpeg4e_attr.ratectl.bitrate = bitrate;
        mpeg4e_attr.ratectl.bitrate_max = bitrate;
        gm_set_attr(param->enc[rec_track].obj, &mpeg4e_attr);
        memcpy(&param->enc[rec_track].codec.mpeg4e_attr, &mpeg4e_attr, sizeof(gm_mpeg4e_attr_t));
        break;
    case SDK_VIDEO_FORMAT_MJPEG:
        DECLARE_ATTR(mjpege_attr, gm_mjpege_attr_t);
        mjpege_attr.dim.width = width;
        mjpege_attr.dim.height = height;
        mjpege_attr.frame_info.framerate = framerate;
        mjpege_attr.quality = 30;
        gm_set_attr(param->enc[rec_track].obj, &mjpege_attr);
        memcpy(&param->enc[rec_track].codec.mjpege_attr, &mjpege_attr, sizeof(gm_mjpege_attr_t));
        break;
#endif
    default:
        warning("Not support enc_type %d \n", enc_type);
        break;
    }

		
	printf("--------------------video_set_enc_attr   ch is %d------------------------- \n",ch);
	printf("pvideo_info->video_width	= %d \n",pvideo_info->video_width);
	printf("pvideo_info->video_height   = %d \n",pvideo_info->video_height);
	printf("pvideo_info->video_type     = %d \n",pvideo_info->video_type);
	printf("pvideo_info->frame_rate 	= %d \n",pvideo_info->frame_rate);
	printf("--------------------END-------------------------\n");

	//wch add
	/*
	gm_cap_flip_t cap_flip_cfg;
	memset(&cap_flip_cfg, 0, sizeof(cap_flip_cfg));

	cap_flip_cfg.h_flip_enabled = 0;
	cap_flip_cfg.v_flip_enabled = 0;
	gm_set_cap_flip(0, &cap_flip_cfg);
	/****************************************/
	
    gm_set_attr(pvideo_info->video_object, &h264e_attr);
	
    if (gm_apply(g_av_info.video_groupfd) < 0)
    {
        // active setting (使生效)
        error("Error! gm_apply fail, AP procedure something wrong! \n");
        return -1;
    }
    return 0;
}


//视频遮挡区域设置
int video_set_overlay_attr(int ch)
{
    return 0;
}
/**********移动侦测*****************/
///int video_init_motion(int total_ch,sdk_vda_codec_cfg_t *motion_cfg)
int video_init_motion()
{
    int ret = -1;

    ret = init_motion_server();
    //这里目前只有一个主通道所以
    //ret = set_motion_param(0, motion_cfg);
    return ret;

}

int video_uninit_motion(int total_ch)
{
    uninit_motion_server();
}

//移动侦测报警区域设置
int video_set_motion_attr(int ch,sdk_vda_codec_cfg_t *motion_cfg)
{
    return set_motion_param(ch, motion_cfg);

}

void set_motion_param_test()
{
#if 1
    sdk_vda_codec_cfg_t md;
    sdk_vda_codec_cfg_t mt_cfg;
    sdk_motion_cfg_t    motion_cfg;
    //获取移动侦测的参数
    //adapt_param_get_motion_cfg(&motion_cfg);

    memset(&mt_cfg,0,sizeof(sdk_vda_codec_cfg_t));
    mt_cfg.enable=1;
    mt_cfg.sensitive=5;
    int j;
    for (j = 0; j < 4; j++)
    {

        mt_cfg.area[j].x=0;
        mt_cfg.area[j].y=0;
        mt_cfg.area[j].width=1280;
        mt_cfg.area[j].height=720;
        mt_cfg.mode=0;
    }

    warning("======================================================= \n");
    sdk_av_set_motion_param(0, &mt_cfg);
#endif
}


int video_motion_server_start()
{
    start_motion_server();
    return 0;
}

int video_motion_server_stop()
{
    stop_motion_server();//这是对的；姑桓耐戡
    return 0;
}
/*********OSD *********************/
//osd 配置
int video_osd_server_start();

int video_init_osd(int total_ch)
{
    pthread_t enc_thread_osd;
    int ret,ret1;
    gm_palette_table_t palette=
    {
palette_table:
        {
            OSD_PALETTE_COLOR_AQUA,
            OSD_PALETTE_COLOR_BLACK,
            OSD_PALETTE_COLOR_BLUE,
            OSD_PALETTE_COLOR_BROWN,
            OSD_PALETTE_COLOR_DODGERBLUE,
            OSD_PALETTE_COLOR_GRAY,
            OSD_PALETTE_COLOR_GREEN,
            OSD_PALETTE_COLOR_KHAKI,
            OSD_PALETTE_COLOR_LIGHTGREEN,
            OSD_PALETTE_COLOR_MAGENTA,
            OSD_PALETTE_COLOR_ORANGE,
            OSD_PALETTE_COLOR_PINK,
            OSD_PALETTE_COLOR_RED,
            OSD_PALETTE_COLOR_SLATEBLUE,
            OSD_PALETTE_COLOR_WHITE,
            OSD_PALETTE_COLOR_YELLOW
        }
    };
	#if 0
	//设置调色板颜色
    ret = gm_set_palette_table(&palette);
    if (ret < 0)
    {
        perror("Set osd palette failed");
        return -1;
     }
	#endif
    info("video_init_osd start !!!!!!!!! \n");
    ret1 = pthread_create(&enc_thread_osd, NULL, video_osd_server_start, NULL);
    if (ret1 < 0)
    {
        perror("create osd_start thread failed");
        return -1;
    }

    info("video_init_osd end !!!!!!!!! \n");
    return 0;
}


int char2int(char v[2])
{
    int a,b;

    if(v[0] >= '0' && v[0] <= '9')
        a = v[0] - '0';
    else if(v[0] >= 'a' && v[0] <= 'f')
        a = v[0] - 'a' + 10;
    else if(v[0] >= 'A' && v[0] <= 'F')
        a = v[0] - 'A' + 10;
    else
        return -1;
    if(v[1] >= '0' && v[1] <= '9')
        b = v[1] - '0';
    else if(v[1] >= 'a' && v[1] <= 'f')
        b = v[1] - 'a' + 10;
    else if(v[1] >= 'A' && v[1] <= 'F')
        b = v[1] - 'A' + 10;
    else
        return -1;
    return a*16+b;
}

int get_i2c_val(unsigned char *val, int num)
{
    char 	cmd[256];
    int		fd = -1;
    char	*p = NULL;
    int 	ret = 0;
    int i ;
    fd = fopen("/tmp/i2c_val", "a+");
    if (fd == NULL)
    {
        printf("i2c value:: fd==NULL\n");
        return -1;
    }
    memset(cmd, 0, sizeof(cmd));
    for(i = 0; i < num; i++)
    {
        if(fgets(cmd, 256, fd) != NULL)
        {
            p = strstr(cmd, "read:0x");
            if(p)
            {
                p = p + 7;
                val[i] = char2int(p);
                ret = 0;

            }
            else
            {
                ret = -1;
            }

        }
        else
        {
            ret = -1;
        }
    }
    system("rm -rf /tmp/i2c_val");
    return ret;
}

void gettmp()
{
    int ret;
    unsigned char datatmp[2];
    unsigned short u16sT;
    //float temperatureC;
    //e3  tmperature
    system("i2c_access 0x81 w 0xe3 r 0x02 > /tmp/i2c_val");
    //get high8 low8
    ret = get_i2c_val(datatmp,2);
    u16sT= (datatmp[0]<<8)|datatmp[1];
    u16sT &= ~0x0003;
    //printf("ending tmp16: 0x%x  \n",u16sT);
    temperatureC= -46.85 + ((175.72/65536.00000) *(float) u16sT); //T= -46.85 + 175.72 * ST/2^16
    printf("temperatureC:%6.2fC\n.",temperatureC);
    //return temperatureC;
}

void getRH()
{
    //e5  RH
    int ret;
    unsigned char dataRH[2];
    unsigned short u16sRH;
    //float humidityRH;              // variable for result
    char t ='%';
    system("i2c_access 0x81 w 0xe5 r 0x02 > /tmp/i2c_val");
    //get high8 low8
    ret = get_i2c_val(dataRH,2);
    u16sRH= (dataRH[0]<<8)|dataRH[1];
    u16sRH &= ~0x0003;
    //printf("ending rh16: 0x%x \n",u16sRH);
    humidityRH = -6.0 + ((125.0/65536.00000) * (float)u16sRH); // RH= -6 + 125 * SRH/2^16
    printf("humidityRH: %f%c\n.",humidityRH,t);
    //return humidityRH;
}

void update_osd_mask(int ch,int mask_idx, int x, int y, int width, int height, int alpha, int palette_idx)
{
    gm_osd_mask_t osd_mask;

    /** setup OSD mask */
    osd_mask.enabled = 1;
    osd_mask.mask_idx = mask_idx;
    osd_mask.x = x;
    osd_mask.y = y;
    osd_mask.width = width;
    osd_mask.height = height;
    osd_mask.alpha = alpha;		//(0-7)   0%->100%
    osd_mask.palette_idx = palette_idx;
    osd_mask.border.type = GM_OSD_MASK_BORDER_TYPE_TRUE;
    osd_mask.border.width = 1;
    osd_mask.align_type = GM_OSD_ALIGN_TOP_LEFT;
    gm_set_osd_mask(g_av_info.video_info[ch].capture_object, &osd_mask, GM_ALL_PATH);//set OSD attribute(设置OSD属性)
}
//font_idx =400(sdk底层里面分了400个font使用)/8*wind_iex(win_idx 共有八个)   如果这两个参数没有使用正确会导致闪烁和跌该
int init_osdtime()
{
    time_t tm;
    struct tm *ptr;
    time_t lt;
    time(&lt);
    ptr = localtime(&lt);//获取当前时间
    char dates[100];
    memset(dates,0,100);

	sprintf(dates,"%4d-%02d-%02d  %02d:%02d:%02d",ptr->tm_year+1900,ptr->tm_mon+1,ptr->tm_mday,ptr->tm_hour,ptr->tm_min,ptr->tm_sec);
	int font_len = strlen(dates);
	//printf("OSD Date: %s\n", dates);
    /*if (g_timemode == 0)
    {
        sprintf(dates,"%4d/%02d/%02d  %02d:%02d:%02d",ptr->tm_year+1900,ptr->tm_mon+1,ptr->tm_mday,ptr->tm_hour,ptr->tm_min,ptr->tm_sec);
    }
    else if(g_timemode == 1)
    {
        sprintf(dates,"%02d/%02d/%4d  %02d:%02d:%02d",ptr->tm_mon+1,ptr->tm_mday,ptr->tm_year+1900,ptr->tm_hour,ptr->tm_min,ptr->tm_sec);
    }
    else
    {
        sprintf(dates,"%02d/%02d/%4d  %02d:%02d:%02d",ptr->tm_mday,ptr->tm_mon+1,ptr->tm_year+1900,ptr->tm_hour,ptr->tm_min,ptr->tm_sec);
    }*/
	update_osd_dynamic(g_av_info.video_info[0].capture_object, 1, 1, dates, font_len, 20, 10);
	//update_osd_dynamic(g_av_info.video_info[1].capture_object, 16, 4, 100, 1, dates, 0, 0);
    //update_osd_with_hzk(g_av_info.video_info[0].capture_object, 32, 1, 50, 10, 1, dates,  0, 0);//main_capture_object
    //update_osd_with_hzk(g_av_info.video_info[1].capture_object, 16, 4, 200, 10, 1, dates,  0, 0);//sub_capture_object

    return 0;
}
int init_osdinfo()
{
    //test
    //unsigned char names[21] = "ausdom.cn ";
    int ret,length;
	unsigned char names[30];
    unsigned char tmp[21];
    unsigned char RH[21];
    //memset(tmp,0,sizeof(tmp));
    //memset(RH,0,sizeof(RH));

    ret = strlen(username);
	//printf(" init_osdinfo  username   is  %s  ret  is %d \n",username,ret);
    if(0 == ret)
    {
    	strcpy(names,"JOOAN");
		length=5;
    }
	else
	{
	#if 1
		 strcpy(names,username);
		 if(1==ret)
		 {
		 	//length=1;
		 	length=0;//关闭
		 }
		 else
		 {
		 	length=ret/2;
		 }
		 
		 if(ret%2)
		 {
		 	//在字符串尾部添加一个" "
		 	length=length+1;
		 	strcat(names," ");
		 }
		
	 #endif	 
	}
	
	if((0<ret)&&(ret<=2))
	{
		update_osd_with_hzk(g_av_info.video_info[0].capture_object, 32, 6, 300, length, 1, names, 1200, 670);//main_capture_object
		update_osd_with_hzk(g_av_info.video_info[1].capture_object, 16, 5, 250, length, 1, names, 610, 460);//sub_capture_object
	}
	else if((2<ret)&&(ret<=6))
    {
		update_osd_with_hzk(g_av_info.video_info[0].capture_object, 32, 6, 300, length, 1, names, 1150, 670);//main_capture_object
		update_osd_with_hzk(g_av_info.video_info[1].capture_object, 16, 5, 250, length, 1, names, 544, 460);//sub_capture_object
    }
	else if((6<ret)&&(ret<=12))
	{
		update_osd_with_hzk(g_av_info.video_info[0].capture_object, 32, 6, 300, length, 1, names, 1100, 670);//main_capture_object
		update_osd_with_hzk(g_av_info.video_info[1].capture_object, 16, 5, 250, length, 1, names, 448, 460);//sub_capture_object
	}
	else if(((12<ret)&&(ret<=22))||(ret>22))
	{	
		update_osd_with_hzk(g_av_info.video_info[0].capture_object, 32, 6, 300, length, 1, names, 900, 670);//main_capture_object
		update_osd_with_hzk(g_av_info.video_info[1].capture_object, 16, 5, 250, length, 1, names, 352, 460);//sub_capture_object
	}
    return 0;
}

int init_osdrecord()
{
    //update_osd_with_hzk(g_av_info.video_info[0].capture_object, 32, 1, 10, 1, 1 ,"R", 1150, 0);//main_capture_object
#if 1
	if(0 == g_record_show)
	{
		update_osd_with_hzk(g_av_info.video_info[0].capture_object, 32, 2, 32+100+32-16, 1, 0, "  ", 1200, 0);//main_capture_object
		update_osd_with_hzk(g_av_info.video_info[1].capture_object, 16, 3, 32+64+32+32, 1, 0, "  ", 620,0);//sub_capture_object 
		return 0;	
	}
	update_osd_with_hzk(g_av_info.video_info[0].capture_object, 32, 2, 32+100+32-16, 2, 1, "Rec ", 1200, 0);//main_capture_object
    update_osd_with_hzk(g_av_info.video_info[1].capture_object, 16, 3, 32+64+32+32, 2, 1, "Rec ", 620,0);//sub_capture_object 

#endif
   
    return 0;
}


int video_uninit_osd(int total_ch)
{
    return 0;
}
int video_set_osd_attr(int ch, sdk_osd_cfg_t *osd_cfg)
{
    video_info_t  *pvideo_info = &g_av_info.video_info[ch];
    //set_osd_font_chinese(pvideo_info->capture_object);
    g_timemode = osd_cfg->time.format;
    g_osd_show = osd_cfg->time.valid;
    g_channelname_show = osd_cfg->chann_name.valid;
	//设置osd的名字  在右下角显示
	memset(username,0,64);
	if(0 != strlen(osd_cfg->chann_name.str))
	strcpy(username,osd_cfg->chann_name.str); 
	//g_record_show=osd_cfg->ext_info.valid;
    warning("  set  g_timemode is %d   g_osd_show is %d g_channelname_show %d  is  username %s   record %d\n",g_timemode,g_osd_show,g_channelname_show,username,g_record_show);
    return 0;
}
int video_set_osd_recod_flag(int flag)
{ 
	g_record_show=flag;
    return 0;
}

int video_get_osd_attr(int ch)
{
    return ;
}
int video_set_osd_time_zone(int time_zone)
{ 
	int  old_zone=g_osd_time_zone;
	g_osd_time_zone=time_zone - 12;// 传入值0-24, 对应-12 ~12 个时区
	#if 0
	if( 0 != (g_osd_time_zone - old_zone))
	{
		time_zone_change =1;//刷新参数 和时区
	}
	#endif
    return 0;
}


//g_record_show
static void *osd_capture_thread(void *arg)
{
	info("@@@@@@@@@ osd_capture_thread running....\n");
    int times = 0;
	g_osd_capture = 1;
    while(g_osd_capture)
    {
		if(g_video_capture == 0)
		{
			usleep(100*1000);
			continue;
		}
		init_osdtime();
		#if 0
        if(!g_channelname_show && g_osd_show && !g_record_show)//时间
        {
            times = 0;
            init_osdtime();
        }
        else if(g_channelname_show && !g_osd_show&& !g_record_show)//name
        {
            times = 0;
            init_osdinfo();
        }
		else if(!g_channelname_show && !g_osd_show&& g_record_show)//录像
        {
       		 times = 0;
            init_osdrecord();
			
        }
		else if(g_channelname_show && g_osd_show&& !g_record_show)//name 、 时间
        {
        	times = 0;
            init_osdtime();
            init_osdinfo();
            init_osdrecord();
        }
		else if(!g_channelname_show && g_osd_show&& g_record_show)//时间 、录像
        {
        	times = 0;
            init_osdtime();
            init_osdrecord();
        }
		else if(g_channelname_show && !g_osd_show&& g_record_show)//name、录像
        {
        	times = 0;
            init_osdinfo();
            init_osdrecord();
        }
		else if(g_channelname_show && g_osd_show&& g_record_show)//name 、时间、录像
        {
        	times = 0;
            init_osdtime();
            init_osdinfo();
            init_osdrecord();
        }
        else
        {
            if (times == 0)
            {
                times ++;
                gm_unbind(g_av_info.video_info[0].capture_object);
                gm_unbind(g_av_info.video_info[1].capture_object);
                gm_apply(g_av_info.video_info[0].capture_object);
                gm_apply(g_av_info.video_info[1].capture_object);
                gm_delete_obj(g_av_info.video_info[0].capture_object);
                gm_delete_obj(g_av_info.video_info[1].capture_object);
                gm_release();
            }
        }
		#endif
        usleep(100*1000);

    }
}



static pthread_t osd_capture_id = 0;

//刷新时间 更新标题使用
int video_osd_server_start()
{
    pthread_attr_t attr;
    int ret ;

	info("osd server start.........\n");
#if	1
    /* Record Thread */
    if (osd_capture_id == (pthread_t)NULL)
    {
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);  //分离的线程
        ret = pthread_create(&osd_capture_id, &attr, &osd_capture_thread, NULL);
        pthread_attr_destroy(&attr);
    }
    else
    {
        info("osd_capture_thread is already runing..... \n");
    }
#endif
    //

    return 0;
}
/******rgb 转yuv*******/
/*图片的处理*/
static void RGB_To_YUV(const RGBQUAD * rgb, osd_clut_t *yuv)
{
    yuv->y = (unsigned char)(0.257f * rgb->rgbRed + 0.504f * rgb->rgbGreen + 0.098f * rgb->rgbBlue + 16);
    yuv->u = (unsigned char)(0.439f * rgb->rgbBlue - 0.291f * rgb->rgbGreen - 0.148f * rgb->rgbRed + 128);
    yuv->v = (unsigned char)(0.439f * rgb->rgbRed - 0.368f * rgb->rgbGreen - 0.071f * rgb->rgbBlue + 128);
    // yuv->alpha = 255;

    if( rgb->rgbRed==255 && rgb->rgbGreen==255 && rgb->rgbBlue==255)      // 示例bmp中背景为白色
    {
        yuv->alpha = 0;   //  全透明
    }
    else
    {
        yuv->alpha = 255;
    }
}

int max(int x,int y)
{
    if(x>y)return x;
    return y;
}
int min(int x,int y)
{
    if(x>y)return y;
    return x;
}

void BMP2RGB(unsigned char *src,unsigned char *dst,int size)
{
    unsigned char r,g,b;
    int i;
    for(i=0; i<size; i++)
    {
        b = *src++;
        g = *src++;
        r = *src++;
        *(dst++) = r;
        *(dst++) = g;
        *(dst++) = b;
    }
}
void RGBtoYUV422(char* SrcBuf,char* DestBuf,int DataWidth,int DataHeight)
{
    int   i,j,R,G,B,Y,U,V;
    int Run1,Run2;

    Run1=Run2=0;
    for(j=0; j<DataHeight; j++)
        for(i=0; i<DataWidth; i++)
        {
            R=SrcBuf[Run1+0];
            G=SrcBuf[Run1+1];
            B=SrcBuf[Run1+2];
            Y = (( 77 * R + 150 * G + 29 * B) >> 8);
            U = ((-43 * R - 85 * G + 128 * B) >> 8) + 128;
            V = ((128 * R - 107 * G - 21 * B) >> 8) + 128;
            DestBuf[Run2+0]=(unsigned char)max(0,min(255,Y));
            if(i&0x01)
                DestBuf[Run2+1]=(unsigned char)max(0,min(255,V));
            else
                DestBuf[Run2+1]=(unsigned char)max(0,min(255,U));
            Run1+=3;
            Run2+=2;
        }
}

void update_osd_marks(void)
{
    int mark_idx;
    int ret;
    gm_osd_mark_img_table_t osd_mark_img;
    gm_osd_mark_t osd_mark;
    FILE *logo_file;
    static char *logo_img_buf,*logo_img_buf2,*logo_img_buf3;

    logo_img_buf = (char *)malloc((LOGO_WIDTH * LOGO_HEIGHT * 2));
    //logo_img_buf2 = (char *)malloc((LOGO_WIDTH * LOGO_HEIGHT * 2));
    //logo_img_buf3 = (char *)malloc((LOGO_WIDTH * LOGO_HEIGHT * 2));
    if (logo_img_buf == NULL)
    {
        perror("Error to allocate logo buf.\n");
        return;
    }

    logo_file = fopen(LOGO_YUV422_PATTERNS, "rb");
    if (!logo_file)
    {
        printf("Error to open logo file %s!", LOGO_YUV422_PATTERNS);
        free(logo_img_buf);
        return;
    }

    fread(logo_img_buf, (LOGO_WIDTH * LOGO_HEIGHT * 2), 1, logo_file);

    memset(&osd_mark_img, 0, sizeof(gm_osd_mark_img_table_t));
    mark_idx = 0; //idx value from 0 ~ GM_MAX_OSD_MARK_IMG_NUM
    osd_mark_img.mark_img[mark_idx].mark_exist = 1;
    osd_mark_img.mark_img[mark_idx].mark_yuv_buf = (char *)logo_img_buf;
    osd_mark_img.mark_img[mark_idx].mark_width = GM_OSD_MARK_DIM_64;
    osd_mark_img.mark_img[mark_idx].mark_height = GM_OSD_MARK_DIM_64;
    osd_mark_img.mark_img[mark_idx].mark_yuv_buf_len = (LOGO_WIDTH * LOGO_HEIGHT * 2);

    ret = gm_set_osd_mark_image(&osd_mark_img);
    if (ret < 0)
    {
        printf("Error to set OSD mark image.\n");
        free(logo_img_buf);
        free(logo_img_buf2);
        free(logo_img_buf3);
        return;
    }

    osd_mark.enabled = 1;
    osd_mark.mark_idx = mark_idx;
    osd_mark.x = 0;
    osd_mark.y = 120;
    osd_mark.alpha = GM_OSD_MARK_ALPHA_75;
    osd_mark.zoom = GM_OSD_MARK_ZOOM_2X;
    osd_mark.align_type = GM_OSD_ALIGN_TOP_LEFT;
    gm_set_osd_mark(g_av_info.video_info[0].capture_object, &osd_mark);//set OSD attribute(设置OSD属性)
    if (logo_img_buf)
        free(logo_img_buf);
    if (logo_img_buf2)
        free(logo_img_buf2);

    if (logo_img_buf3)
        free(logo_img_buf3);

    fclose(logo_file);
}

//osg只能显示rgb
void update_osg_marks(void)
{
    int mark_idx;
    int ret;
    gm_osd_mark_img_table_t osd_mark_img;
    gm_osd_mark_t osd_mark;
    FILE *logo_file;
    static char *logo_img_buf;

    logo_img_buf = (char *)malloc((OSG_LOGO_WIDTH * OSG_LOGO_HEIGHT * 2));
    if (logo_img_buf == NULL)
    {
        perror("Error to allocate logo buf.\n");
        return;
    }

    logo_file = fopen(OSG_LOGO_RGB1555_PATTERNS, "rb");
    if (!logo_file)
    {
        printf("Error to open logo file %s!", OSG_LOGO_RGB1555_PATTERNS);
        free(logo_img_buf);
        return;
    }

    fread(logo_img_buf, (OSG_LOGO_WIDTH * OSG_LOGO_HEIGHT * 2), 1, logo_file);

    memset(&osd_mark_img, 0, sizeof(gm_osd_mark_img_table_t));
    mark_idx = 4; //idx value from 0 ~ GM_MAX_OSD_MARK_IMG_NUM
    osd_mark_img.mark_img[0].mark_exist = 1;
    osd_mark_img.mark_img[0].mark_yuv_buf = (char *)logo_img_buf;
    osd_mark_img.mark_img[0].mark_width = OSG_LOGO_WIDTH;
    osd_mark_img.mark_img[0].mark_height = OSG_LOGO_HEIGHT;
    osd_mark_img.mark_img[0].mark_yuv_buf_len = (OSG_LOGO_WIDTH * OSG_LOGO_HEIGHT * 2);
    osd_mark_img.mark_img[0].osg_mark_idx = 4;



    ret = gm_set_osd_mark_image(&osd_mark_img);
    if (ret < 0)
    {
        printf("Error to set OSD mark image.\n");
        free(logo_img_buf);
        return;
    }

    osd_mark.enabled = 1;
    osd_mark.mark_idx = mark_idx;
    osd_mark.x = 1000;
    osd_mark.y = 20;
    //osd_mark.alpha = GM_OSD_MARK_ALPHA_75;
    osd_mark.alpha = GM_OSD_MARK_ALPHA_0;
    osd_mark.zoom = GM_OSD_MARK_ZOOM_1X;
    osd_mark.align_type = GM_OSD_ALIGN_TOP_LEFT;
    osd_mark.osg_mark_idx = 4;
    gm_set_osd_mark(pvideo_info->capture_object, &osd_mark);//set OSD attribute(设置OSD属性)
    //if (logo_img_buf)
    //    free(logo_img_buf);

    fclose(logo_file);
}


int video_osd_server_stop()
{
	g_osd_capture = 0;
    return 0;
}

/*********************/
//感兴趣区域设置
int video_set_roi_attr(int ch,sdk_roi_cfg_t *roi_cfg )
{

    if(!roi_cfg)
    {
        return -1;
    }
    if( (roi_cfg->roiRect[0].x + roi_cfg->roiRect[0].width) > g_grain_system.cap[0].dim.width
            ||( roi_cfg->roiRect[0].y + roi_cfg->roiRect[0].height )>  g_grain_system.cap[0].dim.height)
    {
        error("param error!!! \n");
        return -1;
    }
    void *video_object = get_video_object(0, 0);
    DECLARE_ATTR(h264e_roi_attr, gm_enc_roi_attr_t);
    h264e_roi_attr.enabled = roi_cfg->enable;
    h264e_roi_attr.rect.x = roi_cfg->roiRect[0].x;
    h264e_roi_attr.rect.y = roi_cfg->roiRect[0].x;
    h264e_roi_attr.rect.width = roi_cfg->roiRect[0].width;
    h264e_roi_attr.rect.height = roi_cfg->roiRect[0].height;
    gm_set_attr(video_object, &h264e_roi_attr);//set roi attribute(设置ROI属性)
    if(gm_apply(g_av_info.video_groupfd) < 0)
    {
        error("gm_apply error \n");
        return -1;
    }
    return 0;
}

int video_set_3di_attr(int ch, sdk_3di_cfg_t *p3di_cfg )
{
    if(!p3di_cfg)
    {
        return -1;
    }
    DECLARE_ATTR(cap_3di_attr, gm_3di_attr_t);
    void *video_object = get_video_object(0, 0);

    /*** set 3di attribute to capture */
    cap_3di_attr.deinterlace = (p3di_cfg->deinterlace == 1)? 1:2; // 1 enable (开启), 2 disable(关闭)
    cap_3di_attr.denoise = (p3di_cfg->denoise== 1 )? 1:2; // 1 enable (开启), 2 disable(关闭)
    gm_set_attr(video_object, &cap_3di_attr); //set 3DI attribute(设置3DI属性)
    if (gm_apply(g_av_info.video_groupfd) < 0)   // active setting (使生效)
    {
        error("Error! gm_apply fail, AP procedure something wrong!");
        return -1;
    }
    return 0;

}

/*********************/
//抓拍功能
//这里只是提供接口
#if 0
SendstreamCallback	mSnapCallback = NULL;
static sdk_snap_pic_t g_snap_attr;
static int g_snap_fd = -1;


#define MAX_SNAPHSOT_LEN    (128*1024)
static char  snapshot_buf[MAX_SNAPHSOT_LEN] = { 0 };

int _snap_init(int ch,SendstreamCallback snap_callback)
{
    if(snap_callback)
        mSnapCallback = snap_callback;

    g_snap_fd = get_video_bind_handle(0,0);
    //warning("g_snap_fd  %d \n",g_snap_fd);
    return 0;
}
int _snap_unint(int ch)
{
    g_snap_fd = -1;
    mSnapCallback = NULL;
    return 0;
}

//默认通道就是0
int _set_snap_attr(int ch,sdk_snap_pic_t *snap_attr)
{
    if(snap_attr)
        memcpy(&g_snap_attr,snap_attr,sizeof(sdk_snap_pic_t));
    return 0;
}

//在8138s中这一个函数就可以搞定了
//void *video_snap_thread(void *arg)
int _snap_process(sdk_snap_info_t *snap_info)
{
    static int filecount = 0;
    FILE    *snapshot_fd = NULL;

    int     snapshot_len = 0;
    char    filename[40];
    snapshot_t snapshot;

    if(g_snap_fd < 0 ||!snap_info|| snap_info->data)
    {
        error(" snap_process  param error \n");
        return -1;
    }
    snapshot.bindfd = g_snap_fd;
    snapshot.image_quality = 30;//g_snap_attr.quilty;  // The value of image quality from 1(worst) ~ 100(best)

    snapshot.bs_buf = snapshot_buf;//snap_info->data;
    snapshot.bs_buf_len = MAX_SNAPHSOT_LEN;//snap_info->max_len;

    snapshot.bs_width = 176;//g_snap_attr.width   如何计算
    snapshot.bs_height = 144;//g_snap_attr.height

    snapshot_len = gm_request_snapshot(&snapshot, 500); // Timeout value 500ms
    warning("-------------->g_snap_fd [%d]      snapshot_len [%d] \n",g_snap_fd,snapshot_len);
    if (snapshot_len > 0)
    {
        snap_info->data_len	 = snapshot_len;

        sprintf(filename, "/mnt/mtd/snap/snapshot_%d.jpg", filecount++);
        printf("Get %s size %d bytes\n", filename, snapshot_len);
        snapshot_fd = fopen(filename, "wb");
        if (snapshot_fd == NULL)
        {
            printf("Fail to open file %s\n", filename);
            exit(1);
        }
        fwrite(snapshot_buf, 1, snapshot_len, snapshot_fd);
        fclose(snapshot_fd);
        return 0;
    }
    else
    {
        error("snapshot_len :%d  \n",snapshot_len);
    }
    return -1;
}


#if 0
//抓拍消耗时间 创建线程解决
int _snap_process(int ch,int snap_type,int snap_num)
{

    pthread_attr_t attr;
    int ret ;
    init_write_fd();

    video_capture_start();
    /* Record Thread */
    if (video_capture_id == (pthread_t)NULL)
    {
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);  //分离的线程
        ret = pthread_create(&video_capture_id, &attr, &video_capture_thread, NULL);
        pthread_attr_destroy(&attr);
    }
    else
    {
        info("video_capture_thread is already runing..... \n");
    }

    return 0;
}
#endif
#endif
int video_snap_init(int ch,SendstreamCallback snap_callback)
{
    _snap_init(ch,snap_callback);
	
    return 0;
}
int video_snap_unint(int ch)
{
    _snap_unint( ch);
    return 0;
}
//在8138s中这一个函数就可以搞定了 应该枷锁防止多个同时调用
int video_snap_process(int ch,sdk_snap_info_t *snap_info,char *path)
{
    _snap_process(snap_info,path);
    return 0;
}

// 0 :60  1:50
int sensor_power_hz_param(int hz_type)
{
    ov9715_control(Powerfreq,  hz_type);
}


int av_mirror_param(int ch, int mirror_rataion_enable, int flip_ratation_enable)
{
	gm_cap_flip_t cap_flip_cfg;
	memset(&cap_flip_cfg, 0, sizeof(cap_flip_cfg));

	cap_flip_cfg.h_flip_enabled = mirror_rataion_enable;
	cap_flip_cfg.v_flip_enabled = flip_ratation_enable;
	gm_set_cap_flip(ch, &cap_flip_cfg);
}

/*************************************************/
//临时增加
typedef enum
{
    GRD_COLOR_MODE_AUTO = 0,    // Color Mode: Auto, detected by light sensor
    GRD_COLOR_MODE_DAY,         // Color Mode: Day - Chromatic, bInfraredLed = 0, bInfraredCut = 1, nSaturation > 0
    GRD_COLOR_MODE_NIGHT,       // Color Mode: Night - Monochrome, bInfraredLed = 1, bInfraredCut = 0, nSaturation = 0
    //  GRD_COLOR_MODE_SCHEDULE_ON_TIMER,   //Color Mode: time Schedule  current mode.
} GRD_COLOR_MODE_E;


//add for isp_config thread
typedef enum enum_scene_mode
{
	ENUM_SCENE_DAY_MODE=0,
	ENUM_SCENE_NIGHT_MODE=1
}ENUM_SCENE_MODE;

int color_isp_reload_apply_scene(int scene_mode)
{
	char filename[60];
    memset(filename,0,sizeof(filename));
	if(scene_mode == ENUM_SCENE_DAY_MODE)
	{
		sprintf(filename, "/mnt/mtd/isp328_sc1135_day.cfg");
	}
	else if(scene_mode == ENUM_SCENE_NIGHT_MODE)
	{
		sprintf(filename, "/mnt/mtd/isp328_sc1135_night.cfg");
	}
	int ret;
	warning("COLOR MODE %d,filename:[%s]\n",scene_mode,filename);
	ret = isp320_reloadCfg_control( filename);
}

int color_isp_reload_apply(int  color_mode)
{
    char filename[60];
    memset(filename,0,sizeof(filename));
    if (0 == color_mode)
    {
        //sprintf(filename, "/mnt/mtd/isp328_ov9715_DAY.cfg");
        sprintf(filename, "/mnt/mtd/isp328_sc1135.cfg");
    }
    else if (1 == color_mode)
    {
        sprintf(filename, "/mnt/mtd/isp328_ov9715_NIGHT.cfg");
    }
    else if (2 == color_mode)
    {
        sprintf(filename, "/mnt/mtd/isp328_ov9715_DAYLOW.cfg");
    }
    else if (3 == color_mode)
    {
        sprintf(filename, "/mnt/mtd/isp328_ov9715_NIGHT.cfg");
    }
	else if (4 == color_mode)
	{
		sprintf(filename, "/mnt/mtd/isp328_ov9715_DAYLOWLOW.cfg");
		warning("DAYLOWLOW MODE open...........\n");
	}
	int ret;
	warning("COLOR MODE %d,filename:[%s]\n",color_mode,filename);
	ret = isp320_reloadCfg_control( filename);
}
/*
int  color_scene_num   0: 室外白天  1:室外黑夜  2:室内白天 3:室内黑夜
*/
int grd_color_Dynamic_mode_apply(GRD_COLOR_MODE_E enColorMode, int color_scene_num)
{
    //grd_color_set_powerfreq(pstColorScheme->stColorCfg.nPowerFreq);

	int i;
    /* Night -> Day, set IR-Cut before ISP configuration */

    if (enColorMode == GRD_COLOR_MODE_NIGHT ) //夜晚  降帧
    {
        Set_DayNight_mode(1);
        ov9715_control(SenFps,  10);
		for(i=0;i<2;i++)
		{
			g_av_info.video_info[i].frame_rate=10;
		}
    }
    else  // 白天
    {
#if 0
		Set_DayNight_mode(0);  
        ov9715_control(SenFps,  25);
		for(i=0;i<2;i++)
		{
			g_av_info.video_info[i].frame_rate=25;
		}
#endif
#if 0
		//change by zw 20160114	
		if(0 ==VGA_flag)
		{
			Set_DayNight_mode(0);  
	        ov9715_control(SenFps,  g_av_info.video_info[0].frame_rate);
		}
		else 
		{
			Set_DayNight_mode(0);  
	        ov9715_control(SenFps,  g_av_info.video_info[1].frame_rate);
		}
#endif
		Set_DayNight_mode(0);  
	    ov9715_control(SenFps,  g_av_info.video_info[0].frame_rate);
		
    }

    if(color_scene_num == 0)//室外白天
    {
    }
    else  if(color_scene_num == 1)//1:室外黑夜 
    {
    }
    else  if(color_scene_num == 2)//2:室内白天 
    {
    	warning("Door In-------->DAY  mode..........\n");
		//ov9715_control(Brightness,  135);
		//ov9715_control(Contrast,  128);
		//ov9715_control(Sharpness,  128);
	    //ov9715_control(Hue,  128);
		//ov9715_control(Saturation,  128);
		//ov9715_control(Denoise,  150);
	    //ov9715_control(Gama,  1);
		ov9715_control(AEMaxGain, 1024);
		ov9715_control(AEMinGain, 64);

    }
    else  if(color_scene_num == 3) //3:室内黑夜
    {
		warning("Door In-------->NIGHT  mode..........\n");
		//ov9715_control(Brightness,  60);
		//ov9715_control(Contrast,  200);
		//ov9715_control(Sharpness,  140);
	    //ov9715_control(Hue,  135);
		//ov9715_control(Saturation,  0);
		//ov9715_control(Denoise,  150);
	    //ov9715_control(Gama,  1);
      
    }
    return 0;
}

int g_force_ircut_switch;
void *isp_config_thread(void *arg)
{
	static int bak_scene_mode = -1;
	int cur_scene_mode;
	int gpio0_17_irLed = 17;
	int gpio1_28_irCut = 28+32;
printf("++++++++++++++++> test 2\n");

	sdk_gpioEx_defpin(gpio1_28_irCut, 1); // set to output direction 
	while(1)
	{
		if(g_force_ircut_switch > 0)
		{}
		else
		{
			if(gpioEx_read(gpio0_17_irLed) == 0)
				cur_scene_mode = ENUM_SCENE_DAY_MODE;
			else
				cur_scene_mode = ENUM_SCENE_NIGHT_MODE;
		//printf("^^^^^^^^^^^ cur_scene_mode=%d\n", cur_scene_mode);sleep(2);
			if(bak_scene_mode < 0)
			{
				bak_scene_mode = cur_scene_mode;
			}
			if((bak_scene_mode != cur_scene_mode) && (cur_scene_mode == ENUM_SCENE_DAY_MODE))
			{
				bak_scene_mode = cur_scene_mode;
				printf("################### switch to Day mode!\n");
				printf("--- IR-CUT GPIO1_28 value[%d]\n", gpioEx_read(gpio1_28_irCut));
				gpioEx_clear(gpio1_28_irCut);
				system_ex("/mnt/mtd/isp_config_scene_mode.sh day");
				//system_ex("echo 1 2 1 2 8 2 1 2 1 > /proc/isp328/ae/win_weight");
        		//system_ex("echo 0 > /proc/isp328/ae/hi_light_supp");
				//color_isp_reload_apply_scene(ENUM_SCENE_DAY_MODE);
				//sleep(2);
			}
			else if((bak_scene_mode != cur_scene_mode) && (cur_scene_mode == ENUM_SCENE_NIGHT_MODE))
			{
				bak_scene_mode = cur_scene_mode;
				printf("################### switch to Night mode!\n");
				printf("--- IR-CUT GPIO1_28 value[%d]\n", gpioEx_read(gpio1_28_irCut));
				gpioEx_set(gpio1_28_irCut); 
				system_ex("/mnt/mtd/isp_config_scene_mode.sh night");
				//system_ex("echo 1 2 1 2 8 2 1 2 1 > /proc/isp328/ae/win_weight");
        		//system_ex("echo 5 > /proc/isp328/ae/hi_light_supp");
				//color_isp_reload_apply_scene(ENUM_SCENE_NIGHT_MODE);
				//sleep(2);
			}
			else
			{
				continue;
			}
		}
	}
}

static pthread_t thread_isp_id = 0;
int isp_config_start()
{
	pthread_attr_t isp_pattr;
	//pthread_t thread_isp_id;
    int ret ;
printf("++++++++++++++++> test 1\n");
    if (thread_isp_id == (pthread_t)NULL)
    {
        pthread_attr_init(&isp_pattr);
        pthread_attr_setdetachstate(&isp_pattr, PTHREAD_CREATE_DETACHED);  //分离的线程
        ret = pthread_create(&thread_isp_id, &isp_pattr, &isp_config_thread, NULL);
        pthread_attr_destroy(&isp_pattr);
    }
    else
    {
        info("isp_thread is already runing..... \n");
    }

}

void *hw_handle_thread(void *arg)
{
	int gpio_reset = 32+19;
	int value;
	int tickcount;
	unsigned long long ullprevstamp, ullcurstamp;
	
	while(1)
	{
		//COMM_GetSystemUpMSecs(&ullcurstamp);
		//if(ullprevstamp == 0)
		//	ullprevstamp = ullcurstamp;
		
		value = gpioEx_read(gpio_reset);
		if(value == 0)
		{
			tickcount++;
			//printf("&&&&&&&&&&&&&& 0000000000 &&&&&&&&&&&&&\n");
		}
		else
		{
			tickcount = 0;
			//printf("&&&&&&&&&&&&&& 1111111111 &&&&&&&&&&&&&\n");
		}

		if(tickcount >= 5)
		{
			printf("\n\n##### press reset button over 5 seconds, will clear network config...\n\n");
			tickcount = 0;

			system_ex("/mnt/mtd/reset_handle.sh");
		}
		sleep(1);
	}
}

static pthread_t thread_hw_id = 0;
void hw_control_init()
{
	pthread_attr_t hw_attr;
	int ret ;

	if (thread_hw_id == (pthread_t)NULL)
	{
		pthread_attr_init(&hw_attr);
		pthread_attr_setdetachstate(&hw_attr, PTHREAD_CREATE_DETACHED);  //分离的线程
		ret = pthread_create(&thread_hw_id, &hw_attr, &hw_handle_thread, NULL);
		pthread_attr_destroy(&hw_attr);
	}
	else
	{
		info("hw_handle_thread is already runing..... \n");
	}
}



int g_exit = 0;

extern int g_force_close_infrared ;

static void *icut_thread(void *arg)
{
	char buf_saturation[100];
	int vals1 ,vals2;
	int mode = 1;
	int old_mode = 2;
	int day_mod = 1;
	int old_day_mode=2;
	//int gpio_Infrared = 32+3;//962使用的就是这一个 
	int gpio_ircut = 17;
  int gpio_ir_cut28 = 32+28;
  //int gpio_ir_cut27 = 32+27;
	int Curgain = -1;
	int CurEV = -1;
	int EVcounts = 0;
	int infrared_status = 0;
	int day_frame_ch1=0;
	int day_frame_ch2=0;
	ir_cut_delay_flag=0;
	ircut_change_time = 0;
	error(" ===================== g_exit:%d ",g_exit);
	//sdk_gpioEx_defpin(gpio_Infrared, 1);//设置GPIO1_25为输出状态 在这里在设置一次 因为设备第一次会设置失败 暂时不知道原因
#if 0
	//因为硬件的线序存在问题启动时设置切片   使用962时需要打开这个 966时不需要除非ircut又换了
	gpioEx_clear(gpio_ir_cut26);
	gpioEx_set(gpio_ir_cut27);
	usleep(300*1000);			
	gpioEx_clear(gpio_ir_cut27);
#else
	//因为硬件的线序存在问题启动时设置切片962关闭  966打开
	/*gpioEx_clear(gpio_ir_cut27);
	gpioEx_set(gpio_ir_cut26);
	usleep(300*1000);			
	gpioEx_clear(gpio_ir_cut26);*/
#endif		
	while(!g_exit)
	{
	
#if  0 //#ifdef MAC_966 //无光敏和ICUT 判断CurEV
		//get_Sensor_CurGain(&Curgain);    //也可以用来判断，最好用CurEV，CurGain用来调室内外
		get_Sensor_CurEV(&CurEV);
		if(CurEV <= 0)
		{
			break;
		}
		else if(CurEV < 400 && CurEV > 0)
		{
			if(old_mode != 0)
			{
				int ch = 0;
		 		sdk_image_attr_t attr_cfg;
		 		attr_cfg.saturation = 0;
		 		attr_cfg.icut_flag= 1;  //icut 切换 修改视频参数标识，必须加
		 		sdk_av_set_attr_param(ch, &attr_cfg);
		 		color_isp_reload_apply(1);
				old_mode = 0;
		 	    // grd_color_Dynamic_mode_apply(GRD_COLOR_MODE_NIGHT, 3);
			}
		}
		else 
		{
			if(old_mode !=1)
			{
				int ch = 0;
				sdk_image_attr_t attr_cfg;
				attr_cfg.saturation = 128;
				attr_cfg.icut_flag= 1; //icut 切换 修改视频参数标识，必须加
				sdk_av_set_attr_param(ch, &attr_cfg);
	            color_isp_reload_apply(2);
	           	grd_color_Dynamic_mode_apply(GRD_COLOR_MODE_DAY, 2);
				old_mode = 1;
			}
		}

		// 带光敏和ICUT 执行下面
#else
		// read the value of gpio17, 0 -- ir_led dark, 1 -- ir_led light
		//if (get_light_sensor_value())
		if(gpioEx_read(gpio_ircut) == 0)
		{
			mode = 1;
		}
		else
		{
			if(1 == g_force_close_infrared)
			{
				mode = 1;
			}
			else
			{
				mode = 0;
			}
		}
		//mode=1;//暂时强制白天状态
#if 0	
		get_Sensor_CurEV(&CurEV); //读EV值低照切DAYLOWLOW
		if(CurEV <= 1000)
		{
			if(EVcounts <30)
			{
				EVcounts ++;	
			}
			if(EVcounts == 30)
			{
				day_mod =0;//day-lowlow 状态
				//warning("-----------change daylowlow   day_mod =0;//day-lowlow \n");
			}
		}
		else
		{
			EVcounts = 0;
			day_mod=1;
		}
#endif
		if(mode == 0)//夜间模式
		{	
			if(old_mode != mode )
			{
				ir_cut_delay_flag=1;
				ircut_flag_set(ir_cut_delay_flag);
				warning("-----------ICUT   CHANGED \n");
				#if 0
				if(!g_force_close_infrared)
				gpioEx_set(gpio_Infrared);  //开启红外灯
				else	
    	  		gpioEx_clear(gpio_Infrared); // 关闭红外灯 		
    	  		#endif
				warning("----------->night_tread gpio_ir_cut28	->	[%d]\n",gpioEx_read(gpio_ir_cut28));
				infrared_status = g_force_close_infrared;
				//gpioEx_set(gpio_ir_cut26);
                gpioEx_clear(gpio_ir_cut28);          
               // usleep(200*1000);
                //gpioEx_clear(gpio_ir_cut26);


                int ch = 0;
                sdk_image_attr_t attr_cfg;
                attr_cfg.saturation = 0;
                attr_cfg.icut_flag= 1;
                sdk_av_set_attr_param(ch, &attr_cfg);
                color_isp_reload_apply(1);
				ov9715_control(SenFps,  10);
				day_frame_ch1=g_av_info.video_info[0].frame_rate;
				day_frame_ch2=g_av_info.video_info[1].frame_rate;
				g_av_info.video_info[0].frame_rate=10;
				g_av_info.video_info[1].frame_rate=10;
				//gm_set_attr(pvideo_info->video_object, &h264e_attr);//要不要在这里设置
               // grd_color_Dynamic_mode_apply(GRD_COLOR_MODE_NIGHT, 3);
                old_mode = 0;
            }
        }
        else if(mode == 1||1 == g_force_close_infrared) //白天模式
        {
            if(old_mode != mode)
            {
            	ir_cut_delay_flag=1;
				ircut_flag_set(ir_cut_delay_flag);
            	warning("-----------ICUT   CHANGED \n");
            	//gpioEx_clear(gpio_Infrared);
            	//gpioEx_set(gpio_Infrared);
				warning("----------->day_tread gpio_ir_cut28	->	[%d]\n",gpioEx_read(gpio_ir_cut28));
				//gpioEx_clear(gpio_ir_cut26);
				gpioEx_set(gpio_ir_cut28);
				//usleep(200*1000);			
				//gpioEx_clear(gpio_ir_cut27);

				int ch = 0;
				sdk_image_attr_t attr_cfg;
				attr_cfg.saturation = 128;
				attr_cfg.icut_flag= 1;
				sdk_av_set_attr_param(ch, &attr_cfg);

				if(0!=day_frame_ch1&&0!=day_frame_ch2)
            	{
            		g_av_info.video_info[0].frame_rate=day_frame_ch1;
					g_av_info.video_info[1].frame_rate=day_frame_ch2;
            	}
				
                color_isp_reload_apply(0);
                grd_color_Dynamic_mode_apply(GRD_COLOR_MODE_DAY, 2);

                old_mode = 1;
				old_day_mode = 1;	
				
            	
            	warning("-----------ICUT   CHANGED  END!!!!! \n");
            }
        }
#endif
		if(1 == ir_cut_delay_flag)
		{
			ircut_change_time++;
			//printf("ircut_change_time  is %d \n",ircut_change_time);
			if(30 == ircut_change_time)
			{
				warning("+_+_+_+_+_+_ircut_change_time  is %d \n",ircut_change_time);
				ir_cut_delay_flag =0 ;
				ircut_change_time=0;
			}
		}
        usleep(100*1000);
    }
}
//  1:no alarm    0  :push alarm
int get_icut_delay()
{
	return ir_cut_delay_flag;
}

static pthread_t icut_capture_id = 0;
int I_CUT_MONITER()
{
	pthread_attr_t icut;
    int ret ;

    if (icut_capture_id == (pthread_t)NULL)
    {
        pthread_attr_init(&icut);
        pthread_attr_setdetachstate(&icut, PTHREAD_CREATE_DETACHED);  //分离的线程
        ret = pthread_create(&icut_capture_id, &icut, &icut_thread, NULL);
        pthread_attr_destroy(&icut);
    }
    else
    {
        info("Test icut_thread is already runing..... \n");
    }

}

int video_set_attr_param(int ch, sdk_image_attr_t* attr_cfg)
{
	char buf_brightness[128];
	char buf_hue[128];
	char buf_contrast[128];
	char buf_saturation[128];
	char buf_sharpness[128];
	if(attr_cfg->icut_flag== 0)
	{
		sprintf(buf_brightness,"echo w brightness %d > /proc/isp328/command",attr_cfg->brightness);
		sprintf(buf_contrast,"echo w contrast %d > /proc/isp328/command",attr_cfg->contrast);
		sprintf(buf_hue,"echo w hue %d > /proc/isp328/command",attr_cfg->hue);
		sprintf(buf_saturation,"echo w saturation %d > /proc/isp328/command",attr_cfg->saturation);
		sprintf(buf_sharpness,"echo w sharpness %d > /proc/isp328/command",attr_cfg->sharpness);
		system(buf_brightness);
		system(buf_contrast);
		system(buf_hue);
		system(buf_saturation);
		system(buf_sharpness);
	}
	else if(attr_cfg->icut_flag == 1)
	{
		sprintf(buf_saturation,"echo w saturation %d > /proc/isp328/command",attr_cfg->saturation);
		system(buf_saturation);
	}
	sleep(1);
	return 0;
}

//
