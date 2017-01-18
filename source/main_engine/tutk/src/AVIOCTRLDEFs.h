/*
 * AVIOCTRLDEFs.h
 *	Define AVIOCTRL Message Type and Context
 *  Created on: 2011-08-12
 *  Author: TUTK
 *
 */


#ifndef _AVIOCTRL_DEFINE_H_
#define _AVIOCTRL_DEFINE_H_

#define BYTE 	unsigned char
#define DWORD 	unsigned int 
#define WORD 	unsigned int
#define BOOL 	unsigned int


#define MACADDR_LEN 	20  
#define NAME_LEN      	32
#define PASSWD_LEN 		16


/////////////////////////////////////////////////////////////////////////////////
/////////////////// Message Type Define//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

// AVIOCTRL Message Type
typedef enum 
{
	IOTYPE_USER_IPCAM_START 					= 0x01FF,
	IOTYPE_USER_IPCAM_STOP	 					= 0x02FF,
	IOTYPE_USER_IPCAM_AUDIOSTART 				= 0x0300,
	IOTYPE_USER_IPCAM_AUDIOSTOP 				= 0x0301,

	IOTYPE_USER_IPCAM_SPEAKERSTART 				= 0x0350,
	IOTYPE_USER_IPCAM_SPEAKERSTOP 				= 0x0351,

	IOTYPE_USER_IPCAM_SETSTREAMCTRL_REQ			= 0x0320,
	IOTYPE_USER_IPCAM_SETSTREAMCTRL_RESP		= 0x0321,
	IOTYPE_USER_IPCAM_GETSTREAMCTRL_REQ			= 0x0322,
	IOTYPE_USER_IPCAM_GETSTREAMCTRL_RESP		= 0x0323,

	IOTYPE_USER_IPCAM_SETMOTIONDETECT_REQ		= 0x0324,
	IOTYPE_USER_IPCAM_SETMOTIONDETECT_RESP		= 0x0325,
	IOTYPE_USER_IPCAM_GETMOTIONDETECT_REQ		= 0x0326,
	IOTYPE_USER_IPCAM_GETMOTIONDETECT_RESP		= 0x0327,
	
	IOTYPE_USER_IPCAM_GETSUPPORTSTREAM_REQ		= 0x0328,	// Get Support Stream
	IOTYPE_USER_IPCAM_GETSUPPORTSTREAM_RESP		= 0x0329,	

	IOTYPE_USER_IPCAM_DEVINFO_REQ				= 0x0330,
	IOTYPE_USER_IPCAM_DEVINFO_RESP				= 0x0331,

	IOTYPE_USER_IPCAM_SETPASSWORD_REQ			= 0x0332,
	IOTYPE_USER_IPCAM_SETPASSWORD_RESP			= 0x0333,

	IOTYPE_USER_IPCAM_LISTWIFIAP_REQ			= 0x0340,
	IOTYPE_USER_IPCAM_LISTWIFIAP_RESP			= 0x0341,
	IOTYPE_USER_IPCAM_SETWIFI_REQ				= 0x0342,
	IOTYPE_USER_IPCAM_SETWIFI_RESP				= 0x0343,
	IOTYPE_USER_IPCAM_GETWIFI_REQ				= 0x0344,
	IOTYPE_USER_IPCAM_GETWIFI_RESP				= 0x0345,
	IOTYPE_USER_IPCAM_SETWIFI_REQ_2				= 0x0346,
	IOTYPE_USER_IPCAM_GETWIFI_RESP_2			= 0x0347,

	IOTYPE_USER_IPCAM_SETRECORD_REQ				= 0x0310,
	IOTYPE_USER_IPCAM_SETRECORD_RESP			= 0x0311,
	IOTYPE_USER_IPCAM_GETRECORD_REQ				= 0x0312,
	IOTYPE_USER_IPCAM_GETRECORD_RESP			= 0x0313,

	IOTYPE_USER_IPCAM_SETRCD_DURATION_REQ		= 0x0314,
	IOTYPE_USER_IPCAM_SETRCD_DURATION_RESP  	= 0x0315,
	IOTYPE_USER_IPCAM_GETRCD_DURATION_REQ		= 0x0316,
	IOTYPE_USER_IPCAM_GETRCD_DURATION_RESP  	= 0x0317,

	IOTYPE_USER_IPCAM_LISTEVENT_REQ				= 0x0318,
	IOTYPE_USER_IPCAM_LISTEVENT_RESP			= 0x0319,
	
	IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL 		= 0x031A,
	IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP 	= 0x031B,
	
	IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_REQ		= 0x032A,
	IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_RESP	= 0x032B,

	IOTYPE_USER_IPCAM_GET_EVENTCONFIG_REQ		= 0x0400,	// Get Event Config Msg Request
	IOTYPE_USER_IPCAM_GET_EVENTCONFIG_RESP		= 0x0401,	// Get Event Config Msg Response
	IOTYPE_USER_IPCAM_SET_EVENTCONFIG_REQ		= 0x0402,	// Set Event Config Msg req
	IOTYPE_USER_IPCAM_SET_EVENTCONFIG_RESP		= 0x0403,	// Set Event Config Msg resp

	IOTYPE_USER_IPCAM_SET_ENVIRONMENT_REQ		= 0x0360,
	IOTYPE_USER_IPCAM_SET_ENVIRONMENT_RESP		= 0x0361,
	IOTYPE_USER_IPCAM_GET_ENVIRONMENT_REQ		= 0x0362,
	IOTYPE_USER_IPCAM_GET_ENVIRONMENT_RESP		= 0x0363,
	
	IOTYPE_USER_IPCAM_SET_VIDEOMODE_REQ			= 0x0370,	// Set Video Flip Mode
	IOTYPE_USER_IPCAM_SET_VIDEOMODE_RESP		= 0x0371,
	IOTYPE_USER_IPCAM_GET_VIDEOMODE_REQ			= 0x0372,	// Get Video Flip Mode
	IOTYPE_USER_IPCAM_GET_VIDEOMODE_RESP		= 0x0373,
	
	IOTYPE_USER_IPCAM_FORMATEXTSTORAGE_REQ		= 0x0380,	// Format external storage
	IOTYPE_USER_IPCAM_FORMATEXTSTORAGE_RESP		= 0x0381,	
	
	IOTYPE_USER_IPCAM_PTZ_COMMAND				= 0x1001,	// P2P PTZ Command Msg

	IOTYPE_USER_IPCAM_EVENT_REPORT				= 0x1FFF,	// Device Event Report Msg
	IOTYPE_USER_IPCAM_RECEIVE_FIRST_IFRAME		= 0x1002,	// Send from client, used to talk to device that
															// client had received the first I frame
	
	
	IOTYPE_USER_IPCAM_GET_FLOWINFO_REQ			= 0x0390,
	IOTYPE_USER_IPCAM_GET_FLOWINFO_RESP			= 0x0391,
	IOTYPE_USER_IPCAM_CURRENT_FLOWINFO			= 0x0392,
	
	IOTYPE_USER_IPCAM_GET_TIMEZONE_REQ          = 0x3A0,
	IOTYPE_USER_IPCAM_GET_TIMEZONE_RESP         = 0x3A1,
	IOTYPE_USER_IPCAM_SET_TIMEZONE_REQ          = 0x3B0,
	IOTYPE_USER_IPCAM_SET_TIMEZONE_RESP         = 0x3B1,
    

    // dropbox support
    IOTYPE_USER_IPCAM_GET_SAVE_DROPBOX_REQ      = 0x500,
    IOTYPE_USER_IPCAM_GET_SAVE_DROPBOX_RESP     = 0x501,
    IOTYPE_USER_IPCAM_SET_SAVE_DROPBOX_REQ      = 0x502,
    IOTYPE_USER_IPCAM_SET_SAVE_DROPBOX_RESP     = 0x503,
    
//user defined
	IOTYPE_USER_IPCAM_USER_DEFINED              = 0x40000,

	IOTYPE_USER_IPCAM_GET_ATTRIBUTE_REQ			= 0x40001,	//获取设备支持的属性
	IOTYPE_USER_IPCAM_GET_ATTRIBUTE_RESP		= 0x40002,

	IOTYPE_USER_IPCAM_GET_VIDEO_DISPLAY_REQ		= 0x40003,	//视频显示参数
	IOTYPE_USER_IPCAM_GET_VIDEO_DISPLAY_RESP	= 0x40004,
	IOTYPE_USER_IPCAM_SET_VIDEO_DISPLAY_REQ		= 0x40005,
	IOTYPE_USER_IPCAM_SET_VIDEO_DISPLAY_RESP	= 0x40006,

	IOTYPE_USER_IPCAM_GET_ALARM_ARG_REQ			= 0x40007,	//告警相关的参数
	IOTYPE_USER_IPCAM_GET_ALARM_ARG_RESP		= 0x40008,
	IOTYPE_USER_IPCAM_SET_ALARM_ARG_REQ			= 0x40009,
	IOTYPE_USER_IPCAM_SET_ALARM_ARG_RESP		= 0x40010,

	IOTYPE_USER_IPCAM_GET_EMAIL_ARG_REQ			= 0x40011,	//email相关的参数
	IOTYPE_USER_IPCAM_GET_EMAIL_ARG_RESP		= 0x40012,
	IOTYPE_USER_IPCAM_SET_EMAIL_ARG_REQ			= 0x40013,
	IOTYPE_USER_IPCAM_SET_EMAIL_ARG_RESP		= 0x40014,

	IOTYPE_USER_IPCAM_FACTORY_SETTINGS_REQ		= 0x40015,	//恢复出厂设置
	IOTYPE_USER_IPCAM_FACTORY_SETTINGS_RESP		= 0x40016,

	IOTYPE_USER_IPCAM_SET_DEVICE_INFO_REQ		= 0x40017,	//设置设备信息
	IOTYPE_USER_IPCAM_SET_DEVICE_INFO_RESP		= 0x40018,	

	IOTYPE_USER_IPCAM_REBOOT_SYSTEM_REQ			= 0x40019,	//重启系统
	IOTYPE_USER_IPCAM_REBOOT_SYSTEM_RESP		= 0x40020,

	IOTYPE_USER_IPCAM_GET_PTZ_STATUS_REQ		= 0x40021,	//获取云台状态	
	IOTYPE_USER_IPCAM_GET_PTZ_STATUS_RESP		= 0x40022,	

	IOTYPE_USER_IPCAM_GET_VIDEO_QUALITY_REQ		= 0x40023,	//读取视频质量	
	IOTYPE_USER_IPCAM_GET_VIDEO_QUALITY_RESP	= 0x40024,
	IOTYPE_USER_IPCAM_SET_VIDEO_QUALITY_REQ		= 0x40025,	//设置视频质量	
	IOTYPE_USER_IPCAM_SET_VIDEO_QUALITY_RESP	= 0x40026,

	IOTYPE_USER_IPCAM_TEST_EMAIL_ARG_REQ		= 0X40027,	//测试email参数
	IOTYPE_USER_IPCAM_TEST_EMAIL_ARG_RESP		= 0X40028,
	IOTYPE_USER_IPCAM_UPGRADE_SYSTEM_REQ		= 0X40029,	//升级系统
	IOTYPE_USER_IPCAM_UPGRADE_SYSTEM_RESP		= 0X40030,
    
	IOTYPE_USER_IPCAM_GET_ALARM_LIST_REQ		= 0X40031,	//查询事件列表日志（0x0318用于查询事件录像文件）
	IOTYPE_USER_IPCAM_GET_ALARM_LIST_RESP		= 0X40032,

	IOTYPE_USER_IPCAM_GET_AUDIO_MOTION_REQ		= 0X40033,	//获取声音侦测
	IOTYPE_USER_IPCAM_GET_AUDIO_MOTION_RESP		= 0X40034,
	IOTYPE_USER_IPCAM_SET_AUDIO_MOTION_REQ		= 0X40035,	//设置声音侦测配置
	IOTYPE_USER_IPCAM_SET_AUDIO_MOTION_RESP		= 0X40036,

	IOTYPE_USER_IPCAM_GET_NET_REQ				= 0X40037,	//获取网络配置
	IOTYPE_USER_IPCAM_GET_NET_RESP				= 0X40038,
	IOTYPE_USER_IPCAM_SET_NET_REQ				= 0X40039,	//设置网络配置
	IOTYPE_USER_IPCAM_SET_NET_RESP				= 0X40040,

	IOTYPE_USER_IPCAM_GET_SNAP_REQ				= 0X40041,	//客户端抓拍请求
	IOTYPE_USER_IPCAM_GET_SNAP_RESP				= 0X40042,

	IOTYPE_USER_IPCAM_DEFAULT_VIDEO_REQ			= 0X40043,	//客户端请求默认的视频分辨率、码流、帧率，视频参数
	IOTYPE_USER_IPCAM_DEFAULT_VIDEO_RESP		= 0X40044,
	
	IOTYPE_USER_IPCAM_WIRTE_DEV_CONFIG_REQ		= 0X40045,	//写入设备的配置信息  (mac地址 serid等需要永久保存不删除的)
	IOTYPE_USER_IPCAM_WIRTE_DEV_CONFIG_RESP		= 0X40046,

	IOTYPE_USER_IPCAM_READ_DEV_CONFIG_REQ		= 0X40047,	//读取设备的配置信息
	IOTYPE_USER_IPCAM_READ_DEV_CONFIG_RESP		= 0X40048,

	IOTYPE_USER_IPCAM_RESET_STATUS_RESP			= 0X40049,	//复位键状态变化的通知

	IOTYPE_USER_IPCAM_SET_NTP_CONFIG_REQ		= 0X40050,	//写入ntp的配置信息
	IOTYPE_USER_IPCAM_SET_NTP_CONFIG_RESP		= 0X40051,

	IOTYPE_USER_IPCAM_GET_NTP_CONFIG_REQ		= 0X40052,	//读取ntp的配置信息
	IOTYPE_USER_IPCAM_GET_NTP_CONFIG_RESP		= 0X40053,

	IOTYPE_USER_IPCAM_GET_COMPANY_CONFIG_REQ	= 0X40054,	//读取公司信息
	IOTYPE_USER_IPCAM_GET_COMPANY_CONFIG_RESP	= 0X40055,

	IOTYPE_USER_IPCAM_GET_DEVICEMODEL_CONFIG_REQ	= 0X40056,	//读取装备信息
	IOTYPE_USER_IPCAM_GET_DEVICEMODEL_CONFIG_RESP	= 0X40057,

	IOTYPE_USER_IPCAM_SET_DEVICEMODEL_CONFIG_REQ	= 0X40058,	//设置设备的信息   设备型号  制造商等
	IOTYPE_USER_IPCAM_SET_DEVICEMODEL_CONFIG_RESP	= 0X40059,

	IOTYPE_USER_IPCAM_GET_DROPBOX_CONFIG_REQ	= 0X4005A,	//读取DROPBOX信息
	IOTYPE_USER_IPCAM_GET_DROPBOX_CONFIG_RESP	= 0X4005B,

	IOTYPE_USER_IPCAM_SET_DROPBOX_CONFIG_REQ	= 0X4005C,	//设取DROPBOX信息
	IOTYPE_USER_IPCAM_SET_DROPBOX_CONFIG_RESP	= 0X4005D,

	IOTYPE_USER_IPCAM_GET_UPLOAD_FUNCTION_CONFIG_REQ	= 0X4005e,	//读取DROPBOX功能信息
	IOTYPE_USER_IPCAM_GET_UPLOAD_FUNCTION_CONFIG_RESP	= 0X4005f,

	IOTYPE_USER_IPCAM_SET_UPLOAD_FUNCTION_CONFIG_REQ	= 0X40060,
	IOTYPE_USER_IPCAM_SET_UPLOAD_FUNCTION_CONFIG_RESP	= 0X40061,

	
	IOTYPE_USER_IPCAM_START_OAUTH_CONFIG_REQ	= 0X40062,	//开始认证
	IOTYPE_USER_IPCAM_START_OAUTH_CONFIG_RESP	= 0X40063,

	IOTYPE_USER_IPCAM_OAUTH_OK_CONFIG_REQ	= 0X40064,	//授权OK
	IOTYPE_USER_IPCAM_OAUTH_OK_CONFIG_RESP	= 0X40065,

	IOTYPE_USER_IPCAM_CLOSE_LED_CONFIG_REQ	= 0X40066,	//关闭led指示灯
	IOTYPE_USER_IPCAM_CLOSE_LED_CONFIG_RESP	= 0X40067,

	IOTYPE_USER_IPCAM_CLOSE_LED_STATUS_CONFIG_REQ	= 0X40068,	//led指示灯关闭状态
	IOTYPE_USER_IPCAM_CLOSE_LED_STATUS_CONFIG_RESP	= 0X40069,

	//add by jy
	IOTYPE_USER_IPCAM_GET_PICTURE_REQ			= 0X40070,	// send picture to server
	IOTYPE_USER_IPCAM_GET_PICTURE_RESP			= 0X40071,


	IOTYPE_USER_IPCAM_GET_PICTURE_TO_SHARE_REQ			= 0X40072,	// send picture to server to share
	IOTYPE_USER_IPCAM_GET_PICTURE_TO_SHARE_RESP			= 0X40073,

	IOTYPE_USER_IPCAM_SET_TIMEMODE_TO_SHARE_REQ			= 0X40074,	// set time mode 0 Chinese 1 America 2 Europe
	IOTYPE_USER_IPCAM_SET_TIMEMODE_TO_SHARE_RESP		= 0X40075,

	IOTYPE_USER_IPCAM_GET_TIMEMODE_TO_SHARE_REQ			= 0X40076,	// get time mode 0 Chinese 1 America 2 Europe
	IOTYPE_USER_IPCAM_GET_TIMEMODE_TO_SHARE_RESP		= 0X40077,
	//end

	IOTYPE_USER_IPCAM_GET_MOTION_AREA_REQ			= 0X40078,	// 移动侦测区域 参数获取
	IOTYPE_USER_IPCAM_GET_MOTION_AREA_RESP			= 0X40079,	// 

	IOTYPE_USER_IPCAM_SET_MOTION_AREA_REQ			= 0X4007A,	// 移动侦测区域 参数设置
	IOTYPE_USER_IPCAM_SET_MOTION_AREA_RESP			= 0X4007B,	// 

	IOTYPE_USER_IPCAM_SET_PLAYBACK_RATE_REQ			= 0X4007C,	//设置回放速率
	IOTYPE_USER_IPCAM_SET_PLAYBACK_RATE_RESP		= 0X4007D,	// 
	//录像排程
	IOTYPE_USER_IPCAM_SET_RECORD_SCHEDULE_REQ		 = 0X4007E,	//设置录像排程
	IOTYPE_USER_IPCAM_SET_RECORD_SCHEDULE_RES		 = 0X4007F,	// 

	IOTYPE_USER_IPCAM_GET_RECORD_SCHEDULE_REQ 		= 0X40080,	//读取录像排程
	IOTYPE_USER_IPCAM_GET_RECORD_SCHEDULE_RES 		= 0X40081,	// 

	//新增
	IOTYPE_USER_IPCAM_SET_LED_STATUS_REQ			= 0X40082,	//设置指示灯状态
	IOTYPE_USER_IPCAM_SET_LED_STATUS_RESP			= 0X40083,	// 

	IOTYPE_USER_IPCAM_GET_LED_STATUS_REQ			= 0X40084,	//读取指示灯状态
	IOTYPE_USER_IPCAM_GET_LED_STATUS_RESP			= 0X40085,

	IOTYPE_USER_IPCAM_SET_BUZZER_STATUS_REQ			= 0X40086,	//设置蜂鸣器状态
	IOTYPE_USER_IPCAM_SET_BUZZER_STATUS_RESP		= 0X40087,	// 

	IOTYPE_USER_IPCAM_GET_BUZZER_STATUS_REQ			= 0X40088,	//读取蜂鸣器状态
	IOTYPE_USER_IPCAM_GET_BUZZER_STATUS_RESP		= 0X40089,
	//FOR AWS
	// AWS support 亚马逊录像上传功能 查询和设置 
    IOTYPE_USER_IPCAM_GET_AWS_STATUS_REQ      = 0X4008A,
    IOTYPE_USER_IPCAM_GET_AWS_STATUS_RESP     = 0X4008B,
    IOTYPE_USER_IPCAM_SET_AWS_STATUS_REQ      = 0X4008C,
    IOTYPE_USER_IPCAM_SET_AWS_STATUS_RESP     = 0X4008D,

	 // REMOTE UPGRADE   升级请求
    IOTYPE_USER_IPCAM_REMOTE_UPGRADE_REQ      = 0X4008E,
    IOTYPE_USER_IPCAM_REMOTE_UPGRADE_RESP     = 0X4008F,
    
    // UPGRADE PROGRESS  升级进度查询
    IOTYPE_USER_IPCAM_UPGRADE_PROGRESS_REQ      = 0X40090,
    IOTYPE_USER_IPCAM_UPGRADE_PROGRESS_RESP     = 0X40091,

	/*
	20150408
	1 为了兼容性，旧有的消息ID限定范围是0X40000 ~0X49999，应该能满足目前使用需求
	2 新增ID 配合新的方案使用，比如用在百度的测试工具中
	 增量很少每个ID需要协商确定，ID限定范围是:0X50000 ~0X59999， 
	3 新增ID的顺序 和main.h 中的SDK_MAIN_MSG_ID_E 定义尽量保持一致
	4 新增的ID 配套使用的结构体，尽量和sdk_struct.h  中的结构体保持一致
	*/

	IOTYPE_USER_IPCAM_GET_MDSYS_INFO_REQ			= 0X50001,	//获取魔蛋系统参数(生产厂家 版本 等等) sdk_sys_cfg_t
	IOTYPE_USER_IPCAM_GET_MDSYS_INFO_RESP		    = 0X50002,
	//IOTYPE_USER_IPCAM_GET_BAIDU_INFO_REQ			= 0X50003,	//获取百度相关信息  sdk_register_cfg_t
	//IOTYPE_USER_IPCAM_GET_BAIDU_INFO_RESP		    = 0X50004,
	IOTYPE_USER_IPCAM_GET_MAC_ADDR_REQ				= 0X50003,
	IOTYPE_USER_IPCAM_GET_MAC_ADDR_RESP				= 0X50004,
	IOTYPE_USER_CMD_MAX
	}ENUM_AVIOCTRL_MSGTYPE;


/////////////////////////////////////////////////////////////////////////////////
/////////////////// Type ENUM Define ////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
typedef enum
{
	AVIOCTRL_OK					= 0x00,
	AVIOCTRL_ERR				= -0x01,
	AVIOCTRL_ERR_PASSWORD		= AVIOCTRL_ERR - 0x01,
	AVIOCTRL_ERR_STREAMCTRL		= AVIOCTRL_ERR - 0x02,
	AVIOCTRL_ERR_MONTIONDETECT	= AVIOCTRL_ERR - 0x03,
	AVIOCTRL_ERR_DEVICEINFO		= AVIOCTRL_ERR - 0x04,
	AVIOCTRL_ERR_LOGIN			= AVIOCTRL_ERR - 5,
	AVIOCTRL_ERR_LISTWIFIAP		= AVIOCTRL_ERR - 6,
	AVIOCTRL_ERR_SETWIFI		= AVIOCTRL_ERR - 7,
	AVIOCTRL_ERR_GETWIFI		= AVIOCTRL_ERR - 8,
	AVIOCTRL_ERR_SETRECORD		= AVIOCTRL_ERR - 9,
	AVIOCTRL_ERR_SETRCDDURA		= AVIOCTRL_ERR - 10,
	AVIOCTRL_ERR_LISTEVENT		= AVIOCTRL_ERR - 11,
	AVIOCTRL_ERR_PLAYBACK		= AVIOCTRL_ERR - 12,

	AVIOCTRL_ERR_INVALIDCHANNEL	= AVIOCTRL_ERR - 0x20,
}ENUM_AVIOCTRL_ERROR; //APP don't use it now


// ServType, unsigned long, 32 bits, is a bit mask for function declareation
// bit value "0" means function is valid or enabled
// in contract, bit value "1" means function is invalid or disabled.
// ** for more details, see "ServiceType Definitation for AVAPIs"
// 
// Defined bits are listed below:
//----------------------------------------------
// bit		fuction
// 0		Audio in, from Device to Mobile
// 1		Audio out, from Mobile to Device 
// 2		PT function
// 3		Event List function
// 4		Play back function (require Event List function)
// 5		Wi-Fi setting function
// 6		Event Setting Function
// 7		Recording Setting function
// 8		SDCard formattable function
// 9		Video flip function
// 10		Environment mode
// 11		Multi-stream selectable
// 12		Audio out encoding format

// The original enum below is obsoleted.
typedef enum
{
	SERVTYPE_IPCAM_DWH					= 0x00,
	SERVTYPE_RAS_DWF					= 0x01,
	SERVTYPE_IOTCAM_8125				= 0x10,
	SERVTYPE_IOTCAM_8125PT				= 0x11,
	SERVTYPE_IOTCAM_8126				= 0x12,
	SERVTYPE_IOTCAM_8126PT				= 0x13,	
}ENUM_SERVICE_TYPE;

// AVIOCTRL Quality Type
typedef enum 
{
//手机用
	AVIOCTRL_QUALITY_UNKNOWN			= 0x00,	
	AVIOCTRL_QUALITY_MAX				= 0x01,	// ex. 640*480, 15fps, 320kbps (or 1280x720, 5fps, 320kbps)
	AVIOCTRL_QUALITY_HIGH				= 0x02,	// ex. 640*480, 10fps, 256kbps
	AVIOCTRL_QUALITY_MIDDLE				= 0x03,	// ex. 320*240, 15fps, 256kbps
	AVIOCTRL_QUALITY_LOW				= 0x04, // ex. 320*240, 10fps, 128kbps
	AVIOCTRL_QUALITY_MIN				= 0x05,	// ex. 160*120, 10fps, 64kbps

//cms用

	AVIOCTRL_QUALITY_CMS1				= 0x0A,	
	AVIOCTRL_QUALITY_CMS2				= 0x0B,	// ex. 640*480, 15fps, 320kbps (or 1280x720, 5fps, 320kbps)
	AVIOCTRL_QUALITY_CMS3				= 0x0C,	// ex. 640*480, 10fps, 256kbps
	AVIOCTRL_QUALITY_CMS4				= 0x0D,	// ex. 320*240, 15fps, 256kbps
	AVIOCTRL_QUALITY_CMS5				= 0x0E, // ex. 320*240, 10fps, 128kbps

}ENUM_QUALITY_LEVEL;


typedef enum
{
	AVIOTC_WIFIAPMODE_NULL				= 0x00,
	AVIOTC_WIFIAPMODE_MANAGED			= 0x01,
	AVIOTC_WIFIAPMODE_ADHOC				= 0x02,
}ENUM_AP_MODE;


typedef enum
{
	AVIOTC_WIFIAPENC_INVALID			= 0x00, 
	AVIOTC_WIFIAPENC_NONE				= 0x01, //
	AVIOTC_WIFIAPENC_WEP				= 0x02, //WEP, for no password
	AVIOTC_WIFIAPENC_WPA_TKIP			= 0x03, 
	AVIOTC_WIFIAPENC_WPA_AES			= 0x04, 
	AVIOTC_WIFIAPENC_WPA2_TKIP			= 0x05, 
	AVIOTC_WIFIAPENC_WPA2_AES			= 0x06,

	AVIOTC_WIFIAPENC_WPA_PSK_TKIP  = 0x07,
	AVIOTC_WIFIAPENC_WPA_PSK_AES   = 0x08,
	AVIOTC_WIFIAPENC_WPA2_PSK_TKIP = 0x09,
	AVIOTC_WIFIAPENC_WPA2_PSK_AES  = 0x0A,

}ENUM_AP_ENCTYPE;


// AVIOCTRL Event Type
typedef enum 
{
	AVIOCTRL_EVENT_ALL					= 0x00,	// all event type(general APP-->IPCamera)
	AVIOCTRL_EVENT_MOTIONDECT			= 0x01,	// motion detect start//==s==
	AVIOCTRL_EVENT_VIDEOLOST			= 0x02,	// video lost alarm
	AVIOCTRL_EVENT_IOALARM				= 0x03, // io alarmin start //---s--

	AVIOCTRL_EVENT_MOTIONPASS			= 0x04, // motion detect end  //==e==
	AVIOCTRL_EVENT_VIDEORESUME			= 0x05,	// video resume
	AVIOCTRL_EVENT_IOALARMPASS			= 0x06, // IO alarmin end   //---e--

	AVIOCTRL_EVENT_EXPT_REBOOT			= 0x10, // system exception reboot
	AVIOCTRL_EVENT_SDFAULT				= 0x11, // sd record exception
	AVIOCTRL_EVENT_AUDIO_MOTION			= 0x12, //
}ENUM_EVENTTYPE;

// AVIOCTRL Record Type
typedef enum
{
	AVIOTC_RECORDTYPE_OFF				= 0x00,
	AVIOTC_RECORDTYPE_FULLTIME			= 0x01,
	AVIOTC_RECORDTYPE_ALARM				= 0x02,
	AVIOTC_RECORDTYPE_MANUAL			= 0x03,
}ENUM_RECORD_TYPE;

// AVIOCTRL Play Record Command
typedef enum 
{
	AVIOCTRL_RECORD_PLAY_PAUSE			= 0x00,
	AVIOCTRL_RECORD_PLAY_STOP			= 0x01,
	AVIOCTRL_RECORD_PLAY_STEPFORWARD	= 0x02, //now, APP no use
	AVIOCTRL_RECORD_PLAY_STEPBACKWARD	= 0x03, //now, APP no use
	AVIOCTRL_RECORD_PLAY_FORWARD		= 0x04, //now, APP no use
	AVIOCTRL_RECORD_PLAY_BACKWARD		= 0x05, //now, APP no use
	AVIOCTRL_RECORD_PLAY_SEEKTIME		= 0x06, //now, APP no use
	AVIOCTRL_RECORD_PLAY_END			= 0x07,
	AVIOCTRL_RECORD_PLAY_START			= 0x10,
}ENUM_PLAYCONTROL;

// AVIOCTRL Environment Mode
typedef enum
{
	AVIOCTRL_ENVIRONMENT_INDOOR_50HZ 	= 0x00,
	AVIOCTRL_ENVIRONMENT_INDOOR_60HZ	= 0x01,
	AVIOCTRL_ENVIRONMENT_OUTDOOR		= 0x02,
	AVIOCTRL_ENVIRONMENT_NIGHT			= 0x03,	
}ENUM_ENVIRONMENT_MODE;

// AVIOCTRL Video Flip Mode
typedef enum
{
	AVIOCTRL_VIDEOMODE_NORMAL 			= 0x00,
	AVIOCTRL_VIDEOMODE_FLIP				= 0x01,
	AVIOCTRL_VIDEOMODE_MIRROR			= 0x02,
	AVIOCTRL_VIDEOMODE_FLIP_MIRROR 		= 0x03,
}ENUM_VIDEO_MODE;

// AVIOCTRL PTZ Command Value
typedef enum 
{
	AVIOCTRL_PTZ_STOP					= 0,
	AVIOCTRL_PTZ_UP						= 1,
	AVIOCTRL_PTZ_DOWN					= 2,
	AVIOCTRL_PTZ_LEFT					= 3,
	AVIOCTRL_PTZ_LEFT_UP				= 4,
	AVIOCTRL_PTZ_LEFT_DOWN				= 5,
	AVIOCTRL_PTZ_RIGHT					= 6, 
	AVIOCTRL_PTZ_RIGHT_UP				= 7, 
	AVIOCTRL_PTZ_RIGHT_DOWN				= 8, 
	AVIOCTRL_PTZ_AUTO					= 9, 
	AVIOCTRL_PTZ_SET_POINT				= 10,
	AVIOCTRL_PTZ_CLEAR_POINT			= 11,
	AVIOCTRL_PTZ_GOTO_POINT				= 12,

	AVIOCTRL_PTZ_SET_MODE_START			= 13,
	AVIOCTRL_PTZ_SET_MODE_STOP			= 14,
	AVIOCTRL_PTZ_MODE_RUN				= 15,

	AVIOCTRL_PTZ_MENU_OPEN				= 16, 
	AVIOCTRL_PTZ_MENU_EXIT				= 17,
	AVIOCTRL_PTZ_MENU_ENTER				= 18,

	AVIOCTRL_PTZ_FLIP					= 19,
	AVIOCTRL_PTZ_START					= 20,

	AVIOCTRL_LENS_APERTURE_OPEN			= 21,
	AVIOCTRL_LENS_APERTURE_CLOSE		= 22,

	AVIOCTRL_LENS_ZOOM_IN				= 23, 
	AVIOCTRL_LENS_ZOOM_OUT				= 24,

	AVIOCTRL_LENS_FOCAL_NEAR			= 25,
	AVIOCTRL_LENS_FOCAL_FAR				= 26,

	AVIOCTRL_AUTO_PAN_SPEED				= 27,
	AVIOCTRL_AUTO_PAN_LIMIT				= 28,
	AVIOCTRL_AUTO_PAN_START				= 29,

	AVIOCTRL_PATTERN_START				= 30,
	AVIOCTRL_PATTERN_STOP				= 31,
	AVIOCTRL_PATTERN_RUN				= 32,

	AVIOCTRL_SET_AUX					= 33,
	AVIOCTRL_CLEAR_AUX					= 34,
	AVIOCTRL_MOTOR_RESET_POSITION		= 35,
	AVIOCTRL_PTZ_AUTO_LEFT_RIGHT		= 36,	//左右巡航
	AVIOCTRL_PTZ_AUTO_UP_DOWN			= 37,	//上下巡航
	AVIOCTRL_PTZ_AUTO_LEFT_RIGHT_STOP	= 38,	//左右巡航
	AVIOCTRL_PTZ_AUTO_UP_DOWN_STOP		= 39,	//上下巡航
	AVIOCTRL_PTZ_AUTO_TRACK				= 40,	//轨迹巡航

	AVIOCTRL_PTZ_AUTO_LEFT_RIGHT_EX		= 41,	//左右巡航(不自动停止)
	AVIOCTRL_PTZ_AUTO_UP_DOWN_EX		= 42,	//上下巡航(不自动停止)
}ENUM_PTZCMD;



/////////////////////////////////////////////////////////////////////////////
///////////////////////// Message Body Define ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/*
IOTYPE_USER_IPCAM_START 				= 0x01FF,
IOTYPE_USER_IPCAM_STOP	 				= 0x02FF,
IOTYPE_USER_IPCAM_AUDIOSTART 			= 0x0300,
IOTYPE_USER_IPCAM_AUDIOSTOP 			= 0x0301,
IOTYPE_USER_IPCAM_SPEAKERSTART 			= 0x0350,
IOTYPE_USER_IPCAM_SPEAKERSTOP 			= 0x0351,
** @struct SMsgAVIoctrlAVStream
*/
typedef struct
{
	unsigned int channel; // Camera Index
	unsigned char reserved[4];
} SMsgAVIoctrlAVStream;


/*
IOTYPE_USER_IPCAM_GETSTREAMCTRL_REQ		= 0x0322,
** @struct SMsgAVIoctrlGetStreamCtrlReq
*/
typedef struct
{
	unsigned int channel;	// Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetStreamCtrlReq;

/*
IOTYPE_USER_IPCAM_SETSTREAMCTRL_REQ		= 0x0320,
IOTYPE_USER_IPCAM_GETSTREAMCTRL_RESP	= 0x0323,
** @struct SMsgAVIoctrlSetStreamCtrlReq, SMsgAVIoctrlGetStreamCtrlResq
*/
typedef struct
{
	unsigned int  channel;	// Camera Index
	unsigned char quality;	//refer to ENUM_QUALITY_LEVEL
	unsigned char reserved[3];
} SMsgAVIoctrlSetStreamCtrlReq, SMsgAVIoctrlGetStreamCtrlResq;

/*
IOTYPE_USER_IPCAM_SETSTREAMCTRL_RESP	= 0x0321,
** @struct SMsgAVIoctrlSetStreamCtrlResp
*/
typedef struct
{
	int result;	// 0: success; otherwise: failed.
	unsigned char reserved[4];
}SMsgAVIoctrlSetStreamCtrlResp;


/*
IOTYPE_USER_IPCAM_GETMOTIONDETECT_REQ	= 0x0326,
** @struct SMsgAVIoctrlGetMotionDetectReq
*/
typedef struct
{
	unsigned int channel; 	// Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetMotionDetectReq;


/*
IOTYPE_USER_IPCAM_SETMOTIONDETECT_REQ		= 0x0324,
IOTYPE_USER_IPCAM_GETMOTIONDETECT_RESP		= 0x0327,
** @struct SMsgAVIoctrlSetMotionDetectReq, SMsgAVIoctrlGetMotionDetectResp
*/
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned int sensitivity; 	// 0(Disabled) ~ 100(MAX):
								// index		sensitivity value
								// 0			0
								// 1			25
								// 2			50
								// 3			75
								// 4			100
}SMsgAVIoctrlSetMotionDetectReq, SMsgAVIoctrlGetMotionDetectResp;


/*
IOTYPE_USER_IPCAM_SETMOTIONDETECT_RESP	= 0x0325,
** @struct SMsgAVIoctrlSetMotionDetectResp
*/
typedef struct
{
	int result;	// 0: success; otherwise: failed.
	unsigned char reserved[4];
}SMsgAVIoctrlSetMotionDetectResp;


/*
IOTYPE_USER_IPCAM_DEVINFO_REQ			= 0x0330,
** @struct SMsgAVIoctrlDeviceInfoReq
*/
typedef struct
{
	unsigned char reserved[4];
}SMsgAVIoctrlDeviceInfoReq;


/*
IOTYPE_USER_IPCAM_DEVINFO_RESP			= 0x0331,
** @struct SMsgAVIoctrlDeviceInfo
*/
typedef struct
{
	unsigned char model[16];	// IPCam mode
	unsigned char vendor[16];	// IPCam manufacturer
	unsigned int version;		// IPCam firmware version	ex. v1.2.3.4 => 0x01020304;  v1.0.0.2 => 0x01000002
	unsigned int channel;		// Camera index
	unsigned int total;			// 0: No cards been detected or an unrecognizeable sdcard that could not be re-formatted.
								// -1: if camera detect an unrecognizable sdcard, and could be re-formatted
								// otherwise: return total space size of sdcard (MBytes)								
								
	unsigned int free;			// Free space size of sdcard (MBytes)
	unsigned char reserved[8];	// reserved
}SMsgAVIoctrlDeviceInfoResp;

/*
IOTYPE_USER_IPCAM_SETPASSWORD_REQ		= 0x0332,
** @struct SMsgAVIoctrlSetPasswdReq
*/
typedef struct
{
	char oldpasswd[32];			// The old security code
	char newpasswd[32];			// The new security code
}SMsgAVIoctrlSetPasswdReq;


/*
IOTYPE_USER_IPCAM_SETPASSWORD_RESP		= 0x0333,
** @struct SMsgAVIoctrlSetPasswdResp
*/
typedef struct
{
	int result;	// 0: success; otherwise: failed.
	unsigned char reserved[4];
}SMsgAVIoctrlSetPasswdResp;


/*
IOTYPE_USER_IPCAM_LISTWIFIAP_REQ		= 0x0340,
** @struct SMsgAVIoctrlListWifiApReq
*/
typedef struct
{
	unsigned char reserved[4];
}SMsgAVIoctrlListWifiApReq;

typedef struct
{
	char ssid[32]; 				// WiFi ssid
	char mode;	   				// refer to ENUM_AP_MODE
	char enctype;  				// refer to ENUM_AP_ENCTYPE
	char signal;   				// signal intensity 0--100%
	char status;   				// 0 : invalid ssid or disconnected
								// 1 : connected with default gateway
								// 2 : unmatched password
								// 3 : weak signal and connected
								// 4 : selected:
								//		- password matched and
								//		- disconnected or connected but not default gateway
}SWifiAp;

/*
IOTYPE_USER_IPCAM_LISTWIFIAP_RESP		= 0x0341,
** @struct SMsgAVIoctrlListWifiApResp
*/
typedef struct
{
	unsigned int number; // MAX number: 1024(IOCtrl packet size) / 36(bytes) = 28
	SWifiAp stWifiAp[0];
}SMsgAVIoctrlListWifiApResp;

/*
IOTYPE_USER_IPCAM_SETWIFI_REQ			= 0x0342,
** @struct SMsgAVIoctrlSetWifiReq
*/
typedef struct
{
	unsigned char ssid[32];			//WiFi ssid
	unsigned char password[32];		//if exist, WiFi password
	unsigned char mode;				//refer to ENUM_AP_MODE
	unsigned char enctype;			//refer to ENUM_AP_ENCTYPE
	unsigned char reserved[10];
}SMsgAVIoctrlSetWifiReq;

//IOTYPE_USER_IPCAM_SETWIFI_REQ_2		= 0x0346,
typedef struct
{
	unsigned char ssid[32];		// WiFi ssid
	unsigned char password[64];	// if exist, WiFi password
	unsigned char mode;			// refer to ENUM_AP_MODE
	unsigned char enctype;		// refer to ENUM_AP_ENCTYPE
	unsigned char reserved[10];
}SMsgAVIoctrlSetWifiReq2;

/*
IOTYPE_USER_IPCAM_SETWIFI_RESP			= 0x0343,
** @struct SMsgAVIoctrlSetWifiResp
*/
typedef struct
{
	int result; //0: wifi connected; 1: failed to connect
	unsigned char reserved[4];
}SMsgAVIoctrlSetWifiResp;

/*
IOTYPE_USER_IPCAM_GETWIFI_REQ			= 0x0344,
** @struct SMsgAVIoctrlGetWifiReq
*/
typedef struct
{
	unsigned char reserved[4];
}SMsgAVIoctrlGetWifiReq;

/*
IOTYPE_USER_IPCAM_GETWIFI_RESP			= 0x0345,
** @struct SMsgAVIoctrlGetWifiResp //if no wifi connected, members of SMsgAVIoctrlGetWifiResp are all 0
*/
typedef struct
{
	unsigned char ssid[32];		// WiFi ssid
	unsigned char password[32]; // WiFi password if not empty
	unsigned char mode;			// refer to ENUM_AP_MODE
	unsigned char enctype;		// refer to ENUM_AP_ENCTYPE
	unsigned char signal;		// signal intensity 0--100%
	unsigned char status;		// refer to "status" of SWifiAp
}SMsgAVIoctrlGetWifiResp;

//changed: WI-FI Password 32bit Change to 64bit 
//IOTYPE_USER_IPCAM_GETWIFI_RESP_2    = 0x0347,
typedef struct
{
 unsigned char ssid[32];	 // WiFi ssid
 unsigned char password[64]; // WiFi password if not empty
 unsigned char mode;	// refer to ENUM_AP_MODE
 unsigned char enctype; // refer to ENUM_AP_ENCTYPE
 unsigned char signal;  // signal intensity 0--100%
 unsigned char status;  // refer to "status" of SWifiAp
}SMsgAVIoctrlGetWifiResp2;

/*
IOTYPE_USER_IPCAM_GETRECORD_REQ			= 0x0312,
** @struct SMsgAVIoctrlGetRecordReq
*/
typedef struct
{
	unsigned int channel; // Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetRecordReq;

/*
IOTYPE_USER_IPCAM_SETRECORD_REQ			= 0x0310,
IOTYPE_USER_IPCAM_GETRECORD_RESP		= 0x0313,
** @struct SMsgAVIoctrlSetRecordReq, SMsgAVIoctrlGetRecordResq
*/
typedef struct
{
	unsigned int channel;		// Camera Index
	unsigned int recordType;	// Refer to ENUM_RECORD_TYPE
	unsigned char reserved[4];
}SMsgAVIoctrlSetRecordReq, SMsgAVIoctrlGetRecordResq;

/*
IOTYPE_USER_IPCAM_SETRECORD_RESP		= 0x0311,
** @struct SMsgAVIoctrlSetRecordResp
*/
typedef struct
{
	int result;	// 0: success; otherwise: failed.
	unsigned char reserved[4];
}SMsgAVIoctrlSetRecordResp;


/*
IOTYPE_USER_IPCAM_GETRCD_DURATION_REQ	= 0x0316,
** @struct SMsgAVIoctrlGetRcdDurationReq
*/
typedef struct
{
	unsigned int channel; // Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetRcdDurationReq;

/*
IOTYPE_USER_IPCAM_SETRCD_DURATION_REQ	= 0x0314,
IOTYPE_USER_IPCAM_GETRCD_DURATION_RESP  = 0x0317,
** @struct SMsgAVIoctrlSetRcdDurationReq, SMsgAVIoctrlGetRcdDurationResp
*/
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned int presecond; 	// pre-recording (sec)
	unsigned int durasecond;	// recording (sec)
}SMsgAVIoctrlSetRcdDurationReq, SMsgAVIoctrlGetRcdDurationResp;


/*
IOTYPE_USER_IPCAM_SETRCD_DURATION_RESP  = 0x0315,
** @struct SMsgAVIoctrlSetRcdDurationResp
*/
typedef struct
{
	int result;	// 0: success; otherwise: failed.
	unsigned char reserved[4];
}SMsgAVIoctrlSetRcdDurationResp;


typedef struct
{
	unsigned short year;	// The number of year.
	unsigned char month;	// The number of months since January, in the range 1 to 12.
	unsigned char day;		// The day of the month, in the range 1 to 31.
	unsigned char wday;		// The number of days since Sunday, in the range 0 to 6. (Sunday = 0, Monday = 1, ...)
	unsigned char hour;     // The number of hours past midnight, in the range 0 to 23.
	unsigned char minute;   // The number of minutes after the hour, in the range 0 to 59.
	unsigned char second;   // The number of seconds after the minute, in the range 0 to 59.
}STimeDay;

/*
IOTYPE_USER_IPCAM_LISTEVENT_REQ			= 0x0318,
** @struct SMsgAVIoctrlListEventReq
*/
typedef struct
{
	unsigned int channel; 		// Camera Index
	STimeDay stStartTime; 		// Search event from ...
	STimeDay stEndTime;	  		// ... to (search event)
	unsigned char event;  		// event type, refer to ENUM_EVENTTYPE
	unsigned char status; 		// 0x00: Recording file exists, Event unreaded
								// 0x01: Recording file exists, Event readed
								// 0x02: No Recording file in the event
	unsigned char reserved[2];
}SMsgAVIoctrlListEventReq;


typedef struct
{
	STimeDay stTime;
	unsigned char event;
	unsigned char status;	// 0x00: Recording file exists, Event unreaded
							// 0x01: Recording file exists, Event readed
							// 0x02: No Recording file in the event
	unsigned char reserved[2];
}SAvEvent;
	
/*
IOTYPE_USER_IPCAM_LISTEVENT_RESP		= 0x0319,
** @struct SMsgAVIoctrlListEventResp
*/
#define MAX_VIDEO_NUM 100

typedef struct
{
	unsigned int  channel;		// Camera Index
	unsigned int  total;		// Total event amount in this search session
	unsigned char index;		// package index, 0,1,2...; 
								// because avSendIOCtrl() send package up to 1024 bytes one time, you may want split search results to serveral package to send.
	unsigned char endflag;		// end flag; endFlag = 1 means this package is the last one.
	unsigned char count;		// how much events in this package
	unsigned char reserved[1];
	SAvEvent stEvent[0];		// The first memory address of the events in this package
	int videotime[MAX_VIDEO_NUM];  //max is 100
}SMsgAVIoctrlListEventResp;

	
/*
IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL 	= 0x031A,
** @struct SMsgAVIoctrlPlayRecord
*/
typedef struct
{
	unsigned int channel;	// Camera Index
	unsigned int command;	// play record command. refer to ENUM_PLAYCONTROL
	unsigned int Param;		// command param, that the user defined
	STimeDay stTimeDay;		// Event time from ListEvent
	unsigned char reserved[4];
} SMsgAVIoctrlPlayRecord;

/*
IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP 	= 0x031B,
** @struct SMsgAVIoctrlPlayRecordResp
*/
typedef struct
{
	unsigned int command;	// Play record command. refer to ENUM_PLAYCONTROL
	unsigned int result; 	// Depends on command
							// when is AVIOCTRL_RECORD_PLAY_START:
							//	result>=0   real channel no used by device for playback
							//	result <0	error
							//			-1	playback error
							//			-2	exceed max allow client amount
	unsigned char reserved[4];
} SMsgAVIoctrlPlayRecordResp; // only for play record start command


/*
IOTYPE_USER_IPCAM_PTZ_COMMAND	= 0x1001,	// P2P Ptz Command Msg 
** @struct SMsgAVIoctrlPtzCmd
*/
typedef struct
{
	unsigned char control;	// PTZ control command, refer to ENUM_PTZCMD
	unsigned char speed;	// PTZ control speed
	unsigned char point;	// no use in APP so far. preset position, for RS485 PT
	unsigned char limit;	// no use in APP so far. 
	unsigned char aux;		// no use in APP so far. auxiliary switch, for RS485 PT
	unsigned char channel;	// camera index
	unsigned char reserve[2];
} SMsgAVIoctrlPtzCmd;

/*
IOTYPE_USER_IPCAM_EVENT_REPORT	= 0x1FFF,	// Device Event Report Msg 
*/
/** @struct SMsgAVIoctrlEvent
 */
typedef struct
{
	STimeDay stTime;
	unsigned long time; 	// UTC Time
	unsigned int  channel; 	// Camera Index
	unsigned int  event; 	// Event Type
	unsigned char reserved[4];
} SMsgAVIoctrlEvent;



#if 0

/* 	IOTYPE_USER_IPCAM_GET_EVENTCONFIG_REQ	= 0x0400,	// Get Event Config Msg Request 
 */
/** @struct SMsgAVIoctrlGetEventConfig
 */
typedef struct
{
	unsigned int	channel; 		  //Camera Index
	unsigned char   externIoOutIndex; //extern out index: bit0->io0 bit1->io1 ... bit7->io7;=1: get this io value or not get
    unsigned char   externIoInIndex;  //extern in index: bit0->io0 bit1->io1 ... bit7->io7; =1: get this io value or not get
	char reserved[2];
} SMsgAVIoctrlGetEventConfig;
 
/*
	IOTYPE_USER_IPCAM_GET_EVENTCONFIG_RESP	= 0x0401,	// Get Event Config Msg Response 
	IOTYPE_USER_IPCAM_SET_EVENTCONFIG_REQ	= 0x0402,	// Set Event Config Msg req 
*/
/* @struct SMsgAVIoctrlSetEventConfig
 * @struct SMsgAVIoctrlGetEventCfgResp
 */
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned char mail;			// enable send email
	unsigned char ftp;			// enable ftp upload photo
	unsigned char localIO;		// enable local io output
	unsigned char p2pPushMsg;	// enable p2p push msg
	char reserved[4];
} SMsgAVIoctrlSetEventConfig, SMsgAVIoctrlGetEventCfgResp;

/*
	IOTYPE_USER_IPCAM_SET_EVENTCONFIG_RESP	= 0x0403,	// Set Event Config Msg resp 
*/
/** @struct SMsgAVIoctrlSetEventCfgResp
 */
typedef struct
{
	unsigned int channel; 	// Camera Index
	unsigned int result;	// 0: success; otherwise: failed.
}SMsgAVIoctrlSetEventCfgResp;

#endif


/*
IOTYPE_USER_IPCAM_SET_ENVIRONMENT_REQ		= 0x0360,
** @struct SMsgAVIoctrlSetEnvironmentReq
*/
typedef struct
{
	unsigned int channel;		// Camera Index
	unsigned char mode;			// refer to ENUM_ENVIRONMENT_MODE
	unsigned char reserved[3];
}SMsgAVIoctrlSetEnvironmentReq;


/*
IOTYPE_USER_IPCAM_SET_ENVIRONMENT_RESP		= 0x0361,
** @struct SMsgAVIoctrlSetEnvironmentResp
*/
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned char result;		// 0: success; otherwise: failed.
	unsigned char reserved[3];
}SMsgAVIoctrlSetEnvironmentResp;


/*
IOTYPE_USER_IPCAM_GET_ENVIRONMENT_REQ		= 0x0362,
** @struct SMsgAVIoctrlGetEnvironmentReq
*/
typedef struct
{
	unsigned int channel; 	// Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetEnvironmentReq;

/*
IOTYPE_USER_IPCAM_GET_ENVIRONMENT_RESP		= 0x0363,
** @struct SMsgAVIoctrlGetEnvironmentResp
*/
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned char mode;			// refer to ENUM_ENVIRONMENT_MODE
	unsigned char reserved[3];
}SMsgAVIoctrlGetEnvironmentResp;


/*
IOTYPE_USER_IPCAM_SET_VIDEOMODE_REQ			= 0x0370,
** @struct SMsgAVIoctrlSetVideoModeReq
*/
typedef struct
{
	unsigned int channel;	// Camera Index
	unsigned char mode;		// refer to ENUM_VIDEO_MODE
	unsigned char reserved[3];
}SMsgAVIoctrlSetVideoModeReq;


/*
IOTYPE_USER_IPCAM_SET_VIDEOMODE_RESP		= 0x0371,
** @struct SMsgAVIoctrlSetVideoModeResp
*/
typedef struct
{
	unsigned int channel; 	// Camera Index
	unsigned char result;	// 0: success; otherwise: failed.
	unsigned char reserved[3];
}SMsgAVIoctrlSetVideoModeResp;


/*
IOTYPE_USER_IPCAM_GET_VIDEOMODE_REQ			= 0x0372,
** @struct SMsgAVIoctrlGetVideoModeReq
*/
typedef struct
{
	unsigned int channel; 	// Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetVideoModeReq;


/*
IOTYPE_USER_IPCAM_GET_VIDEOMODE_RESP		= 0x0373,
** @struct SMsgAVIoctrlGetVideoModeResp
*/
typedef struct
{
	unsigned int  channel; 	// Camera Index
	unsigned char mode;		// refer to ENUM_VIDEO_MODE
	unsigned char reserved[3];
}SMsgAVIoctrlGetVideoModeResp;


/*
/IOTYPE_USER_IPCAM_FORMATEXTSTORAGE_REQ			= 0x0380,
** @struct SMsgAVIoctrlFormatExtStorageReq
*/
typedef struct
{
	unsigned int storage; 	// Storage index (ex. sdcard slot = 0, internal flash = 1, ...)
	unsigned char reserved[4];
}SMsgAVIoctrlFormatExtStorageReq;


/*
IOTYPE_USER_IPCAM_FORMATEXTSTORAGE_REQ		= 0x0381,
** @struct SMsgAVIoctrlFormatExtStorageResp
*/
typedef struct
{
	unsigned int  storage; 	// Storage index
	unsigned char result;	// 0: success;
							// -1: format command is not supported.
							// otherwise: failed.
	unsigned char reserved[3];
}SMsgAVIoctrlFormatExtStorageResp;


typedef struct
{
	unsigned short index;		// the stream index of camera
	unsigned short channel;		// the channel index used in AVAPIs, that is ChID in avServStart2(...,ChID)
	char reserved[4];
}SStreamDef;


/*	IOTYPE_USER_IPCAM_GETSUPPORTSTREAM_REQ			= 0x0328,
 */
typedef struct
{
	unsigned char reserved[4];
}SMsgAVIoctrlGetSupportStreamReq;


/*	IOTYPE_USER_IPCAM_GETSUPPORTSTREAM_RESP			= 0x0329,
 */
typedef struct
{
	unsigned int number; 		// the quanity of supported audio&video stream or video stream
	SStreamDef streams[0];
}SMsgAVIoctrlGetSupportStreamResp;


/* IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_REQ			= 0x032A, //used to speak. but once camera is connected by App, send this at once.
 */
typedef struct
{
	unsigned int channel;		// camera index
	char reserved[4];
}SMsgAVIoctrlGetAudioOutFormatReq;

/* IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_RESP			= 0x032B,
 */
typedef struct
{
	unsigned int channel;		// camera index
	int format;					// refer to ENUM_CODECID in AVFRAMEINFO.h
	char reserved[4];
}SMsgAVIoctrlGetAudioOutFormatResp;

/* IOTYPE_USER_IPCAM_RECEIVE_FIRST_IFRAME			= 0x1002,
 */
typedef struct
{
	unsigned int channel;		// camera index
	char reserved[4];
}SMsgAVIoctrlReceiveFirstIFrame;

/* IOTYPE_USER_IPCAM_GET_FLOWINFO_REQ              = 0x390
 */
typedef struct
{
	unsigned int channel;			// camera index
	unsigned int collect_interval;	// seconds of interval to collect flow information
									// send 0 indicates stop collecting.
}SMsgAVIoctrlGetFlowInfoReq;

/* IOTYPE_USER_IPCAM_GET_FLOWINFO_RESP            = 0x391
 */
typedef struct
{
	unsigned int channel;			// camera index
	unsigned int collect_interval;	// seconds of interval client will collect flow information
}SMsgAVIoctrlGetFlowInfoResp;

/* IOTYPE_USER_IPCAM_CURRENT_FLOWINFO              = 0x392
 */
typedef struct
{
	unsigned int channel;						// camera index
	unsigned int total_frame_count;				// Total frame count in the specified interval
	unsigned int lost_incomplete_frame_count;	// Total lost and incomplete frame count in the specified interval
	unsigned int total_expected_frame_size;		// Total expected frame size from avRecvFrameData2()
	unsigned int total_actual_frame_size;		// Total actual frame size from avRecvFrameData2()
	unsigned int timestamp_ms;					// Timestamp in millisecond of this report.
	char reserved[8];
}SMsgAVIoctrlCurrentFlowInfo;

/* IOTYPE_USER_IPCAM_GET_TIMEZONE_REQ               = 0x3A0
 * IOTYPE_USER_IPCAM_GET_TIMEZONE_RESP              = 0x3A1
 * IOTYPE_USER_IPCAM_SET_TIMEZONE_REQ               = 0x3B0
 * IOTYPE_USER_IPCAM_SET_TIMEZONE_RESP              = 0x3B1
 */
typedef struct
{
	int cbSize;							// the following package size in bytes, should be sizeof(SMsgAVIoctrlTimeZone)
	int nIsSupportTimeZone;
	int nGMTDiff;						// the difference between GMT in hours
	char szTimeZoneString[256];			// the timezone description string in multi-bytes char format
}SMsgAVIoctrlTimeZone;

/*
// dropbox support
IOTYPE_USER_IPCAM_GET_SAVE_DROPBOX_REQ      = 0x500,
IOTYPE_USER_IPCAM_GET_SAVE_DROPBOX_RESP     = 0x501,
*/
typedef struct
{
    unsigned short nSupportDropbox;     // 0:no support/ 1: support dropbox
    unsigned short nLinked;             // 0:no link/ 1:linked
    char szLinkUDID[64];                // Link UDID for App
}SMsgAVIoctrlGetDropbox;


/*
 // dropbox support
 IOTYPE_USER_IPCAM_SET_SAVE_DROPBOX_REQ      = 0x502,
 IOTYPE_USER_IPCAM_SET_SAVE_DROPBOX_RESP     = 0x503,
 */
typedef struct
{
    unsigned short nLinked;             // 0:no link/ 1:linked
    char szLinkUDID[64];                // UDID for App
    char szAccessToken[128];             // Oauth token
    char szAccessTokenSecret[128];       // Oauth token secret
	char szAppKey[128];                  // App Key (reserved)
	char szSecret[128];                  // Secret  (reserved)
}SMsgAVIoctrlSetDropbox;




/* IOTYPE_USER_IPCAM_GET_ATTRIBUTE_RESP			= 0X40002,
 * resolution参数说明：1：160*120；2：320*240；3：640*480；4：800*600；5：1024*768；6：1280*720。 7 640*368  8 ;320*200
 * video_display参数说明：1：亮度；2：对比度；3：灰度；4：饱和度；5：锐度。
 */
typedef struct
{
	unsigned long resolution; 		//支持的分辨率类型，每个bit代表一种分辨率。
	unsigned long video_display;	//支持的video_display，每个bit代表一种属性。
	short brightness_min;			//亮度调节的最小值
	short brightness_max;			//亮度调节的最大值
	short contrast_min;				//对比度调节的最小值
	short contrast_max;				//对比度调节的最大值
	short hue_min;					//灰度调节的最小值
	short hue_max;					//灰度调节的最大值
	short saturation_min;			//色饱和度调节的最小值
	short saturation_max;			//色饱和度调节的最大值
	short sharpness_min;			//锐度调节的最小值
	short sharpness_max;			//锐度调节的最大值
	short pt_rate_min;				//云台运行速度的最小值
	short pt_rate_max;				//云台运行速度的最大值
	short motion_detect_min;		//移动侦测的最小值
	short motion_detect_max;		//移动侦测的最大值
	short sound_detect_min;			//声音侦测的最小值
	short sound_detect_max;			//声音侦测的最大值
	unsigned char mic;				//是否有麦克风
	unsigned char speaker;			//是否有扬声器
	unsigned char reserved[2];		
} SMsgAVIoctrlGetAttributeResp;

/* IOTYPE_USER_IPCAM_GET_VIDEO_DISPLAY_RESP				= 0X40004,
 * IOTYPE_USER_IPCAM_SET_VIDEO_DISPLAY_REQ				= 0X40005,
 */
typedef struct
{ 
	unsigned char osd;				//是否开启osd功能
	unsigned char closeInfrared;	//是否关闭红外 (1关闭)
	short brightness;				//亮度
	short contrast;					//对比度
	short hue;						//灰度
	short saturation;				//饱和度
	short sharpness;				//锐度
	unsigned char osdMode;	// 0:年-月-日  时：分：秒  1: 月-日-年  时：分：秒  2:日-月-年  时：分：秒
	unsigned char reserved[3];	
} SMsgAVIoctrlGetVideoDisplayResp, SMsgAVIoctrlSetVideoDisplayReq;

/* IOTYPE_USER_IPCAM_SET_VIDEO_DISPLAY_RESP				= 0X40006,
 */
typedef struct
{
	int result;                   	//return 0 if succeed
	unsigned char reserved[4];
} SMsgAVIoctrlSetVideoDisplayResp;

typedef struct
{ 
	unsigned char io;				//是否开启io报警
	unsigned char open;				//是否打开 1: 常开
} AlarmIoCtrl;

/* IOTYPE_USER_IPCAM_GET_ALARM_ARG_RESP				= 0X40008,
 * IOTYPE_USER_IPCAM_SET_ALARM_ARG_REQ				= 0X40009,
 */
 #if 0
typedef struct
{ 
	unsigned char md_sensitivity;	//移动侦测灵敏度（0表示禁用）
	unsigned char sd_sensitivity;	//声音侦测灵敏度（0表示禁用）
	AlarmIoCtrl io_in;				//是否开启io报警
	AlarmIoCtrl alarm_io;			//报警时是否输出到io
	unsigned char alarm_record;		//报警时是否录像至SD卡
	unsigned char alarm_snapshot;	//报警时是否拍照至SD卡
	unsigned char alarm_mail;		//报警时是否拍照至email 
	unsigned char reserved[3];		
} SMsgAVIoctrlGetAlarmArgResp, SMsgAVIoctrlSetAlarmArgReq;
#else
typedef struct
{ 
	unsigned char md_sensitivity;	//移动侦测灵敏度（0表示禁用 1 2 3）
	unsigned char sd_sensitivity;	//声音侦测灵敏度（0表示禁用 1 2 3）
	unsigned char buzzer_enable;	//蜂鸣器开关（0表示禁用 1 使能）

	unsigned char md_time;			//移动侦测间隔时间  0 到 120 秒
	unsigned char audio_time;		//声音检测间隔时间  0 到 120 秒
	unsigned char buzzer_time;		//蜂鸣器持续时间  0 到 120 秒
	
	unsigned char alarm_record;		//报警时是否录像至SD卡
	unsigned char alarm_record_time;		//0 到 250秒
	
	unsigned char alarm_snapshot;	//报警时是否拍照至SD卡
	unsigned char alarm_mail;		//报警时是否拍照至email 
	unsigned char reserved[4];		
} SMsgAVIoctrlGetAlarmArgResp, SMsgAVIoctrlSetAlarmArgReq;

#endif

/* IOTYPE_USER_IPCAM_SET_ALARM_ARG_RESP				= 0X40010,
 */
typedef struct
{
	int result;            	       	//return 0 if succeed
	unsigned char reserved[4];
} SMsgAVIoctrlSetAlarmArgResp;

/* IOTYPE_USER_IPCAM_GET_EMAIL_ARG_RESP				= 0X40012,
 * IOTYPE_USER_IPCAM_SET_EMAIL_ARG_REQ					= 0X40013,
 */
typedef struct
{ 
	unsigned char sender[64];			// email发送邮箱
	unsigned char receiver[64];			// email接收邮箱
	unsigned char smtp_svr[64];			// smtp 服务器地址
	unsigned char smtp_user[32];		// smtp 用户
	unsigned char smtp_pwd[32];			// smtp 密码
	unsigned long smtp_port;			// smtp 服务器端口
	unsigned char smtp_auth;			// smtp 服务器是否需要认证
	unsigned char smtp_ssl;				// /*是否启用ssl安全登录1:是0:否*/
	unsigned char reserved[2];	
} SMsgAVIoctrlGetEmailArgResp, SMsgAVIoctrlSetEmailArgReq;

/* IOTYPE_USER_IPCAM_SET_EMAIL_ARG_RESP				= 0X40014,
 */
typedef struct
{
	int result;            	       	//return 0 if succeed
	unsigned char reserved[4];
} SMsgAVIoctrlSetEmailArgResp;

/* IOTYPE_USER_IPCAM_FACTORY_SETTINGS_RESP				= 0X40016,
 */
typedef struct
{
	int result;                   	//return 0 if succeed
	unsigned char reserved[4];
} SMsgAVIoctrlFactorySettingsResp;

/* IOTYPE_USER_IPCAM_UPGRADE_SYSTEM_REQ				= 0X40017,
 */
typedef struct
{ 
	unsigned char device_name[24];		//设备名称	
	unsigned char reserved[4];
} SMsgAVIoctrlSetDeviceInfoReq;

/* IOTYPE_USER_IPCAM_UPGRADE_SYSTEM_RESP				= 0X40018,
 */
typedef struct
{
	int result;                   	//return 0 if succeed
	unsigned char reserved[4];
} SMsgAVIoctrlSetDeviceInfoResp;

/* IOTYPE_USER_IPCAM_REBOOT_SYSTEM_RESP				= 0X40020,
 */
typedef struct
{
	int result;                   	//return 0 if succeed
	unsigned char reserved[4];
} SMsgAVIoctrlRebootSystemResp;

/* IOTYPE_USER_IPCAM_RESTORE_DEFAULT_RESP			= 0X40021,
 * 将当前会话的分辨率、码流、帧率、云台转速、OSD显示、电源频率、亮度、对比度、饱和度、灰度和锐度恢复到系统默认值。
 */
typedef struct
{
	unsigned char status;			//云台状态，参考ENUM_PTZ_STATUS
	unsigned char speed;			//云台速度
	unsigned char reserved[2];
} SMsgAVIoctrlGetPtzStatusResp;
typedef enum 
{ 
	AVIOCTRL_PTZ_NO_ACTION_PATROL		= 0x0,	//无巡航
	AVIOCTRL_PTZ_LEFT_RIGHT_PATROL		= 0x1,	//左右巡航
	AVIOCTRL_PTZ_UP_DOWN_PATROL			= 0x2,	//上下巡航
	AVIOCTRL_PTZ_TRACK_PATROL			= 0x3,	//轨迹巡航
	AVIOCTRL_PTZ_PATROL					= 0x4,	//轨迹巡航
}ENUM_PTZ_STATUS;

typedef enum 
{
	RESOLUTION_160_120		= 0x01,		//分辨率160*120
	RESOLUTION_320_240		= 0x02, 	//分辨率320*240
	RESOLUTION_640_480		= 0x03, 	//分辨率640*480
	RESOLUTION_800_600		= 0x04, 	//分辨率800*600
	RESOLUTION_1024_768		= 0x05, 	//分辨率1024*768
	RESOLUTION_1280_720		= 0x06,		//分辨率1280*720
	RESOLUTION_640_368		= 0x07,		//分辨率640*368
	RESOLUTION_320_200		= 0x08		//分辨率320*200
}ENUM_RESOLUTION;

/* IOTYPE_USER_IPCAM_GET_VIDEO_QUALITY_RESP			= 0X40024,
 * IOTYPE_USER_IPCAM_SET_VIDEO_QUALITY_REQ			= 0X40025,
 * 
 */
typedef struct
{
	short bitrate;					//码流
	unsigned char quant;			//质量
	unsigned char fps;				//帧率
	unsigned char resolution;		//分辨率,参见ENUM_RESOLUTION 
	unsigned char hmethod;	//是否固定码流
	unsigned char reserved[2];
} SMsgAVIoctrlSetVideoQualityReq, SMsgAVIoctrlGetVideoQualityResp;

typedef struct
{
	int result;                   	//return 0 if succeed
	unsigned char reserved[4];
} SMsgAVIoctrlSetVideoQualityResp;

/* IOTYPE_USER_IPCAM_TEST_EMAIL_ARG_RESP			= 0X40028,
 */
typedef struct
{
	int result;                   	//return 0 if succeed
	unsigned char reserved[4];
} SMsgAVIoctrlTestEmailArgResp;


/* IOTYPE_USER_IPCAM_UPGRADE_SYSTEM_REQ			= 0X40029,
 */
typedef struct
{
	unsigned int	total;			//升级包的总大小
	unsigned short	index;			//当前包的索引
	unsigned short	endflag;		//结束标志
	unsigned int	size;			//当前包的大小
	unsigned char	reserved[4];
	unsigned char	szBuf[0];		//升级包的数据
}SMsgAVIoctrlUpgradeSystemReq;


/* IOTYPE_USER_IPCAM_UPGRADE_SYSTEM_RESP			= 0X40030,
 */
typedef struct
{
	int result;                   	//return 0 if succeed
	unsigned char reserved[4];
} SMsgAVIoctrlUpgradeSystemResp;

/* IOTYPE_USER_IPCAM_GET_EVENT_LIST_REQ				= 0X40031,
 */
typedef struct
{
	unsigned int nStartTime; 		// Search event from ...
	unsigned int nEndTime;	  		// ... to (search event)
	unsigned char reserved[4];
}SMsgAVIoctrlGetAlarmListReq;



typedef struct alarm_log_t
{
	unsigned int 	time;;
	ENUM_EVENTTYPE 	event_type;
}alarm_log;

typedef struct
{
	unsigned int  	total;		// Total event amount in this search session
	unsigned char 	index;		// package index, 0,1,2...; 
								// because avSendIOCtrl() send package up to 1024 bytes one time, you may want split search results to serveral package to send.
	unsigned char 	endflag;		// end flag; endFlag = 1 means this package is the last one.
	unsigned char 	count;		// how much events in this package
	unsigned char 	reserved[1];
	alarm_log 		stalarm[0];		// The first memory address of the events in this package
}SMsgAVIoctrlAlarmListResp;


/*
IOTYPE_USER_IPCAM_GET_AUDIO_MOTION_REQ		= 0X40033,	//获取声音侦测
IOTYPE_USER_IPCAM_GET_AUDIO_MOTION_RESP		= 0X40034,
IOTYPE_USER_IPCAM_SET_AUDIO_MOTION_REQ		= 0X40035,	//设置声音侦测配置
IOTYPE_USER_IPCAM_SET_AUDIO_MOTION_RESP		= 0X40036,
*/
typedef struct
{
	int				enable;		//使能
	int				sensitive;	//灵敏度(1-3:低-中-高)
	int  			timeout ;       /*持续时间*/
	//联动
	BYTE 			msdrec_enable;
	BYTE			mmail_enable;
	BYTE			mftp_enable;
	BYTE			malarmout_enable;
	unsigned char 	reserved[4];
} SMsgAVIoctrlSetAudioMotionReq, SMsgAVIoctrlGetAudioMotionResp;

typedef struct
{
	int 			result;     //return 0 if succeed
	unsigned char 	reserved[4];
} SMsgAVIoctrlSetAudioMotionResp;

	
/*
IOTYPE_USER_IPCAM_GET_NET_REQ				= 0X40037,	//获取网络配置
IOTYPE_USER_IPCAM_GET_NET_RESP				= 0X40038,
IOTYPE_USER_IPCAM_SET_NET_REQ				= 0X40039,	//设置网络配置
IOTYPE_USER_IPCAM_SET_NET_RESP				= 0X40040,
*/
typedef struct
{
	int 	web_port;
	int 	video_port;
	int 	onvif_port;					/*onvif通讯端口*/
	int 	rtsp_port;					/*RTSP通讯端口*/
	char 	ipcamIP[16]; 				/* ipcam IP地址 */ 
	char 	ipcamIPMask[16]; 			/* ipcam IP地址掩码 */ 
	char 	ipcamGatewayIP[16]; 		/* 网关地址 */ 
	char 	byMACAddr[MACADDR_LEN]; 	/* 只读：服务器的物理地址 */ 
	char 	byDnsaddr[2][16]; 			/* DNS地址 */ 
	char 	sPPPoEUser[NAME_LEN]; 		/* PPPoE用户名 */ 
	char 	sPPPoEPassword[PASSWD_LEN];	/* PPPoE密码 */ 
	char 	sPPPoEIP[16]; 				//PPPoE IP地址(只读)
	BYTE 	dwPPPOE; 					/* 0-不启用,1-启用 */ 
	BYTE	conn_mod;					/*0-静态地址 1-DHCP 2-pppoe*/
	BYTE	reserved[2];
} SMsgAVIoctrlSetNetReq, SMsgAVIoctrlGetNetResp;

typedef struct
{
	int 			result;     //return 0 if succeed
	unsigned char 	reserved[4];
} SMsgAVIoctrlSetNetResp;

/*
IOTYPE_USER_IPCAM_GET_SNAP_RESP			= 0X40041	//客户端抓拍请求
*/
#define SNAP_BUFFER_SIZE 990
typedef struct
{
	short int 	size;//总大小
	short int 	singleSize;//当前包大小
	BYTE		packNum;//总包数
	BYTE		packNo;//包号，从0开始
	char 	snapBuf[SNAP_BUFFER_SIZE];
} SMsgAVIoctrlSetSnapResp;

/*
IOTYPE_USER_IPCAM_DEFAULT_VIDEO_REQ			= 0X40043,	//客户端请求默认的视频分辨率、码流、帧率，视频参数
IOTYPE_USER_IPCAM_DEFAULT_VIDEO_RESP		= 0X40044
*/
typedef struct
{
	int 			client_type; 	//0:cms and web;1: mobile
	unsigned char	reserved[4];
} SMsgAVIoctrlDefaultVideoReq;


typedef struct
{
	int 			result; 	//return 0 if succeed
	unsigned char	reserved[4];
} SMsgAVIoctrlDefaultVideoResp;

/* 
IOTYPE_USER_IPCAM_WIRTE_DEV_CONFIG_REQ			= 0X40045,
IOTYPE_USER_IPCAM_READ_DEV_CONFIG_RESP			= 0X40048,
 */
typedef struct
{
	char 	szUID[24];//百度使用的设备ID  
	char 	szPwd[32];//百度使用时pwd暂时不用 全部写0
	char 	szMacAddr[20];
	unsigned char	reserved[4];
} SMsgAVIoctrlWriteDevConfigReq, SMsgAVIoctrlReadDevConfigResp;

typedef struct
{
	char 	szMacAddr[24];
	char 	szUID[32];//百度使用的设备ID  
	char 	szPwd[32];//百度使用时pwd暂时不用 全部写
} SDEMsgAVIoctrlWriteDevConfigReq, SDEMsgAVIoctrlReadDevConfigResp;

/* IOTYPE_USER_IPCAM_WIRTE_DEV_CONFIG_RESP			= 0X40046
 */
typedef struct
{
	int 			result; 	//return 0 if succeed
	unsigned char	reserved[4];
} SMsgAVIoctrlWriteDevConfigResp;

/* IOTYPE_USER_IPCAM_RESET_STATUS_RESP			= 0X40049
 */
typedef struct
{
	unsigned char	result; 	//0：松开；1：按下
	unsigned char	reserved[3];
} SMsgAVIoctrlResetStatusResp;

/*
IOTYPE_USER_IPCAM_SET_NTP_CONFIG_REQ		= 0X40050,	//写入ntp的配置信息
IOTYPE_USER_IPCAM_SET_NTP_CONFIG_RESP		= 0X40051,
IOTYPE_USER_IPCAM_GET_NTP_CONFIG_REQ		= 0X40052,	//读取ntp的配置信息
IOTYPE_USER_IPCAM_GET_NTP_CONFIG_RESP		= 0X40053,

*/
typedef struct _ntp_set_time
{
	unsigned int year;
	unsigned int month;
	unsigned int date;
	
	unsigned int hour;
	unsigned int minute;
	unsigned int second;

}ntp_set_time;
	
typedef struct
{
	unsigned int 	mod;	 ///1,ntp 2 manul
	unsigned char 	Server[32];//NTP Server:
	ntp_set_time 	time;//manul time
	unsigned char 	TimeZone;//TimeZone:  0~25:(GMT-12)~GMT~(GMT+12)
} SMsgAVIoctrlSetNtpConfigReq, SMsgAVIoctrlGetNtpConfigResp;

typedef struct
{
	int 			result; 	//return 0 if succeed
	unsigned char	reserved[4];
} SMsgAVIoctrlSetNtpConfigResp;



/*
	IOTYPE_USER_IPCAM_GET_COMPANY_CONFIG_REQ	= 0X40054,	//读取公司信息
	IOTYPE_USER_IPCAM_GET_COMPANY_CONFIG_RESP	= 0X40055
*/

typedef struct
{
	unsigned char model[64];	// IPCam mode
	unsigned char vendor[64];	// IPCam manufacturer
	unsigned int version;		// IPCam firmware version	ex. v1.2.3.4 => 0x01020304;  v1.0.0.2 => 0x01000002
	unsigned int channel;		// Camera index
	unsigned int total;			// 0: No cards been detected or an unrecognizeable sdcard that could not be re-formatted.
								// -1: if camera detect an unrecognizable sdcard, and could be re-formatted
								// otherwise: return total space size of sdcard (MBytes)								
								
	unsigned int free;			// Free space size of sdcard (MBytes)
	unsigned char reserved[8];	// reserved
}SMsgAVIoctrlGetCompanyConfigResp;

/*
IOTYPE_USER_IPCAM_GET_DEVICEMODEL_CONFIG_REQ	= 0X40056,	//读取装备信息
IOTYPE_USER_IPCAM_GET_DEVICEMODEL_CONFIG_RESP	= 0X40057,

IOTYPE_USER_IPCAM_SET_DEVICEMODEL_CONFIG_REQ	= 0X40058,	//写入装备信息
IOTYPE_USER_IPCAM_SET_DEVICEMODEL_CONFIG_RESP	= 0X40059
*/
typedef struct
{
	char			deviceModel[64];	//设备型号
	char			manufacturer[64];	//制造商
	unsigned int	serverMask;			//位数说明(0:mic 1:speak 2:wifi 3:I/O输入输出报警 4:SD卡录像 5:云台) 该位1有效，0无效。
	char 			reserv[4]; 			//保留
} SMsgAVIoctrlSetDevModelConfigReq, SMsgAVIoctrlGetDevModelConfigResp;

typedef struct
{
	int 			result; 	//return 0 if succeed
	unsigned char	reserved[4];
} SMsgAVIoctrlSetDevModelConfigResp;

/*
IOTYPE_USER_IPCAM_GET_DROPBOX_CONFIG_REQ	= 0X4005A,	//读取DROPBOX信息
IOTYPE_USER_IPCAM_GET_DROPBOX_CONFIG_RESP	= 0X4005B,

IOTYPE_USER_IPCAM_SET_DROPBOX_CONFIG_REQ	= 0X4005B,	//设取DROPBOX信息
IOTYPE_USER_IPCAM_SET_DROPBOX_CONFIG_RESP	= 0X4005D

*/
#if 0
typedef struct
{
	char			enable_dropbox; /*0:disable 1:enable*/
	char			upload_mediaType;/*0:none 1:picture 2:alarm record 3:all time record 4:all picture and record*/
	unsigned short 	timing_snapshot_interval;/*定时抓拍功能的时间间隔值 0:disable定时抓拍 >0:抓拍间隔(单位.秒)*/
	char			appKey[64];
	char			appSecret[64];
	char			oauth_accessToken[64];
	char			oauth_accessTokenSecret[64];
	char 			reserv[4]; 			//保留
} SMsgAVIoctrlSetDropboxConfigReq, SMsgAVIoctrlGetDropboxConfigResp;
#endif

typedef struct
{
	int 			result; 	//return 0 if succeed
	unsigned char	reserved[4];
} SMsgAVIoctrlSetDropboxConfigResp;

typedef struct
{
	int 			result; 	//return 0 if succeed
	unsigned char	reserved[4];
}SMsgAVIoctrlSetUploadFunctionConfigResp;

/*
IOTYPE_USER_IPCAM_CLOSE_LED_CONFIG_REQ	= 0X40066,	//关闭led指示灯
IOTYPE_USER_IPCAM_CLOSE_LED_CONFIG_RESP	= 0X40067,

IOTYPE_USER_IPCAM_CLOSE_LED_STATUS_CONFIG_REQ	= 0X40068,	//led指示灯关闭状态
IOTYPE_USER_IPCAM_CLOSE_LED_STATUS_CONFIG_RESP	= 0X40069
*/
typedef struct
{
	char			isCloseLed;	//1:关闭LED 0:LED功能正常显示
	char			reserv[7];	//保留
} SMsgAVIoctrlCloseLedConfigReq, SMsgAVIoctrlCloseLedStatusConfigResp;

typedef struct
{
	int 			result; 	//return 0 if succeed
	unsigned char	reserved[4];
} SMsgAVIoctrlCloseLedConfigResp;

//add by jy
typedef struct
{
	int 			result; 	//return 0 if succeed
	unsigned char	reserved[4];
} SMsgAVIoctrlSendPicConfigResp;

typedef struct
{
	int 			result; 	//return 0 if succeed
	unsigned char	reserved[4];
} SMsgAVIoctrlSendPicToShareConfigResp;


//time mode
/*typedef struct
{
	int 			mode; 		//0:China   1:America  2:Europe
	unsigned char	reserved[4];
} SMsgAVIoctrlSetTimeModeConfigReq, SMsgAVIoctrlGetTimeModeConfigResp;*/


typedef struct
{
	int 			result; 	//return 0 if succeed
	unsigned char	reserved[4];
} SMsgAVIoctrlSetTimeModeConfigResp;

//time mode add by jy
typedef struct
{
	int 			mode; 		//0:China   1:America  2:Europe
	unsigned char	reserved[4];
} SMsgAVIoctrlSetTimeModeConfigReq, SMsgAVIoctrlGetTimeModeConfigResp;

typedef struct
{
	double			rate; 		//速率
	unsigned char	reserved[4];
} SMsgAVIoctrlSetPlayBackRateConfigReq;

typedef struct
{
	int 			result; 	//return 0 if succeed
	unsigned char	reserved[4];
} SMsgAVIoctrlPlaybackrateConfigResp;

//end add
//IOTYPE_USER_IPCAM_SET_RECORD_SCHEDULE_REQ		= 0X4007E,	//设置录像排程
//IOTYPE_USER_IPCAM_SET_RECORD_SCHEDULE_RESP		= 0X4007F,	// 

//IOTYPE_USER_IPCAM_GET_RECORD_SCHEDULE_REQ		= 0X40080,	//读取录像排程
//IOTYPE_USER_IPCAM_GET_RECORD_SCHEDULE_RESP		= 0X40081,	// 

#define MAX_WEEK_NUM	7
#define MAX_DAY_NUM		7
#define MAX_DATE_LEN	12

typedef struct  
{
	int slot;						//每一位代表一个小时
	char szDate[MAX_DATE_LEN];
}DayData;

typedef struct
{
	int week[MAX_WEEK_NUM];			//每一位代表一个小时
	DayData day[MAX_DAY_NUM];
	unsigned char	reserved[4];
} SMsgAVIoctrlSetRecordScheduleReq, SMsgAVIoctrlGetRecordScheduleResp;

typedef struct
{
	int 			result; 	//return 0 if succeed
	unsigned char	reserved[4];
} SMsgAVIoctrlSetRecordScheduleResp;

//end


//	IOTYPE_USER_IPCAM_SET_MOTION_AREA_REQ			= 0X40079,	// 移动侦测区域 参数设置
//	IOTYPE_USER_IPCAM_SET_MOTION_AREA_RESP			= 0X4007A,	// 

#define MAX_MOTION_AREA 4

typedef struct _rect_s
{
	int 	start_x;	//起始坐标
	int		start_y;	
	int 	end_x;		//终点坐标
	int		end_y;	
}rect_t;

typedef struct
{
	int area_enable[MAX_MOTION_AREA];  // area_enable : 1 区域使能  0 区域禁止   暂时没有用到保留
	rect_t area[MAX_MOTION_AREA]; //区域内的坐标值 必须是16的整数倍

}SMsgAVIoctrlSetMotionAreaReq, SMsgAVIoctrlGetMotionAreaResp;


typedef struct
{
	int 			result; 	//return 0 if succeed
	unsigned char	reserved[4];
} SMsgAVIoctrlSetMotionAreaResp;


typedef struct
{
	int result;	// 0: success; otherwise: failed.
	unsigned char reserved[4];
}SMsgAVIoctrlSetBaiDuInfoResp;


#define MAX_ACCESS_TOKEN_LEN         128      //
#define MAX_MANU_STR_LEN         64      //制造商
#define MAX_VER_STR_LEN         32      //版本信息长度
#define MAX_SERIAL_NO_LEN       32		//序列号长度
#define MAX_USR_ID_LEN			32		//用户ID长度
#define MAX_DVR_NAME_LEN        32      //设备名称长度
#define MAX_CHANN_NAME_LEN      32      //通道名称长度
#if 0
typedef struct
{
	unsigned char			usr_no[MAX_USR_ID_LEN];			//用户ID			//new add 
	unsigned char			serial_no[MAX_SERIAL_NO_LEN];	//设备序列号 //需支持百度  TUTK
	unsigned char 			device_type[MAX_VER_STR_LEN];       /* 产品型号 */
	unsigned char			manufacturer[MAX_SERIAL_NO_LEN];	//制造商
	int						serverMask;							//产品支持的功能
	char 					reserv[4]; 							//保留
}SMsgAVIoctrlGetMdSysInfoResp;
#else
typedef struct 
{
   
    /*---------- 以下信息不可更改 ----------*/
    unsigned char serial_1[MAX_SERIAL_NO_LEN];       //序列号  百度使用
    unsigned char serial_2[MAX_SERIAL_NO_LEN];       //序列号 tutk 使用
    unsigned char manufacturer[MAX_MANU_STR_LEN];    //制造商
     unsigned char device_type[MAX_VER_STR_LEN];      //设备型号
    unsigned char software_ver[MAX_VER_STR_LEN];      //软件版本号
    unsigned char software_date[MAX_VER_STR_LEN];     //软件生成日期
    unsigned char hardware_ver[MAX_VER_STR_LEN];      //硬件版本
    
    unsigned int dev_type;                          //设备类型SDK_DEV_TYPE_E   需要重新定义
	unsigned int server_mask;						// 每一位代表设备支持的服务功能  列如 百度  tutk  onvif 
	unsigned int io_mask;						  // 每一位代表设备外设  需要重新定义 0:mic 1:speak 2:wifi 3:I/O输入输出报警 4:SD卡录像 5:云台)

	unsigned char  max_stream;                         //视频流数目
    unsigned char  max_resolution;                    //最大编码分辨率
    unsigned char  res[2];                    //最大编码分辨率
    
}SMsgAVIoctrlGetMdSysInfoResp;
#endif


#if 0
#define MAX_LED_NUM	4
typedef struct
{
	char led_status[MAX_LED_NUM];	//led_status[0]:   red----1:on 0:off
									// led_status[1]:green----1:on 0:off
									// led_status[2]: blue----1:on 0:off
} SMsgAVIoctrlSetLedStatusReq, SMsgAVIoctrlGetLedStatusResp;

typedef struct
{
	int 			result; 	//return 0 if succeed
	unsigned char	reserved[4];
} SMsgAVIoctrlSetLedStatusResp;


typedef struct
{
	int  buzzer_status;	// buzzer_status ----1:on 0:off
									
} SMsgAVIoctrlSetBuzzerStatusReq, SMsgAVIoctrlGetBuzzerStatusResp;

typedef struct
{
	int 			result; 	//return 0 if succeed
	unsigned char	reserved[4];
} SMsgAVIoctrlSetBuzzerStatusResp;
#endif

// AWS support 亚马逊录像上传功能 查询和设置 
//IOTYPE_USER_IPCAM_GET_AWS_STATUS_REQ	  = 0X4008A,
//IOTYPE_USER_IPCAM_GET_AWS_STATUS_RESP	  = 0X4008B,
//IOTYPE_USER_IPCAM_SET_AWS_STATUS_REQ	  = 0X4008C,
//IOTYPE_USER_IPCAM_SET_AWS_STATUS_RESP	  = 0X4008D,
typedef struct
{
	char			aws_record;	//0: 视频持续上传  1:有报警事件才上传视频
	char			reserv[3];			//保留
} SMsgAVIoctrlSetAwsStatusReq, SMsgAVIoctrlGetAwsStatusResp;

 typedef struct
 {
	 int			 result;	 //return 0 if succeed
	 unsigned char	 reserved[4];
 } SMsgAVIoctrlSetAwsStatusResp;

 // REMOTE UPGRADE	 升级请求
//IOTYPE_USER_IPCAM_REMOTE_UPGRADE_REQ	  = 0X4008E,
//IOTYPE_USER_IPCAM_REMOTE_UPGRADE_RESP	  = 0X4008F,
typedef struct
{
	char			 new_version[64];	 //即将升级的产品版本号
	char			 url_parth[128]; 		 //升级文件存放的服务器地址
} SMsgAVIoctrlRemoteUpgradeReq;
	 
typedef struct
{
	int			  result;	  //0 : 可以升级 1:当前已经是最新版本，无需升级   -1:未知错误
	unsigned char   reserved[4];
} SMsgAVIoctrlRemoteUpgradeResp;



// UPGRADE PROGRESS  升级进度查询
//IOTYPE_USER_IPCAM_UPGRADE_PROGRESS_REQ		= 0X40090,
//IOTYPE_USER_IPCAM_UPGRADE_PROGRESS_RESP 	= 0X40091,
typedef struct
{
		int			progress_value;	//升级进度 :0~100  表示升级进度   -1: 未知错误
		char		reserv[4];	//保留
} SMsgAVIoctrlProgressResp;


typedef struct
{
		char		type; // 1 有线 2 wifi 
		char		reserv[4];	//保留
} SMsgAVIoctrlGetMacAddrReq; 


#define MAX_MAC_ADDR_LEN        20      //MAC地址长度
typedef struct
{
		 char		type; // 1 有线 2 wifi 
		 char 		mac[MAX_MAC_ADDR_LEN];   
		 char		reserv[4];	//保留
} SMsgAVIoctrlGetMacAddrResp; 
#endif
