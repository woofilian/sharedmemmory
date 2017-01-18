/*
 * =====================================================================================
 *
 *       Filename:  main.h
 *
 *    Description:  主控模块接口
 *
 *        Version:  1.0
 *        Created:  2014年10月10日 00时56分41秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  
 *        Company:  
 *
 * =====================================================================================
 */
#ifndef __main_h__
#define __main_h__


/*
 * 全局数据类型定义
 */
#include "sdk_global.h"

/*
 * 参数、控制 数据结构定义
 */
#include "sdk_struct.h"

/*
 * 网络协议及接口定义
 */
#include "sdk_netlib.h"

/*----------- 主控消息ID定义 -----------*/

/*
 * 消息ID遵循"只允许追加修改，不允许改变原序列
 * 参数模块使用消息ID作为参数存储单元ID，修改消息ID会影响参数配置文件.
 */
typedef enum _SDK_MAIN_MSG_ID
{
    SDK_MAIN_MSG_KEEPLIVE       = 0X00, //    	
    SDK_MAIN_MSG_VERSION        = 0x01, //  版本信息       sdk_version_t
    SDK_MAIN_MSG_SYS_CFG        = 0x02, //	系统配置       sdk_sys_cfg_t
    SDK_MAIN_MSG_STATUS         = 0x03, //	系统状态       sdk_status_t
    SDK_MAIN_MSG_PARAM_DEFAULT  = 0x04, //	4 默认参数     sdk_default_param_t, SDK_PARAM_MASK_ID_E
    SDK_MAIN_MSG_CLOUD_ID  	    = 0x05, //  云平台UID信息  sdk_cloud_uid_t 

    SDK_MAIN_MSG_WIFI_SEARCH    = 0X06, //	WIFI搜索	  sdk_wifi_search_t
    SDK_MAIN_MSG_LOG                  , //	日志管理    SDK_LOG_OP_E
    SDK_MAIN_MSG_NET_MNG_CFG          , //	网络配置    sdk_net_mng_cfg_t
    SDK_MAIN_MSG_PTZ_CFG              , //  云台配置    sdk_ptz_param_t
    SDK_MAIN_MSG_PTZ_CONTROL          , //  云台控制    SDK_PTZ_MSG_ID_E

    SDK_MAIN_MSG_BAIDU_INFO     = 0x0B, // 百度第一次配置信息 	sdk_register_cfg_t
    SDK_MAIN_MSG_WIFI_PARAM_CFG = 0X0C, //	WIFI配置	 sdk_wifi_cfg_t
    SDK_MAIN_MSG_CLOSE_SYSTEM		  , //  13 关闭系统  pmsg->args:SDK_CLOSE_SYS_OP_E
    SDK_MAIN_MSG_ENCODE_CFG           , //  14 编码配置       sdk_encode_t 
    SDK_MAIN_MSG_RECORD_CFG           , //	15 录像配置       sdk_record_cfg_t
    SDK_MAIN_MSG_RECORD_QUERY         , //  16 录像查询       sdk_record_item_t, sdk_record_cond_t
    SDK_MAIN_MSG_MOTION_CFG           , //  17 视频移动配置   sdk_motion_cfg_t
    SDK_MAIN_MSG_ALARM_IN_CFG         , //	18 外部报警输入配置sdk_alarm_in_cfg_t
    SDK_MAIN_MSG_HIDE_CFG             , //	19 视频遮蔽报警配置sdk_hide_cfg_t
    SDK_MAIN_MSG_LOST_CFG             , //	20 视频丢失配置   sdk_lost_cfg_t

    SDK_MAIN_MSG_OSD_CFG              , //  22 OSD配置       sdk_osd_cfg_t
    SDK_MAIN_MSG_AUDIO_CONTROL	      , //  22 音频控制  args:SDK_AUDIO_CONTROL_OP_E; extend:开启/关闭
    SDK_MAIN_MSG_SNAP_CONTROL	  	  , //  23 抓拍图片  
    SDK_MAIN_MSG_SYSTIME		  	  , //	25 系统时间配置	  sdk_date_t
    SDK_MAIN_MSG_NET_LINK_CFG		  , //	24 网络链接配置 sdk_eth_cfg_t

    SDK_MAIN_MSG_USER                 , //  用户管理    SDK_USER_OP_E
    SDK_MAIN_MSG_COMM_CFG			  , //  常规配置       sdk_comm_cfg_t
    SDK_MAIN_MSG_OVERLAY_CFG		  , //  遮挡区域配置   sdk_overlay_cfg_t
    SDK_MAIN_MSG_DISK		  		  , //	磁盘管理	pmsg->args:SDK_DISK_OP_E
    SDK_MAIN_MSG_SERIAL_CFG		      , //	串口参数配置	  sdk_serial_func_cfg_t
    SDK_MAIN_MSG_IMAGE_ATTR_CFG       , //	图像属性配置   sdk_image_attr_t
    SDK_MAIN_MSG_TV_CFG         	  , //	TV设置   sdk_vo_sideSize_t
    SDK_MAIN_MSG_NOTIFY               , //  通知事件发生/停止 args:SDK_EVENT_TYPE_E, chann:通道, extend:发生/停止.
    SDK_MAIN_MSG_DEVLIST              , //  GUI 查询设备列表 sdk_device_t
    SDK_MAIN_MSG_MANUAL_REC		      , //	手动打开(关闭)录像 	sdk_manual_record_t
    SDK_MAIN_MSG_MANUAL_ALMIN		  , //	手动打开(关闭)报警输入 	sdk_manual_alarmin_t
    SDK_MAIN_MSG_MANUAL_ALMOUT	      , //	手动打开(关闭)报警输出  sdk_manual_alarmout_t
    SDK_MAIN_MSG_RECORD_BACKUP	      , //	录像备份  pmsg->args:SDK_BACKUP_OP_E
    SDK_MAIN_MSG_PLATFORM_CFG         , //  平台信息配置
    SDK_MAIN_MSG_CHANGE_REMOTE_IP     , //  改变远程设备的ip sdk_remote_net_t
    SDK_MAIN_MSG_DDNS_CFG			  , //	DDNS配置 sdk_ddns_cfg_t
    SDK_MAIN_MSG_CAPABILITY		      , //	设备能力集信息 sdk_device_capability_t
    SDK_MAIN_MSG_UPNP_CFG             , //  UPNP 配置
    SDK_MAIN_MSG_SCREEN_CFG           , //  screen配置 sdk_screen_t
    SDK_MAIN_MSG_CRUISE_CTL           , //  开启巡航     无对应结构体，只认ID
    SDK_MAIN_MSG_PRESET_SET           , //  预置点集合 sdk_preset_param_t
    SDK_MAIN_MSG_PREVIEW_TOUR         , //  预览巡回配置    sdk_tour_t
    SDK_MAIN_MSG_3A_ATTR_CFG          , //	3A属性配置   sdk_3a_attr_t
    SDK_MAIN_MSG_UPGRAD               , //	升级        SDK_UPGRAD_OP_E
    SDK_MAIN_MSG_SCHED_CFG		      , //	报警排程	   
    SDK_MAIN_MSG_REGIONAL_CFG         , //  区域参数设置 包括hz 语言 室内外等  sdk_regional_param_t
    SDK_MAIN_MSG_IODEV_PARAM		  , //	iodev设备参数  led  蜂鸣器等  sdk_iodev_param_t
    SDK_MAIN_MSG_AUDIO_DETECT_CFG     , //  声音侦测   sdk_audio_detect_cfg_t
    SDK_MAIN_MSG_MIRROR_FLIP_CFG      , //  镜像翻转实现  sdk_mirror_flip_cfg_t  
    SDK_MAIN_MSG_POWER_FREQ_CFG       , // power frequency config msg
    SDK_MAIN_MSG_STOP_SERVER    	  , //  停止 main函数中的所有服务
    SDK_MAIN_MSG_AWS 				  , //  AWS
    SDK_MAIN_MSG_BUTT
}SDK_MAIN_MSG_ID_E;

typedef enum _SDK_MAIN_USR_ID{
    SDK_USER_AAA,
    SDK_USER_ADD,
    SDK_USER_DEL,
    SDK_USER_MODI,
    SDK_USER_QUERY

}_SDK_MAIN_USR_ID_E;

typedef struct __msg_deviceinfo
{
	unsigned int total; 		// 0: No cards been detected or an unrecognizeable sdcard that could not be re-formatted.
								// -1: if camera detect an unrecognizable sdcard, and could be re-formatted
								// otherwise: return total space size of sdcard (MBytes)								
								
	unsigned int free;			// Free space size of sdcard (MBytes)

	uint8_t szMacAddr[24];	 //MAC STRING
	uint8_t szUID[32];
	uint8_t szPwd[32];	
	uint8_t hardware_ver[64];	   //硬件版本
}SDK_MSG_DEVICEINFO;

typedef int(*sdk_msg_dispatch_cb)(sdk_msg_t *pmsg);

int net_dispatch(sdk_msg_t *pmsg);
int msg_ptz_dispatch(sdk_msg_t *pmsg);
char *net_msg2main_msg_str(int msg_id);
int mcu_operate(int oper_type, int arg, void *p_arg);
int dm_msg_send(int fd, int id, int arg, int ch, void *data, int size, int timeout);
int main_free_system_resources();

#endif //__main_h__




