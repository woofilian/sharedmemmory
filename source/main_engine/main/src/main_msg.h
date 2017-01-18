/*
 * =====================================================================================
 *
 *       Filename:  main_msg.h
 *
 *    Description:  主控消息处理
 *
 *        Version:  1.0
 *        Created:  2011年07月16日 15时08分48秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  maohw (), maohongwei@gmail.com
 *        Company:  jxj
 *         Modify:  
 *
 * =====================================================================================
 */

#ifndef __main_msg_h__
#define __main_msg_h__

#include "main.h"
#include "adapt.h"

int main_msg_def(sdk_msg_t *pmsg); 
int main_msg_version(sdk_msg_t *pmsg);
int main_msg_status(sdk_msg_t *pmsg);
int main_msg_param_def(sdk_msg_t *pmsg);
int main_msg_upgrad(sdk_msg_t *pmsg);
int main_msg_record_cfg(sdk_msg_t *pmsg);
int main_msg_encode_cfg(sdk_msg_t *pmsg);
int main_msg_chann_cfg(sdk_msg_t *pmsg);
int main_msg_user(sdk_msg_t *pmsg);
int main_msg_log(sdk_msg_t *pmsg);
int main_msg_sys_cfg(sdk_msg_t *pmsg);
int main_msg_platform_cfg(sdk_msg_t *pmsg);
int main_msg_net_server_cfg(sdk_msg_t *pmsg);
int main_msg_ptz_cfg(sdk_msg_t *pmsg);
int main_msg_cruise_cfg(sdk_msg_t *pmsg);
int main_msg_preview_cfg(sdk_msg_t *pmsg);
int main_msg_tour_cfg(sdk_msg_t *pmsg);
int main_msg_vo_binding(sdk_msg_t *pmsg);
int main_msg_motion_cfg(sdk_msg_t *pmsg);
int main_msg_alarm_in_cfg(sdk_msg_t *pmsg);
int main_msg_hide_cfg(sdk_msg_t *pmsg);
int main_msg_lost_cfg(sdk_msg_t *pmsg);
int main_msg_hide_cfg(sdk_msg_t *pmsg);
int main_msg_osd_cfg(sdk_msg_t *pmsg);

int main_msg_comm_cfg(sdk_msg_t *pmsg);
int main_msg_overlay_cfg(sdk_msg_t *pmsg);
int main_msg_disk(sdk_msg_t *pmsg);
int main_msg_systime(sdk_msg_t *pmsg);
int main_msg_serial_cfg(sdk_msg_t *pmsg);
int main_msg_image_attr_cfg(sdk_msg_t *pmsg);
int main_msg_i3a_attr_cfg(sdk_msg_t *pmsg);

int main_msg_record_query(sdk_msg_t *pmsg);
int main_msg_tv_cfg(sdk_msg_t *ptv_cfg);
int main_msg_notify(sdk_msg_t *pmsg);
int main_msg_group_new(sdk_msg_t *pmsg);
int main_msg_group_del(sdk_msg_t *pmsg);
int main_msg_group_ctl(sdk_msg_t *pmsg);
int main_msg_manual_rec(sdk_msg_t *pmsg);
int main_msg_manual_almin(sdk_msg_t *pmsg);
int main_msg_manual_almout(sdk_msg_t *pmsg);
int main_msg_record_backup(sdk_msg_t *pmsg);
int main_msg_close_system(sdk_msg_t *pmsg);
int main_msg_audio_control(sdk_msg_t *pmsg);
int main_msg_snap_control(sdk_msg_t *pmsg);
int main_msg_startup_log();
int main_msg_ddns_cfg(sdk_msg_t *pmsg);
int main_msg_capability(sdk_msg_t *pmsg);
int main_msg_net_link_cfg(sdk_msg_t *pmsg);
int main_msg_upnp_cfg(sdk_msg_t *pmsg);

int main_msg_baidu_info(sdk_msg_t *pmsg);
int main_msg_wifiparam_cfg(sdk_msg_t *pmsg);

int main_msg_devlist(sdk_msg_t *pmsg);
int main_msg_ch_ip(sdk_msg_t *pmsg);

int main_alarm_sched_cfg(sdk_msg_t *pmsg);
int main_regional_cfg(sdk_msg_t *pmsg);
int main_iodev_cfg(sdk_msg_t *pmsg);

int alarm_snap_control(sdk_msg_t *pmsg);

int main_audio_detect_cfg(sdk_msg_t *pmsg);

//new add 
int main_mirror_flip_cfg(sdk_msg_t *pmsg);
int main_power_freq_cfg(sdk_msg_t *pmsg);

int  main_msg_search_wifilist(sdk_msg_t *pmsg);

int main_msg_aws_cfg(sdk_msg_t *pmsg);







#endif //__main_msg_h__

