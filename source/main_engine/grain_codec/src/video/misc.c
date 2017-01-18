#include "sdk_struct.h"
#include "video.h"

static resolution_map_s resolution_pal[SDK_VIDEO_RESOLUTION_BUTT]=
{
	{SDK_VIDEO_RESOLUTION_QCIF  ,	176,	144		}
    ,{SDK_VIDEO_RESOLUTION_CIF  ,	352,	288		}
    ,{SDK_VIDEO_RESOLUTION_HD1  ,	704,	288		}
    ,{SDK_VIDEO_RESOLUTION_D1   ,	704,	576		} 
    ,{SDK_VIDEO_RESOLUTION_960H ,	960,	576		} 
    ,{SDK_VIDEO_RESOLUTION_QVGA ,	320,	240		} 
    ,{SDK_VIDEO_RESOLUTION_VGA  ,	640,	480		}
    ,{SDK_VIDEO_RESOLUTION_XGA  ,	1024,	768		}
    ,{SDK_VIDEO_RESOLUTION_SXGA ,	1400,	1050	}
    ,{SDK_VIDEO_RESOLUTION_UXGA ,	1600,	1200	}
    ,{SDK_VIDEO_RESOLUTION_QXGA ,	2048,	1536	}
    ,{SDK_VIDEO_RESOLUTION_WVGA ,	854,	480		}
    ,{SDK_VIDEO_RESOLUTION_WSXGA,	1680,	1050	}
    ,{SDK_VIDEO_RESOLUTION_WUXGA,	1920,	1200	}
    ,{SDK_VIDEO_RESOLUTION_WQXGA,	2560,	1600	}
    ,{SDK_VIDEO_RESOLUTION_720p,	1280,	720		}
    ,{SDK_VIDEO_RESOLUTION_1080p,	1920,	1080	}
    ,{SDK_VIDEO_RESOLUTION_960p,	1280,	960		} 
    ,{SDK_VIDEO_RESOLUTION_SVGA,	800,	600		}
};

static resolution_map_s resolution_ntsc[SDK_VIDEO_RESOLUTION_BUTT]=
{
	{SDK_VIDEO_RESOLUTION_QCIF  ,	176,	120		}
    ,{SDK_VIDEO_RESOLUTION_CIF  ,	352,	240		}
    ,{SDK_VIDEO_RESOLUTION_HD1  ,	704,	240		}
    ,{SDK_VIDEO_RESOLUTION_D1   ,	704,	480		} 
    ,{SDK_VIDEO_RESOLUTION_960H ,	960,	576		} 
    ,{SDK_VIDEO_RESOLUTION_QVGA ,	320,	240		} 
    ,{SDK_VIDEO_RESOLUTION_VGA  ,	640,	480		}
    ,{SDK_VIDEO_RESOLUTION_XGA  ,	1024,	768		}
    ,{SDK_VIDEO_RESOLUTION_SXGA ,	1400,	1050	}
    ,{SDK_VIDEO_RESOLUTION_UXGA ,	1600,	1200	}
    ,{SDK_VIDEO_RESOLUTION_QXGA ,	2048,	1536	}
    ,{SDK_VIDEO_RESOLUTION_WVGA ,	854,	480		}
    ,{SDK_VIDEO_RESOLUTION_WSXGA,	1680,	1050	}
    ,{SDK_VIDEO_RESOLUTION_WUXGA,	1920,	1200	}
    ,{SDK_VIDEO_RESOLUTION_WQXGA,	2560,	1600	}
    ,{SDK_VIDEO_RESOLUTION_720p,	1280,	720		}
    ,{SDK_VIDEO_RESOLUTION_1080p,	1920,	1080	}
    ,{SDK_VIDEO_RESOLUTION_960p,	1280,	960		} 
    ,{SDK_VIDEO_RESOLUTION_SVGA,	800,	600		}
};
 int pic_2_resolution(int video_mode,int width,int height,int *resolution)
{
	int mode;
	int temp = -1;
	int i;
	if(video_mode == 0)
	{
		for(i=0;i<SDK_VIDEO_RESOLUTION_BUTT;i++)
		{
			if((width == resolution_pal[i].width)&&(height == resolution_pal[i].height))
			{
				temp = resolution_pal[i].id;
				break;
			}
		}
		if(temp == -1)
		{
			temp = SDK_VIDEO_RESOLUTION_D1;
		}
	}
	else
	{
		for(i=0;i<19;i++)
		{
			if((width == resolution_ntsc[i].width)&&(height == resolution_ntsc[i].height))
			{
				temp = resolution_ntsc[i].id;
				break;
			}
		}
		if(temp == -1)
		{
			temp = SDK_VIDEO_RESOLUTION_D1;
		}
	}
	*resolution = temp;
	return 0;
}

 int resolution_2_pic(int video_mode,int resolution,int *width,int *height)
{
	int mode;
	int i;
	int pic_width,pic_height;
	printf("XXX !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! resolution:%d\n", resolution);
	if(video_mode == 0)   //pal
	{
		for(i = 0;i < SDK_VIDEO_RESOLUTION_BUTT;i++)
		{
			if(resolution == resolution_pal[i].id)
			{
				pic_width = resolution_pal[i].width;
				pic_height = resolution_pal[i].height;
				break;
			}
		}
		if((pic_width == 0) || (pic_height == 0))
		{	
			pic_width = 704;
			pic_height = 576;
		}
	}
	else   //ntsc
	{
		for(i = 0;i < SDK_VIDEO_RESOLUTION_BUTT;i++)
		{
			if(resolution == resolution_ntsc[i].id)
			{
				pic_width = resolution_ntsc[i].width;
				pic_height = resolution_ntsc[i].height;
				break;
			}
		}
		if((pic_width == 0) || (pic_height == 0))
		{	
			pic_width = 704;
			pic_height = 480;
		}
	}
	*width = pic_width;
	*height = pic_height;
	return 0;
}