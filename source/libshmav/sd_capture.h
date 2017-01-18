#ifndef _SD_CAPTURE_H_
#define _SD_CAPTURE_H_

#ifdef __cplusplus
extern "C"
{
#endif

// video codec types
typedef enum 
{
    ENUM_SD_VIDEO_CODEC_TYPE_UNKNOWN=0,
    ENUM_SD_VIDEO_CODEC_TYPE_MPEG4=1,
    ENUM_SD_VIDEO_CODEC_TYPE_MJPEG=2,
    ENUM_SD_VIDEO_CODEC_TYPE_H264=3
}ENUM_SD_VIDEO_CODEC_TYPE;

//video quality grades
typedef enum 
{
    ENUM_SD_VIDEO_QUALITY_GRADE_LOWEST =1,
    ENUM_SD_VIDEO_QUALITY_GRADE_LOW    =2,
    ENUM_SD_VIDEO_QUALITY_GRADE_MEDIUM =3,
    ENUM_SD_VIDEO_QUALITY_GRADE_HIGH   =4,
    ENUM_SD_VIDEO_QUALITY_GRADE_HIGHEST=5
}ENUM_SD_VIDEO_QUALITY_GRADE;

//video resolutions
typedef enum 
{
    ENUM_SD_VIDEO_SIZE_TYPE_QQVGA=1,
    ENUM_SD_VIDEO_SIZE_TYPE_QVGA=2,
    ENUM_SD_VIDEO_SIZE_TYPE_HVGA=4,
    ENUM_SD_VIDEO_SIZE_TYPE_VGA=8,
    ENUM_SD_VIDEO_SIZE_TYPE_720P=16,
    ENUM_SD_VIDEO_SIZE_TYPE_2MP=32,
    ENUM_SD_VIDEO_SIZE_TYPE_1080P=64
}ENUM_SD_VIDEO_SIZE_TYPE;


//audio codec types
typedef enum 
{
    ENUM_SD_AUDIO_CODEC_TYPE_UNKNOWN,
    ENUM_SD_AUDIO_CODEC_TYPE_PCM,
    ENUM_SD_AUDIO_CODEC_TYPE_G711_URAW,
}ENUM_SD_AUDIO_CODEC_TYPE;

//audio codec samples per second
typedef enum
{
	ENUM_SD_AUDIO_SAMPLES_8K_PER_SEC,
	ENUM_SD_AUDIO_SAMPLES_16K_PER_SEC,
	ENUM_SD_AUDIO_SAMPLES_44K_PER_SEC //44.1k
}ENUM_SD_AUDIO_SAMPLES_PER_SEC;


// 32 bytes video header info in the share memory
typedef struct tagVideoHeaderInfo
{
	char media_type; // const 1
	char codec;      //check the values in the enum ENUM_SD_VIDEO_CODEC_TYPE
	char resolution; //check the values in the enum ENUM_SD_VIDEO_SIZE_TYPE
	char quality;    //check the values in the enum ENUM_SD_VIDEO_QUALITY_GRADE
	char undef[28];  //undefined 
}VideoHeaderInfo;

// 32 bytes audio header info in the share memory
typedef struct tagAudioHeaderInfo
{
	char media_type;      // const 0
	char codec;           // check the values in the enum ENUM_SD_AUDIO_CODEC_TYPE
	char samples_per_sec; // check the values in the enum ENUM_SD_AUDIO_SAMPLES_PER_SEC
	char undef[29];       // undefined
}AudioHeaderInfo;


/*****************************************************************************************************
** DESCRIPTION: init capture thread for video.
**
**
** Parameters:
**     cProfileNO        : char, profile number for the video stream, 
**                         values: for single stream, just use 1 , for dual-stream, 1 for stream1, 2 for stream2
**     iMaxFrameSize     : int, max frame size for a single frame
**     pstructHeaderInfo : VideoHeaderInfo,  some information about this stream, media type, codec, etc. 
**     funcCapture       : int, a call back function to capture frames
**   
** return:
**  -1: error
**   0: success
*************************************************/
int  VideoInitCapture(char cProfileNO, int iMaxFrameSize, 
	VideoHeaderInfo *pstructHeaderInfo, 
	int (*funcCapture)(char cProfileNO, char *pcBuf, int *piBufLen));


/*****************************************************************************************************
** DESCRIPTION: init capture thread for video.
**
**
** Parameters:
**    cProfileNO        : char, profile number for the audio stream, 
**                        values: single audio stream, so just use 1, dual-stream audio, 1 for stream1, 2 for stream2
**    iMaxFrameSize     : int, max size for a single frame
**    pstructHeaderInfo : VideoHeaderInfo,  some information about this stream, media type, codec, etc. 
**    funcCapture       : int, a call back function to capture frames
**   
** return:
**  -1: error
**   0: success
*************************************************/
int  AudioInitCapture(char cProfileNO, int iMaxFrameSize, 
	AudioHeaderInfo *pstructHeaderInfo, 
	int (*funcCapture)(char cProfileNO, char *pcBuf, int *piBufLen));
#ifdef __cplusplus
}
#endif


#endif

