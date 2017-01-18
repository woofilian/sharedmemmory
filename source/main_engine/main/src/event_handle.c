/******************************************************************************

                  版权所有 (C), 2012-2022, bingchuan

 ******************************************************************************
  文 件 名   : event_handle.c
  版 本 号   : v1.0
  作    者   : bingchuan
  生成日期   : 2015年6月1日
  功能描述   : 负责判断布防时间，以及事件触发后做出相应动作
  函数列表   :
  修改历史   :
  1.日    期   : 2015年6月1日
    作    者   : bingchuan
    修改内容   : 创建文件

******************************************************************************/
#include "sdk_struct.h"

#include "log.h"

#define 	DETECT_INTERVAL_TIME    5   //布防最小时间间隔为5秒

extern RECORD_STATUS record_status;


//下面的函数是做布防时间段判断的


//在962 机型中 报警输入也有很多个种类  比如烟感  火警等信息  需要后期判断
//目前统一作为输入报警使用


//判断是不是在布防时间段内
static int _sched_calc(time_t now
                       , int year, int month, int day
                       , sdk_day_time_t *_sched)
{
    int detect_flag;
    time_t start, end;
    detect_flag = 0;


    if(_sched->start_hour == _sched->stop_hour
            && _sched->start_min == _sched->stop_min)
    {
        return detect_flag;
    }
    start = os_make_time(year, month, day
                         ,_sched->start_hour
                         ,_sched->start_min
                         ,0);

    end  = os_make_time(year, month, day
                        ,_sched->stop_hour
                        ,_sched->stop_min
                        ,59);
    if(start < end)
    {
        if(now >= start && now <= end)
        {
            detect_flag=1;
        }
    }

    return detect_flag;
}

/*****************************************************************************
 函 数 名  : event_sched_motion
 功能描述  : 检测是否在布防时间段内
 输入参数  : 无
 输出参数  : 无
 返 回 值  : 0:表示在检测范围内  -1:表示不能布防
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2015年6月2日
    作    者   : bingchuan
    修改内容   : 新生成函数

*****************************************************************************/
static  int event_sched_check(sdk_sched_time_t *psched_time, time_t *prev_occur_time)
{
    uint32_t 		in_sched_flags = 0;
    uint32_t        def_signal_value = 0;                  //默认信号值
    uint32_t        interval_time   = DETECT_INTERVAL_TIME;//控制报警最小停止时间
    int time_seg_num = 0;
    unsigned int weekday, year, month, day;

    sdk_sched_time_t sched_time = *psched_time;

    if(!sched_time.enable)
        return -1;
	info("000000000000000000\n");
    interval_time  = (sched_time.interval_time >=  DETECT_INTERVAL_TIME)? sched_time.interval_time : DETECT_INTERVAL_TIME;    //控制报警最小停止时间

    time_t now_time = time(NULL);

    //时间间隔没达标
    if(now_time - *prev_occur_time  <  interval_time)
        return -1;
	
	info("111111111111111111\n");
    //根据布防方式判断布防时间段  目前默认是按照周布防
    //if(sched_time.plan_flag == 1)

    os_local_time(now_time, &year, &month, &day, NULL, NULL, NULL, &weekday);
    info("======= %04d-%02d-%02d-%02d \n",year, month, day, weekday);

    for (time_seg_num=0; time_seg_num <4; time_seg_num++)
    {
        if (sched_time.week_sched.week_sched_info[weekday][time_seg_num].enable)
        {
            in_sched_flags = _sched_calc(now_time, year, month, day, &sched_time.week_sched.week_sched_info[weekday][time_seg_num]);
            if (in_sched_flags)
                break;
        }
    }
	info("22222222222222222\n");
    if(1 == in_sched_flags)
    {
        *prev_occur_time = now_time;
		
		info("333333333333333333333333\n");
        return 0;
    }
    else
    {
		info("4444444444444444444444444\n");
        return -1;
    }

}
#if 0
//#include "tutk.h"
//////////////////////////0708
typedef struct tag_BUZZER_PRA
{
	int  buzzer_flag; // buzzer_status ----1:sound 0:off
    int	 buzzer_time;  //  0 到 120 秒
    int	 buzzer_event;  //  0 到 120 秒

} REMOTE_BUZZER_PRA;


///////////////////////
#endif
#include "tutk.h"
extern LED_BUZZER_INFO  led_buzzer_info; 

int pir_event_handle_motion()
{
    sdk_motion_cfg_t motion_cfg;
    sdk_alarm_handle_t alarm_handle;                            //联动处理
    memset(&motion_cfg,0,sizeof(sdk_motion_cfg_t));
    memset(&alarm_handle,0,sizeof(sdk_alarm_handle_t));
    adapt_param_get_motion_cfg(0, &motion_cfg);

	printf("%d\n",motion_cfg.alarm_handle.record_enable);
	printf("beep_enable=%d beep_time=%d\n",motion_cfg.alarm_handle.beep_enable,motion_cfg.alarm_handle.beep_time);

    //联动处理  可能有延时
    alarm_handle = motion_cfg.alarm_handle;
    //录像
    if( alarm_handle.record_enable)
    {
		record_status.record_alarm_triger |= 1; //设置报警录像

    }

    //报警输出
    alarm_handle.alarm_out_enable = 1;

    //蜂鸣器打开
    alarm_handle.beep_enable = 1;
	//处理LED BUZZER
	 pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock); 
	led_buzzer_info.led_status 		|= (1 << LED_ALARM_TRIGGER);
	led_buzzer_info.buzzer_status 	|= (1 <<BUZZER_ENABLE_BEEP);
	 pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock); 
	
	//抓拍图片
	//发送邮件
    // 联动PTZ

}

//移动侦测
int event_handle_motion(sdk_event_t *event_obj)
{
    sdk_motion_cfg_t motion_cfg;
    static time_t prev_occur_time = 0; //记录上一次发生事件的时间
    sdk_alarm_handle_t alarm_handle;                            //联动处理

    memset(&motion_cfg,0,sizeof(sdk_motion_cfg_t));
    memset(&alarm_handle,0,sizeof(sdk_alarm_handle_t));

    adapt_param_get_motion_cfg(0, &motion_cfg);
	//////////////////////////////////////////
	#if 0
	printf("%d\n",motion_cfg.alarm_handle.email_enable);
	printf("%d\n",motion_cfg.alarm_handle.record_enable);
	printf("%d\n",motion_cfg.alarm_handle.snap_enable);
	printf("%d\n",motion_cfg.alarm_handle.alarm_out_time);
	printf("%d\n",motion_cfg.alarm_handle.beep_enable);
	printf("%d\n",motion_cfg.alarm_handle.beep_time);
	printf("%d\n",motion_cfg.sched_time.enable);
	printf("%d\n",motion_cfg.sched_time.plan_flag);
	printf("%d\n",motion_cfg.sched_time.interval_time);
	printf("%d\n",motion_cfg.codec_vda_cfg.area[0].x);
	printf("%d\n",motion_cfg.codec_vda_cfg.area[0].y);
	printf("%d\n",motion_cfg.codec_vda_cfg.area[0].width);
	printf("%d\n",motion_cfg.codec_vda_cfg.area[0].height);
	printf("%d\n",motion_cfg.codec_vda_cfg.enable);
	printf("%d\n",motion_cfg.codec_vda_cfg.mask);
	printf("%d\n",motion_cfg.codec_vda_cfg.mode);
	printf("%d\n",motion_cfg.codec_vda_cfg.res);
	printf("%d\n",motion_cfg.codec_vda_cfg.sensitive);
	#endif
	//////////////////////////////////////////

    if(0 == motion_cfg.codec_vda_cfg.enable || 0 >  event_sched_check(&motion_cfg.sched_time,&prev_occur_time))
    {
		info("motion_cfg.codec_vda_cfg.enable=%d\n",motion_cfg.codec_vda_cfg.enable);
        info("motion comming ,but not pass!!! \n");
        return -1;
    }
    //联动处理  可能有延时
    alarm_handle = motion_cfg.alarm_handle;
    //录像
    #if 1
    if( alarm_handle.record_enable)
    {
    	//if(record_status.record_last_status	== RECORD_ALARM)
    	if(record_status.record_flags == RECORD_ALARM)
    	{
    		pthread_mutex_lock(&record_status.record_lock);
			record_status.record_alarm_triger |= 1; //设置报警录像
			pthread_mutex_unlock(&record_status.record_lock);
    	}

    }
	#endif

#if 1
    //报警输出
    alarm_handle.alarm_out_enable = 1;
	
	printf("========alarm_handle.alarm_out_enable11111\n");
    if(alarm_handle.alarm_out_enable)
    {
    }
    //蜂鸣器打开
    alarm_handle.beep_enable = 1;

	//处理LED BUZZER
	 pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock); 
	led_buzzer_info.led_status 		|= (1 << LED_ALARM_TRIGGER);
	led_buzzer_info.buzzer_status 	|= (1 << BUZZER_ENABLE_BEEP);
	 pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock); 
	
	//抓拍图片
	if(alarm_handle.snap_enable)
	{
		#if 0
		sdk_snap_info_t snap_info;
		snap_info.width = 1280;
		snap_info.height = 720;
		sdk_av_snap(0,&snap_info,"/mnt/nfs/motion_event.bmp");
		#endif
	}
	
	//发送邮件
	if(alarm_handle.email_enable)
	{

	}
    // 联动PTZ
    #if 0
    if (alarm_handle.ptz_enable)
    {

    }
	#endif
#endif
}

/**************************************************/

static event_sched_alarm_in()
{


}

/****************************/
//事件处理

//告警输入
int event_handle_alarm_in(sdk_event_t *event_obj)
{


}

//声音侦测侦测AUDIOALERT
int event_handle_audio_alert(sdk_event_t *event_obj)
{


}



