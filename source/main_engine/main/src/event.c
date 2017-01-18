
#include <sys/reboot.h>
#include <stdlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include<time.h>

//#include <basetypes.h>

#include "sdk_struct.h"
#include "streamlib.h"
#include "log.h"
#include "storage.h"
#include "adapt.h"

//#include "../../tutk/src/AVIOCTRLDEFs.h"

#define MAX_EVENT_NUM 20
/*
extern POOL_HDL sdk_pool_init(int data_len, int data_num);
extern void sdk_pool_uninit(POOL_HDL handle);
extern int sdk_event_wait(POOL_HDL handle);
extern int sdk_event_broadcast(POOL_HDL handle);
extern int sdk_event_send_to_pool(POOL_HDL handle,void *data);
extern EV_HDL sdk_get_event_handle(POOL_HDL handle);
extern int sdk_get_one_event(POOL_HDL handle,EV_HDL event,void *data);

*/

static  POOL_HDL  g_pool_handle = NULL;
//static  pthread_mutex_t g_enent_lock;
static int             g_event_process_runing = 0;
static pthread_t       g_event_prober_pid;
//pthread_t		pid_event_aws[10]={0};
//pthread_t		pid_event_cli[10]={0};
sdk_pri_event_t pir_event;


sdk_event_t event_info ;//时间结构体
extern RECORD_STATUS record_status;
extern int pir_event_handle_motion();
extern int event_handle_motion(sdk_event_t *event_obj);

#if 0

extern REMOTE_ALARM_PRA 	g_alarm_pra;			//告警全局配置
extern REMOTE_MOTION_PRA 	g_motion_pra;			//移动侦测全局配置


extern int					g_detect_alarm;			//移动侦测报警
extern int					g_audio_motion_state;   //声音侦测报警
extern int					g_send_alarm_flag;
extern int					g_alarm_out;			//报警输出
extern ALARM_STATE			g_alarm_state;
#endif 

int event_init()
{
	g_pool_handle =  sdk_pool_init(sizeof(sdk_event_t), MAX_EVENT_NUM);
	info("event pool handle:%d \n",g_pool_handle);
	return 0;
}
int event_uninit()
{
	sdk_pool_uninit(g_pool_handle);
	g_pool_handle = NULL;
	return 0;
}

//内部有锁
int event_send_to_pool(void *data)
{
	if(data==NULL)
    {
		printf("====================data==NULL\n");
		return 0;
    }
	sdk_event_t *evet=(sdk_event_t*)data;

	printf("+_+_+_+_+_+_+_+_+_  event_send_to_pool evet->enevt_type is %d \n",evet->enevt_type);
	//info("+_+_+_+_+_+_+_+_+_  event_send_to_pool evet->result is %d \n",evet->result);
	
//	info("+_+_+_+_+_+_+_+_+_  event_time = %x \n",evet->event_time);

	int ret = sdk_event_send_to_pool(g_pool_handle,data);
	printf("+_+_+_+_+_+_+_+_+_ sdk_event_send_to_pool ret = %d \n",ret);
	return 0;
}
extern void SendMsgToClient(sdk_event_t *event_obj);
extern void SendPushMessage(int eventType, unsigned int time);
extern void *SendMsgToClientThrd(void *param);


/*
*1 先检测出报警事件 
*2 判断是不是在 布防时间段
*3 做出相应判断
*4 侦测事件只有开始  没有结束状态
*/

//消费者  负责做相应的处理  写日志 录像  报警 io输出
#include "tutk.h"

//extern char *bucket ;
//extern pthread_mutex_t aws_lock;
int filesize(FILE *stream)
{
    int curpos, length;
    curpos = ftell(stream);
    fseek(stream, 0L, SEEK_END);    
    length = ftell(stream);
    fseek(stream, curpos, SEEK_SET);
    return length;
}

static void *event_process(void *param)
{
	info("********************************* alarm_process start. tid:%d\n", os_gettid());
    pthread_detach(pthread_self());  //分离线程
	//char buf[1280];
    int type = 0, ret = 0;
    sdk_event_t *event_obj = NULL;
	sdk_msg_t *msg_t = NULL;
	EV_HDL event_handle = sdk_get_event_handle(g_pool_handle);
	static  long last_time = 0; //保存时间的上一次的时间

	while(g_event_process_runing)
    {
		sdk_event_wait( g_pool_handle);
        while(1)
        {
        	event_obj=NULL;
        	//info("+_+_+_+_+_+_+_+_+_+_+_+_+_+event_process_+_+_+_+_+_+_+_+_+_+_+_+_\n");
			ret = sdk_get_one_event(g_pool_handle,event_handle,(void *)(&event_obj));
			if (ret < 0)
			{
	            info("^^^^^^^^^ alignstream_getFromPool < 0 ^^^^^^^^^\n");
                event_obj = NULL;
			}
			
            if(event_obj == NULL)
            {
            	info("event_obj == NULL \n");
                break;
            }
			printf("+_+_+_+_+_+_+_+_+_+_+__+ event_obj->event_type  is %d    event_obj->result %d    \n",event_obj->enevt_type,event_obj->result);
		    switch (event_obj->enevt_type)
	        {
		        case SDK_EVENT_AUDIOALERT ://声音侦测
		        {
		             printf(" SDK_EVENT_AUDIOALERT !!!! \n");//移动侦测
		            //处理时间逻辑  然后调用录像  抓图  网传报警等
	            	//event_handle_audio_alert(event_obj);
	            	info("SDK_EVENT_AUDIOALERT  is start  \n");
		            
					//当上一次的开始时间和本次开始时间有5s的时候 报警才是有效的 
		            break;
		        }

				case SDK_EVENT_IR ://433 一类的射频信号的告警 本质也是IO告警

		        {
		            printf(" SDK_EVENT_IOALERT !!!! \n");
		            break;
		        }
				
		        default:
		        {
		            printf("nothing!!!! \n");

		            break;
		        }
		    }
			usleep(40000);
        }
    }
	warning("alarm_process exit!\n");
    return NULL;
}
static void *event_pir_set(void *param)
{
	int gpio_pir_pwm=32+21;
	unsigned int index = 0;
	unsigned int count = 0;
	unsigned char percent = 0;
	unsigned char val_bak = 0;
    pthread_detach(pthread_self());  //分离线程
	while(1 == pir_event.pir_pthread_is_processing)	
	{
		if((pir_event.alarm_enable > 0)&& (pir_event.alarm_enable & 0x10000))
		{
			//printf("pir_event.alarm_enable & 0xffff=0x%x\n",pir_event.alarm_enable & 0xffff);
			pir_event.pir_setting = 1;
			if(1 == (pir_event.alarm_enable & 0xffff))
			{
				percent = 5;
			}
			else if(2 == (pir_event.alarm_enable & 0xffff))
			{
				percent = 35;
			}
			else if(3 == (pir_event.alarm_enable & 0xffff))
			{
				percent = 65;
			}
			else if(4 == (pir_event.alarm_enable & 0xffff))
			{
				percent = 80;
			}
			else if(5 == (pir_event.alarm_enable & 0xffff))
			{
				percent = 100;
			}
			else 
			{
				percent = 0;
			}
			count++; //计数50MS
			if(0 == index)
			{
				sdk_gpioEx_set(gpio_pir_pwm);
				if(count > 6) index = 1;
			}
			else if(1 == index)
			{
				sdk_gpioEx_clear(gpio_pir_pwm);
				if(count > (6 + percent/5)) index = 2;
			}
			else if(2 == index)
			{
				sdk_gpioEx_set(gpio_pir_pwm);
				if(count > (6 + percent/5 + 6)) index = 3;	
			}
			else if(3 == index)
			{
				sdk_gpioEx_clear(gpio_pir_pwm);
				count = 0;
				index = 0;
				//val_bak = val;
				pir_event.alarm_enable &= ~0x10000;
				pir_event.pir_setting = 0;
			}
			printf("percent=%d count=%d index=%d\n",percent,count,index);	
			//printf("percent=%d pir_event.alarm_enable=0x%x\n",percent,pir_event.alarm_enable);
		
		}
		if(0 == pir_event.pir_pthread_is_processing)
		{
			break;
		}
		usleep(50*1000);
		//sleep_time(0,50);
		
	}
	return NULL;
}


static void *event_pir_process(void *param)
{
	int val = -1;
	struct timeval pri_alarm_time;
	sdk_event_t pir_event_obj;
    pthread_detach(pthread_self());  //分离线程
	while(1 == pir_event.pir_pthread_is_processing)	
	{
	#if 0
		val = adapt_detect_pir_alarm();
		if(val == 1)
		{
			printf("pri alarm!\n");
		}
	#else
		if((pir_event.alarm_enable > 0) || (1 == pir_event.pir_setting ))
		{
			val = adapt_detect_pir_alarm();
			gettimeofday(&pri_alarm_time, NULL);
			if(1 == val)//PIR报警
			{
				pir_event.event_time = (unsigned long)pri_alarm_time.tv_sec;
				if((pir_event.event_time - pir_event.last_time <= record_status.md_interval_time ))
				{
					printf("interval time <%d!\n",record_status.md_interval_time);
					usleep(100*1000);
					continue;
				}
				if((pir_event.event_time - pir_event.last_time > record_status.md_interval_time ) 
					 							&&(0 == pir_event.md_status)) //PIR报警 且md 未报警
				{
					printf("pri alarm!!!!!!!!!\n");
					pir_event.last_time 	= pir_event.event_time;
					pir_event.event_type 	= SDK_EVENT_MOTION;//SDK_EVENT_PIR;
					pir_event.pir_status 	= 1;
					memset(&pir_event_obj,0,sizeof(pir_event_obj)); //构造一个EVENT_OBJ
					pir_event_obj.enevt_type = pir_event.event_type;
					pir_event_obj.event_time = pir_event.event_time;

			        pir_event_handle_motion();
					//SendMsgToClient(&pir_event_obj);
					//SendPushMessage(pir_event_obj.enevt_type - 1, pir_event_obj.event_time);
				}
				else if((pir_event.event_time - pir_event.last_time > record_status.md_interval_time ) 
					 							&&(1 == pir_event.md_status)) //如果PIR报警了且MD 已经报警 则只需推送消息
				{
					printf("pri alarm!!!!!! md   alarm!!!\n");
					pir_event.last_time 	= pir_event.event_time;
					pir_event.event_type 	= SDK_EVENT_MOTION;
					pir_event.pir_status 	= 1;
					memset(&pir_event_obj,0,sizeof(pir_event_obj)); //构造一个EVENT_OBJ
					pir_event_obj.enevt_type = pir_event.event_type;
					pir_event_obj.event_time = pir_event.event_time;

					//SendMsgToClient(&pir_event_obj);
					//SendPushMessage(pir_event_obj.enevt_type - 1, pir_event_obj.event_time);	
				}
				usleep(500*1000);
			}
			else 
			{
				if(pir_event.event_time - pir_event.last_time > record_status.md_interval_time )
				{
					pir_event.pir_status = 0;
				}
				usleep(30*1000); //感应到后输出一个50ms的高电平  所以此处延时不能太大
			}
		}
		else if(0 == pir_event.alarm_enable)
		{
			usleep(1000*1000);
		}
    #endif
		if(0 == pir_event.pir_pthread_is_processing)
		{
			break;
		}
	}
	return NULL;
}

int event_server_start()
{
	g_event_process_runing = 1;
	sdk_motion_cfg_t motion_cfg;
	memset(&pir_event,0,sizeof(pir_event));
	memset(&motion_cfg,0,sizeof(motion_cfg));
	pir_event.pir_pthread_is_processing = 1;
	adapt_param_get_motion_cfg(0,&motion_cfg);

	pir_event.alarm_enable |= motion_cfg.codec_vda_cfg.sensitive;
	pir_event.alarm_enable |= 0x10000;
	printf("sensitive=%x alarm_enable=0x%x\n",motion_cfg.codec_vda_cfg.sensitive,pir_event.alarm_enable);
    pthread_create(&g_event_prober_pid, NULL, event_process, NULL);
#if 0 //disable pir alarm
	pthread_create(&pir_event.pir_pthread_control_handle, NULL, event_pir_set, NULL);
	usleep(100*1000);
	pthread_create(&pir_event.pir_pthread_read_handle, NULL, event_pir_process, NULL);
#endif
	
}

/*********************************/
//比较精准的时间定时器
void sleep_time(int seconds, int mseconds)
{
    struct timeval temp;

    temp.tv_sec = seconds;
    temp.tv_usec = mseconds;
    select(0, NULL, NULL, NULL, &temp);
    ///info(">>>>>>>>>>>>> timer end \n");
    return ;
}
//系统时间监控
static void *SystemMonitorThread(void *param)
{
#if 0
	unsigned int count = 0;
	while (1)
	{
		UpdateSystemRunTimeInfo();
		if (count % 5 == 0)
		{
			monitor_work_mode();
			monitor_timer_capture();
			monitor_instant_capture();
			monitor_timer_record();
			monitor_video_lost();
			monitor_motion_detection();
			monitor_camera_overlay();
			monitor_io_input();
			monitor_reboot();
			monitor_timer_color2gray();
		}
		count++;
		monitor_record_switch();
		sleep_time(0, 200*1000);
	}
#endif

	return(NULL);
}

int start_system_monitor_thread()
{
	pthread_t monitor_tid;
	if (pthread_create(&monitor_tid, NULL, SystemMonitorThread, NULL) < 0)
	{
		perror("pthread_create:start_system_monitor_thread");
		return(-1);
	}
}
