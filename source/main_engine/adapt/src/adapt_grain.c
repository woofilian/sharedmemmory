/*
 * =====================================================================================
 *
 *       Filename:  adapt_grain.c
 *
 *    Description:  智源编解码适配层
 *
 *        Version:  1.0
 *        Created:  2014年06月01日 00时15分05秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  bingchuan
 *        Company:
 *
 * =====================================================================================
 */
#ifdef _GRAIN_
#include "wrapper.h"
#include "log.h"
#include "adapt.h"

#ifdef _NO_CODEC
#define sdk_codec_register(arg...)      (0)
#define sdk_codec_init(arg...)          (0)
#define sdk_codec_dispatch_msg(arg...)  (0)
#define sdk_codec_getChMdStat(arg...)   (0)
#define sdk_codec_req_snap(arg...)      (0)
#define sdk_codec_getChVloss(arg...)    (0)
#define sdk_codec_forceIFrame(arg...)   (0)
#define sdk_codec_setAudioAo(arg...)	(0)
#endif
#if 0
uint8_t resolution;     //SDK_VIDEO_RESOLUTION_E
uint8_t bitrate_type;   //码率类型  SDK_VIDEO_BR_TYPE_E
uint8_t pic_quilty;     //编码质量0-最好，1-次好，2-较好，3-一般，4-较差，5-差
uint8_t frame_rate;     //编码帧率1-25(PAL)/30
uint8_t gop;            //I 帧间隔1-200
uint8_t video_type;      //视频编码格式0--h264 1--MJPEG 2--JPEG  SDK_VIDEO_FORMAT_E
uint8_t res;      		//保留
uint8_t mix_type;       //音视频流: 0, 视频流: 1.
uint16_t bitrate;       //编码码率 32kbps-16000kbps
uint8_t level;          //编码等级，h264: 0--baseline, 1--main, 2--high;
uint8_t h264_ref_mod;   //h264编码帧参考模式
uint8_t audio_enc;      //音视编码格式0:G711A  SDK_AUDIO_CODEC_FORMAT_E
uint8_t audio_sample;      //音频采样  SDK_AUDIO_SAMPLE_RATE_E

#endif
//编码库的系统初始化
int adapt_av_init(EventCallback EventCB,
                  SendstreamCallback	VideoStreamCB,
                  SendstreamCallback	AudioStreamCB,
                  SendstreamCallback	PicStreamCB)
{
    sdk_sys_cfg_t sys_cfg;
    memset(&sys_cfg, 0, sizeof(sdk_sys_cfg_t));
    adapt_param_get_sys_cfg(&sys_cfg);

    //首先注册回调函数 实现音频视频流  事件的传输
    sdk_av_init_sys_callback( EventCB,
                              VideoStreamCB,
                              AudioStreamCB,
                              PicStreamCB);


    //初始化编码参数(音视频)
    Init_enc_cfg();
	
	//初始化移动侦测
	Init_motion_cfg();
	
	//初始化声音侦测
	//Init_AudioDet_cfg();

	//初始化OSD
	Init_osd_cfg();	
	
	//初始化GPIO 相关引脚
	Init_gpio_cfg();
	
	//初始化ISP
	Init_isp_cfg();
	
	//初始化镜像参数
	Init_mirror_cfg();

	Init_power_freq_cfg();
	
	warning("111111111111111111111111111111 \n");
    return 0;
}

#if 1
void Init_AudioDet_cfg()
{
	warning("Audio Dection Init>>>>>>>>>>>>>>>>>>\n");
	
}
#endif

void Init_mirror_cfg()
{
	warning("MIRROR Init>>>>>>>>>>>>>>>>>>\n");
	char buf [1024] = {0};	
	int ch =0;

	sdk_mirror_flip_cfg_t ongoing_Rotaion;
	memset(&ongoing_Rotaion, 0, sizeof(ongoing_Rotaion));
	//Add by wch @20170106
	//Modification for the current hardware
	ongoing_Rotaion.mirror = 1;
	ongoing_Rotaion.flip = 1;
	
	sdk_mirror_flip_cfg_t *desired_Rotation = (sdk_mirror_flip_cfg_t *)buf;
	adapt_param_get_mirror_param_param(desired_Rotation);
	printf("Get_mirror_param>> H_flip=%d | V_flip=%d\n", 
		desired_Rotation->mirror, desired_Rotation->flip);

	sdk_av_mirror_param(ch, &ongoing_Rotaion, desired_Rotation);
}

void Init_isp_cfg()
{
	warning("ISP Init>>>>>>>>>>>>>>>>>>\n");
	adapt_init_ISP();
}

void Init_gpio_cfg()
{
	warning("GPIO Init>>>>>>>>>>>>>>>>>>\n");
	adapt_init_gpio();//初始化gpio的功能
}

void Init_power_freq_cfg()
{
	char buf[1024] = {0};
	sdk_power_freq_cfg_t *power_freq = (sdk_power_freq_cfg_t *)buf;
	
	adapt_param_get_power_freq_param(power_freq);
	//sdk_power_freq_cfg_t power_freq;
	//memset(&power_freq, 0, sizeof(power_freq));
	//power_freq.powerfreq = 1;
	//adapt_param_set_power_freq_param(&power_freq);
	warning("default power frequency is %s", power_freq->powerfreq==1?"50Hz":"60Hz");
	sdk_sensor_powerhz_param(power_freq->powerfreq);
}

void Init_enc_cfg()
{
	warning("Enc funtion Init>>>>>>>>>>>>>>>>>>\n");
    int i;
	int ret = -1;
	int ch_cap = 0;
	//set the init value of enocde_info  
	char buf [1024] = {0};	
	sdk_encode_t *enc_param = (sdk_encode_t *)buf;

	int streamNo = 0;
	adapt_param_get_encode_cfg(ch_cap, enc_param);

	enc_param->av_enc_info[streamNo].audio_enc = SDK_AUDIO_FORMAT_G711A;//SDK_AUDIO_FORMAT_AAC;
	enc_param->av_enc_info[streamNo].audio_sample = SDK_AUDIO_SAMPLE_R8K;//SDK_AUDIO_SAMPLE_R44_1K;
	if(enc_param->av_enc_info[streamNo].frame_rate>=20)
	{
		 enc_param->av_enc_info[streamNo].frame_rate = 10;//20;//25;
	}
	enc_param->av_enc_info[streamNo].gop = 50;//30;//60;
	enc_param->av_enc_info[streamNo].h264_ref_mod = 0;
	enc_param->av_enc_info[streamNo].level = 2;
	if(enc_param->av_enc_info[streamNo].mix_type == 0)
	{
		enc_param->av_enc_info[streamNo].mix_type = 1;
	}
	enc_param->av_enc_info[streamNo].pic_quilty = 0;
	enc_param->av_enc_info[streamNo].video_type = 0;//h264

	if(enc_param->av_enc_info[streamNo].resolution == 0)
	{
		enc_param->av_enc_info[streamNo].resolution = SDK_VIDEO_RESOLUTION_VGA; //SDK_VIDEO_RESOLUTION_720p;
		//enc_param->av_enc_info[streamNo].resolution = SDK_VIDEO_RESOLUTION_960p; 
		enc_param->av_enc_info[streamNo].is_using = 1;
	}
	enc_param->av_enc_info[streamNo].resolution = SDK_VIDEO_RESOLUTION_720p;//SDK_VIDEO_RESOLUTION_960p; 
	printf("resolution	= %d\n",enc_param->av_enc_info[streamNo].resolution);
	printf("bitrate_type= %s\n",(enc_param->av_enc_info[streamNo].bitrate_type == 0)? "CBR":"VBR");
	printf("pic_quilty	= %d\n",enc_param->av_enc_info[streamNo].pic_quilty);
	printf("frame_rate	= %d\n",enc_param->av_enc_info[streamNo].frame_rate);
	printf("gop 		= %d\n",enc_param->av_enc_info[streamNo].gop);
	printf("video_type	= %s\n",(enc_param->av_enc_info[streamNo].video_type == 0)?"H264":"OTHER");
	printf("mix_type	= %s\n",(enc_param->av_enc_info[streamNo].mix_type == 1)?"VIDEO":"AUDIO");
	printf("bitrate 	= %d\n",enc_param->av_enc_info[streamNo].bitrate);
	printf("level		= %d\n",enc_param->av_enc_info[streamNo].level);
	printf("h264_ref_mod= %d\n",enc_param->av_enc_info[streamNo].h264_ref_mod);	
	printf("is_using	= %d\n",enc_param->av_enc_info[streamNo].is_using);
	
#if	0
	for(i = 0 ; i < 2 ; i++)
	{
		//adapt_param_get_encode_cfg(i, enc_param);
		
		enc_param->av_enc_info[i].audio_enc = SDK_AUDIO_FORMAT_G711A;//SDK_AUDIO_FORMAT_AAC;
		enc_param->av_enc_info[i].audio_sample = SDK_AUDIO_SAMPLE_R8K;//SDK_AUDIO_SAMPLE_R44_1K;

		if(enc_param->av_enc_info[i].frame_rate>=20)
		{
			 enc_param->av_enc_info[i].frame_rate = 10;//20;//25;
		}
		enc_param->av_enc_info[i].gop = 60;//30;//60;
		enc_param->av_enc_info[i].h264_ref_mod = 0;
		enc_param->av_enc_info[i].level = 2;
		if(enc_param->av_enc_info[i].mix_type == 0)
		{
			enc_param->av_enc_info[i].mix_type = 1;
		}
		enc_param->av_enc_info[i].pic_quilty = 0;
		enc_param->av_enc_info[i].video_type = 0;//h264

		if(i == 0)
		{
			//Mainstream: used for realtime liveview
			//Up to 960P(sc1145 is 720P), can be configured to 960P(720P)/VGA/QVGA
			enc_param->av_enc_info[i].resolution = SDK_VIDEO_RESOLUTION_720p;
			enc_param->av_enc_info[i].is_using = 1;
		}
		else
		{
			//Substream: used for recording (Local SD card or Cloud Storage)
			//fixed to VGA
			enc_param->av_enc_info[i].resolution = SDK_VIDEO_RESOLUTION_VGA;
			enc_param->av_enc_info[i].is_using = 0;
		}
		
		//if(i == 0)
		//	enc_param->av_enc_info[i].resolution = 15;
	printf("resolution 	= %d\n",enc_param->av_enc_info[i].resolution);
	printf("bitrate_type= %s\n",(enc_param->av_enc_info[i].bitrate_type == 0)? "CBR":"VBR");
	printf("pic_quilty 	= %d\n",enc_param->av_enc_info[i].pic_quilty);
	printf("frame_rate 	= %d\n",enc_param->av_enc_info[i].frame_rate);
	printf("gop 		= %d\n",enc_param->av_enc_info[i].gop);
	printf("video_type 	= %s\n",(enc_param->av_enc_info[i].video_type == 0)?"H264":"OTHER");
	printf("mix_type 	= %s\n",(enc_param->av_enc_info[i].mix_type == 1)?"VIDEO":"AUDIO");
	printf("bitrate 	= %d\n",enc_param->av_enc_info[i].bitrate);
	printf("level 		= %d\n",enc_param->av_enc_info[i].level);
	printf("h264_ref_mod= %d\n",enc_param->av_enc_info[i].h264_ref_mod);	
	printf("is_using	= %d\n",enc_param->av_enc_info[i].is_using);

	}
#endif 	
	adapt_param_set_encode_cfg(ch_cap, enc_param);
	
	ret = sdk_av_sys_init(ch_cap, streamNo+1, enc_param);
	//ret = sdk_av_sys_init(0, 2, enc_param);
	return;

__error:
	printf("SET sdk_encode_t +_+_+_+_+_+_+_+_+_+_+_+_+ ERROR \n");
	//end
	//warning("set first%d %d %d %d %d\n",encode_info.av_enc_info[0].pic_quilty, encode_info.av_enc_info[0].bitrate, encode_info.av_enc_info[0].h264_ref_mod, encode_info.av_enc_info[0].pic_quilty, encode_info.av_enc_info[0].frame_rate);

	//读取参数错误时 直接GOTO ERROR .上面的初始化也就跳过了 所以不用设置
	// ret = sdk_av_sys_init(0,2,enc_param);
}

void Init_osd_cfg()
{
	warning("OSD funtion Init>>>>>>>>>>>>>>>>>>\n");
	sdk_osd_cfg_t osd_param;
	adapt_param_get_osd_cfg(0,&osd_param);
	sdk_av_set_osd_param(0, &osd_param);
    warning("==================Init_osd_cfg=================== \n");
	warning("=============osd_param.chann_name.str is %s============ \n",osd_param.chann_name.str);
	warning("=============osd_param.chann_name.valid is %d========== \n",osd_param.chann_name.valid);
	warning("=============osd_param.time.valid is %d================ \n",osd_param.time.valid);
	warning("======================================================= \n");
}

void Init_motion_cfg()
{
	warning("MD funtion Init>>>>>>>>>>>>>>>>>>\n");
	video_init_motion();
	sdk_vda_codec_cfg_t mt_cfg;
	sdk_motion_cfg_t    motion_cfg;
	//获取移动侦测的参数	
    adapt_param_get_motion_cfg(0, &motion_cfg);
	memcpy(&mt_cfg, &motion_cfg.codec_vda_cfg, sizeof(sdk_vda_codec_cfg_t));
	
    warning("======================================================= \n");
	warning("[%d][%d][%d][%d]\n",mt_cfg.area[0].x,mt_cfg.area[0].y,mt_cfg.area[0].width,mt_cfg.area[0].height);
	warning("[%d][%d][%d][%d]\n",mt_cfg.area[1].x,mt_cfg.area[1].y,mt_cfg.area[1].width,mt_cfg.area[1].height);
	warning("[%d][%d][%d][%d]\n",mt_cfg.area[2].x,mt_cfg.area[2].y,mt_cfg.area[2].width,mt_cfg.area[2].height);
	warning("[%d][%d][%d][%d]\n",mt_cfg.area[3].x,mt_cfg.area[3].y,mt_cfg.area[3].width,mt_cfg.area[3].height);
	warning("======================================================= \n");
	sdk_av_set_motion_param(0, &mt_cfg);
}


//反初始化
int adapt_av_deinit(void)
{
    sdk_av_sys_uninit();
    return 0;
}

//启动服务
int adapt_av_server_start()
{
    return sdk_av_server_start();
}

int adapt_av_server_stop()
{
	sdk_av_server_stop();
	return 0;
}
/************功能函数*********************************/

//强制I帧
int adapt_av_force_i_frame(int sub_ch)
{
    // return sdk_codec_forceIFrame(ch, sub_ch);
    return  sdk_av_request_i_frame(sub_ch);
}



// 同一时刻只支持一种音频输出
int adapt_av_audio_control(sdk_msg_t *pmsg)
{
    static int _old_ao_mode = -1;
    static int _is_create_dec_ch = 0;
    int oper = pmsg->args;
    int _ao_dev = 0;
    int _ao_mode = 0;
    int _ao_preview_ch = 0;
    int ret = 0;

    pmsg->size = 0;
    return 0;

_error:
    pmsg->size = 0;
    return -1;
}

int sdk_read_osd_param()
{
	int ret = 0;
	char msg_buf[1024] = {0};
	sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;	
	sdk_osd_cfg_t *osd_param = (sdk_osd_cfg_t *)pMsg->data;
	
	adapt_param_get_osd_cfg(0, osd_param);
	pMsg->size = sizeof(sdk_osd_cfg_t);

	ret = sdk_av_set_osd_param(0, osd_param);
__error:
	printf("GET osd_param +_+_+_+_+_+_+_+_+_+_+_+_+ ERROR \n");
}

int adapt_av_capability_info(sdk_msg_t *pmsg)
{
    int _ch = pmsg->chann;
    if(_ch < 0 || _ch >= MAX_CHANN_NUM)
    {
        error("Invalid ch!\n");
        return -1;
    }

    return 0;
}


/***************设置参数****************************/
int adapt_av_encode_cfg(sdk_msg_t *pmsg)
{
	#if 0
    int ret = 0;
    int ch =  (pmsg->chann >> 16)|0xFFFF;
    int sub_ch = pmsg->chann|0xFFFF;

    printf("adapt_av_encode_cfg ch:%d sub_ch:%d \n",ch,sub_ch);

    sdk_av_enc_t *av_enc = (sdk_encode_t*)pmsg->data;
    //sdk_encode_t *p_enc_cfg = (sdk_encode_t*)pmsg->data;

    if(pmsg->args == SDK_PARAM_SET)
    {
        ret = sdk_av_set_encode_param(sub_ch,av_enc);
    }

    return ret;
	#endif
	
	int i,ret;
	sdk_encode_t *enc_cfg = (sdk_encode_t *)pmsg->data;
	for(i = 0; i < 2; i++)
	{
		ret = sdk_av_set_encode_param(i,&(enc_cfg->av_enc_info[i]));//写入参数
	}
	return ret;
	
	
}

int adapt_av_motion_cfg(sdk_msg_t *pmsg)
{
    int ret = 0;
    int ch =  (pmsg->chann >> 16)|0xFFFF;
    int sub_ch = pmsg->chann|0xFFFF;

    printf("adapt_av_motion_cfg ch:%d sub_ch:%d \n",ch,sub_ch);
    sdk_vda_codec_cfg_t motion_cfg;
    sdk_vda_codec_cfg_t *pmotion = (sdk_vda_codec_cfg_t*)pmsg->data;
    if(pmsg->args == SDK_PARAM_SET)
    {
        memset(&motion_cfg, 0, sizeof(sdk_vda_codec_cfg_t));
        memcpy(&motion_cfg, pmotion, sizeof(sdk_vda_codec_cfg_t));
        ret = sdk_av_set_motion_param(0,&motion_cfg);
    }
    return ret ;
}


//OSD 设置
#if 0
int adapt_av_osd_cfg(sdk_msg_t *pmsg)
{
    int ret = 0;
    int ch =  (pmsg->chann >> 16)|0xFFFF;
    int sub_ch = pmsg->chann|0xFFFF;

    printf("adapt_av_osd_cfg ch:%d sub_ch:%d \n",ch,sub_ch);
    sdk_osd_cfg_t osd_cfg;
    sdk_osd_cfg_t *posd = (sdk_vda_codec_cfg_t*)pmsg->data;
    if(pmsg->args == SDK_PARAM_SET)
    {
        memset(&osd_cfg, 0, sizeof(sdk_osd_cfg_t));
        memcpy(&osd_cfg, posd, sizeof(sdk_osd_cfg_t));
        ret = sdk_av_set_osd_param(sub_ch,&osd_cfg);
    }
    return ret ;
}
#endif


//视频遮挡 设置
int adapt_av_overlay_cfg(sdk_msg_t *pmsg)
{
    sdk_overlay_cfg_t *p_cover = (sdk_overlay_cfg_t *)pmsg->data;
    int ch = pmsg->chann;

    if(ch < 0 || ch >= MAX_CHANN_NUM)
    {
        error("ch error!\n");
        return -1;
    }
    printf("adapt_codec_overlay_cfg ch = %d \n",ch);
    return 0;   /////sdk_codec_set_ch_cover(ch, p_cover);
}

int adapt_av_snap(int ch,sdk_snap_info_t *snap_info,char *path)
{
    if(ch < 0 || ch >= MAX_CHANN_NUM || NULL == snap_info)
	    return -1;
    return sdk_av_snap(ch,snap_info,path);
}

inline int adapt_audio_play(int audio_type,const char *audio_data,int date_len)
{
	sdk_audio_play(audio_type,audio_data,date_len);
	return  0;
}

int adapt_av_md_status( uint32_t *status )
{
    return 0;
}

int adapt_audio_detect_cfg( sdk_msg_t *pmsg )
{
  	//设置同步声音侦测
    return 0;
}

int adapt_mirror_flip_cfg(sdk_msg_t *pmsg)
{
	int ret;
	int ch = 0;
	sdk_mirror_flip_cfg_t ongoing_rotation;
	adapt_param_get_mirror_param_param(&ongoing_rotation);
	
	sdk_mirror_flip_cfg_t *desired_rotation = (sdk_mirror_flip_cfg_t *)pmsg->data;
	printf("^^^^ mirror changed: %d --> %d\n", ongoing_rotation.mirror, desired_rotation->mirror);
	printf("^^^^ flip changed: %d --> %d\n", ongoing_rotation.flip, desired_rotation->flip);

	ret = sdk_av_mirror_param(ch, &ongoing_rotation, desired_rotation);//写入参数
	
    return ret;
}

int adapt_power_frequency_cfg(sdk_msg_t *pmsg)
{
	int powerHz;

	sdk_power_freq_cfg_t *powerfreq_cfg = (sdk_power_freq_cfg_t *)pmsg->data;
	powerHz = powerfreq_cfg->powerfreq;
	warning("set power frequency to %s\n", powerHz==1?"50HZ":"60HZ");
	
	sdk_sensor_powerhz_param(powerHz);
}

//全局变量刷新
int adapt_codec_encode_cfg(sdk_msg_t *pmsg)
{
	int i,ret;
	sdk_encode_t *enc_cfg = (sdk_encode_t *)pmsg->data;
	for(i=0;i<MAX_VIDEO_STREAM_NUM;i++)
	{
		ret = sdk_av_set_encode_param(i,&(enc_cfg->av_enc_info[i]));//写入参数
	}
    return ret;
}

int adapt_av_osd_cfg(sdk_msg_t *pmsg)
{
	int i,ret;
	sdk_osd_cfg_t *osd_param = (sdk_osd_cfg_t *)pmsg->data;

	for(i=0;i<2;i++)
	{
		ret = sdk_av_set_osd_param(i,osd_param);//写入参数
	}
    return ret;
}

int adapt_record_osd(int flag) //flag=1显示rec  FLAG=0 不显示REC
{
	return sdk_av_set_record_osd(flag);
}


/*****************************************************************************
 函 数 名  :adapt_set_time_zone
 功能描述  : 刷新osd显示时区
 输入参数  : time_zone 时区
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年10月9日
    作    者   : 张伟
    修改内容   : 新生成函数

*****************************************************************************/
int  adapt_set_time_zone(int time_zone)
{
	if( time_zone < 0 || time_zone > 24)
		return -1;
	//sdk_gpioEx_read(time_zone);
	sdk_set_time_zone(time_zone);
	//printf("WangRong time zone:[%d] --> son of a bitch!!!\n", time_zone);
	return 0;
}
	


int adapt_codec_set_image_attr(sdk_msg_t *pmsg)
{
	
	int ret;
	int i = 0;
	sdk_image_attr_t *attr_param = (sdk_image_attr_t *)pmsg->data;
	ret = sdk_av_set_attr_param(i, attr_param);
}

//add by jensen
int  adapt_init_gpio()
{

	int gpio_led_r = 30;
    int gpio_led_g = 29;
    int gpio_led_b = 28;
	int gpio_pwm   = 32+21; //GPIO1-21
	int gpio_pir   = 32+22;
	int gpio_Infrared = 32+3;
	int gpio_reset    = 32+4;
	int gpio_ir_cut26 = 32+26;
	int gpio_ir_cut27 = 32+27;
	int gpio_spk_pwm1 = 32+29;

	if(0 == sdk_init_gpio())
	{
		sdk_gpioEx_defpin(gpio_reset, 0);     //设置GPIO1_3为输入状态
		sdk_gpioEx_defpin(gpio_pir, 0);       //设置GPIO1_3为输入状态
		sdk_gpioEx_defpin(gpio_pwm, 1);       //设置pir PWM 为输出状态
		sdk_gpioEx_defpin(gpio_ir_cut26, 1);  //设置GPIO1_26为输出状态
		sdk_gpioEx_defpin(gpio_ir_cut27, 1);  //设置GPIO1_27为输出状态
		sdk_gpioEx_defpin(gpio_Infrared, 1);  //设置GPIO1_4为输出状态
        sdk_gpioEx_defpin(gpio_led_r, 1);     //设置GPIO0_30为输出状态
        sdk_gpioEx_defpin(gpio_led_g, 1);     //设置GPIO0_29为输出状态
        sdk_gpioEx_defpin(gpio_led_b, 1);     //设置GPIO0_28为输出状态
        sdk_gpioEx_defpin(gpio_spk_pwm1, 1);  //设置GPIO1_29为输出状态
	    printf("+_+_+_+_+_+_+_+_+_+_     adapt_init_gpio  has been   done  \n");
		sdk_gpioEx_set(32+29); //关闭功放

		sdk_gpioEx_clear(gpio_pwm);//初始化时默认为低电平
	     return 0;
	}
	
    printf("+_+_+_+_+_+_+_+_+_+_     adapt_init_gpio   is error \n");
	return  -1;
}

int adapt_init_ISP()
{
	return sdk_init_ISP();
}

int adapt_gpioEx_defpin(int gpiopin, int direction) //set dir about gpio pin
{
	return sdk_gpioEx_defpin(gpiopin, direction);
}

void adapt_gpioEx_set(int gpiopin) //set 1 
{
	sdk_gpioEx_set(gpiopin);
}

void adapt_gpioEx_clear(int gpiopin)//set 0
{
	sdk_gpioEx_clear(gpiopin);
}

int adapt_gpioEx_read(int gpiopin)//read gpio pin value
{
	return sdk_gpioEx_read(gpiopin);
}
//end

/****************************************/
/*目前962 产品的GPIO定义
GPIO1-3  //强制红外灯 开关
GPIO1-4  复位引脚
这里模仿上面的接口做一个例子
*/
// wch: now the GPIO1-19 is the Reset PIN
/*
	返回值 : 0  表示没有按键按下  1 表示有按键按下
*/
int adapt_read_reset_value()
{
	//int gpio_reset = 32+4;
    int gpio_reset = 32+19;
	int ret;
	ret = gpioEx_read(gpio_reset);
	if(0 ==ret )   
        info("read 51 pin value :%d  reset  done \n",ret);
	return (ret == 0 ? 0 :1);
}

/*
红外灯控制接口
infrared_status:  0 表示更具环境自动控制红外灯
				  1  表示强制关闭红外灯

该参数是系统一启动就必须设置的功能
*/
int  adapt_set_infrared_status(int infrared_status)
{
	sdk_set_infrared_status(infrared_status);

	return 0 ;
}

//spk控制
/* status:
 *     1 - enable speaker out
 *     0 - disable speaker out, mute
 * return:
 *     0 - success
 *    -1: internal error (GPIO configuration error)
 */
int  adapt_speaker_ctrl(int status)
{
	int ret ;
	int gpio_spk_pwm1 = 32+29;
   	//高低电平没确定 多以先这样试一下
   	printf("adapt_speaker_ctrl:status=%d\n",status);
    if (1 == status)
    {	
		sdk_gpioEx_clear(gpio_spk_pwm1);
		ret = sdk_gpioEx_read(gpio_spk_pwm1);
    	//printf("status=1 read GPIO%d-%d is  %d \n",gpio_spk_pwm1/32,gpio_spk_pwm1%32,ret);
    }
    else if(0 == status)
    {
    	sdk_gpioEx_set(gpio_spk_pwm1); 
	 	ret= sdk_gpioEx_read(gpio_spk_pwm1);
	  //  printf("status=0 read GPIO%d-%d is  %d \n",gpio_spk_pwm1/32,gpio_spk_pwm1%32,ret);
    }
    return 0;
}


int  adapt_get_ircut_dalay()
{
    return sdk_get_icut_delay();
}

int  adapt_detect_pir_alarm()
{
		int ret ;
		int  gpio_pir=32+22;
		ret = sdk_gpioEx_read(gpio_pir);	
		//if(0 ==ret )   info("read adapt_detect_pir_alarm value :%d \n",ret);
		return ret;
}
#if 0
//移动侦测 类似
int adapt_av_md_status(uint32_t *status)
{
    int i = 0;
    int ch_num = adapt_param_get_ana_chan_num();

    return 0;
}
//视屏遮挡
int adapt_av_od_status(uint32_t *status)
{
    int i = 0;
    int ch_num = adapt_param_get_ana_chan_num();
    return 0;
}

int adapt_av_lost_status(uint32_t *status)
{


    return 0;
}



int adapt_av_set_image_attr(sdk_msg_t *pmsg)
{
    int ch = pmsg->chann;
    if(ch < 0 || ch >= MAX_CHANN_NUM)
    {
        error("Invalid ch!\n");
        return -1;
    }

    ///sdk_image_attr_t *p_image = (sdk_image_attr_t *)pmsg->data;
    return 0;///sdk_codec_set_image_attr(ch, p_image);
}

int adapt_av_set_3a_attr(sdk_msg_t *pmsg)
{
    int ch = pmsg->chann;
    if(ch < 0 || ch >= MAX_CHANN_NUM)
    {
        error("Invalid ch!\n");
        return -1;
    }

    //sdk_3a_attr_t *p3a_attr = (sdk_3a_attr_t *)pmsg->data;
    return 0;//sdk_codec_set_3a_attr(ch, p3a_attr);
}
/*******************************/
int adapt_av_get_def_3a_attr(int ch ,sdk_3a_attr_t *p3a_attr)
{
    if(ch < 0 || ch >= MAX_CHANN_NUM || NULL == p3a_attr)
    {
        error("Invalid ch!\n");
        return -1;
    }

    ////sdk_3a_attr_t *p3a_attr = (sdk_3a_attr_t *)pmsg->data;
    return 0;// sdk_codec_get_def_3a_attr(ch, p3a_attr);
}
int adapt_av_get_def_image_attr(int ch ,sdk_image_attr_t *pimage_attr)
{
    if(ch < 0 || ch >= MAX_CHANN_NUM || NULL == pimage_attr )
    {
        error("Invalid ch!\n");
        return -1;
    }

    ////sdk_image_attr_t *pimage_attr = (sdk_image_attr_t *)pmsg->data;
    return 0;//sdk_codec_get_def_image_attr(ch, pimage_attr);
}
/*******************************/



}

#endif


#endif /*_GRAIN_*/

