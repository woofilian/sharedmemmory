#ifndef __GRD_AVI_PB_API_H
#define __GRD_AVI_PB_API_H

#include "avi_typedef.h"


int avi_read_header_and_index(AviPBHandle *pPBHandle);
int avi_pb_open(char *file_path, AviPBHandle *pPBHandle);
int avi_get_frame_loop(AviPBHandle *pPBHandle, char *p_buff, int *p_size);
int avi_pb_close(AviPBHandle *pPBHandle);
int avi_read_data_to_buf(AviPBHandle *pPBHandle);
int avi_pb_mov_index_open(char *file_path, AviPBHandle *pPBHandle);

#ifdef _PLAYBACK_AV_SIMULTANEOUS
int avi_pb_get_frame_AVsimultaneous(int bGetVideo, AviPBHandle *pPBHandle, char *p_buff, int *p_size);
int avi_pb_seek_by_timeoffset_AVSync(int playHandle, float sec_offset);
#endif

/*
return value: 	0 success, -1 param error , -2 frame no is over
*/
int avi_get_frame_timestamp_loop(AviPBHandle *pPBHandle,  struct timeval *ptimeval,  int *frameType);

int avi_pb_mov_index_open(char *file_path, AviPBHandle *pPBHandle);




#endif

