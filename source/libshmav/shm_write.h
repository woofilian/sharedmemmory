#ifndef __SHM_WRITE_H__
#define __SHM_WRITE_H__

#include "shm_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************************
**
**
*/
#define SHM_AUDIO_16K_SIZE 409600  //400k
#define SHM_AUDIO_8K_SIZE  102400  //100k

int  SHM_VideoInitCapture(char cProfileNO, int iShmSize, int iMaxFrameSize, 
	ShmVideoHeaderInfo *pstructShmHeaderInfo, 
	int (*funcCapture)(char cProfileNO, char *pcBuf, int *piBufLen));

int  SHM_VideoInitWrite(char cProfileNO, int iShmSize, int iMaxFrameSize, ShmVideoHeaderInfo *pstructShmHeaderInfo); 
int  SHM_VideoWriteFrame(char cProfileNO, char* pcBuf, int iBufLen, ShmVideoExtraData* pstructShmVideoExtras);
void SHM_VideoDetachWrite(char cProfileNO);

int  SHM_AudioInitCapture(char cProfileNO, int iShmSize, int iMaxFrameSize, 
	ShmAudioHeaderInfo *pstructShmHeaderInfo, 
	int (*funcCapture)(char cProfileNO, char *pcBuf, int *piBufLen));

int  SHM_AudioInitWrite(char cProfileNO, int iShmSize, int iMaxFrameSize, ShmAudioHeaderInfo *pstructShmHeaderInfo);
int  SHM_AudioWriteFrame(char cProfileNO, char* pcBuf, int iBufLen, ShmAudioExtraData* pstructShmAudioExtras);
void SHM_AudioDetachWrite(char cProfileNO);


int SHM_VideoSetHeaderInfoWithProfileNO(char cProfileNO, char pcHeaderInfo[32]);
int SHM_VideoGetHeaderInfoWithProfileNO(char cProfileNO, char pcHeaderInfo[32]);
int SHM_AudioSetHeaderInfoWithProfileNO(char cProfileNO, char pcHeaderInfo[32]);
int SHM_AudioGetHeaderInfoWithProfileNO(char cProfileNO, char pcHeaderInfo[32]);

#ifdef __cplusplus
}
#endif


#endif /*__SHM_WRITE_H__*/
