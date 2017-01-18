/*
 * =====================================================================================
 *
 *       Filename:  adapt_param.c
 *
 *    Description:  adapt_param
 *
 *        Version:  1.0
 *        Created:  2011年06月13日 19时50分44秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  
 *        Company:  
 *
 * =====================================================================================
 */
 
#include <stdio.h>
#include <unistd.h>
#include "wrapper.h"
#include "log.h"
#include "paramlib.h"
#include "adapt.h"

static int g_param_dev_type = 0;
static param_handle_t  	param_handle = 0;
static param_handle_t	default_param_handle = 0;

/* ----------------------------------------------- */
#define SDK_PARAM_SET_(ch, id, param) do{\
    int size = sizeof(*param);\
    printf("set >>>> ch:%d, id:0x%08x(:%d), size:%d\n", ch, (id << 8)|(ch), (id << 8)|(ch), size);\
    if((param_handle)&&(param))sdk_param_set(param_handle, (id << 8)|(ch), 1, &size, param);}while(0)

#define SDK_PARAM_GET_(ch, id, param) do{\
    int size = sizeof(*param);\
    int size2 = 0;\
    if((param_handle)&&(param))sdk_param_get(param_handle, (id << 8)|(ch), size, &size2, param);\
    /*printf("get ++++ ch:%d, id:0x%08x, size:%d\n", ch, (id << 8)|(ch), size);*/\
    /*if(size2 != size)printf("size:%d != size2:%d\n", size, size2);*/}while(0)


// 设置默认参数，仅提供2个接口(系统、网络)
#define SDK_PARAM_SET_DEF(ch, id, param) do{\
    int size = sizeof(*param);\
    printf("set >>>> ch:%d, id:0x%08x(:%d), size:%d\n", ch, (id << 8)|(ch), (id << 8)|(ch), size);\
    if((default_param_handle)&&(param))sdk_param_set(default_param_handle, (id << 8)|(ch), 1, &size, param);}while(0)

#define SDK_PARAM_GET_DEF(ch, id, param) do{\
    int size = sizeof(*param);\
    int size2 = 0;\
    if((default_param_handle)&&(param))sdk_param_get(default_param_handle, (id << 8)|(ch), size, &size2, param);\
    /*printf("get ++++ ch:%d, id:0x%08x, size:%d\n", ch, (id << 8)|(ch), size);*/\
    /*if(size2 != size)printf("size:%d != size2:%d\n", size, size2);*/}while(0)

#define YEAR2 ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10 \
+ (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))

#define MONTH2 (__DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 0 : 5) \
: __DATE__ [2] == 'b' ? 1 \
: __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 2 : 3) \
: __DATE__ [2] == 'y' ? 4 \
: __DATE__ [2] == 'l' ? 6 \
: __DATE__ [2] == 'g' ? 7 \
: __DATE__ [2] == 'p' ? 8 \
: __DATE__ [2] == 't' ? 9 \
: __DATE__ [2] == 'v' ? 10 : 11)

#define DAY2 ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 + (__DATE__ [5] - '0'))


/* ----------------------------------------------- */
int _open_file(const char *src_file, const char *dst_file)
{
	int ret = 0, fd = 0;
	fd = open(dst_file, O_RDWR);
	if (fd < 0)
	{
		warning("-------- copy file:%s\n", dst_file);
		// ".sysparam"文件不存在
		ret = os_copy_file(src_file, dst_file, 1);
		if (ret < 0)
		{
			error("copy_file error!\n");
			return -1;
		}
	}
	close(fd);

	return 0;
}

int adapt_param_init()
{
	int ret = 0;
	int upg_is_exist = 0;
	
	upg_is_exist = os_is_file_exist(PARAM_DEF_FILE_UPG)==1?1:0;
	warning("---------------------------------> upg_is_exist:%d\n", upg_is_exist);
	
	// 默认参数
	if (upg_is_exist)
	{
		ret = os_copy_file(PARAM_DEF_FILE_UPG, PARAM_DEF_FILE, 0);
		if (ret < 0)
		{
			error("os_copy_file error!\n");
			return -1;
		}

		ret = os_rm_file(PARAM_DEF_FILE_UPG);
		if (ret < 0)
		{
			error("os_rm_file error! PARAM_DEF_FILE_UPG \n");
			return 0;
		}

		ret = os_rm_file(PARAM_CURR_FILE_1);
		if (ret < 0)
		{
			error("os_rm_file error! PARAM_CURR_FILE_1 \n");
			return 0;
		}
		
		ret = os_rm_file(PARAM_CURR_FILE_2);
		if (ret < 0)
		{
			error("os_rm_file error! PARAM_CURR_FILE_2 \n");
			return 0;
		}
	}
	
	default_param_handle = sdk_param_load(PARAM_DEF_FILE, NULL, 1);
	if(default_param_handle == NULL)
	{
		error("sdk_param_load error!\n");
	    return -1;
	}

	// 装载".sysparam1"文件前保证此文件已经存在，如果不存在则从默认配置文件复制一份
	ret = _open_file(PARAM_DEF_FILE, PARAM_CURR_FILE_1);
	if (ret < 0)
	{
		error("_open_file 1 error!\n");
		return -1;
	}
	// 装载".sysparam2"文件前保证此文件已经存在，如果不存在则从默认配置文件复制一份
	ret = _open_file(PARAM_DEF_FILE, PARAM_CURR_FILE_2);
	if (ret < 0)
	{
		error("_open_file 2 error!\n");
		return -1;
	}
	
    param_handle = sdk_param_load(PARAM_CURR_FILE_1, PARAM_CURR_FILE_2, 1);
    if(param_handle)
    {
		// set up device type
		{
			sdk_sys_cfg_t _sys_cfg;
			memset(&_sys_cfg, 0, sizeof(_sys_cfg));
			SDK_PARAM_GET_(0, SDK_MAIN_MSG_SYS_CFG, &_sys_cfg);
			g_param_dev_type = _sys_cfg.dev_type;
			warning("---------------------------------> g_param_dev_type:%d\n", g_param_dev_type);
		}
	
        return 0;
    }
    else
    {
    	error("sdk_param_load error!\n");
        return -1;
    }
}

// 用于生成默认配置文件
int adapt_param_init_default_param(void)
{
	// 默认参数
    param_handle = sdk_param_load(".sysparam_def", NULL, 1);
    if(param_handle)
    {
    	info("sdk_param_load ok!\n");
        return 0;
    }
    else
    {
    	error("sdk_param_load error!\n");
        return -1;
    }
}

int adapt_param_deinit(void)
{
	sdk_param_sync(param_handle);
    sdk_param_destroy(param_handle);
    param_handle = 0;

    sdk_param_destroy(default_param_handle);
    default_param_handle = 0;
	warning("adapt_param_deinit!\n");
    return 0;
}

/*********************  默认参数 ******************/
int adapt_param_get_def_sys_cfg(sdk_sys_cfg_t *sys_cfg)
{
    SDK_PARAM_GET_DEF(0, SDK_MAIN_MSG_SYS_CFG, sys_cfg);
	return 0;
}

int adapt_param_set_def_sys_cfg(sdk_sys_cfg_t *sys_cfg)
{
    SDK_PARAM_SET_DEF(0, SDK_MAIN_MSG_SYS_CFG, sys_cfg);
	return 0;
}

int adapt_param_get_def_encode_cfg(int ch, sdk_encode_t *cfg)
{
    SDK_PARAM_GET_DEF(ch, SDK_MAIN_MSG_ENCODE_CFG, cfg);
    return 0;
}

int adapt_param_get_def_ptz_cfg(int ch, sdk_ptz_param_t *cfg)
{
    SDK_PARAM_GET_DEF(ch, SDK_MAIN_MSG_PTZ_CFG, cfg);
    return 0;
}

int adapt_param_get_def_record_cfg(int ch, sdk_record_cfg_t *cfg)
{
    SDK_PARAM_GET_DEF(ch, SDK_MAIN_MSG_RECORD_CFG, cfg);
    return 0;
}

int adapt_param_get_def_net_mng_cfg(sdk_net_mng_cfg_t *cfg)
{
    SDK_PARAM_GET_DEF(0, SDK_MAIN_MSG_NET_MNG_CFG, cfg);
    return 0;
}

int adapt_param_set_def_net_mng_cfg(sdk_net_mng_cfg_t *cfg)
{
    SDK_PARAM_SET_DEF(0, SDK_MAIN_MSG_NET_MNG_CFG, cfg);
    return 0;
}


#define CHECK_NET_IDX(idx) do{if(((idx) < 0) || ((idx) >=SDK_NET_IDX_BUTT)) return -1;}while(0)


int adapt_param_get_def_eth_cfg(sdk_eth_cfg_t *cfg)
{
    CHECK_NET_IDX(0);
    SDK_PARAM_GET_DEF(0, SDK_MAIN_MSG_NET_LINK_CFG, cfg);
    return 0;
}
int adapt_param_set_def_eth_cfg(sdk_eth_cfg_t *cfg)
{
    CHECK_NET_IDX(0);
    SDK_PARAM_SET_DEF(0,SDK_MAIN_MSG_NET_LINK_CFG, cfg);
    return 0;
}




int adapt_param_get_def_serial_cfg(int ch, sdk_serial_func_cfg_t *cfg)
{
    SDK_PARAM_GET_DEF(ch, SDK_MAIN_MSG_SERIAL_CFG, cfg);
    return 0;
}

int adapt_param_get_def_overlay_cfg(int ch, sdk_overlay_cfg_t *cfg)
{
    SDK_PARAM_GET_DEF(ch, SDK_MAIN_MSG_OVERLAY_CFG, cfg);
    return 0;
}

int adapt_param_get_def_motion_cfg(int ch, sdk_motion_cfg_t *cfg)
{
    SDK_PARAM_GET_DEF(ch, SDK_MAIN_MSG_MOTION_CFG, cfg);
    return 0;
}

int adapt_param_get_def_osd_cfg(int ch, sdk_osd_cfg_t *cfg)
{
    SDK_PARAM_GET_DEF(ch, SDK_MAIN_MSG_OSD_CFG, cfg);
    return 0;
}

int adapt_param_get_def_alarm_in_cfg(int ch, sdk_alarm_in_cfg_t *cfg)
{
    SDK_PARAM_GET_DEF(ch, SDK_MAIN_MSG_ALARM_IN_CFG, cfg);
    return 0;
}

int adapt_param_get_def_hide_cfg(int ch, sdk_hide_cfg_t *cfg)
{
    SDK_PARAM_GET_DEF(ch, SDK_MAIN_MSG_HIDE_CFG, cfg);
    return 0;
}

int adapt_param_get_def_lost_cfg(int ch, sdk_lost_cfg_t *cfg)
{
    SDK_PARAM_GET_DEF(ch, SDK_MAIN_MSG_LOST_CFG, cfg);
    return 0;
}

int	adapt_param_get_def_baidu_cfg(sdk_baidu_info_t  *baidu_cfg)
{
    SDK_PARAM_GET_DEF(0, SDK_MAIN_MSG_BAIDU_INFO, baidu_cfg);
    return 0;

}

int	adapt_param_get_def_wifi_cfg(sdk_wifi_cfg_t  *wifi_cfg)
{
    SDK_PARAM_GET_DEF(0, SDK_MAIN_MSG_WIFI_PARAM_CFG, wifi_cfg);
    return 0;

}


/**************************************************/


/* ----------------------------------------------- */
int adapt_param_get_sys_cfg(sdk_sys_cfg_t *sys_cfg)
{	
	SDK_PARAM_GET_(0, SDK_MAIN_MSG_SYS_CFG, sys_cfg);
    return 0;
}

int adapt_param_set_sys_cfg(sdk_sys_cfg_t *sys_cfg)
{
    SDK_PARAM_SET_(0, SDK_MAIN_MSG_SYS_CFG, sys_cfg);
    return 0;
}
#if 0
int adapt_param_get_registerbaidu_cfg(sdk_device_t *devide_cfg)
{
    SDK_PARAM_GET_(0, SDK_MAIN_MSG_REGISTER_INFO, devide_cfg);
    return 0;
}
int adapt_param_set_registerbaidu_cfg(sdk_device_t *devide_cfg)
{
    SDK_PARAM_SET_(0, SDK_MAIN_MSG_REGISTER_INFO, devide_cfg);
    return 0;
}
#else
int adapt_param_get_baidu_cfg(sdk_baidu_info_t  *baidu_cfg)
{
    SDK_PARAM_GET_(0, SDK_MAIN_MSG_BAIDU_INFO, baidu_cfg);
    return 0;
}
int adapt_param_set_baidu_cfg(sdk_baidu_info_t  *baidu_cfg)
{
    SDK_PARAM_SET_(0, SDK_MAIN_MSG_BAIDU_INFO, baidu_cfg);
    return 0;
}

#endif

int adapt_param_default2current(sdk_default_param_t *default_param)
{
    return 0;
}

// 恢复默认参数，此接口仅负责恢复参数(不含刷新至其它模块)
int adapt_param_recovery_param(sdk_default_param_t *p_default, uint32_t ch_def)
{
	int i = 0;
	int ret = -1, ch = 0;
    uint8_t msg_buffer[1024]= {0};
    sdk_msg_t *pmsg = (sdk_msg_t*)msg_buffer;
	int max_ch = adapt_param_get_chnum();	
	uint32_t param_mask = p_default->param_mask;


	if (PARAM_MASK_ALL == param_mask)		// 恢复整机参数
	{
		warning("====> param_mask_all...\n");
		unlink("rm -rf /config/param/.sysparam1");
		unlink("rm -rf /config/param/.sysparam2");
		unlink("rm -rf /config/wireless_info.conf"); // remove wifi config info
		
		system_ex("rm -rf /config/param/.sysparam1");
		system_ex("rm -rf /config/param/.sysparam2");
		system_ex("rm -rf /config/wireless_info.conf"); // delete wifi config info
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
								adapt_param_set_encode_cfg(ch, &encode_cfg);
							}
						}
						else
						{
							memset(&encode_cfg, 0, sizeof(encode_cfg));
							adapt_param_get_def_encode_cfg(ch_def, &encode_cfg);
							adapt_param_set_encode_cfg(ch_def, &encode_cfg);
						}
					}
					break;
					
					case SDK_PARAM_MASK_PTZ:	// 云台参数
					{
						sdk_ptz_param_t ptz_cfg;
						if (ch_def == CHANN_TYPE_ALL)
						{
							for (ch=0; ch<max_ch; ch++)
							{
								memset(&ptz_cfg, 0, sizeof(ptz_cfg));
								adapt_param_get_def_ptz_cfg(ch, &ptz_cfg);
								adapt_param_set_ptz_cfg(ch, &ptz_cfg);
							}
						}
						else
						{
							memset(&ptz_cfg, 0, sizeof(ptz_cfg));
							adapt_param_get_def_ptz_cfg(ch_def, &ptz_cfg);
							adapt_param_set_ptz_cfg(ch_def, &ptz_cfg);
						}
					}
					break;

					case SDK_PARAM_MASK_RECORD:	// 录像参数
					{
						sdk_record_cfg_t record_cfg;
						if (ch_def == CHANN_TYPE_ALL)
						{
							for (ch=0; ch<max_ch; ch++)
							{
								memset(&record_cfg, 0, sizeof(record_cfg));
								adapt_param_get_def_record_cfg(0, &record_cfg);
								adapt_param_set_record_cfg(ch, &record_cfg);
							}
						}
						else
						{
							memset(&record_cfg, 0, sizeof(record_cfg));
							adapt_param_get_def_record_cfg(ch_def, &record_cfg);
							adapt_param_set_record_cfg(ch_def, &record_cfg);
						}
					}
					break;

					case SDK_PARAM_MASK_NET:	// 网络参数
					{
						sdk_net_mng_cfg_t net_mng;
						memset(&net_mng, 0, sizeof(net_mng));
						adapt_param_get_def_net_mng_cfg(&net_mng);
						adapt_param_set_net_mng_cfg(&net_mng);
					}
					break;
					case SDK_PARAM_MASK_SERIAL:			// 串口参数
					{
						sdk_serial_func_cfg_t serialcfg;
						if (ch_def == CHANN_TYPE_ALL)
						{
							for (ch=0; ch<max_ch; ch++)
							{
								memset(&serialcfg, 0, sizeof(serialcfg));
								adapt_param_get_def_serial_cfg(ch, &serialcfg);
								adapt_param_set_serial_cfg(ch, &serialcfg);
							}
						}
						else
						{
							memset(&serialcfg, 0, sizeof(serialcfg));
							adapt_param_get_def_serial_cfg(ch_def, &serialcfg);
							adapt_param_set_serial_cfg(ch_def, &serialcfg);
							
						}
					}
					break;

					case SDK_PARAM_MASK_IMAGE_ATTR:		// 图像属性
					{
						#if 0
						sdk_image_attr_t image_attr;
						if (ch_def == CHANN_TYPE_ALL)
						{
							for (ch=0; ch<max_ch; ch++)
							{
								memset(&image_attr, 0, sizeof(image_attr));
								adapt_param_get_def_image_cfg(ch, &image_attr);
								adapt_param_set_image_attr(ch, &image_attr);
							}
						}
						else
						{
							memset(&image_attr, 0, sizeof(image_attr));
							adapt_param_get_def_image_cfg(ch_def, &image_attr);
							adapt_param_set_image_attr(ch_def, &image_attr);
						}
						#endif
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
								adapt_param_set_overlay_cfg(ch, &overlaycfg);
							}
						}
						else
						{
							memset(&overlaycfg, 0, sizeof(overlaycfg));
							adapt_param_get_def_overlay_cfg(ch_def, &overlaycfg);
							adapt_param_set_overlay_cfg(ch_def, &overlaycfg);							
						}
					}
					break;

					case SDK_PARAM_MASK_MOTION:		// 移动侦测
					{
						sdk_motion_cfg_t motioncfg;
						if (ch_def == CHANN_TYPE_ALL)
						{
							for (ch=0; ch<max_ch; ch++)
							{
								memset(&motioncfg, 0, sizeof(motioncfg));
								adapt_param_get_def_motion_cfg(ch, &motioncfg);
								adapt_param_set_motion_cfg(ch, &motioncfg);
							}
						}
						else
						{
							memset(&motioncfg, 0, sizeof(motioncfg));
							adapt_param_get_def_motion_cfg(ch_def, &motioncfg);
							adapt_param_set_motion_cfg(ch_def, &motioncfg);							
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
								adapt_param_set_osd_cfg(ch, &osd_cfg);
							}
						}
						else
						{
							memset(&osd_cfg, 0, sizeof(osd_cfg));						
							adapt_param_get_def_osd_cfg(ch_def, &osd_cfg);
							adapt_param_set_osd_cfg(ch_def, &osd_cfg);
						}
					}
					break;

					case SDK_PARAM_MASK_USER:		// 用户管理
					{
						int i = 0;
						sdk_user_right_t root_user;
						memset(&root_user, 0, sizeof(root_user));
						memcpy(root_user.user.user_name, "admin", strlen("admin"));
						//memcpy(root_user.user.user_pwd, "123456", strlen("123456"));
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
					break;

					case SDK_PARAM_MASK_ALARM_IN:		// 外部报警输入
					{
						sdk_alarm_in_cfg_t alarmin_cfg;
						if (ch_def == CHANN_TYPE_ALL)
						{
							for (ch=0; ch<max_ch; ch++)
							{
								memset(&alarmin_cfg, 0, sizeof(alarmin_cfg));						
								adapt_param_get_def_alarm_in_cfg(ch, &alarmin_cfg);
								adapt_param_set_alarm_in_cfg(ch, &alarmin_cfg);
							}
						}
						else
						{
							memset(&alarmin_cfg, 0, sizeof(alarmin_cfg));						
							adapt_param_get_def_alarm_in_cfg(ch_def, &alarmin_cfg);
							adapt_param_set_alarm_in_cfg(ch_def, &alarmin_cfg);							
						}
					}
					break;

					case SDK_PARAM_MASK_HIDE:		// 视频遮挡侦测参数
					{
						sdk_hide_cfg_t hide_cfg;
						if (ch_def == CHANN_TYPE_ALL)
						{
							for (ch=0; ch<max_ch; ch++)
							{
								memset(&hide_cfg, 0, sizeof(hide_cfg));						
								adapt_param_get_def_hide_cfg(ch, &hide_cfg);
								adapt_param_set_hide_cfg(ch, &hide_cfg);
							}
						}
						else
						{
							memset(&hide_cfg, 0, sizeof(hide_cfg));						
							adapt_param_get_def_hide_cfg(ch_def, &hide_cfg);
							adapt_param_set_hide_cfg(ch_def, &hide_cfg);						
						}
					}
					break;

					case SDK_PARAM_MASK_VIDEO_LOST:		// 视频丢失参数
					{
						sdk_lost_cfg_t lost_cfg;
						if (ch_def == CHANN_TYPE_ALL)
						{
							for (ch=0; ch<max_ch; ch++)
							{
								memset(&lost_cfg, 0, sizeof(lost_cfg));						
								adapt_param_get_def_lost_cfg(ch, &lost_cfg);
								adapt_param_set_lost_cfg(ch, &lost_cfg);
							}
						}
						else
						{
							memset(&lost_cfg, 0, sizeof(lost_cfg));						
							adapt_param_get_def_lost_cfg(ch_def, &lost_cfg);
							adapt_param_set_lost_cfg(ch_def, &lost_cfg);
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
	
	return 0;
}

/* ----------------------------------------------- */

int adapt_param_get_user_array(sdk_user_right_t *user_array, int size)
{
    int ch = 0;
	int size2 = 0;
	sdk_param_get(param_handle, (SDK_MAIN_MSG_USER << 8)|(ch), size, &size2, user_array);
	
    return 0;
}
int adapt_param_set_user_array(sdk_user_right_t *user_array, int size)
{	
	int ch = 0;
	sdk_param_set(param_handle, (SDK_MAIN_MSG_USER << 8)|(ch), 1, &size, user_array);
	
    return 0;
}

/* ----------------------------------------------- */
int adapt_param_get_alarm_in_cfg(int ch, sdk_alarm_in_cfg_t *alarm_in_cfg)
{
    SDK_PARAM_GET_(ch, SDK_MAIN_MSG_ALARM_IN_CFG, alarm_in_cfg);
    return 0;
}
int adapt_param_set_alarm_in_cfg(int ch, sdk_alarm_in_cfg_t *alarm_in_cfg)
{
    SDK_PARAM_SET_(ch, SDK_MAIN_MSG_ALARM_IN_CFG, alarm_in_cfg);
    return 0;
}
/* ----------------------------------------------- */
int adapt_param_get_motion_cfg(int ch, sdk_motion_cfg_t *motion_cfg) 
{
    SDK_PARAM_GET_(ch, SDK_MAIN_MSG_MOTION_CFG, motion_cfg);
    return 0;
}
int adapt_param_set_motion_cfg(int ch, sdk_motion_cfg_t *motion_cfg)
{
    SDK_PARAM_SET_(ch, SDK_MAIN_MSG_MOTION_CFG, motion_cfg);
    return 0;
}
/* ----------------------------------------------- */
int adapt_param_get_lost_cfg(int ch, sdk_lost_cfg_t *lost_cfg) 
{
    SDK_PARAM_GET_(ch, SDK_MAIN_MSG_LOST_CFG, lost_cfg);
    return 0;
}
int adapt_param_set_lost_cfg(int ch, sdk_lost_cfg_t *lost_cfg)
{
    SDK_PARAM_SET_(ch, SDK_MAIN_MSG_LOST_CFG, lost_cfg);
    return 0;
}
/* ----------------------------------------------- */
int adapt_param_get_hide_cfg(int ch, sdk_hide_cfg_t *hide_cfg) 
{
    SDK_PARAM_GET_(ch, SDK_MAIN_MSG_HIDE_CFG, hide_cfg);
    return 0;
}
int adapt_param_set_hide_cfg(int ch, sdk_hide_cfg_t *hide_cfg)
{
    SDK_PARAM_SET_(ch, SDK_MAIN_MSG_HIDE_CFG, hide_cfg);
    return 0;
}
/* ----------------------------------------------- */
int adapt_param_get_record_cfg(int ch, sdk_record_cfg_t *record_cfg) 
{
    SDK_PARAM_GET_(ch, SDK_MAIN_MSG_RECORD_CFG, record_cfg);
    return 0;
}
int adapt_param_set_record_cfg(int ch, sdk_record_cfg_t *record_cfg)
{
    SDK_PARAM_SET_(ch, SDK_MAIN_MSG_RECORD_CFG, record_cfg);
    return 0;
}
/* ----------------------------------------------- */
int adapt_param_get_encode_cfg(int ch, sdk_encode_t *encode)
{
    SDK_PARAM_GET_(ch, SDK_MAIN_MSG_ENCODE_CFG, encode);
    return 0;
}
int adapt_param_set_encode_cfg(int ch, sdk_encode_t *encode)
{
    SDK_PARAM_SET_(ch, SDK_MAIN_MSG_ENCODE_CFG, encode);
    return 0;
}


/* ----------------------------------------------- */
int adapt_param_get_net_mng_cfg(sdk_net_mng_cfg_t *net_mng)
{
    SDK_PARAM_GET_(0, SDK_MAIN_MSG_NET_MNG_CFG, net_mng);
    return 0;
}
int adapt_param_set_net_mng_cfg(sdk_net_mng_cfg_t *net_mng)
{
    SDK_PARAM_SET_(0, SDK_MAIN_MSG_NET_MNG_CFG, net_mng);
    return 0;
}

int adapt_param_get_platform_cfg(sdk_platform_t * platform_cfg)
{
    SDK_PARAM_GET_(0, SDK_MAIN_MSG_PLATFORM_CFG, platform_cfg);
    return 0;
}
int adapt_param_set_platform_cfg(sdk_platform_t * platform_cfg)
{
    SDK_PARAM_SET_(0, SDK_MAIN_MSG_PLATFORM_CFG, platform_cfg);
    return 0;
}


/* ----------------------------------------------- */
int adapt_param_get_ptz_cfg(int ch, sdk_ptz_param_t *ptz_cfg)
{
    SDK_PARAM_GET_(ch, SDK_MAIN_MSG_PTZ_CFG, ptz_cfg);
    return 0;
}
int adapt_param_set_ptz_cfg(int ch, sdk_ptz_param_t *ptz_cfg)
{
    SDK_PARAM_SET_(ch, SDK_MAIN_MSG_PTZ_CFG, ptz_cfg);
    return 0;
}


/* ----------------------------------------------- */
int adapt_param_get_osd_cfg(int ch, sdk_osd_cfg_t *osd_cfg)
{
    SDK_PARAM_GET_(ch, SDK_MAIN_MSG_OSD_CFG, osd_cfg);
    return 0;
}
int adapt_param_set_osd_cfg(int ch, sdk_osd_cfg_t *osd_cfg)
{
    SDK_PARAM_SET_(ch, SDK_MAIN_MSG_OSD_CFG, osd_cfg);
    return 0;
}

/* ----------------------------------------------- */

/* ----------------------------------------------- */
int adapt_param_get_overlay_cfg(int ch, sdk_overlay_cfg_t *poverlay_cfg)
{
    SDK_PARAM_GET_(ch, SDK_MAIN_MSG_OVERLAY_CFG, poverlay_cfg);
    return 0;
}
int adapt_param_set_overlay_cfg(int ch, sdk_overlay_cfg_t *poverlay_cfg)
{
    SDK_PARAM_SET_(ch, SDK_MAIN_MSG_OVERLAY_CFG, poverlay_cfg);
    return 0;
}

/* ----------------------------------------------- */

int adapt_param_get_chnum()
{
	
	return (0);
}

int adapt_param_get_ana_chan_num()
{
	
	return 0;
}

/* ----------------------------------------------- */
int adapt_param_get_serial_cfg(int ch, sdk_serial_func_cfg_t *pserial_cfg)
{
    SDK_PARAM_GET_(ch, SDK_MAIN_MSG_SERIAL_CFG, pserial_cfg);
    return 0;
}

int adapt_param_set_serial_cfg(int ch, sdk_serial_func_cfg_t *pserial_cfg)
{
    SDK_PARAM_SET_(ch, SDK_MAIN_MSG_SERIAL_CFG, pserial_cfg);
    return 0;
}

/* ----------------------------------------------- */
int adapt_param_get_image_attr(int ch, sdk_image_attr_t *pimage_cfg)
{
    SDK_PARAM_GET_(ch, SDK_MAIN_MSG_IMAGE_ATTR_CFG, pimage_cfg);
    return 0;
}

int adapt_param_set_image_attr(int ch, sdk_image_attr_t *pimage_cfg)
{
    SDK_PARAM_SET_(ch, SDK_MAIN_MSG_IMAGE_ATTR_CFG, pimage_cfg);
    return 0;
}

/* ----------------------------------------------- */
#if 0
int adapt_param_get_3a_attr(int ch, sdk_3a_attr_t *p3a_cfg)
{
    SDK_PARAM_GET_(ch, SDK_MAIN_MSG_3A_ATTR_CFG, p3a_cfg);
    return 0;
}

int adapt_param_set_3a_attr(int ch, sdk_3a_attr_t *p3a_cfg)
{
    SDK_PARAM_SET_(ch, SDK_MAIN_MSG_3A_ATTR_CFG, p3a_cfg);
    return 0;
}
#endif
/* ----------------------------------------------- */




/* ----------------------------------------------- */
int adapt_param_get_manual_rec_cfg(sdk_manual_record_t *prec_cfg)
{
	SDK_PARAM_GET_(0, SDK_MAIN_MSG_MANUAL_REC, prec_cfg);
    return 0;
}

int adapt_param_set_manual_rec_cfg(sdk_manual_record_t *prec_cfg)
{
    SDK_PARAM_SET_(0, SDK_MAIN_MSG_MANUAL_REC, prec_cfg);
    return 0;
}

/* ----------------------------------------------- */
int adapt_param_get_manual_almin_cfg(sdk_manual_alarmin_t *palmin_cfg)
{
	SDK_PARAM_GET_(0, SDK_MAIN_MSG_MANUAL_ALMIN, palmin_cfg);
    return 0;
}

int adapt_param_set_manual_almin_cfg(sdk_manual_alarmin_t *palmin_cfg)
{
    SDK_PARAM_SET_(0, SDK_MAIN_MSG_MANUAL_ALMIN, palmin_cfg);
    return 0;
}

/* ----------------------------------------------- */
int adapt_param_get_manual_almout_cfg(sdk_manual_alarmout_t *palmout_cfg)
{
	SDK_PARAM_GET_(0, SDK_MAIN_MSG_MANUAL_ALMOUT, palmout_cfg);
    return 0;
}

int adapt_param_set_manual_almout_cfg(sdk_manual_alarmout_t *palmout_cfg)
{
    SDK_PARAM_SET_(0, SDK_MAIN_MSG_MANUAL_ALMOUT, palmout_cfg);
    return 0;
}


int adapt_os_kill_dhcp()
{
	warning(" _kill_dhcp...\n");
    system("killall udhcpc");
	return 0;
}

int adapt_os_start_dhcp(char *p_net_name)
{
	int i = 0;
	char szStr[256] = {0};

    //adapt_os_kill_dhcp();

	warning("_start_dhcp ...\n");
    
    //sprintf(szStr, "ifconfig %s 0.0.0.0", p_net_name); //clean the ip first
    //system(szStr);
#if 1   
	sprintf(szStr, "wan_udhcpc.sh %s &", p_net_name);

	system(szStr);
#endif
    memset(szStr, 0, sizeof(szStr));
    os_get_ip_addr(p_net_name, szStr, MAX_IP_ADDR_LEN);
    printf("111111111111111111111 ip:%s\n", szStr);
   
	memset(szStr, 0, sizeof(szStr));
	for (i=0; i<=3; ++i)
	{
		sleep(1);
		if (os_get_ip_addr(p_net_name, szStr, MAX_IP_ADDR_LEN) >= 0)
		{
			printf("ip:%s\n", szStr);
			return 0;
		}	
	}
	return -1;
}


char* adapt_os_get_if_name(SDK_NET_IDX_E idx, char *if_name)
{
    switch(idx)
    {
        case SDK_NET_IDX_ETH0:
            strcpy(if_name, OS_ETH0);
            break;
        case SDK_NET_IDX_ETH1:
            strcpy(if_name, OS_ETH1);
            break;
        case SDK_NET_IDX_PPPOE0:
            strcpy(if_name, OS_PPP0);
            break;
        default:
            return NULL;
            break;
    }
    return if_name;
}


int adapt_os_get_ip_info(sdk_ip_info_t *ip_info, int idx)
{
	int ret = 0;
    char if_name[MAX_IF_NAME_LEN] = {0};
    
    CHECK_NET_IDX(idx);
    
	if(adapt_os_get_if_name(idx, if_name) == NULL)
	{
	    return -1;
	}

	ret = os_get_mac_addr(if_name, (char *)ip_info->mac, MAX_MAC_ADDR_LEN);
	if (ret < 0)
	{
		error("os_get_mac_addr error!\n");
	}
	
	ret = os_get_ip_addr(if_name, (char *)ip_info->ip_addr, MAX_IP_ADDR_LEN);
	if (ret < 0)
	{
		error("os_get_ip_addr error!\n");
	}

	ret = os_get_net_mask(if_name, (char *)ip_info->mask, MAX_IP_ADDR_LEN);
	if (ret < 0)
	{
		error("os_get_net_mask error!\n");
	}

	//ret = os_get_gateway(if_name, (char *)ip_info->gateway, MAX_IP_ADDR_LEN);
	ret = os_get_gateway1(if_name, (char *)ip_info->gateway, MAX_IP_ADDR_LEN);

	ret = os_get_dns((char *)ip_info->dns1, (char *)ip_info->dns2, MAX_IP_ADDR_LEN);
	if (ret < 0)
	{
		error("os_get_dns error!\n");
	}
	
	return 0;

}

int adapt_os_set_ip_info(sdk_ip_info_t *ip_info, int idx)
{
    int ret = -1;
    int resave_flags = 0;
    char os_mac[MAX_MAC_ADDR_LEN] = {0};
    char if_name[MAX_IF_NAME_LEN] = {0};
    
    sdk_net_mng_cfg_t net_mng;
    memset(&net_mng, 0, sizeof(sdk_net_mng_cfg_t));
    
    
    CHECK_NET_IDX(idx);
    
	if(adapt_os_get_if_name(idx, if_name) == NULL)
	{
	    return -1;
	}
	
    //MacAddr[0] &= 0xFE;
    os_get_mac_addr(if_name, os_mac, sizeof(os_mac));
    info("if_name:%s, mac:%s\n", if_name, (char *)ip_info->mac);
    ret = os_set_mac_addr(if_name, (char *)ip_info->mac);
    if (ret < 0)
    {
        error("os_set_mac_addr error!\n");
        resave_flags = 1;
    }
    
    info("if_name:%s, ipaddr:%s\n", if_name, (char *)ip_info->ip_addr);
    ret = os_set_ip_addr(if_name, (char *)ip_info->ip_addr);
    if (ret < 0)
    {
        error("os_set_ip_addr error!\n");
    }
    
    info("if_name:%s, mask:%s\n", if_name, (char *)ip_info->mask);
    ret = os_set_net_mask(if_name, (char *)ip_info->mask);
    if (ret < 0)
    {
        error("os_set_net_mask error!\n");
    }
    adapt_param_get_net_mng_cfg(&net_mng);    
    if(net_mng.def_if_no == idx)
    {
        warning("dns1:%s, dns2:%s\n", (char *)ip_info->dns1, (char *)ip_info->dns2);
        ret = os_set_dns(strlen((char *)ip_info->dns1)?(char *)ip_info->dns1:NULL
                    , strlen((char *)ip_info->dns2)?(char *)ip_info->dns2:NULL);
        if (ret < 0)
        {
            error("os_set_dns error!\n");
        }

        //网关的设置需要放在广播地址之后，否则网关将会设置失败    

       // warning("os_del_gateway >>>>>>>>>> gateway:0.0.0.0\n");
    	//os_del_gateway("0.0.0.0");
        warning("os_set_gateway >>>>>>>>>> if_name:%s, gateway:%s\n", if_name, (char *)ip_info->gateway);
        //ret = os_set_gateway(if_name, (char *)ip_info->gateway);
		ret = os_set_gateway1(if_name, (char *)ip_info->gateway);
        if (ret < 0)
        {
            error("os_set_gateway error!\n");
        }
    }
    
    if(ip_info->enable_dhcp)
    {
        //多实个网络接口，同时DHCP时 对应获取的IP信息要传到主程序程序中的结构体中;
        adapt_os_start_dhcp(if_name);
    }
    else
    {   //指定kill掉对应网络接口的DHCP实例;
        adapt_os_kill_dhcp();
    }

    if(resave_flags)
    {
        strncpy((char *)ip_info->mac, os_mac, sizeof(ip_info->mac));
        error("resave parm if_name:%s, mac:[%s]\n", if_name, ip_info->mac);
        //adapt_param_set_ip_info(ip_info, idx);
        //mac 一样的话就不用再往系统里面设置，否则冲掉mac
        //os_set_mac_addr(if_name, (char *)ip_info->mac);
    }

    if(os_check_mac_addr((char *)ip_info->mac)
        || os_check_ip_addr((char *)ip_info->ip_addr))
    {
        return -1;
    }
    return 0;
}

int adapt_os_get_eth_cfg(sdk_eth_cfg_t *peth, int idx)
{
    CHECK_NET_IDX(idx);
    return adapt_os_get_ip_info((sdk_ip_info_t*)peth, idx);

}
int adapt_os_set_eth_cfg(sdk_eth_cfg_t *peth, int idx)
{
    CHECK_NET_IDX(idx);
    return adapt_os_set_ip_info((sdk_ip_info_t*)peth, idx);
}


int adapt_os_get_net_mng_cfg(sdk_net_mng_cfg_t *pnet_mng)
{
    return 0;
}

int adapt_os_set_net_mng_cfg(sdk_net_mng_cfg_t *pnet_mng)
{
    int ret = -1;
    sdk_ip_info_t ip_info;
    char if_name[MAX_IF_NAME_LEN] = {0};
#if 1   
    memset(&ip_info, 0, sizeof(sdk_ip_info_t));

    printf("def_if_no:%d, broadcast:%s\n", pnet_mng->def_if_no, (char *)pnet_mng->multicast);
    
    ret = os_set_broad_cast(OS_ETH0, (char *)pnet_mng->multicast);
    if (ret < 0)
    {
        error("os_set_broad_cast error!\n");
    }
    // 网关的设置需要放在广播地址之后，否则网关将会设置失败
    if(pnet_mng->def_if_no >= 0  || pnet_mng->def_if_no < SDK_NET_IDX_BUTT)
	{   
	    //只获取网络接口配置中的 ip_info;
		adapt_param_get_ip_info(&ip_info, pnet_mng->def_if_no);
		//获取if_name以后需要修改
	    if(adapt_os_get_if_name(pnet_mng->def_if_no, if_name))
	    {
	        warning("dns1:%s, dns2:%s\n", (char *)ip_info.dns1, (char *)ip_info.dns2);
            ret = os_set_dns(strlen((char *)ip_info.dns1)?(char *)ip_info.dns1:NULL
                        , strlen((char *)ip_info.dns2)?(char *)ip_info.dns2:NULL);
            if (ret < 0)
            {
                error("os_set_dns error!\n");
            }
	        
	       // warning("os_del_gateway >>>>>>>>>> gateway:0.0.0.0\n");
	       // os_del_gateway("0.0.0.0");
	        
	        warning("os_set_gateway >>>>>>>>>> if_name:%s, gateway:%s\n", if_name, (char *)ip_info.gateway);
    		//ret = os_set_gateway(if_name, (char *)ip_info.gateway);
			ret = os_set_gateway1(if_name, (char *)ip_info.gateway);
    	    if (ret < 0)
    	    {
    	        error("os_set_gateway error!\n");
    	    }	    
	    }
	}
#endif
    return 0;
}


int adapt_param_version_def(char *ver, int ver_len, char *date, int date_len)
{
	char tmp[64] = {0};

	sprintf((char*)tmp, "%s", _VR_VERSION);
    if(!strcmp(MODULE_NAME, "daemon"))
    {
        strcat(tmp, "-dm");
    }
    strncpy((char*)ver, (char*)tmp, ver_len);
	
    sprintf((char*)tmp, "%04d%02d%02d", YEAR2, MONTH2+1, DAY2);
	strncpy(date, (char*)tmp, date_len);

	return 0;
}

/* ----------------------------------------------- */
int adapt_param_get_ddns_cfg(sdk_ddns_cfg_t *pddns_cfg)
{
	SDK_PARAM_GET_(0, SDK_MAIN_MSG_DDNS_CFG, pddns_cfg);
    return 0;
}

int adapt_param_set_ddns_cfg(sdk_ddns_cfg_t *pddns_cfg)
{
    SDK_PARAM_SET_(0, SDK_MAIN_MSG_DDNS_CFG, pddns_cfg);
    return 0;
}

/* ------------------- eth ---------------------------- */
int adapt_param_get_eth_cfg(sdk_eth_cfg_t *eth_cfg)
{
    CHECK_NET_IDX(0);
    SDK_PARAM_GET_(0, SDK_MAIN_MSG_NET_LINK_CFG, eth_cfg);
    return 0;
}

int adapt_param_set_eth_cfg(sdk_eth_cfg_t *eth_cfg)
{
    CHECK_NET_IDX(0);
    SDK_PARAM_SET_(0, SDK_MAIN_MSG_NET_LINK_CFG, eth_cfg);
    return 0;
}

/* --------------------- pppoe -------------------------- */
int adapt_param_get_pppoe_cfg(sdk_pppoe_t *ppppoe_cfg, int idx)
{
    CHECK_NET_IDX(idx);
	SDK_PARAM_GET_(idx, SDK_MAIN_MSG_NET_LINK_CFG, ppppoe_cfg);
    return 0;
}

int adapt_param_set_pppoe_cfg(sdk_pppoe_t *ppppoe_cfg, int idx)
{
    CHECK_NET_IDX(idx);
    SDK_PARAM_SET_(idx, SDK_MAIN_MSG_NET_LINK_CFG, ppppoe_cfg);
    return 0;
}


/* ---------------------- wifi ------------------------- */

int adapt_param_get_wifi_cfg(sdk_wifi_cfg_t *wifi_cfg)
{
    CHECK_NET_IDX(0);
    SDK_PARAM_GET_(0, SDK_MAIN_MSG_WIFI_PARAM_CFG, wifi_cfg);
    return 0;
}

int adapt_param_set_wifi_cfg(sdk_wifi_cfg_t *wifi_cfg)
{
    CHECK_NET_IDX(0);
    SDK_PARAM_SET_(0, SDK_MAIN_MSG_WIFI_PARAM_CFG, wifi_cfg);
    return 0;
}
/* ---------------------- 3g ------------------------- */

int adapt_param_get_3g_cfg(sdk_3g_cfg_t *_3g_cfg, int idx)
{
    CHECK_NET_IDX(idx);
    SDK_PARAM_GET_(idx, SDK_MAIN_MSG_NET_LINK_CFG, _3g_cfg);
    return 0;
}

int adapt_param_set_3g_cfg(sdk_3g_cfg_t *_3g_cfg, int idx)
{
    CHECK_NET_IDX(idx);
    SDK_PARAM_SET_(idx, SDK_MAIN_MSG_NET_LINK_CFG, _3g_cfg);
    return 0;
}


int adapt_param_get_ip_info(sdk_ip_info_t *ip_info, int idx)
{
    CHECK_NET_IDX(idx);
    SDK_PARAM_GET_(idx, SDK_MAIN_MSG_NET_LINK_CFG, ip_info);
    return 0;
}

int adapt_param_set_ip_info(sdk_ip_info_t *ip_info, int idx)
{
    CHECK_NET_IDX(idx);
    SDK_PARAM_SET_(idx, SDK_MAIN_MSG_NET_LINK_CFG, ip_info);
    return 0;
}


/* ----------------------------------------------- */

int adapt_param_get_preset_cfg(int ch, sdk_preset_param_t *preset)
{
	SDK_PARAM_GET_(ch, SDK_MAIN_MSG_PRESET_SET, preset);
    return 0;
}

int adapt_param_set_preset_cfg(int ch, sdk_preset_param_t *preset)
{
	SDK_PARAM_SET_(ch, SDK_MAIN_MSG_PRESET_SET, preset);
	return 0;
}

/*-------------------add by zw for baidu-----------------------*/

//sdk_sched_time_t
int adapt_param_get_sched_param(sdk_sched_time_t *sched_param)
{
	SDK_PARAM_GET_(0, SDK_MAIN_MSG_SCHED_CFG, sched_param);
    return 0;
}

int adapt_param_set_sched_param(sdk_sched_time_t *sched_param)
{
	SDK_PARAM_SET_(0, SDK_MAIN_MSG_SCHED_CFG, sched_param);
	return 0;
}

//sdk_regional_param_t
int adapt_param_get_regional_param(sdk_regional_param_t *regional_param)
{
	SDK_PARAM_GET_(0, SDK_MAIN_MSG_REGIONAL_CFG, regional_param);
    return 0;
}

int adapt_param_set_regional_param(sdk_regional_param_t *regional_param)
{
	SDK_PARAM_SET_(0, SDK_MAIN_MSG_REGIONAL_CFG, regional_param);
	return 0;
}

//sdk_iodev_param_t  这边可能还需要修改因为部分代码是控制  并非需要写入参数
int adapt_param_get_iodev_param(sdk_iodev_param_t *iodev_param)
{
	SDK_PARAM_GET_(0, SDK_MAIN_MSG_REGIONAL_CFG, iodev_param);
    return 0;
}

int adapt_param_set_iodev_param(sdk_iodev_param_t *iodev_param)
{
	SDK_PARAM_SET_(0, SDK_MAIN_MSG_REGIONAL_CFG, iodev_param);
	return 0;
}

//main_audio_detect_cfg
//sdk_audio_detect_cfg_t  这边可能还需要修改因为部分代码是控制  并非需要写入参数
int adapt_param_get_audio_detect_param(sdk_audio_detect_cfg_t *audio_detect_cfg)
{
	SDK_PARAM_GET_(0, SDK_MAIN_MSG_AUDIO_DETECT_CFG, audio_detect_cfg);
	return 0;
}

int adapt_param_set_audio_detect_param(sdk_audio_detect_cfg_t *audio_detect_cfg)
{
	SDK_PARAM_SET_(0, SDK_MAIN_MSG_AUDIO_DETECT_CFG, audio_detect_cfg);
	return 0;
}

int adapt_param_set_mirror_param_param( sdk_mirror_flip_cfg_t *mirror_param)
{
	SDK_PARAM_SET_(0, SDK_MAIN_MSG_MIRROR_FLIP_CFG, mirror_param);
	return 0;
}

int adapt_param_get_mirror_param_param( sdk_mirror_flip_cfg_t *mirror_param)
{
	SDK_PARAM_GET_(0, SDK_MAIN_MSG_MIRROR_FLIP_CFG, mirror_param);
	return 0;
}

int adapt_param_set_power_freq_param( sdk_power_freq_cfg_t *power_freq)
{
	SDK_PARAM_SET_(0, SDK_MAIN_MSG_POWER_FREQ_CFG, power_freq);
	return 0;
}

int adapt_param_get_power_freq_param( sdk_power_freq_cfg_t *power_freq)
{
	SDK_PARAM_GET_(0, SDK_MAIN_MSG_POWER_FREQ_CFG, power_freq);
	return 0;
}


int adapt_param_get_systime( sdk_date_t *sys_time)
{
	SDK_PARAM_GET_(0, SDK_MAIN_MSG_SYSTIME, sys_time);
	return 0;
}

int adapt_param_set_systime( sdk_date_t *sys_time)
{
	SDK_PARAM_SET_(0, SDK_MAIN_MSG_SYSTIME, sys_time);
	return 0;
}
int adapt_param_get_aws_cfg(sdk_aws_info_t  *aws_cfg)
{
    SDK_PARAM_GET_(0, SDK_MAIN_MSG_AWS, aws_cfg);
    return 0;
}
int adapt_param_set_aws_cfg(sdk_aws_info_t  *aws_cfg)
{
    SDK_PARAM_SET_(0, SDK_MAIN_MSG_AWS, aws_cfg);
    return 0;
}

