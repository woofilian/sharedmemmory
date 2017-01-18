//must define _GNU_SOURCE for using CLOCK_MONOTONIC
#define _GNU_SOURCE

#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include "sdk_global.h"
#include "sdk_struct.h"
#include "../../codec_type.h"
#include "gmlib.h"
#include "motion_detection.h"
#include "motion.h"
#include "log.h"
#include "adapt.h"

#define MAX_SUB_REGION_NUM  4

static motion_handle_t g_motion_handle;
mdt_alg_t mdt_alg[MAX_CAP_CH_NUM] = {{sub_region: NULL}};

struct mdt_result_t mdt_result[MAX_CAP_CH_NUM] = {{sub_region: NULL}};
static  int g_motion_capture;

MotionControl  g_structMotionCtl;


static int set_cap_motion(int cap_vch, unsigned int id, unsigned int value)
{
    int ret = 0;
    gm_cap_motion_t cap_motion;

    cap_motion.id = id; //alpha
    cap_motion.value = value;

    ret = gm_set_cap_motion(cap_vch, &cap_motion);
    if (ret < 0)
    {
        printf("MD_Test: gm_set_cap_motion error!\n");
        return -1;
    }
    return 0;
}

int init_motion_param()
{
 	int ret = 0;
    int ch = 0, mb_w_num = 0,mb_h_num = 0;
    memset(mdt_alg,0,sizeof(mdt_alg_t)*MAX_CAP_CH_NUM);

    mdt_alg[ch].u_width = g_motion_handle.max_width;
    mdt_alg[ch].u_height = g_motion_handle.max_height;
    mdt_alg[ch].u_mb_width = 32;
    mdt_alg[ch].u_mb_height = 32;
    mdt_alg[ch].training_time = 15;
    mdt_alg[ch].frame_count = 0;
    mdt_alg[ch].sensitive_th = 80; //灵敏度
    //计算宏块  个数
    mb_w_num  = (mdt_alg[ch].u_width + (mdt_alg[ch].u_mb_width - 1)) / mdt_alg[ch].u_mb_width;
    mb_h_num  = (mdt_alg[ch].u_height + (mdt_alg[ch].u_mb_height - 1)) / mdt_alg[ch].u_mb_height;
    mdt_alg[ch].mb_w_num = mb_w_num;
    mdt_alg[ch].mb_h_num = mb_h_num;

	mdt_alg[ch].sub_region = (struct mdt_reg_t *)malloc(sizeof(struct mdt_reg_t) * MAX_MOTION_NUM);
    if (mdt_alg[ch].sub_region == NULL)
    {
        ret = MOTION_INIT_ERROR;
        goto err_ext;
    }
	memset(mdt_alg[ch].sub_region, 0, sizeof(struct mdt_reg_t) * MAX_MOTION_NUM);
	return 0;

err_ext:
	return ret;
}

unsigned long long UTIL_GetSystemUpUsecs()
{
    /*
    struct timespec {
    time_t tv_sec; // seconds 
    long tv_nsec; // nanoseconds 
    };
    */
    struct timespec structTimeSpec;
    clock_gettime(CLOCK_MONOTONIC,&structTimeSpec);
    return (unsigned long long)structTimeSpec.tv_sec*1000*1000 + (unsigned long long)structTimeSpec.tv_nsec/1000;
}

static void *motion_thread(void *arg)
{
    int ch;
    int ret;
    gm_multi_cap_md_t *cap_md = NULL;
    int reg_idx;
	unsigned long long ullSysUpTime,ullBakTime;
	int mdTrigger;

    cap_md = (gm_multi_cap_md_t *) malloc(sizeof(gm_multi_cap_md_t) * MAX_CAP_CH_NUM);
    if (cap_md == NULL)
    {
        printf("Error to allocate capture motion info!\n");
        goto thread_exit;
    }
    memset((void *) cap_md, 0, (sizeof(gm_multi_cap_md_t) * MAX_CAP_CH_NUM));
    for (ch = 0; ch < MAX_CAP_CH_NUM; ch++)
    {
        cap_md[ch].bindfd = g_motion_handle.motion_fd;
        cap_md[ch].cap_md_info.md_buf_len = CAP_MOTION_SIZE;
        cap_md[ch].cap_md_info.md_buf = (char *) malloc(CAP_MOTION_SIZE);
		warning("parameter to get motion data g_motion_handle.motion_fd[%d]\n",g_motion_handle.motion_fd);
        if (cap_md[ch].cap_md_info.md_buf == NULL)
        {
            printf("Error to allocate capture motion buffer!\n");
            goto thread_exit;
        }
    }
    for (ch = 0; ch < MAX_CAP_CH_NUM; ch++)
    {
        mdt_result[ch].sub_region = (struct mdt_reg_result_t *)
                                    malloc(sizeof(struct mdt_reg_result_t) * MAX_MOTION_NUM);
        if (mdt_result[ch].sub_region == NULL)
        {
            printf("Error to allocate result of sub_region info!\n");
            goto thread_exit;
        }
        mdt_result[ch].sub_region_num = MAX_MOTION_NUM;  //最大侦测区域个数
    }
	g_motion_capture =1;
    while (g_motion_capture)
    {
    	//warning("start g_motion_handle.motion_enable %d\n",g_motion_handle.motion_enable);
    	if(g_motion_handle.motion_enable)
    	{
			ullSysUpTime = UTIL_GetSystemUpUsecs(); // return microsecond
			
	        ret = gm_recv_multi_cap_md(cap_md, MAX_CAP_CH_NUM);
	        if (ret < 0)   //-1:error,0:sucess
	        {
	            printf("Error parameter to get motion data g_motion_handle.motion_fd[%d]\n",g_motion_handle.motion_fd);
	            continue;
	        }
	        ret = motion_detection_handling(cap_md, &mdt_result[0], MAX_CAP_CH_NUM);
	        if (ret < 0)   //-1:error,0:sucess
	        {
	            printf("Error to do motion_detection_handling\n");
	            goto thread_exit;
	        }
	        for (ch = 0; ch < MAX_CAP_CH_NUM; ch++)
	        {
	            if (mdt_result[ch].ch_result == MOTION_IS_TRAINING)
	            {
	                printf("[---Motion Is training---] at CH(%d)\n", ch);
	            }
	            else if (mdt_result[ch].ch_result == MOTION_ALGO_ERROR)
	            {
	                printf("[---Motion Algorithm Error---] at CH(%d)\n", ch);
	            }
	            else if (mdt_result[ch].ch_result == MOTION_PARSING_ERROR)
	            {
	                printf("[---Paring data error---] at CH(%d)\n", ch);
	            }
	            else if (mdt_result[ch].ch_result == MOTION_IS_READY)
	            {
	                //  sub-region MD
	                for (reg_idx = 0; reg_idx < MAX_MOTION_NUM; reg_idx++)
	                {
	                    if (mdt_result[ch].sub_region[reg_idx].reg_result == MOTION_DETECTED)
	                    {
	                        printf("[---sub-region %d Has Motion Detected at CH(%d)---]\n",
	                               reg_idx, ch);

							mdTrigger = 1;
							#if 0
							uint32_t alarm_level = 2;
							uint32_t msg_type = 1;//DANA_VIDEO_PUSHMSG_MSG_TYPE_MOTION_DETECT;
							char	 *msg_title = "TEST danavideo_alarm_cloud";
							char	 *msg_body	= "lib_danavideo_util_pushmsg danavideo_alarm_cloud";
							int64_t  cur_time = time(NULL);
							uint32_t att_flag = 0;
							int64_t  start_time = time(NULL);
							
							uint32_t chan_no = 1;
							uint32_t save_site = 0;
							uint32_t record_flag = 0;
							if (lib_danavideo_util_pushmsg(chan_no, alarm_level, msg_type, msg_title, msg_body, cur_time, att_flag, NULL, NULL, record_flag, 0, 0, 0, NULL)) {
								printf("\x1b[32mtestdanavideo TEST lib_danavideo_util_pushmsg success\x1b[0m\n");
							} else {
								printf("\x1b[34mtestdanavideo TEST lib_danavideo_util_pushmsg failed\x1b[0m\n");
							}
							#endif
						    //push_alarm_info(ch,SDK_EVENT_MOTION,(void *)reg_idx); //移动这侧发生
	                    }
	                }
	            }
	            else
	            {
	                printf("[---Motion Message not Defined Error---] at CH(%d)\n", ch);
	            }
	        }

			if(ullSysUpTime-ullBakTime > 1000*1000LL)
			{
				if(mdTrigger == 1)
				{
					mdTrigger = 0;
					g_structMotionCtl.ullbitsMDstate <<= 1; 
					g_structMotionCtl.ullbitsMDstate |= 1; 
				}
				else
				{
					g_structMotionCtl.ullbitsMDstate <<= 1; 
				}
				ullBakTime = ullSysUpTime;
			}
    	}
        usleep(100000);   //one second period to detect motion
    }

thread_exit:

    if (cap_md)
    {
        for (ch = 0 ; ch < MAX_CAP_CH_NUM; ch++)
        {
            if (cap_md[ch].cap_md_info.md_buf)
                free(cap_md[ch].cap_md_info.md_buf);
        }
    }

    if (cap_md)
        free(cap_md);

    for (ch = 0; ch < MAX_CAP_CH_NUM; ch++)
    {
        if (mdt_result[ch].sub_region)
        {
            free(mdt_result[ch].sub_region);
            mdt_result[ch].sub_region = NULL;
        }
    }

    motion_detection_end();

    return 0;
}

/*******************************/
int init_motion_server()
{
    gm_system_t grain_system;
    memset(&g_motion_handle,0,sizeof(motion_handle_t));

    get_sys_info(&grain_system);
    g_motion_handle.motion_fd = get_video_bind_handle(0,0);
    g_motion_handle.max_width = grain_system.cap[0].dim.width;
    g_motion_handle.max_height = grain_system.cap[0].dim.height;
	warning("grain_system.cap[0].dim.width [%d],grain_system.cap[0].dim.height [%d]\n",grain_system.cap[0].dim.width,grain_system.cap[0].dim.height);
    if(init_motion_param()< 0)
    {
        g_motion_handle.motion_init = 0;
        return -1;
	}

	int width = g_motion_handle.max_width;
	int height = g_motion_handle.max_height;
    if(motion_detection_init(width, height) < 0)//motion detection initial(移动侦测初始化)
    {
        g_motion_handle.motion_init = 0;
        return -1;
    }
    g_motion_handle.motion_init = 1;
	//init_motion_param();

    return 0;
}
int uninit_motion_server()
{
    motion_detection_end();
    g_motion_handle.motion_init = 0;
    return 0;
}

//设置移动侦测的参数
int set_motion_param(int cap_ch, sdk_vda_codec_cfg_t *motion_cfg)
{
	int reg_idx;
	int mb_w_num, mb_h_num;
    int ch = 0;
    int ret = -1;
	int md_width, md_height;
	int h, w, i;
#if 0
	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n");
	printf("++++++++++++++++++QJC  wo ri o++++++++++++++++++++++++++ \n");
	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n");	
	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n");
	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n");
	printf(" motion_cfg->area[0].x  is %d \n",motion_cfg->area[0].x);
	printf(" motion_cfg->area[0].y  is %d \n",motion_cfg->area[0].y);
	printf(" motion_cfg->area[0].width   is %d  \n",motion_cfg->area[0].width);
	printf(" motion_cfg->area[0].height is %d  \n",motion_cfg->area[0].height);
	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n");
	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n");
	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n");	
	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n");
	printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++ \n");
#endif
	
    if(!g_motion_handle.motion_init)
    {
        error("motion sys not init..... \n");
        return -1;
    }
    if(!motion_cfg)
    {
        error("set_motion_param motion_cfg == NULL \n");
        return -1;
    }
	md_width = g_motion_handle.max_width;
	md_height = g_motion_handle.max_height;
	
	mdt_alg[ch].u_mb_width = 32;
    mdt_alg[ch].u_mb_height = 32;
    mdt_alg[ch].training_time = 15;
    mdt_alg[ch].frame_count = 0;
    mdt_alg[ch].u_width = md_width;//1280;
    mdt_alg[ch].u_height = md_height;//720;
    
    int block_width = mdt_alg[ch].u_mb_width;
    int block_height = mdt_alg[ch].u_mb_height;
	
	mb_w_num  = (mdt_alg[ch].u_width + (mdt_alg[ch].u_mb_width - 1)) / mdt_alg[ch].u_mb_width;
	mb_h_num  = (mdt_alg[ch].u_height + (mdt_alg[ch].u_mb_height - 1)) / mdt_alg[ch].u_mb_height;
	//warning("mb_w_num  %d     mb_h_num%d\n", mb_w_num,mb_h_num);
	#if 1
	for(reg_idx = 0;reg_idx < 4;reg_idx++)
	{
	    /* set interesting region[0]-[4]*/
		if(TEST_BIT(motion_cfg->mask, reg_idx+1))
		{
	    	mdt_alg[ch].sub_region[reg_idx].is_enabled  = 1;
		}
	    mdt_alg[ch].sub_region[reg_idx].start_block_x = mb_w_num * motion_cfg->area[reg_idx].x /md_width;
	    mdt_alg[ch].sub_region[reg_idx].start_block_y = mb_h_num * motion_cfg->area[reg_idx].y /md_height;
	    mdt_alg[ch].sub_region[reg_idx].end_block_x   = mb_w_num * motion_cfg->area[reg_idx].width /md_width - 1;
	    mdt_alg[ch].sub_region[reg_idx].end_block_y   = mb_h_num * motion_cfg->area[reg_idx].height /md_height - 1;
	    mdt_alg[ch].sub_region[reg_idx].alarm_th      = motion_cfg->sensitive * 20;
	    mdt_alg[ch].sub_region[reg_idx].alarm         = NO_MOTION;
		//warning("enable[%d] regidx[%d]  area [%d][%d][%d][%d]\n",mdt_alg[ch].sub_region[reg_idx].is_enabled,reg_idx,motion_cfg->area[reg_idx].x,motion_cfg->area[reg_idx].y,motion_cfg->area[reg_idx].width,motion_cfg->area[reg_idx].height);
		warning("MD_PARAM-->[%d][%d][%d][%d]\n",mdt_alg[ch].sub_region[reg_idx].start_block_x,mdt_alg[ch].sub_region[reg_idx].start_block_y, mdt_alg[ch].sub_region[reg_idx].end_block_x, mdt_alg[ch].sub_region[reg_idx].end_block_y);
	}
	#endif
	#if 0
	  reg_idx = 0;
    /* set interesting region[0]*/
    mdt_alg[ch].sub_region[reg_idx].is_enabled    = 1;
    mdt_alg[ch].sub_region[reg_idx].start_block_x = 0;
    mdt_alg[ch].sub_region[reg_idx].start_block_y = 0;
    mdt_alg[ch].sub_region[reg_idx].end_block_x   = mb_w_num / 2;
    mdt_alg[ch].sub_region[reg_idx].end_block_y   = mb_h_num / 2;
    mdt_alg[ch].sub_region[reg_idx].alarm_th      = 80;
    mdt_alg[ch].sub_region[reg_idx].alarm         = NO_MOTION;
    reg_idx++;

    /* set interesting region[1]*/
    mdt_alg[ch].sub_region[reg_idx].is_enabled    = 1;
    mdt_alg[ch].sub_region[reg_idx].start_block_x = mb_w_num / 2;
    mdt_alg[ch].sub_region[reg_idx].start_block_y = 0;
    mdt_alg[ch].sub_region[reg_idx].end_block_x   = mb_w_num - 1;
    mdt_alg[ch].sub_region[reg_idx].end_block_y   = mb_h_num / 2;
    mdt_alg[ch].sub_region[reg_idx].alarm_th      = 80;
    mdt_alg[ch].sub_region[reg_idx].alarm         = NO_MOTION;
    reg_idx++;

    /* set interesting region[2]*/
    mdt_alg[ch].sub_region[reg_idx].is_enabled    = 1;
    mdt_alg[ch].sub_region[reg_idx].start_block_x = 0;
    mdt_alg[ch].sub_region[reg_idx].start_block_y = mb_h_num / 2;
    mdt_alg[ch].sub_region[reg_idx].end_block_x   = mb_w_num / 2;
    mdt_alg[ch].sub_region[reg_idx].end_block_y   = mb_h_num - 1;
    mdt_alg[ch].sub_region[reg_idx].alarm_th      = 80;
    mdt_alg[ch].sub_region[reg_idx].alarm         = NO_MOTION;
    reg_idx++;

    /* set interesting region[3]*/
    mdt_alg[ch].sub_region[reg_idx].is_enabled    = 1;
    mdt_alg[ch].sub_region[reg_idx].start_block_x = mb_w_num / 2;
    mdt_alg[ch].sub_region[reg_idx].start_block_y = mb_h_num / 2;
    mdt_alg[ch].sub_region[reg_idx].end_block_x   = mb_w_num - 1;
    mdt_alg[ch].sub_region[reg_idx].end_block_y   = mb_h_num - 1;
    mdt_alg[ch].sub_region[reg_idx].alarm_th      = 80;
    mdt_alg[ch].sub_region[reg_idx].alarm         = NO_MOTION;
    reg_idx++;
	#endif
    mdt_alg[ch].sub_region_num = reg_idx;
	

	warning("mb_w_num  %d   mb_h_num  %d\n",mb_w_num,mb_h_num);
	#if 0
	if(mdt_alg[ch].sub_region[0].is_enabled || mdt_alg[ch].sub_region[1].is_enabled ||mdt_alg[ch].sub_region[2].is_enabled ||mdt_alg[ch].sub_region[3].is_enabled )
	{
		g_motion_handle.motion_enable = 1;
	}
	else
	{
		g_motion_handle.motion_enable = 0;
	}
	#endif
	#if 1
	if(motion_cfg->sensitive > 0)
	{
		g_motion_handle.motion_enable = 1;
	}
	else
	{
		g_motion_handle.motion_enable = 0;
	}
	#endif
	printf("g_motion_handle.motion_enable = %d\n",g_motion_handle.motion_enable);

#if 0  //old vaule
    set_cap_motion(ch, 0, 32); //alpha
    set_cap_motion(ch, 1, 7371); //tbg
    set_cap_motion(ch, 2, 7); //init val
    set_cap_motion(ch, 3, 9); //tb
    set_cap_motion(ch, 4, 11); //sigma
    set_cap_motion(ch, 5, 15); //prune
    set_cap_motion(ch, 7, 0x9ffb0); //alpha accuracy
    set_cap_motion(ch, 8, 9); //tg
    set_cap_motion(ch, 10, 0x7fe0); //one min alpha
#endif
	//new vaule  for night 
	set_cap_motion(ch, 0, 0xffff); //alpha
	set_cap_motion(ch, 1, 0x1ccb); //tbg
	set_cap_motion(ch, 2, 7); //init val
	set_cap_motion(ch, 3, 5); //tb
	set_cap_motion(ch, 4, 11); //sigma
	set_cap_motion(ch, 5, 15); //prune
	set_cap_motion(ch, 7, 0x7ffc0); //alpha accuracy
	set_cap_motion(ch, 8, 5); //tg
	set_cap_motion(ch, 10, 0x7fe0); //one min alpha


    ret = motion_detection_update(g_motion_handle.motion_fd, &mdt_alg[ch]);
    if (ret != 0)
    {
        printf("Error to do motion_detection_update at CH(%d)\n", ch);
        ret = -1;
        goto err_ext;
    }
	return 0;
err_ext:

    if (mdt_alg[ch].sub_region)
        free(mdt_alg[ch].sub_region);

    return ret;

}
//get_motion_handle
/**********************************/

#if 0
int set_interesting_region(int ch)
{
    int ret = 0;
    int mb_w_num, mb_h_num;
    int reg_idx;

    mdt_alg[ch].sub_region = (struct mdt_reg_t *) malloc(sizeof(struct mdt_reg_t) *
                                                              MAX_SUB_REGION_NUM);
    if (mdt_alg[ch].sub_region == NULL) {
        printf("Error to allocate sub_region\n");
        ret = -1;
        goto err_ext; 
    } 
    memset(mdt_alg[ch].sub_region, 0, (sizeof(struct mdt_reg_t) * MAX_SUB_REGION_NUM));
	sdk_motion_cfg_t motion_cfg;
	adapt_param_get_motion_cfg(ch, motion_cfg);
	#if 0
    mdt_alg[ch].u_width = 1280;
    mdt_alg[ch].u_height = 720;
    mdt_alg[ch].u_mb_width = 32;
    mdt_alg[ch].u_mb_height = 32;
    mdt_alg[ch].training_time = 15;
    mdt_alg[ch].frame_count = 0;
    mdt_alg[ch].sensitive_th = 80;
    mb_w_num  = (mdt_alg[ch].u_width + (mdt_alg[ch].u_mb_width - 1)) / 
                mdt_alg[ch].u_mb_width;
    mb_h_num  = (mdt_alg[ch].u_height + (mdt_alg[ch].u_mb_height - 1)) / 
                mdt_alg[ch].u_mb_height;
    mdt_alg[ch].mb_w_num = mb_w_num;
    mdt_alg[ch].mb_h_num = mb_h_num;
    reg_idx = 0;
    /* set interesting region[0]*/
    mdt_alg[ch].sub_region[reg_idx].is_enabled    = 1;
    mdt_alg[ch].sub_region[reg_idx].start_block_x = 0;
    mdt_alg[ch].sub_region[reg_idx].start_block_y = 0;
    mdt_alg[ch].sub_region[reg_idx].end_block_x   = mb_w_num / 2;
    mdt_alg[ch].sub_region[reg_idx].end_block_y   = mb_h_num / 2;
    mdt_alg[ch].sub_region[reg_idx].alarm_th      = motion_cfg.codec_vda_cfg.sensitive * 15;
    mdt_alg[ch].sub_region[reg_idx].alarm         = NO_MOTION;
    reg_idx++;

    /* set interesting region[1]*/
    mdt_alg[ch].sub_region[reg_idx].is_enabled    = 1;
    mdt_alg[ch].sub_region[reg_idx].start_block_x = mb_w_num / 2;
    mdt_alg[ch].sub_region[reg_idx].start_block_y = 0;
    mdt_alg[ch].sub_region[reg_idx].end_block_x   = mb_w_num - 1;
    mdt_alg[ch].sub_region[reg_idx].end_block_y   = mb_h_num / 2;
    mdt_alg[ch].sub_region[reg_idx].alarm_th      = motion_cfg.codec_vda_cfg.sensitive * 15;
    mdt_alg[ch].sub_region[reg_idx].alarm         = NO_MOTION;
    reg_idx++;

    /* set interesting region[2]*/
    mdt_alg[ch].sub_region[reg_idx].is_enabled    = 1;
    mdt_alg[ch].sub_region[reg_idx].start_block_x = 0;
    mdt_alg[ch].sub_region[reg_idx].start_block_y = mb_h_num / 2;
    mdt_alg[ch].sub_region[reg_idx].end_block_x   = mb_w_num / 2;
    mdt_alg[ch].sub_region[reg_idx].end_block_y   = mb_h_num - 1;
    mdt_alg[ch].sub_region[reg_idx].alarm_th      = motion_cfg.codec_vda_cfg.sensitive * 15;
    mdt_alg[ch].sub_region[reg_idx].alarm         = NO_MOTION;
    reg_idx++;

    /* set interesting region[3]*/
    mdt_alg[ch].sub_region[reg_idx].is_enabled    = 1;
    mdt_alg[ch].sub_region[reg_idx].start_block_x = mb_w_num / 2;
    mdt_alg[ch].sub_region[reg_idx].start_block_y = mb_h_num / 2;
    mdt_alg[ch].sub_region[reg_idx].end_block_x   = mb_w_num - 1;
    mdt_alg[ch].sub_region[reg_idx].end_block_y   = mb_h_num - 1;
    mdt_alg[ch].sub_region[reg_idx].alarm_th      = motion_cfg.codec_vda_cfg.sensitive * 15;
    mdt_alg[ch].sub_region[reg_idx].alarm         = NO_MOTION;
    reg_idx++;
	
    mdt_alg[ch].sub_region_num = reg_idx;
	#endif
	int block_width = 1280;//mdt_alg[ch].u_mb_width;
    int block_height = 720;//mdt_alg[ch].u_mb_height;
	for(reg_idx = 0;reg_idx <4;reg_idx++)
	{
	    /* set interesting region[0]*/
	    mdt_alg[ch].sub_region[reg_idx].is_enabled    = TEST_BIT(motion_cfg.codec_vda_cfg.mask, reg_idx+1);
	    mdt_alg[ch].sub_region[reg_idx].start_block_x = (motion_cfg.codec_vda_cfg.area[reg_idx].x + block_width -1)/block_width;;
	    mdt_alg[ch].sub_region[reg_idx].start_block_y = (motion_cfg.codec_vda_cfg.area[reg_idx].y + block_height -1)/block_height;
	    mdt_alg[ch].sub_region[reg_idx].end_block_x   = (motion_cfg.codec_vda_cfg.area[reg_idx].x+ motion_cfg.codec_vda_cfg.area[reg_idx].width + block_width -1)/block_width;
	    mdt_alg[ch].sub_region[reg_idx].end_block_y   = (motion_cfg.codec_vda_cfg.area[reg_idx].y+ motion_cfg.codec_vda_cfg.area[reg_idx].height + block_height -1)/block_height;
	    mdt_alg[ch].sub_region[reg_idx].alarm_th      = motion_cfg.codec_vda_cfg.sensitive *15;
	    mdt_alg[ch].sub_region[reg_idx].alarm         = NO_MOTION;
	}
	mdt_alg[ch].sub_region_num = MAX_MOTION_NUM;
    set_cap_motion(ch, 0, 32); //alpha
    set_cap_motion(ch, 1, 7371); //tbg
    set_cap_motion(ch, 2, 7); //init val
    set_cap_motion(ch, 3, 9); //tb
    set_cap_motion(ch, 4, 11); //sigma
    set_cap_motion(ch, 5, 15); //prune
    set_cap_motion(ch, 7, 0x9ffb0); //alpha accuracy
    set_cap_motion(ch, 8, 9); //tg
    set_cap_motion(ch, 10, 0x7fe0); //one min alpha

    ret = motion_detection_update(g_motion_handle.motion_fd, &mdt_alg[ch]);
    if (ret != 0) {
        printf("Error to do motion_detection_update at CH(%d)\n", ch);
        ret = -1;
        goto err_ext;
    }

err_ext:

    if (mdt_alg[ch].sub_region)
        free(mdt_alg[ch].sub_region);

    return ret;

}
#endif


int start_motion_server()
{
    int ret = 0;
	#if 0
    if(!g_motion_handle.motion_init)
    {
        error("motion server not init!!!! \n");
        return -1;
    }
	#endif
	memset(&g_structMotionCtl, 0, sizeof(g_structMotionCtl));
	sleep(5); //wait for video ready
    //g_motion_handle.motion_enable = 1;

    ret = pthread_create(&g_motion_handle.motion_thread_id, NULL, motion_thread, (void *) NULL);
    if (ret < 0)
    {
        error("create motion thread failed");
        goto end;
    }
    return 0;
end:
    if (g_motion_handle.motion_thread_id)
        pthread_join(g_motion_handle.motion_thread_id, NULL);
    return -1;
}

int stop_motion_server()
{
	//
	g_motion_capture=0;
    g_motion_handle.motion_enable = 0;
    if (g_motion_handle.motion_thread_id)
        pthread_join(g_motion_handle.motion_thread_id, NULL);
    motion_detection_end();
    return 0;
}



