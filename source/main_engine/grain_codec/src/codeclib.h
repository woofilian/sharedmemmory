/******************************************************************************

                  版权所有 (C), 2012-2022, bingchuan

 ******************************************************************************
  文 件 名   : codeclib.h
  版 本 号   : v1.0
  作    者   : 9527
  生成日期   : 2014年9月15日
  功能描述   : codeclib.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年9月15日
    作    者   : 9527
    修改内容   : 创建文件

******************************************************************************/


#ifndef __CODECLIB_H__
#define __CODECLIB_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
extern int sdk_audio_play(int audio_type,const char *audio_data,int date_len);

extern int sdk_av_get_audio_info(int audio_ch,sdk_a_frame_info_t	*dudio_info);
extern int  sdk_av_get_sys_capability();
extern int sdk_av_get_video_info(int video_ch,sdk_v_frame_info_t *video_info);
extern int sdk_av_init_sys_callback(EventCallback EventCB,
                                      SendstreamCallback	VideoStreamCB,
                                      SendstreamCallback	AudioStreamCB,
                                      SendstreamCallback	PicStreamCB);







extern int sdk_av_request_i_frame(int ch);
extern int sdk_av_server_start();
extern int sdk_av_server_stop();
extern int sdk_av_set_3di_param(int ch,sdk_3di_cfg_t * p3di_cfg);
extern int sdk_av_set_encode_param(int ch,sdk_av_enc_t *pav_enc);
extern int sdk_av_set_motion_param(int ch,sdk_vda_codec_cfg_t *motion_cfg);
extern int sdk_audio_set_detect_param(sdk_audio_detect_cfg_t *audio_detect_cfg);
extern int sdk_audio_get_detect_param(sdk_audio_detect_cfg_t *audio_detect_cfg);

extern int sdk_av_set_osd_param(int ch,sdk_osd_cfg_t *osd_cfg);

extern int sdk_av_set_roi_param(int ch,sdk_roi_cfg_t *roi_cfg);
extern int sdk_av_snap(int ch,sdk_snap_info_t *snap_info,char *path);
extern int sdk_av_sys_init(int sensor_type,int total_ch,sdk_encode_t *pencode_info);
extern int sdk_av_sys_uninit();


extern int sdk_av_set_attr_param(int ch,sdk_image_attr_t *attr_cfg);
extern int sdk_av_set_record_osd(int flag);
extern int sdk_set_time_zone(int time_zone);

extern int sdk_av_mirror_param(int ch,sdk_mirror_flip_cfg_t *ongoing,sdk_mirror_flip_cfg_t *desired);
extern int ov9715_init();
//add by jensen about hw_gpio
extern int init_gpio(); //仅初始化一次,上电已完成初始化，后面直接配置相应pin  dir即可
extern int gpioEx_defpin(int num, int dir); //set dir about gpio pin
extern void gpioEx_set(int num); //set 1 
extern void gpioEx_clear(int num);//set 0
extern int gpioEx_read(int num);//read gpio pin value



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CODECLIB_H__ */

