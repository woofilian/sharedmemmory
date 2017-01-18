#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include "gmlib.h"
#include "sdk_struct.h"
#include "../video/video.h"
#include "log.h"
#include "shm_common.h"
#include "shm_write.h"
#include "shm_read.h"

//#include "AED.h"

/*
1 智源的音频 编码 支持 PCM  AAC  G711_ALAW  G711_ULAW  ADPCM  五种格式
2 解码也同时支持上述五种
3 鉴于我们目前CMS 就支持G711A的  我们目前的代码也改为G711a  即可
4 具体例子 参看samples/audio_XXXX.c
*/

//#define  AUDIO_TEST
#define BITSTREAM_LEN       (720 * 576 * 3 / 2)
#define PATTERN_NAME      "/mnt/nfs/audio_8khz_16bit"

extern av_info_t	g_av_info;
static int  g_audio_capture = 0;

enum ENUM_AUDIO_CODEC_TYPE
{
    ENUM_AUDIO_CODEC_TYPE_UNKNOWN,
    ENUM_AUDIO_CODEC_TYPE_PCM,
    ENUM_AUDIO_CODEC_TYPE_G711_URAW,
    ENUM_AUDIO_CODEC_TYPE_G726,
    ENUM_AUDIO_CODEC_TYPE_OPUS,
    ENUM_AUDIO_CODEC_TYPE_AAC,
    ENUM_AUDIO_CODEC_TYPE_BUTT
};


static int g_audio_num[MAX_AUDIO_STREAM_NUM] = {0};
extern SendstreamCallback	mAudioStreamCB ;  //音频回调

static int  audio_detect_level;
static int AED_InitFinish = 0;
static int AED_PwrSensitive = 6;
static int AED_SignalVariation = 3;
static int AED_Cnt = 0;
static int AED_Trigger = 5;
static int pwr_sensitive = 6;
static int signal_variation = 3;
static int aed_senisitive_diff=0;

//音频编码部分
static inline int write_audio_streaming(int ch, gm_enc_bitstream_t *bs)
{
	struct timeval tv;
    video_info_t  *pvideo_info = &g_av_info.audio_info[0];
    sdk_stream_info_t stream_info;
    memset(&stream_info,0,sizeof(sdk_stream_info_t));
    stream_info.ch = ch ;
    stream_info.sub_ch = 0;

    stream_info.frame_head.frame_type = SDK_AUDIO_FRAME_A;
    stream_info.frame_head.frame_size = bs->bs_len;//20160323320;
    stream_info.frame_head.frame_no = g_audio_num[ch]++;
    gettimeofday(&tv, NULL);
    stream_info.frame_head.sec = tv.tv_sec;				   //帧时间（秒）
    stream_info.frame_head.usec = tv.tv_usec;             //帧时间（微秒）
    stream_info.frame_head.pts =  bs->timestamp;		   //
    stream_info.frame_addr = (const char *)(bs->bs_buf) ;
    stream_info.frame_head.audio_info.encode_type =SDK_AUDIO_FORMAT_G711A; //SDK_AUDIO_FORMAT_ADPCM;////SDK_AUDIO_FORMAT_AAC;
    stream_info.frame_head.audio_info.samples = SDK_AUDIO_SAMPLE_R8K;

    stream_info.frame_head.audio_info.bits = SDK_AUDIO_SAMPLE_WIDTH_16;
    stream_info.frame_head.audio_info.channels = 1;

    if(mAudioStreamCB)
        mAudioStreamCB((void *)&stream_info);
    return 0;

}

int AE_record_ShmInit(char profileno, int iCodec, int iSamplePerSec, int iShmSize, int iMaxFrameSize)
{
    ShmAudioHeaderInfo structShmAudioHeaderInfo;
    memset(&structShmAudioHeaderInfo, -1, sizeof(structShmAudioHeaderInfo));
    structShmAudioHeaderInfo.ucMediaType = SHM_ENUM_MEDIA_TYPE_AUDIO; //audio flag
    structShmAudioHeaderInfo.ucCodec = iCodec;
    structShmAudioHeaderInfo.ucSamplesPerSec = iSamplePerSec;
    SHM_AudioInitWrite(profileno, iShmSize, iMaxFrameSize, &structShmAudioHeaderInfo);//iMaxFrameSize default for 4096
    return 0;
}

int AE_AudioEngineInput(int iStreamID, char* pBuf, int iLen, unsigned int uitv_ms)
{
	ShmAudioExtraData structShmAudioExtraData;
	
	memset(&structShmAudioExtraData, -1, sizeof(ShmAudioExtraData));

	unsigned long long ullcurtime;
	COMM_GetSystemUpMSecs(&ullcurtime);
	structShmAudioExtraData.ullTimeStamp = ullcurtime;

	SHM_AudioWriteFrame(iStreamID, pBuf, iLen, &structShmAudioExtraData);
}

int audio_enc_init(int total_ch,sdk_av_enc_t *pav_enc)
{
    int i = 0;
    sdk_av_enc_t *pav_info = NULL;
    DECLARE_ATTR(audio_grab_attr, gm_audio_grab_attr_t);
    DECLARE_ATTR(audio_encode_attr, gm_audio_enc_attr_t);

    g_av_info.audio_groupfd = gm_new_groupfd();
    
    pav_info = (sdk_av_enc_t *)((char *)pav_enc + i * sizeof(sdk_av_enc_t));

    g_av_info.audio_info[0].audio_buf_len = AUDIO_BITSTREAM_LEN;
    g_av_info.audio_info[0].audio_buf     = malloc(g_av_info.audio_info[i].audio_buf_len);
    memset(g_av_info.audio_info[0].audio_buf, 0, AUDIO_BITSTREAM_LEN);

    g_av_info.audio_info[0].grab_object = gm_new_obj(GM_AUDIO_GRAB_OBJECT);
    g_av_info.audio_info[0].audio_object= gm_new_obj(GM_AUDIO_ENCODER_OBJECT);

    audio_grab_attr.vch = 0;

    /* The supported sample_rate/sample_size/channel_type can be lookup
      from cat /proc/videograph/vpd/au_grab
      */
	audio_grab_attr.sample_rate =  8000;//44100;// 8000;
    audio_grab_attr.sample_size = 16;
    audio_grab_attr.channel_type = GM_MONO;

    gm_set_attr(g_av_info.audio_info[0].grab_object, &audio_grab_attr);

    audio_encode_attr.encode_type = GM_G711_ALAW;//GM_PCM ;//GM_AAC; // GM_PCM
    audio_encode_attr.bitrate = 32000; //SDK_AUDIO_SAMPLE_R32K
    audio_encode_attr.frame_samples = 320;//1024;
    gm_set_attr(g_av_info.audio_info[0].audio_object, &audio_encode_attr);

    g_av_info.audio_info[0].audio_fd = gm_bind(g_av_info.audio_groupfd, g_av_info.audio_info[0].grab_object, g_av_info.audio_info[0].audio_object);
    return 0;
}

int audio_enc_uninit()
{
    int ch = 0;

    gm_unbind(g_av_info.audio_groupfd);
    gm_apply(g_av_info.audio_groupfd);
    //for (ch = 0; ch < MAX_AUDIO_STREAM_NUM; ch++)
    {
        if(g_av_info.audio_info[0].audio_buf)
            free(g_av_info.audio_info[0].audio_buf);

        g_av_info.audio_info[0].audio_buf = NULL;
        g_av_info.audio_info[0].audio_buf_len = 0;

        gm_delete_obj(g_av_info.audio_info[0].grab_object);
        gm_delete_obj(g_av_info.audio_info[0].audio_object);
    }

    gm_delete_groupfd(g_av_info.audio_groupfd);
	

}

int  audio_capture_start()
{
    int ret = -1;
    ret = gm_apply(g_av_info.audio_groupfd); // active setting
    if(ret < 0 )
    {
        info("audio_capture_start failed!!! \n");
        return -1;
    }
    return 0;
}

static void *audio_capture_thread(void *arg)
{
    int i, ret;
    int ch = (int)arg;
	int audio_len, snd_len;

    FILE *bs_fd[MAX_AUDIO_STREAM_NUM];
    gm_pollfd_t poll_fds[MAX_AUDIO_STREAM_NUM];
    gm_enc_multi_bitstream_t multi_bs[MAX_AUDIO_STREAM_NUM];
#if 1//JFG_API	
	int streamAID = SHM_ENUM_AUDIO_16K_PROFILE;
	AE_record_ShmInit(streamAID, ENUM_AUDIO_CODEC_TYPE_G711_URAW, SHM_ENUM_AUDIO_16K_PER_SEC, 0, 0);
#endif
    memset(poll_fds, 0, sizeof(poll_fds));
    for (i = 0; i < MAX_AUDIO_STREAM_NUM; i++)
    {
        poll_fds[i].bindfd = g_av_info.audio_info[0].audio_fd;
        poll_fds[i].event = GM_POLL_READ;
    }

    g_audio_capture = 1;
    while (g_audio_capture)		
    {
        ret = gm_poll(poll_fds, MAX_AUDIO_STREAM_NUM, 1000);
        if (ret == GM_TIMEOUT)
        {
            printf("Poll timeout!!\n");
            continue;
        }

        memset(multi_bs, 0, sizeof(multi_bs));
        for (i = 0; i < MAX_AUDIO_STREAM_NUM; i++)
        {
            if (poll_fds[i].revent.event != GM_POLL_READ)
            {
                continue;
            }
            if (poll_fds[i].revent.bs_len > AUDIO_BITSTREAM_LEN)
            {
                printf("buffer length is not enough! %d, %d\n",
                       poll_fds[i].revent.bs_len, AUDIO_BITSTREAM_LEN);
                continue;
            }
            multi_bs[i].bindfd = g_av_info.audio_info[0].audio_fd;
            multi_bs[i].bs.bs_buf = g_av_info.audio_info[0].audio_buf ;
            multi_bs[i].bs.bs_buf_len = g_av_info.audio_info[0].audio_buf_len;//这里有区别
            multi_bs[i].bs.mv_buf = NULL;
            multi_bs[i].bs.mv_buf_len = 0;
        }

        if ((ret = gm_recv_multi_bitstreams(multi_bs, MAX_AUDIO_STREAM_NUM)) < 0)
        {
            error("Error return value %d\n", ret);
        }
        else
        {
            for (i = 0; i < MAX_AUDIO_STREAM_NUM; i++)
            {
                if (!multi_bs[i].bindfd)
                    continue;
                if (multi_bs[i].retval < 0)
                {
                    error("get bitstreame error! ret = %d\n", ret);
                }
                else if (multi_bs[i].retval == GM_SUCCESS)
                {
					AE_AudioEngineInput(streamAID, multi_bs[i].bs.bs_buf, multi_bs[i].bs.bs_len, multi_bs[i].bs.timestamp);
//#if JFG_API
#if 0
					for( audio_len = 0; audio_len < multi_bs[i].bs.bs_len; audio_len += snd_len){
						if( multi_bs[i].bs.bs_len - audio_len>= 160)
							snd_len = 160;
						else
							snd_len = multi_bs[i].bs.bs_len - audio_len;

						SHM_AudioWriteFrame(SHM_ENUM_AUDIO_16K_PROFILE , multi_bs[i].bs.bs_buf, snd_len, &structShmAudioExtraData);
				    }
#endif
                    write_audio_streaming(0, &multi_bs[i].bs);

				}
            }
        }
    }

	SHM_AudioRemoveID(streamAID);
    return 0;
}

static pthread_t audio_capture_id = 0;
int audio_server_start()
{
    pthread_attr_t attr;
    int ret ;
    audio_capture_start();
    /* Record Thread */
    if (audio_capture_id == (pthread_t)NULL)
    {
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);  //分离的线程
        ret = pthread_create(&audio_capture_id, &attr, &audio_capture_thread, NULL);
        pthread_attr_destroy(&attr);
    }
    else
    {
        info("video_capture_thread is already runing..... \n");
    }

    return 0;

}
int audio_server_stop()
{
    g_audio_capture = 0;
    return 0;
}

/******************************************/
//一下为解码部分
/***********************/
static int audio_write_fd = -1;
int init_write_fd()
{   
	char name_buf[64] = {0};
	int ch = 0;
	int sub_ch = 0;
	audio_write_fd = open("/mnt/nfs/recv.g711a",  O_WRONLY | O_CREAT, 0644);
	return 0;
}

int uninit_write_fd()
{
	close(audio_write_fd);
}

int main_write_audio_data(int ch,const char *video_data,int data_len)
{   
	
	int ret =  write(audio_write_fd,video_data,data_len);
	if(ret < 0)    
	{   
		error("write_video_data !!! ret = %d \n",ret); 
	}    
}

int audio_dec_init(int total_ch,sdk_av_enc_t *pav_enc)
{
    int i = 0;
    sdk_av_enc_t *pav_info = NULL;
    DECLARE_ATTR(file_attr, gm_file_attr_t);
    DECLARE_ATTR(audio_render_attr, gm_audio_render_attr_t);

    g_av_info.audio_groupfd2 = gm_new_groupfd();

    //  for (i = 0; i < MAX_AUDIO_STREAM_NUM; i++)
    {
        pav_info = (sdk_av_enc_t *)((char *)pav_enc + i * sizeof(sdk_av_enc_t));

        //g_av_info.audio_info[1].audio_buf_len = AUDIO_BITSTREAM_LEN;
        // g_av_info.audio_info[1].audio_buf     = malloc(g_av_info.audio_info[1].audio_buf_len);
        // memset(g_av_info.audio_info[i].audio_buf, 0, AUDIO_BITSTREAM_LEN);

        file_attr.sample_rate = 8000;
        file_attr.sample_size = 16;
        file_attr.channel_type = GM_MONO;//单声道

        g_av_info.audio_info[1].file_object = gm_new_obj(GM_FILE_OBJECT);
        g_av_info.audio_info[1].render_object= gm_new_obj(GM_AUDIO_RENDER_OBJECT);
        gm_set_attr(g_av_info.audio_info[1].file_object, &file_attr);

        // block_size: PCM(don't care) AAC(1024) ADPCM(256) G711(320)
        audio_render_attr.vch = 0;
        audio_render_attr.encode_type = GM_PCM;//GM_G711_ALAW;  //GM_PCMGM_PCM;//
        audio_render_attr.block_size = 320;
        gm_set_attr(g_av_info.audio_info[1].render_object, &audio_render_attr);
        g_av_info.audio_info[1].audio_fd = gm_bind(g_av_info.audio_groupfd2, g_av_info.audio_info[1].file_object, g_av_info.audio_info[1].render_object);
    }
    
	init_write_fd();
    return 0;
}

int audio_dec_uninit()
{
    int ch = 0;

    gm_unbind(g_av_info.audio_groupfd2);
    gm_apply(g_av_info.audio_groupfd2);
    //for (ch = 0; ch < MAX_AUDIO_STREAM_NUM; ch++)
    {
        if(g_av_info.audio_info[1].audio_buf)
            free(g_av_info.audio_info[1].audio_buf);

        g_av_info.audio_info[1].audio_buf = NULL;
        g_av_info.audio_info[1].audio_buf_len = 0;

        gm_delete_obj(g_av_info.audio_info[1].grab_object);
        gm_delete_obj(g_av_info.audio_info[1].audio_object);
    }

    gm_delete_groupfd(g_av_info.audio_groupfd2);

}
int  audio_dec_start()
{
    int ret = -1;
    ret = gm_apply(g_av_info.audio_groupfd2);
    if(ret < 0 )
    {
        info("video_playback_start failed!!! \n");
        return -1;
    }
    return 0;
}

inline int audio_playback(int audio_type,const char *audio_data,int date_len)
{
	int ret;

#if 1
   
	static char dec_buf[1024] = {0};
	//info("audio_playback =====audio_type:%d   date_len:%d ,audio_data:%d \n",audio_type,date_len,audio_data );
    int bfcount = G711ADec((unsigned char*)audio_data, (short *)dec_buf,date_len);
	
	/// main_write_audio_data(0,audio_data,date_len);
	
	 
    gm_dec_multi_bitstream_t multi_bs[1];


    multi_bs[0].bindfd = g_av_info.audio_info[1].audio_fd;
    multi_bs[0].bs_buf = dec_buf;
    multi_bs[0].bs_buf_len = bfcount;

    if ((ret = gm_send_multi_bitstreams(multi_bs, 1, 500)) < 0)
    {
        warning("<send bitstream fail(%d)!>\n", ret);
        return -1;
    }
#else
gm_dec_multi_bitstream_t multi_bs[1];


multi_bs[0].bindfd = g_av_info.audio_info[1].audio_fd;
multi_bs[0].bs_buf = audio_data;
multi_bs[0].bs_buf_len = date_len;

if ((ret = gm_send_multi_bitstreams(multi_bs, 1, 100)) < 0)
{
	warning("<send bitstream fail(%d)!>\n", ret);
	return -1;
}

#endif
    warning("audio_playback .....................> end  \n");
    return 0;
}

//还得做声音侦测的模块
//首先在初始化声音侦测长度和波特率
 int audio_detection_init()
{
	int raw_data_len;
	int sampling_rate;
#if 0
    if (GM_AED_Init(raw_data_len , sampling_rate , 0) != 0)
    {
        warning("Audio Event Detector Init Fail\n\r");
        return -1;
    }
	GM_AED_End();
    AED_InitFinish = 1;
#endif
	return 1;
}


 int set_audio_det_param(int level)
{
	AED_PwrSensitive=level;
	return 0;
}

 void *audio_detection_thread(void *arg)
{
   printf("audio_detection  has been  done \n ");
	char *raw_data_buf;//音频buf
	int AED_ret;
#if 0
	//g_av_info
    tAlarmObject* obj = &alarmObj;
    tAlarmConfig* pcfg = &obj->audioEventDetect[0];
    int AED_ret;
    int i = 0, j;
    static double cur_t=0,pre_t=0;

    if (AED_InitFinish)
    {
        if(!cur_t && !pre_t)
            cur_t = pre_t = GetTickCount();
        GM_AED_Main((short *)raw_data_buf, &AED_ret, AED_PwrSensitive, AED_SignalVariation);
        if(AED_ret == 1)
        {
            MSG("Audio Event Detected!\n\r");
            fflush(stdout);
            //AED_Cnt++;
            //if(AED_Cnt > 5)
            {
             	//AED_Cnt = 0;
                pre_t=GetTickCount();
                alarm_in_set(AU_AED, 0);
            }
        }
        else if(pcfg->state.get(&pcfg->state))
        {
            cur_t = GetTickCount();
            if ((cur_t - pre_t) > pcfg->OSDdisplayTime*1000)
            {
                alarm_in_clear(AU_AED, 0);
            }
        }
    }
#endif
}

int start_audio_detection()
{
#if 0
    int ret = 0;

    ret = pthread_create(&g_motion_handle.motion_thread_id, NULL, audio_detection, (void *) NULL);
    if (ret < 0)
    {
        error("create motion thread failed");
        goto end;
    }
    return 0;
end:
    if (g_motion_handle.motion_thread_id)
        pthread_join(g_motion_handle.motion_thread_id, NULL);
    return -1;
#endif
}

int stop_audio_detection()
{
#if 0
    g_motion_handle.motion_enable = 0;
    if (g_motion_handle.motion_thread_id)
        pthread_join(g_motion_handle.motion_thread_id, NULL);
    motion_detection_end();
    return 0;
	
#endif
}



int audio_detection_server_start()
{
	warning("AUDIO DET FUNCTION START>>>>>>>>>>>>>>>>>>\n");
	start_audio_detection();
	return 0;
}

#if 0
void aed_sensitive_update()
{
    if( aed_senisitive_diff != g_pstGrdObjects->grd_audio_event_detect.byAEDSensitive )
    {
    	aed_senisitive_diff = g_pstGrdObjects->grd_audio_event_detect.byAEDSensitive;
        switch(g_pstGrdObjects->grd_audio_event_detect.byAEDSensitive)
        {
            case 0:
            	pwr_sensitive = read_ini_value(ALARM_CFG_FILE, "AED_INFO", "pwr_sensitive_0", 1);
            	signal_variation = read_ini_value(ALARM_CFG_FILE, "AED_INFO", "signal_variation_0", 1);
            	break;
            case 1:
            	pwr_sensitive = read_ini_value(ALARM_CFG_FILE, "AED_INFO", "pwr_sensitive_1", 4);
            	signal_variation = read_ini_value(ALARM_CFG_FILE, "AED_INFO", "signal_variation_1", 3);
            	break;
            case 2:
            	pwr_sensitive = read_ini_value(ALARM_CFG_FILE, "AED_INFO", "pwr_sensitive_2", 6);
            	signal_variation = read_ini_value(ALARM_CFG_FILE, "AED_INFO", "signal_variation_2", 3);
            	break;
            case 3:
            	pwr_sensitive = read_ini_value(ALARM_CFG_FILE, "AED_INFO", "pwr_sensitive_3", 8);
            	signal_variation = read_ini_value(ALARM_CFG_FILE, "AED_INFO", "signal_variation_3", 2);
            	break;
            case 4:
            	pwr_sensitive = read_ini_value(ALARM_CFG_FILE, "AED_INFO", "pwr_sensitive_4", 9);
            	signal_variation = read_ini_value(ALARM_CFG_FILE, "AED_INFO", "signal_variation_4", 3);
            	break;
            case 5:
            	pwr_sensitive = read_ini_value(ALARM_CFG_FILE, "AED_INFO", "pwr_sensitive_5", 10);
            	signal_variation = read_ini_value(ALARM_CFG_FILE, "AED_INFO", "signal_variation_5", 2);
            	break;
            default:
            	pwr_sensitive = 6;
            	signal_variation = 3;
            	break;
        }

        AED_PwrSensitive = pwr_sensitive;
        AED_SignalVariation = signal_variation;
        //printf("aed parameter %d = %d,%d\n",
        //		g_pstGrdObjects->grd_audio_event_detect.byAEDSensitive,pwr_sensitive,signal_variation);
    }
}


void AudioEventDetectorEnd()
{
    GM_AED_End();

    AED_InitFinish = 0;
}

void AudioEventDetectorParamUpdate(int pwr_sensitive, int signal_variation)
{
    if (pwr_sensitive >= 0 &&
        pwr_sensitive <= 10 &&
        signal_variation >= 1 &&
        signal_variation <= 3)
    {
        AED_PwrSensitive = pwr_sensitive;
        AED_SignalVariation = signal_variation;
    }
}

#if 0
void AudioEventDetectorProcess(char *raw_data_buf, int raw_data_len)
{
    tAlarmObject* obj = &alarmObj;
    tAlarmConfig* pcfg = &obj->audioEventDetect[0];
    int AED_ret;
    int i = 0, j;
    static double cur_t=0,pre_t=0;

    if (AED_InitFinish)
    {
        if(!cur_t && !pre_t)
            cur_t = pre_t = GetTickCount();
        GM_AED_Main((short *)raw_data_buf, &AED_ret, AED_PwrSensitive, AED_SignalVariation);
        if(AED_ret == 1)
        {
            MSG("Audio Event Detected!\n\r");
            fflush(stdout);
            pre_t=GetTickCount();
            alarm_in_set(AU_AED, 0);
            for(j = 0; j < PlATFORM_HANDLE_TOTAL; j++)
            {
                if(pcfg->alarm_callback[j] != NULL)
                {
                    //printf("[%d]AED Alarm\n", j);
                    pcfg->alarm_callback[j](i, GRD_EVENT_TYPE_AUDIO_EVENT_OCCUR, 1, NULL);
                }
            }
        }
        else if(pcfg->state.get(&pcfg->state))
        {
            cur_t = GetTickCount();
            if ((cur_t - pre_t) > pcfg->OSDdisplayTime*1000)
            {
                alarm_in_clear(AU_AED, 0);
            }
        }
    }
}
#endif

void *AudioEventDetectorThread(char *raw_data_buf)
{
    tAlarmObject* obj = &alarmObj;
    tAlarmConfig* pcfg = &obj->audioEventDetect[0];
    int AED_ret;
    int i = 0, j;
    static double cur_t=0,pre_t=0;

    if (AED_InitFinish)
    {
        if(!cur_t && !pre_t)
            cur_t = pre_t = GetTickCount();
        GM_AED_Main((short *)raw_data_buf, &AED_ret, AED_PwrSensitive, AED_SignalVariation);
        if(AED_ret == 1)
        {
            MSG("Audio Event Detected!\n\r");
            fflush(stdout);
            //AED_Cnt++;
            //if(AED_Cnt > 5)
            {
             	//AED_Cnt = 0;
                pre_t=GetTickCount();
                alarm_in_set(AU_AED, 0);
            }
        }
        else if(pcfg->state.get(&pcfg->state))
        {
            cur_t = GetTickCount();
            if ((cur_t - pre_t) > pcfg->OSDdisplayTime*1000)
            {
                alarm_in_clear(AU_AED, 0);
            }
        }
    }
}
#endif

