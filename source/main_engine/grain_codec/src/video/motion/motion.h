#ifndef __MOTION_H__
#define __MOTION_H__



//保存移动侦测信息
typedef struct _motion_handle_s
{
	int motion_init ; //初始化标志
	int motion_enable; //侦测使能
	int motion_fd; //设置参数的句柄
	
	int max_width; //移动侦测的宽
	int max_height;//移动侦测的高
	
	pthread_t  motion_thread_id;

}motion_handle_t;



#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */





#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif
