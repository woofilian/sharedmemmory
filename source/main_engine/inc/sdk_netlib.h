/*
 * =====================================================================================
 *
 *       Filename:  sdk_netlib.h
 *
 *    Description:  网络接入模块数据结构定义 
 *
 *        Version:  1.0
 *        Created:  2011年05月22日 21时08分09秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  
 *        Company:  
 *
 * =====================================================================================
 */
#ifndef __sdk_netlib_h__
#define __sdk_netlib_h__

#include "sdk_struct.h"

/*----------- 网络命令字定义 -----------*/


/*
 * 网络模块消息ID定义
 */

typedef enum _SDK_NET_PTZ_ID {
      SDK_NET_PTZ_UP_START = 0x001      //云台控制-上
    , SDK_NET_PTZ_UP_STOP
    , SDK_NET_PTZ_DOWN_START            //云台控制-下
    , SDK_NET_PTZ_DOWN_STOP
    , SDK_NET_PTZ_LEFT_START            //云台控制-左
    , SDK_NET_PTZ_LEFT_STOP
    , SDK_NET_PTZ_RIGHT_START           //云台控制-右
    , SDK_NET_PTZ_RIGHT_STOP
    , SDK_NET_PTZ_LEFT_UP_START         //云台控制-左上
    , SDK_NET_PTZ_LEFT_UP_STOP
    , SDK_NET_PTZ_RIGHT_UP_START        //云台控制-右上
    , SDK_NET_PTZ_RIGHT_UP_STOP
    , SDK_NET_PTZ_LEFT_DOWN_START       //云台控制-左下
    , SDK_NET_PTZ_LEFT_DOWN_STOP
    , SDK_NET_PTZ_RIGHT_DOWN_START      //云台控制-右下
    , SDK_NET_PTZ_RITHT_DOWN_STOP
    , SDK_NET_PTZ_PRESET_SET            //云台控制-预置位
    , SDK_NET_PTZ_PRESET_CALL
    , SDK_NET_PTZ_PRESET_DEL
    , SDK_NET_PTZ_ZOOM_ADD_START        //云台控制-变倍
    , SDK_NET_PTZ_ZOOM_ADD_STOP
    , SDK_NET_PTZ_ZOOM_SUB_START
    , SDK_NET_PTZ_ZOOM_SUB_STOP
    , SDK_NET_PTZ_FOCUS_ADD_START       //云台控制-聚焦
    , SDK_NET_PTZ_FOCUS_ADD_STOP
    , SDK_NET_PTZ_FOCUS_SUB_START
    , SDK_NET_PTZ_FOCUS_SUB_STOP
    , SDK_NET_PTZ_IRIS_ADD_START        //云台控制-光圈
    , SDK_NET_PTZ_IRIS_ADD_STOP
    , SDK_NET_PTZ_IRIS_SUB_START
    , SDK_NET_PTZ_IRIS_SUB_STOP
    , SDK_NET_PTZ_GOTO_ZERO_PAN         //云台控制-归位
    , SDK_NET_PTZ_FLIP_180              //云台控制-翻转
    , SDK_NET_PTZ_SET_PATTERN_START     //云台控制-模式
    , SDK_NET_PTZ_SET_PATTERN_STOP
    , SDK_NET_PTZ_RUN_PATTERN
    , SDK_NET_PTZ_SET_AUXILIARY         //云台控制-辅助开关
    , SDK_NET_PTZ_CLEAR_AUXILIARY
    , SDK_NET_PTZ_AUTO_SCAN_START       //云台控制-自动线扫
    , SDK_NET_PTZ_AUTO_SCAN_STOP
    , SDK_NET_PTZ_RANDOM_SCAN_START     //云台控制-随机线扫
    , SDK_NET_PTZ_RANDOM_SCAN_STOP
    , SDK_NET_PTZ_LEFT_OFFSET           //云台控制-3D
    , SDK_NET_PTZ_RIGHT_OFFSET
    , SDK_NET_PTZ_DOWN_OFFSET
    , SDK_NET_PTZ_UP_OFFSET
    , SDK_NET_PTZ_ZOOM_MULTIPLE
    , SDK_NET_PTZ_POINT_CENTER
    , SDK_NET_PTZ_VIEW_CENTER
    , SDK_NET_PTZ_BUTT
}SDK_NET_PTZ_ID_E;

#define SDK_NET_MSG_NET_OFFSET  0x0000
#define SDK_NET_MSG_MAIN_OFFSET 0x0200  //512
#define SDK_NET_MSG_XXX_OFFSET  0x0400  //512

typedef enum _SDK_NET_MSG_ID {
    /* ---------------------------------------------------- */
      SDK_NET_MSG_NET_BEGIN = SDK_NET_MSG_NET_OFFSET
    , SDK_NET_MSG_LOGIN         //登录 sdk_login_req_t
    , SDK_NET_MSG_LOGOUT        //退出
    , SDK_NET_MSG_KEEPLIVE      //心跳 sdk_keeplive_req_t
    , SDK_NET_MSG_UPGRAD        //升级 SDK_UPGRAD_OP_E, upgrade_packet_t : sdk_upgrad_rsp_t
    , SDK_NET_MSG_DATA_REQ      //数据命令 sdk_business_desc_t
    , SDK_NET_MSG_OPEN_CH       //打开实时流 sdk_open_chann_req_t
    , SDK_NET_MSG_CLOSE_CH      //关闭实时流 sdk_business_desc_t
    , SDK_NET_MSG_RECORD_PLAY   //录像回放 sdk_record_item_t
    , SDK_NET_MSG_RECORD_DOWN   //录像下载 sdk_record_item_t
    , SDK_NET_MSG_TALK_START    //开始对讲 sdk_talk_rsp_t
    , SDK_NET_MSG_TALK_STOP     //结束对讲 sdk_business_desc_t

/* ---------------------------------------------------- */
    , SDK_NET_MSG_MAIN_BEGIN = SDK_NET_MSG_MAIN_OFFSET
    , SDK_NET_MSG_VERSION       //版本信息          sdk_version_t
    , SDK_NET_MSG_STATUS        //系统状态          SDK_STATUS_OP_E, sdk_status_t
    , SDK_NET_MSG_PARAM_DEFAULT //默认参数          sdk_default_param_t
    , SDK_NET_MSG_USER          //用户管理          SDK_USER_OP_E
    , SDK_NET_MSG_LOG           //日志管理          sdk_log_item_t
    , SDK_NET_MSG_SYS_CFG       //系统配置          sdk_sys_cfg_t
    , SDK_NET_MSG_NET_CFG       //网络配置          sdk_net_mng_cfg_t
    , SDK_NET_MSG_PTZ_CFG       //云台配置          sdk_ptz_param_t
    , SDK_NET_MSG_PTZ_CONTROL   //云台控制          SDK_NET_PTZ_ID_E
    , SDK_NET_MSG_CRUISE_CFG    //巡航配置          sdk_cruise_param_t
    , SDK_NET_MSG_PREVIEW_CFG   //本地预览配置      sdk_preview_t
    , SDK_NET_MSG_CHANN_CFG     //通道配置          sdk_channel_t
    , SDK_NET_MSG_ENCODE_CFG    //编码配置          sdk_encode_t
    , SDK_NET_MSG_RECORD_CFG    //录像配置          sdk_record_cfg_t
    , SDK_NET_MSG_RECORD_QUERY  //录像查询          sdk_record_cond_t
    , SDK_NET_MSG_MOTION_CFG    //视频移动配置      sdk_motion_cfg_v2_t
    , SDK_NET_MSG_ALARM_IN_CFG  //外部报警输入配置  sdk_alarm_in_cfg_t
    , SDK_NET_MSG_HIDE_CFG      //视频遮蔽报警配置  sdk_hide_cfg_t
    , SDK_NET_MSG_LOST_CFG      //视频丢失配置      sdk_lost_cfg_t
    , SDK_NET_MSG_OSD_CFG       //OSD配置           sdk_osd_cfg_t
    , SDK_NET_MSG_COMM_CFG      //常规配置          sdk_comm_cfg_t
    , SDK_NET_MSG_OVERLAY_CFG   //遮挡区域配置      sdk_overlay_cfg_t
    , SDK_NET_MSG_DISK          //磁盘管理          SDK_DISK_OP_E
    , SDK_NET_MSG_SYSTIME       //系统时间配置      sdk_time_t
    , SDK_NET_MSG_SERIAL_CFG    //串口参数配置      sdk_serial_func_cfg_t
    , SDK_NET_MSG_IMAGE_ATTR    //图像属性配置      sdk_image_attr_t
    
/* ---------------------------------------------------- */
    , SDK_NET_MSG_XXX_BEGIN = SDK_NET_MSG_XXX_OFFSET //XXXXXX
    
    , SDK_NET_MSG_BUTT
}SDK_NET_MSG_ID_E;


typedef enum _SDK_NET_CONN_TYPE {
      SDK_NET_CONN_LONG = 0x00  //长连接
    , SDK_NET_CONN_SHORT= 0x01  //短连接
}SDK_NET_CONN_TYPE_E;

/*----------- 网络结构体定义 -----------*/

/*
 * 视频流分包结构体定义
 */
typedef struct sdk_packet_s{
    uint8_t  version;          //协议版本号
    uint8_t  frame_type;       //帧类型
    uint16_t pack_count;       //包个数
    
    uint32_t frame_no;        //帧序号
    uint32_t frame_size;      //帧长度
    
    uint16_t pack_size;       //包长度(最大支持65535字节)
    uint16_t pack_no;         //包序号
    uint8_t  data[0];         //数据
}sdk_packet_t;


/*-------------------------------*/

typedef struct sdk_dev_desc_s {
    char dev_name[MAX_DVR_NAME_LEN];    //设备名称;
    uint32_t dev_id;                    //设备ID;
}sdk_dev_desc_t;


/*
 * 登录消息
 */
typedef struct sdk_login_req_s{
    sdk_user_t  user;                //DVR 用户名＋密码 
    uint32_t    conn_type;           //请求链接类型 0:长链接，1：短链接
}sdk_login_req_t;

typedef struct sdk_login_rsp_s{
    sdk_user_right_t right;         //用户权限
    uint32_t         conn_type;     //返回链接类型 0:长链接，1：短链接
    sdk_dev_desc_t   dev_desc;      //设备描述信息
}sdk_login_rsp_t;

/*
 * 心跳
 */
typedef struct sdk_keeplive_req_s{
    sdk_time_t time;
}sdk_keeplive_req_t, sdk_keeplive_rsp_t;


typedef enum _SDK_NET_PROTOCAL {
    SDK_NET_PROTOCAL_TCP     = 0,        //TCP
    SDK_NET_PROTOCAL_UDP     = 1,        //UDP
    SDK_NET_PROTOCAL_MB      = 2,        //MBroad
}SDK_NET_PROTOCAL_E;

typedef enum _SDK_NET_BUSINESS_TYPE {
      SDK_NET_BUSINESS_UPGRAD = 0       //升级
    , SDK_NET_BUSINESS_STREAM           //实时流
    , SDK_NET_BUSINESS_TALK             //对讲
    , SDK_NET_BUSINESS_RECORD_PLAY      //录像回放
    , SDK_NET_BUSINESS_RECORD_DOWN      //录像下载
}SDK_NET_BUSINESS_TYPE_E;


typedef struct sdk_business_desc_s {
    uint16_t business_type;     //业务类型 SDK_NET_BUSINESS_TYPE_E
    uint16_t business_id;       //业务ID(设备端分配)
}sdk_business_desc_t;


/*
 * 实时视频
 */
typedef struct sdk_open_chann_req_s{
    uint16_t chann_type;     //通道类型
    uint16_t proto_type;     //视频传输协议 SDK_NET_PROTOCAL_E
}sdk_open_chann_req_t;


typedef struct sdk_open_chann_rsp_s{    
    uint8_t  ip_addr[16];    //客户端主动链接[ip_addr, port],并发送SDK_NET_MSG_DATA_REQ到设备端
    uint16_t port;           //port
    uint8_t  res[2];
    sdk_business_desc_t business_desc;//设备为客户端分配的业务(TYPE,ID)
}sdk_open_chann_rsp_t;

/*
 * 升级请求成功时回应
 */
typedef sdk_open_chann_rsp_t sdk_upgrad_rsp_t;



/*
 * 对讲
 */
typedef sdk_open_chann_rsp_t sdk_talk_rsp_t;


#endif //__sdk_netlib_h__


