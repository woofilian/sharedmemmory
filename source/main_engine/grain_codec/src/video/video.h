/******************************************************************************

                  版权所有 (C), 2012-2022, bingchuan

 ******************************************************************************
  文 件 名   : video.h
  版 本 号   : v1.0
  作    者   : 9527
  生成日期   : 2014年7月24日
  功能描述   : video.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年7月24日
    作    者   : 9527
    修改内容   : 创建文件

******************************************************************************/


#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "sdk_struct.h"
#include <stdint.h>


#define AUDIO_BITSTREAM_LEN       12800


#define CAP_PATH_NUM        4
#define ENC_TRACK_NUM       4

#define OSD_PALETTE_COLOR_AQUA              0xCA48CA93        /* YCrYCb */
#define OSD_PALETTE_COLOR_BLACK             0x10801080
#define OSD_PALETTE_COLOR_BLUE              0x296e29f0
#define OSD_PALETTE_COLOR_BROWN             0x51A1515B
#define OSD_PALETTE_COLOR_DODGERBLUE        0x693F69CB
#define OSD_PALETTE_COLOR_GRAY              0xB580B580
#define OSD_PALETTE_COLOR_GREEN             0x5151515B
#define OSD_PALETTE_COLOR_KHAKI             0x72897248
#define OSD_PALETTE_COLOR_LIGHTGREEN        0x90229036
#define OSD_PALETTE_COLOR_MAGENTA           0x6EDE6ECA
#define OSD_PALETTE_COLOR_ORANGE            0x98BC9851
#define OSD_PALETTE_COLOR_PINK              0xA5B3A589
#define OSD_PALETTE_COLOR_RED               0x52F0525A
#define OSD_PALETTE_COLOR_SLATEBLUE         0x3D603DA6
#define OSD_PALETTE_COLOR_WHITE             0xEB80EB80
#define OSD_PALETTE_COLOR_YELLOW            0xD292D210

typedef struct resolution_map_s
{
	uint8_t id;
	uint16_t width;
	uint16_t height;
}resolution_map_s;


/******************************************/
typedef struct video_info_t
{
    void *capture_object;
    void *video_object;
    int  video_fd;
	int  video_type; ///SDK_VIDEO_FORMAT_E
	int  video_width;
	int  video_height;
	int  frame_rate ;
	int  video_buf_len;
	char *video_buf;
} video_info_t;

typedef struct audio_info_s
{
	void *grab_object;
	void *audio_object;
	void *file_object;
	void *render_object;
	int  audio_fd;
	//int  audio_fd2;
	int  audio_buf_len;
	char *audio_buf;

} audio_info_t;

typedef struct av_info_s
{
    void *video_groupfd; //视屏组
    void *audio_groupfd; //音频组 接受
    void *audio_groupfd2; //音频组 播放
    video_info_t  video_info[MAX_VIDEO_STREAM_NUM];
	audio_info_t  audio_info[2];	 // 0 做编码 1 做解码
    int max_video_stream_num;			 // 当前系统系统支持的最大通道数
    int max_audio_stream_num;			 // 当前系统系统支持的最大通道数
    int reserved[5];         ///< Reserved words
} av_info_t;

typedef struct av_sys_info_s
{
    int stream_num ; //最大流数目
    SDK_AUDIO_CODEC_FORMAT_E    audio_format; //音频编码格式
    SDK_VIDEO_STANDARD_E  video_standard;    /* 视频制式 N P */ 
    int max_ch;			 // 系统支持的最大通道数
    int reserved[5];         ///< Reserved words
} av_sys_info_t;

typedef struct tagRGBQUAD
{
    char rgbBlue;
    char rgbGreen;
    char rgbRed;
    char rgbReserved;
} RGBQUAD;

typedef struct _osd_clut_s {
	unsigned char v;
	unsigned char  u;
	unsigned char  y;
	unsigned char  alpha;
} osd_clut_t;





#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern int video_stream_cb();
extern int video_sys_init();
extern int video_sys_uninit();
extern int video_enc_init(int ch,sdk_av_enc_t *pav_enc);
extern int video_enc_uninit(int ch);

extern int  video_server_start();
extern int  video_server_stop();

//抓拍
extern int  video_snap_start();
extern int  video_snap_stop();

//强制I帧
extern int video_force_i_frame(int ch);

//设置编码参数
extern int video_set_enc_attr(int ch,sdk_av_enc_t *pav_enc);

//视频遮挡区域设置
extern int video_set_overlay_attr(int ch);
//移动侦测报警区域设置
extern int video_set_motion_attr(int ch,sdk_vda_codec_cfg_t *motion_cfg);
//感兴趣区域设置
extern int video_set_roi_attr(int ch,sdk_roi_cfg_t *roi_cfg );

//osd 配置
extern int video_set_osd_attr(int ch,sdk_osd_cfg_t *osd_cfg);
extern int video_get_osd_attr(int ch);


//抓拍函数
extern int video_snap_init(int ch,SendstreamCallback snap_callback);
extern int video_snap_unint(int ch);
extern int video_snap_process(int ch,sdk_snap_info_t *snap_info,char *path);//


#if 0
//===========================================================
//sensor 接口
//
//
//
//===========================================================

/*****************************************************************************
 函 数 名  : hi_av_sns_get_type
 功能描述  : 获取sensor类型
 输入参数  : 无
 输出参数  : 无
 返 回 值  : sensor类型
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2012年1月4日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/
int	    hi_av_sns_get_type();

/*****************************************************************************
 函 数 名  : hi_av_sns_get_supp_ima
 功能描述  : 获取sensor支持的图象设置选项
 输入参数  : 无
 输出参数  : 无
 返 回 值  : sensor支持的图象设置选项集合
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2012年1月4日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/
unsigned int  hi_av_sns_get_supp_ima();

/*****************************************************************************
 函 数 名  : hi_av_sns_get_supp_3a
 功能描述  : 获取sensor支持的3a设置选项
 输入参数  : 无
 输出参数  : 无
 返 回 值  : sensor支持的3a设置选项集合
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2012年1月4日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/
unsigned int	hi_av_sns_get_supp_3a();

/*****************************************************************************
 函 数 名  : hi_av_sns_get_ima_size
 功能描述  : 获取sensor采集视频分辨率
 输入参数  : HI_U16 *pU16W  视频采集宽度
             HI_U16 *pU16H  视频采集高度
 输出参数  : 无
 返 回 值  : 成功返回0，失败返回-1
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2012年1月4日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/
int	    hi_av_sns_get_ima_size(int nSnsType, HI_U16 *pU16W, HI_U16 *pU16H);

/*****************************************************************************
 函 数 名  : hi_av_sns_set_ima_size
 功能描述  : 设置sensor采集视频分辨率
 输入参数  : HI_U16 u16W  视频采集宽
             HI_U16 u16H  视频采集高
 输出参数  : 无
 返 回 值  : 成功返回0，失败返回-1
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2012年1月4日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/
int	    hi_av_sns_set_ima_size(int nSnsType, HI_U16 u16W, HI_U16 u16H);

/*****************************************************************************
 函 数 名  : hi_av_sns_get_default_ima
 功能描述  : 获取默认图象属性
 输入参数  : int nPort       视频采集LENS           
             HI_DEV_IMA_CFG_S *pImaCfg  指向通道图象属性结构的指针
 输出参数  : HI_DEV_IMA_CFG_S *pImaCfg  指向通道图象属性结构的指针
 返 回 值  : 成功返回0，失败返回-1
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2012年1月4日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/    
int	    hi_av_sns_get_default_ima(int nPort, int nSnsType, int nMachineType, HI_DEV_IMA_CFG_S *pImaCfg);

/*****************************************************************************
 函 数 名  : hi_av_sns_get_default_3a
 功能描述  : 获取默认3a属性
 输入参数  : int nPort      视频采集LENS             
             HI_DEV_3A_CFG_S *p3ACfg  指向通道3a属性结构的指针
 输出参数  : HI_DEV_3A_CFG_S *p3ACfg  指向通道3a属性结构的指针
 返 回 值  : 成功返回0，失败返回-1
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2012年1月4日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/
int	    hi_av_sns_get_default_3a(int nPort, int nSnsType, int nMachineType, HI_DEV_3A_CFG_S *p3ACfg);

/*****************************************************************************
 函 数 名  : hi_av_sns_check_ima
 功能描述  : 检测图象属性值
 输入参数  : int nPort         视频采集LENS            
             HI_DEV_IMA_CFG_S *pImaCfg   需要检测的属性值
             HI_DEV_IMA_MASK_E eImaMask  需要检测的属性项
 输出参数  : 无
 返 回 值  : 成功返回0，失败返回-1
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2012年1月4日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/
int	    hi_av_sns_check_ima(int nPort, int nSnsType, int nMachineType, HI_DEV_IMA_CFG_S *pImaCfg, HI_DEV_IMA_MASK_E u32Mask);

/*****************************************************************************
 函 数 名  : hi_av_sns_check_3a
 功能描述  : 检测3a属性值
 输入参数  : int nPort      视频采集LENS              
             HI_DEV_3A_CFG_S *p3ACfg   需要检测的属性值
             HI_DEV_3A_MASK_E e3AMask  需要检测的属性项
 输出参数  : 无
 返 回 值  : 成功返回0，失败返回-1
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2012年1月4日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/
int	    hi_av_sns_check_3a(int nPort, int nSnsType, int nMachineType, HI_DEV_3A_CFG_S *p3ACfg, HI_DEV_3A_MASK_E u32Mask);

/*****************************************************************************
 函 数 名  : hi_av_sns_set_ima
 功能描述  : 设置图象属性
 输入参数  : int nPort     视频采集LENS               
             HI_DEV_IMA_CFG_S *pImaCfg  需要设置的属性值
 输出参数  : 无
 返 回 值  : 成功返回0，失败返回-1
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2012年1月4日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/
int     hi_av_sns_set_ima(int nPort, HI_DEV_IMA_CFG_S *pImaCfg);

/*****************************************************************************
 函 数 名  : hi_av_3a_ircut_switch
 功能描述  : 当启动红外模式时，将一些3a参数改为一组红外模式下的特定值。 
 输入参数  : int nDayStatus            
             HI_DEV_3A_CFG_S * p3ACfg  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2012年8月24日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/
int hi_av_3a_ircut_switch(int nDayStatus, int nColor, HI_DEV_3A_CFG_S * p3ACfg);

/*****************************************************************************
 函 数 名  : hi_av_ima_ircut_switch
 功能描述  : 当启动红外模式时，将一些图像参数改为一组红外模式下的特定值。
 输入参数  : int nSensor                   
             int nDayStatus                
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2012年8月21日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/
int hi_av_ima_ircut_switch(int nDayStatus, HI_DEV_IMA_CFG_S * pImaCfg, HI_DEV_3A_CFG_S  * p3ACfg);

/*****************************************************************************
 函 数 名  : hi_av_sns_set_3a
 功能描述  : 设置3a属性
 输入参数  : int nPort     视频采集LENS               
             HI_DEV_3A_CFG_S *p3ACfg  需要设置的属性值
 输出参数  : 无
 返 回 值  : 成功返回0，失败返回-1
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2012年1月4日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/
int     hi_av_sns_set_3a(int nPort, HI_DEV_3A_CFG_S *pImaCfg);

/*****************************************************************************
 函 数 名  : hi_av_sns_get_supp_size
 功能描述  : 获取sensot支持的采集大小
 输入参数  : int nPort      视频采集LENS                  
             int nStreamType    视频码流类型: 主码流，次码流。。。。          
             int nIndex         支持分辨率的序列号          
             HI_SUPP_IMA_SIZE *pSuppSize  分辨率结构体
 输出参数  : HI_SUPP_IMA_SIZE *pSuppSize  分辨率结构体
 返 回 值  : 成功返回0，失败返回-1
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2012年1月4日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/
int	    hi_av_sns_get_supp_size(int nPort, int nStreamType, int nIndex, int nSnsType, HI_SUPP_IMA_SIZE *pImaSize);

/*****************************************************************************
 函 数 名  : hi_av_sns_get_frame_rate
 功能描述  : 获取sensor帧率
 输入参数  : 无
 输出参数  : 无
 返 回 值  : sensor帧率
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2012年1月4日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/
int	    hi_av_sns_get_frame_rate(int nSnsType, int nStand);

/*****************************************************************************
 函 数 名  : hi_av_sns_check_change_lib
 功能描述  : 检查sensor库是否需要重新加载
 输入参数  : int nWdrStatus  
 输出参数  : 无
 返 回 值  : 需要重新加载返回1，不需要重新加载返回0，出错返回-1
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2012年2月13日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/
int     hi_av_sns_check_change_lib(int nSnsType,int nWdrStatus);

/*****************************************************************************
 函 数 名  : hi_av_isp_IrisTrig_Start
 功能描述  : 启动自动光圈校正功能。
 输入参数  :     0:不启动自动光圈校正；非0: 启动自动光圈校正。
 输出参数  : 无
 返 回 值  :    HI_SUCCESS
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2012年3月23日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/
int hi_av_isp_IrisTrig(IrisTrigCb IrisResiveCb);


/*****************************************************************************
 函 数 名  : hi_av_isp_DetectPix_Start
 功能描述  : 启动坏点检测功能。
 输入参数  : DetectPixelCb
 输出参数  : 无
 返 回 值     :   HI_SUCCESS
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2012年3月23日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/
int hi_av_isp_DetectPix(DetectPixelCb DetectPixCb);

/*****************************************************************************
 函 数 名  : hi_av_isp_GetIspStatus
 功能描述  : 获取ISP状态
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2012年7月9日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/
int hi_av_isp_GetIspStatus();

/*****************************************************************************
 函 数 名  : hi_av_set_wdr_type
 功能描述  : 设置sensorWDR值
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2012年11月15日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/
int hi_av_set_wdr_type(HI_S32 s32SnsType, int nWdrValue);

#ifdef HI3518_ARCH
/*****************************************************************************
 函 数 名  : hi_av_get_rotate
 功能描述  : 获取图象旋转属性值
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2012年11月15日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/
int hi_av_get_rotate(int nViCh);

#endif

/*****************************************************************************
 函 数 名  : hi_av_sys_set_size
 功能描述  : 设置编码库支持的主码流的最大编码宽高。
 输入参数  : int nCapW  编码通道的宽
             int nCapH  编码通道的高
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2013年2月26日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/
int hi_av_sys_set_size(int nCapW, int nCapH);

/*****************************************************************************
 函 数 名  : hi_av_sys_get_size
 功能描述  : 获取编码库支持的主码流的最大编码宽高。
 输入参数  : int nCapW  编码通道的宽
             int nCapH  编码通道的高
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2013年2月26日
    作    者   : 周毅
    修改内容   : 新生成函数

*****************************************************************************/
int hi_av_sys_get_size(int *pS32W, int *pS32H);

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __VIDEO_H__ */
