#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <assert.h>

#include <errno.h>

#include "log.h"
#ifdef _AMBARELA_
#include <basetypes.h>
typedef enum
{
    GPIO_UNEXPORT = 0,
    GPIO_EXPORT = 1
} gpio_ex;

typedef enum
{
    GPIO_IN = 0,
    GPIO_OUT = 1
} gpio_direction;

typedef enum
{
    GPIO_LOW = 0,
    GPIO_HIGH = 1
} gpio_state;



int do_gpio_export(int gpio_id, gpio_ex ex)
{
    int ret = -1;
    int fd = -1;
    char buf[128] = {0};
    char vbuf[4] = {0};

    sprintf(buf, "/sys/class/gpio/%s", ex == GPIO_EXPORT ? "export" : "unexport");
    if ((fd = open(buf, O_WRONLY)) < 0)
    {
        error("do_gpio_export open");
    }
    else
    {
        sprintf(vbuf, "%d", gpio_id);
        if (strlen(vbuf) != write(fd, vbuf, strlen(vbuf)))
        {
            error("write");
        }
        else
        {
            ret = 0;
        }
        close(fd);
    }

    return ret;
}

int set_gpio_direction(int gpio_id, gpio_direction direction)
{
    int ret = -1;
    int fd = -1;
    char buf[128] = {0};
    char *vbuf = NULL;

    sprintf(buf, "/sys/class/gpio/gpio%d/direction", gpio_id);
    if ((fd = open(buf, O_WRONLY)) < 0)
    {
        error("set_gpio_direction open");
    }
    else
    {
        switch(direction)
        {
        case GPIO_OUT:
            vbuf = "out";
            break;
        case GPIO_IN:
            vbuf = "in";
            break;
        default:
            fprintf(stderr, "Invalid direction!\n");
            break;
        }
        if (vbuf)
        {
            if (strlen(vbuf) != write(fd, vbuf, strlen(vbuf)))
            {
                error("write");
            }
            else
            {
                ret = 0;
            }
        }
        close(fd);
    }

    return ret;
}

int get_gpio_direction(int gpio_id)  //return 1:out, 0:in, -1:error
{
    int ret = -1;
    int fd = -1;
    char buf[128] = {0};
    char vbuf[4] = {0};

    sprintf(buf, "/sys/class/gpio/gpio%d/direction", gpio_id);
    if ((fd = open(buf, O_RDONLY)) < 0)
    {
        error("get_gpio_direction open");
    }
    else
    {
        if (read(fd, vbuf, sizeof(vbuf)) < 0)
        {
            error("read");
        }
        else
        {
            if (vbuf[0] == 'o')
            {
                ret = 1;
            }
            else if (vbuf[0] == 'i')
            {
                ret = 0;
            }
            else
            {
                fprintf(stderr, "Invalid direction!\n");
            }
        }
        close(fd);
    }

    return ret;
}

int get_gpio_state(int *fd, int gpio_id)
{
    char buf[128] = {0};
    char vbuf[4] = {0};

    sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio_id);
    if (*fd < 0)
    {
        if ((*fd = open(buf, O_RDWR)) < 0)
        {
            error("get_gpio_state open");
            return -1;
        }
    }

    if (read(*fd, vbuf, sizeof(vbuf)) < 0)
    {
        error("read");
        return -1;
    }
    else
    {
        lseek(*fd, 0, SEEK_SET);
    }

    return atoi(vbuf);
}

int set_gpio_state(int *fd, int gpio_id, int state)
{
    char buf[128] = {0};
    char *vbuf = NULL;

    sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio_id);
    if (*fd < 0)
    {
        if ((*fd = open(buf, O_RDWR)) < 0)
        {
            error("set_gpio_state open");
            return -1;
        }
    }
    if (state == GPIO_LOW)
    {
        vbuf = "0";
    }
    else if (state == GPIO_HIGH)
    {
        vbuf = "1";
    }
    else
    {
        fprintf(stderr, "Invalid gpio state: %d\n", state);
        return -1;
    }
    if (vbuf)
    {
    	info("buf:%s vbuf:%s \n",buf,vbuf);
        if (strlen(vbuf) != write(*fd, vbuf, strlen(vbuf)))
        {
            error("write");
            return -1;
        }
        else
        {
            lseek(*fd, 0, SEEK_SET);
        }
		info("set_gpio_state  ok ok ok ok !!!! \n");
    }

    return 0;
}


int get_adc_value(int *fd, int adc_ch_id, int *value)
{
    char value_str[128] = {0};
    char vbuf[8] = {0};
    char adc_device[128] = "/sys/devices/e8000000.apb/e801d000.adc/adcsys";
    sprintf(vbuf, "adc%d=0x", adc_ch_id);

    if (!value)
    {
        fprintf(stderr, "Invalid pointer!");
        return -1;
    }
    if (*fd < 0)
    {
        if ((*fd = open(adc_device, O_RDONLY)) < 0)
        {
            fprintf(stderr, "get_adc_value open %s\n", strerror(errno));
            return -1;
        }
    }
    if (read(*fd, value_str, sizeof(value_str)) <= 0)
    {
        error("read");
        return -1;
    }
    else
    {
        sscanf(strstr(value_str, vbuf), "%*[^x]x%x", value);
        lseek(*fd, 0, SEEK_SET);
    }

    return 0;
}


/* functions for IR cut switch */
int ir_cut_gpio1_fd = -1;
int ir_cut_gpio2_fd = -1;
int ir_led_adc_fd = -1;

int ir_cut_is_supported(void)
{
    return 1;//(int)SUPPORT_IR_CUT;
}

int ir_cut_set_state(int value)
{
    int ret = -1;
    if(value == 1)
    {
        ret = set_gpio_state(&ir_cut_gpio1_fd, 34, GPIO_HIGH);
        if (ret < 0)
        {
            fprintf(stderr, " set gpio34 state to 1 failed!\n");
        }
        ret = set_gpio_state(&ir_cut_gpio2_fd, 38, GPIO_LOW);
        if (ret < 0)
        {
            fprintf(stderr, " set gpio38 state to 0 failed!\n");
        }

    }
    else
    {
        ret = set_gpio_state(&ir_cut_gpio1_fd, 34, GPIO_LOW);
        if (ret < 0)
        {
            fprintf(stderr, " set gpio34 state to 1 failed!\n");
        }
        ret = set_gpio_state(&ir_cut_gpio2_fd, 38, GPIO_HIGH);
        if (ret < 0)
        {
            fprintf(stderr, " set gpio38 state to 0 failed!\n");
        }

    }
    return ret;
}
int ir_cut_off()
{
    int ret = -1;
    ret = set_gpio_state(&ir_cut_gpio1_fd, 34, 0);
    if (ret < 0)
    {
        fprintf(stderr, " set gpio34 state to 1 failed!\n");
    }
    ret = set_gpio_state(&ir_cut_gpio2_fd, 38, 0);
    if (ret < 0)
    {
        fprintf(stderr, " set gpio38 state to 0 failed!\n");
    }


    return ret;
}

#if 0
int ir_cut_get_state(void)
{
    int ret =- 1;
    ret = get_gpio_state(&ir_cut_gpio_fd, GPIO_ID_IR_CUT_CTRL);
    if (ret < 0)
    {
        fprintf(stderr, "ir cut get state failed!\n");
    }

    return ret;
}
#endif

int ir_cut_init(int init)/* init=1:initiate IR cut, init=0:de-initiate IR cut*/
{
    int ret = -1;
    char gpio_addr[128] = {0};

    sprintf(gpio_addr, "/sys/class/gpio/gpio%d", 34);
    sprintf(gpio_addr, "/sys/class/gpio/gpio%d", 38);
    //set IR CUT control GPIO to default value 0, make sure it is covered
    if ((0 != access(gpio_addr, F_OK))&&(0 != access(gpio_addr, F_OK)))
    {
        do_gpio_export(34, GPIO_EXPORT);
        set_gpio_direction(34, GPIO_OUT);

		 do_gpio_export(38, GPIO_EXPORT);
        set_gpio_direction(38, GPIO_OUT);

		
    }
    ret = ir_cut_off();

    return ret;
}


/*****/
int  ir_led_get_adc_value(int *value)
{
	int ret = -1;
	ret = get_adc_value(&ir_led_adc_fd, 2, value);
	if (ret < 0) {
		error( "ir led get adc value failed!\n");
	}

	return ret;
}
//////////////////////////////////////////led colour control start
 
 
 
 void set_gpio_value(int gpiogroup,int pin,int gpiovalue)
 {
	 GPIO_PIN_TYPE set;
	 if(pin>31)
	 {
		 if(gpiogroup==1)
		 {
			 gpiogroup = IOCTL_GPIO1_DATA_W - IOCTL_GPIO_DATA_W;
		 }
		 if (ioctl(fd_gpio, IOCTL_GPIO_DATA_W + gpiogroup, &gpiovalue) != 0)
		 {
			 printf("set_gpio_value::222direct set pin dir failed.\n");
		 }
	 }
	 else
	 {
		 if(gpiogroup==1)
		 {
			 gpiogroup = IOCTL_GPIO1_PIN_VLAUE_W - IOCTL_GPIO_PIN_VLAUE_W;
		 }
		 set.pin=pin;
		 set.value = gpiovalue;
 
		 if (ioctl(fd_gpio, IOCTL_GPIO_PIN_VLAUE_W + gpiogroup, &set) != 0)
		 {
			 printf("set_gpio_value::333direct set pin dir failed.\n");
		 }
	 }
 }

 int set_led_status(int led_type,int on_off)
{
#ifdef OEM_TREE_LED
    set_gpio_value(LED_GREEN_GROUP,RUN_GREEN_LED,LED_OFF); //电源开
    set_gpio_value(LED_RED_GROUP,RUN_RED_LED,LED_OFF); //报警
    set_gpio_value(LED_BLUE_GROUP,RUN_BLUE_LED,LED_OFF); //APP 使用

    int led_status = (on_off == LED_ON)?LED_ON:LED_OFF;
    int real_status = 0;
    ///printf(" set_led_status led_type:%d led_status:%d \n",led_type,led_status);
		/*printf("ZW_TEST +_+_+_ led_status[0]:%d   led_status[1]:%d led_status[2]:%d\n"
           		,g_led_status.led_status[0]
           		,g_led_status.led_status[1]
           		,g_led_status.led_status[2]);*/
    switch(led_type)
    {
    case POWER_LED:				////= 0x00, //GREEN
    {
        if(g_led_status.led_status[1] == 0) 						//表示关闭
        led_status = LED_OFF;
		//printf("ZW_TEST POWER_LED led_status is %d\n",led_status);
        set_gpio_value(LED_GREEN_GROUP,RUN_GREEN_LED,led_status); //电源开
        break;
    }
    case ALARM_LED:			////= 0x01, //RED
    {
        if(g_led_status.led_status[0] == 0) 						//表示关闭
        led_status = LED_OFF;
		//printf("ZW_TEST ALARM_LED led_status is %d\n",led_status);
        set_gpio_value(LED_RED_GROUP,RUN_RED_LED,led_status); //报警
        break;
    }
    case APP_LED:			////	= 0x02, //BLUE
    {
        if(g_led_status.led_status[2] == 0) 						//表示关闭
        led_status = LED_OFF;
		//printf("ZW_TEST APP_LED led_status is %d\n",led_status);
        set_gpio_value(LED_BLUE_GROUP,RUN_BLUE_LED,led_status); //APP 使用蓝色
        break;
    }

    case ALL_LED:
    {
		  if(g_led_status.led_status[1] == 0) 						//表示关闭
          led_status = LED_OFF;
        set_gpio_value(LED_GREEN_GROUP,RUN_GREEN_LED,led_status); //电源开

		  if(g_led_status.led_status[0] == 0) 						//表示关闭
          led_status = LED_OFF;
        set_gpio_value(LED_RED_GROUP,RUN_RED_LED,led_status); //报警
        
          if(g_led_status.led_status[2] == 0) 						//表示关闭
          led_status = LED_OFF;
        set_gpio_value(LED_BLUE_GROUP,RUN_BLUE_LED,led_status); //APP 使用
		//printf("ZW_TEST  ALL_LED led_status is %d\n",led_status);
		break;
    }

    default:
        return -1;
    }
#endif
    return 0;
}

//////////////////////////////////////////led colour control end
#endif



#if 0
#define GPIO_DEV_NAME "/dev/gpio_drv"
#define GPIO_SET	0xA1
#define GPIO_CLEAR  0xA2		 
#define GPIO_DEFPIN	0xA3

int gpio_ir_cut;
int gpio_ir_cut_sub;
int gpio_ir_cut_light;
int gpio_fd;

static pthread_mutex_t gpio_lock;
typedef struct
{
	int			num;
	int			dir;
} gpio_info_t;

gpio_info_t gpio_def;

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

	//test
	#if 0
	gpio_ir_cut_light = 32+28;
	int vals1 ,vals2;
	gpioEx_defpin(gpio_ir_cut_light, 1);  //set output
	while(1)
	{
		
		gpioEx_clear(gpio_ir_cut_light);   //Infrared LED ON
		vals1 = gpioEx_read(gpio_ir_cut_light);
		warning("----------->Jensen light vals11111  ->  [%d]\n",vals1);
	
		usleep(300*1000);
	
		gpioEx_set(gpio_ir_cut_light);//Infrared LED OFF
		vals2 = gpioEx_read(gpio_ir_cut_light);
		warning("----------->Jensen light vals22222  ->  [%d]\n",vals2);
		usleep(300*1000);
	}
	#endif
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
	warning("num [%d]   \n",num);
	if(num < 0)
		return;

	gpio_def.num = num;
	if(gpio_fd >= 0)
		ioctl(gpio_fd, GPIO_SET, &gpio_def);
}

void gpioEx_clear(int num)
{
	if(num < 0)
		return;

	gpio_def.num = num;

	if(gpio_fd>=0)
		ioctl(gpio_fd, GPIO_CLEAR, &gpio_def);
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
#endif


