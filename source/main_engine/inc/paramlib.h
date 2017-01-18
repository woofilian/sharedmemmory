/*
 * =====================================================================================
 *
 *       Filename:  paramlib.h
 *
 *    Description:  参数管理
 *
 *        Version:  1.0
 *        Created:  2016:11:22 
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  maohw (), maohongwei@gmail.com
 *        Company:  jxj
 *
 * =====================================================================================
 */


#ifndef __paramlib_h__
#define __paramlib_h__

#include <stdint.h>

/*
 * |- 8 -|----- 16 ----|-- 8 --| 
 * |---------------------------|
 * |type |  PARAM ID   |  ch   |
 * |---------------------------|
 */

#define PARAM_CURR_NAME ".param_cur"
#define PARAM_BAK_NAME ".param_bak"
#if 1
//ϵͳ�����������ı�־
#define PARAM_DEF_FILE_UPG		"/config/param/sysparam_upg"
#define PARAM_DEF_FILE			"/config/param/.sysparam_def"
#define PARAM_CURR_FILE_1 		"/config/param/.sysparam1"
#define PARAM_CURR_FILE_2 		"/config/param/.sysparam2"
#else
#define PARAM_DEF_FILE_UPG		"/mnt/nfs/config/param/sysparam_upg"
#define PARAM_DEF_FILE			"/mnt/nfs/param/.sysparam_def"
#define PARAM_CURR_FILE_1 		"/mnt/nfs/param/.sysparam1"
#define PARAM_CURR_FILE_2 		"/mnt/nfs/param/.sysparam2"

#endif
/* node */
typedef struct param_node_s {
    uint32_t id;
    uint32_t size;
    uint32_t offset;
}param_node_t;
/* head */
typedef struct param_head_s {
    char         magic[32];
	uint64_t	 w_seq;		// д�ļ����к�
    uint32_t     crc;
    uint32_t     node_num;
    uint32_t     data_size;
    param_node_t nodes[0];
}__attribute__ ((packed))param_head_t;




typedef void* param_handle_t;

typedef struct param_file_op_s {
    int             fd;
    int             fd2;
    uint64_t        cur_seq;
    int             cur_fd;
    int (*open)(struct param_file_op_s *op,  char *file_name, char *file_name2, int flags);
    int (*seek)(struct param_file_op_s *op,  uint32_t offset, int whence);
    int (*write)(struct param_file_op_s *op, char *buf, int size);
    int (*sync)(struct param_file_op_s *op);
    int (*read)(struct param_file_op_s *op,  char *buf, int size);
    int (*close)(struct param_file_op_s *op);
}param_file_op_t;



/*
 * 装载参数到内存
 * flags: 1: create; 0: rw;
 */
param_handle_t sdk_param_load(char *file_name, char *file_name2, int flags);

/*
 * 存储参数到磁盘
 */
int sdk_param_save(param_handle_t handle, char *file_name);

/*
 * 销毁内存参数
 */
int sdk_param_destroy(param_handle_t handle);

/*
 * 查询参数
 */
typedef enum _PARAM_ERR_NO
{
    PARAM_ERR_ID    = -1,
    PARAM_ERR_LEN   = -2,
}PARAM_ERR_NO_E;

int sdk_param_get(param_handle_t handle
        , uint32_t id
        , int32_t size
        , int32_t *param_size
        , void *param);

/*
 * 设置参数
 */
int sdk_param_set(param_handle_t handle
        , uint32_t id
        , int32_t sync
        , int32_t *param_size
        , void *param);

/*
 * 同步参数
 */
int sdk_param_sync(param_handle_t handle);


/* ---------------------------------------------------------- */

#endif //__paramlib_h__
