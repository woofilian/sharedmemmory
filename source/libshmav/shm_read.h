#ifndef __SHM_READ_H__
#define __SHM_READ_H__

#include "shm_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SHM_VideoInitID(cProfileNO, iID) SHM_InitID(cProfileNO, iID, SHM_ENUM_MEDIA_TYPE_VIDEO)
#define SHM_AudioInitID(cProfileNO, iID) SHM_InitID(cProfileNO, iID, SHM_ENUM_MEDIA_TYPE_AUDIO)

#define SHM_VideoCopyID(iID, piCopyID) SHM_CopyID(iID, piCopyID, SHM_ENUM_MEDIA_TYPE_VIDEO)
#define SHM_AudioCopyID(iID, piCopyID) SHM_CopyID(iID, piCopyID, SHM_ENUM_MEDIA_TYPE_AUDIO)

#define SHM_VideoCompareWithID(iID1, iID2) SHM_CompareWithID(iID1, iID2, SHM_ENUM_MEDIA_TYPE_VIDEO)
#define SHM_AudioCompareWithID(iID1, iID2) SHM_CompareWithID(iID1, iID2, SHM_ENUM_MEDIA_TYPE_AUDIO)

#define SHM_VideoReadFrameWithExtras(iID, pcBuf, piBufLen, pcExtras) SHM_ReadFrameWithExtras(iID, pcBuf, piBufLen, pcExtras, SHM_ENUM_MEDIA_TYPE_VIDEO)
#define SHM_AudioReadFrameWithExtras(iID, pcBuf, piBufLen, pcExtras) SHM_ReadFrameWithExtras(iID, pcBuf, piBufLen, pcExtras, SHM_ENUM_MEDIA_TYPE_AUDIO)

#define SHM_VideoSetHeaderInfoWithID(iID, pcHeaderInfo) SHM_SetHeaderInfoWithID(iID, pcHeaderInfo, SHM_ENUM_MEDIA_TYPE_VIDEO)
#define SHM_AudioSetHeaderInfoWithID(iID, pcHeaderInfo) SHM_SetHeaderInfoWithID(iID, pcHeaderInfo, SHM_ENUM_MEDIA_TYPE_AUDIO)

#define SHM_VideoGetHeaderInfoWithID(iID, pcHeaderInfo) SHM_GetHeaderInfoWithID(iID, pcHeaderInfo, SHM_ENUM_MEDIA_TYPE_VIDEO)
#define SHM_AudioGetHeaderInfoWithID(iID, pcHeaderInfo) SHM_GetHeaderInfoWithID(iID, pcHeaderInfo, SHM_ENUM_MEDIA_TYPE_AUDIO)

#define SHM_VideoGetFrameExtras(iID, carrExtras) SHM_GetFrameExtras(iID, carrExtras, SHM_ENUM_MEDIA_TYPE_VIDEO)
#define SHM_AudioGetFrameExtras(iID, carrExtras) SHM_GetFrameExtras(iID, carrExtras, SHM_ENUM_MEDIA_TYPE_AUDIO)

#define SHM_VideoReadFrame(iID, pcBuf, piBufLen) SHM_ReadFrame(iID, pcBuf, piBufLen, SHM_ENUM_MEDIA_TYPE_VIDEO)
#define SHM_AudioReadFrame(iID, pcBuf, piBufLen) SHM_ReadFrame(iID, pcBuf, piBufLen, SHM_ENUM_MEDIA_TYPE_AUDIO)

#define SHM_VideoRemoveID(iID) SHM_RemoveID(iID, SHM_ENUM_MEDIA_TYPE_VIDEO)
#define SHM_AudioRemoveID(iID) SHM_RemoveID(iID, SHM_ENUM_MEDIA_TYPE_AUDIO)

#define SHM_VideoDistance2Newest(iID) SHM_Distance2Newest(iID,  SHM_ENUM_MEDIA_TYPE_VIDEO)
#define SHM_AudioDistance2Newest(iID) SHM_Distance2Newest(iID,  SHM_ENUM_MEDIA_TYPE_AUDIO)

#define SHM_VideoBackNFrame(iID, iSteps, iKeyFlag)  SHM_BackNFrame(iID, iSteps, iKeyFlag, SHM_ENUM_MEDIA_TYPE_VIDEO)
#define SHM_AudioBackNFrame(iID, iSteps, iKeyFlag)  SHM_BackNFrame(iID, iSteps, iKeyFlag, SHM_ENUM_MEDIA_TYPE_AUDIO)

#define SHM_VideoForwardNFrame(iID, iSteps, iKeyFlag)  SHM_ForwardNFrame(iID, iSteps, iKeyFlag, SHM_ENUM_MEDIA_TYPE_VIDEO)
#define SHM_AudioForwardNFrame(iID, iSteps, iKeyFlag)  SHM_ForwardNFrame(iID, iSteps, iKeyFlag, SHM_ENUM_MEDIA_TYPE_AUDIO)


#define SHM_VideoRefresh(iID, iKeyFlag) SHM_Refresh(iID, iKeyFlag, SHM_ENUM_MEDIA_TYPE_VIDEO)
#define SHM_AudioRefresh(iID, iKeyFlag) SHM_Refresh(iID, iKeyFlag, SHM_ENUM_MEDIA_TYPE_AUDIO)

#define SHM_Video_Debug_GetSeq(iID, picurPos, picurSeq) SHM_Debug_GetSeq(iID, SHM_ENUM_MEDIA_TYPE_VIDEO, picurPos, picurSeq)
#define SHM_Audio_Debug_GetSeq(iID, picurPos, picurSeq) SHM_Debug_GetSeq(iID, SHM_ENUM_MEDIA_TYPE_AUDIO, picurPos, picurSeq)



/****************************************************
** attach and iID to the share memory 
*/
int SHM_InitID (char cProfileNO, int* piID, SHM_ENUM_MEDIA_TYPE enumMType);

/************************************************
**description: copy the shm info from id to *piCopyID 
**
**param:
**   
**   
** return:
**  -1: error  //the id not exist
**  >0: success
*************************************************/
int SHM_CopyID(int iID, int *piCopyID, SHM_ENUM_MEDIA_TYPE enumMType);


/************************************************
**description: compare the sequnce of iID1 and iID2
**
**param:
**   
**   
** return:
**   (<0): seq iID1 little than seq iID2  //the id not exist
**   (=0): seq iID1 equals seq iID2
**   (>0): seq iID1 greater than seq iID2
**   (==SHM_ENUM_ID1_NOT_EXIT) : iID1 is not exist
**   (==SHM_ENUM_ID2_NOT_EXIT) : iID2 is not exist
*************************************************/
int SHM_CompareWithID(int iID1, int iID2, SHM_ENUM_MEDIA_TYPE enumMType);


/************************************************
**description: set the current shm's extras header
**
**param:
**   
**   
** return:
**  SHM_ENUM_ERROR(=-1): error  //the id not exist
**  SHM_ENUM_OK (=0): success
*************************************************/
int SHM_SetHeaderInfoWithID(int iID, char pcHeaderInfo[32], SHM_ENUM_MEDIA_TYPE enumMType);


/************************************************
**description: get the current shm's extras header through ID, output it to pcHeaderInfo
**
**param:
**   
**   
** return:
**  SHM_ENUM_ERROR(=-1): error  //the id not exist
**  SHM_ENUM_OK (=0): success
*************************************************/
int SHM_GetHeaderInfoWithID(int iID, char pcHeaderInfo[32], SHM_ENUM_MEDIA_TYPE enumMType);

/************************************************
**description: get the current frame's extras, output it form carrExtras
**
**param:
**   
**   
** return:
**  SHM_ENUM_ERROR(=-1): error  //the id not exist, or frame error
**  SHM_ENUM_OK (=0): success
**  SHM_ENUM_NEXT_FRAME_OF_NEWEST(=4): //it is an unfinished frame which is the next frame of newest frame
*************************************************/
int SHM_GetFrameExtras(int iID, char carrExtras[16], SHM_ENUM_MEDIA_TYPE enumMType);


/*****************************************************************************************************
** DESCRIPTION: read next frame from share memory 
**
**
** Parameters:
**     iID        : int*, every peer who access the share memory should have an unique id.
**     pcBuf       : char*, to output the frame data
**     iBufLen       : len,  length of frame data
**     pcExtras      : char*, extras data, you can use struct of ShmVideoExtraData or ShmAudioExtraData to input the pa
**     enumMType     : SHM_ENUM_MEDIA_TYPE, video stream: SHM_ENUM_MEDIA_TYPE_VIDEO, audio stream: SHM_ENUM_MEDIA_TYPE_Audio 
**   
** return:
**  -1: error
**   0: no new frame
**   1: frame size is too large, will be discard this frame
**   2: get a new frame
**   3: get a new key frame  (only for h264 in current projects)
*************************************************/
int SHM_ReadFrameWithExtras(int iID, char *pcBuf, int *piBufLen, char* pcExtras,SHM_ENUM_MEDIA_TYPE enumMType);


/*****************************************************************************************************
** DESCRIPTION: read next frame from share memory 
**
**
** Parameters:
**     iID        : int*, every peer who access the share memory should have an unique id.
**     pcBuf       : char*, to output the frame data
**     iBufLen       : len,  length of frame data
**     enumMType     : SHM_ENUM_MEDIA_TYPE, video stream: SHM_ENUM_MEDIA_TYPE_VIDEO, audio stream: SHM_ENUM_MEDIA_TYPE_Audio 
**   
** return:
**  -1: error
**   0: no new frame
**   1: frame size is too large, will be discard this frame
**   2: get a new frame
**   3: get a new key frame  (only for h264 in current projects)
*************************************************/
int SHM_ReadFrame(int iID, char *pcBuf, int *piBufLen ,SHM_ENUM_MEDIA_TYPE enumMType);

/***************************************************
** detach the id from the share memory
*/
void SHM_RemoveID(int iID, SHM_ENUM_MEDIA_TYPE enumMType);



int SHM_Debug_GetSeq(int iID, SHM_ENUM_MEDIA_TYPE enumMType, int* picurPos, int* picurSeq);

/******************************************************
**  get the distance from the current frame to the newest frame
**  do not refresh the position of the current frame
*/
int SHM_Distance2Newest(int iID,  SHM_ENUM_MEDIA_TYPE enumMType);


/********************************************
** steps: steps to current frame 
** iKeyFlag: point out if steps is the normal frame steps or the key frame steps
** enumMType: enum, 0 video, 1 audio
** return: -1, not find, 0 success
** description: get the frame info of the frame front of current frame 
**  if the current frame pos=-1, it means current frame is the newest frame.
**/
int SHM_BackNFrame(int iID, int iSteps, int iKeyFlag, SHM_ENUM_MEDIA_TYPE enumMType);

int SHM_ForwardNFrame(int iID, int iSteps, int iKeyFlag, SHM_ENUM_MEDIA_TYPE enumMType);

/******************************************************
**  
**  refresh the current frame position to the newest
*/
int SHM_Refresh(int iID, int iKeyFlag, SHM_ENUM_MEDIA_TYPE enumMType);

#ifdef __cplusplus
}
#endif


#endif /*__SHM_READ_H__*/
