
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <semaphore.h>
#include <pthread.h>
#include <sched.h>
#include <assert.h>
#include <sys/time.h>
#include <time.h>
#include <utime.h>

#include "sdk_global.h"
#include "sdk_struct.h"
#include "streamlib.h"
#include "avi_typedef.h"
#include "record.h"
#include "log.h"
#include "storage.h"
#include "mmc_api.h"
#include "../../libshmav/shm_common.h"
#include "../../libshmav/shm_write.h"
#include "../../libshmav/shm_read.h"

record_work_t g_record_work_t;


#define TIMEVAL_DIFF_SEC(start, end) (((long)(end)-(long)(start))*1000000)
#define TIMEVAL_DIFF_MS(start, end) (((long)(end)-(long)(start)))

record_manage_t g_manage_record =
{
    .record_ctl_flag = 0,
};
//ST_HDL video_handle =  NULL;
//ST_HDL audio_handle =  NULL;


/*------------下面的函数主要实现手动 和定时录像的功能------------------------*/
typedef  enum RECORD_STATE_E
{
    RECORD_STATE_START  = 0,
    RECORD_STATE_STOP,
    RECORD_STATE_WRITE,
    RECORD_STATE_CREATE_FILE,
} RECORD_STATE_E;



typedef  enum _VIDEO_RATE_E
{
    VIDEO_RATE_1_0 = 1,
    VIDEO_RATE_1_25,
    VIDEO_RATE_1_50,
    VIDEO_RATE_2_0,
    VIDEO_RATE_2_25,
    VIDEO_RATE_2_50,
} VIDEO_RATE_E;

/******************************/
//比较精准的时间定时器
static void sleep_time(int seconds, int mseconds)
{
    struct timeval temp;

    temp.tv_sec = seconds;
    temp.tv_usec = mseconds;
    select(0, NULL, NULL, NULL, &temp);
    ///info(">>>>>>>>>>>>> timer end \n");
    return ;
}

/**********常规录像取流*****************/
#if 1
static ST_HDL g_normal_video_handle =  NULL;
static ST_HDL g_normal_audio_handle =  NULL;

static  int normal_audio_priority = 0;
static int normal_sync_flag = 0; //同步标志位
static int normal_start_sec = 0; // 开始取视屏的时间，用来作为一个标准的开始时间
static int normal_video_offset = 0; //最新视频相对于g_start_ms的偏移
static int normal_audio_offset = 0; //最新音频相对于g_start_ms的偏移

static int normal_audio_count = 0; //记录音频连续取了10个之后 取视频
static int audio_count = 1;

static int open_normal_stream()
{	
    g_normal_video_handle =  sdk_stream_Open(1, 0);
    g_normal_audio_handle =  sdk_stream_Open(0, 0);
	normal_audio_priority = 0;
	normal_sync_flag = 0;

	normal_start_sec = 0;
	info(">>>>>>>>>>>>>>> open_normal_stream  normal_start_sec:%d <<<<<<<<<<<<<<<<  \n",normal_start_sec);

    return 0;
}
int need_av_sync()
{
	normal_sync_flag = 0;
	return 0;

}

//音视频同步
inline int video_audio_sync(ST_HDL video_handle,ST_HDL audio_handle)
{
	char *audio_buff = NULL;
	char *video_buff = NULL;

	sdk_frame_t *audio_head = NULL;
    sdk_frame_t *video_head = NULL;

	//一定要找到AV    header一定不能为空
	while(!(audio_head =(sdk_frame_t *) sdk_stream_ReadOneFrame(audio_handle)));	
	while(!(video_head =(sdk_frame_t *) sdk_stream_ReadOneFrame(video_handle)));
//如果音频比较新，则更新视频
	  if(video_head->pts  <  audio_head->pts) //如果音频跑在前面就更新视频
	  {
	  
		  while(1)
		  {
			if(NULL == (video_buff = sdk_stream_ReadOneFrame(video_handle)))
		     continue;
			video_head = (sdk_frame_t *)(video_buff);
			if(video_head->pts  >=  (audio_head->pts - 100))
		  	break;
		  }
		  
		  
		  info("av sync %lld =======%lld \n",video_head->pts,audio_head->pts );
	  }
	  else if(audio_head->pts < video_head->pts   ) //如果视频跑在前面就更新音频
	  {

	  	  while(1)
		  {
			if(NULL == (audio_buff = sdk_stream_ReadOneFrame(audio_handle)))
		     continue;
			audio_head = (sdk_frame_t *)(audio_buff);
			if(audio_head->pts  >=  (video_head->pts + 100))
				break;
			
		  }
		
		info("av sync %lld ------ %lld \n",video_head->pts,audio_head->pts );
	  }
	  
	  normal_video_offset = video_head->pts - normal_start_sec;
	  normal_audio_offset = audio_head->pts - normal_start_sec;
	  audio_count = 1;
	  info(" sync==normal_video_offset:%d  normal_audio_offset:%d \n",normal_video_offset,normal_audio_offset);
	  	return 0;
}
/*
    VIDEO_RATE_1_0 = 1,
    VIDEO_RATE_1_25,
    VIDEO_RATE_1_50,
    VIDEO_RATE_2_0,
    VIDEO_RATE_2_25,
    VIDEO_RATE_2_50,
*/
static int32_t pull_normal_stream(char **frame_buff,VIDEO_RATE_E rate)
{
    int ret;
	int audio_rate = rate;
	int rate_cout  = 0;
	char *audio_buff = NULL;
	char *video_buff = NULL;
	
	sdk_frame_t *audio_head = NULL;
    sdk_frame_t *video_head = NULL;
	
	*frame_buff = NULL;//外部变量初始化一下
	//先做同步
	if(UNLIKELY(!normal_sync_flag))
	{
	   video_audio_sync( g_normal_video_handle, g_normal_audio_handle);
		normal_sync_flag = 1;
	}
		//为了同步使用的
	
	if(normal_video_offset  > normal_audio_offset  )//一秒的时间
	{
		normal_audio_priority = 1; //说明要取音频了
	}	
	
	//只有同步之后才能取音频
    if(normal_audio_priority &&(*frame_buff = audio_buff = sdk_stream_ReadOneFrame(g_normal_audio_handle))&&(*frame_buff))
	{
	   audio_head = (sdk_frame_t *)audio_buff;
	   normal_audio_offset = audio_head->pts - normal_start_sec ;
	  
	  audio_count++;
	  if((VIDEO_RATE_1_25 < audio_rate)&&(audio_count%2) )
	  {
		normal_audio_priority = 1;
	  }
	  else
	  {
		normal_audio_priority = 0;
		////info(" normal_audio_priority = 0; \n");
	 }
	  
        return 0;
    }
   else  if((!normal_audio_priority)&&(*frame_buff =  video_buff = sdk_stream_ReadOneFrame(g_normal_video_handle))&&(*frame_buff))
   {
		video_head = (sdk_frame_t*)video_buff;
		normal_video_offset = video_head->pts 	- normal_start_sec;
		 normal_audio_priority = 1;
        return 0;
    }
    return -1;
}


static void close_normal_stream()
{
    info(">>>>>>>>>>>>>>> close_stream<<<<<<<<<<<<<<<<  \n");

	if(g_normal_video_handle)
	{
		sdk_stream_Close(g_normal_video_handle);
		g_normal_video_handle = NULL;
	}
    if(g_normal_audio_handle)
    {
		sdk_stream_Close(g_normal_audio_handle);
		g_normal_audio_handle = NULL;
	}
    
	normal_audio_priority = 0; //一定要执行这个操作 保证第一次取到的是视频帧
	normal_video_offset = 0;
	normal_audio_offset = 0;
	normal_sync_flag = 0;
    return 0;
}


#endif

int init_record()
{
    pthread_mutexattr_t t_mutexattr;
    pthread_mutexattr_init(&t_mutexattr);
    pthread_mutexattr_settype(&t_mutexattr,PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&g_record_work_t.record_normal_mutex, &t_mutexattr);
    pthread_mutex_init(&g_record_work_t.record_event_mutex, &t_mutexattr);
    pthread_mutexattr_destroy(&t_mutexattr);
    g_manage_record.record_ctl_flag = RECORD_STATE_STOP;

    return 0;
}

int deinit_record()
{

    pthread_mutex_destroy(&g_record_work_t.record_normal_mutex);
    pthread_mutex_destroy(&g_record_work_t.record_event_mutex);
    return 0;
}


typedef struct tag_media_info{
	unsigned long long ullMediaVideo_ms;
	unsigned long long ullMediaAudio_ms;

	int iCloud_ShmVID;
	int iCloud_ShmAID;
}recoder_media_info;
int recoder_media_stream_init(recoder_media_info *p_dana_cloud_media)
{
	int ret;
	SHM_VideoInitID(SHM_ENUM_VIDEO_STREAM0_PROFILE, &p_dana_cloud_media->iCloud_ShmVID);
	SHM_AudioInitID(SHM_ENUM_AUDIO_16K_PROFILE, &p_dana_cloud_media->iCloud_ShmAID);
	
	printf("Cloud Storage Video StreamID = %d\n", p_dana_cloud_media->iCloud_ShmVID);
	printf("Cloud Storage Audio StreamID = %d\n", p_dana_cloud_media->iCloud_ShmAID);

	return 0;
}

void recoder_media_stream_release(recoder_media_info *p_dana_cloud_media)
{
	SHM_VideoRemoveID(p_dana_cloud_media->iCloud_ShmVID);
	SHM_AudioRemoveID(p_dana_cloud_media->iCloud_ShmAID);
}
static void *normal_record_thread(void *arg)
#if 1
{
    uint32_t  start_rec_ms = 0; // 记录开始录像的时间毫秒数
    uint32_t  save_frame = 0;	// 每次在I帧的时候判断
    int start_flag = 0;
    int frameType,ret;
    AviFile   avi_handle;
	int isKeyFrame;
	int current_fps = 0;
	int current_tmp = 0;
    struct timeval tv_frame = { 0, 0 };
	struct timeval tv1;
  	memset(&avi_handle,0,sizeof(AviFile));

	ShmVideoExtraData struVExtra;
    ShmAudioExtraData struAExtra;
    char tmpbuf[SHM_VIDEO_STREAM_720P_MAX_FRAME_SIZE];
	int framelen=0;
	int	iNoNewFrame;

	recoder_media_info cloud_media_info;
	memset(&cloud_media_info, 0, sizeof(cloud_media_info));
	
	recoder_media_stream_init(&cloud_media_info);
	int shmVid = cloud_media_info.iCloud_ShmVID;
	int shmAid = cloud_media_info.iCloud_ShmAID;

    g_record_work_t.g_normal_status = 1;
    while (g_record_work_t.g_normal_status)
	{
	    framelen=sizeof(tmpbuf);  
	    if(SHM_VideoReadFrameWithExtras(shmVid, tmpbuf, &framelen, (char*)&struVExtra) > 1)
	    {
			cloud_media_info.ullMediaVideo_ms = struVExtra.ullTimeStamp;
			if(struVExtra.ucKeyFrameFlag == 1)
				isKeyFrame = 1;
			else
				isKeyFrame = 0;
			//第一帧存储必须从I帧开始
			if(start_flag == 0 && isKeyFrame == 1)
			{
			  memset(&avi_handle,0,sizeof(AviFile));
			  avi_handle.normal_buf_flag = 1;
			  g_record_work_t.g_normal_duration = g_manage_record.record_switch_time;
			  start_rec_ms = struVExtra.ullTimeStamp;
			  record_set_start_time(&avi_handle,struVExtra.ullTimeStamp);
			  current_fps = 20;
			  info(">>>>>>>>file_start=%llu struVExtra.ullTimeStamp====%llu\n",avi_handle.file_info.start,struVExtra.ullTimeStamp);
			  g_record_work_t.g_normal_duration = g_manage_record.record_switch_time;
			  ret = _record_normal_start(&avi_handle,g_manage_record.record_type,g_manage_record.record_switch_time,current_fps);
			  if(ret < 0)
			  {
				  error("_record_start Error!!\n");
				  break;
			  }
			  
			  start_flag = 1;
		    }
						
			//每次收到一个I帧的时候需要判断是否需要切断
			if(start_flag == 1)
			{
			    if(SDK_VIDEO_FRAME_I == struVExtra.ucKeyFrameFlag)
				   info("[NORMAL] recording time [%d] \n", (struVExtra.ullTimeStamp - start_rec_ms)/1000);					

				if((SDK_VIDEO_FRAME_I == struVExtra.ucKeyFrameFlag)&& 
					( ( struVExtra.ullTimeStamp - start_rec_ms ) >= (avi_handle.init_param.duration*1000) ))
				{
					info("rec time of one file is %d sec, time to stop.\n", avi_handle.init_param.duration);
					if( SDK_VIDEO_FRAME_I == struVExtra.ucKeyFrameFlag)  save_frame  = 1;
					//切换录像文件
					if(save_frame)
					{
						record_set_end_time(&avi_handle,struVExtra.ullTimeStamp);
						avi_handle.duration = TIMEVAL_DIFF_MS(start_rec_ms, struVExtra.ullTimeStamp) ;	//ms
						info("Total video count=%d, audio_count:%d Duration=%u(ms)\n", avi_handle.video_count,avi_handle.audio_count
							 ,avi_handle.duration);
					}
					else
					{
						gettimeofday(&tv1, NULL);
						record_set_end_time(&avi_handle,tv1.tv_sec);	
						avi_handle.duration = TIMEVAL_DIFF_MS(start_rec_ms, tv1.tv_sec*1000) ;	//ms
					}
					
					ret = _record_normal_stop(&avi_handle);
					//重新循环录像
		            start_flag = 0;
					continue;
				}
				else
				{
					frameType = (SDK_VIDEO_FRAME_I == struVExtra.ucKeyFrameFlag)? 1:0;
					
					ret = avi_record_write_extend(&avi_handle,tmpbuf,framelen, frameType,struAExtra.ullTimeStamp);
					if(ret < 0)
					{
						//pthread_mutex_lock(&g_record_work_t.record_normal_mutex);
						//g_manage_record.record_ctl_flag = RECORD_STATE_STOP;
						//pthread_mutex_unlock(&g_record_work_t.record_normal_mutex);
						//break;
					}
				}
			}
			
		}
		else
		{
			iNoNewFrame++;
		}
	    // write audio frame, maybe more than one
	    if(start_flag == 1)
	    {
		    while(cloud_media_info.ullMediaAudio_ms < cloud_media_info.ullMediaVideo_ms)
		    {
		        framelen=sizeof(tmpbuf);
				if(SHM_AudioReadFrameWithExtras(shmAid, tmpbuf, &framelen, (char *)&struAExtra)>1)
				{
					cloud_media_info.ullMediaAudio_ms = struAExtra.ullTimeStamp;
					
					
					frameType = 2;
					ret = avi_record_write_extend(&avi_handle,tmpbuf,framelen, frameType,struAExtra.ullTimeStamp);
					if(ret < 0)
					{
						//break;
					}
				}
				else
				{
					iNoNewFrame++;
				    break;
				}
		    }
	    }
		if(iNoNewFrame == 2)
			usleep(20000);
    }
    
fin:
	if(start_flag)
	{
		 record_set_end_time(&avi_handle,struAExtra.ullTimeStamp);	 
		 avi_handle.duration = TIMEVAL_DIFF_MS(start_rec_ms, struAExtra.ullTimeStamp) ;  //ms
		 ret = _record_normal_stop(&avi_handle);
		 if(-1 == ret)
		 {
			 error("_record_stop:Error!!\n");
		 }
	}
	save_frame = 0;
	g_record_work_t.g_normal_status = 0;
	printf("======> st_free channel = %d\n");
	recoder_media_stream_release(&cloud_media_info);
    printf("normal_recoder thread exit\n");
    return NULL;
}

#else
{
    int i;
    int ret;
	int audio_ret;
    sdk_frame_t *frame_head = NULL;
    uint32_t  start_rec_ms = 0; // 记录开始录像的时间毫秒数
    uint32_t  save_frame = 0;	// 每次在I帧的时候判断
    int start_flag = 0;
    int frameType;
    char time64str[30];;
    AviFile   avi_handle;
    char *frame_buff = NULL;
    struct timeval tv_frame = { 0, 0 };
	int current_fps = 0;
	int current_tmp = 0;
	int audio_rate = 1;
	uint32_t  time_count = 0;
  	memset(&avi_handle,0,sizeof(AviFile));

	open_normal_stream();
   
    g_record_work_t.g_normal_status = 1;
    while (g_record_work_t.g_normal_status)
    {
        switch (g_manage_record.record_ctl_flag)
        {

			//start record. 找到I帧
		   case    RECORD_STATE_START: //开始录像
		   {
			   g_record_work_t.record_time_zone_pre = g_record_work_t.record_time_zone;
			   memset(&avi_handle,0,sizeof(AviFile));
			   start_flag = 0;
			   avi_handle.normal_buf_flag = 1;
			   g_record_work_t.g_normal_duration = g_manage_record.record_switch_time;
			   info("============g_normal_duration :%d save_frame:%d \n",g_record_work_t.g_normal_duration,save_frame);
				   
			   //如果没有保存关键帧  就寻找关键帧				   
			   if(!save_frame)
			   {
				   while(1) //video
				   {
					   if(0 !=  pull_normal_stream(&frame_buff,VIDEO_RATE_1_0))
						   continue;
					   
					   frame_head = (sdk_frame_t  *)frame_buff;
	
					   if(SDK_VIDEO_FRAME_I == frame_head->frame_type)
						   break;
	
				   }
				   
				   info("find  key fram !!! \n");
			   }
				   
			   info("======frame_no:%d =======frame_type:%d == frame_size:%d  start_flag:%d\n"
					,frame_head->frame_no,frame_head->frame_type,frame_head->frame_size,start_flag);
			  
			  //为录像设定开始时间UNLIKELY
			  if(UNLIKELY(!start_flag))
			  {			  
				current_tmp = frame_head->video_info.frame_rate ;
				//25 20 15 10
				if(current_tmp >= 25) 
				{ 
					current_fps = current_tmp;
					audio_rate = VIDEO_RATE_1_0;
				}
				else if(current_tmp < 25 && current_tmp <= 20) 
				{ 
					current_fps = 20;
					audio_rate =  VIDEO_RATE_1_25;
				}
				else if(current_tmp < 20 && current_tmp <= 15)
				{
					current_fps = 15;
					audio_rate =  VIDEO_RATE_1_50;
				}
				else 
				{
					current_fps = 10;
					audio_rate =  VIDEO_RATE_2_50;
				}
			   avi_handle.normal_buf_flag = 1;			   
			   start_rec_ms = frame_head->pts;
			   record_set_start_time(&avi_handle,frame_head->sec);
			   
			   info(">>>>>>>>file_start=%llu frame_head->sec=%d\n",avi_handle.file_info.start,frame_head->sec);
			   g_record_work_t.g_normal_duration = g_manage_record.record_switch_time;
			   ret = _record_normal_start(&avi_handle,g_manage_record.record_type,g_manage_record.record_switch_time,current_fps);
				if(ret < 0)
				{
				   error("_record_start Error!!\n");
				   pthread_mutex_lock(&g_record_work_t.record_normal_mutex);
				   if(RECORD_STATE_STOP == g_manage_record.record_ctl_flag)
				   {
					   warning("==============xxxxxxxxxxxxxxxxxxx=====2=========\n");
					   pthread_mutex_unlock(&g_record_work_t.record_normal_mutex);
					   break;
				   }
				   g_manage_record.record_ctl_flag = RECORD_STATE_START;
				   pthread_mutex_unlock(&g_record_work_t.record_normal_mutex);
				   usleep(100*1000);
				   break;
				}
				  ///start_rec_sec = frame_head->sec;
				  start_flag = 1;
			  }

			  // 走到这里的一定是I帧

			   frameType =  1;
			   tv_frame.tv_sec =frame_head->sec;//frame_head->pts/1000;
			   tv_frame.tv_usec=frame_head->usec;//(frame_head->pts%1000)*1000;
			   //pthread_mutex_lock(&g_record_work_t.record_normal_mutex);
			   ret = avi_record_write(&avi_handle, frame_head->data, frame_head->frame_size, frameType,&tv_frame);
			   //pthread_mutex_unlock(&g_record_work_t.record_normal_mutex);
			   warning("===================4====ret [%d]===========\n",ret);
			   if(ret  < 0)
			   {
				   error("avi_record_write Error!!\n");
				   avi_record_nornal_close(&avi_handle);
				   pthread_mutex_lock(&g_record_work_t.record_normal_mutex);
				   g_manage_record.record_ctl_flag = RECORD_STATE_START;
				   pthread_mutex_unlock(&g_record_work_t.record_normal_mutex);
				   start_flag = 0;
				   sleep(1);
				   break;
			   }
				save_frame = 0;		   
			   pthread_mutex_lock(&g_record_work_t.record_normal_mutex);
			   g_manage_record.record_ctl_flag = RECORD_STATE_WRITE;//进入开始录像状态
			   pthread_mutex_unlock(&g_record_work_t.record_normal_mutex);
			   info("jump to RECORD_STATE_WRITE!!!! \n");
			   break;
		   }
		   //close record.
		  case RECORD_STATE_STOP:
		  {
			   if(start_flag)
			   {
					if(frame_head)
					{
					record_set_end_time(&avi_handle,frame_head->sec);	
						//avi_handle.duration = TIMEVAL_DIFF_SEC(start_rec_ms, tv1.tv_usec) / 1000; //ms
					avi_handle.duration = TIMEVAL_DIFF_MS(start_rec_ms, frame_head->pts) ;	//ms
					}
					else
					{
						struct timeval tv1;
						gettimeofday(&tv1, NULL);
				record_set_end_time(&avi_handle,tv1.tv_sec);    
						//avi_handle.duration = TIMEVAL_DIFF_SEC(start_rec_ms, tv1.tv_usec) / 1000; //ms
				avi_handle.duration = TIMEVAL_DIFF_MS(start_rec_ms, tv1.tv_sec*1000) ;  //ms
					}

				   ret = _record_normal_stop(&avi_handle);
					if(-1 == ret)
					{
						error("_record_stop:Error!!\n");
					}
			   }
			   
			   pthread_mutex_lock(&g_record_work_t.record_normal_mutex);
			   g_manage_record.record_ctl_flag = RECORD_STATE_START;
			   pthread_mutex_unlock(&g_record_work_t.record_normal_mutex);
	
			   save_frame = 0;
			   
			   g_record_work_t.g_normal_status = 0;
			   printf("======> st_free channel = %d\n", i);
			   break;
		   	}
        case	RECORD_STATE_WRITE:
			g_record_work_t.g_normal_status = 1;

            while(RECORD_STATE_STOP != g_manage_record.record_ctl_flag) //video
            {
            			
				// 取音视频流
				if(pull_normal_stream(&frame_buff,audio_rate) != 0)
				{
					sleep_time(0, 1*1000);
					continue;
				}
				
					frame_head = (sdk_frame_t  *)frame_buff;
					time_count++;  //计数器的目的是避免平凡的执行abs
	                //每次收到一个I帧的时候需要判断是否需要切断
	                //用来判断是按照 文件大小切断 还是按照时间切断
					if((SDK_VIDEO_FRAME_I == frame_head->frame_type)||
						(abs( frame_head->pts - start_rec_ms ) >= avi_handle.init_param.duration*2*1000))
	                {
	                
	                  if(UNLIKELY(abs( current_tmp - frame_head->video_info.frame_rate)>= 10)) //晚上降帧切换)
	                  {
						
						info("test current_fps:%d , frame_head->video_info.frame_rate:%d \n", current_fps , frame_head->video_info.frame_rate); 
						pthread_mutex_lock(&g_record_work_t.record_normal_mutex);
	                    g_manage_record.record_ctl_flag = RECORD_STATE_CREATE_FILE;//进入开始录像状态
	                    pthread_mutex_unlock(&g_record_work_t.record_normal_mutex);
						if( SDK_VIDEO_FRAME_I == frame_head->frame_type)  save_frame  = 1;
						current_tmp = frame_head->video_info.frame_rate;
						break;
					  }
	                    else if( RECORD_IS_FIXED_DURATION(avi_handle.init_param.mode))
	                    {
	                        info("[NORMAL] recording time [%d]\n", (frame_head->pts - start_rec_ms)/1000);                	
							if ( ( frame_head->pts - start_rec_ms ) >= (avi_handle.init_param.duration*1000) )
	                        {
	                            info("rec time of one file is %d sec, time to stop.\n", avi_handle.init_param.duration);
	                            pthread_mutex_lock(&g_record_work_t.record_normal_mutex);
	                            g_manage_record.record_ctl_flag = RECORD_STATE_CREATE_FILE;//进入开始录像状态
	                            pthread_mutex_unlock(&g_record_work_t.record_normal_mutex);
								if( SDK_VIDEO_FRAME_I == frame_head->frame_type)  save_frame  = 1;
	                            break;

	                        }
	                    }
	                }
				
					if(frame_head->frame_type == SDK_AUDIO_FRAME_A)
		     		{
						frameType	= 2;			
		      		}
			 		else
		     		{
                   		frameType = (SDK_VIDEO_FRAME_I == frame_head->frame_type)? 1:0;
		     		}
	
					tv_frame.tv_sec =frame_head->sec;//frame_head->pts/1000;
					tv_frame.tv_usec=frame_head->usec;//(frame_head->pts%1000)*1000;
					//pthread_mutex_lock(&g_record_work_t.record_normal_mutex);
					ret = avi_record_write(&avi_handle,frame_head->data, frame_head->frame_size, frameType,&tv_frame);
					//pthread_mutex_unlock(&g_record_work_t.record_normal_mutex);
					if(ret < 0)
					{
						pthread_mutex_lock(&g_record_work_t.record_normal_mutex);
						g_manage_record.record_ctl_flag = RECORD_STATE_STOP;
						pthread_mutex_unlock(&g_record_work_t.record_normal_mutex);
						break;
					}
            }
			
            break;
            //用来切换文件
        case RECORD_STATE_CREATE_FILE:
        {
            if(save_frame)
            {

                record_set_end_time(&avi_handle,frame_head->sec);
                //avi_handle.duration = TIMEVAL_DIFF_SEC(start_rec_ms, frame_head->pts) / 1000;  //ms
                avi_handle.duration = TIMEVAL_DIFF_MS(start_rec_ms, frame_head->pts) ;  //ms
				info("Total video count=%d, audio_count:%d Duration=%u(ms)\n", avi_handle.video_count,avi_handle.audio_count
                     ,avi_handle.duration);
            }
			else
			{
			
			struct timeval tv1;
			gettimeofday(&tv1, NULL);
			record_set_end_time(&avi_handle,tv1.tv_sec);	
			//avi_handle.duration = TIMEVAL_DIFF_SEC(start_rec_ms, tv1.tv_usec) / 1000;	//ms
			avi_handle.duration = TIMEVAL_DIFF_MS(start_rec_ms, tv1.tv_sec*1000) ;	//ms
			}
			
            ret = _record_normal_stop(&avi_handle);
			
			pthread_mutex_lock(&g_record_work_t.record_normal_mutex);
            g_manage_record.record_ctl_flag = RECORD_STATE_START;//初始化 进入开始录像状态
			pthread_mutex_unlock(&g_record_work_t.record_normal_mutex);

            break;
        }
       
        //write record.在状态不明的情况下等待....
        default:
        {
			g_record_work_t.g_normal_status = 0;
            usleep(500*1000);
			
            break;
        }
        }
    }
	
	close_normal_stream();
    printf("out of record thread[%d]...............\n", i);
	return(NULL);
}
#endif
int start_normal_record_stream(SDK_RECORD_TYPE_E record_type,int duration)
{
	if(1 == g_record_work_t.g_normal_status)
	{
		error(" Normal recording.........\n");
		return 0;
	}	
	
    g_manage_record.record_type = record_type;
    g_manage_record.record_ctl_flag = RECORD_STATE_START;
    g_manage_record.record_switch_time = (15 > duration) ? 15:duration;  // 最小录像文件为15秒

    int ret = pthread_create(&g_record_work_t.g_record_thread_id,NULL, normal_record_thread, NULL);
    if (ret < 0)
    {
        perror("st_record_startRecordThread():pthread_create record");
		return -1;
    }
	//分离线程
	pthread_detach(g_record_work_t.g_record_thread_id);

    return(0);
}



int stop_normal_record_stream()
{
	pthread_mutex_lock(&g_record_work_t.record_normal_mutex);
	//g_manage_record.record_ctl_flag = RECORD_STATE_STOP;
	g_record_work_t.g_normal_status = 0;
	pthread_mutex_unlock(&g_record_work_t.record_normal_mutex);
   	sleep(1);

    return 0;
}

/******************************/

#if 1
static ST_HDL g_event_video_handle =  NULL;
static ST_HDL g_event_audio_handle =  NULL;

static  int g_audio_priority = 0;
static int g_sync_flag = 0; //同步标志位
static int g_start_ms = 0; // 开始取视屏的时间，用来作为一个标准的开始时间
static int g_video_offset = 0; //最新视频相对于g_start_ms的偏移
static int g_audio_offset = 0; //最新音频相对于g_start_ms的偏移

static int g_audio_count = 0; //记录音频连续取了10个之后 取视频

static void open_event_stream()
{
    
	
	struct timeval tv1;
	
    g_event_video_handle =  sdk_stream_Open_Pre(1,0,7);
	usleep(100*1000);
    g_event_audio_handle =  sdk_stream_Open_Pre(0,0,7);
	g_audio_priority = 0;
	g_sync_flag = 0;

	g_start_ms= 0;

	info(">>>>>>>>>>>>>>> open_event_stream  g_start_ms:%d <<<<<<<<<<<<<<<<  \n",g_start_ms);

    return 0;
}

static int32_t pull_event_stream(char **frame_buff)
{
    int ret;
	char *audio_buff = NULL;
	char *video_buff = NULL;
	
	sdk_frame_t *audio_head = NULL;
    sdk_frame_t *video_head = NULL;
	
	*frame_buff = NULL;//外部变量初始化一下

    if((!g_audio_priority) &&(*frame_buff =  video_buff = sdk_stream_ReadOneFrame(g_event_video_handle))&&(*frame_buff))
    {
    	video_head = (sdk_frame_t*)video_buff;
   		g_video_offset = video_head->pts  - g_start_ms;
    	while(!g_sync_flag)
    	{
			#if 1
			audio_buff = sdk_stream_ReadOneFrame(g_event_audio_handle);
			
			if(NULL == audio_buff)  continue; // 有可能取不到音频
			
			audio_head = (sdk_frame_t *)(audio_buff);
				
	     info("current video audio pts:%d ====== :%d   \n",video_head->sec,audio_head->sec );
			if((video_head->pts <= audio_head->pts)||(video_head->pts - audio_head->pts < 200) )
			{
				g_sync_flag = 1;
				g_audio_priority = 1;
				g_audio_offset = audio_head->pts - g_start_ms ;
				return 0; //直接返回  
			}
			#endif
		}
		 if( g_video_offset > g_audio_offset )
		 {
			g_audio_priority = 1;
		 }
		 else
		 {
			 g_audio_priority  = 0; //说明取音频
		 }
        return 0;
    }
	//只有同步之后才能取音频
    if(g_audio_priority&&(*frame_buff = audio_buff = sdk_stream_ReadOneFrame(g_event_audio_handle))&&(*frame_buff))
    {
	   // info("pull_event_stream ====== audio g_video_pts:%d \n",g_video_pts);
	
	   audio_head = (sdk_frame_t *)audio_buff;
	  g_audio_offset = audio_head->pts - g_start_ms ;
	  if((g_audio_offset - g_video_offset) > 200) //大于100ms
	  {
		  g_audio_priority	= 0; //说明要取视频了
	  }
	  else
         g_audio_priority  = 1; //说明取音频
        return 0;
    }

	// 都没有的情况下还是先取音频
	g_audio_priority = 1;
    return -1;
}

static void close_event_stream()
{
    info(">>>>>>>>>>>>>>> close_stream<<<<<<<<<<<<<<<<  \n");

    sdk_stream_Close(g_event_video_handle);
    sdk_stream_Close(g_event_audio_handle);
	g_event_video_handle = NULL;
	g_event_audio_handle = NULL;
	g_audio_priority = 0; //一定要执行这个操作 保证第一次取到的是视频帧
	g_video_offset = 0;
	g_audio_offset = 0;
	g_sync_flag = 0;

    return 0;
}


#endif


//下面的函数主要实现事件录像功能

static void *event_record_thread(void *arg)
{
    int duration = g_record_work_t.g_event_duration;
	g_record_work_t.record_time_zone_pre = g_record_work_t.record_time_zone;

    struct timeval tv_frame = { 0, 0 };
    int i;
    int ret;
    uint32_t  start_rec_ms = 0; // 记录开始录像的时间秒数
    int start_flag = 0;
    int frameType;
	int frameAType;
    char time64str[30];;
    AviFile   avi_handle;
	
    char *frame_buff = NULL;
    sdk_frame_t *frame_head = NULL;	
	
    int  stream_status = -1;
	static ST_HDL video_handle  = NULL;
	static ST_HDL audio_handle = NULL;
	
    int event_type = *((int *)arg);
	
	memset(&avi_handle,0,sizeof(AviFile));
	
	avi_handle.event_buf_flag = 1;

	if (g_sd_tats < 0)
	{
		error("SD card Error!!!\n");
		pthread_mutex_lock(&g_record_work_t.record_event_mutex);
		g_record_work_t.g_event_status = 0;
		pthread_mutex_unlock(&g_record_work_t.record_event_mutex);
			
		pthread_exit(&g_record_work_t.g_event_thread_id);
		return ;
	}
	
	open_event_stream();
	printf("AAAAAAAAAAa ============duration:%d \n",duration);
	while (g_record_work_t.g_event_status)
    {
		if(0 != pull_event_stream(&frame_buff))
		{
			sleep_time(0, 2*1000);
			continue;
		}
			
        frame_head = (sdk_frame_t  *)frame_buff;
		if(UNLIKELY(!start_flag))  // 第一次创建找I帧
        {
            if(SDK_VIDEO_FRAME_I != frame_head->frame_type)
                continue;
			
			start_rec_ms = frame_head->pts;
			info("===========start_rec_sec:%d  \n",frame_head->sec);
            record_set_start_time(&avi_handle,frame_head->sec);

			printf("AAAAAAAAAAa ============00000000000000 \n");
			ret =  _record_event_start(&avi_handle,SDK_RECORD_TYPE_ALARM,duration,frame_head->video_info.frame_rate);  // 录像时长
   			if(ret == -1)
   			{
   			error("_record_start Error!!!\n");
			close_event_stream();
			pthread_mutex_lock(&g_record_work_t.record_event_mutex);
			g_record_work_t.g_event_status = 0;
			pthread_mutex_unlock(&g_record_work_t.record_event_mutex);
			return NULL ;
   			}
			printf("AAAAAAAAAAa ============111111111111111 \n");
            start_flag = 1;
		
        }
		///info("frame_type:%d ============ frame_no:%d ====sec:%d \n",frame_head->frame_type,frame_head->frame_no,frame_head->sec);
		//准备切断  此处为了防止流库里面一直没有I帧 导致没办法切断的处理
        if(UNLIKELY(SDK_VIDEO_FRAME_I == frame_head->frame_type)||(abs( frame_head->pts - start_rec_ms ) >= (avi_handle.init_param.duration*2*1000)))
        {
            if( RECORD_IS_FIXED_SIZE(avi_handle.init_param.mode))
            {
                int avi_size = avi_handle.data_offset + 8
                               + avi_handle.index_count * 16 + 100; //leave 100 BYTE space
                if( avi_size > avi_handle.init_param.size_m * 1024 * 1024 )
                {
                    info("rec size of one file is %d MB, time to stop.\n",
                         avi_handle.init_param.size_m);
                    break;
                }
            }
            else if( RECORD_IS_FIXED_DURATION(avi_handle.init_param.mode))
            {
            	// count recording time
				
            	info("[WANGRONG] Motion detecting recording ==> [%d]\n", ( frame_head->pts - start_rec_ms )/1000);
                if ( ( frame_head->pts - start_rec_ms ) >= (avi_handle.init_param.duration *1000) )
                {
                    info("rec time of one file is %d sec, time to stop.\n", avi_handle.init_param.duration);
                    break;

                }
            }
        }
        if(frame_head->frame_type == SDK_AUDIO_FRAME_A)
        {
            frameType  = 2;
        }
        else
        {
            frameType = (SDK_VIDEO_FRAME_I == frame_head->frame_type)? 1:0;
        }
        tv_frame.tv_sec =frame_head->pts/1000;
        tv_frame.tv_usec=(frame_head->pts%1000)*1000;
        ret = avi_record_write(&avi_handle, frame_head->data, frame_head->frame_size, frameType,&tv_frame);
		if(ret < 0)
		{
			error("avi_record_write Error!!\n");
			break;
		}
		
    }

    record_set_end_time(&avi_handle,frame_head->sec);
    avi_handle.duration = TIMEVAL_DIFF_MS(start_rec_ms, frame_head->pts) ;  //ms
    
    if(avi_handle.duration < 1000) avi_handle.duration = 1000;
	
    info("Event Total video count=%d, Duration=%u(ms)\n", avi_handle.video_count,avi_handle.duration);
    ret = _record_event_stop(&avi_handle);
	info("========= _record_event_stop \n");

	
    close_event_stream();
	
	if(-1 == ret)
	{
		error("_record_event_stop Error!!\n");
	}		
   	//dropbox_sendFile_record_add(avi_handle.filename);
    //info("EventRecordThread ==== g_event_run_flag:%d \n", g_record_work_t.g_event_duration);
		sync();

	//avi_record_close()
	pthread_mutex_lock(&g_record_work_t.record_event_mutex);
	g_record_work_t.g_event_status = 0;
	pthread_mutex_unlock(&g_record_work_t.record_event_mutex);

	return NULL;
}



int start_event_record_stream(SDK_RECORD_TYPE_E record_type,int duration)
{
    
	int ret = -1;
	int event_type = 0;
	pthread_mutex_t lock_mutex ;
	
	// 如果正在录像，则退出
	if(g_record_work_t.g_event_status == 1)
	{
		error("[WARING]::::Event recording is working,EXIT!!!!\n");
		return -1;
	}

	//g_record_work_t.record_time_zone = TimeZone;
			
	pthread_mutex_lock(&g_record_work_t.record_event_mutex);
    g_record_work_t.g_event_status = 1;
	pthread_mutex_unlock(&g_record_work_t.record_event_mutex);
	warning("Start EVENT Recording....!!!duration:%d \n",duration);
	
	// at least 10s video time  
    int local_duration = (duration > 10) ? duration:10;
	
	g_record_work_t.g_event_duration = local_duration;
	
    info("event_record_stream  record_type:%d duration:%d \n",record_type,local_duration);

    ret = pthread_create(&g_record_work_t.g_event_thread_id,NULL, event_record_thread, (void *)&event_type);
    if (ret < 0)
    {
        perror("st_record_startRecordThread():pthread_create record");
        return -1;
    }
	
	pthread_detach(g_record_work_t.g_event_thread_id);
	
    return 0;

}


//*这里算是强制停止 事件录像
int stop_event_record_stream()
{
	///if(1 == g_record_work_t.g_event_status)
	{
		pthread_mutex_lock(&g_record_work_t.record_event_mutex);
		g_record_work_t.g_event_status = 0;
		pthread_mutex_unlock(&g_record_work_t.record_event_mutex);
		usleep(10000*5);
	}
    return 0;
}




#define DROPBOX_SEND_FILE_RECORD_FILE "/mnt/sd_card/sysconfig/dropbox_file"
#define DROPBOX_SEND_FILE_RECORD_FILE_TMP "/mnt/sd_card/sysconfig/dropbox_tmp"

pthread_mutex_t g_dropboxFileMutex;


int dropbox_sendFile_record_add(char *pFileStr)
{
	int 	ret = 0;
	FILE 	* fp = NULL;
	pthread_mutex_lock(&g_dropboxFileMutex);
	if(pFileStr != NULL)
	{
		if((fp = fopen(DROPBOX_SEND_FILE_RECORD_FILE,"a+")) == NULL)
		{
			//printf("dropbox_sendFile_record_add Error!\n");
			ret =  -1;
		}
		else
		{
			fputs(pFileStr, fp);
		}
	}

	if(fp != NULL)
	{
		fclose(fp);
	}
	pthread_mutex_unlock(&g_dropboxFileMutex);
	return ret;
}

int dropbox_sendFile_record_get(char *pBuf, int bufSize)
{
	int ret = 0;
	FILE * fp = NULL;
	pthread_mutex_lock(&g_dropboxFileMutex);
	if(pBuf != NULL)
	{
		if((fp = fopen(DROPBOX_SEND_FILE_RECORD_FILE,"r")) == NULL)
		{
			//printf("Open dropbox_sendFile_record_get Error!\n");
			ret =  -1;
		}
		else
		{
			fgets(pBuf, bufSize, fp);
			if(strstr(pBuf, "/mnt/sd_card/") == NULL)
			{
				ret =  -2;
			}
		}
		
	}

	if(fp != NULL)
	{
		fclose(fp);
	}
	pthread_mutex_unlock(&g_dropboxFileMutex);
	return ret;
}

int dropbox_sendFile_record_del()
{
	char cmd[256];
	memset(cmd, 0,sizeof(cmd));
	pthread_mutex_lock(&g_dropboxFileMutex);
	sprintf(cmd, "sed '1d' %s >> %s", DROPBOX_SEND_FILE_RECORD_FILE, DROPBOX_SEND_FILE_RECORD_FILE_TMP);
	system(cmd);
	//system("sed '1d' "DROPBOX_SEND_FILE_RECORD_FILE" >> "DROPBOX_SEND_FILE_RECORD_FILE_TMP"");

	memset(cmd, 0,sizeof(cmd));
	sprintf(cmd, "mv %s %s", DROPBOX_SEND_FILE_RECORD_FILE_TMP, DROPBOX_SEND_FILE_RECORD_FILE);
	//system("mv "DROPBOX_SEND_FILE_RECORD_FILE_TMP" "DROPBOX_SEND_FILE_RECORD_FILE_TMP"");
	system(cmd);
	pthread_mutex_unlock(&g_dropboxFileMutex);
	return 0;
}



