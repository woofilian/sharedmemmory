#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "log.h"

#include "ioctl_isp328.h"
extern int isp_fd;

static int ov2715_ae_pwr_freq(int val)
{
    int mode = val;
    int fps;
    int ret = -1;
    static int cur_mode = -1;

    if (cur_mode == mode)
        return 0;

    cur_mode = mode;

    info( "Anti-Flicker Mode: %s\n", (mode == AE_PWR_FREQ_60) ? "60Hz" : "50Hz");
    if(mode == AE_PWR_FREQ_60)
        fps = 30;
    else
        fps = 25;

    // ioctl(isp_fd, AE_IOC_GET_PWR_FREQ, &temp);
    ret = ioctl(isp_fd, AE_IOC_SET_PWR_FREQ, &mode);
    info( "Set Sensor FPS: %d\n", fps);
    ioctl(isp_fd, ISP_IOC_SET_SENSOR_FPS, &fps);

    return ret;
}

static int ov2715_init(void)
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
	    error( "Start ISP IO Control fail\n");
		return -1;
	}

	// check chip ID
	ret = ioctl(isp_fd, ISP_IOC_GET_CHIPID, &id);
	if (ret < 0)
		return -1;
	info( "Chip ID: %x\n", id);

	#if defined(CHIP_GM813x)
	if (id != 0x8139)
		return -1;
	#endif

	if (id != 0x8136)
		return -1;

	return 0;
}



int Set_DayNight_mode(int val)
{
    int mode = val;
    int curmode;
    int ret = -1;

    ret = ioctl(isp_fd, ISP_IOC_GET_DAYNIGHT_MODE, &curmode);
    if (curmode == mode)
        return 0;

    ret = ioctl(isp_fd, ISP_IOC_SET_DAYNIGHT_MODE, &mode);
    info( "Day-Night Mode: %s\n", (mode == 1) ? "Night" : "Day");

    return ret;
}

int Set_ISP_MaxGain(int value)
{
    int ret;
    u32 gain =value;
    if(value < 0)
        return -1;

    if (isp_fd < 0)
        ov2715_init();

    ret = ioctl(isp_fd, AE_IOC_SET_MAX_ISP_GAIN, &gain);
    return ret;
}


static int isp320_reloadCfg_control(char *fname)
{
    int ret = -1;
    if(NULL == fname)
    {
        error( "*******error fname******\n");
        ret = -1;
    }
    if (isp_fd < 0)
        ov2715_init();
    
    ret = ioctl(isp_fd, ISP_IOC_LOAD_CFG, fname);
    usleep(100*1000);
    return ret;
}

int get_encode_size( int width,int height)
{
    win_size_t win_size;
    int ret = -1;
    if (isp_fd < 0)
        ov2715_init();

    ret = ioctl(isp_fd, ISP_IOC_GET_SIZE, &win_size);
    width = win_size.width;
    height = win_size.height;
    return ret;
}

int set_encode_size(int width,int height)
{
    int ret = -1;
    win_size_t win_size;
    if (isp_fd < 0)
        ov2715_init();

    ret = ioctl(isp_fd, ISP_IOC_GET_SIZE, &win_size);
    info( "-------get sensor size:%dx%d---------\n", win_size.width,win_size.height = height);
    if(win_size.width != width||win_size.height != height)
    {
        win_size.width = width;
        win_size.height = height;
        ret = ioctl(isp_fd, ISP_IOC_SET_SENSOR_SIZE, &win_size);usleep(100);
        ret = ioctl(isp_fd, ISP_IOC_SET_SIZE, &win_size);usleep(100);
        return 1;
    }
    return ret;
}

int get_Sensor_CurGain(int* value)
{
    int ret;
    u32 gain;
    if(NULL == value)
        return -1;

    if (isp_fd < 0)
        ov2715_init();

    ret = ioctl(isp_fd, ISP_IOC_GET_SENSOR_GAIN, &gain);
    // grdLog_debug(GRD_LOG_VIDEO, "Gain = %d\n", gain);
    info("-------------->Read  CurGain = %d\n", gain);
    *value = gain;
    return ret;
}

int get_Sensor_CurEV(int* value)
{
    int ret;
    u32 CurEV;
    if(NULL == value)
        return -1;

    if (isp_fd < 0)
        ov2715_init();

    ret = ioctl(isp_fd, AE_IOC_GET_EV_VALUE, &CurEV);
    *value = CurEV;
    return ret;
}


int ov9715_control(int type, int data)
{
	int ret = -1;
	int ISP_Data;

	switch (type)
	{
		case Brightness: //0~50
			{
				//value = data * 50/ 100; //randal
				ioctl(isp_fd, ISP_IOC_GET_BRIGHTNESS, &ISP_Data);
				if(ISP_Data != data)
				ret = ioctl(isp_fd, ISP_IOC_SET_BRIGHTNESS, &data);
			}
			break;
		case Contrast: //0~10
			{
				//value = data * 10 / 100; //randal
				ioctl(isp_fd, ISP_IOC_GET_CONTRAST, &ISP_Data);
				if(ISP_Data != data)
				ret = ioctl(isp_fd, ISP_IOC_SET_CONTRAST, &data);
			}
			break;
		case Hue: //Hue: 0~32
			{
				//value = data * 32 / 100; //randal
				ioctl(isp_fd, ISP_IOC_GET_HUE, &ISP_Data);
				if(ISP_Data != data)
				ret = ioctl(isp_fd, ISP_IOC_SET_HUE, &data);
			}
			break;
		case Saturation: //0~16
			{
				//value = data * 16 / 100; //randal
				ioctl(isp_fd, ISP_IOC_GET_SATURATION, &ISP_Data);
				printf("---->old saturation is %d\n",ISP_Data);
				if(ISP_Data != data)
				ret = ioctl(isp_fd, ISP_IOC_SET_SATURATION, &data);
			}
			break;
		case HMirror:
			{
				ioctl(isp_fd, ISP_IOC_GET_SENSOR_MIRROR, &ISP_Data);
				if(ISP_Data != data)
				ret = ioctl(isp_fd, ISP_IOC_SET_SENSOR_MIRROR, &data);
			}
			break;
		case VMirror:
			{
				ioctl(isp_fd, ISP_IOC_GET_SENSOR_FLIP, &ISP_Data);
				if(ISP_Data != data)
				ret = ioctl(isp_fd, ISP_IOC_SET_SENSOR_FLIP, &data);
			}
			break;
		case Powerfreq:
			ret = ov2715_ae_pwr_freq(data); // 1=50Hz 0=60Hz. 电源频率需要正确设置，否则画面会闪烁
			break;
		case Awbmode: // 色温调整
			// ret = adjust_awb_scene_mode(data);
			break;
		case Sharpness:
			{
				ioctl(isp_fd, ISP_IOC_GET_SHARPNESS, &ISP_Data);
				if(ISP_Data != data)
				ret = ioctl(isp_fd, ISP_IOC_SET_SHARPNESS , &data);
			}
			break;

		case Denoise:
			{
				ioctl(isp_fd, ISP_IOC_GET_DENOISE, &ISP_Data);
				if(ISP_Data != data)
				ret = ioctl(isp_fd, ISP_IOC_SET_DENOISE , &data);
			}
			break;
		case Gama:
			// ret = ioctl(isp_fd, ISP_IOC_SET_GAMMA, &data);
			break;
		case AETargetY:
			{
				ioctl(isp_fd, AE_IOC_GET_TARGETY, &ISP_Data);
				if(ISP_Data != data)
				ret = ioctl(isp_fd, AE_IOC_SET_TARGETY, &data);
			}
			break;
		case AEConvergeSpeed:
			{
				ioctl(isp_fd, AE_IOC_GET_SPEED, &ISP_Data);
				if(ISP_Data != data)
				ret = ioctl(isp_fd, AE_IOC_SET_SPEED, &data);
			}
			break;
		case nAEEnabled:
			{
				ioctl(isp_fd, AE_IOC_GET_ENABLE, &ISP_Data);
				if(ISP_Data != data)
				ret = ioctl(isp_fd,AE_IOC_SET_ENABLE,&data);
			}
			break;
		case AEMaxExpTime:
			{
				if(data > 0){
					ioctl(isp_fd, AE_IOC_GET_MAX_EXP, &ISP_Data);
					if(ISP_Data != data)
					ret = ioctl(isp_fd, AE_IOC_SET_MAX_EXP, &data);
				}else{
					ret = -1;
				}
			}
			break;
		case AEMinExpTime:
			{
				if(data > 0){
					ioctl(isp_fd, AE_IOC_GET_MIN_EXP, &ISP_Data);
					if(ISP_Data != data)
						ret = ioctl(isp_fd, AE_IOC_SET_MIN_EXP,  &data);
				}else{
					ret = -1;
				}
			}
			break;
		case AEMaxGain:
			{
				ioctl(isp_fd, AE_IOC_GET_MAX_GAIN, &ISP_Data);
				warning("---------->ISP_Data  %d \n",ISP_Data);
				warning("data  %d \n",data);
				if(ISP_Data != data)
				ret = ioctl(isp_fd, AE_IOC_SET_MAX_GAIN, &data);
			}
			break;
		case AEMinGain:
			{
				ioctl(isp_fd, AE_IOC_GET_MIN_GAIN, &ISP_Data);
				if(ISP_Data != data)
				ret = ioctl(isp_fd, AE_IOC_SET_MIN_GAIN, &data);
			}
			break;
        case drcStrength:
            {
                    ioctl(isp_fd, ISP_IOC_GET_DRC_STRENGTH,  &ISP_Data);
                    if(ISP_Data != data)
                            ret = ioctl(isp_fd, ISP_IOC_SET_DRC_STRENGTH,  &data);
            }
            break;
        case senCurgain:
            {
                    ioctl(isp_fd, ISP_IOC_GET_SENSOR_GAIN, &ISP_Data);
                    if(ISP_Data != data)
                            ret = ioctl(isp_fd, ISP_IOC_SET_SENSOR_GAIN, &data);
            }
            break;
        case AdjustNr:
            {
                    ioctl(isp_fd, ISP_IOC_GET_ADJUST_NR_EN, &ISP_Data);
                    if(ISP_Data != data)
                            ret = ioctl(isp_fd, ISP_IOC_SET_ADJUST_NR_EN, &data);
            }
            break;
        case SenFps:
            {
                    ioctl(isp_fd, ISP_IOC_GET_SENSOR_FPS, &ISP_Data);
                    if(ISP_Data != data)
                            ret = ioctl(isp_fd, ISP_IOC_SET_SENSOR_FPS, &data);
            }
            break;
		default:
			break;
	}

	//if (ret < 0)
	//	error( "Start ISP IO Control Enable fail, type=%d[%s], data=%d\n", type, color_2715[type], data);

	// close(isp_fd);
	// isp_fd = -1;
       usleep(100);
	return ret;
}
