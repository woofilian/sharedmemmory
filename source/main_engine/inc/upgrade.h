
#ifndef __upgrade_h__
#define __upgrade_h__

#include <stdint.h>

#define UPG_T_UBOOT     "uboot"
#define UPG_T_IMAGE     "image"
#define UPG_T_CRAMFS    "cramfs"
#define UPG_T_JFFS2     "jffs2"
#define UPG_T_UBIFS     "ubifs"
#define UPG_T_FILE      "file"
#define UPG_T_PARM_FILE "parm"


#define UPG_P_MTD0      "/dev/mtd0"
#define UPG_P_MTD5      "/dev/mtd5"

/*
 * 升级文件信息定义
 */
typedef struct upgrade_file_s {
    uint8_t     name[64];    //文件名
    uint8_t     type[16];    //文件类型
    uint8_t     ver[16];     //版本
    uint32_t    addr;        //文件地址
    uint32_t    size;        //文件大小
    uint8_t     dst_path[32];//目标路经
    uint32_t    crc32;       //文件内容CRC校验
}upgrade_file_t;

/*
 * 升级包信息定义
 */
typedef struct upgrade_packet_s {
    uint32_t  crc32;         //描述信息CRC校验(upgrade_packet_t+N*upgrade_file_t)
    uint8_t   device_type[16];//设备类型
    uint8_t   ver[16];       //版本
    uint8_t   date[16];      //日期
    uint32_t  file_num;      //文件个数
    uint32_t  size;          //升级包大小(N*upgrade_file_t + file[N].size)
    uint32_t  ex_addr;       //扩展头地址
    upgrade_file_t file[0];  //文件信息
}upgrade_packet_t;


typedef int(upgrade_fun_t)(int status, void *args);

/*
 * 初始化
 */
int upgrade_init(char *device_type, char *ver, int *upgrade_mode, upgrade_fun_t *func, void *args);

int upgrade_deinit(void);


/*
 * 开始升级
 */
int upgrade_start(char *buf, int is_file);

/*
 * 查询升级状态
 */
// 获取升级进度
int upgrade_get_status(void);
// 查询是否正在升级
int upgrade_tid_runing(void);
// 设置升级进度
int upgrade_set_percent(int up_percent);

/*
 * 检查升级包描述信息;
 */
int upgrade_check(char *buf, int is_file);

/*
 * 只检查升级包头不包含文件信息;
 */
int upgrade_check_pkg_head(char *buf, int is_file);




#endif //__upgrade_h__
