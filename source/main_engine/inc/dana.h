#ifndef __DANA_H__
#define __DANA_H__

#include "main.h"
#include "libdanavideo/danavideo.h"
#include "storage.h"

#define DANAVIDEO_CMD_GETWIFIAP_MAX_COUNT  20

sdk_msg_dispatch_cb g_dana_msg_cb;

#define DANA_MSG_CTRL(_id, _arg, _ext, _ch, pmsg) ({\
				int _ret = -1;\
				pmsg->msg_id	= _id;\
				pmsg->msg_dir 	= SDK_MSG_REQ;\
				pmsg->chann 	= _ch;\
				pmsg->args		= _arg;\
				pmsg->extend	= _ext;\
				if(g_dana_msg_cb)\
				_ret = g_dana_msg_cb(pmsg);\
				})
 
typedef struct _dana_media_info
{
	int video_current_chn;
	void  * video_handle;
	void  * audio_handle;
}dana_media_info;

typedef struct _avi_list_s
{
	NodeIndex       uNodeInfo;
	uint32_t        dwFileSize;
}dana_avi_list_t;

typedef struct danaAviPBHandle {
	float avi_fps;

    FILE    *file;
    //index
    int    index_count;   //real index count总帧数
    //int    frame_type; //帧类型
	int    pb_file_end;
    int    video_count;
    int    audio_count;
	int    index_current;  //回放的当前索引
    int    *idx_array; //回放索引数组
    int    idx_array_count;

    //buffer for playback
    char *pb_buf;
    int  pb_buf_size;
} DANAAviPBHandle;


typedef struct _MyData {
    pdana_video_conn_t *danavideoconn;

    volatile bool run_media;
    volatile bool exit_media;
    pthread_t thread_media;

    volatile bool run_talkback;
    volatile bool exit_talback;
    pthread_t thread_talkback;

    volatile bool run_audio_media;
    volatile bool exit_audio_media;
    pthread_t thread_audio_media;

	volatile bool run_history_media;
    volatile bool exit_history_media;
    pthread_t thread_history_media;


    uint32_t chan_no;

    char appname[32]; 

} MYDATA;
typedef struct
{
	unsigned short year;	// The number of year.
	unsigned char month;	// The number of months since January, in the range 1 to 12.
	unsigned char day;		// The day of the month, in the range 1 to 31.
	unsigned char wday;		// The number of days since Sunday, in the range 0 to 6. (Sunday = 0, Monday = 1, ...)
	unsigned char hour;     // The number of hours past midnight, in the range 0 to 23.
	unsigned char minute;   // The number of minutes after the hour, in the range 0 to 59.
	unsigned char second;   // The number of seconds after the minute, in the range 0 to 59.
}TimeDay;

/*
IOTYPE_USER_IPCAM_LISTEVENT_REQ			= 0x0318,
** @struct SMsgAVIoctrlListEventReq
*/
typedef struct
{
	unsigned int channel; 		// Camera Index
	TimeDay stStartTime; 		// Search event from ...
	TimeDay stEndTime;	  		// ... to (search event)
	unsigned char event;  		// event type, refer to ENUM_EVENTTYPE
	unsigned char status; 		// 0x00: Recording file exists, Event unreaded
								// 0x01: Recording file exists, Event readed
								// 0x02: No Recording file in the event
	unsigned char reserved[2];
}MsgAVIoctrlListEventReq;

/* 实时流接口 */
typedef struct _dana_ls_avs_ops
{
	unsigned int encode_current_chn;//表示当前取那个通道的视频流
    int32_t (*open)(dana_media_info *m, int32_t channel, int32_t level, void *media_info);/* 打开 */
    int32_t (*ctrl)(dana_media_info *m, uint32_t cmd, void *data);           /* 控制 */
    void    (*close)(dana_media_info *m,int32_t channel);                                    /* 关闭 */
    int32_t (*pull)(dana_media_info *m, int32_t channel,char **frm);                          /* 拉流 */
	//tutk 对声音是单独线程处理的
	int32_t (*open_audio)(dana_media_info *m, int32_t channel, int32_t level, void *media_info);/* 打开 */
    void    (*close_audio)(dana_media_info *m,int32_t channel);                                    /* 关闭 */
    int32_t (*pull_audio)(dana_media_info *m, int32_t channel,char **frm);                          /* 拉流 */

	//增加对讲功能
	int32_t (*play_start)(dana_media_info*  m,int32_t channel);  /* 播放开始 */
	int32_t (*send_audio)(dana_media_info *m, int32_t channel,char *audio_buf,int32_t audio_len);     /* 增加这个是为了语音对讲*/
    int32_t (*play_stop)(dana_media_info*  m,int32_t channel);     /* 播放结束 */                                 /* 播放 */
}dana_ls_avs_ops;

void dana_server_start();
void dana_server_stop();
int dana_server_init(sdk_msg_dispatch_cb msg_cb);
int dana_server_uninit();


//int tutk_server_reg_device_ops(tutk_device_ops *ops);        /* 注册设备参数接口*/
int dana_server_reg_avs_ops(dana_ls_avs_ops *ls);/* 注册流媒体接口 */




#endif
