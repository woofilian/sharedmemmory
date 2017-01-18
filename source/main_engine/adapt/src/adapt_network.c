/*
 * =====================================================================================
 *
 *       Filename:  adapt_av.c
 *
 *    Description:  编解码适配层
 *
 *        Version:  1.0
 *        Created:  2014年06月01日 00时15分05秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  
 *        Company:
 *
 * =====================================================================================
 */
#if 1

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdk_struct.h"
#include "netlib.h"
#include "log.h"


#define FILE_NAME "/mnt/sd_card/wifi_config.txt"



typedef struct _WIFI_CONFIG_
{	
	unsigned char status;	
	unsigned char authmode;	
	unsigned char encryptype;	
	unsigned char ssid[32];	
	unsigned char pwd[32];
	unsigned char secrity[32];
	unsigned char dhcp;
	unsigned char ipaddr[32];	
	unsigned char netmask[32];	
	unsigned char gateway[32];
}WIFI_CONFIG;

/*parse_config  解析SD卡的WIFI配置文件RETURN 1 没有配置文件*/
int parse_config(WIFI_CONFIG *config)
{	
	FILE *fp;	
	unsigned char *p = NULL;	
	char *p_save = NULL;	
	char *p_str = NULL;	
	int ret;	
	unsigned int len;	
	unsigned char buf[256];
	memset(buf,0,sizeof(buf));	

	fp = fopen(FILE_NAME,"r");	
	if(NULL == fp)	
	{		
		config->status = 0;	
		printf("no wifi config file: %s\n", FILE_NAME);	
		return 1;	
	}	
	len = fread(buf,1,256,fp);
	fclose(fp);	
	p = strtok_r(buf,"\r\n",&p_save);	
	//printf("p=%s\n",p);	
	while(NULL != (p = strtok_r(NULL,"\r\n",&p_save)))	
	{		
		//printf("p1=%s\n",p);		
		if((0 == strncmp(p,"//",2)) || (0 == strncmp(p,"/*",2)))	
		{			
			//printf("11111111\n");		
			p = NULL;			
			continue;		
		}		
		else		
		{		
			if(NULL != (p_str=strstr(p,"AuthMode=")))	
			{				
				p_str +=strlen("AuthMode=");		
				//printf("%s\n",p_str);			
				if(0 == strcmp(p_str,"OPEN"))		
				{					
					config->authmode = 0;		
				}				
				else if(0 == strcmp(p_str,"SHARED"))	
				{			
					config->authmode = 1;	
				}		
				else if(0 == strcmp(p_str,"WPAPSK"))	
				{			
					config->authmode = 2;	
				}				
				else if(0 == strcmp(p_str,"WPA2PSK"))	
				{							
					config->authmode = 3;			
				}		
			}		
			else if(NULL != (p_str=strstr(p,"EncrypType=")))
			{			
				p_str +=strlen("EncrypType=");					
				if(0 == strcmp(p_str,"NONE"))		
				{							
					config->encryptype = 0;		
				}				
				else if(0 == strcmp(p_str,"WEP"))	
				{							
					config->encryptype = 1;			
				}				
				else if(0 == strcmp(p_str,"TKIP"))
				{						
					config->encryptype = 2;		
				}				
				else if(0 == strcmp(p_str,"AES"))	
				{							
					config->encryptype = 3;		
				}		
			}			
			else if(NULL != (p_str=strstr(p,"SSID=")))	
			{				
				p_str +=strlen("SSID=");	
				strcpy(config->ssid,p_str);
			}			
			else if(NULL != (p_str=strstr(p,"PWD=")))		
			{				
				p_str +=strlen("PWD=");			
				strcpy(config->pwd,p_str);	
			}		
			else if(NULL != (p_str=strstr(p,"DHCP=")))	
			{			
				p_str +=strlen("DHCP=");		
				if((0==strcmp(p_str,"YES"))||(0==strcmp(p_str,"yes")))	
				{					
					config->dhcp = 1;			
				}				
				else if((0==strcmp(p_str,"NO"))||(0==strcmp(p_str,"no")))	
				{					
					config->dhcp = 0;		
				}		
			}			
			else if(NULL != (p_str=strstr(p,"IPADDR=")))	
			{			
				p_str +=strlen("IPADDR=");				
				strcpy(config->ipaddr,p_str);	
			}			
			else if(NULL != (p_str=strstr(p,"NETMASK=")))		
			{			
				p_str +=strlen("NETMASK=");				
				strcpy(config->netmask,p_str);	
			}			
			else if(NULL != (p_str=strstr(p,"GATEWAY=")))	
			{				
				p_str +=strlen("GATEWAY=");				
				strcpy(config->gateway,p_str);	
			}				
			else if(NULL != (p_str=strstr(p,"</WIFI CONFIG>")))	
			{	
				config->status = 1;
				break;			
			}	
		}	
	}	
	return 0;
}


int sd_parse_config(WiFi_Item_Config *wifi_config)
{
	FILE *fp = NULL;
	char buffer[256] = {0};
	char myssid[32] = {0};
	char mypwd[32] = {0};
	char *pstr;
	
	if(access(FILE_NAME, F_OK)!=0)
	{
		wifi_config->status = 0;
		printf("no wifi config file : %s\n", FILE_NAME);
		return 1;
	}

	fp = fopen(FILE_NAME, "r");
	if(fp != NULL)
	{
		memset(buffer, 0, sizeof(buffer));
		while(fgets(buffer, sizeof(buffer), fp) != NULL)
		{
			if((pstr=strstr(buffer, "SSID")) != NULL)
			{
				pstr += strlen("SSID=");
				snprintf(myssid, sizeof(myssid), "%s", pstr);
			}
			if((pstr=strstr(buffer, "PASSWORD")) != NULL)
			{
				pstr += strlen("PASSWORD=");
				snprintf(mypwd, sizeof(mypwd), "%s", pstr);
			}
		}
		fclose(fp);
	}

	check_iwlist_scanning(myssid, mypwd, wifi_config);

    return 0;
}


int adapt_network_init()
{
	int ret;
	WiFi_Item_Config wifi_ItemConf;

	memset(&wifi_ItemConf, 0, sizeof(wifi_ItemConf));
	ret = sd_parse_config(&wifi_ItemConf);
	if(ret == 0 && wifi_ItemConf.status == 1)
	{
		check_connect_handle(&wifi_ItemConf);
	}
	else
	{
		sdk_network_server_start();
	}
}


///	sdk_init_net_server( net_param,wifi_param);
///	sdk_start_net_server(network_status_callback);
//在正常情况下，第一次启动 需要sta(搜索热点)->ap(和设备创建关联)->sta(连接手机热点)
int adapt_init_net()
{
	int ret;
	char buf[1024]={0};
	WIFI_CONFIG wifi_sd_cfg;
	sdk_eth_cfg_t net_param;
	sdk_wifi_cfg_t wifi_param;
	sdk_wifi_search_t ap_list;
	
	memset(&wifi_sd_cfg, 0, sizeof(wifi_sd_cfg));
	memset(&net_param, 0, sizeof(sdk_eth_cfg_t));
	memset(&wifi_param, 0, sizeof(sdk_wifi_cfg_t));
	memset(&ap_list, 0, sizeof(ap_list));
	
	printf("start parse!!!!!!!!!!!!!!\n");
	//ret = parse_config(&wifi_sd_cfg);
	ret = sd_parse_config(&wifi_sd_cfg);
	if((0 == ret ) && (1 == wifi_sd_cfg.status ))
	{
		/*printf("\nparse:\n");	
		printf("AuthMode = %d \nEncrypType=%d \nssid=%s \npwd=%s \ndhcp=%d \nip=%s \nnetmask=%s \ngateway=%s\n",		
			wifi_sd_cfg.authmode,
			wifi_sd_cfg.encryptype,
			wifi_sd_cfg.ssid,
			wifi_sd_cfg.pwd,
			wifi_sd_cfg.dhcp,
			wifi_sd_cfg.ipaddr,
			wifi_sd_cfg.netmask,
			wifi_sd_cfg.gateway);*/
		printf("\nparse:\n");	
		printf("SSID = %s \nPWD=%s \nSecrity=%s\n",		
			wifi_sd_cfg.ssid,
			wifi_sd_cfg.pwd,
			wifi_sd_cfg.secrity);
		
		strcpy(wifi_param.ssid, wifi_sd_cfg.ssid);
		strcpy(wifi_param.pwd, wifi_sd_cfg.pwd);
		strcpy(wifi_param.secrity, wifi_sd_cfg.secrity);
		//wifi_param.encrypt_type = wifi_sd_cfg.authmode;
		//wifi_param.secret_key_type = wifi_sd_cfg.encryptype;
		net_param.if_mode = SDK_NET_MODE_WIFI;
		//net_param.wifi_type = 1;
		
		strcpy(net_param.ip_info.if_name, "wlan0");
        //strcpy(net_param.ip_info.if_name, "ra0");
		
		net_param.ip_info.enable_dhcp = 1;//wifi_sd_cfg.dhcp;
		if(0 == wifi_sd_cfg.dhcp)
		{
			strcpy(net_param.ip_info.ip_addr, wifi_sd_cfg.ipaddr);
			strcpy(net_param.ip_info.mask, wifi_sd_cfg.netmask);
			strcpy(net_param.ip_info.gateway, wifi_sd_cfg.gateway);
		}
	}
	else
	{
		//从参数库取数据
		adapt_param_get_eth_cfg(&net_param);
		printf(" \n +_+_+_+_+_+_+_+_+_+_  net_param.ip_info.mac %s   \n ",net_param.ip_info.mac);
		printf(" \n +_+_+_+_+_+_+_+_+_+_  net_param.ip_info.ip_addr  %s  \n ",net_param.ip_info.ip_addr);
		printf(" \n +_+_+_+_+_+_+_+_+_+_  net_param.ip_info.if_mode  %d  \n ",net_param.if_mode);
		adapt_get_wifi_params(&wifi_param);
	}
	
	return sdk_init_net_server(&net_param, &wifi_param);
}


int adapt_search_ap(sdk_wifi_search_t *ap_list)
{
	if(!ap_list)
		return -1;

	return sdk_search_ap(ap_list);
}

int adapt_get_wifi_list(sdk_wifi_search_t *ap_list)
{
	return sdk_get_wifi_list(ap_list);
}

 


int adapt_set_net_params(sdk_eth_cfg_t *net_param)
{
	if(!net_param)
		return -1;
	adapt_param_set_eth_cfg(net_param);//保存参数
	return sdk_set_net_params(net_param);//同步修改参数
}

int adapt_get_net_params(sdk_eth_cfg_t *net_param)
{
	if(!net_param)
		return -1;

	return  sdk_get_net_params(net_param);
}

/********************************/
int adapt_set_wifi_params(sdk_wifi_cfg_t *wifi_cfg)
{
	printf("========adapt_set_wifi_params88888888\n");
	printf("========wifi_cfg=%d\n",wifi_cfg);
    if(!wifi_cfg)
        return -1;
    sdk_eth_cfg_t net_param;
    memset(&net_param,0,sizeof(sdk_eth_cfg_t));
    sdk_get_net_params(&net_param);

    info("adapt_set_wifi_params net_param.if_mode ==== %d \n",net_param.if_mode );
	//一般只要设置了WIFI 参数 就意味着 要配置SSID 了  所以切换到STA模式
	if(net_param.if_mode == SDK_NET_MODE_AP)
    {
        net_param.if_mode = SDK_NET_MODE_WIFI;
        net_param.ip_info.enable_dhcp = 1;
    }


    sdk_set_net_params(&net_param);
	
    adapt_param_set_wifi_cfg(wifi_cfg);
	adapt_param_set_eth_cfg(&net_param);// 将if_mode保存起来，它代表了AP还是STA
    sdk_set_wifi_params(wifi_cfg);
    return 0;
}

int adapt_get_wifi_params(sdk_wifi_cfg_t *wifi_cfg)
{
    if(!wifi_cfg)
        return -1;

    return adapt_param_get_wifi_cfg(wifi_cfg);

}

/***************/

int adapt_status_callback()
{
	return  0;
}

 //NetStatusCB networkCB
 int adapt_start_net_server(sdk_sys_cfg_t  *sys_cfg)
{
	//adapt_status_callback在这里要注意一下 直接return 0 可能需要完善
	printf("sys_cfg->serial_2   +_+_+_+_+_+_+_+_+_     %s   \n",sys_cfg->serial_2);
	return sdk_start_net_server(adapt_status_callback/*,sys_cfg->serial_2*/);
 }

 int adapt_stop_net_server()
 {
 	return sdk_stop_net_server();
 }

//返回值: AP:0  STA:1
int adapt_get_wifi_mode()
{
    return sdk_get_wifi_mode();
}
//返回值: eth :1
int adapt_get_eth_mode()
{
    return sdk_get_eth_mode();
}

int adapt_set_wifi_mode(int  flag)
{
    return sdk_set_wifi_mode(flag);
}
int adapt_set_ap_info(char *ssid)//或者是传入参数
{
    return 0;
}
#endif

