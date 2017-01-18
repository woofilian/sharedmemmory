#ifndef __SHM_WRITE_CLASS_H__
#define __SHM_WRITE_CLASS_H__

#include "shm_base_class.h"

class ShmWrite: public ShmBase
{
public:
	ShmWrite(key_t shmKey, key_t semKey,  int iShmSize, int iMaxFrameSize, char pcHeaderInfo[32]);
	virtual ~ShmWrite();
	int WriteFrame(char* pcBuf, int iBufLen, char *pcExtras);
};


#endif /*__SHM_WRITE_CLASS_H__*/
