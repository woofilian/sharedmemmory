#ifndef __adapt_h__
#define __adapt_h__

#include "sdk_global.h"
#include "sdk_struct.h"
#include "codeclib.h"
#include "main.h"




typedef enum _ADAPT_CODEC_DEC_MOD_E{
	  ADAPT_CODEC_DEC_MOD_COMM = 0
	, ADAPT_CODEC_DEC_MOD_PREVIEW
	, ADAPT_CODEC_DEC_MOD_PLAYBACK
	, ADAPT_CODEC_DEC_MOD_TALKBACK

	, ADAPT_CODEC_DEC_MOD_BUTT
}ADAPT_CODEC_DEC_MOD_E;

int adapt_init();
int adapt_deinit();

int adapt_compile_is_grain(void);
int adapt_compile_is_grain(void);


/* __adapt_codec_h__ */
#ifdef _NO_CODEC
    #define sdk_av_sendFrame2Dec(arg...)     (0)
    #define sdk_av_getChMdStat(args...)      (0)
#endif
#define adapt_av_setVoFrameRate  		sdk_av_set_vo_ch_frame_rate
#define adapt_av_resetDecCh      		sdk_av_resetDecCh 
#define adapt_av_queryDecCh      		sdk_av_queryDecCh
//#define adapt_codec_clear_audioDecBuf      	sdk_codec_clearAudioDecChnBuf


int adapt_av_init(EventCallback EventCB,
                  SendstreamCallback	VideoStreamCB,
                  SendstreamCallback	AudioStreamCB,
                  SendstreamCallback	PicStreamCB);

int adapt_av_deinit(void);
int adapt_av_encode_start();
int adapt_av_dec_frame(int _ch, char *frame_buf, ADAPT_CODEC_DEC_MOD_E dec_mod);
int adapt_av_clear_dec(int _ch);

//int adapt_codec_snap(codec_snap_req_t *snap_req);

int adapt_av_encode_cfg(sdk_msg_t *pmsg);
int adapt_av_vo_binding(sdk_msg_t *pmsg, int second_fb);
int adapt_av_osd_cfg(sdk_msg_t *pmsg);
int adapt_record_osd(int flag);
int  adapt_set_time_zone(int time_zone);



int adapt_av_vda_cfg(sdk_msg_t *pmsg);
int adapt_av_md_status(uint32_t *status);
int adapt_audio_detect_status( sdk_msg_t *pmsg);




int adapt_baidu_upload_file (const char *path,const char * file_name,const char *target_path);
int adapt_codec_encode_cfg(sdk_msg_t *pmsg);
int adapt_param_set_ip_info(sdk_ip_info_t *ip_info, int idx);
int adapt_detect_pir_alarm();



#if 0
int adapt_av_overlay_cfg(sdk_msg_t *pmsg);
int adapt_av_set_sidesize(sdk_msg_t *pmsg);
int adapt_av_set_image_attr(sdk_msg_t *pmsg);
int adapt_av_set_3a_attr(sdk_msg_t *pmsg);

int adapt_av_get_def_3a_attr(int ch ,sdk_3a_attr_t *p3a_attr);
int adapt_av_get_def_image_attr(int ch ,sdk_image_attr_t *pimage_attr);


#endif



/* __adapt_codec_h__ */

#if 0
/* __adapt_param_h__ */
int adapt_param_init();
int adapt_param_deinit(void);
int adapt_param_init_default_param(void);
int adapt_param_default2current(sdk_default_param_t *default_param);
int adapt_param_recovery_param(sdk_default_param_t *p_default, uint32_t ch_def);
int adapt_param_get_chnum();
int adapt_param_get_ana_chan_num();

int adapt_param_get_sys_cfg(sdk_sys_cfg_t *sys_cfg);
int adapt_param_set_sys_cfg(sdk_sys_cfg_t *sys_cfg);

int adapt_param_get_user_array(sdk_user_right_t *user_array, int size);
int adapt_param_set_user_array(sdk_user_right_t *user_array, int size);

int adapt_param_get_net_mng_cfg(sdk_net_mng_cfg_t *net_mng);
int adapt_param_set_net_mng_cfg(sdk_net_mng_cfg_t *net_mng);

int adapt_param_get_preview_cfg(int ch, sdk_preview_t *preview_cfg);
int adapt_param_set_preview_cfg(int ch, sdk_preview_t *preview_cfg);

int adapt_param_get_tour_cfg(int ch, sdk_tour_t *tour);
int adapt_param_set_tour_cfg(int ch, sdk_tour_t *tour);

int adapt_param_get_chann_cfg(int ch, sdk_channel_t *chann_cfg);
int adapt_param_set_chann_cfg(int ch, sdk_channel_t *chann_cfg);

int adapt_param_get_ptz_cfg(int ch, sdk_ptz_param_t *ptz_cfg);
int adapt_param_set_ptz_cfg(int ch, sdk_ptz_param_t *ptz_cfg);


int adapt_param_get_osd_cfg(int ch, sdk_osd_cfg_t *osd_cfg);
int adapt_param_set_osd_cfg(int ch, sdk_osd_cfg_t *osd_cfg);


int adapt_param_get_alarm_in_cfg(int ch, sdk_alarm_in_cfg_t *alarm_in_cfg); 
int adapt_param_set_alarm_in_cfg(int ch, sdk_alarm_in_cfg_t *alarm_in_cfg); 

int adapt_param_get_motion_cfg(int ch, sdk_motion_cfg_v2_t *motion_cfg); 
int adapt_param_set_motion_cfg(int ch, sdk_motion_cfg_v2_t *motion_cfg); 

int adapt_param_get_lost_cfg(int ch, sdk_lost_cfg_t *lost_cfg); 
int adapt_param_set_lost_cfg(int ch, sdk_lost_cfg_t *lost_cfg); 

int adapt_param_get_hide_cfg(int ch, sdk_hide_cfg_t *hide_cfg); 
int adapt_param_set_hide_cfg(int ch, sdk_hide_cfg_t *hide_cfg); 

int adapt_param_get_record_cfg(int ch, sdk_record_cfg_t *record_cfg); 
int adapt_param_set_record_cfg(int ch, sdk_record_cfg_t *record_cfg);

int adapt_param_get_encode_cfg(int ch, sdk_encode_t *encode);
int adapt_param_set_encode_cfg(int ch, sdk_encode_t *encode);

int adapt_param_get_comm_cfg(sdk_comm_cfg_t *comm_cfg);
int adapt_param_set_comm_cfg(sdk_comm_cfg_t *comm_cfg);

int adapt_param_get_overlay_cfg(int ch, sdk_overlay_cfg_t *poverlay_cfg);
int adapt_param_set_overlay_cfg(int ch, sdk_overlay_cfg_t *poverlay_cfg);

int adapt_param_get_def_cfg(int ch, int module_id, void *cfg);
int adapt_param_set_def_cfg(int ch, int module_id, void *cfg);

int adapt_param_get_serial_cfg(int ch, sdk_serial_func_cfg_t *pserial_cfg);
int adapt_param_set_serial_cfg(int ch, sdk_serial_func_cfg_t *pserial_cfg);

int adapt_param_get_image_attr(int ch, sdk_image_attr_t *pimage_cfg);
int adapt_param_set_image_attr(int ch, sdk_image_attr_t *pimage_cfg);

int adapt_param_set_3a_attr(int ch, sdk_3a_attr_t * p3a_cfg);
int	adapt_param_get_3a_attr(int ch, sdk_3a_attr_t * p3a_cfg);

int adapt_param_get_tv_cfg(sdk_vo_sideSize_t *ptv_cfg);
int adapt_param_set_tv_cfg(sdk_vo_sideSize_t *ptv_cfg);

int adapt_param_get_def_sys_cfg(sdk_sys_cfg_t *sys_cfg);
int adapt_param_set_def_sys_cfg(sdk_sys_cfg_t *sys_cfg);
int adapt_param_get_def_encode_cfg(int ch, sdk_encode_t *cfg);
int adapt_param_get_def_ptz_cfg(int ch, sdk_ptz_param_t *cfg);
int adapt_param_get_def_record_cfg(int ch, sdk_record_cfg_t *cfg);
int adapt_param_get_def_net_mng_cfg(sdk_net_mng_cfg_t *cfg);
int adapt_param_set_def_net_mng_cfg(sdk_net_mng_cfg_t *cfg);
int adapt_param_get_def_channel_cfg(int ch, sdk_channel_t *cfg);
int adapt_param_get_def_image_cfg(int ch, sdk_image_attr_t *cfg);
int adapt_param_get_def_serial_cfg(int ch, sdk_serial_func_cfg_t *cfg);
int adapt_param_get_def_overlay_cfg(int ch, sdk_overlay_cfg_t *cfg);
int adapt_param_get_def_motion_cfg(int ch, sdk_motion_cfg_v2_t *cfg);
int adapt_param_get_def_comm_cfg(int ch, sdk_comm_cfg_t *cfg);
int adapt_param_get_def_osd_cfg(int ch, sdk_osd_cfg_t *cfg);
int adapt_param_get_def_alarm_in_cfg(int ch, sdk_alarm_in_cfg_t *cfg);
int adapt_param_get_def_hide_cfg(int ch, sdk_hide_cfg_t *cfg);
int adapt_param_get_def_lost_cfg(int ch, sdk_lost_cfg_t *cfg);
int adapt_param_get_def_tv_cfg(int ch, sdk_vo_sideSize_t *cfg);
int adapt_param_get_def_preview_cfg(int ch, sdk_preview_t *cfg);
int adapt_param_get_def_tour_cfg(int ch, sdk_tour_t *cfg);

int adapt_param_get_manual_rec_cfg(sdk_manual_record_t *prec_cfg);
int adapt_param_set_manual_rec_cfg(sdk_manual_record_t *prec_cfg);
int adapt_param_get_manual_almin_cfg(sdk_manual_alarmin_t *palmin_cfg);
int adapt_param_set_manual_almin_cfg(sdk_manual_alarmin_t *palmin_cfg);
int adapt_param_get_manual_almout_cfg(sdk_manual_alarmout_t *palmout_cfg);
int adapt_param_set_manual_almout_cfg(sdk_manual_alarmout_t *palmout_cfg);

int adapt_os_set_net_mng_cfg(sdk_net_mng_cfg_t *pnet_mng);
int adapt_os_get_net_mng_cfg(sdk_net_mng_cfg_t *pnet_mng);

char* adapt_os_get_if_name(SDK_NET_IDX_E idx, char *if_name);

int adapt_os_get_ip_info(sdk_ip_info_t *ip_info, int idx);
int adapt_os_set_ip_info(sdk_ip_info_t *ip_info, int idx);

int adapt_os_get_eth_cfg(sdk_eth_cfg_t *peth, int idx);
int adapt_os_set_eth_cfg(sdk_eth_cfg_t *peth, int idx);

int adapt_param_version_def(char *ver, int ver_len, char *date, int date_len);
int adapt_param_set_platform_cfg(sdk_platform_t * platform_cfg);
int adapt_param_get_platform_cfg(sdk_platform_t * platform_cfg);
int adapt_param_get_ddns_cfg(sdk_ddns_cfg_t *pddns_cfg);
int adapt_param_set_ddns_cfg(sdk_ddns_cfg_t *pddns_cfg);


int adapt_param_get_ip_info(sdk_ip_info_t *ip_info, int idx);
int adapt_param_set_ip_info(sdk_ip_info_t *ip_info, int idx);


int adapt_param_get_eth_cfg(sdk_eth_cfg_t *eth_cfg, int idx);
int adapt_param_set_eth_cfg(sdk_eth_cfg_t *eth_cfg, int idx);

int adapt_param_get_pppoe_cfg(sdk_pppoe_t *ppppoe_cfg, int idx);
int adapt_param_set_pppoe_cfg(sdk_pppoe_t *ppppoe_cfg, int idx);

int adapt_param_get_wifi_cfg(sdk_wifi_cfg_t *wifi_cfg, int idx);
int adapt_param_set_wifi_cfg(sdk_wifi_cfg_t *wifi_cfg, int idx);

int adapt_param_get_3g_cfg(sdk_3g_cfg_t *_3g_cfg, int idx);
int adapt_param_set_3g_cfg(sdk_3g_cfg_t *_3g_cfg, int idx);

int adapt_param_get_upnp_cfg(sdk_upnp_t *upnp_cfg);
int adapt_param_set_upnp_cfg(sdk_upnp_t *upnp_cfg);

int adapt_param_get_screen_cfg(int ch, sdk_screen_t *screen_cfg);
int adapt_param_set_screen_cfg(int ch, sdk_screen_t *screen_cfg);

int adapt_os_kill_dhcp();
int adapt_os_start_dhcp(char *p_net_name);

int adapt_param_dev_is_dvr(int dev_type);
int adapt_param_dev_is_nvr(int dev_type);
int adapt_param_dev_is_dec(int dev_type);
int adapt_param_dev_is_dec_card(int dev_type);
int adapt_param_dev_is_dec_box(int dev_type);
int adapt_param_dev_is_3520(int dev_type);
int adapt_param_dev_is_3521(int dev_type);
int adapt_param_dev_is_3531(int dev_type);

/* __adapt_param_h__ */


/* __adapt_periph_h__ */
int adapt_periph_init(void);
int adapt_periph_deinit(void);

typedef struct rtc_time_s
{   
    int year;
    int mon;
    int day;
    int hour;
    int min;
    int sec;
    int wday;
}rtc_time_t;
int adapt_periph_rtc_set(rtc_time_t *rtc_time);
int adapt_periph_rtc_get(rtc_time_t *rtc_time);

int adapt_periph_alarm_in(unsigned int *status);
int adapt_periph_alarm_out(int alarm_out_no, unsigned int value);
int adapt_periph_alarm_out_once(int alarm_out_no, unsigned int value, unsigned int hold_sec);
int adapt_periph_alarm_out_manual(int alarm_out_no, unsigned int value);

int adapt_periph_run_led(unsigned int value);
int adapt_periph_alarm_led(unsigned int value);

int adapt_periph_buzzer(unsigned int value);
int adapt_periph_buzzer_once(unsigned int value, unsigned int hold_sec);

int adapt_periph_ptz(int ch, unsigned int value);

int adapt_periph_usb1_power_control(unsigned int value);
int adapt_periph_usb1_power_status();
int adapt_periph_usb2_power_control(unsigned int value);
int adapt_periph_usb2_power_status();

int adapt_periph_restore_button_status();
/* __adapt_periph_h__ */


/* __adapt_record_h__ */
int adapt_record_init(int fd, alignstream_t *p_align_mem);

int adapt_record_start(int ch, int type);
int adapt_record_stop(int ch, int type);
int adapt_record_delay_stop(int ch, int type, unsigned int delay);

int adapt_led_on(int ch, int type);
int adapt_led_off(int ch, int type);
int adapt_led_delay_off(int ch, int type, unsigned int delay);

int adapt_record_start_msg_send(int ch, int type);
int adapt_record_stop_msg_send(int ch, int type);
int adapt_record_delay_stop_msg_send(int ch, int type, unsigned int delay);

int adapt_record_start_msg_send2pool(int ch, int type);
int adapt_record_stop_msg_send2pool(int ch, int type);
int adapt_record_delay_stop_msg_send2pool(int ch, int type, unsigned int delay);

int adapt_alarmin_start_msg_send2pool(int ch, int type);
int adapt_alarmin_stop_msg_send2pool(int ch, int type);
int adapt_alarmin_delay_stop_msg_send2pool(int ch, int type, unsigned int delay);

int adapt_md_start_msg_send2pool(int ch, int type);
int adapt_md_stop_msg_send2pool(int ch, int type);
int adapt_md_delay_stop_msg_send2pool(int ch, int type, unsigned int delay);

int adapt_lost_start_msg_send2pool(int ch, int type);
int adapt_lost_stop_msg_send2pool(int ch, int type);
int adapt_lost_delay_stop_msg_send2pool(int ch, int type, unsigned int delay);

int adapt_record_query(sdk_msg_t *msg);
int adapt_snap_query(sdk_msg_t *msg);
//time_t make_time(sdk_time_t *sdk_time);
//void local_time(time_t time, sdk_time_t *sdk_time);
int adapt_disk_query(sdk_msg_t *msg);
int adapt_disk_format(sdk_msg_t *pmsg);
int adapt_disk_fm_progress(int _disk_no);
int adapt_set_fm_progress(int val);
int adapt_backup_disk_search(sdk_msg_t *msg);
int adapt_backup_start(sdk_msg_t *pmsg);
int adapt_backup_progress();
int adapt_record_query_ref_count(int disk_no);
int adapt_record_query_disk_state();
int adapt_record_active_disk();
int adapt_disk_umount(sdk_msg_t *pmsg);
int adapt_record_disk_search();
int adapt_disk_query_check_disk_is_full();
int adapt_record_get_usb_mount_path(char *p_buf_path, int buf_len);

/* __adapt_record_h__ */

#endif

#endif /*__adapt_h__*/
