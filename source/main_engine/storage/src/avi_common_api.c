//#include "avi_common.h"
#include "avi_api.h"
#include "record.h"
//#include "AvObj.h"

extern record_work_t g_record_work_t;


int write_char(FILE *file, char c)
{
    return fwrite(&c, sizeof(char), 1, file);
}
int write_int32(FILE *file, int i)
{
    return fwrite(&i, sizeof(int), 1, file);
}
int write_fourcc(FILE *file, int fourcc)
{
    return fwrite(&fourcc, sizeof(char)*4, 1, file);
}
int write_data(FILE *file, char *data, int size)
{
    return fwrite(data, size, 1, file);
}

int read_char(FILE *file, char *c)
{
    return fread(c, sizeof(char), 1, file);
}
int read_int32(FILE *file, int *i)
{
    return fread(i, sizeof(int), 1, file);
}
int read_fourcc(FILE *file, int *fourcc)
{
    return fread(fourcc, sizeof(char)*4, 1, file);
}
int read_data(FILE *file, unsigned char *data, int size)
{
    return fread(data, size, 1, file);
}



void avi_set_32(void *pp, int w) 
{ 
    char *p = pp; 

    p[0] = ( w      )&0xff; 
    p[1] = ( w >> 8 )&0xff; 
    p[2] = ( w >> 16)&0xff; 
    p[3] = ( w >> 24)&0xff; 
} 

/**********************************************************************
函数描述：从字符串中截取字符串
入口参数：char *dest: 目标字符串
          char *src: 源字符串
          int start: 截取的起始下标位置，0开始
          int end: 截取的结束下标位置，end - start就是截取的字符串长度
返回值：  无    
**********************************************************************/
void avi_substring(char *dest, char *src, int start, int end)  
{  
    int i=start;  
    if(start>strlen(src))
        return;  
    if(end>strlen(src))  
        end=strlen(src);  
    while(i<end)  
    {     
        dest[i-start]=src[i];  
        i++;  
    }  
    dest[i-start]='\0';  
    return;  
}   

/**********************************************************************
函数描述：得到字符串中某字符的下标位置
入口参数：char *src: 源字符串
          char c: 查找的字符
返回值：  int: 找到则返回该字符的下标位置，如果没有该字符，则返回-1    
**********************************************************************/
int last_index_at(char *str, char c)
{
    int n;
    n = strlen(str);

    while(n--)
    {
        if(c == str[n-1])
            return n-1 ;
    }
    return -1;
}


/**********************************************************************
函数描述：从avi文件名得到通道号
入口参数：char *file_name: avi文件名(不带路径名)
返回值：  unsigned long: 通道号       
**********************************************************************/
unsigned long avi_get_ch_num(char *file_name)
{
    unsigned long tmp;
    tmp = (unsigned long)(file_name[2] - 48);
    return tmp;
}

/**********************************************************************
函数描述：从avi文件名得到起始时间
入口参数：char *file_name: avi文件名(不带路径名)
返回值：  unsigned long long: 起始时间       
**********************************************************************/
unsigned long long avi_get_start_time(char *file_name)
{
    char tmp_str[20];
    unsigned long long tmp64;
    avi_substring(tmp_str, file_name, 4, 4+14);
    tmp64 = (unsigned long long)atoll(tmp_str);
    return tmp64;
}

/**********************************************************************
函数描述：从avi文件名得到结束时间
入口参数：char *file_name: avi文件名(不带路径名)
返回值：  unsigned long long: 结束时间       
**********************************************************************/
unsigned long long avi_get_stop_time(char *file_name)
{
    char tmp_str[20];
    unsigned long long tmp64;
    avi_substring(tmp_str, file_name, 19, 19+14);
    tmp64 = (unsigned long long)atoll(tmp_str);
    return tmp64;    
}

/**********************************************************************
函数描述：得到当前日期的字符串格式
入口参数：char *str: 当前日期的字符串格式
返回值：  无        
**********************************************************************/
void get_date_str(char *str)
{
    struct tm *ptm; 
    long ts; 

    ts = time(NULL); 
    struct tm tt = {0}; 
    ptm = localtime_r(&ts, &tt); 

	sprintf(str, "%04d%02d%02d", ptm->tm_year+1900,	
								 ptm->tm_mon+1,
								 ptm->tm_mday);    

    return;
}

/**********************************************************************
函数描述：得到当前时间的字符串格式
入口参数：char *str: 当前时间的字符串格式
返回值：  无        
**********************************************************************/
void get_nowtime_str(char *str)
{
    struct tm *ptm; 
    time_t ts;
	int i = 0;
   

	 time_t lt;
	 time(&lt);
	 //lt = lt+(g_record_work_t.record_time_zone_pre - 12)*3600;//获取时间后 对应处理时区的时间
	 //if( lt < 0 ) lt = 0;
	 
	 ptm = localtime(&lt);//获取当前时间

	
	sprintf(str, "%04d%02d%02d%02d%02d%02d", ptm->tm_year+1900,	
											 ptm->tm_mon+1,
											 ptm->tm_mday,
											 ptm->tm_hour ,
											 ptm->tm_min,							
											 ptm->tm_sec);    
    printf(">>>>>>>>>>get_nowtime_str=====%s\n",str);
    return;
}



#if 0
/**********************************************************************
函数描述：根据参数打印出错误提示.
入口参数：int ret: avi_record_open,avi_record_write,avi_record_close
                   的返回值作为参数
返回值：空
**********************************************************************/
void avi_print_err(int ret)
{
    switch(ret)
    {
        case STS_OPEN_FAILED:       PRINT_INFO("open the file failed. \n"); break;
        case STS_INVALID_INPUT:     PRINT_INFO("invalid input. \n"); break;
        case STS_MKDIR_ERROR:       PRINT_INFO("mkdir error. \n"); break;    
        case STS_INVALID_FORAMT:    PRINT_INFO("invalid format. \n"); break;
        case STS_MALLOC_FAILED:     PRINT_INFO("malloc error. \n"); break;  
        case STS_FTRUNCATE_FAILED:  PRINT_INFO("ftruncate file failed. \n"); break;
        case STS_RENAME_FAILED:     PRINT_INFO("rename error. \n"); break;   
        case STS_POLL_FAILED:       PRINT_INFO("poll error. \n"); break; 
        
        case STS_RECORD_MODE_ERR:   PRINT_INFO("record mode error. \n"); break;
        case STS_WRITE_FAILED:      PRINT_INFO("write failed. \n"); break;  
        case STS_READ_FAILED:       PRINT_INFO("read failed. \n"); break;
        case STS_SDCARD_NOT_MOUNT:  PRINT_INFO("sd card is not mounted. \n"); break;   
        case STS_SDCARD_NO_SPACE:   PRINT_INFO("sd card don't have enough space. \n"); break;        
        case STS_FISTFRAME_NOT_KEY: PRINT_INFO("the fist frame is not key frame. \n"); break;

        case STS_INDEX_COUNT_ERR:   PRINT_INFO("the avi index count malloc error. \n"); break;
        case STS_DOUBLE_CLOSE:      PRINT_INFO("double call close function. \n"); break;
        default: PRINT_INFO(" error. \n"); break;
    }
}
#endif

/**********************************************************************
函数描述：时间的tm格式转换成时间的GRD_TIME格式
入口参数：struct tm *src: 源操作数
          sdk_time_t *dst: 目标操作数
返回值：  无
**********************************************************************/
void tm_to_dmstime(sdk_time_t *dst, struct tm *src)
{
	dst->year = src->tm_year;
	dst->mon = src->tm_mon;
	dst->day =  src->tm_mday;
	dst->hour = src->tm_hour;
	dst->min =src->tm_min;
	dst->sec = src->tm_sec;
}

/**********************************************************************
函数描述：时间的tm格式加上秒数，得到时间的tm格式
入口参数：struct tm *dst: 目标操作数
          struct tm *src: 第一个操作数
          int seconds: 第二个操作数
返回值：  无
**********************************************************************/
void tm_add_seconds(struct tm *dst, struct tm *src, int time_seconds)
{
	time_t timep;
    struct tm t1 = {0}; 
	struct tm *p = &t1;  

	p->tm_year = src->tm_year - 1900;
	p->tm_mon = src->tm_mon - 1;
	p->tm_mday = src->tm_mday;
	p->tm_hour = src->tm_hour;
	p->tm_min = src->tm_min;
	p->tm_sec = src->tm_sec; 


	timep = mktime(p);

	timep += time_seconds;
    memset(&t1, 0, sizeof(struct tm));
	p = localtime_r(&timep, &t1);
	
	dst->tm_year = p->tm_year + 1900;
	dst->tm_mon = p->tm_mon + 1;
	dst->tm_mday =  p->tm_mday;
	dst->tm_hour = p->tm_hour;
	dst->tm_min =p->tm_min;
	dst->tm_sec = p->tm_sec;

}

/**********************************************************************
函数描述：时间的u64t格式加上秒数，返回相加后的值
入口参数：u64t opr1: 第一个操作数
          int seconds: 第二个操作数
返回值：  uint64_t: 相加后的值
**********************************************************************/
uint64_t u64t_add_seconds(uint64_t opr1, int seconds)
{
    sdk_time_t src;
    u64t_to_time(&src, opr1);

    time_t timep;
	struct tm *p; 
    
	time(&timep);

    struct tm t1 = {0};    
    p = localtime_r(&timep, &t1);    
    
	p->tm_year = src.year- 1900;
	p->tm_mon = src.mon- 1;
	p->tm_mday = src.day;
	p->tm_hour = src.hour;
	p->tm_min = src.min;
	p->tm_sec = src.sec; 

	timep = mktime(p);
	timep += seconds;
    memset(&t1, 0, sizeof(struct tm));
	p = localtime_r(&timep, &t1); 

    sdk_time_t dst;    
	dst.year = p->tm_year + 1900;
	dst.mon = p->tm_mon + 1;
	dst.day =  p->tm_mday;
	dst.hour = p->tm_hour;
	dst.min = p->tm_min;
	dst.sec = p->tm_sec;    

    uint64_t opr2 = time_to_u64t(&dst);

    return opr2;
}

