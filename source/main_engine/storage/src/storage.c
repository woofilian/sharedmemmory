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
#include <sys/vfs.h>

#include <assert.h>

#include "sdk_global.h"
#include "sdk_struct.h"
#include "log.h"

#include "avi_typedef.h"
#include "storage.h"
#include "avi_common.h"
#include "record.h"
#include "mmc_api.h"

#define _MSG_DEBUG 0

/*************************************************/

#define	HAND_RECORD_FLAG	0x00000001	//??????
#define	MOVE_RECORD_FLAG	0x00000002	//??????
#define	LOST_RECORD_FLAG	0x00000004	//??????
#define	INPUT_RECORD_FLAG	0x00000008	//????
#define	TIMER_RECORD_FLAG	0x00000010	//??????
#define	ALL_RECORD_FLAG		0x0000001f	//??????
sna_callback  cb;

extern record_work_t g_record_work_t;

/*****************************************************************************
 ? ? ?  : st_refresh_time_zone
 ????  : ?��??
 ????  : time_zone ??
 ????  : ?
 ? ? ?  : 
 ????  : 
 ????  : 
 
 ??z?      :
  1.?    ��   : 2015��6?25?
    ?    ?   : bingchuan
    ????   : ��????

*****************************************************************************/

int st_refresh_time_zone(int time_zone)
{
	static int i = 0;
	unsigned char old_zone;
	if(i != 0)
	{
		old_zone = g_record_work_t.record_time_zone ;
	}
	if( time_zone < 0 || time_zone > 24)
		return -1;

	g_record_work_t.record_time_zone  = time_zone;

	if(i != 0)
	{
		if(old_zone != g_record_work_t.record_time_zone)
		{
			warning("change time Zone!!!!\n");
			if(1 == g_record_work_t.g_event_status)
			{
				stop_event_record_stream();
				sleep(1);
				start_event_record_stream(SDK_RECORD_TYPE_MOTION, g_record_work_t.g_event_duration);
			}
			if(1 == g_record_work_t.g_normal_status)
			{
				stop_normal_record_stream();
				usleep(1000);
				start_normal_record_stream(SDK_RECORD_TYPE_MANU, g_record_work_t.g_normal_duration);
			}
		}
	}
	i++;
	return 0;
}

/*****************************************************************************
 ? ? ?  : st_record_init
 ????  : ??????��?
 ????  : ?
 ????  : ?
 ? ? ?  : 
 ????  : 
 ????  : 
 
 ??z?      :
  1.?    ��   : 2015��6?25?
    ?    ?   : bingchuan
    ????   : ��????

*****************************************************************************/

int st_record_init(sna_callback func)
{
    /*
    * Start detecting SD card 
    */
    sd_card_detecte_thread();
	//sd_card_file_fixed_thread();
    init_record();

}

/*****************************************************************************
 ? ? ?  : st_record_deinit
 ????  : ?????��????
 ????  : ?
 ????  : ?
 ? ? ?  : 
 ????  : 
 ????  : 
 
 ??z?      :
  1.?    ��   : 2015��6?25?
    ?    ?   : bingchuan
    ????   : ��????

*****************************************************************************/
int st_record_deinit()
{
    stop_normal_record_stream();
    stop_event_record_stream(SDK_RECORD_TYPE_ALL);
    deinit_record();
}
/*****************************************************************************
 ? ? ?  : st_get_event_status
 ????  : ?????��??��1  : ???????�� 0 : ???��???????��
 ????  : status
 ????  : ?

*****************************************************************************/

int st_get_event_status()
{
	if(g_record_work_t.g_event_status == 1)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


/*****************************************************************************
 ? ? ?  : st_record_start
 ????  : ???��
 ????  : SDK_RECORD_TYPE_E record_type  ?��`�� ??SDK_RECORD_TYPE_E
             int duration     ???��??? ?������
 ????  : 
 ? ? ?  :  ?? ??-1,  ??????0   ???????��???????��`�� SDK_RECORD_TYPE_E
 ????  :
 ????  :

 ??z?      :
  1.?    ��   : 2015��6?18?
    ?    ?   : bingchuan
    ????   : ��????

*****************************************************************************/
int	st_record_start(SDK_RECORD_TYPE_E record_type,int duration)
{

		int ret = -1;
		if(-1 == get_sd_stats())//??SD?��???
		{
			error("==>Found no SD card!!\n");
			return -1;
		}

		switch (record_type)
	    {
		    case SDK_RECORD_TYPE_MANU :   // ??
		    case SDK_RECORD_TYPE_SCHED    :  //   ??
		    {
			  ret = start_normal_record_stream(record_type,duration);
			  break;
		    }
		    case SDK_RECORD_TYPE_MOTION   :  // ????
		    case SDK_RECORD_TYPE_ALARM    : // ????
			case SDK_RECORD_TYPE_MOTION_PRE	:	// ???? ??��
		    {
		        ret = start_event_record_stream(record_type, duration);
		        break;
		    }
		    case SDK_RECORD_TYPE_AED     :  // ????
		    case SDK_RECORD_TYPE_ALL 	: //?????????
		    case SDK_RECORD_TYPE_BCD      :  // ??
		    case SDK_RECORD_TYPE_TAMPER   :  //
		    default:
		    {
		        ret = -1;
		        break;
		    }
	    }
		
		return 0;	
}
/*****************************************************************************
 ? ? ?  : st_record_stop
 ????  : ???��
 ????  : SDK_RECORD_TYPE_E record_type  
 ????  : ?
 ? ? ?  :  ????0  ???? -1
 ????  : 
 ????  : 
 
 ??z?      :
  1.?    ��   : 2015��6?25?
    ?    ?   : bingchuan
    ????   : ��????

*****************************************************************************/
int st_record_stop(SDK_RECORD_TYPE_E record_type)
{
    int ret = -1;
    switch (record_type)
    {
    case SDK_RECORD_TYPE_ALL: //?????????
    {
        stop_normal_record_stream();
        stop_event_record_stream();
        break;
    }
    case SDK_RECORD_TYPE_MANU :   // ??
    case SDK_RECORD_TYPE_SCHED    :  //   ??
    {
        ret = stop_normal_record_stream();
        break;
    }
    case SDK_RECORD_TYPE_MOTION   :  // ????
    case SDK_RECORD_TYPE_ALARM    : // ????
    case SDK_RECORD_TYPE_AED     :  // ????
    case SDK_RECORD_TYPE_BCD      :  // ??
	case SDK_RECORD_TYPE_MOTION_PRE:
    case SDK_RECORD_TYPE_TAMPER   :  //
    {
        ret = stop_event_record_stream();
        break;
    }
    default:
    {
        ret = -1;
        break;
    }
    }
    return 0;
}

