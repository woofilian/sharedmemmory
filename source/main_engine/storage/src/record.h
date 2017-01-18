

#ifndef __RECORD_H__
#define __RECORD_H__

#include	<semaphore.h>
#include	<pthread.h>

typedef struct _record_work_t_
{
	pthread_mutex_t record_normal_mutex;  //常规录像锁
	pthread_mutex_t record_event_mutex;	  //事件录像锁
	pthread_t	g_record_thread_id;		  //常规录像线程
	pthread_t   g_event_thread_id;

	char g_normal_status;   		      // 1: 表示常规录像，0: 停止常规录像
 	char g_event_status ;  		          // 1 : 表示时间录像， 0 : 停止事件录像
	int g_event_duration;			      // 记录事件录像时间
	int g_normal_duration;		          // 记录常规录像时间 
	unsigned char  record_time_zone;				  //ntp时间区域
	unsigned char  record_time_zone_pre;				  //记录之前的时域

	SDK_RECORD_TYPE_E g_current_record_type;//录像类型
}record_work_t;

extern record_work_t g_record_work_t;



typedef struct _record_manage_t
{
    
	int	channel;		
	
	int	record_type; // 录像类型
	int	record_switch_time; //单位是秒	 录像时长
	int	record_ctl_flag; //录像控制标志位 start stop  write create 
	void *record_handle;
            
    //int (*send_msgToMainCB)(struct st_msg_s* msg);
   // int (*getBlockStar)();
}record_manage_t;


int st_record_initRecord();
int st_record_deinitRecord();

int st_record_startRecordThread();
int st_record_stopRecordThread();

int st_record_startRecordStream();
int st_record_stopRecordStream();

#endif	//__RECORD_H__

