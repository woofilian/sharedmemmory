#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <assert.h>

#include <errno.h>
#include "adapt.h"
#include "log.h"

/*********************/



#define GPIO_DEV_NAME "/dev/gpio_drv"
#define GPIO_SET	0xA1
#define GPIO_CLEAR  0xA2		 
#define GPIO_DEFPIN	0xA3

int gpio_status_light;
int gpio_ir_cut28;
int gpio_ir_cut29;

 /*
  * type: 0 - ADC0, 1 - ADC1, 2 - ADC2
  * return:
  *     0:  success
  *     -1: failed
 */
#define KEY_DEV_NAME    	"/dev/sar_adc_drv"
#define KEY_IN      1
#define KEY_REPEAT  2
#define KEY_XAIN_0  4
#define KEY_XAIN_1  8
#define KEY_XAIN_2  16
#ifndef _IOWR
# define _IOWR(x,y,z)	(((x)<<8)|y)
#endif
#ifndef _IOW
# define _IOW(x,y,z)	(((x)<<8)|y)
#endif
#ifndef _IOR
# define _IOR(x,y,z)	(((x)<<8)|y)
#endif
#ifndef _IO
# define _IO(x,y)	(((x)<<8)|y)
#endif

#define SAR_ADC_MAGIC 'S'
#define SAR_ADC_KEY_ADC_DIRECT_READ              _IOR(SAR_ADC_MAGIC, 1, int) 
#define SAR_ADC_KEY_SET_XGAIN_NUM                _IOWR(SAR_ADC_MAGIC, 2, int) 
#define SAR_ADC_KEY_SET_REPEAT_DURATION          _IOWR(SAR_ADC_MAGIC, 3, int) 

#define ISP_IOC_COMMON 'c'
#define ISP_IOC_ISP   's'
#define ISP_IOC_LOAD_CFG                _IOW(ISP_IOC_COMMON, 133, char *)
#if 0
#define ISP_IOC_START               _IO(ISP_IOC_ISP, 1)
#define ISP_IOC_STOP                _IO(ISP_IOC_ISP, 2)
#else

#define ISP_IOC_START                   _IO(ISP_IOC_COMMON, 1)
#define ISP_IOC_STOP                    _IO(ISP_IOC_COMMON, 2)

#endif
#define ISP_IOC_GET_CHIPID              _IOR(0, 0, u32)
#define ISP_IOC_GET_DRIVER_VER          _IOR(0, 1, u32)
#define ISP_IOC_GET_INPUT_INF_VER       _IOR(0, 2, u32)
#define ISP_IOC_GET_ALG_INF_VER         _IOR(0, 3, u32)

int gpio_fd;

static pthread_mutex_t gpio_lock;
typedef struct
{
	int			num;
	int			dir;
} gpio_info_t;
gpio_info_t gpio_def;

typedef struct sar_adc_data_tag{
    unsigned int adc_val;
    int status;
}sar_adc_pub_data;
/***********icut-colormode*****/


typedef enum
{
         COLOR_MODE_AUTO = 0,    // Color Mode: Auto, detected by light sensor
         COLOR_MODE_DAY,         // Color Mode: Day - Chromatic, bInfraredLed = 0, bInfraredCut = 1, nSaturation > 0
         COLOR_MODE_NIGHT,       // Color Mode: Night - Monochrome, bInfraredLed = 1, bInfraredCut = 0, nSaturation = 0
      //  GRD_COLOR_MODE_SCHEDULE_ON_TIMER,   //Color Mode: time Schedule  current mode.
}COLOR_MODE_E;

#define LIGHT_SENSOR_SWITCH_RANGE 10
COLOR_MODE_E realtime_user_mode_cfg = COLOR_MODE_AUTO;
COLOR_MODE_E mode = COLOR_MODE_DAY;

/**************************/



int apply_gpio()
{
    gpio_fd = open(GPIO_DEV_NAME, O_RDONLY | O_NONBLOCK);// | O_NONBLOCK);
    pthread_mutex_init(&gpio_lock, NULL);
    if (gpio_fd < 0)
    {
        warning("init_gpio:: Can not open /dev/gpio_drv\n");
        return -1;
    }
}
int init_gpio()
{
	//设置相关GPIO口的复用
	int ret = apply_gpio();
	if(ret < 0)
	{
		return -1;
	}
    return 0;
}

int gpioEx_defpin(int num, int dir) 
{
	if(num < 0)
		return -1;

	gpio_def.num = num;
	gpio_def.dir = dir;

	if(gpio_fd >= 0)
		ioctl(gpio_fd, GPIO_DEFPIN, &gpio_def);
	return 0;
}

void gpioEx_set(int num)
{
	//warning("num [%d]   \n",num);
	if(num < 0)
		return;

	gpio_def.num = num;
	if(gpio_fd >= 0)
		ioctl(gpio_fd, GPIO_SET, &gpio_def);
}

void gpioEx_clear(int num)
{
	//info("============================= start !!! gpio_fd:%d \n",gpio_fd);
	if(num < 0)
		return;

	gpio_def.num = num;

	if(gpio_fd>=0)
		ioctl(gpio_fd, GPIO_CLEAR, &gpio_def);
	
	//info("============================= end !!! \n");
}

int gpioEx_read(int num)
{
	char val = 0;

	if(num < 0)
		return -1;

	if(gpio_fd>=0)
		read(gpio_fd, &val, num);

	return val;
}



int gm_sar_adc_read(int type, sar_adc_pub_data *data)
{
	int key_fd;
	key_fd = open( KEY_DEV_NAME, O_RDONLY | O_NONBLOCK );
	int i;
	int ret = 0;
	int value = 0;

 	if( key_fd)
	{
		if( ioctl( key_fd, SAR_ADC_KEY_SET_XGAIN_NUM, &type) < 0)
		{
            error("SAR_ADC_KEY_SET_XGAIN_NUM fail\n");
            ret = -1;
        }

		for ( i = 0; i < 3; i++)
		{
			if ( ioctl(key_fd, SAR_ADC_KEY_ADC_DIRECT_READ, &value ) < 0)
			{
	            error("SAR_ADC_KEY_ADC_DIRECT_READ fail\n");
				ret = -1;
			}

			if ( value >= 0 ) 	//
				break;

			if( 0 > value )
			{
				//info("===================ADC %d,", value);
				usleep(100 * 1000);
				continue;
			}
		}
		close( key_fd );
	}

	//warning("===================value %d\n", value);
	data->status  = 4 << type;
	data->adc_val = ( ret == 0 ) ? value:0;
	//info("[%d]......   status, %d,%d\n",  type, data->status,data->adc_val);
	return ret;
}

int get_light_sensor_value()
{
	static unsigned char value = 1; // DAY
	int adc_num =0 , gpio_num = 0;
	int ret;
	
	int photoSen_threshold = 15; //adc 采样经验值
	//36s 962
	#if 0
	gpio_num = 29;
	if( gpio_num )
    {
        value = gpioEx_read(gpio_num);
        return value;
    }
	#endif
	sar_adc_pub_data data;	
	if((ret = gm_sar_adc_read(adc_num, &data) ) == 0)
	{
		if( data.adc_val > photoSen_threshold + LIGHT_SENSOR_SWITCH_RANGE ) // range value : LIGHT_SENSOR_SWITCH_RANGE
		{
	        value = 1;
		}
	    else if( data.adc_val < photoSen_threshold - LIGHT_SENSOR_SWITCH_RANGE )
	    {
	        value = 0;
	    }
	}
	//warning("now adc-value  [%d]\n",value);
	return value;
}

int isp_fd;
int ov9715_init(void)
{
	int ret = 0;
	u32 id;

	isp_fd = open("/dev/isp328", O_RDWR);
	if (isp_fd < 0)
	{
		printf("Open ISP328 fail\n");
		return -1;
	}
	

	// just ensure ISP is start, generally ISP has been started after insert capture driver
	ret = ioctl(isp_fd, ISP_IOC_START, NULL);
	if (ret < 0)
	{
	    printf("Start ISP IO Control fail\n");
		return -1;
	}

	// check chip ID
	ret = ioctl(isp_fd, ISP_IOC_GET_CHIPID, &id);
	if (ret < 0)
		return -1;

	return 0;
}


int isp320_reloadCfg_control(char *fname)
{
    int ret = -1;
    if(NULL == fname)
    {
         printf("*******error fname******\n");
         ret = -1;
    }
    ret = ioctl(isp_fd, ISP_IOC_LOAD_CFG, fname);
    usleep(100*1000);
    return ret;
}

