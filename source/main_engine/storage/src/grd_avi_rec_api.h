#ifndef __GRD_AVI_API_H
#define __GRD_AVI_API_H

#include "avi_typedef.h"

#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif

int init_rec_param( AviInitParam *avi_init_parm );
int avi_record_open( AviFile *avi_file);
int avi_record_write(AviFile *avi_file,  char *data, int length, int intra, struct timeval *pTimeval);
int avi_record_close(AviFile *avi_file);
int avi_restore_record_file( void );

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif

