
#ifndef __baidu_h__
#define __baidu_h__

#include "main.h"
#ifdef __cplusplus
extern "C" {
#endif


#define RD_SUCCESS		0
#define RD_FAILED		  1
#define RD_INCOMPLETE	2
#define PACKET_SIZE 1024*1024

typedef  void* media_handle;

/* 设备参数接口 */
typedef struct _baidu_device_ops
{
	int (*get_param)(int param_id, int channel, void *param, void *user); /* 获取设备参数 */
	int (*set_param)(int param_id, int channel, void *param, void *user); /* 设置设备参数 */
}baidu_device_ops;

/* 实时流接口 */
typedef struct _baidu_ls_avs_ops
{
    int32_t (*open)(media_handle m, int32_t channel, int32_t level, void *media_info);/* 打开 */
    int32_t (*play)(media_handle m,int32_t channel);                                     /* 播放 */
    int32_t (*ctrl)(media_handle m, uint32_t cmd, void *data);           /* 控制 */
    void    (*close)(media_handle m,int32_t channel);                                    /* 关闭 */
    int32_t (*pull)(media_handle m, int32_t channel,char **frm);                          /* 拉流 */
    int32_t (*get)(media_handle m, int32_t channel,char *frm);     /* 增加这个是为了语音对讲*/
}baidu_ls_avs_ops;

/* 历史流接口 */
typedef struct _baidu_hs_avs_ops
{
    int32_t (*open)(media_handle m, int32_t channel, int32_t level
                    , int32_t type, uint8_t *start_time, uint8_t *end_time
                    , uint8_t *property, void *mi);                 /* 打开 时间格式:YYYYMMDDHHMMSS */
    int32_t (*play)(media_handle m,int32_t channel);                                     /* 开始 */
    int32_t (*pause)(media_handle m,int32_t channel);                                    /* 暂停 */
    int32_t (*lseek)(media_handle m, uint32_t ts);                       /* 定位 */
    void    (*close)(media_handle m,int32_t channel);                                    /* 关闭 */
    int32_t (*pull)(media_handle m, int32_t channel,sdk_frame_t *frm);                       /* 拉流 */
}baidu_hs_avs_ops;

/* SDK 接口定义 */
int baidu_server_init(sdk_msg_dispatch_cb msg_cb);
int baidu_server_uninit();


int baidu_server_reg_device_ops(baidu_device_ops *ops);        /* 注册设备参数接口*/
int baidu_server_reg_avs_ops(baidu_ls_avs_ops *ls, baidu_hs_avs_ops *hs);/* 注册流媒体接口 */

int baidu_server_start();	/* 启动服务 */
int baidu_server_stop();

int baidu_server_submit(int parm_id, int channel, void *parm);             /* 上报信息接口 */

int baidu_set_property(); //设置百度的属性
int baidu_upload_file(const char *path,const char *file_name,const char *target_path); //抓拍图片上传

#ifdef __cplusplus
}
#endif

#endif //__baidu_h__

