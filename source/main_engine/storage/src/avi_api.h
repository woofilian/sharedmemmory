#ifndef __GRD_AVI_API_H__
#define __GRD_AVI_API_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <string.h>
#include "avi_typedef.h"
#include "avi_common.h"



/**********************************************************************
函数描述：检查SD卡存储空间，创建文件，写AVI头信息
入口参数：AviFile *avi_file, AVI文件的控制句柄
          AviInitParam *param, 传入的参数信息
返回值： 0: 正常返回
        <0: 错误返回         
**********************************************************************/
extern int avi_record_open(AviFile *avi_file);


/**********************************************************************
函数描述：向AVI文件中写入一帧数据
入口参数：AviFile *avi_file:  AVI文件的控制句柄
          AviInitParam *param, 传入的参数信息
          unsigned char *data:  写入的数据的地址
          int length: 写入的数据的长度
          int intra: 1,表示为关键桢数据;0表示不为关键桢数据
          
返回值： 0: 正常返回
        <0: 错误返回
**********************************************************************/
extern int avi_record_write(AviFile *avi_file,
                                char *data, 
                                int length, 
                                int intra,
                                struct timeval *pTimeval);


/**********************************************************************
函数描述：重命名AVI文件为最终格式，并关闭文件，释放内存
入口参数：AviFile *avi_file:  AVI文件的控制句柄         
返回值： 0: 正常返回
        <0: 错误返回       
**********************************************************************/
extern  int avi_record_close(AviFile *avi_file);


/**********************************************************************
回放部分接口函数使用描述：
1，调用 avi_pb_open 打开要播放的avi文件,文件名为file_name。
2，调用 avi_pb_seek_by_percent 或者 avi_pb_seek_by_time 定位播放起始位置，
   如果不使用这2个函数，则默认从头开始播放。
3，调用 set_frame_info 去设置 avi帧数据控制句柄 AviFrameInfo
4，调用 avi_pb_set_speed 去设置播放速度，不使用则播放速度默认为1倍速。
5，循环调用 avi_pb_get_frame 去得到每一帧数据。返回值为1003则播放到了末尾。
6，调用 avi_pb_close 结束。
注意: 当开启一个线程进行回放时，对回放速度的操作(avi_pb_set_speed)是
在另一个线程中(一般是主线程)，2个线程对 avi_pb_speed 的操作是互斥的。
**********************************************************************/
/**********************************************************************
函数描述：通过读AVI文件信息，得到他的控制句柄和参数信息
入口参数：char *file_name: 要打开的文件路径名    
          AviFile *avi_file: avi文件的控制句柄
          AviInitParam *param: avi文件中的参数信息
返回值：  <0: 错误
           0: 正常        
**********************************************************************/
extern int avi_pb_open(char *file_path, AviPBHandle *pPBHandle);

/**********************************************************************
函数描述：通过AVI文件的控制句柄和参数信息, 计算出该百分比位置是第几帧
          数据，并更新到 AviFrameInfo 的 no 参数中
入口参数：AviFile *avi_file: avi文件的控制句柄
          AviInitParam *param: avi文件中的参数信息
          int percent: 百分比，如25%，percent = 25
          AviFrameInfo *p_frame: avi文件中帧数据的控制句柄
返回值：  <0: 错误
           0: 正常        
**********************************************************************/
extern int avi_pb_seek_by_percent(AviFile *avi_file, AviInitParam *param, int percent, AviPBHandle *pPBHandle);

/**********************************************************************
函数描述：通过AVI文件的控制句柄和参数信息, 计算出该定位时间是第几帧
          数据，并更新到 AviFrameInfo 的 no 参数中
入口参数：AviFile *avi_file: avi文件的控制句柄
          AviInitParam *param: avi文件中的参数信息
          char *seek_time: 通过时间在进度条中定位
          AviFrameInfo *p_frame: avi文件中帧数据的控制句柄
返回值：  <0: 错误
           0: 正常        
**********************************************************************/
extern int avi_pb_seek_by_time(AviFile *avi_file, AviInitParam *param, char *seek_time, AviPBHandle *pPBHandle);

/**********************************************************************
函数描述：通过AVI文件的控制句柄和参数信息，来设置帧数据的控制句柄
入口参数：AviFile *avi_file: avi文件的控制句柄
          AviInitParam *param: avi文件中的参数信息
          AviFrameInfo *p_frame: avi文件中帧数据的控制句柄
返回值：  无        
**********************************************************************/
//extern void set_frame_info(AviFile *avi_file, AviInitParam *param, AviFrameInfo *p_frame);

/**********************************************************************
函数描述：打印出avi文件中帧数据所在的avi文件中的信息
入口参数：AviPBHandle *pPBHandle: avi文件中帧数据的控制句柄
返回值：  无        
**********************************************************************/
extern void print_pb_handle(AviPBHandle *pPBHandle);

/**********************************************************************
函数描述：打印出avi文件中帧数据的信息
入口参数：AviPBHandle *pPBHandle: avi文件中帧数据的控制句柄
返回值：  无        
**********************************************************************/
extern void print_frame_node(AviPBHandle *pPBHandle);

/**********************************************************************
函数描述：通过AVI文件的控制句柄和参数信息, 计算出该定位时间是第几帧
          数据，并更新到 AviFrameInfo 的 no 参数中
入口参数：AviFrameInfo *p_frame: avi文件中帧数据的控制句柄
          char *pBuff: 向该缓存区位置指针存入数据
          int *pSize: 得到这帧数据的大小
返回值：  <0: 错误
           0: 正常  
          GRD_NET_NOMOREFILE: 播放结束(值为1003) 
**********************************************************************/
extern int avi_pb_get_frame(AviPBHandle *pPBHandle, char *pBuff, int *pSize);

/**********************************************************************
函数描述：设置avi文件播放的速度，目前只支持快进
入口参数：int speed: 播放速度，如 1,2,4,8,16,32
返回值：  <0: 错误
           0: 正常        
**********************************************************************/
extern int avi_pb_set_speed(int speed);

/**********************************************************************
函数描述：关闭帧数据的控制句柄
入口参数：AviFrameInfo *p_frame: avi文件中一帧数据的控制句柄
返回值：  <0: 错误
           0: 正常        
**********************************************************************/
extern int avi_pb_close(AviPBHandle *pPBHandle);


/**********************************************************************
函数描述：以下6函数只针对安普对接      
**********************************************************************/
extern int avi_pb_start(void);
extern int avi_pb_restart(AviPBHandle *pPBHandle);
extern int avi_pb_pause(void);
extern int avi_pb_get_pos(AviPBHandle *pPBHandle);
extern int avi_pb_set_pos(AviPBHandle *pPBHandle, int *pTimePos);
extern int avi_pb_get_time(AviPBHandle *pPBHandle,  struct tm *p_nowtime);
extern int avi_get_frame_loop(AviPBHandle *pPBHandle, char *p_buff, int *p_size);


/**********************************************************************
函数描述：修复sd卡中所有的未录完成的avi文件        
入口参数：无          
返回值：  无       
**********************************************************************/
extern void fix_all_avi_file(void);


/**********************************************************************
函数描述：根据参数打印出错误提示.
入口参数：int ret: avi_record_open,avi_record_write,avi_record_close
                   的返回值作为参数
返回值：空
**********************************************************************/
//extern  void avi_print_err(int ret);

#ifdef __cplusplus
}
#endif

#endif /* __GRD_AVI_API_H__ */

