#ifndef __SHM_BASE_CLASS_H__
#define __SHM_BASE_CLASS_H__

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>

#include "shm_common.h"

#define SHM_FRAME_INITIAL_POSITION 0
#define SHM_FRAME_INITIAL_SEQUENCE 0

#define SHM_SIZE_OF_USER_DEFINED_INFO	sizeof(ShmVideoHeaderInfo) //32 bytes
#define SHM_SIZE_OF_FRAME_EXTRAS		sizeof(ShmVideoExtraData)
#define SHM_FRAME_MAX_SEQUENCE			4294967295  //max unsigned int
#define SHM_FRAME_MAGIC_NUM				(0x7F7E7D7C)

typedef unsigned char uchar;
typedef unsigned int  uint;

enum OBJECT_STATUES
{
	OBJECT_STATUES_ERROR=-1,
	OBJECT_STATUES_NORMAL=0
};

enum SHM_READ_WRITE_FLAG
{
	SHM_FLAG_READ,
	SHM_FLAG_WRITE,
};

//shm head, total 52bytes
typedef struct shm_head_struct
{
	uint uiMagic;  
	uint uiNewestFramePos;
	uint uiNewestFrameSeq;
	int iMaxFrameSize; 
	int iShmSize;
	char carrUserDefinedInfo[SHM_SIZE_OF_USER_DEFINED_INFO]; //32 bytes
}ShmHeadStruct;

//shm frame head
typedef struct frame_head_struct
{
	uint uiMagic;
	uint uiPrevFramePos;
	uint uiNextFramePos; 
	uint uiSequence;
	int iFrameSize;
	char carrExtras[SHM_SIZE_OF_FRAME_EXTRAS];
}ShmFrameHeadStruct;

#define SHM_SIZE_OF_SHM_HEAD			sizeof(ShmHeadStruct)
#define SHM_SIZE_OF_FRAME_HEAD			sizeof(ShmFrameHeadStruct)
#define SHM_FRAME_START_POSITION		SHM_SIZE_OF_SHM_HEAD


class SemLock
{
	public:
		SemLock(key_t semKey);
		~SemLock();
		void Lock();
		void UnLock();
		int GetObjectStatus();
	private:
		int m_iSemID;
		int m_iObjectStatus; 
		struct sembuf m_structLockOP[2];
		struct sembuf m_structUnLockOP[1];
};

class ShmBase
{
	public:
		ShmBase(key_t shmKey, key_t semKey, int iShmSize, SHM_READ_WRITE_FLAG enumShmFlag);
		ShmBase(const ShmBase &refShmBase); // ref short for  refferrece
		virtual ~ShmBase();
		int InitShm();
		int DetachShm();

		int SetHeaderInfo(char pcHeaderInfo[SHM_SIZE_OF_USER_DEFINED_INFO]);
		int GetHeaderInfo(char pcHeaderInfo[SHM_SIZE_OF_USER_DEFINED_INFO]);
		int GetMaxFrameSize();
		int GetObjectStatus();

	protected:
		char* m_pcShm;
		int m_iShmSize;
		uint m_uiFramePos;
		uint m_uiFrameSeq;

		key_t m_ShmKey;
		key_t m_SemKey;

		char m_iObjectStatus;
		SHM_READ_WRITE_FLAG m_enumShmFlag; 

		SemLock *m_classSemLock;
		ShmHeadStruct *m_structShmHead;
};

#endif /*__SHM_BASE_CLASS_H__*/
