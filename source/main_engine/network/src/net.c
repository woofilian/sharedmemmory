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

#include "netlib.h"
#include "netwifi.h"
#include "log.h"
#include "sdk_struct.h"
#include "adapt.h"


#define RTL8188
//#define MTK7601
#ifdef RTL8188
#define OS_wifi0  "wlan0"
#else
#define OS_wifi0  "ra0"
#endif


#define OS_ETH0   "eth0"

//int dhcp_enable = 0;
int network_is_ok = 0;
int network_is_setting = 0;

net_manager_info_t g_manager;
static sdk_wifi_cfg_t  g_wifi_cfg; // wifi 参数设置
sdk_eth_cfg_t g_ip_info; //IP信息配置
static NetStatusCB  g_NetworkCB = NULL;

extern LED_BUZZER_INFO  led_buzzer_info;

extern sdk_Network_Info g_network_info;


static pthread_t thread_manager_id = -1;

/******************************************************************/
//重写get /set gatewayip
int os_get_gateway1(const char *if_name,char *gatewayip, int maxlen)
{
char buf[256];
FILE *fp;
char *front=NULL;
char *end =NULL;	
	memset(buf,0,256);	
	fp = popen("ip route show","r");	
	if(NULL == fp)		
	{		
		printf("fopen fail!\n");	
		return 1;	
	}
	else	
	{	
		fgets(buf,sizeof(buf),fp);	
		//printf("buf =%s\n",buf);
		if(NULL == strstr(buf,if_name))
		{			
			printf("not find %s\n",if_name);
			pclose(fp);	
			return 1;
		}		
		else		
		{			
			//printf("this is %s\n",if_name);	
			if(NULL == (front = strstr(buf,"default via")))		
			{			
				printf("no find char:default via\n");	
				pclose(fp);			
				return 1;			
			}		
			else		
			{		
				front += strlen("default via ");	
			}			
			if(NULL == (end = strstr(buf,"dev")))		
			{			
				printf("no find char: dev eth0\n");		
				pclose(fp);			
				return 1;		
			}			
			else		
			{			
				end-=1;		
				*end ='\0';	
			}
			
			strncpy(gatewayip,front, maxlen);	
		}		
	pclose(fp);		
	return 0;	
	}
}

int os_set_gateway1(const char *if_name,char *gatewayip)
{
char buf[256];
FILE *fp =NULL;
	memset(buf,0,sizeof(buf));	
	sprintf(buf,"route add default gw %s",gatewayip);
	printf("%s\n",buf);;	
	if(NULL == (fp = popen(buf,"r")))	
	{		
		printf("popen fail\n");	
		return 1;
	}	
	else	
	{
		pclose(fp);	
		return 0;	
	}
}



/*******************************************************************/
int system_ex(const char *cmd_buf)
{
	if(!cmd_buf)
	{
		return -1;
	}
	FILE * fp;
	char buffer[256];
	fp = popen(cmd_buf,"r");
	fgets(buffer,sizeof(buffer),fp);
	//printf("test TTTTTTTT  %s  \n RRRRRRRRRRRR %s \n",cmd_buf,buffer);
	pclose(fp);

}


int net_os_kill_dhcp()
{
    warning(" _kill_dhcp...\n");
    system("killall udhcpc");
    system("killall -9 udhcpc");
	sleep(2);
    return 0;
}

int net_os_start_dhcp(char *p_net_name)
{
    char szStr[256] = {0};

	printf("call dhcp......................\n");
	sprintf(szStr, "wan_udhcpc.sh %s &", p_net_name);
	printf("_start_dhcp:%s \n",szStr);
	system_ex(szStr);
	return 0;
}


//
// return value:
// -1 -- error , details can check errno
// 1 -- interface link up
// 0 -- interface link down.
static int eth_connect_stat()
{
 	 //return 1;
    int skfd;
    struct ifreq ifr;
    struct ethtool_value edata;
    edata.cmd = ETHTOOL_GLINK;
    edata.data = 0;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, "eth0", sizeof(ifr.ifr_name) - 1);
    ifr.ifr_data = (char *) &edata;
    if (( skfd = socket( AF_INET, SOCK_DGRAM, 0 )) < 0)
    {
        error("socketerr skfd=%d strerror(errno)=%s\n",skfd,strerror(errno));
		
        return -1;
    }
    if(ioctl( skfd, SIOCETHTOOL, &ifr ) == -1)
    {
       // error(" ioctl err skfd=%d\n",skfd);
        close(skfd);
		
		//info("55555555555555555555555555 \n");
        return -1;
    }
    close(skfd);
    return edata.data;
}

int _os_set_ip_info(char *if_name,sdk_ip_info_t *ip_info)
{
    int ret = -1;
    int resave_flags = 0;
    char os_mac[MAX_MAC_ADDR_LEN] = {0};

    if(!if_name)
    {
        error("if_name is NULL \n");
        return -1;
    }
	if(1 == ip_info->enable_dhcp) //使用DHCP
	{
		//net_os_kill_dhcp(); //如果有DHCP 先干掉
		//sleep(1);
		net_os_start_dhcp(if_name); //启动DHCP
		//sleep(5);
		//dhcp_enable = 1;
	}
	else if(0 == ip_info->enable_dhcp) //关闭DHCP
	{
		printf("no dhcp: set network\n");
		//设置IP
		printf("if_name:%s, ipaddr:%s\n", if_name, (char *)ip_info->ip_addr);
    	ret = os_set_ip_addr(if_name, (char *)ip_info->ip_addr);
   		if (ret < 0)
   		{
        	error("os_set_ip_addr error!\n");
    	}
		//设置掩码
		printf("if_name:%s, mask:%s\n", if_name, (char *)ip_info->mask);
    	ret = os_set_net_mask(if_name, (char *)ip_info->mask);
   		if (ret < 0)
    	{
        	error("os_set_net_mask error!\n");
   		}
		//设置DNS1
		printf("dns1:%s, dns2:%s\n", (char *)ip_info->dns1, (char *)ip_info->dns2);
    	ret = os_set_dns(strlen((char *)ip_info->dns1)?(char *)ip_info->dns1:NULL,strlen((char *)ip_info->dns2)?(char *)ip_info->dns2:NULL);
    	if (ret < 0)
    	{
        	error("os_set_dns error!\n");
    	}

    	//网关的设置需要放在广播地址之后，否则网关将会设置失败

  		 // warning("os_del_gateway >>>>>>>>>> gateway:0.0.0.0\n");
   		// os_del_gateway("0.0.0.0");
   		 printf("os_set_gateway >>>>>>>>>> if_name:%s, gateway:%s\n", if_name, (char *)ip_info->gateway);
   		 //ret = os_set_gateway(if_name, (char *)ip_info->gateway);
		ret = os_set_gateway1(if_name, (char *)ip_info->gateway);
    	if (ret < 0)
    	{
       	 	error("os_set_gateway error!\n");
   		}
		//不设置MAC
	}
    return 0;
}

/**
 * 检测WIFI连接
 * iwpriv ra0 connStatus       
 *  ra0       connStatus:Disconnected
 *  ra0       connStatus:Connected
 * 只检测WIFI的情况
 * 有变化返回1 无变化返回0
 */
static int check_ip_status()
{
	FILE *fp;
	unsigned char buf[128];

    sdk_ip_info_t ip_info;
    memset(&ip_info,0,sizeof(sdk_ip_info_t));
    if(SDK_NET_MODE_AP == g_manager.current_mode  || 0 == g_manager.connect_status )
        return 0;
 
	if(SDK_NET_MODE_WIFI == g_manager.current_mode)
	{
		memset(buf,0,sizeof(buf));
        #if 0 //wch modify: donot control the buzzer
		//#ifndef RTL8188
		fp  = popen("iwpriv wlan0 connStatus","r");
		if(NULL != fp)
		{
			fgets(buf,sizeof(buf),fp);
			//printf("buf=%s\n",buf);
			pclose(fp);

			if(strstr(buf,"Disconnected"))
			{
				//printf("Disconnected!!!!!!!!!!!!!!!!!!\n");
				pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
				led_buzzer_info.led_status |= 1<<LED_NO_WIFI;
				pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);
				network_is_ok = 0;

			}
			else if(strstr(buf,"Connected"))
			{
				//printf("Connected!!!!!!!!!!!!!!!!!!\n");
				pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
				led_buzzer_info.led_status &= ~(1<<LED_NO_WIFI);
				led_buzzer_info.led_status &= ~(1<<LED_WIFI_CONFIGING);
				pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);
				network_is_ok = 1;
			}
			fp = NULL;
		}
		#endif
	}
    if(!strlen(g_manager.ip_addr))
    {
        info("now first get ip info if_name:%s \n",g_manager.if_name);

        os_get_ip_addr(g_manager.if_name,g_manager.ip_addr,MAX_IP_ADDR_LEN);
        os_get_net_mask(g_manager.if_name,g_manager.mask,MAX_IP_ADDR_LEN);
       // os_get_gateway(g_manager.if_name,g_manager.gateway,MAX_IP_ADDR_LEN);
       	os_get_gateway1(g_manager.if_name,g_manager.gateway,MAX_IP_ADDR_LEN);
		os_get_mac_addr(g_manager.if_name,g_manager.mac,MAX_MAC_ADDR_LEN);

        os_get_dns(g_manager.dns1,g_manager.dns2,MAX_IP_ADDR_LEN);
		info("first get in info: !!! \n");
		info("\n ip_addr:%s\n"
             "mask:%s \n"
             "gateway:%s \n"
             "dns1:%s \n"
             "dns2:%s  \n"
             ,g_manager.ip_addr
             ,g_manager.mask
             ,g_manager.gateway
             ,g_manager.dns1
             ,g_manager.dns2);
#if 0
		sprintf(buf ,"arp -s  %s %s" ,g_manager.ip_addr,g_manager.mac);
		//在这里还要同步更新 arp(路由表)  不仅是这里 其他改变了mac地址的位置全部都需要进行相应的修改
		system("buf");//刷新arp
		//*添加一对IP和MAC地址的绑定： 
		//# arp -s 10.1.1.1 00:11:22:33:44:55:66 

#endif
        return 0;
    }
	
    os_get_ip_addr(g_manager.if_name,ip_info.ip_addr,MAX_IP_ADDR_LEN);
    os_get_net_mask(g_manager.if_name,ip_info.mask,MAX_IP_ADDR_LEN);
    //os_get_gateway(g_manager.if_name,ip_info.gateway,MAX_IP_ADDR_LEN);
	os_get_gateway1(g_manager.if_name,ip_info.gateway, MAX_IP_ADDR_LEN);
    os_get_mac_addr(g_manager.if_name,ip_info.mac,MAX_MAC_ADDR_LEN);
    os_get_dns(ip_info.dns1,ip_info.dns2,MAX_IP_ADDR_LEN);
	sleep(1);
	//printf("os_get_dns  dns1=%s dns2=%s\n",ip_info.dns1,ip_info.dns2);
    if(strncmp(g_manager.ip_addr,ip_info.ip_addr,MAX_IP_ADDR_LEN)
            ||strncmp(g_manager.mask,ip_info.mask,MAX_IP_ADDR_LEN)
            ||strncmp(g_manager.gateway,ip_info.gateway,MAX_IP_ADDR_LEN))
    {

       // strncpy(g_manager.ip_addr,ip_info.ip_addr,MAX_DEV_ID_LEN) ;
      //  strncpy(g_manager.mask,ip_info.mask,MAX_PASSWD_LEN);
      //  strncpy(g_manager.gateway,ip_info.gateway,MAX_PASSWD_LEN);
      //  strncpy(g_manager.dns1,ip_info.dns1,MAX_PASSWD_LEN);
      //  strncpy(g_manager.dns2,ip_info.dns2,MAX_PASSWD_LEN);
      //  strncpy(g_manager.mac,ip_info.mac,MAX_PASSWD_LEN);
      
        strcpy(g_manager.ip_addr,ip_info.ip_addr) ;
        strcpy(g_manager.mask,ip_info.mask);
        strcpy(g_manager.gateway,ip_info.gateway);
        strcpy(g_manager.dns1,ip_info.dns1);
        strcpy(g_manager.dns2,ip_info.dns2);
        strcpy(g_manager.mac,ip_info.mac);
        info("current ip change !!! \n");
        info("ip ip_addr:%s\n"
             "mask:%s \n"
             "gateway:%s \n"
             "dns1:%s \n"
             "dns2:%s  \n"
             ,g_manager.ip_addr
             ,g_manager.mask
             ,g_manager.gateway
             ,g_manager.dns1
             ,g_manager.dns2);
		//adapt_param_set_ip_info(&ip_info, 0);  //设置保存参数

			return 1;
    }

    return 0;
}
static int refresh_net_server();

static int monitor_net_status()
{
	refresh_net_server();
	if(check_ip_status())
	{
		info("ip changed need call back!!!! \n");
	}
	
    return 0;
}


int change_ssid()
{
	int mac[6] = {0};
	char mac_addr[20] = {0};
	char ssid_name[32]={0};
	FILE *fp;
	FILE *pfile;
	char temp[25], macbuf[18], line[100];
	
	os_get_mac_addr(OS_wifi0, mac_addr, 20);
    info("current mac addr :%s \n",mac_addr);
	sscanf(mac_addr,"%02x:%02x:%02x:%02x:%02x:%02x",&mac[0],&mac[1],&mac[2],&mac[3],&mac[4],&mac[5]);
	sprintf(ssid_name, "SSID=Guardzilla-%02X%02X",mac[4],mac[5]);
	printf("ssid=%s\n",ssid_name);

	strcpy(temp,"SSID=Guardzilla-1234");
	//pfile=fopen("/mnt/mtd/run/Wireless/RT2870AP/RT2870AP.dat","r+");    
	pfile=fopen("/etc/Wireless/RT2870AP/RT2870AP.dat","r+");  
	if(!pfile)   
	{	
		perror("NO SUCH FILE RT2870AP.dat\n");	
		return -1;  
	}	
	
	while(!feof(pfile))//未到结尾	
	{		 
		char *index=NULL;
		fgets(line, sizeof(line), pfile);
		index=strstr(line, temp);	
		if(index)	
		{		
			int d2=strlen(ssid_name);
			int d1=strlen(temp);
			memmove(index+d1+d2-d1, index+d1, strlen(line));	
			memcpy(index, ssid_name, strlen(temp)); 		
			fseek(pfile,-strlen(line)+d2-d1,SEEK_CUR);		
			fputs(line,pfile);		
			fflush(pfile);		
		} 
	}  
	fclose(pfile);
	return 0;
}


int  sdk_init_net_server(sdk_eth_cfg_t *net_param, sdk_wifi_cfg_t *wifi_param)
{
    //uninit_wifi_mode(0);
    usleep(10*1000);
    memset(&g_manager, 0, sizeof(net_manager_info_t));
    memset(&g_ip_info, 0, sizeof(sdk_ip_info_t));
    memset(&g_wifi_cfg, 0, sizeof(sdk_wifi_cfg_t));

    memcpy(&g_ip_info, net_param, sizeof(sdk_eth_cfg_t));
    memcpy(&g_wifi_cfg, wifi_param, sizeof(sdk_wifi_cfg_t));
	 
    strcpy(g_manager.if_name, OS_wifi0); //临时这么设定
    //下面是关键参数
    g_manager.current_mode = net_param->if_mode;  //当前处于什么模式
    g_manager.enable_dhcp = net_param->ip_info.enable_dhcp; //DHCP 功能
    strncpy(g_manager.ssid,wifi_param->ssid,strlen(wifi_param->ssid));
	strncpy(g_manager.passwd,wifi_param->pwd,strlen(wifi_param->pwd));
    info(" init param: current_mode:%d  enable_dhcp:%d ssid:%s passwd:%s \n"
         ,g_manager.current_mode,g_manager.enable_dhcp,g_manager.ssid,g_manager.passwd);
    pthread_mutex_init(&g_manager.manager_mutex, NULL);
	//g_manager.current_mode = SDK_NET_MODE_AP;

#ifdef MTK7601
	if((SDK_NET_MODE_AP == g_manager.current_mode )&& (1 != eth_connect_stat()))
	{
		system("/bin/sh /mnt/mtd/run/ap_search_1.sh");//搜索一边周围的热点
		change_ssid();	
    	system("/bin/sh /mnt/mtd/run/ap_search_2.sh");//搜索一边周围的热点
	}
#endif /*end of MTK7601*/

	usleep(1000*1000);
    return 0;
}


int sdk_start_ap_server()
{
#ifdef RTL8188
	ap_rtl_config();
#else /* MT7601 branch */
	ap_setting();
#endif /* endif RTL8188 */
    return 0 ;
}


int sdk_start_sta_server()
{
    int ret = start_sta(&g_wifi_cfg);
printf("^^^^^^^^^^^ debug: return value of start_sta is %d\n", ret);
    return 0;
}

static int thread_net_manager(void *arg)
{
    int ret = -1;
    sdk_sys_cfg_t   system_info;
    unsigned char sta_buf[64];
	
    while(g_manager.enable)
	{	
	    ret = eth_connect_stat();//判断是不是有线连接		
	    if((1 != ret) && (0 == g_manager.connect_status)) //无线设备  表示要重新连接
	    {
	      	info("wifi  work+_+_+__+_+_+_+g_manager.current_mode  is %d_+__+_+_+_+_+_+_+_+_ \n",g_manager.current_mode);

		 	ret = wifi_work_start(g_ip_info.if_mode);//启动ap  还是station
		 	printf("wifi_work_start return %d\n",ret);
			if(0 != ret)
			{	
				sleep(1);
				continue;
			}
			memset(g_manager.if_name,0,NET_CARD_NAME_LEN);
			strcpy(g_manager.if_name,OS_wifi0);
		    g_manager.connect_status = 1;
			g_manager.config_change = 0;
			if(g_ip_info.if_mode != SDK_NET_MODE_AP)//暂时修改成这样
			{
				printf("g_ip_info.if_mode != SDK_NET_MODE_AP\n");	
				g_ip_info.if_mode = SDK_NET_MODE_WIFI; //强制赋值  因为可能从配置文件中读取到的上一次的值
				g_manager.current_mode = SDK_NET_MODE_WIFI;

				printf("in sleep!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
				printf("g_ip_info.dhcp=%d\n",g_ip_info.ip_info.enable_dhcp);
				sleep(8);
				_os_set_ip_info(OS_wifi0,&g_ip_info);//获取 dhcp相关信息
			}
	    }
	    else if(g_manager.connect_status == 0) // 有线设备
	    {
			info("eth0  work+_+_+__+_+_+_+_+__+_+_+_+_+_+_+_+_ \n");
			g_ip_info.if_mode = SDK_NET_MODE_ETH;
			g_manager.current_mode = SDK_NET_MODE_ETH;
			_os_set_ip_info(OS_ETH0,&g_ip_info);
			memset(g_manager.if_name,0,NET_CARD_NAME_LEN);

			strcpy(g_manager.if_name,OS_ETH0);
			strcpy(g_ip_info.ip_info.if_name,OS_ETH0);
			g_manager.connect_status = 1;
			g_manager.config_change = 0 ;
		//	有线时  不检测 直接清0  出厂产品是没有有线的
			pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
			led_buzzer_info.led_status &= ~(1<<LED_NO_WIFI);
			led_buzzer_info.led_status &= ~(1<<LED_WIFI_CONFIGING);
			pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);
			network_is_ok = 1;
			
		#ifdef RTL8188
			snprintf(sta_buf, sizeof(sta_buf), "iwlist wlan0 scanning > /var/wifilist.sh");
			system_ex(sta_buf);
			sleep(1);
		#elif defined (MTK7601)
			//启STA 然后搜索wifi 保存到wifilist
			memset(sta_buf,0,sizeof(sta_buf));
			strcpy(sta_buf,"/sbin/insmod /mnt/mtd/grain/ko/mtutil7601Usta.ko");
			system_ex(sta_buf);
			memset(sta_buf,0,sizeof(sta_buf));
			strcpy(sta_buf,"/sbin/insmod /mnt/mtd/grain/ko/mt7601Usta.ko");
			system_ex(sta_buf);
			sleep(1);
			memset(sta_buf,0,sizeof(sta_buf));
			strcpy(sta_buf,"/sbin/insmod /mnt/mtd/grain/ko/mtnet7601Usta.ko");
			system_ex(sta_buf);
			sleep(1);
			memset(sta_buf,0,sizeof(sta_buf));
			strcpy(sta_buf,"ifconfig ra0 up");
			system_ex(sta_buf);
			sleep(1);
			memset(sta_buf,0,sizeof(sta_buf));
			strcpy(sta_buf,"iwlist ra0 scanning > /var/wifilist.sh");
			system_ex(sta_buf);
			sleep(1);
			
		#endif
	    }
	    monitor_net_status();//刷新状态
	    sleep(2);
	}

}
int wifi_work_start(int wifi_mode/*,char *ssid*/)
{
	//此时 不管是AP还是STA 都是无网络的
	pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
	led_buzzer_info.led_status |= 1<<LED_NO_WIFI;
	pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);
	network_is_ok = 0 ;

	warning("wifi_work_start mode=%s\n",(wifi_mode == 0)?"AP":"STA" );
    if(wifi_mode == SDK_NET_MODE_AP)
	{
		return sdk_start_ap_server();
	}
    else
    {
        return sdk_start_sta_server();
    }
}

int sdk_stop_ap_server()
{
    stop_ap();
    return 0;
}

int sdk_stop_sta_server()
{
    stop_sta();
    return 0;
}

static int wifi_work_stop(int wifi_mode)
{
    if(wifi_mode == SDK_NET_MODE_AP)
    {
        return sdk_stop_ap_server();
    }
    else
    {
        return sdk_stop_sta_server();
    }
}




int sdk_start_net_server(NetStatusCB networkCB/*,char *ssid*/)
{
	int ret ;
	if(networkCB)
		g_NetworkCB = networkCB;

	g_manager.enable = 1;
	ret = pthread_create(&thread_manager_id,NULL,(void *) thread_net_manager,NULL);
	if (ret < 0)
	{
		printf( "thread_manager_net create error\n");
		return -1;
	}
	info("thread_net_manager start !!! \n");
	return 0;

}

//完全停止现有的网络状态 这个函数应该很少调用
int sdk_stop_net_server()
{
    wifi_work_stop(g_manager.current_mode);
    g_manager.enable = 0;
    pthread_cancel(thread_manager_id);
    return 0;

}

//在适当的位置调用 传入wifi_list  此函数只是执行查找并保存这个参数
//实际使用时的获取是下一个函数
int  sdk_search_ap(sdk_wifi_search_t *ap_list)
{
    if(!ap_list)
    {
        error("ap_list == NULL ");
    }
#if 1
    if(g_manager.current_mode == SDK_NET_MODE_AP) // ap 模式下 提前搜索好
    {
		get_wifi_list(ap_list);
        return 0;
    }
#endif	
    if(wifi_find_list() != 0)
    {
        return -1;
    }
    get_wifi_list(ap_list);

    return 0;
}

int  sdk_get_wifi_list(sdk_wifi_search_t *ap_list)
{
    if(NULL == ap_list) return -1;
    get_wifi_list(ap_list);

    return 0;
}


//判断关键参数的改变情况
static int refresh_net_server()
{
    pthread_mutex_lock(&g_manager.manager_mutex);
	/*printf("----------g_manager.current_mode=%d,g_ip_info.if_mode=%d\n",
					g_manager.current_mode,g_ip_info.if_mode);*/
					
    if(g_manager.current_mode != g_ip_info.if_mode  //模式变化
            ||strncmp(g_wifi_cfg.ssid,g_manager.ssid,MAX_DEV_ID_LEN) //ssid 变化
            ||strncmp(g_wifi_cfg.pwd,g_manager.passwd,MAX_PASSWD_LEN) //密码变化
      )
    {
		printf("g_manager.current_mode=%d g_ip_info.if_mode=%d\n",g_manager.current_mode,g_ip_info.if_mode);
		printf("g_wifi_cfg.ssid=%s g_manager.ssid=%s \n",g_wifi_cfg.ssid,g_manager.ssid);
		printf("g_wifi_cfg.pwd=%s g_manager.passwd=%s \n",g_wifi_cfg.pwd,g_manager.passwd);

        g_manager.config_change = 1;
        info(" config_change :%d  =========== \n",g_manager.config_change);

		
		if(g_manager.current_mode == SDK_NET_MODE_ETH)
		{
			strcpy(g_manager.if_name, "eth0"); //有线
		}
		else
		{
			strcpy(g_manager.if_name, OS_wifi0); //无线
		}


		//if(g_manager.current_mode != g_ip_info.if_mode)
		{
			g_manager.connect_status = 0; // 表示要重新连接一次
		}

        //下面是关键参数
        memset(g_manager.ssid,0,sizeof(g_manager.ssid));
		memset(g_manager.passwd,0,sizeof(g_manager.passwd));
		
        g_manager.current_mode = g_ip_info.if_mode;  //当前处于什么模式
        g_manager.enable_dhcp = g_ip_info.ip_info.enable_dhcp; //DHCP 功能

        strncpy(g_manager.ssid,g_wifi_cfg.ssid,strlen(g_wifi_cfg.ssid));
        strncpy(g_manager.passwd,g_wifi_cfg.pwd,strlen(g_wifi_cfg.pwd));

	 
    }

	if((1 == network_is_setting) &&(1 == g_manager.connect_status))
	{
		_os_set_ip_info(g_manager.if_name,&g_ip_info.ip_info);
		g_manager.enable_dhcp = g_ip_info.ip_info.enable_dhcp;
		network_is_setting = 0;
	}

    pthread_mutex_unlock(&g_manager.manager_mutex);

    return 0;
}


int  sdk_set_net_params(sdk_eth_cfg_t *net_param)
{
	if(!net_param)
	{
		error("net_cfg == NULL \n");
		return -1;
	}
	printf("net_param.if_mode=%d g_ip_info.info=%d dhcp=%d\n",net_param->if_mode,g_ip_info.if_mode,g_ip_info.ip_info.enable_dhcp);
	pthread_mutex_lock(&g_manager.manager_mutex);
	memcpy(&g_ip_info, net_param,sizeof(sdk_eth_cfg_t));
	network_is_setting = 1;
	printf("network_is_setting = %d\n",network_is_setting);
	pthread_mutex_unlock(&g_manager.manager_mutex);
	printf("net_param.if_mode=%d g_ip_info.info=%d dhcp=%d\n",net_param->if_mode,g_ip_info.if_mode,g_ip_info.ip_info.enable_dhcp);
	printf("sdk_set_net_params end!\n");
	return 0;
}

int  sdk_get_net_params(sdk_eth_cfg_t *net_param)
{
    if(!net_param)
    {
        error("net_cfg == NULL \n");
        return -1;
    }

    strncpy(net_param->ip_info.ip_addr, g_manager.ip_addr, MAX_IP_LEN) ;
    strncpy(net_param->ip_info.mask, g_manager.mask, MAX_IP_LEN);
    strncpy(net_param->ip_info.gateway, g_manager.gateway, MAX_IP_LEN);
    strncpy(net_param->ip_info.dns1, g_manager.dns1, MAX_IP_LEN);
    strncpy(net_param->ip_info.dns2, g_manager.dns2, MAX_IP_LEN);
    strncpy(net_param->ip_info.mac, g_manager.mac, MAX_IP_LEN);

	net_param->if_mode = g_manager.current_mode;
    net_param->ip_info.enable_dhcp = g_manager.enable_dhcp;             //是否开启DHCP
	net_param->wifi_type = g_manager.wifi_type;
	info("==========net_param->if_mode:%d mac:%s \n",net_param->if_mode,net_param->ip_info.mac);
    return 0;
}

int sdk_set_wifi_params(sdk_wifi_cfg_t *wifi_cfg)
{
    if(!wifi_cfg)
    {
        error("wifi_cfg == NULL \n");
        return -1;
    }
    pthread_mutex_lock(&g_manager.manager_mutex);

    memcpy(&g_wifi_cfg,wifi_cfg,sizeof(sdk_wifi_cfg_t));
    pthread_mutex_unlock(&g_manager.manager_mutex);

    //refresh_net_server();
    return 0;
}

//写入mac地址
int sdk_set_mac_params(const char *if_name,const char *mac)
{
    return os_set_mac_addr(if_name,mac);
}

int sdk_get_wifi_mode()
{
	warning("g_manager.current_mode=%s\n",g_manager.current_mode == SDK_NET_MODE_AP?"   AP":"	STA");
	if(1 == g_manager.connect_status)
	{
		if(SDK_NET_MODE_AP == g_manager.current_mode)
		{
			return  0;
		}
		else if(SDK_NET_MODE_WIFI == g_manager.current_mode)
		{
			return 1;
		}
	}
	else
	{
		return 0;
	}
	
}

int sdk_get_eth_mode()
{
	if(1 == g_manager.connect_status)
	{
		return (g_manager.current_mode == SDK_NET_MODE_ETH ? 1:0);
	}
	else
	{
		return 0;
	}
	
}


static int g_net_factory_flag;//ap模式

int sdk_set_wifi_mode( int flag )
{
	g_net_factory_flag=flag;
	return 0;
}

int check_wpacli_state()
{
	char csConnState[256];
    unsigned char ucConnState;
	if(access("/var/run/wpa_supplicant.pid", F_OK) != 0)
	{
		return -1;
	}
	FILE *fp = popen("wpa_cli -iwlan0 status | grep wpa_state", "r");
	if(fp != NULL)
	{
		fread(csConnState, sizeof(char), sizeof(csConnState), fp);
		pclose(fp);
	}
    ucConnState=(strstr(csConnState, "COMPLETED")!=NULL?1:0);
    return ucConnState;
}

/**
* check if the wireless_info.conf file exist or not
*	return 0 if it existed, or return -1
*/
int check_config_wireless()
{
	if(!access(WIFI_CONFIG_INI, F_OK))
		return 0;
	else
		return -1;
}

int check_iwlist_scanning(char *inSsid, char *inKey, WiFi_Item_Config *poutItemInfo)
{
	FILE *fp_scanning = NULL;
	char buf[1024];
	char *ptmp, *ptmp1;
	Ap_Info head;
	Ap_Info *pap = NULL,*pcur = NULL, *prev = NULL, *tmp;
	int itmp;
	int iMatched=0;

	int iApCount=0;
	int isAuthOn=-1;  //-1--uninitialize, 0-off, 1-on
	int sec_mode=0;   // 1-wep, 2-wpa 0-none
	int isApinfoEnd=0;
	int isApinfoHead=0;
	
	fp_scanning = popen("iwlist wlan0 scanning", "r");
	if(fp_scanning == NULL)
	{
		return -1;
	}

	memset(&head, 0, sizeof(Ap_Info));

	while(fgets(buf, sizeof(buf), fp_scanning) != NULL)
	{
		//printf("%s", buf);	usleep(10000);
		//it is a new Ap
		//Cell 01 - Address: D4:12:BB:08:CB:64
		if(((ptmp=strstr(buf, "Address")) != NULL) && (isApinfoHead==0))
		{
			isApinfoHead = 1;
			pap = (Ap_Info*)malloc(sizeof(Ap_Info));
			memset(pap, 0, sizeof(Ap_Info));

			ptmp += strlen("Address:");
			ptmp++; //skip the whitespace
			ptmp1 = strchr(ptmp, '\n');
			if(ptmp1 != NULL) *ptmp1='\0';
			snprintf(pap->mac, sizeof(pap->mac), "%s", ptmp);
		}
		else if((ptmp=strstr(buf, "ESSID")) != NULL)
		{	
			ptmp += strlen("ESSID:");
			ptmp++;//skipped the \"  
			ptmp1 = strchr(ptmp, '"');
			if(ptmp1 != NULL) *ptmp1='\0';//replace \n to \0
			snprintf(pap->ssid, sizeof(pap->ssid), "%s", ptmp);
		}
		else if((ptmp=strstr(buf, "Channel")) != NULL)
		{
			ptmp += strlen("Channel");
			ptmp++; //skip the whitespace
			itmp = strtol(ptmp, NULL, 10);
			pap->channel = itmp;
		}
		else if((ptmp=strstr(buf, "IE:")) != NULL)
		{
			if((ptmp=strstr(buf, "WPA")) != NULL)
				sec_mode = 2;
			else if((ptmp=strstr(buf, "WEP")) != NULL) 
				sec_mode = 1;
		}
		else if((ptmp=strstr(buf, "Encryption key:on")) != NULL)
		{
			isAuthOn = 1;
		}
		else if((ptmp=strstr(buf, "Encryption key:off")) != NULL)
		{
			isAuthOn = 0;
		}
		
		if(isAuthOn >= 0)		
		{			
			if(isAuthOn == 0)			
				snprintf(pap->secrity, sizeof(pap->secrity), "%s", "NONE");
			if((isAuthOn > 0) && (sec_mode == 1))
				snprintf(pap->secrity, sizeof(pap->secrity), "%s", "WEP");
			if((isAuthOn > 0) && (sec_mode == 2))
				snprintf(pap->secrity, sizeof(pap->secrity), "%s", "WPA-PSK");
			
			//reset encropytion type params, and end reading an ap's info
			sec_mode = 0;
			isAuthOn = -1;
			isApinfoEnd = 1;// read a new ap's info over
		}

		if(isApinfoEnd == 1)		
		{			
			if(head.next == NULL)			
			{				
				head.next = pap;			
			}			
			else			
			{				
				pcur->next = pap;			
			}			
			iApCount++;			
			pcur = pap;	
			isApinfoEnd = 0;
			isApinfoHead = 0;
		}
	}
	
	pcur = head.next;
	while(pcur != NULL)
	{
		//printf("^^^ SSID=%s\n", pcur->ssid);
		//printf("^^^ MAC=%s\n", pcur->mac);
		//printf("^^^ SEC=%s\n", pcur->sec);
		if((strcmp(pcur->ssid, inSsid) == 0))
		{
			snprintf(poutItemInfo->ssid, sizeof(poutItemInfo->ssid), "%s", pcur->ssid);
			snprintf(poutItemInfo->bssid, sizeof(poutItemInfo->bssid), "%s", pcur->mac);
			snprintf(poutItemInfo->secrity, sizeof(poutItemInfo->secrity), "%s", pcur->secrity);
			snprintf(poutItemInfo->pwd, sizeof(poutItemInfo->pwd), "%s", inKey);
		printf("====> find the match router, ssid=%s, mac=%s, sec=%s\n", poutItemInfo->ssid, poutItemInfo->bssid, poutItemInfo->secrity);
			iMatched = 1;
			poutItemInfo->status = 1;
		}

		tmp = pcur;
		pcur = pcur->next;
		free(tmp);
    }

    pclose(fp_scanning);
	
	return iMatched;
}


int check_parse_result(char *ssid_buf, int ssid_Len, char *pwd_buf, int pwd_Len)
{
	FILE *fp_info;
	char pData[64];
	char *str;
	char *p;

	fp_info = fopen(WIFI_CONFIG_INI, "r");
	if(fp_info != NULL)
	{
		while(fgets(pData, sizeof(pData), fp_info) != NULL)
		{
			p=pData;
			while(*p != '\0')
			{
				if(*p == '\n')
					*p = '\0';
				p++;
			}
			if((str=strstr(pData, "SSID=")) != NULL)
			{
				str += strlen("SSID=");
				snprintf(ssid_buf, ssid_Len, "%s", str);
				//ssid_buf[strlen(ssid_buf)] = '\0';
			}
			else if((str=strstr(pData, "PASSWORD=")) != NULL)
			{
				str += strlen("PASSWORD=");
				snprintf(pwd_buf, pwd_Len, "%s", str);
				//pwd_buf[strlen(pwd_buf)] = '\0';
			}
		}
		fclose(fp_info);
	}

	return 0;
}

int check_connect_handle(WiFi_Item_Config *pItemApInfo)
{
	int secrity_type;// 0: OPEN-NONE; 1: WEP-OPEN; 2: WEP-SHARED; 3: WPA-PSK/WPA2-PSK
	char cmd[256];
	int iFlag=0;
	
	if(!strcmp(pItemApInfo->secrity, "WPA-PSK"))
		secrity_type = 3;
	if(!strcmp(pItemApInfo->secrity, "WEP"))
		secrity_type = 2;  // cannot distinguish OPEN-WEP or SHARED-WEP at present 
	if(!strcmp(pItemApInfo->secrity, "NONE"))
		secrity_type = 0;

	if(access("/tmp/wifi_connect_cmd", F_OK) != 0)
	{
		system("cp -f /mnt/mtd/run/rtl_wifi_cmd.sh /tmp/wifi_connect_cmd.sh");
		snprintf(cmd, sizeof(cmd), "sed -i \"s/@SSID@/%s/g\" /tmp/wifi_connect_cmd.sh", pItemApInfo->ssid);
		system(cmd);
		snprintf(cmd, sizeof(cmd), "sed -i \"s/@PWD@/%s/g\" /tmp/wifi_connect_cmd.sh", pItemApInfo->pwd);
		system(cmd);
	}
	snprintf(cmd, sizeof(cmd), "/tmp/wifi_connect_cmd.sh %d", secrity_type);
	printf("###### will system cmd: %s\n", cmd);
	while(1)
	{
		if(iFlag == 0)
		{
			system(cmd);
			iFlag = 1;
			printf("[debug]run wifi cmd done...\n");
		}
		if(check_wpacli_state() > 0)
			break;
		else
			sleep(3);
	}
	g_network_info.ucNetworkState = 1;

	system("killall udhcpc");
	system("wan_udhcpc.sh wlan0 &");
	while(1)
	{
		if(!access("/var/run/udhcpc_wlan0_pid.txt", F_OK))
			break;
		else
			sleep(2);
	}
}

int check_wifi_connect()
{
	char myssid[64];
	char mypwd[64];
	unsigned char ucMatched = 0;
	int iCount;

	check_parse_result(myssid, sizeof(myssid), mypwd, sizeof(mypwd));
	
	WiFi_Item_Config ItemApInfo;
	memset(&ItemApInfo, 0, sizeof(ItemApInfo));
	for(iCount = 0; iCount < 5; iCount++) // give 5 chance to scann and find matched ssid
	{
		ucMatched = check_iwlist_scanning(myssid, mypwd, &ItemApInfo);
		if(ucMatched > 0)
			break;
		else
			sleep(3);
	}
	if(ucMatched < 0 || iCount >= 5)
	{
		printf("########## Error: connot find the matched router, please reset the camera!\n");
		return -1;
	}
	
	check_connect_handle(&ItemApInfo);
	
	return 0;
}

void *check_network_thread(void *arg)
{
	int isEhernet;
	char result[32];
	int ret;
	memset(&g_network_info, 0, sizeof(g_network_info));
	
	while(1)
	{	
	    isEhernet = eth_connect_stat();
		if(isEhernet > 0)
			g_network_info.ucNetworkType = 0; // ethernet
		else
			g_network_info.ucNetworkType = 1; // wireless
			
		if(g_network_info.ucNetworkinit == 0)
			g_network_info.ucNetworkinit = 1;

		if((1 == g_network_info.ucNetworkType) && (1 == g_network_info.ucNeedreconfig))
		{
			g_network_info.ucNeedreconfig = 0;
			g_network_info.ucNetworkState = 0;
			unlink("/tmp/wifi_connect_cmd");
		}
		
	    if((1 == g_network_info.ucNetworkType) && (0 == g_network_info.ucNetworkState))
	    {
	      	info("wifi  work+_+_+__+_+_+_+_+__+_+_+_+_+_+_+_+_ \n");
			if(check_config_wireless() < 0)
			{
				sleep(2);
				continue;
			}
			else
			{
				check_wifi_connect();
				os_get_ip_addr("wlan0", result, sizeof(result));
				if(strlen(result) > 0)
				{
					printf("---------> Camrea has been link to internet!\n");
					g_network_info.ucNetworkState = 2;
				}
			}
	    }
	    else if(g_network_info.ucNetworkState == 0) 
	    {
			info("eth0  work+_+_+__+_+_+_+_+__+_+_+_+_+_+_+_+_ \n");
			
			system("wan_udhcpc.sh eth0 &");
			g_network_info.ucNetworkState = 1;
			do{
				memset(result, 0, sizeof(result));
				ret = os_get_ip_addr("eth0", result, sizeof(result));
				sleep(3);
			}while(ret < 0);
			g_network_info.ucNetworkState = 2;
	    }
		
	    //monitor_net_status();//刷新状态
	    sleep(2);
	}
}


int sdk_network_server_start()
{
	int ret;
	pthread_t network_thread = -1;
	ret = pthread_create(&network_thread,NULL,(void *)check_network_thread,NULL);
	if (ret < 0)
	{
		printf( "thread_manager_net create error\n");
		return -1;
	}
	info("thread_net_manager start !!! \n");
	return 0;
}


