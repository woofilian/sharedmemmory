
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <pthread.h>
#include <sys/time.h>
#include "gmlib.h"

#include "arr_to_osd.c"

#include "log.h"
#define FONT_PATH_ASC32	"/font/ASC32"	
//#define FONT_PATH_HZK32	"/font/HZK32"
#define FONT_PATH_HZK32	"/mnt/nfs/HZK32"
#define FONT_PATH_ASC24	"/font/ASC24"	
#define FONT_PATH_HZK24	"/font/HZK24F"
#define FONT_PATH_HZK16	"/font/HZK16"
#define FONT_PATH_ASC16	"/font/ASC16"			
#define FONT_PATH_ASC12	"/font/ASC12"	
#define FONT_PATH_HZK12	"/font/HZK12"


#define OSG_LOGO_RGB1555_PATTERN     "osg_16x72_rgb1555.bin"
#define OSG_LOGO_WIDTH               16
#define OSG_LOGO_HEIGHT              72

#define BITSTREAM_LEN           (720 * 576 * 3 / 2)
#define MAX_BITSTREAM_NUM   1



 
 typedef struct {
	 int font;
	 unsigned short fbitmap[18];   //Font: 12x18, its format is (12bits(MSB) + 4bits_reserve(LSB)) x 18
 } my_font_data_t;
#define OSD_LINE_PER_FONT         (18)
#define OSD_BIT_PER_LINE          (12)
 
 
#define BITSTREAM_LEN       (720 * 576 * 3 / 2)
 
 gm_system_t gm_system;
 void *groupfd;  // return of gm_new_groupfd() for main & sub stream
 void *main_bindfd, *sub_bindfd;	// return of gm_bind()
 void *main_capture_object, *sub_capture_object;
 void *main_enc_object, *sub_enc_object;

 int enc_exit = 0;	 // Notify program exit
 int g_fd_zk = -1;
void update_osd_with_hzk(void *capture_obj,int sizes ,int win_idx, int font_idx, int hz_n,int enable,unsigned char incode[30],int _x, int _y)
#if 0
{
/*
#define sizes 32            //字库大小
#define Horizontal_N 8		//汉字横向字数个数
#define Vertical_N   1		//汉字纵向(也就是行数)

//如果整除刚好，非整除得 + 1
#define FONT_V sizes*Vertical_N/18 + !!((sizes*Vertical_N)%18)  //OSD font vertical count
#define FONT_H sizes*Horizontal_N/12+!!((sizes*Horizontal_N)%12) //OSD font horizontal count
*/
	//int sizes = 32;            //字库大小
	int Horizontal_N =hz_n;//8;	//汉字横向字数个数
	int Vertical_N   =1;		//汉字纵向(也就是行数)
	int FONT_V = sizes*Vertical_N/18 + !!((sizes*Vertical_N)%18);  //OSD font vertical count
	int FONT_H = sizes*Horizontal_N/12+!!((sizes*Horizontal_N)%12); //OSD font horizontal count
	int hzk_fd;	
	unsigned char qh = 0, wh = 0; 
	unsigned long offset = 0;
	unsigned char mat[sizes][sizes/8];
	unsigned char matasc[sizes][(sizes/8 + 1) / 2];
    gm_osd_font_update_t new_font;
    gm_osd_font2_t osd_font;
	char hz[32];
	int i,j,k,m,n=0;
	int hz_len = strlen(incode);
	int v_words = FONT_V;
	int h_words = FONT_H;
	int vertical_hz_n = Vertical_N;
	int horizonl_hz_n = Horizontal_N;
	int v_sz = sizes;
	int h_sz = sizes;
	int ret =0; 
	int idx = 0;
	int be_newline = 0;	
	my_font_data_t hzk[FONT_V][FONT_H];
	unsigned char hz_arr[sizes*Vertical_N][sizes/8*Horizontal_N]; //汉字存储空间
	
	//printf("hz_len is %d, FONT_V: %d, FONT_H:%d\n", hz_len, FONT_V, FONT_H);
	memset(hzk, 0, sizeof(hzk));
	//printf("font_idx : %d\n", font_idx);
	
	osd_font.font_index = (unsigned short *) malloc(v_words* h_words* sizeof(unsigned short));
    if (osd_font.font_index == NULL) 
	{
        printf("Error to malloc memory!\n"); 
		exit(0);
    }

	for(m=0;m<hz_len;)
	{
	if(incode[m] & 0x80)// 判断是否是汉字，常用的0-127超过之后
	{
		//printf("这是汉字\n");
		h_sz = sizes;
		
		qh = incode[m] - 0xa0;
		wh = incode[m+1] - 0xa0; 

		sprintf(hz , "/home/font/HZK%d" , v_sz);
		offset = (94*(qh-1)+(wh-1))*(v_sz*h_sz/8); 
		hzk_fd = open(hz , O_RDONLY);
		if(hzk_fd < 0) {  
			printf("Can't Open hzk%d  - %d\n" , v_sz, hzk_fd);  
			return; 
		}

		lseek(hzk_fd, offset, SEEK_SET); 
		read(hzk_fd, mat, v_sz*h_sz/8);
		close(hzk_fd); 
		for(i=0;i<v_sz;i++){
			if( ((idx)%(sizes/8*horizonl_hz_n) + h_sz/8) <=  sizes/8*horizonl_hz_n){
				memcpy(&hz_arr[i+v_sz*(idx/(sizes/8*horizonl_hz_n))][(idx)%(sizes/8*horizonl_hz_n)],mat[i], h_sz/8);
				be_newline = 0;
			}else{	
				memcpy(&hz_arr[i+v_sz*(idx/(sizes/8*horizonl_hz_n))][(idx)%(sizes/8*horizonl_hz_n)],mat[i], h_sz/8*horizonl_hz_n - (idx)%(sizes/8*horizonl_hz_n));
				be_newline = 1;
			}
		}
			
#if 0
		for(i=0; i<v_sz; i++){  
			for(j=0; j<h_sz/8; j++) {  
				for(k=0; k<8 ; k++) {   
					if( hz_arr[i+v_sz*(idx/(sizes/8*horizonl_hz_n))][(idx)%(sizes/8*horizonl_hz_n)+j] & (0x80>>k) ){
						printf("*");
					} else 	{ 
						printf(" ");
					} 
				}
			}
			printf("\n");
		}
#endif

		m+=2;
		if(be_newline){
			idx += (h_sz/8 )/2;
		}else{
			idx += h_sz/8;
		}
	}else{
		//printf("这是英文字母\n");
		h_sz = ((sizes/8+1)/2)*8;
		//printf("A: h_sz : %d\n", h_sz);
		
		sprintf(hz , "/home/font/ASCII%d" , v_sz);
		offset = incode[m]*v_sz*h_sz/8; 
		hzk_fd = open(hz , O_RDONLY);
		if(hzk_fd < 0) {  
			printf("Can't Open ASCII%d  - %d\n" , v_sz, hzk_fd);  
			return; 
		}

		lseek(hzk_fd, offset, SEEK_SET); 
		read(hzk_fd, matasc, v_sz*h_sz/8);
		close(hzk_fd); 
		for(i=0;i<v_sz;i++)
		{
			memcpy(&hz_arr[i+v_sz*(idx/(sizes/8*horizonl_hz_n))][(idx)%(sizes/8*horizonl_hz_n)],matasc[i], h_sz/8);
			
		}
#if 1
		for(i=0; i<v_sz; i++){  
			for(j=0; j<h_sz/8; j++) {  
				for(k=0; k<8 ; k++) {   
					if( hz_arr[i+v_sz*(idx/(sizes/8*horizonl_hz_n))][idx%(sizes/8*horizonl_hz_n)+j] & (0x80>>k) ){
					//if(matasc[i][j] & (0x80>>k) ){
						//printf("*");
					} else 	{ 
						//printf(" ");
					} 
				}
			}
			//printf("\n");
		}
#endif
		
		m++;
		idx += h_sz/8;

	}

}


#if 0 //printf hz_arr
	for(i=0;i<v_sz*vertical_hz_n;i++){
			for(j=0;j<sizes/8*horizonl_hz_n;j++){
				for(k=0;k<8;k++){
					if(hz_arr[i][j] & (0x80>>k)){					
						printf("*");
					} else	{ 
						printf(" ");
					} 
				}
			}
			printf("\n");
		}
#endif


	for(i=0;i<v_sz*vertical_hz_n;i++){
		for( j=0;j<(sizes*horizonl_hz_n/8);j+=3){//自增3个字节
			if( ((sizes/8*horizonl_hz_n)%3 == 1) && (j==(sizes/8*horizonl_hz_n-1))){ //一次性处理3个字节就是2个font宽
				hzk[i/18][j*8/12].fbitmap[i%18] = hz_arr[i][j]<<4;
				
			}else if( ((sizes/8*horizonl_hz_n)%3 == 2) && (j==(sizes/8*horizonl_hz_n-2))){
				hzk[i/18][j*8/12].fbitmap[i%18]=(hz_arr[i][j]<<4)|((hz_arr[i][j+1]&0xf0)>>4);
				hzk[i/18][j*8/12+1].fbitmap[i%18] = (hz_arr[i][j+1]&0xf)<<8; 

			}else{			
				hzk[i/18][j *8/12].fbitmap[i%18] = (hz_arr[i][j]<<4) | ((hz_arr[i][j+1]&0xf0)>>4);
				hzk[i/18][j*8/12+1].fbitmap[i%18]=(( hz_arr[i][j+1]&0xf )<<8)| ( hz_arr[i][j+2] );
			}
		}
	}

	for(i = 0;i<v_words; i++){
		for(j = 0;j<h_words;j++){
			hzk[i][j].font = font_idx + i*h_words+j;//fontidx;

			//printf("font_idx:%d set hzk[%d][%d].font: %d\n", font_idx, i,j, hzk[i][j].font);
#if 0

			for(k=0;k<GM_OSD_FONT_MAX_ROW;k++)
			{	
				for(n=0;n<16;n++){
					if( hzk[i][j].fbitmap[k] & (0x8000>>n) ){
						printf("*");
					} else	{ 
						printf(" ");
					} 
				}
				printf("\n");
			}
#endif
		}
	}
	//printf("======================================================\n");

	for(i=0;i<v_words;i++){
		for(j=0;j<h_words;j++){
			new_font.font_idx = hzk[i][j].font;
			//printf("hzk[%d][%d].font : %d\n", i,j,new_font.font_idx);
			for (k = 0; k < 18; k++) {
				new_font.bitmap[k] = ( hzk[i][j].fbitmap[k]>>4 )
							| ( (hzk[i][j].fbitmap[k]&0xf) <<12 );
#if 0
				for(n=0;n<16;n++){
						if( new_font.bitmap[k] & (0x8000>>n) ){
							printf("*");
						} else	{ 
							printf(" ");
						} 
					}
				printf("\n");
#endif 
			}
			gm_update_new_font(&new_font);
		}
	}

	/** setup OSD font */	
    osd_font.enabled = enable;
    osd_font.win_idx = win_idx;
    osd_font.align_type = GM_OSD_ALIGN_TOP_LEFT;
    osd_font.x = _x;
    osd_font.y = _y;
    osd_font.h_words = h_words;  ///< The horizontal number of words of OSD window
    osd_font.v_words = v_words;   ///< The vertical number of words of OSD window
    osd_font.h_space = 0;   ///< The vertical space between charater and charater
    osd_font.v_space = 0;   ///< The horizontal space between charater and charater

	osd_font.font_index_len = v_words*h_words;
	for(i=0;i<v_words*h_words;i++){
		osd_font.font_index[i] = (unsigned short)(font_idx + i);
	}
    
	osd_font.font_alpha = GM_OSD_FONT_ALPHA_75;
    osd_font.win_alpha = GM_OSD_FONT_ALPHA_25;
    osd_font.win_palette_idx = 0;
	if(0 == strcmp(incode,"Rec "))
	{
		 osd_font.win_alpha = GM_OSD_FONT_ALPHA_0;
		 osd_font.font_palette_idx = 2;
	}
	else
	{
		 osd_font.font_palette_idx = 1;
	}
    osd_font.priority = GM_OSD_PRIORITY_MARK_ON_OSD;
    osd_font.smooth.enabled = 1;
    osd_font.smooth.level = GM_OSD_FONT_SMOOTH_LEVEL_WEAK;
    osd_font.marquee.mode = GM_OSD_MARQUEE_MODE_NONE;
    osd_font.border.enabled = 0;
    osd_font.border.width = 0;
    osd_font.border.type = GM_OSD_BORDER_TYPE_WIN;
    osd_font.border.palette_idx = 3;
    osd_font.font_zoom = GM_OSD_FONT_ZOOM_NONE;
    ret = gm_set_osd_font2(capture_obj, &osd_font); 
	//printf("ret : %d\n", ret);
	
	if (osd_font.font_index != NULL)
        free(osd_font.font_index);
}
#else
{
/*
#define sizes 32            //字库大小
#define Horizontal_N 8		//汉字横向字数个数
#define Vertical_N   1		//汉字纵向(也就是行数)

//如果整除刚好，非整除得 + 1
#define FONT_V sizes*Vertical_N/18 + !!((sizes*Vertical_N)%18)  //OSD font vertical count
#define FONT_H sizes*Horizontal_N/12+!!((sizes*Horizontal_N)%12) //OSD font horizontal count
*/
	//int sizes = 32;            //字库大小
	int Horizontal_N =hz_n;//8;	//汉字横向字数个数
	int Vertical_N   =1;		//汉字纵向(也就是行数)
	int FONT_V = sizes*Vertical_N/18 + !!((sizes*Vertical_N)%18);  //OSD font vertical count
	int FONT_H = sizes*Horizontal_N/12+!!((sizes*Horizontal_N)%12); //OSD font horizontal count
	int hzk_fd;	
	unsigned char qh = 0, wh = 0; 
	unsigned long offset = 0;
	unsigned char mat[sizes][sizes/8];
	unsigned char matasc[sizes][(sizes/8 + 1) / 2];
    gm_osd_font_update_t new_font;
    gm_osd_font2_t osd_font;
	char hz[32];
	int i,j,k,m,n=0;
	int hz_len = strlen(incode);
	int v_words = FONT_V;
	int h_words = FONT_H;
	int vertical_hz_n = Vertical_N;
	int horizonl_hz_n = Horizontal_N;
	int v_sz = sizes;
	int h_sz = sizes;
	int ret =0; 
	int idx = 0;
	int be_newline = 0;	
	my_font_data_t hzk[FONT_V][FONT_H];
	unsigned char hz_arr[sizes*Vertical_N][sizes/8*Horizontal_N]; //汉字存储空间
	
	//printf("hz_len is %d, FONT_V: %d, FONT_H:%d\n", hz_len, FONT_V, FONT_H);
	memset(hzk, 0, sizeof(hzk));
	//printf("font_idx : %d\n", font_idx);
	
	osd_font.font_index = (unsigned short *) malloc(v_words* h_words* sizeof(unsigned short));
    if (osd_font.font_index == NULL) 
	{
        printf("Error to malloc memory!\n"); 
		exit(0);
    }

	for(m=0;m<hz_len;)
	{
	if(incode[m] & 0x80)// 判断是否是汉字，常用的0-127超过之后
	{
		//printf("这是汉字\n");
		h_sz = sizes;
		
		qh = incode[m] - 0xa0;
		wh = incode[m+1] - 0xa0; 

		sprintf(hz , "/home/font/HZK%d" , v_sz);
		offset = (94*(qh-1)+(wh-1))*(v_sz*h_sz/8); 
		hzk_fd = open(hz , O_RDONLY);
		if(hzk_fd < 0) {  
			printf("Can't Open hzk%d  - %d\n" , v_sz, hzk_fd);  
			return; 
		}

		lseek(hzk_fd, offset, SEEK_SET); 
		read(hzk_fd, mat, v_sz*h_sz/8);
		close(hzk_fd); 
		for(i=0;i<v_sz;i++){
			if( ((idx)%(sizes/8*horizonl_hz_n) + h_sz/8) <=  sizes/8*horizonl_hz_n){
				memcpy(&hz_arr[i+v_sz*(idx/(sizes/8*horizonl_hz_n))][(idx)%(sizes/8*horizonl_hz_n)],mat[i], h_sz/8);
				be_newline = 0;
			}else{	
				memcpy(&hz_arr[i+v_sz*(idx/(sizes/8*horizonl_hz_n))][(idx)%(sizes/8*horizonl_hz_n)],mat[i], h_sz/8*horizonl_hz_n - (idx)%(sizes/8*horizonl_hz_n));
				be_newline = 1;
			}
		}
			
#if 0
		for(i=0; i<v_sz; i++){  
			for(j=0; j<h_sz/8; j++) {  
				for(k=0; k<8 ; k++) {   
					if( hz_arr[i+v_sz*(idx/(sizes/8*horizonl_hz_n))][(idx)%(sizes/8*horizonl_hz_n)+j] & (0x80>>k) ){
						printf("*");
					} else 	{ 
						printf(" ");
					} 
				}
			}
			printf("\n");
		}
#endif

		m+=2;
		if(be_newline){
			idx += (h_sz/8 )/2;
		}else{
			idx += h_sz/8;
		}
	}
	else{
		//printf("这是英文字母\n");
		h_sz = ((sizes/8+1)/2)*8;
		//printf("A: h_sz : %d\n", h_sz);
		
		sprintf(hz , "/home/font/ASCII%d" , v_sz);
		offset = incode[m]*v_sz*h_sz/8; 
		hzk_fd = open(hz , O_RDONLY);
		if(hzk_fd < 0) {  
			printf("Can't Open ASCII%d  - %d\n" , v_sz, hzk_fd);  
			return; 
		}

		lseek(hzk_fd, offset, SEEK_SET); 
		read(hzk_fd, matasc, v_sz*h_sz/8);
		close(hzk_fd); 
		for(i=0;i<v_sz;i++)
		{
			memcpy(&hz_arr[i+v_sz*(idx/(sizes/8*horizonl_hz_n))][(idx)%(sizes/8*horizonl_hz_n)],matasc[i], h_sz/8);
			
		}
#if 1
		for(i=0; i<v_sz; i++){  
			for(j=0; j<h_sz/8; j++) {  
				for(k=0; k<8 ; k++) {   
					if( hz_arr[i+v_sz*(idx/(sizes/8*horizonl_hz_n))][idx%(sizes/8*horizonl_hz_n)+j] & (0x80>>k) ){
					//if(matasc[i][j] & (0x80>>k) ){
						//printf("*");
					} else 	{ 
						//printf(" ");
					} 
				}
			}
			//printf("\n");
		}
#endif
		
		m++;
		idx += h_sz/8;

	}

}


#if 0 //printf hz_arr
	for(i=0;i<v_sz*vertical_hz_n;i++){
			for(j=0;j<sizes/8*horizonl_hz_n;j++){
				for(k=0;k<8;k++){
					if(hz_arr[i][j] & (0x80>>k)){					
						printf("*");
					} else	{ 
						printf(" ");
					} 
				}
			}
			printf("\n");
		}
#endif


	for(i=0;i<v_sz*vertical_hz_n;i++){
		for( j=0;j<(sizes*horizonl_hz_n/8);j+=3){//自增3个字节
			if( ((sizes/8*horizonl_hz_n)%3 == 1) && (j==(sizes/8*horizonl_hz_n-1))){ //一次性处理3个字节就是2个font宽
				hzk[i/18][j*8/12].fbitmap[i%18] = hz_arr[i][j]<<4;
				
			}else if( ((sizes/8*horizonl_hz_n)%3 == 2) && (j==(sizes/8*horizonl_hz_n-2))){
				hzk[i/18][j*8/12].fbitmap[i%18]=(hz_arr[i][j]<<4)|((hz_arr[i][j+1]&0xf0)>>4);
				hzk[i/18][j*8/12+1].fbitmap[i%18] = (hz_arr[i][j+1]&0xf)<<8; 

			}else{			
				hzk[i/18][j *8/12].fbitmap[i%18] = (hz_arr[i][j]<<4) | ((hz_arr[i][j+1]&0xf0)>>4);
				hzk[i/18][j*8/12+1].fbitmap[i%18]=(( hz_arr[i][j+1]&0xf )<<8)| ( hz_arr[i][j+2] );
			}
		}
	}

	for(i = 0;i<v_words; i++){
		for(j = 0;j<h_words;j++){
			hzk[i][j].font = font_idx + i*h_words+j;//fontidx;

			//printf("font_idx:%d set hzk[%d][%d].font: %d\n", font_idx, i,j, hzk[i][j].font);
#if 0

			for(k=0;k<GM_OSD_FONT_MAX_ROW;k++)
			{	
				for(n=0;n<16;n++){
					if( hzk[i][j].fbitmap[k] & (0x8000>>n) ){
						printf("*");
					} else	{ 
						printf(" ");
					} 
				}
				printf("\n");
			}
#endif
		}
	}
	//printf("======================================================\n");

	for(i=0;i<v_words;i++){
		for(j=0;j<h_words;j++){
			new_font.font_idx = hzk[i][j].font;
			//printf("hzk[%d][%d].font : %d\n", i,j,new_font.font_idx);
			for (k = 0; k < 18; k++) {
				new_font.bitmap[k] = ( hzk[i][j].fbitmap[k]>>4 )
							| ( (hzk[i][j].fbitmap[k]&0xf) <<12 );
#if 0
				for(n=0;n<16;n++){
						if( new_font.bitmap[k] & (0x8000>>n) ){
							printf("*");
						} else	{ 
							printf(" ");
						} 
					}
				printf("\n");
#endif 
			}
			gm_update_new_font(&new_font);
		}
	}

	/** setup OSD font */	
    osd_font.enabled = enable;
    osd_font.win_idx = win_idx;
    osd_font.align_type = GM_OSD_ALIGN_TOP_LEFT;
    osd_font.x = _x;
    osd_font.y = _y;
    osd_font.h_words = h_words;  ///< The horizontal number of words of OSD window
    osd_font.v_words = v_words;   ///< The vertical number of words of OSD window
    osd_font.h_space = 0;   ///< The vertical space between charater and charater
    osd_font.v_space = 0;   ///< The horizontal space between charater and charater

	osd_font.font_index_len = v_words*h_words*2;
	for(i=0;i<v_words*h_words;i++){
		osd_font.font_index[i] = (unsigned short)(font_idx + i);
	}
    
	osd_font.font_alpha = GM_OSD_FONT_ALPHA_75;
   // osd_font.win_alpha = GM_OSD_FONT_ALPHA_25;
    osd_font.win_alpha = GM_OSD_FONT_ALPHA_100;
    osd_font.win_palette_idx = 0;
	if(0 == strcmp(incode,"Rec "))
	{
		 osd_font.win_alpha = GM_OSD_FONT_ALPHA_0;
		 osd_font.font_palette_idx = 2;
	}
	else
	{
		 osd_font.font_palette_idx = 1;
	}
    osd_font.priority = GM_OSD_PRIORITY_MARK_ON_OSD;
    osd_font.smooth.enabled = 1;
    osd_font.smooth.level = GM_OSD_FONT_SMOOTH_LEVEL_WEAK;
    osd_font.marquee.mode = GM_OSD_MARQUEE_MODE_NONE;
    osd_font.border.enabled = 0;
    osd_font.border.width = 0;
    osd_font.border.type = GM_OSD_BORDER_TYPE_WIN;
    osd_font.border.palette_idx = 3;
    osd_font.font_zoom = GM_OSD_FONT_ZOOM_NONE;
    ret = gm_set_osd_font2(capture_obj, &osd_font); 
	//printf("ret : %d\n", ret);
	
	if (osd_font.font_index != NULL)
        free(osd_font.font_index);
}

#endif

int set_osd_font_chinese(int osd_handle)
{
	int i;
	int index_len = 16;
	unsigned char incode[3] = {0xcc, 0xcc, '\0'}; /* 0xb0, 0xa1 */
	struct font_info_t *info;
	gm_osd_font_t osd_font;
	struct bitmap_t *bitmap;

	bitmap = pos_code(incode);
	if (bitmap == NULL) {
		return -1;
	}
	info = get_font(bitmap);
	if (info == NULL) {
		return -1;
	}
	replace_font(info->data, info->row, info->col);

	osd_font.enabled = 1;
	osd_font.win_idx = 0;
	osd_font.align_type = GM_OSD_ALIGN_TOP_LEFT;
	osd_font.x = 0;
	osd_font.y = 0;
	osd_font.h_words = info->col;  
	osd_font.v_words = info->row;  
	osd_font.h_space = 0;  
	osd_font.v_space = 0;  

	index_len = info->col * info->row;
	osd_font.font_index_len = index_len;
	for (i = 0; i < osd_font.font_index_len; i++)
		osd_font.font_index[i] = (unsigned short)(FONT_INDEX_STR + i);//这里是实际显示汉字的

	osd_font.font_alpha = GM_OSD_FONT_ALPHA_75;
	osd_font.win_alpha = GM_OSD_FONT_ALPHA_0;
	osd_font.win_palette_idx = 0;
	osd_font.font_palette_idx = 1;
	osd_font.priority = GM_OSD_PRIORITY_MARK_ON_OSD;
	osd_font.smooth.enabled = 1;
	osd_font.smooth.level = GM_OSD_FONT_SMOOTH_LEVEL_WEAK;
#if 0
	if (marquee == 1) {
		osd_font.marquee.mode = GM_OSD_MARQUEE_MODE_HLINE;
		osd_font.marquee.length = GM_OSD_MARQUEE_LENGTH_64;
		osd_font.marquee.speed = 1;
	} else {
#endif
		osd_font.marquee.mode = GM_OSD_MARQUEE_MODE_NONE;
///	}

	osd_font.border.enabled = 1;
	osd_font.border.width = 1;
	osd_font.border.type = GM_OSD_BORDER_TYPE_WIN;
	osd_font.border.palette_idx = 3;
	osd_font.font_zoom = GM_OSD_FONT_ZOOM_NONE;
	
	gm_set_osd_font(osd_handle, &osd_font); //显示字体的

	put_font(info);
	free(bitmap);
	return 0;
}


int update_osd_dynamic(void *capture_obj, int enable, int win_idx, unsigned char incode[100], int font_len, int _x, int _y)
{
	gm_osd_font2_t osd_font2;
	int idx;

    osd_font2.font_index = (unsigned short *) malloc(font_len * sizeof(unsigned short));
    if (osd_font2.font_index == NULL) {
        printf("Error to malloc memory!\n");
        return 0;
    }

    /** setup OSD font */
    osd_font2.enabled = enable;
    osd_font2.win_idx = win_idx;
    osd_font2.align_type = GM_OSD_ALIGN_TOP_LEFT;
    osd_font2.x = _x;
    osd_font2.y = _y;
    osd_font2.h_words = font_len;   ///< The horizontal number of words of OSD window
    osd_font2.v_words = 1;   		///< The vertical number of words of OSD window
    osd_font2.h_space = 0;   		///< The vertical space between charater and charater
    osd_font2.v_space = 0;   		///< The horizontal space between charater and charater

    osd_font2.font_index_len = font_len;
	for(idx=0; idx<font_len; idx++)
	{
		osd_font2.font_index[idx] = (unsigned short)incode[idx];
	}

    osd_font2.font_alpha = GM_OSD_FONT_ALPHA_100;
    osd_font2.win_alpha = GM_OSD_FONT_ALPHA_25;
    osd_font2.win_palette_idx = 14;
    osd_font2.font_palette_idx = 0;
    osd_font2.priority = GM_OSD_PRIORITY_MARK_ON_OSD;
    osd_font2.smooth.enabled = 1;
    osd_font2.smooth.level = GM_OSD_FONT_SMOOTH_LEVEL_WEAK;
    osd_font2.marquee.mode = GM_OSD_MARQUEE_MODE_NONE;
    osd_font2.border.enabled = 1;
    osd_font2.border.width = 1;
    osd_font2.border.type = GM_OSD_BORDER_TYPE_WIN;
    osd_font2.border.palette_idx = 14;
    osd_font2.font_zoom = GM_OSD_FONT_ZOOM_NONE;
    gm_set_osd_font2(capture_obj, &osd_font2);//set OSD attribute(éè??OSDê?D?)

    if (osd_font2.font_index != NULL)
        free(osd_font2.font_index);

	return 0;
}

