/*
 * =====================================================================================
 *
 *       Filename:  event_2.h
 *
 *        Version:  1.0
 *        Created:  2012年07月06日
 *       Revision:  none
 *       Compiler:  gcc
 *	  Description:  本模块基于线程间事件控制，用于DDNS,PPPOE,SMTP等模块。
 *
 *         Author:  huangws
 *        Company:  jxj
 *
 * =====================================================================================
 */

#ifndef EVENT_H
#define EVENT_H

/*************************************************************
* 函数介绍：创建EVENT
* 输入参数：无
* 输出参数：无
* 返回值  ：<0-失败，>0-EVENT句柄
*************************************************************/
int event_open();

/*************************************************************
* 函数介绍：等待事件
* 输入参数：handle: 事件句柄
*           want: 欲接收的event
* 输出参数：events:接收到的事件消息
* 返回值  ：<0-失败，0-成功
*************************************************************/
int event_wait(int handle, unsigned int want, unsigned int* events);

/*************************************************************
* 函数介绍：发送事件
* 输入参数：handle: 事件句柄
* 输出参数：events:待发送事件
* 返回值  ：<0-失败，0-成功
*************************************************************/
int event_post(int handle, unsigned int events);

/*************************************************************
* 函数介绍：销毁事件，释放资源
* 输入参数：handle: 事件句柄
* 输出参数：无
* 返回值  ：<0-失败，0-成功
*************************************************************/
int event_close(int handle);

#endif


