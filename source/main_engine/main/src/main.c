/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description: 主控模块实现
 *
 *        Version:  1.0
 *        Created:  2016年11月1日9时56分41秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 * =====================================================================================
 */

#include "wrapper.h"
#include "log.h"
//#include "ipcs.h"

#include "main.h"
#include "paramlib.h"
#include "streamlib.h"
#include "main_msg.h"
//#include "timer.h"
#include "adapt.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
//#include <linux/in.h>
#include <stdlib.h>
#include <sys/socket.h>


extern void main_event_cb(void *param);
extern void main_audio_stream_cb(void *param);
extern void main_video_stream_cb(void *param);
extern int power_read_rtc();
RECORD_STATUS record_status;
aws_status  aws;

LED_BUZZER_INFO led_buzzer_info;

///////////////////////
sdk_Network_Info g_network_info;
int g_main_dev_type = 0;
int gm_upgrade_is_file_mode = 0;

static int g_hd_is_exist = 0;
static int g_dev_valid_ch = 0;
static int g_probe_chip_num = 1;

static int _msg_dispatch(sdk_msg_t *pmsg);
static int net_ptz_dispatch(sdk_msg_t *pmsg);

//wch add tmp
char *string2hexascii(char *inString, char *outHexstr, int max_size)
{
	char *p = inString;
	int len;
	
	while(*p != '\0')
	{
		len = strlen(outHexstr);
		snprintf(&outHexstr[len], max_size-len, "%X", *p);
		p++;
	}
	len = strlen(outHexstr);
	outHexstr[len] = '\0';
	return outHexstr;
}

/* ==================== 系统启动 ================== */
static void int_handler(int signo)
{
	printf("revice signal! int_handler\n");
    error("SIGINT exit !!!\n");
    adapt_av_server_stop();
    exit(0);
}

static void pipe_handler(int signo)
{
	//printf("revice signal! pipe_handler\n");
   // adapt_av_server_stop();
    //error("SIGPIPE ERROR!!!\n");
}
static void sigstop(int arg)
{
	printf("revice signal! sigstop\n");
    adapt_av_server_stop();
    exit(1);
}

typedef void (*sighandler_t)(int);
static void reg_signal_handler(void)
{

    printf("signal SIGPIPE, ret:%d\n"   ,signal(SIGPIPE, (sighandler_t)pipe_handler));
    printf("signal SIGINT, ret:%d\n"    ,signal(SIGINT, (sighandler_t)int_handler));
    printf("signal SIGQUIT, ret:%d\n"   ,signal(SIGINT, (sighandler_t)sigstop));
    printf("signal SIGTERM, ret:%d\n"   ,signal(SIGINT, (sighandler_t)sigstop));
    //printf("signal SIGINT, ret:%d\n"  , signal(SIGINT, (sighandler_t)int_handler));


}
static int sys_reset_delay_task(void *parm)
{
    char msg_buf[MAX_MSG_SIZE] = {0};
    sdk_msg_t *pmsg = (sdk_msg_t *)msg_buf;

    warning("===> main_msg_close_system.\n");
    pmsg->msg_id = SDK_MAIN_MSG_CLOSE_SYSTEM;
    pmsg->args = SDK_CLOSE_SYS_REBOOT;
    main_msg_close_system(pmsg);

    return 0;
}
/*----------------------------------*/
//视频
#define VIDEO_DATA_SPACE		    (4*512*1024)    /* 默认空间(8*512*1024) */
#define VIDEO_INDEX_FRAME_NUM       (4*512)	        /* 默认 index num */
#define VIDEO_MAX_FRAME_SIZE        (512*1024)


#define VIDEO_SLAVE_DATA_SPACE	    (3*256*1024)    /* 默认空间 */
#define VIDEO_SLAVE_INDEX_NUM	    (3*512)
#define VIDEO_SLAVE_MAX_FRAME_SIZE	(200*1024)

//音频
#define AUDIO_DATA_SPACE	        (256*1024)      /* 默认空间 */
#define AUDIO_INDEX_NUM	            (2*512)
#define AUDIO_MAX_FRAME_SIZE	    (2*1024)

static stream_ch_info_t g_ch_stream_info[2][4] =
{
    //  主码流
    {   {AUDIO_DATA_SPACE		,AUDIO_INDEX_NUM		,AUDIO_MAX_FRAME_SIZE},
        {VIDEO_DATA_SPACE		,VIDEO_INDEX_FRAME_NUM	,VIDEO_MAX_FRAME_SIZE},
        {VIDEO_SLAVE_DATA_SPACE	,VIDEO_SLAVE_INDEX_NUM	,VIDEO_SLAVE_MAX_FRAME_SIZE},
        {VIDEO_SLAVE_DATA_SPACE	,VIDEO_SLAVE_INDEX_NUM	,VIDEO_SLAVE_MAX_FRAME_SIZE},
    },

    //没有用到
    {   {AUDIO_DATA_SPACE		,AUDIO_INDEX_NUM		,AUDIO_MAX_FRAME_SIZE},
        {VIDEO_DATA_SPACE		,VIDEO_INDEX_FRAME_NUM	,VIDEO_MAX_FRAME_SIZE},
        {VIDEO_SLAVE_DATA_SPACE	,VIDEO_SLAVE_INDEX_NUM	,VIDEO_SLAVE_MAX_FRAME_SIZE},
        {VIDEO_SLAVE_DATA_SPACE	,VIDEO_SLAVE_INDEX_NUM	,VIDEO_SLAVE_MAX_FRAME_SIZE},
    },

};

static int get_ch_streamInfoCB(int ch, int ch_type, stream_ch_info_t *ch_stream_info)	/* 获取没个通道的配置的最大编码能力按此分配空间 为空时设置默认值 */
{
    //printf("get_ch_streamInfoCB 000000000 ch:%d  ch_type:%d \n",ch, ch_type);
    if((ch < 2 && ch >= 0)&&(ch_type >= 0 &&ch_type < 2))
    {
        memcpy(ch_stream_info,&g_ch_stream_info[ch_type][ch],sizeof(stream_ch_info_t));
#if 0
        printf("get_ch_streamInfoCB>>> main_ch:%d sub_ch:%d total_space:%x   index_num:%x  max_frame_size:%x \n",ch,ch_type
               ,ch_stream_info->total_space
               ,ch_stream_info->index_num
               ,ch_stream_info->max_frame_size);
#endif
    }
    return 0;
}
static int stream_init()
{
    stream_manage_t stream_manage;
    
    memset(&stream_manage, 0, sizeof(stream_manage));
    //流库使用注册
    stream_manage.ch_num = 2;
    stream_manage.two_stream = SDK_STREAM_TWO_STREAM;
    stream_manage.get_recordStatCB = NULL;//st_record_getChannelRecordStat;
    stream_manage.get_contrlTypeCB = NULL;//st_record_getStartRecordFlag;
    stream_manage.get_ch_streamInfoCB = get_ch_streamInfoCB; //如果系统默认缓冲区分配的不合理  可以自己重新分配
    sdk_stream_init(&stream_manage);

}
static  pthread_mutex_t g_msg_mutex;

int g_main_run = 1;
int  stop_main_server()
{
		g_main_run = 0;
		return 0;
}

#define IP_FOUND "JOOAN"
#include "tutk.h"
#include <sys/statfs.h> 

static void Get_SD_TotalAndFreeSize(int *total,int *free)
{
	struct statfs diskInfo; 		
	/*
	  *  get SD card information	
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
	
	
	if(0 > get_sd_stats())//判断SD卡是否存在
	{
		  *total = 0;
	}
	else
	{
		*total = totalsize>>30;
	}
	
	*free = freeDisk>>30;

}
static void UDPBoardcastThread()
{
#define MSG_BUF_SIZE 1024

	int ret = -1;
    int sock;
    struct sockaddr_in server_addr; //服务器端地址
    struct sockaddr_in from_addr; //客户端地址
    int from_len = sizeof(struct sockaddr_in);
    int count = -1;
    fd_set readfd; //读文件描述符集合
	char msg_buf[MSG_BUF_SIZE] = {0};
	SDK_MSG_DEVICEINFO resp;
	
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0); //建立数据报套接字
    if (sock < 0)
    {
        perror("sock error");
        return -1;
    }

    memset((void*) &server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;//inet_addr("239.255.255.250"); 
	server_addr.sin_port = htons(7788); //服务端监听端口 

    //将地址结构绑定到套接字上
    ret = bind(sock, (struct sockaddr*) &server_addr, sizeof(server_addr));
    if (ret < 0)
    {
        perror("bind error");
        return -1;
    }

    /**
     * 循环等待客户端
     */
    while (1)
    {
        timeout.tv_sec = 100;
        timeout.tv_usec = 0;

        //文件描述符集合清0
        FD_ZERO(&readfd);

        //将套接字描述符加入到文件描述符集合
        FD_SET(sock, &readfd);

        //select侦听是否有数据到来
        ret = select(sock + 1, &readfd, NULL, NULL, &timeout); //侦听是否可读
        switch (ret)
        {
        case -1: //发生错误
            perror("select error:");
            break;
        case 0: //超时
            printf("select timeout\n");
            break;
        default:
            if (FD_ISSET(sock,&readfd))
            {
                memset(msg_buf,0x0,MSG_BUF_SIZE);
                count = recvfrom(sock, msg_buf, MSG_BUF_SIZE, 0,
                        (struct sockaddr*)&from_addr, &from_len); //接收客户端发送的数据

                //from_addr保存客户端的地址结构
                if (strstr(msg_buf, IP_FOUND))
                {
					//将数据发送给客户端
					//响应客户端请求
					//打印客户端的IP地址和端口号
					printf("Client connection information:\n\t IP: %s, Port: %d,%s\n",(char *)inet_ntoa(from_addr.sin_addr),
						   ntohs(from_addr.sin_port),msg_buf);

					memset(&resp,0x0,sizeof(SDK_MSG_DEVICEINFO));
                	De_get_device_info_handle(&resp);
					Get_SD_TotalAndFreeSize(&resp.total,&resp.free);
					printf(">>>>>>>resp.total===%d\n",resp.total);
					printf(">>>>>>>resp.szmacaddr===%s\n",resp.szMacAddr);
					printf(">>>>>>>resp.szpwd===%s\n",resp.szPwd);
					printf(">>>>>>>resp.szuid===%s\n",resp.szUID);
                    count = sendto(sock, (const void *)&resp, sizeof(SDK_MSG_DEVICEINFO), 0,
                            (struct sockaddr*) &from_addr, from_len);
                }
            }
            break;
        }
    }
    return 0;
	
}
void *ClearMemory(void *args)
{
    while(1)
    {
       sleep(1000);
       system_ex("echo 1 > /proc/sys/vm/drop_caches");
	   
    }
}
void ByTimeDurationClearMemory()
{
	pthread_attr_t attr;
	pthread_t clear_thread;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);  //分离的线程
	pthread_create(&clear_thread, &attr, ClearMemory,NULL);
	pthread_attr_destroy(&attr);

}

int main(int argc, char *argv[])
{
    int ret = 0;
    int led_handle = 0;
    int dm_handle = 0;
    int spot_handle = 0;
    int tour_handle = 0;
	sdk_sys_cfg_t  sys_cfg;
	sdk_motion_cfg_t  motion_cfg;
	sdk_audio_detect_cfg_t audio_detect_cfg;
	
	memset(&sys_cfg,0,sizeof(sdk_sys_cfg_t));
	adapt_param_get_sys_cfg(&sys_cfg);
    
    memset(&motion_cfg, 0, sizeof(sdk_motion_cfg_t));
    memset(&audio_detect_cfg, 0, sizeof(sdk_audio_detect_cfg_t));

    //初始化日志模块；
    ret = sdk_log_init(LOG_DBG_STDERR|LOG_DBG_MEM|LOG_LOG_FILE, NULL, "/config/log/.syslog");
#if 1
    if (os_is_file_exist("/mnt/mtd/debug") != 1)
    {
        // 默认正式版本，无打印
        error("=================  Official Version  =================\n");

		//wch modify here: will detect the watch to a single process.
		// Because when do upgrade, will kill the mainengine process.
        //启动喂狗线程 防止程序意外崩溃
        watchdog_server_start();
		
        sdk_dbg_level(0);
    }
    else
    {
        // debug版本，带打印
        error("=================  Debug Version  =================\n");
    }
    info("****************** sdk_log_init, ret:%d\n", ret);
#endif


#if  1
    //初始化信号处理；
    reg_signal_handler();
    printf("reg_signal_over!\n");
#endif

#if 1
    //初始化参数模块；
    ret = adapt_param_init();
    info("****************** adapt_param_init, ret:%d\n", ret);
    g_dev_valid_ch = adapt_param_get_chnum();	// 20
    g_main_dev_type = sys_cfg.dev_type;
    warning(" g_dev_valid_ch:%d, chip_num:%d, g_main_dev_type:%d, argv[0]:%s, argv[1]:%s\n"
            , g_dev_valid_ch
            , g_probe_chip_num
            , g_main_dev_type
            , argv[0]
            , argv[1]);
#endif

	
	//start_keep_live();//守护进程同步开启

	//power_read_rtc();
	//从文件里读取保存的时间戳
	//power_read_time_form_file();
	
#if 0
//读取icut标志位
	sdk_image_attr_t image_cfg;
	memset(&image_cfg,0,sizeof(image_cfg));
	adapt_param_get_image_attr(0, &image_cfg);
	printf("read icut flag=%d\n",image_cfg.icut_flag);

	if(1 == image_cfg.icut_flag) //强制关闭红外
	{
		printf("force close icut\n");
		adapt_set_infrared_status(1);
	}
	else
	{
		printf("auto icut\n");
		adapt_set_infrared_status(0);
	}
#endif


	stream_init();
	st_record_init(NULL);
	//sleep(1); //等待SD卡成功挂载
#if 0
	led_buzzer_init(); //把led线程提前至此  WIFI也需要LED
	usleep(10000);

#if 0
	// network module init...
	printf("^^^^^^^^^^^ network module init...\n");
    adapt_init_net();
    adapt_start_net_server(&sys_cfg);//启动网络管理服务
#endif
#endif
	//wch add
	adapt_network_init();


#if 1
    info("start av server!!! \n");
    //启动编码模块
    adapt_av_init(main_event_cb, main_video_stream_cb, main_audio_stream_cb, NULL);
    adapt_av_server_start();
    info("av server start now \n");
        
#endif

#if 0

    //启动事件模块
    event_init();
    event_server_start();
    
#endif

#if 0

	memset(&aws,0,sizeof(aws));
	sdk_date_t time_cfg;
	memset(&time_cfg, 0, sizeof(time_cfg));
	adapt_param_get_systime(&time_cfg);
	printf("+++++++++++++++++++++timezone=%d+++++++++++++++++++++++++++\n",time_cfg.tzone);
	reflush_timezone(time_cfg.tzone);
#endif

	//ntp同步线程
	ntp_update_thread();
	

    /**************/
    {
        // 开机日志
        //main_msg_startup_log();
    }
	
    //启动TUTK 服务
    #if 0
    adapt_tutk_init(_msg_dispatch,NULL);
	#endif
	//启动本地服务器用于私有通信
	//local_server_start(_msg_dispatch);

	adapt_dana_init(_msg_dispatch,NULL);
	
	#if 0 //wch modify
	while(!adapt_get_wifi_mode())
	{
		info(" WAITE WIWI STA MODE!!!  \n");
		sleep(2);
	}
	sleep(5);
	#endif
	
	g_main_run  =1 ;
		//读取保存的录像参数 决定是否上电就开始录像
	#if 1
	
	sdk_record_cfg_t record_param;
	sdk_motion_cfg_t motion_param;
	memset(&record_status,0,sizeof(record_status));
	memset(&record_param,0,sizeof(sdk_record_cfg_t));
	memset(&motion_param,0,sizeof(sdk_motion_cfg_t));
	adapt_param_get_record_cfg(0,&record_param);
	adapt_param_get_motion_cfg(0,&motion_param);
	record_status.record_flags = record_param.record_types;//读取到的录像类型
	record_status.record_last_status = 0;
	record_status.record_alarm_time = motion_param.alarm_handle.record_time; //读取到的报警录像时长
	record_status.md_interval_time	= motion_param.sched_time.interval_time;//间隔时间
	printf("record_status.md_interval_time=%d\n",record_status.md_interval_time);
	printf("motion_param.sensitive=%d,record_param.record_types==%d\n",motion_param.codec_vda_cfg.sensitive,record_param.record_types);
	printf("record_flags=%d %d %d\n",record_status.record_flags,record_status.record_last_status,record_status.record_alarm_time);

	dana_record_init();
	
	#endif

	//AWS
	#if 0
	sdk_encode_t encode;
	sdk_aws_info_t aws_cfg;
	sdk_date_t time_zone;
	memset(&encode,0,sizeof(encode));
	//memset(&aws,0,sizeof(aws)); //提至时区前面去
	memset(&time_zone,0,sizeof(time_zone));
	adapt_param_get_encode_cfg(0, &encode);//获取编码参数
	adapt_param_get_aws_cfg(&aws_cfg);
	adapt_param_get_systime(&time_zone);
	//还需要获取保存的FULL/ALARM 类型aws.full_or_alarm
	aws.full_or_alarm 	= aws_cfg.aws_record_status;
	aws.alarm_time 		= motion_param.alarm_handle.record_time;//报警上传的时长=报警录像的时长
	aws.current_fps 	= encode.av_enc_info[0].frame_rate;		//设置当前帧率
	aws.time_zone 		= time_zone.tzone;
	printf("aws.full_or_alarm=%d\n",aws.full_or_alarm);
	printf("aws.alarm_time=%d\n",aws.alarm_time);
	printf("aws.time_zone=%d\n",aws.time_zone);
	aws_init();
	#endif
	
	pthread_t createboardThread;
	pthread_create(&createboardThread,NULL,(void *)UDPBoardcastThread,NULL);
	ByTimeDurationClearMemory();
	ShceduleRecordeByCurrentTime();

    while(g_main_run)
    {
 #if 0
		if(1 == dhcp_enable)
		{
			if(1 == adapt_get_wifi_mode()) //STA时才dhcp 
			{
				info("start  udhcpc wireless!!!\n");
				sleep(2);
				system("wan_udhcpc.sh wlan0 &");
			}
			if( 1 == adapt_get_eth_mode())
			{
				info("start  udhcpc  ethernet!!!\n");
				sleep(2);
				system("wan_udhcpc.sh eth0 &");
			}
			dhcp_enable = 0;
		}
#endif
        usleep(1000000);
        continue;
    }
	#if 0
	adapt_tutk_stop();
	#endif
	st_record_deinit();//停止录像
	adapt_av_server_stop();//停止编码服务
	sdk_stream_destroy();
	info("exit all server...... \n");
	
    return 0;
}
// 释放系统资源
int main_free_system_resources()
{
#if 0
    adapt_baidu_stop();
#endif
    event_uninit();//释放事件库
    return 0;
}

/* ==================== 消息处理 ================== */
typedef struct msg_desc_s
{
    int  id;
    int(*local_msg_cb)(sdk_msg_t *pmsg);
    int(*remote_msg_cb)(sdk_msg_t *pmsg);
    char desc[128];
} msg_desc_t;


/*
 * main map tab;
 * ID顺序与main.h:SDK_MAIN_MSG_ID_E顺序一致;
 */
static msg_desc_t _msg_desc[SDK_MAIN_MSG_BUTT] =
{
     {SDK_MAIN_MSG_KEEPLIVE         , NULL                   , NULL    ,"NULL"}
    ,{SDK_MAIN_MSG_VERSION          , main_msg_version       , NULL    ,"SDK_MAIN_MSG_VERSION"}          //版本信息    sdk_version_t 版本信息                       1
    ,{SDK_MAIN_MSG_SYS_CFG          , main_msg_sys_cfg       , NULL    ,"SDK_MAIN_MSG_SYS_CFG"}          //系统配置    sdk_sys_cfg_t                                2
    ,{SDK_MAIN_MSG_STATUS           , main_msg_status        , NULL    ,"SDK_MAIN_MSG_STATUS"}           //系统状态    sdk_status_t                                 3
    ,{SDK_MAIN_MSG_PARAM_DEFAULT    , main_msg_param_def     , NULL    ,"SDK_MAIN_MSG_PARAM_DEFAULT"}    //默认参数    sdk_default_param_t, SDK_PARAM_MASK_ID_E     4
    ,{SDK_MAIN_MSG_CLOUD_ID         , NULL                   , NULL    ,"SDK_MAIN_MSG_CLOUD_ID"}         //云ID        SDK_UPGRAD_OP_E                              5

    ,{SDK_MAIN_MSG_WIFI_SEARCH      , main_msg_search_wifilist, NULL    ,"SDK_MAIN_MSG_WIFI_SEARCH"}      //wifi查询    SDK_MAIN_MSG_WIFI_SEARCH                     6
    ,{SDK_MAIN_MSG_LOG              , main_msg_log           , NULL    ,"SDK_MAIN_MSG_LOG"}              //日志        SDK_LOG_OP_E                                 7
    ,{SDK_MAIN_MSG_NET_MNG_CFG      , main_msg_net_server_cfg, NULL    ,"SDK_MAIN_MSG_NET_MNG_CFG"}     //网络管理配置    sdk_net_mng_cfg_t                        8
    ,{SDK_MAIN_MSG_PTZ_CFG          , main_msg_ptz_cfg       , NULL    ,"SDK_MAIN_MSG_PTZ_CFG"}          //云台配置    sdk_ptz_param_t                              9
    ,{SDK_MAIN_MSG_PTZ_CONTROL      , NULL      			 , NULL    ,"SDK_MAIN_MSG_PTZ_CONTROL"}      //云台控制                                                 10

    ,{SDK_MAIN_MSG_BAIDU_INFO	    , main_msg_baidu_info 	 , NULL    ,"SDK_MAIN_MSG_BAIDU_INFO"}	     //设备第一次配置信息  sdk_register_cfg_t                   11
    ,{SDK_MAIN_MSG_WIFI_PARAM_CFG	, main_msg_wifiparam_cfg , NULL    ,"SDK_MAIN_MSG_WIFI_PARAM_CFG"}	 //WIFI配置 	sdk_wifi_cfg_t                   			12
    ,{SDK_MAIN_MSG_CLOSE_SYSTEM		, main_msg_close_system	 , NULL    ,"SDK_MAIN_MSG_CLOSE_SYSTEM"}	 //关闭系统  SDK_CLOSE_SYS_OP_E  							13
    ,{SDK_MAIN_MSG_ENCODE_CFG       , main_msg_encode_cfg    , NULL    ,"SDK_MAIN_MSG_ENCODE_CFG"}       //编码配置       sdk_encode_t   							14
    ,{SDK_MAIN_MSG_RECORD_CFG       , main_msg_record_cfg    , NULL    ,"SDK_MAIN_MSG_RECORD_CFG"}       //录像配置       sdk_record_cfg_t  						15

    ,{SDK_MAIN_MSG_RECORD_QUERY     , main_msg_record_query  , NULL    ,"SDK_MAIN_MSG_RECORD_QUERY"}     //录像查询       sdk_record_item_t, sdk_record_cond_t 		16
    ,{SDK_MAIN_MSG_MOTION_CFG       , main_msg_motion_cfg    , NULL    ,"SDK_MAIN_MSG_MOTION_CFG"}	     //视频移动配置   sdk_motion_cfg_t           17
    ,{SDK_MAIN_MSG_ALARM_IN_CFG     , main_msg_alarm_in_cfg  , NULL    ,"SDK_MAIN_MSG_ALARM_IN_CFG"}     //外部报警输入配置sdk_alarm_in_cfg_t  //hongmb:这个用本地的,原因见pu_MsgHandle 1650行  18
    ,{SDK_MAIN_MSG_HIDE_CFG         , main_msg_hide_cfg      , NULL    ,"SDK_MAIN_MSG_HIDE_CFG"}         //视频遮挡配置   sdk_hide_cfg_t   							19
    ,{SDK_MAIN_MSG_LOST_CFG         , main_msg_lost_cfg      , NULL    ,"SDK_MAIN_MSG_LOST_CFG"}         //视频丢失配置   sdk_lost_cfg_t   							20

    ,{SDK_MAIN_MSG_OSD_CFG          , main_msg_osd_cfg       , NULL    ,"SDK_MAIN_MSG_OSD_CFG"}          //OSD配置   		sdk_osd_cfg_t   						21
    ,{SDK_MAIN_MSG_AUDIO_CONTROL	, main_msg_audio_control , NULL    ,"SDK_MAIN_MSG_AUDIO_CONTROL"}	 //音频控制  SDK_AUDIO_CONTROL_OP_E  						22
    ,{SDK_MAIN_MSG_SNAP_CONTROL		, main_msg_snap_control  , NULL    ,"SDK_MAIN_MSG_SNAP_CONTROL"}	 //抓拍图片                    							    23
    ,{SDK_MAIN_MSG_SYSTIME      	, main_msg_systime  	 , NULL    ,"SDK_MAIN_MSG_SYSTIME"}    	     //系统时间配置	sdk_time_t      						    24
    ,{SDK_MAIN_MSG_NET_LINK_CFG		, main_msg_net_link_cfg  , NULL    ,"SDK_MAIN_MSG_NET_LINK_CFG"}	 //PPPOE配置 SDK_NET_IDX_E    						        25

	,{SDK_MAIN_MSG_USER 			, main_msg_user 		 , NULL    ,"SDK_MAIN_MSG_USER"}			 //用户管理    SDK_USER_OP_E   						        26
    ,{SDK_MAIN_MSG_COMM_CFG         , main_msg_comm_cfg   	 , NULL    ,"SDK_MAIN_MSG_COMM_CFG"}         //常规配置   	sdk_comm_cfg_t  	        	            27
    ,{SDK_MAIN_MSG_OVERLAY_CFG      , main_msg_overlay_cfg   , NULL    ,"SDK_MAIN_MSG_OVERLAY_CFG"}      //遮挡区域配置   	sdk_overlay_cfg_t  	        	        28
    ,{SDK_MAIN_MSG_DISK      		, main_msg_disk  		 , NULL    ,"SDK_MAIN_MSG_DISK"}      		 //格式化硬盘		SDK_DISK_OP_E     	        	        29
    ,{SDK_MAIN_MSG_SERIAL_CFG       , main_msg_serial_cfg    , NULL    ,"SDK_MAIN_MSG_SERIAL_CFG"}    	 //串口参数配置	sdk_serial_func_cfg_t 	        	        30

	,{SDK_MAIN_MSG_IMAGE_ATTR_CFG	, main_msg_image_attr_cfg,NULL	   ,"SDK_MAIN_MSG_IMAGE_ATTR_CFG"}   //图像属性配置   sdk_image_attr_t; hongmb:	        	    31
    ,{SDK_MAIN_MSG_TV_CFG			, main_msg_tv_cfg   	 , NULL    ,"SDK_MAIN_MSG_TV_CFG"}    	  	 //TV配置   sdk_vo_sideSize_t               	        	32
    ,{SDK_MAIN_MSG_NOTIFY           , main_msg_notify		 , NULL    ,"SDK_MAIN_MSG_NOTIFY"}  		 // 通知事件发生/停止 args:SDK_EVENT_TYPE_E, chann:通道, extend:发生/停止  33
    ,{SDK_MAIN_MSG_DEVLIST          , main_msg_devlist       , NULL    ,"SDK_MAIN_MSG_DEVLIST"}          //查询设备列表 sdk_device_t     	        	            34
    ,{SDK_MAIN_MSG_MANUAL_REC		, main_msg_manual_rec	 , NULL    ,"SDK_MAIN_MSG_MANUAL_REC"}		 //手动开启(停止)录像 sdk_manual_record_t			        35

	,{SDK_MAIN_MSG_MANUAL_ALMIN		, main_msg_manual_almin	 , NULL    ,"SDK_MAIN_MSG_MANUAL_ALMIN"}	 //手动开启(停止)报警输入 sdk_manual_alarmin_t			    36
    ,{SDK_MAIN_MSG_MANUAL_ALMOUT	, main_msg_manual_almout , NULL    ,"SDK_MAIN_MSG_MANUAL_LAMOUT"}	 //手动开启(停止)报警输出 sdk_manual_alarmout_t			    37
    ,{SDK_MAIN_MSG_RECORD_BACKUP	, main_msg_record_backup , NULL    ,"SDK_MAIN_MSG_RECORD_BACKUP"}	 //录像备份  SDK_BACKUP_OP_E			        	        38
    ,{SDK_MAIN_MSG_PLATFORM_CFG     , main_msg_platform_cfg  , NULL    ,"SDK_MAIN_MSG_PLATFORM_CFG"}     //平台信息sdk_platform_t      	        	        	    39
    ,{SDK_MAIN_MSG_CHANGE_REMOTE_IP , main_msg_ch_ip         , NULL	   ,"SDK_MAIN_MSG_CHANGE_REMOTE_IP"} //改变远程设备IP sdk_remote_net_t 	        	            40

	,{SDK_MAIN_MSG_DDNS_CFG			, main_msg_ddns_cfg 	 , NULL    ,"SDK_MAIN_MSG_DDNS_CFG"}	     // DDNS配置 sdk_ddns_cfg_t    	            	            41
    ,{SDK_MAIN_MSG_CAPABILITY		, NULL                   , NULL    ,"SDK_MAIN_MSG_CAPABILITY"}  	 //设备能力集信息 sdk_device_capability_t 	        	    42
    ,{SDK_MAIN_MSG_UPNP_CFG         , main_msg_upnp_cfg		 , NULL	   ,"SDK_MAIN_MSG_UPNP_CFG"}		 //UPNP 配置      	        	        	        	    43
    ,{SDK_MAIN_MSG_SCREEN_CFG       , NULL                   , NULL    ,"SDK_MAIN_MSG_SCREEN_CFG"}       //	screen配置 sdk_screen_t					        	    44
    ,{SDK_MAIN_MSG_CRUISE_CTL       , NULL                   , NULL    ,"SDK_MAIN_MSG_CRUISE_CTL"}       //开启巡航，无对应结构体					        	    45

	,{SDK_MAIN_MSG_PRESET_SET       , NULL                   , NULL    ,"SDK_MAIN_MSG_PRESET_GET"}       //获取预置点集合，sdk_preset_param_t   	        	    46
    ,{SDK_MAIN_MSG_PREVIEW_TOUR     , main_msg_tour_cfg      , NULL    ,"SDK_MAIN_MSG_PREVIEW_TOUR"}     //预览巡回配置 sdk_tour_t               	        	    47
    ,{SDK_MAIN_MSG_3A_ATTR_CFG	    , main_msg_i3a_attr_cfg  , NULL	   ,"SDK_MAIN_MSG_3A_ATTR_CFG"}      //图像3A属性配置   sdk_3a_attr_t *p3a_cfg			        48
    ,{SDK_MAIN_MSG_UPGRAD	        , main_msg_upgrad        , NULL	   ,"SDK_MAIN_MSG_UPGRAD"}           //升级									        	        49
    ,{SDK_MAIN_MSG_SCHED_CFG	    , main_alarm_sched_cfg   , NULL	   ,"SDK_MAIN_MSG_SCHED_CFG"}        //录像排程   sdk_sched_time_t      	        	        50

	,{SDK_MAIN_MSG_REGIONAL_CFG	    , main_regional_cfg  	 , NULL	   ,"SDK_MAIN_MSG_POWER_HZ_CFG"}     //地域使用参数  regional_param      	        	        51
    ,{SDK_MAIN_MSG_IODEV_PARAM	    , main_iodev_cfg  		 , NULL	   ,"SDK_MAIN_MSG_IODEV_PARAM"}      //iodev设备参数	led  蜂鸣器等  sdk_iodev_param_t		52
    ,{SDK_MAIN_MSG_AUDIO_DETECT_CFG , main_audio_detect_cfg , NULL    ,"SDK_MAIN_MSG_AUDIO_DETECT_CFG"}	     //声音侦测配置   sdk_audio_detect_cfg_t           53
	//add by jensen temporarily
	,{SDK_MAIN_MSG_MIRROR_FLIP_CFG	,main_mirror_flip_cfg    , NULL    ,"SDK_MAIN_MSG_MIRROR_FLIP_CFG"}	     //镜像翻转实现  sdk_mirror_flip_cfg_t           54
	,{SDK_MAIN_MSG_POWER_FREQ_CFG   ,main_power_freq_cfg     , NULL    ,"SDK_MAIN_MSG_POWER_FREQ_CFG"}
	,{SDK_MAIN_MSG_STOP_SERVER		,NULL    				 , NULL    ,"SDK_MAIN_MSG_STOP_SERVER"}//0xS35
	,{SDK_MAIN_MSG_AWS				,main_msg_aws_cfg    	 , NULL    ,"SDK_MAIN_MSG_AWS"}//0xS35
	//end
    ,{SDK_MAIN_MSG_BUTT             , NULL                  ,"NULL"}
};


//是不是考虑加锁 防止多用户操作?
int _msg_dispatch(sdk_msg_t *pmsg)
{
    int ret = -1;
    int msg_id = pmsg->msg_id;
    ///int msg_src = pmsg->id.src;
    pmsg->ack = SDK_ERR_SUCCESS;//重置ack

    if(msg_id < 0 || msg_id >= SDK_MAIN_MSG_BUTT)
    {
        error("error: msg_id:0x%x\n", msg_id);
        goto __error;
    }

    info(" msg_id:0x%x, desc:%s, size=%u,  args=%u, ack=%d extend=%u, chann=%u\n"
         , msg_id
         , _msg_desc[msg_id].desc
         , pmsg->size
         , pmsg->args
         , pmsg->ack
         , pmsg->extend
         , pmsg->chann);
    if ( _msg_desc[msg_id].local_msg_cb )
    {

        info("+_+_+_+_+_+_ ID  id %d +_+_+_+_+_+_+_+_+ \n",msg_id);
        ret = _msg_desc[msg_id].local_msg_cb(pmsg);
    }


    if(ret < 0)
    {
        goto __error;
    }

    pmsg->msg_id = msg_id;
    pmsg->ack    = SDK_ERR_SUCCESS;
    pmsg->msg_dir = SDK_MSG_RESP;

    return 0;

__error:
    error(">>>>>>>>>>>>>> error.\n");
    pmsg->magic= MODULE_ID_MAIN;
    pmsg->msg_id = msg_id;
    pmsg->msg_dir = SDK_MSG_RESP;
    pmsg->ack = SDK_ERR_FAILURE; // 错误码不在此处设置

    return -1;
}

