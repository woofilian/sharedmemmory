#ifndef __AED_h__
#define __AED_h__ 

// -------------------------------------------------
// Description : Get pointer of version string
// Return : 
// -------------------------------------------------
int  GM_AED_ver(const char ** pVersion);

// --------------------------------------------------
// Description: initialize GM_Audio_Detector library, 
// responsible for memory allocation
// Input :
//        debug : Recording Option , 1: Open recording file 0: Don¡¦t record
// Return : 0 (Success)
//        100 (wrong platform, 813x series please)
//        101 (fail to allocate memory space)
//        102 (fail to open file)
//        103 (wrong input size)
//        104 (wrong sampling rate)
// --------------------------------------------------
int  GM_AED_Init(int inSize , int SampleRate , int debug);

// --------------------------------------------------
// Description: Main program
//Input :
//        AED_status : 0 : No Audio event ; 1 : Audio event
//        Pwr_Sensitive : 0~10
//        Signal_Variation : 1~3
// Return: 0  (Success)
//        200  (Can¡¦t save recording file)
//        201 (fail to set sensitive level)
//        202 (fail to set correlation level)
// --------------------------------------------------
int  GM_AED_Main(short *v ,  int *AED_status , int Pwr_Sensitive , int Signal_Variation) ;

// --------------------------------------------------
// Description: Responsible for releasing allocated 
// memory space.
// Return: 0 (Success)
//			300 (fail to free memory space)
// --------------------------------------------------
int GM_AED_End(void);
 
#endif

