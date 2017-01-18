/******************************************************************************

                  版权所有 (C), 2012-2022, bingchuan

 ******************************************************************************
  文 件 名   : tutk_msg_handle.c
  版 本 号   : v1.0
  作    者   : bingchuan
  生成日期   : 2015年5月4日
  功能描述   : 处理tutk的消息收发
  函数列表   :
  修改历史   :
  1.日    期   : 2015年5月4日
    作    者   : bingchuan
    修改内容   : 创建文件

******************************************************************************/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>


#include "IOTCAPIs.h"
#include "AVAPIs.h"
#include "AVFRAMEINFO.h"
#include "AVIOCTRLDEFs.h"

#include "sdk_struct.h"
#include "log.h"
#include "tutk_Server.c"
#include "storage.h"
#include "main.h"

#define MSG_BUF_SIZE 16*1024
pthread_mutex_t g_log_lock = PTHREAD_MUTEX_INITIALIZER;
#define UPGRADE_TMP_FILE_PATH "/ambarella/upgrade_packet"

#define  ALARMLOG_SIZE	2048//2*1024
#define  ALARM_PATH		"/mnt/mtd/alarmfile"
#define  ALARM_PATH1	"/mnt/mtd/alarmfile1"

#define TUTK_MSG_CTRL(_id, _arg, _ext, _ch, pmsg) ({\
				int _ret = -1;\
				pmsg->msg_id	= _id;\
				pmsg->msg_dir 	= SDK_MSG_REQ;\
				pmsg->chann 	= _ch;\
				pmsg->args		= _arg;\
				pmsg->extend	= _ext;\
				if(g_tutk_msg_cb)\
				_ret = g_tutk_msg_cb(pmsg);\
				if(_ret != 0)\
				goto __error;})
#define 	_MSG_DEBUG 0


typedef struct _avi_list_s
{
	sdk_time_t     stStartTime;
    sdk_time_t     stStopTime;
	char          csFileName[128];
	unsigned int   FileType;
}avi_list_t;

int g_file_count;

extern avi_list_t *list_avi = NULL;

SMsgAVIoctrlPlayRecord *play_record;

void tk_Set_Playback_rate_req(int avIndex, char *buf)
{
    SMsgAVIoctrlPlaybackrateConfigResp resp;
    SMsgAVIoctrlSetPlayBackRateConfigReq *p =(SMsgAVIoctrlPlaybackrateConfigResp *)buf;
	
    info("set rate is %lf\n",p->rate);
	
    resp.result = 1;
    avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_PLAYBACK_RATE_RESP, (char *)&resp, sizeof(SMsgAVIoctrlPlaybackrateConfigResp));
}

void *thread_PlayBack(void *arg)
{
	int num = 1;
	int SID = *((int *)arg);
	free(arg);
	AV_Client *p;
	int avIndex;
	char filename[GRD_MAX_PATH_LEN];
	int ret;
	int playHandler;
	unsigned char frame_buf[ONE_FRAME_BUF_SIZE];
	int buf_size = ONE_FRAME_BUF_SIZE;
	GRD_NET_FRAME_HEADER pHeader;
	FRAMEINFO_t video_frameInfo;


	SMsgAVIoctrlPlayRecordResp resp;
	memset(&resp, 0, sizeof(SMsgAVIoctrlPlayRecordResp));
	
	memset(&video_frameInfo, 0, sizeof(FRAMEINFO_t));

	p = &gClientInfo[SID];

	/*
	  * Get tutk user name and pwd   2015-07-08
	  */
	char msg_buf[MSG_BUF_SIZE] = {0};
	sdk_user_right_t * user_msg_cfg;
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
	TUTK_MSG_CTRL(SDK_MAIN_MSG_USER,SDK_USER_QUERY,0,0,pMsg);
	if(NULL == pMsg)
    {
        goto __error;
    }
	user_msg_cfg = (sdk_user_right_t *)pMsg->data;

	strcpy(g_tutk_cfg.ViewAccount, user_msg_cfg->user.user_name);
	strcpy(g_tutk_cfg.ViewPassword, user_msg_cfg->user.user_pwd);


#if _MSG_DEBUG
		info("count :%s , pwd: %s\n", user_msg_cfg->user.user_name, user_msg_cfg->user.user_pwd);
		info("ViewAccount:[%s] ViewPassword:%s\n",\
			g_tutk_cfg.ViewAccount, g_tutk_cfg.ViewPassword);
#endif

	avIndex = avServStart(SID, g_tutk_cfg.ViewAccount, g_tutk_cfg.ViewPassword, 3, SERVTYPE_STREAM_SERVER, p->playBackCh);

	if(avIndex < 0)
	{
		printf("avServStart failed SID[%d] code[%d]!!!\n", SID, avIndex);
		pthread_exit(0);
	}
	printf("thread_PlayBack start OK[%d]\n", avIndex);

	/*
	  *	search play back by p->playRecord.stTimeDay
	  */

	int  i = playBack_index;
    playBack_index = -1;
	
    if(pAvEvent[i].playback != 0x01)
    {
		error("play back Error!\n");
	
		resp.command = AVIOCTRL_RECORD_PLAY_END;
		avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP, (char *)&resp, sizeof(SMsgAVIoctrlPlayRecordResp));
		usleep(100);
		avServStop(avIndex);
		return NULL;

    }				
	
		strcpy(filename, pAvEvent[i].file_name);
#if _MSG_DEBUG
		info("playbackfile[%d] = %s \n",i,pAvEvent[i].file_name);
#endif

	
		GRD_PLAYBACK_REQ rec_param;
		strncpy(rec_param.csFileName,filename,GRD_MAX_PATH_LEN);

		st_playback_by_name(&rec_param, &playHandler); //Find Avi Header and index file
		
		while(1)
		{
			if(p->bStopPlayBack == 1)
        	{
           		printf("bStopPlayBack bStopPlayBack\n");
            	goto END;
        	}
        	if(gClientInfo[SID].bPausePlayBack)
       		{
            	usleep(50000);
            	continue;
        	}
			ret = st_playback_get_frame_AorV(1,playHandler,frame_buf,&buf_size,&pHeader); //get data blocks
			g_videostop = playHandler;
			num ++;
			//Fixed  Date:2015-07-02
			if(ret == -1)	
			{
				info("====>get  frame Error !! \n");
				continue;				
			
			}
			else if(1 == ret)
			{
				info("====>play back Ending \n");
				goto END;
			}
			else
			{

				video_frameInfo.codec_id = MEDIA_CODEC_VIDEO_H264;
		        video_frameInfo.timestamp = 0;//getTimeStamp();
		        video_frameInfo.onlineNum = gOnlineNum;

		        if (pHeader.byFrameType == 0)
		        {
		            video_frameInfo.flags = IPC_FRAME_FLAG_IFRAME;
		        }
		        else
		        {
		            video_frameInfo.flags = IPC_FRAME_FLAG_PBFRAME;

		        }

				ret = avSendFrameData(avIndex, frame_buf, buf_size, &video_frameInfo, sizeof(FRAMEINFO_t));
				if(ret == AV_ER_NoERROR)
	            {
					// Send successfully
	            }
	            else if(ret == AV_ER_EXCEED_MAX_SIZE) // means data not write to queue, send too slow, I want to skip it
	            {
	                error("AV_ER_EXCEED_MAX_SIZE[%d]\n", avIndex);
	            }
	            else if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE)
	            {
	                error("thread_VideoFrameData AV_ER_SESSION_CLOSE_BY_REMOTE\n");
	                break;
	            }
	            else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT)
	            {
	                error("thread_VideoFrameData AV_ER_REMOTE_TIMEOUT_DISCONNECT\n");
	                break;

	            }
	            else if(ret == IOTC_ER_INVALID_SID)
	            {
	                error("Session cant be used anymore\n");
	                break;
	            }
	            else if(ret < 0)
	            {
	                error("avSendFrameData error[%d] idx[%d]\n", ret, avIndex);
	            }
				usleep(1000*1000/30);
	
			}
		}
	
				
	
	END:
	st_playback_stop(playHandler);

    info("line:%d------------------avIndex:%d\n", __LINE__, avIndex);
    avServStop(avIndex);
    //p->playIndex  = -1;
    
    resp.command = AVIOCTRL_RECORD_PLAY_END;
    ret = avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP, (char *)&resp, sizeof(SMsgAVIoctrlPlayRecordResp));
    if(ret ==AV_ER_NoERROR)
        printf("thread_PlayBack exit\n");
    info("stop avindex [%d]\n",avIndex);
	
	pthread_rwlock_wrlock(&gClientInfo[SID].sLock);
    gClientInfo[SID].bStopPlayBack = 1;
    gClientInfo[SID].bPausePlayBack = 0;
    gClientInfo[SID].playBackCh = -1;
	pthread_rwlock_unlock(&gClientInfo[SID].sLock);
   
    info("thread_PlayBack exit\n");
	pthread_exit(0);
	return NULL;
__error:
	pthread_exit(0);
	return NULL;

}

int get_avi_list(SMsgAVIoctrlListEventReq *filereq , int *num)
{
	sdk_record_cond_t param;
	int FindHandle;
	GRD_REC_FILE_INFO file_info;
	int i = 0;
	int ret;
	memset(&param, 0, sizeof(sdk_record_cond_t));
	param.start_time.year =(DWORD) filereq->stStartTime.year;
	param.start_time.mon= (DWORD)filereq->stStartTime.month;
	param.start_time.day=(DWORD) filereq->stStartTime.day;
	param.start_time.hour= (DWORD)filereq->stStartTime.hour;
	param.start_time.min= (DWORD)filereq->stStartTime.minute;
	param.start_time.sec= (DWORD)filereq->stStartTime.second;

	param.stop_time.year = (DWORD)filereq->stEndTime.year;
	param.stop_time.mon = (DWORD)filereq->stEndTime.month;
	param.stop_time.day = (DWORD)filereq->stEndTime.day;
	param.stop_time.hour =(DWORD) filereq->stEndTime.hour;
	param.stop_time.min =(DWORD) filereq->stEndTime.minute;
	param.stop_time.sec = (DWORD)filereq->stEndTime.second;
	
	param.record_type = SDK_RECORD_TYPE_ALL;
	param.channel = 0 ;
	
	
	char tmp[32] = {0}, tmp1[32] = {0};
	sprintf(tmp, "%04u%02u%02u%02u%02u%02u", param.start_time.year, param.start_time.mon, param.start_time.day,
											param.start_time.hour, param.start_time.min, param.start_time.sec);

	sprintf(tmp1, "%04u%02u%02u%02u%02u%02u", param.stop_time.year, param.stop_time.mon, param.stop_time.day,
											param.stop_time.hour, param.stop_time.min, param.stop_time.sec);

	uint64_t start = atoll(tmp);
	uint64_t stop  = atoll(tmp1);
	
		
	if(list_avi)
	{
		printf("list_avi is not null!\n");
		free(list_avi);
	}
	printf("list_avi is null!\n");
	int avi_num = st_find_file_open(&param, &FindHandle);
	if( 0 >= avi_num )
		return -1;

	info("find avi_num==>:%d\n", avi_num);
	
	avi_list_t *li_avi = (avi_list_t *)malloc(sizeof(avi_list_t)*avi_num);
	if(li_avi == NULL)
	{
		printf("li_avi is malloc err!\n");
		return -1;
	}
	list_avi = li_avi;
	
	while(1)
	{
		if(st_find_next_file(FindHandle,&file_info))
		{
			printf("find file end!\n");
			break;
		}
		else
		{
#if _MSG_DEBUG

			printf("file name:%s\n",file_info.csFileName);
			info("file count: [%d]\n",i);
#endif
			if(strstr(file_info.csFileName,".avi")==NULL)
			{
				continue;
			}
			
			
			(list_avi+i)->stStartTime.year = file_info.stStartTime.year ;
			(list_avi+i)->stStartTime.mon= file_info.stStartTime.mon ;
			(list_avi+i)->stStartTime.day = file_info.stStartTime.day;
			(list_avi+i)->stStartTime.hour= file_info.stStartTime.hour;
			(list_avi+i)->stStartTime.min = file_info.stStartTime.min;
			(list_avi+i)->stStartTime.sec = file_info.stStartTime.sec;
				
			if(file_info.stStopTime.year > 0)
			{
#if _MSG_DEBUG
				printf("file_info.stStartTime.year=%d\n",file_info.stStartTime.year);
				printf("file_info.stStartTime.mon=%d\n",file_info.stStartTime.mon);
				printf("file_info.stStartTime.day=%d\n",file_info.stStartTime.day);
				printf("file_info.stStartTime.hour=%d\n",file_info.stStartTime.hour);
				printf("file_info.stStartTime.min=%d\n",file_info.stStartTime.min);
				printf("file_info.stStartTime.sec=%d\n\n\n",file_info.stStartTime.sec);
				
				
				printf("file_info.stStopTime.year=%d\n",file_info.stStopTime.year);
				printf("file_info.stStopTime.mon=%d\n",file_info.stStopTime.mon);
				printf("file_info.stStopTime.day=%d\n",file_info.stStopTime.day);
				printf("file_info.stStopTime.hour=%d\n",file_info.stStopTime.hour);
				printf("file_info.stStopTime.min=%d\n",file_info.stStopTime.min);
				printf("file_info.stStopTime.sec=%d\n",file_info.stStopTime.sec);
#endif
				
				strncpy((list_avi+i)->csFileName,file_info.csFileName,128);
				i++;
				if(i >= avi_num)
					break;
			}
		}
		
	}
	
	st_find_file_close(FindHandle);
	*num = i;
	
	return 0;
}

void get_listEvent_req_handle(int avIndex, char *buf)
{

	SMsgAVIoctrlListEventReq	*p = (SMsgAVIoctrlListEventReq *)buf;
	
	int file_count;
	void *pRecList[8] = {NULL};
	int counter = 0;
	int for_count = 0;
	int ret;
	
	ret = get_avi_list(buf,&file_count);
		
	if(ret < 0 )
	{
		printf("get_avi_list is error!\n");
	}
	if(file_count == 0 || file_count > 1000000000)//判断获取到的文件个数
	{
		error("Found no file!!\n");
		return ;
	}
	
	int leftNum = file_count;
	g_file_count = file_count;
	
	int i = 0 , j = 0 , k = 0;
	if(file_count > 0)
    {
        info("file_num = %d \n",file_count);

        //每次最多1024个字节
        //计算每次最大发送多少个数据包
        int max_file_num = (1024 - sizeof(SMsgAVIoctrlListEventResp))/sizeof(SAvEvent) - 1;
		
        SMsgAVIoctrlListEventResp *pFilelist;
        pFilelist = (SMsgAVIoctrlListEventResp *)malloc(sizeof(SMsgAVIoctrlListEventResp)+sizeof(SAvEvent)*max_file_num);

		pFilelist->total = file_count;
		
        counter = (file_count + max_file_num)/max_file_num; //总包数
        		
        for(i = 0 ; i < counter ; i ++)
        {
            for_count = (leftNum > max_file_num)?max_file_num:leftNum;

	 		info("Max_file_num = %d \n",max_file_num);
			info("Counter 	  = %d \n",counter);
            for( j = 0 ; j < for_count ; j++)
            {
               
				k = i*for_count + j;

			
				pFilelist->stEvent[j].stTime.year = (list_avi+k)->stStartTime.year ;
				pFilelist->stEvent[j].stTime.month = (list_avi+k)->stStartTime.mon;
				pFilelist->stEvent[j].stTime.day = (list_avi+k)->stStartTime.day;
				pFilelist->stEvent[j].stTime.hour = (list_avi+k)->stStartTime.hour;
				pFilelist->stEvent[j].stTime.minute = (list_avi+k)->stStartTime.min;
				pFilelist->stEvent[j].stTime.second = (list_avi+k)->stStartTime.sec;
			
									
				pFilelist->stEvent[j].event =0x0;

				/*
				  * Access to video duration   2015-07-13
				  */
				//int all_time = cal_pb_all_time_by_file_path((list_avi+k)->csFileName);
				//warning("the file duration: %d\n", all_time);
            }

            
            pFilelist->index = i;
            pFilelist->count = for_count;
            leftNum -= for_count;
            pFilelist->endflag = (leftNum == 0) ? 1:0;
#if 0
            info("testleftNum:%d  total:%d==index:%d===count:%d=====endflag:%d \n"
                   ,leftNum
                   ,pFilelist->total
                   ,pFilelist->index
                   ,pFilelist->count
                   ,pFilelist->endflag);
#endif
            ret = avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_LISTEVENT_RESP, (char *)pFilelist, sizeof(SMsgAVIoctrlListEventResp)+sizeof(SAvEvent)*pFilelist->count);
            if(ret < 0)
            {
                printf("send error IOTYPE_USER_IPCAM_LISTEVENT_RESP :%d \n",ret);
            }
            if(leftNum == 0) break;
        }
        //info(" =====================leftNum :%d \n",leftNum);
        free(pFilelist);

    }
    else
    {
        SMsgAVIoctrlListEventResp Filelist;
        Filelist.total = 0;
        Filelist.endflag = 1;
        ret = avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_LISTEVENT_RESP, (char *)&Filelist, sizeof(SMsgAVIoctrlListEventResp));
    }
    return ;
	
}

void DE_get_listEvent_req_handle(int avIndex, char *buf)
{

	SMsgAVIoctrlListEventReq	*p = (SMsgAVIoctrlListEventReq *)buf;
	
	int file_count;
	void *pRecList[8] = {NULL};
	int counter = 0;
	int for_count = 0;
	int ret;
	
	ret = get_avi_list(buf,&file_count);
		
	if(ret < 0 )
	{
		printf("get_avi_list is error!\n");
	}
	if(file_count == 0 || file_count > 1000000000)//判断获取到的文件个数
	{
		error("Found no file!!\n");
		return ;
	}
	
	int leftNum = file_count;
	g_file_count = file_count;
	
	int i = 0 , j = 0 , k = 0;
	if(file_count > 0)
    {
        info("file_num = %d \n",file_count);

        //每次最多1024个字节
        //计算每次最大发送多少个数据包
        int max_file_num = (1024 - sizeof(SMsgAVIoctrlListEventResp))/sizeof(SAvEvent) - 1;
		
        SMsgAVIoctrlListEventResp *pFilelist;
        pFilelist = (SMsgAVIoctrlListEventResp *)malloc(sizeof(SMsgAVIoctrlListEventResp)+sizeof(SAvEvent)*max_file_num);

		pFilelist->total = file_count;
		
        counter = (file_count + max_file_num)/max_file_num; //总包数
        		
        for(i = 0 ; i < counter ; i ++)
        {
            for_count = (leftNum > max_file_num)?max_file_num:leftNum;

	 		info("Max_file_num = %d \n",max_file_num);
			info("Counter 	  = %d \n",counter);
            for( j = 0 ; j < for_count ; j++)
            {
               
				k = i*for_count + j;

			
				pFilelist->stEvent[j].stTime.year = (list_avi+k)->stStartTime.year ;
				pFilelist->stEvent[j].stTime.month = (list_avi+k)->stStartTime.mon;
				pFilelist->stEvent[j].stTime.day = (list_avi+k)->stStartTime.day;
				pFilelist->stEvent[j].stTime.hour = (list_avi+k)->stStartTime.hour;
				pFilelist->stEvent[j].stTime.minute = (list_avi+k)->stStartTime.min;
				pFilelist->stEvent[j].stTime.second = (list_avi+k)->stStartTime.sec;
			
									
				pFilelist->stEvent[j].event =0x0;

				/*
				  * Access to video duration   2015-07-13
				  */
				//int all_time = cal_pb_all_time_by_file_path((list_avi+k)->csFileName);
				//warning("the file duration: %d\n", all_time);
            }

            
            pFilelist->index = i;
            pFilelist->count = for_count;
            leftNum -= for_count;
            pFilelist->endflag = (leftNum == 0) ? 1:0;
			ret = avSendDefaultIOCtrl(avIndex, IOTYPE_USER_IPCAM_LISTEVENT_RESP, (char *)pFilelist, sizeof(SMsgAVIoctrlListEventResp)+sizeof(SAvEvent)*pFilelist->count);
			if(ret < 0)
            {
                printf("send error IOTYPE_USER_IPCAM_LISTEVENT_RESP :%d \n",ret);
            }
            if(leftNum == 0) break;
        }
        free(pFilelist);

    }
    else
    {
        SMsgAVIoctrlListEventResp Filelist;
        Filelist.total = 0;
        Filelist.endflag = 1;
        ret = avSendDefaultIOCtrl(avIndex, IOTYPE_USER_IPCAM_LISTEVENT_RESP, (char *)&Filelist, sizeof(SMsgAVIoctrlListEventResp));
    }
    return ;
	
}
	
void *tutk_to_update(void *pvar)
{

    return NULL;
}


void tutk_start_updatedeal(int avindex)
{
    int index = avindex;
    pthread_t threadc;
    pthread_attr_t	attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&threadc, &attr, &tutk_to_update, &index);
}

void app_upgrade_req_handle(int avIndex, char *buf)
{
    SMsgAVIoctrlUpgradeSystemReq *p = (SMsgAVIoctrlUpgradeSystemReq *)buf;
    SMsgAVIoctrlUpgradeSystemResp resp;
    int percent = 0;
    //free_driver_pages();

    if(!g_upgrade_fp)
    {
        ///system("rm -rf /tmp/app.csf");
        if (!(g_upgrade_fp=fopen(UPGRADE_TMP_FILE_PATH,"w"))) // open file pointer
        {
            printf("::%s open failed##########################################\n",UPGRADE_TMP_FILE_PATH);
            resp.result = -1;
            tutk_updateing = -1;
            upgrade_size_add = 0;
            upgrade_pack_index = 0;
            avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_UPGRADE_SYSTEM_RESP, (char *)&resp, sizeof(SMsgAVIoctrlUpgradeSystemResp));
            fflush(g_upgrade_fp);
            fclose(g_upgrade_fp);
            g_upgrade_fp = NULL;
            return ;
        }
    }

    if((p->index != 0) && (p->index != (upgrade_pack_index + 1)))
    {
        resp.result = -1;
        tutk_updateing = -1;
        upgrade_size_add = 0;
        upgrade_pack_index = 0;
        avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_UPGRADE_SYSTEM_RESP, (char *)&resp, sizeof(SMsgAVIoctrlUpgradeSystemResp));
        fflush(g_upgrade_fp);
        fclose(g_upgrade_fp);
        g_upgrade_fp = NULL;
        return ;
    }
    else if(p->index == 0)
    {
        tutk_updateing = 1;
        resp.result = 0;
        avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_UPGRADE_SYSTEM_RESP, (char *)&resp, sizeof(SMsgAVIoctrlUpgradeSystemResp));

    }
#if 1
    else if((p->index)%100 == 0)
    {

        resp.result = p->index;
        avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_UPGRADE_SYSTEM_RESP, (char *)&resp, sizeof(SMsgAVIoctrlUpgradeSystemResp));
    }
#endif
    fwrite(p->szBuf, p->size, 1, g_upgrade_fp);

    upgrade_size_add += p->size;
    upgrade_pack_index = p->index;
    printf("::write len=%d index[%d]######################################\n",p->size, p->index);

    if(p->endflag && upgrade_size_add == p->total)
    {
        fflush(g_upgrade_fp);
        fclose(g_upgrade_fp);
        g_upgrade_fp = NULL;
        upgrade_pack_index = 0;
        g_upgrade_percent =  resp.result;
        tutk_start_updatedeal(avIndex); //开始升级
    }
}
int  list_alarmall_log(SMsgAVIoctrlAlarmListResp *listAlarm, unsigned int max_alarm_num, unsigned int nStartTime, unsigned int nEndTime)
{
    int 			index=0;
    char 			*ch;
    char 			seps[]="=";
    char 			*token,*time, *opt_type;
    int 			alarm_type = 0;
    unsigned int 	timeVal = 0;
    char 		pattern[512]= {0},dec_pattern[512];
    FILE 		*conf_fp;

    int			i = 0;
    pthread_mutex_lock(&g_log_lock);
    for(i = 0; i < 2; i++)
    {
        if(i == 0)
        {
            if (!(conf_fp=fopen(ALARM_PATH1,"r"))) // open file pointer
            {
                printf("/mnt/mtd/alarmfile1 open failed...\n");
                continue ;
            }
        }
        else
        {
            if (!(conf_fp=fopen(ALARM_PATH,"r"))) // open file pointer
            {
                printf("/mnt/mtd/alarmfile open failed...\n");
                goto end;
            }
        }

        fseek( conf_fp, 0, SEEK_SET );

        while(feof(conf_fp) == 0&&feof(conf_fp)!= 255)
        {
            ch = fgets(pattern,512,conf_fp);
            if(ch == NULL)
            {
                ch = fgets(pattern,512,conf_fp);
                if(ch == NULL)
                    break;
            }
            token = strtok(pattern,"\n");
            strcpy(dec_pattern,token);

            time = strtok(dec_pattern,seps);
            opt_type = strtok(NULL,seps);
            if(time==NULL||opt_type==NULL)
                continue;
            timeVal = atoi(time);
            alarm_type = atoi(opt_type);
            //printf("timeVal[%d] alarm_type[%d] \n", timeVal, alarm_type);

            if(timeVal >= nStartTime && timeVal <= nEndTime && index < max_alarm_num)
            {
                listAlarm->stalarm[index].time = timeVal;
                listAlarm->stalarm[index].event_type = alarm_type;
                index++;
            }
        }
        fflush(conf_fp);
        fclose(conf_fp);
    }
end:
    pthread_mutex_unlock(&g_log_lock);
    if(index == 0)
    {
        printf("get alarm list count=%d======================\n",index);
        return 1;
    }
    else
    {
        listAlarm->total = index;
        printf("get alarm list count=%d======================\n",listAlarm->total);
        return 1;
    }

}
int get_alarmlog_number()
{
    FILE 	*conf_fp;
    int 	index = 0;
    char 	*ch;
    char 	pattern[512] = {0};
    int		i = 0;
    pthread_mutex_lock(&g_log_lock);
    for(i = 0; i < 2; i++)
    {
        if(i == 0)
        {
            if (!(conf_fp=fopen(ALARM_PATH1,"r"))) // open file pointer
            {
                printf("/mnt/mtd/alarmfile1 open failed...\n");
                continue ;
            }
        }
        else
        {
            if (!(conf_fp=fopen(ALARM_PATH,"r"))) // open file pointer
            {
                printf("/mnt/mtd/alarmfile open failed...\n");
                pthread_mutex_unlock(&g_log_lock);
                return index;
            }
        }

        fseek( conf_fp, 0, SEEK_SET );
        while(feof(conf_fp) == 0&&feof(conf_fp)!= 255)
        {

            ch = fgets(pattern,512,conf_fp);
            if(ch)
            {
                index++;
            }
        }
        fflush(conf_fp);
        fclose(conf_fp);
    }

    printf("get alarm number:%d==========================\n",index);
    pthread_mutex_unlock(&g_log_lock);
    return index;
}
void get_alarmList_req_handle(int avIndex, char *buf)
{
   unsigned int 				max_list_num = 0;
    int 						onePackAlarmNum = 0;
    int							alarmSendNum = 0;
    int							index = 0;
    int							i = 0;
    SMsgAVIoctrlAlarmListResp 	*resp;
    SMsgAVIoctrlAlarmListResp 	*tmp;
    SMsgAVIoctrlGetAlarmListReq *p = (SMsgAVIoctrlGetAlarmListReq *)buf;

    int headSize = sizeof(SMsgAVIoctrlAlarmListResp);

    printf("headSize[%d] \n",headSize);

    onePackAlarmNum = (1024 - headSize)/sizeof(alarm_log) -1;
    max_list_num = get_alarmlog_number();

    printf("max_list_num[%d]\n", max_list_num);
    resp = (SMsgAVIoctrlAlarmListResp *)malloc(headSize + onePackAlarmNum*sizeof(alarm_log) + 8);
    if(!resp)
    {
        printf("malloc resp failed!\n");
        return ;
    }

    tmp = (SMsgAVIoctrlAlarmListResp *)malloc(headSize + max_list_num*sizeof(alarm_log) + 8);
    if(!tmp)
    {
        printf("malloc tmp failed!\n");
        return ;
    }
    memset(resp, 0, headSize + onePackAlarmNum*sizeof(alarm_log) + 8);
    memset(tmp, 0, headSize + max_list_num*sizeof(alarm_log) + 8);
    list_alarmall_log(tmp, max_list_num, p->nStartTime, p->nEndTime);

    alarmSendNum = tmp->total;

    if(alarmSendNum == 0)
    {
        resp->total = 0;
        resp->endflag = 1;
        resp->index = 0;
        resp->count = 0;
        avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_ALARM_LIST_RESP, (char *)resp, headSize);
    }

    while(alarmSendNum > 0)
    {
        memset(resp, 0, headSize + onePackAlarmNum*sizeof(alarm_log) + 8);
        resp->total = tmp->total;
        resp->index = index;
        if(alarmSendNum > onePackAlarmNum)
        {
            resp->endflag = 0;
            resp->count = onePackAlarmNum;
            alarmSendNum -= onePackAlarmNum;
        }
        else
        {
            resp->endflag = 1;
            resp->count = alarmSendNum;
            alarmSendNum -= alarmSendNum;
        }

        for(i = 0; i < resp->count; i++)
        {
            resp->stalarm[i].time = tmp->stalarm[index * onePackAlarmNum + i].time;
            resp->stalarm[i].event_type = tmp->stalarm[index * onePackAlarmNum + i].event_type;
            //printf("resp->stalarm[%d] time[%d] event_type[%d]\n", i, resp->stalarm[i].time, resp->stalarm[i].event_type);
        }

        printf("total[%d] index[%d] endflag[%d] count[%d] size[%d]\n", resp->total, resp->index, resp->endflag, resp->count,headSize + resp->count*sizeof(alarm_log));

        int ret = avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_ALARM_LIST_RESP, (char *)resp, headSize + resp->count*sizeof(alarm_log));
        printf("ret = %d   \n", ret);
        index++;
    }

    if(resp)
    {
        free(resp);
    }

    if(tmp)
    {
        free(tmp);
    }

}
static int count = 0;
extern void tk_get_alarmArg_req_handle_EX(int avIndex,char *buf);
void Handle_IOCTRL_Cmd(int SID, int avIndex, char *buf, int type)
{
    printf("sid(%d)avIndex(%d)Handle_IOCTRL_Cmd(0x%X)-- start \n ", SID,avIndex,type);
    switch(type)
    {
    case IOTYPE_USER_IPCAM_START:
    {
        SMsgAVIoctrlAVStream *p = (SMsgAVIoctrlAVStream *)buf;
        printf("==========IOTYPE_USER_IPCAM_START[%d:%d]==================\n", p->channel, avIndex);
        regedit_client_to_video(SID, avIndex);//这里启动了视屏传输
        count ++;
		//error("+++++++++++++++++++++++++OPEN VIDEO ++ %d+++++++++++++++++++++++++++\n",count);
		//非报警拉流才置标志位
		if(( 0 == ((led_buzzer_info.led_status & (1<<LED_ALARM_TRIGGER)) >> LED_ALARM_TRIGGER))
				&&( 0 == ((led_buzzer_info.led_status & (1<<LED_ALARM_MONITOR)) >> LED_ALARM_MONITOR)))
		{
			printf(" no in LED_ALARM_TRIGGER\n");
			pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
			led_buzzer_info.led_status |= 1<< LED_MONITOR;
			led_buzzer_info.led_status |= 1<< LED_ALARM_MONITOR;
			pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);
		}
		///////////////
        printf("regedit_client_to_video OK\n");
    }
    break;
    case IOTYPE_USER_IPCAM_STOP:
    {
        SMsgAVIoctrlAVStream *p = (SMsgAVIoctrlAVStream *)buf;
        printf("================IOTYPE_USER_IPCAM_STOP[%d:%d]=======================\n", p->channel, avIndex);
        unregedit_client_from_video(SID);
		
		 count --;
		//warning("+++++++++++++++++++++++++CLOSE VIDEO -- %d+++++++++++++++++++++++++++\n",count);
		if(1 == ((led_buzzer_info.led_status & (1<<LED_MONITOR)) >>LED_MONITOR))
		{
			pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
			led_buzzer_info.led_status &= ~(1<< LED_MONITOR);
			led_buzzer_info.led_status &= ~(1<< LED_ALARM_MONITOR);
			pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);
		}
        printf("unregedit_client_from_video OK\n");
    }
    break;

    case IOTYPE_USER_IPCAM_AUDIOSTART:
    {
        SMsgAVIoctrlAVStream *p = (SMsgAVIoctrlAVStream *)buf;
        printf("IOTYPE_USER_IPCAM_AUDIOSTART[%d:%d]\n", p->channel, avIndex);
        regedit_client_to_audio(SID, avIndex);
        printf("regedit_client_to_audio OK\n");
    }
    break;
    case IOTYPE_USER_IPCAM_AUDIOSTOP:
    {
        SMsgAVIoctrlAVStream *p = (SMsgAVIoctrlAVStream *)buf;
        printf("IOTYPE_USER_IPCAM_AUDIOSTOP[%d:%d]\n", p->channel, avIndex);
        unregedit_client_from_audio(SID);
        printf("unregedit_client_from_audio OK\n");
    }
    break;


    case IOTYPE_USER_IPCAM_SPEAKERSTART:
    {
        SMsgAVIoctrlAVStream *p = (SMsgAVIoctrlAVStream *)buf;
        printf("IOTYPE_USER_IPCAM_SPEAKERSTART[%d:%d]\n", p->channel, avIndex);
        if(g_tutkSpeaker == 0 && gClientInfo[SID].bEnableSpeaker == 0)
        {
            gClientInfo[SID].speakerCh = (unsigned char)p->channel;
            gClientInfo[SID].bEnableSpeaker = 1;

			//打开spk使能
			adapt_speaker_ctrl(1);
            //// use which channel decided by client
            creat_recv_audio_func(SID);
        }
    break;
    }
    case IOTYPE_USER_IPCAM_SPEAKERSTOP:
    {
        printf("IOTYPE_USER_IPCAM_SPEAKERSTOP\n");
        gClientInfo[SID].bEnableSpeaker = 0;

        g_tutkSpeaker = 0;
		//disable spk
		adapt_speaker_ctrl(0);
    }
    break;
    case IOTYPE_USER_IPCAM_LISTEVENT_REQ:			//获取录像事件列表
    {
        get_listEvent_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL:		//录像回放 重点部分
    {
        SMsgAVIoctrlPlayRecord *p = (SMsgAVIoctrlPlayRecord *)buf;

        SMsgAVIoctrlPlayRecordResp res;
		play_record = (SMsgAVIoctrlPlayRecord *)buf;
		int i = 0;



	    if(p->command == AVIOCTRL_RECORD_PLAY_START)
        {
#if _MSG_DEBUG
		warning("play back command ==>AVIOCTRL_RECORD_PLAY_START\n");
#endif
            memcpy(&gClientInfo[SID].playRecord, p, sizeof(SMsgAVIoctrlPlayRecord));

            pthread_rwlock_wrlock(&gClientInfo[SID].sLock);
			
            gClientInfo[SID].bPausePlayBack = 0;
            gClientInfo[SID].bStopPlayBack = 0;
            gClientInfo[SID].playBackCh = IOTC_Session_Get_Free_Channel(SID);
            res.command = AVIOCTRL_RECORD_PLAY_START;
            res.result = gClientInfo[SID].playBackCh;

            pthread_rwlock_unlock(&gClientInfo[SID].sLock);
     
#if _MSG_DEBUG
			
			info("IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL ==> cmd[0x%x]\n", p->command);
			info("Time -Type:%d-%d-%d %d:%d:%d\n",p->stTimeDay.year, p->stTimeDay.month,\
			p->stTimeDay.day, p->stTimeDay.hour, p->stTimeDay.minute, p->stTimeDay.second);
#endif
			if( res.result >= 0 )
			{
				printf("%d start playback\n", res.result);

				for(i = 0; i < g_file_count; i++)
				{
					if(p->stTimeDay.year == (list_avi+i)->stStartTime.year && \
						p->stTimeDay.month == (list_avi+i)->stStartTime.mon && \
						p->stTimeDay.day == (list_avi+i)->stStartTime.day && \
						p->stTimeDay.hour == (list_avi+i)->stStartTime.hour && \
						p->stTimeDay.minute == (list_avi+i)->stStartTime.min &&\
						p->stTimeDay.second ==(list_avi+i)->stStartTime.sec)
						{
#if _MSG_DEBUG
							info("=======EVENT========\n");
							info("===File Name :%s==\n",(list_avi+i)->csFileName);
#endif
							pAvEvent[i].playback= 0x01;
							playBack_index = i;
							strcpy(pAvEvent[i].file_name, (list_avi+i)->csFileName);
						}
						

				}
				
	
				int *sid = (int *)malloc(sizeof(int));
				*sid = SID;
				pthread_t ThreadID;
				int ret;
				if((ret = pthread_create(&ThreadID, NULL, &thread_PlayBack, (void *)sid)))
				{
					printf("pthread_create ret=%d\n", ret);
					exit(-1);
				}
				pthread_detach(ThreadID);
			}
			else
			{
				printf("Playback on SID %d is still functioning\n", SID );
			}
            if(avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP, (char *)&res, sizeof(SMsgAVIoctrlPlayRecordResp)) < 0)
                break;

        }
        else if(p->command == AVIOCTRL_RECORD_PLAY_PAUSE)
        {
#if _MSG_DEBUG
		warning("play back command ==>AVIOCTRL_RECORD_PLAY_PAUSE\n");
#endif
            res.command = AVIOCTRL_RECORD_PLAY_PAUSE;
            res.result = 0;
            if(avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP, (char *)&res, sizeof(SMsgAVIoctrlPlayRecordResp)) < 0)
                break;
            //get writer lock
            int lock_ret = pthread_rwlock_wrlock(&gClientInfo[SID].sLock);
            if(lock_ret)
                printf("Acquire SID %d rwlock error, ret = %d\n", SID, lock_ret);

			gClientInfo[SID].bPausePlayBack = !gClientInfo[SID].bPausePlayBack;
            //release lock
            lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
            if(lock_ret)
                printf("Release SID %d rwlock error, ret = %d\n", SID, lock_ret);
        }
        else if(p->command == AVIOCTRL_RECORD_PLAY_STOP)
        {
#if _MSG_DEBUG
		warning("play back command ==>AVIOCTRL_RECORD_PLAY_STOP\n");
#endif
            //get writer lock
            int lock_ret = pthread_rwlock_wrlock(&gClientInfo[SID].sLock);
            if(lock_ret)
                printf("Acquire SID %d rwlock error, ret = %d\n", SID, lock_ret);
            gClientInfo[SID].bStopPlayBack = 1;
            //release lock
            lock_ret = pthread_rwlock_unlock(&gClientInfo[SID].sLock);
            if(lock_ret)
                printf("Release SID %d rwlock error, ret = %d\n", SID, lock_ret);
        }

    }
    break;
    case IOTYPE_USER_IPCAM_PTZ_COMMAND:
    {
    }
    break;
   case IOTYPE_USER_IPCAM_GETSTREAMCTRL_REQ:
    {
		tk_get_streamctrl(avIndex, buf);
	}
    break;
    case IOTYPE_USER_IPCAM_SETSTREAMCTRL_REQ:
    {
		tk_set_streamctrl(avIndex, buf);
    }
	break;
    case IOTYPE_USER_IPCAM_GET_VIDEOMODE_REQ:
    {
    }
    break;
    case IOTYPE_USER_IPCAM_SET_VIDEOMODE_REQ:
    {
    }
    break;
    case IOTYPE_USER_IPCAM_GET_ENVIRONMENT_REQ:
    {
    }
    break;
    case IOTYPE_USER_IPCAM_SET_ENVIRONMENT_REQ:
    {
    }
    break;
    case IOTYPE_USER_IPCAM_DEVINFO_REQ:
    {
		TK_get_dev_info_req_handle(avIndex,buf);
    }
    break;
    case IOTYPE_USER_IPCAM_GETMOTIONDETECT_REQ:
    {
		tk_get_MDSensitive_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_SETMOTIONDETECT_REQ:
    {
		tk_set_MDSensitive_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_GETRECORD_REQ:
    {
		tk_get_record_req_handle(avIndex, buf);
	#if 0
        SMsgAVIoctrlGetRecordResq resp;
        resp.channel = gClientInfo[SID].avIndex;
        resp.recordType = AVIOTC_RECORDTYPE_OFF;
        avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETRECORD_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetRecordResq));
	#endif
	}
    break;
	case IOTYPE_USER_IPCAM_GET_MOTION_AREA_REQ:
	{
		tk_get_MDParam_req_handle(avIndex, buf);
	}
	break;

	case IOTYPE_USER_IPCAM_SET_MOTION_AREA_REQ:
	{
		tk_set_MDParam_req_handle(avIndex, buf);
	}
	break;

    case IOTYPE_USER_IPCAM_SETPASSWORD_REQ:
    {
		tk_set_userMsg_req_handle(avIndex, buf);
	}
    break;
    // dropbox support
    case IOTYPE_USER_IPCAM_GET_SAVE_DROPBOX_REQ:     // 0x500,
    case IOTYPE_USER_IPCAM_GET_SAVE_DROPBOX_RESP:    // 0x501,
    case IOTYPE_USER_IPCAM_SET_SAVE_DROPBOX_REQ:     // 0x502,
    case IOTYPE_USER_IPCAM_SET_SAVE_DROPBOX_RESP:    // 0x503,
    {
        break;
    }

    case IOTYPE_USER_IPCAM_LISTWIFIAP_REQ://获取wifilist
    {
        tk_list_wifi_req_handle(avIndex, buf);
    }
    break;

    case IOTYPE_USER_IPCAM_SETWIFI_REQ://设置wifi
    {
        TK_set_wifi_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_GETWIFI_REQ://获取wifi
    {
        TK_get_wifi_req_handle(avIndex, buf);
    }
    break;

    case IOTYPE_USER_IPCAM_FORMATEXTSTORAGE_REQ:// Format external storage
    {
        tk_format_extStorage_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_SETRECORD_REQ:// set record par
    {
        tk_set_record_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_SETRCD_DURATION_REQ:// set record duration
    {
       set_recordDuration_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_GETRCD_DURATION_REQ:// get record duration
    {
        get_recordDuration_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_REQ:
    {
         TK_get_audioOutFormat_req_handle(avIndex, buf);
    }
    break;

    case IOTYPE_USER_IPCAM_GETSUPPORTSTREAM_REQ:
    {
        //TK_get_supportStream_req_handle(avIndex, buf);
    }
    break;
    /*start CMS自扩展************************************************************/
#ifndef  _CAR_DV_
	#if 0
    case IOTYPE_USER_IPCAM_GET_ATTRIBUTE_REQ:  //0x40001
    {
        TK_get_attribute_req_handle(avIndex, buf);
    }
    break;
	#endif
#if 1
	case IOTYPE_USER_IPCAM_GET_ATTRIBUTE_REQ:  //0x40001
	{
		TK_get_attribute_req_handle(avIndex, buf);
	}
	break;

    case IOTYPE_USER_IPCAM_GET_VIDEO_DISPLAY_REQ://0x40003 //请求视频显示参数
    {
        TK_get_videoDisplay_req_handle(avIndex, buf);
    }
    break;

    case IOTYPE_USER_IPCAM_SET_VIDEO_DISPLAY_REQ://0x40005 //设置视频显示参数
    {
        TK_set_videoDisplay_req_handle(avIndex, buf);
    }
    break;

    case IOTYPE_USER_IPCAM_GET_VIDEO_QUALITY_REQ:  // 0x40023
    {
        TK_get_videoQuality_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_SET_VIDEO_QUALITY_REQ:///0x40025
    {
        TK_set_videoQuality_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_GET_ALARM_ARG_REQ://请求告警相关的参数
    {
        tk_get_alarmArg_req_handle(avIndex, buf);
    }
    break;

    case IOTYPE_USER_IPCAM_SET_ALARM_ARG_REQ://设置告警相关的参数
    {
        tk_set_alarmArg_req_handle(avIndex, buf);
    }
    break;

    case IOTYPE_USER_IPCAM_GET_EMAIL_ARG_REQ://请求email相关的设置
    {
        TK_get_emailArg_req_handle(avIndex, buf);
    }
    break;

    case IOTYPE_USER_IPCAM_SET_EMAIL_ARG_REQ://设置email相关的参数
    {
        TK_set_emailArg_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_FACTORY_SETTINGS_REQ://0x40015	//请求恢复出厂设置
    {
        TK_factory_setting_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_REBOOT_SYSTEM_REQ: //请求重启系统  = 0X40019
    {
        system("reboot");
    }
    break;
    case IOTYPE_USER_IPCAM_SET_DEVICE_INFO_REQ: //设置设备信息  = 0X40017
    {
        TK_set_deviceInfo_req_handle(avIndex, buf);//设置百度相关的设备信息
    }
    break;
    case IOTYPE_USER_IPCAM_GET_PTZ_STATUS_REQ://获取云台状态  = 0X40021
    {
        //if(g_device_info.serverMask & SUPPORT_YUNTAI)
        //{
        //	get_ptzStatus_req_handle(avIndex, buf);
        //}
    }
    break;
    case IOTYPE_USER_IPCAM_TEST_EMAIL_ARG_REQ: //测试email  = 0X40027
    {
        //test_email_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_UPGRADE_SYSTEM_REQ://app升级 = 0X40029
    {
        app_upgrade_req_handle(avIndex, buf);
    }
    break;
												//0x40031
    case IOTYPE_USER_IPCAM_GET_ALARM_LIST_REQ://查询事件列表日志（0x0318用于查询事件录像文件）
    {
        get_alarmList_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_GET_AUDIO_MOTION_REQ://	= 0X40033,	//获取声音侦测
    {
        //get_audio_motion_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_SET_AUDIO_MOTION_REQ	://	= 0X40035,	//设置声音侦测配置
    {
        //set_audio_motion_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_GET_NET_REQ:///获取网络配置0x40037
    {
        tk_get_net_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_SET_NET_REQ:///设置网络配置 0x40039
    {
        tk_set_net_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_GET_SNAP_REQ://			= 0X40041	//客户端抓拍请求
    {
        TK_get_snap_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_DEFAULT_VIDEO_REQ:	/// 0X40043
    {
        //TK_default_video_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_WIRTE_DEV_CONFIG_REQ:		//0X40045  写入uid   mac
    {
        TK_set_dev_cfg_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_READ_DEV_CONFIG_REQ:		//  0X40047
    {
        TK_get_dev_cfg_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_SET_NTP_CONFIG_REQ://		= 0X40050,	//写入ntp的配置信息
    {
        TK_set_ntp_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_GET_NTP_CONFIG_REQ://		= 0X40052,	//读取ntp的配置信息
    {
        TK_get_ntp_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_GET_COMPANY_CONFIG_REQ://	= 0X40054,	//读取公司信息
    {
        TK_get_company_req_handle(avIndex, buf);
    }
    break;
    case IOTYPE_USER_IPCAM_SET_DEVICEMODEL_CONFIG_REQ:
    {
        TK_set_deviceModel_req_handle(avIndex, buf);
    }
    break;
	case IOTYPE_USER_IPCAM_SET_PLAYBACK_RATE_REQ:			//0X4007C,	//EeOA>O.AEUAE
    {
        printf("set playback rate\n");
        tk_Set_Playback_rate_req(avIndex, buf);
    }
    break;
	//add by jensen
	case IOTYPE_USER_IPCAM_SET_TIMEMODE_TO_SHARE_REQ:
	{	
		tk_set_timemode_cfg(avIndex, buf);
	}
	break;

	case IOTYPE_USER_IPCAM_GET_TIMEMODE_TO_SHARE_REQ:
	{
		tk_get_timemode_cfg(avIndex, buf);
	}
	break;
	
	//end


	
    /*end CMS自扩展************************************************************/
#endif

    case IOTYPE_USER_IPCAM_GET_MDSYS_INFO_REQ://获取魔蛋相关参数信息   sdk_sys_cfg_t
    {
#if 0
        SMsgAVIoctrlSetBaiDuInfoResp resp;
        sdk_baidu_info_t baidu_info;
        sdk_register_cfg_t *register_info = (sdk_register_cfg_t *)buf;//获取到的百度信息


        if(NULL == register_info)
        {
            //百度参数设置
            adapt_param_get_baidu_cfg(&baidu_info);
            //strcpy(baidu_info.access_token,register_info->access_token);//获取百度的用回accesstoke
            // strcpy(baidu_info.user_id,register_info->usr_no);//获取用户的UID
            adapt_param_set_baidu_cfg(&baidu_info);//设置百度参数

            if(open(BAIDU_INFO_GET_PATH, O_CREAT,0644)) /* 创建并打开文件 */
            {
                //resp.result = 0;
                resp.result=0;//设置百度参数成功
            }
        }
        else
        {
            resp.result=-1;//设置百度参数失败
        }
        //在这里写入一个参数
        avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_MDSYS_INFO_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetBaiDuInfoResp ));
#else
        SMsgAVIoctrlGetMdSysInfoResp resp;
        sdk_sys_cfg_t  mdsys_info;
        memset(&mdsys_info,0,sizeof(sdk_sys_cfg_t));

        // TUTK_MSG_CTRL(SDK_MAIN_MSG_SYS_CFG,SDK_PARAM_GET,0,0,pMsg);

        //百度参数设置
        // adapt_param_get_baidu_cfg(&baidu_info);
        // strcpy(baidu_info.access_token,register_info->access_token);//获取百度的用回accesstoke
        //  strcpy(baidu_info.user_id,register_info->usr_no);//获取用户的UID
        // adapt_param_set_baidu_cfg(&baidu_info);//设置百度参数


        //在这里写入一个参数
        avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_MDSYS_INFO_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetBaiDuInfoResp ));


#endif

    }
    break;
	case IOTYPE_USER_IPCAM_SET_LED_STATUS_REQ:
	{
		tk_set_ledStatus_req_handle(avIndex, buf);
	}
	break;
	case IOTYPE_USER_IPCAM_GET_LED_STATUS_REQ:
	{
		tk_get_ledStatus_req_handle(avIndex, buf);
	}
	break;
	case IOTYPE_USER_IPCAM_SET_BUZZER_STATUS_REQ:
	{
		tk_set_buzStatus_req_handle(avIndex, buf);
	}
	break;
	case IOTYPE_USER_IPCAM_GET_BUZZER_STATUS_REQ:
	{
		tk_get_buzStatus_req_handle(avIndex, buf);
	}
	break;
			
#endif
 case IOTYPE_USER_IPCAM_GET_DEVICEMODEL_CONFIG_REQ: ////  0X40056 读取装备信息
    {
        TK_get_deviceModel_req_handle(avIndex, buf);
   		 break;
    }

//for aws
case IOTYPE_USER_IPCAM_SET_AWS_STATUS_REQ:
	{
		TK_set_aws_req_handle(avIndex,buf);
		break;
	}
case IOTYPE_USER_IPCAM_GET_AWS_STATUS_REQ:
	{
		TK_get_aws_req_handle(avIndex,buf);
		break;
	}
//升级

case IOTYPE_USER_IPCAM_REMOTE_UPGRADE_REQ:
	{
		app_upgrade_req_handle1(avIndex,buf);
		break;
	}
case IOTYPE_USER_IPCAM_UPGRADE_PROGRESS_REQ: //查询升级进度
	{
		app_upgrade_progress_handle1(avIndex,buf);
		break;
	}
case IOTYPE_USER_IPCAM_GET_MAC_ADDR_REQ: //查询MAC
	{
		tk_get_mac_addr(avIndex,buf);
		break;
	}

    default:
        printf("non-handle type[%X]\n", type);
        break;
    }
    printf("sid(%d)avIndex(%d)Handle_IOCTRL_Cmd(0x%X)-- end \n ", SID,avIndex,type);
}



/*把握自己需要的那一部分从下边弄回来*/
