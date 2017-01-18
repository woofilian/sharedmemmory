/*
 * Copyright   : General public 
 * Description : ring fix size log && ring shm log 
 * Created     :  
 */


#ifndef __log_h__
#define __log_h__


#ifdef __cplusplus
extern "C"{
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#ifdef _DMALLOC
#include "dmalloc.h"
#endif

#define LOG_VERSION     "V1.3"

#define LOG_MAX_ITEM_NUM    (1024)
#define LOG_MAX_DESC_LEN     (128)

typedef enum _SDK_LOG_CMD_E{
	  SDK_LOG_CMD_SET_USER = 0x01
	, SDK_LOG_CMD_SET_IP

	, SDK_LOG_CMD_BUTT
}SDK_LOG_CMD_E;

typedef struct log_item_s {
    unsigned int time;
    unsigned int type;
    unsigned int type2;
    unsigned int args;
    char         user[32];
    char         ip_addr[16];
    char         desc[LOG_MAX_DESC_LEN];
}log_item_t;

typedef struct log_list_s {
    log_item_t  node;
    struct log_list_s *next;
}log_list_t;


void _printd( int level
                , const char *module_name
                , const char *file_name
                , int  line
                , const char *function_name
                , const char *fmt,...);

int _log_write(  char *module_name
                    , unsigned int type
                    , unsigned int type2
                    , unsigned int args
                    , char *fmt,...);

int _log_query(  char *module_name
                    , unsigned int type
                    , unsigned int begin_time
                    , unsigned int end_time
                    , log_list_t **pp_log_list);

int _log_delete(  char *module_name
                    , unsigned int type
                    , unsigned int begin_time
                    , unsigned int end_time);

int _log_sync(void);

int _log_ioctrl(  char *module_name
					, SDK_LOG_CMD_E cmd
					, void *param
					, int param_len);

/**
 *  exprot interface
 *
 */
#ifndef MODULE_NAME
#warning ">>>>>>>>>>>>>>>>>> MODULE_NAME is NO DEFINE(default:log)"
#define MODULE_NAME "log"
#endif


/**
 *
 *  dbg_mask: 初始化掩码
 *  LOG_DBG_STDERR：输出调试信息到标准错误终端
 *  LOG_DBG_MEM   ：输出调试信息到内存缓冲
 *  LOG_LOG_FILE  ：初始化日志功能（初始化后日志接口才能使用）
 *  dbg_file： 调试信息记录文件
 *  log_file： 日志功能记录文件
 *
 */
#define LOG_DBG_STDERR  1
#define LOG_DBG_MEM     2
#define LOG_LOG_FILE    4

int sdk_log_init(int dbg_mask, char *dbg_file, char *log_file);

/**
 *
 *
 */
int sdk_log_deinit(void);

#define COLOR_NONE              "\033[0000m"
#define COLOR_BLACK             "\033[0;30m"
#define COLOR_LIGHT_GRAY        "\033[0;37m"
#define COLOR_DARK_GRAY         "\033[1;30m"
#define COLOR_BLUE              "\033[0;34m"
#define COLOR_LIGHT_BLUE        "\033[1;34m"
#define COLOR_GREEN             "\033[1;32m"
#define COLOR_LIGHT_GREEN       "\033[1;32m"
#define COLOR_CYAN              "\033[0;36m"
#define COLOR_LIGHT_CYAN        "\033[1;36m"
#define COLOR_RED               "\033[1;31m"
#define COLOR_LIGHT_RED         "\033[1;31m"
#define COLOR_PURPLE            "\033[0;35m"
#define COLOR_LIGHT_PURPLE      "\033[1;35m"
#define COLOR_BROWN             "\033[0;33m"
#define COLOR_YELLOW            "\033[1;33m"
#define COLOR_WHITE             "\033[1;37m"

#define DEBUG_LEVEL_ERROR   0x001
#define DEBUG_LEVEL_WARNING 0x002
#define DEBUG_LEVEL_INFO    0x003
#define DEBUG_LEVEL_PRINTF  0x004


int sdk_dbg_level(int level);
void log_puts(char *str);
#define _printf(level, args...)  _printd(level,MODULE_NAME,__FILE__,__LINE__,__FUNCTION__,##args)
#if 1
#ifndef __PRINTF
#define printf(args...)          _printf(DEBUG_LEVEL_PRINTF,##args)
#endif
#define error(fmt, args...)      _printf(DEBUG_LEVEL_ERROR,COLOR_RED fmt COLOR_NONE, ##args)
#define warning(fmt, args...)    _printf(DEBUG_LEVEL_WARNING,COLOR_YELLOW fmt COLOR_NONE, ##args)
#define info(fmt, args...)       _printf(DEBUG_LEVEL_INFO,COLOR_GREEN fmt COLOR_NONE, ##args)
#else
#define printf printf
#define error printf
#define warning    printf
#define info       printf
#endif

#define sdk_printf(args...)     _printd(DEBUG_LEVEL_PRINTF,MODULE_NAME,__FILE__,__LINE__,__FUNCTION__,##args)
#define sdk_log_write(type, type2, arg, args...) _log_write(MODULE_NAME, type, type2, arg, ##args)
#define sdk_log_query(type, begin_time, end_time, log_list)  _log_query(MODULE_NAME, type, begin_time, end_time, log_list)
#define sdk_log_delete(type, begin_time, end_time)  _log_delete(MODULE_NAME, type, begin_time, end_time)
#define sdk_log_flush   _log_sync 
#define sdk_log_ioctrl(cmd, param, param_len) _log_ioctrl(MODULE_NAME, cmd, param, param_len)

#ifdef __cplusplus
}
#endif

#endif //__log_h__
