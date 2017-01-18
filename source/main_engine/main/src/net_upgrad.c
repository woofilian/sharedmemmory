#include "wrapper.h"
#include "log.h"
#include "upgrade.h"
#include "sdk_struct.h"

#define NO_CHAN_ITEM			0		// 不用记录通道项
#define NO_ALMIN_ITEM			0		// 不用记录报警输入项

#define WRITE_LOG(major_type, minor_type, ch, almin_no)\
    do{\
		uint32_t args = (ch) | (almin_no);\
        sdk_log_write(major_type, minor_type, args, "");\
    }while(0)


typedef struct pu_upg_handle_s
{
    pthread_t report_process;
    int upgrad_session;

    int file_fd;
    int total_len;
    int count_len;
    int packet_num;
    int total_packet;
} pu_upg_handle_t;

//#define DEFAULT_UPG_FILE "/mnt/nfs/upgrade_packet"
#define DEFAULT_UPG_FILE "/tmp/upgrade_packet"

static pu_upg_handle_t pu_upg_handle = {0,0,-1,0,0};


static void _reset_upg()
{
    if(pu_upg_handle.file_fd >= 0)
    {
        close(pu_upg_handle.file_fd);
        memset(&pu_upg_handle, 0, sizeof(pu_upg_handle));
        pu_upg_handle.file_fd = -1;
    }

}

int _msg_net_upgrade_req(sdk_msg_t *pmsg)
{
    sdk_upgrad_pkg_info_t *upgrad_pkg_info = (sdk_upgrad_pkg_info_t *)pmsg->data;

    printf("_msg_upgrade_req\n");
    if (upgrade_tid_runing())
    {
        error("upgrade running ......\n");
        pmsg->ack = SDK_ERR_UPGRADING;
        pmsg->size = 0;
        return -1;
    }
    // 初始化一些升级参数
    info("upg total_len :%d,total_packet:%d\n", upgrad_pkg_info->total_len, upgrad_pkg_info->total_packet);
    if(pu_upg_handle.report_process != 0)
    {
        error("network upgrading now, please try later!\n");
        pmsg->ack = SDK_ERR_UPGRADING;
        pmsg->size = 0;
        return -1;
    }
    _reset_upg();
    pu_upg_handle.file_fd = open(DEFAULT_UPG_FILE, O_CREAT|O_RDWR|O_TRUNC, 0766);
    if(pu_upg_handle.file_fd < 0)
    {
        error("opne file %s\n", DEFAULT_UPG_FILE);
        return -1;
    }
    pu_upg_handle.total_len = upgrad_pkg_info->total_len;
    pu_upg_handle.total_packet = upgrad_pkg_info->total_packet;
    pu_upg_handle.upgrad_session = upgrad_pkg_info->upgrad_session = os_get_rand(0, 0xFFFFFFFF);
	info("========= upgrad_session:%d \n", pu_upg_handle.upgrad_session);
	pu_upg_handle.packet_num = 0;
    // 重置升级进度为0
    upgrade_set_percent(0);
    return 0;
}
int _msg_net_upgrade_recv(sdk_msg_t *pmsg)
{
    sdk_bin_stream_t *bin_stream = (sdk_bin_stream_t *)pmsg->data;
    int ret = 0;
    int is_file = 1;
	info("upgrade file_fd:%d upgrad_session:%d flag:%d \n",pu_upg_handle.file_fd,pu_upg_handle.upgrad_session , bin_stream->flag);
    if((pu_upg_handle.file_fd < 0 )|| (pu_upg_handle.upgrad_session != bin_stream->flag))
    {
        error("fd < 0, drop packet!\n");
        return -1;
    }
	info("writen file seq:%d ==== size:%d  \n",bin_stream->seq,bin_stream->size);
    ret = os_writen(pu_upg_handle.file_fd, bin_stream->data, bin_stream->size);
    pu_upg_handle.count_len += bin_stream->size;
    pu_upg_handle.packet_num++;

    //发送完成 通知主程序可以升级了
    if(pu_upg_handle.packet_num == pu_upg_handle.total_packet
            && pu_upg_handle.count_len == pu_upg_handle.total_len)
    {
    	info("TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT \n");
        _reset_upg();

        ret = upgrade_start(DEFAULT_UPG_FILE, is_file);
        if (ret < 0)
        {
            error("upgrade_start error!\n");
            pmsg->ack = SDK_ERR_UPGRADE;
            pmsg->size = 0;
            return -1;
        }

        WRITE_LOG(LOG_MAJOR_OPERATE, L_OPERATE_MINOR_UPGRADE, NO_CHAN_ITEM, NO_ALMIN_ITEM);
        sdk_log_flush();
    }
	#if 0
    //发送完成 通知主程序可以升级了
    else
    {
        _reset_upg();
        error("upgrade_start error!\n");
        pmsg->ack = SDK_ERR_UPGRADE;
        pmsg->size = 0;
        return -1;

    }
	#endif
	pmsg->size = 0;
	return 0;
}




