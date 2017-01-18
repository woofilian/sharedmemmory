#ifndef __tutk_h__
#define __tutk_h__

#include "main.h"
#include "storage.h"
#ifdef __cplusplus
extern "C" {
#endif


#define RD_SUCCESS		0
#define RD_FAILED		  1
#define RD_INCOMPLETE	2
#define PACKET_SIZE 1024*1024

typedef  void* media_handle;

typedef struct
{
    char guid[21];
    char ViewAccount[32];
    char ViewPassword[32];
    int  tutk_enable;
    int  quality;
    char reserv[128]; //保留
} TUTK_INFO;

/*typedef enum _ARM_STATUS
{
		ARM_DISABLE =0,
		ARM_ENABLE
}ARM_STAUS;
typedef enum _LED_STATUS
{
		LED_DISABLE =0,
		LED_ENABLE_GREEN,
		LED_ENABLE_GREEN_FLASH,
		LED_ENABLE_BLUE,
		LED_ENABLE_BLUE_FLASH,
		LED_ENABLE_RED,
		LED_ENABLE_RED_FLASH
}LED_STAUS;*/
/*typedef enum _LED_STATUS  //定义LED的优先级
{
		LED_IS_ENABLE =0, 	// 布防下使能 不布防不使能  涉及到>= OR >  
		LED_NORMAL,       	//正常运行情况下	
		LED_ARM,     	 	//布防下
		LED_MONITOR,  		//观看视频下
		LED_ALARM_TRIGGER, 	//触发报警时
		LED_PANIC,			//手动
		LED_FORCE_CLOSE,	//	强制关闭 
		LED_NO_WIFI,		//没有网络的情况 AP
		LED_RESTART			//重启下
}LED_STATUS;
typedef enum _BUZZER_STATUS
{
		BUZZER_IS_ENABLE =0,//布防了 使能  不布防不使能 涉及到IF判断的>= OR >  
		BUZZER_ENABLE_NO_BEEP,//不响
		BUZZER_ENABLE_BEEP,  //蜂鸣器响
		BUZZER_TESTING		//手动强制
}BUZZER_STATUS;



typedef struct _led_buzzer_info_
{
	pthread_t 		led_buzzer_pthread_pid;//线程ID
	unsigned int	led_buzzer_pthread_is_processing;
	pthread_mutex_t 	led_buzzer_lock;
	ARM_STAUS  arm_status; //是否布防
	unsigned int led_status;//LED的状态
	unsigned char led_time; //led 时间
	BUZZER_STATUS buzzer_status; //蜂鸣器的状态 //测试状态 OR 正常状态
	unsigned char buzzer_time; //蜂鸣器响的时长
}LED_BUZZER_INFO;
*/
typedef struct _tutk_media_info
{
	int video_current_chn;
	void  * video_handle;
	void  * audio_handle;
}media_info;

/* 设备参数接口 */
typedef struct _tutk_device_ops
{
	int (*get_param)(int param_id, int channel, void *param, void *user); /* 获取设备参数 */
	int (*set_param)(int param_id, int channel, void *param, void *user); /* 设置设备参数 */
}tutk_device_ops;

/* 实时流接口 */
typedef struct _tutk_ls_avs_ops
{
	unsigned int encode_current_chn;//表示当前取那个通道的视频流
    int32_t (*open)(media_info *m, int32_t channel, int32_t level, void *media_info);/* 打开 */
    int32_t (*ctrl)(media_info *m, uint32_t cmd, void *data);           /* 控制 */
    void    (*close)(media_info *m,int32_t channel);                                    /* 关闭 */
    int32_t (*pull)(media_info *m, int32_t channel,char **frm);                          /* 拉流 */
	//tutk 对声音是单独线程处理的
	int32_t (*open_audio)(media_info *m, int32_t channel, int32_t level, void *media_info);/* 打开 */
    void    (*close_audio)(media_info *m,int32_t channel);                                    /* 关闭 */
    int32_t (*pull_audio)(media_info *m, int32_t channel,char **frm);                          /* 拉流 */

	//增加对讲功能
	int32_t (*play_start)(media_info*  m,int32_t channel);  /* 播放开始 */
	int32_t (*send_audio)(media_info *m, int32_t channel,char *audio_buf,int32_t audio_len);     /* 增加这个是为了语音对讲*/
    int32_t (*play_stop)(media_info*  m,int32_t channel);     /* 播放结束 */                                 /* 播放 */
}tutk_ls_avs_ops;

/* 历史流接口 */
typedef struct _tutk_hs_avs_ops
{
    int32_t (*open)(media_handle m, int32_t channel, int32_t level
                    , int32_t type, uint8_t *start_time, uint8_t *end_time
                    , uint8_t *property, void *mi);                 /* 打开 时间格式:YYYYMMDDHHMMSS */
    int32_t (*play)(media_handle m,int32_t channel);                                     /* 开始 */
    int32_t (*pause)(media_handle m,int32_t channel);                                    /* 暂停 */
    int32_t (*lseek)(media_handle m, uint32_t ts);                       /* 定位 */
    void    (*close)(media_handle m,int32_t channel);                                    /* 关闭 */
    int32_t (*pull)(media_handle m, int32_t channel,sdk_frame_t *frm);                       /* 拉流 */
}tutk_hs_avs_ops;

/* SDK 接口定义 */
int tutk_server_init(sdk_msg_dispatch_cb msg_cb);
int tutk_server_uninit();


int tutk_server_reg_device_ops(tutk_device_ops *ops);        /* 注册设备参数接口*/
int tutk_server_reg_avs_ops(tutk_ls_avs_ops *ls, tutk_hs_avs_ops *hs);/* 注册流媒体接口 */

int tutk_server_start();	/* 启动服务 */
int tutk_server_stop();
int tutk_server_restart();	/* 重新启动服务 */


int tutk_server_submit(int parm_id, int channel, void *parm);             /* 上报信息接口 */

int tutk_upload_file(const char *path,const char *file_name); //抓拍图片上传

#ifdef __cplusplus
}
#endif

#endif //__baidu_h__

