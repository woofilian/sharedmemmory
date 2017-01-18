/******************************************************************************

                  版权所有 (C), 2012-2022, 

 ******************************************************************************
  文 件 名   : defparam.c
  版 本 号   : v1.0
  功能描述   : 系统默认参数的生成工具
  函数列表   :
              _config_encode_param
              _config_image_attr_param
              _config_motion_param
              _config_osd_param
              _config_root_user
              _config_serial_param
              _config_system_param
              main
  修改历史   :
    修改内容   : 创建文件

******************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "wrapper.h"
#include "log.h"
#include "paramlib.h"
#include "main.h"
#include "sdk_global.h"
#include "sdk_struct.h"

#include "adapt_param.c"

/* codeclib.h 中的结构体定义与 sdk_struct.h中的定义冲突 */
//#include "codeclib.h"
//#include "adapt.h"

#define YEAR ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10 \
+ (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))

#define MONTH (__DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 0 : 5) \
: __DATE__ [2] == 'b' ? 1 \
: __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 2 : 3) \
: __DATE__ [2] == 'y' ? 4 \
: __DATE__ [2] == 'l' ? 6 \
: __DATE__ [2] == 'g' ? 7 \
: __DATE__ [2] == 'p' ? 8 \
: __DATE__ [2] == 't' ? 9 \
: __DATE__ [2] == 'v' ? 10 : 11)

#define DAY ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 + (__DATE__ [5] - '0'))


static void _config_system_param()
{
	sdk_sys_cfg_t sys_cfg;
	memset(&sys_cfg, 0, sizeof(sys_cfg));
#ifdef _AMBA_S2L_
#warning >>>>>>>>>>>>>>>>>>> _AMBA_S2L_<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	//sys_cfg.dev_type        = SDK_DEV_TYPE_NVR_9_3521;
	sys_cfg.server_mask 	= 0x0F; // 每一位表示支持一种功能
    sys_cfg.io_mask      	= 0x0F;
	sys_cfg.max_stream		= 2;
	sys_cfg.max_resolution 	= 1;
    memcpy(sys_cfg.device_type, "IPC", strlen("IPC"));
	memcpy(sys_cfg.hardware_ver, "V0.0.1", strlen("V0.0.1"));
	memcpy(sys_cfg.serial_1, "0000000000001", strlen("0000000000001"));
#elif defined (_GRAIN_)
#warning >>>>>>>>>>>>>>>>>>> _GRAIN_<<<<<<<<<<<<<<<<<<<<<<<
	//sys_cfg.dev_type        = SDK_DEV_TYPE_NVR_9_3521;
	sys_cfg.server_mask 	= 0x0F; // 每一位表示支持一种功能
    sys_cfg.io_mask      	= 0x0F;
	sys_cfg.max_stream		= 2;
	sys_cfg.max_resolution 	= 1;
    memcpy(sys_cfg.device_type, "IPC", strlen("IPC"));
	memcpy(sys_cfg.hardware_ver, "V0.0.1", strlen("V0.0.1"));
	memcpy(sys_cfg.serial_1, "0000000000001", strlen("0000000000001"));
	memcpy(sys_cfg.manufacturer, "JOOAN", strlen("JOOAN"));
#else
	//sys_cfg.dev_type        = SDK_DEV_TYPE_NVR_9_3521;
	sys_cfg.server_mask 	= 0x0F; // 每一位表示支持一种功能
    sys_cfg.io_mask      	= 0x0F;
	sys_cfg.max_stream		= 2;
	sys_cfg.max_resolution 	= 1;
    memcpy(sys_cfg.device_type, "IPC", strlen("IPC"));
	memcpy(sys_cfg.hardware_ver, "V0.0.1", strlen("V0.0.1"));
	memcpy(sys_cfg.serial_1, "0000000000001", strlen("0000000000001"));

#endif
	adapt_param_set_sys_cfg(&sys_cfg);
}

#if 0
static void _config_platform_param()
{
    sdk_platform_t platform_cfg;
    memset(&platform_cfg, 0 ,sizeof(platform_cfg));
    strncpy((char *)platform_cfg.pu_id, "JXJ-NVR-11111111", sizeof(platform_cfg.pu_id));
    strncpy((char *)platform_cfg.cms_ip, "10.10.10.10", sizeof(platform_cfg.cms_ip));
    strncpy((char *)platform_cfg.mds_ip, "10.10.10.10", sizeof(platform_cfg.mds_ip));
    platform_cfg.cms_port = 9902;
    platform_cfg.mds_port = 9902;
    platform_cfg.protocol = 1;
    platform_cfg.plat_enable = 0;
	platform_cfg.slot.magic = 0x55aa55aa;
	platform_cfg.slot.index[0] = 0;
	platform_cfg.slot.index[1] = 1;
    adapt_param_set_platform_cfg(&platform_cfg);
}
#endif
static void _config_record_param(int ch_num)
{
	int weekday;
	int i;
	sdk_record_cfg_t record_cfg;	
	for (i=0; i<MAX_WEEK_NUM; i++) {
		weekday = i+1;
		record_cfg.record_sched[weekday-1].record_type = RECORD_FULLTIME;
		record_cfg.record_sched[weekday-1].sched_time[0].plan_flag = 1;//按周布放
		record_cfg.record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].start_hour=0;
		record_cfg.record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].start_min=0;
		record_cfg.record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].start_sec=0;
		record_cfg.record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].stop_hour=23;
		record_cfg.record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].stop_min=59;
		record_cfg.record_sched[weekday-1].sched_time[0].week_sched.week_sched_info[weekday-1][0].stop_sec=59;
	} 
	record_cfg.record_types = RECORD_FULLTIME;
	record_cfg.enable_audio = 1;
	record_cfg.enable = RECORD_FULLTIME;
	adapt_param_set_record_cfg(0, &record_cfg);
}
static void _config_encode_param(int ch_num)
{
	int ch;
	
    sdk_encode_t encode_cfg;
    memset(&encode_cfg, 0, sizeof(encode_cfg));
	//encode_cfg.av_enc_info[0]
    encode_cfg.av_enc_info[0].bitrate = 800;//1*1024;
    encode_cfg.av_enc_info[0].bitrate_type = SDK_BR_CBR;
    encode_cfg.av_enc_info[0].frame_rate = 15;
    encode_cfg.av_enc_info[0].gop = 50;
    encode_cfg.av_enc_info[0].resolution = SDK_VIDEO_RESOLUTION_720p;
    encode_cfg.av_enc_info[0].mix_type = 0;
	encode_cfg.av_enc_info[0].pic_quilty = 0;
	encode_cfg.av_enc_info[0].audio_enc = 0;
	encode_cfg.av_enc_info[0].video_type = 0;
	encode_cfg.av_enc_info[0].is_using = 0;
	
    encode_cfg.av_enc_info[1].bitrate = 256;//1*1024;
    encode_cfg.av_enc_info[1].bitrate_type = SDK_BR_CBR;
    encode_cfg.av_enc_info[1].frame_rate = 25;
    encode_cfg.av_enc_info[1].gop = 60;
    encode_cfg.av_enc_info[1].resolution = SDK_VIDEO_RESOLUTION_VGA;
    encode_cfg.av_enc_info[1].mix_type = 0;
	encode_cfg.av_enc_info[1].pic_quilty = 0;
	encode_cfg.av_enc_info[1].audio_enc = 0;
	encode_cfg.av_enc_info[1].video_type = 0;
	encode_cfg.av_enc_info[1].is_using = 1;
#if 0 //不配置chn2
    encode_cfg.av_enc_info[2].bitrate = 2048;//1*1024;
    encode_cfg.av_enc_info[2].bitrate_type = SDK_BR_CBR;
    encode_cfg.av_enc_info[2].frame_rate = 25;
    encode_cfg.av_enc_info[2].gop = 25;
    encode_cfg.av_enc_info[2].resolution = SDK_VIDEO_RESOLUTION_D1;
    encode_cfg.av_enc_info[2].mix_type = 0;
	encode_cfg.av_enc_info[2].pic_quilty = 0;
	encode_cfg.av_enc_info[2].audio_enc = 0;
	encode_cfg.av_enc_info[2].video_type = 0;
#endif	
   adapt_param_set_encode_cfg(0, &encode_cfg);
}
static void _config_network_param()
{
	sdk_eth_cfg_t eth_cfg;
	memset(&eth_cfg, 0, sizeof(eth_cfg));
	eth_cfg.ip_info.enable_dhcp = 1;
    adapt_param_set_eth_cfg(&eth_cfg);
}
static void _config_timezone_param()
{
	sdk_date_t time_cfg;
	memset(&time_cfg, 0, sizeof(time_cfg));
	time_cfg.tzone = 8; //默认为美国东部时区
    adapt_param_set_systime(&time_cfg);
}
static void _config_aws_param()
{
	sdk_aws_info_t aws_cfg;
	memset(&aws_cfg, 0, sizeof(aws_cfg));
	aws_cfg.aws_record_status = 255;
    adapt_param_set_aws_cfg(&aws_cfg);
}

static void _config_power_freq_param()
{
	sdk_power_freq_cfg_t power_freq_cfg;
	memset(&power_freq_cfg, 0, sizeof(power_freq_cfg));
	power_freq_cfg.powerfreq = 1; // 0: 60Hz, 1: 50Hz
	adapt_param_set_power_freq_param(&power_freq_cfg);
}

#if 0
static void _config_channel_param(int ch_num)
{
	int ch;
	sdk_channel_t channel_cfg;
	memset(&channel_cfg, 0, sizeof(channel_cfg));
	char channel_name[MAX_CHANN_NAME_LEN] = {0};
	
	for (ch=0; ch<ch_num; ch++)
	{	
		channel_cfg.enable = 1;
		channel_cfg.is_local = 1;
		channel_cfg.video_in = ch;
        channel_cfg.chann_sync_time = 1;
		
		if (ch == 0)
			sprintf((char*)channel_name, "CH%d", ch);
		else
			sprintf((char*)channel_name, "CH%02d", ch);
		strncpy((char*)channel_cfg.name, (char*)channel_name, MAX_CHANN_NAME_LEN);

		strncpy((char *)channel_cfg.user.user_name, "admin", MAX_USER_NAME_LEN);
		strncpy((char *)channel_cfg.user.user_pwd, "admin", MAX_PASSWD_LEN);
		channel_cfg.device.port = 3321;
		strncpy((char *)channel_cfg.device.ip_addr, "192.168.1.100", MAX_IP_ADDR_LEN);
		strncpy((char *)channel_cfg.device.user.user_name, "admin", MAX_USER_NAME_LEN);
		strncpy((char *)channel_cfg.device.user.user_pwd, "admin", MAX_PASSWD_LEN);
		
		adapt_param_set_chann_cfg(ch, &channel_cfg);
	}
}

static int _product_rand_number()
{
	int num = 0;
	struct timespec ts = {0, 0};
	uint64_t curr_times_ns = 0;
	
	while (1)
	{
		usleep(0);
		clock_gettime(CLOCK_MONOTONIC, &ts);
		curr_times_ns = ts.tv_sec*1000*1000*1000 + ts.tv_nsec;
		num = curr_times_ns%130;

		//		  0 -- 9				A   --  F			  a	  ---  f
		if ((num>=48&&num<=57) || (num>=65&&num<=70) || (num>=97&&num<=102))
			break;
	}
	
	//printf("$$$$$$$$$$   Rand num= %d\n", num);
	return num;
}

static void _product_mac(char mac[])
{  
    char *p = mac;
    p[0] = '0';
	p[1] = '0';
    int i = 0;
	
    for (i=3; i<17; i++)
	{
    	usleep(1);
        p[i] = _product_rand_number();
    }  
    p[2] = p[5] = p[8] = p[11] = p[14] = ':';
    p[17] = '\0';

    printf("%s\n", p);
}

static void _config_network_param()
{
	sdk_net_mng_cfg_t net_mng;
	sdk_eth_cfg_t eth_cfg;
	
	memset(&net_mng, 0, sizeof(net_mng));
	memset(&eth_cfg, 0, sizeof(eth_cfg));
	
	strncpy((char *)eth_cfg.ip_info.ip_addr, "192.168.1.123", MAX_IP_ADDR_LEN);
	strncpy((char *)eth_cfg.ip_info.gateway, "192.168.1.1",   MAX_IP_ADDR_LEN);
	strncpy((char *)eth_cfg.ip_info.mask,    "255.255.255.0", MAX_IP_ADDR_LEN);
    printf("--- eth[0].mac:%s\n", eth_cfg.ip_info.mac); // MAC地址由启动脚本随机生产;
	eth_cfg.ip_info.enable_dhcp = 0;
    strncpy((char *)eth_cfg.ip_info.dns1, "202.96.134.133", MAX_IP_ADDR_LEN);
    strncpy((char *)eth_cfg.ip_info.dns2, "192.168.1.1", MAX_IP_ADDR_LEN);
    
    adapt_param_set_eth_cfg(&eth_cfg, SDK_NET_IDX_ETH0);

	strncpy((char *)eth_cfg.ip_info.ip_addr, "192.168.2.123", MAX_IP_ADDR_LEN);
	strncpy((char *)eth_cfg.ip_info.gateway, "192.168.2.1",   MAX_IP_ADDR_LEN);
	strncpy((char *)eth_cfg.ip_info.mask,    "255.255.255.0", MAX_IP_ADDR_LEN);
	_product_mac((char *)eth_cfg.ip_info.mac);          // MAC地址随机生成;
	printf("--- eth[1].mac:%s\n", eth_cfg.ip_info.mac);
	eth_cfg.ip_info.enable_dhcp = 0;
	strncpy((char *)eth_cfg.ip_info.dns1, "202.96.134.133", MAX_IP_ADDR_LEN);
    strncpy((char *)eth_cfg.ip_info.dns2, "192.168.2.1", MAX_IP_ADDR_LEN);
    
    adapt_param_set_eth_cfg(&eth_cfg, SDK_NET_IDX_ETH1);
	
	/////////////////////////////////////////////////////////////////////////
	strncpy((char *)net_mng.multicast, "192.168.1.255", MAX_IP_ADDR_LEN); // 组播地址范围为"224.0.0.0～239.255.255.255"
	net_mng.def_if_no     = 0;
	net_mng.dvr_cmd_port  = 3321;
    net_mng.dvr_data_port = 7554;
	net_mng.http_port     = 80;
	net_mng.dvr_talk_port = 3322;
	
	strcpy(net_mng.cloud_addr, "www.ipdcs.com");
	net_mng.cloud_port = 8814;
    strcpy(net_mng.cloud_user, "admin");
    strcpy(net_mng.cloud_pass, "admin");
    net_mng.cloud_interval  = 15;   //sec

	adapt_param_set_net_mng_cfg(&net_mng);
}

static void _config_ptz_param(int ch_num)
{
	int ch;
	sdk_ptz_param_t ptz_cfg;
	memset(&ptz_cfg, 0, sizeof(ptz_cfg));

	ptz_cfg.protocol = SDK_PTZ_PELCO_D;
	
	for (ch=0; ch<ch_num; ch++)
	{
		ptz_cfg.address = 1;
		ptz_cfg.comm.baud_rate = 115200;
		ptz_cfg.comm.data_bit = 8;
		ptz_cfg.comm.stop_bit = 1;

		adapt_param_set_ptz_cfg(ch, &ptz_cfg);
	}
}

static void _config_preview_param()
{
	int _preview_mode = 0;
	int wind = 0;
	int _screen_num = 0;
	sdk_preview_t preview_cfg;
	memset(&preview_cfg, 0, sizeof(preview_cfg));

    preview_cfg.enable       = 0;
	preview_cfg.init_mode    = 0xFF;
	
	for (_screen_num=0; _screen_num<MAX_SCREEN_NUM; _screen_num++)
	{
	    if(_screen_num == 0)
	        preview_cfg.enable = 1;
	    else
	        preview_cfg.enable = 0;
	        
		for (_preview_mode=0; _preview_mode<MAX_PREVIEW_MODE; _preview_mode++)
		{
			for (wind=0; wind<MAX_WINDOWS_NUM; wind++)
			{
				preview_cfg.windows[_preview_mode][wind] = wind;
			}
		}
		
		adapt_param_set_preview_cfg(_screen_num, &preview_cfg);
	}
}

static void _config_tour_param()
{
	int _preview_mode = 0;
	int wind = 0;
	int _screen_num = 0;
	sdk_tour_t tour;
	memset(&tour, 0, sizeof(tour));
	tour.enable = 0;
	tour.enable_audio = 0;
	tour.switch_time  = 0;

	for (_screen_num=0; _screen_num<MAX_SCREEN_NUM; _screen_num++)
	{
		for (_preview_mode=0; _preview_mode<MAX_PREVIEW_MODE; _preview_mode++)
		{
			for (wind=0; wind<MAX_WINDOWS_NUM; wind++)
			{
				tour.windows[_preview_mode][wind] = 0;
			}
		}
		
		adapt_param_set_tour_cfg(_screen_num, &tour);
	}
}

#endif



static void _config_osd_param(int ch_num)
{
	int ch;
	sdk_osd_cfg_t osd_cfg;
	
	for (ch=0; ch<ch_num; ch++)
	{
        char tmpbuf[32];
        memset((char *)&osd_cfg, 0, sizeof(osd_cfg));
		memset(tmpbuf, 0, sizeof(tmpbuf));
        osd_cfg.time.valid = 1;
		osd_cfg.chann_name.valid = 1;
		osd_cfg.chann_name.pos.x = 32;
		osd_cfg.chann_name.pos.y = 32;
		osd_cfg.time.pos.x = 720/2;
		osd_cfg.time.pos.y = 720/2;
		//sprintf(tmpbuf, "CH%02d", ch+1);
		strcpy(tmpbuf,"Guardzilia");
        strncpy((char *)osd_cfg.chann_name.str, tmpbuf, strlen(tmpbuf));
        osd_cfg.chann_name.str_len = strlen((char *)osd_cfg.chann_name.str);
		adapt_param_set_osd_cfg(ch, &osd_cfg);
	}
}

static void _config_root_user()
{
	int i;
	sdk_user_right_t root_user;
	memset(&root_user, 0, sizeof(root_user));
	
	memcpy(root_user.user.user_name, "admin", strlen("admin"));
	memcpy(root_user.user.user_pwd, "admin", strlen("admin"));
    sdk_user_right_t user_array[MAX_USER_NUM];
	memset(user_array, 0, sizeof(user_array));
    memcpy(&user_array[0], &root_user, sizeof(sdk_user_right_t));

	for (i=0; i<31; i++)
	{
		user_array[0].local_right |= 1<<i;
		user_array[0].remote_right |= 1<<i;
		user_array[0].local_backup_right[i] = 1;
		user_array[0].net_preview_right[i] = 1;
	}
	
    adapt_param_set_user_array(user_array, sizeof(user_array));
}

#if 0
static void _config_comm_param()
{
	sdk_comm_cfg_t comm_cfg;
	memset(&comm_cfg, 0, sizeof(comm_cfg));

	strncpy((char *)comm_cfg.dvr_name, "Super_DVR", strlen("Super_DVR"));
	comm_cfg.dvr_id = 000;              // DVR默认ID
	comm_cfg.recycle_record = 1;		// 默认循环录像
	comm_cfg.boot_guide = 1;			// 默认启用开机向导
	comm_cfg.language = 0; 				// 默认中文
	comm_cfg.out_device = 1;			// 默认VGA
	comm_cfg.resolution = 0;			// 主屏默认分辨率1024 * 768
	comm_cfg.standby_time = 3;
	comm_cfg.video_mode = 0;			// 默认PAL制式
	comm_cfg.resolution2 = 0;			// 副屏默认分辨率1024 * 768
	
	adapt_param_set_comm_cfg(&comm_cfg);
}

static void _config_overlay_param(int ch_num)
{
	int ch;
    sdk_overlay_cfg_t overlaycfg;
	memset(&overlaycfg, 0, sizeof(overlaycfg));
	overlaycfg.enable = 0;
	overlaycfg.max_num = 4;
	overlaycfg.area[0].x = 0;
	overlaycfg.area[0].y = 0;
	overlaycfg.area[0].width = 80;
	overlaycfg.area[0].height = 80;

    for(ch=0; ch<ch_num; ch++)
    {
    	adapt_param_set_overlay_cfg(ch, &overlaycfg);
    }
}
#endif

static void _config_motion_param(int ch_num)
{
	int ch;
	sdk_motion_cfg_t motioncfg;

		memset(&motioncfg, 0, sizeof(motioncfg));
		motioncfg.codec_vda_cfg.enable = 0;
		motioncfg.alarm_handle.record_enable = 1;
		motioncfg.alarm_handle.record_time = 30;
		motioncfg.sched_time.interval_time = 15;
		
		motioncfg.alarm_handle.snap_enable = 0;
		motioncfg.alarm_handle.snap_num = 1;
		
		motioncfg.alarm_handle.beep_enable = 0;
		motioncfg.alarm_handle.beep_time = 5;
		
		motioncfg.alarm_handle.alarm_out_enable = 0;
		motioncfg.alarm_handle.alarm_out_time = 5;

		motioncfg.alarm_handle.email_enable = 0;
		/*motioncfg.alarm_handle.ptz_enable = 0;
		motioncfg.alarm_handle.ptz_type = 0;
		motioncfg.alarm_handle.ptz_param = 0;*/
		motioncfg.alarm_handle.led_enable = 1;
		motioncfg.alarm_handle.led_time = 0;

		//初始化MD区域
		motioncfg.codec_vda_cfg.area[0].x =  128;
		motioncfg.codec_vda_cfg.area[0].y =  72;
		motioncfg.codec_vda_cfg.area[0].width =  1280 - 128 - 128;
		motioncfg.codec_vda_cfg.area[0].height =  720 - 72 - 72;
		adapt_param_set_motion_cfg(0, &motioncfg);
}

static void _config_serial_param()
{
	int i;
	int serial_num = 4;
	sdk_serial_func_cfg_t serialcfg;
	memset(&serialcfg, 0, sizeof(serialcfg));

	serialcfg.type = 1;
	serialcfg.serial_param.baud_rate = 115200;
	serialcfg.serial_param.data_bit = 8;
	serialcfg.serial_param.flow_ctl = 0;
	serialcfg.serial_param.parity = 0;
	serialcfg.serial_param.stop_bit = 1;

	for (i=0; i<serial_num; i++)
	{
		adapt_param_set_serial_cfg(i, &serialcfg);
	}
}

static void _config_image_attr_param(int ch_num)
{
	int ch;
	sdk_image_attr_t image_attr;
	memset(&image_attr, 0, sizeof(image_attr));

	image_attr.brightness = 0x80;
	image_attr.contrast = 0x64;
	image_attr.hue = 0x80;
	image_attr.saturation = 0x80;
	image_attr.sharpness = 0x80;

	for (ch=0; ch<ch_num; ch++)
	{
		adapt_param_set_image_attr(ch, &image_attr);
	}
}

#if 0
static void _config_cruise_param(int ch_num)
{
	int ch;
	sdk_cruise_param_t cruise_cfg;
	memset(&cruise_cfg, 0, sizeof(cruise_cfg));

	for (ch=0; ch<ch_num; ch++)
	{
		adapt_param_set_cruise_cfg(ch, &cruise_cfg);
	}
}


static void _config_alarm_in_param(int ch_num)
{
	int ch;
	sdk_alarm_in_cfg_t alarm_in_cfg;

	for (ch=0; ch<ch_num; ch++)
	{
		memset(&alarm_in_cfg, 0, sizeof(alarm_in_cfg));
		alarm_in_cfg.type = 1;		// 默认常开
		alarm_in_cfg.enable = 0;
		alarm_in_cfg.alarm_handle.alarm_out_enable = 1;
		alarm_in_cfg.alarm_handle.beep_enable = 1;
		alarm_in_cfg.alarm_handle.alarm_out_time = 5;
		alarm_in_cfg.alarm_handle.beep_time = 5;
		alarm_in_cfg.alarm_handle.record_time = 60;
		alarm_in_cfg.alarm_handle.alarm_out_mask[0] = 1;
		alarm_in_cfg.alarm_handle.alarm_out_mask[1] = 1;
		alarm_in_cfg.alarm_handle.alarm_out_mask[2] = 1;
		alarm_in_cfg.alarm_handle.alarm_out_mask[3] = 1;
		alarm_in_cfg.alarm_handle.record_enable = 0;
		alarm_in_cfg.alarm_handle.record_mask[ch] = 1;
		alarm_in_cfg.alarm_handle.snap_mask[ch] = 1;
	
		adapt_param_set_alarm_in_cfg(ch, &alarm_in_cfg);
	}
}

static void _config_hide_param(int ch_num)
{
	int ch;
	sdk_hide_cfg_t hide_cfg;
	memset(&hide_cfg, 0, sizeof(hide_cfg));

	for (ch=0; ch<ch_num; ch++)
	{
		adapt_param_set_hide_cfg(ch, &hide_cfg);
	}
}

static void _config_lost_param(int ch_num)
{
	int ch;
	sdk_lost_cfg_t lost_cfg;
		
	for (ch=0; ch<ch_num; ch++)
	{
		memset(&lost_cfg, 0, sizeof(lost_cfg));
		lost_cfg.enable = 0;
		lost_cfg.alarm_handle.alarm_out_enable = 1;
		lost_cfg.alarm_handle.beep_enable = 1;
		lost_cfg.alarm_handle.alarm_out_time = 5;
		lost_cfg.alarm_handle.beep_time = 5;
		lost_cfg.alarm_handle.record_enable = 0;
		lost_cfg.alarm_handle.record_time = 60;
		lost_cfg.alarm_handle.alarm_out_mask[0] = 1;
		lost_cfg.alarm_handle.alarm_out_mask[1] = 1;
		lost_cfg.alarm_handle.alarm_out_mask[2] = 1;
		lost_cfg.alarm_handle.alarm_out_mask[3] = 1;	
		lost_cfg.alarm_handle.record_mask[ch] = 1;
		lost_cfg.alarm_handle.snap_mask[ch] = 1;
		
		adapt_param_set_lost_cfg(ch, &lost_cfg);
	}

}

static void _config_tv_param()
{
	sdk_vo_sideSize_t tv_cfg;
	memset(&tv_cfg, 0, sizeof(tv_cfg));

	adapt_param_set_tv_cfg(&tv_cfg);
}

static void _config_manual_rec_param()
{
	sdk_manual_record_t rec_cfg;
	memset(&rec_cfg, 0, sizeof(rec_cfg));
	adapt_param_set_manual_rec_cfg(&rec_cfg);
}

static void _config_manual_almin_param()
{
	int ch;
	sdk_manual_alarmin_t almin_cfg;
	memset(&almin_cfg, 0, sizeof(almin_cfg));

	for (ch=0; ch<MAX_ALARM_IN_NUM; ch++)
	{
		almin_cfg.enable_alarmin[ch] = 1;
	}

	adapt_param_set_manual_almin_cfg(&almin_cfg);
}

static void _config_manual_almout_param()
{
	sdk_manual_alarmout_t almout_cfg;
	memset(&almout_cfg, 0, sizeof(almout_cfg));

	adapt_param_set_manual_almout_cfg(&almout_cfg);
}


static void _config_manual_screen_param(int ch_num)
{
	int ch;
	sdk_screen_t screen_cfg;
		
	for (ch=0; ch<ch_num; ch++)
	{
		memset(&screen_cfg, 0, sizeof(screen_cfg));
		screen_cfg.valid = 0;
		screen_cfg.mode  = 0xff;
		screen_cfg.count = 0;
		int i = 0;
		for (i = 0; i < MAX_WINDOWS_NUM; i++)
		{
			memset(&screen_cfg.win[i], 0, sizeof(sdk_win_t));
			screen_cfg.win[i].win_no = i;
		}
		adapt_param_set_screen_cfg(ch, &screen_cfg);
	}
}

static void _config_ddns_parm()
{
	sdk_ddns_cfg_t ddns_cfg;
	memset(&ddns_cfg, 0, sizeof(ddns_cfg));
	adapt_param_set_ddns_cfg(&ddns_cfg);
}

static void _config_upnp_parm()
{
	sdk_upnp_t upnp_cfg;
	memset(&upnp_cfg, 0, sizeof(upnp_cfg));
	strncpy(upnp_cfg.server_ip, "192.168.1.1", sizeof(upnp_cfg.server_ip));
	upnp_cfg.cmd_port   = 3321;
	upnp_cfg.data_port  = 7554;
	upnp_cfg.web_port   = 80;
	adapt_param_set_upnp_cfg(&upnp_cfg);
}

static void _config_pppoe_parm()
{
	sdk_pppoe_t pppoe_cfg;
	memset(&pppoe_cfg, 0, sizeof(pppoe_cfg));
	adapt_param_set_pppoe_cfg(&pppoe_cfg, SDK_NET_IDX_PPPOE0); 
}
#endif

int main(int argc, char *argv[])
{
	sdk_log_init(LOG_DBG_STDERR|LOG_DBG_MEM, NULL, NULL);

    printf("------------------------------------- main demo!\n\n\n");
	int max_ch = 0;
	int i = 0;

    adapt_param_init_default_param();	// 创建默认配置文件(.sysparam_def)

	_config_system_param();				// 设置系统配置参数

	_config_root_user();				// 配置root用户
		
	//_config_comm_param();				// 常规配置

	//max_ch = adapt_param_get_ana_chan_num();
	max_ch = MAX_CHANN_NUM;

    _config_record_param(max_ch);		// 设置录像参数
	
	_config_encode_param(max_ch);		// 设置编码参数

	//_config_channel_param(max_ch);		// 设置通道参数

	_config_network_param();			// 设置网络参数
	
	//_config_ptz_param(max_ch);			// 设置云台参数

	_config_serial_param();				// 串口参数配置

	//_config_preview_param();			// 设置预览参数
	
	//_config_tour_param();               // 设置预览巡回参数
	
	_config_osd_param(max_ch);			// OSD配置

	_config_image_attr_param(max_ch);	// 图像属性配置

	//_config_cruise_param(max_ch);		// 巡航配置

	_config_motion_param(max_ch);		// 移动侦测配置
	_config_timezone_param(); 			//时区配置

	_config_power_freq_param();         //set default powerfreq
#if 0
	_config_aws_param();				//设置AWS
	//_config_alarm_in_param(max_ch);		// 外部报警输入配置
	
	_config_overlay_param(max_ch);		// 遮挡区域配置

	_config_hide_param(max_ch);			// 遮档侦测配置

	_config_lost_param(max_ch);			// 视频丢失配置

	_config_tv_param();					// TV参数配置

	_config_manual_rec_param();			// 手动录像配置

	_config_manual_almin_param();		// 手动报警输入配置

	_config_manual_almout_param();		// 手动报警输出配置

    _config_platform_param();           //平台信息

	_config_manual_screen_param(2);
	
	_config_ddns_parm();            //ddns
	
	_config_upnp_parm();            //upnp
	
	_config_pppoe_parm();           //pppoe
#endif

	printf("we will exit! 5 seconds...\n");
	for (i=5; i>0; i--)
	{
		printf("\t%d seconds......\n\n", i);
		sleep(1);
	}

    return 0;
}


