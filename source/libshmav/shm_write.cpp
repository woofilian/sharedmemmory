#include "shm_common.h"
#include "shm_write_class.h"
#include "shm_write.h"
#include "sd_capture.h"
#include <stdlib.h>
#include <pthread.h>
#include <map>

using namespace std;

#define SHM_VIDEO_DEFAULT_MAX_FRAME_SIZE 100*1024//102400 //default for 100k, need to confirm.
#define SHM_AUDIO_DEFAULT_MAX_FRAME_SIZE 4096   //default for 4k, need to confirm.

#define SHM_VIDEO_DEFAULT_SHM_SIZE 3670016// 3.5M//1048576 //1//1M=1024*1024  
#define SHM_AUDIO_DEFAULT_SHM_SIZE 409600 // 400K



typedef int (*FuncCapture)(char cProfileNO, char *pcBuf, int *piBufLen);

typedef struct 
{
	char cProfileNO;
	ShmWrite* pclassShmWrite;
	FuncCapture funcCapture;
}StructCaptureWrite;

map<char, ShmWrite*> g_mapShmVideoWrite;
map<char, ShmWrite*> g_mapShmAudioWrite;

map<char, FuncCapture> g_mapShmVideoCapture;
map<char, FuncCapture> g_mapShmAudioCapture;


/*****************************************************************************************************
** DESCRIPTION: write a frame to the sharememory assigned by cProfileNO and pmapShmWrite
**    
**
** Parameters:
**     cProfileNO: a unique number of a stream 
**     pcBuf      : char*, frame data.
**     iBufLen      : int, frame size
**     pcExtras   : char*, 16 bytes extras infos about this frame(user defined info) (key frame, resolution.... defined by the caller)
**     pmapShmWrite: map<char, ShmWrite*> *, g_mapShmVideoWrite, g_mapShmAudioWrite
**
** Returns:
**     -1, error
**      0, success
**/
static int SHM_WriteFrame(char cProfileNO, char* pcBuf, int iBufLen, char *pcExtras, map<char, ShmWrite*> *pmapShmWrite)
{
	map<char, ShmWrite*>::iterator itr;
	ShmWrite* pclassShmWrite;
	int rtn=-1;

	itr = pmapShmWrite->find(cProfileNO);
	if(itr != pmapShmWrite->end())
	{
		pclassShmWrite = itr->second;
		rtn = pclassShmWrite->WriteFrame(pcBuf, iBufLen, pcExtras);
	}
	else
	{
		SHM_DEBUG("can not find cProfileNO=%d\n", cProfileNO);
	}

	return  rtn;
}


int SHM_GetHeaderInfoWithProfileNO(char cProfileNO, char pcHeaderInfo[32], map<char, ShmWrite*> *pmapShmWrite)
{
    map<char, ShmWrite*>::iterator itr;
	ShmWrite* pclassShmWrite;
    int rtn = SHM_ENUM_ERROR;

	itr = pmapShmWrite->find(cProfileNO);
	if(itr != pmapShmWrite->end())
	{
		pclassShmWrite = itr->second;
		pclassShmWrite->GetHeaderInfo(pcHeaderInfo);
        rtn = SHM_ENUM_OK;
	}

    return rtn;
}


int SHM_SetHeaderInfoWithProfileNO(char cProfileNO, char pcHeaderInfo[32], map<char, ShmWrite*> *pmapShmWrite)
{
    map<char, ShmWrite*>::iterator itr;
	ShmWrite* pclassShmWrite;
    int rtn = SHM_ENUM_ERROR;

	itr = pmapShmWrite->find(cProfileNO);
	if(itr != pmapShmWrite->end())
	{
		pclassShmWrite = itr->second;
		pclassShmWrite->SetHeaderInfo(pcHeaderInfo);
        rtn = SHM_ENUM_OK;
	}

    return rtn;
}

static void SHM_DetachWrite(char cProfileNO, map<char, ShmWrite*> *pmapShmWrite)
{
	map<char, ShmWrite*>::iterator itr;
	ShmWrite* pclassShmWrite;

	itr = pmapShmWrite->find(cProfileNO);
	if(itr != pmapShmWrite->end())
	{
		pclassShmWrite = itr->second;
		pmapShmWrite->erase(itr);
		delete(pclassShmWrite);
	}
}

//return: 0 success, -1 error 
int SHM_VideoInitWrite(char cProfileNO, int iShmSize, int iMaxFrameSize, ShmVideoHeaderInfo *pstructShmHeaderInfo)
{
	ShmWrite* pclassShmWrite;
	char pcHeaderInfo[32];
	key_t shmKey;
	key_t semKey;
	
	if (g_mapShmVideoWrite.find(cProfileNO) == g_mapShmVideoWrite.end())
	{		
		shmKey = SHM_VIDEO_DEVICE_BASE_KEY + ((int)cProfileNO)*SHM_KEY_MULTI_BASE;
		semKey = SEM_VIDEO_DEVICE_BASE_KEY + ((int)cProfileNO)*SEM_KEY_MULTI_BASE;
		
		memcpy(pcHeaderInfo, pstructShmHeaderInfo, sizeof(pcHeaderInfo));
		
		if (iShmSize == 0)
		{
			iShmSize = SHM_VIDEO_DEFAULT_SHM_SIZE;
		}

		if (iMaxFrameSize == 0)
		{
			iMaxFrameSize = SHM_VIDEO_DEFAULT_MAX_FRAME_SIZE;
		}

        printf("Line=%d, media_type=%d, codec=%d, res=%d, key1=%d, key2=%d, iShmSize=%d, iMaxFrameSize=%d\n", __LINE__, pcHeaderInfo[0], pcHeaderInfo[1], pcHeaderInfo[2], shmKey, semKey, iShmSize, iMaxFrameSize);

		pclassShmWrite = new ShmWrite(shmKey, semKey, iShmSize, iMaxFrameSize, pcHeaderInfo);		
		if(pclassShmWrite->GetObjectStatus()== -1)
		{			
			SHM_DEBUG("error happened while create video ShmWrite.....\n");
			delete pclassShmWrite;
			return SHM_ENUM_ERROR;
		}
		else
		{
			SHM_DEBUG("create video ShmWrite for profile=%d success.....\n", cProfileNO);
			g_mapShmVideoWrite.insert(pair<char, ShmWrite*>(cProfileNO, pclassShmWrite));
		}			
	}
	else
	{
		SHM_DEBUG("The cProfileNO=%d for VideoWrite  aready exist\n", cProfileNO);
	}
	return SHM_ENUM_OK;
}


/*****************************************************************************************************
** DESCRIPTION: write a video frame to the sharememory assigned by cProfileNO
**    
**
** Parameters:
**     cProfileNO   : a unique number of a stream (1/2) 
**     pcBuf         : char*, frame data.
**     iBufLen         : int, frame size
**     iKeyFrameFlag: int, key frame flag (1: key frame, 0: normal frame), still haven't use
**     pcExtras      : char*, 16 bytes extras infos about this frame(user defined info) (key frame, resolution.... defined by the caller)
** Returns:
**     -1, error
**      0, success
**/

//-1 error, 0 succuss
int SHM_VideoWriteFrame(char cProfileNO, char* pcBuf, int iBufLen, ShmVideoExtraData* pstructShmExtras)
{
	static int vwr[4] = {0,0,0,0};
	if(cProfileNO<4 && cProfileNO>0 && vwr[cProfileNO]==0)
	{
		vwr[cProfileNO] = 1;
		SHM_DEBUG("write first frame of Video cProfileNO=%d, iBufLen=%d\n", cProfileNO, iBufLen);
	}

	return SHM_WriteFrame(cProfileNO, pcBuf, iBufLen, (char *)pstructShmExtras, &g_mapShmVideoWrite);
}

void SHM_VideoDetachWrite(char cProfileNO)
{
	SHM_DetachWrite(cProfileNO, &g_mapShmVideoWrite);
}

int SHM_VideoSetHeaderInfoWithProfileNO(char cProfileNO, char pcHeaderInfo[32])
{
    return SHM_SetHeaderInfoWithProfileNO(cProfileNO, pcHeaderInfo, &g_mapShmVideoWrite);
}

int SHM_VideoGetHeaderInfoWithProfileNO(char cProfileNO, char pcHeaderInfo[32])
{
    return SHM_GetHeaderInfoWithProfileNO(cProfileNO, pcHeaderInfo, &g_mapShmVideoWrite);
}

int SHM_AudioSetHeaderInfoWithProfileNO(char cProfileNO, char pcHeaderInfo[32])
{
    return SHM_SetHeaderInfoWithProfileNO(cProfileNO, pcHeaderInfo, &g_mapShmAudioWrite);
}

int SHM_AudioGetHeaderInfoWithProfileNO(char cProfileNO, char pcHeaderInfo[32])
{
    return SHM_GetHeaderInfoWithProfileNO(cProfileNO, pcHeaderInfo, &g_mapShmAudioWrite);
}

int SHM_AudioInitWrite(char cProfileNO, int iShmSize, int iMaxFrameSize, ShmAudioHeaderInfo *pstructShmHeaderInfo)
{
	ShmWrite* pclassShmWrite;
	char pcHeaderInfo[32];
	key_t shmKey;
	key_t semKey;
	
	if (g_mapShmAudioWrite.find(cProfileNO) == g_mapShmAudioWrite.end())
	{		
		shmKey = SHM_AUDIO_DEVICE_BASE_KEY + ((int)cProfileNO)*SHM_KEY_MULTI_BASE;
		semKey = SEM_AUDIO_DEVICE_BASE_KEY + ((int)cProfileNO)*SEM_KEY_MULTI_BASE;
		
		memcpy(pcHeaderInfo, pstructShmHeaderInfo, sizeof(pcHeaderInfo));
		

		if (iShmSize == 0)
		{
			iShmSize = SHM_AUDIO_DEFAULT_SHM_SIZE;
		}

		if (iMaxFrameSize == 0)
		{
			iMaxFrameSize = SHM_AUDIO_DEFAULT_MAX_FRAME_SIZE;
		}

		printf("Line=%d, media_type=%d, codec=%d, key1=%d, key2=%d, iShmSize=%d, iMaxFrameSize=%d\n", __LINE__, pcHeaderInfo[0], pcHeaderInfo[1], shmKey, semKey, iShmSize, iMaxFrameSize);

		pclassShmWrite = new ShmWrite(shmKey, semKey, iShmSize, iMaxFrameSize, pcHeaderInfo);		
		if(pclassShmWrite->GetObjectStatus()== -1)
		{			
			SHM_DEBUG("error happened while create video ShmWrite.....\n");
			delete pclassShmWrite;
			return SHM_ENUM_ERROR;
		}
		else
		{
			SHM_DEBUG("create Audio ShmWrite for profile=%d success.....\n", cProfileNO);
			g_mapShmAudioWrite.insert(pair<char, ShmWrite*>(cProfileNO, pclassShmWrite));
		}	
	}
	else
	{
		SHM_DEBUG("The cProfileNO=%d for AudioWrite aready exist\n", cProfileNO);
	}
	return SHM_ENUM_OK;
}


//-1 error, 0 succuss
int SHM_AudioWriteFrame(char cProfileNO, char* pcBuf, int iBufLen, ShmAudioExtraData* pstructShmAudioExtras)
{	
	static int awr[4]= {0,0,0,0};
	if(cProfileNO<4&&cProfileNO>0&& awr[cProfileNO]==0)
	{
		awr[cProfileNO]=1;
		SHM_DEBUG("write first frame of audio cProfileNO=%d, iBufLen=%d\n", cProfileNO, iBufLen);
	}
	return SHM_WriteFrame(cProfileNO, pcBuf, iBufLen, (char*)pstructShmAudioExtras, &g_mapShmAudioWrite);
}

void  SHM_AudioDetachWrite(char cProfileNO)
{
	SHM_DetachWrite(cProfileNO, &g_mapShmAudioWrite);
}


//static char g_siVideoCaptureThreadRunning = 0; 
//static char g_siAudioCaptureThreadRunning = 0; 
static void* SHM_CaptureThread(void *args)
{
	StructCaptureWrite *structCaptureWrite = (StructCaptureWrite *)args;
	char* pcBuf = (char *) malloc(structCaptureWrite->pclassShmWrite->GetMaxFrameSize()); 
	int iBufSize;
	char carrExtras[sizeof(ShmVideoExtraData)];

	printf("222 start CaptureThread....... max_frame_size=%d, %x\n", structCaptureWrite->pclassShmWrite->GetMaxFrameSize(), structCaptureWrite->funcCapture);
	while(1)
	{
		memset(carrExtras, -1, sizeof(carrExtras));
		structCaptureWrite->funcCapture(structCaptureWrite->cProfileNO, pcBuf, &iBufSize);
		
		ShmVideoExtraData *structShmVideoExtras=(ShmVideoExtraData *)carrExtras;
		if(iBufSize>0)
		{
			if(pcBuf[5]==0x10)
			{
				structShmVideoExtras->ucKeyFrameFlag = SHM_ENUM_VIDEO_FRAME_TYPE_KEY; //keyframe
				printf("write I frame.....%02x%02x len=%d\n", (unsigned char)pcBuf[0], (unsigned char)pcBuf[1], iBufSize);
			}
			else 
			{
				structShmVideoExtras->ucKeyFrameFlag = SHM_ENUM_VIDEO_FRAME_TYPE_NORMAL; //normal frame
			}
		//printf("write frame.....%02x%02x len=%d\n", (unsigned char)pcBuf[0], (unsigned char)pcBuf[1], iBufSize);
		
			//printf("Frame Header:");
			//for(int i=0; i<30; i++) printf("%02x ", pcBuf[i]);printf("\n");
			//printf("write frame.....%02x%02x len=%d\n", (unsigned char)pcBuf[0], (unsigned char)pcBuf[1], iBufSize);
			structCaptureWrite->pclassShmWrite->WriteFrame(pcBuf, iBufSize, carrExtras);
		}
	}

	free(pcBuf);
}

int  SHM_VideoInitCapture(char cProfileNO, int iShmSize, int iMaxFrameSize, 
	ShmVideoHeaderInfo *pstructShmHeaderInfo, 
	int (*funcCapture)(char cProfileNO, char *pcBuf, int *piBufLen))
{
	if(SHM_VideoInitWrite(cProfileNO, iShmSize, iMaxFrameSize, pstructShmHeaderInfo) == 0)
	{
		//g_mapShmVideoCapture.insert(pair<char, FuncCapture>(cProfileNO, funcCapture));
		pthread_t shmCaptureID;
		StructCaptureWrite* structCaptureWrite = (StructCaptureWrite* )malloc(sizeof(StructCaptureWrite));
		structCaptureWrite->cProfileNO = cProfileNO;
		structCaptureWrite->funcCapture = funcCapture;
		structCaptureWrite->pclassShmWrite = g_mapShmVideoWrite.find(cProfileNO)->second;

		printf("111 create CaptureThread....... max_frame_size=%d, %x\n", structCaptureWrite->pclassShmWrite->GetMaxFrameSize(), structCaptureWrite->funcCapture);
		pthread_create(&shmCaptureID, NULL, SHM_CaptureThread, (void *)structCaptureWrite);

		return 0;
	}
	else
	{
		return -1;	
	}
}

int  VideoInitCapture(char cProfileNO, int iMaxFrameSize, 
	VideoHeaderInfo *pstructHeaderInfo, 
	int (*funcCapture)(char cProfileNO, char *pcBuf, int *piBufLen))
{
	return SHM_VideoInitCapture(cProfileNO, SHM_VIDEO_DEFAULT_SHM_SIZE, iMaxFrameSize, (ShmVideoHeaderInfo *)pstructHeaderInfo, funcCapture);
}

int  SHM_AudioInitCapture(char cProfileNO, int iShmSize, int iMaxFrameSize, 
	ShmAudioHeaderInfo *pstructShmHeaderInfo, 
	int (*funcCapture)(char cProfileNO, char *pcBuf, int *piBufLen))
{
	if(SHM_AudioInitWrite( cProfileNO, iShmSize, iMaxFrameSize, pstructShmHeaderInfo) == 0)
	{
		//g_mapShmAudioCapture.insert(pair<char, FuncCapture>(cProfileNO, funcCapture));
		pthread_t shmCaptureID;
		StructCaptureWrite* structCaptureWrite = (StructCaptureWrite* )malloc(sizeof(StructCaptureWrite));
		structCaptureWrite->cProfileNO = cProfileNO;
		structCaptureWrite->funcCapture = funcCapture;
		structCaptureWrite->pclassShmWrite = g_mapShmAudioWrite.find(cProfileNO)->second;
		pthread_create(&shmCaptureID, NULL, SHM_CaptureThread, (void *)&structCaptureWrite);
		return 0;
	}
	else
	{
		return -1;	
	}
}

int  AudioInitCapture(char cProfileNO, int iMaxFrameSize, 
	AudioHeaderInfo *pstructHeaderInfo, 
	int (*funcCapture)(char cProfileNO, char *pcBuf, int *piBufLen))
{
	return SHM_AudioInitCapture(cProfileNO, SHM_AUDIO_DEFAULT_SHM_SIZE, iMaxFrameSize, (ShmAudioHeaderInfo *)pstructHeaderInfo, funcCapture);
}


