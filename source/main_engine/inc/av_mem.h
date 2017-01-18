#ifndef __av_mem_h__
#define __av_mem_h__

#include "pthread.h"
#include "streamlib.h"




#define STREAM_TWO_STREAM	(2)

#define STREAM_ALIGN_ENABLE
#define STREAM_ALIGN_SIZE (4)

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

/* ----------------------------------------------------------- */

typedef struct _alignstream_t
{
    int data_len;
    int data_num;
    int wpos;
    int circle;
    char **data;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
}alignstream_t;

typedef struct  _alignstream_rpos_t
{
    int rpos;
    int circle;
}alignstream_rpos_t;


/* --------------------- 变长音视频帧内存管理 ---------------------- */


int av_stream_avRegister(const stream_manage_t *p_stream_manage);
int av_stream_init();
int avstream_destroy();


int av_stream_sendFrameToPool(int ch, int ch_type, frame_t *frame);
int av_stream_sendFrameToPoolEx(int ch, int ch_type, frame_head_t *frame_head,char *frame_addr);

int av_stream_writeFrameToPool(stream_writeFrame_t *writeFrame);

int av_stream_getLastPos(int ch, int ch_type, stream_rpos_t *rpos);       /* 最新帧位置 */
int av_stream_getStartPos(int ch, int ch_type, stream_rpos_t *rpos);       /* 预录等情况 */
int av_stream_getOneFrame(int ch, int ch_type, int real_frame_diff, stream_rpos_t *rpos);	/* 取一帧数据前做数据安全位置检查和调整 */
typedef struct stream_getOneIn_s
{
    int ch;
    int ch_type;
    int real_frame_diff;
    int change_iGetNum;     /* 实时调整到I帧后取继续帧数 */
}stream_getOneIn_t;
int av_stream_getOneFrame2(stream_getOneIn_t *getOneIn, stream_rpos_t *rpos);	/* 取一帧数据前做数据安全位置检查和调整 */

int av_stream_getBlock(int ch, int ch_type, int size, stream_rpos_t *rpos); /* 以帧边界对齐 尽量>=size 之前做安全位置调整后 做有效性检查 录像lseek 到有效帧位置 */
int av_stream_dataVailCheck(int ch, int ch_type, stream_rpos_t *rpos); /* 判断数据有没有发生覆盖 */

void av_stream_StreamBroadcast(int channel, int ch_type);
void av_stream_StreamWaiting(int channel, int ch_type);

typedef struct st_pic_parm_s {
    unsigned int  sync_flags;
    unsigned int  sps_size; /* sps size */
    unsigned int  pps_size; /* pps size */
    unsigned char sps[64];  /* without start code */
    unsigned char pps[64];  /* without start code */
}st_pic_parm_t;
int av_stream_getPicParm(int ch, int ch_type, st_pic_parm_t *parm);

/* --------------------- 定长数据内存管理 ---------------------- */

int alignstream_alloc(int data_len, int data_num, alignstream_t **alignstream);
void alignstream_free(alignstream_t *alignstream);

int alignstream_sendToPool(alignstream_t *alignstream, void *data);
int alignstream_getFromPool(alignstream_t *alignstream, void **data, alignstream_rpos_t *rpos);

int alignstream_wait(alignstream_t *alignstream);
int alignstream_broadcast(alignstream_t *alignstream);
//
int alignstream_getLastPos(alignstream_t *alignstream, alignstream_rpos_t *rpos);




#define GET_LAST_IFRAME     /* 要做相应的i 帧后安全检查有可能已经发生覆盖 */
#ifdef GET_LAST_IFRAME
int av_stream_getLastIPos(int ch, int ch_type, stream_rpos_t *rpos);
#endif

#endif/*__av_mem_h__*/

