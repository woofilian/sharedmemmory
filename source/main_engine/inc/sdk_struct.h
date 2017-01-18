#ifndef __sdk_struct_h__
#define __sdk_struct_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <pthread.h>
#include <sys/time.h>

//access_token
#define MAX_ACCESS_TOKEN_LEN    128      //
#define MAX_MANU_STR_LEN        64      //制造商
#define MAX_VER_STR_LEN         32      //版本信息长度
#define MAX_SERIAL_NO_LEN       32		//序列号长度
#define MAX_USR_ID_LEN			32		//用户ID长度
#define MAX_DVR_NAME_LEN        32      //设备名称长度
#define MAX_CHANN_NAME_LEN      32      //通道名称长度

//#define MAX_CHANN_NUM           32      //最大通道个数
#define MAX_CHANN_NUM           2      //最大通道个数  对于IPC来说三个通道够用
#define MAX_USER_NAME_LEN       32      //用户名长度
#define MAX_PASSWD_LEN          32      //密码长度
#define MAX_ALARM_IN_NAME_LEN   32      //报警输入名称长度
#if 0
#define MAX_WEEK_NUM            8       //星期天数
#define MAX_TIME_SEG_NUM        4       //时间段个数
#endif
//chang  bu zw for baidu
#define MAX_WEEK_NUM            7       //星期天数
#define MAX_TIME_SEG_NUM        5       //时间段个数


#define MAX_ALARM_OUT_NUM       16      //最大报警输出个数
#define MAX_ALARM_IN_NUM        16      //最大报警输入个数
#define MAX_IP_ADDR_LEN         40      //IP地址长度
#define MAX_MAC_ADDR_LEN        20      //MAC地址长度
#define MAX_ETHERNET_NUM        4       //网卡个数
#define MAX_RECORD_ITEM_LEN     64      //录像记录名称长度
#define MAX_LOG_DESC_LEN        128     //日志记录内容长度
#define MAX_USER_NUM            16      //最大用户个数
#define MAX_PRESET_NUM          256     //最大云台预置位个数
#define MAX_CRUISE_NUM          256     //最大云台巡航个数
#define MAX_CRUISE_PRESET_NUM   16      //每条巡航中最大支持的预置位个数
#define MAX_CHANN_CONN_NUM      8       //每个通道最大支持的链接数
#define MAX_DISK_NUM            8       //最大磁盘个数
#define MAX_ALARM_DESC_LEN      64      //报警记录内容长度
#define MAX_SCREEN_NUM          4       //本地最大屏幕个数
#define MAX_WINDOWS_NUM         32      //本地预览视频最大窗口个数
#define MAX_PREVIEW_MODE        8       //最大预览布局个数
#define MAX_OVERLAY_NUM         4       //每个通道最大覆盖区域个数
#define MAX_MOTION_NUM          4       //每个通道最大移动侦测区域个数
#define MAX_OSD_STR_LEN         64      //OSD字符串长度
#define MAX_NTP_SETVER_STR_LEN  32      //NTP服务器地址长度
#define MAX_BACKUP_ITEM			16		//最大备份段数

//add by zw
#define MAX_LED_NUM             4      //最大的LED等数
#define MAX_URL_LEN			    128     //升级包URL最大长度

//ceshi

//编码相关
#define MAX_ROI_REG       		4  //感兴趣区域编码区域数

#define MAX_DAY_NUM	            32	//用来布防使用


#define MAX_VDA_SENSITIVE_LEVEL 9
#define MAX_STREAM_NUM    		3    	/* 最大码流路数, 0:主码流；1:从码流 */
#define MAX_STOR_FRAGMENT_LEN	16		// sizeof(stor_fragment_t)+6个int的保留大小

#define MAX_NAL_DESC_SIZE       (0)//视频帧数据前段部分长度

#define MAX_DEV_ID_LEN          32      //最大设备ID长度
#define MAX_WIFI_AP_NUM			16		//最大WIFI-AP个数
#define MAX_IF_NAME_LEN         8       //网络接口名称最大长度

#define NTP_SERVER_NUM          2
#define NTP_SERVER_LEN          64

#define MAX_AP_NUM              30
/*------------------------------------------------*/
#define MAX_MSG_SIZE        (64*1024)
#define CHANN_TYPE_ALL      0xFA
#define PARAM_MASK_ALL		0xFFFF		// 恢复整机参数



#define BAIDU_INFO_GET_PATH "/config/param/device_info" //获取百度信息成功后填入参数

//以下三条在回复出厂设置是也不会删除
#define DEVICE_CFG_PATH 	"/config/param/DEVICE_INFO" //
#define MAC_CFG_PATH 		"/config/param/MAC_INFO" //获取百度信息成功后填入参数
#define UID_CFG_PATH 		"/config/param/UID_INFO" //获取百度信息成功后填入参数



#define ROOT_DIR            "/apps/ausdom/pic/"         //抓图目录
#define WARNING_DIR         "/apps/ausdom/Warning_PIC/" //报警目录


#define WIFI_CONFIG_INI "/config/wireless_info.conf"


//#define  MAX_VIDEO_STREAM_NUM 2 
#define  MAX_VIDEO_STREAM_NUM 1 

#define  MAX_AUDIO_STREAM_NUM 1




/*----------- 错误码定义 -----------*/

typedef enum _SDK_ERR_CODE 
{
    SDK_ERR_SUCCESS = 0x00         //成功
    , SDK_ERR_FAILURE               //失败
    , SDK_ERR_PASS                  //密码错误
    , SDK_ERR_USER_NOT_EXIST		//用户名不存在
    , SDK_ERR_ROOT_USER				//root用户不能删除
    , SDK_ERR_TOO_MANY_USER			//用户已满(最多可添加16个用户名)
    , SDK_ERR_USER_ALREADY_EXIST	//用户名已存在
    , SDK_ERR_NET_CFG				//配置网络参数失败
    , SDK_ERR_PERMIT                //权限错误
    , SDK_ERR_CHANN                 // 10 通道号错误
    , SDK_ERR_CONN_MAX              //超出最大链接数
    , SDK_ERR_CONN                  //链接错误
    , SDK_ERR_SEND                  //数据发送错误
    , SDK_ERR_RECV                  //数据接收错误
    , SDK_ERR_BUSY                  //设备正忙
    , SDK_ERR_DATA                  //数据格式错误
    , SDK_ERR_CMD                   //命令字错误
    , SDK_ERR_VER                   //协议版本错误
    , SDK_ERR_NO_DISK               //无磁盘
    , SDK_ERR_DISK_ERR              // 20 磁盘错误
    , SDK_ERR_RESOURCE              //资源不可用
    , SDK_ERR_FLASH                 //FLASH读写错误
    , SDK_ERR_SET_SYSTIME			//设置系统时间错误
    , SDK_ERR_SET_ENCODE_PARAM		//设置编码参数错误
    , SDK_ERR_SET_MD_PARAM			//设置移动侦测参数错误
    , SDK_ERR_SET_OSD_PARAM			//设置OSD参数错误
    , SDK_ERR_SET_OVERLAY_PARAM		//设置遮挡区域参数错误
    , SDK_ERR_SET_TV_PARAM			//设置边距大小错误
    , SDK_ERR_SET_IMAGE_ATTR		//设置图像属性错误
    , SDK_ERR_SET_3A_ATTR			//设置3A属性错误
    , SDK_ERR_LOG_QUERY				//查询日志失败
    , SDK_ERR_LOG_EMPTY				//清空日志失败
    , SDK_ERR_LOG_BACKUP			//备份日志失败
    , SDK_ERR_SERIAL_NO				//串口号错误
    , SDK_ERR_SERIAL_OPEN			//打开串口失败
    , SDK_ERR_SERIAL_PARAM_SET		//设置串口参数失败
    , SDK_ERR_DISK_MOUNT			//磁盘挂载失败
    , SDK_ERR_UMOUNT				//磁盘卸载失败
    , SDK_ERR_BACKUP				//备份失败
    , SDK_ERR_LOG_DEL				//删除日志失败

    , SDK_ERR_GET_DISK_PART_INOF	//获取磁盘分区信息错误
    , SDK_ERR_UPGRADING				//正在升级中
    , SDK_ERR_UPGRADE_CHECK_HEAD	//校验升级包头错误
    , SDK_ERR_UPGRADE_FILE_FIND		// 查找不到升级文件
    , SDK_ERR_UPGRADE				// 升级失败

    , SDK_ERR_NVR_NOT_LOGIN				// 前端设备还没登陆成功

}
SDK_ERR_CODE_E;


//cgi 消息传输接口
typedef enum _SDK_MSG_DIR
{
    SDK_MSG_RESP= 0x00,
    SDK_MSG_REQ = 0x01
} SDK_MSG_DIR_E;

/*----------- 参数方向 -----------*/

typedef enum _SDK_PARAM_DIR
{
    SDK_PARAM_SET	= 0x01,	    //设置参数
    SDK_PARAM_GET        		//查询参数
} SDK_PARAM_DIR_E;

//由于部分字段暂时不使用 决定去掉部分
#if 0
typedef struct sdk_msg_s
{
    uint16_t magic;         //保留，扩展(如设备类型)
    uint16_t version;       //版本号
#if 1
    union
    {
        struct
        {
            uint8_t src;    //源模块ID
            uint8_t dst;    //目标模块ID
        } __attribute__((packed)) id;
        uint16_t session_id;//用户会话ID(设备端分配)
    } __attribute__((packed));
    uint16_t sequence_id;   //命令序号
#endif
#if 1
    uint16_t msg_id;        //命令ID
    uint8_t  f_dir:1;       //（SDK_PARAM_DIR_E)
    uint8_t  f_pkg:1;       //1:extend字段用作分包;
    uint8_t  f_res:6;       //保留
    uint8_t  ack;           //错误码
#endif
    uint32_t args;          //命令参数(设置、获取参数;或者是子命令类型[用户管理、升级、磁盘管理])
    uint32_t extend;        //扩展字段(8:pkg_total,8:pkg_num, 8:, 8:)
    uint32_t chann;         //通道号(CHANN_TYPE_ALL)
    uint32_t size;          //消息长度
    uint8_t  data[0];       //消息体
} sdk_msg_t;
#else
//一共24个字节
typedef struct sdk_msg_s
{
    uint16_t magic;         //保留，扩展(如设备类型) 可作为发送消息的模块识别码
    uint16_t version;       //版本号
#if 1
    uint16_t msg_id;        //命令ID
    uint8_t  msg_dir;       //（SDK_PARAM_DIR_E)
    uint8_t  ack;           //错误码
#endif
    uint32_t args;          //SDK_PARAM_DIR_E  命令参数(设置、获取参数;或者是子命令类型[用户管理、升级、磁盘管理])
    uint32_t extend;        //扩展字段(8:pkg_total,8:pkg_num, 8:, 8:)
    uint32_t chann;         //通道号(CHANN_TYPE_ALL)
    uint32_t size;          //消息长度
    uint8_t  data[0];       //消息体
} __attribute__ ((packed))sdk_msg_t;
#endif

/*
*进程类型
*
*/
typedef enum _PROC_TYPE
{
    PROC_TYPE_CMS =0,
    PROC_TYPE_GUI,
    PROC_TYPE_MAIN,
    PROC_TYPE_WEB
} PROC_TYPE_E;

/*
 *
 * 发送心跳消息
 */
typedef struct sdk_keeplive_s
{
    char process_name[64];
    long keep_count;   //心跳间隔 但是秒 默认是60秒  之后没有收到心跳消息  系统重启
    int  process_type; //PROC_TYPE_E
    int  kill_server;
} sdk_keeplive_t;

/*
 *
 * 停止主控服务  目前也未不熟悉 所以 暂时这么定义
 */
typedef struct sdk_stop_server_s
{
    char process_name[64]; // 空置方的进程名称
    char passwd[32]; 	// 控制方 需带有控制密码  否则认为是非法指令
    long server_type;   //要停止那些服务  每一位代表一个服务  暂且规划成停止所有
} sdk_stop_server_t;

/*
 *
 * 系统版本信息
 */

typedef struct sdk_version_s
{
    uint8_t serial_no[MAX_SERIAL_NO_LEN];       /* 产品序列号 */
    uint8_t device_type[MAX_VER_STR_LEN];       /* 产品型号 */
    uint8_t software_ver[MAX_VER_STR_LEN];	    /* 版本号 */
    uint8_t software_date[MAX_VER_STR_LEN];	    /* 版本日期 */
    uint8_t panel_ver[MAX_VER_STR_LEN];         //前面板版本
    uint8_t hardware_ver[MAX_VER_STR_LEN];      //硬件版本
} sdk_version_t;

/*
 *
 * 获取设备配置信息  该参数是只读的 真正的配置在 系统配置信息里面
 */
typedef struct sdk_device_s
{
    uint8_t			usr_no[MAX_USR_ID_LEN];			//用户ID			//new add
    uint8_t			serial_no[MAX_SERIAL_NO_LEN];	//设备序列号 //需支持百度  TUTK
    uint8_t 		device_type[MAX_VER_STR_LEN];       /* 产品型号 */
    uint8_t			manufacturer[MAX_SERIAL_NO_LEN];	//制造商
    uint32_t		serverMask;			//产品支持的功能
    char 			reserv[4]; 			//保留
} sdk_device_t ;

typedef struct sdk_cloud_uid_s
{
    uint8_t			uid_1[MAX_USR_ID_LEN];		//用户UID 生产的时候手动配置好的
    uint8_t			uid_2[MAX_SERIAL_NO_LEN];	//备用
    char 			reserv[4]; 			        //保留
} sdk_cloud_uid_t ;

//#define MAX_NAME_LEN          32      //密码长度

typedef struct sdk_register_cfg_s
{
    uint8_t	 usr_no[MAX_USR_ID_LEN];					//用户ID
    uint8_t	 access_token[MAX_ACCESS_TOKEN_LEN];		//手机端提供的信息
    uint8_t	 stream_id[MAX_ACCESS_TOKEN_LEN];			//手机端提供的信息

    uint8_t ssid[MAX_DEV_ID_LEN];		//ssid
    uint8_t pwd[MAX_PASSWD_LEN];		//密码
//	uint8_t name[MAX_NAME_LEN];		//device_name

    uint32_t encrypt_type;				//加密类型0:NONE  1:WPA  2:WPA2 3:WEP
    uint32_t auth_mode;			//认证方式0:NONE  1:EAP 2:PSK 3:OPEN 4:SHARED
    uint32_t secret_key_type;		//密钥管理方式 0:NONE  1:AES 2:TKIP 只对应于加密类型为WPA/WPA2的情况
} sdk_register_cfg_t;



/*
 *
 * 系统配置参数 这个参数应该是烧写好之后 只读的
 * 提供设备一切可用信息
 */
typedef struct sdk_sys_cfg_s
{

    /*---------- 以下信息不可更改 ----------*/
    uint8_t serial_1[MAX_SERIAL_NO_LEN];       //序列号  百度使用
    uint8_t serial_2[MAX_SERIAL_NO_LEN];       //序列号 tutk 使用
    uint8_t manufacturer[MAX_MANU_STR_LEN];    //制造商
    uint8_t device_type[MAX_MANU_STR_LEN];       //设备型号//之前是32  修改64
    uint8_t software_ver[MAX_VER_STR_LEN];      //软件版本号
    uint8_t software_date[MAX_VER_STR_LEN];     //软件生成日期
    uint8_t hardware_ver[MAX_VER_STR_LEN];      //硬件版本

    uint32_t dev_type;                          //设备类型SDK_DEV_TYPE_E   需要重新定义
    uint32_t server_mask;						// 每一位代表设备支持的服务功能  列如 百度  tutk  onvif
    uint32_t io_mask;						  // 每一位代表设备外设  需要重新定义 0:mic 1:speak 2:wifi 3:I/O输入输出报警 4:SD卡录像 5:云台)

    uint8_t  max_stream;                         //视频流数目
    uint8_t  max_resolution;                    //最大编码分辨率
    uint8_t  res[2];                    //最大编码分辨率

} sdk_sys_cfg_t;


/*
 * 设备能力集
 */

typedef enum SDK_VIDEO_CAP_RESOLUTION
{
    SDK_VIDEO_CAP_RESOLUTION_QCIF   = 0x1,				//176*144  n制176*120
    SDK_VIDEO_CAP_RESOLUTION_CIF    = 0x2,				//352*288  n制352*240
    SDK_VIDEO_CAP_RESOLUTION_HD1    = 0x4,				//704*288  n制704*240
    SDK_VIDEO_CAP_RESOLUTION_D1     = 0x8,				//704*576  n制704*480
    SDK_VIDEO_CAP_RESOLUTION_960H   = 0x10,				//960x576
    SDK_VIDEO_CAP_RESOLUTION_QQVGA  = 0x20,				//160*112
    SDK_VIDEO_CAP_RESOLUTION_QVGA   = 0x40,				//320*240
    SDK_VIDEO_CAP_RESOLUTION_VGA    = 0x80,				//640*480
    SDK_VIDEO_CAP_RESOLUTION_SVGA   = 0x100,			//800*600
    SDK_VIDEO_CAP_RESOLUTION_UXGA   = 0x200,			//1600*1200
    SDK_VIDEO_CAP_RESOLUTION_720P   = 0x400,			//1280*720
    SDK_VIDEO_CAP_RESOLUTION_960    = 0x800,			//1280*960
    SDK_VIDEO_CAP_RESOLUTION_1080P	= 0x1000			//1920*1080
} SDK_VIDEO_CAP_RESOLUTION_E;

typedef struct device_chan_capability_s
{
    uint32_t   res_cap[MAX_STREAM_NUM];			// SDK_VIDEO_CAP_RESOLUTION_E
    uint32_t   max_frame_rate[MAX_STREAM_NUM]; 	// 码流最大支持帧率
} device_chan_capability_t;

typedef struct sdk_device_capability_s
{
    uint8_t    screen_num;                     			// 屏幕个数（3531两个，3521一个）
    uint8_t    eth_num;                        			// 网口个数（设备类型为xx_e2时两个，其它一个）
    uint8_t    res[2];                         			// 保留
    device_chan_capability_t ch_cap[MAX_CHANN_NUM];
} sdk_device_capability_t;


/*
 * 恢复默认参数
 */

typedef enum _SDK_PARAM_MASK_ID
{
    SDK_PARAM_MASK_ENCODE = 0x00,    	//bit.0: 编码参数
    SDK_PARAM_MASK_PTZ          ,     	//bit.1: PTZ云台参数
    SDK_PARAM_MASK_RECORD       , 		//bit.2: 录像参数
    SDK_PARAM_MASK_NET          ,      	//bit.3: 网络参数
    SDK_PARAM_MASK_CHANNEL      ,  		//bit.4: 通道参数
    SDK_PARAM_MASK_IMAGE_ATTR   ,		//bit.5: 视频属性
    SDK_PARAM_MASK_SERIAL	    ,		//bit.6: 串口参数
    SDK_PARAM_MASK_OVERLAY      ,		//bit.7: 遮挡区域参数
    SDK_PARAM_MASK_MOTION	    ,		//bit.8: 移动侦测
    SDK_PARAM_MASK_COMM         ,		//bit.9: 常规参数
    SDK_PARAM_MASK_OSD          ,		//bit.10: osd参数
    SDK_PARAM_MASK_USER         ,		//bit.11: 用户管理
    SDK_PARAM_MASK_ALARM_IN     ,		//bit.12: 外部报警输入参数
    SDK_PARAM_MASK_HIDE         ,		//bit.13: 视频遮挡侦测参数
    SDK_PARAM_MASK_VIDEO_LOST   ,		//bit.14: 视频丢失参数
    SDK_PARAM_MASK_TV           ,		//bit.15: 边距调节参数
    SDK_PARAM_MASK_PREVIEW      ,   	//bit.16: 预览参数
    SDK_PARAM_MASK_TOUR         ,       //bit.17: 巡回参数

    SDK_PARAM_MASK_BUIT
} SDK_PARAM_MASK_ID_E;


typedef struct sdk_default_param
{

    uint32_t param_mask;    //参数内容掩码 SDK_PARAM_MASK_ID_E
    uint32_t res[1];
} sdk_default_param_t;


/*
 * 系统运行状态信息
 */
typedef enum _SDK_STATUS_OP
{
    SDK_STATUS_DEVICE = 0x01,  //设备运行状态
    SDK_STATUS_DISK,           //磁盘状态
    SDK_STATUS_CHANN,          //通道状态
    SDK_STATUS_ALARM_IN,       //输入报警状态
    SDK_STATUS_ALARM_OUT       //输出报警状态
} SDK_STATUS_OP_E;

typedef struct sdk_chan_status_s
{

    uint8_t chan_type;          //通道类型 0：本地，1：IP
    uint8_t record_state;       //录像状态,0-不录像,1-录像
    uint8_t signal_state;       //信号状态, 0:正常，1：信号丢失
    uint8_t encode_state;       //编码状态, 0:不编码，1：编码

    uint32_t bit_rate;          //实际码率
    uint32_t conn_num;          //通道连接数
    uint8_t  conn_ip[MAX_CHANN_CONN_NUM][MAX_IP_ADDR_LEN];//连接IP
} sdk_chan_status_t;


typedef struct sdk_disk_status_s
{
    uint32_t volume;        //硬盘的容量
    uint32_t free_space;    //硬盘的剩余空间
    uint32_t disk_state;   //硬盘的状态,0-活动,1-休眠,2-不正常
} sdk_disk_status_t;

/*
 * 系统状态(后续拆分成 SDK_STATUS_OP_E)
 */

typedef struct sdk_status_s
{

    uint32_t            device_status;                      //设备状态，0-正常,1-CPU占用率太高,超过85%,2-硬件错误
    sdk_disk_status_t   disk_status[MAX_DISK_NUM];          //硬盘状态
    sdk_chan_status_t   chan_status[MAX_CHANN_NUM];         //通道状态
    uint8_t             alarm_in_status[MAX_ALARM_IN_NUM];  //报警端口的状态,0-没有报警,1-有报警
    uint8_t             alarm_out_status[MAX_ALARM_OUT_NUM];//报警输出端口的状态,0-没有输出,1-有报警输出

    uint8_t             local_display;                      //本地显示状态, 0:正常，1：不正常
    uint8_t             audio_chan_status;                  //语音通道的状态 0-未使用，1-使用中, 0xff无效
    uint8_t             res[2];

} sdk_status_t;

/*
 * 关闭系统
 */
typedef enum _SDK_CLOSE_SYS_OP
{
    SDK_CLOSE_SYS_SHUTDOWN = 0x001,		//关机
    SDK_CLOSE_SYS_REBOOT,			    //重启
    SDK_COLSE_SYS_LOGOUT,
    SDK_CLOSE_SYS_BUTT
} SDK_CLOSE_SYS_OP_E;

/*
 * 用户定义(用户名＋密码)
 */
typedef struct sdk_user_s
{
    uint8_t user_name[MAX_USER_NAME_LEN];
    uint8_t user_pwd[MAX_PASSWD_LEN];
} sdk_user_t;


/*
 * 用户权限定义
 */
typedef struct sdk_user_right_s
{

    sdk_user_t user;
    uint32_t local_right;           //本地GUI权限
    /*bit.0: 本地控制云台*/
    /*bit.1: 本地手动录象*/
    /*bit.2: 本地回放*/
    /*bit.3: 本地设置参数*/
    /*bit.4: 本地查看状态、日志*/
    /*bit.5: 本地高级操作(升级，格式化，重启，关机)*/
    /*bit.6: 本地查看参数 */
    /*bit.7: 本地管理模拟和IP camera */
    /*bit.8: 本地备份 */
    /*bit.9: 本地关机/重启 */

    uint32_t remote_right;          //远程权限
    /*bit.0: 远程控制云台*/
    /*bit.1: 远程手动录象*/
    /*bit.2: 远程回放 */
    /*bit.3: 远程设置参数*/
    /*bit.4: 远程查看状态、日志*/
    /*bit.5: 远程高级操作(升级，格式化，重启，关机)*/
    /*bit.6: 远程发起语音对讲*/
    /*bit.7: 远程预览*/
    /*bit.8: 远程请求报警上传、报警输出*/
    /*bit.9: 远程控制，本地输出*/
    /*bit.10: 远程控制串口*/
    /*bit.11: 远程查看参数 */
    /*bit.12: 远程管理模拟和IP camera */
    /*bit.13: 远程关机/重启 */

    uint8_t local_backup_right[MAX_CHANN_NUM];     //通道权限
    uint8_t net_preview_right[MAX_CHANN_NUM];

} sdk_user_right_t;

/*-----------------百度配置信息----------------------------*/
typedef struct sdk_baidu_info_s
{
    // 系统配置
    uint8_t user_id[MAX_USR_ID_LEN];             //百度服务器 提供的用户ID  根据该ID  获取access_token
    uint8_t access_token[MAX_ACCESS_TOKEN_LEN];  //保存获取的access_token
    //uint8_t device_name[MAX_ACCESS_TOKEN_LEN];  //用户给百度气的名字
    //uint64_t token_live ;  //access_token  的生命周期

    uint8_t image_quality ;			 //图像质量 0 -4 档: 高清，清晰，普通，一般，流畅   默认2
    uint8_t rec_status ;   			 // 录像状态  这个也只能控制 百度云的录像情况
    uint8_t video_enable ;   	     //  视频开启状态  0  不传输视频  1 传输视频  默认开启
    uint8_t audio_enable ;   	    // 声音开启状态  0  关闭声音 1 开启声音		 默认关闭

    uint8_t res[12];
} sdk_baidu_info_t;


/*-----------网络模块-----------------------*/
/*
 * 网络链接索引定义
 */
typedef enum _SDK_NET_IDX
{
	SDK_NET_IDX_ETH0    = 0x00, //eth0   sdk_eth_cfg_t
    SDK_NET_IDX_ETH1    = 0x01, //eth1   sdk_eth_cfg_t
    SDK_NET_IDX_PPPOE0  = 0x02, //pppoe  sdk_pppoe_t
    SDK_NET_IDX_WIFI0  	= 0x03, //wifi   sdk_wifi_cfg_t
    SDK_NET_IDX_3G0		= 0x04, //3g     sdk_3g_cfg_t
    SDK_NET_IDX_PPPOE1  = 0x05,
    SDK_NET_IDX_WIFI1   = 0x06,
    SDK_NET_IDX_3G1     = 0x07,
    SDK_NET_IDX_BUTT    = 0x08
} SDK_NET_IDX_E;


//这个表示网络状态
typedef enum _SDK_NET_MODE
{
	SDK_NET_MODE_AP      	= 0x00, //ap    在smartconfig里面也可以表示为需要等待接受广播并解析链接到AP
	SDK_NET_MODE_ETH		= 0x01, //eth1
	SDK_NET_MODE_WIFI    	= 0x02, //WIFI 在smartconfig里面表示为已经保存了SSID pwd 无须等待广播解析
	SDK_NET_MODE_PPPOE1   	= 0x03,
	SDK_NET_MODE_3G  		= 0x04,//wifi
	SDK_NET_MODE_BUTT     	= 0x05
} SDK_NET_MODE_E;

/*
 * 网络配置（子结构体）
 */
typedef struct sdk_ip_info_s
{
    uint8_t if_name[MAX_IF_NAME_LEN];//接口名称(保留)
    uint8_t ip_addr[MAX_IP_ADDR_LEN];//IP ADDR
    uint8_t mask[MAX_IP_ADDR_LEN];   //IP MASK
    uint8_t gateway[MAX_IP_ADDR_LEN];//网关
    uint8_t mac[MAX_MAC_ADDR_LEN];   //MAC STRING
    uint8_t dns1[MAX_IP_ADDR_LEN];   //DNS1
    uint8_t dns2[MAX_IP_ADDR_LEN];   //DNS2
    uint8_t enable_dhcp;             //是否开启DHCP
    uint8_t dns_auto_en;             //自动获取DNS(当开启DHCP时可自动获取DNS)
    uint8_t res[2];
} sdk_ip_info_t;

/*
 * 有线网口配置
 */
typedef struct sdk_eth_cfg_s
{
    sdk_ip_info_t ip_info; //IP信息配置

    uint8_t if_mode;       //SDK_NET_MODE_E
    uint8_t wifi_type;     //指定wifi 的类型
   // uint8_t is_setting;  
    uint8_t res[2];

} sdk_eth_cfg_t;

#define MAX_IP_LEN          20     //IP地址长度
#define NET_CARD_NAME_LEN   8      //IP地址长度

typedef struct _net_manager_info_s
{
    uint8_t enable;			    //控制管理线程运行的
    uint8_t current_mode;		//SDK_NET_MODE_E 当前网络的模式 有线 , AP , sta , 用来控制当前的WIFI模式
    uint8_t connect_status;		//   0  表示没有任何连接   1 表示连接上 ,
    uint8_t config_change;		//  0 没有改变配置    1 改变配置
    uint8_t enable_dhcp;        //是否开启DHCP
    uint8_t wifi_type;			//wifi 的类型
    uint8_t res[2]   ;
    uint8_t ssid[MAX_DEV_ID_LEN] ; //ssid 信息
    uint8_t passwd[MAX_PASSWD_LEN] ; //ssid 信息

    //存储当前 ip 信息
    uint8_t if_name[NET_CARD_NAME_LEN];// 网卡名字  "eth0" "ra0" "wlan0"
    uint8_t ip_addr[MAX_IP_LEN];//IP ADDR
    uint8_t mask[MAX_IP_LEN];   //IP MASK
    uint8_t gateway[MAX_IP_LEN];//网关
    uint8_t mac[MAX_IP_LEN];   //MAC STRING
    uint8_t dns1[MAX_IP_LEN];   //DNS1
    uint8_t dns2[MAX_IP_LEN];   //DNS2
    pthread_mutex_t manager_mutex;

} net_manager_info_t;


/*
 * wifi 配置
 */
typedef struct sdk_wifi_cfg_s
{

    //sdk_ip_info_t ip_info;

    uint8_t ssid[MAX_DEV_ID_LEN];		//服务区别号
    uint8_t pwd[MAX_PASSWD_LEN];		//密码
    uint32_t enable;				    //0：禁止，1：wifi client 2: wifi ap
    uint32_t encrypt_type;				//加密类型0:NONE  1:WPA  2:WPA2 3:WEP
    uint32_t auth_mode;					//认证方式0:NONE  1:EAP 2:PSK 3:OPEN 4:SHARED
    uint32_t secret_key_type;			//密钥管理方式 0:NONE  1:AES 2:TKIP 只对应于加密类型为WPA/WPA2的情况
	uint8_t secrity[32];
    uint32_t stat;					    //无线网卡连接状态 0 无连接 1 正在连接2 成功
} sdk_wifi_cfg_t;


/*
* 搜索
*/

//wifi信息
typedef struct _wifi_ap_info_s
{
    uint8_t ssid[MAX_DEV_ID_LEN];    /*ssid */

    uint8_t encrypt_type;				//加密类型0:NONE  1:WPA  2:WPA2 3:WEP
    uint8_t auth_mode;					//认证方式0:NONE  1:EAP 2:PSK 3:OPEN 4:SHARED
    uint8_t secret_key_type;			//密钥管理方式 0:NONE  1:AES 2:TKIP 只对应于加密类型为WPA/WPA2的情况

    uint8_t signal;   					// signal intensity 0--100%

    uint8_t res[4];

} wifi_ap_info_t;

#if 0
typedef struct _wifi_ap_info_s
{
    uint8_t ssid[MAX_DEV_ID_LEN];    /*ssid */

	uint8_t channel;
	uint8_t authmode;        /*0-OPEN, 1-SHARED, 2-WPAPSK/WPA2PSK*/
	uint8_t encryption;      /*0-NONE, 1-WEP, 2-TKIP/AES*/
    uint8_t signal;   					// signal intensity 0--100%
    uint8_t res[4];

} wifi_ap_info_t;
#endif


typedef struct sdk_wifi_search_s
{

    int   ap_count;  //热点数目
    wifi_ap_info_t  ap_info[MAX_AP_NUM];
} sdk_wifi_search_t;



/*
 * pppoe配置
 */
typedef struct sdk_pppoe_s
{
    sdk_ip_info_t ip_info;              //PPPOE 获取到的IP地址

    uint8_t user[MAX_USER_NAME_LEN];    //PPPOE 用户名
    uint8_t pass[MAX_PASSWD_LEN];       //PPPOE 密码
    uint8_t enable;                    	//是否开启PPPOE
    uint8_t if_no;                      //网络接口
    uint8_t res[2];                     //保留

} sdk_pppoe_t;



/*
 * 3g 配置
 */
typedef struct sdk_3g_cfg_s
{

    sdk_ip_info_t   ip_info;            //3g PPP 获取到的IP地址

    uint32_t enable;
    uint32_t mod_id;
    uint32_t stat;
    uint8_t  res[64];
} sdk_3g_cfg_t;



typedef struct network_info
{
	unsigned char ucNetworkinit; // 0: not-init, 1: has inited
	unsigned char ucNetworkType; // 0: ethernet, 1: wireless
	unsigned char ucNetworkState; // 0: no network, 1: network connected, 2: get ip
	unsigned char ucNeedreconfig; // 0: no need to re-config, 1: need to re-config network(change network)
}sdk_Network_Info;



typedef struct ApInfo
{
    unsigned char channel;
    unsigned char ssid[33];
    unsigned char mac[18];
    unsigned char secrity[23];
	struct ApInfo *next;
}Ap_Info;

typedef struct tag_wifi_config
{
	int status;
	int channel;	
	char ssid[33];
	char pwd[32];
	char bssid[20];	
	char secrity[23];	
}WiFi_Item_Config;



/*
 * 网络服务配置
 */
typedef struct sdk_net_mng_cfg_s
{

    uint8_t        def_if_no;                     //默认网络接口(当设备有多个网络链接时优先使用指定的链接)
    uint8_t        res[3];

    uint8_t        multicast[MAX_IP_ADDR_LEN];    //组播地址
    uint8_t        net_manager[MAX_IP_ADDR_LEN];  //管理服务器IP()

    uint16_t       net_manager_port;              //管理服务器PORT
    uint16_t       http_port;                     //DVR HTTP PORT
    uint16_t       dvr_cmd_port;                  //DVR SERVICE PROT
    uint16_t       dvr_data_port;                 //DVR DATA PORT
    uint16_t       dvr_talk_port;                 //DVR TALK PORT
    uint16_t       cloud_port;                    //云服务端口
    uint8_t        cloud_addr[128];               //云服务地址
    uint8_t        cloud_user[MAX_USER_NAME_LEN]; //云服务用户名
    uint8_t        cloud_pass[MAX_PASSWD_LEN];    //云服务密码
    uint32_t       cloud_interval;                //云服务更新间隔
} sdk_net_mng_cfg_t;



/*
 * 域名解析
 */

#define 	MAX_DDNS_USER_LEN   	64				// DDNS用户名最大长度
#define 	MAX_DDNS_PWD_LEN    	32				// DDNS密码最大长度
#define 	MAX_DDNS_NAME_LEN   	128				// DDNS域名最大长度

typedef enum _SDK_DDNS_TYPE
{
    SDK_DDNS_TYPE_DYNDNS = 0x00,	// Dyndns
    SDK_DDNS_TYPE_3322,				// 3322
    SDK_DDNS_TYPE_ORAY,				// Oray
    SDK_DDNS_TYPE_BUTT
} SDK_DDNS_TYPE_E;

typedef enum _SDK_DDNS_ORAY_USERTYPE
{
    SDK_ORAY_USERTYPE_COMMON = 0x01,	// 普通用户
    SDK_ORAY_USERTYPE_EXPERT,			// 专业用户
    SDK_ORAY_USERTYPE_BUTT
} SDK_DDNS_ORAY_USERTYPE;

typedef struct sdk_ddns_cfg_s
{
    uint8_t enable;						// 是否启用DDNS 0-否,1-是
    uint8_t type;						// DDNS类型,SDK_DDNS_TYPE_E
    uint8_t user_type;					// 用户类型,SDK_DDNS_ORAY_USERTYPE (仅花生壳有此选项)
    uint8_t res[1];					    // 保留
    uint32_t interval;					// 更新周期(单位:秒)
    uint32_t port;						// DDNS端口
    uint8_t server[MAX_DDNS_NAME_LEN];	// DDNS协议对应的服务器地址(IP和域名均可)
    uint8_t user[MAX_DDNS_USER_LEN];	// 用户名
    uint8_t pwd[MAX_DDNS_PWD_LEN];		// 密码
    uint8_t domain[MAX_DDNS_NAME_LEN];	// 用户申请的域名地址(目前 3322, dyndns 用此参数,多个domain使用','隔开)
    /*----------------------------------*/
    uint8_t client_ip[MAX_IP_ADDR_LEN];	// 设备端IP
    uint8_t domain_num;					// 解析出来的域名的个数
    uint8_t res1[3];                    // 保留
    uint8_t dn[16][MAX_DDNS_NAME_LEN]; 	// 服务器返回的域名
} sdk_ddns_cfg_t;

/*
 * 平台信息 子结构体
 *
 */

typedef struct sdk_screen_slot_s
{
    uint32_t magic;				/* 0x55aa55aa */
    uint32_t index[MAX_SCREEN_NUM];
} sdk_screen_slot_t;

/*
 * 平台信息
 *
 */

typedef struct sdk_platform_s
{
    uint8_t pu_id[MAX_USER_NAME_LEN];
    uint8_t cms_ip[MAX_IP_ADDR_LEN];            //平台mds ip
    uint8_t mds_ip[MAX_IP_ADDR_LEN];            //平台mds ip
    uint32_t cms_port;                          //平台cms端口号
    uint32_t mds_port;                          //平台mds端口号
    uint32_t protocol;                          //0:tcp   1:udp
    uint32_t plat_enable;                       //平台启用

    sdk_screen_slot_t slot;						//解码器屏号
} sdk_platform_t;


/*--------------------基本结构单元------------------------*/
/*
 * 时间结构体定义
 */
typedef struct sdk_time_s
{
    uint32_t year;
    uint32_t mon;
    uint32_t day;
    uint32_t hour;
    uint32_t min;
    uint32_t sec;
} sdk_time_t;

/*
 * 详细时间结构体定义
 */
typedef struct sdk_date_s
{
    sdk_time_t _time;
    uint8_t     wday;
    uint8_t     tzone;
	uint8_t     ntp_or_manual;
    uint8_t     res[2-1];
} sdk_date_t;



/*
 * 区域结构体定义
 */
typedef struct sdk_rect_s
{
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
} sdk_rect_t;

/*
 * 点结构体定义
 */
typedef struct sdk_point_s
{
    uint16_t x;
    uint16_t y;
} sdk_point_t;



/*----------- 音视频类型定义 视频流的帧头信息用到-----------*/

typedef enum _SDK_AUDIO_SAMPLE_RATE
{
    SDK_AUDIO_SAMPLE_R8K        = 0,   /* 8K Sample rate     */
    SDK_AUDIO_SAMPLE_R11_025K   = 1,   /* 11.025K Sample rate*/
    SDK_AUDIO_SAMPLE_R16K       = 2,   /* 16K Sample rate    */
    SDK_AUDIO_SAMPLE_R22050     = 3,   /* 22.050K Sample rate*/
    SDK_AUDIO_SAMPLE_R24K       = 4,   /* 24K Sample rate    */
    SDK_AUDIO_SAMPLE_R32K       = 5,   /* 32K Sample rate    */
    SDK_AUDIO_SAMPLE_R44_1K     = 6,   /* 44.1K Sample rate  */
    SDK_AUDIO_SAMPLE_R48K       = 7,   /* 48K Sample rate    */
    SDK_AUDIO_SAMPLE_BUTT,
} SDK_AUDIO_SAMPLE_RATE_E;

typedef enum _SDK_AUDIO_CODEC_FORMAT
{
    SDK_AUDIO_FORMAT_NULL	        = 0,   /*                    */
    SDK_AUDIO_FORMAT_G711A	        = 1,   /* G.711 A            */
    SDK_AUDIO_FORMAT_G711Mu	        = 2,   /* G.711 Mu           */
    SDK_AUDIO_FORMAT_ADPCM	        = 3,   /* ADPCM              */
    SDK_AUDIO_FORMAT_G726_16        = 4,   /* G.726              */
    SDK_AUDIO_FORMAT_G726_24        = 5,   /* G.726              */
    SDK_AUDIO_FORMAT_G726_32        = 6,   /* G.726              */
    SDK_AUDIO_FORMAT_G726_40        = 7,   /* G.726              */
    SDK_AUDIO_FORMAT_AMR	        = 8,   /* AMR encoder format */
    SDK_AUDIO_FORMAT_AMRDTX	        = 9,   /* AMR encoder formant and VAD1 enable */
    SDK_AUDIO_FORMAT_AAC	        = 10,  /* AAC encoder        */
    SDK_AUDIO_FORMAT_ADPCM_DVI4	    = 11,  /* ADPCM              */
    SDK_AUDIO_FORMAT_ADPCM_IMA	    = 12,  /* ADPCM              */
    SDK_AUDIO_FORMAT_MEDIA_G726_16  = 13,  /* G.726              */
    SDK_AUDIO_FORMAT_MEDIA_G726_24  = 14,  /* G.726              */
    SDK_AUDIO_FORMAT_MEDIA_G726_32  = 15,  /* G.726              */
    SDK_AUDIO_FORMAT_MEDIA_G726_40  = 16,  /* G.726              */
    SDK_AUDIO_FORMAT_BUTT,
} SDK_AUDIO_CODEC_FORMAT_E;

typedef enum _SDK_AUDIO_SAMPLE_WIDTH
{
    SDK_AUDIO_SAMPLE_WIDTH_8  = 1,    /* 8bits */
    SDK_AUDIO_SAMPLE_WIDTH_16 = 2,    /* 16bits */
    SDK_AUDIO_SAMPLE_WIDTH_BUTT,
} SDK_AUDIO_SAMPLE_WIDTH_E;


/*
 * 音频帧信息(子结构体)
 */
typedef struct sdk_a_frame_info_s
{
    uint8_t encode_type;        //编码类型 1: G711A SDK_AUDIO_CODEC_FORMAT_E
    uint8_t samples;            //采样频率 0: 8000, SDK_AUDIO_SAMPLE_RATE_E
    uint8_t bits;               //位宽    SDK_AUDIO_SAMPLE_WIDTH_E
    uint8_t channels;           //通道数
    uint8_t res[4];
} sdk_a_frame_info_t;


typedef enum _SDK_VIDEO_FORMAT
{
    SDK_VIDEO_FORMAT_H264    = 0, //H.264
    SDK_VIDEO_FORMAT_MPEG4   = 1, //MPEG4
    SDK_VIDEO_FORMAT_MJPEG   = 2, //MJPEG
} SDK_VIDEO_FORMAT_E;


typedef enum _SDK_VIDEO_STANDARD
{
    SDK_VIDEO_STANDARD_PAL  = 0, // PAL
    SDK_VIDEO_STANDARD_NTSC = 1, //NTSC
} SDK_VIDEO_STANDARD_E;

typedef enum _SDK_VIDEO_RESOLUTION
{
    SDK_VIDEO_RESOLUTION_QCIF    = 0 ,/* 176 * 144 */
    SDK_VIDEO_RESOLUTION_CIF     = 1 ,/* 352 * 288 */
    SDK_VIDEO_RESOLUTION_HD1     = 2 ,/* 704 * 288 */
    SDK_VIDEO_RESOLUTION_D1      = 3 ,/* 704 * 576 */
    SDK_VIDEO_RESOLUTION_960H    = 4 ,/* 960 * 576 */
    SDK_VIDEO_RESOLUTION_QVGA    = 5 ,/* 320 * 240 */
    SDK_VIDEO_RESOLUTION_VGA     = 6 ,/* 640 * 480 */
    SDK_VIDEO_RESOLUTION_XGA     = 7 ,/* 1024 * 768 */
    SDK_VIDEO_RESOLUTION_SXGA    = 8 ,/* 1400 * 1050 */
    SDK_VIDEO_RESOLUTION_UXGA    = 9 ,/* 1600 * 1200 */
    SDK_VIDEO_RESOLUTION_QXGA    = 10,/* 2048 * 1536 */
    SDK_VIDEO_RESOLUTION_WVGA    = 11,/* 854  * 480  */
    SDK_VIDEO_RESOLUTION_WSXGA   = 12,/* 1680 * 1050 */
    SDK_VIDEO_RESOLUTION_WUXGA   = 13,/* 1920 * 1200 */
    SDK_VIDEO_RESOLUTION_WQXGA   = 14,/* 2560 * 1600 */
    SDK_VIDEO_RESOLUTION_720p    = 15,/* 1280 * 720  */
    SDK_VIDEO_RESOLUTION_1080p   = 16,/* 1920 * 1080 */
    SDK_VIDEO_RESOLUTION_960p    = 17,/* 1280 * 960  */
    SDK_VIDEO_RESOLUTION_SVGA    = 18,/* 800  * 600  */
    SDK_VIDEO_RESOLUTION_BUTT      // ----------

} SDK_VIDEO_RESOLUTION_E;

/*
 * 视频帧信息(子结构体)
 */
typedef struct sdk_v_frame_info_s
{
    uint8_t encode_type;        //编码类型 0: H.264  1 mjeg
    uint8_t standard;           //制式     0: PAL, 1: NTSC
    uint8_t resolution;         //分辨率   ENUM_RESOLUTION_E
    uint8_t frame_rate;         //帧率     1-25/30
    uint16_t width;             //宽
    uint16_t height;            //高
} sdk_v_frame_info_t;



typedef enum _SDK_FRAME_TYPE
{
    SDK_VIDEO_FRAME_I = 0x01,   //I帧
    SDK_VIDEO_FRAME_P = 0x02,   //P帧
    SDK_VIDEO_FRAME_B = 0x03,   //B帧
    SDK_AUDIO_FRAME_A = 0x04,   //音频帧A
} SDK_FRAME_TYPE_E;

/*
 * 音视频帧结构体定义
 */
typedef struct sdk_frame_s
{
    uint16_t  magic;            //保留，扩展
    uint8_t   res[1];           //保留
    uint8_t   frame_type;       //帧类型, 视频（I, P, B）, 音频（A）
    uint32_t  frame_size;       //帧长度
    uint32_t  frame_no;         //帧序号
    uint32_t  sec;              //帧时间（秒）
    uint32_t  usec;             //帧时间（微秒）
    uint64_t  pts;              //帧PTS
    union
    {
        sdk_v_frame_info_t video_info;//视频帧信息
        sdk_a_frame_info_t audio_info;//音频帧信息
    };
    uint8_t   data[0];          //帧数据
} __attribute__ ((packed))sdk_frame_t;

typedef sdk_frame_t frame_t;


#define MAX_NAL_NUM             9              //最大NAL个数
//#define MAX_NAL_DESC_SIZE       ((4+9*(4+4)))  //视频帧数据前段部分长度

typedef    enum NAL_TYPE_S
{
    NAL_UNSPECIFIED = 0,
    NAL_NON_IDR,
    NAL_IDR=5,
    NAL_SEI,
    NAL_SPS,
    NAL_PPS,
    NAL_AUD
} NAL_TYPE_E;

typedef struct sdk_nal_desc_s
{
    uint32_t nal_num;           /*NAL个数*/
    struct nal_d_s
    {
        uint32_t nal_type;       /*NAL类型 NAL_TYPE_E*/
        uint32_t nal_off;       /*NAL地址,从frame->data开始*/
        uint32_t nal_size;      /*NAL大小*/
    } nal[MAX_NAL_NUM];
    uint8_t rawdata[0];         /* 纯编码数据 */
} sdk_nal_desc_t;


/*****************编码模块参数******************************/


/*------------ 编码参数设置 获取------*/
typedef enum _SDK_CODEC_BR_TYPE
{
    SDK_BR_CBR = 0, ///< Constant Bitrate  定码流
    SDK_BR_VBR,     ///< Variable Bitrate  变码流
    SDK_BR_ECBR,    ///< Enhanced Constant Bitrate
    SDK_BR_EVBR,    ///< Enhanced Variable Bitrate
    SDK_BR_BUTT

} SDK_VIDEO_BR_TYPE_E;
/*
 * 通道编码参数(子结构体)
 */
typedef struct sdk_av_enc_s
{
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
    uint8_t is_using;		//表示该通道配置是否正在使用[为1表示该通道配置当前生效]
    uint8_t  res2[1];
} sdk_av_enc_t;
/*
 * 编码参数索引(pmsg->extend传递)
 */
typedef enum _SDK_ENC_IDX
{
	SDK_ENC_IDX_MAIN  = 0,
	SDK_ENC_IDX_SECOND= 1,
	SDK_ENC_IDX_THREE = 2,
	SDK_ENC_IDX_ALL   = 4
} SDK_ENC_IDX_T;


/*
 * 通道编码参数 参数保存
 */
typedef struct sdk_encode_s
{
    sdk_av_enc_t av_enc_info[SDK_ENC_IDX_ALL];  //主码流  SDK_ENC_IDX_T
    uint32_t     au_in_mod;                     //前端音频输入类型
} sdk_encode_t;


/*----------------- 报警配置结构体定义 -----------------*/
/*
 * 时间段结构体定义
 */
typedef struct sdk_sched_time_s
{
    uint8_t enable;	    //激活, 当定时录像时表示录像类型;
    uint8_t res;
    uint8_t start_hour; //开始时间
    uint8_t start_min;
	uint8_t start_sec;
    uint8_t stop_hour;  //结束时间
    uint8_t stop_min;
	uint8_t stop_sec;
} sdk_day_time_t;

/*
*重新定义一个布放时间定义 以前的过于繁琐 其实这个也不简单 9527-----20140802
*
*/
//按月布防
typedef struct mon_sched_s
{
    uint32_t mon_enable;	 //每一位代表一个月 一年12月 第13位是1 的话 说明每个月都这样布防  切优先级最高
    uint32_t date_sched[MAX_DAY_NUM];  //数组索引代表一天  具体每一位代表一个小时 索引31代表在mon_enable 的bit13 为1的时候有效
} mon_sched_t;

//按周布防
typedef struct week_sched_s
{
    //uint8_t week_enable;	 //每一位代表一个星期 bit8  == 1 的时候  表示每天都按照则个计划 切优先级最高
    sdk_day_time_t week_sched_info[MAX_WEEK_NUM][MAX_TIME_SEG_NUM];//每天都有五个部分
} week_sched_t;


//两种布防方式配合使用
typedef struct sdk_sched_time_ex_s
{
    uint8_t enable;	    //  0:关闭   1:打开  布防时间总开关
    uint8_t plan_flag;	//  0: 全时布防,  1:按周布防    2:按月布防
    uint8_t interval_time;		//两次报警时间间隔 5~240S
    uint8_t res[1];		//保留

    union
    {
        //二者选其一
        mon_sched_t 	mon_sched;  //按月布防
        week_sched_t	week_sched; //按周布防
    };
} sdk_sched_time_t;



/*----------- 告警类型定义 -----------*/
//此处后期会根据实际的情况增加 类型
typedef enum _SDK_EVENT_TYPE
{
    SDK_EVENT_MANUAL_REC = 0x000,	//手动录像事件
    SDK_EVENT_TIMER_REC  		,   //定时录像事件
	SDK_EVENT_MOTION    		,   //移动侦测事件
    SDK_EVENT_AUDIOALERT 		,	//声音侦测
    SDK_EVENT_ALARM_IN  		,  	//输入告警事件
    SDK_EVENT_LOST     			,   //视频丢失事件
    SDK_EVENT_HIDE     			,   //视频遮挡事件
    SDK_EVENT_IR       			,   //433 一类的射频信号的告警 本质也是IO告警
    SDK_EVENT_HD_FORMAT  		,	//格式化   上报成功 或者失败
    SDK_EVENT_HD_ERR     		,   //磁盘错误事件
    SDK_EVENT_HD_NOT_EXIST		,	//录像盘不存在
    SDK_EVENT_FORMAT  			,	//格式化   上报成功 或者失败
    SDK_EVENT_PIR				, 	//PIR报警
    SDK_EVENT_ALL				,
    SDK_EVENT_BUTT
} SDK_EVENT_TYPE_E;
/*----------事件侦测-------------------*/


/*
 *  报警联动定义
 */
typedef struct sdk_alarm_handle_s
{


    uint8_t  record_enable;
    uint8_t  record_time;                 //录像延时时长 15秒 默认
    uint8_t  snap_enable;
    uint8_t  snap_num;                    //连续抓拍张数

    uint8_t  beep_enable;                 //是否蜂鸣器
    uint8_t  beep_time;                   //蜂鸣时间
    
    uint8_t  alarm_out_enable;			// 使能
    uint8_t  alarm_out_time;              //联动输出时间

    uint8_t  email_enable;               //发送邮件email 9527 20140802新增

    uint8_t led_enable;  //灯是否需要联动   如果有多颗灯的话 每一位代表一个灯  
    uint8_t led_time;     //灯持续闪烁时间

    uint8_t  res[2+4];                    //email, ftp, 3g;
} sdk_alarm_handle_t;

/*
 * 报警输入（IO）报警配置
 */
typedef struct sdk_alarm_in_cfg_s
{
    uint8_t name[MAX_ALARM_IN_NAME_LEN];                        //报警输入名称
    uint8_t type;                                               //报警输入类型，1：常开(默认)，0：常闭
    uint8_t enable;                                             //允许联动
    uint8_t res[2];                                             //保留
    sdk_sched_time_t sched_time;								//布防时间段
    sdk_alarm_handle_t alarm_handle;                            //联动处理
} sdk_alarm_in_cfg_t;


/*
 * 视频移动侦测配置
 */
typedef enum _SDK_VDA_MOD_E
{
    SDK_VDA_MD_MOD = 0, 
	SDK_VDA_OD_MOD = 1
} SDK_VDA_MOD_E;

typedef struct sdk_vda_codec_cfg_s
{
    uint8_t enable;                //允许联动
    uint8_t sensitive;             //灵敏度(0 :禁止  1:最低 2:低 3:中  4:高  5:最高  )
    uint8_t mode;                  /* 0:MD 移动 1:OD 遮挡 */
    uint8_t res;
    uint32_t mask;				 //按位 区域使能 最多课代表32个区域使能
    sdk_rect_t area[MAX_MOTION_NUM];  //区域
} sdk_vda_codec_cfg_t;


typedef struct sdk_motion_cfg_s
{
    sdk_vda_codec_cfg_t codec_vda_cfg;
    sdk_sched_time_t sched_time;                //布防时间段
    sdk_alarm_handle_t alarm_handle;            //联动处理
} sdk_motion_cfg_t;


//考虑后期直接何进sdk_motion_cfg_t中 布防时间与移动侦测同步公用
typedef struct sdk_audio_detect_cfg_s
{
    uint8_t enable;                //允许联动
    uint8_t sensitive;             //灵敏度(0 :禁止  1:最低 2:低 3:中  4:高  5:最高  )     //联动处理
} sdk_audio_detect_cfg_t;

typedef struct sdk_mirror_flip_cfg_s
{
    uint8_t mirror;                //水平反转(0:normal  1: mirror)
    uint8_t flip;             //垂直反转(0:normal  1: flip)
    uint8_t res[2];     		//保留
} sdk_mirror_flip_cfg_t;

typedef struct sdk_power_freq_cfg_s
{                
    uint8_t powerfreq;  // 0: 60hz  1: 50hz     
    uint8_t res[3];  
} sdk_power_freq_cfg_t;


/*
 * 视频遮挡侦测配置
 */
typedef struct sdk_hide_cfg_s
{
    uint8_t enable;                                         //允许联动
    uint8_t sensitive;                                          //灵敏度(0[灵敏度最高]----6[最低])
    uint8_t res[2];                                             //保留
    uint32_t mask;												 // 按位
    sdk_rect_t area[MAX_MOTION_NUM];                            //区域
    sdk_sched_time_t sched_time;								//布防时间段
    sdk_alarm_handle_t alarm_handle;                            //联动处理
} sdk_hide_cfg_t;

/*
 * 视频丢失侦测配置
 */
typedef struct sdk_lost_cfg_s
{
    uint8_t enable;                                             //允许联动
    uint8_t res[3];                                             //保留
    sdk_sched_time_t sched_time;								//布防时间段
    sdk_alarm_handle_t alarm_handle;                            //联动处理
} sdk_lost_cfg_t;
/*
 *
 *视频覆盖区域设置  参数保存
 */

typedef struct sdk_overlay_cfg_s
{
    uint8_t   enable;                  //是否开启
    uint8_t   max_num;
    uint8_t   res[2];
    uint32_t   mask;                    //bit.0: area[0], bit.1: area[1]
    sdk_rect_t area[MAX_OVERLAY_NUM];   //区域坐标
} sdk_overlay_cfg_t;



/*
 *
 *视频OSD区域(子结构体)
 */

typedef struct sdk_osd_info_s
{

    sdk_point_t pos;            //位置
    uint8_t     valid;          // 当前区域是否有效
    uint8_t     font;           // 字体类型
    // [7 6 5 4 - 3 2 1 0]
    // bit[4-7]: display type, 0: time, 1: string, 2 .....
    // bit[0-3]: display format
    //           sample time format:
    //           bit[2-3], 0: YYYYMMDD, 1: MMDDYYYY, 2: DDMMYYYY
    //           bit[1],   0: '-'       1: '/'
    //           bit[0],   0: 24hour    1: ampm
    uint8_t     format;         // 显示格式
    uint8_t     str_len;        // 字符串长度
    uint8_t     str[MAX_OSD_STR_LEN];//ascii+gb2312字符串
} sdk_osd_info_t;



/*
 *
 *视频OSD配置  包括图片 文字
 */
typedef struct sdk_osd_cfg_s
{
    sdk_osd_info_t  time;        //OSD时间
    sdk_osd_info_t  chann_name;  //OSD通道名称
    sdk_osd_info_t  ext_info;       //码流数据 或者图片信息
    uint32_t  osd_invert_color; // 背景色
    uint32_t  time_color;	//时间显示颜色
    uint32_t  chann_name_color; //通道信息显示颜色
    uint8_t  res2[32];

} sdk_osd_cfg_t;


/*
 *
 *图像属性配置
 */
typedef struct sdk_image_attr_s
{
    uint8_t brightness;         //亮度
    uint8_t contrast;           //对比度
    uint8_t saturation;         //饱和度
    uint8_t hue;                //色度
    uint8_t sharpness;          //锐度
    uint8_t icut_flag;          //标识
    uint8_t res[2];
} sdk_image_attr_t;


/*
 * 抓拍结构体定义 保存参数适用
 */
typedef struct sdk_snap_pic_s
{
    uint16_t width;             //宽度
    uint16_t height;            //高度
    uint8_t  encode;            //编码格式,JPEG 目前唯一支持这种格式
    uint8_t  quilty;            //图像质量 0 --100
    uint8_t  res[2];            //保留
} sdk_snap_pic_t;

/*
*  传输数据适用
*/

typedef struct sdk_snap_info_s
{
    uint16_t width;	//support range 0~ 1920
    uint16_t height;//support range 0 ~ 1080

    //uint32_t  image_quality;  //图片质量 0~100
    uint32_t  data_len;  //实际数据长度

    uint32_t  buffer_max_len;  //数据长度
    uint8_t	*data;  //存储图片的空间
} sdk_snap_info_t;

/*
* 连续抓拍情况下适用
*/

typedef struct sdk_snap_data_s
{
    uint8_t pic_num;  //抓拍总张数
    sdk_snap_info_t *snap_data;
} sdk_snap_data_t;

//感兴趣编码区域设置
typedef struct _sdk_roi_cfg_s
{
    uint32_t enable;						//1:开启	0: 不开启
    uint32_t abs_qp;						//0 相对质量 1 绝对质量
    uint32_t qp_val;						//质量值 0-51

    sdk_rect_t	roiRect[MAX_ROI_REG]; //区域  在8138s中只支持一个区
} sdk_roi_cfg_t;


//3d降噪
typedef struct _sdk_3di_cfg_s
{
    uint32_t deinterlace;  ///< temporal_deInterlace=1(enabled), 0(disabled)
    uint32_t denoise;  ///< temporal_deInterlace=1(enabled), 0(disabled)
} sdk_3di_cfg_t;


/* ------------SENSOR---------------------*/

typedef enum _SDK_SENSOR_TYPE_
{
    SENSOR_TYPE_OV9712 = 0,   /*      */
    SENSOR_TYPE_OV7725,
    SENSOR_TYPE_AR0130,
    SENSOR_TYPE_AR0330,
    SENSOR_TYPE_HM1375,
    SENSOR_TYPE_MT9P031

} SDK_SENSOR_TYPE;

typedef enum _SDK_STREAM_TYPE_
{
    STREAM_AUDIO_INDEX  		=	0  ,
    STREAM_VIDEO_INDEX_1    	=	1  ,
    STREAM_VIDEO_INDEX_2    	=	2  ,
    STREAM_VIDEO_INDEX_3    	=	3  ,
    STREAM_VIDEO_INDEX_4    	=	4  ,

    STREAM_VIDEO_INDEX_BUT
} SDK_STREAM_TYPE;

//音视频数据流
typedef struct _sdk_stream_info_s
{
    int ch;
    int sub_ch;
    uint64_t time;
    frame_t frame_head;
    char *frame_addr;

} sdk_stream_info_t;

// 这个需要 优化再定义 适应过重事件上报
typedef struct sdk_event_s
{
    SDK_EVENT_TYPE_E enevt_type;        //事件类型
    int event_time ;		//产生时间的时间 每8bit表示一个单位  xxHHMMSS
    int result;            //结果  1  start  0 stop
    int res[2];            //结果
} sdk_event_t;

typedef struct sdk_pir_event_s
{
	pthread_t 				pir_pthread_read_handle;//线程ID
	pthread_t 				pir_pthread_control_handle;//线程ID
	unsigned int			alarm_enable; //BIT16表示更改了等级
	unsigned int			pir_pthread_is_processing; //线程运行标志位
    SDK_EVENT_TYPE_E 		event_type;        //事件类型
    unsigned long 			event_time ;		//产生时间的时间
    unsigned long 			last_time;	 //上一次报警的时间
    int 					pir_status; //PIR报警当前的状态
    int 					md_status;
	unsigned int 			pir_setting; //设置是为0  设置完成为1  
    int res[2-1];            //结果
}sdk_pri_event_t;

/************/
//事件回调
typedef void (*EventCallback)(void *data); //事件回调 包括声音侦测  移动侦测 等需要向外部告警的信息
typedef void (*AudioAlertCallback)(); //声音侦测
typedef void (*FrameStatCallback)(void *data); //码流信息
typedef void (*AppMessageCallback)(void *data); //用户回调
typedef void (*SendstreamCallback)(void *data); //音视频数据回调

///struct stream_writeFrame_s;
//typedef uint32_t (*SendstreamCallback)(struct stream_writeFrame_s *writeFrame); //发送数据流

/*------------录像 回放-----------------*/
/*
*录像类型定义  查询条件也用这个
*/

typedef enum _SDK_RECORD_TYPE
{
    SDK_RECORD_TYPE_ALL 	 	= 0X0,
    SDK_RECORD_TYPE_MANU     	= 0x1,   // 手动
    SDK_RECORD_TYPE_SCHED    	= 0x2,  //   排程
    SDK_RECORD_TYPE_MOTION   	= 0x3,  // 移动侦测
    SDK_RECORD_TYPE_ALARM    	= 0x4,  // 报警输入
    SDK_RECORD_TYPE_AED      	= 0x5,  // 声音侦测
    SDK_RECORD_TYPE_BCD      	= 0x6,  // 待定
    SDK_RECORD_TYPE_TAMPER   	= 0x7,  //
    SDK_RECORD_TYPE_MOTION_PRE	= 0x8,  // 移动侦测 预录像
} SDK_RECORD_TYPE_E;


/*
 * 录像控制
 */
typedef struct sdk_manual_record_s
{
    uint8_t manual_record[MAX_CHANN_NUM];	// 手动录像
    uint8_t stop_record[MAX_CHANN_NUM];		// 禁止录像
    uint8_t res[MAX_CHANN_NUM];				// 保留位
} sdk_manual_record_t;

/*
 * 手动开启(停止)报警输入检测
 */
typedef struct sdk_manual_alarmin_s
{
    uint8_t enable_alarmin[MAX_ALARM_IN_NUM];	// 0:停止; 1:开启(默认全开启)
    uint8_t res[MAX_ALARM_IN_NUM];				// 保留位
} sdk_manual_alarmin_t;

/*
 * 手动开启(停止)报警输出
 */
typedef struct sdk_manual_alarmout_s
{
    uint8_t enable_alarmout[MAX_ALARM_OUT_NUM];	// 0:停止(默认全停止); 1:开启
    uint8_t res[MAX_ALARM_OUT_NUM];				// 保留位
} sdk_manual_alarmout_t;


/*
 * 录像备份
 */
typedef enum _SDK_BACKUP_OP
{
    SDK_BACKUP_START = 0x001,		//sdk_record_backup_t
    SDK_BACKUP_PROGRESS,		    //pmsg->chan:备份进度
    SDK_BACKUP_BUTT
} SDK_BACKUP_OP_E;

/*
 * 备份输出文件格式
 */
typedef enum _SDK_BACKUP_FMT
{
    SDK_BACKUP_FMT_JAV  = 0,     /* jav */
    SDK_BACKUP_FMT_AVI  = 1     /* avi */
} SDK_BACKUP_FMT_E;

typedef struct sdk_record_handle_s
{
    uint32_t  	item_handle[MAX_STOR_FRAGMENT_LEN];
    uint32_t 	item_size;				//段大小
    sdk_time_t 	start_time;          	//开始时间
    sdk_time_t 	end_time;           	//结束时间
    uint8_t  	res[4];					//保留
} sdk_record_handle_t;

typedef struct sdk_record_backup_s
{
    sdk_record_handle_t item_arr[MAX_BACKUP_ITEM];
    uint8_t item_num;				//实际备份段数
    uint8_t record_type;            //录像类型
    uint8_t out_fmt;                //备份输出文件格式  SDK_BACKUP_FMT_E
    uint8_t res[1];					//保留
} sdk_record_backup_t;
/*
 * 录像参数（子结构体）
 */
typedef struct sdk_record_sched_s
{
    uint8_t is_allday;                              //是否全天录像
    uint8_t record_type;                            //录像类型
    uint8_t res[2];                                 //保留
    sdk_sched_time_t sched_time[MAX_TIME_SEG_NUM];  //布防时间段

} sdk_record_sched_t;

/*
 * 录像参数
 */
typedef struct sdk_record_cfg_s
{
    uint8_t enable;                                //开启定时录像
    uint8_t record_types;						//保存录像类型 0 不录像 1 全时录像 2报警录像
    uint8_t res[3-1];                                 //保留
    sdk_record_sched_t record_sched[MAX_WEEK_NUM];  //布防时间段(0:星期日; 1:星期一,2:星期二,... ,6:星期六)

    uint32_t pre_record_time;                       //预录时间
    uint32_t record_duration_time;                  //录像保留时间

    uint8_t  enable_redundancy;                     //是否冗余备份
    uint8_t  enable_audio;                          //是否录制音频
    uint8_t  res2[2];                                //保留

} sdk_record_cfg_t;


/*
 * 录像记录定义
 */
typedef struct sdk_record_item_s
{
    uint8_t item_name[MAX_RECORD_ITEM_LEN]; //记录名称
    uint32_t item_handle[MAX_STOR_FRAGMENT_LEN];   //sizeof(stor_fragment_t)+6个res
    sdk_time_t start_time;                  //开始时间
    sdk_time_t stop_time;                   //结束时间
    uint32_t item_size;                     //数据大小

    uint8_t is_locked;                      //是否锁定
    uint32_t record_type;                    //录像类型
    // card no;
} sdk_record_item_t;

/*
 * 录像查询条件
 */
typedef struct sdk_record_cond_s
{
    uint8_t channel;              //通道  在IPC 里面 默认就是0 通道
    uint8_t res[3];                 //保留
    uint32_t record_type;            //录像类型  SDK_RECORD_TYPE_E
    sdk_time_t start_time;          //开始时间
    sdk_time_t stop_time;           //结束时间
    // card no;
} sdk_record_cond_t;


/*
 *
 * 回放控制命令字
 */
typedef enum _SDK_PB_GROUP_CONTROL
{
    SDK_PB_CONTROL_ONNE  = 0x00, 	//
    SDK_PB_CONTROL_PAUSE       , 	// 暂停
    SDK_PB_CONTROL_SETP        ,    // 单帧进
    SDK_PB_CONTROL_NORMAL      ,    // 正常
    SDK_PB_CONTROL_DRAG        ,    // 拖拽
    SDK_PB_CONTROL_FORWARD     ,    // 前进
    SDK_PB_CONTROL_BACKWARD    ,    // 后退
    SDK_PB_CONTROL_QUERY_TIME  ,	// 查询当前所播放帧的时间，单位秒

    SDK_PB_CONTROL_BUTT
} SDK_PB_GROUP_CONTROL_E;

/*
 *
 * 前进播放速度枚举 [--------- | ---- -> -> ----]
 */
typedef enum _SDK_PB_GROUP_CONTROL_FORWARD
{
    SDK_PB_CONTROL_FW_NORMAL = 0x00,  				// 正常
    SDK_PB_CONTROL_FF_1X,     						// 1倍快进 (FF-fast forward-向前快进)
    SDK_PB_CONTROL_FF_2X,    						// 2倍快进
    SDK_PB_CONTROL_FF_4X,         					// 4倍快进
    SDK_PB_CONTROL_FF_8X,							// 8倍快进
    SDK_PB_CONTROL_FF_16X,							// 16倍快进

    SDK_PB_CONTROL_SF_1_2X,						// 1倍慢放 (SF-slow forward-向前慢放)
    SDK_PB_CONTROL_SF_1_4X,						// 2倍慢放
    SDK_PB_CONTROL_SF_1_8X,						// 4倍慢放
    SDK_PB_CONTROL_SF_1_16X						// 8倍慢放

} SDK_PB_GROUP_CONTROL_FORWARD_E;

/*
 *
 * 后退播放速度枚举 [--- <- <- ---- | ----------]
 */
typedef enum _SDK_PB_GROUP_CONTROL_BACKWARD
{
    SDK_PB_CONTROL_BW_NORMAL = 0x00,  				// 正常
    SDK_PB_CONTROL_FB_1X,     						// 1倍快退 (FB-fast backward-向后快进)
    SDK_PB_CONTROL_FB_2X,   						// 2倍快退
    SDK_PB_CONTROL_FB_4X,         					// 4倍快退
    SDK_PB_CONTROL_FB_8X,							// 8倍快退
    SDK_PB_CONTROL_FB_16X,							// 16倍快退

    SDK_PB_CONTROL_SB_1_2X,						// 1倍慢退 (SB-slow backward-向后慢放)
    SDK_PB_CONTROL_SB_1_4X,						// 2倍慢退
    SDK_PB_CONTROL_SB_1_8X,						// 4倍慢退
    SDK_PB_CONTROL_SB_1_16X						// 8倍慢退

} SDK_PB_GROUP_CONTROL_BACKWARD_E;


/*
 * 回放通道组定义
 */
typedef struct sdk_pb_group_s
{
    sdk_time_t start_time;             //开始时间
    sdk_time_t stop_time;              //结束时间
    uint32_t main_chann;               //主通道号
    uint8_t chann_mask[MAX_CHANN_NUM];//通道掩码（多路回放时）
} sdk_pb_group_t;


/*****************************************/
/*
 * 日志操作类型
 */
typedef enum _SDK_LOG_OP
{
    SDK_LOG_QUERY = 0x01,  // 查询 sdk_log_item_t, sdk_log_cond_t
    SDK_LOG_EMPTY,         // 清空(全部)
    SDK_LOG_BACKUP,        // 备份
    SDK_LOG_DEL			   // 删除(按时间段)
} SDK_LOG_OP_E;

/*
 * 日志类型定义（主类型，最大支持32种）
 */
typedef enum _LOG_MAJOR_TYPE
{
    LOG_MAJOR_SYSTEM 		= 0x00000001,  //系统日志
    LOG_MAJOR_ALARM  		= 0x00000002,  //告警日志
    LOG_MAJOR_OPERATE		= 0x00000004,  //操作日志
    LOG_MAJOR_NETWORK		= 0x00000008,  //网络日志
    LOG_MAJOR_PARAM  		= 0x00000010,  //参数日志
    LOG_MAJOR_EXCEPTION	    = 0x00000020,  //异常日志

    LOG_MAJOR_ALL    		= 0xffffffff  //所有日志
} LOG_MAJOR_TYPE_E;

/*
 * 系统日志子类型
 */
typedef enum _L_SYSTEM_MINOR_TYPE
{
    L_SYSTEM_MINOR_STARTUP = 0x0001,   //开机
    L_SYSTEM_MINOR_SHUTDOWN,           //关机
    L_SYSTEM_MINOR_REBOOT,				//重启

    L_SYSTEM_MINOR_ALL     = 0xffff   //所有系统日志
} L_SYSTEM_MINOR_TYPE_E;

/*
 * 告警日志子类型
 */
typedef enum _L_ALARM_MINOR_TYPE
{
    L_ALARM_MINOR_DI_START = 0x0001,   //输入告警发生
    L_ALARM_MINOR_DI_STOP ,            //输入告警停止
    L_ALARM_MINOR_MD_START,            //移动侦测告警发生
    L_ALARM_MINOR_MD_STOP,             //移动侦测告警停止
    L_ALARM_MINOR_VL_START,			   //视频丢失告警发生
    L_ALARM_MINOR_VL_STOP,			   //视频丢失告警停止

    L_ALARM_MINOR_ALL		= 0xffff   //所有告警日志
} L_ALARM_MINOR_TYPE_E;

/*
 * 操作日志子类型
 */
typedef enum _L_OPERATE_MINOR_TYPE
{
    L_OPERATE_MINOR_LOGIN = 0x0001	// 登陆
                            , L_OPERATE_MINOR_LOGOUT 			// 注销
    , L_OPERATE_MINOR_USER_ADD			// 用户管理-增加
    , L_OPERATE_MINOR_USER_DEL			// 用户管理-删除
    , L_OPERATE_MINOR_USER_MODI			// 用户管理-修改
    , L_OPERATE_MINOR_SETTIME 			// 设置系统时间
    , L_OPERATE_MINOR_FORMAT_DISK 		// 格式化硬盘
    , L_OPERATE_MINOR_DEFAULT 			// 恢复默认
    , L_OPERATE_MINOR_UPGRADE 			// 升级
    , L_OPERATE_MINOR_PLAYBACK 			// 回放
    , L_OPERATE_MINOR_PTZ 				// 云台控制
    , L_OPERATE_MINOR_BACKUP 			// 备份
    , L_OPERATE_MINOR_RECORD_START 		// 启动录像
    , L_OPERATE_MINOR_RECORD_STOP 		// 停止录像
    , L_OPERATE_MINOR_CLR_ALARM 		// 清除报警
    , L_OPERATE_MINOR_TALKBACK_START 	// 对讲开始
    , L_OPERATE_MINOR_TALKBACK_STOP 	// 对讲结束
    , L_OPERATE_MINOR_LOG_DEL			// 删除日志
    , L_OPERATE_MINOR_LOG_EMPTY			// 清空日志
    , L_OPERATE_MINOR_LOG_BACKUP		// 备份日志
    , L_OPERATE_MINOR_MANUAL_RECORD_START 		// 手动启动录像
    , L_OPERATE_MINOR_MANUAL_RECORD_STOP 		// 手动停止录像
    , L_OPERATE_MINOR_FORMAT_DISK_U 	// 格式化U盘

    , L_OPERATE_MINOR_ALL	= 0xffff	// 所有操作日志
} L_OPERATE_TYPE_E;

/*
 * 参数日志子类型
 */
typedef enum _L_PARAM_MINOR_TYPE
{
    L_PARAM_MINOR_PARAM_NETWORK = 0x0001 	// 网络参数配置
                                  , L_PARAM_MINOR_PARAM_UART 				// 串口参数配置
    , L_PARAM_MINOR_PARAM_PTZ 				// 云台参数配置
    , L_PARAM_MINOR_PARAM_CHAN 				// 通道参数配置
    , L_PARAM_MINOR_PARAM_VENC 				// 编码参数配置
    , L_PARAM_MINOR_PARAM_TV 				// TV参数配置
    , L_PARAM_MINOR_PARAM_PIC_ATTR			// 图像属性配置
    , L_PARAM_MINOR_PARAM_RECORD			// 录像参数配置
    , L_PARAM_MINOR_PARAM_MOTION			// 移动侦测配置
    , L_PARAM_MINOR_PARAM_VIDEOLOST			// 视频丢失配置
    , L_PARAM_MINOR_PARAM_ALARMIN			// 输入告警配置
    , L_PARAM_MINOR_PARAM_POLL				// 轮巡配置
    , L_PARAM_MINOR_PARAM_PREVIEW			// 预览配置
    , L_PARAM_MINOR_PARAM_OVERLAY			// 遮挡区域配置
    , L_PARAM_MINOR_PARAM_OSD				// OSD配置
    , L_PARAM_MINOR_PARAM_COMM				// 常规配置

    , L_PRARM_MINOR_ALL	= 0xffff			// 所有参数日志
} L_PARAM_MINOR_TYPE_E;

/*
 * 异常日志子类型
 */
typedef enum _L_EXCEPTION_MINOR_TYPE
{
    L_EXCEPTION_HD_FULL 		= 0x0001,   	//硬盘满
    L_EXCEPTION_HD_ERROR,           		    //硬盘错误
    L_EXCEPTION_NET_DISCONNECT,		        	//网络断开
    L_EXCEPTION_IP_CONFLICT,			    	//IP冲突

    L_EXCEPTION_MINOR_ALL     = 0xffff   	//所有异常日志
} L_EXCEPTION_MINOR_TYPE_E;

/*
 * 日志记录定义
 */
typedef struct sdk_log_item_s
{
    sdk_time_t  time;                       //时间
    uint32_t    major_type;                 //类型LOG_MAJOR_TYPE_E
    uint32_t    minor_type;                 //0x0000:直接显示desc内容，否则通过L_XXX_MINOR_TYPE_E解析内容.
    uint32_t    args;                       /*子类型参数(args=0时不用解析,否则根据minor_type解析内容。
    										如果minor_type和通道号相关，args表示通道号;如果minor_type和告警相关，args表示告警输入号)*/
    uint8_t     user[MAX_USER_NAME_LEN];    //操作用户
    uint8_t     ip_addr[MAX_IP_ADDR_LEN];   //用户IP
    uint8_t     desc[MAX_LOG_DESC_LEN];     //日志内容(minor_type=0时有效)
} sdk_log_item_t;


/*
 * 日志查询条件
 */
typedef struct sdk_log_cond_s
{
    sdk_time_t  begin_time;                 //开始时间
    sdk_time_t  end_time;                   //结束时间
    uint32_t    type;                       //类型
    uint8_t     user[MAX_USER_NAME_LEN];    //操作用户
    uint8_t     ip_addr[MAX_IP_ADDR_LEN];   //用户IP
} sdk_log_cond_t;



/*
 * 报警信息定义
 */
typedef struct sdk_alarm_info_s
{
    sdk_time_t  time;                   //报警时间
    uint32_t    type;                   //报警类型
    uint32_t    args;                   //报警参数
    uint8_t     desc[MAX_ALARM_DESC_LEN];//报警内容
} sdk_alarm_info_t;


/*
 * 串口参数配置
 */
typedef struct sdk_serial_param_s
{
    uint32_t baud_rate;             //串口波特率 115200, 57600, ...
    uint8_t  data_bit;              //数据位 5, 6, 7, 8
    uint8_t  stop_bit;              //停止位 1, 2
    uint8_t  parity;                //校验 0:无，1:奇校验 2:偶校验
    uint8_t  flow_ctl;              //流控
} sdk_serial_param_t;

/*
 * 串口用途配置
 */
typedef struct sdk_serial_func_cfg_s
{
    uint8_t type;	// 0: normal, 1: ptz, 2...
    uint8_t res[3];
    sdk_serial_param_t serial_param;
} sdk_serial_func_cfg_t;

/*-------------升级功能------------------------*/

/*
*发送二进制数据流
*/
typedef struct _sdk_bin_stream_s

{
    uint32_t type;							//二进制数据类型
    uint32_t flag;							//包的标志
    uint32_t seq;							//包的序号
    uint32_t size;							//整个结构体的 size
    uint8_t data[0];						//附加二进制数据起始位置
} sdk_bin_stream_t;

//SDK_MAIN_MSG_UPGRAD
typedef struct _sdk_upgrad_pkg_info_s
{
    uint32_t upgrad_session;   // 服务端返回给客户端一个回话ID
    uint32_t total_packet;     //  总的包数
    uint32_t total_len;		  // 升级文件总长度
} sdk_upgrad_pkg_info_t;

/*
 * U盘查询升级包文件列表
 */
typedef struct upgrad_pkg_file_s
{
    uint8_t   device_type[16];//设备类型
    uint8_t   ver[16];        //版本
    uint8_t   date[16];       //日期
    uint32_t  size;           //大小
    uint8_t   name[64];       //名字 aa.upg
    uint8_t   file_path[128];  //升级文件全路经 /tmp/aa.upg
} upgrad_pkg_file_t;


/*
 * 网络升级流程： SDK_UPGRAD_REQUEST => recv upg_pkg => SDK_UPGRAD_DISK_FILE => SDK_UPGRAD_PROGRESS;
 * U盘升级流程 :  SDK_UPGRAD_FIND_FILE => SDK_UPGRAD_DISK_FILE => SDK_UPGRAD_PROGRESS;
 */
typedef enum _SDK_UPGRAD_OP
{
    //网络接收升级文件
    SDK_UPGRAD_NET_REQUEST = 0x0, // 升级请求 sdk_upgrad_pkg_info_t
    SDK_UPGRAD_NET_RECV_UPG	 	, // 接收数据流sdk_bin_stream
    //开始升级的流程
    SDK_UPGRAD_REQUEST          , //升级请求	upgrade_packet_t
    SDK_UPGRAD_DISK_FILE        , //升级包在磁盘文件
    SDK_UPGRAD_MEM_FILE         , //升级包在内存
    SDK_UPGRAD_PROGRESS         , //升级进度
    SDK_UPGRAD_FIND_FILE          //U盘查询升级包文件列表	upgrad_pkg_file_t
} SDK_UPGRAD_OP_E;



/*
 * 云台配置参数
 */
/*
*  _0: debug, _1: ptz/normal, _2: rs232, _3 mcu
*  -------------------------, _2: mcu;
*  0: 1
*
*/
typedef struct sdk_ptz_param_s
{

    sdk_serial_param_t comm;                //串口参数，gui固定只配置串口1
    uint8_t          address;               //云台地址
    uint8_t          protocol;              //云台协议SDK_PTZ_PROTOCOL_E
    uint8_t          res[2];
} sdk_ptz_param_t;

/*
 * 预置点定义
 */
typedef struct sdk_preset_s
{
    uint8_t enable;         //是否设置
    uint8_t no;             //预置点号
    uint8_t res[2];         //保留
    uint8_t name[32];       //预置点名称
} sdk_preset_t;


/*
 * 预置点集合(每个通道一个集合)
 */
typedef struct sdk_preset_param_s
{
    uint8_t         num;                       //已设置的预置点个数
    uint8_t         res[3];                    //保留
    sdk_preset_t    preset_set[MAX_PRESET_NUM];//预置点
} sdk_preset_param_t;


/*
 *DVR实现巡航数据结构(每个通道一条路径)
 */
typedef struct sdk_cruise_param_s
{
    uint8_t enable;                           //是否启动
    uint8_t res[2];                           //保留
    uint8_t no;                               //这条巡航路径的号码(暂时没用，发什么给gui，gui回什么)
    uint8_t name[32];                         //巡航路径名称(暂时没用，发什么给gui，gui回什么)
    uint8_t preset[MAX_CRUISE_PRESET_NUM];    //预置点
    uint8_t speed[MAX_CRUISE_PRESET_NUM];     //移动速度
    uint8_t dwell_time[MAX_CRUISE_PRESET_NUM];//停留时间
} sdk_cruise_param_t;
/**********************************************/



typedef enum _SDK_LANUAGE_TYPE
{
    SDK_LANUAGE_TYPE_CHINA	= 0x00,		// 汉语
    SDK_LANUAGE_TYPE_ENGLISH= 0x01,		// 英语
    SDK_LANUAGE_TYPE_MAX	= 0xff		 //最大是支持数

} SDK_LANUAGE_TYPE_E;

typedef enum _SDK_POWER_HZ_
{
    SDK_POWER_HZ_50HZ	= 0x00,		// 50hz
    SDK_POWER_HZ_60HZ	= 0x01		// 60hz
} SDK_POWER_HZ_E;


/*
 *由于地域原因会产生以下参数的改变一下参数归地域参数
 */
typedef struct sdk_regional_param_s
{
    uint8_t pow_hz;                           //电源频率    	0 :50hz   1:60hz
    uint8_t lanuage;                          //语言选择   		对应SDK_LANUAGE_TYPE_E枚举
    uint8_t in_out_door;                      //室内/外模式   	0:室内模式 1:室外模式
    uint8_t res;                              //保留

} sdk_regional_param_t;
/**********************************************/

/*********直接通过IO控制的设备如下*********/
/****************包括后期接入的433
或者其他的mcu 或者外设 全部都是iodev
*********/
typedef struct sdk_buzzer_param_s
{
    uint8_t buzzer;                           //蜂鸣器控制   1:开启吧     0:关闭
    uint8_t res[3];                           //保留
} sdk_buzzer_param_t;

typedef struct sdk_led_param_s
{
    uint8_t led;                           //LED 等编号   与硬件定义结合确定
    uint8_t enable;                      // 0 关闭  1 使能
    uint8_t res[2];                           //保留
} sdk_led_param_t;

typedef struct sdk_iodev_param_s
{
    sdk_buzzer_param_t buzzer_param;
    sdk_led_param_t led_param;

} sdk_iodev_param_t;



typedef struct sdk_upgrade_info_s
{
    int progress;//升级进度 按照%
    uint8_t old_version[MAX_VER_STR_LEN];		//上一个版本版本号
    uint8_t url[MAX_URL_LEN];		            //升级包下载地址
} sdk_upgrade_info_t;

////////////////////////////////////////////led 状态20150526
typedef struct
{
    char led_status[MAX_LED_NUM];	 //led_status[0]:   red----1:on 0:off
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
    int	 buzzer_time;  //  0 到 120 秒

} SMsgAVIoctrlSetBuzzerStatusReq, SMsgAVIoctrlGetBuzzerStatusResp;

typedef struct
{
    int 			result; 	//return 0 if succeed
    unsigned char	reserved[4];
} SMsgAVIoctrlSetBuzzerStatusResp;

////////////////////////////////////////////


typedef struct tagMotionControl
{
    int MotionLevel; // 1~5
    int rsvd[3];
    unsigned long long ullbitsMDstate;   // 1bit mean 1 sec
}MotionControl;



#if 0
typedef enum  _SDK_PTZ_CTRL_E
{
    PTZ_CTRL_STOP					= 0,
    PTZ_CTRL_UP						= 1,
    PTZ_CTRL_DOWN					= 2,
    PTZ_CTRL_LEFT					= 3,
    PTZ_CTRL_LEFT_UP				= 4,
    PTZ_CTRL_LEFT_DOWN				= 5,
    PTZ_CTRL_RIGHT					= 6,
    PTZ_CTRL_RIGHT_UP				= 7,
    PTZ_CTRL_RIGHT_DOWN				= 8,
    PTZ_CTRL_AUTO					= 9,
    PTZ_CTRL_SET_POINT				= 10,
    PTZ_CTRL_CLEAR_POINT			= 11,
    PTZ_CTRL_GOTO_POINT				= 12,

    PTZ_CTRL_SET_MODE_START			= 13,
    PTZ_CTRL_SET_MODE_STOP			= 14,
    PTZ_CTRL_MODE_RUN				= 15,

    PTZ_CTRL_MENU_OPEN				= 16,
    PTZ_CTRL_MENU_EXIT				= 17,
    PTZ_CTRL_MENU_ENTER				= 18,

    PTZ_CTRL_FLIP					= 19,
    PTZ_CTRL_START					= 20,

    PTZ_CTRL_LENS_APERTURE_OPEN			= 21,
    PTZ_CTRL_LENS_APERTURE_CLOSE		= 22,

    PTZ_CTRL_LENS_ZOOM_IN				= 23,
    PTZ_CTRL_LENS_ZOOM_OUT				= 24,

    PTZ_CTRL_LENS_FOCAL_NEAR			= 25,
    PTZ_CTRL_LENS_FOCAL_FAR				= 26,

    PTZ_CTRL_AUTO_PAN_SPEED				= 27,
    PTZ_CTRL_AUTO_PAN_LIMIT				= 28,
    PTZ_CTRL_AUTO_PAN_START				= 29,

    PTZ_CTRL_PATTERN_START				= 30,
    PTZ_CTRL_PATTERN_STOP				= 31,
    PTZ_CTRL_PATTERN_RUN				= 32,

    PTZ_CTRL_SET_AUX					= 33,
    PTZ_CTRL_CLEAR_AUX					= 34,
    PTZ_CTRL_MOTOR_RESET_POSITION		= 35,
    PTZ_CTRL_AUTO_LEFT_RIGHT		= 36,	//左右巡航
    PTZ_CTRL_AUTO_UP_DOWN			= 37,	//上下巡航
    PTZ_CTRL_AUTO_LEFT_RIGHT_STOP	= 38,	//左右巡航
    PTZ_CTRL_AUTO_UP_DOWN_STOP		= 39,	//上下巡航
    PTZ_CTRL_AUTO_TRACK				= 40,	//轨迹巡航

    PTZ_CTRL_AUTO_LEFT_RIGHT_EX		= 41,	//左右巡航(不自动停止)
    PTZ_CTRL_AUTO_UP_DOWN_EX		= 42,	//上下巡航(不自动停止)
} SDK_PTZ_CTRL_E;

typedef struct sdk_ptz_cmd_s
{
    unsigned char control;	// PTZ control command, refer to SDK_PTZ_CTRL_E
    unsigned char speed;	// PTZ control speed
    unsigned char point;	// no use in APP so far. preset position, for RS485 PT
    unsigned char limit;	// no use in APP so far.
    unsigned char aux;		// no use in APP so far. auxiliary switch, for RS485 PT
    unsigned char channel;	// camera index
    unsigned char reserve[2];
} sdk_ptz_cmd_t;

#endif

/*************************** 用户参数设置命令**************************/
/*****所有命令除恢复出厂设置 reboot外  配合参数分为 GET/SET *****/
/*SDK userComand */

typedef enum _SDK_USER_COMMAND_TYPE
{
    SDK_USRCMD_DEVICE_INFO           = 1 ,//设备信息
    SDK_USRCMD_PARAM_FACTORY  		 = 4, //恢复出厂设置
    SDK_USRCMD_WIFI_INFO   			 = 12,//wifi参数
    SDK_USRCMD_REBOOT   			 = 13,//重启设备
    SDK_USRCMD_IMAGE_QUALITY		 = 14,//图像质量
    SDK_USRCMD_REC_STATUS   		 = 15,//录像查询
    SDK_USRCMD_MOTION_DETE		     = 17,//移动侦测查询
    SDK_USRCMD_AUDIO_STATUS			 = 22,//声音控制状态查询
    SDK_USRCMD_SNAPSHOT				 = 23,//抓拍图片
    SDK_USRCMD_TIME_INFO			 = 24,//时间参数
    SDK_USRCMD_UNDETERMINED			 = 25,//保留项
    SDK_USRCMD_PIR_STATUS			 = 26,//红外报警状态
    SDK_USRCMD_PERIPHERAL_SATTUS	 = 27,//外设参数(蜂鸣器，led灯等)  废弃  34是自己的喇叭   38是外设报警器
    SDK_USRCMD_COLOR_PARAM           = 28,//色彩参数
    SDK_USRCMD_AUDIO_DETECT          = 29,//声音侦测设置
    SDK_USRCMD_UPGRADE				 = 30,//升级(开始或查询进度)
    SDK_USRCMD_CODE					 = 31,//编码参数
    SDK_USRCMD_POWER				 = 32,//电源频率
    SDK_USRCMD_ALARM_AREA			 = 33,//报警防区  (不行出是否是移动侦测区域  还是红外烟雾报警器等的应用场景)
    SDK_USRCMD_ALARM_OUT			 = 34,//报警输出(蜂鸣器控制)
    SDK_USRCMD_ARMING_24			 = 35,//24小时布防撤防
    SDK_USRCMD_ARNING_PLAN			 = 36,//排程布防撤防
    SDK_USRCMD_LANGUAGE				 = 37,//摄像机语种
    SDK_USRCMD_ALARM_EXTERNAL		 = 38,//外部报警开关
    SDK_USRCMD_GETWIFILIST           = 39,//获取wifilist
    SDK_USRCMD_IN_OUT_DOOR		     = 40,//室内外模式

} SDK_USR_CMD_E;


typedef enum _RECORD_FLAGS
{
	RECORD_NO = 0,
	RECORD_FULLTIME,
	RECORD_ALARM,
	RECORD_NULL
}BUZZER_FLAGS;

typedef enum _SHCEDULE_RECORD_FLAGS
{
	RECORD_OFF = 0,
	RECORD_ON,
}SHCEDULE_FLAGS;

typedef struct _SDK_RECORD_STATUS_
{				
	unsigned int 	pthread_is_processing;
	pthread_mutex_t record_lock;

	unsigned int	record_flags; //录像状态 由保存的参数和TUTK下发的参数来设置
	unsigned int	record_last_status; //上一次的录像状态
	unsigned int	record_alarm_triger; //报警录像时用来作为触发录像的开关
	unsigned int    record_alarm_opened; //表示start过了

	unsigned int    record_alarm_time; //报警录像时长
	pthread_t  		record_pthread_handle;//线程句柄
	//wu add it to recorde sched 2016-12-28
	//sdk_record_sched_t record_sched[MAX_WEEK_NUM];	//布防时间段(0:星期日; 1:星期一,2:星期二,... ,6:星期六)

	unsigned int    md_interval_time; 	//移动侦测的时间间隔  【暂放在这个结构体】
}RECORD_STATUS;  //0 disable record 		1 record fulltime    2 record alarm


//亚马逊结构体
typedef enum
{
	AWS_FULLTIME = 0,
	AWS_ALARM,
	AWS_NO =255
}AWS_TYPE;

/*typedef struct _sdk_aws_status_
{
	pthread_t 		aws_pthread_handle;//亚马逊上传线程ID
	pthread_t 		aws_snap_pthread_handle;//报警抓图线程ID
	unsigned int	aws_pthread_is_processing;
	pthread_mutex_t aws_lock;			//锁
	unsigned int 	full_or_alarm;		//全时上传还是报警上传
	unsigned int 	alarm_trigger;		//触发报警上传
	unsigned int    snap_flags;		//抓图标志位
	//unsigned int	snap_finish;		//是否完成抓图    完成抓图时才上传
	unsigned int 	alarm_time;			//报警的时长 == 报警录像的时长
	unsigned int	current_fps;		//当前帧率     用以计算上传多少帧
	unsigned int	aws_flags;			
	//BIT16 表示FULLTIME时是否找到I帧  BIT17表示报警录像时是否找到I帧
	//bit0-15表示报警录像的计数值
	//BIT 31 表示处理完毕 该帧可以上传了
} aws_status;*/

typedef enum
{
	BUF_EMPTY =0,	//BUF 里面的数据已无用 可以清0
	BUF_HAVE_DATA, //BUF里面有正常的数据 但缓存还未满
	BUF_FULL		//BUF 已满   也可以指缓存时间已到
}AWS_BUF_FLAGS;
typedef enum
{
	STATUS_DAY =0,
	STATUS_NIGHT =1,
	STATUS_NULL		
}AWS_FRAME_STATUS;


typedef struct _sdk_aws_status_
{
	unsigned int	aws_pthread_is_processing;
	pthread_mutex_t aws_lock;			//锁
	unsigned int 	full_or_alarm;		//全时上传还是报警上传
	unsigned int 	alarm_trigger;		//触发报警上传
	unsigned int    snap_flags;		//抓图标志位
	//unsigned int	snap_finish;		//是否完成抓图    完成抓图时才上传
	unsigned int 	alarm_time;			//报警的时长 == 报警录像的时长
	unsigned int	current_fps;		//当前帧率     用以计算上传多少帧
	unsigned int	aws_flags;			
	//BIT16 表示FULLTIME时是否找到I帧  BIT17表示报警录像时是否找到I帧
	//BIT18表示打开视频流了
	//bit0-15表示报警录像的计数值
	//BIT 31 表示处理完毕 该帧可以上传了
	pthread_t 		aws_video_pthread_handle;//亚马逊上传线程ID
	pthread_t 		aws_audio_pthread_handle;//亚马逊上传线程ID
	pthread_t 		aws_upload_pthread_handle;//亚马逊上传线程ID
	unsigned char *video_buf;
	unsigned int  video_len;
	AWS_BUF_FLAGS video_buf_flags; //缓存标志位
	struct timeval  video_time; //缓存第一个I帧的时间
	unsigned char  frame_rate; //缓存第一个I帧的帧率
	pthread_cond_t aws_cond ; //aws条件变量 用以 发送线程通知缓存线程
	unsigned char  send_finish; //发送线程发送完毕标志位 0 已发完 1 未发完
	unsigned int iframe_count;
	AWS_FRAME_STATUS day_or_night;//白天还是夜视 0白天 1夜视
	int time_zone;
	void * audio_handle ;
} aws_status;


typedef struct sdk_aws_info_s
{

    uint8_t aws_record_status;
    uint8_t res[3];
} sdk_aws_info_t;


typedef enum _ARM_STATUS
{
	ARM_DISABLE =0,
	ARM_ENABLE
}ARM_STAUS;

typedef enum _LED_STATUS  //定义LED的优先级
{
	LED_IS_ENABLE =0, 	// 布防下使能 不布防不使能  涉及到>= OR >  
	LED_NORMAL,       	//正常运行情况下	
	LED_ARM,     	 	//布防下
	LED_MONITOR,  		//观看视频下
	LED_ALARM_TRIGGER, 	//触发报警时
	LED_PANIC,			//手动
	LED_FORCE_CLOSE,	//	强制关闭 
	LED_NO_WIFI,		//没有网络的情况 AP
	LED_WIFI_CONFIGING,  //WIFI 配置中
	LED_RESTART,		//重启下
	LED_ALARM_MONITOR=24 // 报警触发时是否于观看状态
	//LED_ALARM_AND_MONITOR=25 // 报警触发时,	处于观看状态
}LED_STATUS;
typedef enum _BUZZER_STATUS
{
	BUZZER_IS_ENABLE =0,//布防了 使能  不布防不使能 涉及到IF判断的>= OR >  
	BUZZER_ENABLE_NO_BEEP,//不响
	BUZZER_ENABLE_BEEP,  //蜂鸣器响
	BUZZER_TESTING		//手动强制
}BUZZER_STATUS;
typedef struct _led_buzzer_info_
{
	pthread_t 		led_buzzer_pthread_pid;//线程ID
	unsigned int	led_buzzer_pthread_is_processing;
	pthread_mutex_t 	led_buzzer_lock;
	ARM_STAUS  arm_status; //是否布防
	unsigned int led_status;//LED的状态
	unsigned char led_time; //led 时间
	BUZZER_STATUS buzzer_status; //蜂鸣器的状态 //测试状态 OR 正常状态
	unsigned char buzzer_time; //蜂鸣器响的时长
}LED_BUZZER_INFO;

#ifdef __cplusplus
}
#endif

#endif //__sdk_struct_h__
