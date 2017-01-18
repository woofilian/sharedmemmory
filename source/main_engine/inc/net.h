#ifndef __NET_H__
#define __NET_H__
#define BYTE 	unsigned char

typedef struct _netinfo 
{
	int conn_mod;	/*0-STATIC  1-DHCP */
	char ipaddr[16];
	char netmask[16];
	char gateway[16];
	char dns[2][16];
} NETINFO;

//wifi设置数据
#define SSID_LEN          37
#define WIFIPASS_LEN  50

//wifi状态
enum wifi_stat_type
{
	WIFI_STAT_CLOSE=0,  /*关闭wifi*/
	WIFI_STAT_OPEN=1    /*启用wifi*/
};

//wifi网络类型
enum wifi_net_type
{
	WIFI_TYPE_INFRA=0,
	WIFI_TYPE_ADHOC=1
};

//wifi安全模式
enum wifi_enc_type
{
	WIFI_ENC_NONE=0,
	WIFI_ENC_WEP,
	WIFI_ENC_TKIP,
	WIFI_ENC_AES
};

//wifi验证模式
enum wifi_auth_mode
{
	WIFI_AUTH_OPEN=0,
	WIFI_AUTH_SHARED,
	WIFI_AUTH_WPAPSK,
	WIFI_AUTH_WPA2PSK
};

//wifi配置
typedef struct tag_remote_wificfg
{
	BYTE  netstat;   /*状态0:关闭1:启用*/
	
	BYTE  nettype;  /*网络类型0:Infra 1:Adhoc  */         

	BYTE  enctype; /*安全模式0:NONE 1:WEP 2:TKIP 3:AES */
	
	BYTE  authmode;  /*验证模式0:OPEN 1:SHARED 2:WPAPSK 3:WPA2PSK */        

	BYTE  defkeyid;  /*默认密钥ID号(1,2,3,4) */

	char   ssid[SSID_LEN];    /*ssid */

	char   pass[WIFIPASS_LEN];   /*密码*/
	
}REMOTE_WIFICFG;

///////////////搜索----begin///////////
//wifi信息
typedef struct tag_remote_wifi_info
{
	BYTE  nettype;  /*网络类型0:Infra 1:Adhoc  */         

	BYTE  enctype; /*安全模式0:NONE 1:WEP 2:TKIP 3:AES */
	
	BYTE  authmode;  /*验证模式0:OPEN 1:SHARED 2:WPAPSK 3:WPA2PSK */        

	char   ssid[SSID_LEN];    /*ssid */

}REMOTE_WIFI_INFO;

//wifi搜索
#define WIFI_FIND_LEN 30
typedef struct tag_remote_wifi_find
{
	int   count;
	REMOTE_WIFI_INFO  wifi_info[WIFI_FIND_LEN];	
}REMOTE_WIFI_FIND;

//wifi信息
typedef struct tag_wifi_info_com
{
	BYTE  nettype;  /*网络类型0:Infra 1:Adhoc  */         

	BYTE  enctype; /*安全模式0:NONE 1:WEP 2:TKIP 3:AES */
	
	BYTE  authmode;  /*验证模式0:OPEN 1:SHARED 2:WPAPSK 3:WPA2PSK */
	BYTE  signal;   // signal intensity 0--100%

	char   ssid[SSID_LEN];    /*ssid */

}WIFI_INFO_COM;
typedef struct tag_wifi_find_com
{
	int   count;
	WIFI_INFO_COM  wifi_info[WIFI_FIND_LEN];	
}WIFI_FIND_COM;



/*****************************************************************************
 函 数 名  : init_net_server
 功能描述  : 初始化网络库
 输入参数  : 无
 输出参数  : 无
 返 回 值  :  -1:faile  0:sucess
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月18日
    作    者   : video_device.h
    修改内容   : 新生成函数

*****************************************************************************/
int sdk_init_net_server();


/*****************************************************************************
 函 数 名  : start_net_server
 功能描述  : 启动网络服务功能
 输入参数  : 无
 输出参数  : 无
 返 回 值  : -1:faile  0:sucess
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月18日
    作    者   : video_device.h
    修改内容   : 新生成函数

*****************************************************************************/
int sdk_start_net_server();
int sdk_stop_net_server();

/**********wireless *************/
/*****************************************************************************
 函 数 名  : set_netwifi_params
 功能描述  : 设置wifi参数
 输入参数  : REMOTE_WIFICFG *wifi_vars  
 输出参数  : 无
 返 回 值  : -1:fail, 0:succ
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月18日
    作    者   : video_device.h
    修改内容   : 新生成函数

*****************************************************************************/

int   sdk_set_netwifi_params(REMOTE_WIFICFG *wifi_vars);

/*****************************************************************************
 函 数 名  : set_netwifi_params_notsavecfg
 功能描述  : 获取wifi参数
 输入参数  : REMOTE_WIFICFG *wifi_vars  
 输出参数  : 无
 返 回 值  : -1:fail, 0:succ
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月18日
    作    者   : video_device.h
    修改内容   : 新生成函数

*****************************************************************************/
int sdk_get_netwifi_params(REMOTE_WIFICFG *wifi_vars);


/*****************************************************************************
 函 数 名  : search_netwifi_com
 功能描述  : 搜索无线路由器0
 输入参数  : WIFI_FIND_COM *wifi_find  
 输出参数  : 无
 返 回 值  : 0:succ , -1:fail
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月18日
    作    者   : video_device.h
    修改内容   : 新生成函数

*****************************************************************************/
int  sdk_search_netwifi_com(WIFI_FIND_COM *wifi_find);

//wired net
/*****************************************************************************
 函 数 名  : get_net_params
 功能描述  : 设置有线网络参数
 输入参数  : NETINFO *net_param  
 输出参数  : 无
 返 回 值  :  -1:faile  0:sucess
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月18日
    作    者   : 9527
    修改内容   : 新生成函数

*****************************************************************************/

int  sdk_set_net_params(NETINFO *net_param);


/*****************************************************************************
 函 数 名  : get_net_params
 功能描述  : 获取有线网络参数
 输入参数  : NETINFO *net_param  
 输出参数  : 无
 返 回 值  : -1:faile  0:sucess

 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2014年6月18日
    作    者   : 9527
    修改内容   : 新生成函数

*****************************************************************************/
int sdk_get_net_params(NETINFO *net_param);




#endif

