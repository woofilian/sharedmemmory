#include "shm_common.h"
#include "shm_read.h"
#include "shm_read_class.h"

#include <pthread.h>
#include <sys/time.h>

#include <map>
using namespace std;

#define LOCK_SHM(enumMType)                             \
do{													\
	if(enumMType==SHM_ENUM_MEDIA_TYPE_VIDEO) 				\
		pthread_mutex_lock(&g_mutexShmVideo);		\
	else if(enumMType==SHM_ENUM_MEDIA_TYPE_AUDIO)			\
		pthread_mutex_lock(&g_mutexShmAudio);		\
	else											\
		SHM_DEBUG("LOCK_MEDIA: enumMType error!\n");		\
}while(0)

#define UNLOCK_SHM(enumMType)							\
do{													\
	if(enumMType==SHM_ENUM_MEDIA_TYPE_VIDEO) 				\
		pthread_mutex_unlock(&g_mutexShmVideo);	\
	else if(enumMType==SHM_ENUM_MEDIA_TYPE_AUDIO)			\
		pthread_mutex_unlock(&g_mutexShmAudio);	\
	else											\
		SHM_DEBUG("UNLOCK_MEDIA: enumMType error!\n");		\
}while(0)

#define MAX_IDS 1000000
#define MAX_PROFILES 127

typedef struct _shm_info{
	char cProfileNO;
	uint uiPos;
	uint uiSeq;
}SHM_INFO_STRUCT;

static unsigned long g_sulLogCount=0;

//id + shm_info
map<int, SHM_INFO_STRUCT> g_mapShmVideoInfo;
map<int, SHM_INFO_STRUCT> g_mapShmAudioInfo;

//profile + ShmRead
map<char, ShmRead*> g_mapShmVideoRead;
map<char, ShmRead*> g_mapShmAudioRead;

pthread_mutex_t g_mutexShmVideo=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_mutexShmAudio=PTHREAD_MUTEX_INITIALIZER;


static void SHM_GetMapObject(map<int, SHM_INFO_STRUCT> **ppmapShmInfo, map<char, ShmRead*> **ppmapShmRead, SHM_ENUM_MEDIA_TYPE enumMType)
{
	if(enumMType==SHM_ENUM_MEDIA_TYPE_VIDEO)
	{
		*ppmapShmInfo = &g_mapShmVideoInfo;
		*ppmapShmRead  = &g_mapShmVideoRead;
	}
	else 
	{
		*ppmapShmInfo = &g_mapShmAudioInfo;
		*ppmapShmRead  = &g_mapShmAudioRead;
	}
}

static ShmRead* SHM_GetReadObject(int iID, SHM_INFO_STRUCT** ppStructInfo, SHM_ENUM_MEDIA_TYPE enumMType)
{
	ShmRead* pclassShmRead;
	map<int, SHM_INFO_STRUCT>* pmapShmInfo;
	map<char, ShmRead*> *pmapShmRead;

	map<char, ShmRead*>::iterator itrRead;
	map<int, SHM_INFO_STRUCT>::iterator itrInfo;

	SHM_GetMapObject(&pmapShmInfo, &pmapShmRead, enumMType);

	itrInfo = pmapShmInfo->find(iID);
	if(itrInfo == pmapShmInfo->end())
	{
		SHM_DEBUG("The %s id=%d is not exist\n", enumMType==SHM_ENUM_MEDIA_TYPE_VIDEO?"video":"audio", iID);
		return NULL;
	}
	*ppStructInfo = &(itrInfo->second);
	
	itrRead = pmapShmRead->find(itrInfo->second.cProfileNO);
	if(itrRead == pmapShmRead->end())
	{
		SHM_DEBUG("The %s id=%d is not exist\n", enumMType==SHM_ENUM_MEDIA_TYPE_VIDEO?"video":"audio", iID);
		return NULL;
	}
	
	return (itrRead->second);
}

static int SHM_BackNFrameNolock(int iID, int iSteps, int iKeyFlag, int isBack, SHM_ENUM_MEDIA_TYPE enumMType)
{
	ShmRead* pclassShmRead;
	SHM_INFO_STRUCT* pStructInfo;
	int rtn=SHM_ENUM_ERROR;
	int back_cnt=0;
	uint uiBakPos, uiBakSeq;

	//struct timeval tv1, tv2;
	//gettimeofday(&tv1, NULL);
	//printf("id=%d, steps=%d, %s\n", iID, iSteps, enumMType==SHM_ENUM_MEDIA_TYPE_VIDEO?"video":"audio");
	pclassShmRead = SHM_GetReadObject(iID, &pStructInfo, enumMType);
	if(pclassShmRead != NULL && iSteps>0)
	{
		uiBakPos = pStructInfo->uiPos;
		uiBakSeq = pStructInfo->uiSeq;
		if(iKeyFlag == 1)
		{
		    ShmVideoExtraData structExtraData;
			while(iSteps > 0)
			{
				//check key frame flag first, 
				pclassShmRead->GetExtras((char *)&structExtraData, pStructInfo->uiPos, pStructInfo->uiSeq);
				if(structExtraData.ucKeyFrameFlag == SHM_ENUM_VIDEO_FRAME_TYPE_KEY) //key frame
				{
				   // printf("back_cnt:%d,  iSteps:%d\n", back_cnt, iSteps);
//					printf("find a keyframe.............. steps=%d, uiPos=%d, uiSeq=%d\n", iSteps, pStructInfo->uiPos, pStructInfo->uiSeq);

					if(back_cnt>0)
						iSteps--;
					
					if(iSteps<=0)
						break;
				}
				
				if(isBack==1 )
				{
				    int ret;
                    if((ret = pclassShmRead->MoveBack(&pStructInfo->uiPos, &pStructInfo->uiSeq)) < 0)
                    {
                        SHM_DEBUG("(ret %d = pclassShmRead->MoveBack(&pStructInfo->uiPos, &pStructInfo->uiSeq)) < 0)\n", ret);
					    break;
					}    
				}
                else
                {
                    if(pclassShmRead->MoveForward(&pStructInfo->uiPos, &pStructInfo->uiSeq) < 0)
					    break;
                }

				back_cnt++;
			}
		}
		else
		{
			do{
                if(isBack==1 )
				{
                    if(pclassShmRead->MoveBack(&pStructInfo->uiPos, &pStructInfo->uiSeq) < 0)
					    break;
				}
                else
                {
                    if(pclassShmRead->MoveForward(&pStructInfo->uiPos, &pStructInfo->uiSeq) < 0)
					    break;
                }                
			}while(--iSteps > 0);
		}

		if(iSteps<=0)
		{
			rtn = SHM_ENUM_OK;
		}
		else
		{
			pStructInfo->uiPos = uiBakPos;
			pStructInfo->uiSeq = uiBakSeq;
			SHM_DEBUG("line=%d, func=%s, iSteps=%d, SHM_ERROR to move to iSteps frame position\n", __LINE__, __FUNCTION__, iSteps);
			rtn = SHM_ENUM_ERROR;
		}
	}
	else if(iSteps == 0)
	{
		rtn = SHM_ENUM_OK;
	}

	//gettimeofday(&tv2, NULL);
	//printf("*****time of SHM_BackNFrameNolock: sec(%d), usec(%d)\n", tv2.tv_sec-tv1.tv_sec, tv2.tv_usec-tv1.tv_usec);
	return rtn;
}

static int SHM_RefreshNolock(int iID, int iKeyFlag, SHM_ENUM_MEDIA_TYPE enumMType)
{
	ShmRead* pclassShmRead;
	SHM_INFO_STRUCT* pStructInfo;
	int rtn=SHM_ENUM_ERROR;

	pclassShmRead = SHM_GetReadObject(iID, &pStructInfo, enumMType);
	if(pclassShmRead != NULL)
	{
		rtn = pclassShmRead->Refresh(&pStructInfo->uiPos, &pStructInfo->uiSeq);
        if(g_sulLogCount++ % 2000 == 0)
			printf("newest frame.............. uiPos=%d, uiSeq=%d\n", pStructInfo->uiPos, pStructInfo->uiSeq);
		if(iKeyFlag == 1)
		{
			ShmVideoExtraData structExtraData;
			pclassShmRead->GetExtras((char *)&structExtraData, pStructInfo->uiPos, pStructInfo->uiSeq);
			if(structExtraData.ucKeyFrameFlag != SHM_ENUM_VIDEO_FRAME_TYPE_KEY)
				rtn = SHM_BackNFrameNolock(iID, 1, iKeyFlag, 1, enumMType);
		}
	}
	
	return rtn;
}

/*****************************************************************************************************
** DESCRIPTION: init a ShmRead object
**
**
** Parameters:
**     cProfileNO : char, to.
**     iID        : int *, every peer who access the share memory should have an unique id.
**     enumMType     : SHM_ENUM_MEDIA_TYPE, video stream: SHM_ENUM_MEDIA_TYPE_VIDEO, audio stream: SHM_ENUM_MEDIA_TYPE_Audio 
**
** return:
**   -1    : eorror
**   >=0   : init success
**/
int SHM_InitID (char cProfileNO, int* piID, SHM_ENUM_MEDIA_TYPE enumMType)
{
	ShmRead* pclassShmRead;
	int idx;
	key_t keyShmBase;
	key_t keySemBase;
	key_t shmKey;
	key_t semKey;

	map<int, SHM_INFO_STRUCT>* pmapShmInfo;
	map<char, ShmRead*>* pmapShmRead;

	*piID = SHM_ENUM_ERROR;

	LOCK_SHM(enumMType);

	// create the ShmRead Object
	SHM_GetMapObject(&pmapShmInfo, &pmapShmRead, enumMType);
	if (pmapShmRead->find(cProfileNO) == pmapShmRead->end())
	{	
		if(enumMType==SHM_ENUM_MEDIA_TYPE_VIDEO)
		{
			keyShmBase = SHM_VIDEO_DEVICE_BASE_KEY;
			keySemBase = SEM_VIDEO_DEVICE_BASE_KEY;
		}
		else 
		{
			keyShmBase = SHM_AUDIO_DEVICE_BASE_KEY;
			keySemBase = SEM_AUDIO_DEVICE_BASE_KEY;
		}
		shmKey = keyShmBase + ((int)cProfileNO)*SHM_KEY_MULTI_BASE;
		semKey = keySemBase + ((int)cProfileNO)*SEM_KEY_MULTI_BASE;
		
		pclassShmRead = new ShmRead(shmKey, semKey, 0);		
		if(pclassShmRead->GetObjectStatus()== -1)
		{		
			SHM_DEBUG("create %s ShmRead error!, profile=%d\n", enumMType==SHM_ENUM_MEDIA_TYPE_VIDEO?"video":"audio", cProfileNO);
			delete pclassShmRead;
			goto RETURN;
		}

		SHM_DEBUG("create %s ShmRead for profile=%d!\n", enumMType==SHM_ENUM_MEDIA_TYPE_VIDEO?"video":"audio", cProfileNO);
		pmapShmRead->insert(pair<char, ShmRead*>(cProfileNO, pclassShmRead));
	}
	else
	{ // cProfileNO is aready exist
		//SHM_DEBUG("The cProfileNO=%d aready exist\n", cProfileNO);
	}

	// create id for user
	for(idx=0; idx<MAX_IDS; idx++)
	{
		if (pmapShmInfo->find(idx) == pmapShmInfo->end())
		{		
			SHM_INFO_STRUCT shm_info;
			shm_info.uiPos = SHM_FRAME_INITIAL_POSITION;
			shm_info.uiSeq = SHM_FRAME_INITIAL_SEQUENCE;
			shm_info.cProfileNO = cProfileNO;
			*piID = idx;
			pmapShmInfo->insert(pair<int, SHM_INFO_STRUCT>(*piID, shm_info));

			break;
		}
		else
		{
			//SHM_DEBUG("The %s id = %d aready exist\n", enumMType==SHM_ENUM_MEDIA_TYPE_VIDEO?"video":"audio", idx);
		}
	}

RETURN:

	SHM_DEBUG("++++++++++++++++++++++++++=  create %s id = %d\n", enumMType==SHM_ENUM_MEDIA_TYPE_VIDEO?"video":"audio", *piID);
	UNLOCK_SHM(enumMType);
	return *piID;
}

int SHM_CopyID(int iID, int *piCopyID, SHM_ENUM_MEDIA_TYPE enumMType)
{
	map<int, SHM_INFO_STRUCT>::iterator itrInfo;
	map<int, SHM_INFO_STRUCT>* pmapShmInfo;
	map<char, ShmRead*>* pmapShmRead;
	
	SHM_INFO_STRUCT structShmInfoCopy;
	int idx;
	
	*piCopyID = SHM_ENUM_ERROR;
	
	LOCK_SHM(enumMType);

	SHM_GetMapObject(&pmapShmInfo, &pmapShmRead, enumMType);
	itrInfo = pmapShmInfo->find(iID);
	if( itrInfo != pmapShmInfo->end())
	{
		structShmInfoCopy.cProfileNO = itrInfo->second.cProfileNO;
		structShmInfoCopy.uiPos = itrInfo->second.uiPos;
		structShmInfoCopy.uiSeq = itrInfo->second.uiSeq;
		SHM_DEBUG("structShmInfoCopy.pos = %d, shm_info_copy.seq = %d\n",structShmInfoCopy.uiPos, structShmInfoCopy.uiSeq );

		for(idx=0; idx<MAX_IDS; idx++)
		{
			if (pmapShmInfo->find(idx) == pmapShmInfo->end())
			{		
				*piCopyID = idx;
				pmapShmInfo->insert(pair<int, SHM_INFO_STRUCT>(*piCopyID, structShmInfoCopy));

				break;
			}
			else
			{
				SHM_DEBUG("The %s id = %d aready exist\n", enumMType==SHM_ENUM_MEDIA_TYPE_VIDEO?"video":"audio", idx);
			}
		}	
	}

	UNLOCK_SHM(enumMType);
	return *piCopyID;
}

int SHM_CompareWithID(int iID1, int iID2, SHM_ENUM_MEDIA_TYPE enumMType)
{
    ShmRead* pclassShmRead1;
	SHM_INFO_STRUCT* pStructInfo1;
    ShmRead* pclassShmRead2;
	SHM_INFO_STRUCT* pStructInfo2;
    int rtn = SHM_ENUM_ERROR;
    
    LOCK_SHM(enumMType);
	pclassShmRead1 = SHM_GetReadObject(iID1, &pStructInfo1, enumMType);
    pclassShmRead2 = SHM_GetReadObject(iID2, &pStructInfo2, enumMType);

    if(pclassShmRead1 != NULL  && pclassShmRead2 != NULL)
    {
        rtn = pStructInfo1->uiSeq - pStructInfo2->uiSeq;
    }
    else if(pclassShmRead1 == NULL && pclassShmRead2 == NULL)
    {
        rtn = SHM_ENUM_ID1_ID2_NOT_EXIT;
    }
    else if(pclassShmRead1 == NULL)
    {
        rtn = SHM_ENUM_ID1_NOT_EXIT; 
    }
    else if(pclassShmRead2 == NULL)
    {
        rtn = SHM_ENUM_ID2_NOT_EXIT;
    }
    
	UNLOCK_SHM(enumMType);
	
	return rtn;
}


int SHM_ReadFrameWithExtras(int iID, char *pcBuf, int *piBufLen, char* pcExtras, SHM_ENUM_MEDIA_TYPE enumMType)
{
	ShmRead* pclassShmRead;
	SHM_INFO_STRUCT* pStructInfo;
	
	int rtn=SHM_ENUM_ERROR;
	//printf("read_shm_frame %s\n", enumMType==SHM_ENUM_MEDIA_TYPE_VIDEO?"video":"audio");
	
	LOCK_SHM(enumMType);
	pclassShmRead = SHM_GetReadObject(iID, &pStructInfo, enumMType);
	if(pclassShmRead != NULL)
	{  
		if(pclassShmRead->IsInitState(pStructInfo->uiPos))
		{
            if(g_sulLogCount % 2000 == 0)
			    SHM_DEBUG("++++++in init state and refresh to the newest frame\n");
            
			int iKeyFlag=0;
			if(enumMType == SHM_ENUM_MEDIA_TYPE_VIDEO)
            {
			    ShmVideoHeaderInfo structHeaderInfo;
			    pclassShmRead->GetHeaderInfo((char*) &structHeaderInfo);
			    if(structHeaderInfo.ucCodec == SHM_ENUM_VIDEO_CODEC_TYPE_H264)
			    {
				    iKeyFlag=1;
				}
			}
			
			if(SHM_RefreshNolock(iID, iKeyFlag, enumMType)<0)
			{
                if(g_sulLogCount % 2000 == 0)
			        SHM_DEBUG("error refresh_shm_pos_nolock\n");
                rtn = SHM_ENUM_NO_FRAME;
				goto RETURN;
			}

            g_sulLogCount++;

		}
		
		rtn = pclassShmRead->ReadFrame(pcBuf, piBufLen, pcExtras, &pStructInfo->uiPos, &pStructInfo->uiSeq);
		if(rtn==SHM_ENUM_GET_A_NEW_FRAME && enumMType == SHM_ENUM_MEDIA_TYPE_VIDEO)
		{
			ShmVideoExtraData *pstructShmVideoExtras = (ShmVideoExtraData *)pcExtras;
			if(pstructShmVideoExtras->ucKeyFrameFlag == SHM_ENUM_VIDEO_FRAME_TYPE_KEY)//keyframe
			{
				//printf("get a key frame......................\n");
				rtn = SHM_ENUM_GET_A_NEW_KEY_FRAME;
			}
		}
	}
	
RETURN:
	UNLOCK_SHM(enumMType);
	
	return rtn;
}


int SHM_GetHeaderInfoWithID(int iID, char pcHeaderInfo[32], SHM_ENUM_MEDIA_TYPE enumMType)
{
    ShmRead* pclassShmRead;
	SHM_INFO_STRUCT* pStructInfo;
	int rtn=SHM_ENUM_ERROR;
    
    LOCK_SHM(enumMType);
	pclassShmRead = SHM_GetReadObject(iID, &pStructInfo, enumMType);
	if(pclassShmRead != NULL)
	{  
        pclassShmRead->GetHeaderInfo(pcHeaderInfo);
        rtn = SHM_ENUM_OK;
	}
    UNLOCK_SHM(enumMType);

    return rtn;
}


int SHM_SetHeaderInfoWithID(int iID, char pcHeaderInfo[32], SHM_ENUM_MEDIA_TYPE enumMType)
{
    ShmRead* pclassShmRead;
	SHM_INFO_STRUCT* pStructInfo;
	int rtn=SHM_ENUM_ERROR;
    
    LOCK_SHM(enumMType);
	pclassShmRead = SHM_GetReadObject(iID, &pStructInfo, enumMType);
	if(pclassShmRead != NULL)
	{  
        pclassShmRead->SetHeaderInfo(pcHeaderInfo);
        rtn = SHM_ENUM_OK;
	}
    UNLOCK_SHM(enumMType);

    return rtn;
}

int SHM_GetFrameExtras(int iID, char* pcarrExtras, SHM_ENUM_MEDIA_TYPE enumMType)
{
    ShmRead* pclassShmRead;
	SHM_INFO_STRUCT* pStructInfo;
    
    pclassShmRead = SHM_GetReadObject(iID, &pStructInfo, enumMType);
    
    if(pclassShmRead != NULL)
        return pclassShmRead->GetExtras(pcarrExtras, pStructInfo->uiPos, pStructInfo->uiSeq);
    else
        return SHM_ENUM_ERROR;
}


int SHM_ReadFrame(int iID, char *pcBuf, int *piBufLen, SHM_ENUM_MEDIA_TYPE enumMType)
{
	char carrExtras[SHM_SIZE_OF_FRAME_EXTRAS];
	int rtn;

	rtn = SHM_ReadFrameWithExtras(iID, pcBuf, piBufLen, carrExtras, enumMType);

	return rtn;
}

int SHM_Debug_GetSeq(int iID, SHM_ENUM_MEDIA_TYPE enumMType, int* picurPos, int* picurSeq)
{
	ShmRead* pclassShmRead;
	SHM_INFO_STRUCT* pStructInfo;
	
	int rtn=SHM_ENUM_ERROR;
	//printf("read_shm_frame %s\n", enumMType==SHM_ENUM_MEDIA_TYPE_VIDEO?"video":"audio");
	
	LOCK_SHM(enumMType);
	pclassShmRead = SHM_GetReadObject(iID, &pStructInfo, enumMType);
	*picurPos=pStructInfo->uiPos;
	*picurSeq=pStructInfo->uiSeq;
	UNLOCK_SHM(enumMType);
	
	return rtn;
}

/*****************************************************************************************************
** DESCRIPTION: detach the id with share memory , and free it's resource
**
**
** Parameters:
**     iID        : int*, every peer who access the share memory should have an unique id.
**     enumMType     : SHM_ENUM_MEDIA_TYPE, video stream: SHM_ENUM_MEDIA_TYPE_VIDEO, audio stream: SHM_ENUM_MEDIA_TYPE_Audio 
**   
** return:
*************************************************/
void SHM_RemoveID(int iID, SHM_ENUM_MEDIA_TYPE enumMType)
{
	map<int, SHM_INFO_STRUCT>* pmapShmInfo;
	map<char, ShmRead*>* pmapShmRead;
	map<int, SHM_INFO_STRUCT>::iterator itrInfo;

	LOCK_SHM(enumMType);
	SHM_GetMapObject(&pmapShmInfo, &pmapShmRead, enumMType);

	itrInfo = pmapShmInfo->find(iID);
	if(itrInfo != pmapShmInfo->end())
	{
		pmapShmInfo->erase(itrInfo);
		SHM_DEBUG("++++++++erase %s id = %d\n", enumMType==SHM_ENUM_MEDIA_TYPE_VIDEO?"video":"audio", iID);
		//maybe need to erase the ShmVideoRead object if the peers=0, i suggest not todo this.
	}
	else
	{
		SHM_DEBUG("The enumMType=%d(0 video, 1 audio), id=%d is not exist\n", enumMType, iID);
	}
	
	UNLOCK_SHM(enumMType);
}



/*****************************************************************************************************
** DESCRIPTION: get the distance from the current frame to the newest frame
**
**
** Parameters:
**     iID        : int*, every peer who access the share memory should have an unique id.
**     enumMType     : SHM_ENUM_MEDIA_TYPE, video stream: SHM_ENUM_MEDIA_TYPE_VIDEO, audio stream: SHM_ENUM_MEDIA_TYPE_Audio 
**   
** return:
**     <0 : error
**    >=0 : success get the distance
*************************************************/
int SHM_Distance2Newest(int iID, SHM_ENUM_MEDIA_TYPE enumMType)
{
	ShmRead* pclassShmRead;
	SHM_INFO_STRUCT* pStructInfo;
	int distance=SHM_ENUM_ERROR;

	LOCK_SHM(enumMType);
	pclassShmRead = SHM_GetReadObject(iID, &pStructInfo, enumMType);
	if(pclassShmRead != NULL)
	{
		distance = pclassShmRead->Distance2Newest(pStructInfo->uiPos, pStructInfo->uiSeq);
	}
	else
	{
		SHM_DEBUG("function SHM_Distance2Newest shm_read  is NULL\n");
	}
	UNLOCK_SHM(enumMType);
	
	return distance;
}

/*****************************************************************************************************
** DESCRIPTION: back to the frame postion which distant iSteps to current frame
**  if the current frame uiPos=-1, it means current frame is the newest frame.
**
**
** Parameters:
**     iID       : int*, every peer who access the share memory should have an unique id.
**     iSteps    : int, steps for move back 
**     iKeyFlag  : int, point out if iSteps is the normal frame iSteps or the key frame iSteps
**     enumMType     : SHM_ENUM_MEDIA_TYPE, video stream: SHM_ENUM_MEDIA_TYPE_VIDEO, audio stream: SHM_ENUM_MEDIA_TYPE_Audio 
**   
** return:
**     <0 : error
**    >=0 : success back to the frame postion which distant iSteps to current frame
*************************************************/
int SHM_BackNFrame(int iID, int iSteps, int iKeyFlag, SHM_ENUM_MEDIA_TYPE enumMType)
{
	
	int rtn=SHM_ENUM_ERROR;

	LOCK_SHM(enumMType);
	rtn = SHM_BackNFrameNolock(iID, iSteps, iKeyFlag, 1, enumMType);
	UNLOCK_SHM(enumMType);
		
	return rtn;	
}

int SHM_ForwardNFrame(int iID, int iSteps, int iKeyFlag, SHM_ENUM_MEDIA_TYPE enumMType)
{
    int rtn=SHM_ENUM_ERROR;
    
    LOCK_SHM(enumMType);
    rtn = SHM_BackNFrameNolock(iID, iSteps, iKeyFlag, 0, enumMType);
    UNLOCK_SHM(enumMType);
	
	return rtn;
}

/*****************************************************************************************************
** DESCRIPTION: refresh the current frame position to the newest one
**
**
** Parameters:
**     iID        : int*, every peer who access the share memory should have an unique id.
**     iKeyFlag  : point out if front_n is the normal frame steps or the key frame steps
**     enumMType     : SHM_ENUM_MEDIA_TYPE, video stream: SHM_ENUM_MEDIA_TYPE_VIDEO, audio stream: SHM_ENUM_MEDIA_TYPE_Audio 
**   
** return:
**     <0 : error
**    >=0 : success get to the newest (key)frame postion
*************************************************/
int SHM_Refresh(int iID, int iKeyFlag, SHM_ENUM_MEDIA_TYPE enumMType)
{  
	int rtn=SHM_ENUM_ERROR;

	LOCK_SHM(enumMType);
	rtn = SHM_RefreshNolock(iID, iKeyFlag, enumMType);
	UNLOCK_SHM(enumMType);

	return rtn;
}

