/******************************************************************************

                  版权所有 (C), 2012-2022, bingchuan

 ******************************************************************************
  文 件 名   : main_msg.c
  版 本 号   : v1.0
  作    者   : bingchuan
  生成日期   : 2015年5月15日
  功能描述   : 消息接口API
  函数列表   :
  修改历史   :
  1.日    期   : 2015年5月15日
    作    者   : bingchuan
    修改内容   : 创建文件

******************************************************************************/


#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "main_msg.h"
#include "sdk_struct.h"
#include "log.h"
#include "thread.h"
#include "adapt.h"

#if 1  // 提供没有实现接口时候 操作的 在实现之后要关闭 add by zw
#define upgrade_tid_runing(arg...)      (0)
#define adapt_record_get_usb_mount_path(arg...)          (0)
//已经实现可以暂时屏蔽

//#define adapt_init_net(arg...)      (0)
//#define adapt_search_ap(arg...)          (0)
//#define adapt_set_net_params(arg...)  (0)
//#define adapt_get_net_params(arg...)   (0)
//#define adapt_set_wifi_params(arg...)    (0)
#define adapt_get_wifi_params(arg...)    (0)
#define adapt_status_callback(arg...)   (0)
#define adapt_start_net_server(arg...)	(0)
#define adapt_stop_net_server(arg...)	(0)
#define adapt_get_wifi_mode(arg...)	(0)


#define adapt_baidu_init(arg...)      (0)
#define adapt_baidu_set_stream_ch(arg...)      (0)
#define adapt_baidu_set_rec_status(arg...)          (0)
#define adapt_baidu_set_audio_status(arg...)  (0)
#define adapt_baidu_set_video_status(arg...)   (0)
//可以使用暂时屏蔽
#define adapt_baidu_upload_file(arg...)      (0)
#define adapt_baidu_set_sched_params(arg...)      (0)

static int number;

static int rand_number_flag;

#endif



#define MAX_UPGRADE_FILE		8
#define MAX_FILE_NAME_LEN		64//32
#define MAX_FILE_PATH_LEN		64

#define NO_CHAN_ITEM			0		// 不用记录通道项
#define NO_ALMIN_ITEM			0		// 不用记录报警输入项

typedef struct _log_relative_info_s
{
    uint8_t ip_addr[MAX_IP_ADDR_LEN];
    uint8_t user_name[MAX_USER_NAME_LEN];
    uint8_t reserve[32];
} log_relative_info_t;


extern int dm_fd; 				// from main.c
extern int g_main_dev_type;		// from main.c
extern int gm_upgrade_is_file_mode;		// from main.c
extern unsigned int second_fb;  // from main.c

static int g_already_close;
static sdk_time_t g_rec_query_time;
static log_relative_info_t g_log_info;

static int _msg_log_backup(sdk_msg_t *pmsg);
#define WRITE_LOG(major_type, minor_type, ch, almin_no)\
    do{\
		uint32_t args = (ch) | (almin_no);\
        sdk_log_write(major_type, minor_type, args, "");\
    }while(0)

#define DEBUG_LOG(debug_buf)\
    do{\
        sdk_log_write(LOG_MAJOR_OPERATE, 0, 0, debug_buf);\
    }while(0)


static int _msg_upgrade_req(sdk_msg_t *pmsg);
static int _msg_upgrad_pro(sdk_msg_t *pmsg);
static int _msg_upgrad_find_file(sdk_msg_t *pmsg);
static int _trave_dir(const char* path, const char *p_find_str, int *pfile_no);
static int _msg_upgrad_trave_dir(const char *path, const char *p_find_str, sdk_msg_t *pmsg, int *pfile_no);
//static int _parse_file_head(const char *path, upgrad_pkg_file_t *p_pkg_file);
static int _msg_user_aaa(sdk_msg_t *pmsg);
static int _msg_user_del(sdk_msg_t *pmsg);
static int _msg_user_add(sdk_msg_t * pmsg);
static int _msg_user_modif(sdk_msg_t * pmsg);
static int _msg_user_query(sdk_msg_t * pmsg);
static int _msg_log_query(sdk_msg_t *pmsg);
static int _msg_log_empty(sdk_msg_t *pmsg);
static int _msg_log_backup(sdk_msg_t *pmsg);
static int _msg_log_delete(sdk_msg_t *pmsg);
static int _msg_recovery_param_effective_immediately(sdk_default_param_t *p_default, uint32_t ch_def);
static int _encode_param_validity_check(sdk_encode_t *p_enc_cfg);
static int _main_msg_make_someday(sdk_time_t *ptime, uint32_t *pbegin_time, uint32_t *pend_time);
static int _main_msg_adjust_pb_time(sdk_pb_group_t *p_group);
static time_t _main_msg_make_time(sdk_time_t *sdk_time);
static int _main_msg_local_time(time_t _utc, sdk_time_t *p_time);

int main_msg_startup_log()
{
    sdk_log_write(LOG_MAJOR_SYSTEM, L_SYSTEM_MINOR_STARTUP, 0, "");
    return 0;
}

int main_msg_def(sdk_msg_t *pmsg)
{
    pmsg->size = 0;
    return 0;
}

#ifndef _VR_VERSION
#error   "O_O   O_O   O_O   O_O _VR_VERSION NOT DEFINE O_O   O_O   O_O   O_O"
#else
#warning "^_^   ^_^   ^_^   ^_^ _VR_VERSION IS DEFINE  ^_^   ^_^   ^_^   ^_^"
#endif

int main_msg_version(sdk_msg_t *pmsg)
{
    sdk_version_t *version = (sdk_version_t *)pmsg->data;
    sdk_sys_cfg_t sys_cfg;
    memset(&sys_cfg, 0, sizeof(sys_cfg));
    adapt_param_get_sys_cfg(&sys_cfg);

#if 1
    memcpy(version->device_type, &sys_cfg.device_type, MAX_VER_STR_LEN);
    memcpy(version->hardware_ver, &sys_cfg.hardware_ver, MAX_VER_STR_LEN);
    memcpy(version->serial_no, &sys_cfg.serial_1, MAX_SERIAL_NO_LEN);
#else
    memcpy(version->device_type, "JXJ 3520 DVR/NVR", strlen("JXJ 3520 DVR/NVR"));
    memcpy(version->hardware_ver, "V0.0.1 Build20120629", strlen("V0.0.1 Build20120629"));
    memcpy(version->panel_ver, "V0.0.1 Build20120629", strlen("V0.0.1 Build20120629"));
    memcpy(version->serial_no, "0123456789", strlen("0123456789"));
#endif
    adapt_param_version_def((char *)version->software_ver,
                            sizeof(version->software_ver),
                            (char *)version->software_date,
                            sizeof(version->software_date));

    pmsg->size = sizeof(sdk_version_t);
    return 0;
}

int main_msg_status(sdk_msg_t *pmsg)
{
    int i = 0;

    sdk_status_t *status = (sdk_status_t *)pmsg->data;

    for(i = 0; i < MAX_CHANN_NUM; i++)
    {
        status->chan_status[i].encode_state = 1;
    }
    pmsg->size = sizeof(sdk_status_t);
    return 0;
}

// 恢复默认参数，使其它相关模块立即生效
static int _msg_recovery_param_effective_immediately(sdk_default_param_t *p_default, uint32_t ch_def)
{
    int i = 0;
    int ret = -1, ch = 0;
    uint8_t msg_buffer[8*1024]= {0};
    sdk_msg_t *pmsg = (sdk_msg_t*)msg_buffer;
    int max_ch = adapt_param_get_chnum();
    uint32_t param_mask = p_default->param_mask;
#if 0
    if (PARAM_MASK_ALL == param_mask)		// 恢复整机参数
    {
        warning("====> param_mask_all...\n");
        for (i=0; i<SDK_PARAM_MASK_BUIT; i++)
        {
            switch (i)
            {
            case SDK_PARAM_MASK_ENCODE:	// 编码参数
            {
                sdk_encode_t encode_cfg;

                for (ch=0; ch<max_ch; ch++)
                {
                    memset(&encode_cfg, 0, sizeof(encode_cfg));
                    adapt_param_get_def_encode_cfg(ch, &encode_cfg);

                    printf("--- ch=%d, frame_rate=%d, gop=%d, resolution=%d\n",
                           ch, encode_cfg.main.frame_rate, encode_cfg.main.gop, encode_cfg.main.resolution);

                    pmsg->chann = ch;
                    pmsg->args = SDK_PARAM_SET;
                    pmsg->size = sizeof(encode_cfg);
                    memcpy(pmsg->data, &encode_cfg, sizeof(encode_cfg));
                    if (adapt_param_dev_is_dvr(g_main_dev_type))
                    {
                        ret = adapt_codec_encode_cfg(pmsg);
                        if(ret < 0)
                        {
                            error("adapt_codec_encode_cfg error!\n");
                            pmsg->ack = SDK_ERR_SET_ENCODE_PARAM;
                            pmsg->size = 0;
                            return SDK_ERR_SET_ENCODE_PARAM;
                        }
                    }
                }
                break;
            }

            case SDK_PARAM_MASK_RECORD:	// 录像参数
            {
                sdk_record_cfg_t record_cfg;
                record_param_t rec_parm;
                memset(&rec_parm, 0, sizeof(rec_parm));
                sdk_record_getRecordParam(&rec_parm);
                for (ch=0; ch<max_ch; ch++)
                {
                    memset(&record_cfg, 0, sizeof(record_cfg));
                    adapt_param_get_def_record_cfg(0, &record_cfg);
                    rec_parm.en_prerecord = record_cfg.pre_record_time?1:0;
                    sdk_record_setRecordParam(&rec_parm);
                }
                break;
            }

            case SDK_PARAM_MASK_NET:	// 网络参数
            {
                // 目前把恢复整机参数里面的网络参数去掉
#if 0
                sdk_net_mng_cfg_t net_mng;
                memset(&net_mng, 0, sizeof(net_mng));
                adapt_param_get_def_net_mng_cfg(&net_mng);
                ret = _set_network_cfg(&net_mng);
                if (ret < 0)
                {
                    error("_set_network_cfg error!\n");
                    pmsg->ack = SDK_ERR_NET_CFG;
                    pmsg->size = 0;
                    return SDK_ERR_NET_CFG;
                }
#endif
                break;
            }

            case SDK_PARAM_MASK_IMAGE_ATTR:		// 图像属性
            {
                sdk_image_attr_t image_attr;
                for (ch=0; ch<max_ch; ch++)
                {
                    memset(&image_attr, 0, sizeof(image_attr));
                    adapt_param_get_def_image_cfg(ch, &image_attr);
                    pmsg->chann = ch;
                    pmsg->args = SDK_PARAM_SET;
                    pmsg->size = sizeof(image_attr);
                    memcpy(pmsg->data, &image_attr, sizeof(image_attr));
                    if (adapt_param_dev_is_dvr(g_main_dev_type))
                    {
                        ret = adapt_codec_set_image_attr(pmsg);
                        if(ret < 0)
                        {
                            error("adapt_codec_set_image_attr error!\n");
                            pmsg->size = 0;
                            pmsg->ack = SDK_ERR_SET_IMAGE_ATTR;
                            return SDK_ERR_SET_IMAGE_ATTR;
                        }
                    }
                }
                break;
            }

            case SDK_PARAM_MASK_OVERLAY:		// 遮挡区域参数
            {
                sdk_overlay_cfg_t overlaycfg;
                for (ch=0; ch<max_ch; ch++)
                {
                    memset(&overlaycfg, 0, sizeof(overlaycfg));
                    adapt_param_get_def_overlay_cfg(ch, &overlaycfg);

                    pmsg->chann = ch;
                    pmsg->size = sizeof(overlaycfg);
                    memcpy(pmsg->data, &overlaycfg, sizeof(overlaycfg));
                    if (adapt_param_dev_is_dvr(g_main_dev_type))
                    {
                        ret = adapt_codec_overlay_cfg(pmsg);
                        if(ret < 0)
                        {
                            error("adapt_codec_overlay_cfg error!\n");
                            pmsg->ack = SDK_ERR_SET_OVERLAY_PARAM;
                            pmsg->size = 0;
                            return SDK_ERR_SET_OVERLAY_PARAM;
                        }
                    }
                }
                break;
            }

            case SDK_PARAM_MASK_MOTION:		// 移动侦测
            {
                sdk_motion_cfg_v2_t motioncfg;
                for (ch=0; ch<max_ch; ch++)
                {
                    memset(&motioncfg, 0, sizeof(motioncfg));
                    adapt_param_get_def_motion_cfg(ch, &motioncfg);

                    pmsg->chann = ch;
                    pmsg->size = sizeof(motioncfg);
                    memcpy(pmsg->data, &motioncfg, sizeof(motioncfg));
                    if (adapt_param_dev_is_dvr(g_main_dev_type))
                    {
                        ret = adapt_codec_vda_cfg(pmsg);
                        if(ret < 0)
                        {
                            error("adapt_codec_vda_cfg error!\n");
                            pmsg->ack = SDK_ERR_SET_MD_PARAM;
                            pmsg->size = 0;
                            return SDK_ERR_SET_MD_PARAM;
                        }
                    }

                    //adapt_param_set_motion_cfg(ch, &motioncfg); move to adapt_param_recovery_param;
                }
                break;
            }

            case SDK_PARAM_MASK_OSD:		// OSD参数
            {
                sdk_osd_cfg_t osd_cfg;

                for (ch=0; ch<max_ch; ch++)
                {
                    memset(&osd_cfg, 0, sizeof(osd_cfg));
                    adapt_param_get_def_osd_cfg(ch, &osd_cfg);

                    pmsg->chann = ch;
                    pmsg->size = sizeof(osd_cfg);
                    memcpy(pmsg->data, &osd_cfg, sizeof(osd_cfg));
                    if (adapt_param_dev_is_dvr(g_main_dev_type))
                    {
                        ret = adapt_codec_osd_cfg(pmsg);
                        if(ret < 0)
                        {
                            error("adapt_codec_osd_cfg error!\n");
                            pmsg->ack = SDK_ERR_SET_OSD_PARAM;
                            pmsg->size = 0;
                            return SDK_ERR_SET_OSD_PARAM;
                        }
                    }
                    //adapt_param_set_osd_cfg(ch, &osd_cfg); move to adapt_param_recovery_param
                }
                break;
            }

            case SDK_PARAM_MASK_TV:		// 边距调节参数
            {
                sdk_vo_sideSize_t tv_cfg;
                memset(&tv_cfg, 0, sizeof(tv_cfg));
                adapt_param_get_def_tv_cfg(0, &tv_cfg);

                //pmsg->chann = ch;
                pmsg->args = SDK_PARAM_SET;
                pmsg->size = sizeof(tv_cfg);
                memcpy(pmsg->data, &tv_cfg, sizeof(tv_cfg));
                if (adapt_param_dev_is_dvr(g_main_dev_type))
                {
                    ret = adapt_codec_set_sidesize(pmsg);
                    if(ret < 0)
                    {
                        error("adapt_codec_set_sidesize error!\n");
                        pmsg->size = 0;
                        pmsg->ack = SDK_ERR_SET_TV_PARAM;
                        return SDK_ERR_SET_TV_PARAM;
                    }
                }
                //adapt_param_set_tv_cfg(&tv_cfg); move to adapt_param_recovery_param
                break;
            }

            default:
            {
                error("mask error!\n");
                pmsg->size = 0;
                return -1;
            }

            } //switch
        }// for
    }
    else	// 按模块恢复参数
    {
        for (i=0; i<SDK_PARAM_MASK_BUIT; i++)
        {
            if ((p_default->param_mask >> i) & 0x01)
            {
                info("---- i=%d, default->param_mask=%d\n", i, p_default->param_mask);
                switch (i)
                {
                case SDK_PARAM_MASK_ENCODE:	// 编码参数
                {
                    sdk_encode_t encode_cfg;
                    if (ch_def == CHANN_TYPE_ALL)
                    {
                        for (ch=0; ch<max_ch; ch++)
                        {
                            memset(&encode_cfg, 0, sizeof(encode_cfg));
                            adapt_param_get_def_encode_cfg(ch, &encode_cfg);

                            printf("--- ch=%d, frame_rate=%d, gop=%d, resolution=%d\n",
                                   ch, encode_cfg.main.frame_rate, encode_cfg.main.gop, encode_cfg.main.resolution);

                            pmsg->chann = ch;
                            pmsg->args = SDK_PARAM_SET;
                            pmsg->size = sizeof(encode_cfg);
                            memcpy(pmsg->data, &encode_cfg, sizeof(encode_cfg));
                            if (adapt_param_dev_is_dvr(g_main_dev_type))
                            {
                                ret = adapt_codec_encode_cfg(pmsg);
                                if(ret < 0)
                                {
                                    error("adapt_codec_encode_cfg error!\n");
                                    pmsg->ack = SDK_ERR_SET_ENCODE_PARAM;
                                    pmsg->size = 0;
                                    return SDK_ERR_SET_ENCODE_PARAM;
                                }
                            }
                        }
                    }
                    else
                    {
                        memset(&encode_cfg, 0, sizeof(encode_cfg));
                        adapt_param_get_def_encode_cfg(ch_def, &encode_cfg);

                        printf("--- ch=%d, frame_rate=%d, gop=%d, resolution=%d\n",
                               ch_def, encode_cfg.main.frame_rate, encode_cfg.main.gop, encode_cfg.main.resolution);

                        pmsg->chann = ch_def;
                        pmsg->args = SDK_PARAM_SET;
                        pmsg->size = sizeof(encode_cfg);
                        memcpy(pmsg->data, &encode_cfg, sizeof(encode_cfg));
                        if (adapt_param_dev_is_dvr(g_main_dev_type))
                        {
                            ret = adapt_codec_encode_cfg(pmsg);
                            if(ret < 0)
                            {
                                error("adapt_codec_encode_cfg error!\n");
                                pmsg->ack = SDK_ERR_SET_ENCODE_PARAM;
                                pmsg->size = 0;
                                return SDK_ERR_SET_ENCODE_PARAM;
                            }
                        }
                    }
                }
                break;

                case SDK_PARAM_MASK_RECORD:	// 录像参数
                {
                    sdk_record_cfg_t record_cfg;
                    record_param_t rec_parm;
                    memset(&rec_parm, 0, sizeof(rec_parm));
                    sdk_record_getRecordParam(&rec_parm);
                    for (ch=0; ch<max_ch; ch++)
                    {
                        memset(&record_cfg, 0, sizeof(record_cfg));
                        adapt_param_get_def_record_cfg(0, &record_cfg);
                        rec_parm.en_prerecord = record_cfg.pre_record_time?1:0;
                        sdk_record_setRecordParam(&rec_parm);
                    }
                    break;
                }

                case SDK_PARAM_MASK_NET:	// 网络参数
                {
                    int net_port_no;
                    sdk_eth_cfg_t eth_cfg;
                    sdk_sys_cfg_t sys_cfg;
                    memset(&sys_cfg, 0, sizeof(sys_cfg));
                    memset(&eth_cfg, 0, sizeof(eth_cfg));
                    adapt_param_get_sys_cfg(&sys_cfg);

                    //ret = _set_network_cfg(&net_mng);
                    for (net_port_no = 0; net_port_no < sys_cfg.net_port_num; net_port_no++)
                    {
                        adapt_param_get_def_eth_cfg(&eth_cfg, net_port_no);
                        ret = adapt_os_set_eth_cfg(&eth_cfg, net_port_no);
                        if (ret < 0)
                        {
                            error("adapt_os_set_eth_cfg error!\n");
                            pmsg->ack = SDK_ERR_NET_CFG;
                            pmsg->size = 0;
                            return SDK_ERR_NET_CFG;
                        }
                    }
                }
                break;

                case SDK_PARAM_MASK_IMAGE_ATTR:		// 图像属性
                {
                    sdk_image_attr_t image_attr;
                    if (ch_def == CHANN_TYPE_ALL)
                    {
                        for (ch=0; ch<max_ch; ch++)
                        {
                            memset(&image_attr, 0, sizeof(image_attr));
                            adapt_param_get_def_image_cfg(ch, &image_attr);

                            pmsg->chann = ch;
                            pmsg->args = SDK_PARAM_SET;
                            pmsg->size = sizeof(image_attr);
                            memcpy(pmsg->data, &image_attr, sizeof(image_attr));
                            if (adapt_param_dev_is_dvr(g_main_dev_type))
                            {
                                ret = adapt_codec_set_image_attr(pmsg);
                                if(ret < 0)
                                {
                                    error("adapt_codec_set_image_attr error!\n");
                                    pmsg->size = 0;
                                    pmsg->ack = SDK_ERR_SET_IMAGE_ATTR;
                                    return SDK_ERR_SET_IMAGE_ATTR;
                                }
                            }
                        }
                    }
                    else
                    {
                        memset(&image_attr, 0, sizeof(image_attr));
                        adapt_param_get_def_image_cfg(ch_def, &image_attr);
                        pmsg->chann = ch_def;
                        pmsg->args = SDK_PARAM_SET;
                        pmsg->size = sizeof(image_attr);
                        memcpy(pmsg->data, &image_attr, sizeof(image_attr));
                        if (adapt_param_dev_is_dvr(g_main_dev_type))
                        {
                            ret = adapt_codec_set_image_attr(pmsg);
                            if(ret < 0)
                            {
                                error("adapt_codec_set_image_attr error!\n");
                                pmsg->size = 0;
                                pmsg->ack = SDK_ERR_SET_IMAGE_ATTR;
                                return SDK_ERR_SET_IMAGE_ATTR;
                            }
                        }
                    }
                }
                break;

                case SDK_PARAM_MASK_OVERLAY:		// 遮挡区域参数
                {
                    sdk_overlay_cfg_t overlaycfg;
                    if (ch_def == CHANN_TYPE_ALL)
                    {
                        for (ch=0; ch<max_ch; ch++)
                        {
                            memset(&overlaycfg, 0, sizeof(overlaycfg));
                            adapt_param_get_def_overlay_cfg(ch, &overlaycfg);

                            pmsg->chann = ch;
                            pmsg->size = sizeof(overlaycfg);
                            memcpy(pmsg->data, &overlaycfg, sizeof(overlaycfg));
                            if (adapt_param_dev_is_dvr(g_main_dev_type))
                            {
                                ret = adapt_codec_overlay_cfg(pmsg);
                                if(ret < 0)
                                {
                                    error("adapt_codec_overlay_cfg error!\n");
                                    pmsg->ack = SDK_ERR_SET_OVERLAY_PARAM;
                                    pmsg->size = 0;
                                    return SDK_ERR_SET_OVERLAY_PARAM;
                                }
                            }
                        }
                    }
                    else
                    {
                        memset(&overlaycfg, 0, sizeof(overlaycfg));
                        adapt_param_get_def_overlay_cfg(ch_def, &overlaycfg);
                        pmsg->chann = ch_def;
                        pmsg->size = sizeof(overlaycfg);
                        memcpy(pmsg->data, &overlaycfg, sizeof(overlaycfg));
                        if (adapt_param_dev_is_dvr(g_main_dev_type))
                        {
                            ret = adapt_codec_overlay_cfg(pmsg);
                            if(ret < 0)
                            {
                                error("adapt_codec_overlay_cfg error!\n");
                                pmsg->ack = SDK_ERR_SET_OVERLAY_PARAM;
                                pmsg->size = 0;
                                return SDK_ERR_SET_OVERLAY_PARAM;
                            }
                        }
                    }
                }
                break;

                case SDK_PARAM_MASK_MOTION:		// 移动侦测
                {
                    sdk_motion_cfg_v2_t motioncfg;
                    if (ch_def == CHANN_TYPE_ALL)
                    {
                        for (ch=0; ch<max_ch; ch++)
                        {
                            memset(&motioncfg, 0, sizeof(motioncfg));
                            adapt_param_get_def_motion_cfg(ch, &motioncfg);

                            pmsg->chann = ch;
                            pmsg->size = sizeof(motioncfg);
                            memcpy(pmsg->data, &motioncfg, sizeof(motioncfg));
                            if (adapt_param_dev_is_dvr(g_main_dev_type))
                            {
                                ret = adapt_codec_vda_cfg(pmsg);
                                if(ret < 0)
                                {
                                    error("adapt_codec_vda_cfg error!\n");
                                    pmsg->ack = SDK_ERR_SET_MD_PARAM;
                                    pmsg->size = 0;
                                    return SDK_ERR_SET_MD_PARAM;
                                }
                            }
                        }
                    }
                    else
                    {
                        memset(&motioncfg, 0, sizeof(motioncfg));
                        adapt_param_get_def_motion_cfg(ch_def, &motioncfg);
                        pmsg->chann = ch_def;
                        pmsg->size = sizeof(motioncfg);
                        memcpy(pmsg->data, &motioncfg, sizeof(motioncfg));
                        if (adapt_param_dev_is_dvr(g_main_dev_type))
                        {
                            ret = adapt_codec_vda_cfg(pmsg);
                            if(ret < 0)
                            {
                                error("adapt_codec_vda_cfg error!\n");
                                pmsg->ack = SDK_ERR_SET_MD_PARAM;
                                pmsg->size = 0;
                                return SDK_ERR_SET_MD_PARAM;
                            }
                        }
                    }
                }
                break;

                case SDK_PARAM_MASK_OSD:		// OSD参数
                {
                    sdk_osd_cfg_t osd_cfg;
                    if (ch_def == CHANN_TYPE_ALL)
                    {
                        for (ch=0; ch<max_ch; ch++)
                        {
                            memset(&osd_cfg, 0, sizeof(osd_cfg));
                            adapt_param_get_def_osd_cfg(ch, &osd_cfg);
                            pmsg->chann = ch;
                            pmsg->size = sizeof(osd_cfg);
                            memcpy(pmsg->data, &osd_cfg, sizeof(osd_cfg));
                            if (adapt_param_dev_is_dvr(g_main_dev_type))
                            {
                                ret = adapt_codec_osd_cfg(pmsg);
                                if(ret < 0)
                                {
                                    error("adapt_codec_osd_cfg error!\n");
                                    pmsg->ack = SDK_ERR_SET_OSD_PARAM;
                                    pmsg->size = 0;
                                    return SDK_ERR_SET_OSD_PARAM;
                                }
                            }
                        }
                    }
                    else
                    {
                        memset(&osd_cfg, 0, sizeof(osd_cfg));
                        adapt_param_get_def_osd_cfg(ch_def, &osd_cfg);
                        pmsg->chann = ch_def;
                        pmsg->size = sizeof(osd_cfg);
                        memcpy(pmsg->data, &osd_cfg, sizeof(osd_cfg));
                        if (adapt_param_dev_is_dvr(g_main_dev_type))
                        {
                            ret = adapt_codec_osd_cfg(pmsg);
                            if(ret < 0)
                            {
                                error("adapt_codec_osd_cfg error!\n");
                                pmsg->ack = SDK_ERR_SET_OSD_PARAM;
                                pmsg->size = 0;
                                return SDK_ERR_SET_OSD_PARAM;
                            }
                        }
                    }
                }
                break;

                case SDK_PARAM_MASK_TV:		// 边距调节参数
                {
                    sdk_vo_sideSize_t tv_cfg;
                    memset(&tv_cfg, 0, sizeof(tv_cfg));
                    adapt_param_get_def_tv_cfg(0, &tv_cfg);
                    pmsg->args = SDK_PARAM_SET;
                    pmsg->size = sizeof(tv_cfg);
                    memcpy(pmsg->data, &tv_cfg, sizeof(tv_cfg));
                    if (adapt_param_dev_is_dvr(g_main_dev_type))
                    {
                        ret = adapt_codec_set_sidesize(pmsg);
                        if(ret < 0)
                        {
                            error("adapt_codec_set_sidesize error!\n");
                            pmsg->size = 0;
                            pmsg->ack = SDK_ERR_SET_TV_PARAM;
                            return SDK_ERR_SET_TV_PARAM;
                        }
                    }
                }
                break;

                default:
                {
                    error("mask error!\n");
                    pmsg->size = 0;
                    return -1;
                }
                }//switch
            }// if
        }// for
    }// if

#endif

    return 0;
}

int main_msg_param_def(sdk_msg_t *pmsg)
{
    int ret = 0;
    sdk_default_param_t param;
	
    sdk_default_param_t	*p_def = (sdk_default_param_t *)pmsg->data;
    uint32_t ch_def = pmsg->chann;
    memset(&param, 0, sizeof(param));
    memcpy(&param, p_def, sizeof(sdk_default_param_t));
    adapt_param_recovery_param(&param, ch_def);
    ret = _msg_recovery_param_effective_immediately(&param, ch_def);
    if (ret != 0)	// 失败
    {
        error("_msg_recovery_param_effective_immediately error! ret:%d\n", ret);
        pmsg->size = 0;
        pmsg->ack = ret;
        return -1;
    }

    pmsg->size = 0;
    WRITE_LOG(LOG_MAJOR_OPERATE, L_OPERATE_MINOR_DEFAULT, NO_CHAN_ITEM, NO_ALMIN_ITEM);
    return 0;
}

//升级
int main_msg_upgrad(sdk_msg_t *pmsg)
{
//	info("main_msg_upgrad+_+_+_+_+_+_+_+_+_+_+_+_ \n");
    int is_file;
    int ret = -1;
    int upgrad_op = pmsg->args;

    switch (upgrad_op)
    {
    case SDK_UPGRAD_NET_REQUEST: //网络升级请求
	{
//		info("main_msg_upgrad+_+_+_+_+_+_+_+_+_+_+_+_ SDK_UPGRAD_NET_REQUEST\n");
		 ret = _msg_net_upgrade_req(pmsg);
		break;
	}
	case SDK_UPGRAD_NET_RECV_UPG://网络升级发送设计包
	{
//		info("main_msg_upgrad+_+_+_+_+_+_+_+_+_+_+_+_ SDK_UPGRAD_NET_RECV_UPG\n");
		ret = _msg_net_upgrade_recv(pmsg);
		break;
	}

    case SDK_UPGRAD_REQUEST:
    {
//		info("main_msg_upgrad+_+_+_+_+_+_+_+_+_+_+_+_ SDK_UPGRAD_REQUEST\n");
        ret = _msg_upgrade_req(pmsg);//查询正在升级
        break;
    }

    case SDK_UPGRAD_DISK_FILE:
    {
        is_file = 1;
        upgrad_pkg_file_t *pfile = (upgrad_pkg_file_t *)pmsg->data;
        info("SDK_UPGRAD_DISK_FILE----- file_path=%s\n", pfile->file_path);

        ret = upgrade_start((char *)pfile->file_path, is_file);
        if (ret < 0)
        {
            error("upgrade_start error!\n");
            pmsg->ack = SDK_ERR_UPGRADE;
            pmsg->size = 0;
            return -1;
        }

        WRITE_LOG(LOG_MAJOR_OPERATE, L_OPERATE_MINOR_UPGRADE, NO_CHAN_ITEM, NO_ALMIN_ITEM);
        sdk_log_flush();
        break;
    }

    // 存储在内存区，基本不用。
#if 1
    case SDK_UPGRAD_MEM_FILE:
    {
        is_file = 0;
        ret = upgrade_start((char *)pmsg->data, is_file); // ???
        if (ret < 0)
        {
            error("upgrade_start error!\n");
            pmsg->ack = SDK_ERR_UPGRADE;
            pmsg->size = 0;
            return -1;
        }
        break;
    }
#endif

    case SDK_UPGRAD_PROGRESS:
    {
        ret = _msg_upgrad_pro(pmsg);
        warning("SDK_UPGRAD_PROGRESS--------- extend=%u\n", pmsg->extend);
        break;
    }

    case SDK_UPGRAD_FIND_FILE:
    {
        ret = _msg_upgrad_find_file(pmsg);
        if (ret < 0)
        {
            error("_msg_upgrad_find_file error!\n");
            pmsg->ack = SDK_ERR_UPGRADE_FILE_FIND;
            pmsg->size = 0;
            return -1;
        }
        break;
    }

    default:
        error("No operation!\n");
    }
    return ret;
}

// 是否正在升级 ret<0: 正在升级; ret=0:非正在升级
static int _msg_upgrade_req(sdk_msg_t *pmsg)
{
    printf("_msg_upgrade_req\n");
    if (upgrade_tid_runing())
    {
        error("upgrade running ......\n");
        pmsg->ack = SDK_ERR_UPGRADING;
        pmsg->size = 0;
        return -1;
    }

    // 重置升级进度为0
    ///upgrade_set_percent(0);
    return 0;
}

static char *_msg_string_join(char *source_buf, char *add_string, int len_of_source_buf)
{
    if ((strlen(source_buf) + strlen(add_string)) >= len_of_source_buf)
    {
        error("buff full! len_of_source_buf:%d\n", len_of_source_buf);
        return -1;
    }

    strcat(source_buf, add_string);
    return source_buf;
}

static char *_msg_add_slash(char *string, int len)
{
    if (NULL == string)
    {
        error("error! NULL == string\n");
        return NULL;
    }

    if (string[strlen(string) - 1] != '\/')
        return _msg_string_join(string, "/", len);
    else
        return string;
}

static int _msg_upgrad_find_file(sdk_msg_t *pmsg)
{
    int file_no = 0;
    int ret = 0;
    pmsg->size = 0;
    char mount_path[32] = {0};

    if (adapt_record_get_usb_mount_path(mount_path, sizeof(mount_path)) < 0)
    {
        error("get usb path error!\n");
        return -1;
    }

    if (NULL == _msg_add_slash(mount_path, sizeof(mount_path)))
    {
        error("_msg_add_slash error!  mount_path:%s\n", mount_path);
        return -1;
    }

    ret = _msg_upgrad_trave_dir(mount_path, ".upg", pmsg, &file_no);
    if (ret < 0)
    {
        error("_msg_trave_dir error!\n");
        return -1;
    }

    return 0;
}

char upgrade_filename[MAX_UPGRADE_FILE][MAX_FILE_NAME_LEN];
static int _msg_upgrad_trave_dir(const char *path, const char *p_find_str, sdk_msg_t *pmsg, int *pfile_no)
{
#if 0
    uint8_t temp_path[64];
    upgrad_pkg_file_t pkg_file;
    int ret = -1;
    int i;

    if (NULL==path || NULL==p_find_str || NULL==pmsg || NULL==pfile_no)
    {
        error("_msg_trave_dir error! NULL poiter\n");
        return -1;
    }
    printf("---- path=%s, p_find_str=%s, fileno=%d\n", path, p_find_str, *pfile_no);

    ret = _trave_dir(path, p_find_str, pfile_no);
    if (ret < 0)
    {
        error("_trave_dir error!\n");
        return -1;
    }

    if (*pfile_no > 0)
    {
        for (i=0; i<*pfile_no; i++)
        {
            memset(&pkg_file, 0, sizeof(pkg_file));
            strncpy((char *)pkg_file.name, upgrade_filename[i], MAX_FILE_NAME_LEN);
            memset(temp_path, 0, sizeof(temp_path));
            strncpy((char *)temp_path, path, strlen(path));
            strcat((char *)temp_path, upgrade_filename[i]);
            strncpy((char *)pkg_file.file_path, (char *)temp_path, MAX_FILE_PATH_LEN);
            printf("----- i=%d, file_name:%s, file_path:%s\n", i, pkg_file.name, pkg_file.file_path);

            ret = _parse_file_head((const char *)pkg_file.file_path, &pkg_file);
            if (ret < 0)
            {
                error("_parse_file_head error!\n");
                return -1;
            }

            memcpy(pmsg->data + pmsg->size, &pkg_file, sizeof(upgrad_pkg_file_t));
            pmsg->size += sizeof(upgrad_pkg_file_t);
        }
    }
    else
    {
        warning("No upgrade file!\n");
        return -1;
    }
    return ret;
#endif
    return 0;
}

static int _trave_dir(const char* path, const char *p_find_str, int *pfile_no)
{
#if 0
    DIR *dp;
    struct dirent *file;
    struct stat sb;
    int ret = 0;
    char temp[64];

    if(NULL == (dp=opendir(path)))
    {
        error("error opendir %s!!!\n", path);
        return -1;
    }

    while((file=readdir(dp)) != NULL)
    {
        // 避免死循环遍历目录
        if(strncmp(file->d_name, ".", 1)==0 || strcmp(file->d_name, "..")==0)
            continue;

        memset(temp, 0, sizeof(temp));
        strncpy(temp, path, strlen(path));
        strcat(temp, file->d_name);		//chdir(path);--不能使用chdir函数，否则/usb/disk80/目录无法卸载
        warning("=======> temp:%s\n", temp);
        ret = stat(temp, &sb);			// 需使用绝对路径，否则stat()函数会因为查找不到文件而出错
        if (-1 == ret)
        {
            error("stat error! file->d_name=%s\n", file->d_name);
            continue;
        }
        else if(0 == ret)
        {
            if (S_ISREG(sb.st_mode))
            {
                char *p_find = NULL;
                char *p_next = NULL;
                if ((p_find=strstr(file->d_name, p_find_str)) != NULL)
                {
                    if (strstr(temp, "/"))
                    {
                        if (*pfile_no < MAX_UPGRADE_FILE)
                            strncpy(upgrade_filename[(*pfile_no)++], file->d_name, MAX_FILE_NAME_LEN);
                    }
                }
            }
        }
    }

    closedir(dp);
#endif
    return 0;
}

//static int _parse_file_head(const char *path, upgrad_pkg_file_t *p_pkg_file)
static int _parse_file_head(const char *path)

{
    int fd = -1;
    int ret = 0;
#if 0
    upgrade_packet_t file_head;
    memset(&file_head, 0, sizeof(upgrade_packet_t));


    if ((fd = open(path, O_RDONLY)) < 0)
    {
        error("open error!\n");
        return -1;
    }

    ret = read(fd, (char *)&file_head, sizeof(upgrade_packet_t));
    if(ret != sizeof(upgrade_packet_t))
    {
        error("size error! ret=%d\n", ret);
        close(fd);
        return -1;
    }

    strncpy((char *)p_pkg_file->date, (char *)file_head.date, strlen((char *)file_head.date));
    strncpy((char *)p_pkg_file->device_type, (char *)file_head.device_type, strlen((char *)file_head.device_type));
    p_pkg_file->size = file_head.size;
    strncpy((char *)p_pkg_file->ver, (char *)file_head.ver, strlen((char *)file_head.ver));

    close(fd);
#endif

    return 0;

}

static int _msg_upgrad_pro(sdk_msg_t *pmsg)
{
    int up_percent = upgrade_get_status();
     pmsg->extend = up_percent;
     pmsg->size = 0;

    return 0;
}

int main_msg_record_cfg(sdk_msg_t *pmsg)
{
    int weekday = 0;
    int is_set = pmsg->args;
    int ch     = pmsg->chann;
    int i = 0;
    int max_ch = adapt_param_get_chnum();
    sdk_record_cfg_t *rec_cfg = (sdk_record_cfg_t *)pmsg->data;
#if 1
    //record_param_t rec_parm;

    if(is_set == SDK_PARAM_SET)
    {
        if (CHANN_TYPE_ALL == ch)
        {
            for (i=0; i<max_ch; i++)
            {
               /* memset(&rec_parm, 0, sizeof(rec_parm));
                sdk_record_getRecordParam(&rec_parm);
                if (rec_cfg->pre_record_time > 0)
                {
                    rec_parm.en_prerecord = 1;
                    sdk_record_setRecordParam(&rec_parm);
                }
                else
                {
                    rec_parm.en_prerecord = 0;
                    sdk_record_setRecordParam(&rec_parm);
                }*/

                adapt_param_set_record_cfg(i, rec_cfg);
            }
        }
        else
        {
            /*memset(&rec_parm, 0, sizeof(rec_parm));
            sdk_record_getRecordParam(&rec_parm);
            if (rec_cfg->pre_record_time > 0)
            {
                rec_parm.en_prerecord = 1;
                sdk_record_setRecordParam(&rec_parm);
            }
            else
            {
                rec_parm.en_prerecord = 0;
                sdk_record_setRecordParam(&rec_parm);
            }*/

            adapt_param_set_record_cfg(ch, rec_cfg);
        }

        pmsg->size = 0;
        WRITE_LOG(LOG_MAJOR_PARAM, L_PARAM_MINOR_PARAM_RECORD, NO_CHAN_ITEM, NO_ALMIN_ITEM);
    }
    else
    {
        adapt_param_get_record_cfg(ch, rec_cfg);
        pmsg->size = sizeof(sdk_record_cfg_t);

#if 0
        printf("<><><><><><><><><><> ch = %d \n",ch);
        for (weekday=0; weekday<8; weekday++)
        {
            printf("--- weekday=%u, enable=%u, time[0].enable=%u, b_h=%u, b_m=%u, e_h=%u, e_m=%u\n",
                   weekday, rec_cfg->enable, rec_cfg->record_sched[weekday].sched_time[0].enable,
                   rec_cfg->record_sched[weekday].sched_time[0].start_hour,
                   rec_cfg->record_sched[weekday].sched_time[0].start_min,
                   rec_cfg->record_sched[weekday].sched_time[0].stop_hour,
                   rec_cfg->record_sched[weekday].sched_time[0].stop_min);
        }
#endif
    }
#endif

    return 0;
}

int main_msg_record_query(sdk_msg_t *pmsg)
{

    return 0;
}

static int _encode_param_validity_check(sdk_encode_t *p_enc_cfg)
{
	
#if 0
    sdk_comm_cfg_t comm_cfg;
    memset(&comm_cfg, 0, sizeof(comm_cfg));
    adapt_param_get_comm_cfg(&comm_cfg);

    if (p_enc_cfg->main.bitrate<32 || p_enc_cfg->main.bitrate>16000)
    {
        p_enc_cfg->main.bitrate = 2048;
    }
    if (p_enc_cfg->second.bitrate<32 || p_enc_cfg->second.bitrate>16000)
    {
        p_enc_cfg->second.bitrate = 128;
    }

    if (comm_cfg.video_mode==0 && (p_enc_cfg->main.frame_rate<1 || p_enc_cfg->main.frame_rate>25))
    {
        p_enc_cfg->main.frame_rate = 25;
    }
    else if (comm_cfg.video_mode==1 && (p_enc_cfg->main.frame_rate<1 || p_enc_cfg->main.frame_rate>30))
    {
        p_enc_cfg->main.frame_rate = 25;
    }
    if (comm_cfg.video_mode==0 && (p_enc_cfg->second.frame_rate<1 || p_enc_cfg->second.frame_rate>25))
    {
        p_enc_cfg->second.frame_rate = 12;
    }
    else if (comm_cfg.video_mode==1 && (p_enc_cfg->second.frame_rate<1 || p_enc_cfg->second.frame_rate>30))
    {
        p_enc_cfg->second.frame_rate = 12;
    }

    if (p_enc_cfg->main.gop<1 || p_enc_cfg->main.gop>200)
    {
        p_enc_cfg->main.gop = 25;
    }
    if (p_enc_cfg->second.gop<1 || p_enc_cfg->second.gop>200)
    {
        p_enc_cfg->second.gop = 50;
    }
#ifndef  _ARCH_3518_
    if (p_enc_cfg->main.resolution>SDK_VIDEO_RESOLUTION_QXGA)
    {
        p_enc_cfg->main.resolution = SDK_VIDEO_RESOLUTION_D1;
    }
    if (p_enc_cfg->second.resolution>SDK_VIDEO_RESOLUTION_QXGA)
    {
        p_enc_cfg->second.resolution = SDK_VIDEO_RESOLUTION_QCIF;
    }
#else

    if (p_enc_cfg->main.resolution>SDK_VIDEO_RESOLUTION_SVGA)
    {
        p_enc_cfg->main.resolution = SDK_VIDEO_RESOLUTION_720p;
    }
    if (p_enc_cfg->second.resolution>SDK_VIDEO_RESOLUTION_SVGA)
    {
        p_enc_cfg->second.resolution = SDK_VIDEO_RESOLUTION_D1;
    }
#endif
#endif
    return 0;
}

int main_msg_encode_cfg(sdk_msg_t *pmsg)
{
    int ret = 0;
    int is_set = pmsg->args;
    int ch     = pmsg->chann;
    int i = 0;
    int max_ch = adapt_param_get_ana_chan_num();
    sdk_encode_t *enc_cfg = (sdk_encode_t *)pmsg->data;

#if 1//change by zw
    if(is_set == SDK_PARAM_SET)
    {
        // 编码参数合法性检查
        _encode_param_validity_check(enc_cfg);
		warning("----->enc_cfg->av_enc_info[0].frame_rate %d \n",enc_cfg->av_enc_info[0].frame_rate);
        {
            ret = adapt_codec_encode_cfg(pmsg);
            if(ret < 0)
            {
                error("adapt_codec_encode_cfg error!\n");
                pmsg->size = 0;
                pmsg->ack = SDK_ERR_SET_ENCODE_PARAM;
                return -1;
            }

           // printf("1111111 enc_cfg->main.resolution:%d \n",enc_cfg->main.resolution);
            ret = adapt_param_set_encode_cfg(0, enc_cfg);
        }
        pmsg->size = 0;
        WRITE_LOG(LOG_MAJOR_PARAM, L_PARAM_MINOR_PARAM_VENC, NO_CHAN_ITEM, NO_ALMIN_ITEM);
    }
    else
    {
        ret = adapt_param_get_encode_cfg(0, enc_cfg);
        pmsg->size = sizeof(sdk_encode_t);
    	for(i=0;i<MAX_VIDEO_STREAM_NUM;i++)
		printf("@@@@@@@@@@@@@@: GET chn%d: %d %d %d\n",i,enc_cfg->av_enc_info[i].resolution,
						enc_cfg->av_enc_info[i].bitrate_type,enc_cfg->av_enc_info[i].frame_rate);

    }
	#endif
   return ret;
}

int main_msg_chann_cfg(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
    int ch     = pmsg->chann;
    int i = 0;
    //int max_ch = adapt_param_get_ana_chan_num();
    int max_ch = adapt_param_get_chnum();
#if 0
    sdk_channel_t ch_cfg;
    sdk_channel_t *chann_cfg = (sdk_channel_t*)pmsg->data;

    if(is_set == SDK_PARAM_SET)
    {
        if (CHANN_TYPE_ALL == ch)
        {
            warning(" SET ===> ALL CH ===> max_ch:%d, dev_type:0x%x\n", max_ch, g_main_dev_type);
            for (i=0; i<max_ch; i++)
            {
                adapt_param_set_chann_cfg(i, chann_cfg);
                if (adapt_param_dev_is_nvr(g_main_dev_type))
                {
                    warning("chann_cfg->enable:%d, manufacturer:%d\n", chann_cfg->enable, chann_cfg->device.manufacturer);
                    if(chann_cfg->enable)
                    {
                        adapt_pua_open(i);
                    }
                    else
                    {
                        adapt_pua_close(i);
                    }
                }
            }
        }
        else
        {
            adapt_param_set_chann_cfg(ch, chann_cfg);
            if (adapt_param_dev_is_nvr(g_main_dev_type))
            {
                warning("chann_cfg->enable:%d, manufacturer:%d\n", chann_cfg->enable, chann_cfg->device.manufacturer);
                if(chann_cfg->enable)
                {
                    adapt_pua_open(ch);
                }
                else
                {
                    adapt_pua_close(ch);
                }
            }
        }

        pmsg->size = 0;
        WRITE_LOG(LOG_MAJOR_PARAM, L_PARAM_MINOR_PARAM_CHAN, NO_CHAN_ITEM, NO_ALMIN_ITEM);
    }
    else
    {
        pmsg->size = 0;

        if (ch == CHANN_TYPE_ALL)
        {
            warning(" GET ===> ALL CH ===> max_ch:%d, dev_type:0x%x\n", max_ch, g_main_dev_type);
            for (i=0; i<max_ch; i++)
            {
                memset(&ch_cfg, 0, sizeof(sdk_channel_t));
                adapt_param_get_chann_cfg(i, &ch_cfg);
                memcpy(pmsg->data+pmsg->size, &ch_cfg, sizeof(sdk_channel_t));
                pmsg->size += sizeof(sdk_channel_t);
            }
        }
        else
        {
            adapt_param_get_chann_cfg(ch, chann_cfg);
            pmsg->size = sizeof(sdk_channel_t);
        }
    }
    pmsg->ack = SDK_ERR_SUCCESS;
#endif

    return 0;
}

int main_msg_user(sdk_msg_t *pmsg)
{
    int ret = -1;
    int user_op = pmsg->args;
#if 1
    warning("======> user_op=0x%x\n", user_op);
    switch (user_op)
    {
    case SDK_USER_AAA:
    {
        ret = _msg_user_aaa(pmsg);
        break;
    }

    case SDK_USER_ADD:
    {
        ret = _msg_user_add(pmsg);
        break;
    }

    case SDK_USER_DEL:
    {
        ret = _msg_user_del(pmsg);
        break;
    }

    case SDK_USER_MODI:
    {
        ret = _msg_user_modif(pmsg);
        break;
    }

    case SDK_USER_QUERY:
    {
        ret = _msg_user_query(pmsg);
        break;
    }

    default:
        error("No operation!\n");
        break;
    }
#endif
    return ret;
}

static int _msg_user_aaa(sdk_msg_t *pmsg)
{
    int i = 0;

    sdk_user_t *user = (sdk_user_t *)pmsg->data;
    sdk_user_right_t user_array[MAX_USER_NUM];

    memset(user_array, 0, sizeof(user_array));
    adapt_param_get_user_array(user_array, sizeof(user_array));

    for(i=0; i<MAX_USER_NUM; i++)
    {
        if (!strcmp((char *)user_array[i].user.user_name, (char *)user->user_name))
        {
            if (!strcmp((char *)user_array[i].user.user_pwd, (char *)user->user_pwd))
            {
                break;
            }
            else
            {
                error("----- password error!!!\n");
                pmsg->ack = SDK_ERR_PASS;
                pmsg->size = 0;
                return -1;
            }
        }
    }

    if(i < MAX_USER_NUM)
    {
        memcpy(pmsg->data, &user_array[i], sizeof(sdk_user_right_t));
        pmsg->size = sizeof(sdk_user_right_t);
    }
    else
    {
        error("-------- user not exist!!!\n");
        pmsg->size = 0;
        pmsg->ack = SDK_ERR_USER_NOT_EXIST;
        return -1;
    }

    // 记录本地当前用户名及IP
    if (strncmp((char *)(g_log_info.user_name), (char *)user->user_name, MAX_USER_NAME_LEN))
    {
        warning("==> g_log_info.user_name:%s, user->user_name:%s\n", g_log_info.user_name, user->user_name);
        memset(&g_log_info, 0, sizeof(g_log_info));
        strncpy((char *)g_log_info.user_name, (char *)user->user_name, strlen(user->user_name));
        sdk_log_ioctrl(SDK_LOG_CMD_SET_USER, (char *)g_log_info.user_name, strlen((char *)g_log_info.user_name));
    }

    pmsg->size = 0;
    // 常规日志
    WRITE_LOG(LOG_MAJOR_OPERATE, L_OPERATE_MINOR_LOGIN, NO_CHAN_ITEM, NO_ALMIN_ITEM);
    // DEBUG日志
    //DEBUG_LOG("This is a test debug log!");

    return 0;
}

static int _msg_user_del(sdk_msg_t *pmsg)
{
    int i = 0;

    sdk_user_t *user = (sdk_user_t *)pmsg->data;
    sdk_user_right_t user_array[MAX_USER_NUM];

    // 超级用户不允许删除
    if (!memcmp(user->user_name, "admin", strlen((char *)user->user_name)))
    {
        error("Cannot delete root user!!!\n");
        pmsg->size = 0;
        pmsg->ack = SDK_ERR_ROOT_USER;
        return -1;
    }

    memset(user_array, 0, sizeof(user_array));
    adapt_param_get_user_array(user_array, sizeof(user_array));

    for(i = 0; i<MAX_USER_NUM; i++)
    {
		if((!strcmp((char *)user_array[i].user.user_name, (char *)user->user_name))
                && (!strcmp((char *)user_array[i].user.user_pwd, (char *)user->user_pwd)))
        {
            break;
        }
    }
    if(i < MAX_USER_NUM)
    {
        printf("####  del user_name: %s\n", user->user_name);
        memset(&user_array[i], 0, sizeof(sdk_user_right_t));
        adapt_param_set_user_array(user_array, sizeof(user_array));
    }
    else
    {
        error("not find user_name: %s\n", user->user_name);
        pmsg->ack = SDK_ERR_USER_NOT_EXIST;
        pmsg->size = 0;
        return -1;
    }

    pmsg->size = 0;
    WRITE_LOG(LOG_MAJOR_OPERATE, L_OPERATE_MINOR_USER_DEL, NO_CHAN_ITEM, NO_ALMIN_ITEM);

    return 0;
}

static int _msg_user_add(sdk_msg_t *pmsg)
{
    int i = 0, ret = 0;
    int is_already = 0;

    sdk_user_right_t *user_right  = (sdk_user_right_t*)pmsg->data;
    sdk_user_right_t user_array[MAX_USER_NUM];

    memset(user_array, 0, sizeof(user_array));
    adapt_param_get_user_array(user_array, sizeof(user_array));

    for(i = 0; i < MAX_USER_NUM; i++)
    {
        if(!strcmp((char *)user_array[i].user.user_name, (char *)user_right->user.user_name))
        {
            break;
        }
    }
    if(i < MAX_USER_NUM)
    {
        error("user_name: %s already exist\n", user_right->user.user_name);
        is_already = 1;
        pmsg->ack = SDK_ERR_USER_ALREADY_EXIST;
        pmsg->size = 0;
        ret = -1;
    }

    if(is_already == 0)
    {
        for(i = 0; i < MAX_USER_NUM; i++)
        {
            if(strlen((char *)user_array[i].user.user_name) == 0)
            {
                break;
            }
        }
        if(i < MAX_USER_NUM)
        {
            printf("^^^ i=%d, add user_name=%s, user_pwd=%s, sizeof(sdk_user_right_t)=%d, sizeof(user_array)=%d\n",
                   i, user_right->user.user_name, user_right->user.user_pwd, sizeof(sdk_user_right_t),
                   sizeof(user_array));

            memcpy(&user_array[i], user_right, sizeof(sdk_user_right_t));
            adapt_param_set_user_array(user_array, sizeof(user_array));
        }
        else
        {
            error("user num > MAX_USER_NUM\n");
            pmsg->ack = SDK_ERR_TOO_MANY_USER;
            pmsg->size = 0;
            ret = -1;
        }
    }
    pmsg->size = 0;
    WRITE_LOG(LOG_MAJOR_OPERATE, L_OPERATE_MINOR_USER_ADD, NO_CHAN_ITEM, NO_ALMIN_ITEM);

    return ret;
}

static int _msg_user_modif(sdk_msg_t *pmsg)
{
    int i = 0, ret = 0;
	printf("===================modif\n");
    sdk_user_right_t *user_right  = (sdk_user_right_t*)pmsg->data;
    sdk_user_right_t user_array[MAX_USER_NUM];

    memset(user_array, 0, sizeof(user_array));
    adapt_param_get_user_array(user_array, sizeof(user_array));

    for(i = 0; i < MAX_USER_NUM; i++)
    {
        if(!strcmp((char *)user_array[i].user.user_name, (char *)user_right->user.user_name))
        {
            break;
        }
    }
    if(i < MAX_USER_NUM)
    {
        if (0 == i)	//超级用户的用户名不允许删除、修改，但可以修改密码。
        {
			strcpy(user_array[0].user.user_name,"admin");
			strcpy(user_array[0].user.user_pwd,user_right->user.user_pwd);
			printf("================111111111\n");
		}
        else
        {
            memcpy(&user_array[i], user_right, sizeof(sdk_user_right_t));
		}
        int vvv= adapt_param_set_user_array(user_array, sizeof(user_array));
    }
    else
    {
        error("user_name: %s not exist\n", user_right->user.user_name);
        pmsg->ack = SDK_ERR_USER_NOT_EXIST;
        pmsg->size = 0;
        ret = -1;
    }

    pmsg->size = 0;
    WRITE_LOG(LOG_MAJOR_OPERATE, L_OPERATE_MINOR_USER_MODI, NO_CHAN_ITEM, NO_ALMIN_ITEM);
	printf("================5555555555\n");

    return ret;
}

static int _msg_user_query(sdk_msg_t *pmsg)
{
    int i = 0;

    sdk_user_right_t user_array[MAX_USER_NUM];

    pmsg->size = 0;

    memset(user_array, 0, sizeof(user_array));
    adapt_param_get_user_array(user_array, sizeof(user_array));

    for(i=0; i<MAX_USER_NUM; i++)
    {
        //printf("----- i=%d, user_name=%s, pwd=%s, pmsg->size=%d, strlen(user_array[%d].user.user_name)=%d\n",
        //i, user_array[i].user.user_name, user_array[i].user.user_pwd, pmsg->size,
        //i, strlen(user_array[i].user.user_name));

        if(strlen((char *)user_array[i].user.user_name) != 0)
        {
            printf("--------- i=%d\n", i);
            memcpy(pmsg->data+pmsg->size, &user_array[i], sizeof(sdk_user_right_t));
            pmsg->size += sizeof(sdk_user_right_t);
        }
    }

    return 0;
}

int main_msg_log(sdk_msg_t *pmsg)
{
    int op_type = pmsg->args;
    int ret = 0;

    switch (op_type)
    {
    case SDK_LOG_QUERY:
    {
        ret = _msg_log_query(pmsg);
        if(ret < 0)
        {
            error("_log_query error!\n");
            pmsg->size = 0;
            pmsg->ack = SDK_ERR_LOG_QUERY;
            return -1;
        }
        break;
    }

    case SDK_LOG_EMPTY:
    {
        ret = _msg_log_empty(pmsg);
        if (ret < 0)
        {
            error("_log_empty error!\n");
            pmsg->size = 0;
            pmsg->ack = SDK_ERR_LOG_EMPTY;
            return -1;
        }
        break;
    }

    case SDK_LOG_BACKUP:
    {
        ret = _msg_log_backup(pmsg);
        if (ret < 0)
        {
            error("_log_backup error!\n");
            pmsg->size = 0;
            pmsg->ack = SDK_ERR_LOG_BACKUP;
            return -1;
        }
        break;
    }

    case SDK_LOG_DEL:
    {
        ret = _msg_log_delete(pmsg);
        if (ret < 0)
        {
            error("_msg_log_delete error!\n");
            pmsg->size = 0;
            pmsg->ack = SDK_ERR_LOG_DEL;
            return -1;
        }
        break;
    }


    default:
    {
        error("main_msg_log error!\n");
        return -1;
    }
    }

    return ret;
}

static int  _msg_log_query(sdk_msg_t *pmsg)
{
    int left_num = 0;
    sdk_log_cond_t *p_log = (sdk_log_cond_t *)pmsg->data;
    sdk_log_cond_t log_cond;
    memset(&log_cond, 0, sizeof(log_cond));
    memcpy(&log_cond, p_log, sizeof(sdk_log_cond_t));
    warning("===> b_time:%u-%u-%d %u:%u:%u  e_time:%u-%u-%u %u:%u:%u, type=0x%x\n",
            log_cond.begin_time.year, log_cond.begin_time.mon, log_cond.begin_time.day, log_cond.begin_time.hour, log_cond.begin_time.min, log_cond.begin_time.sec,
            log_cond.end_time.year, log_cond.end_time.mon, log_cond.end_time.day, log_cond.end_time.hour, log_cond.end_time.min, log_cond.end_time.sec,
            log_cond.type);

    time_t begin_time = 0;
    time_t end_time = 0;
    log_list_t *log_list = NULL;
    log_list_t *loop = NULL;
    sdk_log_item_t log_item;
    uint32_t year, month ,day, hour, minute, second, weekday;

    begin_time = os_make_time(log_cond.begin_time.year, log_cond.begin_time.mon, log_cond.begin_time.day,
                              log_cond.begin_time.hour, log_cond.begin_time.min, log_cond.begin_time.sec);

    end_time = os_make_time(log_cond.end_time.year, log_cond.end_time.mon, log_cond.end_time.day,
                            log_cond.end_time.hour, log_cond.end_time.min, log_cond.end_time.sec);

    sdk_log_query(log_cond.type, begin_time, end_time, &log_list);

    memset(p_log, 0, sizeof(sdk_log_cond_t));
    pmsg->size = 0;
    pmsg->extend = 0;

    if (log_list == NULL)
        error("=========  log_list NULL NULL\n");

    loop = log_list;

    while(loop)
    {
        log_list_t *tmp = loop;
        if (pmsg->size < (128*1024 - sizeof(sdk_msg_t) - sizeof(sdk_log_item_t)))
        {
            //info("time:%d, type:%u, type2:%u, args:%u, user:%s, ip:%s, desc:%s\n",
            //loop->node.time, loop->node.type, loop->node.type2, loop->node.args, loop->node.user, loop->node.ip_addr, loop->node.desc);
            memset(&log_item, 0, sizeof(log_item));
            os_local_time(loop->node.time, &year, &month, &day, &hour, &minute, &second, &weekday);
            log_item.time.year = year;
            log_item.time.mon = month;
            log_item.time.day = day;
            log_item.time.hour = hour;
            log_item.time.min = minute;
            log_item.time.sec = second;
            log_item.major_type = loop->node.type;
            log_item.minor_type = loop->node.type2;
            log_item.args = loop->node.args;
            strncpy((char *)log_item.user, loop->node.user, MAX_USER_NAME_LEN);
            strncpy((char *)log_item.ip_addr, loop->node.ip_addr, MAX_IP_ADDR_LEN);
            strncpy((char *)log_item.desc, loop->node.desc, MAX_LOG_DESC_LEN);
            //warning("---- major_type:0x%x, minor_type:0x%x, args:%u, user:%s, ip:%s, desc:%s\n",
            //log_item.major_type, log_item.minor_type, log_item.args, log_item.user, log_item.ip_addr, log_item.desc);
            memcpy(pmsg->data + pmsg->size, &log_item, sizeof(sdk_log_item_t));
            pmsg->size += sizeof(sdk_log_item_t);
            pmsg->extend++;

        }
        else
        {
            left_num++;
        }
        loop = loop->next;
        free(tmp);
    }
    warning(">>>>> log query fill[ item_num:%d, size:%d], left_num:%d\n"
            , pmsg->extend
            , pmsg->size
            , left_num);
    return 0;
}

static int _msg_log_empty(sdk_msg_t *pmsg)
{
    int year = 2037;
    int mon  = 12;
    int day  = 31;
    int hour = 23;
    int min  = 59;
    int sec  = 59;
    uint32_t type = 0xFFFFFFFF;
    time_t begin_time = 0;
    time_t end_time = 0;

    begin_time = 0;
    end_time = os_make_time(year, mon, day, hour, min, sec);
    info("====> type:0x%x, begin_time:%u, end_time:%u\n", type, begin_time, end_time);
    sdk_log_delete(type, begin_time, end_time);

    pmsg->size = 0;
    WRITE_LOG(LOG_MAJOR_OPERATE, L_OPERATE_MINOR_LOG_EMPTY, NO_CHAN_ITEM, NO_ALMIN_ITEM);

    return 0;
}

static int _msg_log_backup(sdk_msg_t *pmsg)
{
    warning("_log_backup is not realize....\n");

    pmsg->size = 0;
    return 0;
}

static int _msg_log_delete(sdk_msg_t *pmsg)
{
    warning("_msg_log_delete ....\n");

    sdk_log_cond_t *p_log = (sdk_log_cond_t *)pmsg->data;
    sdk_log_cond_t log_cond;
    memset(&log_cond, 0, sizeof(log_cond));
    memcpy(&log_cond, p_log, sizeof(sdk_log_cond_t));
    warning("===> b_time:%u-%u-%d %u:%u:%u  e_time:%u-%u-%u %u:%u:%u, type=0x%x\n",
            log_cond.begin_time.year, log_cond.begin_time.mon, log_cond.begin_time.day, log_cond.begin_time.hour, log_cond.begin_time.min, log_cond.begin_time.sec,
            log_cond.end_time.year, log_cond.end_time.mon, log_cond.end_time.day, log_cond.end_time.hour, log_cond.end_time.min, log_cond.end_time.sec,
            log_cond.type);

    uint32_t type = log_cond.type;
    time_t begin_time = 0;
    time_t end_time = 0;

    begin_time = os_make_time(log_cond.begin_time.year, log_cond.begin_time.mon, log_cond.begin_time.day,
                              log_cond.begin_time.hour, log_cond.begin_time.min, log_cond.begin_time.sec);
    end_time = os_make_time(log_cond.end_time.year, log_cond.end_time.mon, log_cond.end_time.day,
                            log_cond.end_time.hour, log_cond.end_time.min, log_cond.end_time.sec);
    sdk_log_delete(type, begin_time, end_time);

    pmsg->size = 0;
    WRITE_LOG(LOG_MAJOR_OPERATE, L_OPERATE_MINOR_LOG_DEL, NO_CHAN_ITEM, NO_ALMIN_ITEM);

    return 0;
}

int rand_number(void)
{
	if(0 ==rand_number_flag)
	{
		srand((int)time(0));//获取一个随机数组成设备ID  测试时使用真正的会在出厂时写入
		number=1+(int)(10.0*rand()/(RAND_MAX+1.0));
		rand_number_flag ++;
	}
	
}
int main_msg_sys_cfg(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
    sdk_sys_cfg_t *sys_cfg = (sdk_sys_cfg_t*)pmsg->data;
	char buf[128];

    if(is_set == SDK_PARAM_SET)
    {
        error("\n Set SYS_CFG is not support current!!!\n");//暂时支持一下
        adapt_param_set_sys_cfg(sys_cfg);
        adapt_param_set_def_sys_cfg(sys_cfg);

        pmsg->size = 0;
    }
    else
    {
        printf("------- get sys_cfg param --------\n");
        adapt_param_get_sys_cfg(sys_cfg);

        //测试使用  添加一个随机数组成设备id
        // rand_number();

		//sprintf(buf,"AONI_MD-%d ",number);
		//strcpy(sys_cfg->serial_1,buf);//设备唯一ID
		 
        strcpy(sys_cfg->serial_1,"JOOAN_001");//设备唯一ID
        //strcpy(sys_cfg->serial_1,"AONI_MD-ZW");//设备唯一ID
        adapt_param_set_sys_cfg(sys_cfg);
        adapt_param_set_def_sys_cfg(sys_cfg);
      
        adapt_param_version_def((char *)sys_cfg->software_ver,
                                sizeof(sys_cfg->software_ver),
                                (char *)sys_cfg->software_date,
                                sizeof(sys_cfg->software_date));

        pmsg->size = sizeof(sdk_sys_cfg_t);
    }
    return 0;
}

int main_msg_platform_cfg(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
    sdk_platform_t *plat_cfg = (sdk_platform_t*)pmsg->data;

    if(is_set == SDK_PARAM_SET)
    {
        printf("------- set plat_cfg param --------\n");
        adapt_param_set_platform_cfg(plat_cfg);
        pmsg->size = 0;
    }
    else
    {
        printf("------- get plat_cfg param --------\n");
        adapt_param_get_platform_cfg(plat_cfg);
        pmsg->size = sizeof(sdk_platform_t);
    }
    return 0;
}
static int _change_web_port(int port)
{
#if 0
    char msg_buf[MAX_MSG_SIZE] = {0};
    sdk_msg_t *pmsg = (sdk_msg_t *)msg_buf;
    webs_cfg_t *cfg = pmsg->data;

    cfg->port = port;
    pmsg->msg_id = WEBS_MSG_CFG;
    pmsg->size = sizeof(webs_cfg_t);        //must
    pmsg->ack = 0;
    pmsg->f_dir = SDK_MSG_REQ;              //must
    apcs_msg_send(MODULE_ID_WEB, pmsg);
#endif

    return 0;
}

int main_msg_net_server_cfg(sdk_msg_t *pmsg)
{
    int ret = 0;
    int is_set = pmsg->args;
    sdk_net_mng_cfg_t *net_mng = (sdk_net_mng_cfg_t*)pmsg->data;
    sdk_net_mng_cfg_t old_mng;
    adapt_param_get_net_mng_cfg(&old_mng);

    warning(">>>>>> def_if_no:%d, old_http_port:%d, new_http_port:%d\n"
            , net_mng->def_if_no
            , old_mng.http_port
            , net_mng->http_port);

    if(is_set == SDK_PARAM_SET)
    {
        if(!memcmp(&old_mng, net_mng, sizeof(sdk_net_mng_cfg_t)))
        {
            pmsg->size = 0;
            return 0;
        }
        if(adapt_os_set_net_mng_cfg(net_mng) == 0)
        {
            adapt_param_set_net_mng_cfg(net_mng);
            pmsg->size = 0;

            if(old_mng.http_port != net_mng->http_port)
            {
                _change_web_port(net_mng->http_port);
            }
            WRITE_LOG(LOG_MAJOR_PARAM, L_PARAM_MINOR_PARAM_NETWORK, NO_CHAN_ITEM, NO_ALMIN_ITEM);
        }
        else
        {
            pmsg->ack = SDK_ERR_NET_CFG;
            pmsg->size = 0;
            return -1;
        }
    }
    else
    {
        adapt_param_get_net_mng_cfg(net_mng);
        ret = adapt_os_get_net_mng_cfg(net_mng);
        if (ret < 0)
        {
            error("adapt_os_get_net_mng_cfg error!\n");
            pmsg->ack = SDK_ERR_NET_CFG;
            pmsg->size = 0;
            return -1;
        }
        pmsg->size = sizeof(sdk_net_mng_cfg_t);
    }
    return 0;
}

int main_msg_ptz_cfg(sdk_msg_t *pmsg)
{
    uint32_t is_set = pmsg->args;
    uint32_t ch     = pmsg->chann;
    int i = 0;
    int max_ch = adapt_param_get_ana_chan_num();
    sdk_ptz_param_t *ptz_cfg = (sdk_ptz_param_t*)pmsg->data;

    printf("+++++ is_set=%u, ch=%u\n", is_set, ch);

    if(is_set == SDK_PARAM_SET)
    {
        printf("---- ch=%u, address=%u, protocol=%u, baud_rate=%u, data_bit=%u, flow_ctl=%u, parity=%u, stop_bit=%u\n",
               ch,
               ptz_cfg->address,
               ptz_cfg->protocol,
               ptz_cfg->comm.baud_rate,
               ptz_cfg->comm.data_bit,
               ptz_cfg->comm.flow_ctl,
               ptz_cfg->comm.parity,
               ptz_cfg->comm.stop_bit);

        if (CHANN_TYPE_ALL == ch)
        {
            for (i=0; i<max_ch; i++)
            {
                adapt_param_set_ptz_cfg(i, ptz_cfg);
            }
        }
        else
        {
            adapt_param_set_ptz_cfg(ch, ptz_cfg);
        }

        pmsg->size = 0;
        WRITE_LOG(LOG_MAJOR_PARAM, L_PARAM_MINOR_PARAM_PTZ, NO_CHAN_ITEM, NO_ALMIN_ITEM);
    }
    else
    {
        adapt_param_get_ptz_cfg(ch, ptz_cfg);
        pmsg->size = sizeof(sdk_ptz_param_t);

        printf("---- ch=%u, address=%u, protocol=%u, baud_rate=%u, data_bit=%u, flow_ctl=%u, parity=%u, stop_bit=%u\n",
               ch,
               ptz_cfg->address,
               ptz_cfg->protocol,
               ptz_cfg->comm.baud_rate,
               ptz_cfg->comm.data_bit,
               ptz_cfg->comm.flow_ctl,
               ptz_cfg->comm.parity,
               ptz_cfg->comm.stop_bit);
    }
    return 0;
}

//还要做逻辑处理 定义等级

//返回当前应该使用的通道号
static int modify_image_quality(uint8_t image_quality )
{
    int channel = 0;
    char msg_buffer[2048] = {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buffer;
    sdk_av_enc_t *av_enc = (sdk_av_enc_t *)pMsg->data;
    sdk_av_enc_t *penc = NULL;
    pMsg->chann = 0;
    sdk_encode_t encode;
    memset(&encode,0,sizeof(sdk_encode_t));
    adapt_param_get_encode_cfg(0, &encode);
    info("=========image_quality:%d  \n",image_quality);
    switch(image_quality)
    {
    case 0:  // 720p 15pfs   1024kbps
    {
        pMsg->chann = 0 ;
        penc = &encode.av_enc_info[0];
        penc->resolution = SDK_VIDEO_RESOLUTION_720p;
        penc->frame_rate = 15;
        penc->gop = 15;
        penc->bitrate = 1024;
        break ;
    }
    case 1:		// 720p 20pfs   768kbps
    {
        pMsg->chann = 0 ;
        penc = &encode.av_enc_info[0];
        penc->resolution = SDK_VIDEO_RESOLUTION_720p;
        penc->frame_rate = 20;
        penc->gop = 20;
        penc->bitrate = 768;
        break ;
    }
    case 2:		// VGA 20pfs   512kbps
    {
        pMsg->chann = 1 ;
        penc = &encode.av_enc_info[1];
        penc->resolution = SDK_VIDEO_RESOLUTION_VGA;
        penc->frame_rate = 20;
        penc->gop = 20;
        penc->bitrate = 512;
        break ;
    }
    case 3:  	// VGA 20pfs   384kbps
    {
        pMsg->chann = 1 ;
        penc = &encode.av_enc_info[1];
        penc->resolution = SDK_VIDEO_RESOLUTION_VGA;
        penc->frame_rate = 20;
        penc->gop = 20;
        penc->bitrate = 384;
        break ;
    }
    case 4:		// QVGA 25pfs   128kbps
    {
        pMsg->chann = 0 ;
        penc = &encode.av_enc_info[1];
        penc->resolution = SDK_VIDEO_RESOLUTION_QVGA;
        penc->frame_rate = 20;
        penc->gop = 20;
        penc->bitrate = 128;

        break ;
    }
    default: //默认是处于第二档 // VGA 20pfs   512kbps
    {
        pMsg->chann = 1 ;
        penc = &encode.av_enc_info[1];
        penc->resolution = SDK_VIDEO_RESOLUTION_VGA;
        penc->frame_rate = 20;
        penc->gop = 20;
        penc->bitrate = 512;

        break;
    }

    }
    channel = pMsg->chann;
    memcpy(av_enc,penc,sizeof(sdk_av_enc_t));
    adapt_param_set_encode_cfg(0, &encode); //保存参数
    adapt_av_encode_cfg(pMsg); // 操作实际的编码参数
    return channel;
}

int  main_msg_baidu_info(sdk_msg_t *pmsg)
{
    int channel = 0;
    int is_set = pmsg->args;
    sdk_baidu_info_t  *baidu_info = (sdk_baidu_info_t*)pmsg->data;
    sdk_baidu_info_t  old_baidu_info;
    memset(&old_baidu_info,0,sizeof(sdk_baidu_info_t));

    if(is_set == SDK_PARAM_SET)
    {
        adapt_param_get_baidu_cfg(&old_baidu_info);

        //逻辑处理
        info("old image_quality:%d ===== new image_quality:%d \n",old_baidu_info.image_quality,baidu_info->image_quality);
        //调整图像质量
        if(old_baidu_info.image_quality != baidu_info->image_quality)
        {
            channel = modify_image_quality(baidu_info->image_quality);
            info("========channel:%d \n",channel);
            adapt_baidu_set_stream_ch(channel);
        }

        // 设置百度的录像状态
        if(old_baidu_info.rec_status != baidu_info->rec_status)
        {
            adapt_baidu_set_rec_status(baidu_info->rec_status);
        }

        // 设置百度是否传声音
        if(old_baidu_info.audio_enable != baidu_info->audio_enable)
        {
            adapt_baidu_set_audio_status(baidu_info->audio_enable);
        }

        // 设置百度是否传声音
        if(old_baidu_info.audio_enable != baidu_info->audio_enable)
        {
            adapt_baidu_set_video_status(baidu_info->audio_enable);
        }


        pmsg->size = 0;
        info("set ===== user_id :%s \n image_quality:%d \n rec_status:%d \n video_enable:%d \n ==== end\n"
             ,baidu_info->user_id,baidu_info->image_quality,baidu_info->rec_status,baidu_info->video_enable);
        adapt_param_set_baidu_cfg(baidu_info);
        WRITE_LOG(LOG_MAJOR_PARAM, L_PARAM_MINOR_PARAM_POLL, NO_CHAN_ITEM, NO_ALMIN_ITEM);
    }
    else
    {
        pmsg->size = sizeof(sdk_baidu_info_t);
        memset(baidu_info,0,sizeof(sdk_baidu_info_t));
        adapt_param_get_baidu_cfg(baidu_info);
        info("get ===== user_id :%s \n image_quality:%d \n rec_status:%d \n video_enable:%d \n ==== end\n"
             ,baidu_info->user_id,baidu_info->image_quality,baidu_info->rec_status,baidu_info->video_enable);

        //测试使用
        strcpy(baidu_info->user_id,"4130398509");//测试使用 之后可以关闭 zw_test
    }

    return 0;
}
//#include "sdk_struct.h"
extern int adapt_set_wifi_params(sdk_wifi_cfg_t *wifi_cfg);
int main_msg_wifiparam_cfg(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
    sdk_wifi_cfg_t  *wifi_info = (sdk_wifi_cfg_t*)pmsg->data;
    sdk_wifi_cfg_t  old_wifi_info;
    memset(&old_wifi_info,0,sizeof(sdk_wifi_cfg_t));
    if(is_set == SDK_PARAM_SET)
    {
        adapt_param_get_wifi_cfg(&old_wifi_info);
        //逻辑处理
        pmsg->size = 0;
        printf("jy---test save3 wifi_info.ssid[%s],wifi_info.pwd[%s],wifi_info.enable[%d]\n",wifi_info->ssid,wifi_info->pwd,wifi_info->enable);
        adapt_param_set_wifi_cfg(wifi_info);
        //WRITE_LOG(LOG_MAJOR_PARAM, L_PARAM_MINOR_PARAM_POLL, NO_CHAN_ITEM, NO_ALMIN_ITEM);
		printf("========main_msg_wifiparam_cfg999999999\n");
		
		printf("========wifi_cfg111=%d\n",wifi_info);
		adapt_set_wifi_params(wifi_info);
		
		printf("========wifi_cfg222=%d\n",wifi_info);
    }
    else
    {
        pmsg->size = sizeof(sdk_wifi_cfg_t);
        adapt_param_get_wifi_cfg(wifi_info);
    }

    return 0;
    return 0;
}

int main_msg_cruise_cfg(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
    int ch     = pmsg->chann;
    int i = 0;
    return 0;
}

int main_msg_preview_cfg(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
#if 0
    sdk_preview_t *preview_cfg = (sdk_preview_t*)pmsg->data;

    //info(">>>>>>>> pmsg->ch:%d, is_set:%d\n", pmsg->chann, (is_set==SDK_PARAM_SET)?1:0);
    if(is_set == SDK_PARAM_SET)
    {
        // 主屏不允许禁止;
        if(pmsg->chann == 0)
            preview_cfg->enable = 1;

        adapt_param_set_preview_cfg(pmsg->chann, preview_cfg);
        pmsg->size = 0;
        WRITE_LOG(LOG_MAJOR_PARAM, L_PARAM_MINOR_PARAM_PREVIEW, NO_CHAN_ITEM, NO_ALMIN_ITEM);
    }
    else
    {
#if 0
        int i, j;
        printf("preview_cfg===> enable:%d, init_mode:%d\n", preview_cfg->enable, preview_cfg->init_mode);
        for(i = 0; i < MAX_PREVIEW_MODE; i++)
        {
            fprintf(stderr, "MODE:%d\n", i);
            for(j = 0; j < MAX_WINDOWS_NUM; j++)
            {
                fprintf(stderr, "%02d ", preview_cfg->windows[i][j]);
            }
            fprintf(stderr, "\n======\n");
        }
#endif
        adapt_param_get_preview_cfg(pmsg->chann, preview_cfg);
        pmsg->size = sizeof(sdk_preview_t);
    }
#endif

    return 0;
}

int main_msg_tour_cfg(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
#if 0
    sdk_tour_t *tour_cfg = (sdk_tour_t*)pmsg->data;

    //info(">>>>>>>> pmsg->ch:%d, is_set:%d\n", pmsg->chann, (is_set==SDK_PARAM_SET)?1:0);
    if(is_set == SDK_PARAM_SET)
    {
        adapt_param_set_tour_cfg(pmsg->chann, tour_cfg);
        pmsg->size = 0;
        WRITE_LOG(LOG_MAJOR_PARAM, L_PARAM_MINOR_PARAM_PREVIEW, NO_CHAN_ITEM, NO_ALMIN_ITEM);
    }
    else
    {
        adapt_param_get_tour_cfg(pmsg->chann, tour_cfg);
        pmsg->size = sizeof(sdk_tour_t);
    }
#endif
    return 0;
}


int main_msg_vo_binding(sdk_msg_t *pmsg)
{
    int i;
    int ret;
    int real_win_num;
    int is_preview = pmsg->args;
#if 0
    sdk_vo_binding_t *pvobinding_cfg = (sdk_vo_binding_t*)pmsg->data;
#if 0
    pn_clear_all();
    if(is_preview)
    {
        //网络通道预览处理
        pn_add_ch(pvobinding_cfg->mode, pvobinding_cfg->win_num, pvobinding_cfg->windows);
    }
#else
    info("is_preview:%u chann:%u mode:%u win_num:%u\n"
         ,pmsg->args
         ,pmsg->chann
         ,pvobinding_cfg->mode
         ,pvobinding_cfg->win_num);
    real_win_num = pvobinding_cfg->win_num;

    for(i = 0; i<pvobinding_cfg->win_num; i++)
    {
        if(pvobinding_cfg->windows[i] >= MAX_CHANN_NUM)
        {
            real_win_num--;
        }
        else
        {
            info("dec ch:%d\n", pvobinding_cfg->windows[i]);
        }
    }
    pvobinding_cfg->win_num = real_win_num;
    sdk_preview_t preview2;
    memset(&preview2, 0, sizeof(sdk_preview_t));
    adapt_param_get_preview_cfg(1, &preview2);
    pn2_lock();
    ret = adapt_codec_vo_binding(pmsg, second_fb);

    if (preview2.enable == 0 && pmsg->chann == 1)	/* 清除副屏 */
    {
        info(">>>>>>>> pn2_clear(1).\n");
        pn2_clear(1);
        pvobinding_cfg->mode    = 0;
        pvobinding_cfg->win_num = 0;
    }
    else if (1 == is_preview)	/* 预览 */
    {
        info(">>>>>>>> pn2_refresh(%d). Preview mode:%d\n", pmsg->chann, pvobinding_cfg->mode);
        pn2_refresh(pmsg->chann, pvobinding_cfg);
    }
    else if (0 == is_preview) 	/* 回放 */
    {
        info("Playback mode!\n");
        pn2_clear(0);
        pn2_clear(1);
        warning("CLEAR END!\n");
        pb_msg_vo_binding(pvobinding_cfg);
    }
    else
    {
        warning("\n\nNothing done!!!\n\n");
    }
    pn2_unlock();
#endif

#endif

    return ret;
}

static time_t _main_msg_make_time(sdk_time_t *sdk_time)
{
    if(sdk_time == NULL)
    {
        return 0;
    }
    return os_make_time(sdk_time->year
                        , sdk_time->mon
                        , sdk_time->day
                        , sdk_time->hour
                        , sdk_time->min
                        , sdk_time->sec);
}

static int _main_msg_make_someday(sdk_time_t *ptime, uint32_t *pbegin_time, uint32_t *pend_time)
{
    *pbegin_time = os_make_time(ptime->year, ptime->mon, ptime->day, 0, 0, 1);
    *pend_time = os_make_time(ptime->year, ptime->mon, ptime->day, 23, 59, 59);
    return 0;
}

static int _main_msg_local_time(time_t _utc, sdk_time_t *p_time)
{
    os_local_time(_utc
                  , &p_time->year
                  , &p_time->mon
                  , &p_time->day
                  , &p_time->hour
                  , &p_time->min
                  , &p_time->sec
                  , NULL);
    return 0;
}

/***************
				查询当天00:00:00					  查询当天23:59:59
 req time 		:		|-------------------------------------|
 实际播放时间	:
 		case 1	:	|--------------------------------------------------|
 		case 2	:				|----------------|
 		case 3	:										|-----------|
 		case 4	:	|---------|

 ***************/
static int _main_msg_adjust_pb_time(sdk_pb_group_t *p_group)
{
    uint32_t s_time=0, e_time=0;
    _main_msg_make_someday(&g_rec_query_time, &s_time, &e_time);
    /* case 1: */
    if (_main_msg_make_time(&p_group->start_time)<s_time && _main_msg_make_time(&p_group->stop_time)>e_time)
    {
        warning("11111\n");
        _main_msg_local_time(s_time, &p_group->start_time);
        _main_msg_local_time(s_time + 60, &p_group->stop_time);
    }
    /* case 2: */
    else if (_main_msg_make_time(&p_group->start_time)>=s_time && _main_msg_make_time(&p_group->stop_time)<=e_time)
    {
        warning("22222222\n");
        if ((_main_msg_make_time(&p_group->stop_time) - _main_msg_make_time(&p_group->start_time)) > 30*60)
        {
            /* 播放录像段起始时间在查询时间之内，但录像跨度很长，可能有一两个小时(不接视频通道的录像) */
            p_group->stop_time.year = p_group->start_time.year;
            p_group->stop_time.mon = p_group->start_time.mon;
            p_group->stop_time.day = p_group->start_time.day;
            p_group->stop_time.hour = p_group->start_time.hour;
            p_group->stop_time.min = p_group->start_time.min;
            p_group->stop_time.sec = p_group->start_time.sec + 60;
        }
        else
        {
            // We don't need to adjust time!
        }
    }
    /* case 3: */
    else if (_main_msg_make_time(&p_group->start_time)<e_time && _main_msg_make_time(&p_group->stop_time)>e_time)
    {
        warning("33333333333\n");
        if ((_main_msg_make_time(&p_group->stop_time) - _main_msg_make_time(&p_group->start_time)) > 30*60)
        {
            _main_msg_local_time(e_time, &p_group->stop_time);
        }
        else
        {
            // We don't need to adjust time!
        }
    }
    /* case 4: */
    else if (_main_msg_make_time(&p_group->start_time)<s_time && _main_msg_make_time(&p_group->stop_time)>s_time)
    {
        warning("4444444444444\n");
        if ((_main_msg_make_time(&p_group->stop_time) - _main_msg_make_time(&p_group->start_time)) > 30*60)
        {
            _main_msg_local_time(s_time, &p_group->start_time);
        }
        else
        {
            // We don't need to adjust time!
        }
    }
    else
    {
        error("error condition!!!\n");
    }

    return 0;
}

int main_msg_group_new(sdk_msg_t *pmsg)
{
    int i;
    return 0;
}

int main_msg_group_del(sdk_msg_t *pmsg)
{
    return  0;// pb_msg_group_del(pmsg);
}

int main_msg_group_ctl(sdk_msg_t *pmsg)
{
    return  0 ;//pb_msg_group_ctl(pmsg);
}

int main_msg_motion_cfg(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
    int ch     = pmsg->chann;
    int ret = 0;
    int i = 0;
#if 1
    int max_ch = adapt_param_get_ana_chan_num();
	sdk_motion_cfg_t *motion_cfg = (sdk_motion_cfg_t*)pmsg->data;

   // sdk_motion_cfg_v2_t *motion_cfg = (sdk_motion_cfg_v2_t*)pmsg->data;

    if(is_set == SDK_PARAM_SET)
    {
        if (CHANN_TYPE_ALL == ch)
        {
            for (i=0; i<max_ch; i++)
            {
#if 0
				pmsg->chann = i;
               ret = adapt_av_motion_cfg(pmsg); //for pir
                if(ret < 0)
                {
                    error("adapt_codec_vda_cfg error!\n");
                    pmsg->size = 0;
                    pmsg->ack = SDK_ERR_SET_MD_PARAM;
                    return -1;
                }
#endif				
                adapt_param_set_motion_cfg(i, motion_cfg);
            }
        }
        else
        {
#if 0
            ret = adapt_av_motion_cfg(pmsg);//for pir
            if(ret < 0)
            {
                error("adapt_codec_vda_cfg error!\n");
                pmsg->size = 0;
                pmsg->ack = SDK_ERR_SET_MD_PARAM;
                return -1;
            }
#endif
            adapt_param_set_motion_cfg(ch, motion_cfg);
        }
        pmsg->size = 0;
        WRITE_LOG(LOG_MAJOR_PARAM, L_PARAM_MINOR_PARAM_MOTION, NO_CHAN_ITEM, NO_ALMIN_ITEM);
    }
    else
    {
        adapt_param_get_motion_cfg(ch, motion_cfg);
        pmsg->size = sizeof(sdk_motion_cfg_t);
    }
#endif
    return 0;
}

int main_msg_alarm_in_cfg(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
    int ch     = pmsg->chann;
    int i = 0;
    int max_ch = adapt_param_get_ana_chan_num();
    sdk_alarm_in_cfg_t *alarm_in_cfg = (sdk_alarm_in_cfg_t *)pmsg->data;

    if(is_set == SDK_PARAM_SET)
    {
        if (CHANN_TYPE_ALL == ch)
        {
            for (i=0; i<max_ch; i++)
            {
                adapt_param_set_alarm_in_cfg(i, alarm_in_cfg);
            }
        }
        else
        {	
        	#if 0
            warning("===> ch:%d, ptz_enable:%d, ptz_type:%d, ptz_param:%d\n",
                    ch,
                    alarm_in_cfg->alarm_handle.ptz_enable,
                    alarm_in_cfg->alarm_handle.ptz_type,
                    alarm_in_cfg->alarm_handle.ptz_param);
			#endif
            adapt_param_set_alarm_in_cfg(ch, alarm_in_cfg);
        }
        pmsg->size = 0;
        WRITE_LOG(LOG_MAJOR_PARAM, L_PARAM_MINOR_PARAM_ALARMIN, NO_CHAN_ITEM, NO_ALMIN_ITEM);
    }
    else
    {
        adapt_param_get_alarm_in_cfg(ch, alarm_in_cfg);
		#if 0
        warning("===> ch:%d, ptz_enable:%d, ptz_type:%d, ptz_param:%d\n",
                ch,
                alarm_in_cfg->alarm_handle.ptz_enable,
                alarm_in_cfg->alarm_handle.ptz_type,
                alarm_in_cfg->alarm_handle.ptz_param);
		#endif
        pmsg->size = sizeof(sdk_alarm_in_cfg_t);
    }
    return 0;

}

int main_msg_hide_cfg(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
    int ch     = pmsg->chann;
    int i = 0;
    int max_ch = adapt_param_get_ana_chan_num();
    sdk_hide_cfg_t *hide_cfg = (sdk_hide_cfg_t*)pmsg->data;

    if(is_set == SDK_PARAM_SET)
    {
        if (CHANN_TYPE_ALL == ch)
        {
            for (i=0; i<max_ch; i++)
            {
                adapt_param_set_hide_cfg(i, hide_cfg);
            }
        }
        else
        {
            adapt_param_set_hide_cfg(ch, hide_cfg);
        }

        pmsg->size = 0;
    }
    else
    {
        adapt_param_get_hide_cfg(ch, hide_cfg);
        pmsg->size = sizeof(sdk_hide_cfg_t);
    }

    return 0;
}

int main_msg_overlay_cfg(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
    int ch     = pmsg->chann;
    int ret = 0;
    int i = 0;
    int max_ch = adapt_param_get_ana_chan_num();
    sdk_overlay_cfg_t *poverlay_cfg = (sdk_overlay_cfg_t*)pmsg->data;
#if 0
    if(is_set == SDK_PARAM_SET)
    {
        if (CHANN_TYPE_ALL == ch)
        {
            for (i=0; i<max_ch; i++)
            {
                pmsg->chann = i;
                ret = adapt_codec_overlay_cfg(pmsg);
                if(ret < 0)
                {
                    error("adapt_codec_overlay_cfg error!\n");
                    pmsg->size = 0;
                    pmsg->ack = SDK_ERR_SET_OVERLAY_PARAM;
                    return -1;
                }
                adapt_param_set_overlay_cfg(i, poverlay_cfg);
            }
        }
        else
        {
            ret = adapt_codec_overlay_cfg(pmsg);
            if(ret < 0)
            {
                error("adapt_codec_overlay_cfg error!\n");
                pmsg->size = 0;
                pmsg->ack = SDK_ERR_SET_OVERLAY_PARAM;
                return -1;
            }
            adapt_param_set_overlay_cfg(ch, poverlay_cfg);
        }
        pmsg->size = 0;
        WRITE_LOG(LOG_MAJOR_PARAM, L_PARAM_MINOR_PARAM_OVERLAY, NO_CHAN_ITEM, NO_ALMIN_ITEM);
    }
    else
    {
        adapt_param_get_overlay_cfg(ch, poverlay_cfg);
        pmsg->size = sizeof(sdk_overlay_cfg_t);
    }
#endif

    return 0;
}

int main_msg_lost_cfg(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
    int ch     = pmsg->chann;
    int i = 0;
    int max_ch = adapt_param_get_ana_chan_num();
    sdk_lost_cfg_t *lost_cfg = (sdk_lost_cfg_t*)pmsg->data;

    if(is_set == SDK_PARAM_SET)
    {
        if (CHANN_TYPE_ALL == ch)
        {
            for (i=0; i<max_ch; i++)
            {
                adapt_param_set_lost_cfg(i, lost_cfg);
            }
        }
        else
        {
            adapt_param_set_lost_cfg(ch, lost_cfg);
        }
        pmsg->size = 0;
        WRITE_LOG(LOG_MAJOR_PARAM, L_PARAM_MINOR_PARAM_VIDEOLOST, NO_CHAN_ITEM, NO_ALMIN_ITEM);
    }
    else
    {
        adapt_param_get_lost_cfg(ch, lost_cfg);
        pmsg->size = sizeof(sdk_lost_cfg_t);
    }
    return 0;
}

int main_msg_osd_cfg(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
    int ch     = pmsg->chann;
    int ret = 0;
    int i = 0;
    int max_ch = adapt_param_get_ana_chan_num();
    sdk_osd_cfg_t *osd_cfg = (sdk_osd_cfg_t*)pmsg->data;
#if 1
    if(is_set == SDK_PARAM_SET)
    {
        if (CHANN_TYPE_ALL == ch)
        {
            for (i=0; i<max_ch; i++)
            {
                pmsg->chann = i;
                ret = adapt_av_osd_cfg(pmsg);
                if(ret < 0)
                {
                    error("adapt_codec_osd_cfg error!\n");
                    pmsg->size = 0;
                    pmsg->ack = SDK_ERR_SET_OSD_PARAM;
                    return -1;
                }
                adapt_param_set_osd_cfg(i, osd_cfg);
            }
        }
        else
        {
            ret = adapt_av_osd_cfg(pmsg);
            if(ret < 0)
            {
                error("adapt_codec_osd_cfg error!\n");
                pmsg->size = 0;
                pmsg->ack = SDK_ERR_SET_OSD_PARAM;
                return -1;
            }
            adapt_param_set_osd_cfg(ch, osd_cfg);
        }
        pmsg->size = 0;
        WRITE_LOG(LOG_MAJOR_PARAM, L_PARAM_MINOR_PARAM_OSD, NO_CHAN_ITEM, NO_ALMIN_ITEM);
    }
    else
    {
        adapt_param_get_osd_cfg(ch, osd_cfg);
        printf("--- chan_name.valid:%d, str=%s, str_len=%d, chan.x=%d, chan.y=%d, time.valid:%d, time.x=%d, time.y=%d\n"
               , osd_cfg->chann_name.valid
               , osd_cfg->chann_name.str
               , osd_cfg->chann_name.str_len
               , osd_cfg->chann_name.pos.x
               , osd_cfg->chann_name.pos.y
               , osd_cfg->time.valid
               , osd_cfg->time.pos.x
               , osd_cfg->time.pos.y);

        pmsg->size = sizeof(sdk_osd_cfg_t);
    }
#endif
    return 0;
}

int main_msg_comm_cfg(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
#if 0
    static int _resolution = -1;
    sdk_comm_cfg_t *comm_cfg = (sdk_comm_cfg_t*)pmsg->data;
    sdk_comm_cfg_t old_cfg;
    memset(&old_cfg, 0, sizeof(old_cfg));
    adapt_param_get_comm_cfg(&old_cfg);
    record_param_t rec_param;
    memset(&rec_param, 0, sizeof(rec_param));
    sdk_record_getRecordParam(&rec_param);
    warning("en_prerecord:%d, en_over:%d\n", rec_param.en_prerecord, rec_param.en_over);

    if(is_set == SDK_PARAM_SET)
    {
        if(comm_cfg->out_device != 1 && comm_cfg->out_device != 2)
        {
            pmsg->size = 0;
            return -1;
        }
        if( comm_cfg->resolution != 0 && comm_cfg->resolution != 1
                &&comm_cfg->resolution != 2 && comm_cfg->resolution != 3)
        {
            pmsg->size = 0;
            return -1;
        }

        if(memcmp(comm_cfg, &old_cfg, sizeof(old_cfg)))
        {
            warning("===== SET =====> out_device:%d, video_mode:%d, resolution:%d, recycle_record:%d, id:%u\n"
                    , comm_cfg->out_device
                    , comm_cfg->video_mode
                    , comm_cfg->resolution
                    , comm_cfg->recycle_record
                    , comm_cfg->dvr_id);

            rec_param.en_over = comm_cfg->recycle_record;
            sdk_record_setRecordParam(&rec_param);

            //初始化分辩率;
            if (_resolution!=comm_cfg->resolution && old_cfg.resolution!=comm_cfg->resolution)
            {
                _resolution = comm_cfg->resolution;
                //adapt_vo_resolution_set(comm_cfg);
            }
            //保存参数到flash;


            adapt_param_set_comm_cfg(comm_cfg);
            pmsg->size = 0;
            dm_msg_send(dm_fd
                        , SDK_PRO_MSG_CHANGE_DVRID
                        , 0
                        , 0
                        , comm_cfg
                        , sizeof(sdk_comm_cfg_t)
                        , 3);
            WRITE_LOG(LOG_MAJOR_PARAM, L_PARAM_MINOR_PARAM_COMM, NO_CHAN_ITEM, NO_ALMIN_ITEM);
        }
    }
    else
    {
        adapt_param_get_comm_cfg(comm_cfg);
        pmsg->size = sizeof(sdk_comm_cfg_t);

        warning("====== GET =====> out_device:%d, video_mode:%d, resolution:%d, recycle_record:%d, id:%d\n"
                , comm_cfg->out_device
                , comm_cfg->video_mode
                , comm_cfg->resolution
                , comm_cfg->recycle_record
                , comm_cfg->dvr_id);
    }
#endif
    return 0;
}

#include <stdio.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


/*unsigned int SetSystemTime(char *dt)
{
	 struct rtc_time tm;
	 struct tm _tm;
	 struct timeval tv;
	 time_t timep;
	 sscanf(dt, "%d-%d-%d %d:%d:%d", &tm.tm_year,
			&tm.tm_mon, &tm.tm_mday,&tm.tm_hour,
			&tm.tm_min, &tm.tm_sec);
	_tm.tm_sec = tm.tm_sec;
	_tm.tm_min = tm.tm_min;
	_tm.tm_hour = tm.tm_hour;
	_tm.tm_mday = tm.tm_mday;
	_tm.tm_mon = tm.tm_mon - 1;
	_tm.tm_year = tm.tm_year - 1900;

	timep = mktime(&_tm);
	tv.tv_sec = timep;
	tv.tv_usec = 0;
	
	if(settimeofday(&tv, (struct timezone *) 0) < 0)
	{
		printf("SetSystemTime:: Set system datatime error!\n");
		return -1;
	}
	else
	{
		///printf("Set system datatime successfully!\n");
		//realtime = timep;
		//save_realtimetofile(realtime);//save real time to file
		return timep;
	}
}*/
extern aws_status aws;
void reflush_timezone(unsigned char zone)
{
	st_refresh_time_zone(zone); //录像库的时区刷新
	adapt_set_time_zone(zone); //刷新OSD 时区
	aws.time_zone = zone; //刷新AWS的时区值 用以加减
}
int main_msg_systime(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
    time_t total_seconds = 0;
    int ret = -1;
    sdk_date_t *systime_cfg = (sdk_date_t*)pmsg->data;
	sdk_date_t sys_time;

    if(is_set == SDK_PARAM_SET)
    {
    	if(systime_cfg->ntp_or_manual == 2) //手动下执行
    	{
        	warning("================== time=>  [%d]%d-%d-%d %d:%d:%d\n", systime_cfg->tzone,
               systime_cfg->_time.year, systime_cfg->_time.mon, systime_cfg->_time.day,
               systime_cfg->_time.hour, systime_cfg->_time.min, systime_cfg->_time.sec);
		
        total_seconds = os_make_time(systime_cfg->_time.year, systime_cfg->_time.mon, systime_cfg->_time.day,
                                     systime_cfg->_time.hour/*systime_cfg->_time.hour -(systime_cfg->tzone -12)*/, systime_cfg->_time.min, systime_cfg->_time.sec);
		printf("total_seconds =%ld\n",total_seconds);
        if (total_seconds <= 0)
        {
            error("os_make_time error!   total_seconds=%u\n", (unsigned int)total_seconds);
            pmsg->size = 0;
            pmsg->ack = SDK_ERR_SET_SYSTIME;
            return -1;
        }
        // 修改时间前写LOG
        WRITE_LOG(LOG_MAJOR_OPERATE, L_OPERATE_MINOR_SETTIME, NO_CHAN_ITEM, NO_ALMIN_ITEM);
        ret = os_set_time(total_seconds);
        if (ret < 0)
        {
            error("os_set_time error!\n");
            pmsg->size = 0;
            return -1;
        }
        pmsg->size = 0;

        //系统时间设置RTC时间；
#if 0
        rtc_time_t rtc_time;
        memset(&rtc_time, 0, sizeof(rtc_time_t));
        rtc_time.year = systime_cfg->year;
        rtc_time.mon  = systime_cfg->mon;
        rtc_time.day  = systime_cfg->day;
        rtc_time.hour = systime_cfg->hour;
        rtc_time.min  = systime_cfg->min;
        rtc_time.sec  = systime_cfg->sec;
        os_local_time(total_seconds, NULL, NULL, NULL, NULL, NULL, NULL, &rtc_time.wday);
        adapt_periph_rtc_set(&rtc_time);
#else
        
        memset(&sys_time, 0, sizeof(sys_time));
        sys_time._time.year = systime_cfg->_time.year;
        sys_time._time.mon  = systime_cfg->_time.mon;
        sys_time._time.day  = systime_cfg->_time.day;
		sys_time._time.hour = systime_cfg->_time.hour - (systime_cfg->tzone - 12);
        sys_time._time.min  = systime_cfg->_time.min;
        sys_time._time.sec  = systime_cfg->_time.sec;
        sys_time.tzone  = systime_cfg->tzone;
		 	
        os_local_time(total_seconds, NULL, NULL, NULL, NULL, NULL, NULL, (unsigned int *)&sys_time.wday);

		}
		else if(1 == systime_cfg->ntp_or_manual)
		{
			sys_time.tzone  = systime_cfg->tzone;
		}
		//这里关键保存时区信息
		adapt_param_set_systime( &sys_time);
		reflush_timezone(sys_time.tzone);
        memset(&sys_time,0,sizeof(sys_time));
		adapt_param_get_systime( &sys_time);
		printf("=============tone=%d %d:%d:%d===================\n",sys_time.tzone,sys_time._time.hour,sys_time._time.min,sys_time._time.sec);
#endif
    }
    else
    {
		sdk_date_t sys_time;
        memset(&sys_time, 0, sizeof(sys_time));
		adapt_param_get_systime( &sys_time);
        //sys_time.tzone  = systime_cfg->tzone;
		systime_cfg->tzone = sys_time.tzone ;
        ret = os_get_time(&(systime_cfg->_time.year), &(systime_cfg->_time.mon), &(systime_cfg->_time.day),
                          (&systime_cfg->_time.hour), (&systime_cfg->_time.min), &(systime_cfg->_time.sec));
        if (ret < 0)
        {
            error("os_get_time error!\n");
            pmsg->size = 0;
            return -1;
        }

        pmsg->size = sizeof(sdk_time_t);
    }
    return 0;
}


/*
 *  _0: debug, _1: ptz/normal, _2: rs232, _3 mcu
 *  -------------------------, _2: mcu;
 *  0: 1
 *
 */
int main_msg_serial_cfg(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
    int ch     = pmsg->chann;	// 串口配置，pmsg->chann代表串口号(0--3)
    int ret = -1, fd = 0;
    sdk_serial_func_cfg_t *pfunc_cfg = (sdk_serial_func_cfg_t*)pmsg->data;
    uint8_t type = pfunc_cfg->type;		// 0:normal, 1:ptz, 2...

    // 配置串口参数时固定只配置串口1
    if (ch != 1)
    {
        error("serial number error!\n");
        pmsg->size = 0;
        pmsg->ack = SDK_ERR_SERIAL_NO;
        return -1;
    }

    char serial_dev[256] = {0};
    sprintf((char*)serial_dev, "/dev/ttyAMA%d", ch);

    printf("-------- ch=%d, type=%u, is_set=%d, serial_dev=%s\n", ch, type, is_set, serial_dev);

    if (is_set == SDK_PARAM_SET)
    {
        if (0 == type)
        {

            fd = open(serial_dev, O_RDWR | O_NOCTTY | O_NDELAY);
            if (fd < 0)
            {
                error("open error!\n");
                pmsg->size = 0;
                pmsg->ack = SDK_ERR_SERIAL_OPEN;
                return -1;
            }

            printf("--- fd=%d, baud_rate=%d, parity=%d, stop_bit=%d, data_bit=%d\n",
                   fd,
                   pfunc_cfg->serial_param.baud_rate,
                   pfunc_cfg->serial_param.parity,
                   pfunc_cfg->serial_param.stop_bit,
                   pfunc_cfg->serial_param.data_bit);

            ret = os_set_com_param(fd,
                                   pfunc_cfg->serial_param.baud_rate,
                                   pfunc_cfg->serial_param.parity,
                                   pfunc_cfg->serial_param.stop_bit,
                                   pfunc_cfg->serial_param.data_bit);
            if (ret < 0)
            {
                error("os_set_com_param error!\n");
                pmsg->size = 0;
                pmsg->ack = SDK_ERR_SERIAL_PARAM_SET;
                close(fd);
                return -1;
            }
            close(fd);

            adapt_param_set_serial_cfg(ch, pfunc_cfg);
            pmsg->size = 0;
            WRITE_LOG(LOG_MAJOR_PARAM, L_PARAM_MINOR_PARAM_UART, NO_CHAN_ITEM, NO_ALMIN_ITEM);
        }
        else if (1 == type)			// 云台
        {
            adapt_param_get_serial_cfg(ch, pfunc_cfg);
            pfunc_cfg->type = 1;
            adapt_param_set_serial_cfg(ch, pfunc_cfg);
            pmsg->size = 0;
        }
    }
    else
    {
        adapt_param_get_serial_cfg(ch, pfunc_cfg);
        pmsg->size = sizeof(sdk_serial_func_cfg_t);

        printf("======> baud_rate:%u, data_bit:%u, flow_ctl:%u, parity:%u, stop_bit:%u\n",
               pfunc_cfg->serial_param.baud_rate,
               pfunc_cfg->serial_param.data_bit,
               pfunc_cfg->serial_param.flow_ctl,
               pfunc_cfg->serial_param.parity,
               pfunc_cfg->serial_param.stop_bit);
    }

    return 0;
}

int main_msg_image_attr_cfg(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
    int ch     = pmsg->chann;
    int i = 0;
    int max_ch = adapt_param_get_ana_chan_num();
    int ret = 0;
    sdk_image_attr_t *attr_cfg = (sdk_image_attr_t*)pmsg->data;

    printf("--- is_set=%d, ch=%d\n",
           pmsg->args,
           pmsg->chann);
#if 1
    if(is_set == SDK_PARAM_SET)
    {
        if (CHANN_TYPE_ALL == ch)
        {
            for (i=0; i<max_ch; i++)
            {
                pmsg->chann = i;
				#if 1
                ret = adapt_codec_set_image_attr(pmsg);//这里要改变编码时的图像属性 暂时先不做先保存参数 后续完成在处理
                if(ret < 0)
                {
                    error("adapt_codec_set_image_attr error!\n");
                    pmsg->size = 0;
                    pmsg->ack = SDK_ERR_SET_IMAGE_ATTR;
                    return -1;
                }
				#endif
                adapt_param_set_image_attr(i, attr_cfg);
            }
        }
        else
        {
        #if 1
            ret = adapt_codec_set_image_attr(pmsg);//这里要改变编码时的图像属性 暂时先不做先保存参数 后续完成在处理
            if(ret < 0)
            {
                error("adapt_codec_set_image_attr error!\n");
                pmsg->size = 0;
                pmsg->ack = SDK_ERR_SET_IMAGE_ATTR;
                return -1;
            }
		#endif
#if 1
            printf("--------- ch=%d, brightness=%d, contrast=%d, hue=%d, saturation=%d, sharpness=%d\n",
                   ch,
                   attr_cfg->brightness,
                   attr_cfg->contrast,
                   attr_cfg->hue,
                   attr_cfg->saturation,
                   attr_cfg->sharpness);
#endif
            adapt_param_set_image_attr(ch, attr_cfg);//写入参数到设备内存里  
        }
        pmsg->size = 0;
        WRITE_LOG(LOG_MAJOR_PARAM, L_PARAM_MINOR_PARAM_PIC_ATTR, NO_CHAN_ITEM, NO_ALMIN_ITEM);
    }
    else
    {
        adapt_param_get_image_attr(ch, attr_cfg);
        pmsg->size = sizeof(sdk_image_attr_t);
#if 0
        printf("--------- ch=%d, brightness=%d, contrast=%d, hue=%d, saturation=%d, sharpness=%d\n",
               ch,
               attr_cfg->brightness,
               attr_cfg->contrast,
               attr_cfg->hue,
               attr_cfg->saturation,
               attr_cfg->sharpness);
#endif
    }
#endif

    return 0;
}
//add by 9527 20139527
int main_msg_i3a_attr_cfg(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
    int ch     = pmsg->chann;
    int i = 0;
    int max_ch = adapt_param_get_ana_chan_num();
    int ret = 0;
#if 0
    sdk_3a_attr_t *p3a_cfg = (sdk_3a_attr_t*)pmsg->data;

    printf("--- is_set=%d, ch=%d\n",
           pmsg->args,
           pmsg->chann);

    if(is_set == SDK_PARAM_SET)
    {
        if (CHANN_TYPE_ALL == ch)
        {
            for (i=0; i<max_ch; i++)
            {
                pmsg->chann = i;
                ret = adapt_codec_set_3a_attr(pmsg);
                if(ret < 0)
                {
                    error("adapt_codec_set_image_attr error!\n");
                    pmsg->size = 0;
                    pmsg->ack = SDK_ERR_SET_3A_ATTR;
                    return -1;
                }
                adapt_param_set_3a_attr(i, p3a_cfg);
            }
        }
        else
        {
            ret = adapt_codec_set_3a_attr(pmsg);
            if(ret < 0)
            {
                error("adapt_codec_set_image_attr error!\n");
                pmsg->size = 0;
                pmsg->ack = SDK_ERR_SET_3A_ATTR;
                return -1;
            }
#if 0
            printf("--------- ch=%d, brightness=%d, contrast=%d, hue=%d, saturation=%d, sharpness=%d\n",
                   ch,
                   attr_cfg->brightness,
                   attr_cfg->contrast,
                   attr_cfg->hue,
                   attr_cfg->saturation,
                   attr_cfg->sharpness);
#endif
            adapt_param_set_3a_attr(ch, p3a_cfg);
        }
        pmsg->size = 0;
        WRITE_LOG(LOG_MAJOR_PARAM, L_PARAM_MINOR_PARAM_3A_ATTR, NO_CHAN_ITEM, NO_ALMIN_ITEM);
    }
    else
    {
        adapt_param_get_3a_attr(ch, p3a_cfg);
        pmsg->size = sizeof(sdk_3a_attr_t);
#if 0
        printf("--------- ch=%d, brightness=%d, contrast=%d, hue=%d, saturation=%d, sharpness=%d\n",
               ch,
               attr_cfg->brightness,
               attr_cfg->contrast,
               attr_cfg->hue,
               attr_cfg->saturation,
               attr_cfg->sharpness);
#endif
    }
#endif

    return 0;
}

static int _g_format_percent = 0;

int main_msg_disk(sdk_msg_t *pmsg)
{
    int ret = -1;
    int disk_op = pmsg->args;	// args是命令参数
    int _disk_no = 0;
#if 0
    switch (disk_op)
    {
    case SDK_DISK_QUERY:
    {
        ret = adapt_disk_query(pmsg);
        break;
    }

    case SDK_DISK_FORMAT:	// chan-表示磁盘号
    {
        _disk_no = pmsg->chann;
        ret = adapt_record_query_ref_count(pmsg->chann);
        if (ret > 0)
        {
            error("Disk is using!\n");
            pmsg->size = 0;
            pmsg->ack = SDK_ERR_BUSY;
            return -1;
        }
        else if (ret < 0)
        {
            error("adapt_record_query_ref_count error!\n");
            pmsg->size = 0;
            pmsg->ack = SDK_ERR_NO_DISK;
            return -1;
        }

        if (_g_format_percent>0 && _g_format_percent<100)	// 正在格式化
            return 0;

        adapt_set_fm_progress(0);
        ret = adapt_disk_format(pmsg);
        warning("_disk_no:%d\n", _disk_no);
        if (_disk_no==11 || _disk_no==12)
            WRITE_LOG(LOG_MAJOR_OPERATE, L_OPERATE_MINOR_FORMAT_DISK_U, NO_CHAN_ITEM, NO_ALMIN_ITEM);
        else
            WRITE_LOG(LOG_MAJOR_OPERATE, L_OPERATE_MINOR_FORMAT_DISK, NO_CHAN_ITEM, NO_ALMIN_ITEM);
        break;
    }

    case SDK_DISK_PROGRESS:		// chan-格式化进度
    {
        ret = 0;
        pmsg->size = 0;
        if ((ret=adapt_disk_fm_progress(pmsg->chann)) < 0)
        {
            pmsg->ack = SDK_ERR_DISK_ERR;
            return -1;
        }
        else
        {
            pmsg->chann = _g_format_percent = ret;
        }

        printf("================ percent=%d\n", _g_format_percent);
        break;
    }

    case SDK_DISK_UMOUNT:
    {
        ret = adapt_disk_umount(pmsg);
        if (ret < 0)
        {
            error("adapt_disk_umount error!\n");
            pmsg->size = 0;
            pmsg->ack = SDK_ERR_UMOUNT;
            return -1;
        }

        break;
    }

    default:
        error("No operation!\n");
        break;
    }
#endif
    return ret;
}

int main_msg_tv_cfg(sdk_msg_t *pmsg)
{
    return 0;
}

int main_msg_notify(sdk_msg_t *pmsg)
{
    // 主控主动通知GUI报警信息命令
    return 0;
}

int main_msg_manual_rec(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
    int ch = pmsg->chann;
    int _ch = 0;
    int max_ch = adapt_param_get_ana_chan_num();
    sdk_manual_record_t *pm_rec_cfg = (sdk_manual_record_t*)pmsg->data;
    sdk_manual_record_t old_rec;
    memset(&old_rec, 0, sizeof(old_rec));
    adapt_param_get_manual_rec_cfg(&old_rec);

    if(is_set == SDK_PARAM_SET)
    {
        adapt_param_set_manual_rec_cfg(pm_rec_cfg);

        if (CHANN_TYPE_ALL == ch)
        {
            if (pm_rec_cfg->manual_record[0])
            {
                warning("111111111111 L_OPERATE_MINOR_MANUAL_RECORD_START\n");
                //WRITE_LOG(LOG_MAJOR_OPERATE, L_OPERATE_MINOR_MANUAL_RECORD_START, CHANN_TYPE_ALL, NO_ALMIN_ITEM);
            }
        }
        else
        {
            for (_ch=0; _ch<max_ch; _ch++)
            {
                if (pm_rec_cfg->manual_record[_ch]==1 && pm_rec_cfg->manual_record[_ch]^old_rec.manual_record[_ch])
                {
                    warning("33333333 L_OPERATE_MINOR_MANUAL_RECORD_START =========== _ch:%d\n", _ch);
                    //WRITE_LOG(LOG_MAJOR_OPERATE, L_OPERATE_MINOR_MANUAL_RECORD_START, _ch, NO_ALMIN_ITEM);
                }
            }
        }
        pmsg->size = 0;
    }
    else
    {
        adapt_param_get_manual_rec_cfg(pm_rec_cfg);
        pmsg->size = sizeof(sdk_manual_record_t);
    }

    return 0;
}

int main_msg_manual_almin(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
    sdk_manual_alarmin_t *p_almin_cfg = (sdk_manual_alarmin_t*)pmsg->data;

    if(is_set == SDK_PARAM_SET)
    {
        adapt_param_set_manual_almin_cfg(p_almin_cfg);
        pmsg->size = 0;
    }
    else
    {
        adapt_param_get_manual_almin_cfg(p_almin_cfg);
        pmsg->size = sizeof(sdk_manual_alarmin_t);
    }

    return 0;
}

int main_msg_manual_almout(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
    int index;
    sdk_manual_alarmout_t *p_almout_cfg = (sdk_manual_alarmout_t*)pmsg->data;
#if 0
    if(is_set == SDK_PARAM_SET)
    {
        //for (index=0; index<MAX_ALARM_OUT_NUM; index++)
        for (index=0; index<4; index++)
        {
            info("========> alarm_out[%d]:%d\n", index, p_almout_cfg->enable_alarmout[index]);
            if (0 == p_almout_cfg->enable_alarmout[index])
            {
                adapt_periph_alarm_out_manual(index, 0x0);
            }
            else if (1 == p_almout_cfg->enable_alarmout[index])
            {
                adapt_periph_alarm_out_manual(index, 0x1);
            }
        }

        adapt_param_set_manual_almout_cfg(p_almout_cfg);
        pmsg->size = 0;
    }
    else
    {
        adapt_param_get_manual_almout_cfg(p_almout_cfg);
        pmsg->size = sizeof(sdk_manual_alarmout_t);
    }
#endif

    return 0;
}

// pmsg->args: args是命令参数
// pmsg->chann: chann是通道号
// 目前默认同时只能接一个备份盘
int main_msg_record_backup(sdk_msg_t *pmsg)
{
    int per = 0;
    int ret = 0;
    int backup_op = pmsg->args;


    return 0;
}

int main_msg_close_system(sdk_msg_t *pmsg)
{

    warning("===> main_msg_close_system.\n");
    pmsg->msg_id = SDK_MAIN_MSG_CLOSE_SYSTEM;
    pmsg->args = SDK_CLOSE_SYS_REBOOT;
    int oper = pmsg->args;

    switch (oper)
    {
    case SDK_CLOSE_SYS_SHUTDOWN:
    {

        break;
    }

    case SDK_CLOSE_SYS_REBOOT:
    {

        WRITE_LOG(LOG_MAJOR_SYSTEM, L_SYSTEM_MINOR_REBOOT, NO_CHAN_ITEM, NO_ALMIN_ITEM);
        sdk_log_flush();
        main_free_system_resources();
        sync();
        sleep(1);
        system("reboot");
        break;
    }

    case SDK_COLSE_SYS_LOGOUT:
    {
        // ...
        break;
    }

    default:
    {
        error("NO operation!\n");
        return -1;
    }
    }
    return 0;
}

int main_msg_audio_control(sdk_msg_t *pmsg)
{
#if 0
    // 进入回放界面时不主动唤醒硬盘
    if (pmsg->args==SDK_AUDIO_CONTROL_SPECIAL_CMD && pmsg->extend==1)
    {
        adapt_record_active_disk();
    }
#endif
    return  0;//adapt_codec_audio_control(pmsg);
}


int alarm_snap_control(sdk_msg_t *pmsg)
{
    char    filename[40] = {0};
    char path[128]= {0};
    char cmd_buf[128] = {0};
    FILE    *snapshot_fd = NULL;

    int filecount = 0;
    unsigned int year,mon,day,hour,min,sec,weekday;

    sdk_snap_info_t snap_info;
    snap_info.data = NULL;
    snap_info.width = 1920;
    snap_info.height = 1080;
    snap_info.data = (char *)malloc(snap_info.width * snap_info.height*2);
    snap_info.buffer_max_len = snap_info.width * snap_info.height*2;
    if(!snap_info.data)
        return -1;

    memset(snap_info.data,0,snap_info.buffer_max_len);


    os_get_time(&year, &mon, &day,&hour, &min, &sec);
    sprintf(filename, "warning_%02d-%02d-%02d.jpg"
            , hour
            , min
            , sec);

    sprintf(path, "/var/run/%s", filename);
	
    if(!adapt_av_snap(0,&snap_info,path))//截图没有添加参数  截图的最终路径存在问题
    {	
    }
    free(snap_info.data);
    if(0 == adapt_baidu_upload_file("/var/run/",filename,WARNING_DIR))
    {
        info("send %s to baidu success!!! \n",path);
    }
    else
    {
        error("send %s to baidu fail!!! \n",path);
    }
    sprintf(cmd_buf," rm -Rf %s",path);
    system(cmd_buf);
	system("rm -Rf  ch-* ");

    return  0;//adapt_codec_audio_control(pmsg);
}



int main_msg_snap_control(sdk_msg_t *pmsg)
{
    char    filename[40] = {0};
    char path[128]= {0};
    char cmd_buf[128] = {0};
    FILE    *snapshot_fd = NULL;

    int filecount = 0;
    unsigned int year,mon,day,hour,min,sec,weekday;

    sdk_snap_info_t snap_info;
    snap_info.data = NULL;
    snap_info.width = 1920;
    snap_info.height = 1080;
    snap_info.data = (char *)malloc(snap_info.width * snap_info.height*2);
    snap_info.buffer_max_len = snap_info.width * snap_info.height*2;
    if(!snap_info.data)
        return -1;

    memset(snap_info.data,0,snap_info.buffer_max_len);


    os_get_time(&year, &mon, &day,&hour, &min, &sec);
    sprintf(filename, "snapshot_%02d-%02d-%02d.jpg"
            , hour
            , min
            , sec);

    sprintf(path, "/var/run/%s", filename);
	
    if(!adapt_av_snap(0,&snap_info,path))//截图没有添加参数  截图的最终路径存在问题
    {	
#if 0    //暂时屏蔽  优化参数之后再行打开
info("+_+_+_+_+_+_+_+   adapt_av_snap  _+_+_+_+_+_+_+_+_\n");
        os_get_time(&year, &mon, &day,&hour, &min, &sec);
        sprintf(filename, "snapshot_%02d-%02d-%02d.jpg"
                , hour
                , min
                , sec);

        sprintf(path, "/var/run/%s", filename);
		info(" +_+_+_++_+_+_+_  path %s +_+_+_+_+_+_+_+_+_+ \n",path);
        snapshot_fd = fopen(path, "wb");
        if (snapshot_fd == NULL)
        {
            printf("Fail to open file %s\n", filename);
            exit(1);
        }
        printf("snap ok file name:%s ,data:%p len:%d  \n",filename,snap_info.data,snap_info.data_len);
        fwrite(snap_info.data, 1, snap_info.data_len, snapshot_fd);
        fclose(snapshot_fd);*/
#endif
    }
    free(snap_info.data);
    if(0 == adapt_baidu_upload_file("/var/run/",filename,ROOT_DIR))
    {
        info("send %s to baidu success!!! \n",path);
    }
    else
    {
        error("send %s to baidu fail!!! \n",path);
    }
    sprintf(cmd_buf," rm -Rf %s",path);
    system(cmd_buf);
	system("rm -Rf  ch-* ");

    return  0;//adapt_codec_audio_control(pmsg);
}

int main_msg_ddns_cfg(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
    sdk_ddns_cfg_t *pddns_cfg = (sdk_ddns_cfg_t*)pmsg->data;

    if(is_set == SDK_PARAM_SET)
    {
        adapt_param_set_ddns_cfg(pddns_cfg);
        pmsg->size = 0;
    }
    else
    {
        adapt_param_get_ddns_cfg(pddns_cfg);
        pmsg->size = sizeof(sdk_ddns_cfg_t);
    }

    return  0;//dns_cfg_process(pmsg);
}

int main_msg_capability(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;

    if(is_set == SDK_PARAM_SET)
    {
        ;
    }
    else
    {
        ///adapt_codec_capability_info(pmsg);
        pmsg->size = sizeof(sdk_device_capability_t);
    }
    return 0;
}

static int _net_link_eth(sdk_msg_t *pmsg)
{
    int ret = 0;
    int is_set = pmsg->args;
    sdk_eth_cfg_t *p_eth_cfg = (sdk_eth_cfg_t*)pmsg->data;

    if(is_set == SDK_PARAM_SET)
    {
        warning("111 SET if_idx =%d, ip:%s, mask:%s, gateway:%s, mac:%s\n"
                , pmsg->chann
                , p_eth_cfg->ip_info.ip_addr
                , p_eth_cfg->ip_info.mask
                , p_eth_cfg->ip_info.gateway
                , p_eth_cfg->ip_info.mac);


        if(adapt_os_set_eth_cfg(p_eth_cfg, pmsg->chann) < 0)
        {
            pmsg->ack = SDK_ERR_NET_CFG;
            pmsg->size = 0;
            return -1;
        }
        adapt_param_set_eth_cfg(p_eth_cfg, pmsg->chann);
        pmsg->size = 0;
    }
    else
    {
        adapt_param_get_eth_cfg(p_eth_cfg, pmsg->chann);
        adapt_os_get_eth_cfg(p_eth_cfg, pmsg->chann);
        pmsg->size = sizeof(sdk_eth_cfg_t);

        warning("222 GET if_idx = %d, ip:%s, mask:%s, gateway:%s, mac:%s\n"
                , pmsg->chann
                , p_eth_cfg->ip_info.ip_addr
                , p_eth_cfg->ip_info.mask
                , p_eth_cfg->ip_info.gateway
                , p_eth_cfg->ip_info.mac);
    }

    return 0;
}

static int _net_link_pppoe(sdk_msg_t *pmsg)
{
    int ret = 0;
    int is_set = pmsg->args;
    sdk_pppoe_t *p_pppoe_cfg = (sdk_pppoe_t*)pmsg->data;

    if(is_set == SDK_PARAM_SET)
    {
        warning("111 SET enable:%d, pass:%s, user:%s\n"
                , p_pppoe_cfg->enable
                , p_pppoe_cfg->pass
                , p_pppoe_cfg->user);

        adapt_param_set_pppoe_cfg(p_pppoe_cfg, pmsg->chann);
        ///pppoe_cfg_process(pmsg);
        pmsg->size = 0;
    }
    else
    {
        adapt_param_get_pppoe_cfg(p_pppoe_cfg, pmsg->chann);
        //此处获取pppoe ip 本应有 pppoe_cfg_process 实现;
        ///pppoe_cfg_process(pmsg);
        adapt_os_get_ip_info(p_pppoe_cfg, pmsg->chann);
        pmsg->size = sizeof(sdk_pppoe_t);
    }

    return 0;
}


int main_msg_net_link_cfg(sdk_msg_t *pmsg)
{
    int ret = 0;
    int is_set = pmsg->args;
    sdk_eth_cfg_t *net_param = (sdk_eth_cfg_t*)pmsg->data;

    if(is_set == SDK_PARAM_SET)
    {
        adapt_set_net_params(net_param);
    }
    else
    {
        adapt_get_net_params(net_param);
		 pmsg->size = sizeof(sdk_eth_cfg_t);
    }

    return ret;
}

int main_msg_upnp_cfg(sdk_msg_t *pmsg)
{
    return 0;
}


int main_msg_devlist(sdk_msg_t *pmsg)
{
    return 0;
}


int main_msg_ch_ip(sdk_msg_t *pmsg)
{
    return 0;
}
/*add  by   zw  for baidu    20150316*/
int main_alarm_sched_cfg(sdk_msg_t *pmsg)
{
	#if 1
	int is_set = pmsg->args;
    sdk_sched_time_t *sched_time_param = (sdk_sched_time_t*)pmsg->data;

    if(is_set == SDK_PARAM_SET)
    {
        adapt_param_set_sched_param(sched_time_param);
        pmsg->size = 0;
    }
    else
    {
        adapt_param_get_sched_param(sched_time_param);
        pmsg->size = sizeof(sdk_sched_time_t);
    }
#endif
    return  0;
}
//SDK_MAIN_MSG_REGIONAL_CFG
int main_regional_cfg(sdk_msg_t *pmsg)
{
#if 1
	int is_set = pmsg->args;
    sdk_regional_param_t *regional_param = (sdk_regional_param_t*)pmsg->data;

    if(is_set == SDK_PARAM_SET)
    {
        adapt_param_set_regional_param(regional_param);
        pmsg->size = 0;
    }
    else
    {
        adapt_param_get_regional_param(regional_param);
        pmsg->size = sizeof(sdk_regional_param_t);
    }
#endif
    return  0;

}
//sdk_iodev_param_t
int main_iodev_cfg(sdk_msg_t *pmsg)
{
#if 1
	int is_set = pmsg->args;
    sdk_iodev_param_t *iodev_param = (sdk_iodev_param_t*)pmsg->data;
//在这里还要继续处理   现在直接使用了adapt的底层参数  还需要同步sdk发过来的信息  所以在另外一个函数然后同步全局变量
    if(is_set == SDK_PARAM_SET)
    {
        adapt_param_set_iodev_param(iodev_param);
        pmsg->size = 0;
    }
    else
    {
        adapt_param_get_iodev_param(iodev_param);
        pmsg->size = sizeof(sdk_iodev_param_t);
    }
#endif
    return  0;
}
//sdk_audio_detect_cfg_t *audio_detect_cfg
int main_audio_detect_cfg(sdk_msg_t *pmsg)
{
#if 1
	int is_set = pmsg->args;
    sdk_audio_detect_cfg_t *audio_detect_cfg = (sdk_audio_detect_cfg_t*)pmsg->data;
//在这里还要继续处理   现在直接使用了adapt的底层参数  还需要同步sdk发过来的信息  所以在另外一个函数然后同步全局变量
    if(is_set == SDK_PARAM_SET)
    {
    	adapt_audio_detect_cfg(pmsg);//同步数据到安霸code
        adapt_param_set_audio_detect_param(audio_detect_cfg);
		//adapt_amba_xxx  设置参数
        pmsg->size = 0;
    }
    else
    {
        adapt_param_get_audio_detect_param(audio_detect_cfg);
        pmsg->size = sizeof(sdk_audio_detect_cfg_t);
    }
#endif
    return  0;
}

int main_mirror_flip_cfg(sdk_msg_t *pmsg)
{
#if 1
	int is_set = pmsg->args;
    sdk_mirror_flip_cfg_t *mirror_param = (sdk_mirror_flip_cfg_t*)pmsg->data;
    if(is_set == SDK_PARAM_SET)
    {
    	adapt_mirror_flip_cfg(pmsg);//同步数据到编码库
        adapt_param_set_mirror_param_param(mirror_param);
        pmsg->size = 0;
    }
    else
    {
        adapt_param_get_mirror_param_param(mirror_param);
        pmsg->size = sizeof(sdk_mirror_flip_cfg_t);
    }
#endif
    return  0;
}

int main_power_freq_cfg(sdk_msg_t *pmsg)
{
#if 1
	int is_set = pmsg->args;
    sdk_power_freq_cfg_t *power_freq = (sdk_power_freq_cfg_t*)pmsg->data;
    if(is_set == SDK_PARAM_SET)
    {
    	adapt_power_frequency_cfg(pmsg);
        adapt_param_set_power_freq_param(power_freq);
        pmsg->size = 0;
    }
    else
    {
        adapt_param_get_power_freq_param(power_freq);
        pmsg->size = sizeof(sdk_power_freq_cfg_t);
    }
#endif
    return  0;
}


int msg_ptz_dispatch(sdk_msg_t *pmsg)
{
	return 0;
}


int  main_msg_search_wifilist(sdk_msg_t *pmsg)
{
    int is_set = pmsg->args;
    sdk_wifi_search_t *ap_list =(sdk_wifi_search_t*)pmsg->data;
   // if(is_set == SDK_PARAM_SET)
   // {
	adapt_search_ap(ap_list);
        //sdk_get_wifi_list(ap_list);//暂时用这个实现功能 
        pmsg->size = sizeof(sdk_wifi_search_t);
	printf("ap_list->ap_count   is %d  \n",ap_list->ap_count);
	printf("ap_list->ap_info[1].ssid   is %s\n",ap_list->ap_info[1].ssid);
	printf("ap_list->ap_info[1].auth_mode   is %d\n",ap_list->ap_info[1].auth_mode);				
   // }
    return  0;
}


int main_msg_aws_cfg(sdk_msg_t *pmsg)
{
    int ret = 0;
    int is_set = pmsg->args;
    int ch     = pmsg->chann;
    int i = 0;
    int max_ch = adapt_param_get_ana_chan_num();
    sdk_aws_info_t *aws_cfg = (sdk_aws_info_t *)pmsg->data;
    if(is_set == SDK_PARAM_SET)
    {
    	printf("main_msg_aws_cfg %d\n",aws_cfg->aws_record_status);
    	ret = adapt_param_set_aws_cfg(aws_cfg);
    
        pmsg->size = 0;
        WRITE_LOG(LOG_MAJOR_PARAM, L_PARAM_MINOR_PARAM_COMM, NO_CHAN_ITEM, NO_ALMIN_ITEM);
	}
    else
    {
        ret = adapt_param_get_aws_cfg(aws_cfg);
        pmsg->size = sizeof(sdk_aws_info_t);
    }
   return ret;
}


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

#define  USER_IPCAM_REBOOT_SYSTEM_REQ 	 0x40019	//重启系统
#define  USER_IPCAM_WIRTE_DEV_CONFIG_REQ	 0X40045	//写入设备的配置信息  (mac地址 serid等需要永久保存不删除的)
#define  USER_IPCAM_WIRTE_DEV_CONFIG_RESP 0X40046
#define  USER_IPCAM_READ_DEV_CONFIG_REQ		0X40047	//读取设备的配置信息
#define  USER_IPCAM_READ_DEV_CONFIG_RESP		0X40048

sdk_msg_dispatch_cb g_local_msg_cb;
#define MSG_BUF_SIZE 1024

#define LOCAL_MSG_CTRL(_id, _arg, _ext, _ch, pmsg) ({\
				int _ret = -1;\
				pmsg->msg_id	= _id;\
				pmsg->msg_dir 	= SDK_MSG_REQ;\
				pmsg->chann 	= _ch;\
				pmsg->args		= _arg;\
				pmsg->extend	= _ext;\
				if(g_local_msg_cb)\
				_ret = g_local_msg_cb(pmsg);\
				if(_ret != 0)\
				goto __error;})

typedef struct
{
	char 	szMacAddr[24];
	char 	szUID[32];//百度使用的设备ID  
	char 	szPwd[32];//百度使用时pwd暂时不用 全部写
} AVIoctrlWriteDevConfigReq, AVIoctrlReadDevConfigResp;
				

int avRecvDefaultIOCtrl(int fd_socket,unsigned int * pnIOCtrlType,char * abIOCtrlData,int nIOCtrlMaxDataSize,unsigned int nTimeout)
{
	int ret;
	if( fd_socket < 0)
	{
	    printf(">>>>>>>>>avSendDefaultIOCtl>>>>>>>fd>>failed\n");
		return -1;
	}
	
	ret = hi_tcp_block_recv(fd_socket,pnIOCtrlType,sizeof(int));
	if(ret < 0)
	{
	    printf(">>>>>>>>>avSendDefaultIOCtl>>>>>>>fd>>failed\n");
		return -1;
	}
	
	ret = hi_tcp_block_recv(fd_socket,abIOCtrlData,nIOCtrlMaxDataSize);
	if(ret < 0)
	{
	    printf(">>>>>>>>>avSendDefaultIOCtl>>>>>>>fd>>failed\n");
		return -1;
	}
	

	return ret;

}

void DE_get_dev_cfg_req_handle(int avIndex, char *buf)
{
    pthread_mutex_t get_lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_init(&get_lock,NULL);

    AVIoctrlReadDevConfigResp resp;
    char msg_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;


    LOCAL_MSG_CTRL(SDK_MAIN_MSG_NET_LINK_CFG,SDK_PARAM_GET,0,0,pMsg);//获取MAC 地址

    if(pMsg == NULL)
    {
        goto __error;
    }

    sdk_eth_cfg_t * get_mac_msg = (sdk_eth_cfg_t*)pMsg->data;

    strncpy(resp.szMacAddr, get_mac_msg->ip_info.mac, 20);

    pthread_mutex_lock(&get_lock);
    //strcpy(resp.szUID, g_tutk_cfg.guid);
    //strcpy(resp.szPwd, g_tutk_cfg.ViewPassword);

    pthread_mutex_unlock(&get_lock);
    avSendDefaultIOCtrl(avIndex, USER_IPCAM_READ_DEV_CONFIG_RESP, (char *)&resp, sizeof(AVIoctrlReadDevConfigResp));
    return ;
__error:

    warning("===>get MAC UID error!!\n");
    return ;
}
int avSendDefaultIOCtrl(int fd_socket,unsigned int nIOCtrlType, const char *cabIOCtrlData, int nIOCtrlDataSize)
{
	int ret;
	if( fd_socket < 0)
	{
	    printf(">>>>>>>>>avSendDefaultIOCtl>>>>>>>fd>>failed\n");
		return -1;
	}

	ret = hi_tcp_block_send(fd_socket,cabIOCtrlData,nIOCtrlDataSize);
	if(ret < 0)
	{
	    printf(">>>>>>>>>avSendDefaultIOCtl>>>>>>>fd>>failed\n");
		return -1;
	}

	return 1;
}

int De_get_device_info_handle(void *response)
{
	char msg_buf[MSG_BUF_SIZE] = {0};

	memset(msg_buf,0x0,MSG_BUF_SIZE);

	SDK_MSG_DEVICEINFO *resp = (SDK_MSG_DEVICEINFO *)response;
	pthread_mutex_t get_lock = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_init(&get_lock,NULL);		
	
	memset(msg_buf,0x0,MSG_BUF_SIZE);
	sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
	
	LOCAL_MSG_CTRL(SDK_MAIN_MSG_SYS_CFG,SDK_PARAM_GET,0,0,pMsg);
	if(NULL == pMsg)
	{
	   goto __error;
	}
	sdk_sys_cfg_t *version_msg_cfg	= (sdk_sys_cfg_t *)pMsg->data;
	strncpy(resp->hardware_ver, version_msg_cfg->hardware_ver,MAX_MANU_STR_LEN);
	memset(msg_buf,0x0,MSG_BUF_SIZE);
	
	LOCAL_MSG_CTRL(SDK_MAIN_MSG_NET_LINK_CFG,SDK_PARAM_GET,0,0,pMsg);//获取MAC 地址
	
	if(pMsg == NULL)
	{
	    goto __error;
	}
	
	sdk_eth_cfg_t * get_mac_msg = (sdk_eth_cfg_t*)pMsg->data;
	
	strncpy(resp->szMacAddr, get_mac_msg->ip_info.mac, 20);
	pthread_mutex_lock(&get_lock);
	//strcpy(resp->szUID, g_tutk_cfg.guid);
	//strcpy(resp->szPwd, g_tutk_cfg.ViewPassword);
    return 1;
__error:
	 return -1;

}

void DE_set_dev_cfg_req_handle(int avIndex, char *buf)
{
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_init(&mutex,NULL);

    AVIoctrlReadDevConfigResp resp;
    AVIoctrlReadDevConfigResp *p = (AVIoctrlReadDevConfigResp *)buf;
    if(NULL == p)
    {
        goto __error;
    }
	sdk_sys_cfg_t sys_cfg;
	sdk_user_right_t right;
    char msg_buf[MSG_BUF_SIZE]= {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;

    pthread_mutex_lock(&mutex);
	
    memset(&resp, 0, sizeof(resp));
    //strncpy(g_tutk_cfg.guid, p->szUID, 32); //设置UID
    //strncpy(g_tutk_cfg.ViewPassword, p->szPwd, 32);
	
	//info(">>>>>>DE_set_dev_cfg_req_handle>>>UID:%s   PWD:%s,   MAC=%s\n", g_tutk_cfg.guid, g_tutk_cfg.ViewPassword,p->szMacAddr);
	//先读
	LOCAL_MSG_CTRL(SDK_MAIN_MSG_SYS_CFG,SDK_PARAM_GET,0,0,pMsg);
	memcpy(&sys_cfg,pMsg->data,sizeof(sdk_sys_cfg_t));
  	strncpy(sys_cfg.serial_2, p->szUID, 21); //设置UID
  	
	memset(pMsg,0,MSG_BUF_SIZE);
	pMsg->size = sizeof(sdk_msg_t);
	memcpy(pMsg->data, &sys_cfg,sizeof(sdk_sys_cfg_t));
    LOCAL_MSG_CTRL(SDK_MAIN_MSG_SYS_CFG,SDK_PARAM_SET,0,0,pMsg);

	//设置密码  此处为设置ADMIN的密码
	//此处不用先获取  因为下层接口在修改时已获取 
	//然后比较用户名进行修改密码
	memcpy(right.user.user_name,"admin",sizeof("admin"));
	memcpy(right.user.user_pwd,p->szPwd,sizeof(p->szPwd));
	memset(pMsg,0,MSG_BUF_SIZE);
	memcpy(pMsg->data,(unsigned char *)&right,sizeof(sdk_user_right_t));
	pMsg->size = sizeof(sdk_user_right_t);
	printf(">>>>>>DE_set_dev_cfg_req_handle>> usr_name---passwd = %s %s\n",((sdk_user_right_t *)pMsg->data)->user.user_name,
							((sdk_user_right_t *)pMsg->data)->user.user_pwd);
	LOCAL_MSG_CTRL(SDK_MAIN_MSG_USER,SDK_USER_MODI,0,0,pMsg);
	pthread_mutex_unlock(&mutex);
	
	memset(pMsg,0,MSG_BUF_SIZE);
	LOCAL_MSG_CTRL(SDK_MAIN_MSG_NET_LINK_CFG,SDK_PARAM_GET,0,0,pMsg);//获取MAC 地址

	if(pMsg == NULL)
	{
		goto __error;
	}

	sdk_eth_cfg_t * get_mac_msg = (sdk_eth_cfg_t*)pMsg->data;
	strncpy(resp.szMacAddr, get_mac_msg->ip_info.mac, 20);
	pthread_mutex_lock(&mutex);
	//strcpy(resp.szUID, g_tutk_cfg.guid);
	//strcpy(resp.szPwd, g_tutk_cfg.ViewPassword);	
	pthread_mutex_unlock(&mutex);

    avSendDefaultIOCtrl(avIndex, USER_IPCAM_WIRTE_DEV_CONFIG_RESP, (char *)&resp, sizeof(AVIoctrlReadDevConfigResp));
    return;
__error:
    avSendDefaultIOCtrl(avIndex, USER_IPCAM_WIRTE_DEV_CONFIG_RESP, (char *)&resp, sizeof(AVIoctrlReadDevConfigResp));
	return ;
}

void Handle_DefaultServer_Cmd(int SID, int avIndex, char *buf, int type)
{
	printf(">>>sid(%d)avIndex(%d)Handle_IOCTRL_Cmd(0x%X)-- start \n ", SID,avIndex,type);
	switch(type)
	{
	case USER_IPCAM_REBOOT_SYSTEM_REQ: //请求重启系统  = 0X40019
	{
		system("reboot");
	}
	break;
	case USER_IPCAM_WIRTE_DEV_CONFIG_REQ:		//0X40045  写入uid	 mac
	{
		DE_set_dev_cfg_req_handle(SID, buf);
	}
	break;
	case USER_IPCAM_READ_DEV_CONFIG_REQ: 	//	0X40047
	{
		DE_get_dev_cfg_req_handle(SID, buf);
	}
	break;
	default:
		printf("non-handle type[%X]\n", type);
		break;
	}
	printf("sid(%d)avIndex(%d)Handle_IOCTRL_Cmd(0x%X)-- end \n ", SID,avIndex,type);
}

void *thread_ForDefaultServerStart(void *arg)
{
#define MAX_SIZE_IOCTRL_BUF   1024
	char ioCtrlBuf[MAX_SIZE_IOCTRL_BUF]; 
    int  fd = (int )arg;
	int  ioType;
    int  			ret;

    if(fd < 0)
    {
        printf(">>>>>>>>>>>Client is from<<<<<<<<<<exit<<thread_ForDefaultServerStart<<<<<failed\n");
		pthread_exit(pthread_self());
    }

    while(1)
    {
		memset(ioCtrlBuf, 0, MAX_SIZE_IOCTRL_BUF);
		ret = avRecvDefaultIOCtrl(fd, &ioType, (char *)&ioCtrlBuf, MAX_SIZE_IOCTRL_BUF, 1000);
		if(ret > 0)
		{		    
			printf("<++++++++++++++++++++++<fd:%d>>---------\n", fd);
			Handle_DefaultServer_Cmd(fd, 0, ioCtrlBuf, ioType);
		}
		else 
		{
			printf(">>>>>>recv error, code[%d]\n", ret);
			break;
		}
    }
    close(fd);
	return;
}

static void* thread_server_handle(void *arg)
{
#define PORT        8999
    pthread_detach(pthread_self());  //分离线程

	int ret;
	int sockfd, peer, len;	
	fd_set readfd; //读文件描述符集合
    struct timeval timeout;
	struct sockaddr_in serv_addr, clnt_addr;
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);//创建socket
	if (sockfd < 0) {
	    perror("socket error");
	    return -1;
	}
	/*  配置socket */
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(serv_addr.sin_zero), 8);

	/* 绑定socket */
	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0) {
	    perror("bind err");
	    return -1;
	}

	/* 主动socket转为被动监听socket */
	if (listen(sockfd, 1024) < 0) {
	    perror("listen error");
	    return -1;
	}

	/* 循环监听 */
	len = sizeof(clnt_addr);
	while (1) {
		timeout.tv_sec = 100;
        timeout.tv_usec = 0;

        //文件描述符集合清0
        FD_ZERO(&readfd);

        //将套接字描述符加入到文件描述符集合
        FD_SET(sockfd, &readfd);

        //select侦听是否有数据到来
        ret = select(sockfd + 1, &readfd, NULL, NULL, &timeout); //侦听是否可读
        switch (ret)
        {
        case -1: //发生错误
            perror("select error:");
            break;
        case 0: //超时
            printf("select timeout\n");
            break;
        default:
            if (FD_ISSET(sockfd,&readfd))
            {
			    peer = accept(sockfd, (struct sockaddr *)&clnt_addr, &len);/* 接收连接 */
			    if (peer < 0) {
			        perror(">>>>>>>>>>Server: accept failed.\n");
			        continue;
			    }

				pthread_t Thread_ID;
				pthread_create(&Thread_ID, NULL,NULL, thread_ForDefaultServerStart, (void *)peer);
				pthread_detach(Thread_ID);
            }
			break;
        	}
		}
	return 0;
}

int local_server_start(sdk_msg_dispatch_cb msg_cb)
{
    int ret;
	
    //初始化
	if(msg_cb)
		g_local_msg_cb = msg_cb;
	pthread_t   server_pthread_pid;
	ret = pthread_create(&server_pthread_pid,NULL,NULL, thread_server_handle, NULL);//蜂鸣器主动报警检测
    if(ret < 0)
    {
        printf("thread_led_control_handle ret[%d]\n", ret);
        return -1;
    }
    return 0;
}


