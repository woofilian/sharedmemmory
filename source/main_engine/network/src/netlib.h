/******************************************************************************

                  版权所有 (C), 2012-2022, bingchuan

 ******************************************************************************
  文 件 名   : net.h
  版 本 号   : v1.0
  作    者   : 9527
  生成日期   : 2014年10月27日
  功能描述   : net.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2014年10月27日
    作    者   : 9527
    修改内容   : 创建文件

******************************************************************************/

 /*******************包含头文件******************/

 /*******************外部变量说明****************/

 /*****************外部函数原型说明**************/

 /*****************内部函数原型说明**************/

 /********************全局变量*******************/

 /*******************常量定义********************/

 /*******************宏定义**********************/

 /*******************函数实现********************/

#ifndef __NET_H__
#define __NET_H__

#include "sdk_struct.h"
#include "adapt.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef int (*NetStatusCB)(void *data); //音视频数据回调

extern int  sdk_init_net_server(sdk_eth_cfg_t *net_param,sdk_wifi_cfg_t *wifi_param);
extern int  sdk_get_net_params(sdk_eth_cfg_t *net_param);
extern int  sdk_search_ap(sdk_wifi_search_t *ap_list);
extern int sdk_set_mac_params(const char *if_name,const char *mac);
extern int  sdk_set_net_params(sdk_eth_cfg_t *net_param);
extern int sdk_set_wifi_params(sdk_wifi_cfg_t *wifi_cfg);

extern int sdk_start_net_server(NetStatusCB networkCB/*,char *ssid*/);

extern int sdk_stop_net_server();

extern int check_iwlist_scanning(char *inSsid, char *inKey, WiFi_Item_Config *poutItemInfo);
extern int check_connect_handle(WiFi_Item_Config *pItemApInfo);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __NET_H__ */

