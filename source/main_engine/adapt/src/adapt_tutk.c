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
#include "tutk.h"

static  int audio_priority = 0;
static ST_HDL g_video_hdl  = NULL;
static ST_HDL g_audio_hdl  = NULL;
static void * open_ls_stream(media_info* m, int32_t channel, int32_t level, void *media_info)
{
    info("tutk  ======open_ls_stream \n");
    if(m == NULL || channel > 2)
        return -1;
    //g_video_hdl= sdk_stream_Open(1);
   m->video_handle= (void *)sdk_stream_Open(1,channel);//暂时修改推第一路720p  chang  by zw
   // m->video_handle= (void *)sdk_stream_Open(1,0);//暂时修改推第一路720p  chang  by zw
    
	m->video_current_chn = channel;
    info(">>>>>>>>>>>>>>> open_ls_stream m->video_handle:%d  chnnel=%d\n",m->video_handle,channel);
    return 0;
}
static void * open_audio_ls_stream(media_info* m, int32_t channel, int32_t level, void *media_info)
{
    info("tutk  ======open_audio_ls_stream \n");
    if(m == NULL || channel > 2)
        return -1;

    //g_video_hdl= sdk_stream_Open(1);
    m->audio_handle= (void *)sdk_stream_Open(0,0);
    info(">>>>>>>>>>>>>>> open_ls_stream m->audio_handle:%d  \n",m->audio_handle);
    return 0;
}


//释放资源
static void close_ls_stream(media_info*  m,int32_t channel)
{
	if(m==NULL){
	printf("=============close_ls_stream\n");
	return -1;
	}
    //ST_HDL stream_hdl = (ST_HDL)baidu_server_media_get_u(m);
   // sdk_stream_Close(g_video_hdl);
	sdk_stream_Close((ST_HDL)m->video_handle);
	m->video_handle = NULL;
	m->video_current_chn = -1;
	//sdk_stream_Close((ST_HDL)m->audio_handle);
    return 0;
}
static void close_audio_ls_stream(media_info*  m,int32_t channel)
{
   // sdk_stream_Close(g_video_hdl);
	//sdk_stream_Close((ST_HDL)m->video_handle);
	info("tutk  ====== close_audio_ls_stream audio_handle %d \n",m->audio_handle);
	sdk_stream_Close((ST_HDL)m->audio_handle);
    return 0;
}

static int32_t pull_ls_stream(media_info*  m,int32_t channel, char **frame_buff)
{
    int ret;
    sdk_frame_t *frame_head = NULL;
    sdk_frame_t *frm = (sdk_frame_t *)frame_buff;
    //ST_HDL stream_hdl = (ST_HDL)baidu_server_media_get_u(m);
	*frame_buff = sdk_stream_ReadOneFrame((ST_HDL)(m->video_handle));
	//*frame_buff = sdk_stream_ReadOneFrame(g_video_hdl);
	//info("sdk_stream_ReadOneFrame stream_hdl:%d  buffer_value:%d \n",m->video_handle,*frame_buff);
    if(*frame_buff == NULL)
    {
       // info("======= buffer == NULL \n");
        return -1;
    }
   
	//info("sdk_stream_ReadOneFrame success !!! \n");

	return 0;
}

static int32_t pull_audio_ls_stream(media_info*  m,int32_t channel, char **frame_buff)
{
    int ret;
    sdk_frame_t *frame_head = NULL;
    sdk_frame_t *frm = (sdk_frame_t *)frame_buff;
    *frame_buff = sdk_stream_ReadOneFrame((ST_HDL)(m->audio_handle));
    if(*frame_buff == NULL)
    {
        return -1;
    }
//    info("sdk_stream_ReadOneFrame success !!! \n");
    return 0;
}

/*******************************/
//先这么定义 后期需要移植到中控程序里面实现

/*
语音对讲
*/
int audio_play_flag = 0;

int adapt_audio_play_stream(frame_t *frame_head,const char *audio_data)
{
    if(frame_head && audio_data)
        sdk_stream_WriteOneFrameEx(0,1,frame_head,audio_data);
    return 0;
}
static void *playback_thread(void *arg)
{
    ST_HDL audio_play_handle =  sdk_stream_Open(0,1);
    while (audio_play_flag)
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

static int32_t  audio_play_start(media_info*  m,int32_t channel)
{
#if 0
    audio_play_flag = 1;
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, playback_thread, (void *)0))
    {
        perror("Create au_thread[sample_send_audio_bitstream] failed");
        exit(1);
    }
    pthread_detach(thread_id);
#endif
return 0;
}
static int32_t  audio_play_stop(media_info*  m,int32_t channel)
{
    audio_play_flag = 0;
	return 0;

}

static int32_t send_stream(media_info*  m,int32_t channel,char *audio_buf,int32_t audio_len)
{
    info(">>>>>>>>>>>>>>> play_ls_stream start!!!!audio_len:%d  \n",audio_len);
	
	struct timeval pre_time = {0, 0},curr_time = {0, 0};
	 gettimeofday(&pre_time, NULL);
	 
	#if 0
    frame_t frame_head;
    frame_head.frame_size = audio_len;
    frame_head.frame_type = SDK_AUDIO_FRAME_A;

    frame_head.audio_info.encode_type = SDK_AUDIO_FORMAT_G711A;
    frame_head.audio_info.samples = SDK_AUDIO_SAMPLE_R8K;
    frame_head.audio_info.bits = 16;
    frame_head.audio_info.channels = 1;
	#endif
	adapt_audio_play(0,audio_buf,audio_len);
	gettimeofday(&curr_time, NULL);
	info("send_stream ======time value:%d  \n",(curr_time.tv_sec - pre_time.tv_sec) *1000 + (curr_time.tv_usec - pre_time.tv_usec)/1000);
   // adapt_audio_play_stream(&frame_head,audio_buf);
    return 0;
}
static int32_t get_ls_stream(media_info*  m,int32_t channel, char *frame_buff)
{
#if 0
    int ret;
    sdk_frame_t *frame_head = NULL;
    sdk_frame_t *frm = (sdk_frame_t *)frame_buff;
	info("get audio frame_type:%d frame_size:%d \n",frm->frame_type,frm->frame_size);
	if(frm->frame_type == SDK_AUDIO_FRAME_A)
		return adapt_audio_play(0, frm->data, frm->frame_size);
#endif
    return -1;
}
//一个是消息句柄 一个是视频流的处理
int adapt_tutk_init(sdk_msg_dispatch_cb msg_cb,void *stream_handle)
{
    int ret;
	
	
    //初始化
    tutk_server_init(msg_cb);

    //实时流接口
    tutk_ls_avs_ops ls;
	ls.encode_current_chn = 0;

    ls.open  = open_ls_stream;
    ls.pull  = pull_ls_stream;
    ls.close = close_ls_stream;

    ls.open_audio  = open_audio_ls_stream;
    ls.close_audio = close_audio_ls_stream;
    ls.pull_audio  = pull_audio_ls_stream;

    //语音对讲使用
    ls.play_start = audio_play_start;
    ls.send_audio = send_stream;
    ls.play_stop  = audio_play_stop;

    tutk_server_reg_avs_ops(&ls,NULL);/* 注册流媒体接口 */
    tutk_server_start();	/* 启动服务 */
    return 0;
}


