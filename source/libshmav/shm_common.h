#ifndef __SHM_COMMON_H__
#define __SHM_COMMON_H__

#define SHM_VIDEO_DEVICE_BASE_KEY 17830186
#define SHM_AUDIO_DEVICE_BASE_KEY 54607402
#define SHM_KEY_MULTI_BASE 10000

#define SEM_VIDEO_DEVICE_BASE_KEY (SHM_VIDEO_DEVICE_BASE_KEY + 1)
#define SEM_AUDIO_DEVICE_BASE_KEY (SHM_AUDIO_DEVICE_BASE_KEY + 1)
#define SEM_KEY_MULTI_BASE 10100

//#define SHM_VIDEO_STREAM_STORAGE_720P_SIZE (5*1024*1024 + 512*1024) //5.5M 1280x720
//#define SHM_VIDEO_STREAM_PREVIEW_720P_SIZE (3*1024*1024 + 512*1024) //3.5M 1280x720
#define SHM_VIDEO_STREAM_720P_SIZE (3*1024*1024 + 512*1024) //3.5M 1280x720
#define SHM_VIDEO_STREAM_VGA_SIZE  (1*1024*1024 + 256*1024)
//#define SHM_VIDEO_STREAM_QVGA_SIZE (300*1024)

#define SHM_VIDEO_STREAM_720P_MAX_FRAME_SIZE (256*1024) //500k for 720P
#define SHM_VIDEO_STREAM_VGA_MAX_FRAME_SIZE  (100*1024) //100k for VGA
//#define SHM_VIDEO_STREAM_QVGA_MAX_FRAME_SIZE (40*1024)  //40k for QVGA

typedef enum
{
	SHM_ENUM_FRAME_TYPE_VIDEO_MJPEG_FRAME=0,
	SHM_ENUM_FRAME_TYPE_VIDEO_H264_IFRAME=1,
	SHM_ENUM_FRAME_TYPE_VIDEO_H264_PFRAME=2,
	SHM_ENUM_FRAME_TYPE_VIDEO_H264_BFRAME=3,
	SHM_ENUM_FRAME_TYPE_AUDIO_PCM_8K_FRAME=100,
	SHM_ENUM_FRAME_TYPE_AUDIO_PCM_16K_FRAME=101
}SHM_ENUM_FRAME_TYPE;

typedef enum 
{
	SHM_ENUM_VIDEO_CODEC_TYPE_UNKNOWN=0,
	SHM_ENUM_VIDEO_CODEC_TYPE_MPEG4=1,
	SHM_ENUM_VIDEO_CODEC_TYPE_MJPEG=2,
	SHM_ENUM_VIDEO_CODEC_TYPE_H264=3,
	SHM_ENUM_VIDEO_CODEC_TYPE_YUV=4
}SHM_ENUM_VIDEO_CODEC_TYPE;

typedef enum 
{
	SHM_ENUM_VIDEO_STREAM0_PROFILE=0, 
	SHM_ENUM_VIDEO_STREAM1_PROFILE=1,  
	SHM_ENUM_VIDEO_STREAM2_PROFILE=2,   
	SHM_ENUM_VIDEO_STREAM_AES_PROFILE = 3
}SHM_ENUM_VIDEO_STREAM_PROFILE;

typedef enum 
{
	SHM_ENUM_AUDIO_AAC_PROFILE=0,
	SHM_ENUM_AUDIO_8K_PROFILE=1,
	SHM_ENUM_AUDIO_16K_PROFILE=2,
	SHM_ENUM_AUDIO_TALKBACK_PROFILE=3,
	SHM_ENUM_AUDIO_8K_AES_PROFILE=4,
	SHM_ENUM_AUDIO_8K_PROFILE_1=5,
}SHM_ENUM_AUDIO_PROFILE;

typedef enum 
{
	SHM_ENUM_MEDIA_TYPE_AUDIO=0,
	SHM_ENUM_MEDIA_TYPE_VIDEO=1
}SHM_ENUM_MEDIA_TYPE;

typedef enum
{
	SHM_ENUM_VIDEO_FRAME_TYPE_NORMAL=0,
	SHM_ENUM_VIDEO_FRAME_TYPE_KEY=1
}SHM_ENUM_VIDEO_FRAME_TYPE;

typedef enum
{
	SHM_ENUM_AUDIO_8K_PER_SEC,
	SHM_ENUM_AUDIO_16K_PER_SEC,
	SHM_ENUM_AUDIO_44K_PER_SEC, //44.1k
	SHM_ENUM_AUDIO_48K_PER_SEC  //48k
}SHM_ENUM_AUDIO_SAMPLES_PER_SEC;

typedef enum 
{
	SHM_ENUM_EMPTY_SPACE=-3,  //the shm have no data, it is an empty space
	SHM_ENUM_WRITE_OVERFLOW=-2,
	SHM_ENUM_NO_FRAME=-2,
	SHM_ENUM_ERROR=-1,
	SHM_ENUM_OK=0,
	SHM_ENUM_NO_NEW_FRAME=0, 
	SHM_ENUM_FRAME_TOO_LARGE=1,
	SHM_ENUM_GET_A_NEW_FRAME=2,
	SHM_ENUM_GET_A_NEW_KEY_FRAME=3,
	SHM_ENUM_NEXT_FRAME_OF_NEWEST=4,
	SHM_ENUM_ID1_NOT_EXIT = 10000000,
	SHM_ENUM_ID2_NOT_EXIT = 10000001,
	SHM_ENUM_ID1_ID2_NOT_EXIT = 10000002,
}SHM_ENUM_RETURN_CODE;

// 32 bytes video header info in the share memory
typedef struct tagShmVideoHeaderInfo
{
	unsigned char ucMediaType;  //SHM_ENUM_MEDIA_TYPE
	unsigned char ucCodec;      //SHM_ENUM_VIDEO_CODEC_TYPE
	unsigned char ucResolution; //
	unsigned char ucQuality;    //
	unsigned char ucArrReserve[28];   
}ShmVideoHeaderInfo;

// 32 bytes audio header info in the share memory
typedef struct tagShmAudioHeaderInfo
{
	unsigned char ucMediaType; //video/audio flage
	unsigned char ucCodec;
	unsigned char ucSamplesPerSec; //-k: for example 8 for 8k, 16 for 16k  
	unsigned char ucArrReserve[29];
}ShmAudioHeaderInfo;

//NOTICE: remember that the size of ShmVideoExtraData and ShmAudioExtraData should keep the same
// 20 bytes video extra data  
typedef struct tagShmVideoExtraData
{
	unsigned long long ullTimeStamp; //8 bytes 
	int nbitsRes;      // resolution in every frame packet.;
	unsigned char ucArrReserve1[7];   // undefined data, remember for align 8 bytes.

	unsigned char ucKeyFrameFlag;  // 0 for normal frame, 1 for key frame
	//unsigned char ucAudioEnable;
	//unsigned char ucAudioChangedCount;
}ShmVideoExtraData;

// 20 bytes audio extra data, 
// remember that the size of ShmVideoExtraData and ShmAudioExtraData should keep the same
typedef struct tagShmAudioExtraData
{
	unsigned long long ullTimeStamp; //8 bytes

	unsigned char ucArrReserve[sizeof(ShmVideoExtraData)-8]; 
}ShmAudioExtraData;


//#define DEBUG

#ifdef DEBUG
#define SHM_DEBUG(...) \
    do{\
        fprintf(stderr, " DBG(%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); \
        fprintf(stderr, __VA_ARGS__);\
    }while(0)
#else
#define SHM_DEBUG(...)
#endif


#endif /*__SHM_COMMON_H__*/
