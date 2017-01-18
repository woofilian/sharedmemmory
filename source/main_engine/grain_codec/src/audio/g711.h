/******************************************************************************

                  版权所有 (C), 2012-2022, bingchuan

 ******************************************************************************
  文 件 名   : g711.h
  版 本 号   : v1.0
  作    者   : bingchuan
  生成日期   : 2015年3月28日
  功能描述   : g711.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2015年3月28日
    作    者   : bingchuan
    修改内容   : 创建文件

******************************************************************************/


#ifndef __G711_H__
#define __G711_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


extern int G711ADec(unsigned char* pSrc, short* pDst, int len);
extern int G711AEnc(short* pSrc, unsigned char* pDst, int len);
extern int G711UDec(unsigned char* pSrc, short* pDst, int len);
extern int G711UEnc(short* pSrc, unsigned char* pDst, int len);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __G711_H__ */
