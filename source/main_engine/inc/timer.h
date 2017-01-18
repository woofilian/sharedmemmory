


#ifndef __TIMER_H__
#define __TIMRE_H__

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>


// 以下宏定义为timer_ioctrl()的cmd类型
typedef enum {
  	TIMER_CMD_GETTMCOUNT = 0x00 		// 获取当前启动的计数器个数
  ,	TIMER_CMD_GETTMPASST 				// 获取当前定时器已经经过的时间(ms)
  ,	TIMER_CMD_GETTMREACH 				// 获取当前定时器离到达的时间(ms)
  , TIMER_CMD_RESET_STARTTIME  			// 重新设置当前定时器的开始时间(ms)
  ,	TIMER_CMD_BUIT
} TIMER_CMD;

typedef int (*timer_cb)(void *param);

/*************************************************************
* 函数介绍：初始化一个定时器(主控模块初始化一次即可)
* 输入参数：无
* 输出参数：无
* 返回值  ：>0-成功; <0-失败
*************************************************************/
int timer_init();


/*************************************************************
* 函数介绍：启动一个定时器
* 输入参数：once:     -1:循环定时，N(N>0):运行N次后停止
            interval: 定时器时间间隔，单位：毫秒
            func:     定时器回调函数
            param:回调函数参数
* 输出参数：无
* 返回值  ：>0-成功，表示定时器句柄；<=0-错误代码
*************************************************************/
int timer_start(int run_time, int interval, char *cb_func_name, timer_cb func, void *param);


/*************************************************************
* 函数介绍：TIMER配置
* 输入参数：handle: TIMER句柄；
            cmd: 命令；
            channel: 通道号，此处无效
            param：输入参数；
            size_of_param: param长度，特别对于GET命令时，输出参数应先判断缓冲区是否足够
* 输出参数：param：输出参数；
* 返回值  ：>=0-成功，<0-错误代码
*************************************************************/
int timer_ioctrl(int handle, int cmd, int channel, void* param, int size_of_param);


/*************************************************************
* 函数介绍：停止一个定时器
* 输入参数：handle: 定时器句柄
* 输出参数：无
* 返回值  ：>=0-成功，<0-失败
*************************************************************/
int timer_stop(int handle);

#endif // __TIMER_H__

