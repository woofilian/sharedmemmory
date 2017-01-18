#ifndef __SHM_READ_CLASS_H__
#define __SHM_READ_CLASS_H__

#include "shm_base_class.h"


class ShmRead: public ShmBase
{
	public:
		ShmRead(key_t shmKey, key_t semKey,  int iShmSize);
		ShmRead(const ShmRead &refShmRead);
		virtual ~ShmRead();

		int ReadFrame(char* pcBuf, int* piBufLen, char *pcExtras, uint *puiPos, uint *puiSeq);	
		int Distance2Newest(uint uiPos, uint uiSeq);
		int Refresh(uint *puiPos, uint *puiSeq);
		int MoveBack(uint *puiPos, uint *puiSeq); 
		int MoveForward(uint *puiPos, uint *puiSeq);
		int GetExtras(char *pcExtras, uint uiPos, uint uiSeq);
		int CheckOverflowed();
		bool IsInitState(uint uiPos);
};


#endif /*__SHM_READ_CLASS_H__*/
