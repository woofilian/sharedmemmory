#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include<unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "avi_common.h"
#include "avi_api.h"
#include "grd_avi_rec_api.h"
#include "mmc_api.h"

#include "sdk_struct.h"

extern int avi_add_index(AviFile *avi_file);
extern int is_fix_format_file(char *str);

static int avi_flag_pause = 0;
//pthread_mutex_t avi_pb_speed_x;
int avi_pb_speed = 1;


#define PB_N_LOOP 80


/**********************************************************************
函数描述：通过读取每一帧的信息得到index，index保存在内存中
入口参数：AviFile *avi_file: avi文件的控制句柄
          AviInitParam *param: avi文件的设置参数
返回值：  <0: 错误
           0: 正常
**********************************************************************/
static int avi_fix_read_data(AviFile *avi_file,int indexlen)
{
#if 0
    int i;
    int ret;

    if( (!avi_file))
    {
        PRINT_ERR_MSG("parameter error.\n");
        return -1;
    }

    //read frame data frame by frame
    avi_file->idx_array_count = 0;
    avi_file->video_count = 0;
	avi_file->audio_count = 0;
    avi_file->audio_bytes = 0;
    avi_file->index_count = 0;
    avi_file->data_offset = sizeof(AviHeader);
    avi_file->idx_array = NULL;

    int last_audio_bytes;
	
    avi_file->index_count = indexlen/16;
	avi_file->idx_array = (int *)malloc(indexlen);
	fseek(avi_file->idxfile, 0L, SEEK_SET);	
	ret = fread(avi_file->idx_array, indexlen, 1, avi_file->idxfile);
	if(ret != 1)
    {
       error("read avi index array failed.\n");
       return -1;
    }
	//计算index的音视频个数
	for(i=0;i<avi_file->index_count;i++)
	{
	   if(avi_file->idx_array[4*i]== MAKE_FOURCC('0','0','d','c'))
	   	avi_file->video_count++;
	   else if(avi_file->idx_array[4*i]== MAKE_FOURCC('0','1','w','b'))
	   	avi_file->audio_count++;
	}
	PRINT_INFO(">>>>>>>>>>>>>>>avi_file->video_count=%d,avi_file->audio_count=%d\n",avi_file->video_count,avi_file->audio_count);
    avi_file->data_offset=avi_file->idx_array[4*(avi_file->index_count-1)+2];
	PRINT_INFO(">>>>>>>>>>>>>>>avi_file-..data_offset=%d",avi_file->data_offset);
#endif
	return 0;
}

/**********************************************************************
函数描述：修复单个未录完成的avi文件
入口参数：char *file_path: avi文件路径名
返回值：  <0: 错误
           0: 正常返回
**********************************************************************/
static int fix_single_avi_file(char *file_path,char *index_path)
{
#if 0
    AviFile avi_file;
	int startTime, gstopTime;
	int duration;

    char name_tmp[128];
    int ret;
	
	int fd = access(file_path,F_OK);
	if(fd == -1)
	{		 
	   PRINT_INFO(">>>>>>>fix_single_avi_file in not here\n");
	   remove(index_path);
	   return -1;
	}

    //open file
    strncpy(avi_file.filename, file_path, sizeof(avi_file.filename));
	strncpy(avi_file.idxfilename, index_path, sizeof(avi_file.idxfilename));
    PRINT_INFO("fixing file : %s,,,avi_file.idxfilename==%s\n", avi_file.filename,avi_file.idxfilename);
    avi_file.file = fopen(avi_file.filename, "rb+");
    if(NULL == avi_file.file)
    {
        PRINT_ERR_MSG("open %s failed.\n", avi_file.filename);
        return -1;
    }
	
    avi_file.idxfile= fopen(avi_file.idxfilename, "rb+");
    if(NULL == avi_file.idxfile)
    {
        PRINT_ERR_MSG("open %s failed.\n", avi_file.idxfilename);
        return -1;
    }
	
    fseek(avi_file.idxfile, 0L, SEEK_END);
    int indexlen = ftell(avi_file.idxfile);
	PRINT_INFO(">>>>>>>>>>>>>>>indexlen=%d",indexlen);

    if(indexlen <= 48 )
    {
       PRINT_INFO(">>>>>>>>>>>>>>read avi index array failed.\n");
	   if(avi_file.file)
	   {
		   fclose(avi_file.file);
		   avi_file.file = NULL;
		   remove(avi_file.filename);
	   }
	   if(avi_file.idxfile)
	   {
		   fclose(avi_file.idxfile);
		   avi_file.idxfile= NULL;
		   remove(avi_file.idxfilename);
	   }
	   
	   if(avi_file.idx_array)
	   {
		   free(avi_file.idx_array);
		   avi_file.idx_array = NULL;
	   }
	   
       return -1;
    }

    //read data
    ret = avi_fix_read_data(&avi_file,indexlen);
    if(ret < 0)
    {
        PRINT_ERR_MSG("call avi_fix_read_data error.\n");
        if(avi_file.file)
        {
            fclose(avi_file.file);
            avi_file.file = NULL;
        }
        if(avi_file.idxfile)
        {
            fclose(avi_file.idxfile);
            avi_file.idxfile= NULL;
        }

        if(avi_file.idx_array)
        {
            free(avi_file.idx_array);
            avi_file.idx_array = NULL;
        }
        return -1;
    }
	//以下是计算录像的间隔时间
	ret = fseek(avi_file.file, avi_file.data_offset-8, SEEK_SET);
	ftell(avi_file.file);
	ret = read_int32(avi_file.file, &gstopTime);
	if(ret != 1)
    {
       error("read avi index array failed.\n");
       return -1;
    }
	PRINT_INFO(" avi file format gstopTime %d \n",gstopTime);
 	ret = fseek(avi_file.file, avi_file.idx_array[3]+8+sizeof(AviHeader)+8, SEEK_SET);
	ret = read_int32(avi_file.file, &startTime);
	if(ret != 1)
    {
       error("read avi file frame data failed.\n");
       return -1;
    }
	printf(" avi file format startTime=%d,dur=%d,\n",startTime,gstopTime-startTime);
	duration = gstopTime-startTime;
    PRINT_INFO(" duration= %d \n", duration);

    ret = avi_add_index_fixed(&avi_file);
    if(ret < 0)
    {
        PRINT_ERR_MSG("call avi_add_index_fixed error.\n");
        if(avi_file.file)
        {
            fclose(avi_file.file);
            avi_file.file = NULL;
        }
        if(avi_file.idxfile)
        {
            fclose(avi_file.idxfile);
            avi_file.idxfile= NULL;
        }

        if(avi_file.idx_array)
        {
            free(avi_file.idx_array);
            avi_file.idx_array = NULL;
        }
        return -1;
    }

    if(avi_file.file)
    {
        fclose(avi_file.file);
        avi_file.file = NULL;
    }
	if(avi_file.idxfile)
	{
		fclose(avi_file.idxfile);
		avi_file.idxfile= NULL;
	}

    if(avi_file.idx_array)
    {
        free(avi_file.idx_array);
        avi_file.idx_array = NULL;
    }
	
     //rename filename_tmp to filename.avi
    memset(name_tmp, 0, sizeof(name_tmp));
    sprintf(name_tmp, "%s-%d.avi", avi_file.filename, duration); //ch0_20120101010101_20120101011101.avi

    ret = rename(avi_file.filename,name_tmp);
    if(ret)
    {
        PRINT_ERR_MSG("rename %s to %s failed.\n", name_tmp, avi_file.filename);
        return -1;
    }
	remove(avi_file.idxfilename);
    PRINT_INFO("fix %s to %s\n", name_tmp, avi_file.filename);
#endif
    return 0;
}

/**********************************************************************
函数描述：修复sd卡中所有的未录完成的avi文件
入口参数：无
返回值：  无
**********************************************************************/
void fix_all_avi_file(void)
{
    DIR *pDir;
    struct dirent *ent;
    char fullname[128]={0};
	char indexname[64]={0};
	struct tm *tp ;
    uint64_t recodertime;
    pDir = opendir(GRD_SD_MOUNT_POINT);

    if( !pDir )
        return ;
    while((ent = readdir(pDir)) != NULL)
    {
        if(0 > get_sd_stats())// SD卡不存在无需修复
		{
			break;
		}
		
        if(ent->d_type & DT_REG)  //判断是否为非目录
        {
            if(strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0)
                continue;
			recodertime = atoll(ent->d_name);
			if(recodertime < 20170112)
				continue;
			if(abs(get_UTC_Time() - recodertime) < 300)//当前时间与录像时间间隔小于600则不修复
				continue;
			tp = localtime(&recodertime);  
			memset(fullname,0x0, sizeof(fullname));
			snprintf(fullname, sizeof(fullname), "%s/%04d%02d%02d/%s", GRD_SD_MOUNT_POINT,tp->tm_year + 1900, tp->tm_mon+1, tp->tm_mday,ent->d_name);
			memset(indexname,0x0, sizeof(indexname));
			snprintf(indexname, sizeof(indexname), "%s/%s", GRD_SD_MOUNT_POINT,ent->d_name);
			fix_single_avi_file(fullname,indexname);
			sleep(5);
        }
    }

    closedir(pDir);
}

