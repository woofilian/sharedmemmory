#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <pthread.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h> 
#include <errno.h> 
#include <netinet/ip.h> 
#include <arpa/inet.h> 
#include <netinet/ip_icmp.h> 
#include <linux/if_ether.h> 
#include <linux/sockios.h>
#include <linux/ethtool.h>

#include "sdk_struct.h"
#include "netwifi.h"
#include "net.h"
#include "log.h"




int uninit_wifi_mode(int wifi_type)
{
	if(1 == wifi_type)
	{
	    //卸载station模式  注意驱动的卸载顺序
      system("sh /mnt/mtd/run/wifi_sta_uinint.sh");
	  info("uninit_wifi_mode mt7601 station  ok!!! \n");
	}
	return 0;
}

int get_wifi_cfg()
{
	
}
int wifi_state()
{
	
}

int stop_sta()
{
	uninit_wifi_mode(1);//卸载7601 sta
	return 0;
	
}

int create_wifi_connect_file(unsigned char *ssid, unsigned char *pwd, sdk_wifi_cfg_t* param)
{
	char wifi_connect_command[256] = {0};
	char TempStr[256] = {0};
	sdk_wifi_cfg_t* grd_wifi_config = param;
	FILE *wifi_connect = NULL; 				
	FILE *wifi_connect_script = NULL;
	FILE *fd =NULL;
	
    fd = fopen("tmp/wifi_connect.sh", "r"); //文件不存在  搜索station
    if (fd != NULL)
    {
        return -1;
    }
	  
	info("1111111111111111111111111111111111111111111111111111111111111\n");
	printf("ssid=%s\n",ssid);
	printf("pwd=%s\n",pwd);
	printf("encrypt_type=%d\n",param->encrypt_type);
	printf("secret_key_type=%d\n",param->secret_key_type);
	info("111111111111111111111111111111111111111111111111111111111111\n");
	
	sprintf(TempStr, "/tmp/wifi_connect.sh");
	wifi_connect = fopen(TempStr, "w+");
	sprintf(TempStr, "/mnt/mtd/run/wifi_connect.sh");
	wifi_connect_script = fopen(TempStr, "r");
	if (wifi_connect == NULL)
    {
        printf("wifi_connect   file create failed \n");
        if (wifi_connect) 
            fclose(wifi_connect);
        if (wifi_connect_script) 
            fclose(wifi_connect_script);
    }
	else
    {
        while (!feof(wifi_connect_script))
        {
            char *pStr=NULL;
            memset(wifi_connect_command, 0 , 256);
            memset(TempStr, 0 , 256);
            fgets(TempStr, 256, wifi_connect_script);
            if (pStr=strstr(TempStr, "$$_SSID_$$"))
            {
                pStr[0] = '\0';
                strcpy(wifi_connect_command, TempStr);
                strcat(wifi_connect_command, "\"");
                //strcat(wifi_connect_command, grd_wifi_config->ssid);
                strcat(wifi_connect_command, ssid);
                strcat(wifi_connect_command, "\"");
                strcat(wifi_connect_command, pStr+10);
            }
            else if (pStr=strstr(TempStr, "$$_PSK_$$"))
            {
                pStr[0] = '\0';
                strcpy(wifi_connect_command, TempStr);
                strcat(wifi_connect_command, "\"");
                //strcat(wifi_connect_command, grd_wifi_config->pwd);
                strcat(wifi_connect_command, pwd);
                strcat(wifi_connect_command, "\"");
                strcat(wifi_connect_command, pStr+9);
            }
            else
            {
                strcpy(wifi_connect_command, TempStr);
            }
            fputs(wifi_connect_command, wifi_connect);
        }
    }
    fclose(wifi_connect);
    fclose(wifi_connect_script);

    sleep(1);
    system("chmod  755   /tmp/wifi_connect.sh");
	
    printf("sh /tmp/wifi_connect.sh  encrypt_type  is:%d secret_key_type is:%d \n",grd_wifi_config->encrypt_type,grd_wifi_config->secret_key_type);
	sprintf(wifi_connect_command, "sh /tmp/wifi_connect.sh  %d  %d",grd_wifi_config->encrypt_type,grd_wifi_config->secret_key_type);
	system_ex(wifi_connect_command);
    sleep(3);
    return 0;
}

//后期后话之后 全部写在这边 
int create_wifi_ap_server_file(char *ssid)
{
#if 1
	char wifi_connect_command[256] = {0};
	char TempStr[256] = {0};
	char ssid_name[32]={0};
	FILE *wifi_connect = NULL; 				
	FILE *wifi_connect_script = NULL;

	sprintf(ssid_name, "JOOAN-");
	strncat(ssid_name, ssid, sizeof(ssid_name));
	printf("create_wifi_ap_server_file    ssid   is  %s\n",ssid_name);
	sprintf(TempStr, "/tmp/wifi_ap.sh");
	wifi_connect = fopen(TempStr, "w+");
	sprintf(TempStr, "/mnt/mtd/run/wifi_ap.sh");
	wifi_connect_script = fopen(TempStr, "r");
	if (wifi_connect == NULL)
    {
        printf("wifi_ap   file create failed \n");
        if (wifi_connect) 
            fclose(wifi_connect);
        if (wifi_connect_script) 
            fclose(wifi_connect_script);
    }
	else
    {
        while (!feof(wifi_connect_script))
        {	
            char *pStr=NULL;
            memset(wifi_connect_command, 0, sizeof(wifi_connect_command));
            memset(TempStr, 0, sizeof(TempStr));
            fgets(TempStr, sizeof(TempStr), wifi_connect_script);
            if (pStr=strstr(TempStr, "$$_SSID_$$"))
            {
                pStr[0] = '\0';
                strcpy(wifi_connect_command, TempStr);
                strcat(wifi_connect_command, "\"");
                strcat(wifi_connect_command, ssid_name);
                strcat(wifi_connect_command, "\"");
                strcat(wifi_connect_command, pStr+10);
            }
#if 0         //密码修改 后期可添加  暂时不适用
            else if (pStr=strstr(TempStr, "$$_PSK_$$"))
            {
                pStr[0] = '\0';
                strcpy(wifi_connect_command, TempStr);
                strcat(wifi_connect_command, "\"");
                strcat(wifi_connect_command, grd_wifi_config->pwd);
                strcat(wifi_connect_command, "\"");
                strcat(wifi_connect_command, pStr+9);
            }
#endif		  
            else
            {
                strcpy(wifi_connect_command, TempStr);
            }
            fputs(wifi_connect_command, wifi_connect);
        }
    }

    fclose(wifi_connect);
    fclose(wifi_connect_script);
    system("chmod  755   /tmp/wifi_ap.sh");
    usleep(1000);
    system("sh /tmp/wifi_ap.sh ");
#endif
    return 0;
}

int stop_ap()
{
	
	FILE * fp;
	char buffer[80];
	
	fp = popen("killall -9 udhcpd","r");
	fgets(buffer,sizeof(buffer),fp);
	printf("11111 %s \n",buffer);
	pclose(fp);

	fp = popen("ifconfig ra0  down","r");
		fgets(buffer,sizeof(buffer),fp);
		printf("22222222222 %s \n",buffer);
		pclose(fp);

   fp = popen("rmmod mt7601Uap","r");
	fgets(buffer,sizeof(buffer),fp);
	printf("333333 %s \n",buffer);
	pclose(fp);

	sleep(1);

	return 0;
}

int grd_wifi_ssid_escape(char * ssid)
{
int x= 0, y= 0;
char ssid_name[32] = {0};
	strcpy(ssid_name, ssid);
	for(y = 0;ssid_name[y];y++)
	{
		ssid[x] = ssid_name[y];
		if(ssid[x] == '`')
		{
			ssid[x] = 92;// add '\'
			ssid[x+1] = ssid_name[y];
			x++;
		}
		if(ssid[x] == '"')
		{
			ssid[x] = 92;// add '\'
			ssid[x+1] = ssid_name[y];
			x++;
		}
		/*if(ssid[x] == 39)//'
		{
			ssid[x] = 92;// add '\'
			ssid[x+1] = ssid_name[y];
			x++;
		}*/
		if(ssid[x] == 92)
		{
			ssid[x] = 92;// add '\'
			ssid[x+1] = ssid_name[y];
			x++;
		}
		x++;
	}
	ssid[x] = '\0';
}


int start_sta(sdk_wifi_cfg_t  * wifi_param)
{
#ifdef MTK7601
    int ret ;
	unsigned char cmd[64];
	unsigned char ssid[64];
	unsigned char pwd[64];
	memset(cmd, 0, sizeof(cmd));
	memset(ssid, 0, sizeof(ssid));
	memset(pwd, 0, sizeof(pwd));
	strncpy(ssid, wifi_param->ssid, sizeof(ssid));
	strncpy(pwd, wifi_param->pwd, sizeof(pwd));
    if(!wifi_param->ssid )
    {
        error("ssid  is empty!!! \n");
        return -1;
    }
    info("set sta ssid:%s ,pass:%s \n",wifi_param->ssid,wifi_param->pwd);

	grd_wifi_ssid_escape(ssid);
	grd_wifi_ssid_escape(pwd);

	if((0 == wifi_param->encrypt_type) && (1 == wifi_param->secret_key_type))
	{
		system_ex("/mnt/mtd/run/wifi_ap_uinit.sh");
		system_ex("/sbin/insmod /mnt/mtd/grain/ko/mtutil7601Usta.ko");
		system_ex("/sbin/insmod /mnt/mtd/grain/ko/mt7601Usta.ko");
		system_ex("/sbin/insmod /mnt/mtd/grain/ko/mtnet7601Usta.ko");
		system_ex("ifconfig ra0 up");
		usleep(500*1000);
		system_ex("iwpriv ra0 set NetworkType=Infra");
		usleep(50*1000);
		system_ex("iwpriv ra0 set AuthMode=OPEN");
		usleep(50*1000);
		system_ex("iwpriv ra0 set EncrypType=WEP");
		usleep(50*1000);
		system_ex("iwpriv ra0 set DefaultKeyID=1");
		usleep(50*1000);
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,"iwpriv ra0 set Key1=\"%s\"",pwd);
		printf("cmd=%s\n",cmd);
		system_ex(cmd);
		usleep(100*1000);
		memset(cmd,0,sizeof(cmd));
		sprintf(cmd,"iwpriv ra0 set SSID=\"%s\"",ssid);
		printf("cmd=%s\n",cmd);
		system_ex(cmd);
		usleep(500*1000);

	}
	else
	{
    	create_wifi_connect_file(ssid, pwd, wifi_param); //创建连接文件  然后执行
	}

	//create_wifi_connect_file(ssid,pwd,wifi_param);
    info(" create_wifi_connect_file end ++++++++++++++++++++++++++++++++++ \n");
	return 0;
	
#else /* of MTK7601 */
    //RTL8188 will go this branch
	int i;
	int network_id;
	char tmpbuf[64];
	unsigned char cmd[256];
	unsigned char ssid[32];
	unsigned char pwd[32];
	unsigned char sec[32];
	memset(cmd, 0, sizeof(cmd));
	memset(ssid, 0, sizeof(ssid));
	memset(pwd, 0, sizeof(pwd));
	strncpy(ssid, wifi_param->ssid, sizeof(ssid));
	strncpy(pwd, wifi_param->pwd, sizeof(ssid));
	strncpy(sec, wifi_param->secrity, sizeof(sec));

	// run rtl8188 station mode script
	system_ex("/mnt/mtd/run/rtl_station_run.sh");

	if(!wifi_param->ssid)	
	{		
		error("ssid pass error!!! \n");		
		return -1;	
	}	
	
	printf("set sta ssid:%s ,password:%s encrypt_type=%d auth_mode=%d secret_key_type=%d\n",
		wifi_param->ssid,wifi_param->pwd,
		wifi_param->encrypt_type,wifi_param->auth_mode, wifi_param->secret_key_type);

	grd_wifi_ssid_escape(ssid);
	grd_wifi_ssid_escape(pwd);

	// wch add for rtl8188 connection with wpa_supplicant
//#+==================================================+
//#| Security |  0   |  1   |  2     |    3           |
//#|----------|------|------|--------|----------------|
//#| Authmode | OPEN | OPEN | SHARED | WPAPSK/WPA2PSK |
//#|Encryption| NONE | WEP  | WEP    | TKIP/AES       |
//#+==================================================+
	FILE *fp_result = popen("wpa_cli -iwlan0 add_network", "r");
	if(fp_result != NULL)
    {
        fgets(tmpbuf, sizeof(tmpbuf), fp_result);
        network_id = atoi(tmpbuf);
        pclose(fp_result);
    }
	snprintf(cmd, sizeof(cmd), "wpa_cli -iwlan0 set_network %d scan_ssid 1", network_id);
	system_ex(cmd);
	snprintf(cmd, sizeof(cmd), "wpa_cli -iwlan0 set_network %d ssid '\"%s\"'", network_id, ssid);
	system_ex(cmd);
	//if((0 == wifi_param->encrypt_type)&&(0 == wifi_param->secret_key_type))
	if((strstr(sec, "NONE") != NULL))
	{
		//Security=0
		printf("===wch debug: this is an open/none AP...\n");
		snprintf(cmd, sizeof(cmd), "wpa_cli -iwlan0 set_network %d key_mgmt NONE", network_id);
		system_ex(cmd);
	}
	//else if(3 == wifi_param->encrypt_type)
	else if(strstr(sec, "WEP") != NULL)
	{
		if(3 == wifi_param->auth_mode)
		{
			//Security=1
			printf("===wch debug: the AP is encrypt with WEP and the authmode is OPEN\n");
			snprintf(cmd, sizeof(cmd), "wpa_cli -iwlan0 set_network %d key_mgmt NONE", network_id);
			system_ex(cmd);
			snprintf(cmd, sizeof(cmd), "wpa_cli -iwlan0 set_network %d wep_key0 \'\"%s\"\'", network_id, pwd);
			system_ex(cmd);
			snprintf(cmd, sizeof(cmd), "wpa_cli -iwlan0 set_network %d wep_tx_keyidx 0", network_id);
			system_ex(cmd);
			//snprintf(cmd, sizeof(cmd), "/mnt/mtd/run/rtl_wifi_cmd.sh 1 %s %s", ssid, pwd);
			//system_ex(cmd);
		}
		if(4 == wifi_param->auth_mode)
		{
			//Security=2
			printf("===wch debug: the AP is encrypt with WEP and the authmode is SHARED\n");
			snprintf(cmd, sizeof(cmd), "wpa_cli -iwlan0 set_network %d key_mgmt NONE", network_id);
			system_ex(cmd);
			snprintf(cmd, sizeof(cmd), "wpa_cli -iwlan0 set_network %d auth_alg SHARED", network_id);
			system_ex(cmd);
			snprintf(cmd, sizeof(cmd), "wpa_cli -iwlan0 set_network %d wep_key0 \'\"%s\"\'", network_id, pwd);
			system_ex(cmd);
			snprintf(cmd, sizeof(cmd), "wpa_cli -iwlan0 set_network %d wep_tx_keyidx 0", network_id);
			system_ex(cmd);
			//snprintf(cmd, sizeof(cmd), "/mnt/mtd/run/rtl_wifi_cmd.sh 2 %s %s", ssid, pwd);
			//system_ex(cmd);
		}
	}
	//else if(1 == wifi_param->encrypt_type || 2 == wifi_param->encrypt_type)
	if(strstr(sec, "WPA") != NULL)
	{
		//Security=3
		printf("===wch debug: the AP is encrypt with WPAPSK or WPA2PSK or both\n");
		snprintf(cmd, sizeof(cmd), "wpa_cli -iwlan0 set_network %d key_mgmt WPA-PSK", network_id);
		system_ex(cmd);
		snprintf(cmd, sizeof(cmd), "wpa_cli -iwlan0 set_network %d psk \'\"%s\"\'", network_id, pwd);
		system_ex(cmd);
		//snprintf(cmd, sizeof(cmd), "/mnt/mtd/run/rtl_wifi_cmd.sh 3 %s %s", ssid, pwd);
		//system_ex(cmd);
	}
	snprintf(cmd, sizeof(cmd), "wpa_cli -iwlan0 select_network %d", network_id);
	system_ex(cmd);
    snprintf(cmd, sizeof(cmd), "wpa_cli -iwlan0 enable_network %d", network_id);
	system_ex(cmd);

	sleep(2);
	return 0;
#endif /* SMART_CONFIG */
}

int ap_rtl_config()
{
	int mac[6] = {0};
	char mac_addr[20] ={0};
	char ssid_name[32]={0};
	char cmd_line[256]={0};
	
	os_get_mac_addr("wlan0", mac_addr, 20);
    info("current mac addr :%s \n",mac_addr);
	sscanf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	snprintf(ssid_name, sizeof(ssid_name), "JooanIpc-%02X%02X", mac[4], mac[5]);
	printf("ssid=%s\n",ssid_name);

	snprintf(cmd_line, sizeof(cmd_line), "/mnt/mtd/run/rtl_softAp_run.sh %s", ssid_name);
	system_ex(cmd_line);
}


/**************************************/


int ap_setting(/*char *ssid*/)
{
	//之后优化的时候再添加修改ssid的功能
	 
	int mac[6] = {0};
	char mac_addr[20] = {0};
	char ssid_name[16]={0};

	system_ex("/sbin/insmod /mnt/mtd/grain/ko/mt7601Uap.ko");
	usleep(300000);
	system_ex("cp /mnt/mtd/run/udhcpd7601 /etc/udhcpd.conf");
	system_ex("ifconfig ra0  192.168.1.1 up");
	usleep(10000);
	os_get_mac_addr("ra0", mac_addr, 20);

    info("current mac addr :%s \n",mac_addr);

    sscanf(mac_addr,"%02x:%02x:%02x:%02x:%02x:%02x",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
    sprintf(ssid_name,"%02X%02X", mac[4],mac[5]);
	create_wifi_ap_server_file(ssid_name);	

    sleep(1);
	return 0;
}

int wifi_find_list()
{
#ifdef MTK7601
	system("ifconfig ra0 up");
	usleep(500*1000);
	system("iwlist ra0 scanning >/var/wifilist.sh");
#else
    system("ifconfig wlan0 up");
	usleep(500*1000);
	system("iwlist wlan0 scanning >/var/wifilist.sh");
#endif
	 
	return 0;
}




int get_wifi_list(sdk_wifi_search_t *ap_list)
{
    return 0;
	
}

