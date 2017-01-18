#ifndef __GRD_MMC_API_H__
#define __GRD_MMC_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h> 
#include <assert.h>
#include <sys/vfs.h>
#include <dirent.h>  
#include <sys/stat.h>  
#include <sys/mount.h>  
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>

#include "storage.h"
//#include "typedef.h"
extern int g_sd_tats;

#define GRD_SD_PATHNAME             			"/dev/mmcblk[0-9]"
//#define GRD_SD_PARTITION_PATHNAME   		"/dev/mmcblk[0-9]p[0-9]"
#define GRD_SD_PARTITION_PATHNAME   		"/dev/mmcblk0"
//#define GRD_SD_MOUNT_POINT          			"/mnt/sd_card"
#define GRD_SD_PATHNAME_0           			"/dev/mmcblk0"
#define GRD_SD_PARTITION_PATHNAME_0 		"/dev/mmcblk0p1"



#define PRINT_ERR() printf("<<< File: %s, Line: %d, Function: %s >>>\n", __FILE__, __LINE__, __FUNCTION__)

#define PRINT_ERR_MSG(fmt, args...) \
            printf("[File: %s, Line: %d, Function: %s ]  " fmt "\n", \
                    __FILE__, __LINE__, __FUNCTION__, ##args)

#define PRINT_ERR_INFO(fmt, args...) printf(fmt, ##args)

#define PRINT_INFO(fmt, args...) printf(fmt, ##args)

/*
 *  pic_info 图片信息，图片是以时间命名 YYYYMMDDHHMMSS.jpg
 */
typedef struct _time_t_
{
   unsigned short year;
    unsigned char mon;
    unsigned char day;
    unsigned char hour;
    unsigned char min;
    unsigned char sec;
}_time_t;

typedef struct _pic_info
{	
	int count;   //图片的张数
	_time_t pic_num[0];
}pic_info;

/**********************************************************************
函数描述：判断需要修复的文件路径名是否符合自定义的格式
入口参数：char *file_path: 需要修复的文件路径名
返回值：  1: 符合
          0: 不符合
**********************************************************************/
extern int is_fix_format_path(char *file_path);

/**********************************************************************
函数描述：判断路径名是否符合自定义的格式
入口参数：char *str: 需要判断的路径名
返回值：  1: 符合
          0: 不符合
**********************************************************************/
extern int is_avi_dir_format(char *str);

/**********************************************************************
函数描述：判断文件名是否符合自定义的格式
入口参数：char *str: 需要判断的文件名
返回值：  1: 符合
          0: 不符合
**********************************************************************/
extern int is_avi_file_format(char *str);

/**********************************************************************
函数描述：判断文件路径名是否符合自定义的格式
入口参数：char *file_path: 需要判断的文件路径名
返回值：  1: 符合
          0: 不符合
**********************************************************************/
extern int is_avi_file_path_format(char *str);



/**********************************************************************
函数描述：mount sd卡
入口参数：无
返回值：  <0: 错误
          0 : 正常
**********************************************************************/
extern int grd_sd_mount(void);

/**********************************************************************
函数描述：reumount sd卡
入口参数：无
返回值：  <0: 错误
          0 : 正常
**********************************************************************/
extern int grd_sd_remount(void);

/**********************************************************************
函数描述：umount sd卡
入口参数：挂载节点
返回值：-1 : 错误
          0 : 正常
**********************************************************************/
int grd_sd_umount(const char * mount_point);


/**********************************************************************
函数描述：判断sd卡是否被mount上
入口参数：无
返回值：  1: 被mount上
          0: 没有被mount上
**********************************************************************/
extern int grd_sd_is_mount(void); // if sd card is mounted, return 1; else return 0


/**********************************************************************
函数描述：找到最老的avi文件夹中最老的avi文件，然后删除该文件。
          如果在最老的avi文件夹中没有找到avi文件，则删除该文件夹。
          如果找不到最老的avi文件夹，则返回0
入口参数：无          
返回值：  1: 成功,删除了最老的文件或者最老的文件夹，正常返回
          0: 失败，找不到最老的文件夹，没有进行删除动作        
**********************************************************************/
extern int grd_sd_del_oldest_avi_file(void);

/**********************************************************************
函数描述；检测到SD卡，会自动挂载；当sd卡的容量小于30M
                           时，会自动删除最旧的视频。
入口参数： 无
返回值：      无
**********************************************************************/

void sd_card_detecte_thread();

int is_jpg_pic_format(char * file_name);
int st_find_car_picture(const char * file_path, pic_info * pic_infomat);

/**********************************************************************
函数描述：检查 SD 卡是否存在
入口参数：无
返回值： 0 :  sd 卡存在， -1  sd 卡不存在
      
**********************************************************************/

int get_sd_stats(void);
int get_mmc_devname(char *pDevname, int iMaxsize);


#ifdef __cplusplus
}
#endif

#endif /* __GRD_MMC_API_H__ */

