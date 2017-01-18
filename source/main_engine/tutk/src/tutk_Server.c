#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <ctype.h>
#include "IOTCAPIs.h"
#include "AVAPIs.h"
#include "AVFRAMEINFO.h"
#include "AVIOCTRLDEFs.h"

#include "tutk.h"
#include "log.h"
#include "streamlib.h"

/************************/
#define FILE_NAME_LEN_MAX 256
#define STREAM_NUMBER_MAX 4
#define DATE_FMT_LEN_MAX  128

#define TRUE	((int)1)
#define FALSE   ((int) 0)

/***********************/
extern int  g_main_run ;


TUTK_INFO  g_tutk_cfg ;
static int g_tutk_run = 0;
sdk_msg_dispatch_cb g_tutk_msg_cb;
tutk_ls_avs_ops g_tutk_ls; //实时流
tutk_hs_avs_ops g_tutk_hs; //历史流
tutk_device_ops g_tutk_ops ;//参数设置一类
int				g_au_detection;   //声音侦测报警

extern LED_BUZZER_INFO  led_buzzer_info;


extern RECORD_STATUS record_status;


#define P2P_ALARM_PUT_SERVER		"www.papa-eye.com"
#define P2P_ALARM_PUT_SERVER_IP		"74.63.82.50"

#define guardzilla_ALARM_PUT_SERVER  "apps.guardzilla.com"
#define guardzilla_ALARM_PUT_SERVER_IP "54.68.243.114"

#define ONE_FRAME_BUF_SIZE		512*1024
#define _MSG_DEBUG				0
#define MAX_FRAME_LEN 1024*1024


#define SERVTYPE_STREAM_SERVER 0x0 //0000010000000//0x1000 //1000000000000
#define MAX_CLIENT_NUMBER	8
#define MAX_SIZE_IOCTRL_BUF		1024
#define MAX_AV_CHANNEL_NUMBER	16

#ifdef AUDIO_FORMAT_PCM
#define AUDIO_FRAME_SIZE 640
#define AUDIO_FPS 26
#define AUDIO_CODEC 0x8C
#elif defined (AUDIO_FORMAT_ADPCM)
#define AUDIO_FRAME_SIZE 640
#define AUDIO_FPS 26
#define AUDIO_CODEC 0x8B
#elif defined (AUDIO_FORMAT_SPEEX)
#define AUDIO_FRAME_SIZE 38
#define AUDIO_FPS 56
#define AUDIO_CODEC 0x8D
#elif defined (AUDIO_FORMAT_MP3)
#define AUDIO_FRAME_SIZE 380
#define AUDIO_FPS 32
#define AUDIO_CODEC 0x8E
#elif defined (AUDIO_FORMAT_SPEEX_ENC)
#define AUDIO_FRAME_SIZE 160
#define AUDIO_ENCODED_SIZE 160
#define AUDIO_FPS 56
#define AUDIO_CODEC 0x8D
#elif defined (AUDIO_FORMAT_G726_ENC)
#define AUDIO_FRAME_SIZE 320
#define AUDIO_ENCODED_SIZE 40
#define AUDIO_FPS 50
#define AUDIO_CODEC 0x8F
#elif defined (AUDIO_FORMAT_G726)
#define AUDIO_FRAME_SIZE 40
#define AUDIO_FPS 50
#define AUDIO_CODEC 0x8F
#endif

#if 0
typedef struct _AV_SYNC_
{
pthread_mutex_t sync_lock;
unsigned int sync_flag; //同步标志位bit0 Need sync   bit1 VIDEO WAIT   bit2 audio wait  
unsigned int video_sec;
unsigned int audio_sec;
} AV_SYNC;
#endif
typedef struct _AV_Client
{
    int 				avIndex;
    unsigned int  		bEnableVideo;
    unsigned char 		bEnableAudio;
    unsigned char 		bEnableSpeaker;
    unsigned char 		bStopPlayBack;
    unsigned char 		bPausePlayBack;
    unsigned char 		speakerCh;
    unsigned char 		playBackCh;
    unsigned char 		threadVideo_active;
    unsigned char 		threadAudio_active;
    SMsgAVIoctrlPlayRecord playRecord;
	media_info 			real_meadia;
	media_info 			play_meadia;
    pthread_rwlock_t 	sLock;
	//AV_SYNC 			sync;
} AV_Client;

FILE 			*g_upgrade_fp = NULL;
int				upgrade_size_add = 0;
unsigned short	upgrade_pack_index = 0;
int 			tutk_updateing = -1;
int 			g_upgrade_percent = 0;
AV_Client gClientInfo[MAX_CLIENT_NUMBER];
int g_videostop = 0;
typedef struct _AV_File_List
{
    SAvEvent Fileevent;
    char file_name[64];
    unsigned char playback;

} AV_File_List;

#define 		MAX_FILE_NUM 192 //最大搜索文件个数
AV_File_List 	pAvEvent[MAX_FILE_NUM] = {0};
int 			file_count = 0;
int 			playBack_index = -1;


unsigned char				g_tutkSpeaker = 0;
int 						gOnlineNum ;

//static int 					gbSearchEvent = 0;
SMsgAVIoctrlListEventResp 	*gEventList;
static char 				gUID[21];


struct sockaddr_in gPushMsgSrvAddr;
/*****************************************************************************
 函 数 名  : getTimeStamp
 功能描述  : 获取当前的系统时间单位是ms
 输入参数  : 无
 输出参数  : 无
 返 回 值  : unsigned
 调用函数  :
 被调函数  :

 修改历史      :
    修改内容   : 新生成函数

*****************************************************************************/
unsigned int getTimeStamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec*1000 + tv.tv_usec/1000);
}

int get_time(struct tm *tmx, unsigned int time)
{
    char buf[128];
    char str_mon[4],str_mday[3],str_year[5],str_hour[3],str_min[3],str_sec[3],str_day[3];

    int i;
    time_t timep;

    char str_mon_array[][4]=
    {
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec"
    };
    tmx->tm_mon=1;

    timep = time;
    strcpy(buf,asctime(gmtime(&timep)));

    sscanf(buf,"%3s %3s  %2s %2s:%2s:%2s %4s",(char *)&str_day,(char *)&str_mon,(char *)&str_mday,(char *)&str_hour,(char *)&str_min,(char *)&str_sec,(char *)&str_year);

    for(i=0; i<12; i++)
    {
        if(!strcmp(str_mon_array[i],str_mon))
        {
            tmx->tm_mon=i;
            break;
        }
    }

    tmx->tm_mday=atoi(str_mday);
    tmx->tm_year=atoi(str_year);
    tmx->tm_hour=atoi(str_hour);
    tmx->tm_min=atoi(str_min);
    tmx->tm_sec=atoi(str_sec);

    printf("TIME:[%04d/%02d/%02d-%02d:%02d:%02d]\n", tmx->tm_year, tmx->tm_mon+1, tmx->tm_mday,
           tmx->tm_hour, tmx->tm_min, tmx->tm_sec);

    return 0;
}

void ChangeUTCToSystemTime(STimeDay *stStartTime,char *system_time,int UTC)
{
    if(stStartTime == NULL )
    {
        printf("stStartTime == NULL\n");
        return ;

    }

    sprintf(system_time,"%04d%02d%02d%02d%02d%02d",
            stStartTime->year,
            stStartTime->month,
            stStartTime->day,
            stStartTime->hour,
            stStartTime->minute,
            stStartTime->second);

}

/***********************************************/
extern int adapt_param_get_user_array(sdk_user_right_t *user_array, int size);
int AuthCallBackFn(char *viewAcc,char *viewPwd)
{
	int i = 0;
	int rtn = 0;
	printf("========login usr =%s,login pwd=%s\n",viewAcc,viewPwd);

	sdk_user_right_t user_array[MAX_USER_NUM];

	memset(user_array, 0, sizeof(user_array));
	rtn = adapt_param_get_user_array(user_array, sizeof(user_array));
	printf("read param 0 %s %s\n",user_array[0].user.user_name,user_array[0].user.user_pwd);
	if(0 != strcmp((char *)user_array[0].user.user_name,"admin"))
	{
		sdk_user_right_t user_array_default[MAX_USER_NUM];
		strcpy((char *)user_array_default[0].user.user_name,"admin");
		strcpy((char *)user_array_default[0].user.user_pwd,"admin");
		adapt_param_set_user_array(user_array_default, sizeof(user_array_default));
		memset(user_array, 0, sizeof(user_array));
		adapt_param_get_user_array(user_array, sizeof(user_array));
	}

	for(i = 0; i<MAX_USER_NUM; i++)
	{
		if(!strcmp((char *)user_array[i].user.user_name, viewAcc))break;
	}
	//printf("========ori user_name=%s,ori user_pwd=%s\n",user_array[i].user.user_name,user_array[i].user.user_pwd);
	printf("viewAcc111 %s %s \n",viewAcc,viewPwd);
	printf("user_array[0].user.user_pwd=%s\n",user_array[0].user.user_pwd);

	if(!strcmp((char *)user_array[i].user.user_pwd, viewPwd))
	{
		return 1;
	}
	else 
	{
		return 0;
	}
}

void regedit_client_to_video(int SID, int avIndex)
{
    AV_Client *p = &gClientInfo[SID];
    if(p->avIndex == -1)
    {
        p->avIndex = avIndex;
    }
    if(p->bEnableVideo != 1)
    {
        p->bEnableVideo = 1;
        printf("regedit_client_to_video !!!!!!! SID:%d \n",SID);
        creat_videoSend_thread(SID);
    }
}

void unregedit_client_from_video(int SID)
{
    AV_Client *p = &gClientInfo[SID];
    p->bEnableVideo = 0;

	//p->sync.sync_flag = 0;
	//pthread_mutex_destroy(&(p->sync.sync_lock));
}

void regedit_client_to_audio(int SID, int avIndex)
{
    AV_Client *p = &gClientInfo[SID];
    if(p->avIndex == -1)
    {
        p->avIndex = avIndex;
    }

    if(p->bEnableAudio != 1)
    {
        p->bEnableAudio = 1;
        create_audioSend_thread(SID);
    }
}

void unregedit_client_from_audio(int SID)
{
    AV_Client *p = &gClientInfo[SID];
    p->bEnableAudio = 0;
	//p->sync.sync_flag = 0;
}

/*****************************/
static int audio_recv_fd = -1;
int init_recv_fd()
{   
	char name_buf[64] = {0};
	int ch = 0;
	int sub_ch = 0;
	//audio_recv_fd = open("/mnt/nfs/tutk_recv.g711a",  O_WRONLY | O_CREAT, 0644);
	
	audio_recv_fd = open("/mnt/nfs/audio.g711a",  O_RDONLY , 0644);
	
	return 0;
}

int uninit_recv_fd()
{
	close(audio_recv_fd);
}

int main_recv_audio_data(int ch,const char *video_data,int data_len)
{   
	
	int ret =  write(audio_recv_fd,video_data,data_len);
	if(ret < 0)    
	{   
		error("write_video_data !!! ret = %d \n",ret); 
	}    
}

int main_read_audio_data(int ch,const char *video_data,int data_len)
{   
	
	int ret =  read(audio_recv_fd,video_data,data_len);
	if(ret < 0)    
	{   
		error("write_video_data !!! ret = %d \n",ret); 
	}    
}

#define AUDIO_FORMAT_G711
#define AUDIO_BUF_SIZE	1280

void *thread_ReceiveAudio(void *arg)
{
    int 			SID = (int )arg;
    int 			recordCnt = 0, recordFlag = 0;
    AV_Client 		*p = &gClientInfo[SID];
    unsigned long 	servType;
    int avIndex = avClientStart(SID, NULL, NULL, 30000, &servType, p->speakerCh);

	printf("[thread_ReceiveAudio] start ok idx[%d]\n", avIndex);
	int ret = 0;
    if(avIndex > -1)
    {
        g_tutkSpeaker = 1;
        char buf[AUDIO_BUF_SIZE];
        //char buf_dec[AUDIO_BUF_SIZE];
        FRAMEINFO_t frameInfo;
        unsigned int frmNo = 0;
		g_tutk_ls.play_start(NULL,1);
		///init_recv_fd();
        while(p->bEnableSpeaker)
        {
            memset(buf,0,AUDIO_BUF_SIZE);
            //ret 返回实际接收数据的长度  出错的时候就返回的是负值
			//ret = avRecvAudioData(avIndex, buf, AUDIO_BUF_SIZE, (char *)&frameInfo, sizeof(FRAMEINFO_t), &frmNo);
			ret = avRecvAudioData(avIndex, buf, AUDIO_BUF_SIZE, (char *)&frameInfo, sizeof(FRAMEINFO_t), &frmNo);
            //info("avRecvAudioData[%d] \n", ret);
             if(ret == AV_ER_DATA_NOREADY)
			{
				usleep(10000);
				//printf("AV_ER_DATA_NOREADY...error[%d]!!!\n",ret);
				continue;
				
			}
			 
			if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE)
			{
				printf("avRecvAudioData AV_ER_SESSION_CLOSE_BY_REMOTE\n");
				break;
			}
			else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT)
			{
				printf("avRecvAudioData AV_ER_REMOTE_TIMEOUT_DISCONNECT\n");
				break;
			}
			else if(ret == IOTC_ER_INVALID_SID)
			{
				printf("avRecvAudioData Session[%d] cant be used anymore\n", SID);
				break;
			}
			else if(ret == AV_ER_LOSED_THIS_FRAME)
			{
				printf("Audio LOST[%d]\n", frmNo);fflush(stdout);
				continue;
			}
			else if(ret < 0)
			{
				printf("Other error[%d]!!!\n", ret);
				continue;
			}			
			adapt_audio_play(0,buf,ret);
        }
    }
	uninit_recv_fd();

    printf("[thread_ReceiveAudio] exit\n");
	g_tutk_ls.play_stop(NULL,1);
    avClientStop(avIndex);
    p->bEnableSpeaker = 0;
    g_tutkSpeaker = 0;
    pthread_exit(0);
}

int creat_recv_audio_func( int sid)
{
    int SID = sid;
    pthread_t Thread_ID;
    pthread_create(&Thread_ID, NULL, &thread_ReceiveAudio, (void *)SID);
    pthread_detach(Thread_ID);
}


void *thread_ForAVServerStart(void *arg)
{
    char 			name[24] = {0};
    int 			SID = (int )arg;
    //printf("<<SID:%d>>---------\n", SID);
    sprintf(name, "AmbaTutkAVServer_%d", SID);
    //printf("<<name:%s>>---------\n", name);

    prctl(PR_SET_NAME, (unsigned long)name, 0,0,0);

    int  			ret;
    int 			result = -1;
    unsigned int 	ioType;
    char 			ioCtrlBuf[MAX_SIZE_IOCTRL_BUF + 4];
    struct st_SInfo Sinfo;
    AV_Client 		*pClient = &gClientInfo[SID];

    if(IOTC_Session_Check(SID, &Sinfo) == IOTC_ER_NoERROR)
    {
        char *mode[3] = {"P2P", "RLY", "LAN"};
        printf("Client is from[%s:%d] Mode[%s] VPG[%d:%d:%d] VER[%X] NAT[%d] AES[%d]\n", Sinfo.RemoteIP, Sinfo.RemotePort, mode[(int)Sinfo.Mode], Sinfo.VID, Sinfo.PID, Sinfo.GID, (unsigned int)Sinfo.IOTCVersion, Sinfo.NatType, Sinfo.isSecure);
    }

    //printf("thread_ForAVServerStart SID[%d] ViewAccount[%s] ViewPassword[%s]\n", SID,g_tutk_cfg.ViewAccount,g_tutk_cfg.ViewPassword);

    int avIndex = avServStart3(SID, AuthCallBackFn, 0, SERVTYPE_STREAM_SERVER, 0, &result);
    if(avIndex < 0)
    {
        printf("avServStart3 failed SID[%d] code[%d]!!!\n", SID, avIndex);
        printf("[thread_ForAVServerStart] exit index[%d]....\n", avIndex);
        IOTC_Session_Close(SID);
        gOnlineNum--;
        pthread_exit(0);
    }

    avServSetResendSize(avIndex, 1024);
    if(pClient->avIndex == -1)
    {
        pClient->avIndex = avIndex;
    }

    printf("avServStart OK[%d]    SERVTYPE_STREAM_SERVER:0x%X\n", avIndex, SERVTYPE_STREAM_SERVER);
    while(g_tutk_run)
    {
        ret = avRecvIOCtrl(avIndex, &ioType, (char *)&ioCtrlBuf, MAX_SIZE_IOCTRL_BUF, 1000);
        if(ret >= 0)
        {
            Handle_IOCTRL_Cmd(SID, avIndex, ioCtrlBuf, ioType);
			//Handle_IOCTRL_Cmd_Ex(SID, avIndex, ioCtrlBuf, ioType);
            //continue;
        }
        else if(ret != AV_ER_TIMEOUT)
        {
            printf("avRecvIOCtrl error, code[%d]\n", ret);
            break;
        }
    }

    printf("avServStop SID[%d] calling..............\n", SID);
    unregedit_client_from_video(SID);
    unregedit_client_from_audio(SID);

    while(pClient->threadVideo_active != 0 || pClient->threadAudio_active != 0)
    {
        usleep(10000);
    }

    avServStop(avIndex);
    printf("[thread_ForAVServerStart] exit SID[%d]....\n", SID);
    gOnlineNum--;
    IOTC_Session_Close(SID);
    pClient->avIndex = -1;
	return;
}

static char *GetPushMessageString(char *UID, int eventType, unsigned int time)
{
    static char msgBuf[2048];
    //sprintf(msgBuf, "GET /iPhonePush/apns.php?cmd=raise_event&uid=%s&event_type=%d&event_time=%lu HTTP/1.1\r\n"
	sprintf(msgBuf, "GET /apns/apns.php?cmd=raise_event&uid=%s&event_type=%d&event_time=%lu HTTP/1.1\r\n"//change by ODM
			"Host: %s\r\n"
            "Connection: keep-alive\r\n"
            "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.52 Safari/536.5\r\n"
            "Accept: */*\r\n"
            "Accept-Encoding:  gzip,deflate,sdch\r\n"
            "Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n"
            "Accept-Charset: Big5,utf-8;q=0.7,*;q=0.3\r\n"
            "Pragma: no-cache\r\n"
            "Cache-Control: no-cache\r\n"
            "\r\n",
            UID, eventType, time/*time((time_t *)NULL)*/, inet_ntoa(gPushMsgSrvAddr.sin_addr));
	char timestr[128];
	memset(timestr,0,sizeof(timestr));
	os_local_time_str(time,timestr);
	

    return msgBuf;
}


int InitSendSockeParament(int  SenSocket)
{
    int result;
    int flags;
    struct timeval		recv_timeout,send_timeout;
    struct linger 		zeroLinger;
    zeroLinger.l_onoff = 1;
    zeroLinger.l_linger = 1;
    result = setsockopt(SenSocket, SOL_SOCKET, SO_LINGER, (const char *)&zeroLinger, sizeof(zeroLinger) );
    if( result != 0 )
    {
        return result;
    }

    recv_timeout.tv_sec = 5;
    recv_timeout.tv_usec = 0;
    result = setsockopt(SenSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&recv_timeout, sizeof(recv_timeout));
    if( result !=0 )
    {
        return result;
    }

    send_timeout.tv_sec = 5;
    send_timeout.tv_usec = 0;
    result = setsockopt(SenSocket,SOL_SOCKET,SO_SNDTIMEO,(char *)&send_timeout,sizeof(send_timeout));
    if(result != 0)
    {
        return result;
    }

    flags = fcntl(SenSocket, F_GETFL, 0);
    fcntl(SenSocket, F_SETFL, flags & O_NONBLOCK);	//fcntl(SenSocket, F_SETFL, flags &(~O_NONBLOCK));		return 	1;
	return 0;
}
void *SendMsgToClientThrd(void *param)
{
	/////////////////////////////////////////
#if 1
	//sdk_event_t *event_obj=(sdk_event_t *)param;

	struct tm tmx;
	time_t		timep;
	time(&timep);

	SMsgAVIoctrlEvent	resp;
	get_time(&tmx, timep);
	resp.stTime.year = tmx.tm_year;
	resp.stTime.month = tmx.tm_mon;
	resp.stTime.day = tmx.tm_mday;
	resp.stTime.wday = 1;
	resp.stTime.hour = tmx.tm_hour;
	resp.stTime.minute = tmx.tm_min;
	resp.stTime.second = tmx.tm_sec;
	resp.time = timep;//time((time_t *)&resp.time);
	resp.event = 1;
#if 1
	int num;
	for(num = 0 ; num < MAX_CLIENT_NUMBER; num++)
	{
		if(gClientInfo[num].avIndex < 0) continue;
		resp.channel=gClientInfo[num].avIndex;
		printf("Dend even to client[%d][channel:%d event=0x%02x]\n", num, resp.channel, resp.event);
		avSendIOCtrl(gClientInfo[num].avIndex, IOTYPE_USER_IPCAM_EVENT_REPORT, (char *)&resp, sizeof(SMsgAVIoctrlEvent));
	}
#endif
	
	//avSendIOCtrl(0, IOTYPE_USER_IPCAM_EVENT_REPORT, (char *)&resp, sizeof(SMsgAVIoctrlEvent));
#endif
	/////////////////////////////////////////

}

void SendMsgToClient(sdk_event_t *event_obj)
{
	/////////////////////////////////////////
#if 1
	struct tm tmx;
	time_t		timep;
	time(&timep);

	SMsgAVIoctrlEvent	resp;
	get_time(&tmx, timep);
	resp.stTime.year = tmx.tm_year;
	resp.stTime.month = tmx.tm_mon;
	resp.stTime.day = tmx.tm_mday;
	resp.stTime.wday = 1;
	resp.stTime.hour = tmx.tm_hour;
	resp.stTime.minute = tmx.tm_min;
	resp.stTime.second = tmx.tm_sec;
	resp.time = timep;//time((time_t *)&resp.time);
	resp.event = 1;
#if 1
	int num;
	for(num = 0 ; num < MAX_CLIENT_NUMBER; num++)
	{
		if(gClientInfo[num].avIndex < 0) continue;
		resp.channel=gClientInfo[num].avIndex;
		printf("send even to client[%d][channel:%d event=0x%02x]\n", num, resp.channel, resp.event);
		avSendIOCtrl(gClientInfo[num].avIndex, IOTYPE_USER_IPCAM_EVENT_REPORT, (char *)&resp, sizeof(SMsgAVIoctrlEvent));
	}
#endif
	
	//avSendIOCtrl(0, IOTYPE_USER_IPCAM_EVENT_REPORT, (char *)&resp, sizeof(SMsgAVIoctrlEvent));
#endif
	/////////////////////////////////////////

}
void SendPushMessage(int eventType, unsigned int time)
{
    int skt;
    if (gPushMsgSrvAddr.sin_addr.s_addr == 0)
    {
        printf("No push message server\n");
        return;
    }

    if ((skt = (int)socket(AF_INET, SOCK_STREAM, 0)) >= 0)
    {
        int error=-1, len;
        len = sizeof(int);
        struct timeval tm;
        fd_set set;
        unsigned long ul = 1;
        ioctl(skt, FIONBIO, &ul); //设置为非阻塞模式
        int ret = FALSE;

        if (connect(skt, (struct sockaddr *)&gPushMsgSrvAddr, sizeof(struct sockaddr_in)) == -1)
        {

            tm.tv_sec = 5;
            tm.tv_usec = 0;

            FD_ZERO(&set);
            FD_SET(skt, &set);
            if( select(skt+1, NULL, &set, NULL, &tm) > 0)
            {
                getsockopt(skt, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
                if(error == 0)
                    ret = TRUE;
                else
                    ret = FALSE;
            }
            else
            {
                ret = FALSE;
            }
        }
        else
        {
            ret = TRUE;
        }


        ul = 0;
        ioctl(skt, FIONBIO, &ul); //设置为阻塞模式


        if(!ret)
        {
            close( skt );
            fprintf(stderr , "Cannot Connect the server!\n");
        }
        else
        {
            printf("connect HTTP OK\n");
            char *msg = GetPushMessageString(gUID, eventType, time);
            send(skt, msg, strlen(msg) , 0);			
            printf("ZW_TEST  send  %s  \n",msg);
        }

		//change  by zw and  test for20150818 
		char recData[512];
		int zwret = recv(skt, recData, 512, 0);

		if(zwret > 0)
		{
		   recData[zwret] = 0x00;
		   printf("ZW_TEST +_+_+_+_+ recData  %s \n",recData);
		}
		 printf("ZW_TEST gPushMsgSrvAddr.sin_addr.s_addr  is 0x %x\n",gPushMsgSrvAddr.sin_addr.s_addr);
		

        close(skt);
    }
}


static char *GetRegMessageString(char *UID)
{
    static char msgBuf[2048];
    sprintf(msgBuf,"GET /apns/apns.php?cmd=reg_server&uid=%s HTTP/1.1\r\n"//change by ODM
			"Host: %s\r\n"
            "Connection: keep-alive\r\n"
            "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.52 Safari/536.5\r\n"
            "Accept: */*\r\n"
            "Accept-Encoding: gzip,deflate,sdch\r\n"
            "Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n"
            "Accept-Charset: Big5,utf-8;q=0.7,*;q=0.3\r\n"
            "Pragma: no-cache\r\n"
            "Cache-Control: no-cache\r\n"
            "\r\n",
            UID, inet_ntoa(gPushMsgSrvAddr.sin_addr));

    return msgBuf;
}


static void SendRegister()
{
#if 1
    struct hostent *host = gethostbyname(guardzilla_ALARM_PUT_SERVER);
    if (host != NULL)
    {
        //gPushMsgSrvAddr is a global variable of “struct sockaddr_in”
        memcpy(&gPushMsgSrvAddr.sin_addr, host->h_addr_list[0], host->h_length);


        struct sockaddr_in adr_inet; /* AF_INET */
        inet_aton(guardzilla_ALARM_PUT_SERVER_IP, &adr_inet.sin_addr);
        memcpy(&gPushMsgSrvAddr.sin_addr, &adr_inet.sin_addr, sizeof(adr_inet.sin_addr));
        gPushMsgSrvAddr.sin_port = htons(80);
        gPushMsgSrvAddr.sin_family = AF_INET;
    }
    else
    {
        printf("fail to resolve host name\n");
        memset(&gPushMsgSrvAddr, 0, sizeof(gPushMsgSrvAddr));
        return;
    }
#else
    struct sockaddr_in adr_inet; /* AF_INET */
    inet_aton(P2P_ALARM_PUT_SERVER_IP, &adr_inet.sin_addr);
    memcpy(&gPushMsgSrvAddr.sin_addr, &adr_inet.sin_addr, sizeof(adr_inet.sin_addr));
    gPushMsgSrvAddr.sin_port = htons(80);
    gPushMsgSrvAddr.sin_family = AF_INET;
#endif
    printf("line:%d -----------------------------------------------------gPushMsgSrvAddr.sin_addr[%X] \n",__LINE__,gPushMsgSrvAddr.sin_addr);
    int skt;
    if ((skt = (int)socket(AF_INET, SOCK_STREAM, 0)) >= 0)
    {
        printf("line:%d ----------------------------------------------------- skt:%d\n",__LINE__,skt);
        if (connect(skt, (struct sockaddr *)&gPushMsgSrvAddr, sizeof(struct sockaddr_in)) == 0)
        {
            char *msg = GetRegMessageString(gUID);
            printf("%s  \n",msg);
            send(skt , msg, strlen(msg), 0);
            printf("Register OK\n");
        }

		char recData[512];
		int zwret = recv(skt, recData, 512, 0);

		if(zwret > 0)
		{
		   recData[zwret] = 0x00;
		   printf("SendRegister re +_+_+_+_+ recData  %s \n",recData);
		}
		 printf("SendRegister gPushMsgSrvAddr.sin_addr.s_addr  is 0x %x\n",gPushMsgSrvAddr.sin_addr.s_addr);
		
        close(skt);
    }
}


void *login_thread(void *arg)
{
    prctl(PR_SET_NAME, (unsigned long)"login_thread", 0,0,0);
    int ret;
	
	//strcpy(gUID, g_tutk_cfg.guid);
    while(g_tutk_run)
    {
        //ret = IOTC_Device_Login((char *)gUID, NULL, NULL);
        ret = IOTC_Device_Login(gUID, NULL, NULL);
        printf("IOTC_Device_Login() ret = %d\n", ret);
        if(ret == IOTC_ER_NoERROR)
        {
            SendRegister();
            break;
        }
        else
        {
            sleep(30);
			break;
        }
    }

    pthread_exit(0);
}





void *thread_AudioFrameData(void *arg)
{
    int ret;
    char			csBuf[32];
    int 			sid = (int)arg;
    AV_Client		*pClient = &gClientInfo[sid];

    info("audio %d pClient:0x%x ",sid,pClient);
    sprintf(csBuf,"tutkAudio_%d", sid);
    prctl(PR_SET_NAME, (unsigned long)csBuf, 0,0,0);
    pClient->threadAudio_active = 1;
    int 			size = 0;
    unsigned int	cur_timestamp=0;
    FRAMEINFO_t 	frameInfo;
    BOOL			bFirstRead = TRUE;
	char *audio_buffer = NULL;
	sdk_frame_t *frame_head = NULL;

    g_tutk_ls.open_audio(&(pClient->real_meadia) , 0, 0, NULL);

    while(g_tutk_run && (gOnlineNum > 0) && pClient->bEnableAudio)
    {
		memset(&frameInfo, 0, sizeof(FRAMEINFO_t));
        if(g_tutk_ls.pull_audio(&(pClient->real_meadia), 0,&audio_buffer)< 0)
        {
            usleep(10);
            continue;
        }
		frame_head = (frame_t*)(audio_buffer);

		frameInfo.codec_id = MEDIA_CODEC_AUDIO_G726;//MEDIA_CODEC_AUDIO_G711A;
		frameInfo.flags = (AUDIO_SAMPLE_8K << 2) | (AUDIO_DATABITS_16 << 1) | AUDIO_CHANNEL_MONO;
       	frameInfo.timestamp =(unsigned int)frame_head->pts;

		ret = avSendAudioData(pClient->avIndex,(const char *)frame_head->data, frame_head->frame_size, &frameInfo, sizeof(FRAMEINFO_t));
		if(ret != AV_ER_NoERROR)
		{
			if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE)
			{
				info("thread_AudioFrameData AV_ER_SESSION_CLOSE_BY_REMOTE\n");
				unregedit_client_from_audio(sid);
			}
			else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT)
			{
				info("thread_AudioFrameData AV_ER_REMOTE_TIMEOUT_DISCONNECT\n");
				unregedit_client_from_audio(sid);
			}
			else if(ret == IOTC_ER_INVALID_SID)
			{
				info("Session cant be used anymore\n");
				unregedit_client_from_audio(sid);
			}
			else if(ret < 0)
			{
				printf("avSendAudioData error[%d]\n", ret);
			}
		}
       
    }
    printf("leaving thread_AudioFrameData .....\n");
	g_tutk_ls.close_audio(&(pClient->real_meadia),0);
    pClient->threadAudio_active = 0;
	return NULL;

}


//发送视频
void *thread_VideoFrameData(void *arg)
{
     prctl(PR_SET_NAME, (unsigned long)"VideoFrameData", 0,0,0);
    int				counter = 0;
    int			sid = (int)arg;
    AV_Client 		*pClient = &gClientInfo[sid];

    int 			size = 0;
    unsigned int 	cur_timestamp=0;
    FRAMEINFO_t 	frameInfo;
    BOOL			bFirstRead = TRUE;
	char *frame_buffer = NULL;
	sdk_frame_t *frame_head = NULL;

    int 			need_send_Iframe = 1;
    int 			ret = 0;
    memset(&frameInfo, 0, sizeof(FRAMEINFO_t));
    frameInfo.codec_id = MEDIA_CODEC_VIDEO_H264;
    printf("QJZ (%s)---sid(%d)\n", __FUNCTION__, sid);
	info("video  pClient:0x%x ",pClient);

	if (1 == pClient->threadVideo_active) //同一个设备多次进入 
	{
		return NULL;
	}
    frameInfo.timestamp = getTimeStamp();
	//如果句柄不为空说明当前正在获取码流，所以先关闭之前的 再打开现在的
	if(NULL != pClient->real_meadia.video_handle) 
	{
		g_tutk_ls.close(&(pClient->real_meadia),pClient->real_meadia.video_current_chn);
		printf("---------thread_VideoFrameData colse[chn%d]-----------\n",pClient->real_meadia.video_current_chn);

	}
	printf("--------thread_VideoFrameData open[chn%d]--------------\n",g_tutk_ls.encode_current_chn);
  	g_tutk_ls.open(&(pClient->real_meadia) , g_tutk_ls.encode_current_chn, 0, NULL);
	pClient->threadVideo_active = 1;
	 
    while(g_tutk_run &&(gOnlineNum > 0)&& pClient->bEnableVideo)
    {
		if(g_tutk_ls.pull(&(pClient->real_meadia), g_tutk_ls.encode_current_chn,&frame_buffer)< 0)
        {
            usleep(10);
            continue;
        }
              
        frame_head = (sdk_frame_t  *)frame_buffer;
		//printf("read video info:frame_no :%d frame size = %d  frame_type:%d \n",frame_head->frame_no,frame_head->frame_size,frame_head->frame_type);
        if(need_send_Iframe && frame_head->frame_type != SDK_VIDEO_FRAME_I)
        {
        	continue;
        }
		need_send_Iframe = 0;
		frameInfo.flags = (frame_head->frame_type == SDK_VIDEO_FRAME_I) ?IPC_FRAME_FLAG_IFRAME:IPC_FRAME_FLAG_PBFRAME;
      

        if(frame_head->frame_size > ONE_FRAME_BUF_SIZE)
        {
            need_send_Iframe = 1;
			printf("ONE_FRAME_BUF_SIZE :512*1024 \n");
            continue;
        }
		//音视频同步 只有再打开音频时才运行
        frameInfo.timestamp = frame_head->pts;
        frameInfo.onlineNum = gOnlineNum;
        ret = avSendFrameData(pClient->avIndex, frame_head->data, frame_head->frame_size, &frameInfo, sizeof(FRAMEINFO_t));
        if(ret != AV_ER_NoERROR)
        {
            if(ret == AV_ER_EXCEED_MAX_SIZE || ret == AV_ER_EXCEED_MAX_ALARM) // means data not write to queue, send too slow, I want to skip it
            {
                need_send_Iframe = 1;
                printf("AV_ER_EXCEED_MAX_SIZE[%d] ret[%d]\n", pClient->avIndex, ret);

                if(counter < 3)
                {
                    counter ++;
                }
                if(counter >= 3)
                {
                   //这里要做降码流处理
                }
                continue;
            }
            else if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE)
            {
                //printf("thread_VideoFrameData AV_ER_SESSION_CLOSE_BY_REMOTE SID[%d]\n", i);
                unregedit_client_from_video(sid);
                continue;
            }
            else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT)
            {
                //printf("thread_VideoFrameData AV_ER_REMOTE_TIMEOUT_DISCONNECT SID[%d]\n", i);
                unregedit_client_from_video(sid);
                continue;
            }
            else if(ret == IOTC_ER_INVALID_SID)
            {
                //printf("Session cant be used anymore\n");
                unregedit_client_from_video(sid);
                continue;
            }
        }
        else
        {
            

        }

    }
	g_tutk_ls.close(&(pClient->real_meadia), g_tutk_ls.encode_current_chn);
	sleep(1);
    printf("[thread_VideoFrameData] exit sid(%d)\n",sid);
    pClient->threadVideo_active = 0;
	return NULL;
}


void init_AVInfo()
{
    int i;
    for(i=0; i<MAX_CLIENT_NUMBER; i++)
    {
        memset(&gClientInfo[i], 0, sizeof(AV_Client));
        gClientInfo[i].avIndex = -1;
        gClientInfo[i].bEnableVideo = 0;
        gClientInfo[i].bEnableAudio = 0;
        gClientInfo[i].bEnableSpeaker = 0;
        gClientInfo[i].bStopPlayBack = 0;
        gClientInfo[i].bPausePlayBack = 0;
        gClientInfo[i].speakerCh = 0;
        gClientInfo[i].playBackCh = 0;
        gClientInfo[i].threadVideo_active = 0;
        gClientInfo[i].threadAudio_active = 0;
        pthread_rwlock_init(&(gClientInfo[i].sLock), NULL);
		//pthread_mutex_init(&(gClientInfo[i].sync.sync_lock),NULL);
		
    }
}

void creat_videoSend_thread(int SID)
{
    int 			sID = SID;
    printf("creat_videoSend_thread sID:%d \n",sID);
    pthread_t 		ThreadVideoFrameData_ID;
   // pthread_create(&ThreadVideoFrameData_ID, NULL, &thread_VideoFrameData, (void *)sID);
    pthread_create(&ThreadVideoFrameData_ID, NULL, thread_VideoFrameData, (void *)sID);
    printf("creat_videoSend_thread ========== end \n",sID);
    pthread_detach(ThreadVideoFrameData_ID);
}

void create_audioSend_thread(int SID)
{
    int 			sID = SID;
    pthread_t 		ThreadAudioFrameData_ID;
    pthread_create(&ThreadAudioFrameData_ID, NULL, &thread_AudioFrameData, (void *)sID);
    pthread_detach(ThreadAudioFrameData_ID);
}


void LoginInfoCB(unsigned long nLoginInfo)
{
    if((nLoginInfo & 0x04))
        printf("I can be connected via Internet\n");
    else if((nLoginInfo & 0x08))
        printf("I am be banned by IOTC Server because UID multi-login\n");
}


///int vavpis_server(char *guid,char *sname,char *spwd)
void *tutk_server_thread(void *arg)
{
    prctl(PR_SET_NAME, (unsigned long)"tutk_server_thread", 0,0,0);
    pthread_detach(pthread_self());  //分离线程

    int 		i,ret , SID;
    pthread_t 	thread_login_id;
	unsigned int version = 0;
    init_AVInfo();

    printf("StreamServer start...\n");
	IOTC_Get_Version(&version);
	printf("+++++++++++++++++++++0X%X++++++++++++++++++++++++++++\n",version);
    IOTC_Set_Max_Session_Number(MAX_CLIENT_NUMBER);

    //use which Master base on location, port 0 means to get a random port
    //ret = IOTC_Initialize(0, "m1.iotcplatform.com", "m2.iotcplatform.com", "m3.iotcplatform.com", "m5.iotcplatform.com");
	//ret = IOTC_Initialize(0, "46.137.188.54", "122.226.84.253", "m2.iotcplatform.com", "m5.iotcplatform.com");
	ret = IOTC_Initialize2(0);
    printf("IOTC_Initialize2() ret = %d\n", ret);
    if(ret != IOTC_ER_NoERROR)
    {
        printf("IOTCAPIs_Device exit...!!\n");
        return 0;
    }

    IOTC_Get_Login_Info_ByCallBackFn(LoginInfoCB);

    //alloc MAX_CLIENT_NUMBER*3 for every session av data/speaker/play back
    avInitialize(MAX_CLIENT_NUMBER*3);
    avServSetMaxBufSize(3072);

    //create thread to login because without WAN still can work on LAN
    strcpy(gUID, g_tutk_cfg.guid);
    //strcpy(gUID, "DG2TTET8ELFKZCJP111A");
	
    pthread_create(&thread_login_id, NULL, &login_thread, (void *)gUID);
    pthread_detach(thread_login_id);

    while (g_tutk_run)
    {
        //Accept connection only when IOTC_Listen() calling
        SID = IOTC_Listen(0);
        if(SID > -1)
        {
            pthread_t Thread_ID;
            pthread_create(&Thread_ID, NULL, &thread_ForAVServerStart, (void *)SID);
            pthread_detach(Thread_ID);
            gOnlineNum++;
        }
    }
    avDeInitialize();
    IOTC_DeInitialize();

    return 0;
}

int get_gOnlineNum()
{
    return gOnlineNum;
}

/************************************/
//如果合法能用  则返回0   否则返回-1
static int check_uid(const char *uid)
{
	printf(">>>>>>>>>>> check_uid :%s \n",uid);
	//if(!strstr(uid,"ABCDEFNA9ED7P9H")&&(uid[0] == 0 && uid[1] == 0 &&uid[18] == 0 && uid[19] == 0))
	if((0 == uid[0]) && (0 == uid[1]) && (0 == uid[2]) && (0 == uid[3]))
	{
		return -1;
	}
	return 0;
}
//SDK 接口定义

/* SDK 接口定义 */
int tutk_server_init(sdk_msg_dispatch_cb msg_cb)
{
    if(msg_cb)
    	g_tutk_msg_cb = msg_cb;
	
    g_tutk_cfg.tutk_enable = 1;
    g_tutk_cfg.quality = 10;
    sdk_sys_cfg_t sys_cfg;
    memset(&sys_cfg,0,sizeof(sdk_sys_cfg_t));

    tk_get_sys_info(&sys_cfg);
	//检查UID 合法性  不合法的话就随机生成一个
	printf("========sys_cfg.serial_2 = %s\n",sys_cfg.serial_2);
    if(!check_uid(sys_cfg.serial_2) )
    {
        strncpy(g_tutk_cfg.guid,sys_cfg.serial_2,strlen(sys_cfg.serial_2));
    }
    else
    {
        struct timeval nowtimeval;
        int i;
        int value[6] = {0};
        char id_buf[24] = {0};

        for(i = 0; i < 5; i++)
        {
            gettimeofday(&nowtimeval,0);
            srand(nowtimeval.tv_usec%0xFFFF);
            value[i] = abs((rand()%10));
            printf("value[j] = %d \n ",value[i]);
        }
        snprintf(id_buf,21,"ABCDEFNA9ED7P9H%d%d%d%d%d\n",value[0],value[1],value[2],value[3],value[4]);
		//strcpy(id_buf,"VBG1YC4MT4ABDWLWENFJ");
		printf("get uid:%s \n",id_buf);
		strncpy(sys_cfg.serial_2,id_buf,strlen(id_buf));
		strncpy(g_tutk_cfg.guid,id_buf,strlen(id_buf));
		tk_set_sys_info(&sys_cfg);

    }

    strncpy(g_tutk_cfg.ViewAccount,"admin",strlen("admin"));
    strncpy(g_tutk_cfg.ViewPassword,"admin",strlen("admin"));


    return 0;
}

//释放资源
int tutk_server_uninit()
{
    g_tutk_cfg.tutk_enable = 0;
    g_tutk_cfg.quality = 0;
    g_tutk_msg_cb = NULL;
    return 0;
}


int tutk_server_reg_device_ops(tutk_device_ops *ops)                  /* 注册设备参数接口*/
{
    if(ops)
        g_tutk_ops = *ops;
    return 0;

}
int tutk_server_reg_avs_ops(tutk_ls_avs_ops *ls, tutk_hs_avs_ops *hs)/* 注册流媒体接口 */
{
	//先获取保存的编码通道参数
	int i;
    sdk_encode_t enc_param ;
	memset(&enc_param,0,sizeof(sdk_encode_t));
	adapt_param_get_encode_cfg(0,&enc_param);

	printf("-------------------------tutk_server_reg_avs_ops--------------------------------\n");
	for(i=0;i<2;i++)
	{
		printf("resolution	= %d\n",enc_param.av_enc_info[i].resolution);
		printf("bitrate_type= %d %s\n",enc_param.av_enc_info[i].bitrate_type,(enc_param.av_enc_info[i].bitrate_type == 0)? "CBR":"VBR");
		printf("pic_quilty	= %d\n",enc_param.av_enc_info[i].pic_quilty);
		printf("frame_rate	= %d\n",enc_param.av_enc_info[i].frame_rate);
		printf("gop 		= %d\n",enc_param.av_enc_info[i].gop);
		printf("video_type	= %s\n",(enc_param.av_enc_info[i].video_type == 0)?"H264":"OTHER");
		printf("mix_type	= %s\n",(enc_param.av_enc_info[i].mix_type == 1)?"VIDEO":"AUDIO");
		printf("bitrate 	= %d\n",enc_param.av_enc_info[i].bitrate);
		printf("level		= %d\n",enc_param.av_enc_info[i].level);
		printf("h264_ref_mod= %d\n",enc_param.av_enc_info[i].h264_ref_mod);	
		printf("is_using	= %d\n",enc_param.av_enc_info[i].is_using);
	}
	printf("--------------------END-------------------------\n");
	
	if(ls) g_tutk_ls = *ls;
    if(hs) g_tutk_hs = *hs;

	if(1 == enc_param.av_enc_info[0].is_using)
	{
		g_tutk_ls.encode_current_chn 	= 0; //编码0通道 ==720P
	}
	else if(1 == enc_param.av_enc_info[1].is_using)
	{
		g_tutk_ls.encode_current_chn 	= 1; //编码1通道 ==VGA
	}
	else
	{
		g_tutk_ls.encode_current_chn 	= 0; //编码0通道 ==720P
	}
	printf("g_tutk_ls.encode_current_chn=%d\n",g_tutk_ls.encode_current_chn);
	return 0;

}
int  tutk_server_media_set_u(media_handle m, void *u)
{
    /*
        if(m && u)
        {
            STREAMING_SERVER *server = (STREAMING_SERVER *)m;
            server->u = u;
        }
       */
    return 0;

}
void *tutk_server_media_get_u(media_handle m)
{
    /*
        if(m)
        {
            STREAMING_SERVER *server = (STREAMING_SERVER *)m;
            if(server->u)
                return server->u;
            else
                return NULL;
        }
        */
        return NULL;
}

void *thread_led_buzzer_handle(void *arg);
int led_buzzer_init()
{
	memset(&led_buzzer_info,0,sizeof(led_buzzer_info));
	pthread_mutex_init(&led_buzzer_info.led_buzzer_lock,NULL);
	tk_get_alarm_param(&led_buzzer_info);
	led_buzzer_info.led_buzzer_pthread_is_processing = 1;
}

int tutk_server_start()	/* 启动服务 */
{
    g_tutk_run = 1;
    pthread_t   tutk_thread_id;
    int ret = pthread_create(&tutk_thread_id, NULL, &tutk_server_thread, NULL);
    if(ret < 0)
    {
        printf("pthread_create failed ret[%d]\n", ret);
        return -1;
    }
    return 0;
}

int tutk_server_stop()
{
    g_tutk_run = 0;
    return 0;
}
int tutk_server_restart()	/* 重新启动服务 */
{
    g_tutk_run = 0;
    sleep(2);

	memset(gClientInfo,-1,sizeof(gClientInfo));
    g_tutk_run = 1;
    pthread_t   tutk_thread_id;
    int ret = pthread_create(&tutk_thread_id, NULL, &tutk_server_thread, NULL);
    if(ret < 0)
    {
        printf("pthread_create failed ret[%d]\n", ret);
        return -1;
    }

    return 0;
}
void set_led_gpio()
{
	int gpio_led_r = 30;
	int gpio_led_g = 29;
	int gpio_led_b = 28;
	
	gpioEx_defpin(gpio_led_r, 1);  //set output
	gpioEx_defpin(gpio_led_g, 1);  //set output
	gpioEx_defpin(gpio_led_b, 1);  //set output
}
void led_green_on()
{
	int gpio_led_r = 30;
	int gpio_led_g = 29;
	int gpio_led_b = 28;
	
	gpioEx_clear(gpio_led_r);
	gpioEx_set(gpio_led_g);
	gpioEx_clear(gpio_led_b);

}
void led_green_off()
{
	int gpio_led_r = 30;
	int gpio_led_g = 29;
	int gpio_led_b = 28;
	
	gpioEx_clear(gpio_led_r);
	gpioEx_clear(gpio_led_g);
	gpioEx_clear(gpio_led_b);

}
void led_blue_on()
{
	int gpio_led_r = 30;
	int gpio_led_g = 29;
	int gpio_led_b = 28;
	
	gpioEx_clear(gpio_led_r);
	gpioEx_clear(gpio_led_g);
	gpioEx_set(gpio_led_b);

}
void led_blue_off()
{
	int gpio_led_r = 30;
	int gpio_led_g = 29;
	int gpio_led_b = 28;
	
	gpioEx_clear(gpio_led_r);
	gpioEx_clear(gpio_led_g);
	gpioEx_clear(gpio_led_b);

}
void led_red_on()
{
	int gpio_led_r = 30;
	int gpio_led_g = 29;
	int gpio_led_b = 28;
	
	gpioEx_clear(gpio_led_g);
	gpioEx_clear(gpio_led_b);
	
	gpioEx_set(gpio_led_r);

}
void led_red_off()
{
	int gpio_led_r = 30;
	int gpio_led_g = 29;
	int gpio_led_b = 28;
	
	gpioEx_clear(gpio_led_r);
	gpioEx_clear(gpio_led_g);
	gpioEx_clear(gpio_led_b);

}
//上报RESET键按下松开
void send_ResetKeyStatus_handle(unsigned char status)
{
	if(get_gOnlineNum() > 0)
    {
        SMsgAVIoctrlResetStatusResp resp;

        memset(&resp, 0, sizeof(SMsgAVIoctrlResetStatusResp));
        resp.result = status;
        int i = 0;
        for(i = 0; i < MAX_CLIENT_NUMBER; i++)
        {      
            if(gClientInfo[i].avIndex != -1)
            {
           	 //printf("send_ResetKeyStatus_handle(sid:%d) [%d]\n", i,gClientInfo[i].avIndex);
            	avSendIOCtrl(gClientInfo[i].avIndex, IOTYPE_USER_IPCAM_RESET_STATUS_RESP, (char *)&resp, sizeof(SMsgAVIoctrlResetStatusResp));
            }
        }
    }
}

void *thread_led_buzzer_handle(void *arg)
{
#define SLEEP_TIME 100
	static unsigned int led_interval = 0;//LED的闪动 识别
	static unsigned int led_counter = 0; //LED的计数器  将bit16开始按照优先级作为标志位
	static unsigned int buzzer_counter = 0; //buzzer的计数器
	static unsigned int reset_flags = 0; // 1进入重启 常亮2开始闪动 3 闪动时间到4 进入重启
	int gpio_buz_out = 32+24;
	while(1 == led_buzzer_info.led_buzzer_pthread_is_processing)
	{	//printf("led_buzzer_info.led_status=0x%x \n",led_buzzer_info.led_status);
		//printf("led_buzzer_info.buzzer_status=0x%x\n",led_buzzer_info.buzzer_status);
		//reset按键的处理
		if(0 == adapt_read_reset_value())
		{
			pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
			led_buzzer_info.led_status |= 1<<LED_RESTART;
			pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);
			if( 0 == reset_flags)
			{
				send_ResetKeyStatus_handle(1);
				//printf("send_ResetKeyStatus_handle(1)\n");
			}
		}
		else
		{		
			if(0 != reset_flags)
			{
				//led_counter = 0;
				if(reset_flags < 2) //进入闪动 即可松手了
				{
					reset_flags = 0;
					pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
					led_buzzer_info.led_status &= ~(1<<LED_RESTART);
					pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);
					send_ResetKeyStatus_handle(0);
					//printf("send_ResetKeyStatus_handle(0)\n");
				}
				else if(3 == reset_flags)
				{
					send_ResetKeyStatus_handle(0);
					//printf("send_ResetKeyStatus_handle(00)\n");
				}
			}
		}
		if(3 == reset_flags )
		{
			//printf("5S timeout ! restart......\n");
			reset_flags = 4;
			TK_factory_setting_req_handle(0,NULL);
		}



		
		//LED的处理
		//重启时蓝灯闪动
		//优先级问题只比较低16bit  
		if((led_buzzer_info.led_status & 0xffff) >= (1<<LED_RESTART))
		{
			//printf("LED_RESTART\n");
			if(0 == reset_flags)
			{
				led_counter = 0;
			}
			//处理定时的时间 
			if(1 == (led_counter & (1<<(LED_RESTART +16)))>>(LED_RESTART +16))
			{
				led_counter++;
				//reset_flags = 1;
			}
			else
			{
				led_counter = 0;
				led_counter = 1<<(LED_RESTART+16);
				reset_flags = 1;
			}
			//printf("led_counter=0x%x\n",led_counter);
			if((1 == reset_flags) && ((led_counter & 0xffff) < 4 * 1000 / SLEEP_TIME))
			{
				reset_flags = 1;
				//led_blue_on();
			}
			if(((led_counter & 0xffff) >= 5 * 1000 / SLEEP_TIME))
			{
				reset_flags = 2;
			}
			//根据状态处理LED的闪动
			if(2 == reset_flags)
			{
				if(0 == led_interval)
				{
					//led_blue_on();
					led_interval = 1;
				}
				else if(1 == led_interval)
				{
					//led_blue_off();
					led_interval = 0;
				}
				else
				{
					led_interval = 0; 
				}
			}
			//定时到，进一步处理	需要关闭状态位
			if((led_counter & 0xffff)>= 7 * 1000 / SLEEP_TIME) 	//重启6秒 
			{
				led_interval	= 0;
				led_counter 	= 0;
				pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
				led_buzzer_info.led_status &= ~(1<<LED_RESTART);
				pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);
				reset_flags = 3;
			}
		}
		else if((led_buzzer_info.led_status & 0xffff) >= (1<<LED_WIFI_CONFIGING)) 
		{
			if(0 == led_interval)
			{
				//led_red_on();
				led_interval = 1;
			}
			else if(1 == led_interval)
			{
				//led_red_off();
				led_interval = 0;
			}
			else
			{
				led_interval = 0; 
			}
		}
		else if((led_buzzer_info.led_status & 0xffff) >= (1<<LED_NO_WIFI)) 
		{	//printf("LED_NORMAL led_interval=%d\n",led_interval);
			//printf("=======================LED_NO_WIFI=========================\n");
			if(5 == led_interval)
			{
				//led_green_on();
				led_interval ++;
			}
			else if(led_interval >= 10 )
			{
				//led_green_off();
				led_interval = 0;
			}
			else
			{
				led_interval ++;
			}
		}
		else if((led_buzzer_info.led_status & 0xffff) >= (1 << LED_FORCE_CLOSE))
		{
			//printf("LED_FORCE_CLOSE\n");
			//led_red_off();
			//led_blue_off();
			//led_green_off();
		}
		else if((led_buzzer_info.led_status & 0xffff) >= (1 << LED_PANIC))
		{
			//printf("LED_PANIC\n");
			//处理定时的时间 
			if(1 == ((led_counter & (1 << (LED_PANIC+16)))>> (LED_PANIC+16)))
			{
				led_counter++;
			}
			else
			{
				led_counter = 0;
				led_counter = (1 << (LED_PANIC+16));
			}
			//根据状态处理LED的闪动
			if(0 == led_interval)
			{
				//led_red_on();
				led_interval = 1;
			}
			else if(1 == led_interval)
			{
				//led_red_off();
				led_interval = 0;
			}
			else
			{
				led_interval = 0; 
			}
				//定时到，进一步处理    需要关闭状态位
				//printf("led_buzzer_info.led_time = %d\n",led_buzzer_info.led_time);
				if((led_counter & 0xffff) >= (led_buzzer_info.led_time)*1000 / SLEEP_TIME)
				{
					led_interval 	= 0;
					led_counter 	= 0;
					pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
					led_buzzer_info.led_status &= ~(1<<LED_PANIC);
					pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);
				}
		}
		else if((led_buzzer_info.led_status & 0xffff) > (1 << LED_ALARM_TRIGGER)) 
		{
			if(1 == ((led_buzzer_info.led_status >> LED_ARM)&0x1)) //布防状态
			{
				//printf("LED_ALARM_TRIGGER\n");
			//处理定时的时间 
				if(1 == (led_counter & (1 << (LED_ALARM_TRIGGER+16)))>> (LED_ALARM_TRIGGER+16))
				{
					led_counter++;
				}
				else
				{
					led_counter = 0;
					led_counter = 1<<(LED_ALARM_TRIGGER+16);
				}
				//根据状态处理LED的闪动
				if(0 == led_interval)
				{
					//led_red_on();
					led_interval = 1;
				}
				else if(1 == led_interval)
				{
					//led_red_off();
					led_interval = 0;
				}
				else
				{
					led_interval = 0; 
				}
				//定时到，进一步处理    需要关闭状态位
				//printf("led_buzzer_info.led_time = %d\n",led_buzzer_info.led_time);
				if((led_counter & 0xffff) >= (led_buzzer_info.led_time)*1000 / SLEEP_TIME)
				{
					led_interval 	= 0;
					led_counter 	= 0;
					pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
					led_buzzer_info.led_status &= ~(1<<LED_ALARM_TRIGGER);
					//led_buzzer_info.led_status &= ~(1<< LED_ALARM_MONITOR);
					pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);
				}
			}
			else
			{
					pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
					led_buzzer_info.led_status &= ~(1<<LED_ALARM_TRIGGER);
					//led_buzzer_info.led_status &= ~(1<< LED_ALARM_MONITOR);
					pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);

			}
		}
		 //观看视频时 蓝灯常亮
		else if((led_buzzer_info.led_status & 0xffff) >= (1<<LED_MONITOR))
		{ 
			//printf("LED_MONITOR\n");
			//led_blue_on();
		}
		//布防时红灯常亮
		else if((led_buzzer_info.led_status & 0xffff) > (1<<LED_ARM))
		{
			//printf("LED_ARM\n");
			//led_red_on();	
		}
		//LED正常情况下 慢闪
		else if((led_buzzer_info.led_status & 0xffff) >= (1<<LED_NORMAL)) 
		{	
			//led_green_on();
		}
//下面处理蜂鸣器
		//printf("buzzer_status=0x%x\n",led_buzzer_info.buzzer_status);
		if(led_buzzer_info.buzzer_status >= (1<<BUZZER_TESTING)) //不管是否使能蜂鸣器都会想 (>=)
		{
			//printf("BUZZER_TESTING\n");
			if(1 == ((buzzer_counter & (1<<(BUZZER_TESTING +16)))>>(BUZZER_TESTING +16)))
			{
				buzzer_counter++;
			}
			else
			{
				buzzer_counter = 0;
				buzzer_counter = 1<<(BUZZER_TESTING+16);
				gpioEx_set(gpio_buz_out);	
			}
			//gpioEx_set(gpio_buz_out);	
		//定时到，进一步处理	需要关闭状态位
		//printf("led_buzzer_info.buzzer_time=%d %d\n",led_buzzer_info.buzzer_time,buzzer_counter);
			if((buzzer_counter & 0xffff)>= led_buzzer_info.buzzer_time *1000 / SLEEP_TIME)
			{
				buzzer_counter 	= 0;
				gpioEx_clear(gpio_buz_out);
				pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
				led_buzzer_info.buzzer_status &= ~(1<<BUZZER_TESTING);
				pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);
			}
		}
		else if(led_buzzer_info.buzzer_status >= (1<<BUZZER_ENABLE_BEEP)) //报警了 要响铃
		{
			//printf("BUZZER_ENABLE_BEEP\n");
			if(0 == (led_buzzer_info.buzzer_status & 0x1)) //md报警了 但是又撤防了
			{
				pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
				led_buzzer_info.buzzer_status &= ~(1<<BUZZER_ENABLE_BEEP);
				pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);

				buzzer_counter	= 0;
				gpioEx_clear(gpio_buz_out);
				continue;
			}
			if(1 == ((buzzer_counter & (1<<(BUZZER_ENABLE_BEEP +16)))>>(BUZZER_ENABLE_BEEP +16)))
			{
				buzzer_counter++;
			}
			else
			{
				buzzer_counter = 0;
				buzzer_counter = 1<<(BUZZER_ENABLE_BEEP+16);
				gpioEx_set(gpio_buz_out);	
			}
				//gpioEx_set(gpio_buz_out); 
			//定时到，进一步处理	需要关闭状态位
			if((buzzer_counter & 0xffff)>= led_buzzer_info.buzzer_time *1000 / SLEEP_TIME)
			{
				buzzer_counter	= 0;
				gpioEx_clear(gpio_buz_out);
				pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
				led_buzzer_info.buzzer_status &= ~(1<<BUZZER_ENABLE_BEEP);
				pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);
			}
		}

		usleep(SLEEP_TIME *1000); //100MS    如果要更改， 需要对应修改计数器的判断
	}
	printf("thread_led/buzzer exit!!\n");	
}

void *recordstatus_thread(void *args)
{
	static int status =-1;
	static int rec_flag = 0; 
	
	while(g_main_run)
	{	
		//如果当前为不录像时
		if(RECORD_NO == record_status.record_flags)
		{
			switch(record_status.record_last_status)
			{
				case  RECORD_FULLTIME:
				{
					printf("+++++++++++++++++++++++++++++RECORD_NO-----RECORD_FULLTIME++++++++++++++++++++++++\n");
					st_record_stop(SDK_RECORD_TYPE_MANU);
					pthread_mutex_lock(&record_status.record_lock);
					record_status.record_last_status	= RECORD_NO;
					pthread_mutex_unlock(&record_status.record_lock);
					adapt_record_osd(0);
				}break;
				case RECORD_ALARM:
				{
					printf("+++++++++++++++++++++++++++++RECORD_NO-----RECORD_ALARM++++++++++++++++++++++++\n");
					if(1 == record_status.record_alarm_opened)
					{
						st_record_stop(SDK_RECORD_TYPE_MOTION_PRE);
						pthread_mutex_lock(&record_status.record_lock);
						record_status.record_alarm_opened = 0;
						pthread_mutex_unlock(&record_status.record_lock);
					}
					pthread_mutex_lock(&record_status.record_lock);
					record_status.record_last_status	= RECORD_NO;
					pthread_mutex_unlock(&record_status.record_lock);
					adapt_record_osd(0);
				}break;
				default:break;	
			}
		}
		else if(RECORD_FULLTIME == record_status.record_flags) 
		{
			switch(record_status.record_last_status)
			{
				case  RECORD_NO:
				{
					printf("+++++++++++++++++++++++++++++RECORD_FULLTIME-----RECORD_NO++++++++++++++++++++++++\n");
					st_record_start(SDK_RECORD_TYPE_MANU,150); //150
					pthread_mutex_lock(&record_status.record_lock);
					record_status.record_last_status	= RECORD_FULLTIME;
					pthread_mutex_unlock(&record_status.record_lock);
					adapt_record_osd(1);
				}break;
				case RECORD_ALARM:
				{
					printf("+++++++++++++++++++++++++++++RECORD_FULLTIME-----RECORD_ALARM++++++++++++++++++++++++\n");
					if(1 == record_status.record_alarm_opened)
					{
						st_record_stop(SDK_RECORD_TYPE_MOTION_PRE);
						pthread_mutex_lock(&record_status.record_lock);
						record_status.record_alarm_opened = 0;
						pthread_mutex_unlock(&record_status.record_lock);
					}

					//st_record_start(SDK_RECORD_TYPE_MANU,record_status.record_alarm_time);
					st_record_start(SDK_RECORD_TYPE_MANU,150);//150
					pthread_mutex_lock(&record_status.record_lock);
					record_status.record_last_status	= RECORD_FULLTIME;
					pthread_mutex_unlock(&record_status.record_lock);
					adapt_record_osd(1);
				}break;
				default:break;	
			}
		}
		else if(RECORD_ALARM == record_status.record_flags)
		{
			switch(record_status.record_last_status)
			{
				case  RECORD_NO:
				{
					printf("++++++++++++++++++++++++++NOW:RECORD_ALARM-----LAST:RECORD_NO++++++++++++++++++++++++\n");
					/*if(0 != record_status.record_alarm_triger)
					{
						st_record_stop(SDK_RECORD_TYPE_MOTION_PRE);
					}*/
					pthread_mutex_lock(&record_status.record_lock);
					record_status.record_last_status	= RECORD_ALARM;
					pthread_mutex_unlock(&record_status.record_lock);
				}break;
				case RECORD_FULLTIME:
				{
					printf("+++++++++++++++++++++++++++++NOW:RECORD_ALARM-----LAST:RECORD_FULLTIME++++++++++++++++++++++++\n");
					st_record_stop(SDK_RECORD_TYPE_MANU);
					pthread_mutex_lock(&record_status.record_lock);
					record_status.record_last_status	= RECORD_ALARM;
					pthread_mutex_unlock(&record_status.record_lock);
				}break;
				case RECORD_ALARM:
				{
					//printf("+++++++++++++++++++++++++++++NOW:RECORD_ALARM-----LAST:RECORD_ALARM++++++++++++++++++++++++\n");
					if(1 == record_status.record_alarm_triger) //如果触发了报警
					{
						status = st_get_event_status();
						printf("status = %d\n",status);
						if(0 == status)
						{	
							//printf("record time=%d\n",record_status.record_alarm_time);
							st_record_start(SDK_RECORD_TYPE_MOTION_PRE,record_status.record_alarm_time);
							adapt_record_osd(1);
							pthread_mutex_lock(&record_status.record_lock);
							record_status.record_alarm_triger	|= (1<<16);
							record_status.record_alarm_opened 	= 1;
							pthread_mutex_unlock(&record_status.record_lock);
						}
					}
					else if(1 != record_status.record_alarm_triger)
					{
						if(0 == st_get_event_status())
						{
							adapt_record_osd(0);
							pthread_mutex_lock(&record_status.record_lock);
							record_status.record_alarm_triger	= 0 ;
							pthread_mutex_unlock(&record_status.record_lock);
						}
					}
				}break;
				default:break;	
			}
		}
		//if(0 == record_status.pthread_is_processing) break;

		usleep(1000*1000);
	}
	printf("record exit!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
}

//for record
unsigned int record_init(void)
{
	record_status.pthread_is_processing	= 1; 
	pthread_mutex_init(&record_status.record_lock,NULL);
	int ret = pthread_create(&(record_status.record_pthread_handle), NULL, &recordstatus_thread, NULL);
	if(ret < 0)
    {
        printf("pthread_create failed ret[%d]\n", ret);
		record_status.pthread_is_processing	= 0; 
        return -1;
    }
}

