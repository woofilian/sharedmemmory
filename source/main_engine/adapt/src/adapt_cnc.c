#include	<time.h>
#include	<stdio.h>
#include	<string.h>
#include	<pthread.h>
#include	<stdlib.h>
#include 	<fcntl.h>

#if 0
///#include	"cncLib.h"
#include	"streamLib.h"
#include    "protocol.h"

#include "sdk_global.h"
#include "sdk_struct.h"
#include "main.h"

///#include "log.h"
//typedef sdk_frame_t frame_t;
typedef sdk_frame_t frame_head_t;
#define  	CHANNEL_NUM  2
#define     STATE_OFFLINE       0
#define     STATE_ONLINE        1
#define     STATE_FALLLINE      2

#define 	MAX_CHANNEL_NUM		16
#define		MUTEX_CHANNEL		97
#define		POLL_CHANNEL		98
// 全局变量自动初始化为0
////extern	disk_manage_t	*p_disk_manage;
int 	g_uploadMsgFlag;
int		g_videoFlag[2][MAX_CHANNEL_NUM];
int		g_mutexVideoFlag;
int		g_pollVideoFlag;
int		g_audioFlag;
cncMduServer_t	mduServer[2][MAX_CHANNEL_NUM];

int 	g_mutexChannel;

#if 1
/*------------------------frame struct------------------------------*/
#define SANTACHI_I_FRAME_TYPE 1 /*I Frame*/
#define SANTACHI_P_FRAME_TYPE 2 /*P Frame*/
#define SANTACHI_A_FRAME_TYPE 3 /*Audio Frame*/
#define SANTACHI_OSD_FRAME_TYPE 4 /* Osd Frame*/

#define SANTACHI_WSX_COMPATIBLE_FRAME_TYPE 6//网视星录像文件头
#define SANTACHI_GPS_FRAME_TYPE 7//GPS Frame
#define SANTACHI_PH_FRAME_TYPE 8//流平滑帧


typedef struct _st__frame_head_t
{
    unsigned long device_type;
    unsigned long frame_size;
    unsigned long frame_no;
    unsigned char video_resolution;
    unsigned char frame_type;
    unsigned char frame_rate;
    unsigned char video_standard;
    unsigned long sec;
    unsigned long usec;
    uint64_t pts;
} st_frame_head_t;

typedef struct _st_frame_t
{
    st_frame_head_t frame_head;
    unsigned char *frame_data;
} st_frame_t;
/***************/
typedef struct stream_info_s
{
    int channel;                /* 源通道号 */
    int streamType;			 /*0 主码流 1  从码流*/
    int isUse[2];				/* 是否已经比使用*/
    int cur_frame_type;       /*存储取到的帧类型*/

} stream_info_t;
#endif

sdk_msg_dispatch_cb  msg_dispatch_cb = NULL;


static stream_info_t stream[MAX_CHANN_NUM];


static unsigned char frame_buf[2*1024*1024] = {0};

/*************************************/
static int g_wsxStatus = 0;
static stream_rpos_t g_stream_handle[2];

void st_param_setWsxStatus(int status)
{
    g_wsxStatus = status;
}





int thread_startVideoServer(void* param)
{
    cncMduServer_t	*p_mduServer = (cncMduServer_t*)param;
    int 			channel = p_mduServer->channel;
    int 			streamType = p_mduServer->streamType;
    int 			ret;
	int offset = 0;
    int first_play = 0;
    sdk_frame_t *frame = NULL;
    printf("thread_startVideoServer channel :%d streamType:%d \n",channel,streamType);

    if(channel > 2)
    {
        return -1;
    }
    memset(&g_stream_handle[channel],0,sizeof(stream_rpos_t));
    st_stream_getLastPos(channel+1, 0, &g_stream_handle[channel]);
    g_videoFlag[streamType][channel] = 1;
    while (g_videoFlag[streamType][channel])
    {
        if(st_stream_getOneFrame(channel+1,  0, 5, &g_stream_handle[channel]) > 0 )
        {
            frame = (char *)(&g_stream_handle[channel].p_buf_data[g_stream_handle[channel].data_start_pos]);
        }
        else
        {
            usleep(50*1000);
            continue;
        }
		
        if(0 == first_play && frame->frame_type != SDK_VIDEO_FRAME_I)
        {
            usleep(50*1000);
            continue;
        }
        

        st_frame_head_t  *st_frame_head = (st_frame_head_t *)frame_buf;

        st_frame_head->device_type = 0x54530900;

        st_frame_head->frame_no = frame->frame_no;         //帧序号;
        st_frame_head->video_resolution = 0;

        if(frame->frame_type == SDK_AUDIO_FRAME_A)
        {
            st_frame_head->frame_size = frame->frame_size;       //帧长度;
            st_frame_head->frame_type  = SANTACHI_A_FRAME_TYPE;

        }
        else
        {
            st_frame_head->frame_size = frame->frame_size;// - MAX_NAL_DESC_SIZE;       //帧长度;

            st_frame_head->frame_type = (frame->frame_type == SDK_VIDEO_FRAME_I)?SANTACHI_I_FRAME_TYPE:SANTACHI_P_FRAME_TYPE;
                                        st_frame_head->frame_rate = frame->video_info.frame_rate;

        }
		///if(first_play == 1 && frame->frame_type == SDK_VIDEO_FRAME_I)
		//	offset = 21 ;
		//else
			offset = 0 ;
		
        st_frame_head->sec = frame->sec;              //帧时间（秒）;
        st_frame_head->usec = frame->usec;             //帧时间（微秒）;
        st_frame_head->pts =  frame->pts;              //帧PTS
        st_frame_head->frame_size = frame->frame_size - offset;//
#if 0
        printf("ch %d frame no %d frame_type %d fram_size %d\n"
               , channel
               , st_frame_head->frame_no
               , st_frame_head->frame_type
               , st_frame_head->frame_size
              );
#endif
        memcpy((unsigned char *)frame_buf + sizeof(st_frame_head_t),(unsigned char *)frame->data + offset  ,frame->frame_size - offset );//+ MAX_NAL_DESC_SIZE
        if (st_cnc_sendVideoFrame(channel, streamType, (unsigned char *)frame_buf, sizeof(st_frame_head_t)+st_frame_head->frame_size) < 0)
        {
            pthread_detach(pthread_self());
            g_videoFlag[streamType][channel] = 0;
            break;
        }
		first_play = 1;
    }

    printf("thread video[%d][%d] quit\n", streamType, channel);
    return ret;
}
int 	thread_startMutexVideo(void *param)
{

#if 0
    int 				streamType = *(int*)param;
    int 				len, ret = 0;
    read_pos_t			readPos;
    net_frame_t			*p_frame;
    unsigned char		buf[512*1024];
    int 				ch = -1;

    printf("start mutex video thread\n");

    g_mutexVideoFlag = 1;
    while (g_mutexVideoFlag)
    {
        if (ch != g_mutexChannel)
        {
            ch = g_mutexChannel;
            memset(&readPos, 0, sizeof(read_pos_t));
            readPos.read_begin = st_stream_getNetStreamWritePos(ch, streamType);
            readPos.read_end = st_stream_getNetStreamWritePos(ch, streamType);
            st_codec_forceIFrame(ch, streamType);
        }

        p_frame = st_stream_getNetStreamFromPool(ch, streamType, &readPos);
        if (p_frame == NULL)
        {
            st_stream_netStreamWaiting(ch, streamType);
            continue;
        }

        st_stream_lockMutex(ch, streamType, readPos.read_begin);

        len = 0;
        memset(buf, 0, sizeof(buf));
        memcpy(buf, &p_frame->frame_head, sizeof(frame_head_t));
        len += sizeof(frame_head_t);
        memcpy(buf + len, p_frame->frame_data, p_frame->frame_head.frame_size);
        len += p_frame->frame_head.frame_size;
        ret = st_cnc_sendVideoFrame(MUTEX_CHANNEL, streamType, buf, len);

        st_stream_unlockMutex(ch, streamType, readPos.read_begin);

        if (ret < 0)
        {
            printf("st_cnc_sendVideoFrame error.\n");
            g_mutexVideoFlag = 0;
            break;
        }
    }
#endif

    printf("thread mutex video quit\n");
    return 0;
}


int 	thread_startPollVideo(void *param)
{
#if 0
    int 				streamType = *(int*)param;
    int 				second, len, ret = 0;
    read_pos_t			readPos;
    net_frame_t			*p_frame;
    unsigned char		buf[512*1024];
    special_channel_t	specialCh;
    int 				ch = 0;

    printf("start poll video thread.\n");

    g_pollVideoFlag = 1;
    while (g_pollVideoFlag)
    {
        st_param_getSpecialChannelStruct(&specialCh);

        if ((specialCh.change_channel >> ch) & 0x1)
        {
            second = time(NULL);
            memset(&readPos, 0, sizeof(read_pos_t));
            readPos.read_begin = st_stream_getNetStreamWritePos(ch, streamType);
            readPos.read_end = st_stream_getNetStreamWritePos(ch, streamType);
            st_codec_forceIFrame(ch, streamType);

            while (g_pollVideoFlag)
            {
                if (time(NULL) - second >= specialCh.interval_time)
                    break;

                p_frame = st_stream_getNetStreamFromPool(ch, streamType, &readPos);
                if (p_frame == NULL)
                {
                    st_stream_netStreamWaiting(ch, streamType);
                    continue;
                }

                st_stream_lockMutex(ch, streamType, readPos.read_begin);

                len = 0;
                memset(buf, 0, sizeof(buf));
                memcpy(buf, &p_frame->frame_head, sizeof(frame_head_t));
                len += sizeof(frame_head_t);
                memcpy(buf + len, p_frame->frame_data, p_frame->frame_head.frame_size);
                len += p_frame->frame_head.frame_size;
                ret = st_cnc_sendVideoFrame(POLL_CHANNEL, streamType, buf, len);

                st_stream_unlockMutex(ch, streamType, readPos.read_begin);

                if (ret < 0)
                {
                    printf("st_cnc_sendVideoFrame error, and quit\n");
                    g_pollVideoFlag = 0;
                    return -1;
                }
            }
        }
        ch++;
        ch %= g_cbFunc.channelNum;
    }
#endif
    printf("thread poll video quit\n");
    return 0;
}


int 	thread_startSendAudio(void *param)
{
    int 			ret = 0;

#if 0
    talk_frame_t	*p_frame;
    read_pos_t		readPos;

    memset(&readPos, 0, sizeof(read_pos_t));
    readPos.read_begin = st_stream_getTalkStreamWritePos(ENC_TO_NET);
    readPos.read_end = st_stream_getTalkStreamWritePos(ENC_TO_NET);

    g_audioFlag = 1;
    while (g_audioFlag)
    {
        p_frame = st_stream_getFrameFromTalkStream(ENC_TO_NET, &readPos);
        if (p_frame == NULL)
        {
            st_stream_talkStreamWaiting(ENC_TO_NET);
            continue;
        }

        if (p_frame->frame_head.frame_type != SANTACHI_A_FRAME_TYPE)
            continue;

        ret = st_cnc_sendAudioFrame((unsigned char*)p_frame,
                                    sizeof(frame_head_t) + p_frame->frame_head.frame_size);

        if (ret < 0)
        {
            printf("st_cnc_sendAudioFrame error.\n");
            g_audioFlag = 0;
            break;
        }
    }
#endif
    printf("thread audio quit\n");
    return ret;
}


int 	thread_startPlayServer(void *param)
{
#if 0
    cncVodServer_t 		*p_vodServer = (cncVodServer_t*)param;
    fragment_manage_t 	*fragmentManage;
    char				fileName[CNC_FILENAME_LEN] = {0};
    int 				fragementOffset, indexStart, indexEnd;
    int 				ret, frameHeadLen = sizeof(frame_head_t);
    frame_head_t		frameHead;
    unsigned char		buf[512*1024];

    sscanf(p_vodServer->fileName, "%25s:%03d:%08x-%08x",
           fileName, &fragementOffset, &indexStart, &indexEnd);

    ret = st_open_name(fileName, fragementOffset, indexStart, indexEnd, &fragmentManage);
    if(ret < 0)
    {
        printf("st_open_name error.\n");
        return ret;
    }

    st_lseek(fragmentManage, p_vodServer->startTime);
    while (1)
    {
        memset(&frameHead, 0, frameHeadLen);
        ret = st_read(fragmentManage, (char *)&frameHead, frameHeadLen);
        if (ret == 0)
        {
            printf("PLAY server complete. vodId = %d\n", p_vodServer->vodId);
            break;
        }
        else if (ret != frameHeadLen)
        {
            printf("read vod frame head. ret = %d\n", ret);
            break;
        }

        if (frameHead.sec == p_vodServer->endTime)
        {
            printf("PLAY server complete. vodId = %d\n", p_vodServer->vodId);
            ret = 0;
            break;
        }

        memset(buf, 0, sizeof(buf));
        ret = st_read(fragmentManage, buf + frameHeadLen, frameHead.frame_size);
        if (ret != frameHead.frame_size)
        {
            printf("read vod frame data error. ret = %d\n", ret);
            break;
        }

        memcpy(buf, &frameHead, frameHeadLen);
        ret = st_cnc_sendRecordFrame(p_vodServer->vodId, buf,
                                     frameHeadLen + frameHead.frame_size);
        if (ret < 0)
            break;
    }

    if (ret == 0)
    {
        memset(buf, 0, sizeof(buf));
        memcpy(buf, "end", sizeof("end"));
        st_cnc_sendRecordFrame(p_vodServer->vodId, buf, strlen(buf));
    }

    st_close_name(fragmentManage);
#endif
    return 0;
}


int 	thread_startRecordServer(void *param)
{
#if 0
    cncVodServer_t 		*p_vodServer = (cncVodServer_t*)param;
    int 				fragementOffset, indexStart, indexEnd;
    int 				ret, leftSize, sendSize, fileType;
    unsigned char		buf[4096] = {0};
    char				fileName[CNC_FILENAME_LEN] = {0};
    fragment_manage_t 	*fragmentManage = NULL;

    if (strstr(p_vodServer->fileName, ".jpeg") != NULL)
    {
        fileType = 1;
        sscanf(p_vodServer->fileName, "%25s:%03d", fileName, &fragementOffset);
        ret = st_jpeg_open_name(fileName, fragementOffset, &fragmentManage);
    }
    else
    {
        fileType = 2;
        sscanf(p_vodServer->fileName, "%25s:%03d:%08x-%08x",
               fileName, &fragementOffset, &indexStart, &indexEnd);
        ret = st_open_name(fileName, fragementOffset, indexStart,
                           indexEnd, &fragmentManage);
    }
    if(ret < 0)
    {
        printf("st_open_name error.\n");
        return ret;
    }

    leftSize = p_vodServer->fileSize;
    while (leftSize)
    {
        sendSize = (leftSize > sizeof(buf)) ? sizeof(buf) : leftSize;

        if (fileType == 1)
            ret = st_jpeg_read(fragmentManage, buf, sendSize);
        else if (fileType == 2)
            ret = st_read(fragmentManage, buf, sendSize);
        leftSize -= ret;

        if (ret <= 0)
        {
            printf("Download complete. vodId = %d, fileName = %s\n",
                   p_vodServer->vodId, p_vodServer->fileName);
            break;
        }

        if (st_cnc_sendRecordFrame(p_vodServer->vodId, buf, ret) < 0)
        {
            printf("st_cnc_sendRecordFrame() error.\n");
            break;
        }

    }

    if (leftSize == 0)
    {
        memset(buf, 0, sizeof(buf));
        memcpy(buf, "end", sizeof("end"));
        st_cnc_sendRecordFrame(p_vodServer->vodId, buf, strlen(buf));
    }

    if (fileType == 1)
        st_jpeg_close_name(fragmentManage);
    if (fileType == 2)
        st_close_name(fragmentManage);
#endif
    return 0;
}
/************************************/
static int getCurrentTime(cncTime_t *current_time)
{
    time_t tt;
    int ret = 0;
    struct tm local_time;
    ret = time(&tt);
    if(ret < 0)
    {
        printf("get time error.\n");
        return 0;
    }
    local_time = *(localtime(&tt));

    current_time->year = local_time.tm_year + 1900;
    current_time->month = local_time.tm_mon + 1;
    current_time->date = local_time.tm_mday;
    current_time->hour = local_time.tm_hour;
    current_time->minute = local_time.tm_min;
    current_time->second = local_time.tm_sec;

    printf("%d-%d-%d,%d:%d:%d\n", current_time->year, current_time->month, current_time->date

           , current_time->hour, current_time->minute, current_time->second);


    return 1;
}

int     st_3512_dealCsgCommand(const char *command, void* structBuf)
{
    if (command == NULL || structBuf == NULL)
    {
        //st_dbg_print(DBG,L_NORMAL,"st_dealCsgCommand(), The input arg is error.\n");
        printf("st_dealCsgCommand(), The input arg is error.\n");
        return -1;
    }
    printf("==================================== command:%s \n",command);
    if (strcmp(command, "PlayAudioFrame") == 0)
    {
        /*need debug*/
    }

    else if (strcmp(command, "GetPlatformInfo") == 0)
    {
        cncPlatform_t *p_platform = (cncPlatform_t*)structBuf;
#if 1
        strncpy(p_platform->puId, "1002000100000000000000000002", CNC_ID_LEN-1);

        strncpy(p_platform->puName, "admin", CNC_ID_LEN-1);
        strncpy(p_platform->puPasswd, "123456", CNC_ID_LEN-1);

        p_platform->csgPort = 9901;
        p_platform->mduPort = 9910;
        strncpy(p_platform->csgIp, "172.16.1.200", CNC_IP_LEN-1);
        strncpy(p_platform->mduIp,  "172.16.1.200", CNC_IP_LEN-1);
        p_platform->enableFlag = 1;
        p_platform->protocolType = 1;
        p_platform->offUpload = 0;
        p_platform->offRecord = 0;
        printf("YYYYYYYYYYYYYY 1111111111\n");
     //   wsx_PlatformInfo(1,0, structBuf, sizeof(cncPlatform_t));
        ///printf("YYYYYYYYYYYYYY 22222222222\n");
#else
        wsx_PlatformInfo(0,0, structBuf, sizeof(cncPlatform_t));
#endif
    }

    else if (strcmp(command, "SetPlatformInfo") == 0)
    {
        ///  cncPlatform_t *p_platform = (cncPlatform_t*)structBuf;

      //  wsx_PlatformInfo(0,0, structBuf, sizeof(cncPlatform_t));
    }
#if 0
    else if (strcmp(command, "GetPuBasicCfgInfo") == 0)
    {
        cncBasic_t *p_basic = (cncBasic_t*)structBuf;

        record_param_t info;
        memset(&info, 0, sizeof(record_param_t));
        st_param_getRecParamStruct(&info);

        p_basic->coverType = info.cover_type;
        p_basic->prerecStatus = info.prerec_status;
        p_basic->switchFileTime = info.switch_file_time;
        p_basic->twoStreamFlag = info.ds_enable_flag;
        p_basic->manualRecordTime = info.manual_record_time;
        p_basic->deleteSpace = info.delete_space;
        p_basic->deleteSize = info.delete_unit;
    }
    else if (strcmp(command, "SetPuBasicCfgInfo") == 0)
    {
        cncBasic_t *p_basic = (cncBasic_t*)structBuf;

        record_param_t info;
        memset(&info, 0, sizeof(record_param_t));
        st_param_getRecParamStruct(&info);

        info.cover_type = p_basic->coverType;
        info.prerec_status = p_basic->prerecStatus;
        info.switch_file_time = p_basic->switchFileTime;
        info.ds_enable_flag = p_basic->twoStreamFlag;
        info.manual_record_time = p_basic->manualRecordTime;
        info.delete_space = p_basic->deleteSpace;
        info.delete_unit = p_basic->deleteSize;

        st_param_setRecParamStruct(info);
    }
#endif

    else if (strcmp(command, "GetPuIpInfo") == 0)
    {
       // wsx_PuIpInfo(0,0, structBuf, sizeof(cncIp_t));

    }

    else if (strcmp(command, "GetPuStreamInfo") == 0)
    {
    }
    else if (strcmp(command, "GetPuTime") == 0)
    {
        int ret = 0;
        cncTime_t *current_time = (cncTime_t*)structBuf;

        ret =getCurrentTime(current_time);
        if(ret < 0)
        {
            printf("Get Current Time ERROR.\n");
        }

    }
    else if (strcmp(command, "SetPuTime") == 0)
    {
        cncTime_t *p_time = (cncTime_t*)structBuf;
        int ret = 0;
        struct timeval tv;
        struct tm time;

        time.tm_year = p_time->year - 1900;
        time.tm_mon = p_time->month - 1;
        time.tm_mday = p_time->date;
        time.tm_hour = p_time->hour;
        time.tm_min = p_time->minute;
        time.tm_sec = p_time->second;
        tv.tv_sec = mktime(&time);

        tv.tv_usec = 0;
        ret = settimeofday(&tv, NULL);
        if(ret < 0)
        {
            perror("SET SYSTEM TIME ERROR.\n");
            return -1;
        }

        printf("SETTING RTC TIME.\n");
    }

    else if (strcmp(command, "GetPuImageEncodePara") == 0)
    {
        //wsx_EncodeInfo(0,0, structBuf, sizeof(cncEncode_t));

    }

    else if (strcmp(command, "SetPuImageEncodePara") == 0)
    {
       // wsx_EncodeInfo(1,0, structBuf, sizeof(cncEncode_t));

    }

    else if (strcmp(command, "SetPuImageEncodeParaEx") == 0)
    {
        printf("============SetPuImageEncodeParaEx\n\n");
#if 0
        cncEncode_t *p_encode = (cncEncode_t*)structBuf;
        net_msg_t msg;
        memset(&msg, 0, sizeof(net_msg_t));

        int channel_no = 0;
        unsigned short msg_subtype = 0;
        video_encode_t* info = (video_encode_t*)(msg.msg_data);

        memset(&msg, 0, sizeof(net_msg_t));

        channel_no = p_encode->videoId;

        //if (p_encode->guType == 6)
        /*if(p_encode->streamType == 1)
            msg_subtype = PARAM_SLAVE_ENCODE;
        else
            msg_subtype = PARAM_VIDEO_ENCODE;

        cnc_setGetIpcParam(channel_no, MSG_GET_PARAM, msg_subtype, &msg, 0);*/

        info->video_format = p_encode->videoFormat;
        info->frame_rate = p_encode->frameRate;
        info->Iframe_interval = p_encode->IframeInterval;
        info->encode_type = p_encode->encodeType;
        info->resolution = p_encode->resolution;
        info->bitrate_type = p_encode->bitType;
        info->Qp = p_encode->Qp;
        info->prefer_frame = p_encode->preferFrame;
        info->level = p_encode->level;

        cnc_setGetIpcParam(channel_no, MSG_SET_PARAM, msg_subtype, (void *)info, sizeof(video_encode_t));
#endif
    }

    else if (strcmp(command, "GetPuImageDisplayPara") == 0)
    {
       // wsx_ImageInfo(0,0, structBuf, sizeof(cncDisplay_t));
    }
    else if (strcmp(command, "SetPuImageDisplayPara") == 0)
    {
       // wsx_ImageInfo(1,0, structBuf, sizeof(cncDisplay_t));
    }

    else if (strcmp(command, "DefaultPuImageDisplayPara") == 0)
    {
        cncDisplay_t *p_display = (cncDisplay_t*)structBuf;
        //多采用明文
        p_display->contrast = 0x80;
        p_display->bright = 0x80;
        p_display->hue = 0x80;
        p_display->saturation = 0x80;
        //wsx_ImageInfo(1,0, structBuf, sizeof(cncDisplay_t));

    }
#if 0
    else if (strcmp(command, "GetPuImageTextPara") == 0)
    {
        cncOSD_t    *p_OSD = (cncOSD_t*)structBuf;
        net_msg_t msg;
        memset(&msg, 0, sizeof(net_msg_t));
        int channel_no = 0;
        channel_no = p_OSD->videoId;
        osd_info_t* info = (osd_info_t*)(msg.msg_data);

        memset(&msg, 0, sizeof(net_msg_t));
        msg.msg_head.msg_type = MSG_GET_PARAM;
        msg.msg_head.msg_subtype = PARAM_OSD_INFO;
        st_nvr_getOsdInfo(p_OSD->videoId, info);
        //cnc_setGetIpcParam(channel_no, MSG_GET_PARAM, PARAM_OSD_INFO, &msg, 0);

        p_OSD->bitmapTimeEnable = info->osd_time.enable;
        p_OSD->bitmapX = info->osd_time.x_pos;
        p_OSD->bitmapY = info->osd_time.y_pos;
        p_OSD->clrTime = info->osd_time.color_red +
                         (info->osd_time.color_green << 8) + (info->osd_time.color_blue << 16);

        p_OSD->bitmapTextEnable = info->osd_logo.enable;
        bzero(p_OSD->bitmapText, CNC_OSDTEXT_LEN);
        strncpy(p_OSD->bitmapText, info->osd_logo.logo, strlen(info->osd_logo.logo));
        p_OSD->bitmapTextX = info->osd_logo.x_pos;
        p_OSD->bitmapTextY = info->osd_logo.y_pos;
        p_OSD->clrText = info->osd_logo.color_red +
                         (info->osd_logo.color_green << 8) + (info->osd_logo.color_blue << 16);

        //strncpy(p_OSD->bitmapText, info->osd_logo.logo, CNC_OSDTEXT_LEN-1);
    }
    else if (strcmp(command, "SetPuImageTextPara") == 0)
    {
        cncOSD_t    *p_OSD = (cncOSD_t*)structBuf;
        net_msg_t msg;
        memset(&msg, 0, sizeof(net_msg_t));
        int channel_no = 0;
        channel_no = p_OSD->videoId;
        osd_info_t* info = (osd_info_t*)(msg.msg_data);

        //cnc_setGetIpcParam(channel_no, MSG_SET_PARAM, PARAM_OSD_INFO, &msg, 0);
        st_nvr_getOsdInfo(p_OSD->videoId, info);
        info->osd_time.enable = p_OSD->bitmapTimeEnable;
        info->osd_time.x_pos = p_OSD->bitmapX;
        info->osd_time.y_pos = p_OSD->bitmapY;
        info->osd_time.color_red = p_OSD->clrTime & 0xff;
        info->osd_time.color_green = (p_OSD->clrTime >> 8) & 0xff;
        info->osd_time.color_blue = (p_OSD->clrTime >> 16) & 0xff;

        info->osd_logo.enable = p_OSD->bitmapTextEnable;
        info->osd_logo.x_pos = p_OSD->bitmapTextX;
        info->osd_logo.y_pos = p_OSD->bitmapTextY;
        info->osd_logo.color_red = p_OSD->clrText & 0xff;
        info->osd_logo.color_green = (p_OSD->clrText >> 8) & 0xff;
        info->osd_logo.color_blue = (p_OSD->clrText  >> 16) & 0xff;
        strncpy(info->osd_logo.logo, p_OSD->bitmapText, CNC_OSDTEXT_LEN-1);

        st_nvr_setOsdInfo(p_OSD->videoId, info);
        //cnc_setGetIpcParam(channel_no, MSG_SET_PARAM, PARAM_OSD_INFO, info, sizeof(osd_info_t));
    }
    else if (strcmp(command, "GetPuOsdExPara") == 0)
    {
#if 0
        cncOSDEx_t  *p_OSDEx = (cncOSDEx_t*)structBuf;

        p_OSDEx->gpsEnable = 1;
        p_OSDEx->gpsX = 101;
        p_OSDEx->gpsY = 202;
        p_OSDEx->gpsClr = 12345678;

        p_OSDEx->exEnable = 1;
        strncpy(p_OSDEx->exStr, "hello world", CNC_OSDTEXT_LEN-1);
        p_OSDEx->exX = 303;
        p_OSDEx->exY = 404;
        p_OSDEx->exClr = 87654321;
#endif
    }
    else if (strcmp(command, "SetPuOsdExPara") == 0)
    {
#if 0
        cncOSDEx_t  *p_OSDEx = (cncOSDEx_t*)structBuf;
        /*st_dbg_print(DBG,L_NORMAL,"p_OSDEx->videoId(%d)\n", p_OSDEx->videoId);

        st_dbg_print(DBG,L_NORMAL,"p_OSDEx->gpsEnable(%d)\n", p_OSDEx->gpsEnable);
        st_dbg_print(DBG,L_NORMAL,"p_OSDEx->gpsX(%d)\n", p_OSDEx->gpsX);
        st_dbg_print(DBG,L_NORMAL,"p_OSDEx->gpsY(%d)\n", p_OSDEx->gpsY);
        st_dbg_print(DBG,L_NORMAL,"p_OSDEx->gpsClr(%d)\n", p_OSDEx->gpsClr);

        st_dbg_print(DBG,L_NORMAL,"p_OSDEx->exEnable(%d)\n", p_OSDEx->exEnable);
        st_dbg_print(DBG,L_NORMAL,"p_OSDEx->exStr(%s)\n", p_OSDEx->exStr);
        st_dbg_print(DBG,L_NORMAL,"p_OSDEx->exX(%d)\n", p_OSDEx->exX);
        st_dbg_print(DBG,L_NORMAL,"p_OSDEx->exY(%d)\n", p_OSDEx->exY);
        st_dbg_print(DBG,L_NORMAL,"p_OSDEx->exClr(%d)\n", p_OSDEx->exClr);*/
#endif
    }

    else if (strcmp(command, "GetPuSerialPort") == 0)
    {
        cncSerialPort_t *p_serialPort = (cncSerialPort_t*)structBuf;

        serial_info_t   info;
        memset(&info, 0, sizeof(serial_info_t));

        st_param_getSerialStruct(p_serialPort->serialNo, &info);

        p_serialPort->baudRate = info.baut_rate;
        p_serialPort->dataBit = info.data_bit;
        p_serialPort->stopBit = info.stop_bit;
        p_serialPort->verifyRule = info.verify_rule;

    }
    else if (strcmp(command, "SetPuSerialPort") == 0)
    {
        cncSerialPort_t *p_serialPort = (cncSerialPort_t*)structBuf;

        serial_info_t   info;
        memset(&info, 0, sizeof(serial_info_t));
        st_param_getSerialStruct(p_serialPort->serialNo, &info);

        info.baut_rate = p_serialPort->baudRate;
        info.data_bit = p_serialPort->dataBit;
        info.stop_bit = p_serialPort->stopBit;
        info.verify_rule = p_serialPort->verifyRule;

        st_param_setSerialStruct(p_serialPort->serialNo, info);
    }

    else if (strcmp(command, "GetPuPtzParam") == 0)
    {
#if 0
        cncPtz_t        *p_PTZ = (cncPtz_t*)structBuf;

        ptz_config_t    info;
        memset(&info, 0, sizeof(ptz_config_t));
        st_param_getTerminateStruct(p_PTZ->videoId, &info);

        p_PTZ->ptzAddr = info.ptz_addr;
        p_PTZ->ptzType = info.ptz_type;
        p_PTZ->serialPort.serialNo = info.serial_no;
        p_PTZ->serialPort.baudRate = info.serial_info.baut_rate;
        p_PTZ->serialPort.dataBit = info.serial_info.data_bit;
        p_PTZ->serialPort.stopBit = info.serial_info.stop_bit;
        p_PTZ->serialPort.verifyRule = info.serial_info.verify_rule;
#endif
        cncPtz_t *p_PTZ = (cncPtz_t *)structBuf;
        net_msg_t msg;
        ptz_config_t *info = (ptz_config_t *)(msg.msg_data);
        bzero(&msg, sizeof(net_msg_t));
        int channel_no = 0;
        channel_no = p_PTZ->videoId;

//		cnc_setGetIpcParam(channel_no, MSG_GET_PARAM, PARAM_TERM_CONFIG, &msg, 0);
        st_nvr_getTermConfig(channel_no, info);
        p_PTZ->ptzAddr = info->ptz_addr;
        p_PTZ->ptzType = info->ptz_type;
        p_PTZ->serialPort.serialNo = info->serial_no;
        p_PTZ->serialPort.baudRate = info->serial_info.baut_rate;
        p_PTZ->serialPort.dataBit = info->serial_info.data_bit;
        p_PTZ->serialPort.stopBit = info->serial_info.stop_bit;
        p_PTZ->serialPort.verifyRule = info->serial_info.verify_rule;

    }
    else if (strcmp(command, "SetPuPtzParam") == 0)
    {
#if 0
        cncPtz_t        *p_PTZ = (cncPtz_t*)structBuf;

        ptz_config_t    info;
        memset(&info, 0, sizeof(ptz_config_t));
        st_param_getTerminateStruct(p_PTZ->videoId, &info);

        info.ptz_addr = p_PTZ->ptzAddr;
        info.ptz_type = p_PTZ->ptzType;
        info.serial_no = p_PTZ->serialPort.serialNo;
        info.serial_info.baut_rate = p_PTZ->serialPort.baudRate;
        info.serial_info.data_bit = p_PTZ->serialPort.dataBit;
        info.serial_info.stop_bit = p_PTZ->serialPort.stopBit;
        info.serial_info.verify_rule = p_PTZ->serialPort.verifyRule;

        st_param_setTerminateStruct(p_PTZ->videoId, info);
#endif
        cncPtz_t *p_PTZ = (cncPtz_t *)structBuf;
        net_msg_t msg;
        bzero(&msg, sizeof(net_msg_t));
        ptz_config_t *info = (ptz_config_t *)(msg.msg_data);

        int channel_no = 0;
        channel_no = p_PTZ->videoId;
//		cnc_setGetIpcParam(channel_no, MSG_GET_PARAM, PARAM_TERM_CONFIG, &msg, 0);
        info->ptz_addr = p_PTZ->ptzAddr;
        info->ptz_type = p_PTZ->ptzType;
        info->serial_no = p_PTZ->serialPort.serialNo;
        info->serial_info.baut_rate = p_PTZ->serialPort.baudRate;
        info->serial_info.data_bit = p_PTZ->serialPort.dataBit;
        info->serial_info.stop_bit = p_PTZ->serialPort.stopBit;
        info->serial_info.verify_rule = p_PTZ->serialPort.verifyRule;
//		cnc_setGetIpcParam(channel_no, MSG_SET_PARAM, PARAM_TERM_CONFIG, info, sizeof(ptz_config_t));
        st_nvr_setTermConfig(channel_no, info);
    }

    else if (strcmp(command, "GetPuLocalStorageTask") == 0)
    {
#if 1
        cncRecordingSeg_t *p_recordSeg = (cncRecordingSeg_t*)structBuf;

        timer_week_t info;
        int i, j;
        for (i = 0; i < NT200H_MAX_WEEK_DAY; i++)
        {
            memset(&info, 0, sizeof(timer_week_t));
            st_param_getTimerStruct(p_recordSeg->videoId, i, &info);

            p_recordSeg->enableFLag[i] =  info.enable_flag;
            for (j = 0; j < NT200H_MAX_TIME_SEG_NUM; j++)
            {
                p_recordSeg->recTimeSeg[i][j].startHour = info.time_seg[j].start_hour;
                p_recordSeg->recTimeSeg[i][j].startMinute = info.time_seg[j].start_minute;
                p_recordSeg->recTimeSeg[i][j].startSecond = info.time_seg[j].start_second;
                p_recordSeg->recTimeSeg[i][j].endHour = info.time_seg[j].end_hour;
                p_recordSeg->recTimeSeg[i][j].endMinute = info.time_seg[j].end_minute;
                p_recordSeg->recTimeSeg[i][j].endSecond = info.time_seg[j].end_second;
            }
        }
#else
        //added 2013.4.2
        net_msg_t msg;
        bzero(&msg, sizeof(net_msg_t));
        timer_record_t *info = (timer_record_t *)(msg.msg_data);
        cncRecordingSeg_t *p_recordSeg = (cncRecordingSeg_t *)structBuf;

        unsigned char channel_no = 0;
        channel_no = p_recordSeg->videoId;

        cnc_setGetIpcParam(channel_no, MSG_GET_PARAM, PARAM_TIMER_RECORD, &msg, 0);
        int i ,j;
        for(i=0; i < NT200H_MAX_WEEK_DAY; i++)
        {
            p_recordSeg->enableFLag[i] = info->timer_week[i].enable_flag;
            printf("enable_flag[%d]:%d\n", i, info->timer_week[i].enable_flag);
            for(j = 0; j< NT200H_MAX_TIME_SEG_NUM; j++)
            {
                p_recordSeg->recTimeSeg[i][j].startHour = info->timer_week[i].time_seg[j].start_hour;
                p_recordSeg->recTimeSeg[i][j].startMinute = info->timer_week[i].time_seg[j].start_minute;
                p_recordSeg->recTimeSeg[i][j].startSecond = info->timer_week[i].time_seg[j].start_second;
                p_recordSeg->recTimeSeg[i][j].endHour = info->timer_week[i].time_seg[j].end_hour;
                p_recordSeg->recTimeSeg[i][j].endMinute = info->timer_week[i].time_seg[j].end_minute;
                p_recordSeg->recTimeSeg[i][j].endSecond = info->timer_week[i].time_seg[j].end_second;
            }
        }
#endif
    }
    else if (strcmp(command, "SetPuLocalStorageTask") == 0)
    {
#if 1
        cncRecordingSeg_t *p_recordSeg = (cncRecordingSeg_t*)structBuf;

        timer_week_t info;
        int i, j;

        for (i = 0; i < NT200H_MAX_WEEK_DAY; i++)
        {
            memset(&info, 0, sizeof(timer_week_t));
            st_param_getTimerStruct(p_recordSeg->videoId, i, &info);

            info.enable_flag = p_recordSeg->enableFLag[i];
            for (j = 0; j < NT200H_MAX_TIME_SEG_NUM; j++)
            {
                info.time_seg[j].start_hour = p_recordSeg->recTimeSeg[i][j].startHour;
                info.time_seg[j].start_minute = p_recordSeg->recTimeSeg[i][j].startMinute;
                info.time_seg[j].start_second = p_recordSeg->recTimeSeg[i][j].startSecond;
                info.time_seg[j].end_hour = p_recordSeg->recTimeSeg[i][j].endHour;
                info.time_seg[j].end_minute = p_recordSeg->recTimeSeg[i][j].endMinute;
                info.time_seg[j].end_second = p_recordSeg->recTimeSeg[i][j].endSecond;
            }
            st_param_setTimerStruct(p_recordSeg->videoId, i, info);
        }
#else
        //added 2013.4.2
        cncRecordingSeg_t *p_recordSeg = (cncRecordingSeg_t *)structBuf;
        net_msg_t msg;
        bzero(&msg, sizeof(net_msg_t));
        unsigned char channel_no = p_recordSeg->videoId;
        timer_record_t *info = (timer_record_t *)(msg.msg_data);
        int i,j;

        cnc_setGetIpcParam(channel_no, MSG_GET_PARAM, PARAM_TIMER_RECORD, &msg, 0);

        for(i = 0; i < NT200H_MAX_WEEK_DAY; i++)
        {
            info->timer_week[i].enable_flag = p_recordSeg->enableFLag[i];
            for(j=0; j < NT200H_MAX_TIME_SEG_NUM; j++)
            {
                info->timer_week[i].time_seg[j].start_hour = p_recordSeg->recTimeSeg[i][j].startHour;
                info->timer_week[i].time_seg[j].start_minute = p_recordSeg->recTimeSeg[i][j].startMinute;
                info->timer_week[i].time_seg[j].start_second = p_recordSeg->recTimeSeg[i][j].startSecond;
                info->timer_week[i].time_seg[j].end_hour = p_recordSeg->recTimeSeg[i][j].endHour;
                info->timer_week[i].time_seg[j].end_minute = p_recordSeg->recTimeSeg[i][j].endMinute;
                info->timer_week[i].time_seg[j].end_second = p_recordSeg->recTimeSeg[i][j].endSecond;
            }
        }
        cnc_setGetIpcParam(channel_no, MSG_SET_PARAM, PARAM_TIMER_RECORD, info, sizeof(timer_record_t));
#endif
    }

    else if (strcmp(command, "GetPuImageHideArea") == 0)
    {
#if 0
        cncHide_t   *p_hide = (cncHide_t*)structBuf;

        int i;
        overlay_info_t info;
        memset(&info, 0, sizeof(info));
        st_param_getOverlayStruct(p_hide->videoId, &info);

        p_hide->HideAreaEnabled = info.enable;
        p_hide->HideAreaNum = info.num;
        for (i = 0; i < p_hide->HideAreaNum; i++)
        {
            p_hide->HideAreaItem[i].color = info.overlay[i].color;
            p_hide->HideAreaItem[i].xStart = info.overlay[i].x_start;
            p_hide->HideAreaItem[i].yStart = info.overlay[i].y_start;
            p_hide->HideAreaItem[i].width = info.overlay[i].width;
            p_hide->HideAreaItem[i].height = info.overlay[i].height;
        }
#endif
        //added 2013.4.2
        cncHide_t *p_hide = (cncHide_t *)structBuf;
        net_msg_t msg;
        bzero(&msg, sizeof(net_msg_t));
        overlay_info_t *info = (overlay_info_t *)(msg.msg_data);
        int channel_no = p_hide->videoId;
        st_nvr_getOverlayInfo(p_hide->videoId, info);
        //cnc_setGetIpcParam(channel_no, MSG_GET_PARAM, PARAM_OVERLAY_INFO, &msg, 0);
        p_hide->HideAreaEnabled = info->enable;
        p_hide->HideAreaNum = info->num;
        int i;
        for(i= 0; i < p_hide->HideAreaNum; i++)
        {
            p_hide->HideAreaItem[i].color = info->overlay[i].color;
            p_hide->HideAreaItem[i].xStart =  info->overlay[i].x_start;
            p_hide->HideAreaItem[i].yStart = info->overlay[i].y_start;
            p_hide->HideAreaItem[i].width = info->overlay[i].width;
            p_hide->HideAreaItem[i].height = info->overlay[i].height;
        }
    }
    else if (strcmp(command, "SetPuImageHideArea") == 0)
    {
#if 0
        cncHide_t   *p_hide = (cncHide_t*)structBuf;

        int i;
        overlay_info_t info;
        memset(&info, 0, sizeof(info));
        st_param_getOverlayStruct(p_hide->videoId, &info);

        info.enable = p_hide->HideAreaEnabled;
        info.num = p_hide->HideAreaNum;
        for (i = 0; i < p_hide->HideAreaNum; i++)
        {
            info.overlay[i].color = p_hide->HideAreaItem[i].color;
            info.overlay[i].x_start = p_hide->HideAreaItem[i].xStart;
            info.overlay[i].y_start = p_hide->HideAreaItem[i].yStart;
            info.overlay[i].width = p_hide->HideAreaItem[i].width;
            info.overlay[i].height = p_hide->HideAreaItem[i].height;
        }

        st_param_setOverlayStruct(p_hide->videoId, info);
        ]
#endif
        //added 2013.4.2
        cncHide_t *p_hide = (cncHide_t *)structBuf;
        net_msg_t msg;
        bzero(&msg, sizeof(net_msg_t));
        overlay_info_t *info = (overlay_info_t *)(msg.msg_data);
        //bzero(&info, sizeof(overlay_info_t));	//SIGSEGV
        int channel_no = 0;
        channel_no = p_hide->videoId;

        //cnc_setGetIpcParam(channel_no, MSG_GET_PARAM, PARAM_OVERLAY_INFO, &msg, 0);
        st_nvr_getOverlayInfo(p_hide->videoId, info);
        info->enable = p_hide->HideAreaEnabled;
        info->num = p_hide->HideAreaNum;

        int i = 0;
        for (i = 0; i < p_hide->HideAreaNum; i++)
    {
        info->overlay[i].color = p_hide->HideAreaItem[i].color;
            info->overlay[i].x_start = p_hide->HideAreaItem[i].xStart;
            info->overlay[i].y_start = p_hide->HideAreaItem[i].yStart;
            info->overlay[i].width = p_hide->HideAreaItem[i].width;
            info->overlay[i].height = p_hide->HideAreaItem[i].height;
            printf("color:%d, x_start:%d, y_start:%d, widty:%d, height:%d.\n", info->overlay[i].color
                   , info->overlay[i].x_start, info->overlay[i].y_start, info->overlay[i].width, info->overlay[i].height);
        }
        st_nvr_setOverlayInfo(p_hide->videoId, info);
        //cnc_setGetIpcParam(channel_no, MSG_SET_PARAM, PARAM_OVERLAY_INFO, info, sizeof(overlay_info_t));
    }

    else if (strcmp(command, "GetPuMotionDetectionCfg") == 0)
    {
#if 0
        cncMotionDetection_t *p_motion = (cncMotionDetection_t*)structBuf;

        int i;
        video_day_t info;
        memset(&info, 0, sizeof(video_day_t));
        st_param_getVideoMoveStruct(p_motion->videoId, p_motion->weekday, &info);

        p_motion->moveGuardFlag = info.move_guard_flag;
        p_motion->moveEnable = info.move_enable;
        p_motion->beepOutputEnable = info.beep_output_enable;
        p_motion->jointSnapEnableCh = info.joint_snap_enable_ch;
        p_motion->jointRecordEnableCh = info.joint_record_enable_ch;
        p_motion->jointOutputEnableCh = info.joint_output_enable_ch;
        p_motion->sensitiveLevel = info.sensitive_level;
        p_motion->alarmIntervalTime = info.alarm_interval_time;
        p_motion->jointRecordTime = info.joint_record_time;
        p_motion->jointOutputTime = info.joint_output_time;
        p_motion->beepOutputTime = info.beep_output_time;
        for (i = 0; i < 12; i++)
            p_motion->motionarea[i] = info.motion_area.area[i];
        for (i = 0; i < NT200H_MAX_TIME_SEG_NUM; i++)
        {
            p_motion->timeseg[i].startHour = info.time_seg[i].start_hour;
            p_motion->timeseg[i].startMinute = info.time_seg[i].start_minute;
            p_motion->timeseg[i].startSecond = info.time_seg[i].start_second;
            p_motion->timeseg[i].endHour = info.time_seg[i].end_hour;
            p_motion->timeseg[i].endMinute = info.time_seg[i].end_minute;
            p_motion->timeseg[i].endSecond = info.time_seg[i].end_second;
        }
#endif
        //added 2013.4.2
        cncMotionDetection_t *p_motion = (cncMotionDetection_t*)structBuf;
        net_msg_t msg;
        bzero(&msg, sizeof(net_msg_t));
        unsigned int day = p_motion->weekday;
        int channel_no = p_motion->videoId;
        video_move_t *info = (video_move_t *)(msg.msg_data);

        //st_nvr_getVideoMove(p_motion->videoId, info);
        st_nvr_getVideoMoveEx(channel_no, info);
        //cnc_setGetIpcParam(channel_no, MSG_GET_PARAM, PARAM_VIDEO_MOVE, &msg, 0);
        p_motion->moveGuardFlag = info->video_day[day].move_guard_flag;
        p_motion->moveEnable = info->video_day[day].move_enable;
        p_motion->beepOutputEnable = info->video_day[day].beep_output_enable;
        p_motion->jointSnapEnableCh = info->video_day[day].joint_snap_enable_ch;
        p_motion->jointRecordEnableCh= info->video_day[day].joint_record_enable_ch;
        p_motion->jointOutputEnableCh = info->video_day[day].joint_output_enable_ch;
        p_motion->sensitiveLevel = info->video_day[day].sensitive_level;
        p_motion->alarmIntervalTime = info->video_day[day].alarm_interval_time;
        p_motion->jointRecordTime = info->video_day[day].joint_record_time;
        p_motion->jointOutputTime = info->video_day[day].joint_output_time;
        p_motion->beepOutputTime = info->video_day[day].beep_output_time;
        int i = 0;
        for (i = 0; i < 12; i++)
            p_motion->motionarea[i] = info->video_day[day].motion_area.area[i];
        for (i = 0; i < NT200H_MAX_TIME_SEG_NUM; i++)
        {
            p_motion->timeseg[i].startHour = info->video_day[day].time_seg[i].start_hour;
            p_motion->timeseg[i].startMinute = info->video_day[day].time_seg[i].start_minute;
            p_motion->timeseg[i].startSecond = info->video_day[day].time_seg[i].start_second;
            p_motion->timeseg[i].endHour = info->video_day[day].time_seg[i].end_hour;
            p_motion->timeseg[i].endMinute = info->video_day[day].time_seg[i].end_minute;
            p_motion->timeseg[i].endSecond = info->video_day[day].time_seg[i].end_second;
        }

        printf("weekday:%d.\n", day);
        printf("timeseg = %02d%02d%02d-%02d%02d%02d,%02d%02d%02d-%02d%02d%02d,%02d%02d%02d-%02d%02d%02d,%02d%02d%02d-%02d%02d%02d",
               p_motion->timeseg[0].startHour,  p_motion->timeseg[0].startMinute,
               p_motion->timeseg[0].startSecond, p_motion->timeseg[0].endHour,
               p_motion->timeseg[0].endMinute, p_motion->timeseg[0].endMinute,
               p_motion->timeseg[1].startHour,  p_motion->timeseg[1].startMinute,
               p_motion->timeseg[1].startSecond, p_motion->timeseg[1].endHour,
               p_motion->timeseg[1].endMinute, p_motion->timeseg[1].endMinute,
               p_motion->timeseg[2].startHour,  p_motion->timeseg[2].startMinute,
               p_motion->timeseg[2].startSecond, p_motion->timeseg[2].endHour,
               p_motion->timeseg[2].endMinute, p_motion->timeseg[2].endMinute,
               p_motion->timeseg[3].startHour,  p_motion->timeseg[3].startMinute,
               p_motion->timeseg[3].startSecond, p_motion->timeseg[3].endHour,
               p_motion->timeseg[3].endMinute, p_motion->timeseg[3].endMinute);

    }
    else if (strcmp(command, "SetPuMotionDetectionCfg") == 0)
    {
#if 0
        cncMotionDetection_t *p_motion = (cncMotionDetection_t*)structBuf;

        int         i, j;
        int         day = p_motion->weekday;
        video_day_t info;

        if (day == NT200H_MAX_WEEK_DAY)
        {
            day = 0;
            j = 0;
        }
        else
            j = NT200H_MAX_WEEK_DAY - 1;

        for (; j < NT200H_MAX_WEEK_DAY; j++)
        {
            memset(&info, 0, sizeof(video_day_t));
            st_param_getVideoMoveStruct(p_motion->videoId, day, &info);

            info.move_guard_flag = p_motion->moveGuardFlag;
            info.move_enable = p_motion->moveEnable;
            info.beep_output_enable = p_motion->beepOutputEnable;
            info.joint_snap_enable_ch = p_motion->jointSnapEnableCh;
            info.joint_record_enable_ch = p_motion->jointRecordEnableCh;
            info.joint_output_enable_ch = p_motion->jointOutputEnableCh;
            info.sensitive_level = p_motion->sensitiveLevel;
            info.alarm_interval_time = p_motion->alarmIntervalTime;
            info.joint_record_time = p_motion->jointRecordTime;
            info.joint_output_time = p_motion->jointOutputTime;
            info.beep_output_time = p_motion->beepOutputTime;
            for (i = 0; i < 12; i++)
                info.motion_area.area[i] = p_motion->motionarea[i];
            for (i = 0; i < NT200H_MAX_TIME_SEG_NUM; i++)
            {
                info.time_seg[i].start_hour = p_motion->timeseg[i].startHour;
                info.time_seg[i].start_minute = p_motion->timeseg[i].startMinute;
                info.time_seg[i].start_second = p_motion->timeseg[i].startSecond;
                info.time_seg[i].end_hour = p_motion->timeseg[i].endHour;
                info.time_seg[i].end_minute = p_motion->timeseg[i].endMinute;
                info.time_seg[i].end_second = p_motion->timeseg[i].endSecond;
            }

            st_param_setVideoMoveStruct(p_motion->videoId, day, info);
            day++;
        }
#endif
        //added 2013.4.3
        cncMotionDetection_t *p_motion = (cncMotionDetection_t*)structBuf;
        net_msg_t msg;
        unsigned int day = p_motion->weekday;
        bzero(&msg, sizeof(net_msg_t));
        int channel_no = p_motion->videoId;
        video_move_t *info = (video_move_t *)(msg.msg_data);

        //st_nvr_getVideoMove(p_motion->videoId, info);
        st_nvr_getVideoMoveEx(channel_no, info);
        //cnc_setGetIpcParam(channel_no, MSG_GET_PARAM, PARAM_VIDEO_MOVE, &msg, 0);

        info->video_day[day].move_guard_flag = p_motion->moveGuardFlag;
        info->video_day[day].move_enable = p_motion->moveEnable;
        info->video_day[day].beep_output_enable = p_motion->beepOutputEnable;
        info->video_day[day].joint_snap_enable_ch = p_motion->jointSnapEnableCh;
        info->video_day[day].joint_record_enable_ch = p_motion->jointRecordEnableCh;
        info->video_day[day].joint_output_enable_ch = p_motion->jointOutputEnableCh;
        info->video_day[day].sensitive_level = p_motion->sensitiveLevel;
        info->video_day[day].alarm_interval_time = p_motion->alarmIntervalTime;
        info->video_day[day].joint_record_time = p_motion->jointRecordTime;
        info->video_day[day].joint_output_time = p_motion->jointOutputTime;
        info->video_day[day].beep_output_time = p_motion->beepOutputTime;
        int i = 0;
        for (i = 0; i < 12; i++)
            info->video_day[day].motion_area.area[i] = p_motion->motionarea[i];
        for (i = 0; i < NT200H_MAX_TIME_SEG_NUM; i++)
        {
            info->video_day[day].time_seg[i].start_hour = p_motion->timeseg[i].startHour;
            info->video_day[day].time_seg[i].start_minute = p_motion->timeseg[i].startMinute;
            info->video_day[day].time_seg[i].start_second = p_motion->timeseg[i].startSecond;
            info->video_day[day].time_seg[i].end_hour = p_motion->timeseg[i].endHour;
            info->video_day[day].time_seg[i].end_minute = p_motion->timeseg[i].endMinute;
            info->video_day[day].time_seg[i].end_second = p_motion->timeseg[i].endSecond;
        }
        printf("moveGuardFlag = %d", info->video_day[day].move_guard_flag);
        printf("moveEnable = %d", info->video_day[day].move_enable);
        printf("beepOutputEnable = %d", info->video_day[day].beep_output_enable);
        printf("jointSnapEnableCh = %d", info->video_day[day].joint_snap_enable_ch);
        printf("jointRecordEnableCh = %d", info->video_day[day].joint_record_enable_ch);
        printf("jointOutputEnableCh = %d", info->video_day[day].joint_output_enable_ch);
        printf("sensitiveLevel = %d", info->video_day[day].sensitive_level);
        printf("alarmIntervalTime = %d", info->video_day[day].alarm_interval_time);
        printf("jointOutputTime = %d", info->video_day[day].joint_output_time);
        printf("jointRecordTime = %d", info->video_day[day].joint_record_time);
        printf("beepOutputTime = %d", info->video_day[day].beep_output_time);
        printf("timeseg = %02d%02d%02d-%02d%02d%02d,%02d%02d%02d-%02d%02d%02d,%02d%02d%02d-%02d%02d%02d,%02d%02d%02d-%02d%02d%02d",
               info->video_day[day].time_seg[0].start_hour, info->video_day[day].time_seg[0].start_minute,
               info->video_day[day].time_seg[0].start_second, info->video_day[day].time_seg[0].end_hour,
               info->video_day[day].time_seg[0].end_minute, info->video_day[day].time_seg[0].end_second,
               info->video_day[day].time_seg[1].start_hour, info->video_day[day].time_seg[1].start_minute,
               info->video_day[day].time_seg[1].start_second, info->video_day[day].time_seg[1].end_hour,
               info->video_day[day].time_seg[1].end_minute, info->video_day[day].time_seg[1].end_second,
               info->video_day[day].time_seg[2].start_hour, info->video_day[day].time_seg[2].start_minute,
               info->video_day[day].time_seg[2].start_second, info->video_day[day].time_seg[2].end_hour,
               info->video_day[day].time_seg[2].end_minute, info->video_day[day].time_seg[2].end_second,
               info->video_day[day].time_seg[3].start_hour, info->video_day[day].time_seg[3].start_minute,
               info->video_day[day].time_seg[3].start_second, info->video_day[day].time_seg[3].end_hour,
               info->video_day[day].time_seg[3].end_minute, info->video_day[day].time_seg[3].end_second);

        st_nvr_setVideoMoveEx(channel_no, info);
        //cnc_setGetIpcParam(channel_no, MSG_SET_PARAM, PARAM_VIDEO_MOVE, info, sizeof(video_move_t));

    }

    else if (strcmp(command, "GetPuVideoLoseAlarmCfg") == 0)
    {
        cncVideoLose_t *p_lose = (cncVideoLose_t*)structBuf;

        video_lose_t    info;
        memset(&info, 0, sizeof(video_lose_t));
        st_param_getVideoLoseStruct(p_lose->videoId, &info);

        p_lose->loseGuardFlag = info.lose_guard_flag;
        p_lose->loseEnable = info.lose_enable;
        p_lose->beepOutputEnable = info.beep_output_enable;
        p_lose->jointSnapEnableCh = info.joint_snap_enable_ch;
        p_lose->jointRecordEnableCh = info.joint_record_enable_ch;
        p_lose->jointOutputEnableCh = info.joint_output_enable_ch;
        p_lose->alarmIntervalTime = info.alarm_interval_time;
        p_lose->jointRecordTime = info.joint_record_time;
        p_lose->jointOutputTime = info.joint_output_time;
        p_lose->beepOutputTime = info.beep_output_time;
    }
    else if (strcmp(command, "SetPuVideoLoseAlarmCfg") == 0)
    {
        cncVideoLose_t *p_lose = (cncVideoLose_t*)structBuf;

        video_lose_t    info;
        memset(&info, 0, sizeof(video_lose_t));
        st_param_getVideoLoseStruct(p_lose->videoId, &info);

        info.lose_guard_flag = p_lose->loseGuardFlag;
        info.lose_enable = p_lose->loseEnable;
        info.beep_output_enable = p_lose->beepOutputEnable;
        info.joint_snap_enable_ch = p_lose->jointSnapEnableCh;
        info.joint_record_enable_ch = p_lose->jointRecordEnableCh;
        info.joint_output_enable_ch = p_lose->jointOutputEnableCh;
        info.alarm_interval_time = p_lose->alarmIntervalTime;
        info.joint_record_time = p_lose->jointRecordTime;
        info.joint_output_time = p_lose->jointOutputTime;
        info.beep_output_time = p_lose->beepOutputTime;

        st_param_setVideoLoseStruct(p_lose->videoId, info);
    }

    else if (strcmp(command, "GetPuProberDetectionCfg") == 0)
    {
        cncProber_t *p_prober = (cncProber_t*)structBuf;

        int day = p_prober->weekday, i;
        prober_alarm_t info;
        memset(&info, 0, sizeof(prober_alarm_t));
        st_param_getProberStruct(p_prober->proberId, &info);

        p_prober->proberType = info.prober_type;
        p_prober->proberSubType = info.sub_prober_type;
        p_prober->proberGuardFlag = info.prober_day[day].prober_guard_flag;
        p_prober->proberEnable = info.prober_day[day].prober_enable;
        p_prober->beepOutputEnable = info.prober_day[day].beep_output_enable;
        p_prober->jointSnapEnableCh = info.prober_day[day].joint_snap_enable_ch;
        p_prober->jointRecordEnableCh = info.prober_day[day].joint_record_enable_ch;
        p_prober->jointOutputEnableCh = info.prober_day[day].joint_output_enable_ch;
        p_prober->alarmIntervalTime = info.prober_day[day].alarm_interval_time;
        p_prober->jointRecordTime = info.prober_day[day].joint_record_time;
        p_prober->jointOutputTime = info.prober_day[day].joint_output_time;
        p_prober->beepOutputTime = info.prober_day[day].beep_output_time;

        for (i = 0; i < NT200H_MAX_TIME_SEG_NUM; i++)
        {
            p_prober->timeseg[i].startHour = info.prober_day[day].time_seg[i].start_hour;
            p_prober->timeseg[i].startMinute = info.prober_day[day].time_seg[i].start_minute;
            p_prober->timeseg[i].startSecond = info.prober_day[day].time_seg[i].start_second;
            p_prober->timeseg[i].endHour = info.prober_day[day].time_seg[i].end_hour;
            p_prober->timeseg[i].endMinute = info.prober_day[day].time_seg[i].end_minute;
            p_prober->timeseg[i].endSecond = info.prober_day[day].time_seg[i].end_second;
        }

    }
    else if (strcmp(command, "SetPuProberDetectionCfg") == 0)
    {
        cncProber_t *p_prober = (cncProber_t*)structBuf;

        int day = p_prober->weekday, i;
        prober_alarm_t info;
        memset(&info, 0, sizeof(prober_alarm_t));
        st_param_getProberStruct(p_prober->proberId, &info);

        info.prober_type = p_prober->proberType;
        info.sub_prober_type = p_prober->proberSubType;
        info.prober_day[day].prober_guard_flag = p_prober->proberGuardFlag;
        info.prober_day[day].prober_enable = p_prober->proberEnable;
        info.prober_day[day].beep_output_enable = p_prober->beepOutputEnable;
        info.prober_day[day].joint_snap_enable_ch = p_prober->jointSnapEnableCh;
        info.prober_day[day].joint_record_enable_ch = p_prober->jointRecordEnableCh;
        info.prober_day[day].joint_output_enable_ch = p_prober->jointOutputEnableCh;
        info.prober_day[day].alarm_interval_time = p_prober->alarmIntervalTime;
        info.prober_day[day].joint_record_time = p_prober->jointRecordTime;
        info.prober_day[day].joint_output_time = p_prober->jointOutputTime;
        info.prober_day[day].beep_output_time = p_prober->beepOutputTime;

        for (i = 0; i < NT200H_MAX_TIME_SEG_NUM; i++)
        {
            info.prober_day[day].time_seg[i].start_hour = p_prober->timeseg[i].startHour;
            info.prober_day[day].time_seg[i].start_minute = p_prober->timeseg[i].startMinute;
            info.prober_day[day].time_seg[i].start_second = p_prober->timeseg[i].startSecond;
            info.prober_day[day].time_seg[i].end_hour = p_prober->timeseg[i].endHour;
            info.prober_day[day].time_seg[i].end_minute = p_prober->timeseg[i].endMinute;
            info.prober_day[day].time_seg[i].end_second = p_prober->timeseg[i].endSecond;
        }
        st_param_setProberStruct(p_prober->proberId, info);
    }

#endif
    else if (strcmp(command, "GetPuVersionInfo") == 0)
    {
#if 0
        cncVersion_t *p_version = (cncVersion_t*)structBuf;
        net_msg_t msg;
        memset(&msg, 0, sizeof(net_msg_t));
        unsigned char channel_no = 0;
        channel_no = p_version->
                     version_info_t* info = (version_info_t*)(msg.msg_data);


        msg.msg_head.msg_type = MSG_GET_PARAM;
        msg.msg_head.msg_subtype = MSG_GET_VERSION_INFO;
        msg.msg_head.chn_no = 0xFF;
        msg.msg_head.msg_size = sizeof(version_info_t);
        cnc_setGetIpcParam(&msg);

        strncpy(p_version->number, info->version_number, NT200H_VERSION_LEN - 1);
        strncpy(p_version->author, "Santachi Corporation", sizeof("Santachi Corporation"));
        strncpy(p_version->date, info->version_date, NT200H_VERSION_LEN - 1);
        printf("%s,%s,%s___________________________\n",
               p_version->number, p_version->author,p_version->date);
#endif
        cncVersion_t *p_version = (cncVersion_t*)structBuf;
       // wsx_VersionInfo(0,0, structBuf, sizeof(cncVersion_t));

    }
#if 0
    else if (strcmp(command, "OperatePuFileList") == 0) //lius debug
    {
#if 0
        OperateBakupFile_t* p_operatefile = (OperateBakupFile_t*)structBuf;
        int ret = 0;

        char *str1, *token;
        char *saveptr1;
        int i = 0;
        int j = 0;
        char file_path[128] = {};
        int fragment_offset = 0;

        for(i = 0; i < p_operatefile->file_count; i++)
        {
            ret = analyzeFileName(p_operatefile->file_name[i], file_path, sizeof(file_path), &fragment_offset, NULL, NULL, NULL, NULL, NULL, NULL);
            if (ret < 0)
            {
                break;
            }

            if (p_operatefile->operate == 2)//备份
            {
                ret = st_record_backupFile(file_path, fragment_offset);
                if (ret < 0)
                {
                    printf("\033[0;33m st_record_backupFile(): %s fail! \033[0m\n", file_path); //yellow
                }
            }
            else if (p_operatefile->operate == 1)//删除
            {
                ret = st_record_removeBackupFile(file_path);
                if (ret < 0)
                {
                    printf("\033[0;31m st_record_removeBackupFile(): %s fail! \033[0m\n", file_path); //yellow
                }
            }
            else
            {
                printf("\033[0;33m invalue operate commond = %d \033[0m\n", p_operatefile->operate); //yellow
                return -1;
            }
        }
#endif
    }
    else if (strcmp(command, "GetFileList") == 0)
    {
        cncRcdFileList_t *p_fileList = (cncRcdFileList_t*)structBuf;
        int ret = 0;
        st_file_t st_file[800];
        int sleep_num = 0;
        int startTime, endTime;
        startTime = device_makeTime(p_fileList->beginTime.year,
                                    p_fileList->beginTime.month, p_fileList->beginTime.date,
                                    p_fileList->beginTime.hour, p_fileList->beginTime.minute,
                                    p_fileList->beginTime.second);
        endTime = device_makeTime(p_fileList->endTime.year,
                                  p_fileList->endTime.month, p_fileList->endTime.date,
                                  p_fileList->endTime.hour, p_fileList->endTime.minute,
                                  p_fileList->endTime.second);

        if (p_fileList->queryType == 1)     //前端抓拍图片
            ret = st_record_query(p_fileList->videoId, ALL_JPEG_FLAG, startTime, endTime, st_file, sizeof(st_file)/sizeof(st_file_t), &sleep_num);
        else    if (p_fileList->queryType == 0)     //前端录像
            ret = st_record_query(p_fileList->videoId, ALL_RECORD_FLAG, startTime, endTime, st_file, sizeof(st_file)/sizeof(st_file_t), &sleep_num);
        else    if (p_fileList->queryType == 2)     //前端备份录像查询
            ret = st_record_queryBakFile(p_fileList->videoId, ALL_RECORD_FLAG, startTime, endTime, st_file, sizeof(st_file)/sizeof(st_file_t));//需要添加
        else    if (p_fileList->queryType == 3)     //前端备份图片查询
            ret = st_record_queryBakFile(p_fileList->videoId, ALL_JPEG_FLAG, startTime, endTime, st_file, sizeof(st_file)/sizeof(st_file_t));//需要添加

        if (ret < 0)
        {
            printf("GetFileList error.\n");
            return -1;
        }
        if (ret == 0)
        {
            printf("record list is empty\n");
            return 0;
        }

        int             fileType, count = 0;
        cncRcdFile_t    rcdFile;
        int i;

        for(i=0; i<ret; i++)
        {
            memset(&rcdFile, 0, sizeof(cncRcdFile_t));
            fileType = st_file[i].cur_fragment_info.record_type;
            if ((p_fileList->queryType == 1)        //前端抓拍图片
                    || (p_fileList->queryType == 3))    //前端备份图片
            {
                if ((p_fileList->fileType & 0x1) && (fileType == HAND_JPEG_FLAG))
                    rcdFile.fileType = 0x1;     //  手动
                if ((p_fileList->fileType & 0x2) && (fileType == TIME_JPEG_FLAG))
                    rcdFile.fileType = 0x2;     //  定时
                if ((p_fileList->fileType & 0x4) &&
                        ((fileType == MOVE_JPEG_FLAG) || (fileType == LOST_JPEG_FLAG) || (fileType == PROBER_JPEG_FLAG)))
                    rcdFile.fileType = 0x4;     //  报警
                if (rcdFile.fileType == 0)
                    continue;

                rcdFile.fileSize = st_file[i].cur_fragment_info.data_end_offset
                                   - st_file[i].cur_fragment_info.data_start_offset;
                sprintf(rcdFile.fileName,
                        "%s:%03d:%08x-%08x:%08x-%08x:%08x-%08x:%08x/%02d-%02d-%02d.jpeg",
                        st_file[i].file_name,
                        st_file[i].fragment_offset,
                        st_file[i].cur_fragment_info.data_start_offset,
                        st_file[i].cur_fragment_info.data_end_offset,
                        st_file[i].cur_fragment_info.index_start_offset,
                        st_file[i].cur_fragment_info.index_end_offset,
                        (unsigned int)st_file[i].cur_fragment_info.start_time,
                        (unsigned int)st_file[i].cur_fragment_info.end_time,
                        rcdFile.fileSize,
                        device_getHour(st_file[i].cur_fragment_info.start_time),
                        device_getMinute(st_file[i].cur_fragment_info.start_time),
                        device_getSecond(st_file[i].cur_fragment_info.start_time)
                       );
            }
            else//录像
            {
                if ((p_fileList->fileType & 0x1) && (fileType & HAND_RECORD_FLAG))
                    rcdFile.fileType = 0x1;     //  手动
                if ((p_fileList->fileType & 0x2) && (fileType & TIME_RECORD_FLAG))
                    rcdFile.fileType = 0x2;     //  定时
                if ((p_fileList->fileType & 0x4) && (fileType & ALL_ALARM_RECORD_FLAG))
                    rcdFile.fileType = 0x4;     //  报警
                if (rcdFile.fileType == 0)
                    continue;

                rcdFile.fileSize = st_file[i].cur_fragment_info.data_end_offset
                                   - st_file[i].cur_fragment_info.data_start_offset;
                sprintf(rcdFile.fileName,
                        "%s:%03d:%08x-%08x:%08x-%08x:%08x-%08x:%08x/%02d-%02d-%02d.dat",
                        st_file[i].file_name,
                        st_file[i].fragment_offset,
                        st_file[i].cur_fragment_info.data_start_offset,
                        st_file[i].cur_fragment_info.data_end_offset,
                        st_file[i].cur_fragment_info.index_start_offset,
                        st_file[i].cur_fragment_info.index_end_offset,
                        (unsigned int)st_file[i].cur_fragment_info.start_time,
                        (unsigned int)st_file[i].cur_fragment_info.end_time,
                        rcdFile.fileSize,
                        device_getHour(st_file[i].cur_fragment_info.start_time),
                        device_getMinute(st_file[i].cur_fragment_info.start_time),
                        device_getSecond(st_file[i].cur_fragment_info.start_time)
                       );
            }
            p_fileList->rcdFile[count++]= rcdFile;

            if (count >= CNC_RCD_LIST_LEN)
            {
                printf("The file num is too large.\n");
                break;
            }
        }
        p_fileList->nCount = count;
    }

    else if (strcmp(command, "ControlPTZ") == 0)
    {
#if 1
        cncControlPTZ_t *p_CtrlPTZ = (cncControlPTZ_t*)structBuf;
        net_msg_t msg;
        msg_ptz_control_t ptz_ctl;

        memset(&msg, 0, sizeof(net_msg_t));
        memset(&ptz_ctl, 0, sizeof(ptz_ctl));

        int channel_no = p_CtrlPTZ->control_limits.videoid;
        msg.msg_head.msg_size = sizeof(ptz_ctl);
        msg.msg_head.msg_type = MSG_PTZ_CONTROL;

        if (strncmp(p_CtrlPTZ->cmd, "TU", sizeof("TU")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_UPSTART;
            ptz_ctl.ptz_value = p_CtrlPTZ->speed * 12.5;
        }
        else if (strncmp(p_CtrlPTZ->cmd, "TD", sizeof("TD")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_DOWNSTART;
            ptz_ctl.ptz_value = p_CtrlPTZ->speed * 12.5;
        }
        else if (strncmp(p_CtrlPTZ->cmd, "PL", sizeof("PL")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_LEFTSTART;
            ptz_ctl.ptz_value = p_CtrlPTZ->speed * 12.5;
        }
        else if (strncmp(p_CtrlPTZ->cmd, "PR", sizeof("PR")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_RIGHTSTART;
            ptz_ctl.ptz_value = p_CtrlPTZ->speed * 12.5;
        }

        else if (strncmp(p_CtrlPTZ->cmd, "TUPL", sizeof("TUPL")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_LEFTUPSTART;
            ptz_ctl.ptz_value = p_CtrlPTZ->speed * 12.5;
        }
        else if (strncmp(p_CtrlPTZ->cmd, "TUPR", sizeof("TUPR")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_RIGHTUPSTART;
            ptz_ctl.ptz_value = p_CtrlPTZ->speed * 12.5;
        }
        else if (strncmp(p_CtrlPTZ->cmd, "TDPL", sizeof("TDPL")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_LEFTDOWNSTART;
            ptz_ctl.ptz_value = p_CtrlPTZ->speed * 12.5;
        }
        else if (strncmp(p_CtrlPTZ->cmd, "TDPR", sizeof("TDPR")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_RIGHTDOWNSTART;
            ptz_ctl.ptz_value = p_CtrlPTZ->speed * 12.5;
        }

        else if (strncmp(p_CtrlPTZ->cmd, "ZOUT", sizeof("ZOUT")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_ZOOMSUBSTART;
            ptz_ctl.ptz_value = 55;
        }
        else if (strncmp(p_CtrlPTZ->cmd, "ZIN", sizeof("ZIN")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_ZOOMADDSTART;
            ptz_ctl.ptz_value = 55;
        }
        else if (strncmp(p_CtrlPTZ->cmd, "FR", sizeof("FR")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_FOCUSADDSTART;
            ptz_ctl.ptz_value = 55;
        }
        else if (strncmp(p_CtrlPTZ->cmd, "FN", sizeof("FN")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_FOCUSSUBSTART;
            ptz_ctl.ptz_value = 55;
        }
        else if (strncmp(p_CtrlPTZ->cmd, "IO", sizeof("IO")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_IRISADDSTART;
            ptz_ctl.ptz_value = 55;
        }
        else if (strncmp(p_CtrlPTZ->cmd, "IC", sizeof("IC")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_IRISSUBSTART;
            ptz_ctl.ptz_value = 55;
        }

        else if (strncmp(p_CtrlPTZ->cmd, "GOTO_PRESET", sizeof("GOTO_PRESET")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_PREVPOINTCALL;
            ptz_ctl.ptz_value = atoi(p_CtrlPTZ->param);
        }

        else if (strncmp(p_CtrlPTZ->cmd, "CRUISE_START", sizeof("CRUISE_START")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_UPSTART;
            ptz_ctl.ptz_value = p_CtrlPTZ->speed * 12.5;
            memcpy(msg.msg_data, &ptz_ctl, sizeof(ptz_ctl));
            printf("=============CRUISE_START.\n");
            st_nvr_ptzControl(channel_no, PTZ_UPSTART, &ptz_ctl);
            //cnc_setGetIpcParam(channel_no, MSG_PTZ_CONTROL, PTZ_UPSTART, &ptz_ctl, sizeof(ptz_ctl));

            msg.msg_head.msg_subtype = PTZ_UPSTOP;
            ptz_ctl.ptz_value = p_CtrlPTZ->speed * 12.5;
            memcpy(msg.msg_data, &ptz_ctl, sizeof(ptz_ctl));
            st_nvr_ptzControl(channel_no, PTZ_UPSTOP, &ptz_ctl);
            //cnc_setGetIpcParam(channel_no, MSG_PTZ_CONTROL, PTZ_UPSTOP, &ptz_ctl, sizeof(ptz_ctl));

            msg.msg_head.msg_subtype = PTZ_PREVPOINTCALL;
            ptz_ctl.ptz_value = atoi(p_CtrlPTZ->param) + 61;
        }
        else if (strncmp(p_CtrlPTZ->cmd, "CRUISE_STOP", sizeof("CRUISE_STOP")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_UPSTART;
            ptz_ctl.ptz_value = p_CtrlPTZ->speed * 12.5;
            memcpy(msg.msg_data, &ptz_ctl, sizeof(ptz_ctl));
            printf("=============CRUISE_STOP.\n");
            st_nvr_ptzControl(channel_no,  PTZ_UPSTART, &ptz_ctl);
            //cnc_setGetIpcParam(channel_no, MSG_PTZ_CONTROL, PTZ_UPSTART, &ptz_ctl, sizeof(ptz_ctl));

            msg.msg_head.msg_subtype = PTZ_UPSTOP;
            ptz_ctl.ptz_value = p_CtrlPTZ->speed * 12.5;
        }

        else if (strncmp(p_CtrlPTZ->cmd, "AUTO_START", sizeof("AUTO_START")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_AUTOSCANSTART;
        }
        else if (strncmp(p_CtrlPTZ->cmd, "AUTO_STOP", sizeof("AUTO_STOP")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_AUTOSCANSTOP;
        }
        else if (strncmp(p_CtrlPTZ->cmd, "RANDOM_START", sizeof("RANDOM_START")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_RANDOMSCANSTART;
        }
        else if (strncmp(p_CtrlPTZ->cmd, "RANDOM_STOP", sizeof("RANDOM_STOP")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_RANDOMSCANSTOP;
        }
        else if (strncmp(p_CtrlPTZ->cmd, "AUXILIART_OPEN", sizeof("AUXILIART_OPEN")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_SETAUXILIARY;
            ptz_ctl.ptz_value = atoi(p_CtrlPTZ->param) + 1;
        }
        else if (strncmp(p_CtrlPTZ->cmd, "AUXILIART_CLOSE", sizeof("AUXILIART_CLOSE")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_CLEARAUXILIARY;
            ptz_ctl.ptz_value = atoi(p_CtrlPTZ->param) + 1;
        }
        else if (strncmp(p_CtrlPTZ->cmd, "STOP", sizeof("STOP")) == 0)
        {
            if (strncmp(p_CtrlPTZ->param, "TU", sizeof("TU")) == 0)
                msg.msg_head.msg_subtype = PTZ_UPSTOP;
            else if (strncmp(p_CtrlPTZ->param, "TD", sizeof("TD")) == 0)
                msg.msg_head.msg_subtype = PTZ_DOWNSTOP;
            else if (strncmp(p_CtrlPTZ->param, "PL", sizeof("PL")) == 0)
                msg.msg_head.msg_subtype = PTZ_LEFTSTOP;
            else if (strncmp(p_CtrlPTZ->param, "PR", sizeof("PR")) == 0)
                msg.msg_head.msg_subtype = PTZ_RIGHTSTOP;

            else if (strncmp(p_CtrlPTZ->param, "TUPL", sizeof("TUPL")) == 0)
                msg.msg_head.msg_subtype = PTZ_LEFTUPSTOP;
            else if (strncmp(p_CtrlPTZ->param, "TUPR", sizeof("TUPR")) == 0)
                msg.msg_head.msg_subtype = PTZ_RIGHTUPSTOP;
            else if (strncmp(p_CtrlPTZ->param, "TDPL", sizeof("TDPL")) == 0)
                msg.msg_head.msg_subtype = PTZ_LEFTDOWNSTOP;
            else if (strncmp(p_CtrlPTZ->param, "TDPR", sizeof("TDPR")) == 0)
                msg.msg_head.msg_subtype = PTZ_RITHTDOWNSTOP;

            else if (strncmp(p_CtrlPTZ->param, "ZOUT", sizeof("ZOUT")) == 0)
                msg.msg_head.msg_subtype = PTZ_ZOOMSUBSTOP;
            else if (strncmp(p_CtrlPTZ->param, "ZIN", sizeof("ZIN")) == 0)
                msg.msg_head.msg_subtype = PTZ_ZOOMADDSTOP;
            else if (strncmp(p_CtrlPTZ->param, "FR", sizeof("FR")) == 0)
                msg.msg_head.msg_subtype = PTZ_FOCUSADDSTOP;
            else if (strncmp(p_CtrlPTZ->param, "FN", sizeof("FN")) == 0)
                msg.msg_head.msg_subtype = PTZ_FOCUSUBSTOP;
            else if (strncmp(p_CtrlPTZ->param, "IC", sizeof("IC")) == 0)
                msg.msg_head.msg_subtype = PTZ_IRISSUBSTOP;
            else if (strncmp(p_CtrlPTZ->param, "IO", sizeof("IO")) == 0)
                msg.msg_head.msg_subtype = PTZ_IRISADDSTOP;
        }
        else if (strncmp(p_CtrlPTZ->cmd, "CENPOS", sizeof("CENPOS")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_POINTCENTER;
            ptz_ctl.ptz_value = p_CtrlPTZ->speed;
        }
        else if (strncmp(p_CtrlPTZ->cmd, "SZIN", sizeof("SZIN")) == 0)
        {
            msg.msg_head.msg_subtype = PTZ_VIEWCENTER;
            ptz_ctl.ptz_value = p_CtrlPTZ->speed;
        }

        memcpy(msg.msg_data, &ptz_ctl, sizeof(ptz_ctl));
        st_nvr_ptzControl(channel_no,  msg.msg_head.msg_subtype, &ptz_ctl);
        //cnc_setGetIpcParam(channel_no, MSG_PTZ_CONTROL, msg.msg_head.msg_subtype, &ptz_ctl, sizeof(ptz_ctl));
        //cnc_setGetIpcParam(&msg);
#endif
    }

    else if (strcmp(command, "SetPresetPTZ") == 0)
    {
        cncPresetPTZ_t *p_preset = (cncPresetPTZ_t *)structBuf;
        net_msg_t msg;
        msg_ptz_control_t ptz_ctl;
        int channel_no = p_preset->videoId;
        memset(&msg, 0, sizeof(net_msg_t));
        memset(&ptz_ctl, 0, sizeof(ptz_ctl));
        msg.msg_head.msg_size = sizeof(ptz_ctl);
        msg.msg_head.msg_type = MSG_PTZ_CONTROL;

        msg.msg_head.msg_subtype = PTZ_PREVPOINTSET;
        ptz_ctl.ptz_value = p_preset->presetIndex;

        memcpy(msg.msg_data, &ptz_ctl, sizeof(ptz_ctl));
        st_nvr_ptzControl(channel_no,  PTZ_PREVPOINTSET, &ptz_ctl);
        //cnc_setGetIpcParam(channel_no, MSG_PTZ_CONTROL, PTZ_PREVPOINTSET, &ptz_ctl, sizeof(ptz_ctl));
    }
    else if (strcmp(command, "DelPresetPTZ") == 0)
    {
        cncPresetPTZ_t *p_preset = (cncPresetPTZ_t *)structBuf;
        net_msg_t msg;
        msg_ptz_control_t ptz_ctl;
        int channel_no = 0;
        channel_no = p_preset->videoId;
        memset(&msg, 0, sizeof(net_msg_t));
        memset(&ptz_ctl, 0, sizeof(ptz_ctl));
        msg.msg_head.msg_size = sizeof(ptz_ctl);
        msg.msg_head.msg_type = MSG_PTZ_CONTROL;

        msg.msg_head.msg_subtype = PTZ_PREVPOINTDEL;
        ptz_ctl.ptz_value = p_preset->presetIndex;

        memcpy(msg.msg_data, &ptz_ctl, sizeof(ptz_ctl));
        st_nvr_ptzControl(channel_no,  PTZ_PREVPOINTDEL, &ptz_ctl);
        //cnc_setGetIpcParam(channel_no, MSG_PTZ_CONTROL, PTZ_PREVPOINTDEL, &ptz_ctl, sizeof(ptz_ctl));
    }

#if 0
    else if (strcmp(command, "MatrixStepStart") == 0)
    {
        cncMatrix_t *p_matrix = (cncMatrix_t*)structBuf;
        ptz_cmd_param_t info;
        memset(&info, 0, sizeof(ptz_cmd_param_t));

        info.cmd = MATRIX_LOGIN_WITH_PASSWORD;
        strncpy(info.user_name, "40", sizeof("40"));
        strncpy(info.user_pwd, "12345", sizeof("12345"));
        st_ptz_sendCommand(p_matrix->videoId, &info);

        usleep(500*1000);
        info.cmd = MATRIX_MONITOR_SELECT;
        info.value = p_matrix->monitorNo;
        st_ptz_sendCommand(p_matrix->videoId, &info);

        info.cmd = MATRIX_CAMERA_SELECT;
        info.value = p_matrix->cameraNo;
        st_ptz_sendCommand(p_matrix->videoId, &info);
    }
    else if (strcmp(command, "MatrixTourStart") == 0)
    {
        cncMatrix_t *p_matrix = (cncMatrix_t*)structBuf;
        ptz_cmd_param_t info;
        memset(&info, 0, sizeof(ptz_cmd_param_t));

        info.cmd = MATRIX_LOGIN_WITH_PASSWORD;
        strncpy(info.user_name, "40", sizeof("40"));
        strncpy(info.user_pwd, "12345", sizeof("12345"));
        st_ptz_sendCommand(p_matrix->videoId, &info);

        usleep(500*1000);
        info.cmd = MATRIX_TOUR_SELECT;
        info.value = p_matrix->tourNo;
        st_ptz_sendCommand(p_matrix->videoId, &info);
    }
    else if (strcmp(command, "MatrixGroupStart") == 0)
    {
        cncMatrix_t *p_matrix = (cncMatrix_t*)structBuf;
        ptz_cmd_param_t info;
        memset(&info, 0, sizeof(ptz_cmd_param_t));

        info.cmd = MATRIX_LOGIN_WITH_PASSWORD;
        strncpy(info.user_name, "40", sizeof("40"));
        strncpy(info.user_pwd, "12345", sizeof("12345"));
        st_ptz_sendCommand(p_matrix->videoId, &info);

        usleep(500*1000);
        info.cmd = MATRIX_GROUP_SELECT;
        info.value = p_matrix->groupNo;
        st_ptz_sendCommand(p_matrix->videoId, &info);
    }

    else if (strcmp(command, "IoControl") == 0)
    {
        cncIoCtrl_t *p_io = (cncIoCtrl_t*)structBuf;
        st_app_setIoControlFlag(p_io->videoId, p_io->status, p_io->time);
    }
    else if (strcmp(command, "GetAlarmOutState") == 0)
    {
        cncIoCtrl_t *p_io = (cncIoCtrl_t*)structBuf;
        st_app_getIoControlFlag(p_io->videoId, &p_io->status, &p_io->time);
    }
#endif
    else if (strcmp(command, "RemoteAlarmEnable") == 0)
    {
        cncIoCtrl_t     *p_arming = (cncIoCtrl_t*)structBuf;
        int             day;
        video_day_t     moveDay[NT200H_MAX_WEEK_DAY];
        prober_alarm_t  prober;
        video_lose_t    lose;
        memset(&prober, 0, sizeof(prober_alarm_t));

        st_param_getProberStruct(p_arming->videoId, &prober);
        st_param_getVideoLoseStruct(p_arming->videoId, &lose);

        for(day = 0; day < NT200H_MAX_WEEK_DAY; day++)
        {
            prober.prober_day[day].prober_guard_flag = p_arming->status;

            memset(&moveDay[day], 0, sizeof(video_day_t));
            st_param_getVideoMoveStruct(p_arming->videoId, day, &moveDay[day]);
            moveDay[day].move_guard_flag = p_arming->status;
            st_param_setVideoMoveStruct(p_arming->videoId, day, moveDay[day]);
        }
        lose.lose_guard_flag = p_arming->status;

        st_param_setProberStruct(p_arming->videoId, prober);
        st_param_setVideoLoseStruct(p_arming->videoId, lose);
    }

    else if (strcmp(command, "GetPPPOEInfo") == 0)
    {
        cncPPPOE_t      *p_pppoe = (cncPPPOE_t*)structBuf;
        pppoe_info_t    info;
        memset(&info, 0, sizeof(pppoe_info_t));
        st_param_getPppoeInfoStruct(&info);

        strncpy(p_pppoe->ppoeAccount, info.account, CNC_ID_LEN - 1);
        strncpy(p_pppoe->pppoePwd, info.password, CNC_ID_LEN - 1);
        strncpy(p_pppoe->publicNetIp, info.ip, CNC_IP_LEN - 1);
        p_pppoe->enable = info.enable;
        p_pppoe->pppInterface = info.ppp_interface;
        p_pppoe->status = info.status;
        p_pppoe->dnsEnable = info.dns_enable;
        strncpy(p_pppoe->username, info.dns_user, CNC_ID_LEN - 1);
        strncpy(p_pppoe->dnsPwd, info.dns_password, CNC_ID_LEN - 1);
        strncpy(p_pppoe->domainName, info.mydomain, 127 - 1);
        p_pppoe->dnsServerType = info.dns_server;
    }
    else if (strcmp(command, "SetPPPOEInfo") == 0)
    {
        cncPPPOE_t      *p_pppoe = (cncPPPOE_t*)structBuf;
        pppoe_info_t    info;
        memset(&info, 0, sizeof(pppoe_info_t));

        st_param_getPppoeInfoStruct(&info);

        strncpy(info.account, p_pppoe->ppoeAccount, CNC_ID_LEN - 1);
        strncpy(info.password, p_pppoe->pppoePwd, CNC_ID_LEN - 1);
        strncpy(info.ip, p_pppoe->publicNetIp, CNC_IP_LEN - 1);
        info.enable = p_pppoe->enable;
        info.ppp_interface = p_pppoe->pppInterface;
        info.status = p_pppoe->status;
        info.dns_enable = p_pppoe->dnsEnable;
        strncpy(info.dns_user, p_pppoe->username, CNC_ID_LEN - 1);
        strncpy(info.dns_password, p_pppoe->dnsPwd, CNC_ID_LEN - 1);
        strncpy(info.mydomain, p_pppoe->domainName, 127 - 1);
        info.dns_server = p_pppoe->dnsServerType;

        st_param_setPppoeInfoStruct(info);
    }

    else if (strcmp(command, "RemoteStartRecord") == 0)
    {
        cncRemoteAction_t *p_remoteAction = (cncRemoteAction_t*)structBuf;
        hand_record_t  info;
        record_param_t recordParam;
        memset(&info, 0, sizeof(hand_record_t));
        memset(&recordParam, 0, sizeof(record_param_t));

        st_param_getRecParamStruct(&recordParam);

        if(recordParam.manual_record_time == 0)
            recordParam.manual_record_time = 1;
        info.record_hour_len = recordParam.manual_record_time / 60;
        info.record_minute_len = recordParam.manual_record_time - info.record_hour_len * 60;
        info.record_status = 1;
        st_param_setHandRecordStruct(p_remoteAction->channel, info);
    }
    else if (strcmp(command, "RemoteStopRecord") == 0)
    {
        cncRemoteAction_t *p_remoteAction = (cncRemoteAction_t*)structBuf;

        hand_record_t  info;
        memset(&info, 0, sizeof(hand_record_t));

        st_param_getHandRecordStruct(p_remoteAction->channel, &info);
        info.record_status = 0;
        st_param_setHandRecordStruct(p_remoteAction->channel, info);
    }
    else if (strcmp(command, "RemoteCapturePic") == 0)
    {
#if 0
        cncRemoteAction_t *p_remoteAction = (cncRemoteAction_t*)structBuf;
        unsigned long ipc_usr_id;
        ST_U8 *pBuf = NULL;
        int datalen;
        struct timeval tv;

        gettimeofday(&tv, NULL);
        ipc_usr_id = app_getUsrId(p_remoteAction->channel);
        datalen = st_netcli_snapPic(ipc_usr_id, 0, (char **)&pBuf);
        if (datalen < 0)
        {
            printf("st_netcli_snapPic() err!\n");
        }
        else
        {
            st_record_writePictureToHdisk(p_remoteAction->channel, (char *)pBuf, datalen, tv.tv_sec, HAND_JPEG_FLAG);
        }
        free(pBuf);
        pBuf = NULL;
#else
        cncRemoteAction_t *p_remoteAction = (cncRemoteAction_t*)structBuf;
        ST_U8 *pBuf = NULL;
        int datalen;
        struct timeval tv;

        gettimeofday(&tv, NULL);
        datalen = st_nvr_snapPic(p_remoteAction->channel, (char **)&pBuf);
        if (datalen < 0)
        {
            printf("%s.%d: st_nvr_snapPic() ch %d err!\n", __func__, __LINE__, p_remoteAction->channel);
        }
        else
        {
            st_record_writePictureToHdisk(p_remoteAction->channel, (char *)pBuf, datalen, tv.tv_sec, HAND_JPEG_FLAG);
        }
        if (pBuf)
        {
            free(pBuf);
            pBuf = NULL;
        }
#endif
        //if (st_app_startSnapJpeg(1 << p_remoteAction->channel, HANDLE_SNAPJPG) < 0)
        //    return -1;
    }

    else if (strcmp(command, "StartUpdateDevice") == 0)
    {
        cncOtherCmd_t *p_update = (cncOtherCmd_t*)structBuf;
        //st_app_netUpdateSystem(p_update->p_buf, p_update->bufSize);

        cncAlarm_t alarm;
        memset(&alarm, 0, sizeof(cncAlarm_t));
        alarm.alarmType = 7;
        alarm.subType = 1;
        st_cnc_sendAlarmFrame(CNC_UP_ALARM, (void*)&alarm);

        free(p_update->p_buf);
        st_param_setSysRebootFlag();
    }

    else if (strcmp(command, "RestartPuDevice") == 0)
    {
        st_log_write(LOG_TYPE_SYSTEM,"WSX restart system ");
        st_param_setSysRebootFlag();
    }

    else if (strcmp(command, "SetPuDeviceDefault") == 0)
    {
        st_param_resetAllStruct();
    }

    else if (strcmp(command, "RecovePuDeviceFactory") == 0)
    {
        st_param_resetFactoryParam();
    }

    else if (strcmp(command, "RestorePuConfig") == 0)
    {
        st_param_resetAllStruct();
    }
#endif
    else if (strcmp(command, "SetOnlineState") == 0)
    {
#if 0
        int             flag = *(int*)structBuf;
        netcom_config_t netcom;
        static int      state = 0;

        st_param_getNetcomConfig(&netcom);

        if (flag >= 2)
        {
            st_param_setWsxStatus(1);

            if (state == STATE_FALLLINE && netcom.offUpload > 0)
            {
                pthread_t offUploadPid;
                if (pthread_create(&offUploadPid, NULL, (void*)thread_offUpload, NULL) < 0)
                    printf("create thread_uploadRcdFile error.\n");
            }
            state = STATE_ONLINE;
        }
        else if (flag == 0)
        {
            st_param_setWsxStatus(0);
            if (state == STATE_ONLINE && netcom.offRecord > 0)
            {
                state = STATE_FALLLINE;
                pthread_t offRecordPid;
                if (pthread_create(&offRecordPid, NULL, (void*)thread_offRecord, NULL) < 0)
                    printf("create thread_uploadRcdFile error.\n");
            }
            else
                state = STATE_OFFLINE;
        }
        //  printf("-------state=%d\n",state);
#endif
        int     flag = *(int*)structBuf;
        static int state = STATE_OFFLINE;
        int                onOffLineState;
        int                onOffLineUpload;
        onOffLineState = flag&0x0f;
        onOffLineUpload = flag&0xf0;
        if (onOffLineState >= 2)
            st_param_setWsxStatus(1);
        else if (onOffLineState == 0)
            st_param_setWsxStatus(0);


    }


#if 0
    else if (strcmp(command, "QueryDeviceLog") == 0)
    {
        cncLogList_t *p_logList = (cncLogList_t*)structBuf;
        static log_list_t *p_list;
        int startTime, endTime, i = 0;
        log_list_t* p_index = NULL;

        if (p_logList->nPos == 0)
        {
            startTime = device_makeTime(p_logList->startTime.year,
                                        p_logList->startTime.month, p_logList->startTime.date,
                                        p_logList->startTime.hour, p_logList->startTime.minute,
                                        p_logList->startTime.second);
            endTime = device_makeTime(p_logList->endTime.year,
                                      p_logList->endTime.month, p_logList->endTime.date,
                                      p_logList->endTime.hour, p_logList->endTime.minute,
                                      p_logList->endTime.second);

            st_log_query(p_logList->logType, startTime, endTime,&p_list);
        }

        p_index = p_list;
        while (p_index && i < p_logList->count)
        {
            struct tm log_time;
            localtime_r(&p_index->node.time, &log_time);
            strncpy(p_logList->log[i].text, p_index->node.desc, CNC_LOGTEXT_LEN);
            p_logList->log[i].type = p_index->node.type;
            //memcpy(&p_logList->log[i].time, &p_index->node.time, sizeof(cncTime_t));
            p_logList->log[i].time.year = log_time.tm_year +1900;
            p_logList->log[i].time.month = log_time.tm_mon + 1;
            p_logList->log[i].time.date = log_time.tm_mday;
            p_logList->log[i].time.hour = log_time.tm_hour;
            p_logList->log[i].time.minute = log_time.tm_min;
            p_logList->log[i].time.second = log_time.tm_sec;

            i++;
            if (i >= CNC_LOG_LIST_NUM)
                break;
            p_list = p_index->next;
            free(p_index);
            p_index = p_list;
            printf("%d ", i);
        }
        p_logList->count = i;
        p_logList->nPos += i;
    }
    else if (strcmp(command, "GetPuHardDiskInfo") == 0)
    {
        cncHardDisk_t* p_hardDisk = (cncHardDisk_t*)structBuf;
        int     i, j, diskNum, partNum;
        long long   size;
        int ret = 0;
        sys_config_t sys_cfg;
        diskNum = st_hd_getOnlineDiskNum();
        if (diskNum <= 0)
            return 0;
        p_hardDisk->isExist = 1;

        st_param_getSysConfig(&sys_cfg);
        if (sys_cfg.hd_num==0)
            diskNum= 2;
        else if(sys_cfg.hd_num==1)
            diskNum= 8;
        else if(sys_cfg.hd_num==2)
            diskNum= 16;
        else
            diskNum= 2; //zpc: 2 disks by default

        for (i = 0; i < diskNum; i++)
        {
            long long a_disk_total_size= 0;
            long long a_disk_free_size= 0;
//            st_hd_getDiskTotalSize(i, &size);
//            p_hardDisk->totalSize += size;
            if (st_hd_isDiskExist(i)==0)
                continue;

            if (st_hd_isFdisk(i))
                continue;
            partNum = st_hd_getDiskPartNum(i);
            for (j = 0; j < partNum; j++)
            {
//                    if (st_record_getPartIsBad(i, j) == 1)
//                        continue;
                long long a_part_total_size= 0;
                long long a_part_free_size= 0;

                if (st_hd_isFormat(i, j)<0)
                    goto next_disk;

                if (st_hd_isMount(i, j)<=0)
                    goto next_disk;

                if (st_record_partTotalSize(i, j, &a_part_total_size)>=0)
                    a_disk_total_size+= a_part_total_size;
                else
                    goto next_disk;

                if (st_record_partFreeSize(i, j, &a_part_free_size)>=0)
                    a_disk_free_size+= a_part_free_size;
                else
                    goto next_disk;
            }
            p_hardDisk->totalSize += a_disk_total_size;
            p_hardDisk->leftSize += a_disk_free_size;

next_disk:
            ;

        }

        p_hardDisk->totalSize = (p_hardDisk->totalSize >> 10);
        p_hardDisk->leftSize = (p_hardDisk->leftSize >> 10);
        p_hardDisk->usedSize = p_hardDisk->totalSize - p_hardDisk->leftSize;
    }
    else if (strcmp(command, "GetPuRecordState") == 0)
    {
        cncRecordState_t* p_recordState = (cncRecordState_t*)structBuf;
        int part_num;
        //int i;

        part_num = st_record_getPartitionNum();

        p_recordState->recordType = -1;

//        for (i=0; i<part_num; i++)
        /*{
            p_recordState->recordType = 0;//定时录像,手动录像等
            p_recordState->state = st_record_getPartUsed(0, 0);
            printf("recordType = %d\n", p_recordState->recordType);
        }*/
        printf(" GetPuRecordState:====channel_no:%d.\n", p_recordState->videoId);
        {
            p_recordState->recordType = st_record_getRecordState(p_recordState->videoId);
            if(p_recordState->recordType == -1)	//查询录像状态失败
            {
                p_recordState->state = 0;
            }
            else
            {
                p_recordState->state = (p_recordState->recordType & ALL_RECORD_FLAG)>0;
            }

            printf("recordType = %d\n", p_recordState->recordType);
        }
    }
#if 0
    else if (strcmp(command, "GetVideoSwitchInfo") == 0)
    {
        cncSpecialCh_t*     p_specialCh = (cncSpecialCh_t*)structBuf;
        special_channel_t   info;
        st_param_getSpecialChannelStruct(&info);
        p_specialCh->channel = info.change_channel;
        p_specialCh->intervalTime = info.interval_time;
    }

    else if (strcmp(command, "SetVideoSwitchInfo") == 0)
    {
        cncSpecialCh_t* p_specialCh = (cncSpecialCh_t*)structBuf;
        special_channel_t info;

        st_param_getSpecialChannelStruct(&info);
        info.change_channel = p_specialCh->channel;
        info.interval_time = p_specialCh->intervalTime;
        st_param_setSpecialChannelStruct(info);
    }
#endif
    else if (strcmp(command, "SetPuMutexVideo") == 0)
    {
        cncSpecialCh_t* p_specialCh = (cncSpecialCh_t*)structBuf;
        g_mutexChannel = p_specialCh->channel;
    }
    else if (strcmp(command, "NotifyPULogOut") == 0)
    {
        pthread_t   stopPID;
        if (pthread_create(&stopPID, NULL, (void*)st_cnc_stopWsxServer, NULL) < 0)
        {
            printf("st_cnc_stopWsxServer error ");
            return -1;
        }
        pthread_detach(stopPID);
        st_log_write(LOG_TYPE_ALARM, "NotifyPULogOut!");
        g_uploadMsgFlag = 0;
    }
#endif
    else if (strcmp(command, "GetDeviceType") == 0)
    {
        int* pDeviceType = (int*)structBuf;
        *pDeviceType = 0;
    }

    else if (strcmp(command, "GetLoginMode") == 0)
    {
        return 1;
#if 0
        network_config_t    network;
        pppoe_info_t        pppoe;
        //   usb_type_t          usb;
        char                ip_addr[16] = {0};

        st_param_getNetworkStruct(&network);
        st_param_getPppoeInfoStruct(&pppoe);
        //st_param_getUsbTypeStruct(&usb);
        st_param_netConvertToAddr(network.ip_addr, ip_addr);

        if (strcmp(structBuf, ip_addr) == 0)
            return 1;           // ethernet
        else if (strcmp(structBuf, pppoe.ip) == 0)
        {
            if (pppoe.ppp_interface == 0)
                return 2;       // adsl
            else if (pppoe.ppp_interface == 1)
                return 3;       // wifi

#if 0
            else if (pppoe.ppp_interface == 2)
            {
                if (usb.type == 0 || usb.type == 1)
                    return 4;   // 3g evdo
                else if (usb.type == 61)
                    return 5;   // 3g wcdma
                else if (usb.type == 31 || usb.type == 32)
                    return 6;   // 3g td
            }
#endif
        }
#endif
    }

    /****************/
#if 1

#if 0
    else if (strcmp(command, "CuTransparentTrans") == 0)
    {
#if 0
        cncSerialData_t* p_serialData = (cncSerialData_t*)structBuf;

        //      printf("serialNo=%d, bufLen=%d, buf=(", p_serialData->serialNo, p_serialData->bufLen);
        //      int i = 0;
        //      while (i < p_serialData->bufLen)
        //      {
        //          printf("%x ", p_serialData->buf[i]);
        //          i++;
        //      }
        //      printf(")\n");

        transparence_info_t transparence;
        st_param_getTransparenceStruct(&transparence);
        if (transparence.down_flag > 0)
            st_com_sendDataToCom(1, p_serialData->buf, p_serialData->bufLen);
#endif
    }
#endif
    else if (strcmp(command, "GetFlatTimeCheckFlag") == 0)
    {
        int* pFlag = (int*)structBuf;

        // time_check_t info;
        // st_param_getTimeCheckConfig(&info);
        // *pFlag = info.flat.enable;
        *pFlag  = 1;
    }
#if 0
    else if (strcmp(command, "GetPuTransparenceCfg") == 0)
    {
        cncTransparence_t *p_Transparence = (cncTransparence_t*)structBuf;
        transparence_info_t info;
        memset(&info, 0, sizeof(transparence_info_t));
        st_param_getTransparenceStruct(&info);

        p_Transparence->serialNo = info.serial_no;
        p_Transparence->upFlag = info.up_flag;
        p_Transparence->downFlag = info.down_flag;
    }
    else if (strcmp(command, "SetPuTransparenceCfg") == 0)
    {
        cncTransparence_t *p_Transparence = (cncTransparence_t*)structBuf;
        transparence_info_t info;
        memset(&info, 0, sizeof(transparence_info_t));
        st_param_getTransparenceStruct(&info);

        info.serial_no = p_Transparence->serialNo;
        info.up_flag = p_Transparence->upFlag;
        info.down_flag = p_Transparence->downFlag;
        st_param_setTransparenceStruct(info);
    }
    else if (strcmp(command, "GetPuStateInfo") == 0)
    {
#if 0
        cncState_t *p_state = (cncState_t*)structBuf;
        p_state->cpuPercent = st_app_getCpuUsingPercent();
        p_state->runSecond = st_app_getSystemRunTime();
        p_state->memPercent = st_app_getMemUsedPercent();
        p_state->flashSize = 8;
        func_getNetByte(p_state->recvByte, p_state->sendByte);
#endif
    }
#if 0
    else if (strcmp(command, "Get3GConfigInfo") == 0)
    {
        cnc3G_t *p_3G = (cnc3G_t*)structBuf;
        cdma_nonego_enable_t    info;
        memset(&info, 0, sizeof(cdma_nonego_enable_t));
        st_param_getCdmaNonegoEnableStruct(&info);

        p_3G->enable = info.enable;
        strncpy(p_3G->MobileNumber, info.phone, CNC_PHONE_LEN-1);
        strncpy(p_3G->status, info.normal_up, CNC_MMS_LEN-1);
        strncpy(p_3G->OnlineSMS, info.force_up, CNC_MMS_LEN-1);
        strncpy(p_3G->OfflineSMS, info.force_down, CNC_MMS_LEN-1);
    }
#endif
#endif
    else if (strcmp(command, "GetSysConfig") == 0)
    {
        //sys_config_t sysConfig;
        ///st_param_getSysConfig(&sysConfig);
        memcpy(structBuf, "12345678", 8);
    }
#if 0
    /*注意：(protocol_chang)协议改变2013-05-28，*/
    /*增加GetGuStateInfo 专用于NVR,非NVR设备可以不管*/
    else if (strcmp(command, "GetGuStateInfo") == 0)
    {
        cncOnlineState_t  *guState = (cncOnlineState_t*)structBuf;
//		local_chn_info_t local_chn;
//		unsigned long ipc_usr_id = 0;
//		bzero(&local_chn, sizeof(local_chn_info_t));
        //guState->totalChNum =  g_cbFunc.channelNum;
        printf("Get channnelNum ...................\n");
        guState->totalChNum = st_param_getChannelNum();
        printf("\ng_cbFunc.channelNum:%d.\n", g_cbFunc.channelNum);
        printf("\nguState->totalChNum:%d.\n", guState->totalChNum);
        int i = 0;
        while(i < guState->totalChNum)
        {
            guState->chState[i].chInfo = i;
#if 0
            st_param_getLocalChnInfo(i, &local_chn);
            ipc_usr_id = st_nvr_getUserId(i, local_chn.conn_proto);
            if(ipc_usr_id == 0)
            {
                printf("Error.\n");
                guState->chState[i].onlineState = 0;
                i++;
                continue;
            }
#endif
            guState->chState[i].onlineState = st_nvr_getLocalChnState(i);
            /*if(local_chn.conn_proto == CONN_PROTO_ST)
            {
            	guState->chState[i].onlineState = st_netcli_isServLogined(ipc_usr_id);
            }
            else if(local_chn.conn_proto == CONN_PROTO_ONVIF)
            {
            	guState->chState[i].onlineState = 1;
            }
            else
            {
            	return -1;
            }*/
            i++;
        }
    }
#endif
    /*注意：(protocol_chang)协议改变2013-05-28，*/
    /*增加GetPuType用于区分NVR与其他设备，NVR需赋值为1，其他设备赋值为0即可*/
    else if (strcmp(command, "GetPuType") == 0)
    {
        int  *puType = (int*)structBuf;
        //getpuType(puType);
        //*puType =0;
        *puType = 1;
    }
#if 0
    else if (strcmp(command, "GetPuRfAlarmCfg") == 0)
    {
        cncRfAlarm_t*   pRfAlarm = (cncRfAlarm_t*)structBuf;
        rf_alarm_t      info;
        sys_config_t    sc;
        memset(&sc, 0, sizeof(sc));
        st_param_getSysConfig(&sc);
        if (pRfAlarm->videoId >= sc.rf_num)
            return -1;
        st_param_getRfStruct(pRfAlarm->videoId, &info);

        strncpy(pRfAlarm->rfName, info.name, MAX_DEVICE_NAME_LEN-1);
        pRfAlarm->rfId = info.rf_id;
        pRfAlarm->rfType = info.rf_type;
        pRfAlarm->rfEnable = info.rf_enable;
        pRfAlarm->beepOutputEnable = info.beep_output_enable;
        pRfAlarm->beepOutputTime = info.beep_output_time;
        pRfAlarm->jointOutputTime = info.joint_output_time;
        pRfAlarm->snapNum = info.capture_num;
        pRfAlarm->jointRecordTime = info.joint_record_time;
        pRfAlarm->unused[0] = info.unused[0];
        pRfAlarm->jointOutputEnableCh = info.joint_output_enable_ch;
        pRfAlarm->jointSnapEnableCh = info.joint_snap_enable_ch;
        pRfAlarm->jointRecordEnableCh = info.joint_record_enable_ch;
        //////////////////////////////
        int day = pRfAlarm->weekday;
        int i;
        for (i = 0; i < CNC_SEG_NUM; i++)
        {
            pRfAlarm->timeseg[i].startHour = info.rf_day[day].time_seg[i].start_hour;
            pRfAlarm->timeseg[i].startMinute = info.rf_day[day].time_seg[i].start_minute;
            pRfAlarm->timeseg[i].startSecond = info.rf_day[day].time_seg[i].start_second;
            pRfAlarm->timeseg[i].endHour = info.rf_day[day].time_seg[i].end_hour;
            pRfAlarm->timeseg[i].endMinute = info.rf_day[day].time_seg[i].end_minute;
            pRfAlarm->timeseg[i].endSecond = info.rf_day[day].time_seg[i].end_second;
        }
    }
    else if (strcmp(command, "SetPuRfAlarmCfg") == 0)
    {
        cncRfAlarm_t*   pRfAlarm = (cncRfAlarm_t*)structBuf;
        rf_alarm_t      info;
        sys_config_t sc;
        memset(&sc,0,sizeof(sc));

        st_param_getSysConfig(&sc);
        if (pRfAlarm->videoId >= sc.rf_num)
            return -1;
        st_param_getRfStruct(pRfAlarm->videoId, &info);
        info.rf_type = pRfAlarm->rfType;
        info.rf_enable = pRfAlarm->rfEnable;
        info.beep_output_enable = pRfAlarm->beepOutputEnable;
        info.beep_output_time = pRfAlarm->beepOutputTime;
        info.joint_output_time = pRfAlarm->jointOutputTime;
        info.capture_num = pRfAlarm->snapNum;
        info.joint_record_time = pRfAlarm->jointRecordTime;
        info.unused[0] = pRfAlarm->unused[0];
        info.joint_output_enable_ch = pRfAlarm->jointOutputEnableCh;
        info.joint_snap_enable_ch = pRfAlarm->jointSnapEnableCh;
        info.joint_record_enable_ch = pRfAlarm->jointRecordEnableCh;

        int day = pRfAlarm->weekday;
        int i, j;
        if (day == CNC_WEEKDAY_NUM)
        {
            day = 0;
            j = 0;
        }
        else
            j = CNC_WEEKDAY_NUM - 1;
        for (; j < CNC_WEEKDAY_NUM; j++)
        {
            for (i = 0; i < CNC_SEG_NUM; i++)
            {
                info.rf_day[day].time_seg[i].start_hour = pRfAlarm->timeseg[i].startHour;
                info.rf_day[day].time_seg[i].start_minute = pRfAlarm->timeseg[i].startMinute;
                info.rf_day[day].time_seg[i].start_second = pRfAlarm->timeseg[i].startSecond;
                info.rf_day[day].time_seg[i].end_hour = pRfAlarm->timeseg[i].endHour;
                info.rf_day[day].time_seg[i].end_minute = pRfAlarm->timeseg[i].endMinute;
                info.rf_day[day].time_seg[i].end_second = pRfAlarm->timeseg[i].endSecond;
            }
            day++;
        }
        st_param_setRfStruct(pRfAlarm->videoId, info);
    }
    else if (strcmp(command, "GetPuVideoCoverCfg") == 0)
    {
        cncVideoCover_t* pVideoCover = (cncVideoCover_t*)structBuf;
        video_cover_t   info;
        st_param_getVideoCoverStruct(pVideoCover->videoId, &info);

        pVideoCover->coverEnable = info.cover_enable;
        pVideoCover->sensitiveLevel = info.sensitive_level;
        pVideoCover->alarmIntervalTime = info.alarm_interval_time;
        pVideoCover->jointRecordTime = info.joint_record_time;
        pVideoCover->jointOutputTime = info.joint_output_time;
        pVideoCover->beepOutputTime = info.beep_output_time;
        pVideoCover->beepOutputEnable = info.beep_output_enable;
        pVideoCover->jointSnapEnableCh = info.joint_snap_enable_ch;
        pVideoCover->jointOutputEnableCh = info.joint_record_enable_ch;
        pVideoCover->jointRecordEnableCh = info.joint_output_enable_ch;
        //////////////////////////////
        int day = pVideoCover->weekday;
        int i;
        for (i = 0; i < CNC_SEG_NUM; i++)
        {
            pVideoCover->timeseg[i].startHour = info.cover_day[day].time_seg[i].start_hour;
            pVideoCover->timeseg[i].startMinute = info.cover_day[day].time_seg[i].start_minute;
            pVideoCover->timeseg[i].startSecond = info.cover_day[day].time_seg[i].start_second;
            pVideoCover->timeseg[i].endHour = info.cover_day[day].time_seg[i].end_hour;
            pVideoCover->timeseg[i].endMinute = info.cover_day[day].time_seg[i].end_minute;
            pVideoCover->timeseg[i].endSecond = info.cover_day[day].time_seg[i].end_second;
        }
    }
    else if (strcmp(command, "SetPuVideoCoverCfg") == 0)
    {
        cncVideoCover_t* pVideoCover = (cncVideoCover_t*)structBuf;
        video_cover_t info;
        st_param_getVideoCoverStruct(pVideoCover->videoId, &info);

        info.cover_enable = pVideoCover->coverEnable;
        info.sensitive_level = pVideoCover->sensitiveLevel;
        info.alarm_interval_time = pVideoCover->alarmIntervalTime;
        info.joint_record_time = pVideoCover->jointRecordTime;
        info.joint_output_time = pVideoCover->jointOutputTime;
        info.beep_output_time = pVideoCover->beepOutputTime;
        info.beep_output_enable = pVideoCover->beepOutputEnable;
        info.joint_snap_enable_ch = pVideoCover->jointSnapEnableCh;
        info.joint_record_enable_ch = pVideoCover->jointOutputEnableCh;
        info.joint_output_enable_ch = pVideoCover->jointRecordEnableCh;

        int day = pVideoCover->weekday;
        int i, j;
        if (day == CNC_WEEKDAY_NUM)
        {
            day = 0;
            j = 0;
        }
        else
            j = CNC_WEEKDAY_NUM - 1;
        for (; j < CNC_WEEKDAY_NUM; j++)
        {
            for (i = 0; i < CNC_SEG_NUM; i++)
            {
                info.cover_day[day].time_seg[i].start_hour = pVideoCover->timeseg[i].startHour;
                info.cover_day[day].time_seg[i].start_minute = pVideoCover->timeseg[i].startMinute;
                info.cover_day[day].time_seg[i].start_second = pVideoCover->timeseg[i].startSecond;
                info.cover_day[day].time_seg[i].end_hour = pVideoCover->timeseg[i].endHour;
                info.cover_day[day].time_seg[i].end_minute = pVideoCover->timeseg[i].endMinute;
                info.cover_day[day].time_seg[i].end_second = pVideoCover->timeseg[i].endSecond;
            }
            day++;
        }
        st_param_setVideoCoverStruct(pVideoCover->videoId, info);
    }
    else if (strcmp(command, "Set3GConfigInfo") == 0)
    {
        cnc3G_t *p_3G = (cnc3G_t*)structBuf;
        cdma_nonego_enable_t    info;
        memset(&info, 0, sizeof(cdma_nonego_enable_t));
        st_param_getCdmaNonegoEnableStruct(&info);

        info.enable = p_3G->enable;
        strncpy(info.phone, p_3G->MobileNumber, MAX_PHONE_LENGH-1);
        strncpy(info.normal_up, p_3G->status, MAX_ALARM_CONTENT_LENGH-1);
        strncpy(info.force_up, p_3G->OnlineSMS, MAX_ALARM_CONTENT_LENGH-1);
        strncpy(info.force_down, p_3G->OfflineSMS, MAX_ALARM_CONTENT_LENGH-1);
        st_param_setCdmaNonegoEnableStruct(info);
    }
    else if (strcmp(command, "ClearForceBit") == 0)
    {
        st_setCdmaNonegoFlag(0);
    }
    else if (strcmp(command, "Get3GTimeInfo") == 0)
    {
        cnc3GTime_t *p_3GTime = (cnc3GTime_t*)structBuf;
        timer_week_t info;
        int day = p_3GTime->weekday;
        memset(&info, 0, sizeof(cdma_time_t));
        st_param_getCdmaTimeStruct(day, &info);
        p_3GTime->enable = info.enable_flag;

        int i;
        for (i = 0; i < CNC_SEG_NUM; i++)
        {
            p_3GTime->timeseg[i].startHour = info.time_seg[i].start_hour;
            p_3GTime->timeseg[i].startMinute = info.time_seg[i].start_minute;
            p_3GTime->timeseg[i].startSecond = info.time_seg[i].start_second;
            p_3GTime->timeseg[i].endHour = info.time_seg[i].end_hour;
            p_3GTime->timeseg[i].endMinute = info.time_seg[i].end_minute;
            p_3GTime->timeseg[i].endSecond = info.time_seg[i].end_second;
        }
    }
    else if (strcmp(command, "Set3GTimeInfo") == 0)
    {
        cnc3GTime_t *p_3GTime = (cnc3GTime_t*)structBuf;
        timer_week_t info;
        int day = p_3GTime->weekday;
        int i, j;
        if (day == CNC_WEEKDAY_NUM)
        {
            day = 0;
            j = 0;
        }
        else
            j = CNC_WEEKDAY_NUM - 1;
        for (; j < CNC_WEEKDAY_NUM; j++)
        {
            st_param_getCdmaTimeStruct(day, &info);
            info.enable_flag = p_3GTime->enable;
            for (i = 0; i < CNC_SEG_NUM; i++)
            {
                info.time_seg[i].start_hour = p_3GTime->timeseg[i].startHour;
                info.time_seg[i].start_minute = p_3GTime->timeseg[i].startMinute;
                info.time_seg[i].start_second = p_3GTime->timeseg[i].startSecond;
                info.time_seg[i].end_hour = p_3GTime->timeseg[i].endHour;
                info.time_seg[i].end_minute = p_3GTime->timeseg[i].endMinute;
                info.time_seg[i].end_second = p_3GTime->timeseg[i].endSecond;
            }
            st_param_setCdmaTimeStruct(day, info);
            day++;
        }
    }
    else if (strcmp(command, "Get3GModuleStatusInfo") == 0)
    {
        cnc3GState_t *pInfo = (cnc3GState_t*)structBuf;

        at_lib_status_t module_status;
        memset(&module_status,0,sizeof(at_lib_status_t));
        st_atGetLibStatus(&module_status);

        pInfo->enable = module_status.init_flag;
        pInfo->enable_ppp = module_status.enable_ppp;
        pInfo->enable_sms = module_status.enable_sm;
        pInfo->send_sms_flag = module_status.send_sms_flag;
        pInfo->sim_status = module_status.simst;
        pInfo->signal_Strength = module_status.rssi;
        strncpy(pInfo->ppp_name, module_status.ppp_name, 31);
        strncpy(pInfo->sms_name, module_status.sm_name, 31);
        strncpy(pInfo->network, module_status.network, 31);
        strncpy(pInfo->sms_phone, module_status.sm_phone, 31);
    }
#endif


    /**********/
#endif
    return 0;
}


int    st_3512_dealMduCommand(const char *command, cncMduServer_t *p_mduServer)
{
    if (command == NULL || p_mduServer == NULL)
    {
        //st_dbg_print(DBG,L_NORMAL,"st_dealCsgCommand(), The input arg is error.\n");
        printf("st_dealCsgCommand(), The input arg is error.\n");
        return -1;
    }

    int     ret;
    int     channel = p_mduServer->channel;
    int     streamType = p_mduServer->streamType;
    static  pthread_t   mduPid[2][CHANNEL_NUM], mutexMduPid, pollMduPid;
	#if 0
    /*注意：(protocol_chang)协议改变2013-05-02，添加如下七行代码*/
    char doubleStreamFlag = 0x01;
    sys_config_t sys_cfg;
    memset(&sys_cfg, 0, sizeof(sys_config_t));
    st_param_getSysConfig(&sys_cfg);
    doubleStreamFlag &= sys_cfg.ds_enable_flag;
    if((doubleStreamFlag != 0x01) && (streamType == 1))
        return 0;
	#endif
	
    if (channel < 2)
        mduServer[streamType][channel] = *p_mduServer;
    printf("st_3512_dealMduCommand  command:%s ,channel:%d ,streamType:%d \n",command,channel,streamType);

    if (strcmp(command, "StartVideoServer") == 0)
    {
        if (channel == MUTEX_CHANNEL)       // mutex channel
        {
            if (g_mutexVideoFlag > 0)
            {
                g_mutexVideoFlag = 0;
                pthread_join(mutexMduPid, NULL);
            }

            ret = pthread_create(&mutexMduPid, NULL, (void*)thread_startMutexVideo, (void*)&streamType);
            if (ret != 0)
            {
                printf("StartMutexVideo error.\n");
                return ret;
            }
        }
        else if (channel == POLL_CHANNEL)       // poll channel
        {
            if (g_pollVideoFlag > 0)
            {
                g_pollVideoFlag = 0;
                pthread_join(pollMduPid, NULL);
            }

            ret = pthread_create(&pollMduPid, NULL, (void*)thread_startPollVideo, (void*)&streamType);
            if (ret != 0)
            {
                printf("StartPollVideo error.\n");
                return ret;
            }
        }
        else        // common channel
        {
            if (g_videoFlag[streamType][channel] > 0)
            {
                g_videoFlag[streamType][channel] = 0;
                pthread_join(mduPid[streamType][channel], NULL);
                mduPid[streamType][channel] = 0;
            }

            ret = pthread_create(&mduPid[streamType][channel], NULL,
                                 thread_startVideoServer, (void*)&mduServer[streamType][channel]);
            if (ret != 0)
            {
                printf("Start video[%d][%d] thread error.\n", streamType, channel);
                return ret;
            }
        }
    }
    else if (strcmp(command, "StopVideoServer") == 0)
    {
        if (channel == MUTEX_CHANNEL && g_mutexVideoFlag > 0)
        {
            g_mutexVideoFlag = 0;
            pthread_join(mutexMduPid, NULL);
        }

        if (channel == POLL_CHANNEL && g_pollVideoFlag > 0)
        {
            g_pollVideoFlag = 0;
            pthread_join(pollMduPid, NULL);
        }

        if (channel < g_cbFunc.channelNum && g_videoFlag[streamType][channel] > 0)
        {
            g_videoFlag[streamType][channel] = 0;
            pthread_join(mduPid[streamType][channel], NULL);
            mduPid[streamType][channel] = 0;
        }
    }

    else if (strcmp(command, "ForceKeyFrame") == 0)
    {
#if 0
        if (channel < g_cbFunc.channelNum)
        {
            st_codec_forceIFrame(channel, streamType);
            printf("ForceKeyFrame[%d][%d].\n", channel, streamType);
        }
#endif
    }

    else if (strcmp(command, "StartSendAudio") == 0)
    {
#if 0
        if (g_audioFlag > 0)
        {
            g_audioFlag = 0;
            pthread_join(aduPid, NULL);
        }

        st_app_setEnableAudioSpeakerFlag(1);
        st_codec_setAudioAoMute(ST_FALSE);
        st_codec_startTalk(getAudioTalkData);
        ret = pthread_create(&aduPid, NULL, (void*)thread_startSendAudioServer, NULL);
        if (ret != 0)
        {
            printf("StartVideoServer error. streamType = %d, channel = %d\n",
                   streamType, channel);
            return ret;
        }
#endif
    }
    else if (strcmp(command, "StopSendAudio") == 0)
    {
#if 0
        if (g_audioFlag > 0)
        {
            g_audioFlag = 0;
            pthread_join(aduPid, NULL);
        }
        st_app_setEnableAudioSpeakerFlag(0);
        st_codec_stopTalk();
#endif
    }

    return 0;
}


int 	st_3512_dealVodCommand(const char *command, cncVodServer_t *p_VodServer)
{
    if (command == NULL || p_VodServer == NULL)
    {
        printf("st_dealVodCommand(), The input arg is error.\n");
        return -1;
    }

    static	pthread_t		vodPId[CNC_VOD_SERVER_NUM];
    static	cncVodServer_t	vodServer[CNC_VOD_SERVER_NUM];
    //static	char			tmpJpeg[128*1024];
    char    pBuf;
    static	int				tmpJpegLen;
    int		ret;

    if (strcmp(command, "GetRcdFileCfg") == 0)
    {
        if (strstr(p_VodServer->fileName, "_tmp.jpeg") != NULL)
        {
            //memset(&tmpJpeg, 0, sizeof(tmpJpeg));
            int		channel = atoi(p_VodServer->fileName) - 1;

            ///st_codec_getSnapJPEG(channel, NULL,&pBuf, &tmpJpegLen);
            //st_enc_getSnapJPEG(channel, tmpJpeg, &tmpJpegLen);
            p_VodServer->fileSize = tmpJpegLen;
        }
        else if (strstr(p_VodServer->fileName, ".jpeg") == NULL)
            sscanf(p_VodServer->fileName, "%*[^:]:%*[^:]:%*[^:]:%08x-%08x:%08x",
                   &p_VodServer->startTime, &p_VodServer->endTime, &p_VodServer->fileSize);
        else
            sscanf(p_VodServer->fileName, "%*[^:]:%*[^:]:%08x", &p_VodServer->fileSize);
    }
    else if (strcmp(command, "StartPlayServer") == 0)
    {
        vodServer[p_VodServer->vodId] = *p_VodServer;

        ret = pthread_create(&vodPId[p_VodServer->vodId], NULL,
                             (void*)thread_startPlayServer, (void*)&vodServer[p_VodServer->vodId]);
        if (ret != 0)
        {
            printf("StartPlayServer error. vodId = %d, fileName = %s\n",
                   p_VodServer->vodId, p_VodServer->fileName);
            return ret;
        }
    }

    else if (strcmp(command, "StartRecordServer") == 0)
    {
        if (strstr(p_VodServer->fileName, "_tmp.jpeg") != NULL)
        {
            int		leftSize, onceSize, sendSize;

            leftSize = tmpJpegLen;
            sendSize = 0;

            while (leftSize > 0)
            {
                onceSize = (leftSize > 4096) ? 4096 : leftSize;
//				ret = st_cnc_sendRecordFrame(p_VodServer->vodId,
//					tmpJpeg + sendSize, onceSize);
                ret = st_cnc_sendRecordFrame(p_VodServer->vodId,
                                             pBuf + sendSize, onceSize);

                ////st_codec_releaseJPEG(pBuf);
                if (ret < 0)
                {
                    printf("st_cnc_sendRecordFrame error.filename = %s\n",
                           p_VodServer->fileName);
                    return -1;
                }
                sendSize += onceSize;
                leftSize -= onceSize;
            }
            st_cnc_sendRecordFrame(p_VodServer->vodId, "end", 3);
        }
        else
        {
            vodServer[p_VodServer->vodId] = *p_VodServer;

            ret = pthread_create(&vodPId[p_VodServer->vodId], NULL,
                                 (void*)thread_startRecordServer, (void*)&vodServer[p_VodServer->vodId]);
            if (ret != 0)
            {
                printf("StartPlayServer error. vodId = %d, fileName = %s\n",
                       p_VodServer->vodId, p_VodServer->fileName);
                return ret;
            }
        }
    }

    else if (strcmp(command, "StopVodServer") == 0)
    {
        int vodId = p_VodServer->vodId;

        if (vodPId[vodId] > 0)
        {
            pthread_cancel(vodPId[vodId]);
            pthread_join(vodPId[vodId], NULL);
            vodPId[vodId] = 0;
        }
        printf("vodId(%d), fileName(%s) send thread quit.\n",
               vodId, vodServer[vodId].fileName);
    }

    return 0;
}


int 	st_3512_uploadMsgToNvs(void* param)
{
    cncAlarm_t		alarm;
#if 0
    read_pos_t		read_pos;
    alarm_info_t*	p_info;
    netcom_config_t temp;

    read_pos.read_begin = st_stream_getAlarmStreamWritePos();
    read_pos.read_end = st_stream_getAlarmStreamWritePos();

    g_uploadMsgFlag = 1;
    while (g_uploadMsgFlag)
    {
        st_param_getNetcomConfig(&temp);
        if (temp.isOnLine == 0)
        {
            usleep(200*1000);
            continue;
        }

        p_info = st_stream_getAlarmStreamFromPool(&read_pos);
        if (p_info != NULL)
        {
#if 1
            if (p_info->alarm_type == ALARM_VIDEO_LOSE)
            {
                alarm.alarmType = 2;
                alarm.subType = 1;
            }
            else if (p_info->alarm_type == ALARM_VIDEO_MOTION)
            {
                alarm.alarmType = 1;
                alarm.subType = 1;
            }
            else if (p_info->alarm_type == ALARM_PROBER)
            {
                prober_alarm_t prober;
                st_param_getProberStruct(p_info->alarm_channel-1, &prober);
                if (prober.sub_prober_type == 1) //巡更告警
                    alarm.alarmType = 6;
                else
                    alarm.alarmType = 0;
                alarm.subType = 1;
            }
            else if (p_info->alarm_type == ALARM_VIDEO_LOSE_STOP)
            {
                alarm.alarmType = 2;
                alarm.subType = 0;
            }
            else if (p_info->alarm_type == ALARM_VIDEO_MOTION_STOP)
            {
                alarm.alarmType = 1;
                alarm.subType = 0;
            }
            else if (p_info->alarm_type == ALARM_PROBER_STOP)
            {
                prober_alarm_t prober;
                st_param_getProberStruct(p_info->alarm_channel-1, &prober);
                if (prober.sub_prober_type == 1) //巡更告警
                    alarm.alarmType = 6;
                else
                    alarm.alarmType = 0;
                alarm.subType = 0;
            }
            else if(p_info->alarm_type == ALARM_GPS_INFO)
            {
                alarm.alarmType = 99;
                alarm.subType = 1;
            }
            alarm.videoId = p_info->alarm_channel - 1;
#endif
            if (st_cnc_sendAlarmFrame(CNC_UP_ALARM, (void*)&alarm) < 0)
                printf("st_cnc_sendAlarmFrame() error\n");
        }
        sleep(1);
    }
#endif
    return 0;
}


int init_cncPlatform(sdk_msg_dispatch_cb fun)
{
    int i = 0;
    cncCbFunc_t	tmp_cbFunc;
    memset(&tmp_cbFunc,0,sizeof(cncCbFunc_t));
    /* 三个回调函数，分别负责接入、转发、点播 */
    tmp_cbFunc.dealCsgCommand = st_3512_dealCsgCommand;  //消息发送
    tmp_cbFunc.dealMduCommand = st_3512_dealMduCommand; //转发服务器
    tmp_cbFunc.dealVodCommand = st_3512_dealVodCommand;  //点播

    printf("init_cncPlatform channelNum :%d \n",2);
    tmp_cbFunc.channelNum  = 2;//st_param_getChannelNum();
    msg_dispatch_cb  = fun;
    printf("msg_dispatch_cb :%p \n",msg_dispatch_cb);

    st_cnc_startWsxServer(1,&tmp_cbFunc);
    pthread_t	upPID;
    if (pthread_create(&upPID, NULL, (void*)st_3512_uploadMsgToNvs, NULL) != 0) //报警上报
    {
        printf("Create up notice pthread failure!\n");
        return -1;
    }

    return 0;
}

#endif

