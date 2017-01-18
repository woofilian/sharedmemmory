#ifndef __NETWIFI_H__
#define __NETWIFI_H__

#define	LINE_LEN1	(8+8+36+20+12+20+12)

int init_wifi(int wifi_type);
int get_wifi_cfg();
int wifi_find_list();

//int start_sta(void *param);
int start_sta(sdk_wifi_cfg_t  * wifi_param);

int stop_sta();
int wifi_state();

int ap_rtl_config();

#endif
