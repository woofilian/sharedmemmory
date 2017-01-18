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

#include "sdk_struct.h"
#include "main.h"
#include "log.h"

#define MSG_BUF_SIZE 2*1024
static int g_keeplive_run = 0;






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


static int recv_one_msg(int sockfd, sdk_msg_t *pmsg)
{
    int ret;
    if(ret = os_readn(sockfd, (char *)pmsg, sizeof(sdk_msg_t)) != sizeof(sdk_msg_t))
    {
        printf("_msg_recv 0000000000  \n");

        return -1;
    }
    if(pmsg->size)
    {
        pmsg->size = (pmsg->size <= (MSG_BUF_SIZE-sizeof(sdk_msg_t)))
                     ? pmsg->size:(MSG_BUF_SIZE-sizeof(sdk_msg_t));

        if((ret = os_readn(sockfd, pmsg->data, pmsg->size)) != pmsg->size)
        {
            printf("_msg_recv 111111111111111  \n");

            return -1;
        }
    }
    return 0;

}
static int send_one_msg(int fd, sdk_msg_t *p_msg)
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

static void *keep_live_pthread(void *arg)
{
    int opt = 0;
    /************************************/
    char msg_buf[MSG_BUF_SIZE] = {0};
    sdk_msg_t *pMsg = (sdk_msg_t *)msg_buf;
    sdk_keeplive_t tmp_keeplive;
    sdk_keeplive_t *keeplive = (sdk_keeplive_t *)pMsg->data;

    memset(&tmp_keeplive,0,sizeof(sdk_keeplive_t));
    int keep_live_fd = _connect_noblock("127.0.0.1", 20087, 5);
    if(keep_live_fd < 0)
    {
        error("creat keep live socket faile !!! \n");
        //assert(0);
        return -1;
    }

    info("AAAAAAAAAAAAAAAAAAAAAAAAAAAAA \n");
    pMsg->msg_id = SDK_MAIN_MSG_KEEPLIVE;
    pMsg->args = 0;

    strcpy(tmp_keeplive.process_name,"grain_813X");
    tmp_keeplive.process_type = PROC_TYPE_MAIN;
    g_keeplive_run = 1;
    while(g_keeplive_run)
    {

        memcpy(keeplive,&tmp_keeplive,sizeof(sdk_keeplive_t));
        pMsg->size = sizeof(sdk_keeplive_t);
        info("send keep live  msg pMsg->size:%d \n",pMsg->size);
        send_one_msg(keep_live_fd, pMsg);
        usleep(10*1000);
        if(!recv_one_msg(keep_live_fd, pMsg))
        {
            info("=============pMsg->msg_id:%d  SDK_MAIN_MSG_STOP_SERVER:%d \n",pMsg->msg_id,SDK_MAIN_MSG_STOP_SERVER);
            if(pMsg->msg_id == SDK_MAIN_MSG_KEEPLIVE && keeplive->kill_server == 1)
            {
                info("======stop_server->process_name:%s \n",keeplive->process_name);
                stop_main_server();
                sleep(1);
            }
        }
        sleep_time(2, 0);
        continue;

    }

    close(keep_live_fd);
    return 0;
}

int start_keep_live()
{
    pthread_t	keep_live_id;
    int ret;
    ret = pthread_create(&keep_live_id,NULL, keep_live_pthread, NULL);
}

int stop_keep_live()
{
    g_keeplive_run = 0;
    usleep(50*1000);
    return 0;
}
