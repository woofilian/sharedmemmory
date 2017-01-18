
#include "avi_common.h"
#include "avi_api.h"
#include "log.h"
#include "avi_typedef.h"
//#include "AvObj.h"
//#include "grd_sdcard.h"
#include "grd_avi_pb_api.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>

#include "sdk_struct.h"

#define _SINGLE_DEBUG_ 0
#define _DEBUG_MSG_ON_ 1

/**********************************************************************
函数描述：GRD_TIME 类型时间转换成 unsigned long long 类型时间
入口参数：GRD_TIME *p: sdk_time_t 类型时间
返回值：  uint64_t: unsigned long long 类型时间
**********************************************************************/
uint64_t time_to_u64t(sdk_time_t *p)
{
    char tmp[20];
    uint64_t tmp64 = 0;
    memset(tmp, 0, sizeof(tmp));
    sprintf(tmp, "%04lu%02lu%02lu%02lu%02lu%02lu", p->year, p->mon, p->day,
                                            p->hour, p->min, p->sec);
    tmp64 = atoll(tmp);
    return tmp64;
}

/**********************************************************************
函数描述：unsigned long long 类型时间转换成 sdk_time_t 类型时间
入口参数：GRD_TIME *ptime: 得到 sdk_time_t 类型时间
          uint64_t tmp64: unsigned long long 类型时间
返回值：  无
**********************************************************************/
void u64t_to_time(sdk_time_t *ptime, uint64_t tmp64)
{
    char tmp_str[20];
    char tmp[20];

    memset(tmp_str, 0, sizeof(tmp_str));
    sprintf(tmp_str, "%lld", tmp64);

    memset(tmp, 0, sizeof(tmp));
    avi_substring(tmp, tmp_str, 0, 0+4);
    ptime->year= atoi(tmp);

    memset(tmp, 0, sizeof(tmp));
    avi_substring(tmp, tmp_str, 4, 4+2);
    ptime->mon= atoi(tmp);

    memset(tmp, 0, sizeof(tmp));
    avi_substring(tmp, tmp_str, 6, 6+2);
    ptime->day= atoi(tmp);

    memset(tmp, 0, sizeof(tmp));
    avi_substring(tmp, tmp_str, 8, 8+2);
    ptime->hour= atoi(tmp);

    memset(tmp, 0, sizeof(tmp));
    avi_substring(tmp, tmp_str, 10, 10+2);
    ptime->min= atoi(tmp);

    memset(tmp, 0, sizeof(tmp));
    avi_substring(tmp, tmp_str, 12, 12+2);
    ptime->sec= atoi(tmp);

}

/**********************************************************************
函数描述：得到文件的大小
入口参数：FILE_NODE *node: 单个文件的信息节点
返回值：  uint32_t: 文件的大小,以字节为单位
**********************************************************************/
uint32_t get_file_size(FILE_NODE *node)
{
    FILE *fp;
    uint32_t tmp32;
    if(node == NULL)
    {
        PRINT_ERR();
        return -1;
    }

    fp = fopen(node->path, "rb");
    if(fp == NULL)
    {
        PRINT_ERR();
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    tmp32 = ftell(fp);
    fclose(fp);

    return tmp32;
}

static FILE_LIST *creat_empty_list(uint32_t type, uint32_t ch_num, uint64_t start, uint64_t stop)
{

    FILE_NODE *h = (FILE_NODE *)malloc(sizeof(FILE_NODE));
    if(h == NULL)
    {
        PRINT_ERR();
        return NULL;
    }
    h->next = NULL;

    FILE_LIST *list = (FILE_LIST *)malloc(sizeof(FILE_LIST));
    if(list == NULL)
    {
        PRINT_ERR();
        return NULL;
    }
    list->type = type;
    list->ch_num = ch_num;
    list->start = start;
    list->stop = stop;
    list->head = h;
    list->len = 0;
    list->index = 0;

    return list;
}

static int insert_node(FILE_LIST *list, char *file_path)
{
    int i;
    FILE_NODE *p, *q;
    char file_name[128];
    p = NULL;
    q = NULL;

    memset(file_name, 0 ,sizeof(file_name));
    i = last_index_at(file_path, '/');
    strncpy(file_name, file_path + i + 1, sizeof(file_name));

    p = (FILE_NODE *)malloc(sizeof(FILE_NODE));
    if(p == NULL)
    {
        PRINT_ERR();
        return -1;
    }
    strncpy(p->path, file_path, sizeof(p->path));
    p->start = avi_get_start_time(file_name);
    p->stop = avi_get_stop_time(file_name);
    p->next = NULL;

#if _DEBUG_MSG_ON_
    PRINT_INFO("insert node to list: file_path = %s, start = %llu, stop = %llu\n",
            p->path, p->start, p->stop);
#endif

    q = list->head;

    while(q->next != NULL)
    {
        if(q->next->start < p->start)
            q = q->next;
        else if(q->next->start == p->start) //避免重复插入
        {
            free(p);
            return 0;
        }
        else
            break;
    }


    p->next = q->next;
    q->next = p;

    list->len ++;

    return 0;
}

/**********************************************************************
函数描述：判断是否查找到文件
入口参数：FILE_LIST *list: 查找文件的控制句柄
返回值：  1: 空，没有找到文件
          0: 非空，查找到了文件
**********************************************************************/
int is_empty_list(FILE_LIST *list)
{
    return NULL == list->head->next;
}

/**********************************************************************
函数描述：得到查找到的文件数目
入口参数：FILE_LIST *list: 查找文件的控制句柄
返回值：  int: 查找到的文件数目
**********************************************************************/
int get_len_list(FILE_LIST *list)
{
    int i = 0;
    FILE_NODE *p;
    p = list->head;
    while (p->next != NULL)
    {
        i++;
        p = p->next;
    }
    return i;
}

#if _DEBUG_MSG_ON_
/**********************************************************************
函数描述：打印管理文件中的一次录像动作的信息
入口参数：INDEX_NODE *n: 录像动作的信息指针
返回值：  无
**********************************************************************/
void print_index_node(INDEX_NODE *n)
{
    PRINT_INFO("******** INDEX NODE *******\n");
    PRINT_INFO("ch_num = %u\n", n->ch_num);
    PRINT_INFO("start = %llu\n", n->start);
    PRINT_INFO("stop = %llu\n", n->stop);
    PRINT_INFO("******** INDEX NODE *******\n");
}

/**********************************************************************
函数描述：打印查找文件控制句柄中的单个文件信息
入口参数：FILE_NODE *n: 查找后得到的单个文件信息
返回值：  无
**********************************************************************/
void print_node(FILE_NODE *n)
{
    PRINT_INFO("******** NODE *******\n");
    PRINT_INFO("path = %s\n", n->path);
    PRINT_INFO("start = %llu\n", n->start);
    PRINT_INFO("stop = %llu\n", n->stop);
    PRINT_INFO("******** NODE *******\n");
}

/**********************************************************************
函数描述：打印查找文件控制句柄中的所有文件信息
入口参数：FILE_LIST *list: 查找文件控制句柄
返回值：  无
**********************************************************************/
void print_list(FILE_LIST *list)
{
    PRINT_INFO("\n-----------------------------------------------------------------\n");
    PRINT_INFO("print list:\n");
    PRINT_INFO("num = %u, type = %u\n", list->ch_num, list->type);
    PRINT_INFO("start = %llu, stop = %llu\n", list->start, list->stop);
    PRINT_INFO("len = %u, index = %u\n", list->len, list->index);

    FILE_NODE *q;
    q = list->head;
    while(q->next != NULL)
    {
        print_node(q->next);
        q = q->next;
    }

    PRINT_INFO("-----------------------------------------------------------------\n\n");
}

/**********************************************************************
函数描述：打印安普接口的 GRD_NET_FINDDATA 信息
入口参数：GRD_NET_FINDDATA *p: 查找后得到的单个文件信息
返回值：  无
**********************************************************************/
void print_finddata(sdk_record_cond_t *p)
{
#if 0
    PRINT_INFO("\n-----------------------------------------\n");
    PRINT_INFO("print finddata:\n");
    PRINT_INFO("dwSize = %lu\n", p->dwSize);
    PRINT_INFO("csFileName = %s\n", p->csFileName);
    PRINT_INFO("dwFileType = %lu\n", p->dwFileType);

    PRINT_INFO("start = %04lu%02lu%02lu%02lu%02lu%02lu \n", p->stStartTime.dwYear,
                                                        p->stStartTime.dwMonth,
                                                        p->stStartTime.dwDay,
                                                        p->stStartTime.dwHour,
                                                        p->stStartTime.dwMinute,
                                                        p->stStartTime.dwSecond);
    PRINT_INFO("stop  = %04lu%02lu%02lu%02lu%02lu%02lu \n", p->stStopTime.dwYear,
                                                        p->stStopTime.dwMonth,
                                                        p->stStopTime.dwDay,
                                                        p->stStopTime.dwHour,
                                                        p->stStopTime.dwMinute,
                                                        p->stStopTime.dwSecond);
    PRINT_INFO("dwFileSize = %lu\n", p->dwFileSize);
    PRINT_INFO("-----------------------------------------\n");
#endif
}
#endif


static void delete_node(FILE_LIST *list, int index)
{
    if(is_empty_list(list))
    {
        PRINT_ERR();
        return;
    }

    FILE_NODE *p, *q;

    p = list->head;

    if(index >= get_len_list(list) || index < 0)
    {
        PRINT_ERR();
        return;
    }


    while(index--)
        p = p->next;

    q = p->next;
    p->next = q->next;

#if _DEBUG_MSG_ON_
    print_node(q);
    PRINT_INFO("len = %lu\n", list->len);
#endif

    q->next = NULL;
    free(q);

    list->len --;
}

/**********************************************************************
函数描述：查找文件结束
入口参数：FILE_LIST *: 查找文件的控制句柄
返回值：  无
**********************************************************************/
void search_close(FILE_LIST *list)
{
    while(list->len)
        delete_node(list, 0);

    list->head->next = NULL;
    free(list->head);

    list->head = NULL;
    free(list);
}

/**********************************************************************
函数描述：根据时间段查找文件
入口参数：FILE_LIST *: 查找文件的控制句柄
          FILE_NODE *node: 得到文件信息指针
返回值：  安普定义如下:
          GRD_NET_FILE_SUCCESS: 获取文件信息成功
          GRD_NET_NOMOREFILE: 没有更多的文件，查找结束
          GRD_NET_FILE_NOFIND: 未查找到文件
**********************************************************************/
int get_file_node(FILE_LIST *list, FILE_NODE *node)
{
/*
#define GRD_NET_FILE_SUCCESS 1000// 获取文件信息成功
#define GRD_NET_FILE_NOFIND 1001// 未查找到文件
#define GRD_NET_ISFINDING 1002// 正在查找请等待
#define GRD_NET_NOMOREFILE  1003// 没有更多的文件，查找结束
#define GRD_NET_FILE_EXCEPTION  1004// 查找文件时异常
*/
    if(is_empty_list(list))
        return GRD_NET_FILE_NOFIND;

    FILE_NODE *p;
    uint32_t index;
    p = list->head;
    index = list->index;

    if(index  == list->len)
       return GRD_NET_NOMOREFILE;

    while(index--)
        p = p->next;

    strncpy(node->path, p->next->path, sizeof(node->path));
    node->start = p->next->start;
    node->stop = p->next->stop;


    list->index ++;
    return GRD_NET_FILE_SUCCESS;
}

static int add_file_to_list(FILE_LIST *list, uint32_t ch_num, uint64_t begin, uint64_t end, char *dir, int ignore_type)
{
    DIR *pDir;
    struct dirent *ent;
    uint32_t num;
    int ret;

    uint64_t time_start_64, time_stop_64 = 0;


    char dir_path[128];
    char file_path[128];

    memset(dir_path, 0, sizeof(dir_path));
    sprintf(dir_path, "%s/%s", GRD_SD_MOUNT_POINT, dir);


	warning("dir_path ===== %s\n", dir_path);

    pDir = opendir(dir_path);
    if( !pDir )
        return -1;

    while((ent = readdir(pDir)) != NULL)
    {

        if(ent->d_type & DT_REG)  //判断是否为非目录
        {

            if (0 == is_avi_file_format(ent->d_name))
                continue;

#if _DEBUG_MSG_ON_
			warning("dir_path ent->d_name===== %s/%s\n", dir_path,ent->d_name);
#endif
			
            num = avi_get_ch_num(ent->d_name);
#if _DEBUG_MSG_ON_
            PRINT_INFO("ch_num = %lu, file_num = %lu\n", ch_num, num);
#endif
            if(ch_num != num)
                continue;

            time_start_64 = avi_get_start_time(ent->d_name);
            if( 37 == strlen(ent->d_name) )
                time_stop_64 = avi_get_stop_time(ent->d_name);
#if _DEBUG_MSG_ON_
            PRINT_INFO("time_start_64 = %llu, time_stop_64 = %llu\n", time_start_64, time_stop_64);
#endif

            if(( time_stop_64 && time_stop_64 < begin) || time_start_64 > end)
                continue;
            if(!ignore_type)
            {
                if( ( time_stop_64 && time_stop_64 != end ) || time_start_64 != begin)
                    continue;
            }
            memset(file_path, 0, sizeof(file_path));
            sprintf(file_path, "%s/%s", dir_path, ent->d_name);
#if _DEBUG_MSG_ON_
            PRINT_INFO("match file, file_path = %s\n", file_path);
#endif

		    ret = insert_node(list, file_path);
            if(ret != 0)
            {
                PRINT_ERR();
                return -1;
            }
        }
    }

    closedir(pDir);
    return 0;
}

static void add_to_list(FILE_LIST *list, uint32_t ch_num, uint64_t begin, uint64_t end, int ignore_type)
{
    DIR *pDir;
    struct dirent *ent;
    uint32_t date = 0;

    uint32_t date_start_32, date_stop_32;
    int FLAG = 0;
	char filename[120]={0};
#if _DEBUG_MSG_ON_
    PRINT_INFO("\n\n******************** find dir ****************************\n");
    PRINT_INFO("find dir: begin = %llu, end = %llu\n", begin, end);
#endif
    date_start_32 = (uint32_t)(begin / 1000000);
    date_stop_32 = (uint32_t)(end / 1000000);
#if _DEBUG_MSG_ON_
    PRINT_INFO("find dir: dir_start = %lu, dir_stop = %lu\n", date_start_32, date_stop_32);
    PRINT_INFO("******************** find dir ****************************\n");
#endif


    pDir = opendir(GRD_SD_MOUNT_POINT);
    if( !pDir )
        return;

    while((ent = readdir(pDir)) != NULL)
    {
        if(ent->d_type & DT_DIR)  //判断是否为目录
        {
            if(strcmp(ent->d_name,".") == 0 || strcmp(ent->d_name,"..") == 0)
                continue;
#if _DEBUG_MSG_ON_
            PRINT_INFO("\n\nfind dir: \n");
            PRINT_INFO("dir_name = %s\n", ent->d_name);
#endif
			
			//warning("========>>>> ent->d_name:%s\n",ent->d_name);
            if(is_avi_dir_format(ent->d_name) == 0)
                continue;

            date = (uint32_t)atoi(ent->d_name);
#if _DEBUG_MSG_ON_
            PRINT_INFO("date = %lu, start = %lu, stop = %lu\n",
                    date, date_start_32, date_stop_32);
#endif
            if((date == date_start_32) && (date == date_stop_32))
            {
#if _DEBUG_MSG_ON_
                PRINT_INFO("match dir, dir_name = %s\n", ent->d_name);
#endif          FLAG = 1;
                memcpy(filename,ent->d_name,sizeof(filename));
                break;
            }
        }
    }

    closedir(pDir);
	if(FLAG ==1)
	{
		add_file_to_list(list, ch_num, begin, end, filename, ignore_type);
	}
	else
	{
       list = NULL;
	}

}

/**********************************************************************
函数描述：根据时间段查找文件
入口参数：u32t type: 安普定义如下
          SDK_RECORD_TYPE_SCHED   定时录像
          SDK_RECORD_TYPE_MOTION  移到侦测录像
          SDK_RECORD_TYPE_ALARM   手动报警录像
          SDK_RECORD_TYPE_MANU    手工录像
          uint32_t ch_num: 通道号
          uint64_t start: 起始时间
          uint64_t stop:  结束时间
返回值：  FILE_LIST *: 返回的查找文件控制句柄
**********************************************************************/
FILE_LIST *search_file_by_time(uint32_t type, uint32_t ch_num, uint64_t start, uint64_t stop)
{
    FILE *fp;
    char index_file[128];
    int count, i, ret;
    INDEX_NODE node;
    uint64_t begin, end;
#if _DEBUG_MSG_ON_
    PRINT_INFO("\n\n in get_list_by_time: \n");
#endif

    FILE_LIST *list = NULL;
    list = creat_empty_list(type, ch_num, start, stop);
    if(list == NULL)
    {
        PRINT_ERR();
        return NULL;
    }

#if _DEBUG_MSG_ON_
    PRINT_INFO("after create list: num = %lu, type = %lu, start = %llu, stop = %llu\n",
            list->ch_num, list->type, list->start, list->stop);
#endif

    memset(index_file, 0, sizeof(index_file));
	warning("TYPE: %x\n", type);
    switch(type)
    {
        case SDK_RECORD_TYPE_SCHED: //定时录像 0x000001
        
			warning("1-------------------------\n");
            sprintf(index_file, "%s/.sched", GRD_SD_MOUNT_POINT);
            break;
        case SDK_RECORD_TYPE_MOTION: //移到侦测录像 0x000002
			warning("2-------------------------\n");
            sprintf(index_file, "%s/.motion", GRD_SD_MOUNT_POINT);
            break;
        case SDK_RECORD_TYPE_ALARM: //报警录像 0x000004
			warning("3-------------------------\n");
            sprintf(index_file, "%s/.alarm", GRD_SD_MOUNT_POINT);
            break;
        case SDK_RECORD_TYPE_MANU: //手工录像 0x000010
			warning("4-------------------------\n");
            sprintf(index_file, "%s/.manu", GRD_SD_MOUNT_POINT);
            break;
        default:
			warning("5-------------------------\n");
            add_to_list(list, ch_num, start, stop, 1);  //在时间段内的全部录像
            return list;
            break;
    }

#if _DEBUG_MSG_ON_
    PRINT_INFO("index_file =%s\n", index_file);
#endif

	warning("index_file: %s\n", index_file);

    fp = fopen(index_file, "rb");
    if(fp == NULL)
    {
        PRINT_ERR();
        return NULL;
    }
    fseek(fp, 0, SEEK_SET);
    ret = fread(&count, sizeof(int), 1, fp);
    if(ret != 1)
    {
        PRINT_ERR();
        fclose(fp);
        return NULL;
    }

#if _DEBUG_MSG_ON_
    warning("count ====>%d\n\n", count);
#endif

    for(i=0; i<count; i++)
    {
#if _DEBUG_MSG_ON_
        PRINT_INFO(" i = %d, get index node info:\n", i);
#endif

        if(feof(fp))
        {
#if _DEBUG_MSG_ON_
            PRINT_INFO("find %s to end.\n", index_file);
#endif
            break;
        }

        memset(&node, 0, sizeof(INDEX_NODE));
        fread(&node, sizeof(INDEX_NODE), 1, fp);

#if _DEBUG_MSG_ON_
        print_index_node(&node);
#endif

        if(ch_num != node.ch_num)
            continue;

#if _DEBUG_MSG_ON_
    PRINT_INFO("---- position 1 ----\n");
#endif

        if((node.stop < start) || (stop < node.start))
            continue;
#if _DEBUG_MSG_ON_
    PRINT_INFO("---- position 2 ----\n");
#endif
        if((start <= node.start) && (node.stop <= stop))
        {
#if _DEBUG_MSG_ON_
    PRINT_INFO("---- position 3 ----\n");
#endif
            begin = node.start;
            end = node.stop;
        }
        else if((node.start <= start) && (stop <= node.stop))
        {
#if _DEBUG_MSG_ON_
    PRINT_INFO("---- position 4 ----\n");
#endif
            begin = start;
            end = stop;
        }
        else if((node.start <= start) && (start <= node.stop))
        {
#if _DEBUG_MSG_ON_
    PRINT_INFO("---- position 5 ----\n");
#endif
            begin = start;
            end = node.stop;
        }
        else if((node.start <= stop) && (stop <= node.stop))
        {
#if _DEBUG_MSG_ON_
    PRINT_INFO("---- position 6 ----\n");
#endif
            begin = node.start;
            end = stop;
        }

#if _DEBUG_MSG_ON_
    PRINT_INFO("---- position 7 ----\n");
    PRINT_INFO("adjust time : begin = %llu, end = %llu\n", begin, end);
#endif

        if(begin == end)
            continue;

#if _DEBUG_MSG_ON_
        PRINT_INFO("begin != end\n");
#endif

        add_to_list(list, ch_num, begin, end, 0);

    }

    fclose(fp);
    return list;
}

/**********************************************************************
函数描述：将每次录像动作记录到管理文件中
入口参数：u32t type: 安普定义如下
          SDK_RECORD_TYPE_SCHED   定时录像
          SDK_RECORD_TYPE_MOTION  移到侦测录像
          SDK_RECORD_TYPE_ALARM   手动报警录像
          SDK_RECORD_TYPE_MANU    手工录像
          uint32_t ch_num: 通道号
          uint64_t start: 起始时间
          uint64_t stop:  结束时间
返回值：  <0: 错误
           0: 正常
**********************************************************************/
int write_manage_info(uint32_t type, uint32_t ch_num, uint64_t start, uint64_t stop)
{
    char index_file[128];
    FILE *fp;
    int count, ret;
    INDEX_NODE index_node;
    memset(index_file, 0 ,sizeof(index_file));
	info(" write_manage_info  type:%d   \n",type);

	switch(type)
    {
    
        case SDK_RECORD_TYPE_SCHED: //定时录像 0x000001
            sprintf(index_file, "%s/.sched", GRD_SD_MOUNT_POINT);
            break;
        case SDK_RECORD_TYPE_MOTION: //移到侦测录像 0x000002
            sprintf(index_file, "%s/.motion", GRD_SD_MOUNT_POINT);
            break;
        case SDK_RECORD_TYPE_ALARM: //报警录像 0x000004
            sprintf(index_file, "%s/.alarm", GRD_SD_MOUNT_POINT);
            break;
        #if 0
        case GRD_NET_RECORD_TYPE_CMD: //命令录像 0x000008
            sprintf(index_file, "%s/.cmd", GRD_SD_MOUNT_POINT);
            break;
        #endif
        case SDK_RECORD_TYPE_MANU: //手工录像 0x000010
            sprintf(index_file, "%s/.manu", GRD_SD_MOUNT_POINT);
            break;
        #if 0
        case GRD_NET_RECORD_TYPE_SCHED_2: //手工录像 0x000011
            sprintf(index_file, "%s/.sched2", GRD_SD_MOUNT_POINT);
            break;
        #endif
        default:  break;
    }
 info(" write_manage_info  0000000000  \n");
    if(access(index_file, F_OK) != 0)
    {
        fp = fopen(index_file, "wb+");
        if(NULL == fp)
        {
            PRINT_ERR();
            return -1;
        }
        count = 0;
        fseek(fp, 0, SEEK_SET);
        ret = fwrite(&count, sizeof(int), 1, fp);
        if(ret != 1)
        {
            PRINT_ERR();
            return -1;
        }
		
		info(" write_manage_info  11111111111  \n");
#if _DEBUG_MSG_ON_
        PRINT_INFO("first write index.\n");
#endif
    }
    else
    {
        fp = fopen(index_file, "rb+");
        if(NULL == fp)
        {
            PRINT_ERR();
            return -1;
        }
        fseek(fp, 0, SEEK_SET);
        ret = fread(&count, sizeof(int), 1, fp);
        if(ret != 1)
        {
            PRINT_ERR();
            return -1;
        }
		
		info(" write_manage_info  222222222222  \n");
#if _DEBUG_MSG_ON_
        PRINT_INFO("not first write index, get count = %d\n", count);
#endif
    }

    memset(&index_node, 0 ,sizeof(INDEX_NODE));
    index_node.ch_num = ch_num;
    index_node.start = start;
    index_node.stop = stop;


    fseek(fp, 0, SEEK_END);
    ret = fwrite(&index_node, sizeof(INDEX_NODE), 1, fp);
    if(ret != 1)
    {
        PRINT_ERR();
        return -1;
    }

    PRINT_INFO("write to %s :count:%d ch_num = %u, start = %llu, stop = %llu.\n",
        index_file, count, index_node.ch_num, index_node.start, index_node.stop);

    count++;
    fseek(fp, 0, SEEK_SET);
    ret = fwrite(&count, sizeof(int), 1, fp);
    if(ret != 1)
    {
        PRINT_ERR();
        return -1;
    }
#if _DEBUG_MSG_ON_
    PRINT_INFO("after write, count = %d\n", count);
#endif

    fseek(fp, 0, SEEK_SET);
    fclose(fp);
    return 0;
}


/*****************************************************************
param: file: as "ch0_20150127104512_20150127104542.avi"
return: -1 error, 0 success;
*****************************************************************/
int search_check_file_node( char *file, int type)
{
	int count = 0, i = 0;
	char index_file[64];
	INDEX_NODE node;
	FILE *fp = NULL;
	unsigned long long begin = 	avi_get_start_time(file);
	unsigned long long end = avi_get_stop_time(file);
	// printf("%s[%d]=====================%s,%d\n", __FUNCTION__, __LINE__, file, type);

	for( i = 1; i <= 4; i++)
	{
		memset(index_file, 0 ,sizeof(index_file));
		if( type == i )
		{
			continue;
		}
		else
		{
			switch( i )
			{
				case SDK_RECORD_TYPE_SCHED:
					sprintf(index_file, "%s/.sched", GRD_SD_MOUNT_POINT);
					break;
				case SDK_RECORD_TYPE_MOTION:
					sprintf(index_file, "%s/.motion", GRD_SD_MOUNT_POINT);
					break;
				case SDK_RECORD_TYPE_ALARM:
					sprintf(index_file, "%s/.alarm", GRD_SD_MOUNT_POINT);
					break;
				case SDK_RECORD_TYPE_MANU:
					sprintf(index_file, "%s/.manu", GRD_SD_MOUNT_POINT);
					break;
				default:
					break;
			}

			fp = fopen(index_file, "rb");
			if( !fp )
			{
				// printf("%s[%d]=====================no file\n", __FUNCTION__, __LINE__);
				continue;
			}

			fread(&count, sizeof(int), 1, fp);
			for( i = 0;  i < count;  i++ )
			{
				if( feof(fp) )
				{
					PRINT_INFO("find %s to end.\n", index_file);
					break;
				}

				memset(&node, 0, sizeof(INDEX_NODE));
				fread(&node, sizeof(INDEX_NODE), 1, fp);
				if( begin == node.start && end == node.stop )
				{
					fclose(fp);
					return 0;
				}
			}

			if( fp )
			{
				fclose(fp);
				fp = NULL;
			}
		}
	}

	return 1;
}

/*****************************************************************
param: file: as "ch0_20150127104512_20150127104542.avi"
return: -1 error, 0 success;
*****************************************************************/
int search_rm_node_by_name( char *file, int type )
{
	int count = 0, i = 0, offset = 0, ret, size = 0;
	char index_file[64];
	INDEX_NODE node;
	FILE *fp = NULL;
	unsigned long long begin = 	avi_get_start_time(file);
	unsigned long long end = avi_get_stop_time(file);
	// printf("%s[%d]......begin=%lld,end=%lld, %d\n", __FUNCTION__, __LINE__, begin,end, sizeof(INDEX_NODE));

	memset(index_file, 0 ,sizeof(index_file));
	switch(type)
	{
		case SDK_RECORD_TYPE_SCHED:
			sprintf(index_file, "%s/.sched", GRD_SD_MOUNT_POINT);
			break;
		case SDK_RECORD_TYPE_MOTION:
			sprintf(index_file, "%s/.motion", GRD_SD_MOUNT_POINT);
			break;
		case SDK_RECORD_TYPE_ALARM:
			sprintf(index_file, "%s/.alarm", GRD_SD_MOUNT_POINT);
			break;
		case SDK_RECORD_TYPE_MANU:
			sprintf(index_file, "%s/.manu", GRD_SD_MOUNT_POINT);
			break;
		default:
			break;
	}

	fp = fopen(index_file, "rb");
	if( !fp )
		return -1;

	fseek(fp, 0, SEEK_SET);
	ret = fread(&count, sizeof(int), 1, fp);
	if( ret != 1 )
	{
		fclose(fp);
		return -1;
	}

	char *buf = (char *)calloc( (count) * sizeof(INDEX_NODE) , 1); // count-1 -> count;
	 // printf("%s[%d]......count = %d\n", __FUNCTION__, __LINE__, count);

	for( i = 0;  i < count;  i++ )
	{
		if( feof(fp) )
		{
			PRINT_INFO("find %s to end.\n", index_file);
			break;
		}

		memset(&node, 0, sizeof(INDEX_NODE));
		fread(&node, sizeof(INDEX_NODE), 1, fp);
		if( begin == node.start && end == node.stop ||
			(( llabs(begin - node.start) >= 0 && llabs(begin-node.start) <= 5) && ( llabs(end - node.stop) >= 0 && llabs(end - node.stop) <= 5)))
		{
			//printf("......%d,%lld,%lld\n", node.ch_num, node.start,node.stop);
			break;//continue;
		}

		// printf("......%d,%lld,%lld\n", node.ch_num, node.start,node.stop);
		memcpy(buf + i*sizeof(INDEX_NODE),  (char *)&node,  sizeof(INDEX_NODE) );
	}

	if( count == i ) // not find we do nothing
	{
		// printf("%s[%d]......count = %d\n", __FUNCTION__, __LINE__, count);
		fclose(fp);
		free(buf);
		return -2;
	}

	if( count > i+1 )
	{
		size = fread(buf + i * sizeof(INDEX_NODE),  sizeof(INDEX_NODE), count -(i + 1), fp);
	}
	// printf("%s[%d]......count = %d,i = %d,size = %d\n", __FUNCTION__, __LINE__, count, i ,size);
	fclose(fp);
	fp = NULL;

	fp = fopen(index_file, "wb");
	if( !fp )
	{
		// printf("%s[%d]......fopen error: error =%d,%s\n", __FUNCTION__, __LINE__, errno, strerror(errno));
		if( EROFS == errno )
		{
			char sd_cmd3[64],sd_cmd1[64],sd_cmd2[64];
			strncpy(sd_cmd3, "mount -no remount,rw /dev/mmcblk0p1 /mnt/sd_card", sizeof(sd_cmd3));
			ret = gm_system_cmd(sd_cmd3);
		}
		fp = fopen(index_file, "wb");
		if( !fp )
		{
			// printf("%s[%d]......fopen error: error =%d,%s\n", __FUNCTION__, __LINE__, errno, strerror(errno));
			free(buf);
			return -1;
		}
	}

	count--;
	fwrite(&count, sizeof(int), 1, fp);
	size = fwrite(buf, sizeof(INDEX_NODE), count, fp);
	fclose(fp);
	free(buf);

	return 0;
}

/*****************************************************************
param: file; as : "ch0_20150127104512_20150127104542.avi"
return:
	0 :	success;
	-2:	file not exit;
	other: system error;
*****************************************************************/
int search_rm_file_by_name(char *file)
{
	char dirPath[32] = {0}, filePath[64] = {0}, cmd[128];
	int date = 0, ret = -1;

	unsigned long long begin = 	avi_get_start_time(file);
	date = begin/1000000;
	snprintf(dirPath,  32,  "%s/%d", GRD_SD_MOUNT_POINT, date);
	snprintf(filePath, 64, "%s/%s", dirPath, file);

	if( 0 == access(filePath, F_OK ) )
	{
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "rm %s  -f", filePath);
		printf("%s[%d]......%s\n", __FUNCTION__, __LINE__, cmd);
		memset(cmd, 0, sizeof(cmd));
		ret = gm_system_cmd(cmd);
	}
	else
	{
		ret = -2;
	}

	printf("%s[%d]......ret = %d\n", __FUNCTION__, __LINE__, ret);
	return ret;
}


