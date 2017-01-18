#include "shm_base_class.h"
#include <stdlib.h>


SemLock::SemLock(key_t semKey)
{
	m_structLockOP[0].sem_num = 0;   
	m_structLockOP[0].sem_op = 0;   
	m_structLockOP[0].sem_flg = 0;  
	m_structLockOP[1].sem_num = 0;   
	m_structLockOP[1].sem_op = 1;   
	m_structLockOP[1].sem_flg = SEM_UNDO;   

	m_structUnLockOP[0].sem_num = 0;   
	m_structUnLockOP[0].sem_op = -1;   
	m_structUnLockOP[0].sem_flg = (IPC_NOWAIT|SEM_UNDO);

	if((m_iSemID=semget(semKey, 1, IPC_CREAT)) == -1)
	{
		perror("LockSem() Get semaphore error");
		m_iObjectStatus = OBJECT_STATUES_ERROR;
	}
	else
	{
		m_iObjectStatus = OBJECT_STATUES_NORMAL;
	}
}

SemLock::~SemLock()
{
}

void SemLock::Lock()
{
	if(semop(m_iSemID,&m_structLockOP[0],2)==-1)
	{
		perror("LockSem() Lock semaphore error");
	}
}

void SemLock::UnLock()
{
	if(semop(m_iSemID,&m_structUnLockOP[0],1)==-1)
	{
		perror("LockSem() Lock semaphore error");
	}
}

int SemLock::GetObjectStatus()
{
	return m_iObjectStatus;
}

/*****************************************************************************************************
** DESCRIPTION: constructor to create the Shm object. if create failed, 
**     will set m_iObjectStatus=OBJECT_STATUES_ERROR
**
** Parameters:
**     shmKey       : key_t, a unique key for create the share memory.
**     semKey       : key_t, a unique key for create the sempher(used for process synchronization)
**     iShmSize     : int, assign the share memory size 
**     enumShmFlag  : flag to audio or video shm
**/
ShmBase::ShmBase(key_t shmKey, key_t semKey, int iShmSize, SHM_READ_WRITE_FLAG enumShmFlag)
{
	m_ShmKey = shmKey;
	m_SemKey = semKey;
	m_iShmSize = iShmSize;
	m_enumShmFlag = enumShmFlag;

	InitShm();
}

ShmBase::ShmBase(const ShmBase &refShmBase)
{
	m_iShmSize = refShmBase.m_iShmSize;
	m_uiFramePos = refShmBase.m_uiFramePos;
	m_uiFrameSeq = refShmBase.m_uiFrameSeq;
	m_ShmKey   = refShmBase.m_ShmKey;
	m_SemKey   = refShmBase.m_SemKey;
	m_enumShmFlag  = refShmBase.m_enumShmFlag;
	InitShm();

    if(m_iObjectStatus == OBJECT_STATUES_NORMAL)
	    printf("+++++++copy shmbase m_uiFramePos=%d, m_uiFrameSeq=%d\n\n", m_uiFramePos, m_uiFrameSeq);
}

ShmBase::~ShmBase()
{
	DetachShm();
}


int ShmBase::InitShm()
{
	int shm_id;
	m_classSemLock = new SemLock(m_SemKey);
	if(m_classSemLock->GetObjectStatus()== OBJECT_STATUES_ERROR)
	{
		m_iObjectStatus = OBJECT_STATUES_ERROR;
	}
	else
	{
		m_classSemLock->Lock();
		if(m_enumShmFlag == SHM_FLAG_WRITE)
		{//create the shm for write
			shm_id = shmget(m_ShmKey, m_iShmSize, IPC_CREAT);
		}
		else
		{//reference the shm for read, if no one create the shm, it will return false
			shm_id = shmget(m_ShmKey, m_iShmSize, 0);
		}
		
        if(shm_id==-1)
        {
            SHM_DEBUG("Shm shmget error\n");
            m_iObjectStatus = OBJECT_STATUES_ERROR;
        }
		else
		{
			//printf("1..shm_id=%d\n", shm_id);
			m_pcShm = (char*)shmat(shm_id, NULL, 0);
			m_structShmHead= (ShmHeadStruct *)m_pcShm;	
			SHM_DEBUG("shm-----shmKey=%d, iShmSize=%d, address: %08x\n", m_ShmKey, m_iShmSize, m_pcShm);
			m_iObjectStatus = OBJECT_STATUES_NORMAL;
		}
		m_classSemLock->UnLock();
	}

	return 0;
}

int ShmBase::GetHeaderInfo(char pcHeaderInfo[32])
{
	memcpy(pcHeaderInfo, m_structShmHead->carrUserDefinedInfo, SHM_SIZE_OF_USER_DEFINED_INFO);
	return 0;
}

int ShmBase::SetHeaderInfo(char pcHeaderInfo[32])
{
	memcpy(m_structShmHead->carrUserDefinedInfo, pcHeaderInfo, SHM_SIZE_OF_USER_DEFINED_INFO);
	return 0;
}

int ShmBase::GetMaxFrameSize()
{
	return m_structShmHead->iMaxFrameSize;
}

int ShmBase::GetObjectStatus()
{
	return m_iObjectStatus;
}

int ShmBase::DetachShm()
{
    if(m_iObjectStatus == OBJECT_STATUES_NORMAL)
    {
    	if(shmdt(m_pcShm)==-1)
    		printf(" detach error ");
    }
}


