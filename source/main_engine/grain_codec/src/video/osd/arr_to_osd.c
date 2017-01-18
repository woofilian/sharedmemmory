#include "gmlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "sdk_struct.h"
#include "log.h"
#define size 32
#define sum (size*size/8)
#define SET_BIT            "â– "
#define UNSET_BIT	   "â–¡"

#define HZ_SIZE_32 32  //ºº×ÖÏñËØµã
#define HZ_SIZE_32_SUN (HZ_SIZE_32*HZ_SIZE_32/8)  //µ¥¸öºº×ÖÕ¼ÓÃµÄ×Ö½ÚÊý

#define HZ_SIZE_16 16  //ºº×ÖÏñËØµã
#define HZ_SIZE_16_SUN (HZ_SIZE_16*HZ_SIZE_16/8)  //µ¥¸öºº×ÖÕ¼ÓÃµÄ×Ö½ÚÊý

#ifndef FONT_INDEX_STR
#define FONT_INDEX_STR            (32)
#endif

#define OSD_LINE_PER_FONT         (18)
#define OSD_BIT_PER_LINE          (12)

#define DEBUG                      1
#define DEBUG_L2                   0
#define TEST_DOUBLE_CHINESE        1


#if  DEBUG
#define dbg(fmt, args...)      printf(fmt, ##args)
#else
#define dbg(fmt, args...)
#endif


typedef struct bitmap_t
{
    int width;          /*   */
    int height;
    void *arr; /*   */
} bitmap_t;

typedef struct font_data_t
{
    int font;
    unsigned fbitmap[18];
} font_data_t;

typedef struct font_info_t
{
    int row;
    int col;
    struct font_data_t *data;
} font_info_t;

#ifdef DEBUG
/* We use a 32*32 bitmap to debug this */
static unsigned char test[32][4] =
{
    0x80, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
    0x08, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x80, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
    0x00, 0x08, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x10, 0x00,
    0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x10,
    0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01,
};
#endif
/*****************/
extern  int g_fd_zk ;

/*****************/


static int replace_font(font_data_t *font, int row, int col)
{
    int i, j;
    int index_len = row * col;
    gm_osd_font_update_t new_font;

    dbg("%s->E\n", __func__);
    for (i = 0; i < index_len; i++)
    {
        new_font.font_idx = font[i].font;
        for (j = 0; j < GM_OSD_FONT_MAX_ROW; j++)
        {
            new_font.bitmap[j] = (font[i].fbitmap[j]>>4) | ((font[i].fbitmap[j]&0xf) <<12) ;
            dbg("%d bitmap[%d] = %04x fbitmap[%d] - %03x\n", new_font.font_idx, j, new_font.bitmap[j], j, font[i].fbitmap[j]);
        }
        gm_update_new_font(&new_font);  //¸üÐÂ×Ö¿â
        dbg("\n");
    }

    dbg("%s->X\n", __func__);
    return 0;
}


struct font_info_t *get_font(struct bitmap_t *bitmap)
{
    int mat_height = bitmap->height;
    int mat_len = bitmap->width/8; //Ò»ÐÐµÄ×Ö½ÚÊý
    unsigned char (*mat)[mat_len];
    unsigned *fbitmap;
    int  idx;
    int i, j, frow, fcol;
    int row = (bitmap->height / OSD_LINE_PER_FONT) + !!(bitmap->height % OSD_LINE_PER_FONT); //ÐÐ
    int col = (bitmap->width / OSD_BIT_PER_LINE) + !!(bitmap->width % OSD_BIT_PER_LINE); //ÁÐ
    struct font_info_t *info = NULL;
    struct font_data_t (*font)[col] = NULL;

    dbg("%s->E\n", __func__);
    mat = bitmap->arr;

    dbg("row %d col %d h %d w %d\n", row, col, bitmap->height, bitmap->width);

    info = malloc(sizeof(struct font_info_t) + 2*row * col * sizeof (font_data_t));
    if (!info)
    {
        printf("can not malloc \n");
        return NULL;
    }
    font = (void *) (info + 1);

    memset(info, 0, sizeof(struct font_info_t) + 2*row * col * sizeof(font_data_t));
    for (i = 0; i < mat_height; i++) //32
    {
        frow = i / OSD_LINE_PER_FONT; // Ò»¸ö×ÖÕ¼ÓÃ¼¸ÐÐ
        idx = i % OSD_LINE_PER_FONT;
        dbg("===  %d ===\n", i);
        for (j = 0; j < mat_len; j+=3) //Ò»ÐÐµÄ×Ö½ÚÊý
        {
            fcol = (j*8)/ OSD_BIT_PER_LINE;
            if ((i % OSD_LINE_PER_FONT) == 0)
            {
                font[frow][fcol].font = FONT_INDEX_STR + fcol + frow*col;
                dbg("font->%d\n", font[frow][fcol].font);
            }
            fbitmap = font[frow][fcol].fbitmap;
            if(j+1 >= mat_len)
            {
                fbitmap[idx] = mat[i][j]<<4;    /* fbitmap is larger than bitmap  */
#if DEBUG_L2
                dbg("[%d][%d] - %04x mat[%d][%d] - %02x \n",
                    frow, fcol, fbitmap[idx], i, j,
                    mat[i][j]);
#else
                dbg(" %03x", fbitmap[idx]);

#endif
            }
            else
            {
                fbitmap[idx] = (mat[i][j]<<4) | ((mat[i][j+1]&0xf0)>>4);
#if DEBUG_L2
                dbg("[%d][%d] - %04x mat[%d][%d] - %02x mat[%d][%d] - %02x\n",
                    frow, fcol, fbitmap[idx], i, j,
                    mat[i][j], i, j+1, mat[i][j+1]);
#else
                dbg(" %03x", fbitmap[idx]);

#endif
            }

            if ((j+1 > mat_len) || (j+2 > mat_len))
                break;
            if ((i % OSD_LINE_PER_FONT) == 0)
            {
                font[frow][fcol+1].font = FONT_INDEX_STR + fcol + 1 + frow*col;
                dbg("font->%d\n", font[frow][fcol+1].font);
            }
            fbitmap = font[frow][fcol+1].fbitmap;
            if(j+2 >= mat_len)
            {
                fbitmap[idx] = ((mat[i][j+1]&0xf)<<8)| 0;  /* fbitmap is larger than bitmap */
#if DEBUG_L2
                dbg("[%d][%d] - %04x mat[%d][%d] - %02x \n",
                    frow, fcol + 1, fbitmap[idx], i, j + 1,
                    mat[i][j + 1]);
#else
                dbg(" %03x", fbitmap[idx]);
#endif
            }
            else
            {

                fbitmap[idx] = ((mat[i][j+1]&0xf)<<8)| mat[i][j+2];
#if DEBUG_L2
                dbg("[%d][%d] - %04x mat[%d][%d] - %02x mat[%d][%d] - %02x\n",
                    frow, fcol + 1, fbitmap[idx], i, j+1,
                    mat[i][j+1], i, j+2, mat[i][j+2]);
#else
                dbg(" %03x", fbitmap[idx]);

#endif
            }
        }
        dbg("\n\n");
    }

    info->row = row;
    info->col = col;
    info->data = (void *)(info + 1);
    dbg("%s->X\n", __func__);
    return info;
}

void put_font(struct font_info_t *info)
{
    dbg("%s->E\n", __func__);
    free(info);
    dbg("%s->X\n", __func__);
}


struct bitmap_t *pos_code(unsigned char incode[3])
{
    int hzk_fd;
    unsigned char qh = 0, wh = 0;
    unsigned long offset = 0;
    struct bitmap_t *bitmap;
    unsigned char mat[size][size/8];
#if TEST_DOUBLE_CHINESE
    unsigned char mat2[size][size/4];
    unsigned char (*mat_arr)[size/4];
#else
    unsigned char (*mat_arr)[size/8];
#endif
    char hz[64];
    int i,j,k;

    dbg("%s->E\n", __func__);

#if TEST_DOUBLE_CHINESE
    bitmap = malloc(sizeof(struct bitmap_t) + size * size/4 * sizeof(unsigned char));
    bitmap->width = size*2; //Ë«×Ö
#else
    bitmap = malloc(sizeof(struct bitmap_t) + size * size/8 * sizeof(unsigned char));
    bitmap->width = size;
#endif
    bitmap->arr = bitmap + 1;
    bitmap->height  = size;

    //ÇøÎ»Âë
    qh = incode[0] - 0xa0;
    wh = incode[1] - 0xa0;

    sprintf(hz , "HZK%d" , size);

    offset = (94*(qh-1)+(wh-1))*sum;

    dbg("incode %2X%2X code %2X%2X offset %lu \n", incode[0], incode[1], qh, wh, offset);

    ///hzk_fd = open(hz , O_RDONLY);FONT_PATH_HZK32
    hzk_fd = open("/mnt/nfs/HZK32" , O_RDONLY);
    if(hzk_fd < 0)
    {
        printf("Can't Open hzk%d  - %d\n" , size, hzk_fd);
        getchar();
        return NULL;
    }

    dbg("bitmap info: width %d height %d file %s\n", bitmap->width, bitmap->height, hz);
    lseek(hzk_fd, offset, SEEK_SET);

    read(hzk_fd, mat, sum);

    close(hzk_fd);


    mat_arr = bitmap->arr;
    memcpy(mat_arr, mat, size * size/8 * sizeof(unsigned char));
#if TEST_DOUBLE_CHINESE
    for (i = 0; i < bitmap->height; i++)
    {
        for (j = 0; j < (bitmap->width/8); j++)
        {
            mat2[i][j] = mat[i][j];
            mat2[i][j+(bitmap->width/8/2)] = mat[i][j];
        }
    }
    memcpy(mat_arr, mat2, size * size/4 * sizeof(unsigned char));
#endif

    for(i=0; i<bitmap->height; i++)
    {
        for(j=0; j<bitmap->width/8; j++)
        {
            for(k=0; k<8 ; k++)
            {
                if( mat_arr[i][j] & (0x80>>k))
                {
                    dbg(SET_BIT);
                }
                else
                {
                    dbg(UNSET_BIT);
                }
            }
        }
        dbg("\n");
    }

    dbg("%s->X\n", __func__);
    return bitmap;
}

//·µ»Ø×ÖÊý
bitmap_t * hz_pos_code_32(const char *szHz)
{

if(g_fd_zk < 0 || !szHz)
{
	error("g_fd_zk < 0 || !szHz \n");
	return NULL;
}
    unsigned char qh = 0, wh = 0;
    unsigned long offset = 0;
    struct bitmap_t *bitmap;
    unsigned char mat[size][size/8];
#if TEST_DOUBLE_CHINESE
    unsigned char mat2[size][size/4];
    unsigned char (*mat_arr)[size/4];
#else
    unsigned char (*mat_arr)[size/8];
#endif
    char hz[64];
    int i,j,k;

    info("%s->E\n", __func__);

    bitmap = malloc(sizeof(struct bitmap_t) + HZ_SIZE_32_SUN);
	memset(bitmap,0,sizeof(struct bitmap_t) + HZ_SIZE_32_SUN);
    bitmap->width = HZ_SIZE_32; //Ë«×Ö
    bitmap->height  = HZ_SIZE_32;

    //ÇøÎ»Âë
    qh = szHz[0] - 0xa0;
    wh = szHz[1] - 0xa0;

    offset = (94*(qh-1)+(wh-1))*HZ_SIZE_32_SUN;

    dbg("incode %2X%2X code %2X%2X offset %lu \n", szHz[0], szHz[1], qh, wh, offset);
    dbg("bitmap info: width %d height %d \n", bitmap->width, bitmap->height);
    lseek(g_fd_zk, offset, SEEK_SET);
    read(g_fd_zk, bitmap->arr, HZ_SIZE_32_SUN);
	
    info("%s->X\n", __func__);
    return bitmap;
}



