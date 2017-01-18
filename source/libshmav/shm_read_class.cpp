#include "shm_read_class.h"

#define SHM_EMPTY_CHECK() \
    do{\
        if(m_structShmHead->uiNewestFrameSeq == SHM_FRAME_INITIAL_SEQUENCE) \
            return SHM_ENUM_EMPTY_SPACE;\
    }while(0)


ShmRead::ShmRead(key_t shmKey, key_t semKey,  int iShmSize):ShmBase(shmKey, semKey, iShmSize, SHM_FLAG_READ)
{	
	if(m_iObjectStatus == OBJECT_STATUES_NORMAL)
	{
		m_classSemLock->Lock();
		m_iShmSize = m_structShmHead->iShmSize;
		m_uiFramePos = SHM_FRAME_INITIAL_POSITION;
		m_uiFrameSeq = SHM_FRAME_INITIAL_SEQUENCE;
		m_classSemLock->UnLock();
//printf("m_iShmSize=%d, iMaxFrameSize=%d\n", m_iShmSize, m_structShmHead->iMaxFrameSize);
	}
	else
	{
		printf("shm space is not exist... please confirm you have create the space aready!\n");
	}
}

ShmRead::ShmRead(const ShmRead &refShmRead):ShmBase(refShmRead)
{
}

ShmRead::~ShmRead()
{
}


/*****************************************************************************************************
** DESCRIPTION: write the frame to  
**     will set m_iObjectStatus=OBJECT_STATUES_ERROR
**
**
** Parameters:
**     pcBuf   : char*, frame data.
**     piBufLen   : int*, output param, frame size
**     pcExtras  : char*, 16 bytes extras infos about this frame(user defined info) (key frame, resolution.... defined by the caller)
**
** return:
**   SHM_ENUM_ERROR=-1         : eorror
**   SHM_ENUM_NO_NEW_FRAME=0   : no new frame
**   SHM_ENUM_FRAME_TOO_LARGE=1: frame size is too large, will be discard this frame
**   SHM_ENUM_GET_A_NEW_FRAME=2: get a new frame
**   SHM_ENUM_EMPTY_SPACE=5    : still have no data in the shm, it is an empty space
**/
int ShmRead::ReadFrame(char* pcBuf, int *piBufLen, char *pcExtras, uint *puiPos, uint *puiSeq)
{
	struct frame_head_struct *frame, *frame_start;
	int rtn=SHM_ENUM_NO_NEW_FRAME;

    SHM_EMPTY_CHECK();
	
	m_classSemLock->Lock();

	m_uiFramePos = *puiPos;
	m_uiFrameSeq = *puiSeq;
	
	//check if there is newer frame in the share memory
	if(m_structShmHead->uiNewestFrameSeq < m_uiFrameSeq)
	{
		if(m_structShmHead->uiNewestFrameSeq != m_uiFrameSeq - 1)
		{
			//printf("line=%d, m_uiFrameSeq=%d, uiNewestFrameSeq=%d, exception error! reset the postion\n", __LINE__, m_uiFrameSeq, m_structShmHead->uiNewestFrameSeq);
			m_uiFramePos= SHM_FRAME_INITIAL_POSITION;
			m_uiFrameSeq = SHM_FRAME_INITIAL_SEQUENCE;
			rtn = SHM_ENUM_ERROR;
		}
		else
		{// no new frame
			//printf("line=%d, m_uiFrameSeq=%d, uiNewestFrameSeq=%d, no new frame\n", __LINE__, m_uiFrameSeq, m_structShmHead->uiNewestFrameSeq);
		}
		goto RETURN;
	}

	//pre-check the position for the end frame of share memory
	CheckOverflowed();
	frame = (struct frame_head_struct*) (m_pcShm + m_uiFramePos);
	// copy the frame
	if(frame->uiMagic == m_structShmHead->uiMagic)
	{
		if(frame->iFrameSize > m_structShmHead->iMaxFrameSize)
		{
			printf("line=%d, iFrameSize=%d, iMaxFrameSize=%d, iFrameSize>iMaxFrameSize\n", __LINE__, frame->iFrameSize, m_structShmHead->iMaxFrameSize);
			rtn = SHM_ENUM_FRAME_TOO_LARGE;// refresh to the next frame, and drop this frame
		}
        else if(frame->iFrameSize > *piBufLen)
        {
            printf("line=%d, iFrameSize=%d, output buffer size=%d, iFrameSize>output buffer size\n", __LINE__, frame->iFrameSize, *piBufLen);
            rtn = SHM_ENUM_FRAME_TOO_LARGE;
            goto RETURN; // do not refresh to the next frame
        }
		else
		{
            if(pcBuf != NULL)//pcBuf == NULL, when for just need to forward a frame
            {
    			memcpy(pcBuf, &m_pcShm[m_uiFramePos + SHM_SIZE_OF_FRAME_HEAD], frame->iFrameSize);
    			memcpy(pcExtras, frame->carrExtras, SHM_SIZE_OF_FRAME_EXTRAS);
    			
    			*piBufLen = frame->iFrameSize;
            }
			rtn = SHM_ENUM_GET_A_NEW_FRAME;		
		}

		m_uiFrameSeq = frame->uiSequence+1; //next frame sequence
		m_uiFramePos = frame->uiNextFramePos; //next frame pos
	}
	else
	{//lost postion, error uiMagic number
		printf("line=%d, read error, error uiMagic number! reset the postion\n", __LINE__);
		m_uiFramePos= SHM_FRAME_INITIAL_POSITION;
		m_uiFrameSeq = SHM_FRAME_INITIAL_SEQUENCE;
		rtn = SHM_ENUM_ERROR;
	}

	if(m_uiFrameSeq >= SHM_FRAME_MAX_SEQUENCE)
	{
		m_uiFrameSeq = 0;
	}

RETURN:
	*puiPos = m_uiFramePos;
	*puiSeq = m_uiFrameSeq;
	
	//if(rtn != SHM_ENUM_NO_NEW_FRAME)
	//	printf("%x,%d,%d,%x,%x\n", m_uiFrameSeq, m_uiFramePos, *piBufLen, m_structShmHead->iShmSize, m_structShmHead->uiNewestFrameSeq);
	m_classSemLock->UnLock();
	return rtn;
}

/*****************************************************************************************************
** DESCRIPTION: get the distance between current frame to the newest frame (by frame, not by bytes)   
**     
**
** return:
**   -1 : this frame have been covered.
**   >=0: success return the distance.
**/
int ShmRead::Distance2Newest(uint uiPos, uint uiSeq)
{
	struct frame_head_struct *frame;
	int distance;

	m_classSemLock->Lock();

	m_uiFramePos = uiPos;
	m_uiFrameSeq = uiSeq;
	CheckOverflowed();
	frame = (struct frame_head_struct*) (m_pcShm + m_uiFramePos);
	if(m_uiFrameSeq == m_structShmHead->uiNewestFrameSeq + 1)
	{//the m_frame_seq is the next frame sequence of the newest frame
		distance = -1;
	}
	else if(frame->uiMagic == m_structShmHead->uiMagic && frame->uiSequence == m_uiFrameSeq )
	{
		distance = (m_structShmHead->uiNewestFrameSeq - m_uiFrameSeq);
	}
	else
	{//lost postion
		//SHM_DEBUG("this frame have been covered!\n");
		distance = -2;
	}
	
	m_classSemLock->UnLock();

	return distance;
	
}

/*****************************************************************************************************
** DESCRIPTION: move the m_uiFramePos and m_uiFrameSeq to the back frame   
**     
**
** return:
**/
int ShmRead::MoveBack(uint *puiPos, uint *puiSeq)
{
	struct frame_head_struct *frame;
	int rtn=SHM_ENUM_ERROR;

    SHM_EMPTY_CHECK();

	m_classSemLock->Lock();
	
	m_uiFramePos = *puiPos;
	m_uiFrameSeq = *puiSeq;
	
	if(m_uiFramePos == SHM_FRAME_INITIAL_POSITION)
	{
		m_uiFramePos = m_structShmHead->uiNewestFramePos;
		frame = (struct frame_head_struct*) (m_pcShm + m_uiFramePos);
		m_uiFrameSeq = frame->uiSequence;
		rtn = SHM_ENUM_OK;
	}
	else if(m_uiFrameSeq == m_structShmHead->uiNewestFrameSeq + 1)
	{
		m_uiFramePos = m_structShmHead->uiNewestFramePos;
		m_uiFrameSeq = m_structShmHead->uiNewestFrameSeq;
		rtn = SHM_ENUM_OK;
	}
	else
	{
		CheckOverflowed();
		frame = (struct frame_head_struct*) (m_pcShm + m_uiFramePos);
		if(frame->uiMagic != m_structShmHead->uiMagic)
		{
			
			if(m_uiFramePos == SHM_FRAME_START_POSITION)
			{
			    printf("line=%d, SHM_ERROR: this share memory still uninitialized\n", __LINE__);
			}
			else
			{
			    printf("line=%d, SHM_ERROR: current frame is not a real frame, have been covered\n", __LINE__);
			}
		}
		else
		{
		    if(m_uiFramePos == frame->uiPrevFramePos)
		    {
		        SHM_DEBUG(" this is equal with prev frame\n");
		    }
		    else
		    {
    			m_uiFramePos = frame->uiPrevFramePos;			
    			frame = (struct frame_head_struct*) (m_pcShm + m_uiFramePos);
    			if(frame->uiMagic == m_structShmHead->uiMagic)
    			{
    			    m_uiFrameSeq = frame->uiSequence;
    				rtn = SHM_ENUM_OK;
    			}
    			else
    			{
    				SHM_DEBUG("line=%d, SHM_ENUM_ERROR: front frame is not a real frame, may have been covered\n", __LINE__);
    			}
    		}
		}
	}

	//printf("line=%d, Return: pos=%d,puiSeq=%d\n", __LINE__, m_uiFramePos, m_uiFrameSeq);

	if(rtn <0)
	{
		printf("error, move back...........\n");
		m_uiFrameSeq = SHM_FRAME_INITIAL_SEQUENCE;
		m_uiFramePos = SHM_FRAME_INITIAL_POSITION;
	}

	*puiPos = m_uiFramePos;
	*puiSeq = m_uiFrameSeq;

    m_classSemLock->UnLock();
	return rtn;
}

/*****************************************************************************************************
** DESCRIPTION: move the m_uiFramePos and m_uiFrameSeq to the forward frame   
**     
**
** return:
**/
int ShmRead::MoveForward(uint *puiPos, uint *puiSeq)
{
    int iLen = m_structShmHead->iMaxFrameSize;
    int rtn;
    
    rtn = ReadFrame(NULL, &iLen, NULL, puiPos, puiSeq);
    if(rtn == SHM_ENUM_GET_A_NEW_FRAME)
    {
        return 0;;
    }
    else
    {
        return -1;
    }
}


/*****************************************************************************************************
** DESCRIPTION: set the pos and puiSeq to the newest frame   
**     
**
** return:
**/

int ShmRead::Refresh(uint *puiPos, uint *puiSeq)
{
	struct frame_head_struct *frame;
	int rtn=SHM_ENUM_ERROR;
	m_classSemLock->Lock();
	
	m_uiFramePos = m_structShmHead->uiNewestFramePos;
	m_uiFrameSeq = m_structShmHead->uiNewestFrameSeq;

	frame = (struct frame_head_struct*) (m_pcShm + m_uiFramePos);
	if(frame->uiMagic == m_structShmHead->uiMagic)
	{
		rtn = SHM_ENUM_OK;
	}
	else
	{//lost postion
		SHM_DEBUG("ShmRead::Refresh line=%d, shm fatal error!, no frames\n", __LINE__);
		rtn = SHM_ENUM_ERROR;
	}

	*puiPos = m_uiFramePos;
	*puiSeq = m_uiFrameSeq;

	//printf("++++++++++++m_uiFramePos=%d, m_uiFrameSeq=%d\n", m_uiFramePos, m_uiFrameSeq);
	m_classSemLock->UnLock();

	return rtn;
}


int ShmRead::GetExtras(char *pcExtras, uint uiPos, uint uiSeq)
{
	struct frame_head_struct *frame;
	int rtn=SHM_ENUM_ERROR;

	m_classSemLock->Lock();
	m_uiFramePos = uiPos;
	m_uiFrameSeq = uiSeq;
	
	CheckOverflowed();
	frame = (struct frame_head_struct*) (m_pcShm + m_uiFramePos);
	if(frame->uiMagic == m_structShmHead->uiMagic)
	{
		memcpy(pcExtras, frame->carrExtras, SHM_SIZE_OF_FRAME_EXTRAS);
		rtn = SHM_ENUM_OK;
	}
	else if(m_uiFrameSeq == m_structShmHead->uiNewestFrameSeq + 1)
	{//unfinished frame which is the next frame of newest frame
		memset(pcExtras, 0, SHM_SIZE_OF_FRAME_EXTRAS);
		rtn = SHM_ENUM_NEXT_FRAME_OF_NEWEST;
	}
	else
	{//lost postion
		//SHM_DEBUG("line=%d, this frame have been covered!\n", __LINE__);
		rtn = SHM_ENUM_ERROR;
	}
	m_classSemLock->UnLock();
	
	return rtn;
}

int ShmRead::CheckOverflowed()
{
	struct frame_head_struct *frame, *frame_start;
	if( !IsInitState(m_uiFramePos))
	{
		frame = (struct frame_head_struct*) (m_pcShm + m_uiFramePos);
		frame_start = (struct frame_head_struct*) (m_pcShm + SHM_FRAME_START_POSITION);
		if(frame->uiMagic!= m_structShmHead->uiMagic && m_uiFrameSeq == frame_start->uiSequence)
		{// overflowed: refresh to the start of shm frame postion
			m_uiFramePos = SHM_FRAME_START_POSITION;
			return 1;
		}
		else
		{
			return 0;
		}
	}

	return 0;
}

bool ShmRead::IsInitState(uint uiPos)
{
	return (uiPos == SHM_FRAME_INITIAL_POSITION);
}

