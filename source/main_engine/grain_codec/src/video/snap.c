#include "gmlib.h"
#include "sdk_struct.h"
#include "video.h"
#include "log.h"

//#if 0
SendstreamCallback	mSnapCallback = NULL;
static sdk_snap_pic_t g_snap_attr;
static int g_snap_fd = -1;


#define MAX_SNAPHSOT_LEN    (128*1024)
//char  snapshot_buf[MAX_SNAPHSOT_LEN] = { 0 };

int _snap_init(int ch,SendstreamCallback snap_callback)
{
    if(snap_callback)
        mSnapCallback = snap_callback;
	
	g_snap_fd = get_video_bind_handle(0,0);
	warning("--->g_snap_fd  %d \n",g_snap_fd);
    return 0;
}
int _snap_unint(int ch)
{
	g_snap_fd = -1;
	mSnapCallback = NULL;
    return 0;
}

//默认通道就是0
int _set_snap_attr(int ch,sdk_snap_pic_t *snap_attr)
{
	if(snap_attr)
	memcpy(&g_snap_attr,snap_attr,sizeof(sdk_snap_pic_t));
	return 0;
}

//在8136s中这一个函数就可以搞定了
//void *video_snap_thread(void *arg)
//char    *snapBuffer = 0;
char    *snapshot_bufs = 0;

int _snap_process(sdk_snap_info_t *snap_info,char *path)
{
	static int filecount = 0;
	FILE    *snapshot_fd = NULL;
	
    int     snapshot_len = 0;
    char    filename[40];
    snapshot_t snapshot;
	warning(" snap_    %d \n",g_snap_fd);
	#if 0
	if(g_snap_fd < 0 ||!snap_info|| snap_info->data)
	{
		error(" snap_process  param error \n");
		return -1;
	}
	#endif
	snapshot_bufs = (char *)malloc(MAX_SNAPHSOT_LEN);
	if (snapshot_bufs == NULL) 
	{
        warning("alloc  error!!!!!!!!!!!!!!!!!!!!!\n");
    }
    snapshot.bindfd = g_snap_fd; 
    snapshot.image_quality = 30;//g_snap_attr.quilty;  // The value of image quality from 1(worst) ~ 100(best)
	snapshot.bs_buf = snapshot_bufs;
    snapshot.bs_buf_len = MAX_SNAPHSOT_LEN;
	if(snap_info->width > 0 && snap_info->height > 0)
	{
    	snapshot.bs_width = snap_info->width;
		snapshot.bs_height = snap_info->height;
	}
	else
	{
		snapshot.bs_width = 1280;//g_snap_attr.width   如何计算
		snapshot.bs_height = 720;//g_snap_attr.height
	}
    snapshot_len = gm_request_snapshot(&snapshot, 500); // Timeout value 500ms
	warning("---->g_snap_fd [%d]   snapshot_len [%d]  \n",g_snap_fd, snapshot_len);
	if (snapshot_len > 0)
    {
		snap_info->data_len = snapshot_len;
		sprintf(filename,"%s",path);
		if(strlen(filename) < 1)
		{
			warning("no file path now !!!!!!\n");
        	sprintf(filename, "/mnt/mtd/snap/snapshot_%d.jpg", path);
		}
		
        printf("Get %s size %d bytes\n", filename, snapshot_len);
        snapshot_fd = fopen(filename, "wb");
        if (snapshot_fd == NULL) 
		{
            printf("Fail to open file %s\n", filename);
            //exit(1);
			return -1;
        }
        fwrite(snapshot_bufs, 1, snapshot_len, snapshot_fd);
		warning("write snap buffer to file success!!!!!!!\n");
        fclose(snapshot_fd);
		free(snapshot_bufs);
		return 0;
    }
	else
	{
		error("snapshot_len :%d  \n",snapshot_len);
	}
	free(snapshot_bufs);
	return -1;
}


#if 0
//抓拍消耗时间 创建线程解决
int _snap_process(int ch,int snap_type,int snap_num)
{

    pthread_attr_t attr;
    int ret ;
    init_write_fd();

    video_capture_start();
    /* Record Thread */
    if (video_capture_id == (pthread_t)NULL)
    {
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);  //分离的线程
        ret = pthread_create(&video_capture_id, &attr, &video_capture_thread, NULL);
        pthread_attr_destroy(&attr);
    }
    else
    {
        info("video_capture_thread is already runing..... \n");
    }

    return 0;
}
#endif

//#endif
