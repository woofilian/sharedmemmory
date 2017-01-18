
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


#define MAX_BLOCK_SIZE (4096 - (sizeof(sdk_msg_t)) - (sizeof(sdk_bin_stream_t)))

//#define MAX_BLOCK_SIZE  8192
#define MSG_BUF_SIZE 10*1024
typedef enum _SDK_UPGRAD_OP
{
    //网络接收升级文件
    SDK_UPGRAD_NET_REQUEST	 = 0x0 // 升级请求 sdk_upgrad_pkg_info_t
                               ,SDK_UPGRAD_NET_RECV_UPG	 	  // 接收数据流sdk_bin_stream
    //开始升级的流程
    , SDK_UPGRAD_REQUEST     		//升级请求	upgrade_packet_t
    , SDK_UPGRAD_DISK_FILE          //升级包在磁盘文件
    , SDK_UPGRAD_MEM_FILE           //升级包在内存
    , SDK_UPGRAD_PROGRESS           //升级进度
    , SDK_UPGRAD_FIND_FILE          //U盘查询升级包文件列表	upgrad_pkg_file_t
} SDK_UPGRAD_OP_E;


typedef struct sdk_msg_s
{
    uint16_t magic;         //保留，扩展(如设备类型) 可作为发送消息的模块识别码
    uint16_t version;       //版本号
#if 1
    uint16_t msg_id;        //命令ID
    uint8_t  msg_dir;       //（SDK_PARAM_DIR_E)
    uint8_t  ack;           //错误码
#endif
    uint32_t args;          //SDK_PARAM_DIR_E  命令参数(设置、获取参数;或者是子命令类型[用户管理、升级、磁盘管理])
    uint32_t extend;        //扩展字段(8:pkg_total,8:pkg_num, 8:, 8:)
    uint32_t chann;         //通道号(CHANN_TYPE_ALL)
    uint32_t size;          //消息长度
    uint8_t  data[0];       //消息体
} __attribute__ ((packed))sdk_msg_t;

/*
*发送二进制数据流
*/
typedef struct _sdk_bin_stream_s

{
    uint32_t type;							//二进制数据类型
    uint32_t flag;							//包的标志
    uint32_t seq;							//包的序号
    uint32_t size;							//整个结构体的 size
    uint8_t data[0];						//附加二进制数据起始位置
} sdk_bin_stream_t;

//SDK_MAIN_MSG_UPGRAD
typedef struct _sdk_upgrad_pkg_info_s
{
    uint32_t upgrad_session;   // 服务端返回给客户端一个回话ID
    uint32_t total_packet;     //  总的包数
    uint32_t total_len;		  // 升级文件总长度
} sdk_upgrad_pkg_info_t;


#if 0
unsigned long get_file_size(const char *path)
{
    unsigned long filesize = -1;
    FILE *fp;
    fp = fopen(path, "r");
    if(fp == NULL)          return filesize;
    fseek(fp, 0L, SEEK_END);
    filesize = ftell(fp);
    fclose(fp);
    return filesize;
}
#endif

unsigned long get_file_size(const char *path)
{
    unsigned long filesize = -1;
    struct stat statbuff;
    if(stat(path, &statbuff) < 0)
    {
        return filesize;
    }
    else
    {
        filesize = statbuff.st_size;
    }
    return filesize;
}
static int connect_nonb(int sockfd, const struct sockaddr *saptr, socklen_t salen, int sec)
{
	int	tcpBufSize = 4*1024;

    int flags, n, error;
    socklen_t len;
    fd_set rset,wset;
    struct timeval tval;

    flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd,F_SETFL, flags|O_NONBLOCK);

    error = 0;
    if((n = connect(sockfd, saptr, salen)) < 0)
        if(errno!=EINPROGRESS)
            return(-1);
    if(n == 0)
        goto done;

    FD_ZERO(&rset);
    FD_SET(sockfd,&rset);
    wset = rset;
    tval.tv_sec = sec;
    tval.tv_usec =0;

    if(select(sockfd+1, &rset, &wset, NULL, sec?&tval:NULL) == 0)
    {
        close(sockfd);
        errno =ETIMEDOUT;
        return(-1);
    }
    if(FD_ISSET(sockfd,&rset) || FD_ISSET(sockfd,&wset))
    {
        len =sizeof(error);
        if(getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len)<0)
            return (-1);

	if(setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &tcpBufSize, sizeof(int)) == -1)
				return (-1);

    }
    else
    {
        printf("%s ===> select error: sockfd net set\n", __FUNCTION__);
    }
done:
    fcntl(sockfd, F_SETFL,flags);
    if(error)
    {
        close(sockfd);
        errno =error;
        return(-1);
    }
    return(0);
}
static int _connect_noblock(char *ip, unsigned short port, int timeout)
{
    int sock = -1;
    int reuseaddr = 1;
    struct sockaddr_in  msg_addr;

    sock  = socket(PF_INET,SOCK_STREAM,0);
    if(socket < 0)
    {
        printf("err: %s ===> create sock:%d < 0\n", __FUNCTION__, sock);
        return -1;
    }

    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*)&reuseaddr, sizeof(reuseaddr)) == -1)
    {
        printf("err: %s ===> setsockopt SO_REUSEADDR sock:%d\n", __FUNCTION__, sock);
        close(sock);
        return -1;
    }

    msg_addr.sin_family      = PF_INET;
    msg_addr.sin_addr.s_addr = inet_addr(ip);
    msg_addr.sin_port        = htons(port);

    if(connect_nonb(sock, (struct sockaddr *)&msg_addr,sizeof(struct sockaddr), timeout) < 0)
    {
        printf("err: %s ===> connect_nonb fail, sock:%d\n", __FUNCTION__, sock);
        close(sock);

        return -1;
    }
    printf("ok: %s ===> connect_nonb success, sock:%d\n", __FUNCTION__, sock);

    return sock;
}

static     pthread_mutex_t  msg_mutex;
static     pthread_cond_t   msg_cond;

int sync_single_init()
{
    pthread_cond_init(&(msg_cond), NULL);
    pthread_mutex_init(&(msg_mutex), NULL);
}

int sync_single_del()
{


    pthread_mutex_destroy(&msg_mutex);
    pthread_cond_destroy(&msg_cond);
    return 0;
}
int sync_single_wait()
{
    pthread_mutex_lock(&msg_mutex);
    pthread_cond_wait(&msg_cond, &msg_mutex);
    pthread_mutex_unlock(&msg_mutex);

    return 0;
}
int sync_single_post()
{
    pthread_mutex_lock(&msg_mutex);
    pthread_cond_signal(&msg_cond);
    pthread_mutex_unlock(&msg_mutex);

    return 0;
}
static int  g_upgrad_session;
static int  g_up_percent = 0;
static void * _msg_recv(int sockfd)
{
    int ret = 0;
    fd_set rset;
    struct timeval tval;
    char rcv_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pmsg = (sdk_msg_t *)rcv_buf;
    while(1)
    {
        FD_ZERO(&rset);
        FD_SET(sockfd,&rset);
        tval.tv_sec = 1;
        tval.tv_usec =0;

        ret = select(sockfd+1, &rset, NULL, NULL, &tval);
        if(ret < 0 )
        {
            printf(" select  error!!! \n");
            break;
        }
        else if(ret == 0 )
        {
           // printf("select  time out!!!  \n");
            continue;
        }

        if(ret = os_readn(sockfd, pmsg, sizeof(sdk_msg_t)) != sizeof(sdk_msg_t))
        {
            printf("_msg_recv 0000000000  \n");

            break;
        }

        if(pmsg->size)
        {
            pmsg->size = (pmsg->size <= (MSG_BUF_SIZE-sizeof(sdk_msg_t)))
                         ? pmsg->size:(MSG_BUF_SIZE-sizeof(sdk_msg_t));

            if((ret = os_readn(sockfd, pmsg->data, pmsg->size)) != pmsg->size)
            {
                printf("_msg_recv 111111111111111  \n");

                break;
            }
        }

       // printf("pmsg->msg_id:%d \n",pmsg->msg_id);
        if(pmsg->ack != 0)
        {
            printf("msg error!!! \n");
            return 0;
        }

        // 返回查询信息的状态
        if(pmsg->msg_id == 49)
        {
#if 1
            switch(pmsg->args)
            {
            case SDK_UPGRAD_NET_REQUEST:
            {
                sdk_upgrad_pkg_info_t *pkg_info = (sdk_upgrad_pkg_info_t *)(pmsg->data);
                g_upgrad_session = pkg_info->upgrad_session;
                printf("SDK_UPGRAD_NET_REQUEST g_upgrad_session:%d  \n",g_upgrad_session);
                sync_single_post();
                break;
            }
			case SDK_UPGRAD_PROGRESS:           //升级进度
			{
				g_up_percent = pmsg->extend;
			    printf("SDK_UPGRAD_NET_RECV_UPG =====ACK!!!g_up_percent:%d \n",g_up_percent);
                sync_single_post();
				break;
            }
            case SDK_UPGRAD_REQUEST :    		//升级请求	upgrade_packet_t
            case SDK_UPGRAD_DISK_FILE:          //升级包在磁盘文件
            case SDK_UPGRAD_MEM_FILE:           //升级包在内存
                break;
            }
#endif

        }


    }
    printf("  _msg_recv  exit !!!! \n");
    return 0;
}

int create_msg_recv_thread(int sock_fd)
{
    pthread_t      server_pid;

    if(pthread_create(&server_pid, NULL, _msg_recv, (void*)sock_fd) < 0)
    {
        return -1;
    }
    return 0;

}
static int send_msg(int fd, sdk_msg_t *p_msg)
{
    int ret = 0;

    ret = os_writen(fd, (char *)p_msg, sizeof(sdk_msg_t)+p_msg->size);
    if(ret <= 0)
    {
        printf("error: writen fd:%d, ret:%d, write_size:%d\n"
               , fd
               , ret
               , sizeof(sdk_msg_t)+p_msg->size);

        return -1;
    }

    return ret;
}
static int g_pack_num = 0;
static  pthread_mutex_t g_msg_mutex;

int	main(int argc, char **argv)
{
    int opt = 0;
    char* ip = NULL;
    char ip_local[16] = {0};
    char* file_path= NULL;
    if (argc <= 2)
    {
        printf("Usage: %s <-i ip> <-f file_path> \n"
               "   ip     			upgrade device ip.\n"
               "   file_path    	upgrade file path\n"

               "For example:\n"
               "   %s -i 172.16.1.35 -f /home/grain_35S_upgrade.img \n",
               argv[0], argv[0]);
        return -1;
    }
    while ((opt = getopt(argc, argv, "i:f:")) != -1)
    {
        switch (opt)
        {
        case 'i':
            ip = optarg;
            break;
        case 'f':
            file_path = optarg;
            break;
        default:
            break;
        }
    }

    printf(" %s================= %s \n",ip,file_path);
    /************************************/
    char msg_buf[MSG_BUF_SIZE] = {0};
	char data_buf[4096] = {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
    sdk_upgrad_pkg_info_t *pkg_info = (sdk_upgrad_pkg_info_t *)pMsg->data;
    int pack_num = 0;
    int upgrad_session = 0;  //回话ID 的保存处  很重要
    int upgrade_fd = 0;
    int file_fd;
    int  read_len = 0;
    int send_len = 0;
    char file_name[128] = {0};
    strncpy(file_name,file_path,strlen(file_path));

    int file_size = get_file_size(file_name);
    int total_packet =  (file_size + MAX_BLOCK_SIZE)/MAX_BLOCK_SIZE;
    strcpy(ip_local,ip);
    printf("file_name:%s  get file_size:%d total_packet:%d  ip_local:%s \n",file_name,file_size,total_packet,ip_local);

    pthread_mutex_init(&g_msg_mutex, NULL);

    sync_single_init();
    memset(pkg_info,0,sizeof(sdk_upgrad_pkg_info_t));
    upgrade_fd = _connect_noblock(ip_local, 20087, 5);

    create_msg_recv_thread(upgrade_fd);
    printf("1111111111111111111111 \n");
    pMsg->msg_id = 49;
    pMsg->args = SDK_UPGRAD_NET_REQUEST;

    pMsg->size = sizeof(sdk_upgrad_pkg_info_t);
    pkg_info->total_len = file_size;
    pkg_info->total_packet = total_packet;

    send_msg(upgrade_fd, pMsg);
    sync_single_wait();
    printf("222222222222222222222222 total_len :%d total_packet:%d \n",pkg_info->total_len ,pkg_info->total_packet);
    sdk_bin_stream_t  *pbin_stream = (sdk_bin_stream_t *)pMsg->data;
	
    if ((file_fd = open(file_name, O_RDONLY)) < 0)
    {
        printf("open fiel  fail !!! \n");
        return -1;
    }
    g_pack_num = 0;
    printf("33333333333333333333333 \n");
    for(pack_num = 0 ; pack_num < total_packet; pack_num ++)
    {
        pMsg->msg_id = 49;
        pMsg->args = SDK_UPGRAD_NET_RECV_UPG;
        pbin_stream->flag = g_upgrad_session;

        read_len = read(file_fd,(char *)data_buf,MAX_BLOCK_SIZE);
        if(read_len != MAX_BLOCK_SIZE && g_pack_num !=(total_packet-1))
        {
            printf("read file  fail!!! \n");
            return -1;
        }
	    //pthread_mutex_lock(&g_msg_mutex);

        pbin_stream->size = read_len;
        pbin_stream->seq = g_pack_num;
		
		pMsg->size = sizeof(sdk_bin_stream_t) + pbin_stream->size;
		memcpy((char*)pMsg+sizeof(sdk_msg_t)+sizeof(sdk_bin_stream_t),data_buf,pbin_stream->size);
        send_len =  send_msg(upgrade_fd, pMsg);
        //printf("send  data to device msg_len = %d pMsg->size:%d ,pack_num:%d pack_size:%d total_packet:%d \n"
       //        ,send_len,pMsg->size,pbin_stream->seq,pbin_stream->size,total_packet);
        //sync_single_wait();
		 // pthread_mutex_unlock(&g_msg_mutex);

        g_pack_num++;
       //usleep(200*1000);
       ///sleep(1);
    }
    close(file_fd);
    while(1)
    {
    	printf(" wait up_percent!!!!!!!! \n");
        pMsg->msg_id = 49;
        pMsg->args = SDK_UPGRAD_PROGRESS;
        pMsg->size = 0;
        send_msg(upgrade_fd, pMsg);
        //waite single
        sync_single_wait();
		printf("==================g_up_percent:%d \n",g_up_percent);
		if(g_up_percent >= 100)
		{
			printf("upgrade sucess!!!! \n");
			break;
		}
        sleep(1);
    }
	
	close(upgrade_fd);
    sync_single_del();
    return 0;
}


