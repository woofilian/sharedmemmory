#include "mmc_api.h"
#include "log.h"

#include <stdlib.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <pthread.h>
#include <sys/mount.h>
#include <pthread.h>



#include "avi_typedef.h"
#include <errno.h>
#include "record.h"

#define _MSG_DEBUG 0


char grd_sd_partition_pathname[128];
extern RECORDING_FILE_INFO recording_file_info;
int g_sd_tats;
int g_format_SDcard = -1;

pthread_mutex_t sd_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static int SD_CARD_FORMATING = 0;//用于判断是否在格式化
extern void avi_substring(char *dest, char *src, int start, int end);
extern int last_index_at(char *str, char c);
extern void get_nowtime_str(char *str);


/**********************************************************************
函数描述：st_get_sd_card_infomat 获取SD卡信息，可用信息和剩余空间
入口参数：sd_mount_path挂载路径
返回值： 0 : 成功
			 -1 : 失败
 **********************************************************************/
int st_get_sd_card_infomat(const char * sd_mount_path, \
	unsigned long long *total_Mbytes, unsigned long long *available_Mbytes )
{
	struct statfs diskInfo;  
      
    statfs(sd_mount_path, &diskInfo);  

	unsigned long long blocksize = diskInfo.f_bsize;    //每个block里包含的字节数  
    unsigned long long totalsize = blocksize * diskInfo.f_blocks;   //总的字节数，f_blocks为block的数目  

	*total_Mbytes = 	totalsize;
#if _MSG_DEBUG
	info("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n",   
        totalsize, totalsize>>10, totalsize>>20, totalsize>>30);  
#endif       
    unsigned long long freeDisk = diskInfo.f_bfree * blocksize; //剩余空间的大小  
    unsigned long long availableDisk = diskInfo.f_bavail * blocksize;   //可用空间大小 

	*available_Mbytes = availableDisk;
#if _MSG_DEBUG

	info("Disk_free = %llu MB = %llu GB\nDisk_available = %llu MB = %llu GB\n",   
        freeDisk>>20, freeDisk>>30, availableDisk>>20, availableDisk>>30);  
#endif       
    return 0;  
}


/**********************************************************************
函数名称:   st_sd_card_format
函数描述：格式化SD卡
入口参数：mount_path 挂载路径，device_path设备路径
返回值： 0 : 成功
			 -1 : 失败
 **********************************************************************/
//
//if(0 == st_sd_card_format(GRD_SD_MOUNT_POINT, "/dev/mmcblk0p1"))
int st_sd_card_format()
{	
	g_format_SDcard = 1;

	char cmdline[128] = {0};
	char formatstr[128] = {0};
	char deviceName[64] = {0};
	int ret;
	
	//get sd card device name, if failed return -1
	//if(get_sd_stats() < 0)
	//	return -1;
	
	ret = get_mmc_devname(deviceName, sizeof(deviceName));
	if(ret < 0)
	{
		error("[%s][%d]==>no mmc dev detected!!!\n",__FUNCTION__, __LINE__);
		return -1;
	}
	SD_CARD_FORMATING = 1;

	info("%s\n", deviceName);
	stop_normal_record_stream();
	stop_event_record_stream();
	system("rm  -Rf /var/log/has_sd_card");
	sleep(1);
	sprintf(formatstr, "umount -lf %s", GRD_SD_MOUNT_POINT);
	info("%s\n", formatstr);
	system(formatstr);
	sleep(1);
	//格式化
	//sprintf(cmdline, "mkfs.vfat %s", "/dev/mmcblk0p1");
	sprintf(cmdline, "mkfs.vfat %s", deviceName);
	system(cmdline);
	
	printf("============4=================\n");
	sleep(2);
	ret = mount(deviceName, GRD_SD_MOUNT_POINT, "vfat", 0, 0);
	if(0 > ret)
	{
		error("[%s][%d]==>Mount Error!!!!\n",__FUNCTION__, __LINE__);
		return -1;
	}
	
	system("touch /var/log/has_sd_card");
	sleep(1);
	SD_CARD_FORMATING = 0;
	printf("============5=================\n");
	return 0;
}

/**********************************************************************
函数描述：判断挂载目录是否存在，不存在则创建
入口参数：无
返回值： 无
 **********************************************************************/
int  st_creat_mount_dir(void )
{
	char cmd[128];
	int ret = -1;
    if(0 == access(GRD_SD_MOUNT_POINT, F_OK))
	{
		return 0;
	}
	else
	{
	
		if(-1 == mkdir(GRD_SD_MOUNT_POINT, 777))
		{
			error("mkdir /mnt/sd_card error!!\n");
		}
		if(-1 == chmod(GRD_SD_MOUNT_POINT, 777))
		{
			error("chmod /mnt/sd_card error!!\n");
		}
		
		return 0;
	}
}


/**********************************************************************
函数描述：判断sd卡是否被mount上
入口参数：无
返回值：  3: 被mount上
          0: 没有被mount上
**********************************************************************/
 
int grd_sd_is_mount(void)
{
    struct statfs statFS;
    int ret;
    ret=(statfs(GRD_SD_MOUNT_POINT, &statFS )==0) ? 1 : 0 ;
	if(0 == ret)
	{
		return ret;
	}
	
    int flag = 0;
    char line[256];
	char * tmp_line;
	
	//第一检测
    memset(line, 0, sizeof(line));

    ret = system("cat /proc/mounts > /tmp/mounts");
    
    FILE *fp = NULL;
    fp = fopen("/tmp/mounts", "r");
    if(fp == NULL)
    {
        system("rm -rf /tmp/mounts"); //删除临时文件
        return 0;
    }

    while(fgets(line, sizeof(line), fp))
    {
   		if((strstr(line, "mmcblk0p1") != NULL) && (strstr(line, GRD_SD_MOUNT_POINT)) \
			&&(strstr(line, "vfat")))
		{
			flag = 3;
			break;
		}
		else
		{
			flag = 0;
		}
    }

    fclose(fp);
    system("rm -rf /tmp/mounts"); //删除临时文件


	//第二次检测
    ret = system("df > /tmp/mounts_0");
    
    fp = fopen("/tmp/mounts_0", "r");
    if(fp == NULL)
    {
        system("rm -rf /tmp/mounts_0"); //删除临时文件
        return -1;
    }
	
    memset(line, 0, sizeof(line));
    while(fgets(line, sizeof(line), fp))
    {
   		if((strstr(line, "mmcblk0p1") != NULL) && (strstr(line, GRD_SD_MOUNT_POINT)))
		{
			flag = 3;
			//warning("Mounted OK...\n");
			break;
		}
		else
		{
			flag = 0;
		}
    }

	if(0 == flag)
		warning("SD card is not mounted!\n");

    fclose(fp);
    system("rm -rf /tmp/mounts_0"); //删除临时文件
	
	
    return flag;
}

/**********************************************************************
函数描述：mount sd卡
入口参数：无
返回值：  -1 : 错误
          0 : 正常
**********************************************************************/
int grd_sd_mount(void)
{
	int ret = 0;
    char cmd[128];

	st_creat_mount_dir();//首先创建挂载目录

	
    if(0 != access(GRD_SD_MOUNT_POINT, F_OK))
	{
		error("found no %s\n", GRD_SD_MOUNT_POINT);
		return -1;
	}
	
	ret = mount(GRD_SD_PARTITION_PATHNAME, GRD_SD_MOUNT_POINT, "vfat", 0, 0);
	if(0 > ret)
	{
		//error("[%s][%d]==>Mount Error!!!!\n",__FUNCTION__, __LINE__);
		return -1;
	}

	return 0;
}

/**********************************************************************
函数描述：reumount sd卡
入口参数：无
返回值：  <0: 错误
          0 : 正常
**********************************************************************/
int grd_sd_remount(void)
{
	int ret = 0;

	ret = mount(GRD_SD_PARTITION_PATHNAME, GRD_SD_MOUNT_POINT, "vfat", MS_REMOUNT, NULL);
	if(ret != 0)
        PRINT_ERR();


	return ret;
}

/**********************************************************************
函数描述：umount sd卡
入口参数：挂载节点
返回值：-1 : 错误
          0 : 正常
**********************************************************************/
int grd_sd_umount(const char * mount_point)
{
	int ret = 0;

	ret = umount2(mount_point , MNT_DETACH);
	return (ret == 0)? 0:-1;
}
/**********************************************************************
函数描述：判断SD卡是否有效
入口参数；邋SD卡的挂载路径
返回值：  0: 有效
          -1: 无效
**********************************************************************/

int is_sd_card_valid(const char * path)
{
	char filename[24] = {0};
	FILE * fp;
	
	sprintf(filename, "%s/tmp.txt", path);
    fp = fopen(filename, "w+");
	if(fp)
	{
		fclose(fp);
		remove(filename);
		return 0;
	}
	else
	{
		fclose(fp);
		error("SD card is invalid!\n");
		return -1;
	}
}
/**********************************************************************
函数描述：判断字符串是否都是数字
入口参数：char *str: 需要判断的字符串
返回值：  1: 符合
          0: 不符合
**********************************************************************/
static int is_digit(char *str)
{

    int n, i, ret;
    n = strlen(str);
    ret = 1;

    for(i = 0; i < n; i++)
    {
          if( str[i] < '0' || str[i] > '9')
            return 0;
    }
    return 1;
}

int is_fix_format_file(char *str)
{
    int n;

    char tmp[20];

    n = strlen(str);
    if(n != 18)
        return 0;

    if( str[0] != 'c' || str[1] != 'h')
        return 0;

    if( str[2] < '0' || str[2] > '9')
        return 0;

    if(str[3] != '_')
        return 0;

    memset(tmp, 0, sizeof(tmp));
    avi_substring(tmp, str, 4, 4+14);

    if (0 == is_digit(tmp))
        return 0;

    return 1;
}

/**********************************************************************
函数描述：判断需要修复的文件路径名是否符合自定义的格式
入口参数：char *file_path: 需要修复的文件路径名
返回值：  1: 符合
          0: 不符合
**********************************************************************/
int is_fix_format_path(char *file_path)
{
    int n, i;
    char str[128];
    char tmp[128];


    i = last_index_at(file_path, '/');
    if(i < 0)
        return 0;
    memset(str, 0 ,sizeof(str));
    strncpy(str, file_path + i + 1, sizeof(str));
//    PRINT_INFO("check %s is fix format or not.\n", str);

    n = strlen(str);
    if(n != 18)
        return 0;

    if( str[0] != 'c' || str[1] != 'h')
        return 0;

    if( str[2] < '0' || str[2] > '9')
        return 0;

    if(str[3] != '_')
        return 0;

    memset(tmp, 0 ,sizeof(tmp));
    avi_substring(tmp, str, 4, 4+14);

    if (0 == is_digit(tmp))
        return 0;

    return 1;
}

/**********************************************************************
函数描述：判断路径名是否符合自定义的格式
入口参数：char *str: 需要判断的路径名
返回值：  1: 符合
          0: 不符合
**********************************************************************/
int is_avi_dir_format(char *str)
{
	char *temp=NULL;
	temp = strstr(str,".avi");
	if(temp == NULL)
		return 0;
	else
		return 1;
}

/**********************************************************************
函数描述：判断文件名是否符合自定义的格式
入口参数：char *str: 需要判断的文件名
返回值：  1: 符合
          0: 不符合
 
**********************************************************************/
int is_avi_file_format(char *str)
{
    char *tmp = strstr(str,".avi");
	if(tmp==NULL)
		return 0;
	else
    	return 1;
}

/**********************************************************************
函数描述：判断文件路径名是否符合自定义的格式
入口参数：char *file_path: 需要判断的文件路径名
返回值：  1: 符合
          0: 不符合
**********************************************************************/
int is_avi_file_path_format(char *file_path)
{
    char str[128];
    int i, ret;


    i = last_index_at(file_path, '/');
    if(i < 0)
        return 0;
    //PRINT_INFO("i : %d\n", i);
    memset(str, 0 ,sizeof(str));
    strncpy(str, file_path + i + 1, sizeof(str));

    ret = is_avi_file_format(str);
    return ret;
}


/**********************************************************************
函数描述：在SD目录下最老的avi文件夹中，查找最老的avi文件
入口参数： file_name: 传下来的目录中的文件名
           min: 用于比较
           min_filename: 查找成功后，记录最老的avi文件名
返回值：  1: 成功,找到了最老的文件夹
          0: 没有找到最老的文件夹
**********************************************************************/
static void find_oldest_file(char *file_name, long long *min, char *min_filename)
{
    char tmp[128];
    long long tmp32;

    memset(tmp, 0 ,sizeof(tmp));
    avi_substring(tmp, file_name, 0, 10);
    tmp32 = atoll(tmp);
    if(tmp32 < *min)
    {
        *min = tmp32;
        strcpy(min_filename, file_name);
    }
}

/**********************************************************************
函数描述：在SD目录下查找最老的avi文件夹
入口参数：min_dir: 如果查找成功，返回最旧的文件夹名
返回值：  0: 成功,找到了最老的文件夹
          2: 没有找到最老的文件夹
**********************************************************************/
static int find_oldest_dir(char *min_dir)
{
    DIR *pDir;
    struct dirent *ent;
    int min = 0;
    int tmp = 0;

    min = 99999999;

    pDir = opendir(GRD_SD_MOUNT_POINT);

    if( !pDir )
    {
        return 2;
    }

    while((ent = readdir(pDir)) != NULL)
    {
        if(ent->d_type & DT_DIR)  //判断是否为目录
        {
            if(strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0)
                continue;

            tmp = atoi(ent->d_name);
            if(tmp < min)
                min = tmp;
        }
    }

    closedir(pDir);

    if(99999999 == min)
        return 2;  //can't find dir
    sprintf(min_dir, "%s/%d", GRD_SD_MOUNT_POINT, min);
    return 0;  //find dir
}

/**********************************************************************
函数描述：在SD目录下查找最老的avi文件夹并删除最老的文件
入口参数：min_dir: 最旧的文件夹名
返回值：  0: 成功,删除了最旧的文件
         -1: 错误
          3: 在最小的日期的目录中没有找到旧文件，则删除整个目录
**********************************************************************/
static int del_oldest_file(char *min_dir)
{
    DIR *pDir;
    struct dirent *ent;
    long long min;
    char min_filename[128];
    char del_file[128];
    int ret = 0;

    min = 99999999999999ull;
    memset(min_filename, 0, sizeof(min_filename));

    pDir = opendir(min_dir);

    if( !pDir )
        return -1;

    while((ent = readdir(pDir)) != NULL)
    {

        if(ent->d_type & DT_REG)  //判断是否为非目录
        {
            if (0 == is_avi_file_format(ent->d_name))
                continue;

            find_oldest_file(ent->d_name, &min, min_filename);

			info("oldest file name :%s\n", min_filename);
        }
    }

    closedir(pDir);

    if(min != 99999999999999ull) //找到了最旧的文件
    {
        memset(del_file, 0, sizeof(del_file));
        sprintf(del_file, "%s/%s", min_dir, min_filename);
        PRINT_INFO("sd card doesn't have enough space, delete file: %s\n", del_file);
        ret = remove(del_file);
        if(ret != 0)
        {
            perror("remove error : ");
            return -1;
        }
		return 0;
    }
    else //在最小的日期的目录中没有找到旧文件，则删除整个目录
    {
        PRINT_INFO("not find !!!!!!\n");
        PRINT_INFO("del dir = %s\n", min_dir);
        char cmd[128];
        memset(cmd, 0, sizeof(cmd));
        sprintf(cmd, "rm -rf %s", min_dir);
        ret = gm_system_cmd(cmd);
        if(ret != 0)
        {
            perror("remove error : ");
            return -1;
        }
		return 3;
    }

}

/**********************************************************************
函数描述：在SD目录下查找最老的avi文件夹，并重命名最老的文件
入口参数：min_dir: 最旧的文件夹名
返回值：  0: 成功,重命名了最旧的文件夹中的文件
         -1: 错误
          3: 在最小的日期的目录中没有找到旧文件，则删除整个目录
**********************************************************************/
static int rename_oldest_file(char *min_dir, char *new_name, char *old_name)
{
    DIR *pDir;
    struct dirent *ent;
    long long min;
    char min_filename[128];
    char del_file[128];
    int ret = 0;

    min = 99999999999999ull;
    //PRINT_INFO("before min = %lld\n", min);
    memset(min_filename, 0, sizeof(min_filename));

    pDir = opendir(min_dir);
    if( !pDir )
        return 3;

    while((ent = readdir(pDir)) != NULL)
    {

        if(ent->d_type & DT_REG)  //判断是否为非目录
        {
            if (0 == is_avi_file_format(ent->d_name))
                continue;

            //PRINT_INFO("dir = %s, filename = %s\n", min_dir, ent->d_name);
            find_oldest_file(ent->d_name, &min, min_filename);
        }
    }

    closedir(pDir);

    //PRINT_INFO("find result: min = %lld, file name = %s\n", min, min_filename);
    if(min != 99999999999999ull) //找到了最旧的文件
    {
        //PRINT_INFO("find !!!!!!!!!!!\n");
        memset(del_file, 0, sizeof(del_file));
        sprintf(del_file, "%s/%s", min_dir, min_filename);

	    //日期目录不存在，则创建
	    char time_start[20];
		char date[10];
		char date_path[128];
	    memset(time_start, 0, sizeof(time_start));
	    get_nowtime_str(time_start);
	    memset(date, 0, sizeof(date));
	    avi_substring(date, time_start, 0, 0+8);
	    memset(date_path, 0, sizeof(date_path));
	    sprintf(date_path, "%s/%s", GRD_SD_MOUNT_POINT, date); // str = /mnt/sd_card/20120914
	    if(access(date_path, F_OK) != 0 ) //日期目录不存在，则创建
	    {
	        ret = mkdir(date_path, 0777);
	        if(ret < 0)
	        {
	            PRINT_ERR_MSG("mkdir %s failed\n", date_path);
	            return -1;
	        }
	    }

		//memset(new_name, 0, sizeof(new_name));
		sprintf(new_name, "%s/ch", date_path);

        PRINT_INFO("sd card doesn't have enough space, rename file: %s to %s\n", del_file, new_name);
	 strcpy(old_name, del_file);
        ret = rename(del_file, new_name);
        if(ret != 0)
        {
            printf("%s[%d]......rename error!errno=%d,%s\n", __FUNCTION__, __LINE__, errno, strerror(errno));
            return -1;
        }
		return 0;
    }
    else //在最小的日期的目录中没有找到旧文件，则删除整个目录
    {
        PRINT_INFO("not find !!!!!!\n");
        PRINT_INFO("del dir = %s\n", min_dir);

        char cmd[128];
        memset(cmd, 0, sizeof(cmd));
        sprintf(cmd, "rm -rf %s", min_dir);
        ret = gm_system_cmd(cmd);
        if(ret != 0)
        {
            perror("remove error : ");
            return -1;
        }
		return 3;
    }

}

/**********************************************************************
函数描述：检查 SD 卡是否存在
入口参数：无
返回值： 0 :  sd 卡存在， -1  sd 卡不存在
      
**********************************************************************/

int get_sd_stats()
{
	if(access("/var/log/has_sd_card", F_OK) != 0 )//判断该文件不存在 说明没有mount
	{
	//info("sd card is not exit!!!! \n");
		return -1;
	}
	return 0;

}

/******************************************************
Decription: 
		get sd card device name
Params: 
		output device name of sd card by pDevname
Return: 
		0: ok, -1: no sd card detected
*******************************************************/
int get_mmc_devname(char *pDevname, int iMaxsize)
{
	char *pflagfile = NULL;
	char devnode[64] = {0};
	
	if(get_sd_stats() < 0)
		return -1;

	pflagfile = fopen("/var/log/has_sd_card", "r");
	if(pflagfile != NULL)
	{
		fread(devnode, 1, sizeof(devnode), pflagfile);
		if(strcmp(devnode, "") == 0)
		{
			snprintf(pDevname, iMaxsize, "/dev/mmcblk0p1");
		}
		else
		{
			snprintf(pDevname, iMaxsize, "%s", devnode);
		}
		warning(">>>>>>> mmc device name: %s\n", pDevname);

		fclose(pflagfile);
	}
	return 0;
}

/**********************************************************************
函数描述：找到最老的avi文件夹中最老的avi文件，然后删除该文件。
          如果在最老的avi文件夹中没有找到avi文件，则删除该文件夹。
          如果找不到最老的avi文件夹，则返回0
入口参数：无
返回值：  0: 成功,删除了最老的文件或者最老的文件夹，正常返回
          2: SD 卡根目录下找不到文件夹，整张SD卡上都没文件
**********************************************************************/
int grd_sd_del_oldest_avi_file(void)
{
    char min_dir[128];
    int ret;
    memset(min_dir, 0, sizeof(min_dir));
    ret = find_oldest_dir(min_dir);
    if(ret == 2) // 0,find dir;  2, can't find dir
    {
        PRINT_INFO("no dir to del.\n");
        return 2;
    }
    del_oldest_file(min_dir);

    return 0;
}

/**********************************************************************
函数描述：找到最老的avi文件夹中最老的avi文件，然后重命名该文件。
          如果在最老的avi文件夹中没有找到avi文件，则删除该文件夹。
          如果找不到最老的avi文件夹，则返回0
入口参数：无
返回值：  0: 成功,重命名了最老的文件
          2: SD 卡根目录下找不到文件夹，整张SD卡上都没文件
         -1: 操作失败。
**********************************************************************/
int grd_sd_rename_oldest_avi_file(char *new_name, char *old_name)
{

    char min_dir[128];
    int ret;
	#if 0
    memset(min_dir, 0, sizeof(min_dir));
    ret = find_oldest_dir(min_dir);
    if(ret == 2) // 0,find dir;  2, can't find dir
    {
        PRINT_INFO("no dir to del.\n");
        return 2;
    }

	//往下执行代码找到了最老文件夹
    ret = rename_oldest_file(min_dir, new_name, old_name);
    if( 0 == ret ) // rename success, we checkout record index info; such as .sched
    {
        char *p = strstr(old_name, "ch0");
        // printf("%s\t%s[%d]............%s,%s,%d\n", __FILE__,__FUNCTION__, __LINE__, old_name, p,recording_file_info.rec_type);
        // /mnt/sd_card/20150306/ch0_20150306032107_20150306032607.avi
        ret = search_rm_node_by_name(p, recording_file_info.rec_type);
        int i = 0;
	 if( -2 == ret )
	 {
            for( i = 0; i < 4; i++)
            {
                if( i == recording_file_info.rec_type )
                {
                    continue;
                }
                else
                {
                    search_rm_node_by_name(p, i);
                }
            }
	 }
    }
	//文件夹为空，删除文件夹，可改为while循环查找
	if(ret == 3)
	{
		memset(min_dir, 0, sizeof(min_dir));
		ret = find_oldest_dir(min_dir);
		if(ret == 0)
		{
			PRINT_INFO("no dir to del.\n");
			return 0;
		}

		ret = rename_oldest_file(min_dir, new_name, old_name);
		if( 0 == ret ) // rename success, we checkout record index info; such as .sched
		{
		       //printf("%s\t%s[%d]............%s,%d\n", __FILE__,__FUNCTION__, __LINE__, old_name, recording_file_info.rec_type);
			char *p = strstr(old_name, "ch0");
			search_rm_node_by_name(p, recording_file_info.rec_type);
		}
	}
#endif

	return ret;
}

/**********************************************************************
线程描述 :
1. 检测SD  卡，若检测到sd 卡 自动挂载
2. 获取 sd 卡信息，当容量小于30M 时，会自动删除最旧的文件
3. 支持热插拔事件，当录像过程中拔掉sd 卡时，再次插上之后
	会继续录像。
**********************************************************************/

void *thread_sd_card_detecte(void *arg)
{
	int ret = -1;
	unsigned long long total_Mbytes;
	unsigned long long available_Mbytes;
	int tmp_normal_record = 0, tmp_event_record = 0,tmp_shcedule_record = 0;
	int tmp_normal_duration = 0, tmp_event_duration = 0,tmp_shcedule_duration = 0;
	
	char oldest_dir_name[64] = {0};
	unsigned char time_zone = g_record_work_t.record_time_zone;

	pthread_mutex_t sd_detecte = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_init(&sd_detecte,NULL);

	while(1)
	{	
	    if(SD_CARD_FORMATING == 1)
	    {
			continue;
	    }
		
		//这里检测到格式化sd 卡，不能再次挂载了		
		if(1 == g_format_SDcard)
		{
			warning("============2=================\n");
			pthread_mutex_lock(&sd_detecte);
			pthread_cond_broadcast(&cond);
			pthread_mutex_unlock(&sd_detecte);
		 	g_format_SDcard = -1;			
			g_sd_tats = -1;
			tmp_normal_record = 0;
			tmp_event_record = 0;
			
			warning("============3=================\n");
			warning("....g_format_SDcard .....\n");
			sleep(7);
		}
		
		if(0 > get_sd_stats())// SD卡不存在
		{
			g_sd_tats = -1;
		#if 1	
			//保存录像状态
			if(g_record_work_t.g_normal_status == 1 )
			{
				pthread_mutex_lock(&sd_detecte);
				tmp_normal_record = 1;
				tmp_normal_duration = g_record_work_t.g_normal_duration;
				pthread_mutex_unlock(&sd_detecte);
				stop_normal_record_stream();
			}
			if(g_record_work_t.g_event_status == 1)
			{
				pthread_mutex_lock(&sd_detecte);
				tmp_event_record = 1;
				tmp_event_duration = g_record_work_t.g_event_duration;
				pthread_mutex_unlock(&sd_detecte);
				stop_event_record_stream();
			}
			
		#endif
		}
		else
		{   
			g_sd_tats = 1;
#if 1
			//   热插拔之后重新录像
			
			if(1 == tmp_normal_record)
			{
				sleep(3);
				tmp_normal_record = 0;
				warning("restart normal recording..\n");
				st_record_start(SDK_RECORD_TYPE_MANU,tmp_normal_duration);
				tmp_normal_duration = 0;
			}
			
			if(1 == tmp_event_record)
			{
				sleep(3);
				tmp_event_record = 0;
				warning("restart event recording..\n");
				st_record_start(SDK_RECORD_TYPE_MOTION,tmp_event_duration);
				tmp_event_duration = 0;
			}
#endif
			//  files are  removed if available space less than 30M
			st_get_sd_card_infomat(GRD_SD_MOUNT_POINT, &total_Mbytes, &available_Mbytes);
#if _MSG_DEBUG
			info("Total_size = (%llu)B = (%llu)KB = (%llu) MB = [%llu] GB\n",  \ 
        	total_Mbytes, total_Mbytes>>10, total_Mbytes>>20, total_Mbytes>>30);  
			info("available_Mbytes = (%llu)B = (%llu)KB = (%llu) MB = [%llu] GB\n",  \ 
        	available_Mbytes, available_Mbytes>>10, available_Mbytes>>20, available_Mbytes>>30); 
#endif	
			if((available_Mbytes>>20) < 30)//剩余空间小于30M，就去删除最老的文件夹 【所谓循环覆盖】
			{	
				find_oldest_dir(oldest_dir_name);
				
				pthread_mutex_lock(&sd_detecte);
				ret = del_oldest_file(oldest_dir_name);
				pthread_mutex_unlock(&sd_detecte);
				if(3 == ret)
				{
					info("delete oldest directory!\n");
				}
				else if(0 == ret)
				{
					info("delete oldest file!\n");
				}
				else
				{
					info("delete Error!!\n");
				}
				
			}
			
		//	warning("============six=================\n");
		}
		usleep(300*1000);
	}
}
/**********************************************************************
函数描述；检测到SD卡，会自动挂载；当sd卡的容量小于30M
                           时，会自动删除最旧的视频。
入口参数： 无
返回值：      无
**********************************************************************/

void sd_card_detecte_thread()
{
	pthread_t ThreadID;
	int ret;
	if((ret = pthread_create(&ThreadID, NULL, &thread_sd_card_detecte, NULL)))
	{
		printf("pthread_create ret=%d\n", ret);
		exit(-1);
	}
	pthread_detach(ThreadID);
}

/**********************************************************************
线程描述 :
**********************************************************************/
void *thread_sd_card_file_fixed(void *arg)
{
	while(1)
	{	
		if(0 > get_sd_stats())// SD卡不存在无需修复
		{
			
		}
		else
		{   
			fix_all_avi_file();
		}
		sleep(20);
	}
}

void sd_card_file_fixed_thread()
{
	pthread_t ThreadID;
	int ret;
	if((ret = pthread_create(&ThreadID, NULL, &thread_sd_card_file_fixed, NULL)))
	{
		printf("pthread_create ret=%d\n", ret);
		exit(-1);
	}
	pthread_detach(ThreadID);
}

/**********************************************************************
函数描述：判断图片是否是jpg 格式
入口参数：图片名称
返回值：0 : 成功
			-1 : 失败
**********************************************************************/

int is_jpg_pic_format(char * file_name)
{
	int n = strlen(file_name);

	if(n != 18)
	{
		return -1;
	}
	
	if((file_name[14] == '.')&&(file_name[15] == 'j') && (file_name[16] == 'p')&& (file_name[17] == 'g'))
		return 0;
	return -1;
}

/**********************************************************************
函数描述：查找jpg 格式的图片

入口参数：file_path 图片在sd 卡中的路径，
				pic_infomat 图片名称的信息，以时间命名的YYYYMMDDHHMMSS 
				
返回值：0 : 成功
			-1 : 失败
**********************************************************************/

int st_find_car_picture(const char * file_path, pic_info * pic_info_t)
{
	DIR *pDir;
    struct dirent *ent;
	int i = 0;;
	
	int yyyy ,mm, dd, hh, MM, ss;
	char buf[128] = {0};
	if(0 != access(file_path, F_OK))
	{
		error("file path not found!!\n");
		return -1;
	}


    pDir = opendir(file_path);
    while((ent = readdir(pDir)) != NULL)
    {
        if(ent->d_type & DT_DIR)
        {        
            if(strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0)
                continue;
	    }
		if(0 != is_jpg_pic_format(ent->d_name))
		{
			return -1;
		}
		sscanf(ent->d_name, "%04d%02d%02d%02d%02d%02d.jpg",&yyyy, &mm, &dd, &hh, &MM, &ss);
		pic_info_t->pic_num[i].year = yyyy;
		pic_info_t->pic_num[i].mon = (char)mm;
		pic_info_t->pic_num[i].day = (char)dd; 
		pic_info_t->pic_num[i].hour = (char)hh;
		pic_info_t->pic_num[i].min = (char)MM;
		pic_info_t->pic_num[i].sec = (char)ss;
#if 0
		warning("==[%s]====%04d%02u%02u%02u%02u%02u \n",__FUNCTION__, pic_info_t->pic_num[i].year, \
			pic_info_t->pic_num[i].mon, pic_info_t->pic_num[i].day, pic_info_t->pic_num[i].hour, \
			pic_info_t->pic_num[i].min, pic_info_t->pic_num[i].sec);
#endif
		i++;
    }
	
	pic_info_t->count = i;
	
    closedir(pDir);
}


