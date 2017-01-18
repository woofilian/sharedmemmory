#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <assert.h>

#include "sdk_struct.h"
#include "main.h"
#include "rtsp_api.h"

#include "streamlib.h"
#include "log.h"




//#elif defined(_AMBARELLA_)//&& !defined(_WINSOCK2API_)
//安霸方案
//#warning >>>>>>>>>>>>>> "_AMBARELLA_" <<<<<<<<<<<<<<<<<<<



//获取nal 信息
static uint32_t get_one_nalunit(uint8_t *pNaluType,
                         uint8_t *pBuffer,
                         uint32_t data_size)
{
    uint32_t code, tmp, pos;
    for (code=0xffffffff, pos = 0; pos <4; pos++)
    {
        tmp = pBuffer[pos];
        ////info("%d === %d \n",pos,tmp);
        code = (code<<8)|tmp;
    }
    assert(code == 0x00000001); // check start code 0x00000001 (BE)

    *pNaluType = pBuffer[pos++] & 0x1F;
    for (code=0xffffffff; pos < data_size; pos++)
    {
        tmp = pBuffer[pos];
        if ((code=(code<<8)|tmp) == 0x00000001)
        {
            break; //next start code is found
        }
    }
    if (pos == data_size )
    {
        // next start code is not found, this must be the last nalu
        return data_size;
    }
    else
    {
        return pos-4+1;
    }
}

///MAX_NAL_NUM
static int get_nalu_info( uint8_t *videoData,int data_size,sdk_nal_desc_t *nal_info)
{
    uint32_t nal_unit_length;
    uint8_t nal_unit_type;
    uint32_t cursor = 0;
    uint32_t size = data_size;
    int nal_num = 0;
    // info("get_nalu_info start videoData:%p data_size:%d \n",videoData,data_size);
    do
    {
        nal_unit_length = get_one_nalunit(&nal_unit_type, videoData+cursor, size-cursor);
        assert(nal_unit_length > 0 && nal_unit_type > 0);
#if 0
        info("get_nalu_info nal_unit_length:%d  nal_unit_type:%d cursor:%d nal_num:%d \n"
             ,nal_unit_length
             ,nal_unit_type
             ,cursor
             ,nal_num);
#endif

        nal_info->nal[nal_num].nal_type = nal_unit_type;
        nal_info->nal[nal_num].nal_off =  cursor+4 ;//相对于videoData 的偏移
        nal_info->nal[nal_num].nal_size = nal_unit_length - 4;
        nal_info->nal_num++;
        nal_num = nal_info->nal_num;
        if(nal_info->nal_num >= MAX_NAL_NUM)
        {
            info("nal_num :%d >= MAX_NAL_NUM:%d  \n",nal_info->nal_num , MAX_NAL_NUM);
            break;
        }
        cursor += nal_unit_length;
    }
    while(cursor < size);
#if 0
    int i ;
    for(i = 0 ; i < nal_info->nal_num ; i++)
    {
        info("\n*********%d nal_type:%d  nal_off:%d nal_size:%d \n",i
             ,nal_info->nal[i].nal_type
             ,nal_info->nal[i].nal_off
             ,nal_info->nal[i].nal_size);
    }
    info("get nalu info end!!! \n");
#endif
    return 0;
}


static int CheckUser(char *szUser, char *szPsw, int nGetPsw)
{
    return 0;
}
static int DelMsg(RTSP_MSG_NOTIFY eMsgNotify, int nCh, int nStreamType, char *pPar)
{
    info("====eMsgNotify:%d====nCh:%d====nStreamType:%d \n",eMsgNotify,nCh,nStreamType);
    return 0;
}

//大于0 表示成功
static ST_HDL st_handle;
RTSP_AV_HDL OpenStream(int nCh, int nStreamNo, RTSP_MEDIA_INFO *pMediaInfo)
{
    int tmp_ch = nCh +1;
    // int tmp_ch = 1;
    
    info("OpenStream ch >>>>> %d \n",tmp_ch);
    pMediaInfo->eVideoType = RTSP_VIDEO_H264;                 /* 视频码流类型，取值范围参考 av_type_t 定义 */
    if(tmp_ch == 1 || tmp_ch == 3)
    {
        pMediaInfo->u16Width   = 1280;                   /* 视频宽度                                  */
        pMediaInfo->u16Height	 = 720;                  /* 视频高度                                  */
    }
    else
    {
        pMediaInfo->u16Width   = 720;                   /* 视频宽度                                  */
        pMediaInfo->u16Height	 = 480;                  /* 视频高度                                  */
    }
    pMediaInfo->u32Quality = 100;                 /* 视频质量                                  */
    pMediaInfo->u32AvgBit = 1024;                  /* 码流速率，单位: 位/秒                     */
    pMediaInfo->u32Samples = 90000;                /* 样本速率，单位: HZ, 如 90000 HZ           */
    pMediaInfo->u32FrameRate = 30;              /* 帧率, 最大 255 帧/秒                      */
    pMediaInfo->eAudioType = RTSP_AUDIO_G711A;
    pMediaInfo->u32AudioBit = 16;
    pMediaInfo->u32AudioSample = 8000;

     //st_handle =sdk_stream_Open(tmp_ch);
	 //return (RTSP_AV_HDL)st_handle;
	 return sdk_stream_Open(tmp_ch,0);
}

int CloseStream(RTSP_AV_HDL pHdl)
{
	info("============= sdk_stream_Close \n");
    sdk_stream_Close((ST_HDL)st_handle);
    return 0;
}

int RequestIFrameCB(RTSP_AV_HDL pHdl)
{
    ///sdk_stream_ReadNewFrame(pHdl);
    return 0;
}
int SetPlayType(RTSP_AV_HDL pHdl, int nType)
{
    //RTSP_PREV_TYPE
    return RTSP_PREV_TYPE;
}

int GetVideoStream(RTSP_AV_HDL pHdl, RTSP_AV_DATA *pData)
{
	info(" +_+_+_+_+_+_+_+_   GetVideoStream  123 +_+_+_+_+_+_+_+_+_+_+_+_\n");
    int i;
    sdk_nal_desc_t nal_info;
    char *buffer = NULL;
    sdk_frame_t   *frame_head = NULL;
//    info("======= start%d \n",pHdl);
    buffer = sdk_stream_ReadOneFrame(pHdl);
    if(buffer == NULL)
    {
       // info("======= buffer == NULL \n");
        return 0;
    }
    frame_head = (sdk_frame_t *)buffer;


    pData->u32Index = frame_head->frame_no;                  /* 数据编号，或称: 帧号  */
    pData->u32Latest = frame_head->frame_no;                 /* 最新的帧号             */
    pData->eType = RTSP_VIDEO_H264 ;                      /* 数据编码类型            */
    pData->u8IFrame = frame_head->frame_type ;//== SDK_VIDEO_FRAME_I)? RTSP_I_FRAME:RTSP_P_FRAME 定义是一样的
    //pData->u8Res[3];
    pData->u32Size =frame_head->frame_size;                   /* data 中的数据大小     */
    pData->u64TimeStamp =frame_head->pts;              /* 数据的时间戳         */

    pData->u16Width  = frame_head->video_info.width;
    pData->u16Height = frame_head->video_info.height;
	//info("frame_no:%d frame_type:%d frame_size:%d  width:%d height:%d \n"
	//			,frame_head->frame_no,frame_head->frame_type,frame_head->frame_size
	//			,frame_head->video_info.width,frame_head->video_info.height);
#if 1
	memset(&nal_info,0,sizeof(sdk_nal_desc_t));
	memset(pData->u32NalLen,0,sizeof( unsigned long)*RTSP_MAX_NAL_NUM);
    get_nalu_info( frame_head->data,frame_head->frame_size,&nal_info);

    for ( i = 0  ; i < nal_info.nal_num ; i ++ )
    {
        pData->u32NalLen[i] = nal_info.nal[i].nal_size;

    }
#endif
    memcpy(pData->data,frame_head->data,frame_head->frame_size);
    ///info("======= end \n");
    return frame_head->frame_size;
}

int GetAudioStream(RTSP_AV_HDL pHdl, RTSP_AV_DATA *pData)
{
	//获取音频流
    return 0;
}
int CheckAudioEnable(int channel)
{
    //OpenStream
    return 0;
}
int adapt_media_init(sdk_msg_dispatch_cb msg_cb,void *stream_handle)
{

    RTSP_CFG rtsp_cfg;
    RTSP_PLAY_CB rtsp_play_cb;
    memset(&rtsp_cfg,0,sizeof(RTSP_CFG));
    memset(&rtsp_play_cb,0,sizeof(RTSP_PLAY_CB));
    rtsp_cfg.nMaxCh = 4;             //设备支持的视频最大通道数
    rtsp_cfg.nStreamNum = 1;         //设备一个通道支持的码流数量 (1.单码流 2.双码流 3.三码流 以此类推)
    rtsp_cfg.nRtspPort = 554;          //rtsp 服务端口默认为554
    rtsp_cfg.bUseAuth = 0;           //是否启用校验 0 不启用 1启用
    rtsp_cfg.bRtspMode = 0;          // rtsp工作模式 0 被动上传视频 1 主动上传视频


    rtsp_play_cb.funcCheckUser =CheckUser;
    rtsp_play_cb.funcMsg;
    rtsp_play_cb.funcOpenStream = OpenStream;
    rtsp_play_cb.funcCloseStream = CloseStream;
    rtsp_play_cb.funcIdrCb = RequestIFrameCB;
    rtsp_play_cb.funcSetPlayType = SetPlayType;  //HI_RTSP_GET_TYPE
    rtsp_play_cb.funcGetVideoStream = GetVideoStream;
    rtsp_play_cb.funcGetAudioStream = GetAudioStream;
    rtsp_play_cb.funcCheckAudioEnable = CheckAudioEnable;
    rtsp_play_cb.funcWriteLog = NULL;
    sdk_rtsp_start(&rtsp_cfg, &rtsp_play_cb);
    return 0;
}
int adapt_media_stop()
{
    sdk_rtsp_stop();
    return 0;
}


