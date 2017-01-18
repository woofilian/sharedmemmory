/******************************************************************************

                  版权所有 (C), 2012-2022, bingchuan

 ******************************************************************************
  文 件 名   : tutk_msg.c
  版 本 号   : v1.0
  作    者   : bingchuan
  生成日期   : 2015年5月26日
  功能描述   : tutk中对应的具体消息的实现
  函数列表   :

  修改历史   :
  1.日    期   : 2015年5月26日
    作    者   : bingchuan
    修改内容   : 创建文件

******************************************************************************/
#include <stdint.h>
#include "IOTCAPIs.h"
#include <unistd.h>

#include "AVAPIs.h"
#include "AVFRAMEINFO.h"
#include "AVIOCTRLDEFs.h"

#include "main.h"
#include "sdk_struct.h"
#include "log.h"
#include "ntpclient.h"

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/statfs.h>

#include "log.h"
#include "tutk.h"
#include "storage.h"


extern 	TUTK_INFO g_tutk_cfg;
extern sdk_eth_cfg_t g_ip_info; //IP信息配置
extern net_manager_info_t g_manager;
extern int network_is_setting;


extern sdk_msg_dispatch_cb g_tutk_msg_cb;
extern RECORD_STATUS record_status;
extern LED_BUZZER_INFO  led_buzzer_info;
extern aws_status	aws;
extern tutk_ls_avs_ops g_tutk_ls; //实时流

extern  int g_main_run ;
extern 	int download_progress;
extern sdk_pri_event_t pir_event;

#define TUTK_MSG_CTRL(_id, _arg, _ext, _ch, pmsg) ({\
				int _ret = -1;\
				pmsg->msg_id	= _id;\
				pmsg->msg_dir 	= SDK_MSG_REQ;\
				pmsg->chann 	= _ch;\
				pmsg->args		= _arg;\
				pmsg->extend	= _ext;\
				if(g_tutk_msg_cb)\
				_ret = g_tutk_msg_cb(pmsg);\
				if(_ret != 0)\
				goto __error;})


#define MSG_BUF_SIZE 16*1024


static int g_cap_type = SENSOR_TYPE_AR0330;
static sdk_sys_cfg_t g_sys_cfg;


#define TEST_BIT(x,  n)      ((x) & ( 1 << (n)))
#define SET_BIT(x,  n)     do {  x =  (x | ( 1 << (n))) ; } while(0);
#define CLEAR_BIT(x,  n)    do { x =  (x & (~( 1 << (n))));  } while(0);


//IOTYPE_USER_IPCAM_GET_ATTRIBUTE_REQ
int tk_get_sys_info(sdk_sys_cfg_t *sys_cfg)
{
    if(!sys_cfg)
        return -1;

    char msg_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
    sdk_sys_cfg_t *p_sys_cfg = (sdk_sys_cfg_t *)(pMsg->data);
    //通知参数库 刷新所有的参数信息
    TUTK_MSG_CTRL(SDK_MAIN_MSG_SYS_CFG,SDK_PARAM_GET,0,0,pMsg);//发送信息  rest参数成功
    memcpy(sys_cfg,p_sys_cfg,sizeof(sdk_sys_cfg_t));
    return 0;
__error:
    return -1;
}

int tk_set_sys_info(sdk_sys_cfg_t *sys_cfg)
{
    if(!sys_cfg)
        return -1;

    char msg_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
    sdk_sys_cfg_t *p_sys_cfg = (sdk_sys_cfg_t *)(pMsg->data);
    memcpy(p_sys_cfg,sys_cfg,sizeof(sdk_sys_cfg_t));
    //通知参数库 刷新所有的参数信息
    TUTK_MSG_CTRL(SDK_MAIN_MSG_SYS_CFG,SDK_PARAM_SET,0,0,pMsg);//发送信息  rest参数成功
    return 0;
__error:
    return -1;

}

/************************/


//这个是查尊设备属性的取值范围  目前就支持OV9712 1280*720 640*480
void TK_get_attribute_req_handle(int avIndex, char *buf)
{
    SMsgAVIoctrlGetAttributeResp resp;
    memset(&resp, 0, sizeof(SMsgAVIoctrlGetAttributeResp));

    resp.brightness_max = 255;
    resp.brightness_min = 0;
    resp.contrast_max = 255;
    resp.contrast_min = 0;
    resp.hue_max = 255;
    resp.hue_min = 0;
    resp.saturation_max = 255;
    resp.saturation_min = 0;
    resp.sharpness_max = 255;
    resp.sharpness_min = 0;
    resp.motion_detect_max =3;
    resp.motion_detect_min = 1;
    resp.sound_detect_max = 3;
    resp.sound_detect_min = 1;

    resp.mic = 1;
    resp.pt_rate_max = 2;
    resp.pt_rate_min =1;
    resp.resolution |= 0x01 << (6 - 1);//1280*720

    resp.speaker = 1;
    resp.video_display |= 0x01 << 0;
    resp.video_display |= 0x01 << 1;
    resp.video_display |= 0x01 << 2;
    resp.video_display |= 0x01 << 3;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_ATTRIBUTE_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetAttributeResp));
}

#if 1
int write_logfile_update(char *filename,char *conent)
{
    time_t timep;
    struct tm tmx;
    char *p,*token;
    char buf[128];
    char buf1[128];
    int i,index=0;
    char record[512];
    FILE *conf_fp,*temp_fp;
    char chfilename[128];
    char renamecmd[128];
    char str_mon[4],str_mday[3],str_year[5],str_hour[3],str_min[3],str_sec[3],str_day[3];
    char str_mon_array[][4]=
    {
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec"
    };
    tmx.tm_mon = 1;
    time(&timep);
    timep = timep;
    token=strtok(asctime(gmtime(&timep)),"\n");
    sprintf(buf,"%s",token);
    sscanf(buf,"%3s %3s  %2s %2s:%2s:%2s %4s",(char *)&str_day,(char *)&str_mon,(char *)&str_mday,(char *)&str_hour,(char *)&str_min,(char *)&str_sec,(char *)&str_year);
    for(i=0; i<12; i++)
    {
        if(!strcmp(str_mon_array[i],str_mon))
        {
            tmx.tm_mon=i;
            break;
        }
    }

    tmx.tm_mday=atoi(str_mday);
    tmx.tm_year=atoi(str_year);
    tmx.tm_hour=atoi(str_hour);
    tmx.tm_min=atoi(str_min);
    tmx.tm_sec=atoi(str_sec);

    sprintf(buf1,"%04d/%02d/%02d %02d:%02d:%02d"
            ,tmx.tm_year,tmx.tm_mon+1,tmx.tm_mday
            ,tmx.tm_hour,tmx.tm_min,tmx.tm_sec
           );

    sprintf(chfilename,"/mnt/mtd/%s",filename);
    if (!(conf_fp=fopen(chfilename,"a+")))// open file pointer
    {
        printf("conf_fp open[%s] failed...\n",chfilename);
        ///fclose(conf_fp);
        return 0;
    }
    sprintf(chfilename,"/tmp/%s",filename);
    if (!(temp_fp=fopen(chfilename,"w+")))// open file pointer
    {
        printf("temp_fp open[%s] failed...\n",chfilename);
        ///fclose(conf_fp);
        return 0;
    }
    fprintf(temp_fp,"%s:%s\n",buf1, conent);
    ///printf("to write logo while\n");

    while(feof(conf_fp) == 0&&feof(conf_fp)!= 255)
    {
        index ++;
        p = fgets(record,512,conf_fp);
        if(p == NULL)
            break;
        fprintf(temp_fp,"%s",record);
        if(index>=200)
            break;
    }

    fflush(temp_fp);
    fclose(temp_fp);
    fclose(conf_fp);
    sprintf(renamecmd,"cp /tmp/%s /mnt/mtd/%s",filename,filename);
    system(renamecmd);
    return 1;
}
#endif

void tk_list_wifi_req_handle(int avIndex, char *buf)
{

    //搜索无线路由
    int i=0;
    int ap_num = 0;
    char msg_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;

    SMsgAVIoctrlListWifiApResp *pWifiApResp = NULL;

    TUTK_MSG_CTRL(SDK_MAIN_MSG_WIFI_SEARCH,0,0,0,pMsg);
    sdk_wifi_search_t *ap_list = (sdk_wifi_search_t*)pMsg->data;
    if(ap_list->ap_count == 0)
    {
        pWifiApResp = malloc(sizeof(SMsgAVIoctrlListWifiApResp));
        memset(pWifiApResp,0,sizeof(SMsgAVIoctrlListWifiApResp));
        pWifiApResp->number = 0;
		goto __error;
    }
    printf("TK_list_wifi_req_handle ap_count =%d sizeof(SMsgAVIoctrlListWifiApResp):%d \n",ap_list->ap_count,sizeof(SMsgAVIoctrlListWifiApResp));
    pWifiApResp  = (SMsgAVIoctrlListWifiApResp *)malloc(sizeof(SMsgAVIoctrlListWifiApResp)+sizeof(SWifiAp)*(ap_list->ap_count + 1));
    if(NULL == pWifiApResp) //内存都申请不到的话 这个系统 再运行也没意义了
    {
        printf("malloc wifi Rsep memmey failed! break\n");
        assert(0);
        return ;
    }

   // pWifiApResp->number = ap_list->ap_count;

    //for(i = 0; i < pWifiApResp->number; i++)
	for(i = 0; i < ap_list->ap_count; i++)
    {

		if(i>28)break;
		pWifiApResp->number = i;
        SWifiAp *pWifiAp = &pWifiApResp->stWifiAp[i];


        wifi_ap_info_t  *wifi_ap = &ap_list->ap_info[i];
       /* printf("\n\n=======================================\n");
        printf("wifi ap num		:%d \n"
               "essid			:%s		\n"
               "encrypt_type	:%d		\n"
               "auth_mode		:%d		\n"
               "secret_key_type:%d		\n"
               "quality		:%d		\n"
               ,i
               ,wifi_ap->ssid						//服务区别号
               ,wifi_ap->encrypt_type				//加密类型0:NONE  1:WPA  2:WPA2 3:WEP
               ,wifi_ap->auth_mode					//认证方式0:NONE  1:EAP 2:PSK 3:OPEN 4:SHARED
               ,wifi_ap->secret_key_type			//密钥管理方式 0:NONE  1:AES 2:TKIP 只对应于加密类型为WPA/WPA2的情况
               ,wifi_ap->signal					   //信号质量 0-100
              );*/

        strcpy(pWifiApResp->stWifiAp[i].ssid, wifi_ap->ssid);
        pWifiApResp->stWifiAp[i].signal = wifi_ap->signal;
        pWifiApResp->stWifiAp[i].mode = AVIOTC_WIFIAPMODE_NULL;
        pWifiAp->status = 0;//wifi_ap->essid_active;

#if 1
        //加密类型0:NONE  1:WPA  2:WPA2 3:WEP
        //认证方式0:NONE  1:EAP 2:PSK 3:OPEN 4:SHARED

/*
uint8_t encrypt_type;			   加密类型0:OPEN 1:SHARED 2:WPAPSK 3:WPA2PSK 
uint8_t auth_mode;				   认证方式0:NONE  1:EAP 2:PSK 3:OPEN 4:SHARED
uint8_t secret_key_type; 		   密钥管理方式0:NONE 1:WEP 2:TKIP 3:AES  TKIP只对应于加密类型为WPA/WPA2的情况
 

*/
#if 0
        if(0 ==  wifi_ap->encrypt_type && 0 ==  wifi_ap->secret_key_type)
        {
            pWifiAp->enctype = AVIOTC_WIFIAPENC_NONE;
        }
        else if(3 ==  wifi_ap->encrypt_type )
        {
            pWifiAp->enctype = AVIOTC_WIFIAPENC_WEP;
        }
        else if(1 ==  wifi_ap->encrypt_type && 2 ==  wifi_ap->secret_key_type)
        {
            pWifiAp->enctype = AVIOTC_WIFIAPENC_WPA_TKIP;
        }
        else if(1 ==  wifi_ap->encrypt_type && 1 ==  wifi_ap->secret_key_type)
        {
            pWifiAp->enctype = AVIOTC_WIFIAPENC_WPA_AES;
        }
        else if(2 ==  wifi_ap->encrypt_type && 2 ==  wifi_ap->secret_key_type)
        {
            pWifiAp->enctype = AVIOTC_WIFIAPENC_WPA2_TKIP;
        }
        else if(2 ==  wifi_ap->encrypt_type && 1 ==  wifi_ap->secret_key_type)
        {
            pWifiAp->enctype = AVIOTC_WIFIAPENC_WPA2_AES;
        }
#else
	if(0 ==  wifi_ap->encrypt_type && 0 ==  wifi_ap->secret_key_type)
	{
		pWifiAp->enctype = AVIOTC_WIFIAPENC_NONE;
	}
	else if(1 ==  wifi_ap->encrypt_type && 0 ==  wifi_ap->secret_key_type)
	{
		pWifiAp->enctype = AVIOTC_WIFIAPENC_WEP;
	}
	else if(2 ==  wifi_ap->encrypt_type && 2 ==  wifi_ap->secret_key_type)
	{
		pWifiAp->enctype = AVIOTC_WIFIAPENC_WPA_TKIP;
	}
	else if(2 ==  wifi_ap->encrypt_type && 3 ==  wifi_ap->secret_key_type)
	{
		pWifiAp->enctype = AVIOTC_WIFIAPENC_WPA_AES;
	}
	else if(3 ==  wifi_ap->encrypt_type && 2 ==  wifi_ap->secret_key_type)
	{
		pWifiAp->enctype = AVIOTC_WIFIAPENC_WPA2_TKIP;
	}
	else if(3 ==  wifi_ap->encrypt_type && 3 ==  wifi_ap->secret_key_type)
	{
		pWifiAp->enctype = AVIOTC_WIFIAPENC_WPA2_AES;
	}
#endif
#else
        //BYTE	enctype; /*安全模式0:NONE 1:WEP 2:TKIP 3:AES */
        //BYTE	authmode;  /*验证模式0:OPEN 1:SHARED 2:WPAPSK 3:WPA2PSK */

        if(wifi_find.wifi_info[i].enctype ==0 && wifi_find.wifi_info[i].authmode == 0)
        {
            pWifiAp->enctype = AVIOTC_WIFIAPENC_NONE;
        }
        else if(wifi_find.wifi_info[i].enctype ==1 && wifi_find.wifi_info[i].authmode == 0)
        {
            pWifiAp->enctype = AVIOTC_WIFIAPENC_WEP;
        }
        else if(wifi_find.wifi_info[i].enctype ==2 && wifi_find.wifi_info[i].authmode == 2)
        {
            pWifiAp->enctype = AVIOTC_WIFIAPENC_WPA_TKIP;
        }
        else if(wifi_find.wifi_info[i].enctype ==3 && wifi_find.wifi_info[i].authmode == 2)
        {
            pWifiAp->enctype = AVIOTC_WIFIAPENC_WPA_AES;
        }
        else if(wifi_find.wifi_info[i].enctype ==2 && wifi_find.wifi_info[i].authmode == 3)
        {
            pWifiAp->enctype = AVIOTC_WIFIAPENC_WPA2_TKIP;
        }
        else if(wifi_find.wifi_info[i].enctype ==3 && wifi_find.wifi_info[i].authmode == 3)
        {
            pWifiAp->enctype = AVIOTC_WIFIAPENC_WPA2_AES;
        }
#endif
    }


    printf("----------pWifiApResp->number:%d  send size:%d\n", pWifiApResp->number, 4 +(pWifiApResp->number*sizeof(SWifiAp)));
__error:
	avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_LISTWIFIAP_RESP, (char *)pWifiApResp, sizeof(SMsgAVIoctrlListWifiApResp) +(pWifiApResp->number*sizeof(SWifiAp)));
    if(pWifiApResp != NULL)
    {
        free(pWifiApResp);
    }

}


void DE_list_wifi_req_handle(int avIndex, char *buf)
{
    //搜索无线路由
    int i=0;
    int ap_num = 0;
    char msg_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;

    SMsgAVIoctrlListWifiApResp *pWifiApResp = NULL;

    TUTK_MSG_CTRL(SDK_MAIN_MSG_WIFI_SEARCH,0,0,0,pMsg);
    sdk_wifi_search_t *ap_list = (sdk_wifi_search_t*)pMsg->data;
    if(ap_list->ap_count == 0)
    {
        pWifiApResp = malloc(sizeof(SMsgAVIoctrlListWifiApResp));
        memset(pWifiApResp,0,sizeof(SMsgAVIoctrlListWifiApResp));
        pWifiApResp->number = 0;
		goto __error;
    }
    printf("TK_list_wifi_req_handle ap_count =%d sizeof(SMsgAVIoctrlListWifiApResp):%d \n",ap_list->ap_count,sizeof(SMsgAVIoctrlListWifiApResp));
    pWifiApResp  = (SMsgAVIoctrlListWifiApResp *)malloc(sizeof(SMsgAVIoctrlListWifiApResp)+sizeof(SWifiAp)*(ap_list->ap_count + 1));
    if(NULL == pWifiApResp) //内存都申请不到的话 这个系统 再运行也没意义了
    {
        printf("malloc wifi Rsep memmey failed! break\n");
        assert(0);
        return ;
    }

	for(i = 0; i < ap_list->ap_count; i++)
    {

		if(i>28)break;
		pWifiApResp->number = i;
        SWifiAp *pWifiAp = &pWifiApResp->stWifiAp[i];


        wifi_ap_info_t  *wifi_ap = &ap_list->ap_info[i];
       /* printf("\n\n=======================================\n");
        printf("wifi ap num		:%d \n"
               "essid			:%s		\n"
               "encrypt_type	:%d		\n"
               "auth_mode		:%d		\n"
               "secret_key_type:%d		\n"
               "quality		:%d		\n"
               ,i
               ,wifi_ap->ssid						//服务区别号
               ,wifi_ap->encrypt_type				//加密类型0:NONE  1:WPA  2:WPA2 3:WEP
               ,wifi_ap->auth_mode					//认证方式0:NONE  1:EAP 2:PSK 3:OPEN 4:SHARED
               ,wifi_ap->secret_key_type			//密钥管理方式 0:NONE  1:AES 2:TKIP 只对应于加密类型为WPA/WPA2的情况
               ,wifi_ap->signal					   //信号质量 0-100
              );*/

        strcpy(pWifiApResp->stWifiAp[i].ssid, wifi_ap->ssid);
        pWifiApResp->stWifiAp[i].signal = wifi_ap->signal;
        pWifiApResp->stWifiAp[i].mode = AVIOTC_WIFIAPMODE_NULL;
        pWifiAp->status = 0;//wifi_ap->essid_active;

	if(0 ==  wifi_ap->encrypt_type && 0 ==  wifi_ap->secret_key_type)
	{
		pWifiAp->enctype = AVIOTC_WIFIAPENC_NONE;
	}
	else if(1 ==  wifi_ap->encrypt_type && 0 ==  wifi_ap->secret_key_type)
	{
		pWifiAp->enctype = AVIOTC_WIFIAPENC_WEP;
	}
	else if(2 ==  wifi_ap->encrypt_type && 2 ==  wifi_ap->secret_key_type)
	{
		pWifiAp->enctype = AVIOTC_WIFIAPENC_WPA_TKIP;
	}
	else if(2 ==  wifi_ap->encrypt_type && 3 ==  wifi_ap->secret_key_type)
	{
		pWifiAp->enctype = AVIOTC_WIFIAPENC_WPA_AES;
	}
	else if(3 ==  wifi_ap->encrypt_type && 2 ==  wifi_ap->secret_key_type)
	{
		pWifiAp->enctype = AVIOTC_WIFIAPENC_WPA2_TKIP;
	}
	else if(3 ==  wifi_ap->encrypt_type && 3 ==  wifi_ap->secret_key_type)
	{
		pWifiAp->enctype = AVIOTC_WIFIAPENC_WPA2_AES;
	}
    }


    printf("----------pWifiApResp->number:%d  send size:%d\n", pWifiApResp->number, 4 +(pWifiApResp->number*sizeof(SWifiAp)));
__error:
	avSendDefaultIOCtrl(avIndex, IOTYPE_USER_IPCAM_LISTWIFIAP_RESP, (char *)pWifiApResp, sizeof(SMsgAVIoctrlListWifiApResp) +(pWifiApResp->number*sizeof(SWifiAp)));
    if(pWifiApResp != NULL)
    {
        free(pWifiApResp);
    }

}

extern int get_wifi_encrypt_type(sdk_wifi_cfg_t* param,int mode);

void TK_set_wifi_req_handle(int avIndex, char *buf)
{
    SMsgAVIoctrlSetWifiResp resp;
    resp.result = -1;

    SMsgAVIoctrlSetWifiReq *p = (SMsgAVIoctrlSetWifiReq *)buf;
    char msg_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;

    sdk_wifi_cfg_t wifi_ap;
    memset(&wifi_ap,0,sizeof(sdk_wifi_cfg_t));

	int mode =0;

    memcpy(wifi_ap.ssid, p->ssid, MAX_DEV_ID_LEN-1);
    memcpy(wifi_ap.pwd, p->password, MAX_DEV_ID_LEN-1);

	if(SDK_NET_MODE_AP == g_manager.current_mode)
	{
		mode = 1;
	}
	else 
	{
		mode = 0;
	}

	if(-1 == get_wifi_encrypt_type(&wifi_ap,mode))
	{	 //如果自己解析出错，就使用下发的.
		if(p->enctype != AVIOTC_WIFIAPENC_INVALID)
		{
		   if(p->enctype == AVIOTC_WIFIAPENC_NONE)
		   {
			   wifi_ap.encrypt_type  = 0 ;
			   wifi_ap.secret_key_type = 0;
		   }
		   else if(p->enctype == AVIOTC_WIFIAPENC_WEP)
		   {
			   wifi_ap.encrypt_type  = 1 ;
			   wifi_ap.secret_key_type = 0;
		   }
		   else if(p->enctype == AVIOTC_WIFIAPENC_WPA_TKIP)
		   {
			   wifi_ap.encrypt_type  = 2 ;
			   wifi_ap.secret_key_type = 2;
		   }
		   else if(p->enctype == AVIOTC_WIFIAPENC_WPA_AES)
		   {
			   wifi_ap.encrypt_type  = 2 ;
			   wifi_ap.secret_key_type = 3;
		   }
		   else if(p->enctype == AVIOTC_WIFIAPENC_WPA2_TKIP)
		   {
			   wifi_ap.encrypt_type  = 3;
			   wifi_ap.secret_key_type = 2;
		   }
		   else if(p->enctype == AVIOTC_WIFIAPENC_WPA2_AES)
		   {
			   wifi_ap.encrypt_type  = 3 ;
			   wifi_ap.secret_key_type = 3;
		   }
			resp.result = 0;
		}
	   else
	   {
		   printf("::set netwifi fail\n");
		   resp.result = 1;
	   }
	}
	else
	{
		resp.result = 0;
	}

	printf("p->enctype=%d wifi_ap.encrypt_type=%d wifi_ap.secret_key_type=%d\n",p->enctype,wifi_ap.encrypt_type,wifi_ap.secret_key_type);
    memcpy(pMsg->data,(char *)&wifi_ap,sizeof(sdk_wifi_cfg_t));
    TUTK_MSG_CTRL(SDK_MAIN_MSG_WIFI_PARAM_CFG,SDK_PARAM_SET,0,0,pMsg);
    resp.result = 0;
	avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETWIFI_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetWifiResp));
__error:
	resp.result = 1;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETWIFI_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetWifiResp));

}
void DE_get_wifi_req_handle(int avIndex, char *buf)
{

    SMsgAVIoctrlGetWifiResp resp;
    sdk_wifi_cfg_t *wifi_cfg = NULL;
    char msg_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;



    memset(&resp, 0, sizeof(resp));
    TUTK_MSG_CTRL(SDK_MAIN_MSG_WIFI_PARAM_CFG,SDK_PARAM_GET,0,0,pMsg);
    if(pMsg == NULL)goto __error;
    wifi_cfg = (sdk_wifi_cfg_t *)pMsg->data;

    memcpy(resp.ssid, wifi_cfg->ssid, MAX_DEV_ID_LEN-1);
    memcpy(resp.password, wifi_cfg->pwd, MAX_PASSWD_LEN-1);

    if(0 ==  wifi_cfg->encrypt_type && 0 ==  wifi_cfg->secret_key_type)
    {
        resp.enctype = AVIOTC_WIFIAPENC_NONE;
    }
	else if(1 ==  wifi_cfg->encrypt_type && 0 ==  wifi_cfg->secret_key_type)
	{
		resp.enctype = AVIOTC_WIFIAPENC_WEP;
	}
	else if(2 ==  wifi_cfg->encrypt_type && 2 ==  wifi_cfg->secret_key_type)
    {
        resp.enctype = AVIOTC_WIFIAPENC_WPA_TKIP;
    }
    else if(2 ==  wifi_cfg->encrypt_type && 3 ==  wifi_cfg->secret_key_type)
    {
        resp.enctype = AVIOTC_WIFIAPENC_WPA_AES;
    }
    else if(3 ==  wifi_cfg->encrypt_type && 2 ==  wifi_cfg->secret_key_type)
    {
        resp.enctype = AVIOTC_WIFIAPENC_WPA2_TKIP;
    }
    else if(3 ==  wifi_cfg->encrypt_type && 3 ==  wifi_cfg->secret_key_type)
    {
        resp.enctype = AVIOTC_WIFIAPENC_WPA2_AES;
    }
	

    resp.mode = AVIOTC_WIFIAPMODE_ADHOC;

    printf("resp.enctype :%d \n", resp.enctype);
	avSendDefaultIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETWIFI_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetWifiResp));
__error:
    avSendDefaultIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETWIFI_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetWifiResp));

}

void TK_get_wifi_req_handle(int avIndex, char *buf)
{

    SMsgAVIoctrlGetWifiResp resp;
    sdk_wifi_cfg_t *wifi_cfg = NULL;
    char msg_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;



    memset(&resp, 0, sizeof(resp));
    TUTK_MSG_CTRL(SDK_MAIN_MSG_WIFI_PARAM_CFG,SDK_PARAM_GET,0,0,pMsg);
    if(pMsg == NULL)goto __error;
    wifi_cfg = (sdk_wifi_cfg_t *)pMsg->data;

    memcpy(resp.ssid, wifi_cfg->ssid, MAX_DEV_ID_LEN-1);
    memcpy(resp.password, wifi_cfg->pwd, MAX_PASSWD_LEN-1);
#if 0
    if(0 ==  wifi_cfg->encrypt_type && 0 ==  wifi_cfg->secret_key_type)
    {
        resp.enctype = AVIOTC_WIFIAPENC_NONE;
    }
    else if(3 ==  wifi_cfg->encrypt_type )
    {
        resp.enctype = AVIOTC_WIFIAPENC_WEP;
    }
    else if(1 ==  wifi_cfg->encrypt_type && 2 ==  wifi_cfg->secret_key_type)
    {
        resp.enctype = AVIOTC_WIFIAPENC_WPA_TKIP;
    }
    else if(1 ==  wifi_cfg->encrypt_type && 1 ==  wifi_cfg->secret_key_type)
    {
        resp.enctype = AVIOTC_WIFIAPENC_WPA_AES;
    }
    else if(2 ==  wifi_cfg->encrypt_type && 2 ==  wifi_cfg->secret_key_type)
    {
        resp.enctype = AVIOTC_WIFIAPENC_WPA2_TKIP;
    }
    else if(2 ==  wifi_cfg->encrypt_type && 1 ==  wifi_cfg->secret_key_type)
    {
        resp.enctype = AVIOTC_WIFIAPENC_WPA2_AES;
    }
#endif

    if(0 ==  wifi_cfg->encrypt_type && 0 ==  wifi_cfg->secret_key_type)
    {
        resp.enctype = AVIOTC_WIFIAPENC_NONE;
    }
	else if(1 ==  wifi_cfg->encrypt_type && 0 ==  wifi_cfg->secret_key_type)
	{
		resp.enctype = AVIOTC_WIFIAPENC_WEP;
	}
	else if(2 ==  wifi_cfg->encrypt_type && 2 ==  wifi_cfg->secret_key_type)
    {
        resp.enctype = AVIOTC_WIFIAPENC_WPA_TKIP;
    }
    else if(2 ==  wifi_cfg->encrypt_type && 3 ==  wifi_cfg->secret_key_type)
    {
        resp.enctype = AVIOTC_WIFIAPENC_WPA_AES;
    }
    else if(3 ==  wifi_cfg->encrypt_type && 2 ==  wifi_cfg->secret_key_type)
    {
        resp.enctype = AVIOTC_WIFIAPENC_WPA2_TKIP;
    }
    else if(3 ==  wifi_cfg->encrypt_type && 3 ==  wifi_cfg->secret_key_type)
    {
        resp.enctype = AVIOTC_WIFIAPENC_WPA2_AES;
    }
	

    resp.mode = AVIOTC_WIFIAPMODE_ADHOC;
    //resp.signal = wifi_cfg->
    //resp.status = wifi_cfg->stat

    printf("resp.enctype :%d \n", resp.enctype);
	avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETWIFI_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetWifiResp));
__error:
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETWIFI_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetWifiResp));

}



// 亮度对比度的调试
void TK_get_videoDisplay_req_handle(int avIndex, char *buf)
{
    SMsgAVIoctrlGetVideoDisplayResp resp;

    int brightness;
    int hue;
    int contrast;
    int saturation;
    char bufs1[3],bufs2[3],bufs3[3],bufs4[3];
    int conf_fp1,conf_fp2,conf_fp3,conf_fp4;
    int bytes;
	int icut_flag;

    char msg_bufs[MSG_BUF_SIZE] = {0};

    sdk_image_attr_t *attr =(sdk_image_attr_t *)buf;
    sdk_msg_t *pMsgs = (sdk_msg_t *)msg_bufs;
    sdk_image_attr_t *attr_param = (sdk_image_attr_t *)pMsgs->data;

    TUTK_MSG_CTRL(SDK_MAIN_MSG_IMAGE_ATTR_CFG,SDK_PARAM_GET,0,0,pMsgs);
    pMsgs->size = sizeof(sdk_image_attr_t);
    brightness = attr_param->brightness;
    contrast = attr_param->contrast;
    saturation = attr_param->saturation;
    hue = attr_param->hue;
	icut_flag = attr_param->icut_flag;



    char msg_buf[MSG_BUF_SIZE] = {0};

    sdk_osd_cfg_t *osd =(sdk_osd_cfg_t *)buf;
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
    sdk_osd_cfg_t *osd_param = (sdk_osd_cfg_t *)pMsg->data;

    TUTK_MSG_CTRL(SDK_MAIN_MSG_OSD_CFG,SDK_PARAM_GET,0,0,pMsg);


__error:
    printf("GET osd_param +_+_+_+_+_+_+_+_+_+_+_+_+ ERROR \n");
    resp.osd = osd_param ->time.valid;
    resp.brightness = brightness;
    resp.contrast = contrast;
    resp.hue = hue;
    resp.saturation = saturation;
    resp.closeInfrared = icut_flag;//128;//对接未实现
    resp.sharpness = 128;//暂未使用
#if 1
	if(resp.closeInfrared) //强制关闭红外
	{
		printf("p->closeInfrared11--------------------------\n");
		//closeInfrared();
		adapt_set_infrared_status(1);
	}
	else
	{
		printf("p->closeInfrared22--------------------------\n");
		//openInfrared();
		adapt_set_infrared_status(0);
	}
#endif

    warning("get [%d][%d][%d][%d]\n",brightness,contrast,hue,saturation);
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_VIDEO_DISPLAY_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetVideoDisplayResp));
}

void TK_set_videoDisplay_req_handle(int avIndex, char *buf)
{
    int 	saveint = 15;
    SMsgAVIoctrlSetVideoDisplayResp resp;
    SMsgAVIoctrlSetVideoDisplayReq *p = (SMsgAVIoctrlSetVideoDisplayReq *)buf;
    resp.result = -1;
#if 1

    char msg_bufs[MSG_BUF_SIZE] = {0};
    sdk_image_attr_t *attr =(sdk_image_attr_t *)buf;
    sdk_msg_t *pMsgs = (sdk_msg_t *)msg_bufs;
    sdk_image_attr_t *attr_param = (sdk_image_attr_t *)pMsgs->data;

    TUTK_MSG_CTRL(SDK_MAIN_MSG_IMAGE_ATTR_CFG,SDK_PARAM_GET,0,0,pMsgs);
    pMsgs->size = sizeof(sdk_image_attr_t);
	if((p->brightness!=0)||(p->contrast!=0)||(p->saturation!=0)||(p->hue!=0))
	{
	    attr_param->brightness = p->brightness;
	    attr_param->contrast = p->contrast;
	    attr_param->saturation = p->saturation;
	    attr_param->hue = p->hue;
	}
    attr_param->sharpness= 128;
    attr_param->icut_flag= p->closeInfrared;//0;
    TUTK_MSG_CTRL(SDK_MAIN_MSG_IMAGE_ATTR_CFG,SDK_PARAM_SET,0,0,pMsgs);


    /********osd on /off*******/
    char msg_buf[MSG_BUF_SIZE] = {0};

    sdk_osd_cfg_t *osd =(sdk_osd_cfg_t *)buf;
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
    sdk_osd_cfg_t *osd_param = (sdk_osd_cfg_t *)pMsg->data;

    TUTK_MSG_CTRL(SDK_MAIN_MSG_OSD_CFG,SDK_PARAM_GET,0,0,pMsg);
    pMsg->size = sizeof(sdk_osd_cfg_t);
    osd_param->time.valid = p->osd;
    osd_param->chann_name.valid = p->osd;
    TUTK_MSG_CTRL(SDK_MAIN_MSG_OSD_CFG,SDK_PARAM_SET,0,0,pMsg);
	#if 1
	if(p->closeInfrared) //强制关闭红外
	{
		printf("p->closeInfrared11--------------------------\n");
		//closeInfrared();
		adapt_set_infrared_status(1);
	}
	else
	{
		printf("p->closeInfrared22--------------------------\n");
		//openInfrared();
		adapt_set_infrared_status(0);
	}
	#endif
    resp.result = 0;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_VIDEO_DISPLAY_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetVideoDisplayResp));
	return;
__error:
    printf("SET osd_param +_+_+_+_+_+_+_+_+_+_+_+_+ ERROR \n");
    //warning("set [%s]\n[%s]\n[%s]\n[%s]\n",buf_brightness,buf_contrast,buf_hue,buf_saturation);
    printf("::set videoDisplay succeed\n");
#endif
    resp.result = -1;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_VIDEO_DISPLAY_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetVideoDisplayResp));
}

void TK_get_emailArg_req_handle(int avIndex, char *buf)
{
    SMsgAVIoctrlGetEmailArgResp resp;
    memset(&resp, 0, sizeof(SMsgAVIoctrlGetEmailArgResp));
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_EMAIL_ARG_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetEmailArgResp));
}

void TK_set_emailArg_req_handle(int avIndex, char *buf)
{
    SMsgAVIoctrlSetEmailArgResp resp;
    //SMsgAVIoctrlSetEmailArgReq *p = (SMsgAVIoctrlSetEmailArgReq *)buf;
#if 0
    g_email_cfg.enable_email = 1;
    memcpy(g_email_cfg.from_name, g_equinfo.camDesc, 32);
    memcpy(g_email_cfg.from_addr, p->sender, 64);
    memcpy(&g_email_cfg.to_addr[0][0], p->receiver, 64);
    memcpy(g_email_cfg.smtp_serv, p->smtp_svr, 64);
    memcpy(g_email_cfg.user, p->smtp_user, 32);
    memcpy(g_email_cfg.pwd, p->smtp_pwd, 32);
    g_email_cfg.smtp_port = p->smtp_port;
    g_email_cfg.enable_auth = p->smtp_auth;
    g_email_cfg.sslenable = p->smtp_ssl;
    saveSysInfo(EMAIL_CONFIG_PATH, (char *)&g_email_cfg, sizeof(g_email_cfg));
    printf("::set email succeed\n");
#endif
    resp.result = 0;

    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_EMAIL_ARG_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetEmailArgResp));
}



int TK_factory_setting_req_handle(int avIndex, char *buf)
{

    SMsgAVIoctrlFactorySettingsResp resp;
    char msg_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
    sdk_default_param_t   factory_msg_cfg ;
    memset(&factory_msg_cfg, 0, sizeof(sdk_default_param_t));

    factory_msg_cfg.param_mask = PARAM_MASK_ALL;  //恢复整机参数


    memcpy(pMsg->data, &factory_msg_cfg, sizeof(sdk_default_param_t));
    if(NULL == pMsg)
        goto __error;

    TUTK_MSG_CTRL(SDK_MAIN_MSG_PARAM_DEFAULT,SDK_PARAM_SET,0,0,pMsg);

    resp.result = 0;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_FACTORY_SETTINGS_RESP, (char *)&resp, sizeof(SMsgAVIoctrlFactorySettingsResp));
	g_main_run = 0 ;
	//g_tutk_run = 0 ;
	tutk_server_stop();
	led_buzzer_info.led_buzzer_pthread_is_processing = 0;
	aws.aws_pthread_is_processing = 0;
	printf("++++++++++++++++++reboot++++++++++++++++++\n");
	sleep(1);
	
    system("reboot");
    return 0;
__error:
    resp.result = -1;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_FACTORY_SETTINGS_RESP, (char *)&resp, sizeof(SMsgAVIoctrlFactorySettingsResp));
    return -1;

}

#if 1

int set_recordDuration_req_handle(int avIndex, char *buf)
{
    SMsgAVIoctrlSetRcdDurationResp resp;
    SMsgAVIoctrlSetRcdDurationReq *p = (SMsgAVIoctrlSetRcdDurationReq *)buf;
	
    char msg_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
    sdk_motion_cfg_t *md_param = (sdk_motion_cfg_t *)pMsg->data;
    TUTK_MSG_CTRL(SDK_MAIN_MSG_MOTION_CFG,SDK_PARAM_GET,0,0,pMsg);
	printf("recive [channel][durasecond][presecond] = %d %d %d\n",p->channel,p->durasecond,p->presecond);
	printf("get:record_time=%d\n",md_param->alarm_handle.record_time);
	if((30 == p->durasecond)
					|| (60 == p->durasecond)
					|| (120 == p->durasecond)
					|| (240 == p->durasecond))
	{
		md_param->alarm_handle.record_time = p->durasecond;
		TUTK_MSG_CTRL(SDK_MAIN_MSG_MOTION_CFG,SDK_PARAM_SET,0,0,pMsg);
		usleep(100*1000);

		pthread_mutex_lock(&record_status.record_lock);
		record_status.record_alarm_time = p->durasecond ;
		pthread_mutex_unlock(&record_status.record_lock);
	}

	
    resp.result = 0;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETRCD_DURATION_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetRcdDurationResp));
	return 0;
__error:
	resp.result = -1;
	avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETRCD_DURATION_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetRcdDurationResp));
	return 0;
}


int get_recordDuration_req_handle(int avIndex, char *buf)
{
    SMsgAVIoctrlGetRcdDurationResp resp;
    SMsgAVIoctrlGetRcdDurationReq *p = (SMsgAVIoctrlGetRcdDurationReq *)buf;
	char msg_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
    sdk_motion_cfg_t *md_param = (sdk_motion_cfg_t *)pMsg->data;
    TUTK_MSG_CTRL(SDK_MAIN_MSG_MOTION_CFG,SDK_PARAM_GET,0,0,pMsg);
	
    resp.channel = p->channel;
    resp.presecond = 5;
    resp.durasecond = md_param->alarm_handle.record_time;
	printf("get:record_time=%d\n",resp.durasecond);
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETRCD_DURATION_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetRcdDurationResp));
	return 0;
	__error:
	avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETRCD_DURATION_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetRcdDurationResp));
	return 0;
}
#endif
void TK_get_audioOutFormat_req_handle(int avIndex, char *buf)
{
    SMsgAVIoctrlGetAudioOutFormatResp resp;
    SMsgAVIoctrlGetAudioOutFormatReq *p = (SMsgAVIoctrlGetAudioOutFormatReq *)buf;
    resp.channel = p->channel;
#ifdef AUDIO_FORMAT_ADPCM
    resp.format = MEDIA_CODEC_AUDIO_ADPCM;
#else
    resp.format = MEDIA_CODEC_AUDIO_G726;
#endif

    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetAudioOutFormatResp));
}

void TK_get_supportStream_req_handle(int avIndex, char *buf)
{
    SMsgAVIoctrlGetSupportStreamResp *pResp;
    SMsgAVIoctrlGetSupportStreamReq *p = (SMsgAVIoctrlGetSupportStreamReq *)buf;
    char *pTmp = (char *)malloc(sizeof(SStreamDef) + 4);
    if(!pTmp)
    {
        return;
    }
    pResp = (SMsgAVIoctrlGetSupportStreamResp *)pTmp;
    pResp->number = 1;
    pResp->streams[0].channel =  0;
    pResp->streams[0].index = 0;

    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETSUPPORTSTREAM_RESP, (char *)pTmp, sizeof(SStreamDef) + 4);
    if(pTmp)
    {
        free(pTmp);
        pTmp = NULL;
    }
}


void TK_get_videoQuality_req_handle(int avIndex, char *buf)
{
    SMsgAVIoctrlGetVideoQualityResp resp;

    char msg_buf[MSG_BUF_SIZE] = {0};

    sdk_encode_t *enc = (sdk_encode_t *)buf;
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
    sdk_encode_t *enc_param = (sdk_encode_t *)pMsg->data;

    TUTK_MSG_CTRL(SDK_MAIN_MSG_ENCODE_CFG,SDK_PARAM_GET,0,0,pMsg);
    //memcpy(enc_param,enc,sizeof(sdk_encode_t));

    resp.bitrate= enc_param->av_enc_info[0].bitrate;
    resp.fps = enc_param->av_enc_info[0].frame_rate;
    int qlt = enc_param->av_enc_info[0].pic_quilty;
	#if 0
    if(qlt == 14)resp.hmethod = 1;
    else if(qlt == 13)resp.hmethod = 2;
    else if(qlt == 12)resp.hmethod = 3;
    else if(qlt == 11)resp.hmethod = 4;
    else if(qlt == 10)resp.hmethod = 5;
    /********默认写720P暂时**/
    resp.resolution = 0x04;// 4   720p=0x06
    /*********END*********/
	#endif
	resp.resolution = 0x06;
	resp.hmethod = enc_param->av_enc_info[0].bitrate_type;
	resp.quant = qlt +1;
	
    printf("TK_get_videoQuality_req_handle get==>framerate[%d] u32Quant[%d]  hmethod[%d] resolution[%d] bitrate[%d]--------\n",
           resp.fps,resp.quant,resp.hmethod,
           resp.resolution,resp.bitrate);

    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_VIDEO_QUALITY_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetVideoQualityResp));
	return;
	__error:
	printf("SET sdk_encode_t +_+_+_+_+_+_+_+_+_+_+_+_+ ERROR \n");
	avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_VIDEO_QUALITY_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetVideoQualityReq));
	return ;

}


// 安霸只能动态设置 帧率 和码流
void TK_set_videoQuality_req_handle(int avIndex, char *buf)
{
    SMsgAVIoctrlSetVideoQualityResp resp;
    SMsgAVIoctrlSetVideoQualityReq *p = (SMsgAVIoctrlSetVideoQualityReq *)buf;

    char msg_buf[MSG_BUF_SIZE] = {0};

    sdk_encode_t *enc = (sdk_encode_t *)buf;
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
    sdk_encode_t *enc_param = (sdk_encode_t *)pMsg->data;

    TUTK_MSG_CTRL(SDK_MAIN_MSG_ENCODE_CFG,SDK_PARAM_GET,0,0,pMsg);
#if 0
    if(NULL == enc_param)
    {
        goto __error;
    }
#endif
	printf("bitrate=%d quant=%d fps=%d resolution=%d hmethod=%d\n",
			p->bitrate,p->quant,p->fps,p->resolution,p->hmethod);

    pMsg->size = sizeof(sdk_encode_t);
    enc_param->av_enc_info[0].frame_rate= p->fps;
    enc_param->av_enc_info[0].bitrate= p->bitrate;
    enc_param->av_enc_info[0].pic_quilty = 0; //暂时不知道是否匹配编码库
	enc_param->av_enc_info[0].resolution = 15;//固定为720P
	enc_param->av_enc_info[0].bitrate_type = p->hmethod +1;

#if 0
    printf("@@@ZW_QJZ TK_set_videoQuality_req_handle  [%d] [%d] [%d] [%d] [%d]\n",
		enc_param->av_enc_info[0].pic_quilty,
		enc_param->av_enc_info[0].bitrate,
		enc_param->av_enc_info[0].frame_rate,
		enc_param->av_enc_info[0].bitrate_type,
		enc_param->av_enc_info[0].resolution);
#endif
	TUTK_MSG_CTRL(SDK_MAIN_MSG_ENCODE_CFG,SDK_PARAM_SET,0,0,pMsg);

	resp.result = 0;
	avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_VIDEO_QUALITY_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetVideoQualityReq));
	return ;
__error:
    printf("SET sdk_encode_t +_+_+_+_+_+_+_+_+_+_+_+_+ ERROR \n");
    resp.result = -1;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_VIDEO_QUALITY_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetVideoQualityReq));
	return ;
}

void TK_set_deviceInfo_req_handle(int avIndex, char *buf)
{
    SMsgAVIoctrlSetDeviceInfoResp resp;
    SMsgAVIoctrlSetDeviceInfoReq *p = (SMsgAVIoctrlSetDeviceInfoReq *)buf;
	char msg_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
    sdk_osd_cfg_t *osd_cfg = (sdk_osd_cfg_t *)pMsg->data;
    TUTK_MSG_CTRL(SDK_MAIN_MSG_OSD_CFG,SDK_PARAM_GET,0,0,pMsg);

    warning("set==>device_name[%s]\n",p->device_name);
	memset(&osd_cfg->chann_name.str,0,sizeof(osd_cfg->chann_name.str));
	strcpy(&osd_cfg->chann_name.str,p->device_name);
	osd_cfg->chann_name.str_len = strlen(p->device_name);
	TUTK_MSG_CTRL(SDK_MAIN_MSG_OSD_CFG,SDK_PARAM_SET,0,0,pMsg);

    //strcpy(deviceinfo_msg.device_type, p->device_name);
    //没有找到存储设备信息的变量（这里的设备信息是指用户自定义的名称）
    resp.result = 0;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_DEVICE_INFO_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetDeviceInfoResp));
	return;
__error:
	resp.result = 1;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_DEVICE_INFO_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetDeviceInfoResp));
	return;
}

void TK_get_ptzStatus_req_handle(int avIndex, char *buf)
{
	SMsgAVIoctrlGetPtzStatusResp resp;
#if 0
	if(g_device_info.serverMask & SUPPORT_YUNTAI)
	{
    SMsgAVIoctrlGetPtzStatusResp resp;

    resp.speed = g_video_rel.ptzspeed;
    if((get_ptzDirection_h() == PTZ_MODE_RIGHT_LEFT) && (get_ptzDirection_v() == PTZ_MODE_UP_DOWN))
    {
        resp.status = AVIOCTRL_PTZ_PATROL;
    }
    else if(get_ptzDirection_h() == PTZ_MODE_RIGHT_LEFT)
    {
        resp.status = AVIOCTRL_PTZ_LEFT_RIGHT_PATROL;
    }
    else if(get_ptzDirection_v() == PTZ_MODE_UP_DOWN)
    {
        resp.status = AVIOCTRL_PTZ_UP_DOWN_PATROL;
    }
    else if(get_ptzDirection_h() == PTZ_MODE_TOUR && get_ptzDirection_v() == PTZ_MODE_TOUR)
    {
        resp.status = AVIOCTRL_PTZ_TRACK_PATROL;
    }
    else
    {
        resp.status = AVIOCTRL_PTZ_NO_ACTION_PATROL;
    }
#endif
    printf("get==>speed[%d] status[%d]--------\n", resp.speed, resp.status);
        avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_PTZ_STATUS_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetPtzStatusResp));
        /// }
}

void TK_test_email_req_handle(int avIndex, char *buf)
{
    int tmp = 0;
    SMsgAVIoctrlTestEmailArgResp resp;
    ///tmp = test_smtp(&g_email_cfg);  // 测试 发送邮件
    resp.result = 0;//tmp;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_TEST_EMAIL_ARG_RESP, (char *)&resp, sizeof(SMsgAVIoctrlTestEmailArgResp));
}


// 改功能还没实现
void TK_get_audio_motion_req_handle(int avIndex, char *buf)
{
    SMsgAVIoctrlGetAudioMotionResp resp;
#if 0
    resp.enable = g_audio_motion.enable;
    resp.sensitive = g_audio_motion.sensitive;
    resp.timeout = g_audio_motion.timeout;       /*持续时间*/
    resp.msdrec_enable = g_audio_motion.msdrec_enable;
    resp.mmail_enable = g_audio_motion.mmail_enable;
    resp.mftp_enable = g_audio_motion.mftp_enable;
    resp.malarmout_enable = g_audio_motion.malarmout_enable;
#else
    resp.enable = 1;
    resp.sensitive = 3;
    resp.timeout = 5;       /*持续时间*/
    resp.msdrec_enable = 1;
    resp.mmail_enable = 0;
    resp.mftp_enable = 0;
    resp.malarmout_enable = 1;

#endif
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_AUDIO_MOTION_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetAudioMotionResp));
}

void TK_set_audio_motion_req_handle(int avIndex, char *buf)
{
    SMsgAVIoctrlSetAudioMotionResp resp;
    SMsgAVIoctrlSetAudioMotionReq *p = (SMsgAVIoctrlSetAudioMotionReq *)buf;
    int tmp = 0;
#if 0
    tmp = g_audio_motion.enable;
    g_audio_motion.enable = p->enable;
    g_audio_motion.sensitive = p->sensitive;
    g_audio_motion.timeout = p->timeout ;       /*持续时间*/
    g_audio_motion.msdrec_enable = p->msdrec_enable;
    g_audio_motion.mmail_enable = p->mmail_enable;
    g_audio_motion.mftp_enable = p->mftp_enable;
    g_audio_motion.malarmout_enable = p->malarmout_enable;

    if(tmp != g_audio_motion.enable && g_audio_motion.enable == 1)
    {
        audio_motion_sem_post();
    }

    saveSysInfo(AUDIO_MOTION_PATH, (char *)&g_audio_motion, sizeof(g_audio_motion));
#endif
    printf("::set videoQuality succeed\n");
    resp.result = 0;

    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_VIDEO_QUALITY_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetAudioMotionResp));
}


void tk_get_net_req_handle(int avIndex, char *buf)
{
    char msg_buf[MSG_BUF_SIZE] = {0};
    SMsgAVIoctrlGetNetResp *resp = (SMsgAVIoctrlGetNetResp *)malloc(sizeof(SMsgAVIoctrlGetNetResp));

	//直接读取g_manage的参数
    strncpy(resp->ipcamIP, 			g_manager.ip_addr, MAX_IP_ADDR_LEN);
    strncpy(resp->ipcamIPMask, 		g_manager.mask, MAX_IP_ADDR_LEN);
    strncpy(resp->ipcamGatewayIP,	g_manager.gateway, MAX_IP_ADDR_LEN); 	//获取到的网关地址为空?
    strncpy(resp->byDnsaddr[0], 	g_manager.dns1,MAX_IP_ADDR_LEN );
    strncpy(resp->byDnsaddr[1], 	g_manager.dns2,MAX_IP_ADDR_LEN );
	resp->conn_mod =				g_manager.enable_dhcp;
	printf("in tk_get_net_req_handle!!!\n");
	printf("ipcamIP=%s\n",g_manager.ip_addr);
	printf("ipcamIPMask=%s\n",g_manager.mask);
	printf("ipcamIPipcamGatewayIPs\n",g_manager.gateway);
	printf("byDnsaddr0=%s\n",g_manager.dns1);
	printf("byDnsaddr1=%s\n",g_manager.dns2);
	printf("conn_mod=%d\n",g_manager.enable_dhcp);
	
    avSendIOCtrl(avIndex,IOTYPE_USER_IPCAM_GET_NET_RESP,(char *) resp,sizeof(SMsgAVIoctrlGetNetResp));
    free(resp);
    resp = NULL;
	return;
__error:
    avSendIOCtrl(avIndex,IOTYPE_USER_IPCAM_GET_NET_RESP,(char *) resp,sizeof(SMsgAVIoctrlGetNetResp));
    free(resp);
    resp = NULL;
	return;
}

int is_valid_ip(const char *ip)
{
    int section = 0; //每一节的十进制值
    int dot = 0; //几个点分隔符
    int last = -1; //每一节中上一个字符

    while(*ip)
    {
        if(*ip == '.')
        {
            dot++;
            if(dot > 3)
            {
                return 0;
            }
            if(section >= 0 && section <=255)
            {
                section = 0;
            }
            else
            {
                return 0;
            }
        }
        else if(*ip >= '0' && *ip <= '9')
        {
            section = section * 10 + *ip - '0';
            if(last == '0')
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
        last = *ip;
        ip++;
    }

    if(section >= 0 && section <=255)
    {
        if(3 == dot)
        {
            section = 0;
            printf ("IP address success!\n");
            //printf ("%d\n",dot);
            return 1;
        }
    }
    return 0;
}

void tk_set_net_req_handle(int avIndex, char *buf)
{

    SMsgAVIoctrlSetNetResp resp;
    SMsgAVIoctrlSetNetReq *p = (SMsgAVIoctrlSetNetReq *)buf;

    char msg_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
    sdk_eth_cfg_t *eth_cfg = NULL;

    //memset(&eth_cfg, 0, sizeof(sdk_eth_cfg_t));
	eth_cfg = (sdk_eth_cfg_t*)pMsg->data;

    if(NULL == p)
    {
        goto __error;
    }
	printf("p->conn_mod 	= %d\n",p->conn_mod);
	printf("p->ipcamIP 		= %s\n",p->ipcamIP);
	printf("p->ipcamIPMask 	= %s\n",p->ipcamIPMask);
	printf("p->ipcamGatewayIP =%s\n",p->ipcamGatewayIP);
	printf("p->byDnsaddr[0]	= %s\n",p->byDnsaddr[0]);
	printf("p->conn_mod[1] 	= %s\n",p->byDnsaddr[1]);

	//TUTK_MSG_CTRL(SDK_MAIN_MSG_NET_LINK_CFG,SDK_PARAM_GET,0,0,pMsg);
	 printf("g_ip_info.if_mode = %d\n",g_ip_info.if_mode);
	 printf("g_ip_info.wifi_type = %d\n",g_ip_info.wifi_type);
	//保证其他非 sdk_ip_info_t ip_info 的变量不变
	//eth_cfg->if_mode 	= g_ip_info.if_mode;
	//eth_cfg->wifi_type 	= g_ip_info.wifi_type;
	memcpy(eth_cfg,&g_ip_info,sizeof(g_ip_info));
	printf("get:\n");
	printf("dhcp_enable = %d\n",eth_cfg->ip_info.enable_dhcp);
	printf("ip_addr = %s\n",eth_cfg->ip_info.ip_addr);
	printf("mask = %s\n",eth_cfg->ip_info.mask);
	printf("gateway = %s\n",eth_cfg->ip_info.gateway);
	printf("dns1 = %s\n",eth_cfg->ip_info.dns1);
	printf("dns2 = %s\n",eth_cfg->ip_info.dns2);
  	printf("if_mode = %d\n",eth_cfg->if_mode);
	printf("wifi_type = %d\n",eth_cfg->wifi_type);
  	printf("if_mode = %d\n",eth_cfg->if_mode);
	printf("wifi_type = %d\n",eth_cfg->wifi_type);
	printf("if_name = %s %s\n",eth_cfg->ip_info.if_name,g_ip_info.ip_info.if_name);


	if(1 == p->conn_mod) //DHCP
	{
		eth_cfg->ip_info.enable_dhcp = p->conn_mod;
		memset(eth_cfg->ip_info.ip_addr,0,MAX_IP_ADDR_LEN);
		memset(eth_cfg->ip_info.gateway,0,MAX_IP_ADDR_LEN);
		memset(eth_cfg->ip_info.mask,0,MAX_IP_ADDR_LEN);
		memset(eth_cfg->ip_info.dns1,0,MAX_IP_ADDR_LEN);
		memset(eth_cfg->ip_info.dns2,0,MAX_IP_ADDR_LEN);
	}
	else if(0 == p->conn_mod)
	{
		eth_cfg->ip_info.enable_dhcp = p->conn_mod;
		strcpy(eth_cfg->ip_info.ip_addr, p->ipcamIP);
		strcpy(eth_cfg->ip_info.gateway, p->ipcamGatewayIP);
		strcpy(eth_cfg->ip_info.mask, p->ipcamIPMask);
		strcpy(eth_cfg->ip_info.dns1, p->byDnsaddr[0] );
		strcpy(eth_cfg->ip_info.dns2, p->byDnsaddr[1] );
	}
	printf("set:\n");
	printf("dhcp_enable = %d\n",eth_cfg->ip_info.enable_dhcp);
	printf("ip_addr = %s\n",eth_cfg->ip_info.ip_addr);
	printf("mask = %s\n",eth_cfg->ip_info.mask);
	printf("gateway = %s\n",eth_cfg->ip_info.gateway);
	printf("dns1 = %s\n",eth_cfg->ip_info.dns1);
	printf("dns2 = %s\n",eth_cfg->ip_info.dns2);
	printf("if_mode = %d\n",eth_cfg->if_mode);
	printf("wifi_type = %d\n",eth_cfg->wifi_type);
	
//在SDK_MAIN_MSG_NET_LINK_CFG SET时 已调用接口赋值eth_cfg到全局变量g_ip_info 
//此处无须再赋值
    memcpy(pMsg->data, eth_cfg,sizeof(sdk_eth_cfg_t));
    TUTK_MSG_CTRL(SDK_MAIN_MSG_NET_LINK_CFG,SDK_PARAM_SET,0,0,pMsg);


    resp.result = 0;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_NET_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetNetResp));
    return ;

__error:
    resp.result = -1;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_NET_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetNetResp));
    return ;
}

// 改功能还没实现
    void TK_get_snap_req_handle(int avIndex, char *buf)
    {

        SMsgAVIoctrlSetSnapResp resp;

#if 0
        char msg_buf[MSG_BUF_SIZE] = {0};
        sdk_msg_t *pmsg = (sdk_msg_t *)msg_buf;
        TUTK_MSG_CTRL(SDK_MAIN_MSG_SNAP_CONTROL,0,0,0,pmsg);
#endif
        sdk_snap_info_t snap_info;
        snap_info.height=720;
        snap_info.width=1080;
        sdk_av_snap(0,&snap_info,"/mnt/nfs/snap.bmp");
        avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_SNAP_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetSnapResp));
		return 0;
__error:
        avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_SNAP_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetSnapResp));
		return 0;

	}



//根据客户端 来设置码流分辨率  安霸的应该 不设置
    void TK_default_video_req_handle(int avIndex, char *buf)
    {
        SMsgAVIoctrlDefaultVideoResp resp;
        SMsgAVIoctrlDefaultVideoReq *p = (SMsgAVIoctrlDefaultVideoReq *)buf;

        char msg_buf[MSG_BUF_SIZE] = {0};

        sdk_encode_t *enc = (sdk_encode_t *)buf;
        sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
        sdk_encode_t *enc_param = (sdk_encode_t *)pMsg->data;

        TUTK_MSG_CTRL(SDK_MAIN_MSG_ENCODE_CFG,SDK_PARAM_GET,0,0,pMsg);

        pMsg->size = sizeof(sdk_encode_t);

        enc_param->av_enc_info[0].frame_rate= 30;
        enc_param->av_enc_info[0].bitrate = 1024;
        enc_param->av_enc_info[0].pic_quilty = 0;

        warning("enc_param.av_enc_info[0].pic_quilty %d\n,enc_param->av_enc_info[0].frame %d\n)",enc_param->av_enc_info[0].pic_quilty,enc_param->av_enc_info[0].frame_rate);
        TUTK_MSG_CTRL(SDK_MAIN_MSG_ENCODE_CFG,SDK_PARAM_SET,0,0,pMsg);
        resp.result = 0;
		avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_DEFAULT_VIDEO_RESP, (char *)&resp, sizeof(SMsgAVIoctrlDefaultVideoResp));
		return;
__error:
        printf("set default enc_param +_+_+_+_+_+_+_+_+_+_+_+_+ ERROR \n");
		resp.result = 1;
        avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_DEFAULT_VIDEO_RESP, (char *)&resp, sizeof(SMsgAVIoctrlDefaultVideoResp));
    }
void TK_get_dev_cfg_req_handle(int avIndex, char *buf)
{
    pthread_mutex_t get_lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_init(&get_lock,NULL);

    SMsgAVIoctrlReadDevConfigResp resp;
    char msg_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;


    TUTK_MSG_CTRL(SDK_MAIN_MSG_NET_LINK_CFG,SDK_PARAM_GET,0,0,pMsg);//获取MAC 地址

    if(pMsg == NULL)
    {
        goto __error;
    }

    sdk_eth_cfg_t * get_mac_msg = (sdk_eth_cfg_t*)pMsg->data;

    strncpy(resp.szMacAddr, get_mac_msg->ip_info.mac, 20);
    //info("===>MAC:%s\n", resp.szMacAddr);

    pthread_mutex_lock(&get_lock);
   // strcpy(resp.szUID, g_tutk_cfg.ViewAccount);
    strcpy(resp.szUID, g_tutk_cfg.guid);
    strcpy(resp.szPwd, g_tutk_cfg.ViewPassword);

    //info("===>UID:%s\n", resp.szUID);
   // info("===>PWD:%s\n", resp.szPwd);

    pthread_mutex_unlock(&get_lock);
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_READ_DEV_CONFIG_RESP, (char *)&resp, sizeof(SMsgAVIoctrlReadDevConfigResp));
    return ;
__error:

    warning("===>get MAC UID error!!\n");
    return ;
}

void TK_set_dev_cfg_req_handle(int avIndex, char *buf)
{
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_init(&mutex,NULL);

    SMsgAVIoctrlWriteDevConfigResp resp;
    SMsgAVIoctrlWriteDevConfigReq *p = (SMsgAVIoctrlWriteDevConfigReq *)buf;
    if(NULL == p)
    {
        goto __error;
    }
	sdk_sys_cfg_t sys_cfg;
	sdk_user_right_t right;
    char msg_buf[MSG_BUF_SIZE]= {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;

        pthread_mutex_lock(&mutex);
		
        memset(&resp, 0, sizeof(resp));
        strncpy(g_tutk_cfg.guid, p->szUID, 21); //设置UID
        strncpy(g_tutk_cfg.ViewPassword, p->szPwd, 32);
		
		//info("UID:%s   PWD:%s\n", g_tutk_cfg.guid, g_tutk_cfg.ViewPassword);
		//先读
		TUTK_MSG_CTRL(SDK_MAIN_MSG_SYS_CFG,SDK_PARAM_GET,0,0,pMsg);
		memcpy(&sys_cfg,pMsg->data,sizeof(sdk_sys_cfg_t));
      	strncpy(sys_cfg.serial_2, p->szUID, 21); //设置UID
      	
		memset(pMsg,0,MSG_BUF_SIZE);
		pMsg->size = sizeof(sdk_msg_t);
		memcpy(pMsg->data, &sys_cfg,sizeof(sdk_sys_cfg_t));
        TUTK_MSG_CTRL(SDK_MAIN_MSG_SYS_CFG,SDK_PARAM_SET,0,0,pMsg);

	//设置密码  此处为设置ADMIN的密码
	//此处不用先获取  因为下层接口在修改时已获取 
	//然后比较用户名进行修改密码
	memcpy(right.user.user_name,"admin",sizeof("admin"));
	memcpy(right.user.user_pwd,p->szPwd,sizeof(p->szPwd));
	memset(pMsg,0,MSG_BUF_SIZE);
	memcpy(pMsg->data,(unsigned char *)&right,sizeof(sdk_user_right_t));
	pMsg->size = sizeof(sdk_user_right_t);
	TUTK_MSG_CTRL(SDK_MAIN_MSG_USER,SDK_USER_MODI,0,0,pMsg);
	pthread_mutex_unlock(&mutex);
    resp.result = 0;

    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_WIRTE_DEV_CONFIG_RESP, (char *)&resp, sizeof(SMsgAVIoctrlWriteDevConfigResp));
    return;
__error:
    resp.result = -1;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_WIRTE_DEV_CONFIG_RESP, (char *)&resp, sizeof(SMsgAVIoctrlWriteDevConfigResp));
    return ;
}
		
void TK_get_dev_info_req_handle(int avIndex, char *buf) //获取设备信息
{
	SMsgAVIoctrlDeviceInfoResp resp;

	struct statfs diskInfo;
	
	char msg_buf[MSG_BUF_SIZE] = {0};
	sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;

	TUTK_MSG_CTRL(SDK_MAIN_MSG_SYS_CFG,SDK_PARAM_GET,0,0,pMsg);
	if(NULL == pMsg)
	{
		goto __error;
	}
	sdk_sys_cfg_t *version_msg_cfg	= (sdk_sys_cfg_t *)pMsg->data;
	strncpy(resp.model, version_msg_cfg->device_type,
			(strlen(version_msg_cfg->device_type)>15)?15:strlen(version_msg_cfg->device_type));
	strncpy(resp.vendor, version_msg_cfg->manufacturer,
			(strlen(version_msg_cfg->manufacturer)>15)?15:strlen(version_msg_cfg->manufacturer));

	resp.channel = avIndex;

	unsigned int version = 0,version_0 = 0,version_1 = 0,version_2 = 0,version_3 = 0;

	sscanf(version_msg_cfg->software_ver,"V%d.%d.%d.%d",&version_3,&version_2,&version_1,&version_0);
	version_3 = ((version_3&0xff) << 24) ;
	version_2 = ((version_2&0xff) << 16) ;
	version_1 = ((version_1&0xff) << 8) ;
	version_0 = ((version_0&0xff) << 0) ;
	version = version_3| version_2 | version_1 | version_0;
	info("version:%d\n", version);
	resp.version = version;
	

	/*
	  *  get SD card information	2015-07-03
	  */

	statfs(GRD_SD_MOUNT_POINT, &diskInfo);	

	unsigned long long blocksize = diskInfo.f_bsize;	//每个block里包含的字节数  
	unsigned long long totalsize = blocksize * diskInfo.f_blocks;	//总的字节数，f_blocks为block的数目  

	info("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n",   
		totalsize, totalsize>>10, totalsize>>20, totalsize>>30);  
	  
	unsigned long long freeDisk = diskInfo.f_bfree * blocksize; //剩余空间的大小  
	unsigned long long availableDisk = diskInfo.f_bavail * blocksize;	//可用空间大小	

	info("Disk_free = %llu MB = %llu GB\nDisk_available = %llu MB = %llu GB\n",   
		freeDisk>>20, freeDisk>>30, availableDisk>>20, availableDisk>>30); 


	//st_record_start(SDK_RECORD_TYPE_MANU,60);

	if(0 > get_sd_stats())//判断SD卡是否存在
	{
		  resp.total = 0;
	}
	else
	{
		resp.total = totalsize>>20;
	}
	
	resp.free = freeDisk>>20;
	printf("model=%s vendor=%s version=%d total=%d free=%d\n",
	resp.vendor,resp.vendor,resp.version,resp.total,resp.free);
	avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_DEVINFO_RESP, (char *)&resp, sizeof(SMsgAVIoctrlDeviceInfoResp));
	return ;
__error:
	avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_DEVINFO_RESP, (char *)&resp, sizeof(SMsgAVIoctrlDeviceInfoResp));
	return ;

}


    void TK_get_company_req_handle(int avIndex, char *buf) //获取公司和版本信息
    {
        SMsgAVIoctrlGetCompanyConfigResp resp;

		struct statfs diskInfo;
		
        char msg_buf[MSG_BUF_SIZE] = {0};
        sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;

        TUTK_MSG_CTRL(SDK_MAIN_MSG_SYS_CFG,SDK_PARAM_GET,0,0,pMsg);
        if(NULL == pMsg)
        {
            goto __error;
        }
        sdk_sys_cfg_t *version_msg_cfg  = (sdk_sys_cfg_t *)pMsg->data;
        strcpy(resp.model, version_msg_cfg->device_type);
        strcpy(resp.vendor, version_msg_cfg->manufacturer);

        resp.channel = avIndex;

        unsigned int version = 0,version_0 = 0,version_1 = 0,version_2 = 0,version_3 = 0;

        sscanf(version_msg_cfg->software_ver,"V%d.%d.%d.%d",&version_3,&version_2,&version_1,&version_0);
        version_3 = ((version_3&0xff) << 24) ;
        version_2 = ((version_2&0xff) << 16) ;
        version_1 = ((version_1&0xff) << 8) ;
        version_0 = ((version_0&0xff) << 0) ;
        version = version_3| version_2 | version_1 | version_0;
		info("version:%d\n", version);
        resp.version = version;
		

		/*
		  *  get SD card information    2015-07-03
		  */

		statfs(GRD_SD_MOUNT_POINT, &diskInfo);  

		unsigned long long blocksize = diskInfo.f_bsize;    //每个block里包含的字节数  
	    unsigned long long totalsize = blocksize * diskInfo.f_blocks;   //总的字节数，f_blocks为block的数目  

		info("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n",   
	        totalsize, totalsize>>10, totalsize>>20, totalsize>>30);  
	      
	    unsigned long long freeDisk = diskInfo.f_bfree * blocksize; //剩余空间的大小  
	    unsigned long long availableDisk = diskInfo.f_bavail * blocksize;   //可用空间大小  

		info("Disk_free = %llu MB = %llu GB\nDisk_available = %llu MB = %llu GB\n",   
	        freeDisk>>20, freeDisk>>30, availableDisk>>20, availableDisk>>30); 


		if(0 > get_sd_stats())//判断SD卡是否存在
		{
		      resp.total = 0;
		}
		else
		{
			resp.total = totalsize>>20;
		}
		
        resp.free = freeDisk>>20;
		
        avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_COMPANY_CONFIG_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetCompanyConfigResp));
        return ;
__error:
        avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_COMPANY_CONFIG_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetCompanyConfigResp));
        return ;

    }




    void TK_get_deviceModel_req_handle(int avIndex, char *buf)
    {
        warning("TK_get_deviceModel_req_handle start! \n");
        SMsgAVIoctrlGetDevModelConfigResp resp;

        char msg_buf[MSG_BUF_SIZE] = {0};
        sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;

        TUTK_MSG_CTRL(SDK_MAIN_MSG_SYS_CFG,SDK_PARAM_GET,0,0,pMsg);
        if(NULL == pMsg)
        {
            goto __error;
        }
        sdk_sys_cfg_t * sys_msg_cfg = (sdk_sys_cfg_t *)pMsg->data;


        strcpy(resp.deviceModel,  sys_msg_cfg->device_type);
        strcpy(resp.manufacturer, sys_msg_cfg->manufacturer);
        resp.serverMask = sys_msg_cfg->server_mask = 0Xfffff;

		printf("===>sreial2:%s\n",sys_msg_cfg->serial_2);
        printf("===>deviceModel:%s\n",resp.deviceModel);
        printf("===>manhgufacturer:%s\n",resp.manufacturer);
        printf("===>serverMask:%d\n",resp.serverMask);

        avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_DEVICEMODEL_CONFIG_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetDevModelConfigResp));
        warning("TK_get_deviceModel_req_handle end! \n");
        return;

__error:
        error("TK_get_deviceModel_req_handle failed\n");
        return;

    }



    void TK_set_deviceModel_req_handle(int avIndex, char *buf)
    {
        SMsgAVIoctrlSetDevModelConfigResp resp;
        SMsgAVIoctrlSetDevModelConfigReq *p = (SMsgAVIoctrlSetDevModelConfigReq *)buf;
        if(NULL == p)goto __error;
        char msg_buf[MSG_BUF_SIZE] = {0};
        sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
        char msg_buf_ori[MSG_BUF_SIZE] = {0};
        sdk_msg_t *pMsg_ori = (sdk_msg_t *)msg_buf_ori;
        sdk_sys_cfg_t *sys_cfg_ori = NULL;
        sdk_sys_cfg_t sys_cfg;
		//printf("deviceModel=%s manufacturer=%s\n",p->deviceModel,p->manufacturer);
        TUTK_MSG_CTRL(SDK_MAIN_MSG_SYS_CFG,SDK_PARAM_GET,0,0,pMsg_ori);
        sys_cfg_ori = (sdk_sys_cfg_t *)pMsg_ori->data;
        memcpy(&sys_cfg,sys_cfg_ori, sizeof(sdk_sys_cfg_t));

        strcpy(sys_cfg.device_type,p->deviceModel);
        strcpy(sys_cfg.manufacturer,p->manufacturer);
        sys_cfg.server_mask=p->serverMask;
        memcpy(pMsg->data,&sys_cfg,sizeof(sdk_sys_cfg_t));
		/*printf("serial_2=%s\n manufacturer=%s\n device_type=%s\n software_ver=%s\n",
			sys_cfg.serial_2,sys_cfg.manufacturer,sys_cfg.device_type,sys_cfg.software_ver);*/
       TUTK_MSG_CTRL(SDK_MAIN_MSG_SYS_CFG,SDK_PARAM_SET,0,0,pMsg);
        resp.result = 0;
        avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_DEVICEMODEL_CONFIG_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetDevModelConfigResp));
        return;
__error:
        resp.result = -1;
        avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_DEVICEMODEL_CONFIG_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetDevModelConfigResp));
        return;
    }

    void tk_set_MDSensitive_req_handle(int avIndex, char *buf)
    {
        SMsgAVIoctrlSetMotionDetectReq *p =(SMsgAVIoctrlSetMotionDetectReq *)buf;
        SMsgAVIoctrlSetMotionDetectResp resp;

        char msg_buf[MSG_BUF_SIZE] = {0};

        //sdk_motion_cfg_t *md = (sdk_motion_cfg_t *)buf;
        sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
        sdk_motion_cfg_t *md_param = (sdk_motion_cfg_t *)pMsg->data;

        TUTK_MSG_CTRL(SDK_MAIN_MSG_MOTION_CFG,SDK_PARAM_GET,0,0,pMsg);
        pMsg->size = sizeof(sdk_motion_cfg_t);
        int i =0;
        switch(p->sensitivity)
        {
        case 0:
            md_param->codec_vda_cfg.mask = 0;
            break;
        case 25:
            for(i = 1 ; i<5; i ++)
            {
                SET_BIT(md_param->codec_vda_cfg.mask,i);
            }
            md_param->codec_vda_cfg.sensitive= 2;
            break;
        case 50:
            for(i = 1 ; i<5; i ++)
            {
                SET_BIT(md_param->codec_vda_cfg.mask,i);
            }
            md_param->codec_vda_cfg.sensitive= 3;
            break;
        case 75:
            for(i = 1 ; i<5; i ++)
            {
                SET_BIT(md_param->codec_vda_cfg.mask,i);
            }
            md_param->codec_vda_cfg.sensitive= 4;
            break;
        case 100:
            for(i = 1 ; i<5; i ++)
            {
                SET_BIT(md_param->codec_vda_cfg.mask,i);
            }
            md_param->codec_vda_cfg.sensitive= 5;
            break;
        default:
            md_param->codec_vda_cfg.mask = 0;
            break;
        }
        TUTK_MSG_CTRL(SDK_MAIN_MSG_MOTION_CFG,SDK_PARAM_SET,0,0,pMsg);

		 resp.result = 0;
		 avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETMOTIONDETECT_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetMotionDetectResp));
		 return;
__error:
        printf("GET sdk_motion_cfg_t +_+_+_+_+_+_+_+_+_+_+_+_+ ERROR \n");
		resp.result = 1;
        avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETMOTIONDETECT_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetMotionDetectResp));
		return;

    }

    void tk_get_MDSensitive_req_handle(int avIndex, char *buf)
    {
        SMsgAVIoctrlGetMotionDetectResp resp;
        resp.channel = 0;

        char msg_buf[MSG_BUF_SIZE] = {0};

        //sdk_motion_cfg_t *md = (sdk_motion_cfg_t *)buf;
        sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
        sdk_motion_cfg_t *md_param = (sdk_motion_cfg_t *)pMsg->data;

        TUTK_MSG_CTRL(SDK_MAIN_MSG_MOTION_CFG,SDK_PARAM_GET,0,0,pMsg);
        if(TEST_BIT(md_param->codec_vda_cfg.mask,1))
        {
            switch(md_param->codec_vda_cfg.sensitive)
            {
            case 2:
                resp.sensitivity = 25;
                break;
            case 3:
                resp.sensitivity = 50;
                break;
            case 4:
                resp.sensitivity = 75;
                break;
            case 5:
                resp.sensitivity = 100;
                break;
            default:
                resp.sensitivity = 25;
                break;
            }

        }
        else
            resp.sensitivity = 0;
		avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETMOTIONDETECT_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetMotionDetectResp));
		return;
__error:
        printf("SET sdk_motion_cfg_t +_+_+_+_+_+_+_+_+_+_+_+_+ ERROR \n");
        avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETMOTIONDETECT_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetMotionDetectResp));
		return;
	}


    void tk_get_MDParam_req_handle(int avIndex,char *buf)
    {
        SMsgAVIoctrlGetMotionAreaResp resp;
        int i;
        char msg_buf[MSG_BUF_SIZE] = {0};

        sdk_motion_cfg_t *md = (sdk_motion_cfg_t *)buf;
        sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
        sdk_motion_cfg_t *md_param = (sdk_motion_cfg_t *)pMsg->data;

        TUTK_MSG_CTRL(SDK_MAIN_MSG_MOTION_CFG,SDK_PARAM_GET,0,0,pMsg);
        for(i = 0; i < 4; i++)
        {
            /*resp.area[i].start_x = md_param->codec_vda_cfg.area[i].x *100 /1280;
            resp.area[i].start_y = md_param->codec_vda_cfg.area[i].y *100 /720;
            resp.area[i].end_x = md_param->codec_vda_cfg.area[i].width *100 /1280;
            resp.area[i].end_y = md_param->codec_vda_cfg.area[i].height *100 /720;*/
            resp.area[i].start_x 	= md_param->codec_vda_cfg.area[i].x;
            resp.area[i].start_y 	= md_param->codec_vda_cfg.area[i].y;
            resp.area[i].end_x 		= md_param->codec_vda_cfg.area[i].width + md_param->codec_vda_cfg.area[i].x;
            resp.area[i].end_y 		= md_param->codec_vda_cfg.area[i].height+ md_param->codec_vda_cfg.area[i].y;
            if(TEST_BIT(md_param->codec_vda_cfg.mask,1))
            {
                resp.area_enable[i] = 1;
            }
            else
            {
                resp.area_enable[i] = 0;
            }
        }
		avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_MOTION_AREA_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetMotionAreaResp));
		return;
__error:
        printf("GET sdk_motion_cfg_t +_+_+_+_+_+_+_+_+_+_+_+_+ ERROR \n");
        avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_MOTION_AREA_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetMotionAreaResp));
		return;
	}

    void tk_set_MDParam_req_handle(int avIndex,char *buf)
    {
        SMsgAVIoctrlSetMotionAreaResp resp;
        //SMsgAVIoctrlSetMotionAreaReq gmotion;
        SMsgAVIoctrlSetMotionAreaReq *p = (SMsgAVIoctrlSetMotionAreaReq *)buf;
        int i;
        char msg_buf[MSG_BUF_SIZE] = {0};

        sdk_motion_cfg_t *md = (sdk_motion_cfg_t *)buf;
        sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
        sdk_motion_cfg_t *md_param = (sdk_motion_cfg_t *)pMsg->data;
        printf("==============tk_set_MDParam_req_handle====================\n");
		for(i=0;i<MAX_MOTION_NUM;i++)
		{
			printf("area_enable = %d\n",p->area_enable[i]);
			printf("start_x = %d\n",p->area[i].start_x);
			printf("start_y = %d\n",p->area[i].start_y);
			printf("end_x = %d\n",p->area[i].end_x);
			printf("end_y = %d\n",p->area[i].end_y);
		}
        TUTK_MSG_CTRL(SDK_MAIN_MSG_MOTION_CFG,SDK_PARAM_GET,0,0,pMsg);
        pMsg->size = sizeof(sdk_motion_cfg_t);
        for(i = 0; i < 4; i++)
        {
           /* md_param->codec_vda_cfg.area[i].x		= 	p ->area[i].start_x *1280/100;
            md_param->codec_vda_cfg.area[i].y 		= 	p ->area[i].start_y *720/100;
            md_param->codec_vda_cfg.area[i].width 	= 	(p->area[i].end_x*1280/100) - md_param->codec_vda_cfg.area[i].x;
            md_param->codec_vda_cfg.area[i].height 	= 	(p->area[i].end_y*720/100) - md_param->codec_vda_cfg.area[i].y;*/
		if((0 == p ->area[i].start_x)&&(0 == p ->area[i].start_y)
						&&(0 == p ->area[i].end_x)&&(0 == p ->area[i].end_y))
		{
			md_param->codec_vda_cfg.area[i].x		= 	128;
        	md_param->codec_vda_cfg.area[i].y 		= 	72;
        	md_param->codec_vda_cfg.area[i].width 	= 	1280-128 - md_param->codec_vda_cfg.area[i].x;
        	md_param->codec_vda_cfg.area[i].height 	= 	720-72 - md_param->codec_vda_cfg.area[i].y;
		}
		else
		{
	  		md_param->codec_vda_cfg.area[i].x		= 	p ->area[i].start_x;
        	md_param->codec_vda_cfg.area[i].y 		= 	p ->area[i].start_y;
        	md_param->codec_vda_cfg.area[i].width 	= 	p->area[i].end_x - md_param->codec_vda_cfg.area[i].x;
        	md_param->codec_vda_cfg.area[i].height 	= 	p->area[i].end_y - md_param->codec_vda_cfg.area[i].y;
		}
            printf("md_param->codec_vda_cfg.area[%d].x=%d\n",i,md_param->codec_vda_cfg.area[i].x);
            printf("md_param->codec_vda_cfg.area[%d].y=%d\n",i,md_param->codec_vda_cfg.area[i].y);
            printf("md_param->codec_vda_cfg.area[%d].width=%d\n",i,md_param->codec_vda_cfg.area[i].width);
            printf("md_param->codec_vda_cfg.area[%d].height=%d\n",i,md_param->codec_vda_cfg.area[i].height);

            if(p->area[i].start_x > 0 || p->area[i].start_y > 0 || p->area[i].end_x >0 || p->area[i].end_y > 0)
            {
                printf("set 1 area is %d\n",i+1);
                SET_BIT(md_param->codec_vda_cfg.mask, i+1);
            }
            else
                md_param->codec_vda_cfg.mask &= ~(1 << (i+1));
            warning("enable [%d] set area[%d][%d][%d][%d]\n",md_param->codec_vda_cfg.mask,md_param->codec_vda_cfg.area[i].x,md_param->codec_vda_cfg.area[i].y,md_param->codec_vda_cfg.area[i].width,md_param->codec_vda_cfg.area[i].height);
        }
        TUTK_MSG_CTRL(SDK_MAIN_MSG_MOTION_CFG,SDK_PARAM_SET,0,0,pMsg);
        avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_MOTION_AREA_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetAlarmArgResp));
		return ;


__error:
        printf("SET sdk_motion_cfg_t +_+_+_+_+_+_+_+_+_+_+_+_+ ERROR \n");
        avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_MOTION_AREA_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetAlarmArgResp));
		return ;
    }

void tk_set_streamctrl(int avIndex, char *buf)
{
   // int i;
    SMsgAVIoctrlGetStreamCtrlResq resp;
    SMsgAVIoctrlSetStreamCtrlReq *p = (SMsgAVIoctrlSetStreamCtrlReq *)buf;

    resp.channel=0;
    resp.quality = p->quality;


    char msg_buf[MSG_BUF_SIZE] = {0};

    sdk_encode_t *enc = (sdk_encode_t *)buf;
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
    sdk_encode_t *enc_param = (sdk_encode_t *)pMsg->data;


    TUTK_MSG_CTRL(SDK_MAIN_MSG_ENCODE_CFG,SDK_PARAM_GET,0,0,pMsg);
    pMsg->size = sizeof(sdk_encode_t);
    warning("QJZ CHN[%d]  p-> quility[%d]\n", p->channel,p->quality);
    if( p->quality == 1) //MAX   720p 25FPS 800k
    {
        enc_param->av_enc_info[0].frame_rate	= 25;
        enc_param->av_enc_info[0].bitrate 		= 800;
		enc_param->av_enc_info[0].is_using 		= 1;
		enc_param->av_enc_info[1].is_using 		= 0;
    }

    else if( p->quality == 2) //HIGN  720p 20FPS 400K
    {
        enc_param->av_enc_info[0].frame_rate	= 20;
        enc_param->av_enc_info[0].bitrate 		= 400;
		enc_param->av_enc_info[0].is_using		= 1;
		enc_param->av_enc_info[1].is_using		= 0;
    }
    else if( p->quality == 3)//MED  VGA 25fsp 256K
    {
        enc_param->av_enc_info[1].frame_rate	= 25;
        enc_param->av_enc_info[1].bitrate 		= 256;
		enc_param->av_enc_info[0].is_using		= 0;
		enc_param->av_enc_info[1].is_using		= 1;
    }
    else if( p->quality == 4)//LOW VGA 15fsp 256K
    {
        enc_param->av_enc_info[1].frame_rate	= 15;
        enc_param->av_enc_info[1].bitrate 		= 256;
		enc_param->av_enc_info[0].is_using		= 0;
		enc_param->av_enc_info[1].is_using		= 1;
    }
    else if( p->quality == 5)//MIN vga 10FPS 128k
    {
        enc_param->av_enc_info[1].frame_rate	= 10;
        enc_param->av_enc_info[1].bitrate 		= 128;
		enc_param->av_enc_info[0].is_using		= 0;
		enc_param->av_enc_info[1].is_using		= 1;
    }
	else //OTHER
    {
        enc_param->av_enc_info[0].frame_rate	= 25;
        enc_param->av_enc_info[0].bitrate 		= 800;
		enc_param->av_enc_info[0].is_using 		= 1;
		enc_param->av_enc_info[1].is_using 		= 0;
    }
	#if 0
	printf("-----------------SECOND------------------------\n");
	for(i=0;i<2;i++)
	{
		printf("resolution	= %d\n",enc_param->av_enc_info[i].resolution);
		printf("bitrate_type= %s\n",(enc_param->av_enc_info[i].bitrate_type == 0)? "CBR":"VBR");
		printf("pic_quilty	= %d\n",enc_param->av_enc_info[i].pic_quilty);
		printf("frame_rate	= %d\n",enc_param->av_enc_info[i].frame_rate);
		printf("gop 		= %d\n",enc_param->av_enc_info[i].gop);
		printf("video_type	= %s\n",(enc_param->av_enc_info[i].video_type == 0)?"H264":"OTHER");
		printf("mix_type	= %s\n",(enc_param->av_enc_info[i].mix_type == 1)?"VIDEO":"AUDIO");
		printf("bitrate 	= %d\n",enc_param->av_enc_info[i].bitrate);
		printf("level		= %d\n",enc_param->av_enc_info[i].level);
		printf("h264_ref_mod= %d\n",enc_param->av_enc_info[i].h264_ref_mod);	
		printf("is_using	= %d\n",enc_param->av_enc_info[i].is_using);
	}
	printf("--------------------END-------------------------\n");
	#endif

	TUTK_MSG_CTRL(SDK_MAIN_MSG_ENCODE_CFG,SDK_PARAM_SET,0,0,pMsg);
	
	if(1 == enc_param->av_enc_info[0].is_using)
	{
		g_tutk_ls.encode_current_chn = 0; //编码0通道 ==720P
	}
	else if(1 == enc_param->av_enc_info[1].is_using)
	{
		g_tutk_ls.encode_current_chn = 1; //编码1通道 ==VGA

	}
	else
	{
		g_tutk_ls.encode_current_chn = 0; //编码0通道 ==720P
	}
    resp.quality = p->quality;
    resp.channel= avIndex;
	avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETSTREAMCTRL_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetStreamCtrlResq));
	return;
__error:
    printf("SET sdk_encode_t +_+_+_+_+_+_+_+_+_+_+_+_+ ERROR \n");
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETSTREAMCTRL_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetStreamCtrlResq));
	return;



}


void tk_get_streamctrl(int avIndex, char *buf)
{
    SMsgAVIoctrlGetStreamCtrlResq res;
    res.channel = 0;
	int i;
    char msg_buf[MSG_BUF_SIZE] = {0};

    sdk_encode_t *enc = (sdk_encode_t *)buf;
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
    sdk_encode_t *enc_param = (sdk_encode_t *)pMsg->data;

    TUTK_MSG_CTRL(SDK_MAIN_MSG_ENCODE_CFG,SDK_PARAM_GET,0,0,pMsg);
#if 0
    if(enc_param->av_enc_info[0].frame_rate== 30 && enc_param->av_enc_info[0].bitrate == 2048)
    {
        res.quality = 10;
    }
    else if(enc_param->av_enc_info[0].frame_rate== 25 && enc_param->av_enc_info[0].bitrate == 2048)
    {
        res.quality = 11;
    }
    else if(enc_param->av_enc_info[0].frame_rate== 30 && enc_param->av_enc_info[0].bitrate == 1024)
    {
        res.quality = 12;
    }
    else if(enc_param->av_enc_info[0].frame_rate== 25 && enc_param->av_enc_info[0].bitrate == 1024)
    {
        res.quality = 13;
    }
    else
    {
        res.quality = 14;
    }
    warning("Get quality=%d \n",res.quality);
	
	int qlt = res.quality;
	
	if(qlt == 14)res.quality = 5;
	else if(qlt == 13)res.quality = 4;
	else if(qlt == 12)res.quality = 3;
	else if(qlt == 11)res.quality = 2;
	else if(qlt == 10)res.quality = 1;
	#endif
	printf("tk_get_streamctrl:\n");
	for(i=0;i<2;i++)
	{
		printf("resolution	= %d\n",enc_param->av_enc_info[i].resolution);
		printf("bitrate_type= %d %s\n",enc_param->av_enc_info[i].bitrate_type,(enc_param->av_enc_info[i].bitrate_type == 0)? "CBR":"VBR");
		printf("pic_quilty	= %d\n",enc_param->av_enc_info[i].pic_quilty);
		printf("frame_rate	= %d\n",enc_param->av_enc_info[i].frame_rate);
		printf("gop 		= %d\n",enc_param->av_enc_info[i].gop);
		printf("video_type	= %s\n",(enc_param->av_enc_info[i].video_type == 0)?"H264":"OTHER");
		printf("mix_type	= %s\n",(enc_param->av_enc_info[i].mix_type == 1)?"VIDEO":"AUDIO");
		printf("bitrate 	= %d\n",enc_param->av_enc_info[i].bitrate);
		printf("level		= %d\n",enc_param->av_enc_info[i].level);
		printf("h264_ref_mod= %d\n",enc_param->av_enc_info[i].h264_ref_mod);	
		printf("is_using	= %d\n",enc_param->av_enc_info[i].is_using);
	}
	
	if(1 == enc_param->av_enc_info[0].is_using)
	{
		if((800 == enc_param->av_enc_info[0].bitrate)  //720p 800K 25fps
				&&(25 == enc_param->av_enc_info[0].frame_rate)
				&&(SDK_VIDEO_RESOLUTION_720p == enc_param->av_enc_info[0].resolution))
		{
			res.quality = AVIOCTRL_QUALITY_MAX;
		}
		else if((400 == enc_param->av_enc_info[0].bitrate)  //720p 400K 20fps
				&&(20 == enc_param->av_enc_info[0].frame_rate)
				&&(SDK_VIDEO_RESOLUTION_720p == enc_param->av_enc_info[0].resolution))
		{
			res.quality = AVIOCTRL_QUALITY_HIGH;
		}
	}
	else if(1 == enc_param->av_enc_info[1].is_using)
	{
		if((256 == enc_param->av_enc_info[1].bitrate) //VGA 256K 25FPS
				&&(25 == enc_param->av_enc_info[1].frame_rate)
				&&(SDK_VIDEO_RESOLUTION_VGA == enc_param->av_enc_info[1].resolution))
		{
			res.quality = AVIOCTRL_QUALITY_MIDDLE;
		}
		else if((256 == enc_param->av_enc_info[1].bitrate)  //VGA 256K 15FPS
				&&(15 == enc_param->av_enc_info[1].frame_rate)
				&&(SDK_VIDEO_RESOLUTION_VGA == enc_param->av_enc_info[1].resolution))
		{
			res.quality = AVIOCTRL_QUALITY_LOW;
		}
		else if((128 == enc_param->av_enc_info[1].bitrate)  //VGA 128K 10FPS
			&&(10 == enc_param->av_enc_info[1].frame_rate)
			&&(SDK_VIDEO_RESOLUTION_VGA == enc_param->av_enc_info[1].resolution))
		{
			res.quality = AVIOCTRL_QUALITY_MIN;
		}
	}
	avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETSTREAMCTRL_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetStreamCtrlResq));
	return;
__error:
    printf("GET sdk_encode_t +_+_+_+_+_+_+_+_+_+_+_+_+ ERROR \n");
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETSTREAMCTRL_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetStreamCtrlResq));
	return;
}


//end

// 修改用户密码
void tk_set_userMsg_req_handle(int avIndex, char *buf)
{
    SMsgAVIoctrlSetPasswdResp resp;
    SMsgAVIoctrlSetPasswdReq *pswd = (SMsgAVIoctrlSetPasswdReq *)buf;
    sdk_user_right_t right;
    memcpy(right.user.user_name,"admin",sizeof("admin"));
    memcpy(right.user.user_pwd,pswd->newpasswd,sizeof(pswd->newpasswd));

    unsigned char *ptmp = (unsigned char *)&right;
    char msg_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;

    memcpy(pMsg->data,ptmp,sizeof(sdk_user_right_t));
    printf("=====11111111\n");

    printf("========pswd->oldpasswd = %s\n",pswd->oldpasswd);
    printf("========pswd->newpasswd = %s\n",pswd->newpasswd);



    printf("========pMsg->data = %s\n",pMsg->data);
    TUTK_MSG_CTRL(SDK_MAIN_MSG_USER,SDK_USER_MODI,0,0,pMsg);
    resp.result = 0;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETPASSWORD_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetPasswdResp));
	return;
__error:
    resp.result = -1;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETPASSWORD_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetPasswdResp));
	return;
}

/*
  * Format SD card 2015-07-03
  */

void tk_format_extStorage_req_handle(int avIndex, char *buf)
{
	SMsgAVIoctrlFormatExtStorageResp resp;
	SMsgAVIoctrlFormatExtStorageReq *p = (SMsgAVIoctrlFormatExtStorageReq *)buf;
	
	//格式化之前停掉所有录像
	if(0 != p->storage)
	{
		return NULL;
	}
	if(0 == st_sd_card_format())
	{
		resp.result = 0;
		info("external storage was Formated\n");
	}
	else
	{
		resp.result = -1;
	}
	resp.storage = p->storage;
	printf("resp.result=%d\n",resp.result);
	avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_FORMATEXTSTORAGE_RESP, (char *)&resp, sizeof(SMsgAVIoctrlFormatExtStorageResp));

}
void tk_set_timemode_cfg(int avIndex, char *buf)
{
    SMsgAVIoctrlSetTimeModeConfigResp resp;
    SMsgAVIoctrlSetTimeModeConfigReq *p = (SMsgAVIoctrlSetTimeModeConfigReq *)buf;
    SMsgAVIoctrlSetTimeModeConfigReq g_timemode_cfg;
    printf("set time mode is%d\n",p->mode);


    char msg_buf[MSG_BUF_SIZE] = {0};

    sdk_osd_cfg_t *osd =(sdk_osd_cfg_t *)buf;
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
    sdk_osd_cfg_t *osd_param = (sdk_osd_cfg_t *)pMsg->data;

    TUTK_MSG_CTRL(SDK_MAIN_MSG_OSD_CFG,SDK_PARAM_GET,0,0,pMsg);
    pMsg->size = sizeof(sdk_osd_cfg_t);

    osd_param->time.format = p->mode;
    TUTK_MSG_CTRL(SDK_MAIN_MSG_OSD_CFG,SDK_PARAM_SET,0,0,pMsg);
    resp.result = 0;
	avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_TIMEMODE_TO_SHARE_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetTimeModeConfigResp));
	return ;
__error:
    printf("SET osd_param +_+_+_+_+_+_+_+_+_+_+_+_+ ERROR \n");
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_TIMEMODE_TO_SHARE_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetTimeModeConfigResp));
	return ;
}

void tk_get_timemode_cfg(int avIndex, char *buf)
{
    SMsgAVIoctrlSetTimeModeConfigReq g_timemode_cfgs;

    char msg_buf[MSG_BUF_SIZE] = {0};

    sdk_osd_cfg_t *osd =(sdk_osd_cfg_t *)buf;
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
    sdk_osd_cfg_t *osd_param = (sdk_osd_cfg_t *)pMsg->data;

    TUTK_MSG_CTRL(SDK_MAIN_MSG_OSD_CFG,SDK_PARAM_GET,0,0,pMsg);
    pMsg->size = sizeof(sdk_osd_cfg_t);
    g_timemode_cfgs.mode = osd_param->time.format;

    printf("get time mode [%d] to client \n",g_timemode_cfgs.mode);
	avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_TIMEMODE_TO_SHARE_RESP, (char *)&g_timemode_cfgs, sizeof(g_timemode_cfgs));
	return ;
__error:
    printf("GET osd_param +_+_+_+_+_+_+_+_+_+_+_+_+ ERROR \n");
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_TIMEMODE_TO_SHARE_RESP, (char *)&g_timemode_cfgs, sizeof(g_timemode_cfgs));
	return ;
}

void tk_set_ledStatus_req_handle(int avIndex, char *buf)
{

    printf("=======tk_set_ledStatus_req_handle======\n");
    SMsgAVIoctrlSetLedStatusResp resp;
    SMsgAVIoctrlSetLedStatusReq *led = (SMsgAVIoctrlSetLedStatusReq *)buf;

	char msg_buf[MSG_BUF_SIZE] = {0};
	sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
	sdk_motion_cfg_t *md_param = (sdk_motion_cfg_t *)pMsg->data;
	TUTK_MSG_CTRL(SDK_MAIN_MSG_MOTION_CFG,SDK_PARAM_GET,0,0,pMsg);
	printf("led[0]=%d led[1]=%d led[2]=%d led[3]=%d\n",led->led_status[0],led->led_status[1],led->led_status[2],led->led_status[3]);
	if((1 == led->led_status[0])||(1 == led->led_status[1])||(1 == led->led_status[2])||(1 == led->led_status[3]))
	{	
		md_param->alarm_handle.led_enable = 1;
	}
	else if((0 == led->led_status[0])||(0 == led->led_status[1])||(0 == led->led_status[2])||(0 == led->led_status[3]))
	{	
		md_param->alarm_handle.led_enable = 0;
	}
	printf("led_buzzer_info.led_status1 =0x%x\n",led_buzzer_info.led_status);

	if(1 == md_param->alarm_handle.led_enable)
	{
		pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
		led_buzzer_info.led_status &= ~(1<<LED_FORCE_CLOSE);
		pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);
	}
	else if(0 == md_param->alarm_handle.led_enable)
	{
		pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
		led_buzzer_info.led_status |= 1<<LED_FORCE_CLOSE;
		pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);
	}
	printf("led_buzzer_info.led_status2 =0x%x\n",led_buzzer_info.led_status);
	TUTK_MSG_CTRL(SDK_MAIN_MSG_MOTION_CFG,SDK_PARAM_SET,0,0,pMsg);

    resp.result = 0;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_LED_STATUS_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetLedStatusResp));
	return;
__error:
    resp.result = -1;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_LED_STATUS_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetLedStatusResp));
	return;
}
void tk_get_ledStatus_req_handle(avIndex, buf)
{
	int i;
    SMsgAVIoctrlGetLedStatusResp resp;
	char msg_buf[MSG_BUF_SIZE] = {0};
	sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
	sdk_motion_cfg_t *md_param = (sdk_motion_cfg_t *)pMsg->data;
	TUTK_MSG_CTRL(SDK_MAIN_MSG_MOTION_CFG,SDK_PARAM_GET,0,0,pMsg);
	if(1 == md_param->alarm_handle.led_enable)
	{
		for(i=0;i<MAX_LED_NUM;i++)
		{
			resp.led_status[i]= 1;
		}
	}else if(0 == md_param->alarm_handle.led_enable)
	{
		for(i=0;i<MAX_LED_NUM;i++)
		{
			resp.led_status[i]= 0;
		}
	}
   avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_LED_STATUS_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetLedStatusResp));
	return ;
__error:
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_LED_STATUS_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetLedStatusResp));
	return ;
}
void tk_set_buzStatus_req_handle(avIndex, buf)
{
    SMsgAVIoctrlSetBuzzerStatusResp resp;
    SMsgAVIoctrlSetBuzzerStatusReq *buzzer = (SMsgAVIoctrlSetBuzzerStatusReq *)buf;
	//////////////////////////
	printf("========gpio_buzzer_set panic=====\n");
	pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
	led_buzzer_info.led_status |= (1<<LED_PANIC);
	led_buzzer_info.led_time = buzzer->buzzer_time;
	led_buzzer_info.buzzer_status |= (1<<BUZZER_TESTING);
	led_buzzer_info.buzzer_time = buzzer->buzzer_time;
	pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);

    resp.result = 0;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_BUZZER_STATUS_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetBuzzerStatusResp));
    return ;
}
void tk_get_buzStatus_req_handle(avIndex, buf)
{
    SMsgAVIoctrlGetBuzzerStatusResp resp;

    sdk_iodev_param_t *iodev_ori;

    char msg_buf_get[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pMsg_get = (sdk_msg_t *)msg_buf_get;
    TUTK_MSG_CTRL(SDK_MAIN_MSG_IODEV_PARAM,SDK_PARAM_GET,0,0,pMsg_get);
    iodev_ori = (sdk_iodev_param_t *)pMsg_get->data;
    resp.buzzer_status=iodev_ori->buzzer_param.buzzer;
    resp.buzzer_time=15;
    //缺少buzzer time
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_LED_STATUS_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetBuzzerStatusResp));
	return;
__error:
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_LED_STATUS_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetBuzzerStatusResp));
	return;
}

void tk_get_alarmArg_req_handle_EX(int avIndex, char *buf)
{
    SMsgAVIoctrlGetAlarmArgResp resp;

    printf("========tk_get_alarmArg_req_handle_EX=======\n");
    char msg_buf[MSG_BUF_SIZE] = {0};

    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
    sdk_motion_cfg_t *md_param = (sdk_motion_cfg_t *)pMsg->data;

    TUTK_MSG_CTRL(SDK_MAIN_MSG_MOTION_CFG,SDK_PARAM_GET,0,0,pMsg);
    printf("========md_param->codec_vda_cfg.sensitive=%d\n",md_param->codec_vda_cfg.sensitive);
    resp.md_sensitivity = md_param->codec_vda_cfg.sensitive;
    printf("========111111111=======\n");

#if 1	
    resp.sd_sensitivity 	= 	0;//md_param->codec_vda_cfg.sensitive;	//声音侦测灵敏度（0表示禁用 1 2 3）
    resp.buzzer_enable		=	md_param->alarm_handle.beep_enable;	//蜂鸣器开关（0表示禁用 1 使能）
    resp.md_time			=	md_param->sched_time.interval_time;			//移动侦测间隔时间  0 到 120 秒
    resp.audio_time			=	0;		//声音检测间隔时间  0 到 120 秒
    resp.buzzer_time		=	md_param->alarm_handle.beep_time;//5;		//蜂鸣器持续时间  0 到 120 秒
    resp.alarm_record		=	md_param->alarm_handle.record_enable;		//报警时是否录像至SD卡
	resp.alarm_record_time	=	md_param->alarm_handle.record_time;
	resp.alarm_snapshot		=	md_param->alarm_handle.snap_enable;	//报警时是否拍照至SD卡
    resp.alarm_mail			=	md_param->alarm_handle.email_enable;		//报警时是否拍照至email
#endif

    printf("========2222222222222=======\n");
    memcpy(buf ,(char *)&resp,sizeof(SMsgAVIoctrlGetAlarmArgResp));
    //avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_ALARM_ARG_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetAlarmArgResp));
__error:
    printf("GET sdk_motion_cfg_t +_+_+_+_+_+_+_+_+_+_+_+_+ ERROR \n");
    //avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_ALARM_ARG_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetAlarmArgResp));
}

void tk_get_alarm_param(LED_BUZZER_INFO *info)
{
   sdk_motion_cfg_t md_param;
   adapt_param_get_motion_cfg(0, &md_param);
   printf("beep_enable=%d\n",md_param.alarm_handle.beep_enable);
	pthread_mutex_lock(&info->led_buzzer_lock);
	if( 0 == md_param.codec_vda_cfg.sensitive ) //移动侦测禁止时
	{
		info->arm_status  	= ARM_DISABLE;
		info->buzzer_status = 0;//1<<BUZZER_IS_ENABLE; 
		info->buzzer_time 	= 0;
		info->led_status 	|= 1<<LED_NORMAL; //只是没有布防 其他还正常
		info->led_time		= 0;
	}
	else if(md_param.codec_vda_cfg.sensitive > 0) 
	{
		info->arm_status  	= ARM_ENABLE;
		if(1 == md_param.alarm_handle.beep_enable)
		{
			info->buzzer_status = 1<<BUZZER_IS_ENABLE;
		}
		info->buzzer_time 	= md_param.alarm_handle.beep_time;
		info->led_status 	|= (1<<LED_IS_ENABLE) | (1<<LED_NORMAL) | (1<<LED_ARM);
		info->led_time		= md_param.alarm_handle.beep_time;
	}

	if(0 == md_param.alarm_handle.led_enable)
	{
		info->led_status 	|=  1<<LED_FORCE_CLOSE;
	}

	//上电时WIFI肯定是断网的 
	info->led_status 	|=  1<<LED_NO_WIFI;
	pthread_mutex_unlock(&info->led_buzzer_lock);
}

void tk_get_alarmArg_req_handle(avIndex, buf)
{
    SMsgAVIoctrlGetAlarmArgResp resp;

    printf("========tk_get_alarmArg_req_handle=======\n");
    char msg_buf[MSG_BUF_SIZE] = {0};

    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
    sdk_motion_cfg_t *md_param = (sdk_motion_cfg_t *)pMsg->data;

    TUTK_MSG_CTRL(SDK_MAIN_MSG_MOTION_CFG,SDK_PARAM_GET,0,0,pMsg);
    printf("========md_param->codec_vda_cfg.sensitive=%d\n",md_param->codec_vda_cfg.sensitive);
    if(TEST_BIT(md_param->codec_vda_cfg.mask,1))
    {
        resp.md_sensitivity = md_param->codec_vda_cfg.sensitive;	
    }
    else
	{
        resp.md_sensitivity = 0;
    }
	//全局变量的设置在SET里面设置，不在GET里面设置
	//g_alarm_rec_time=md_param->alarm_handle.record_time;
	//g_buzPra.buzzer_time=md_param->alarm_handle.beep_time;

    resp.sd_sensitivity 	= 	0;//md_param->codec_vda_cfg.sensitive;	//声音侦测灵敏度（0表示禁用 1 2 3）
    resp.buzzer_enable		=	md_param->alarm_handle.beep_enable;	//蜂鸣器开关（0表示禁用 1 使能）
    resp.md_time			=	md_param->sched_time.interval_time;			//移动侦测间隔时间  0 到 120 秒
    resp.audio_time			=	0;		//声音检测间隔时间  0 到 120 秒
    resp.buzzer_time		=	md_param->alarm_handle.beep_time;//5;		//蜂鸣器持续时间  0 到 120 秒
    resp.alarm_record		=	md_param->alarm_handle.record_enable;		//报警时是否录像至SD卡
	resp.alarm_record_time	=	md_param->alarm_handle.record_time;
	resp.alarm_snapshot		=	md_param->alarm_handle.snap_enable;	//报警时是否拍照至SD卡
    resp.alarm_mail			=	md_param->alarm_handle.email_enable;		//报警时是否拍照至email
    

    printf("========tk_get_alarmArg_req_handle=======\n");
	printf("md_sensitivity	=%d\n",resp.md_sensitivity);
	printf("sd_sensitivity	=%d\n",resp.sd_sensitivity);
	printf("buzzer_enable	=%d\n",resp.buzzer_enable);
	printf("md_time			=%d\n",resp.md_time);
	printf("audio_time		=%d\n",resp.audio_time);
	printf("buzzer_time		=%d\n",resp.buzzer_time);
	printf("alarm_record	=%d\n",resp.alarm_record);
	printf("alarm_record_time=%d\n",resp.alarm_record_time);
	printf("alarm_snapshot	=%d\n",resp.alarm_snapshot);
	printf("alarm_mail		=%d\n",resp.alarm_mail);

	printf("tutk1  g_motion_pra.motion_Enable=%d\n",g_motion_pra.motion_Enable);
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_ALARM_ARG_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetAlarmArgResp));
	printf("tutk2  g_motion_pra.motion_Enable=%d\n",g_motion_pra.motion_Enable);
	return;
__error:
    printf("GET sdk_motion_cfg_t +_+_+_+_+_+_+_+_+_+_+_+_+ ERROR \n");
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_ALARM_ARG_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetAlarmArgResp));
	return;
}

void tk_set_alarmArg_req_handle(avIndex, buf)
{
#if 1
    SMsgAVIoctrlSetAlarmArgResp resp;
    resp.result =  -1;

    SMsgAVIoctrlSetAlarmArgReq *setAlarm = (SMsgAVIoctrlSetAlarmArgReq *)buf;

	printf("Set========tk_Set_alarmArg_req_handle=======Set\n");
	printf("md_sensitivity	=%d\n",setAlarm->md_sensitivity);
	printf("sd_sensitivity	=%d\n",setAlarm->sd_sensitivity);
	printf("buzzer_enable	=%d\n",setAlarm->buzzer_enable);
	printf("md_time			=%d\n",setAlarm->md_time);
	printf("audio_time		=%d\n",setAlarm->audio_time);
	printf("buzzer_time		=%d\n",setAlarm->buzzer_time);
	printf("alarm_record	=%d\n",setAlarm->alarm_record);
	printf("alarm_record_time=%d\n",setAlarm->alarm_record_time);
	printf("alarm_snapshot	=%d\n",setAlarm->alarm_snapshot);
	printf("alarm_mail		=%d\n",setAlarm->alarm_mail);

	printf("========tk_set_alarmArg_req_handle=======\n");
   
    int i;
    char msg_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
    sdk_motion_cfg_t *md_param = (sdk_motion_cfg_t *)pMsg->data;
    TUTK_MSG_CTRL(SDK_MAIN_MSG_MOTION_CFG,SDK_PARAM_GET,0,0,pMsg);
	
	//移动侦测灵敏度
	//屏蔽掉 只用PIR
	md_param->codec_vda_cfg.sensitive	= setAlarm->md_sensitivity;

	//声音侦测灵敏度和时间  暂不设置sd_sensitivity  audio_time

	//设置报警间隔时间
	md_param->sched_time.interval_time 	= setAlarm->md_time ;
	
	//设置BUZZER 参数  是否响铃 和响铃持续时间
	md_param->alarm_handle.beep_enable	= setAlarm->buzzer_enable;
	//g_buzPra.buzzer_flag = setAlarm->buzzer_enable ;//qjz 布防撤防时同样要对全局变量置位
	md_param->alarm_handle.beep_time = setAlarm->buzzer_time; 
	printf("md_param->alarm_handle.beep_time = %d\n",md_param->alarm_handle.beep_time);
	//g_buzPra.buzzer_time = setAlarm->buzzer_time;
	
	//设置是否报警录像和报警录像时长
	md_param->alarm_handle.record_enable	= setAlarm->alarm_record ; 

//设置录像报警时长
//对参数进行限制 IOS app撤防时录像时长有问题
//撤防时不设置时间 之前是多少就是多少

	pthread_mutex_lock(&record_status.record_lock);
	record_status.md_interval_time	= md_param->sched_time.interval_time;
	pthread_mutex_unlock(&record_status.record_lock);


	if((30 == setAlarm->alarm_record_time)
					|| (60  == setAlarm->alarm_record_time)
					|| (120 == setAlarm->alarm_record_time)
					|| (240 == setAlarm->alarm_record_time))
	{
		md_param->alarm_handle.record_time = setAlarm->alarm_record_time; 
		pthread_mutex_lock(&record_status.record_lock);
		record_status.record_alarm_time = setAlarm->alarm_record_time ;
		pthread_mutex_unlock(&record_status.record_lock);

		pthread_mutex_lock(&aws.aws_lock);
		aws.alarm_time = setAlarm->alarm_record_time;
		pthread_mutex_unlock(&aws.aws_lock);
		
	}
	else 
	{//为其他值时是撤防状态，
	}
	printf("aws.alarm_time=%d\n",aws.alarm_time);
	//设置是否抓拍 是否发EMAIL
    md_param->alarm_handle.email_enable = setAlarm->alarm_mail;
    md_param->alarm_handle.snap_enable  = setAlarm->alarm_snapshot;
 	//   md_param->alarm_handle.alarm_out_time= setAlarm->audio_time;

    md_param->sched_time.enable = 1;
    md_param->sched_time.plan_flag = 0;
    //md_param->sched_time.interval_time= 10;
    int x = 0;
    int j = 0;
    for(x = 0; x<7; x++)
    {
        for(j = 0; j<5; j++)
        {
            md_param->sched_time.week_sched.week_sched_info[x][j].enable = 1;
            md_param->sched_time.week_sched.week_sched_info[x][j].start_hour = 0;
            md_param->sched_time.week_sched.week_sched_info[x][j].start_min = 0;
            md_param->sched_time.week_sched.week_sched_info[x][j].stop_hour = 23;
            md_param->sched_time.week_sched.week_sched_info[x][j].stop_min = 59;
        }
    }

    //md_param->alarm_handle. = setAlarm->sd_sensitivity;
    //移动侦测 区域上下左右各留出16个像素
   // md_param->codec_vda_cfg.area[0].x = 16;
   // md_param->codec_vda_cfg.area[0].y = 16;
   // md_param->codec_vda_cfg.area[0].width = 1280-32;
  //  md_param->codec_vda_cfg.area[0].height = 720-32;

    md_param->codec_vda_cfg.enable 	= 1;
    md_param->codec_vda_cfg.mask 	= 0xFFFF;
    md_param->codec_vda_cfg.mode 	= 0;
    md_param->codec_vda_cfg.res 	= 0;

	printf("========setAlarm->md_sensitivity=%d\n",setAlarm->md_sensitivity);
	if(0 < setAlarm->md_sensitivity )
	{
    	g_motion_pra.motion_Enable = 1;
		pir_event.alarm_enable |= setAlarm->md_sensitivity;
		pir_event.alarm_enable |= 0x10000;
		pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
		led_buzzer_info.led_status |= (1<<LED_ARM);
		led_buzzer_info.led_time = setAlarm->buzzer_time;
		pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);
		printf("led_buzzer_info.led_time=%d %d\n",led_buzzer_info.led_time,setAlarm->buzzer_time);
	}
	else
	{
		g_motion_pra.motion_Enable = 0;
		pir_event.alarm_enable = 0;
		pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
		led_buzzer_info.led_status &= ~(1<<LED_ARM);
		pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);
	}
//设置BUZZER
	if(1 == setAlarm->buzzer_enable)
	{
		pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
		led_buzzer_info.buzzer_status 	|= (1<<BUZZER_IS_ENABLE);
		led_buzzer_info.buzzer_time 	 = setAlarm->buzzer_time;
		pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);
	}
	else
	{
		pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
		led_buzzer_info.buzzer_status 	&= ~(1<<BUZZER_IS_ENABLE);
		pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);
	}
    printf("========led_buzzer_info.led_status=0x%x\n",led_buzzer_info.led_status);
	printf("========led_buzzer_info.buzzer_status=0x%x\n",led_buzzer_info.buzzer_status);
    TUTK_MSG_CTRL(SDK_MAIN_MSG_MOTION_CFG,SDK_PARAM_SET,0,0,pMsg);
    printf("========md_param->codec_vda_cfg.sensitive=%d\n",md_param->codec_vda_cfg.sensitive);
    resp.result =  0;
	
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_ALARM_ARG_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetAlarmArgResp));
	return;
__error:
    printf("SET sdk_motion_cfg_t +_+_+_+_+_+_+_+_+_+_+_+_+ ERROR \n");
    resp.result =  -1;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_ALARM_ARG_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetAlarmArgResp));
	return;
#endif
}

void TK_get_ntp_req_handle(int avIndex, char *buf)
{

	ntp_conf g_ntp_pra;
    SMsgAVIoctrlGetNtpConfigResp resp;

    char msg_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pmsg = (sdk_msg_t *)msg_buf;



    TUTK_MSG_CTRL(SDK_MAIN_MSG_SYSTIME,SDK_PARAM_GET,0,0,pmsg);
    if(NULL == pmsg)
    {
        goto __error;
    }
    sdk_date_t * time_cfg = (sdk_date_t*)pmsg->data;

    resp.TimeZone = time_cfg->tzone;
    resp.time.year = time_cfg->_time.year;
    resp.time.month = time_cfg->_time.mon;
    resp.time.date = time_cfg->_time.day+8;

    /*if(1 == g_ntp_pra.is_ntp_or_sys)
    {
    	resp.time.hour = time_cfg->_time.hour + 8;
    }
    else*/
    {
        resp.time.hour = time_cfg->_time.hour;
    }
    resp.time.minute = time_cfg->_time.min;
    resp.time.second = time_cfg->_time.sec;
    printf("INIT=GET== (%d) %d-%d-%d   %d:%d:%d==\n",resp.TimeZone,resp.time.year,\
         resp.time.month ,resp.time.date,resp.time.hour,\
         resp.time.minute,resp.time.second);


    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_NTP_CONFIG_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetNtpConfigResp));
    return;
__error:
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_NTP_CONFIG_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetNtpConfigResp));
    return ;
}

    void TK_set_ntp_req_handle(int avIndex, char *buf)
    {
		printf("===========TK_set_ntp_req_handle\n");
        SMsgAVIoctrlSetNtpConfigResp set_resp;

        SMsgAVIoctrlGetNtpConfigResp get_resp;

        SMsgAVIoctrlSetNtpConfigReq *p = (SMsgAVIoctrlSetNtpConfigReq *)buf;

        char msg_buf[MSG_BUF_SIZE] = {0};

        sdk_msg_t *pmsg = (sdk_msg_t *)msg_buf;
        if(NULL == p)
        {
            goto __error;
        }

        sdk_date_t time_cfg;
		printf("mod=%d server=%s TimeZone=%d\n",p->mod,p->Server,p->TimeZone);
		printf("===========TK_set_ntp_req_handle1%d - %d - %d - %d - %d - %d\n",
			p->time.year,p->time.month,p->time.date,p->time.hour,p->time.minute,p->time.second);

		memset(&time_cfg, 0, sizeof(sdk_date_t));
        int i = 0;
        switch(p->mod)
        {
        	case 1:				 //自动获取ntp时间
        
            pthread_mutex_lock(&g_ntp_pra.lock);
           	g_ntp_pra.mod = p->mod;
			g_ntp_pra.TimeZone = p->TimeZone;
		   	strcpy(g_ntp_pra.Server,p->Server);
            pthread_mutex_unlock(&g_ntp_pra.lock);

			memset(pmsg, 0, sizeof(sdk_msg_t));
            TUTK_MSG_CTRL(SDK_MAIN_MSG_SYSTIME,SDK_PARAM_GET,0,0,pmsg);
			sdk_date_t *cfg = (sdk_date_t *)pmsg->data;
			cfg->tzone = p->TimeZone;
			cfg->ntp_or_manual = p->mod;
			TUTK_MSG_CTRL(SDK_MAIN_MSG_SYSTIME,SDK_PARAM_SET,0,0,pmsg);
            break;

        case 2:  //手动设置时间
        
		printf("===========TK_set_ntp_req_handle4\n");
            pthread_mutex_lock(&g_ntp_pra.lock);
            g_ntp_pra.mod = 2; //
            g_ntp_pra.is_ntp_or_sys = 2;
            pthread_mutex_unlock(&g_ntp_pra.lock);
			usleep(100000);
			time_cfg.tzone 			= p->TimeZone;
			time_cfg._time.year 	= p->time.year;
			time_cfg._time.mon 		= p->time.month;
			time_cfg._time.day 		= p->time.date;
			time_cfg._time.hour 	= p->time.hour;
			time_cfg._time.min		= p->time.minute;
			time_cfg._time.sec 		= p->time.second;
			time_cfg.ntp_or_manual 	= p->mod;
			//printf("===========time_cfg1.TimeZone = %d\n",time_cfg1->TimeZone);
            pthread_mutex_lock(&g_ntp_pra.lock);
			g_ntp_pra.TimeZone=p->TimeZone;
            pthread_mutex_unlock(&g_ntp_pra.lock);
            {
                memcpy(pmsg->data, &time_cfg, sizeof(sdk_date_t));

                TUTK_MSG_CTRL(SDK_MAIN_MSG_SYSTIME,SDK_PARAM_SET,0,0,pmsg);
				printf("===========TK_set_ntp_req_handle5\n");
            }
            info("SET== (%d) %d-%d-%d   %d:%d:%d==\n",time_cfg.tzone,time_cfg._time.year,\
                 time_cfg._time.mon ,time_cfg._time.day,time_cfg._time.hour,\
                 time_cfg._time.min,time_cfg._time.sec);

            break;
        default:
            break;
        }
		set_resp.result = 0;
		avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_NTP_CONFIG_RESP, (char *)&set_resp, sizeof(SMsgAVIoctrlSetNtpConfigResp));
        return ;
__error:
        set_resp.result = -1;
        avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_NTP_CONFIG_RESP, (char *)&set_resp, sizeof(SMsgAVIoctrlSetNtpConfigResp));
        return ;
    }
	
void tk_set_record_req_handle(avIndex, buf)
{
    SMsgAVIoctrlSetRecordResp resp;
    SMsgAVIoctrlSetRecordReq *p = (SMsgAVIoctrlSetRecordReq *)buf;
	unsigned int record_type = 0;
    switch(p->recordType)
    {
    case AVIOTC_RECORDTYPE_OFF:
    {	
		record_type = RECORD_NO;
        resp.result = 0;
    }
    break;
    case AVIOTC_RECORDTYPE_FULLTIME:
    {
		printf("set_record_req_handle   has  been  set open_record_always \n");
		record_type = RECORD_FULLTIME;
        resp.result = 0;
    }
    break;
    case AVIOTC_RECORDTYPE_ALARM:
    {
		record_type = RECORD_ALARM;
		resp.result = 0;
        printf("resp.result(%d)---------\n", resp.result);
    }
    break;
    default:
        resp.result = 0;
        break;
    };
	avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETRECORD_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetRecordResp));
	printf("avSendIOCtrl111111---------\n");

	char msg_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
    sdk_record_cfg_t *rec_cfg = (sdk_record_cfg_t *)pMsg->data;
    TUTK_MSG_CTRL(SDK_MAIN_MSG_RECORD_CFG,SDK_PARAM_GET,0,0,pMsg);
	//rec_cfg->record_sched[0].record_type=p->recordType;
	rec_cfg->record_types = record_type;
	printf(" trace ::record_types=%d\n ",rec_cfg->record_types);
	pMsg->size = sizeof(sdk_record_cfg_t);
    TUTK_MSG_CTRL(SDK_MAIN_MSG_RECORD_CFG,SDK_PARAM_SET,0,0,pMsg);
	printf("avSendIOCtr222222---------\n");
	usleep(100*1000);
	record_status.record_flags  = record_type;
	return;
__error:
    resp.result = -1;
	avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETRECORD_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetRecordResp));
	return;
}
void tk_get_record_req_handle(int avIndex, char *buf)
{
    SMsgAVIoctrlGetRecordResq resp;
    SMsgAVIoctrlGetRecordReq *p = (SMsgAVIoctrlGetRecordReq *)buf;
    resp.channel = p->channel;


    char msg_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
    sdk_record_cfg_t *rec_cfg = (sdk_record_cfg_t *)pMsg->data;
    TUTK_MSG_CTRL(SDK_MAIN_MSG_RECORD_CFG,SDK_PARAM_GET,0,0,pMsg);
	//resp.recordType=rec_cfg->record_sched[0].record_type;
	resp.channel 	=	p->channel;
	resp.recordType	=	rec_cfg->record_types;

	printf("rec_cfg->record_types=%d\n",rec_cfg->record_types);
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETRECORD_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetRecordResq));
	return;
__error:
	avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETRECORD_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetRecordResq));
	return;
}


void TK_set_aws_req_handle(int avIndex, char *buf)
{

    SMsgAVIoctrlSetAwsStatusResp resp;
	SMsgAVIoctrlSetAwsStatusReq *p = (SMsgAVIoctrlSetAwsStatusReq *)buf;

    char msg_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pmsg = (sdk_msg_t *)msg_buf;
    TUTK_MSG_CTRL(SDK_MAIN_MSG_AWS,SDK_PARAM_GET,0,0,pmsg);
    if(NULL == pmsg)
    {
        goto __error;
    }
    sdk_aws_info_t *aws_cfg = (sdk_aws_info_t*)pmsg->data;
	printf("p->aws_record =%d\n",p->aws_record);

	aws_cfg->aws_record_status = p->aws_record;
	pthread_mutex_lock(&aws.aws_lock);
	aws.full_or_alarm = p->aws_record; //设置的同时要更新全局变量
	pthread_mutex_unlock(&aws.aws_lock);
	printf("aws.full_or_alarm =%d\n",aws.full_or_alarm);
	pmsg->size = sizeof(sdk_aws_info_t);

	TUTK_MSG_CTRL(SDK_MAIN_MSG_AWS,SDK_PARAM_SET,0,0,pmsg);

	resp.result = 0;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_AWS_STATUS_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetAwsStatusResp));
    return;
__error:
	resp.result = -1;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_AWS_STATUS_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetAwsStatusResp));
    return ;
}

void TK_get_aws_req_handle(int avIndex, char *buf)
{

    SMsgAVIoctrlGetAwsStatusResp resp;

    char msg_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pmsg = (sdk_msg_t *)msg_buf;
    TUTK_MSG_CTRL(SDK_MAIN_MSG_AWS,SDK_PARAM_GET,0,0,pmsg);
    if(NULL == pmsg)
    {
        goto __error;
    }
    sdk_aws_info_t *aws_cfg = (sdk_aws_info_t*)pmsg->data;

	resp.aws_record = aws_cfg->aws_record_status;
	printf("TK_get_aws_req_handle: aws_record_status=%d\n",aws_cfg->aws_record_status);
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_AWS_STATUS_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetAwsStatusResp));
    return;
__error:
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_AWS_STATUS_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetAwsStatusResp));
    return ;
}


#define DEFAULT_UPG_FILE "/tmp/upgrade_packet"	
extern int download_Progress;

int app_upgrade_req_handle1(int avIndex,char * buf)
{
	
	SMsgAVIoctrlRemoteUpgradeResp resp;
	//SMsgAVIoctrlProgressResp resp_progress;
	SMsgAVIoctrlRemoteUpgradeReq *p = (SMsgAVIoctrlRemoteUpgradeReq *)buf;
	int ret;
	char msg_buf[MSG_BUF_SIZE] = {0};
	sdk_msg_t *pmsg = (sdk_msg_t *)msg_buf;

	printf("version=%s url=%s\n",p->new_version,p->url_parth);

//判断版本号
	char version_buf[MSG_BUF_SIZE] = {0};
	sdk_msg_t *pMsg = (sdk_msg_t *)version_buf;

	TUTK_MSG_CTRL(SDK_MAIN_MSG_SYS_CFG,SDK_PARAM_GET,0,0,pMsg);

	sdk_sys_cfg_t *version_msg_cfg	= (sdk_sys_cfg_t *)pMsg->data;
	unsigned int version_local 	= 0;
	unsigned int version_new 	= 0;
	unsigned int version_0 		= 0;
	unsigned int version_1 		= 0;
	unsigned int version_2		= 0;
	unsigned int version_3 		= 0;
	
	sscanf(version_msg_cfg->software_ver,"V%d.%d.%d.%d",&version_3,&version_2,&version_1,&version_0);
	version_3 = ((version_3&0xff) << 24) ;
	version_2 = ((version_2&0xff) << 16) ;
	version_1 = ((version_1&0xff) << 8) ;
	version_0 = ((version_0&0xff) << 0) ;
	version_local = version_3| version_2 | version_1 | version_0;
	printf("version_local:%d\n", version_local);

	version_0=version_1=version_2=version_3 = 0;
	sscanf(p->new_version,"%d.%d.%d.%d",&version_3,&version_2,&version_1,&version_0);
	version_3 = ((version_3&0xff) << 24) ;
	version_2 = ((version_2&0xff) << 16) ;
	version_1 = ((version_1&0xff) << 8) ;
	version_0 = ((version_0&0xff) << 0) ;
	version_new = version_3| version_2 | version_1 | version_0;
	printf("version_new:%d\n", version_new);
	
//测试用 设置为3.0.8
	//version_local = version_3| version_2 | (8&0xff)<<8 | version_0;
	//printf("version_local_test:%d\n", version_local);
	
	if(version_local >= version_new)
	{
		resp.result = 1;
		avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_REMOTE_UPGRADE_RESP, (char *)&resp, sizeof(SMsgAVIoctrlRemoteUpgradeResp));
		return 0;
	}
	else if(version_local < version_new)
	{
		resp.result = 0;
		avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_REMOTE_UPGRADE_RESP, (char *)&resp, sizeof(SMsgAVIoctrlRemoteUpgradeResp));
	}

////////////////////////////////////////////////////////
	ret = http_url_down(p->url_parth);
    if(0 != ret)
    {
    	printf("http_url_down = %d\n",ret);
		return -1;
    }
	else 
	{
		TUTK_MSG_CTRL(SDK_MAIN_MSG_UPGRAD,SDK_UPGRAD_REQUEST,0,0,pmsg); //先查询是否正在更新
		
		if(pmsg->ack == SDK_ERR_UPGRADING)
		{
			goto __error;
		}
		else
		{	
			printf("NO upgrading! can do it\n");
			
			//resp_progress.progress_value = 50;
			//avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_UPGRADE_PROGRESS_RESP, (char *)&resp_progress, sizeof(SMsgAVIoctrlProgressResp));
			download_Progress  = 0;
			download_Progress  |= (1<<17);
			memset(msg_buf,0,sizeof(msg_buf));
			upgrad_pkg_file_t *pfile = (upgrad_pkg_file_t *)pmsg->data;
			strncpy(pfile->file_path, DEFAULT_UPG_FILE, sizeof(pfile->file_path));
			printf("pfile->file_path=%s\n",pfile->file_path);
			TUTK_MSG_CTRL(SDK_MAIN_MSG_UPGRAD,SDK_UPGRAD_DISK_FILE,0,0,pmsg);//通知下层  升级包 已经下载完毕 开始升级
			printf("upgrade_status +_+_+_+_+_+_+_+_+_+ down file successful \n ");

		}
	}
	
	return ;	
__error:
		resp.result = -1;
		avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_REMOTE_UPGRADE_RESP, (char *)&resp, sizeof(SMsgAVIoctrlRemoteUpgradeResp));
		return ;
}



int app_upgrade_progress_handle1(int avIndex, char *buf)
{
	
	SMsgAVIoctrlProgressResp resp;
	int ret;
	char msg_buf[MSG_BUF_SIZE] = {0};
	sdk_msg_t *pmsg = (sdk_msg_t *)msg_buf;

	if(1 == (download_Progress >>16))//下载
	{
		resp.progress_value = download_Progress & (~(1<<16));
		printf("progress_value=%d\n",resp.progress_value);
		avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_UPGRADE_PROGRESS_RESP, (char *)&resp, sizeof(SMsgAVIoctrlProgressResp));
	}
	if(1 == (download_Progress >>17))//升级
	{
		TUTK_MSG_CTRL(SDK_MAIN_MSG_UPGRAD,SDK_UPGRAD_PROGRESS,0,0,pmsg); 
		printf("pmsg->extend=%d\n",pmsg->extend);
		resp.progress_value = (pmsg->extend >> 1) + 50;
		avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_UPGRADE_PROGRESS_RESP, (char *)&resp, sizeof(SMsgAVIoctrlProgressResp));
	}
	return ;	
__error:
	resp.progress_value = -1;
	avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_UPGRADE_PROGRESS_RESP, (char *)&resp, sizeof(SMsgAVIoctrlProgressResp));
	return ;
}

int tk_get_mac_addr(int avIndex, char *buf)
{
	SMsgAVIoctrlGetMacAddrResp resp;
	int ret;
	unsigned char mac[MAX_MAC_ADDR_LEN];
	SMsgAVIoctrlGetMacAddrReq *p = (SMsgAVIoctrlGetMacAddrReq *)buf;
	memset(&resp,0,sizeof(SMsgAVIoctrlGetMacAddrResp));
	if(1 == p->type)
	{
		ret = os_get_mac_addr("eth0",mac,MAX_MAC_ADDR_LEN);
	}
	else if(2 == p->type)
	{	
		#ifndef SMART_CONFIG
		ret = os_get_mac_addr("ra0",mac,MAX_MAC_ADDR_LEN);
		#else
		ret = os_get_mac_addr("wlan0",mac,MAX_MAC_ADDR_LEN);
		#endif
	}
	resp.type = p->type;
	memcpy(resp.mac,mac,MAX_MAC_ADDR_LEN);
	//printf("type=%d mac:%x:%x:%x:%x:%x:%x\n",resp.type,resp.mac[0],resp.mac[1],resp.mac[2],resp.mac[3],resp.mac[4],resp.mac[5]);
	printf("type=%d mac:%s\n",resp.type,resp.mac);
	avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_MAC_ADDR_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetMacAddrResp));
	return 0;
}

