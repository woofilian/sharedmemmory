#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "sdk_struct.h"

#include "streamlib.h"
#include "log.h"
#include "main.h"
#include "dana.h"

sdk_msg_dispatch_cb g_dana_msg_cb;

static  int audio_priority = 0;

static void * dana_open_ls_stream(dana_media_info* m, int32_t channel, int32_t level, void *media_info)
{
    info("  ======dana_open_ls_stream: channel=%d \n", channel);
    if(m == NULL || channel > 2)
        return -1;
    m->video_handle= (void *)sdk_stream_Open(1,channel);//暂时修改推第一路720p  chang  by zw
    
	m->video_current_chn = channel;
    info(">>>>>>>>>>>>>>> dana_open_ls_stream m->video_handle:%d  chnnel=%d\n",m->video_handle,channel);
    return 0;
}
static void * dana_audio_ls_stream(dana_media_info* m, int32_t channel, int32_t level, void *media_info)
{
    info("  ======dana_audio_ls_stream: channel=%d \n", channel);
    if(m == NULL || channel > 2)
        return -1;

    m->audio_handle= (void *)sdk_stream_Open(0,0);
    info(">>>>>>>>>>>>>>> dana_open_ls_stream m->audio_handle:%d  \n",m->audio_handle);
    return 0;
}


//释放资源
static void dana_close_ls_stream(dana_media_info*  m,int32_t channel)
{
	if(m==NULL){
	printf("=============dana_close_ls_stream\n");
	return -1;
	}
	sdk_stream_Close((ST_HDL)m->video_handle);
	m->video_handle = NULL;
	m->video_current_chn = -1;
    return 0;
}
static void dana_close_audio_ls_stream(dana_media_info*  m,int32_t channel)
{
	info("tutk  ====== dana_close_audio_ls_stream audio_handle %d \n",m->audio_handle);
	sdk_stream_Close((ST_HDL)m->audio_handle);
    return 0;
}

static int32_t dana_pull_ls_stream(dana_media_info*  m,int32_t channel, char **frame_buff)
{
    int ret;
    sdk_frame_t *frame_head = NULL;
    sdk_frame_t *frm = (sdk_frame_t *)frame_buff;
	*frame_buff = sdk_stream_ReadOneFrame((ST_HDL)(m->video_handle));
    if(*frame_buff == NULL)
    {
        return -1;
    }
   
	return 0;
}

static int32_t dana_pull_audio_ls_stream(dana_media_info*  m,int32_t channel, char **frame_buff)
{
    int ret;
    sdk_frame_t *frame_head = NULL;
    sdk_frame_t *frm = (sdk_frame_t *)frame_buff;
    *frame_buff = sdk_stream_ReadOneFrame((ST_HDL)(m->audio_handle));
    if(*frame_buff == NULL)
    {
        return -1;
    }
    return 0;
}

/*******************************/
//先这么定义 后期需要移植到中控程序里面实现

/*
语音对讲
*/
int audio_play_flag_dana = 0;

int dana_adapt_audio_play_stream(frame_t *frame_head,const char *audio_data)
{
    if(frame_head && audio_data)
        sdk_stream_WriteOneFrameEx(0,1,frame_head,audio_data);
    return 0;
}
static void *dana_playback_thread(void *arg)
{
    ST_HDL audio_play_handle =  sdk_stream_Open(0,1);
    while (audio_play_flag_dana)
    {
        sdk_frame_t *frame_head = NULL;
        frame_head = sdk_stream_ReadOneFrame(audio_play_handle);
        if(!frame_head)
        {
            usleep(20*1000);
            continue;
        }

        adapt_audio_play(0, frame_head->data, frame_head->frame_size);
    }

    sdk_stream_Close(audio_play_handle);
    return 0;
}

static int32_t  dana_audio_play_start(dana_media_info*  m,int32_t channel)
{
#if 0
    audio_play_flag_dana = 1;
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, dana_playback_thread, (void *)0))
    {
        perror("Create au_thread[sample_send_audio_bitstream] failed");
        exit(1);
    }
    pthread_detach(thread_id);
#endif
return 0;
}
static int32_t  dana_audio_play_stop(dana_media_info*  m,int32_t channel)
{
    audio_play_flag_dana = 0;
	return 0;

}

static int32_t dana_send_stream(dana_media_info*  m,int32_t channel,char *audio_buf,int32_t audio_len)
{
    info(">>>>>>>>>>>>>>> play_ls_stream start!!!!audio_len:%d  \n",audio_len);
	
	struct timeval pre_time = {0, 0},curr_time = {0, 0};
	 gettimeofday(&pre_time, NULL);
	 
	adapt_audio_play(0,audio_buf,audio_len);
	gettimeofday(&curr_time, NULL);
	info("dana_send_stream ======time value:%d  \n",(curr_time.tv_sec - pre_time.tv_sec) *1000 + (curr_time.tv_usec - pre_time.tv_usec)/1000);
   // dana_adapt_audio_play_stream(&frame_head,audio_buf);
    return 0;
}
//一个是消息句柄 一个是视频流的处理
int adapt_dana_init(sdk_msg_dispatch_cb msg_cb,void *stream_handle)
{
    int ret;
	
	
    //初始化
	if(msg_cb)
		g_dana_msg_cb = msg_cb;
#if 0
    //实时流接口
    dana_ls_avs_ops ls;
	ls.encode_current_chn = 0;

    ls.open  = dana_open_ls_stream;
    ls.pull  = dana_pull_ls_stream;
    ls.close = dana_close_ls_stream;

    ls.open_audio  = dana_audio_ls_stream;
    ls.close_audio = dana_close_audio_ls_stream;
    ls.pull_audio  = dana_pull_audio_ls_stream;

    //语音对讲使用
    ls.play_start = dana_audio_play_start;
    ls.send_audio = dana_send_stream;
    ls.play_stop  = dana_audio_play_stop;

    dana_server_reg_avs_ops(&ls);/* 注册流媒体接口 */
#endif
    dana_server_start();	/* 启动服务 */
    return 0;
}

