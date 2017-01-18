/*
 * =====================================================================================
 *
 *       Filename:  ptzlib.h
 *
 *    Description:  ptzlib 
 *
 *        Version:  1.0
 *        Created:  2011年05月22日 21时08分09秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  maohw (), maohongwei@gmail.com
 *        Company:  
 *
 * =====================================================================================
 */
#ifndef __ptzlib_h__
#define __ptzlib_h__

#include "sdk_global.h"

/*
 * 云台协议类型
 */
typedef enum _SDK_PTZ_PROTOCOL {
      SDK_PTZ_PELCO_D = 0x01
    , SDK_PTZ_PELCO_P
    , SDK_PTZ_BUTT    = 0x20
}SDK_PTZ_PROTOCOL_E;


/*
 * 云台控制消息ID定义
 */
typedef enum _SDK_PTZ_MSG_ID {
      SDK_PTZ_MSG_UP_START = 0x001
    , SDK_PTZ_MSG_UP_STOP
    , SDK_PTZ_MSG_DOWN_START
    , SDK_PTZ_MSG_DOWN_STOP
    , SDK_PTZ_MSG_LEFT_START
    , SDK_PTZ_MSG_LEFT_STOP
    , SDK_PTZ_MSG_RIGHT_START    
    , SDK_PTZ_MSG_RIGHT_STOP
    , SDK_PTZ_MSG_LEFT_UP_START
    , SDK_PTZ_MSG_LEFT_UP_STOP
    , SDK_PTZ_MSG_RIGHT_UP_START
    , SDK_PTZ_MSG_RIGHT_UP_STOP
    , SDK_PTZ_MSG_LEFT_DOWN_START
    , SDK_PTZ_MSG_LEFT_DOWN_STOP
    , SDK_PTZ_MSG_RIGHT_DOWN_START
    , SDK_PTZ_MSG_RITHT_DOWN_STOP
    , SDK_PTZ_MSG_PRESET_SET
    , SDK_PTZ_MSG_PRESET_CALL
    , SDK_PTZ_MSG_PRESET_DEL
    , SDK_PTZ_MSG_ZOOM_ADD_START
    , SDK_PTZ_MSG_ZOOM_ADD_STOP
    , SDK_PTZ_MSG_ZOOM_SUB_START
    , SDK_PTZ_MSG_ZOOM_SUB_STOP
    , SDK_PTZ_MSG_FOCUS_ADD_START
    , SDK_PTZ_MSG_FOCUS_ADD_STOP
    , SDK_PTZ_MSG_FOCUS_SUB_START
    , SDK_PTZ_MSG_FOCUS_SUB_STOP
    , SDK_PTZ_MSG_IRIS_ADD_START
    , SDK_PTZ_MSG_IRIS_ADD_STOP
    , SDK_PTZ_MSG_IRIS_SUB_START
    , SDK_PTZ_MSG_IRIS_SUB_STOP
    , SDK_PTZ_MSG_GOTO_ZERO_PAN
    , SDK_PTZ_MSG_FLIP_180
    , SDK_PTZ_MSG_SET_PATTERN_START
    , SDK_PTZ_MSG_SET_PATTERN_STOP
    , SDK_PTZ_MSG_RUN_PATTERN
    , SDK_PTZ_MSG_SET_AUXILIARY
    , SDK_PTZ_MSG_CLEAR_AUXILIARY
    , SDK_PTZ_MSG_AUTO_SCAN_START
    , SDK_PTZ_MSG_AUTO_SCAN_STOP
    , SDK_PTZ_MSG_RANDOM_SCAN_START
    , SDK_PTZ_MSG_RANDOM_SCAN_STOP
    , SDK_PTZ_MSG_LEFT_OFFSET
    , SDK_PTZ_MSG_RIGHT_OFFSET
    , SDK_PTZ_MSG_DOWN_OFFSET
    , SDK_PTZ_MSG_UP_OFFSET
    , SDK_PTZ_MSG_ZOOM_MULTIPLE
    , SDK_PTZ_MSG_POINT_CENTER
    , SDK_PTZ_MSG_VIEW_CENTER
    , SDK_PTZ_MSG_BUTT
}SDK_PTZ_MSG_ID_E;

//云台控制结构体定义

typedef struct ptz_args_s {
    int proto;
    int addr;
    int cmd;
    unsigned int value; //[高16位:速度; 低16位:预置点]
}ptz_args_t;

//云台控制接口定义

int sdk_ptz_init(void);
int sdk_ptz_deinit(void);
int sdk_ptz_control(ptz_args_t *args, char *buf, int *len);

#endif //__ptzlib_h__

