#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>    
#include <sys/stat.h>
#include <sys/statfs.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <dirent.h>

#define DANAVIDEO_CLOUD_TEST

#include "libdanavideo/danavideo.h"
#include "libdanavideo/danavideo_cmd.h"
#ifdef DANAVIDEO_CLOUD_TEST
#include "libdanavideo/danavideo_cloud.h"
#endif
#include "libdanavideo/debug.h"
#include "dana.h"
#include "storage.h"
#include "main.h"
#include "gmlib.h"
#include "ntpclient.h"
#include "sdk_struct.h"
#include "streamlib.h"
#include "avi_typedef.h"

#include "../../libshmav/shm_common.h"
#include "../../libshmav/shm_write.h"
#include "../../libshmav/shm_read.h"

#define MEDIA_VIDEO_FRAME_SIZE  (300*1024)
#define MEDIA_AUDIO_FRAME_SIZE  12800

#define DANA_MAX_FILE_NUM  35  
#define MEDIA_FRAME_SIZE  (300*1024)

#define DIFF_ABS(x,y)				((x)>(y)?(x-y):(y-x))
#define MAKE_FOURCC(a,b,c,d)        (int)((a)|(b)<<8|(c)<<16|(d)<<24)


typedef struct _AV_File_List_
{
    int64_t uiStartTime;//
	int uitimestamp;
    char file_name[64];
    unsigned char playback;
} AV_File_List_DANA;
AV_File_List_DANA 	pDanaAvEvent;

typedef struct _DANA_AV_Client
{
    unsigned char 		bStopPlayBack;
    unsigned char 		bPausePlayBack;
    unsigned char 		playBackCh;
    pthread_rwlock_t 	sLock;
} DANA_AV_Client;
DANA_AV_Client gDanaClientInfo;

dana_ls_avs_ops g_dana_ls; //实时流

extern sdk_Network_Info g_network_info;
extern MotionControl  g_structMotionCtl;
extern ntp_conf g_ntp_pra;

extern RECORD_STATUS record_status;
extern int g_main_run;

static DANAAviPBHandle *gs_pPBHandle = NULL;
static int gsi_danale_recplayback_pause = 0;

static int  GET_DANA_SET_RECPLAN_CLICK = 0;//点击设置录像计划
static int  GET_DANA_SET_TIME_CLICK = 0;//点击设置录像计划

#define MSG_BUF_SIZE 	1024
#define _MSG_DEBUG		1

int parseSecurity(char *pEncstr, int *pEnc_type_i)
{
	if(strstr(pEncstr, "WPA2-PSK") != NULL)
	{
		if(strstr(pEncstr, "CCMP") != NULL)
			*pEnc_type_i = DANAVIDEO_WIFI_ENCTYPE_WPA2_AES;
		else if(strstr(pEncstr, "TKIP") != NULL)
			*pEnc_type_i = DANAVIDEO_WIFI_ENCTYPE_WPA2_TKIP;
	}
	else if(strstr(pEncstr, "WPA-PSK") != NULL)
	{
		if(strstr(pEncstr, "CCMP") != NULL)
			*pEnc_type_i = DANAVIDEO_WIFI_ENCTYPE_WPA_AES;
		else if(strstr(pEncstr, "TKIP") != NULL)
			*pEnc_type_i = DANAVIDEO_WIFI_ENCTYPE_WPA_TKIP;
	}
	else if(strstr(pEncstr, "WEP") != NULL)
		*pEnc_type_i = DANAVIDEO_WIFI_ENCTYPE_WEP;
	else
		*pEnc_type_i = DANAVIDEO_WIFI_ENCTYPE_NONE;

	return 0;
}

int get_wifi_AP_list(libdanavideo_wifiinfo_t *pApInfo, int iMaxCount, int *pOutcount)
{
	FILE *fp_scanning = NULL;
	char buffer[256] = {0};
	char *ptmp, *ptmp1;
	int isApinfoEnd=0;
	int isApinfoHead=0;
	int isAuthOn=0; 
	int sec_mode=0;
	int groupType=0;
	int count;
	
	fp_scanning = popen("iwlist wlan0 scanning", "r");
	if(fp_scanning == NULL)
	{
		return -1;
	}
	
	while(fgets(buffer, sizeof(buffer), fp_scanning) != NULL)
	{
		//printf("%s", buf);	usleep(10000);
		//it is a new Ap
		//Cell 01 - Address: D4:12:BB:08:CB:64
		if(((ptmp=strstr(buffer, "Address")) != NULL) && (isApinfoHead==0))
		{
			if(count >= iMaxCount)
				break;
			
			isApinfoHead = 1;
		}
		else if((ptmp=strstr(buffer, "ESSID")) != NULL)
		{	
			ptmp += strlen("ESSID:");
			ptmp++;//skipped the \"  
			ptmp1 = strchr(ptmp, '"');
			if(ptmp1 != NULL) *ptmp1='\0';//replace \n to \0
			snprintf(pApInfo[count].essid, 33, "%s", ptmp);
		}
		else if((ptmp=strstr(buffer, "IE:")) != NULL)
		{
			if((ptmp=strstr(buffer, "WEP")) != NULL)
				sec_mode |= 0x1;
			if((ptmp=strstr(buffer, "WPA")) != NULL)
				sec_mode |= 0x2;
			if((ptmp=strstr(buffer, "WPA2")) != NULL)
				sec_mode |= 0x4;
		}
		else if((ptmp=strstr(buffer, "Encryption key:on")) != NULL)
		{
			isAuthOn = 1;
		}
		else if((ptmp=strstr(buffer, "Encryption key:off")) != NULL)
		{
			isAuthOn = 0;
		}
		else if((ptmp=strstr(buffer, "Group Cipher :")) != NULL)
		{
			if((ptmp=strstr(buffer, "CCMP")) != NULL)
				groupType |= 0x1;
			if((ptmp=strstr(buffer, "TKIP")) != NULL)
				groupType |= 0x2;
		}
		else if((ptmp=strstr(buffer, "Quality")) != NULL)
		{
			ptmp += strlen("Quality=");
			ptmp1 = strchr(ptmp, '/');
			if(ptmp1 != NULL) *ptmp1='\0';//replace '/' to \0
			sscanf(ptmp, "%d", &(pApInfo[count].quality));

			isApinfoEnd = 1;
		}
		
		if(isApinfoEnd > 0)		
		{			
			if(isAuthOn == 0)	
				pApInfo[count].enc_type = DANAVIDEO_WIFI_ENCTYPE_NONE;
			if((isAuthOn > 0) && (sec_mode == 0x1))
				pApInfo[count].enc_type = DANAVIDEO_WIFI_ENCTYPE_WEP;
			if((isAuthOn > 0) && (sec_mode & 0x2) && (groupType & 0x1))
				pApInfo[count].enc_type = DANAVIDEO_WIFI_ENCTYPE_WPA_AES;
			if((isAuthOn > 0) && (sec_mode & 0x2) && (groupType & 0x2))
				pApInfo[count].enc_type = DANAVIDEO_WIFI_ENCTYPE_WPA_TKIP;
			if((isAuthOn > 0) && (sec_mode & 0x4) && (groupType & 0x1))
				pApInfo[count].enc_type = DANAVIDEO_WIFI_ENCTYPE_WPA2_AES;
			if((isAuthOn > 0) && (sec_mode & 0x4) && (groupType & 0x2))
				pApInfo[count].enc_type = DANAVIDEO_WIFI_ENCTYPE_WPA2_TKIP;

			//reset encropytion type params, and end reading an ap's info
			isAuthOn = 0;
			sec_mode = 0;
			groupType = 0;
			isApinfoEnd = 0;// read a new ap's info over
			isApinfoHead = 0;
			count++;
		}
	}
	*pOutcount = count;

	if(fp_scanning)
		pclose(fp_scanning);

	return 0;
}


int get_wifi_config(char *ssid_buf, int ssid_Len, char *pwd_buf, int pwd_Len)
{
	FILE *fp_info;
	char pData[64];
	char *str;
	char *p;
	
	if(access(WIFI_CONFIG_INI, F_OK) != 0)
	{
		printf("the %s is not exist!\n", WIFI_CONFIG_INI);
		return -1;
	}

	fp_info = fopen(WIFI_CONFIG_INI, "r");
	if(fp_info != NULL)
	{
		while(fgets(pData, sizeof(pData), fp_info) != NULL)
		{
			p=pData;
			while(*p != '\0')
			{
				if(*p == '\n')
					*p = '\0';
				p++;
			}
			if((str=strstr(pData, "SSID=")) != NULL)
			{
				str += strlen("SSID=");
				snprintf(ssid_buf, ssid_Len, "%s", str);
				//ssid_buf[strlen(ssid_buf)] = '\0';
			}
			else if((str=strstr(pData, "PASSWORD=")) != NULL)
			{
				str += strlen("PASSWORD=");
				snprintf(pwd_buf, pwd_Len, "%s", str);
				//pwd_buf[strlen(pwd_buf)] = '\0';
			}
		}
		fclose(fp_info);
	}

	return 0;
}

int set_wifi_config(const char *my_ssid, const char *my_pwd)
{
	FILE *fp_info;
	char pre_ssid[32]={0};
	char pre_psk[32]={0};
	int state;

	state = get_wifi_config(pre_ssid, sizeof(pre_ssid), pre_psk, sizeof(pre_psk));
	if(state == 0 && (!strcmp(pre_ssid, my_ssid)) && (!strcmp(pre_psk, my_pwd)))
	{
		return 0;
	}

	fp_info = fopen(WIFI_CONFIG_INI, "w+");
	if(fp_info == NULL)
	{
		printf("write wifi config info fail: %s", WIFI_CONFIG_INI);
		return -1;
	}

	fprintf(fp_info, "SSID=%s\n", my_ssid);
	fprintf(fp_info, "PASSWORD=%s\n", my_pwd);

	fclose(fp_info);
	return 0;
}

int adjust_video_bps(int quality, int *pbitrate, int *pframerate, int *presolution)
{
	int kbps, fps, res;
	
	if(quality > 100)
		quality = 100;
	if(quality < 1)
		quality = 1;

	if(quality > 75)
	{
		res = SDK_VIDEO_RESOLUTION_960p; //only for sc1135 
		fps = 5 + (quality-75)*20/25;
		if(fps > 25)
			fps = 25;
		kbps = 100 * fps;
	}
	else if(quality > 50)
	{
		res = SDK_VIDEO_RESOLUTION_720p;
		fps = 5 + (quality-50)*20/25;
		if(fps > 25)
			fps = 25;
		kbps = 75 * fps;
	}
	else if(quality > 25)
	{
		res = SDK_VIDEO_RESOLUTION_VGA;
		fps = 5 + (quality-25)*20/25;
		if(fps > 25)
			fps = 25;
		kbps = 20 * fps;
	}
	else
	{
		res = SDK_VIDEO_RESOLUTION_QVGA;
		fps = 5 + quality;
		if(fps > 25)
			fps = 25;
		kbps = 6.4 * fps;
	}
	*pbitrate = kbps;
	*pframerate = fps;
	*presolution = res;
}

int dana_enc_default_set(int video_quality, int video_channel)
{
	int bitrate, framerate, resolution;
	char msg_buf[1024] = {0};
				
	sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
	sdk_encode_t *enc_param = (sdk_encode_t *)pMsg->data;
	DANA_MSG_CTRL(SDK_MAIN_MSG_ENCODE_CFG,SDK_PARAM_GET,0,0,pMsg);
	printf("^^^^^ resolution = %d\n", enc_param->av_enc_info[video_channel].resolution);

	adjust_video_bps(video_quality, &bitrate, &framerate, &resolution);
	
	if(enc_param->av_enc_info[video_channel].resolution == resolution)
		return 0;
	
	enc_param->av_enc_info[video_channel].bitrate = bitrate;
	enc_param->av_enc_info[video_channel].frame_rate = framerate;
	enc_param->av_enc_info[video_channel].resolution = resolution;
	pMsg->size = sizeof(sdk_encode_t);
				
	DANA_MSG_CTRL(SDK_MAIN_MSG_ENCODE_CFG,SDK_PARAM_SET,0,0,pMsg);
}


#define MAX_TIMEZONE_COUNT 32
typedef struct tag_timezone_info
{
	int index;
	char gmtstr[12];
	char zonestr[24];
}Danale_Timezone_Info_Tag;

const Danale_Timezone_Info_Tag danale_timezone_info[MAX_TIMEZONE_COUNT] = 
{
	{0,  "GMT-12:00", "Pacific/Kwajalein"},
	{1,  "GMT-11:00", "Pacific/Samoa"},
	{2,  "GMT-10:00", "US/Hawaii"},
	{3,  "GMT-09:00", "US/Alaska"},
	{4,  "GMT-08:00", "America/Los_Angeles"},
	{5,  "GMT-07:00", "US/Arizona"},
	{6,  "GMT-06:00", "America/Monterrey"},
	{7,  "GMT-05:00", "America/Bogota"},
	{8,  "GMT-04:00", "America/Caracas"},
	{9,  "GMT-03:30", "Canada/Newfoundland"},
	{10, "GMT-03:00", "America/Buenos_Aires"},
	{11, "GMT-02:00", "Atlantic/South_Georgia"},
	{12, "GMT-01:00", "Atlantic/Azores"},
	{13, "GMT-00:00", "Europe/London"},
	{14, "GMT+01:00", "Europe/Rome"},
	{15, "GMT+02:00", "Africa/Cairo"},
	{16, "GMT+03:00", "Asia/Baghdad"},
	{17, "GMT+03:30", "Asia/Tehran"},
	{18, "GMT+04:30", "Asia/Baku"},
	{19, "GMT+04:30", "Asia/Kabul"},
	{20, "GMT+05:00", "Asia/Tashkent"},
	{21, "GMT+05:30", "Asia/Kolkata"},
	{22, "GMT+05:45", "Asia/Kathmandu"},
	{23, "GMT+06:00", "Asia/Almaty"},
	{24, "GMT+06:30", "Asia/Rangoon"},
	{25, "GMT+07:00", "Asia/Bangkok"},
	{26, "GMT+08:00", "Asia/Shanghai"},
	{27, "GMT+09:00", "Asia/Tokyo"},
	{28, "GMT+10:00", "Australia/Canberra"},
	{29, "GMT+11:00", "Asia/Magadan"},
	{30, "GMT+12:00", "Pacific/Auckland"}
};

int dana_update_timezone(char *ptime_zone)
{
	int idx;
	char cmd[64]={0};
	char TZstring[24] = {0};
	
	for(idx = 0; idx <=MAX_TIMEZONE_COUNT; idx++)
	{
		if(strcmp(danale_timezone_info[idx].gmtstr, ptime_zone) == 0)
		{
			strncpy(TZstring, danale_timezone_info[idx].zonestr, sizeof(TZstring));
			break;
		}
	}
	printf("Now TZ is %s\n", TZstring);

	snprintf(cmd, sizeof(cmd), "/mnt/mtd/tz.sh %s &", TZstring);
	system_ex(cmd);
	
	return 0;
}


int danale_util_pullmsg()
{
	uint32_t alarm_level = 2;
	uint32_t msg_type = 1;//DANA_VIDEO_PUSHMSG_MSG_TYPE_MOTION_DETECT;
	char	 *msg_title = "TEST danavideo_alarm_cloud";
	char	 *msg_body	= "lib_danavideo_util_pushmsg danavideo_alarm_cloud";
	int64_t  cur_time = time(NULL);
	uint32_t att_flag = 0;
	int64_t  start_time = time(NULL);

	uint32_t chan_no = 1;
	uint32_t save_site = 0;
	uint32_t record_flag = 0;
	if (lib_danavideo_util_pushmsg(chan_no, alarm_level, msg_type, msg_title, msg_body, cur_time, att_flag, NULL, NULL, record_flag, 0, 0, 0, NULL)) {
	printf("\x1b[32mtestdanavideo TEST lib_danavideo_util_pushmsg success\x1b[0m\n");
	} else {
	printf("\x1b[34mtestdanavideo TEST lib_danavideo_util_pushmsg failed\x1b[0m\n");
	}
}

unsigned int danale_gettickcount()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return 1000*(ts.tv_sec - 100000) + ts.tv_nsec/1000000;
}

int dana_get_realtime_play_chnn()
{
	return g_dana_ls.encode_current_chn;
}


#ifdef DANAVIDEO_CLOUD_TEST
volatile bool lib_danavideo_cloud_inited = false;

typedef struct _danavideo_cloud_realtime_upload_core_s {
    volatile bool run_realtime_upload;
    pthread_t     thread_realtime_upload; // 模拟多chan上传
} danavideo_cloud_realtime_upload_core_t;

typedef struct _danavideo_cloud_alarm_upload_core_s {
    volatile bool run_alarm_upload;
    pthread_t     thread_alarm_upload; 
} danavideo_cloud_alarm_upload_core_t;

danavideo_cloud_realtime_upload_core_t realtimeuploadcore;
danavideo_cloud_alarm_upload_core_t alarmuploadcore;

typedef struct tag_danale_cloud_media_info{
	unsigned long long ullMediaVideo_ms;
	unsigned long long ullMediaAudio_ms;

	int iCloud_ShmVID;
	int iCloud_ShmAID;
}dana_cloud_media_info;

int cloud_media_stream_init(dana_cloud_media_info *p_dana_cloud_media)
{
	int ret;
	SHM_VideoInitID(SHM_ENUM_VIDEO_STREAM0_PROFILE, &p_dana_cloud_media->iCloud_ShmVID);
	SHM_AudioInitID(SHM_ENUM_AUDIO_16K_PROFILE, &p_dana_cloud_media->iCloud_ShmAID);
	
	printf("Cloud Storage Video StreamID = %d\n", p_dana_cloud_media->iCloud_ShmVID);
	printf("Cloud Storage Audio StreamID = %d\n", p_dana_cloud_media->iCloud_ShmAID);

	return 0;
}

void cloud_media_stream_release(dana_cloud_media_info *p_dana_cloud_media)
{
	SHM_VideoRemoveID(p_dana_cloud_media->iCloud_ShmVID);
	SHM_AudioRemoveID(p_dana_cloud_media->iCloud_ShmAID);
}

void* th_realtime_upload(void *arg)
{
    danavideo_cloud_realtime_upload_core_t *realtimeuploadcore = (danavideo_cloud_realtime_upload_core_t *)arg;

    uint32_t timeout_usec = 1000*1000; // 1000ms
    unsigned int timestamp, media_type, isKeyFrame;
    unsigned int tick0;
    unsigned int timestamp_base = 0;

    tick0 = danale_gettickcount();

    uint32_t codec;
    danavideo_cloud_alarm_t alarm = 0;
    int32_t alarm_trigger = 0;

    uint32_t chan_no = 1;

	ShmVideoExtraData struVExtra;
    ShmAudioExtraData struAExtra;
    char tmpbuf[SHM_VIDEO_STREAM_720P_MAX_FRAME_SIZE];
	int framelen=0;
	int	iNoNewFrame;
	unsigned long long ullvtstamp, ullatstamp;

	dana_cloud_media_info cloud_media_info;
	memset(&cloud_media_info, 0, sizeof(cloud_media_info));
	
	cloud_media_stream_init(&cloud_media_info);
	int shmVid = cloud_media_info.iCloud_ShmVID;
	int shmAid = cloud_media_info.iCloud_ShmAID;

    while (realtimeuploadcore->run_realtime_upload) 
	{
		/*while ((danale_gettickcount() - tick0) < timestamp) {
                usleep(1*1000); // sleep 1 ms
        }*/

	    framelen=sizeof(tmpbuf);  
	    if(SHM_VideoReadFrameWithExtras(shmVid, tmpbuf, &framelen, (char*)&struVExtra) > 1)
	    {
			cloud_media_info.ullMediaVideo_ms = struVExtra.ullTimeStamp;
			if(struVExtra.ucKeyFrameFlag == 1)
				isKeyFrame = 1;
			else
				isKeyFrame = 0;

			media_type = video_stream;
	        codec = H264;
	        if (lib_danavideo_cloud_realtime_upload(chan_no, media_type, codec, isKeyFrame, timestamp + timestamp_base, alarm, (const char*)tmpbuf, framelen, timeout_usec)) {
				//printf("cloud_realtime_upload video data success!\n");
	        } else {
	            printf("th_realtime_upload[%u] lib_danavideo_cloud_realtime_upload failed\n", chan_no);
	            usleep(500*1000);
	        }
		}
		else
		{
			iNoNewFrame++;
		}
	    // write audio frame, maybe more than one
	    while(cloud_media_info.ullMediaAudio_ms < cloud_media_info.ullMediaVideo_ms)
	    {
	        framelen=sizeof(tmpbuf);
			if(SHM_AudioReadFrameWithExtras(shmAid, tmpbuf, &framelen, (char *)&struAExtra)>1)
			{
				cloud_media_info.ullMediaAudio_ms = struAExtra.ullTimeStamp;
				media_type = audio_stream;
                codec = G711A;
		        if (lib_danavideo_cloud_realtime_upload(chan_no, media_type, codec, isKeyFrame, timestamp + timestamp_base, alarm, (const char*)tmpbuf, framelen, timeout_usec)) {
					//printf("cloud_realtime_upload audio data success!\n");
		        } else {
		            printf("th_realtime_upload[%u] lib_danavideo_cloud_realtime_upload failed\n", chan_no);
		            usleep(500*1000);
		        }
			}
			else
			{
				iNoNewFrame++;
			    break;
			}
	    }

		if(iNoNewFrame == 2)
			usleep(20000);
    }
    
fin:
	cloud_media_stream_release(&cloud_media_info);
    printf("th_realtime_upload exit\n");
	
    return NULL;
}

static bool alarm_cloud_last_file_uploaded = false;
static char alarm_cloud_last_file_name[128] = {0};

void alarm_cloud_upload_callback(const int8_t retcode, const char *file_name, const char *file_path)
{
    printf("\x1b[32mTEST retcode: %d\x1b[0m\n", retcode);
    printf("\x1b[32mTEST file_name: %s\x1b[0m\n", file_name);
    printf("\x1b[32mTEST file_path: %s\x1b[0m\n", file_path);

    char cmd[256];
    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "rm -rf %s", file_name);
    if (-1 == system(cmd)) {
        printf("th_alarm_upload system(\"%s\") failed\n", cmd);
    }

    if (0 == strcmp(file_name, alarm_cloud_last_file_name)) {
        printf("\x1b[32mTEST DONE\x1b[0m\n");
        // 允许下一个告警文件上传
        alarm_cloud_last_file_uploaded = true;
    } else {
        printf("\x1b[32mTEST DO NOTHING\x1b[0m\n");
    }
}

void *th_alarm_upload(void *arg) 
{
    danavideo_cloud_alarm_upload_core_t *alarmuploadcore = (danavideo_cloud_alarm_upload_core_t *)arg;

    // 定时生成一个告警文件(不能重名,否则会覆盖云存储服务器上同名文件)
    // 某个文件上传完毕就删除它
    // 控制所有文件占用的空间(如果空间允许,可以同时进行多个告警文件的上传)
    // TODO 使用pushmsg

    time_t cur_time;
    cur_time = time(NULL);
    uint32_t file_name_index = 0;
    
    uint32_t chan_no = 1;
    char *file_path = "./";
    char file_name[128];

    char cmd[256];

    char save_path[512];
    uint32_t save_site = 0;
    
    alarm_cloud_last_file_uploaded = true; // 新的开始 

    while (alarmuploadcore->run_alarm_upload) 
	{
        sleep(3);
        if (!alarm_cloud_last_file_uploaded) {
           // 上一个告警文件还没有上传完毕,等待
           continue;
        } 
        memset(file_name, 0, sizeof(file_name));
        snprintf(file_name, sizeof(file_name), "%s_%lu_%u.mp4", "danalevideocloud_alarm", cur_time, file_name_index++); 
        printf("th_alarm_upload: %s\n", file_name);
        memset(cmd, 0, sizeof(cmd));
        sprintf(cmd, "ln -s /mnt/nfs/h264test/0_alarm.mp4 %s", file_name);
        if (-1 == system(cmd)) {
            printf("th_alarm_upload system(\"%s\") failed\n", cmd);
        } else {
            // 保留当前告警模式正在上传的文件名
            memset(alarm_cloud_last_file_name, 0, sizeof(alarm_cloud_last_file_name));
            strcpy(alarm_cloud_last_file_name, file_name);

            memset(save_path, 0, sizeof(save_path));
            if (lib_danavideo_cloud_customfile_async_upload_pre(chan_no, file_name, file_path, &save_site, save_path, sizeof(save_path))) {
                printf("th_alarm_upload lib_danavideo_cloud_customfile_async_upload_pre succeeded\n");
                printf("\tsave_site: %d\n", save_site);
                printf("\tsave_path: %s\n", save_path);
                // 先告警
                // 不设置附件. 设置录像
                uint32_t alarm_level = DANA_VIDEO_PUSHMSG_ALARM_LEVEL_2;
                uint32_t msg_type = DANA_VIDEO_PUSHMSG_MSG_TYPE_OTHER;
                char     *msg_title = "TEST danavideo_alarm_cloud";
                char     *msg_body  = "lib_danavideo_util_pushmsg danavideo_alarm_cloud";
                int64_t  cur_time = time(NULL);
                uint32_t att_flag = 0;
                uint32_t record_flag = 1;
                int64_t  start_time = time(NULL);
                uint32_t time_len = 60;
                if (lib_danavideo_util_pushmsg(chan_no, alarm_level, msg_type, msg_title, msg_body, cur_time, att_flag, NULL, NULL, record_flag, start_time, time_len, save_site, save_path)) {
                    printf("\x1b[32mtestdanavideo TEST lib_danavideo_util_pushmsg success\x1b[0m\n");
                } else {
                    printf("\x1b[34mtestdanavideo TEST lib_danavideo_util_pushmsg failed\x1b[0m\n");
                }
                // 再上传
                if (lib_danavideo_cloud_customfile_async_upload(chan_no, file_name, file_path, alarm_cloud_upload_callback)) {
                    printf("th_alarm_upload lib_danavideo_cloud_customfile_async_upload succeeded\n");
                    alarm_cloud_last_file_uploaded = false;
                } else {
                    printf("th_alarm_upload lib_danavideo_cloud_customfile_async_upload failed\n");
                    memset(cmd, 0, sizeof(cmd));
                    sprintf(cmd, "rm -rf %s", file_name);
                    if (-1 == system(cmd)) {
                        printf("th_alarm_upload system(\"%s\") failed\n", cmd);
                    }

                    memset(alarm_cloud_last_file_name, 0, sizeof(alarm_cloud_last_file_name));

                    alarm_cloud_last_file_uploaded = true; 
                }

            } else {
                printf("th_alarm_upload lib_danavideo_cloud_customfile_async_upload_pre failed\n");
                memset(cmd, 0, sizeof(cmd));
                sprintf(cmd, "rm -rf %s", file_name);
                if (-1 == system(cmd)) {
                    printf("th_alarm_upload system(\"%s\") failed\n", cmd);
                }
                
                memset(alarm_cloud_last_file_name, 0, sizeof(alarm_cloud_last_file_name));

                alarm_cloud_last_file_uploaded = true;
            }
        }
    }

    printf("th_alarm_upload exit\n");
    return NULL;
}


// 实时上传模式
bool danavideo_cloud_enter_realtime_upload()
{
    if (realtimeuploadcore.run_realtime_upload) {
        printf("\x1b[33mdanavideo_cloud_enter_realtime_upload already started\x1b[0m\n");
        return true;
    }

    if (!lib_danavideo_cloud_realtime_on()) {
        printf("\x1b[33mdanavideo_cloud_enter_realtime_upload lib_danavideo_cloud_realtime_on failed\x1b[0m\n");
        return false;
        // XXX 这里应该重试,直至成功
    }

    realtimeuploadcore.run_realtime_upload = true;
    if (0 != pthread_create(&(realtimeuploadcore.thread_realtime_upload), NULL, &th_realtime_upload, &realtimeuploadcore)) {
        memset(&(realtimeuploadcore.thread_realtime_upload), 0, sizeof(realtimeuploadcore.thread_realtime_upload));
        printf("\x1b[33mdanavideo_cloud_enter_realtime_upload can't create th_realtime_upload\x1b[0m\n");
        realtimeuploadcore.run_realtime_upload = false;
        return false;
    } else {
        printf("\x1b[33mdanavideo_cloud_enter_realtime_upload th_realtime_upload is started, enjoy!\x1b[0m\n");
        return true;
    }
}

bool danavideo_cloud_leave_realtime_upload()
{
    if (!realtimeuploadcore.run_realtime_upload) {
        printf("\x1b[33mdanavideo_cloud_leave_realtime_upload already stopped\x1b[0m\n");
        return true;
    }
    realtimeuploadcore.run_realtime_upload = false;
    if (0 != realtimeuploadcore.thread_realtime_upload) {
        if (0 != pthread_join(realtimeuploadcore.thread_realtime_upload, NULL)) {
            perror("danavideo_cloud_leave_realtime_upload th_realtime_upload join failed");
        }
    }
    memset(&(realtimeuploadcore.thread_realtime_upload), 0, sizeof(realtimeuploadcore.thread_realtime_upload));

    lib_danavideo_cloud_realtime_off();

    printf("\x1b[33mdanavideo_cloud_leave_realtime_upload\x1b[0m\n");
    return true;
}

// 告警上传模式
bool danavideo_cloud_enter_alarm_upload()
{
    if (alarmuploadcore.run_alarm_upload) {
        printf("\x1b[33mdanavideo_cloud_enter_alarm_upload already started\x1b[0m\n");
        return true;
    } 
    alarmuploadcore.run_alarm_upload = true;
    if (0 != pthread_create(&(alarmuploadcore.thread_alarm_upload), NULL, &th_alarm_upload, &alarmuploadcore)) {
        memset(&(alarmuploadcore.thread_alarm_upload), 0, sizeof(alarmuploadcore.thread_alarm_upload));
        printf("\x1b[33mdanavideo_cloud_enter_alarm_upload can't create th_alarm_upload\x1b[0m\n");
        alarmuploadcore.run_alarm_upload = false;
        return false;
    } else {
        printf("\x1b[33mdanavideo_cloud_enter_alarm_upload th_alarm_upload is started, enjoy\x1b[0m\n");
        return true;
    }
}

bool danavideo_cloud_leave_alarm_upload()
{
    if (!alarmuploadcore.run_alarm_upload) {
        printf("\x1b[33mdanavideo_cloud_leave_alarm_upload already stopped\x1b[0m\n");
        return true;
    }
    alarmuploadcore.run_alarm_upload = false;
    if (0 != alarmuploadcore.thread_alarm_upload) {
        if (0 != pthread_join(alarmuploadcore.thread_alarm_upload, NULL)) {
            perror("danavideo_cloud_leave_alarm_upload th_alarm_upload join failed");
        }
    }
    memset(&(alarmuploadcore.thread_alarm_upload), 0, sizeof(alarmuploadcore.thread_alarm_upload));

    printf("\x1b[33mdanavideo_cloud_leave_alarm_upload\x1b[0m\n");
    return true;
}


// 主要在该回调函数中完成云存储的开启和关闭及模式切换
void danavideo_cloud_mode_changed_callback(const danavideo_cloud_mode_t cloud_mode)
{
    switch (cloud_mode) {
        case DANAVIDEO_CLOUD_MODE_UNKNOWN:
            {
                printf("danavideo_cloud_mode_changed_callback DANAVIDEO_CLOUD_MODE_UNKNOWN\n");
                // 退出实时/告警上传模式
                danavideo_cloud_leave_realtime_upload();
                danavideo_cloud_leave_alarm_upload();
            }
            break;
        case DANAVIDEO_CLOUD_MODE_REALTIME:
            {
                printf("danavideo_cloud_mode_changed_callback DANAVIDEO_CLOUD_MODE_REALTIME\n");
                // 进入实时上传模式
                danavideo_cloud_leave_alarm_upload();
                danavideo_cloud_enter_realtime_upload();
            }
            break;
        case DANAVIDEO_CLOUD_MODE_ALARM:
            {
                printf("danavideo_cloud_mode_changed_callback DANAVIDEO_CLOUD_MODE_ALARM\n");
                // 进入告警上传模式
                danavideo_cloud_leave_realtime_upload();
                danavideo_cloud_enter_alarm_upload();
            }
            break;
        default:
            printf("danavideo_cloud_mode_changed_callback should never\n");
            return;
    }
}
#endif


#ifdef DANAVIDEO_DEVICE_NO_MIX_MULTI_CHANNEL 
    #define NO_MIX_MULTI_CHANNEL_NUM 20
#elif defined(DANAVIDEO_DEVICE_NO_MIX_NO_MULTI_CHANNEL)
    #define NO_MIX_MULTI_CHANNEL_NUM 6
#endif

void* th_talkback(void *arg) {
    MYDATA *mydata = (MYDATA *)arg;
	
	g_dana_ls.play_start(NULL,1);  
	
    uint32_t timeout_usec = 1000*1000; // 1000ms
    while (mydata->run_talkback)
    {
        dana_audio_packet_t *pmsg = lib_danavideoconn_readaudio(mydata->danavideoconn, timeout_usec);
        if (pmsg) {
            //printf("th_talkback read a audio frame\n");
            //printf("\t\x1b[31mth_talkback audio frame codec is %d\x1b[0m\n", pmsg->codec);
			adapt_audio_play(0,pmsg->data,pmsg->len);
            lib_danavideo_audio_packet_destroy(pmsg);
        } else {
            usleep(10*1000); // 10ms
        }
    }

    printf("th_talkback exit\n");
    mydata->exit_talback = true;
	g_dana_ls.play_stop(NULL,1);
    pthread_exit(0);	
    return NULL;
}

void* th_audio_media(void *arg)
{
    int ret;
	dana_media_info			real_meadia;

    unsigned int	cur_timestamp=0;
	char *audio_buffer = NULL;
	sdk_frame_t *frame_head = NULL;
	
    MYDATA *mydata = (MYDATA *)arg;
	BYTE *pBuff = NULL;
    uint32_t timeout_usec = 1000*1000;// 1000ms

    unsigned int timestamp, media_type, size, isKeyFrame;
    unsigned int tick0;
    unsigned int timestamp_base = 0;

    //g_dana_ls.open_audio(&(real_meadia) , 0, 0, NULL);

	ShmAudioExtraData structShmAudioExtraData;
	int shmid;
	int rtn;

	SHM_AudioInitID(SHM_ENUM_AUDIO_16K_PROFILE, &shmid);
	printf("a_shm_id=%d\n", shmid);

	pBuff = (BYTE*) calloc(1, MEDIA_AUDIO_FRAME_SIZE);
    if (NULL == pBuff) {
        dbg("th_audio_media cant't calloc pBuff\n");
        goto fin;
    }
    tick0 = danale_gettickcount();

    while(mydata->run_audio_media)
    {
		memset(&structShmAudioExtraData, 0, sizeof(structShmAudioExtraData));
		memset(pBuff, 0, MEDIA_AUDIO_FRAME_SIZE);
		size=MEDIA_AUDIO_FRAME_SIZE;
		rtn=SHM_AudioReadFrameWithExtras(shmid, pBuff, &size, (char*)&structShmAudioExtraData);
		if(rtn > 0)
		{
			//timestamp = structShmAudioExtraData.ullTimeStamp;
		}
		else
		{
			//printf("no frame data in shm!\n");
			continue;
		}
		if (lib_danavideoconn_send(mydata->danavideoconn, audio_stream, G711A, mydata->chan_no, isKeyFrame, timestamp + timestamp_base, (const char*)pBuff, size, timeout_usec)) 
		{
			//dbg("th_audio_media send audio data [%u] succeeded\n", size);
		} 
		else 
		{
			dbg("th_audio_media send audio data [%u] failed\n", size);
			usleep(200*1000);
			continue;
		}
	#if 0
        if(g_dana_ls.pull_audio(&(real_meadia), 0,&audio_buffer)< 0)
        {
            usleep(10);
            continue;
        }
		frame_head = (frame_t*)(audio_buffer);
		{
            if (lib_danavideoconn_send(mydata->danavideoconn, audio_stream, G711A, mydata->chan_no, isKeyFrame, timestamp + timestamp_base,(const char *)frame_head->data, frame_head->frame_size, timeout_usec)) 
			{
            } 
			else 
			{
                printf("th_audio_media send test file [%u] failed\n", size);
                usleep(200*1000);
                continue;
            }
        }
	#endif
    }
fin:
	if (pBuff) {
		free(pBuff);
	}
	SHM_AudioRemoveID(shmid);
	
	dbg("th_audio_media exit\n");
	mydata->exit_audio_media = true;
	return NULL;

#if	0
    printf("leaving thread_AudioFrameData .....\n");
	g_dana_ls.close_audio(&(real_meadia),0);
    printf(">>>>>>>>>th_audio_media exit\n");
    mydata->exit_audio_media = true;
	
	return NULL;
#endif
}

void* th_media(void *arg)
{
#define ONE_FRAME_BUF_SIZE 500000

    MYDATA *mydata = (MYDATA *)arg;
#if 0
    int				counter = 0;
    int			sid = (int)arg;
    unsigned int 	cur_timestamp=0;
    BOOL			bFirstRead = true;
	char *frame_buffer = NULL;
	sdk_frame_t *frame_head = NULL;
	dana_media_info			real_meadia;

    int 			need_send_Iframe = 1;
#endif	
    uint32_t timeout_usec = 1000*1000; // 1000ms
	BYTE *pBuff = NULL;
    unsigned int timestamp,size, isKeyFrame;
    unsigned int tick0;
    unsigned int timestamp_base = 0;

    tick0 = danale_gettickcount();
	int shmid;
	int rtn;

	ShmVideoExtraData struVExtra;
	SHM_VideoInitID(SHM_ENUM_VIDEO_STREAM0_PROFILE, &shmid);
	printf("shmid=%d\n", shmid);

	pBuff = (BYTE*) calloc(1, MEDIA_VIDEO_FRAME_SIZE);
    if (NULL == pBuff) {
        dbg("th_media cant't calloc pBuff\n");
        goto fin;
    }
#if 0
	printf("--------thread_VideoFrameData open[chn%d]--------------\n",g_dana_ls.encode_current_chn);
  	g_dana_ls.open(&(real_meadia) , g_dana_ls.encode_current_chn, 0, NULL);
#endif	 
    while(mydata->run_media)
    {
	#if 0
		if(g_dana_ls.pull(&(real_meadia), g_dana_ls.encode_current_chn,&frame_buffer)< 0)
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
		isKeyFrame = (frame_head->frame_type == SDK_VIDEO_FRAME_I) ?1:0;
      
        if(frame_head->frame_size > ONE_FRAME_BUF_SIZE)
        {
            need_send_Iframe = 1;
			printf("ONE_FRAME_BUF_SIZE :512*1024 \n");
            continue;
        }
		//音视频同步 只有再打开音频时才运行
		if (lib_danavideoconn_send(mydata->danavideoconn, video_stream, H264, mydata->chan_no, isKeyFrame, timestamp + timestamp_base, (const char*)frame_head->data,  frame_head->frame_size, timeout_usec)) 
		{
			//printf("th_media send data [%u] succeeded\n", size);
		} 
		else 
		{
			printf("dana_playback send data [%u] failed\n", size);
			usleep(200*1000);
			continue;
		}
	#endif	
		memset(pBuff, 0, MEDIA_VIDEO_FRAME_SIZE);
		size=MEDIA_VIDEO_FRAME_SIZE;
		rtn = SHM_VideoReadFrameWithExtras(shmid, pBuff, &size, (char*)&struVExtra);
		if(rtn >= 2)
		{
			//timestamp = struVExtra.ullTimeStamp;
			if(rtn > 2)
				isKeyFrame = 1;
			else
				isKeyFrame = 0;
		}
		else
		{
			//printf("no frame data in shm!\n");
			continue;
		}
		if (lib_danavideoconn_send(mydata->danavideoconn, video_stream, H264, mydata->chan_no, isKeyFrame, timestamp + timestamp_base, (const char*)pBuff, size, timeout_usec)) 
		{
			//dbg("th_media send data [%u] succeeded\n", size);
		} 
		else 
		{
			printf("th_media send data [%u] failed\n", size);
			usleep(200*1000);
			continue;
		}
		
    }

fin:
    if (pBuff) {
        free(pBuff);
    }
    SHM_VideoRemoveID(shmid);
	
    printf("th_media exit\n");
    mydata->exit_media = true;
    return NULL;
#if 0
	g_dana_ls.close(&(real_meadia), g_dana_ls.encode_current_chn);
	printf(">>>>>>>>>>>>>>>th_media exit\n");
	mydata->run_media = false;
	mydata->exit_media = true;
	return NULL;
#endif
}

unsigned long dana_get_file_size(const char *path)  
{  
    unsigned long filesize = -1;      
    struct stat statbuff;  
    if(stat(path, &statbuff) < 0){  
        return filesize;  
    }else{  
        filesize = statbuff.st_size;  
    }  
    return filesize;  
} 

int dana_playback_handle_create()
{
	if(gs_pPBHandle == NULL)
	{
		gs_pPBHandle = (DANAAviPBHandle *)malloc(sizeof(DANAAviPBHandle));
		if(gs_pPBHandle == NULL)
		{
			printf("[%s]Error! malloc DANAAviPBHandle failed !!!\n", __FUNCTION__);
			return -1;
		}
		memset(gs_pPBHandle, 0, sizeof(DANAAviPBHandle));

		gs_pPBHandle->file = fopen(pDanaAvEvent.file_name, "rb+");
		if(gs_pPBHandle->file == NULL)
		{
			printf("[%s]Error! fopen %s failed !\n", __FUNCTION__, pDanaAvEvent.file_name);
			return -1;
		}
		
		gs_pPBHandle->pb_buf = (char *)malloc(MEDIA_FRAME_SIZE);
		if(gs_pPBHandle->pb_buf == NULL)
		{
			printf("[%s]Error! malloc gs_pPBHandle->pb_buf failed !\n", __FUNCTION__);
			return -1;
		}

    	gs_pPBHandle->index_current = 0;
		gs_pPBHandle->pb_file_end = 0;
		gs_pPBHandle->pb_buf_size = 0;
	}

	return 0;
}

int dana_playback_handle_destroy()
{
	if(gs_pPBHandle->file)
	{
		fclose(gs_pPBHandle->file);
		gs_pPBHandle->file = NULL;
	}
	if(gs_pPBHandle->pb_buf)
	{
		free(gs_pPBHandle->pb_buf);
		gs_pPBHandle->pb_buf = NULL;
	}
	if(gs_pPBHandle->idx_array)
	{
		free(gs_pPBHandle->idx_array);
		gs_pPBHandle->idx_array = NULL;
	}
	if(gs_pPBHandle)
	{
		free(gs_pPBHandle);
		gs_pPBHandle = NULL;
	}
	
	return 0;
}

int dana_avi_read_header_and_index(DANAAviPBHandle *pPBHandle)
{
    int tmp32;
    int ret;

    if(!pPBHandle)
    {
        printf("parameter error.\n");
        return -1;
    }

    AviHeader avi_header;
    ret = fread(&avi_header, sizeof(AviHeader), 1, pPBHandle->file);
    if(ret != 1)
    {
        printf("read avi header failed. ret = %d\n", ret);
        return -1;
    }

    printf("%s  dwMicroSecPerFrame=%d\n", __FUNCTION__, avi_header.main_header.dwMicroSecPerFrame);
    pPBHandle->avi_fps = 1000000.0f/avi_header.main_header.dwMicroSecPerFrame;
    printf("fps = %f \n", pPBHandle->avi_fps);

    //update avi_file infomation
    pPBHandle->video_count = avi_header.main_header.dwTotalFrames;
    pPBHandle->audio_count = avi_header.audio_header.dwLength;

    //rec: int movi_len = avi_file->data_offset - sizeof(AviHeader) + 4;
    ret = fseek(pPBHandle->file, avi_header.movi_len - 4 + sizeof(AviHeader), SEEK_SET);
    if(ret != 0)
    {
        printf("fseek failed.\n");
        return -1;
    }

    //读索引，获取视频帧个数
    int fcc;
    ret = read_fourcc(pPBHandle->file, &fcc);
    if(ret != 1)
    {
        printf("read fcc idx1 failed. ret = %d\n", ret);
        return -1;
    }
    if(fcc != MAKE_FOURCC('i','d','x','1'))
    {
        printf("fcc %08x is not idx1.\n", fcc);
        return -1;
    }

    ret = fread(&tmp32, sizeof(int), 1, pPBHandle->file);
    if(ret != 1)
    {
        printf("read index size failed.\n");
        return -1;
    }
    pPBHandle->index_count = tmp32/16;
    printf("index count = %d\n", pPBHandle->index_count);

    //read index to mem
    pPBHandle->idx_array_count = pPBHandle->index_count;
    pPBHandle->idx_array = (int *)malloc((pPBHandle->idx_array_count+1) * 16);   
    if(pPBHandle->idx_array == NULL)
    {
        printf("avi_read_header, malloc index failed.\n");
        return -1;
    }
	//读取索引
    ret = fread(pPBHandle->idx_array, 16*pPBHandle->index_count, 1, pPBHandle->file);
    if(ret != 1)
    {
        printf("avi_read_header, fread error.\n");
        return -1;
    }

    //sylvia+: pseudo last index
    pPBHandle->idx_array[4* pPBHandle->index_count + 2] = pPBHandle->idx_array[4* (pPBHandle->index_count-1) + 2] +
                                               pPBHandle->idx_array[4* (pPBHandle->index_count-1) + 3] + 8 + 16;
    return 0;
}

//取I帧时间戳
int dana_avi_get_frame_timestamp(DANAAviPBHandle *pPBHandle,struct timeval *ptimeval)
{
    if(!pPBHandle)
    {
        printf("parameter error.\n");
        return -1;
    }

    if(pPBHandle->index_current > pPBHandle->idx_array_count )
    {
        printf("frame index has overstep.\n");
        return -2;
    }
	//读取每帧数据的时间
	// first 8 : type+framelen("00dc"); 	second 8: type+datalen(JUNK)*/
    int ts_offset = 8 + pPBHandle->idx_array[4*pPBHandle->index_current +2] + pPBHandle->idx_array[4*pPBHandle->index_current+3] + 8;

    fseek(pPBHandle->file, ts_offset, SEEK_SET);
    fread(&ptimeval->tv_sec, 1, sizeof(int), pPBHandle->file);
    fread(&ptimeval->tv_usec, 1, sizeof(int), pPBHandle->file);
    return 0;
}

void *th_history_media(void *arg)
{
	static int s_iPlayBackThreadEnterCount = 0;
	printf("####>>>> s_iPlayBackThreadEnterCount = %d\n", s_iPlayBackThreadEnterCount++);
    MYDATA *mydata = (MYDATA *)arg;
	int avIndex;
    int totalsize = 0;
	int next_max_number,timestampdur;

    uint32_t timeout_usec = 1000*1000; // 1000ms

    unsigned int timestamp, media_type, size, isKeyFrame;
    unsigned int tick0;
    unsigned int timestamp_base = 0;
    int fcc32,frametype;
	int  frame_size;
	int  file_offset=0;//读取录像文件的偏移量

    tick0 = danale_gettickcount();
	struct timeval start_time;	
    struct timespec t1, t2;
    int index;
	int ret;
	
	if(dana_playback_handle_create() < 0)
	{
		printf(">> dana_playback_handle_create failed !\n");
		goto Exit;
	}
	DANAAviPBHandle *pPBHandle = gs_pPBHandle;
	dana_avi_read_header_and_index(pPBHandle);
	
	if(pDanaAvEvent.uiStartTime == pDanaAvEvent.uitimestamp)
	{
		pPBHandle->index_current = 0;
	}
	else
	{
	    //遍历取I帧(采取正负10秒模糊取I帧而不是绝对值)
	    for(index=0; index < pPBHandle->index_count; index++)
		{
			if(pPBHandle->idx_array[4*index+1]==0x11)//必须取I帧
			{
				dana_avi_get_frame_timestamp(pPBHandle, &start_time);
				if(DIFF_ABS(start_time.tv_sec, pDanaAvEvent.uitimestamp) < 10)
				{
					//treat as find I-frame
					pPBHandle->index_current = index;
					break;
				}
			}
		}
	}
	timestampdur = 1000/pPBHandle->avi_fps;

	while(mydata->run_history_media)
	{
		if(gsi_danale_recplayback_pause)
		{
			usleep(50000);
        	continue;
		}

		if(pPBHandle->pb_file_end == 1)
		{
			printf(">> file end...\n");
			goto Exit;
		}
		
		if(pPBHandle->index_current > pPBHandle->index_count)
		{
			pPBHandle->pb_file_end = 1;
		}
			
		fcc32 = pPBHandle->idx_array[4*pPBHandle->index_current+0];
		frametype = pPBHandle->idx_array[4*pPBHandle->index_current+1];
		file_offset = pPBHandle->idx_array[4*pPBHandle->index_current+2];
		frame_size = pPBHandle->idx_array[4*pPBHandle->index_current+3];

		fseek(pPBHandle->file, file_offset + 8, SEEK_SET);
		ftell(pPBHandle->file);  // ??
		clock_gettime(CLOCK_MONOTONIC, &t1);

		pPBHandle->pb_buf_size = frame_size;			
		memset(pPBHandle->pb_buf, 0x0, MEDIA_FRAME_SIZE);
		ret = fread(pPBHandle->pb_buf,  frame_size,1, pPBHandle->file);
		
		pPBHandle->index_current++;
		
		clock_gettime(CLOCK_MONOTONIC, &t2);

		if((t2.tv_sec - t1.tv_sec) < timestampdur) 
		{                
			usleep(18*1000); // sleep 1 ms            
		}
			
		if( fcc32 == MAKE_FOURCC('0','0','d','c') )//video
		{
			if(frametype == 0x11)
			{
			   //I frame
			   isKeyFrame = 1;
			}
			else 
			{
			   //other frame
			   isKeyFrame = 0;
			}
			if (lib_danavideoconn_send(mydata->danavideoconn, video_stream, H264, mydata->chan_no, isKeyFrame, timestamp + timestamp_base, (const char*)pPBHandle->pb_buf, pPBHandle->pb_buf_size, timeout_usec)) 
			{
			    //usleep(1*1000);
				//printf("th_media send test file [%u] succeeded\n", size);
			} 
			else 
			{
				printf(">>>>>>>playback send video data [%u] failed\n", size);
				//usleep(1*1000);
				
				//fseek(pPBHandle->file, file_offset + 8, SEEK_SET);
				//ftell(pPBHandle->file);
				continue;
			}				
		}
		else if( fcc32 == MAKE_FOURCC('0','1','w','b') )//audio
		{
			if (lib_danavideoconn_send(mydata->danavideoconn, audio_stream, G711A, mydata->chan_no, isKeyFrame, timestamp + timestamp_base, (const char*)pPBHandle->pb_buf, pPBHandle->pb_buf_size, timeout_usec)) 
			{
				//usleep(1*1000);
				//printf("dana_playback send data [%u] succeeded\n", size);
			} 
			else 
			{
				printf(">>>>>>playback send audio data [%u] failed\n", size);
				//fseek(pPBHandle->file, file_offset + 8, SEEK_SET);
				//ftell(pPBHandle->file);
				//usleep(1*1000);
				continue;
			}
		}
	}

Exit:
	dana_playback_handle_destroy();
    printf("thread_PlayBack exit\n");
	//pthread_exit(0);
	mydata->exit_history_media = true;
	return NULL;
}

int danale_history_recordplay_list(int64_t last_time, libdanavideo_reclist_recordinfo_t *pRec_lists, int *poutCount, int MaxCount)
{
	char fullname[64]={0};
	DIR *pDir;
	struct dirent *ent;
	uint32_t date = 0;
	char uiStartTime[20];
	char durateion[5];
	NodeIndex dana_fileNode;
	int counter;
	
	struct tm *tp = localtime(&last_time);   
	snprintf(fullname, sizeof(fullname), "%s/%04d%02d%02d", GRD_SD_MOUNT_POINT,tp->tm_year + 1900, tp->tm_mon+1, tp->tm_mday);
	if(access(fullname, F_OK) != 0)
	{		 
		printf(">>>>>>>recording path %s is not exist!\n", fullname);
		return -1;
	}
	pDir = opendir(fullname);
	if( !pDir )
	{
		printf(">>>>>>>opendir failed: dir=%s\n", fullname);
		return -1;;
	}
	char *startTime=NULL;

	while((ent = readdir(pDir)) != NULL)
	{
		if(ent->d_type & DT_REG)  //判断是否为非目录
		{
			if(strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0)
				continue;							
			
			if(is_avi_dir_format(ent->d_name) == 0)
				continue;
			
			//printf("========>>>> ent->d_name:%s\n",ent->d_name);
	        memset(uiStartTime,0x0,sizeof(uiStartTime));
			memset(durateion,0x0,sizeof(durateion));
			avi_substring(uiStartTime, ent->d_name,0, 10);
			avi_substring(durateion, ent->d_name,11, 11+3);
			dana_fileNode.uiStartTime = (int64_t)atoll(uiStartTime);
			dana_fileNode.uiDurations= atoi(durateion);
			//printf(">>>>>>dana_fileNode.uiStartTime=%"PRId64",dana_fileNode.uiDurations===%d\n",dana_fileNode.uiStartTime,dana_fileNode.uiDurations);
			if(dana_fileNode.uiStartTime < last_time || dana_fileNode.uiDurations <= 0) 
			{
				//printf("dana file node maybe is damaged\n");
				continue;
			}
			
			if(counter > MaxCount)
			   break;
								
			pRec_lists[counter].start_time = dana_fileNode.uiStartTime;
			pRec_lists[counter].record_type = 1;
			pRec_lists[counter].length = dana_fileNode.uiDurations;
			counter++;
		}
	}

	closedir(pDir);
	*poutCount = counter;

	return 0;
}

int danale_history_recordplay_control(int64_t time_stamp)
{
	int findrecording = 0;
	char fullname[64]={0};
	DIR *pDir;
	struct dirent *ent;
	uint32_t date = 0;
	char uiStartTime[20];
	char durateion[5];
	NodeIndex dana_fileNode;

	struct tm *tp = localtime(&time_stamp);   
	snprintf(fullname, sizeof(fullname), "%s/%04d%02d%02d", GRD_SD_MOUNT_POINT,tp->tm_year + 1900, tp->tm_mon+1, tp->tm_mday);
	if(access(fullname, F_OK) != 0)
	{		 
		printf(">>>>>>>recording path %s is not exist!\n", fullname);
		return -1;
	}
	pDir = opendir(fullname);
	if( !pDir )
	{
		printf(">>>>>>>opendir failed: dir=%s\n", fullname);
		return -1;;
	}

	while((ent = readdir(pDir)) != NULL)
	{
		if(ent->d_type & DT_REG)  //判断是否为非目录
		{
			if(strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0)
				continue;							
			if(is_avi_dir_format(ent->d_name) == 0)
				continue;
			
			//printf("========>>>> ent->d_name:%s\n",ent->d_name);
			memset(uiStartTime,0x0,sizeof(uiStartTime));
			memset(durateion,0x0,sizeof(durateion));
			avi_substring(uiStartTime, ent->d_name,0, 10);
			avi_substring(durateion, ent->d_name,11, 11+3);
			dana_fileNode.uiStartTime = (int64_t)atoll(uiStartTime);
			dana_fileNode.uiDurations= atoi(durateion);

			if((time_stamp >= dana_fileNode.uiStartTime) && (time_stamp < dana_fileNode.uiStartTime + dana_fileNode.uiDurations))
			{
				pDanaAvEvent.playback= 0x01;
				pDanaAvEvent.uiStartTime = dana_fileNode.uiStartTime;
				pDanaAvEvent.uitimestamp = time_stamp;
				memset(pDanaAvEvent.file_name,0x0,sizeof(pDanaAvEvent.file_name));
				snprintf(pDanaAvEvent.file_name,64,"%s/%lld-%d.avi",fullname,dana_fileNode.uiStartTime,dana_fileNode.uiDurations);						
				printf(">>>>>>>>>pDanaAvEvent.file_name====%s\n",pDanaAvEvent.file_name);
				findrecording = 1;
				break;
			}

		}
	}

	closedir(pDir);
	return findrecording;
}

// 处理成功返回0
// 失败返回-1
uint32_t danavideoconn_created(void *arg) // pdana_video_conn_t
{
    printf("TEST danavideoconn_created\n");
    pdana_video_conn_t *danavideoconn = (pdana_video_conn_t *)arg; 
    // set user data  
    MYDATA *mydata = (MYDATA*) calloc(1, sizeof(MYDATA));
    if (NULL == mydata) {
        printf("TEST danavideoconn_created failed calloc\n");
        return -1;
    }


    mydata->thread_media = 0;

    mydata->thread_talkback = 0;
    mydata->danavideoconn = danavideoconn;
    mydata->chan_no = 0;
    strncpy(mydata->appname, "libdanavideo_di3", strlen("libdanavideo_di3"));


    if (0 != lib_danavideo_set_userdata(danavideoconn, mydata)) {
        printf("TEST danavideoconn_created lib_danavideo_set_userdata failed\n");
        free(mydata);
        return -1;
    }

    printf("TEST danavideoconn_created succeeded\n");


    // 测试发送 extend_data
#if 0
    char *extend_data_byte = "test_di3"; // 可以是自定义的二进制数据
    size_t extend_data_size = strlen(extend_data_byte);
    uint32_t timeout_usec = 4000*1000;
    if (lib_danavideoconn_send(mydata->danavideoconn, extend_data, 0, mydata->chan_no, 0, 0, (const char*)extend_data_byte, extend_data_size, timeout_usec)) {
        printf("TEST send extend_data[%u] succeeded\n", extend_data_size);
    } else {
        printf("TEST send extend_data[%u] failed\n", extend_data_size);
    }
#endif

    return 0; 
}

void danavideoconn_aborted(void *arg) // pdana_video_conn_t
{
    printf("\n\n\n\n\n\x1b[34mTEST danavideoconn_aborted\x1b[0m\n\n\n\n\n\n");
    pdana_video_conn_t *danavideoconn = (pdana_video_conn_t *)arg;
    MYDATA *mydata;
    if (0 != lib_danavideo_get_userdata(danavideoconn, (void **)&mydata)) {
        printf("TEST danavideoconn_aborted lib_danavideo_get_userdata failed, mem-leak\n");
        return;
    }
    
    if (NULL != mydata) {
        // stop video thread
        mydata->run_media = false;
        if (0 != mydata->thread_media) {
            void *end;
            if (0 != pthread_join(mydata->thread_media, &end)) {
                perror("danavideoconn_aborted thread_media join faild!\n");
            }
        }
        memset(&(mydata->thread_media), 0, sizeof(mydata->thread_media));
        mydata->run_talkback = false; 
        if (0 != mydata->thread_talkback) {
            void *end;
            if (0 != pthread_join(mydata->thread_talkback, &end)) {
                perror("danavideoconn_aborted thread_talkback join faild!\n");
            }
        }
        memset(&(mydata->thread_talkback), 0, sizeof(mydata->thread_talkback));

        mydata->run_audio_media = false;
        if (0 != mydata->thread_audio_media) {
            if (0 != pthread_join(mydata->thread_audio_media, NULL)) {
                perror("danavideoconn_aborted thread_audio_media join faild!\n");
            }
        }    
        memset(&(mydata->thread_audio_media), 0, sizeof(mydata->thread_audio_media));    

        printf("TEST danavideoconn_aborted APPNAME: %s\n", mydata->appname);

        free(mydata);
    }
    
    printf("TEST danavideoconn_aborted succeeded\n");
    // get user data
    return;
}


void danavideoconn_command_handler(void *arg, dana_video_cmd_t cmd, uint32_t trans_id, void *cmd_arg, uint32_t cmd_arg_len) // pdana_video_conn_t
{
    printf("TEST danavideoconn_command_handler\n");
    pdana_video_conn_t *danavideoconn = (pdana_video_conn_t *)arg;
	MYDATA *mydata;
    // 响应不同的命令
    if (0 != lib_danavideo_get_userdata(danavideoconn, (void**)&mydata)) {
        printf("TEST danavideoconn_command_handler lib_danavideo_get_userdata failed\n");
        return;
    }
    printf("TEST danavideoconn_command_handler APPNAME: %s>>>>>>cmd===%d\n", mydata->appname,cmd);

    uint32_t i;
    uint32_t error_code;
    char *code_msg;
    
    // 发送命令回应的接口进行中
    switch (cmd) {
        case DANAVIDEOCMD_DEVDEF://恢复系统默认设置
            {
				//dana_factory_setting_req_handle(danavideoconn, cmd,trans_id, cmd_arg);
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_DEVDEF\n");
                DANAVIDEOCMD_DEVDEF_ARG *devdef_arg = (DANAVIDEOCMD_DEVDEF_ARG *)cmd_arg;
                printf("devdef_arg\n");
                printf("ch_no: %d\n", devdef_arg->ch_no);
                printf("\n");
				
				char msg_buf[1024] = {0};
				sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
				sdk_default_param_t   factory_msg_cfg ;
				memset(&factory_msg_cfg, 0, sizeof(sdk_default_param_t));
				
				factory_msg_cfg.param_mask = PARAM_MASK_ALL;  //恢复整机参数
				memcpy(pMsg->data, &factory_msg_cfg, sizeof(sdk_default_param_t));
				DANA_MSG_CTRL(SDK_MAIN_MSG_PARAM_DEFAULT,SDK_PARAM_SET,0,0,pMsg);
				dana_server_stop();
				printf("++++++++++++++++++reboot++++++++++++++++++\n");
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
					system_ex("reboot &");
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_DEVDEF send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_DEVDEF send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_DEVREBOOT:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_DEVREBOOT\n");
                DANAVIDEOCMD_DEVREBOOT_ARG *devreboot_arg = (DANAVIDEOCMD_DEVREBOOT_ARG *)cmd_arg;
                printf("devreboot_arg\n");
                printf("ch_no: %d\n", devreboot_arg->ch_no);
                printf("\n");
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_DEVREBOOT send response succeeded\n");
					system_ex("reboot");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_DEVREBOOT send response failed\n");
                } 
            }
            break; 
        case DANAVIDEOCMD_GETSCREEN:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETSCREEN\n");
                DANAVIDEOCMD_GETSCREEN_ARG *getscreen_arg = (DANAVIDEOCMD_GETSCREEN_ARG *)cmd_arg;
                printf("getcreen_arg\n");
                printf("ch_no: %d\n", getscreen_arg->ch_no);
                printf("\n");
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETSCREEN send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETSCREEN send response failed\n");
                }
                // 获取一副图片调用lib_danavideoconn_send()方法发送

            }
            break;
        case DANAVIDEOCMD_GETALARM:
            {
                 printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETALARM\n");
                DANAVIDEOCMD_GETALARM_ARG *getalarm_arg = (DANAVIDEOCMD_GETALARM_ARG *)cmd_arg;
                printf("getalarm_arg\n");
                printf("ch_no: %d\n", getalarm_arg->ch_no);
                printf("\n");
                error_code = 0;
                code_msg = (char *)"";
                uint32_t motion_detection = 0;
                uint32_t opensound_detection = 0;
                uint32_t openi2o_detection = 1;
                uint32_t smoke_detection = 1;
                uint32_t shadow_detection = 1;
                uint32_t other_detection = 1;
				
				int motion = 1;
				char msg_buf[1024] = {0};
				
				sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
				sdk_motion_cfg_t *md_param = (sdk_motion_cfg_t *)pMsg->data;
				
				DANA_MSG_CTRL(SDK_MAIN_MSG_MOTION_CFG,SDK_PARAM_GET,0,0,pMsg);
				motion_detection=md_param->codec_vda_cfg.sensitive;
				
				
				sdk_iodev_param_t *iodev_ori;				
				char msg_buf_get[1024] = {0};
				sdk_msg_t *pMsg_get = (sdk_msg_t *)msg_buf_get;
				DANA_MSG_CTRL(SDK_MAIN_MSG_IODEV_PARAM,SDK_PARAM_GET,0,0,pMsg_get);
				iodev_ori = (sdk_iodev_param_t *)pMsg_get->data;
				opensound_detection=iodev_ori->buzzer_param.buzzer;
				//dana_get_MDSensitive_req_handle(&motion_detection,&opensound_detection);
                printf(">>>>motion_detection: %d,md_param->codec_vda_cfg.sensitive,opensound_detection==%d\n", motion_detection,md_param->codec_vda_cfg.sensitive,opensound_detection);
				
                if (lib_danavideo_cmd_getalarm_response(danavideoconn, trans_id, error_code, code_msg, motion_detection, opensound_detection, openi2o_detection, smoke_detection, shadow_detection, other_detection)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETALARM send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETALARM send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_GETBASEINFO:
            {
				char *dana_id = (char *)"di3_1";
				char *api_ver = (char *)"di3_2";
				char *sn	  = (char *)"di3_3";
				char *device_name = (char *)"di3_4";
				char *rom_ver = (char *)"di3_5";
				uint32_t device_type = 1;
				uint32_t ch_num = 25;
				uint64_t sdc_size = 0;
				uint64_t sdc_free = 0;
				const size_t work_channel_count = 3;
				const uint32_t work_channel[48] = {11, 33, 44};
			
				struct statfs diskInfo;
				
				char msg_buf[1024] = {0};
				sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
			
				DANA_MSG_CTRL(SDK_MAIN_MSG_SYS_CFG,SDK_PARAM_GET,0,0,pMsg);
				sdk_sys_cfg_t *version_msg_cfg	= (sdk_sys_cfg_t *)pMsg->data;
				/*	strncpy(resp.model, version_msg_cfg->device_type,
						(strlen(version_msg_cfg->device_type)>15)?15:strlen(version_msg_cfg->device_type));
				strncpy(resp.vendor, version_msg_cfg->manufacturer,
						(strlen(version_msg_cfg->manufacturer)>15)?15:strlen(version_msg_cfg->manufacturer));
				*/
				unsigned int version = 0,version_0 = 0,version_1 = 0,version_2 = 0,version_3 = 0;
			
				sscanf(version_msg_cfg->software_ver,"V%d.%d.%d.%d",&version_3,&version_2,&version_1,&version_0);
				version_3 = ((version_3&0xff) << 24) ;
				version_2 = ((version_2&0xff) << 16) ;
				version_1 = ((version_1&0xff) << 8) ;
				version_0 = ((version_0&0xff) << 0) ;
				version = version_3| version_2 | version_1 | version_0;
				printf("version:%d\n", version);
				/*
				  *  get SD card information	2015-07-03
				  */
			
				statfs(GRD_SD_MOUNT_POINT, &diskInfo);	
			
				unsigned long long blocksize = diskInfo.f_bsize;	//每个block里包含的字节数  
				unsigned long long totalsize = blocksize * diskInfo.f_blocks;	//总的字节数，f_blocks为block的数目  
			
				printf("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n",   
					totalsize, totalsize>>10, totalsize>>20, totalsize>>30);  
				  
				unsigned long long freeDisk = diskInfo.f_bfree * blocksize; //剩余空间的大小  
				unsigned long long availableDisk = diskInfo.f_bavail * blocksize;	//可用空间大小	
			
				printf("Disk_free = %llu MB = %llu GB\nDisk_available = %llu MB = %llu GB\n",   
					freeDisk>>20, freeDisk>>30, availableDisk>>20, availableDisk>>30); 
			
				if(0 > get_sd_stats())//判断SD卡是否存在
				{
					  sdc_size = 0;
					  sdc_free = 0;
				}
				else
				{
					sdc_size = totalsize;
					sdc_free = availableDisk;
				}
								
				error_code = 0;
				code_msg = (char *)"";
			
				if (lib_danavideo_cmd_getbaseinfo_response(danavideoconn, trans_id, error_code, code_msg, dana_id, api_ver, sn, device_name, rom_ver, device_type, ch_num, sdc_size, sdc_free, work_channel_count, work_channel)) {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETBASEINFO send response succeeded\n");
				} else {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETBASEINFO send response failed\n");
				}
            }
            break;
        case DANAVIDEOCMD_GETCOLOR:
            {
				uint32_t brightness = 0;
				uint32_t contrast = 0;
				uint32_t saturation = 0;
				uint32_t hue = 0;
				
				char msg_bufs[1024] = {0};
				
				sdk_msg_t *pMsgs = (sdk_msg_t *)msg_bufs;
				sdk_image_attr_t *attr_param = (sdk_image_attr_t *)pMsgs->data;
				
				DANA_MSG_CTRL(SDK_MAIN_MSG_IMAGE_ATTR_CFG,SDK_PARAM_GET,0,0,pMsgs);
				pMsgs->size = sizeof(sdk_image_attr_t);
				brightness = attr_param->brightness;
				contrast = attr_param->contrast;
				saturation = attr_param->saturation;
				hue = attr_param->hue;
				
				printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETCOLOR\n");
				DANAVIDEOCMD_GETCOLOR_ARG *getcolor_arg = (DANAVIDEOCMD_GETCOLOR_ARG *)cmd_arg;
				printf("getcolor_arg\n");
				printf("ch_no: %d\n", getcolor_arg->ch_no);
				printf("\n");
				error_code = 0;
				code_msg = (char *)"";
				if (lib_danavideo_cmd_getcolor_response(danavideoconn, trans_id, error_code, code_msg, brightness, contrast, saturation, hue)) {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETCOLOR send response succeeded\n");
				} else {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETCOLOR send response failed\n");
				}
            }
            break; 
        case DANAVIDEOCMD_GETFLIP:
            {
				printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETFLIP\n");
				DANAVIDEOCMD_GETFLIP_ARG *getflip_arg = (DANAVIDEOCMD_GETFLIP_ARG *)cmd_arg;
				printf("getflip_arg_arg\n");
				printf("ch_no: %d\n", getflip_arg->ch_no);
				printf("\n");
				error_code = 0;
				code_msg = (char *)"";
				//0:Upright; 1:Flip Horizontal; 2:Flip Vertical; 3:turn 180
				uint32_t flip_type = 0;
				
				char buf [1024] = {0};	
				int i =0;
				sdk_mirror_flip_cfg_t *mirror = (sdk_mirror_flip_cfg_t *)buf;
				adapt_param_get_mirror_param_param(mirror);
				if(mirror->mirror == 1)
					flip_type |= 0x1;
				if(mirror->flip == 1)
					flip_type |= 0x2;

				printf("^^ DANAVIDEOCMD_GETFLIP ^^: flip_type=%d\n", flip_type);
				if (lib_danavideo_cmd_getflip_response(danavideoconn, trans_id, error_code, code_msg, flip_type)) {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETFLIP send response succeeded\n");
				} else {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETFLIP send response failed\n");
				}
            }
            break; 
        case DANAVIDEOCMD_GETFUNLIST:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETFUNLIST\n");
                DANAVIDEOCMD_GETFUNLIST_ARG *getfunlist_arg = (DANAVIDEOCMD_GETFUNLIST_ARG *)cmd_arg;
                printf("getfunlist_arg\n");
                printf("ch_no: %d\n", getfunlist_arg->ch_no);
                printf("\n");
                error_code = 0;
                code_msg = (char *)"";
                uint32_t methodes_count = 49;
#ifdef DANAVIDEO_FLIGHT_TEST
                methodes_count += 5;
#endif
#ifdef DANAVIDEO_SMARTHOME_TEST
                methodes_count += 2;
#endif
#ifdef DANA_RING_TEST
                methodes_count += 4;
#endif
                // 日夜转换
                methodes_count += 2;
                char *methods[] = { (char *)"DANAVIDEOCMD_DEVDEF", 
                    (char *)"DANAVIDEOCMD_DEVREBOOT", 
                    (char *)"DANAVIDEOCMD_GETSCREEN",
                    (char *)"DANAVIDEOCMD_GETALARM",
                    (char *)"DANAVIDEOCMD_GETBASEINFO",
                    (char *)"DANAVIDEOCMD_GETCOLOR",
                    (char *)"DANAVIDEOCMD_GETFLIP",
                    (char *)"DANAVIDEOCMD_GETFUNLIST",
                    (char *)"DANAVIDEOCMD_GETNETINFO",
                    (char *)"DANAVIDEOCMD_GETPOWERFREQ",
                    (char *)"DANAVIDEOCMD_GETTIME",
                    (char *)"DANAVIDEOCMD_GETWIFIAP",
                    (char *)"DANAVIDEOCMD_GETWIFI",
                    (char *)"DANAVIDEOCMD_PTZCTRL",
                    (char *)"DANAVIDEOCMD_SDCFORMAT",
                    (char *)"DANAVIDEOCMD_SETALARM",
                    (char *)"DANAVIDEOCMD_SETCHAN",
                    (char *)"DANAVIDEOCMD_SETCOLOR",
                    (char *)"DANAVIDEOCMD_SETFLIP",
                    (char *)"DANAVIDEOCMD_SETNETINFO",
                    (char *)"DANAVIDEOCMD_SETPOWERFREQ",
                    (char *)"DANAVIDEOCMD_SETTIME",
                    (char *)"DANAVIDEOCMD_SETVIDEO",
                    (char *)"DANAVIDEOCMD_SETWIFIAP",
                    (char *)"DANAVIDEOCMD_SETWIFI",
                    (char *)"DANAVIDEOCMD_STARTAUDIO",
                    (char *)"DANAVIDEOCMD_STARTTALKBACK",
                    (char *)"DANAVIDEOCMD_STARTVIDEO",
                    (char *)"DANAVIDEOCMD_STOPAUDIO",
                    (char *)"DANAVIDEOCMD_STOPTALKBACK",
                    (char *)"DANAVIDEOCMD_STOPVIDEO",
                    (char *)"DANAVIDEOCMD_RECLIST",
                    (char *)"DANAVIDEOCMD_RECPLAY",
                    (char *)"DANAVIDEOCMD_RECSTOP",
                    (char *)"DANAVIDEOCMD_RECACTION",
                    (char *)"DANAVIDEOCMD_RECSETRATE",
                    (char *)"DANAVIDEOCMD_RECPLANGET",
                    (char *)"DANAVIDEOCMD_RECPLANSET",
                    (char *)"DANAVIDEOCMD_EXTENDMETHOD",
                    (char *)"DANAVIDEOCMD_SETOSD",
                    (char *)"DANAVIDEOCMD_GETOSD",
                    (char *)"DANAVIDEOCMD_SETCHANNAME",
                    (char *)"DANAVIDEOCMD_GETCHANNAME",
#ifdef DANAVIDEO_FLIGHT_TEST 
                    (char *)"DANAIOTCMD_FLIGHTCMD",
                    (char *)"DANAIOTCMD_FLIGHTCONTROL",
                    (char *)"DANAIOTCMD_FLIGHTSTATUS",
                    (char *)"DANAIOTCMD_SETWAYPOINT",
                    (char *)"DANAIOTCMD_GETWAYPOINT",
#endif
#ifdef DANAVIDEO_SMARTHOME_TEST
                    (char *)"DANASMARTHOMECMD_MAKEPAIR",
                    (char *)"DANASMARTHOMECMD_DELPAIR",
#endif
#ifdef DANA_RING_TEST
                    (char *)"DANARINGCMD_GETMODET",
                    (char *)"DANARINGCMD_SETMODET",
                    (char *)"DANARINGCMD_GETMOTIMEPLAN",
                    (char *)"DANARINGCMD_SETMOTIMEPLAN",
#endif
                    (char *)"DANAVIDEOCMD_CALLPSP",
                    (char *)"DANAVIDEOCMD_GETPSP",
                    (char *)"DANAVIDEOCMD_SETPSP",
                    (char *)"DANAVIDEOCMD_SETPSPDEF",
                    (char *)"DANAVIDEOCMD_GETLAYOUT",
                    (char *)"DANAVIDEOCMD_SETCHANADV",
                    (char *)"DANAVIDEOCMD_SETICR",
                    (char *)"DANAVIDEOCMD_GETICR" };
                if (lib_danavideo_cmd_getfunlist_response(danavideoconn, trans_id, error_code, code_msg, methodes_count, (const char**)methods)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETFUNLIST send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETFUNLIST send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_GETNETINFO:
            {
				uint32_t ip_type;
				uint32_t dns_type;
				char ipaddr[16]={0};
				char netmask[16]={0};
				char gateway[16]={0};
				char dns_name1[16]={0};
				char dns_name2[16]={0};
				uint32_t http_port = 21045;
				char ifname[16]={0};

				if(g_network_info.ucNetworkType == 0)
					strncpy(ifname, "eth0", sizeof(ifname));
				else if(g_network_info.ucNetworkType == 1)
					strncpy(ifname, "wlan0", sizeof(ifname));
				ip_type = 1;  // 1: dhcp, 0: fixed
				dns_type = 1; // 1: dhcp, 0: fixed
				os_get_ip_addr(ifname, ipaddr, sizeof(ipaddr));
				os_get_net_mask(ifname, netmask, sizeof(netmask));
				os_get_gateway1(ifname, gateway, sizeof(gateway));
				os_get_dns(dns_name1, dns_name2, sizeof(dns_name1));
				if(strcmp(dns_name2, "")==0)
					strncpy(dns_name2, dns_name1, sizeof(dns_name2));
				
				printf("in dana_get_net_req_handle!!!\n");
				printf("ipcamIP=%s\n",ipaddr);
				printf("ipcamIPMask=%s\n",netmask);
				printf("ipcamGateway=%s\n",gateway);
				printf("byDnsaddr1=%s\n",dns_name1);
				printf("byDnsaddr2=%s\n",dns_name2);
				printf("ip_type=%s\n",ip_type==1?"dhcp":"fixed");
				printf("dns_type=%s\n",dns_type==1?"dhcp":"fixed");
				error_code = 0;
				code_msg = (char *)"";
				
				printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETNETINFO\n");
				if (lib_danavideo_cmd_getnetinfo_response(danavideoconn, trans_id, error_code, code_msg, ip_type, ipaddr, netmask, gateway, dns_type, dns_name1, dns_name2, http_port)) {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETNETINFO send response succeeded\n");
				} else {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETNETINFO send response failed\n");
				}
            }
            break;
        case DANAVIDEOCMD_GETPOWERFREQ:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETPOWERFREQ\n");
                DANAVIDEOCMD_GETPOWERFREQ_ARG *getpowerfreq_arg = (DANAVIDEOCMD_GETPOWERFREQ_ARG *)cmd_arg;
                printf("getpowerfreq_arg\n");
                printf("ch_no: %"PRIu32"\n", getpowerfreq_arg->ch_no);
                printf("\n");

				uint32_t freq;
				char msg_buf[1024] = {0};
				sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
				sdk_power_freq_cfg_t *power_freq = (sdk_power_freq_cfg_t *)pMsg->data;
				DANA_MSG_CTRL(SDK_MAIN_MSG_POWER_FREQ_CFG,SDK_PARAM_GET,0,0,pMsg);
				if(power_freq->powerfreq == 1)
					freq = DANAVIDEO_POWERFREQ_50HZ;
				else
					freq = DANAVIDEO_POWERFREQ_60HZ;

				printf("*** freq=%s\n", freq==DANAVIDEO_POWERFREQ_50HZ?"50HZ":"60HZ");
                error_code = 0;
                code_msg = (char *)"";
                //uint32_t freq = DANAVIDEO_POWERFREQ_50HZ;
                if (lib_danavideo_cmd_getpowerfreq_response(danavideoconn, trans_id, error_code, code_msg, freq)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETPOWERFREQ send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETPOWERFREQ send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_GETTIME:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETTIME\n");
                DANAVIDEOCMD_GETTIME_ARG *gettime_arg = (DANAVIDEOCMD_GETTIME_ARG *)cmd_arg;
                printf("gettime_arg_arg\n");
                printf("ch_no: %d\n", gettime_arg->ch_no);
                printf("\n");
                error_code = 0;
                code_msg = (char *)"";
                int64_t now_time;
                char *time_zone = (char *)"shanghai";
                char *ntp_server_1 = "0.asia.pool.ntp.org";
                char *ntp_server_2 = "1.asia.pool.ntp.org";
				now_time = time(NULL);
                printf("nowtime second:%ld\n",now_time); 				
                if (lib_danavideo_cmd_gettime_response(danavideoconn, trans_id, error_code, code_msg, now_time, time_zone, ntp_server_1, ntp_server_2)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETTIME send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETTIME send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_GETWIFIAP:
            {
				uint32_t wifi_device;
                uint32_t wifi_list_count;
                libdanavideo_wifiinfo_t wifi_list[DANAVIDEO_CMD_GETWIFIAP_MAX_COUNT] = {0};

				/*if(g_network_info.ucNetworkType == 0)
				{
					wifi_device = 0;
					wifi_list_count = 0;
					error_code = 8220;
				}
				else
				{
					wifi_device = 1;
					get_wifi_AP_list(wifi_list, DANAVIDEO_CMD_GETWIFIAP_MAX_COUNT, &wifi_list_count);
				}*/
				wifi_device = 1;
				get_wifi_AP_list(wifi_list, DANAVIDEO_CMD_GETWIFIAP_MAX_COUNT, &wifi_list_count);
				
				printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETWIFIAP\n");
				error_code = 0;
				code_msg = (char *)"";
				if (lib_danavideo_cmd_getwifiap_response(danavideoconn, trans_id, error_code, code_msg, wifi_device, wifi_list_count, wifi_list)) {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETWIFIAP send response succeeded\n");
				} else {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETWIFIAP send response failed\n");
				}
            }
            break; 
        case DANAVIDEOCMD_GETWIFI:
            {
				printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETWIFI\n");
				char curSSID[32] = {0};
				char curKey[32] = {0};
				char buffer[256] = {0};
				char *pStr;
				char *essid;
				char *auth_key;
				uint32_t enc_type;

				get_wifi_config(curSSID, sizeof(curSSID), curKey, sizeof(curKey));
				sprintf(buffer, "wpa_cli -iwlan0 scan_result | grep %s", curSSID);
				FILE *pscan_result = popen(buffer, "r");
				if(pscan_result != NULL)
				{
					memset(buffer, 0, sizeof(buffer));
					fgets(buffer, sizeof(buffer), pscan_result);
					parseSecurity(buffer, &enc_type);
					pclose(pscan_result);
				}
				essid = curSSID;
				auth_key = curKey;
				error_code = 0;
				code_msg = (char *)"";
				if (lib_danavideo_cmd_getwifi_response(danavideoconn, trans_id, error_code, code_msg, essid, auth_key, enc_type)) {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETWIFI send response succeeded\n");
				} else {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETWIFI send response failed\n");
				}
            }
            break;
        case DANAVIDEOCMD_PTZCTRL:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_PTZCTRL\n");
                DANAVIDEOCMD_PTZCTRL_ARG *ptzctrl_arg = (DANAVIDEOCMD_PTZCTRL_ARG *)cmd_arg;
                printf("ptzctrl_arg\n");
                printf("ch_no: %d\n", ptzctrl_arg->ch_no);
                printf("code: %d\n", ptzctrl_arg->code);
                printf("para1: %d\n", ptzctrl_arg->para1);
                printf("para2: %d\n", ptzctrl_arg->para2);
                printf("\n");
                switch (ptzctrl_arg->code) {
                    case DANAVIDEO_PTZ_CTRL_MOVE_UP:
                        printf("DANAVIDEO_PTZ_CTRL_MOVE_UP\n");
                        break;
                    case DANAVIDEO_PTZ_CTRL_MOVE_DOWN:
                        printf("DANAVIDEO_PTZ_CTRL_MOVE_DOWN\n");
                        break;
                    case DANAVIDEO_PTZ_CTRL_MOVE_LEFT:
                        printf("DANAVIDEO_PTZ_CTRL_MOVE_LEFT\n");
                        break;
                    case DANAVIDEO_PTZ_CTRL_MOVE_RIGHT:
                        printf("DANAVIDEO_PTZ_CTRL_MOVE_RIGHT\n");
                        break;
                        // ...
                }
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_PTZCTRL send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_PTZCTRL send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_SDCFORMAT:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SDCFORMAT\n");
                DANAVIDEOCMD_SDCFORMAT_ARG *sdcformat_arg = (DANAVIDEOCMD_SDCFORMAT_ARG *)cmd_arg; 
                printf("sdcformat_arg\n");
                printf("ch_no: %d\n", sdcformat_arg->ch_no);
                printf("\n");
				//格式化之前停掉所有录像
				if(0 == st_sd_card_format())
				{
				    error_code = 0;
					code_msg = (char *)"";
					printf("external storage was Formated\n");
				}
				else
				{
				    error_code = 0;
					code_msg = (char *)"";
				}
				
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SDCFORMAT send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SDCFORMAT send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_SETALARM:
            {
				DANAVIDEOCMD_SETALARM_ARG *setalarm_arg = (DANAVIDEOCMD_SETALARM_ARG *)cmd_arg;
				printf("setalarm_arg\n");
				printf("ch_no: %d\n", setalarm_arg->ch_no);
				printf("motion_detection: %d\n", setalarm_arg->motion_detection);
				printf("opensound_detection: %d\n", setalarm_arg->opensound_detection);
				printf("openi2o_detection: %d\n", setalarm_arg->openi2o_detection);
				printf("smoke_detection: %d\n", setalarm_arg->smoke_detection);
				printf("shadow_detection: %d\n", setalarm_arg->shadow_detection);
				printf("other_detection: %d\n", setalarm_arg->other_detection);
				printf("\n");
				
				char msg_buf[1024] = {0};
				sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
				sdk_motion_cfg_t *md_param = (sdk_motion_cfg_t *)pMsg->data;
				printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETALARM\n");
				
				DANA_MSG_CTRL(SDK_MAIN_MSG_MOTION_CFG,SDK_PARAM_GET,0,0,pMsg);
				pMsg->size = sizeof(sdk_motion_cfg_t);
				int i =0;
				switch(setalarm_arg->motion_detection)
				{
				case 0:
					md_param->codec_vda_cfg.mask = 0;
					break;
				case 1:
					for(i = 1 ; i<5; i ++)
					{
						SET_BIT(md_param->codec_vda_cfg.mask,i);
					}
					md_param->codec_vda_cfg.sensitive= 1;
					break;
				case 2:
					for(i = 1 ; i<5; i ++)
					{
						SET_BIT(md_param->codec_vda_cfg.mask,i);
					}
					md_param->codec_vda_cfg.sensitive= 2;
					break;
				case 3:
					for(i = 1 ; i<5; i ++)
					{
						SET_BIT(md_param->codec_vda_cfg.mask,i);
					}
					md_param->codec_vda_cfg.sensitive= 3;
					break;
				default:
					md_param->codec_vda_cfg.mask = 0;
					md_param->codec_vda_cfg.sensitive= 1;
					break;
				}
				
				printf(">>>md_param->codec_vda_cfg.sensitive: %d\n", md_param->codec_vda_cfg.sensitive);
				DANA_MSG_CTRL(SDK_MAIN_MSG_MOTION_CFG,SDK_PARAM_SET,0,0,pMsg);
				
				if(0 < setalarm_arg->motion_detection )
				{
					g_motion_pra.motion_Enable = 1;
					//pir_event.alarm_enable |= setalarm_arg->motion_detection;
					//pir_event.alarm_enable |= 0x10000;
					//pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
					//led_buzzer_info.led_status |= (1<<LED_ARM);
					//led_buzzer_info.led_time = setAlarm->buzzer_time;
					//pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);
					//printf("led_buzzer_info.led_time=%d %d\n",led_buzzer_info.led_time,setAlarm->buzzer_time);
				}
				else
				{
					g_motion_pra.motion_Enable = 0;
					//pir_event.alarm_enable = 0;
					//pthread_mutex_lock(&led_buzzer_info.led_buzzer_lock);
					//led_buzzer_info.led_status &= ~(1<<LED_ARM);
					//pthread_mutex_unlock(&led_buzzer_info.led_buzzer_lock);
				}
				sdk_iodev_param_t *iodev_ori;
				char msg_buf_get[1024] = {0};
				sdk_msg_t *pMsg_get = (sdk_msg_t *)msg_buf_get;
				DANA_MSG_CTRL(SDK_MAIN_MSG_IODEV_PARAM,SDK_PARAM_GET,0,0,pMsg_get);
				iodev_ori = (sdk_iodev_param_t *)pMsg_get->data;
				iodev_ori->buzzer_param.buzzer = setalarm_arg->opensound_detection;
				printf(">>>iodev_ori->buzzer_param.buzzer: %d\n", iodev_ori->buzzer_param.buzzer);
				DANA_MSG_CTRL(SDK_MAIN_MSG_IODEV_PARAM,SDK_PARAM_SET,0,0,pMsg_get);
				
				error_code = 0;
				code_msg = (char *)"";
				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETALARM send response succeeded\n");
				} else {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETALARM send response failed\n");
				}
            }
            break; 
        case DANAVIDEOCMD_SETCHAN:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHAN\n");
                DANAVIDEOCMD_SETCHAN_ARG *setchan_arg = (DANAVIDEOCMD_SETCHAN_ARG *)cmd_arg; 
                printf("setchan_arg\n");
                printf("ch_no: %d\n", setchan_arg->ch_no);
                printf("chans_count: %zd\n", setchan_arg->chans_count);
                for (i=0; i<setchan_arg->chans_count; i++) {
                    printf("chans[%d]: %d\n", i, setchan_arg->chans[i]);
                }
                printf("\n");
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHAN send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHAN send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_SETCOLOR:
            {				
				printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCOLOR\n"); 
				DANAVIDEOCMD_SETCOLOR_ARG *setcolor_arg = (DANAVIDEOCMD_SETCOLOR_ARG *)cmd_arg;
				printf("setcolor_arg\n");
				printf("ch_no: %d\n", setcolor_arg->ch_no);
				printf("video_rate: %d\n", setcolor_arg->video_rate);
				printf("brightness: %d\n", setcolor_arg->brightness);
				printf("contrast: %d\n", setcolor_arg->contrast);
				printf("saturation: %d\n", setcolor_arg->saturation);
				printf("hue: %d\n", setcolor_arg->hue);
				printf("\n");
				
				char msg_bufs[1024] = {0};
				sdk_msg_t *pMsgs = (sdk_msg_t *)msg_bufs;
				sdk_image_attr_t *attr_param = (sdk_image_attr_t *)pMsgs->data;
				DANA_MSG_CTRL(SDK_MAIN_MSG_IMAGE_ATTR_CFG,SDK_PARAM_GET,0,0,pMsgs);
				pMsgs->size = sizeof(sdk_image_attr_t);
				if((setcolor_arg->brightness!=0)||(setcolor_arg->contrast!=0)||(setcolor_arg->saturation!=0)||(setcolor_arg->hue!=0))
				{
					attr_param->brightness = setcolor_arg->brightness;
					attr_param->contrast = setcolor_arg->contrast;
					attr_param->saturation = setcolor_arg->saturation;
					attr_param->hue = setcolor_arg->hue;
				}
				attr_param->sharpness= 128;
				DANA_MSG_CTRL(SDK_MAIN_MSG_IMAGE_ATTR_CFG,SDK_PARAM_SET,0,0,pMsgs);
				
				error_code = 0;
				code_msg = (char *)"";
				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCOLOR send response succeeded\n");
				} else {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCOLOR send response failed\n");
				}
            }
            break;
        case DANAVIDEOCMD_SETFLIP:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETFLIP\n"); 
                DANAVIDEOCMD_SETFLIP_ARG *setflip_arg = (DANAVIDEOCMD_SETFLIP_ARG *)cmd_arg;
                printf("setflip_arg\n");
                printf("ch_no: %d\n", setflip_arg->ch_no);
                printf("flip_type: %d\n", setflip_arg->flip_type);
                printf("\n");

				//flip_type 0:Upright; 1:Flip Horizontal; 2:Flip Vertical; 3:turn 180
				
				char msg_bufs[1024] = {0};
				sdk_msg_t *pMsgs = (sdk_msg_t *)msg_bufs;
				sdk_mirror_flip_cfg_t *pRotation = (sdk_mirror_flip_cfg_t *)pMsgs->data;
				
				pMsgs->size = sizeof(sdk_mirror_flip_cfg_t);
				if(setflip_arg->flip_type & 0x1)
					pRotation->mirror = 1;
				if(setflip_arg->flip_type & 0x2)
					pRotation->flip = 1;
				DANA_MSG_CTRL(SDK_MAIN_MSG_MIRROR_FLIP_CFG,SDK_PARAM_SET,0,0,pMsgs);
				
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETFLIP send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETFLIP send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_SETNETINFO:
            {
				//dana_set_net_req_handle(danavideoconn,cmd,trans_id,cmd_arg);
				sdk_eth_cfg_t *eth_cfg = NULL;
				char msg_buf[1024] = {0};
				sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
				DANA_MSG_CTRL(SDK_MAIN_MSG_NET_LINK_CFG,SDK_PARAM_GET,0,0,pMsg);//获取MAC 地址
				eth_cfg = (sdk_eth_cfg_t*)pMsg->data;
				
				printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETNETINFO\n"); 
				DANAVIDEOCMD_SETNETINFO_ARG *setnetinfo_arg = (DANAVIDEOCMD_SETNETINFO_ARG *)cmd_arg;
				
				if(NULL == setnetinfo_arg)
				{
					error_code=1;
				}
				
				printf("setnetinfo_arg\n");
				printf("ch_no: %d\n", setnetinfo_arg->ch_no);
				printf("ip_type: %d\n", setnetinfo_arg->ip_type);
				printf("ipaddr: %s\n", setnetinfo_arg->ipaddr);
				printf("netmask: %s\n", setnetinfo_arg->netmask);
				printf("gateway: %s\n", setnetinfo_arg->gateway);
				printf("dns_type: %d\n", setnetinfo_arg->dns_type);
				printf("dns_name1: %s\n", setnetinfo_arg->dns_name1);
				printf("dns_name2: %s\n", setnetinfo_arg->dns_name2);
				printf("http_port: %d\n", setnetinfo_arg->http_port);
				printf("\n");
				if(1 == setnetinfo_arg->ip_type) //DHCP
				{
					//eth_cfg->ip_info.enable_dhcp = p->conn_mod;
					memset(eth_cfg->ip_info.ip_addr,0,MAX_IP_ADDR_LEN);
					memset(eth_cfg->ip_info.gateway,0,MAX_IP_ADDR_LEN);
					memset(eth_cfg->ip_info.mask,0,MAX_IP_ADDR_LEN);
					memset(eth_cfg->ip_info.dns1,0,MAX_IP_ADDR_LEN);
					memset(eth_cfg->ip_info.dns2,0,MAX_IP_ADDR_LEN);
				}
				else if(0 == setnetinfo_arg->ip_type)
				{
					eth_cfg->ip_info.enable_dhcp = setnetinfo_arg->ip_type;
					strcpy(eth_cfg->ip_info.ip_addr, setnetinfo_arg->ipaddr);
					strcpy(eth_cfg->ip_info.gateway, setnetinfo_arg->gateway);
					strcpy(eth_cfg->ip_info.mask, setnetinfo_arg->netmask);
					strcpy(eth_cfg->ip_info.dns1, setnetinfo_arg->dns_name1);
					strcpy(eth_cfg->ip_info.dns2, setnetinfo_arg->dns_name2);
					eth_cfg->ip_info.dns_auto_en = setnetinfo_arg->dns_type;
				}
				memcpy(pMsg->data, eth_cfg,sizeof(sdk_eth_cfg_t));
				DANA_MSG_CTRL(SDK_MAIN_MSG_NET_LINK_CFG,SDK_PARAM_SET,0,0,pMsg);
				error_code = 0;
				code_msg = (char *)"";
				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETNETINFO send response succeeded\n");
				} else {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETNETINFO send response failed\n");
				}
            }
            break; 
        case DANAVIDEOCMD_SETPOWERFREQ:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPOWERFREQ\n"); 
                DANAVIDEOCMD_SETPOWERFREQ_ARG *setpowerfreq_arg = (DANAVIDEOCMD_SETPOWERFREQ_ARG *)cmd_arg;
                printf("setpowerfreq_arg\n");
                printf("ch_no: %d\n", setpowerfreq_arg->ch_no);
				printf("freq: %s\n", setpowerfreq_arg->freq==DANAVIDEO_POWERFREQ_50HZ?"50HZ":"60HZ");
                printf("\n");

				char msg_buf[1024] = {0};
				sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
				sdk_power_freq_cfg_t *power_freq = (sdk_power_freq_cfg_t *)pMsg->data;
				
				if (DANAVIDEO_POWERFREQ_50HZ == setpowerfreq_arg->freq) 
					power_freq->powerfreq = 1;
				else
					power_freq->powerfreq = 0;
				pMsg->size = sizeof(sdk_power_freq_cfg_t);
				DANA_MSG_CTRL(SDK_MAIN_MSG_POWER_FREQ_CFG,SDK_PARAM_SET,0,0,pMsg);
				
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPOWERFREQ send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPOWERFREQ send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_SETTIME:
            {
				printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETTIME\n"); 
				DANAVIDEOCMD_SETTIME_ARG *settime_arg = (DANAVIDEOCMD_SETTIME_ARG *)cmd_arg;
				
				printf("settime_arg\n");
				printf("settime_arg->ntp_server1: %s\n", settime_arg->ntp_server1);
				printf("settime_arg->ntp_server2: %s\n", settime_arg->ntp_server2);
				printf("now_time: %d\n", settime_arg->now_time);
				printf("time_zone: %s\n", settime_arg->time_zone);

				dana_update_timezone(settime_arg->time_zone);
				GET_DANA_SET_TIME_CLICK = 1;
				error_code = 0;
				code_msg = (char *)"";
				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETTIME send response succeeded\n");
				} else {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETTIME send response failed\n");
				}
            }
            break;
        case DANAVIDEOCMD_SETVIDEO:
            {
				//dana_set_videoQuality_req_handle(danavideoconn,cmd,trans_id,cmd_arg);				
				char msg_buf[1024] = {0};
				
				sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
				sdk_encode_t *enc_param = (sdk_encode_t *)pMsg->data;
				
				printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETVIDEO\n"); 
				DANAVIDEOCMD_SETVIDEO_ARG *setvideo_arg = (DANAVIDEOCMD_SETVIDEO_ARG *)cmd_arg;
				printf("setvideo_arg\n");
				printf("ch_no: %d\n", setvideo_arg->ch_no);
				printf("video_quality: %d\n", setvideo_arg->video_quality);
				printf("\n");

				int bitrate, framerate, resolution;
				int ch = 0;//dana_get_realtime_play_chnn();
		
				DANA_MSG_CTRL(SDK_MAIN_MSG_ENCODE_CFG,SDK_PARAM_GET,0,0,pMsg);
				printf("^^^^^ resolution = %d\n", enc_param->av_enc_info[ch].resolution);
				
				adjust_video_bps(setvideo_arg->video_quality, &bitrate, &framerate, &resolution);
				enc_param->av_enc_info[ch].bitrate = bitrate;
				enc_param->av_enc_info[ch].frame_rate = framerate;
				enc_param->av_enc_info[ch].resolution = resolution;
				pMsg->size = sizeof(sdk_encode_t);
				
				DANA_MSG_CTRL(SDK_MAIN_MSG_ENCODE_CFG,SDK_PARAM_SET,0,0,pMsg);
				
				error_code = 0;
				//code_msg = (char *)"success";
				code_msg = (char *)"";
				uint32_t set_video_fps = framerate;
				if (lib_danavideo_cmd_setvideo_response(danavideoconn, trans_id, error_code, code_msg, set_video_fps)) {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETVIDEO send response succeeded\n");
				} else {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETVIDEO send response failed\n");
				}
            }
            break; 
        case DANAVIDEOCMD_SETWIFIAP:
            {
				
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETWIFIAP\n"); 
                DANAVIDEOCMD_SETWIFIAP_ARG *setwifiap_arg = (DANAVIDEOCMD_SETWIFIAP_ARG *)cmd_arg;
                printf("setwifiap_arg\n");
                printf("ch_no: %d\n", setwifiap_arg->ch_no);
                printf("ip_type: %d\n", setwifiap_arg->ip_type);
                printf("ipaddr: %s\n", setwifiap_arg->ipaddr);
                printf("netmask: %s\n", setwifiap_arg->netmask);
                printf("gateway: %s\n", setwifiap_arg->gateway);
                printf("dns_name1: %s\n", setwifiap_arg->dns_name1);
                printf("dns_name2: %s\n", setwifiap_arg->dns_name2);
                printf("essid: %s\n", setwifiap_arg->essid);
                printf("auth_key: %s\n", setwifiap_arg->auth_key);
                printf("enc_type: %d\n", setwifiap_arg->enc_type);
                printf("\n");

				//set wifi params in AP mode, do not supported in current Airlink mode
				/*dana_setwifiap(setwifiap_arg->ch_no,setwifiap_arg->ip_type,setwifiap_arg->ipaddr,
					setwifiap_arg->netmask,setwifiap_arg->gateway,setwifiap_arg->dns_name1,
					setwifiap_arg->dns_name2,setwifiap_arg->essid,setwifiap_arg->auth_key,
					setwifiap_arg->enc_type);*/
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETWIFIAP send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETWIFIAP send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_SETWIFI:
            {
				//dana_set_wifi_req_handle(danavideoconn,cmd,trans_id,cmd_arg);
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETWIFI\n");
                DANAVIDEOCMD_SETWIFI_ARG *setwifi_arg = (DANAVIDEOCMD_SETWIFI_ARG *)cmd_arg; 
                printf("setwifi_arg\n");
                printf("ch_no: %"PRIu32"\n", setwifi_arg->ch_no);
                printf("essid: %s\n", setwifi_arg->essid);
                printf("auth_key: %s\n", setwifi_arg->auth_key);
                printf("enc_type: %"PRIu32"\n", setwifi_arg->enc_type);
                printf("\n");

				set_wifi_config(setwifi_arg->essid, setwifi_arg->auth_key);
				sleep(1);
				g_network_info.ucNeedreconfig = 1;
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETWIFI send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETWIFI send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_STARTAUDIO:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTAUDIO\n"); 
                DANAVIDEOCMD_STARTAUDIO_ARG *startaudio_arg = (DANAVIDEOCMD_STARTAUDIO_ARG *)cmd_arg;
                printf("startaudio_arg\n");
                printf("ch_no: %d\n", startaudio_arg->ch_no);
                printf("\n");
                error_code = 0;
                code_msg = (char *)"";

                uint32_t audio_codec = G711A; // danavidecodec_t
                uint32_t sample_rate = 8000; // 单位Hz
                uint32_t sample_bit  = 16; // 单位bit
                uint32_t track = 1; // (1 mono; 2 stereo)

                if (lib_danavideo_cmd_startaudio_response(danavideoconn, trans_id, error_code, code_msg, NULL, &sample_rate, NULL, &track)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTAUDIO send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTAUDIO send response failed\n");
                }
                // 开启音频生产者线程
                if (mydata->run_audio_media) {
                    printf("ch_no[%u] is already started, enjoy\n", startaudio_arg->ch_no);
                } else {
                    mydata->run_audio_media = true;
                    mydata->exit_audio_media = false;
                    mydata->chan_no = startaudio_arg->ch_no;
                    if (0 != pthread_create(&(mydata->thread_audio_media), NULL, &th_audio_media, mydata)) {
                        memset(&(mydata->thread_audio_media), 0, sizeof(mydata->thread_audio_media));
                        printf("TEST danavideoconn_command_handler pthread_create th_audio_media failed\n");
                        // 发送无法开启视频线程到对端
                    } else {
                        printf("TEST danavideoconn_command_handler th_audio_media is started, enjoy!\n");
                    }
                }
            }
            break;
        case DANAVIDEOCMD_STARTTALKBACK:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTTALKBACK\n"); 
                DANAVIDEOCMD_STARTTALKBACK_ARG *starttalkback_arg = (DANAVIDEOCMD_STARTTALKBACK_ARG *)cmd_arg;
                printf("starttalkback_arg\n");
                printf("ch_no: %d\n", starttalkback_arg->ch_no);
                if (starttalkback_arg->has_sample_bit) {
                    printf("sample_bit: %d\n", starttalkback_arg->sample_bit);
                }

                if (starttalkback_arg->has_sample_rate) {
                    printf("sample_rate: %d\n", starttalkback_arg->sample_rate);
                }

                if (starttalkback_arg->has_audio_codec) {
                    printf("audio_codec: %d\n", starttalkback_arg->audio_codec);
                }

                if (starttalkback_arg->has_track) {
                    printf("track: %d\n", starttalkback_arg->track);
                }
                printf("\n");
                //error_code = 0;
				error_code = 8460; // error code means device has no speaker
                code_msg = (char *)"";
                uint32_t audio_codec = G711A;
                uint32_t sample_rate = 8000; // 单位Hz
                uint32_t sample_bit  = 16; // 单位bit
                uint32_t track = 1; // (1 mono; 2 stereo)
                if (lib_danavideo_cmd_starttalkback_response(danavideoconn, trans_id, error_code, code_msg, audio_codec, &sample_rate, &sample_bit, &track)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTTALKBACK send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_STARTTALKBACK send response failed\n");
                }
				#if 0 // device no speaker
                // 开启读取音频数据的线程
                if (mydata->run_talkback) {
                    printf("ch_no[%u] is already started, enjoy\n", starttalkback_arg->ch_no);
                } else {
                    mydata->run_talkback = true;
                    mydata->exit_talback = false;
                    if (0 != pthread_create(&(mydata->thread_talkback), NULL, &th_talkback, mydata)) {
                        memset(&(mydata->thread_talkback), 0, sizeof(mydata->thread_talkback));
                        printf("TEST danavideoconn_command_handler pthread_create th_talkback failed\n"); 
                    } else {
                        printf("TEST danavideoconn_command_handler th_talkback is started, enjoy!\n");
                    }
                }
				#endif
            }
            break; 
        case DANAVIDEOCMD_STARTVIDEO:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMDSTARTVIDEO\n");
                DANAVIDEOCMD_STARTVIDEO_ARG *startvideo_arg = (DANAVIDEOCMD_STARTVIDEO_ARG *)cmd_arg;
                printf("startvideo_arg\n");
                printf("ch_no: %d\n", startvideo_arg->ch_no);
                printf("client_type: %d\n", startvideo_arg->client_type);
                printf("video_quality: %d\n", startvideo_arg->video_quality);
                printf("vstrm: %d\n", startvideo_arg->vstrm);
                printf("\n");
                error_code = 0;
                code_msg = (char *)"";
                uint32_t start_video_fps = 30;

				dana_enc_default_set(startvideo_arg->video_quality, 0);

                // 开启视频生产者线程
                if (mydata->run_media) {
                    printf("ch_no[%u] is already started, enjoy\n", startvideo_arg->ch_no);
                } else {
                    mydata->run_media = true;
                    mydata->exit_media = false;
                    mydata->chan_no = startvideo_arg->ch_no;
                    if (0 != pthread_create(&(mydata->thread_media), NULL, &th_media, mydata)) {
                        memset(&(mydata->thread_media), 0, sizeof(mydata->thread_media));
                        printf("TEST danavideoconn_command_handler pthread_create th_media failed\n");
                        // 发送无法开启视频线程到对端
                        mydata->run_media = false;
                        mydata->exit_media = true;
                    } else {
                        printf("TEST danavideoconn_command_handler th_media is started, enjoy!\n");
                    }
                }

                if (lib_danavideo_cmd_startvideo_response(danavideoconn, trans_id, error_code, code_msg, start_video_fps)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMDSTARTVIDEO send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMDSTARTVIDEO send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_STOPAUDIO:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_STOPAUDIO\n");
                DANAVIDEOCMD_STOPAUDIO_ARG *stopaudio_arg = (DANAVIDEOCMD_STOPAUDIO_ARG *)cmd_arg;
                printf("stopaudio_arg\n");
                printf("ch_no: %d\n", stopaudio_arg->ch_no);
                printf("\n");
                // 关闭音频生产者线程
                printf("TEST danavideoconn_command_handler stop th_audio_media\n");
                mydata->run_audio_media = false;
                
                if (0 != mydata->thread_audio_media) {
                    if (0 != pthread_join(mydata->thread_audio_media, NULL)) {
                        perror("TEST danavideoconn_command_handler thread_audio_media join faild!\n");
                    }
                }
                memset(&(mydata->thread_audio_media), 0, sizeof(mydata->thread_audio_media));
                
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_STOPAUDIO send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_STOPAUDIO send response failed\n");
                }
            }
            break; 
        case DANAVIDEOCMD_STOPTALKBACK:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_STOPTALKBACK\n");
                DANAVIDEOCMD_STOPTALKBACK_ARG *stoptalkback_arg = (DANAVIDEOCMD_STOPTALKBACK_ARG *)cmd_arg; 
                printf("stoptalkback_arg\n");
                printf("ch_no: %d\n", stoptalkback_arg->ch_no);
                printf("\n");
                // 关闭音频读取线程
                printf("TEST danavideoconn_command_handler stop th_talkback\n");
                mydata->run_talkback = false;
                if (0 != mydata->thread_talkback) {
                    if (0 != pthread_join(mydata->thread_talkback, NULL)) {
                        perror("TEST danavideoconn_command_handler thread_talkback join faild!\n");
                    }
                }
                memset(&(mydata->thread_talkback), 0, sizeof(mydata->thread_talkback));
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_STOPTALKBACK send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_STOPTALKBACK send response failed\n");
                }
            }
            break;
        case DANAVIDEOCMD_STOPVIDEO:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMDSTOPVIDEO\n");
                DANAVIDEOCMD_STOPVIDEO_ARG *stopvideo_arg = (DANAVIDEOCMD_STOPVIDEO_ARG *)cmd_arg;
                printf("stopvideo_arg\n");
                printf("ch_no: %d\n", stopvideo_arg->ch_no);
                printf("\n"); 
                // 关闭视频生产者线程
                printf("TEST danavideoconn_command_handler stop th_media\n");
                mydata->run_media = false;
                if (0 != mydata->thread_media) {
                    if (0 != pthread_join(mydata->thread_media, NULL)) {
                        perror("TEST danavideoconn_command_handler thread_media join faild!\n");
                    }
                }
                memset(&(mydata->thread_media), 0, sizeof(mydata->thread_media));
                error_code = 0;
                code_msg = (char *)"";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMDSTOPVIDEO send response succeeded\n");
                } else {

                    printf("TEST danavideoconn_command_handler DANAVIDEOCMDSTOPVIDEO send response failed\n");
                } 
            }
            break;
        case DANAVIDEOCMD_RECLIST:
             {
				printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RECLIST\n");
				DANAVIDEOCMD_RECLIST_ARG *reclist_arg = (DANAVIDEOCMD_RECLIST_ARG *)cmd_arg;
				printf("reclist_arg\n");
				printf("ch_no: %d\n", reclist_arg->ch_no);
				if (DANAVIDEO_REC_GET_TYPE_NEXT == reclist_arg->get_type) {
					printf("get_type: DANAVIDEO_REC_GET_TYPE_NEXT\n");
				} else if (DANAVIDEO_REC_GET_TYPE_PREV == reclist_arg->get_type) {
					printf("get_type: DANAVIDEO_REC_GET_TYPE_PREV\n");
				} else {
					printf("Unknown get_type: %d\n", reclist_arg->get_type);
				}
				printf("get_num: %d\n", reclist_arg->get_num);
				printf("last_time: %"PRId64"\n", reclist_arg->last_time);
				printf("\n");
				
				libdanavideo_reclist_recordinfo_t rec_lists[35]={0};
				int file_count;
				int64_t reqTimestmp = reclist_arg->last_time;
				int reqNumber = reclist_arg->get_num;
				int ret;
				
				ret = danale_history_recordplay_list(reqTimestmp, rec_lists, &file_count, reqNumber);
				if(ret < 0)
				{
					printf("====================NOT FIND======================= \n");
					file_count = 0;
				}
				error_code = 0;
				code_msg = "";
				if (lib_danavideo_cmd_reclist_response(danavideoconn, trans_id, error_code, code_msg, file_count, rec_lists)) {
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RECLIST send response succeeded\n");
				} else {
				
					printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RECLIST send response failed\n");
				} 
	            break;
        	}
        case DANAVIDEOCMD_RECPLAY:
            {
				printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLAY\n");
				DANAVIDEOCMD_RECPLAY_ARG *recplay_arg = (DANAVIDEOCMD_RECPLAY_ARG *)cmd_arg;
				printf("recplay_arg\n");
				printf("ch_no: %d\n", recplay_arg->ch_no);
				printf("time_stamp: %"PRId64"\n", recplay_arg->time_stamp);
				printf("\n");

				int result;
				result = danale_history_recordplay_control(recplay_arg->time_stamp);
				if(result > 0)
				{
					//open history recording play thread
	                if (mydata->run_history_media) {
	                    printf("ch_no[%u] is already started, enjoy\n", recplay_arg->ch_no);
	                } else {
	                    mydata->run_history_media = true;
	                    mydata->exit_history_media = false;
	                    mydata->chan_no = recplay_arg->ch_no;
	                    if (0 != pthread_create(&(mydata->thread_history_media), NULL, &th_history_media, mydata)) {
	                        memset(&(mydata->thread_history_media), 0, sizeof(mydata->thread_history_media));
	                        printf("TEST danavideoconn_command_handler pthread_create th_history_media failed\n");
	                        // 发送无法开启视频线程到对端
	                        mydata->run_history_media = false;
	                        mydata->exit_history_media = true;
	                    } else {
	                        printf("TEST danavideoconn_command_handler th_media is started, enjoy!\n");
	                    }
	                }
				}
				error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLAY send response succeeded\n");
                } else {

                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLAY send response failed\n");
                } 
            }
            break;
			
        case DANAVIDEOCMD_RECSTOP:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSTOP\n");
                DANAVIDEOCMD_RECSTOP_ARG *recstop_arg = (DANAVIDEOCMD_RECSTOP_ARG *)cmd_arg;
                printf("recstop_arg\n");
                printf("ch_no: %d\n", recstop_arg->ch_no);
                printf("\n");

				printf("TEST danavideoconn_command_handler stop th_history_media\n");
                mydata->run_history_media = false;
                if (0 != mydata->thread_history_media) {
                    if (0 != pthread_join(mydata->thread_history_media, NULL)) {
                        perror("TEST danavideoconn_command_handler thread_history_media join faild!\n");
                    }
                }
                memset(&(mydata->thread_history_media), 0, sizeof(mydata->thread_history_media));
				
				error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSTOP send response succeeded\n");
                } else {

                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSTOP send response failed\n");
                }
            } 
            break;
        case DANAVIDEOCMD_RECACTION:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RECACTION\n");
                DANAVIDEOCMD_RECACTION_ARG *recaction_arg = (DANAVIDEOCMD_RECACTION_ARG *)cmd_arg;
                printf("recaction_arg\n");
                printf("ch_no: %d\n", recaction_arg->ch_no);
                if (DANAVIDEO_REC_ACTION_PAUSE == recaction_arg->action) {
				#if 0
					//get writer lock
					int lock_ret = pthread_rwlock_wrlock(&gDanaClientInfo.sLock);
					gDanaClientInfo.bPausePlayBack = 1;
					//release lock
					lock_ret = pthread_rwlock_unlock(&gDanaClientInfo.sLock);
				#endif
					gsi_danale_recplayback_pause = 1;
                    printf("action: DANAVIDEO_REC_ACTION_PAUSE\n");
                } else if (DANAVIDEO_REC_ACTION_PLAY == recaction_arg->action) {
                #if 0
					//get writer lock
					int lock_ret = pthread_rwlock_wrlock(&gDanaClientInfo.sLock);
					gDanaClientInfo.bPausePlayBack = 0;
					//release lock
					lock_ret = pthread_rwlock_unlock(&gDanaClientInfo.sLock);
				#endif
					gsi_danale_recplayback_pause = 0;
                    printf("action: DANAVIDEO_REC_ACTION_PLAY\n");
                } else {
                    printf("Unknown action: %d\n", recaction_arg->action);
                }
                printf("\n"); 
				//sleep(2);
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RECACTION send response succeeded\n");
                } else {

                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RECACTION send response failed\n");
                } 
            }
            break;
        case DANAVIDEOCMD_RECSETRATE:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSETRATE\n");
                DANAVIDEOCMD_RECSETRATE_ARG *recsetrate_arg = (DANAVIDEOCMD_RECSETRATE_ARG *)cmd_arg;
                printf("recsetrate_arg\n");
                printf("ch_no: %d\n", recsetrate_arg->ch_no);
                if (DANAVIDEO_REC_RATE_HALF == recsetrate_arg->rec_rate) {
                    printf("rec_rate: DANAVIDEO_REC_RATE_HALF\n");
                } else if (DANAVIDEO_REC_RATE_NORMAL == recsetrate_arg->rec_rate) {
                    printf("rec_rate: DANAVIDEO_REC_RATE_NORMAL\n");
                } else if (DANAVIDEO_REC_RATE_DOUBLE == recsetrate_arg->rec_rate) {
                    printf("rec_rate: DANAVIDEO_REC_RATE_DOUBLE\n");
                } else if (DANAVIDEO_REC_RATE_FOUR == recsetrate_arg->rec_rate) {
                    printf("rec_rate: DANAVIDEO_REC_RATE_FOUR\n");
                } else {
                    printf("Unknown rec_rate: %d\n", recsetrate_arg->rec_rate);
                }
                printf("\n"); 
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSETRATE send response succeeded\n");
                } else {

                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RECSETRATE send response failed\n");
                } 
            }
            break;
        case DANAVIDEOCMD_RECPLANGET:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANGET\n");
                DANAVIDEOCMD_RECPLANGET_ARG *recplanget_arg = (DANAVIDEOCMD_RECPLANGET_ARG *)cmd_arg;
                printf("recplanget_arg\n");
                printf("ch_no: %d\n", recplanget_arg->ch_no);
                printf("\n"); 
                error_code = 0;
                code_msg = "";
				
                uint32_t rec_plans_count = 2;
				int startHour=0,startMin=0,startSec=0,endHour=0,endMin=0,endSec=0;
                libdanavideo_recplanget_recplan_t rec_plans[] = {{0, 2, {DANAVIDEO_REC_WEEK_MON, DANAVIDEO_REC_WEEK_SAT}, "12:23:34", "15:56:01", DANAVIDEO_REC_PLAN_OPEN}, {1, 3, {DANAVIDEO_REC_WEEK_MON, DANAVIDEO_REC_WEEK_SAT, DANAVIDEO_REC_WEEK_SUN}, "22:23:24", "23:24:25", DANAVIDEO_REC_PLAN_CLOSE}};

				char msg_buf[10*1024] = {0};
				sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
				sdk_record_cfg_t *rec_cfg = (sdk_record_cfg_t *)pMsg->data;
				DANA_MSG_CTRL(SDK_MAIN_MSG_RECORD_CFG,SDK_PARAM_GET,0,0,pMsg);
				int weekday;
				int week_count = 0;
				for (i=1; i<=7; i++) {
					weekday = i;
					//rec_cfg->record_sched[weekday-1].record_type = record_type;
					if(rec_cfg->record_sched[weekday-1].sched_time[0].plan_flag != 1)
						continue;//按周布放
					
					startHour=rec_cfg->record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].start_hour;
					startMin=rec_cfg->record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].start_min;
					startSec=rec_cfg->record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].start_sec;

					endHour=rec_cfg->record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].stop_hour;
					endMin=rec_cfg->record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].stop_min;
					endSec=rec_cfg->record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].stop_sec;
					snprintf(rec_plans[0].start_time,33,"%02d:%02d:%02d",startHour,startMin,startSec);
					snprintf(rec_plans[0].end_time,33,"%02d:%02d:%02d",endHour,endMin,endSec);
					if (DANAVIDEO_REC_WEEK_MON == weekday) {
				    	printf("week: DANAVIDEO_REC_WEEK_MON\n");
						rec_plans[0].week[week_count] = DANAVIDEO_REC_WEEK_MON;
					} else if (DANAVIDEO_REC_WEEK_TUE == weekday) {
						printf("week: DANAVIDEO_REC_WEEK_TUE\n");
						rec_plans[0].week[week_count] = DANAVIDEO_REC_WEEK_TUE;
					} else if (DANAVIDEO_REC_WEEK_WED == weekday) {
						printf("week: DANAVIDEO_REC_WEEK_WED\n");
						rec_plans[0].week[week_count] = DANAVIDEO_REC_WEEK_WED;
					} else if (DANAVIDEO_REC_WEEK_THU == weekday) {
						printf("week: DANAVIDEO_REC_WEEK_THU\n");
						rec_plans[0].week[week_count] = DANAVIDEO_REC_WEEK_THU;
					} else if (DANAVIDEO_REC_WEEK_FRI == weekday) {
						printf("week: DANAVIDEO_REC_WEEK_FRI\n");
						rec_plans[0].week[week_count] = DANAVIDEO_REC_WEEK_FRI;
					} else if (DANAVIDEO_REC_WEEK_SAT == weekday) {
						printf("week: DANAVIDEO_REC_WEEK_SAT\n");
						rec_plans[0].week[week_count] = DANAVIDEO_REC_WEEK_SAT;
					} else if (DANAVIDEO_REC_WEEK_SUN == weekday) {
						printf("week: DANAVIDEO_REC_WEEK_SUN\n");
						rec_plans[0].week[week_count] = DANAVIDEO_REC_WEEK_SUN;
					} else {
						printf("Unknown week: %d\n", weekday);
					} 
					week_count ++;
				} 
				rec_plans[0].week_count = week_count;

				if (lib_danavideo_cmd_recplanget_response(danavideoconn, trans_id, error_code, code_msg, rec_plans_count, rec_plans)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANGET send response succeeded\n");
                } else {

                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANGET send response failed\n");
                }
            } 
            break;
        case DANAVIDEOCMD_RECPLANSET:
            {
				//dana_set_record_req_handle(danavideoconn,cmd,trans_id,cmd_arg);
				unsigned int record_type = 0;				
				printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANSET\n");
				DANAVIDEOCMD_RECPLANSET_ARG *recplanset_arg = (DANAVIDEOCMD_RECPLANSET_ARG *)cmd_arg;
				printf("recplanset_arg\n");
				printf("ch_no: %d\n", recplanset_arg->ch_no);
				printf("record_no: %d\n", recplanset_arg->record_no);
				size_t i;
				printf("start_time: %s\n", recplanset_arg->start_time);
				printf("end_time: %s\n", recplanset_arg->end_time);
				printf("status: %d\n", recplanset_arg->status);
				printf("\n"); 
				int startHour,startMin,startSec,endHour,endMin,endSec;
				sscanf(recplanset_arg->start_time,"%02d:%02d:%02d",&startHour,&startMin,&startSec);
				sscanf(recplanset_arg->end_time,"%02d:%02d:%02d",&endHour,&endMin,&endSec);
				printf(">>>>>>>>start -time %02d:%02d:%02d",startHour,startMin,startSec);
				printf(">>>>>>>>end----time %02d:%02d:%02d",endHour,endMin,endSec);
				switch( recplanset_arg->status)//录像计划编号 (1-3)
				{
					case RECORD_NO:
					{	
						record_type = RECORD_NO;
					}
					break;
					case RECORD_FULLTIME:
					{
						record_type = RECORD_FULLTIME;
					}
					break;
					case RECORD_ALARM:
					{
						record_type = RECORD_ALARM;
					}
					break;
					default:
						record_type = RECORD_NO;
						break;
				};

				char msg_buf[10*1024] = {0};
				sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
				sdk_record_cfg_t *rec_cfg = (sdk_record_cfg_t *)pMsg->data;
				DANA_MSG_CTRL(SDK_MAIN_MSG_RECORD_CFG,SDK_PARAM_GET,0,0,pMsg);
				memset(rec_cfg,0x0,sizeof(sdk_record_cfg_t));
				rec_cfg->record_types = record_type;
				int weekday;
				for (i=0; i<recplanset_arg->week_count; i++) {
					weekday = recplanset_arg->week[i];
					rec_cfg->record_sched[weekday-1].record_type = record_type;
					rec_cfg->record_sched[weekday-1].sched_time[0].plan_flag = 1;//按周布放
					//if (DANAVIDEO_REC_WEEK_MON == recplanset_arg->week[i]) {
				    //printf("week: DANAVIDEO_REC_WEEK_MON\n");
					rec_cfg->record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].start_hour=startHour;
					rec_cfg->record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].start_min=startMin;
					rec_cfg->record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].start_sec=startSec;
					rec_cfg->record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].stop_hour=endHour;
					rec_cfg->record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].stop_min=endMin;
					rec_cfg->record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].stop_sec=endSec;
					/*} else if (DANAVIDEO_REC_WEEK_TUE == recplanset_arg->week[i]) {
						printf("week: DANAVIDEO_REC_WEEK_TUE\n");
					} else if (DANAVIDEO_REC_WEEK_WED == recplanset_arg->week[i]) {
						printf("week: DANAVIDEO_REC_WEEK_WED\n");
					} else if (DANAVIDEO_REC_WEEK_THU == recplanset_arg->week[i]) {
						printf("week: DANAVIDEO_REC_WEEK_THU\n");
					} else if (DANAVIDEO_REC_WEEK_FRI == recplanset_arg->week[i]) {
						printf("week: DANAVIDEO_REC_WEEK_FRI\n");
					} else if (DANAVIDEO_REC_WEEK_SAT == recplanset_arg->week[i]) {
						printf("week: DANAVIDEO_REC_WEEK_SAT\n");
					} else if (DANAVIDEO_REC_WEEK_SUN == recplanset_arg->week[i]) {
						printf("week: DANAVIDEO_REC_WEEK_SUN\n");
					} else {
						printf("Unknown week: %d\n", recplanset_arg->week[i]);
					} */
				} 
				rec_cfg->record_types = record_type;
				rec_cfg->enable_audio = 1;
				rec_cfg->enable = record_type;
				printf(" trace ::record_types=%d\n ",rec_cfg->record_types);
				pMsg->size = sizeof(sdk_record_cfg_t);
				DANA_MSG_CTRL(SDK_MAIN_MSG_RECORD_CFG,SDK_PARAM_SET,0,0,pMsg);
                error_code = 0;
                code_msg = "";
				GET_DANA_SET_RECPLAN_CLICK = 1;
				record_status.record_flags  = record_type;
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANSET send response succeeded\n");
                } else {

                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RECPLANSET send response failed\n");
                } 
            }
            break;
        case DANAVIDEOCMD_EXTENDMETHOD:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_EXTENDMETHOD\n");
                DANAVIDEOCMD_EXTENDMETHOD_ARG *extendmethod_arg = (DANAVIDEOCMD_EXTENDMETHOD_ARG *)cmd_arg;
                printf("extendmethod_arg\n");
                printf("ch_no: %d\n", extendmethod_arg->ch_no);
                printf("extend_data_size: %zd\n", extendmethod_arg->extend_data.size);
                // extend_data_bytes access via extendmethod_arg->extend_data.bytes
                printf("\n"); 
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_EXTENDMETHOD send response succeeded\n");
                } else {

                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_EXTENDMETHOD send response failed\n");
                }
            } 
            break;
        case DANAVIDEOCMD_SETOSD:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETOSD\n");
                DANAVIDEOCMD_SETOSD_ARG *setosd_arg = (DANAVIDEOCMD_SETOSD_ARG *)cmd_arg;
                printf("ch_no: %d\n", setosd_arg->ch_no);
                printf("osd_info:\n");
                if (DANAVIDEO_OSD_SHOW_OPEN == setosd_arg->osd.chan_name_show) {
                    printf("chan_name_show OPEN\n");
                    if (setosd_arg->osd.has_show_name_x) {
                        printf("show_name_x: %d\n", setosd_arg->osd.show_name_x);
                    }
                    if (setosd_arg->osd.has_show_name_y) {
                        printf("show_name_y: %d\n", setosd_arg->osd.show_name_y);
                    } 
					
                } else if (DANAVIDEO_OSD_SHOW_CLOSE == setosd_arg->osd.chan_name_show) {
                    printf("chan_name_show CLOSE\n");
                } else {
                    printf("chan_name_show unknown type[%d]\n", setosd_arg->osd.chan_name_show);
                }
                if (DANAVIDEO_OSD_SHOW_OPEN == setosd_arg->osd.datetime_show) {
                    printf("datetime_show OPEN\n");
                    if (setosd_arg->osd.has_show_datetime_x) {
                        printf("show_datetime_x: %d\n", setosd_arg->osd.show_datetime_x);
                    }
                    if (setosd_arg->osd.has_show_datetime_y) {
                        printf("show_datetime_y: %d\n", setosd_arg->osd.show_datetime_y);
                    }
                    if (setosd_arg->osd.has_show_format) {
                        printf("show_format:\n");
                        switch (setosd_arg->osd.show_format) {
                            case DANAVIDEO_OSD_DATE_FORMAT_YYYY_MM_DD:
                                printf("DANAVIDEO_OSD_DATE_FORMAT_YYYY_MM_DD\n");
                                break;
                            case DANAVIDEO_OSD_DATE_FORMAT_MM_DD_YYYY:
                                printf("DANAVIDEO_OSD_DATE_FORMAT_MM_DD_YYYY\n");
                                break;
                            case DANAVIDEO_OSD_DATE_FORMAT_YYYY_MM_DD_CH:
                                printf("DANAVIDEO_OSD_DATE_FORMAT_YYYY_MM_DD_CH\n");
                                break;
                            case DANAVIDEO_OSD_DATE_FORMAT_MM_DD_YYYY_CH:
                                printf("DANAVIDEO_OSD_DATE_FORMAT_MM_DD_YYYY_CH\n");
                                break;
                            case DANAVIDEO_OSD_DATE_FORMAT_DD_MM_YYYY:
                                printf("DANAVIDEO_OSD_DATE_FORMAT_DD_MM_YYYY\n");
                                break;
                            case DANAVIDEO_OSD_DATE_FORMAT_DD_MM_YYYY_CH:
                                printf("DANAVIDEO_OSD_DATE_FORMAT_DD_MM_YYYY_CH\n");
                                break;
                            default:
                                printf("DANAVIDEO_OSD_DATE_FORMAT_XXXX\n");
                                break;
                        }
                    }
                    if (setosd_arg->osd.has_hour_format) {
                        printf("hour_format:\n");
                        switch (setosd_arg->osd.hour_format) {
                            case DANAVIDEO_OSD_TIME_24_HOUR:
                                printf("DANAVIDEO_OSD_TIME_24_HOUR\n");
                                break;
                            case DANAVIDEO_OSD_TIME_12_HOUR:
                                printf("DANAVIDEO_OSD_TIME_12_HOUR\n");
                                break;
                            default:
                                printf("DANAVIDEO_OSD_TIME_XXXX\n");
                                break;
                        }
                    }
                    if (setosd_arg->osd.has_show_week) {
                        printf("show_week:\n");
                        switch (setosd_arg->osd.show_week) {
                            case DANAVIDEO_OSD_SHOW_CLOSE:
                                printf("DANAVIDEO_OSD_SHOW_CLOSE\n");
                                break;
                            case DANAVIDEO_OSD_SHOW_OPEN:
                                printf("DANAVIDEO_OSD_SHOW_OPEN\n");
                                break;
                            default:
                                printf("DANAVIDEO_OSD_SHOW_XXXX\n");
                                break;
                        }
                    }
                    if (setosd_arg->osd.has_datetime_attr) {
                        printf("datetime_attr:\n");
                        switch (setosd_arg->osd.datetime_attr) {
                            case DANAVIDEO_OSD_DATETIME_TRANSPARENT:
                                printf("DANAVIDEO_OSD_DATETIME_TRANSPARENT\n");
                                break;
                            case DANAVIDEO_OSD_DATETIME_DISPLAY:
                                printf("DANAVIDEO_OSD_DATETIME_DISPLAY\n");
                                break;
                            default:
                                printf("DANAVIDEO_OSD_DATETIME_XXXX\n");
                                break;
                        }
                    }
                } else if (DANAVIDEO_OSD_SHOW_CLOSE == setosd_arg->osd.datetime_show) {
                    printf("datetime_show CLOSE\n");
                } else {
                    printf("datetime_show unknown type[%d]\n", setosd_arg->osd.datetime_show);
                }
                if (DANAVIDEO_OSD_SHOW_OPEN == setosd_arg->osd.custom1_show) {
                    printf("custom1_show OPEN\n");
                    if (setosd_arg->osd.has_show_custom1_str) {
                        printf("show_custom1_str: %s\n", setosd_arg->osd.show_custom1_str);
                    }
                    if (setosd_arg->osd.has_show_custom1_x) {
                        printf("show_custom1_x: %d\n", setosd_arg->osd.show_custom1_x);
                    }
                    if (setosd_arg->osd.has_show_custom1_y) {
                        printf("show_custom1_y: %d\n", setosd_arg->osd.show_custom1_y);
                    }
                } else if (DANAVIDEO_OSD_SHOW_CLOSE == setosd_arg->osd.custom1_show) {
                    printf("custom1_show CLOSE\n");
                } else {
                    printf("custom1_show unknown type[%d]\n", setosd_arg->osd.custom1_show);
                }
                if (DANAVIDEO_OSD_SHOW_OPEN == setosd_arg->osd.custom2_show) {
                    printf("custom2_show OPEN\n");
                    if (setosd_arg->osd.has_show_custom2_str) {
                        printf("show_custom2_str: %s\n", setosd_arg->osd.show_custom2_str);
                    }
                    if (setosd_arg->osd.has_show_custom2_x) {
                        printf("show_custom2_x: %d\n", setosd_arg->osd.show_custom2_x);
                    }
                    if (setosd_arg->osd.has_show_custom2_y) {
                        printf("show_custom2_y: %d\n", setosd_arg->osd.show_custom2_y);
                    }
                } else if (DANAVIDEO_OSD_SHOW_CLOSE == setosd_arg->osd.custom2_show) {
                    printf("custom2_show CLOSE\n");
                } else {
                    printf("custom2_show unknown type[%d]\n", setosd_arg->osd.custom2_show);
                }
                //
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETOSD send response succeeded\n");
                } else {

                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETOSD send response failed\n");
                }
            }
            break;
        case DANAVIDEOCMD_GETOSD:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETOSD\n");
                DANAVIDEOCMD_GETOSD_ARG *getosd_arg = (DANAVIDEOCMD_GETOSD_ARG *)cmd_arg;
                printf("ch_no: %d\n", getosd_arg->ch_no);
                printf("\n");
                error_code = 0;
                code_msg = (char *)"";
                libdanavideo_osdinfo_t osdinfo;

                osdinfo.chan_name_show = DANAVIDEO_OSD_SHOW_CLOSE;
                osdinfo.show_name_x = 1;
                osdinfo.show_name_y = 2;

                osdinfo.datetime_show = DANAVIDEO_OSD_SHOW_CLOSE;
                osdinfo.show_datetime_x = 3;
                osdinfo.show_datetime_y = 4;
                osdinfo.show_format = DANAVIDEO_OSD_DATE_FORMAT_YYYY_MM_DD_CH;
                osdinfo.hour_format = DANAVIDEO_OSD_TIME_24_HOUR;
                osdinfo.show_week = DANAVIDEO_OSD_SHOW_OPEN;
                osdinfo.datetime_attr = DANAVIDEO_OSD_DATETIME_DISPLAY;

                osdinfo.custom1_show = DANAVIDEO_OSD_SHOW_OPEN;
                strncpy(osdinfo.show_custom1_str, "show_custom1_str", sizeof(osdinfo.show_custom1_str) -1);
                osdinfo.show_custom1_x = 5;
                osdinfo.show_custom1_y = 6;

                osdinfo.custom2_show = DANAVIDEO_OSD_SHOW_CLOSE;
                strncpy(osdinfo.show_custom2_str, "show_custom2_str", sizeof(osdinfo.show_custom2_str) -1);
                osdinfo.show_custom2_x = 7;
                osdinfo.show_custom2_y = 8;

                if (lib_danavideo_cmd_getosd_response(danavideoconn, trans_id, error_code, code_msg, &osdinfo)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETOSD send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETOSD send response failed\n");
                }
            }
            break;
        case DANAVIDEOCMD_SETCHANNAME:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHANNAME\n");
                DANAVIDEOCMD_SETCHANNAME_ARG *setchanname_arg = (DANAVIDEOCMD_SETCHANNAME_ARG *)cmd_arg;
                printf("setchanname_arg\n");
                printf("ch_no: %d\n", setchanname_arg->ch_no);
                printf("chan_name: %s\n", setchanname_arg->chan_name);
                printf("\n"); 
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHANNAME send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHANNAME send response failed\n");
                }
            } 
            break;
        case DANAVIDEOCMD_GETCHANNAME:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETCHANNAME\n");
                DANAVIDEOCMD_GETCHANNAME_ARG *getchanname_arg = (DANAVIDEOCMD_GETCHANNAME_ARG *)cmd_arg;
                printf("getchanname_arg\n");
                printf("ch_no: %d\n", getchanname_arg->ch_no);
                printf("\n"); 
                error_code = 0;
                code_msg = "";

                char *chan_name = "Di3";
                if (lib_danavideo_cmd_getchanname_response(danavideoconn, trans_id, error_code, code_msg, chan_name)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETCHANNAME send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETCHANNAME send response failed\n");
                }
            } 
            break;
#ifdef DANAVIDEO_FLIGHT_TEST
            case DANAIOTCMD_FLIGHTCMD:
            {
                printf("TEST danavideoconn_command_handler DANAIOTCMD_FLIGHTCMD\n");
                DANAIOTCMD_FLIGHTCMD_ARG *flightcmd_arg = (DANAIOTCMD_FLIGHTCMD_ARG *)cmd_arg;
                printf("flightcmd_arg\n");
                printf("ch_no: %d\n", flightcmd_arg->ch_no);
                switch (flightcmd_arg->cmd) {
                    case DANAIOT_FLIGHT_CMD_FLY:
                        printf("DANAIOT_FLIGHT_CMD_FLY\n");
                        break;
                    case DANAIOT_FLIGHT_CMD_RETURN:
                        printf("DANAIOT_FLIGHT_CMD_RETURN\n");
                        break;
                    case DANAIOT_FLIGHT_CMD_LAND:
                        printf("DANAIOT_FLIGHT_CMD_LAND\n");
                        break;
                    case DANAIOT_FLIGHT_CMD_CLEAR_ALLWAYPOINTS:
                        printf("DANAIOT_FLIGHT_CMD_CLEAR_ALLWAYPOINTS\n");
                        break;
                    case DANAIOT_FLIGHT_CMD_FOLLOW:
                        printf("DANAIOT_FLIGHT_CMD_FOLLOW\n");
                        break;
                    case DANAIOT_FLIGHT_CMD_MANUAL:
                        printf("DANAIOT_FLIGHT_CMD_MANUAL\n");
                        break;
                    case DANAIOT_FLIGHT_CMD_HOVER:
                        printf("DANAIOT_FLIGHT_CMD_HOVER\n");
                        break;
                }
                printf("\n"); 
                error_code = 0;
                code_msg = "";

                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAIOTCMD_FLIGHTCMD send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAIOTCMD_FLIGHTCMD send response failed\n");
                }
            }
            break;
        case DANAIOTCMD_FLIGHTCONTROL:
            {
                printf("TEST danavideoconn_command_handler DANAIOTCMD_FLIGHTCONTROL\n");
                DANAIOTCMD_FLIGHTCONTROL_ARG *flightcontrol_arg = (DANAIOTCMD_FLIGHTCONTROL_ARG *)cmd_arg;
                printf("flightcontrol_arg\n");
                printf("ch_no: %d\n", flightcontrol_arg->ch_no);
                if (flightcontrol_arg->has_throttle) {
                   printf("throttle: %u\n", flightcontrol_arg->throttle);
                }
                if (flightcontrol_arg->has_rudder) {
                   printf("rudder: %u\n", flightcontrol_arg->rudder);
                }
                if (flightcontrol_arg->has_elevator) {
                   printf("elevator: %u\n", flightcontrol_arg->elevator);
                }
                if (flightcontrol_arg->has_aileron) {
                   printf("aileron: %u\n", flightcontrol_arg->aileron);
                }
                if (flightcontrol_arg->has_servo_1) {
                   printf("servo_1: %u\n", flightcontrol_arg->servo_1);
                }
                if (flightcontrol_arg->has_servo_2) {
                   printf("servo_2: %u\n", flightcontrol_arg->servo_2);
                }
                if (flightcontrol_arg->has_servo_3) {
                   printf("servo_3: %u\n", flightcontrol_arg->servo_3);
                } 
                printf("\n"); 
                error_code = 0;
                code_msg = "";

                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAIOTCMD_FLIGHTCONTROL send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAIOTCMD_FLIGHTCONTROL send response failed\n");
                }
            }
            break;
        case DANAIOTCMD_FLIGHTSTATUS:
            {
                printf("TEST danavideoconn_command_handler DANAIOTCMD_FLIGHTSTATUS\n");
                DANAIOTCMD_FLIGHTSTATUS_ARG *flightstatus_arg = (DANAIOTCMD_FLIGHTSTATUS_ARG *)cmd_arg;
                printf("flightstatus_arg\n");
                printf("ch_no: %d\n", flightstatus_arg->ch_no);
                printf("\n"); 
                error_code = 0;
                code_msg = "";

                libdanaiot_flightstatus_t flightstatus;
                flightstatus.state = DANAIOT_FLIGHT_STATUE_FLYING;
                flightstatus.gps_num = 3;
                flightstatus.battery = 100;
                flightstatus.flight_time = 10000;
                flightstatus.altitude = 80000.123;
                flightstatus.head = 123.45678;
                flightstatus.speed = 999.888;
                flightstatus.pitch = 120.111;
                flightstatus.roll = 234.333;
                flightstatus.distance = 567.444;
                flightstatus.longitude = 178123.4567;
                flightstatus.latitude = 892345.67;
                flightstatus.way_point_num = 25;
                flightstatus.cur_way_point = 3;
                flightstatus.fly_time = 3600;

                if (lib_danaiot_cmd_flightstatus_response(danavideoconn, trans_id, error_code, code_msg, &flightstatus)) {
                    printf("TEST danavideoconn_command_handler DANAIOTCMD_FLIGHTSTATUS send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAIOTCMD_FLIGHTSTATUS send response failed\n");
                }
            }
            break;
        case DANAIOTCMD_SETWAYPOINT:
            {
                printf("TEST danavideoconn_command_handler DANAIOTCMD_SETWAYPOINT\n");
                DANAIOTCMD_SETWAYPOINT_ARG *setwaypoint_arg = (DANAIOTCMD_SETWAYPOINT_ARG *)cmd_arg;
                printf("setwaypoint_arg\n");
                printf("ch_no: %d\n", setwaypoint_arg->ch_no);
                printf("points_count: %u\n", setwaypoint_arg->points_count);
                size_t i;
                for (i=0; i<setwaypoint_arg->points_count; i++) {
                    printf("waypoint[%zd]\n", i);
                    printf("point_id: %u\n", setwaypoint_arg->points[i].point_id);
                    printf("longitude: %f\n", setwaypoint_arg->points[i].longitude);
                    printf("latitude: %f\n", setwaypoint_arg->points[i].latitude);
                    printf("altitude: %f\n", setwaypoint_arg->points[i].altitude);
                    printf("delay: %u\n", setwaypoint_arg->points[i].delay);
                    printf("=====");
                } 
                printf("\n"); 
                error_code = 0;
                code_msg = "";

                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAIOTCMD_SETWAYPOINT send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAIOTCMD_SETWAYPOINT send response failed\n");
                }
            }
            break;
        case DANAIOTCMD_GETWAYPOINT:
            {
                printf("TEST danavideoconn_command_handler DANAIOTCMD_GETWAYPOINT\n");
                DANAIOTCMD_GETWAYPOINT_ARG *getwaypoint_arg = (DANAIOTCMD_GETWAYPOINT_ARG *)cmd_arg;
                printf("getwaypoint_arg\n");
                printf("ch_no: %d\n", getwaypoint_arg->ch_no);
                printf("query_id: %d\n", getwaypoint_arg->query_id);
                printf("\n"); 
                error_code = 0;
                code_msg = "";

                libdanaiot_waypoint_t points[] = {{1, 11.11, 11.12, 11.13, 12}, 
                                                  {2, 22.22, 22.23, 22.24, 23}, 
                                                  {3, 33.33, 33.34, 33.35, 34}, 
                                                  {4, 44.44, 44.45, 44.46, 45}};
                size_t points_count = sizeof(points)/sizeof(libdanaiot_waypoint_t);
                uint32_t max_id = 4; 

                if (lib_danaiot_cmd_getwaypoint_response(danavideoconn, trans_id, error_code, code_msg, points_count, points, max_id)) {
                    printf("TEST danavideoconn_command_handler DANAIOTCMD_GETWAYPOINT send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAIOTCMD_GETWAYPOINT send response failed\n");
                }
            }
            break;
#endif
#ifdef DANAVIDEO_SMARTHOME_TEST
            case DANASMARTHOMECMD_MAKEPAIR:
            {
                printf("TEST danavideoconn_command_handler DANASMARTHOMECMD_MAKEPAIR\n");
                DANASMARTHOMECMD_MAKEPAIR_ARG *makepair_arg = (DANASMARTHOMECMD_MAKEPAIR_ARG *)cmd_arg;
                printf("makepair_arg\n");
                printf("sensor_id: %s\n", makepair_arg->sensor_id);
                printf("\n"); 
                error_code = 0;
                code_msg = "";

                if (lib_danasmarthome_cmd_makepair_response(danavideoconn, trans_id, error_code, code_msg, makepair_arg->sensor_id)) {
                    printf("TEST danavideoconn_command_handler DANASMARTHOMECMD_MAKEPAIR send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANASMARTHOMECMD_MAKEPAIR send response failed\n");
                }
            } 
            break;
        case DANASMARTHOMECMD_DELPAIR:
            {
                printf("TEST danavideoconn_command_handler DANASMARTHOMECMD_DELPAIR\n");
                DANASMARTHOMECMD_DELPAIR_ARG *delpair_arg = (DANASMARTHOMECMD_DELPAIR_ARG *)cmd_arg;
                printf("delpair_arg\n");
                printf("sensor_id: %s\n", delpair_arg->sensor_id);
                printf("\n"); 
                error_code = 0;
                code_msg = "";

                if (lib_danasmarthome_cmd_delpair_response(danavideoconn, trans_id, error_code, code_msg, delpair_arg->sensor_id)) {
                    printf("TEST danavideoconn_command_handler DANASMARTHOMECMD_DELPAIR send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANASMARTHOMECMD_DELPAIR send response failed\n");
                }
            } 
            break;
#endif
#ifdef DANA_RING_TEST
        case DANARINGCMD_GETMODET:
            {

                printf("TEST danavideoconn_command_handler DANARINGCMD_GETMODET\n");
                DANARINGCMD_GETMODET_ARG *getmodet_arg = (DANARINGCMD_GETMODET_ARG *)cmd_arg;
                printf("getmodet_arg\n");
                printf("ch_no: %u\n", getmodet_arg->ch_no);
                printf("\n"); 
                error_code = 0;
                code_msg = "";

                uint32_t set_flag = 1;
                uint32_t region[] = {1, 2, 3, 4};
                uint32_t length = 9; // 单位:m
                uint32_t time = 5; // 单位:s
                if (lib_danaring_cmd_getmodet_response(danavideoconn, trans_id, error_code, code_msg, set_flag, sizeof(region)/sizeof(region[0]), region, &length, &time)) {
                    printf("TEST danavideoconn_command_handler DANARINGCMD_GETMODET send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANARINGCMD_GETMODET send response failed\n");
                }
            }
            break;
        case DANARINGCMD_SETMODET:
            {
                printf("TEST danavideoconn_command_handler DANARINGCMD_SETMODET\n");
                DANARINGCMD_SETMODET_ARG *setmodet_arg = (DANARINGCMD_SETMODET_ARG *)cmd_arg;
                printf("setmodet_arg\n");
                printf("ch_no: %u\n", setmodet_arg->ch_no);
                printf("set_flag: %u\n", setmodet_arg->set_flag);
                printf("region_count: %zd\n", setmodet_arg->region_count);
                size_t i;
                for (i=0; i<setmodet_arg->region_count; i++) {
                    printf("region[%zd]: %u\n", i, setmodet_arg->region[i]);
                }
                printf("has_length: %s\n", setmodet_arg->has_length?"true":"false");
                printf("length: %u\n", setmodet_arg->length);
                printf("has_time: %s\n", setmodet_arg->has_length?"true":"false");
                printf("time: %u\n", setmodet_arg->time);
                printf("\n"); 
                error_code = 0;
                code_msg = "";

                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANARINGCMD_SETMODET send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANARINGCMD_SETMODET send response failed\n");
                }
            }
            break;
        case DANARINGCMD_GETMOTIMEPLAN:
            {
                printf("TEST danavideoconn_command_handler DANARINGCMD_GETMOTIMEPLAN\n");
                DANARINGCMD_GETMOTIMEPLAN_ARG *getmotimeolan_arg = (DANARINGCMD_GETMOTIMEPLAN_ARG *)cmd_arg;
                printf("getmotimeolan_arg\n");
                printf("ch_no: %u\n", getmotimeolan_arg->ch_no);
                printf("\n"); 
                error_code = 0;
                code_msg = "";

                danaring_singletimeplan_t plans[] = {{1, 2, {2,3}, "08:00:00", "18:00:00", 1}, {2, 3, {4, 5, 7}, "09:00:00", "19:00:00", 0}};

                if (lib_danaring_cmd_getmotimeplan_response(danavideoconn, trans_id, error_code, code_msg, sizeof(plans)/sizeof(plans[0]), plans)) {
                    printf("TEST danavideoconn_command_handler DANARINGCMD_GETMOTIMEPLAN send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANARINGCMD_GETMOTIMEPLAN send response failed\n");
                }
            }
            break;
        case DANARINGCMD_SETMOTIMEPLAN:
            {
                printf("TEST danavideoconn_command_handler DANARINGCMD_SETMOTIMEPLAN\n");
                DANARINGCMD_SETMOTIMEPLAN_ARG *setmotimeolan_arg = (DANARINGCMD_SETMOTIMEPLAN_ARG *)cmd_arg;
                printf("setmotimeolan_arg\n");
                printf("ch_no: %u\n", setmotimeolan_arg->ch_no);
                printf("plans_count: %zd\n", setmotimeolan_arg->plans_count);
                size_t j;
                for (j=0; j<setmotimeolan_arg->plans_count; j++) {
                    printf("plan_no: %u\n", setmotimeolan_arg->plans[j].plan_no);
                    printf("week_count: %zd\n", setmotimeolan_arg->plans[j].week_count);
                    size_t i;
                    for (i=0; i<setmotimeolan_arg->plans[j].week_count; i++) {
                        printf("week[%zd]: %u\n", i, setmotimeolan_arg->plans[j].week[i]);
                    }
                    printf("start_time: %s\n", setmotimeolan_arg->plans[j].start_time);
                    printf("end_time: %s\n", setmotimeolan_arg->plans[j].end_time);
                    printf("status: %u\n\n", setmotimeolan_arg->plans[j].status);
                }
                printf("\n"); 
                error_code = 0;
                code_msg = "";

                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANARINGCMD_SETMOTIMEPLAN send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANARINGCMD_SETMOTIMEPLAN send response failed\n");
                }
            }
            break;
#endif
        case DANAVIDEOCMD_CALLPSP:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_CALLPSP\n");
                DANAVIDEOCMD_CALLPSP_ARG *callpsp_arg = (DANAVIDEOCMD_CALLPSP_ARG *)cmd_arg;
                printf("callpsp_arg\n");
                printf("ch_no: %d\n", callpsp_arg->ch_no);
                printf("psp_id: %d\n", callpsp_arg->psp_id);
                printf("\n"); 
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_CALLPSP send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_CALLPSP send response failed\n");
                }
            } 
            break;
        case DANAVIDEOCMD_GETPSP:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETPSP\n");
                DANAVIDEOCMD_GETPSP_ARG *getpsp_arg = (DANAVIDEOCMD_GETPSP_ARG *)cmd_arg;
                printf("getpsp_arg\n");
                printf("ch_no: %d\n", getpsp_arg->ch_no);
                printf("page: %d\n", getpsp_arg->page);
                printf("page_size: %d\n", getpsp_arg->page_size);
                printf("\n"); 
                error_code = 0;
                code_msg = "";

                uint32_t total = 20;
                uint32_t psp_count = 2;
                libdanavideo_pspinfo_t psp[] = {{1, "Psp_1", true, true}, {2, "Psp_2", false, true}};
                if (lib_danavideo_cmd_getpsp_response(danavideoconn, trans_id, error_code, code_msg, total, psp_count, psp)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETPSP send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETPSP send response failed\n");
                }
            } 
            break;
        case DANAVIDEOCMD_SETPSP:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPSP\n");
                DANAVIDEOCMD_SETPSP_ARG *setpsp_arg = (DANAVIDEOCMD_SETPSP_ARG *)cmd_arg;
                printf("setpsp_arg\n");
                printf("ch_no: %d\n", setpsp_arg->ch_no);
                printf("psp_id: %d\n", setpsp_arg->psp.psp_id);
                printf("psp_name: %s\n", setpsp_arg->psp.psp_name);
                printf("psp_default: %s\n", setpsp_arg->psp.psp_default?"true":"false");
                printf("is_set: %s\n", setpsp_arg->psp.is_set?"true":"false");
                printf("\n"); 
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPSP send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPSP send response failed\n");
                }
            } 
            break;
        case DANAVIDEOCMD_SETPSPDEF:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPSPDEF\n");
               DANAVIDEOCMD_SETPSPDEF_ARG *setpspdef_arg = (DANAVIDEOCMD_SETPSPDEF_ARG *)cmd_arg;
                printf("setpspdef_arg\n");
                printf("ch_no: %d\n", setpspdef_arg->ch_no);
                printf("psp_id: %d\n", setpspdef_arg->psp_id);
                printf("\n"); 
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPSPDEF send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETPSPDEF send response failed\n");
                }
            } 
            break; 
        case DANAVIDEOCMD_GETLAYOUT:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETLAYOUT\n");
                DANAVIDEOCMD_GETLAYOUT_ARG *getlayout_arg = (DANAVIDEOCMD_GETLAYOUT_ARG *)cmd_arg;
                printf("getlayout_arg\n");
                printf("ch_no: %d\n", getlayout_arg->ch_no);
                printf("\n"); 
                error_code = 0;
                code_msg = "";

                uint32_t matrix_x = 4; 
                uint32_t matrix_y = 4;
                size_t chans_count = 16;
                uint32_t chans[] = {1, 1, 2, 3, 1, 1, 4, 5, 6, 7, 8, 9, 10, 11, 0, 0};
                uint32_t layout_change = 0;
                uint32_t chan_pos_change = 0;
                size_t use_chs_count = 16;
                uint32_t use_chs[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
                
                if (lib_danavideo_cmd_getlayout_response(danavideoconn, trans_id, error_code, code_msg, matrix_x, matrix_y, chans_count, chans, layout_change, chan_pos_change, use_chs_count, use_chs)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETLAYOUT send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETLAYOUT send response failed\n");
                }
            } 
            break;
        case DANAVIDEOCMD_SETCHANADV:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHANADV\n");
                DANAVIDEOCMD_SETCHANADV_ARG *setchanadv_arg = (DANAVIDEOCMD_SETCHANADV_ARG *)cmd_arg;
                printf("setchanadv_arg\n");
                printf("ch_no: %d\n", setchanadv_arg->ch_no);
                printf("matrix_x: %d\n", setchanadv_arg->matrix_x);
                printf("matrix_y: %d\n", setchanadv_arg->matrix_y);
                size_t i;
                for (i=0; i<setchanadv_arg->chans_count; i++) {
                    printf("chans[%zd]: %d\n", i, setchanadv_arg->chans[i]);
                }
                printf("\n"); 
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHANADV send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETCHANADV send response failed\n");
                }
            } 
            break;
        
        // 日夜转换接口
        case DANAVIDEOCMD_SETICR:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETICR\n");
                DANAVIDEOCMD_SETICR_ARG *seticr_arg = (DANAVIDEOCMD_SETICR_ARG *)cmd_arg;
                printf("seticr_arg\n");
                printf("ch_no: %d\n", seticr_arg->ch_no);
                switch (seticr_arg->mode) {
                    case DANA_ICR_MODE_COLOR:
                        printf("DANA_ICR_MODE_COLOR\n");
                        break;
                    case DANA_ICR_MODE_BW:
                        printf("DANA_ICR_MODE_BW\n");
                        break;
                    case DANA_ICR_MODE_AUTO:
                        printf("DANA_ICR_MODE_BW\n");
                        break;
                    default:
                        printf("UnKnown mode: %d\n", seticr_arg->mode);
                }
                
                printf("\n"); 
                error_code = 0;
                code_msg = "";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETICR send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_SETICR send response failed\n");
                }
            } 
            break;
        case DANAVIDEOCMD_GETICR:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETICR\n");
                DANAVIDEOCMD_GETICR_ARG *geticr_arg = (DANAVIDEOCMD_GETICR_ARG *)cmd_arg;
                printf("geticr_arg\n");
                printf("ch_no: %d\n", geticr_arg->ch_no);
                printf("\n"); 
                error_code = 0;
                code_msg = "";

                uint32_t mode = DANA_ICR_MODE_COLOR; // DANA_ICR_MODE_COLOR DANA_ICR_MODE_BW DANA_ICR_MODE_AUTO 
                if (lib_danavideo_cmd_geticr_response(danavideoconn, trans_id, error_code, code_msg, mode)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETICR send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETICR send response failed\n");
                }
            } 
            break;

        case DANAVIDEOCMD_GETSDCSTATUS:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETSDCSTATUS\n");
                DANAVIDEOCMD_GETSDCSTATUS_ARG *getsdcstatus_arg = (DANAVIDEOCMD_GETSDCSTATUS_ARG *)cmd_arg;
                printf("getsdcstatus_arg\n");
                printf("ch_no: %d\n", getsdcstatus_arg->ch_no);

                printf("\n"); 
                error_code = 0;
                code_msg = "";
                uint32_t status = 1;
                uint32_t format_progress = 10;
                uint32_t sd_size = 0;
                uint32_t sd_free = 0;
				struct statfs diskInfo; 		
				/*
				 *  get SD card information	
				 */
				
				statfs(GRD_SD_MOUNT_POINT, &diskInfo);	
				
				unsigned long long blocksize = diskInfo.f_bsize;	//每个block里包含的字节数  
				unsigned long long totalsize = blocksize * diskInfo.f_blocks;	//总的字节数，f_blocks为block的数目  
				
				printf("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n",   
					totalsize, totalsize>>10, totalsize>>20, totalsize>>30);  
				  
				unsigned long long freeDisk = diskInfo.f_bfree * blocksize; //剩余空间的大小  
				unsigned long long availableDisk = diskInfo.f_bavail * blocksize;	//可用空间大小	
				
				printf("Disk_free = %llu MB = %llu GB\nDisk_available = %llu MB = %llu GB\n",   
					freeDisk>>20, freeDisk>>30, availableDisk>>20, availableDisk>>30); 
				
				if(0 > get_sd_stats())//判断SD卡是否存在
				{
					sd_size = 0;
					sd_free = 0;
				}
				else
				{
					sd_size = totalsize;
					sd_free = availableDisk;
				}
				status = DANA_SDC_STATUS_NOMAL;

                if (lib_danavideo_cmd_getsdcstatus_response(danavideoconn, trans_id, error_code, code_msg, status, &format_progress, &sd_size, &sd_free)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETSDCSTATUS send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETSDCSTATUS send response failed\n");
                }
            }
            break;

        case DANAVIDEOCMD_GETVIDEO:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETVIDEO\n");
                DANAVIDEOCMD_GETVIDEO_ARG *getvideo_arg = (DANAVIDEOCMD_GETVIDEO_ARG *)cmd_arg;
                printf("getvideo_arg\n");
                printf("ch_no: %d\n", getvideo_arg->ch_no);

                printf("\n"); 
                error_code = 0;
                code_msg = "";
                uint32_t video_quality = 80;

                if (lib_danavideo_cmd_getvideo_response(danavideoconn, trans_id, error_code, code_msg, video_quality)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETVIDEO send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_GETVIDEO send response failed\n");
                }
            }
            break;

        case DANAVIDEOCMD_RESOLVECMDFAILED:
            {
                printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RESOLVECMDFAILED\n");
                // 根据不同的method,调用lib_danavideo_cmd_response
                error_code = 20145;
                code_msg = (char *)"danavideocmd_resolvecmdfailed";
                if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)cmd_arg, trans_id, error_code, code_msg)) {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RESOLVECMDFAILED send response succeeded\n");
                } else {
                    printf("TEST danavideoconn_command_handler DANAVIDEOCMD_RESOLVECMDFAILED send response failed\n");
                }
            }
            break;
        default:
            {
                printf("TEST danavideoconn_command_handler UnKnown CMD: %d\n", cmd);
            }
            break;
    }

    return;
}

dana_video_callback_funs_t danavideocallbackfuns = {
    .danavideoconn_created = danavideoconn_created,
    .danavideoconn_aborted = danavideoconn_aborted,
    .danavideoconn_command_handler = danavideoconn_command_handler,
};



void danavideo_hb_is_ok(void)
{
    printf("THE CONN TO P2P SERVER IS OK\n");
}

void danavideo_hb_is_not_ok(void) 
{
    printf("THE CONN TO P2P SERVER IS NOT OK\n");
}

void danavideo_upgrade_rom(const char* rom_path,  const char *rom_md5, const uint32_t rom_size)
{
    printf("NEED UPGRADE ROM rom_path: %s\trom_md5: %s\trom_size: %d\n", rom_path, rom_md5, rom_size);
}

void danavideo_autoset(const uint32_t power_freq, const int64_t now_time, const char *time_zone, const char *ntp_server1, const char *ntp_server2)
{
    printf("AUTOSET\n\tpower_freq: %d\n\tnow_time: %"PRId64"\n\ttime_zone: %s\n\tntp_server1: %s\n\tntp_server2: %s\n", power_freq, now_time, time_zone, ntp_server1, ntp_server2);
	char cmd[64] = {0};
	struct timeval    tv;
    struct timezone   tz;
	struct tm *now;

	time_t total_secs = now_time;
	now = localtime(&total_secs);
	printf("%02d/%02d/%d %02d:%02d:%02d\n", now->tm_mday, now->tm_mon+1, now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);

	gettimeofday(&tv, &tz);
	if(DIFF_ABS(now_time, tv.tv_sec) > 10)
	{
		sprintf(cmd, "date -s %04d%02d%02d%02d%02d.%02d", now->tm_year+1900, now->tm_mon+1,
						now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
		printf("Danale Platform set system time with comand \"%s\"\n", cmd);
		system(cmd);
	}
}

volatile bool danaairlink_set_wifi_cb_called = false;

void danavideo_setwifiap(const uint32_t ch_no, const uint32_t ip_type, const char *ipaddr, const char *netmask, const char *gateway, const char *dns_name1, const char *dns_name2, const char *essid, const char *auth_key, const uint32_t enc_type)
{
    printf("SETWIFIAP\n\tch_no: %d\n\tip_type: %d\n\tipaddr: %s\n\tnetmask: %s\n\tgateway: %s\n\tdns_name1: %s\n\tdns_name2: %s\n\tessid: %s\n\tauth_key: %s\n\tenc_type: %d\n", ch_no, ip_type, ipaddr, netmask, gateway, dns_name1, dns_name2, essid, auth_key, enc_type);
    printf("Config WiFi...\n");
	set_wifi_config(essid, auth_key);
    
    danaairlink_set_wifi_cb_called = true; // 主线程会再次进入配置状态
}

// 0 succeeded
// 1 failed
uint32_t danavideo_local_auth(const char *user_name, const char *user_pass)
{
    printf("LocalAuth\n\tuser_name: %s\n\tuser_pass: %s\n", user_name, user_pass);
    if(strcasecmp(user_name, "xiaohuozi") == 0 && strcasecmp(user_pass, "daguniang") == 0){
        return 0;
    }

    return 0;
    // 用户名不区分大小写
    // 密码区分大小写
}
   
void danavideo_conf_create_or_updated(const char *conf_absolute_pathname)
{
    printf("CONF_create_or_updated  conf_absolute_pathname: %s\n", conf_absolute_pathname);
}

uint32_t danavideo_get_connected_wifi_quality()
{
	uint32_t wifi_quality = 0;
	char pData[64] = {0};
	char *pointer;

	if(g_network_info.ucNetworkinit == 0 || g_network_info.ucNetworkType == 0)
	{
		return 0;
	}
	
	FILE *pstatus = popen("wpa_cli -iwlan0 status | grep signal_level", "r");
	if(pstatus == NULL)
	{
		wifi_quality = 0;
	}
	else
	{
		fgets(pData, sizeof(pData), pstatus);
		pointer = pData;
		pointer += strlen("signal_level=");
		wifi_quality = atoi(pointer);
		pclose(pstatus);
	}
    //printf("Connect WiFi Quality: %d\n", wifi_quality);
    return wifi_quality;
}

// 生产工具可以设置这些信息
void danavideo_productsetdeviceinfo(const char *model, const char *sn, const char *hw_mac)
{
    printf("danavideo_productsetdeviceinfo\n");
    printf("model: %s\tsn: %s\thw_mac: %s\n", model, sn, hw_mac);
}

#ifdef DANAVIDEO_SMARTHOME_TEST
void dana_smarthome_event_handler(const char *sensor_id, const dana_smarthome_event_t event, const char event_arg[512])
{
    printf("TEST dana_smarthome_event_handler\n");
    if (NULL != sensor_id) {
        printf("sensor_id: %s\n", sensor_id);
        printf("event: %u\n", event);
        if (NULL != event_arg) {
            printf("event_arg: %s\n", event_arg);
        }
    }
}
#endif

int dana_conf_reload()
{
	//Prevent damage of the file when the device power-off
	if(access("/mnt/mtd/run/danale.conf",F_OK)!= 0)
	{
		system("cp -f /config/platform/*.conf /mnt/mtd/run/danale.conf");
	}
}

static  bool g_dana_run = 1;
void *dana_av_init (void *args) {
//dbg_on();
    volatile bool lib_danavideo_inited = false, lib_danavideo_started = false;
    volatile bool danaairlink_inited = false, danaairlink_started = false;

    printf("testdanavideo start\n");

	dana_conf_reload();
    char *danale_path = "/mnt/mtd/run/";
    uint32_t channel_num = 1;
	char ssid[32];
	char key[32];

    printf("testdanavideo USING LIBDANAVIDEO_VERSION %s\n", lib_danavideo_linked_version_str(lib_danavideo_linked_version()));
    //建立一系列回调函数
    lib_danavideo_set_hb_is_ok(danavideo_hb_is_ok);
    lib_danavideo_set_hb_is_not_ok(danavideo_hb_is_not_ok);

    lib_danavideo_set_upgrade_rom(danavideo_upgrade_rom);

    lib_danavideo_set_autoset(danavideo_autoset);
    lib_danavideo_set_local_setwifiap(danavideo_setwifiap);

    lib_danavideo_set_local_auth(danavideo_local_auth);

    lib_danavideo_set_conf_created_or_updated(danavideo_conf_create_or_updated);

    lib_danavideo_set_get_connected_wifi_quality(danavideo_get_connected_wifi_quality);

    lib_danavideo_set_productsetdeviceinfo(danavideo_productsetdeviceinfo);

#if 0//def DANAVIDEO_SMARTHOME_TEST
    lib_danasmarthome_register_event_handle(dana_smarthome_event_handler);
#endif


#if 1 // 测试DanaAirLink
	while(g_network_info.ucNetworkinit == 0)
	{
		sleep(2);
	}
    if((get_wifi_config(ssid, sizeof(ssid), key, sizeof(key)) < 0) && g_network_info.ucNetworkType==1)
    {
	    // DanaAirLink
	    // 目前支持MT7601, RTL8188, RT3070等芯片,如果采用的是其他芯片可以联系Danale
	    // 某些芯片需要采用大拿提供的驱动
	    // 需要lib_danavideo_set_local_setwifiap注册配置Wi-Fi回调函数
	    char *if_name = "wlan0";
	    if (!danaairlink_set_danalepath(danale_path)) {
	        printf("testdanavideo danaairlink_set_danalepath failed\n");
	        return -1;
	    }
	    if (!danaairlink_init(DANAAIRLINK_CHIP_TYPE_RLT8188, if_name)) {
	        printf("testdanavideo danaairlink_set_danalepath failed\n");
	        return -1;
	    }
	    danaairlink_inited = true;
	    printf("testdanavideo danaairlink_init succeeded\n");
	    if (!danaairlink_start_once()) {
	        printf("testdanavideo danaairlink_start_once failed\n");
	        danaairlink_deinit();
	        return -1;
	    }
	    danaairlink_started = true;
	    printf("testdanavideo danaairlink_start_once succeeded\n");

		while (g_network_info.ucNetworkState == 0)
		{
			if (!danaairlink_set_wifi_cb_called)
			{
				if (danaairlink_start_once()) {
	                printf("testdanavideo danaairlink_start_once succeeded\n");
	            } else {
	                printf("testdanavideo WARING danaairlink_start_once failed\n");
	            }
			}
			else
			{
				danaairlink_stop();
				danaairlink_deinit();
			}
			sleep(2);
		}
    }
	else if(g_network_info.ucNetworkType==0)
	{
		while (g_network_info.ucNetworkState == 0)
		{
			sleep(2);
			continue;
		}
	}
#if 0
    while (g_network_info.ucNetworkState == 0) {
        if (danaairlink_set_wifi_cb_called) 
		{
            // 演示再次进入配置状态
            danaairlink_set_wifi_cb_called = false;
            if (danaairlink_start_once()) {
                printf("testdanavideo danaairlink_start_once succeeded\n");
            } else {
                printf("testdanavideo WARING danaairlink_start_once failed\n");
            }
        } else {
            sleep(2);
        }
    }
#endif
    // 可以随时调用danaairlink_stop停止配置
    // 最后调用danaairlink_deinit清理资源
#endif 

    // 参数说明
    // 参数获得请联系大拿.
    // 如果配置文件danale.conf 已经存在或者本地部署生产服务器,则参数agent_user agent_pass的赋值可以为空.
    char *agent_user = NULL;
    char *agent_pass = NULL;
    char *chip_code = "TEST_chip";
    char *schema_code = "TEST_schema";
    char *product_code = "TEST_product";

    int32_t maximum_size = 3*1024*1024;
    lib_danavideo_set_maximum_buffering_data_size(maximum_size);

    uint32_t libdanavideo_startup_timeout_sec = 30;
    lib_danavideo_set_startup_timeout(libdanavideo_startup_timeout_sec);

    // 现在也可以在初始化之前调用了
    printf("testdanavideo lib_danavideo_deviceid: %s\n", lib_danavideo_deviceid_from_conf(danale_path));

#if 1 //  在lib_danavideo_start之前调用 默认是 fixed 34102
    bool listen_port_fixed = true; // false
    uint16_t listen_port = 12349;
    lib_danavideo_set_listen_port(listen_port_fixed, listen_port);
#endif

    printf("testdanavideo 1 lib_danavideo_get_listen_port: %"PRIu16"\n", lib_danavideo_get_listen_port());

    while (!lib_danavideo_init(danale_path, agent_user, agent_pass, chip_code, schema_code, product_code, &danavideocallbackfuns)) {
//wch modify, orignal is #if 0
#if 0 // 测试自动接收配置文件功能
        // 创建(检查)配置文件(如果配置文件存在且有效，则不会修改配置文件)
        while (!lib_danavideo_create_on_check_conf()) {
            printf("testdanavideo lib_danavideo_create_on_check_conf failed, try again\n");
            sleep(2);
        }
        // 需要保证配置文件创建成功
        printf("testdanavideo lib_danavideo_create_on_check_conf succeeded\n");
#endif
        printf("testdanavideo lib_danavideo_init failed, try again\n");
        sleep(1);
    }
    printf("testdanavideo lib_danavideo_init succeeded\n");
    lib_danavideo_inited = true;

    printf("testdanavideo 2 lib_danavideo_get_listen_port: %"PRIu16"\n", lib_danavideo_get_listen_port());


    // lib_danavideo_init成功之后可以调用lib_danavideo_deviceid()获取设备标识码
    printf("testdanavideo lib_danavideo_deviceid: %s\n", lib_danavideo_deviceid());

    while (!lib_danavideo_start()) {
        printf("testdanavideo lib_danavideo_start failed, try again\n");
        sleep(1);
    }
    printf("testdanavideo lib_danavideo_start succeeded\n");
    lib_danavideo_started = true;

    printf("testdanavideo 3 lib_danavideo_get_listen_port: %"PRIu16"\n", lib_danavideo_get_listen_port());

#if 0 //  在lib_danavideo_start之后调用 默认是 fixed 34102
    bool listen_port_fixed = true; // false
    uint16_t listen_port = 12345;
    lib_danavideo_set_listen_port(listen_port_fixed, listen_port);
    printf("testdanavideo 4 lib_danavideo_get_listen_port: %"PRIu16"\n", lib_danavideo_get_listen_port());
#endif

#ifdef DANAVIDEO_CLOUD_TEST
    // 需要在lib_danavideo_init成功之后调用
    printf("testdanavideo USING LIBDANAVIDEOCLOUD_VERSION %s\n", lib_danavideo_cloud_linked_version_str(lib_danavideo_cloud_linked_version()));

    uint32_t max_buffering_data = 1*1024*1024;
    int32_t package_size = 4*1024*1024;

    lib_danavideo_cloud_set_cloud_mode_changed(danavideo_cloud_mode_changed_callback); // 注册云存储计划改变回调函数
    while (!lib_danavideo_cloud_init(channel_num, max_buffering_data, package_size, DANAVIDEO_CLOUD_MODE_REALTIME)) { // 目前只支持以DANAVIDEO_CLOUD_MODE_REALTIME初始化
        printf("testdanavideo lib_danavideo_cloud_init failed, try again\n");
        sleep(2);
    }
    printf("testdanavideo lib_danavideo_cloud_init succeeded\n");
    lib_danavideo_cloud_inited = true;
#endif
#if 1
		dana_video_feature_t feature_list[] = { 
			//DANAVIDEO_FEATURE_HAVE_BATTERY,
			//DANAVIDEO_FEATURE_HAVE_GPS,
			//DANAVIDEO_FEATURE_HAVE_PTZ_L_R_U_D,
			//DANAVIDEO_FEATURE_HAVE_WIPER,
			//DANAVIDEO_FEATURE_HAVE_ZOOM_LENS,
			DANAVIDEO_FEATURE_HAVE_SD,
			//DANAVIDEO_FEATURE_HAVE_MIC,
			//DANAVIDEO_FEATURE_HAVE_SPEAKER,
			DANA_VIDEO_HAVE_CLOUD_STORAGE };
		if (lib_danavideo_util_setdevicefeature(sizeof(feature_list)/sizeof(dana_video_feature_t), feature_list)) {
			dbg("\x1b[32mtestdanavideo TEST lib_danavideo_util_setdevicefeature success\x1b[0m\n");
		} else {
			dbg("\x1b[34mtestdanavideo TEST lib_danavideo_util_setdevicefeature failed\x1b[0m\n");
		}
#endif

    danavideo_device_type_t device_type = DANAVIDEO_DEVICE_IPC;
    channel_num = 1;
    char *rom_ver = "rom_ver";
    char *api_ver = "api_ver";
    char *rom_md5 = "rom_md5";
    if (lib_danavideo_util_setdeviceinfo(device_type, channel_num, rom_ver, api_ver, rom_md5)) {
        printf("\x1b[32mtestdanavideo TEST lib_danavideo_util_setdeviceinfo success\x1b[0m\n");
    } else {
        printf("\x1b[34mtestdanavideo TEST lib_danavideo_util_setdeviceinfo failed\x1b[0m\n");
    }

#if 1 // lib_danavideo_util_pushmsg
    uint32_t ch_no = 1;
    uint32_t alarm_level = DANA_VIDEO_PUSHMSG_ALARM_LEVEL_2;
    uint32_t msg_type = DANA_VIDEO_PUSHMSG_MSG_TYPE_DOOR_SENSOR;
    char     *msg_title = "TEST danavideo_utili";
    char     *msg_body  = "lib_danavideo_util_pushmsg";
    int64_t  cur_time = 0;
#if 0 // 不设置附件. 不设置录像   
    uint32_t att_flag = 0;
    uint32_t record_flag = 0;
    if (lib_danavideo_util_pushmsg(ch_no, alarm_level, msg_type, msg_title, msg_body, cur_time, att_flag, NULL, NULL, record_flag, 0, 0, 0, NULL)) {
        printf("\x1b[32mtestdanavideo TEST lib_danavideo_util_pushmsg success\x1b[0m\n");
    } else {
        printf("\x1b[34mtestdanavideo TEST lib_danavideo_util_pushmsg failed\x1b[0m\n");
    }
#endif

#if 0 // 设置附件. 不设置录像   
    uint32_t att_flag = 1;
    uint32_t record_flag = 0;
    char     *att_path = "./testpushmsg/testatt";
    char     *att_type = "JPG";
    if (lib_danavideo_util_pushmsg(ch_no, alarm_level, msg_type, msg_title, msg_body, cur_time, att_flag, att_path, att_type, record_flag, 0, 0, 0, NULL)) {
        printf("\x1b[32mtestdanavideo TEST lib_danavideo_util_pushmsg success\x1b[0m\n");
    } else {
        printf("\x1b[34mtestdanavideo TEST lib_danavideo_util_pushmsg failed\x1b[0m\n");
    }
#endif

#if 0 // 不设置附件. 设置录像   
    uint32_t att_flag = 0;
    uint32_t record_flag = 1;
    int64_t  start_time = 0;
    uint32_t time_len = 0;
    uint32_t save_site = 1; //1,SD;2, 华为存储; 3, S3存储;4, Google存储;5, 阿里云存储;6, 七牛云存储
    char     *save_path = "./testpushmsg/testrecord";
    if (lib_danavideo_util_pushmsg(ch_no, alarm_level, msg_type, msg_title, msg_body, cur_time, att_flag, NULL, NULL, record_flag, start_time, time_len, save_site, save_path)) {
        printf("\x1b[32mtestdanavideo TEST lib_danavideo_util_pushmsg success\x1b[0m\n");
    } else {
        printf("\x1b[34mtestdanavideo TEST lib_danavideo_util_pushmsg failed\x1b[0m\n");
    }
#endif


#if 0 // 设置附件. 设置录像    
    uint32_t att_flag = 1;
    uint32_t record_flag = 1;
    char     *att_path = "./testpushmsg/testatt";
    char     *att_type = "JPG";
    int64_t  start_time = 0;
    uint32_t time_len = 0;
    uint32_t save_site = 1; //1,SD;2, 华为存储; 3, S3存储;4, Google存储;5, 阿里云存储;6, 七牛云存储
    char     *save_path = "./testpushmsg/testrecord";
    if (lib_danavideo_util_pushmsg(ch_no, alarm_level, msg_type, msg_title, msg_body, cur_time, att_flag, att_path, att_type, record_flag, start_time, time_len, save_site, save_path)) {
        printf("\x1b[32mtestdanavideo TEST lib_danavideo_util_pushmsg success\x1b[0m\n");
    } else {
        printf("\x1b[34mtestdanavideo TEST lib_danavideo_util_pushmsg failed\x1b[0m\n");
    }

#endif
#endif // lib_danavideo_util_pushmsg


    // 测试searchapp接口
#if 0// lib_danavideo_local_searchapp
    char *check_data = "check_data_di3";
    uint32_t encrypt_flag_searchapp = 1; // 0 不加密 1 加密方式1加密  其他(暂不支持)
    int32_t i;
    for (i=0; i<10; i++) {
        if (lib_danavideo_local_searchapp(check_data, encrypt_flag_searchapp)) {
            printf("\x1b[32mtestdanavideo TEST lib_danavideo_local_searchapp success\x1b[0m\n");
        } else {
            printf("\x1b[34mtestdanavideo TEST lib_danavideo_local_searchapp failed\x1b[0m\n");
        }

        usleep(3000*1000);
    }

#endif // lib_danavideo_local_searchapp

#if 0//def DANAVIDEO_SMARTHOME_TEST
    // 测试event_trigger
    char *trigger_event_sensor_id = "trigger_event_sensor_id";
    dana_smarthome_event_t trigger_event_event = 1;
    lib_danasmarthome_trigger_sensorevent(trigger_event_sensor_id, trigger_event_event);
#endif

	static int mdtriggerd = -1;
    while (g_dana_run) 
	{
		if(g_structMotionCtl.ullbitsMDstate & 0x01 == 0)
		{
			mdtriggerd = 0;
		}
		else
		{
			if(mdtriggerd <= 0)
			{
				danale_util_pullmsg();
				mdtriggerd = 1;
			}
			if(g_structMotionCtl.ullbitsMDstate & 0x07 == 0)
			{
				mdtriggerd = 0;
			}
		}
		usleep(100000);
        //sleep(300000);
    }

testdanavideoexit:

#ifdef DANAVIDEO_CLOUD_TEST
    // 退出实时/告警上传模式
    danavideo_cloud_leave_alarm_upload();
    danavideo_cloud_leave_realtime_upload();

    if (lib_danavideo_cloud_inited) {
        lib_danavideo_cloud_deinit();
        lib_danavideo_cloud_inited = false;
    }
#endif

    if (danaairlink_started) {
        danaairlink_stop();
        danaairlink_started = false;
    }
    if (danaairlink_inited) {
        danaairlink_deinit();
    }
    danaairlink_inited = false;

    if (lib_danavideo_started) {
        lib_danavideo_stop();
        lib_danavideo_started = false;
    }
    usleep(800*1000);
    if (lib_danavideo_inited) {
        lib_danavideo_deinit();
        lib_danavideo_inited = false;
    }

    usleep(800*1000);

    printf("testdanavideo exit\n");

    return 0;
}

void dana_server_start()
{
    g_dana_run  = 1;
	pthread_attr_t attr;
	pthread_t dana_server_thread;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);  //分离的线程
	pthread_create(&dana_server_thread, &attr, dana_av_init,NULL);
	pthread_attr_destroy(&attr);
}

void dana_server_stop()
{
    g_dana_run = 0;
}

// 时间结构体
typedef struct
{
    unsigned char   second;     /* 0-59 */
    unsigned char   minute;     /* 0-59 */
    unsigned char   hour;       /* 0-23 */
    unsigned char   day;        /* 1-31 */
    unsigned char   month;      /* 1-12 */
    unsigned short  year;       /* 1994-2099 */
    unsigned char   week;       /* 1-7 */
    unsigned char   Count10ms;  /* 0-99 */
} ClockStruc;

/**********************************************************************
* 功能描述：当前时间
* 输入参数：ptTime-时间结构体
* 输出参数：ptTime-时间结构体
* -------------------------------------------------------------------
***********************************************************************/
void getCurrentTime(ClockStruc *ptTime)
{
    long    dt           = 0;
    struct  tm      *tm1 = NULL;
    struct  timeval  tp  = {0};

    // get real clock from system
    gettimeofday(&tp, NULL);
    dt  = tp.tv_sec;
    tm1 = localtime(&dt);
    ptTime->Count10ms = tp.tv_usec / 10000;
    ptTime->year      = (tm1->tm_year + 1900);
    ptTime->month     = tm1->tm_mon + 1;
    ptTime->day       = tm1->tm_mday;
    ptTime->hour      = tm1->tm_hour;
    ptTime->minute    = tm1->tm_min;
    ptTime->second    = tm1->tm_sec;
    ptTime->week      = tm1->tm_wday;
    if (ptTime->week == 0)   // Sunday
    {
        ptTime->week = 7;
    }
}



/**********************************************************************
* 功能描述：判断当前时间是否在配置的时间范围内
* 输入参数：pTime-时间结构体
* 输出参数：pTime-时间结构体
* 返 回 值：0-在范围内   -1-不在范围内
* 其它说明：无
* 修改日期         版本号        修改人           修改内容
* -------------------------------------------------------------------
***********************************************************************/
int IsInTimeSpan(ClockStruc *pTimeNow, ClockStruc *pBeginTime, ClockStruc *pEndTime)
{
    char iBegLessThanEnd = 0;       // 1-配置的开始时间小于结束时间  0-配置的开始时间大于结束时间

    if (pBeginTime->hour < pEndTime->hour || (pBeginTime->hour == pEndTime->hour && pBeginTime->minute <= pEndTime->minute))
    {
        iBegLessThanEnd = 1;
    }
    else
    {
        iBegLessThanEnd = 0;
    }

    if (iBegLessThanEnd)   // 开始时间小于结束时间
    {
         if ((pTimeNow->hour > pBeginTime->hour || (pTimeNow->hour == pBeginTime->hour && pTimeNow->minute >= pBeginTime->minute))
            && (pTimeNow->hour < pEndTime->hour || (pTimeNow->hour == pEndTime->hour && pTimeNow->minute <= pEndTime->minute)))
         {
             return 0;
         }
         else
         {
             return -1;
         }
    }
    else   // 开始时间大于结束时间, 跨天的情况
    {
         if ((pTimeNow->hour > pBeginTime->hour || (pTimeNow->hour == pBeginTime->hour && pTimeNow->minute >= pBeginTime->minute))
            || (pTimeNow->hour < pEndTime->hour || (pTimeNow->hour == pEndTime->hour && pTimeNow->minute <= pEndTime->minute)))
         {
             return 0;
         }
         else
         {
             return -1;
         }
    }
}

static int  SHCEDULE_PLAN_RECODER = 0;
enum RECORDER_INFO
{
   OFF_RECODER=0,
   NO_WEEKPLAN,
   CURRTIME_NO_IN_WEEKPLAN,
   CURRENTTIME_IN_WEEKPLAN,
};
int get_param_recoder_cfg(ClockStruc *tBeginTime,ClockStruc *tEndTime,int nowweekday)
{
	sdk_record_cfg_t record_param;
	int i,weekday,week_count=0,FLAG=CURRTIME_NO_IN_WEEKPLAN;
	uint32_t week[7];

	memset(&record_param,0,sizeof(sdk_record_cfg_t));
	adapt_param_get_record_cfg(0,&record_param);
	if(record_param.record_types==RECORD_NO)
	{
		SHCEDULE_PLAN_RECODER = 0;
		FLAG=OFF_RECODER;
		goto end;
	}
	
	for (i=1; i<=7; i++) {
	weekday = i;
	
	if(record_param.record_sched[weekday-1].sched_time[0].plan_flag != 1)
		continue;//按周布放
	
	if (DANAVIDEO_REC_WEEK_MON == weekday) {
		printf("week: DANAVIDEO_REC_WEEK_MON\n");
		week[week_count] = DANAVIDEO_REC_WEEK_MON;
	} else if (DANAVIDEO_REC_WEEK_TUE == weekday) {
		printf("week: DANAVIDEO_REC_WEEK_TUE\n");
		week[week_count] = DANAVIDEO_REC_WEEK_TUE;
	} else if (DANAVIDEO_REC_WEEK_WED == weekday) {
		printf("week: DANAVIDEO_REC_WEEK_WED\n");
		week[week_count] = DANAVIDEO_REC_WEEK_WED;
	} else if (DANAVIDEO_REC_WEEK_THU == weekday) {
		printf("week: DANAVIDEO_REC_WEEK_THU\n");
		week[week_count] = DANAVIDEO_REC_WEEK_THU;
	} else if (DANAVIDEO_REC_WEEK_FRI == weekday) {
		printf("week: DANAVIDEO_REC_WEEK_FRI\n");
		week[week_count] = DANAVIDEO_REC_WEEK_FRI;
	} else if (DANAVIDEO_REC_WEEK_SAT == weekday) {
		printf("week: DANAVIDEO_REC_WEEK_SAT\n");
		week[week_count] = DANAVIDEO_REC_WEEK_SAT;
	} else if (DANAVIDEO_REC_WEEK_SUN == weekday) {
		printf("week: DANAVIDEO_REC_WEEK_SUN\n");
		week[week_count] = DANAVIDEO_REC_WEEK_SUN;
	} else {
		printf("Unknown week: %d\n", weekday);
	} 
	week_count ++;
	} 
	
	if(week_count<=0)
	{	
		SHCEDULE_PLAN_RECODER = 0;
		FLAG = NO_WEEKPLAN;
		goto end;
	}
	
	weekday = week[0];
	tBeginTime->hour=record_param.record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].start_hour;
	tBeginTime->minute=record_param.record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].start_min;
	tBeginTime->second=record_param.record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].start_sec;
	
	tEndTime->hour=record_param.record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].stop_hour;
	tEndTime->minute=record_param.record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].stop_min;
	tEndTime->second=record_param.record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].stop_sec;

	for(i=0;i<week_count;i++)
	{
		if(nowweekday==week[i])
		{
			FLAG = CURRENTTIME_IN_WEEKPLAN;
			goto end;
		}
	}
	FLAG = CURRTIME_NO_IN_WEEKPLAN;
end:	
	return FLAG;
	
}

void *shcedule_is_recorde_thread()
{
    ClockStruc tBeginTime  = {0};      // 开始时间
    ClockStruc tEndTime    = {0};      // 结束时间
    ClockStruc tTimeNow    = {0};      // 当前时间
	
	int iRetVal=0,plan_recoder = 0,FLAG=0;
	

	SHCEDULE_PLAN_RECODER = 0;
	getCurrentTime(&tTimeNow);
	FLAG = get_param_recoder_cfg(&tBeginTime,&tEndTime,tTimeNow.week);
	printf("<<<<<<<<enter<<shcedule_is_recorde_thread>>>>>>>>>>>>>\n");
	while(1)
	{	
		getCurrentTime(&tTimeNow);	
	    //如果时间小于2017年则不录像
	    if(tTimeNow.year<2017)
	    {   
			SHCEDULE_PLAN_RECODER = 0;
	        sleep(10);
			continue;
	    }
		printf(">>>>>current weekday=%d time y=%d,m=%d,d=%d,h=%d,m=%d,s=%d\n",tTimeNow.week,tTimeNow.year,tTimeNow.month,tTimeNow.day,tTimeNow.hour,tTimeNow.minute);
		if(GET_DANA_SET_RECPLAN_CLICK==1||GET_DANA_SET_TIME_CLICK==1)
		{		
			GET_DANA_SET_RECPLAN_CLICK=0;
			GET_DANA_SET_TIME_CLICK = 0;
			printf(">>>>>>>>1>>>>GET_DANA_SET_RECPLAN_CLICK==%d>\n",GET_DANA_SET_RECPLAN_CLICK);
			FLAG = get_param_recoder_cfg(&tBeginTime,&tEndTime,tTimeNow.week);
		}

		if(FLAG == OFF_RECODER || FLAG==NO_WEEKPLAN ||FLAG==CURRTIME_NO_IN_WEEKPLAN)
	    {   
	        
			printf("<><><><><><><>ooooo,Wrong week recorder plan<><><><><><><\n");
			SHCEDULE_PLAN_RECODER = 0;
			FLAG = get_param_recoder_cfg(&tBeginTime,&tEndTime,tTimeNow.week);
	        sleep(10);
			continue;
	    }
		else if(FLAG==CURRENTTIME_IN_WEEKPLAN)
		{
		    iRetVal = IsInTimeSpan(&tTimeNow, &tBeginTime, &tEndTime);
		    if (iRetVal == 0)
		    {
		        //printf("当前时间%d点%d分 在 %d点%d分和%d点%d分范围内\n", tTimeNow.hour, tTimeNow.minute, tBeginTime.hour, 
		        //    tBeginTime.minute, tEndTime.hour, tEndTime.minute);
		        printf("<><><><><><><>hahaha,CUURENT time in week recorder plan<><><><SHCEDULE_PLAN_RECODER==%d><><><\n",SHCEDULE_PLAN_RECODER);
		        if(SHCEDULE_PLAN_RECODER==0)
		        {
					SHCEDULE_PLAN_RECODER = 1;
		        }
		    }
		    else
		    {
		        //printf("当前时间%d点%d分 不在 %d点%d分和%d点%d分范围内\n", tTimeNow.hour, tTimeNow.minute, tBeginTime.hour, 
		        //    tBeginTime.minute, tEndTime.hour, tEndTime.minute);
		        //if(SHCEDULE_PLAN_RECODER==1)
		        printf("<><><><><><><>sorry,CUURENT time NOT in week recorder plan<><><><><SHCEDULE_PLAN_RECODER==%d><><\n",SHCEDULE_PLAN_RECODER);
		        {
					SHCEDULE_PLAN_RECODER = 0;
					record_status.record_flags = RECORD_NO;
		        }
		    }
			sleep(10);
			continue;
		}
	}
    return 0;                  // main函数执行成功返回0
}

void ShceduleRecordeByCurrentTime()
{
	pthread_attr_t attr;
	pthread_t shcedule_thread;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);  //分离的线程
	pthread_create(&shcedule_thread, &attr, shcedule_is_recorde_thread,NULL);
	pthread_attr_destroy(&attr);

}
void *dana_recordstatus_thread(void *args)
{
	static int status =-1;
	static int rec_flag = 0; 
	
	while(g_main_run)
	{	
		//如果当前为不录像时
		if(RECORD_NO == record_status.record_flags||SHCEDULE_PLAN_RECODER==0)
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
					//adapt_record_osd(0);
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
					//adapt_record_osd(0);
				}break;
				default:break;	
			}
		}
		else if((RECORD_FULLTIME == record_status.record_flags)&&(SHCEDULE_PLAN_RECODER==1) )
		{
			switch(record_status.record_last_status)
			{
				case  RECORD_NO:
				{
					printf("+++++++++++++++++++++++++++++RECORD_FULLTIME-----RECORD_NO++++++++++++++++++++++++\n");
					st_record_start(SDK_RECORD_TYPE_MANU,300); //150
					pthread_mutex_lock(&record_status.record_lock);
					record_status.record_last_status	= RECORD_FULLTIME;
					pthread_mutex_unlock(&record_status.record_lock);
					//adapt_record_osd(1);
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

					st_record_start(SDK_RECORD_TYPE_MANU,300);//150
					pthread_mutex_lock(&record_status.record_lock);
					record_status.record_last_status	= RECORD_FULLTIME;
					pthread_mutex_unlock(&record_status.record_lock);
					//adapt_record_osd(1);
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
							//adapt_record_osd(1);
							pthread_mutex_lock(&record_status.record_lock);
							record_status.record_alarm_triger	|= (1<<16);
							record_status.record_alarm_opened	= 1;
							pthread_mutex_unlock(&record_status.record_lock);
						}
					}
					else if(1 != record_status.record_alarm_triger)
					{
						if(0 == st_get_event_status())
						{
							//adapt_record_osd(0);
							pthread_mutex_lock(&record_status.record_lock);
							record_status.record_alarm_triger	= 0 ;
							pthread_mutex_unlock(&record_status.record_lock);
						}
					}
				}break;
				default:break;	
			}
		}

		usleep(1000*1000);
	}
	printf("record exit!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
}


//for record
unsigned int dana_record_init(void)
{
	record_status.pthread_is_processing	= 1; 
	pthread_mutex_init(&record_status.record_lock,NULL);
	int ret = pthread_create(&(record_status.record_pthread_handle), NULL, &dana_recordstatus_thread, NULL);
	if(ret < 0)
    {
        printf("pthread_create failed ret[%d]\n", ret);
		record_status.pthread_is_processing	= 0; 
        return -1;
    }
}

int dana_server_reg_avs_ops(dana_ls_avs_ops *ls)/* 注册流媒体接口 */
{
	//先获取保存的编码通道参数
	int i;
    sdk_encode_t enc_param ;
	memset(&enc_param,0,sizeof(sdk_encode_t));
	adapt_param_get_encode_cfg(0,&enc_param);

	printf("-------------------------tudana_server_reg_avs_ops--------------------------------\n");
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
	
	if(ls) g_dana_ls = *ls;

	if(1 == enc_param.av_enc_info[0].is_using)
	{
		g_dana_ls.encode_current_chn 	= 0; //编码0通道 ==720P
	}
	else if(1 == enc_param.av_enc_info[1].is_using)
	{
		g_dana_ls.encode_current_chn 	= 1; //编码1通道 ==VGA
	}
	else
	{
		g_dana_ls.encode_current_chn 	= 0; //编码0通道 ==720P
	}
	printf("g_dana_ls.encode_current_chn=%d\n",g_dana_ls.encode_current_chn);

	//dana_enc_default_set(45, g_dana_ls.encode_current_chn);

	return 0;

}


