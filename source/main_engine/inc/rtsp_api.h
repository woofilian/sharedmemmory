#ifndef _HI_RTSP_GLOBAL_H_
#define _HI_RTSP_GLOBAL_H_

/*
* Copyright (c) 2010
* All rights reserved.
* 
* filename:hi_rtsp_global.h
* 
* func:本文件实现了对rtsp 的初始化和rtsp 回调函数定义
* 
* author : hi
* createdate: 2011-01-27
*
* modify list: 
* 
*/
//#include "rtsp_struct.h"

#ifdef __cplusplus
extern "C" {
#endif 

#pragma pack(push, 4)

#define SUCCESS  0
#define FAILURE  (-1)

#define RTSP_SUPPORT_EXT 1 //支持rtp 扩展数据
#define RTSP_MAX_NAL_NUM 8

//+++++++++++++rongp add+++++++++++++++++//
#define MUL_VIDEO_UDP_ADDR              "239.1.1.10"
#define MUL_AUDIO_UDP_ADDR              "239.1.1.11"
//+++++++++++++rongp add end+++++++++++++++++//

/*RTP Payload type define*/
typedef enum _RTP_PT_E_
{
    RTP_PT_ULAW             = 0,        /* mu-law  PCMU*/
    RTP_PT_GSM              = 3,        /* GSM */
    RTP_PT_G723             = 4,        /* G.723 */
    RTP_PT_ALAW             = 8,        /* a-law  PCMA*/
    RTP_PT_G722             = 9,        /* G.722 */
    RTP_PT_S16BE_STEREO     = 10,       /* linear 16, 44.1khz, 2 channel */
    RTP_PT_S16BE_MONO       = 11,       /* linear 16, 44.1khz, 1 channel */
    RTP_PT_MPEGAUDIO        = 14,       /* mpeg audio */
    RTP_PT_JPEG             = 26,       /* jpeg */
    RTP_PT_H261             = 31,       /* h.261 */
    RTP_PT_MPEGVIDEO        = 32,       /* mpeg video */
    RTP_PT_MPEG2TS          = 33,       /* mpeg2 TS stream */
    RTP_PT_H263             = 34,       /* old H263 encapsulation */
                            
   
    RTP_PT_H264             = 96,       /* */
    RTP_PT_G726             = 97,       /* */
	RTP_PT_AAC              = 96, 
	RTP_PT_ADPCM			= 99,
	
    RTP_PT_INVALID          = 127
}RTP_PT_E;

typedef enum _RTSP_MSG_NOTIFY_
{
	RTSP_MSG_PREVIEW_LINK		= 0x1,
	RTSP_MSG_PREVIEW_UNLINK  	= 0x2,
}RTSP_MSG_NOTIFY;


typedef enum _RTSP_CODEC_TYPE_
{
  RTSP_CODEC_UNKNOWN = 0,

  RTSP_VIDEO_CODEC_START = 10,
  RTSP_VIDEO_H264,
  RTSP_VIDEO_MPEG4,
  RTSP_VIDEO_MJPEG,
  RTSP_VIDEO_END,

  RTSP_AUDIO_CODEC_START = 20,
  RTSP_AUDIO_G711A,
  RTSP_AUDIO_G711U,
  RTSP_AUDIO_CODEC_END
} RTSP_CODEC_TYPE;

typedef enum _HI_RTSP_GET_TYPE
{
    RTSP_PREV_TYPE = 0,     //预览获取方式 音频帧从最新的帧获取，视频帧需等到最新的帧为I帧
    RTSP_REC_TYPE     ,     //录像获取方式，视频从指定的时间前的I帧开始读起 
    RTSP_NEWEST_I_TYPE,    //最新I帧
}HI_RTSP_GET_TYPE;

typedef enum _HI_RTSP_FRAME_TYPE_
{
  RTSP_UNKNOWN_FRAME = 0,
  RTSP_I_FRAME,
  RTSP_P_FRAME,
  RTSP_B_FRAME,
  RTSP_AUDIO_FRAME,
  RTSP_GENERIC_FRAME,  
} RTSP_FRAME_TYPE;

typedef struct _RTSP_MEDIA_INFO_
{
  RTSP_CODEC_TYPE eVideoType;                 /* 视频码流类型，取值范围参考 av_type_t 定义 */
  unsigned short     u16Width;                   /* 视频宽度                                  */
  unsigned short     u16Height;                  /* 视频高度                                  */
  unsigned long      u32Quality;                 /* 视频质量                                  */
  unsigned long      u32AvgBit;                  /* 码流速率，单位: 位/秒                     */ 
  unsigned long      u32Samples;                /* 样本速率，单位: HZ, 如 90000 HZ           */
  unsigned long      u32FrameRate;              /* 帧率, 最大 255 帧/秒                      */
  RTSP_CODEC_TYPE eAudioType;
  unsigned long      u32AudioBit;
  unsigned long      u32AudioSample;
} RTSP_MEDIA_INFO, *LP_RTSP_MEDIA_INFO;

typedef struct _RTSP_AV_DATA_
{
  unsigned long            u32Index;                  /* 数据编号，或称: 帧号  */
  unsigned long            u32Latest;                 /* 最新的帧号             */
  RTSP_CODEC_TYPE       eType;                      /* 数据编码类型            */       
  unsigned long            u32NalLen[RTSP_MAX_NAL_NUM];
  unsigned char            u8IFrame;
  unsigned char            u8Res[3];
  unsigned long            u32Size;                   /* data 中的数据大小     */
  char                     *data;                     /* 数据缓冲              */
  unsigned long long       u64TimeStamp;              /* 数据的时间戳         */  
  unsigned short           u16Width;
  unsigned short           u16Height;
} RTSP_AV_DATA, *LPRTSP_AV_DATA;

typedef struct _RTP_EX_DATA_
{
  unsigned long  magic_no;        // 0x0132DCA9 -- 20110505
  unsigned long  frame_num;       // 帧号
  unsigned long  frame_type;      // j_frame_type_t
  unsigned long  codec;           // j_codec_type_t
  unsigned short width;           // 宽度
  unsigned short height;          // 高度
  long           timestamp_sec;   // 时间戳，秒
  long           timestamp_usec;  // 时间戳，微秒
  long           size;            // frame lens
}RTP_EX_DATA, *LPRTP_EX_DATA;
#if 0
typedef  void* media_handle;

/* 设备参数接口 */
typedef struct _media_device_ops
{
	int (*get_param)(int param_id, int channel, void *param, void *user); /* 获取设备参数 */
	int (*set_param)(int param_id, int channel, void *param, void *user); /* 设置设备参数 */
}baidu_device_ops;

/* 实时流接口 */
typedef struct _media_ls_avs_ops
{
    int32_t (*open)(media_handle m, int32_t channel, int32_t level, void *media_info);/* 打开 */
    int32_t (*play)(media_handle m,int32_t channel);                                     /* 播放 */
    int32_t (*ctrl)(media_handle m, uint32_t cmd, void *data);           /* 控制 */
    void    (*close)(media_handle m,int32_t channel);                                    /* 关闭 */
    int32_t (*pull)(media_handle m,int32_t channel, char **frm);                          /* 拉流 */
}media_ls_avs_ops;

/* 历史流接口 */
typedef struct _media_hs_avs_ops
{
    int32_t (*open)(media_handle m, int32_t channel, int32_t level
                    , int32_t type, uint8_t *start_time, uint8_t *end_time
                    , uint8_t *property, void *mi);                 /* 打开 时间格式:YYYYMMDDHHMMSS */
    int32_t (*play)(media_handle m,int32_t channel);                                     /* 开始 */
    int32_t (*pause)(media_handle m,int32_t channel);                                    /* 暂停 */
    int32_t (*lseek)(media_handle m, uint32_t ts);                       /* 定位 */
    void    (*close)(media_handle m,int32_t channel);                                    /* 关闭 */
    int32_t (*pull)(media_handle m, int32_t channel,sdk_frame_t *frm);                       /* 拉流 */
}media_hs_avs_ops;

#endif

typedef void * RTSP_AV_HDL;
//用户检测回调
typedef int (*LogFun)(int level, const char *fName, const char *func, int nLine, const char *fmt,...);

typedef RTSP_AV_HDL (openStream)(int nCh, int nStreamNo, RTSP_MEDIA_INFO *pMediaInfo);

typedef int (requestIDRCb)(RTSP_AV_HDL pHdl);

typedef int (getVideoStream)(RTSP_AV_HDL pHdl, RTSP_AV_DATA *pData);

typedef int (getAudioStream)(RTSP_AV_HDL pHdl, RTSP_AV_DATA *pData);

typedef int (setPlayType)(RTSP_AV_HDL pHdl, int nType);
    
typedef int (closeStream)(RTSP_AV_HDL pHdl);

typedef int (msgCb)(RTSP_MSG_NOTIFY eMsgNotify, int nCh, int nStreamType, char *pPar);

typedef int (checkUserCb)(char *szUser, char *szPsw, int nGetPsw);

typedef int (checkAudioEnable)(int channel);

typedef struct _RTSP_PLAY_CB_
{
    checkUserCb         *funcCheckUser;
    msgCb               *funcMsg;
    openStream          *funcOpenStream;
    closeStream         *funcCloseStream;
    requestIDRCb        *funcIdrCb;
    setPlayType         *funcSetPlayType;
    getVideoStream      *funcGetVideoStream;
    getAudioStream      *funcGetAudioStream;
    checkAudioEnable    *funcCheckAudioEnable;
	LogFun				 *funcWriteLog;
}RTSP_PLAY_CB, *LP_RTSP_PLAY_CB;

typedef struct _RTSP_CFG_
{
    int     nMaxCh;             //设备支持的视频最大通道数
    int     nStreamNum;         //设备一个通道支持的码流数量 (1.单码流 2.双码流 3.三码流 以此类推)
    int     nRtspPort;          //rtsp 服务端口默认为554
    int     bUseAuth;           //是否启用校验 0 不启用 1启用
    int     bRtspMode;          // rtsp工作模式 0 被动上传视频 1 主动上传视频
    int     nRes[4];
}RTSP_CFG, *LP_RTSP_CFG;
//============================================================================
//函数:hi_rtsp_start 
//
//功能:启动rtsp 服务
//
//HI_RTSP_CFG     *pCft
//
//HI_RTSP_PLAY_CB *pPlayCb
//=============================================================================
int		sdk_rtsp_start(RTSP_CFG *pCfg, RTSP_PLAY_CB *pPlayCb);

//============================================================================
//函数:hi_rtsp_stop 
//
//功能:停止rtsp 服务
//
//=============================================================================
int		sdk_rtsp_stop();

extern RTSP_PLAY_CB	     gRtspPlayCb;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif 

#endif
