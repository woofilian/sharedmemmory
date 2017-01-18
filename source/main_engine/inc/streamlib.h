/******************************************************************************

                  版权所有 (C), 2012-2022, bingchuan

 ******************************************************************************
  文 件 名   : streamlib.h
  版 本 号   : v1.0
  作    者   : bingchuan
  生成日期   : 2015年1月31日
  功能描述   : streamlib.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2015年1月31日
    作    者   : bingchuan
    修改内容   : 创建文件

******************************************************************************/


#ifndef __STREAMLIB_H__
#define __STREAMLIB_H__

#include "sdk_global.h"
#include "sdk_struct.h"

#define SDK_STREAM_SIGNAL_STREAM  (1)
#define SDK_STREAM_TWO_STREAM     (2)

#define STREAM_MAX_FRAME_SIZE     (512*1024)
#define STREAM_MAX_NAL_NUM 	      (16) 

typedef enum _STREAM_FRAME_TYPE_E
{
	STREAM_I_FRAME_TYPE = 1,
	STREAM_P_FRAME_TYPE,
	STREAM_B_FRAME_TYPE,
	STREAM_A_FRAME_TYPE, 
}STREAM_FRAME_TYPE_E;

//typedef sdk_frame_t frame_t;
typedef sdk_frame_t frame_head_t;

typedef  void* ST_HDL;
typedef  void* POOL_HDL;   // 内存空间句柄
typedef  void* EV_HDL;		 //时间句柄


typedef struct _stream_ch_info_t
{
	int total_space;
	int index_num;
	int max_frame_size;
}stream_ch_info_t;

typedef struct __stream_manage_t
{
	int ch_num;					                                    /* 设备通道数 包括GPS 单独算通道优化 */
	int two_stream;					                                /* 双码流标志 1表示只有主码流, 2双码流 */
	int max_frame_size;				                                /* 最大帧数据长度 */
	int (*get_ch_streamInfoCB)(int ch, int ch_type
			, stream_ch_info_t *ch_stream_info); 	                /* 获取没个通道的配置的最大编码能力按此分配空间 为空时设置默认值 */
	//int (*get_g_video_resolutionCB)(int ch, int ch_type);         /* 由采集库提供接口 */
	//int (*get_audio_infoCB)(int ch, audio_encode_t *audio_encode);/* 由采集库提供接口 */
	int (*get_recordStatCB)(int ch, int ch_type);                   /* 必需提供 获取录像状态由录像库提供接口 */
	int (*force_iFrameCB)(int ch, int ch_type);                     /* 强制I帧*/
	int (*get_contrlTypeCB)(int ch); //int ch_type);                /* 目前表示，录像启动，停止标志 */
}stream_manage_t;

typedef struct stream_writeFrame_s
{
	int ch;
	int ch_type;
	int frame_len; /*包挂帧头 */
	
	int time;
    int time_us;
	int frame_type;
    int nal_num;                     	//nal-------------------当前帧nal数
    int nal_size[STREAM_MAX_NAL_NUM];	//nal-------------------每个nal的长度
    void *userdata;
	int (*userfillfun)(void *_handle, int size, char *buf);
}stream_writeFrame_t;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
/* --------------------- 变长音视频帧内存管理 ---------------------- */
/*
概念问题:
1  一个流通道 可以分多个子通道，目前有主通道  次通道 之分
2  视频流通道 主通道(0):高分辨率图像   次通道(1):低分辨率图像
3  音频流的主通道作为音频编码缓冲区  次通道作为音频解码缓冲区
4 第0通道 作为音频流使用，第1通道作为视频流使用
*/

/*****************************************************************************
 函 数 名  : sdk_stream_init
 功能描述  : 初始化流库，包括定义流路数
 输入参数  : const stream_manage_t *p_stream_manage  
 输出参数  : 无
 返 回 值  : extern
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : bingchuan
    修改内容   : 新生成函数

*****************************************************************************/
extern int sdk_stream_init(const stream_manage_t *p_stream_manage);
extern int sdk_stream_destroy();

//Note:
// ch=1 ----- VIDEO 
// ch=0 ----- AUDIO 
/*****************************************************************************
 函 数 名  : sdk_stream_Open
 功能描述  : 打开预录像通道
 输入参数  : int ch      流通道号 
             int ch_type  0 主码流  1 次码流
             int pre_sec 预录秒数，最大是10秒
 输出参数  : 无
 返 回 值  : 返回流通道句柄
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : bingchuan
    修改内容   : 新生成函数

*****************************************************************************/
extern ST_HDL  sdk_stream_Open_Pre(int ch,int ch_type,int pre_sec);


/*****************************************************************************
 函 数 名  : sdk_stream_Open
 功能描述  : 打开流通道
 输入参数  : int ch      流通道号 
             int ch_type  0 主码流  1 次码流
 输出参数  : 无
 返 回 值  : 返回流通道句柄
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : bingchuan
    修改内容   : 新生成函数

*****************************************************************************/
extern ST_HDL sdk_stream_Open(int ch,int ch_type);//CH=1 表示VIDEO ch=0表示AUDIO 
extern int sdk_stream_Close(ST_HDL  handle);


/*****************************************************************************
 函 数 名  : sdk_stream_ReadOneFrame
 功能描述  : 顺序读取完整一帧数据
 输入参数  : ST_HDL  handle   :流句柄
 输出参数  : 无
 返 回 值  : extern
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : bingchuan
    修改内容   : 新生成函数

*****************************************************************************/
extern char *sdk_stream_ReadOneFrame(ST_HDL  handle);


/*****************************************************************************
 函 数 名  : sdk_stream_ReadNewFrame
 功能描述  : 读取当前流中最新的一帧数据
 输入参数  : ST_HDL  handle  
 输出参数  : 无
 返 回 值  : extern
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : bingchuan
    修改内容   : 新生成函数

*****************************************************************************/
extern char *sdk_stream_ReadNewFrame(ST_HDL  handle);


/*****************************************************************************
 函 数 名  : sdk_stream_WriteFrameCB
 功能描述  : 往流通道写数据的回调函数
 输入参数  : stream_writeFrame_t *writeFrame  
 输出参数  : 无
 返 回 值  : extern
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : bingchuan
    修改内容   : 新生成函数

*****************************************************************************/
extern int sdk_stream_WriteFrameCB(stream_writeFrame_t *writeFrame);



/*****************************************************************************
 函 数 名  : sdk_stream_WriteOneFrame
 功能描述  : 向流库写数据
 输入参数  : int ch   流的通道号        
             int ch_type   0:主码流  1:次码流   
             frame_t * frame  
 输出参数  : 无
 返 回 值  : extern
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年5月22日
    作    者   : bingchuan
    修改内容   : 新生成函数

*****************************************************************************/
extern int sdk_stream_WriteOneFrame(int ch,int ch_type, frame_t * frame);
extern int sdk_stream_WriteOneFrameEx(int ch, int ch_type, frame_head_t *frame_head,char *frame_addr);



/* --------------------- 定长数据内存管理 ---------------------- */

extern POOL_HDL sdk_pool_init(int data_len, int data_num);
extern void sdk_pool_uninit(POOL_HDL handle);
extern int sdk_event_wait(POOL_HDL handle);
extern int sdk_event_broadcast(POOL_HDL handle);
extern int sdk_event_send_to_pool(POOL_HDL handle,void *data);
extern EV_HDL sdk_get_event_handle(POOL_HDL handle);
extern int sdk_get_one_event(POOL_HDL handle,EV_HDL event,void **data);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __STREAMLIB_H__ */
