#ifndef __CODEC_TYPE_H__
#define __CODEC_TYPE_H__

/*
*做这个得目的是为了 木块的独立性  可以不依赖外部结构体运行
*
*/
#define MAX_CAP_CH_NUM          1 //最大的通道数  每个通道里面还有子通道
#define CODEC_MAX_STREAM		3 //子码流的个数
#define CODEC_MOTION_NUM          4       //每个通道最大移动侦测区域个数

typedef struct codec_rect_s{
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
}codec_rect_t;

typedef struct _motion_cfg_s
{
    uint8_t enable;                                             //允许联动
    uint8_t sensitive;                                          //灵敏度(0[灵敏度最高]----6[最低])
    uint8_t res[2];
    uint32_t mask;												//按位
    codec_rect_t area[CODEC_MOTION_NUM];                            //区域
}codec_motion_cfg_t;


#endif
