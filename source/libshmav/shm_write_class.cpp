#include "shm_write_class.h"
#include "stdlib.h"

ShmWrite::ShmWrite(key_t shmKey, key_t semKey,  int iShmSize, int iMaxFrameSize, char pcHeaderInfo[32]):ShmBase(shmKey, semKey, iShmSize, SHM_FLAG_WRITE)
{
	//share memory will create the base class(ShmBase)
	if(m_iObjectStatus == OBJECT_STATUES_NORMAL)
	{
		m_classSemLock->Lock();
		memset(m_pcShm, 0, iShmSize);
		m_structShmHead->uiMagic = SHM_FRAME_MAGIC_NUM;
		m_structShmHead->iMaxFrameSize = iMaxFrameSize;
		m_structShmHead->iShmSize       = iShmSize;
		m_structShmHead->uiNewestFramePos = SHM_FRAME_START_POSITION;
		m_structShmHead->uiNewestFrameSeq= SHM_FRAME_INITIAL_SEQUENCE;
		
		m_uiFramePos = SHM_FRAME_START_POSITION;
		m_uiFrameSeq = SHM_FRAME_INITIAL_SEQUENCE;
		memcpy(m_structShmHead->carrUserDefinedInfo, pcHeaderInfo, SHM_SIZE_OF_USER_DEFINED_INFO);

		m_iShmSize = iShmSize;
		m_classSemLock->UnLock();
//printf("ShmWrite++++++++m_uiFramePos=%d, m_uiFrameSeq=%d, m_iShmSize=%d, iMaxFrameSize=%d\n", m_uiFramePos, m_uiFrameSeq, m_iShmSize, m_structShmHead->iMaxFrameSize);
	}
	else
	{
		printf("ShmWrite::m_iObjectStatus != OBJECT_STATUES_NORMAL\n");
	}
}

ShmWrite::~ShmWrite()
{
    //memset(m_pcShm, 0, m_structShmHead->iShmSize);
}

/*****************************************************************************************************
** DESCRIPTION: write the frame to  sharememory
**    
**
** Parameters:
**     pcBuf   : char*, frame data.
**     iBufLen   : int, frame size
**     extras  : char*, sizeof(ShmVideoExtraData) bytes extras infos about this frame(user defined info) (key frame, resolution.... defined by the caller)
**
**/
int ShmWrite::WriteFrame(char* pcBuf, int iBufLen, char *pcExtras)
{
	struct frame_head_struct *frame;
	uint uiNextFramePos = 0; 
	int complement = 0;

	if(iBufLen<=0)
	{
		printf("ERROR video data, len=%d\n", iBufLen);
		return SHM_ENUM_ERROR;
	}

	m_classSemLock->Lock();
	if(iBufLen > m_structShmHead->iMaxFrameSize)
	{
		printf("ShmWrite::Write: the video frame you write is too big! iFrameSize=%d, max_frame_size=%d\n", iBufLen, m_structShmHead->iMaxFrameSize);

		m_classSemLock->UnLock();
		return SHM_ENUM_WRITE_OVERFLOW;
	}

	//if(pcExtras[0] == 1)
	//printf("11ShmWrite::Write-----m_uiFrameSeq=%d, m_uiFramePos=%d, m_iShmSize=%d\n", m_uiFrameSeq , m_uiFramePos, m_iShmSize);
	uiNextFramePos = m_uiFramePos + iBufLen + SHM_SIZE_OF_FRAME_HEAD;//SIZE_OF_FRAME_HEAD*2 contains the next frame's head
	complement = uiNextFramePos&3==0?0:(4 - uiNextFramePos&3);
	uiNextFramePos += complement; //align=4

	if(uiNextFramePos >= m_iShmSize)
	{
		m_uiFramePos = SHM_FRAME_START_POSITION;
		frame = (struct frame_head_struct*)(m_pcShm+m_structShmHead->uiNewestFramePos);
		frame->uiNextFramePos = m_uiFramePos;

		uiNextFramePos = m_uiFramePos + iBufLen + SHM_SIZE_OF_FRAME_HEAD;
		complement = uiNextFramePos&3==0?0:(4 - uiNextFramePos&3);
		uiNextFramePos += complement; //align=4     
		//printf("+++++++++++++++++reset the position, m_uiFramePos=%d++++++++++++++++++\n", m_uiFramePos);
	}
	else if( (uiNextFramePos + SHM_SIZE_OF_FRAME_HEAD) >= m_iShmSize) 
	{// if next frame's head end pos greater than m_iShmSize 
		uiNextFramePos = SHM_FRAME_START_POSITION;
	}

	m_uiFrameSeq++;
	if(m_uiFrameSeq >= SHM_FRAME_MAX_SEQUENCE)
	{
		m_uiFrameSeq=1;
	}

	frame = (struct frame_head_struct*) (m_pcShm + m_uiFramePos);
	memcpy(m_pcShm + m_uiFramePos + SHM_SIZE_OF_FRAME_HEAD, pcBuf, iBufLen);

	memset(m_pcShm + uiNextFramePos, 0, SHM_SIZE_OF_FRAME_HEAD);
	frame->iFrameSize = iBufLen;
	frame->uiSequence  = m_uiFrameSeq;
	frame->uiNextFramePos = uiNextFramePos;
	frame->uiPrevFramePos = m_structShmHead->uiNewestFramePos;
	frame->uiMagic = m_structShmHead->uiMagic;
	memcpy(frame->carrExtras, pcExtras, SHM_SIZE_OF_FRAME_EXTRAS);
	m_structShmHead->uiNewestFramePos = m_uiFramePos;
	m_structShmHead->uiNewestFrameSeq= m_uiFrameSeq;
	m_uiFramePos  = uiNextFramePos;

	m_classSemLock->UnLock();
	//if(pcExtras[0] == 1 && m_structShmHead->carrUserDefinedInfo[2] == 1)
	//printf("ShmWrite::Write-----m_uiFrameSeq=%d, m_uiFramePos=%d, iFrameSize=%d, keyflag=%d\n", m_uiFrameSeq , m_uiFramePos, frame->iFrameSize, pcExtras[1]);
	return SHM_ENUM_OK;
}

