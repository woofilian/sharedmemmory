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

#if 0
#include "sdk_struct.h"
#include "baidu.h"

#include "streamlib.h"
#include "log.h"





typedef struct __stream_index_t
{
	char frame_type;	                  /* i帧 p帧 音频 GPS */
	char consume_type; 	                  /* 预录 启动录像停止 */
	char nuse[2];		                  /* ch */
	int time;
	/* int tim_ms; */
	int frame_pos; 		                  /* 帧在buf中的位置 */
	int frame_end;		                  /* frame len  = frame head + size */ /* cp前面 */
	int contrl_type;                      /* alarm move */  
	/*int frame_no;*/
	int data_circle_no;  	              /* 当前帧是buf的第几环上的数据 */
	int index_circle_no;
	/* int set_nal_num; */	              /* 按最大分辨率分设置的nal数 unuse*/
	int nal_num;		                  /* 当前帧实际的nal 数 */	
	int nal_size[STREAM_MAX_NAL_NUM];/* nal数组空间 nal在帧中的位置动态分配 */
}stream_index_t;


typedef struct __stream_rpos_t  /* 流buf读操作控制 */
{

	int index_start_pos;        /* 取帧索引开始位置 */  
	int index_end_pos;          /* 取帧索引结束帧位置*/	
	int index_circle_no;        /* 环数开始帧的环数 不跨环 */
	int index_num;              /* 总的索引节点个数 */
	int index_block_pos;        /* */
	int index_count;            /*取块的时候用，本次取到count个帧*/
    stream_index_t *p_stream_index;/* 索引起始地址 */

	int data_start_pos;         /* 取帧数据开始位置 */ 
	int data_end_pos;           /* 取帧数据结束位置 */
	int data_circle_no;         /*  */

	char *p_buf_data;           /* 数据起始地址 */
    int change_count;
    unsigned int last_vframe_no;	// channel buffer中最新视频帧序号
	unsigned int last_aframe_no;	// channel buffer中最新音频帧序号    
}stream_rpos_t;


//st_stream_getLastPos(0, 0, &rpos);
static stream_rpos_t g_stream_handle[2];
static ST_HDL g_audio_handle;
static ST_HDL g_video_handle;


static  int audio_priority = 0;
static ST_HDL audio_stream_hdl;//音频只有一个通道   绑定通道  

static int32_t open_ls_stream(media_handle m, int32_t channel, int32_t level, void *media_info)
{
    if(m == NULL || channel > 2)
        return -1;

	g_video_handle = sdk_stream_Open(1);//取第一路720p的  控制到1M的码流
	
	g_audio_handle = sdk_stream_Open(0);//0通道就是音频  打开音频流
    //baidu_server_media_set_u(m, (void *)(stream_hdl));

	 // baidu_server_media_set_u(m, (void *)(audio_stream_hdl));

    info(">>>>>>>>>>>>>>> open_ls_stream g_video_handle:%d  audio_stream_hdl:%d \n",g_video_handle,g_audio_handle);

   // memset(&g_audio_handle,0,sizeof(stream_rpos_t));
   // st_stream_getLastPos(0, 0, &g_audio_handle);
   // baidu_server_media_set_u(m, (void *)(&g_audio_handle));
    return 0;
}

static int32_t play_ls_stream(media_handle m,int32_t channel)
{
    ST_HDL stream_hdl = (ST_HDL)baidu_server_media_get_u(m);
    info(">>>>>>>>>>>>>>> pull_ls_stream start!!!!stream_hdl:%d  \n",stream_hdl);
    return 0;
}

//释放资源
static void close_ls_stream(media_handle m,int32_t channel)
{
    ST_HDL stream_hdl = (ST_HDL)baidu_server_media_get_u(m);
	sdk_stream_Close(g_video_handle);

	sdk_stream_Close(g_audio_handle);
	
    return 0;
}


/*********/
static int32_t pull_ls_stream(media_handle m,int32_t channel, char **frame_buff)
{
    int ret;
    sdk_frame_t *frame_head = NULL;
	char *buf =NULL;
    ST_HDL stream_hdl = (ST_HDL)baidu_server_media_get_u(m);
	
	
    if((!audio_priority) &&(*frame_buff = sdk_stream_ReadOneFrame(g_video_handle))&&(*frame_buff))
    {
        return 0;
    }
    if((*frame_buff = sdk_stream_ReadOneFrame(g_audio_handle))&&(*frame_buff))
    {
		audio_priority  = 1; //说明有音频
        return 0;
    }
    audio_priority = 0;
    return -1;
}

static int32_t get_ls_stream(media_handle m,int32_t channel, char *frame_buff)
{
    int ret;
    sdk_frame_t *frame_head = NULL;
    sdk_frame_t *frm = (sdk_frame_t *)frame_buff;
	info("get audio frame_type:%d frame_size:%d \n",frm->frame_type,frm->frame_size);
	if(frm->frame_type == SDK_AUDIO_FRAME_A)
		return adapt_audio_play(0, frm->data, frm->frame_size);

	return -1;
}
//一个是消息句柄 一个是视频流的处理
int adapt_baidu_init(sdk_msg_dispatch_cb msg_cb,void *stream_handle)
{
    int ret;
    //初始化
    baidu_server_init(msg_cb);

    //实时流接口
    baidu_ls_avs_ops ls;
    ls.open  = open_ls_stream;
    ls.play  = play_ls_stream;
    ls.pull  = pull_ls_stream;
    ls.close = close_ls_stream;
	ls.get= get_ls_stream;

    baidu_server_reg_avs_ops(&ls, NULL);

    baidu_server_start();	/* 启动服务 */
//测试使用
	//init_write_fd();
    return 0;
}
int adapt_baidu_stop()
{
    baidu_server_stop();	/* 启动服务 */
    return 0;
}


// 1 链接上  0 未链接
int adapt_baidu_get_net_status()
{
    return baidu_get_connect_status();
}
int adapt_baidu_set_stream_ch(int channel)
{
    return baidu_set_stream_ch(channel);

}

int adapt_baidu_set_rec_status(int rec_status)
{
    return baidu_set_rec_status(rec_status);
}
int adapt_baidu_set_audio_status (int audio_eanble)
{

    return baidu_set_audio_status (audio_eanble);

}
//视频停止时候 不会有声音
int adapt_baidu_set_video_status (int video_eanble)
{
    return baidu_set_video_status ( video_eanble);

}
int adapt_baidu_upload_file (const char *path,const char * file_name,const char *target_path)
{
    return baidu_upload_file(path,file_name,target_path);
}


#else
#define adapt_baidu_init(arg...)      (0)
#define adapt_baidu_set_stream_ch(arg...)      (0)
#define adapt_baidu_set_rec_status(arg...)          (0)
#define adapt_baidu_set_audio_status(arg...)  (0)
#define adapt_baidu_set_video_status(arg...)   (0)
#define adapt_baidu_upload_file(arg...)      (0)


#endif


