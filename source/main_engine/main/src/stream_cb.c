#include "sdk_struct.h"
#include "log.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

void *main_event_cb(void *param)
{
	//return 0;
	warning("Jensen event pull.............\n");
    if(param)
    {
        sdk_event_t *event = (sdk_event_t *)param;
        // printf("format_result_cb enevt_type:%d result:%d \n",event->enevt_type,event->result);

	    //暂时在这里打印一下 观察一下函数的运行情况
		if(1 == event->result)//事件开始
		{
			//printf(" main_event_cb  event start \n");
		}
		else
		{
			//printf(" main_event_cb  event end\n");
		}
            
        switch ( event->enevt_type)
        {
            case SDK_EVENT_MOTION :
            {
                //printf(" ======================SDK_EVENT_MOTION ==========================+++++!!!! sdk_av_snap\n");
    			//sdk_snap_info_t snap_info;
    			//snap_info.width = 1280;
    			//snap_info.height = 720;
    			//sdk_av_snap(0,&snap_info,"/mnt/nfs/motion_event.bmp");
                break;
            }
            case SDK_EVENT_AUDIOALERT :
            {
                printf(" SDK_EVENT_AUDIOALERT !!!! \n");
                break;
            }
    		case SDK_EVENT_IR :
            {
                printf(" SDK_EVENT_IOALERT !!!! \n");
                break;
            }
            default:
            {
                printf("nothing!!!! \n");

                break;
            }
        }
		if((SDK_EVENT_MOTION == event->enevt_type)&&(1 == adapt_get_ircut_dalay()))
		{
			return ;
		}
		printf("event->enevt_type=%d\n",event->enevt_type);
    }
	
	event_send_to_pool(param);//发送一个事件到事件缓冲池
    return ;

}

void *main_audio_stream_cb(void *param)
{
    if(!param)
        return;
    sdk_stream_info_t *stream_info = (sdk_stream_info_t *)param;
#if 0
printf("+_+_+_+_+_+_+_+_+_+_   ZW_TEST   start+_+_+_+_+_+_+_+_+_+_+_  \n");
    printf("\n\n =====zw_test  :======= \n"
           " ch========:%d \n"
           " sub_ch========:%d \n"
           " frame_no========:%d \n"
           " frame_type========:%d \n"
           " frame_size========:%d \n"
           " %d:::%d ========\n"
           " pts========:%lld \n"
           " encode_type========:%d \n"
           " channels========:%d \n"
           " samples========:%d \n"
           " bits========:%d \n"
           " frame_addr========:%p \n"
           " ===end====:\n"
           ,stream_info->ch
           ,stream_info->sub_ch
           ,stream_info->frame_head.frame_no
           ,stream_info->frame_head.frame_type
           ,stream_info->frame_head.frame_size
           ,stream_info->frame_head.sec
           ,stream_info->frame_head.usec
           ,stream_info->frame_head.pts
           ,stream_info->frame_head.audio_info.encode_type
           ,stream_info->frame_head.audio_info.channels
           ,stream_info->frame_head.audio_info.samples
           ,stream_info->frame_head.audio_info.bits
           ,stream_info->frame_addr);
    printf("+_+_+_+_+_+_+_+_+_+_   ZW_TEST  END  +_+_+_+_+_+_+_+_+_+_+_  \n");
#endif

    if(stream_info->ch == STREAM_AUDIO_INDEX)
    {
        //main_write_audio_data(0,stream_info->frame_addr,stream_info->frame_head.frame_size);
        sdk_stream_WriteOneFrameEx(0, 0, &stream_info->frame_head, stream_info->frame_addr);
    }

}

void *main_video_stream_cb(void *param)
{
    if(!param)
        return;
    sdk_stream_info_t *stream_info = (sdk_stream_info_t *)param;
    if(stream_info->ch >= 0 && stream_info->ch < 2)
    {
        sdk_stream_WriteOneFrameEx(1,stream_info->ch, &stream_info->frame_head,stream_info->frame_addr);
    }
    return ;

}

