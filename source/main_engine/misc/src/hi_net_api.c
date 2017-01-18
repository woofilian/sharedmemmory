/******************************************************************************

                  版权所有 (C), 2011-2015, 佳信捷电子

 ******************************************************************************
  文 件 名   : hi_net_api.c
  版 本 号   : v2.0
  作    者   : 梁锦明
  生成日期   : 2011年9月8日
  最近修改   :
  功能描述   : linux socket网络编程常用接口
  函数列表   :
              cal_chksum
              hi_bind_sock
              hi_close_socket
              hi_create_local_tcp_server
              hi_create_local_udp_server
              hi_create_sock
              hi_get_host_ip
              hi_get_sock_ip
              hi_get_sock_port
              hi_get_wan_ip
              hi_ip_a2n
              hi_ip_n2a
              hi_itoa
              hi_local_tcp_connect
              hi_local_tcp_recv
              hi_local_tcp_send
              hi_local_udp_recv
              hi_local_udp_send
              hi_ping
              hi_select
              hi_set_sock_attr
              hi_set_sock_block
              hi_set_sock_boardcast
              hi_set_sock_keepalive
              hi_set_sock_multicast_noloop
              hi_set_sock_multi_membership
              hi_set_sock_noblock
              hi_set_sock_nodelay
              hi_set_sock_rm_multi_membership
              hi_sock_bind_interface
              hi_sock_set_linger
              hi_sock_unset_linger
              hi_tcp_block_accept
              hi_tcp_block_connect
              hi_tcp_block_recv
              hi_tcp_block_send
              hi_tcp_listen
              hi_tcp_noblock_accept
              hi_tcp_noblock_connect
              hi_tcp_noblock_mtu_send
              hi_tcp_noblock_recv
              hi_tcp_noblock_send
              hi_udp_recv
              hi_udp_send
              Listen
              reverse
  修改历史   :
  1.日    期   : 2011年9月8日
    作    者   : 梁锦明
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/ip_icmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <net/if.h>

#include "hi_net_api.h"
#include "log.h"


/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 函数实现                                     *
 *----------------------------------------------*/

//***********************套接字相关************************************//
int hi_close_socket(int *pSock)
{
	if (pSock != NULL && *pSock > 0)
	{
//		shutdown(*pSock, 2); //ljm close 2010-07-15
		close(*pSock);
		*pSock = -1;
		return HI_RETURN_OK;
	}
	return HI_RETURN_FAIL;	
}

int hi_create_sock(int iType)
{
	int fd = socket((iType/3) ? (AF_UNIX):(AF_INET),
					(iType%2) ? (SOCK_STREAM):(SOCK_DGRAM) ,0);
	return fd;
}

int hi_bind_sock(int sockfd, int ip, int nPort)
{
	struct sockaddr_in addr_local;
	memset(&addr_local, 0, sizeof(struct sockaddr_in));

	addr_local.sin_family      = AF_INET;
	addr_local.sin_addr.s_addr = htonl(ip);
	addr_local.sin_port        = htons(nPort);	
	
	hi_set_sock_attr(sockfd, 1, 0, 0, 0, 0);  //set the sock reuser_addr attribute
	if (bind(sockfd,(struct sockaddr*)&addr_local, sizeof(struct sockaddr_in)) < 0)
		return HI_RETURN_FAIL;
	return HI_RETURN_OK;
}

int hi_sock_set_linger(int sockfd)
{
	struct linger nLinger;

	if (sockfd <= 0)
		return HI_RETURN_FAIL;
	
	memset(&nLinger, 0, sizeof(struct linger));
	nLinger.l_onoff = 1;
	nLinger.l_linger= 0;

	if (setsockopt(sockfd,SOL_SOCKET,SO_LINGER,(void *)&nLinger,sizeof(struct linger)) < 0)
    		return HI_RETURN_FAIL;
	return HI_RETURN_OK;
}

int hi_sock_unset_linger(int sockfd)
{
	struct linger nLinger;

	if (sockfd <= 0)
		return HI_RETURN_FAIL;
	
	memset(&nLinger, 0, sizeof(struct linger));
	nLinger.l_onoff = 0;
	nLinger.l_linger= 0;

	if (setsockopt(sockfd,SOL_SOCKET,SO_LINGER,(void *)&nLinger,sizeof(struct linger)) < 0)
    		return HI_RETURN_FAIL;
	return HI_RETURN_OK;
}

int hi_sock_bind_interface(int sockfd, char *interfaceName)
{
	struct ifreq ifReq;
	int ret = 0;
	
    strncpy(ifReq.ifr_name, interfaceName, IFNAMSIZ);

 	ret = setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE,
            (char *)&ifReq, sizeof(ifReq));	
	return ret;
}

int hi_get_sock_ip(int sock)
{
	struct sockaddr_in sockAddr;
	int addrLen = sizeof(struct sockaddr);
	
	if (0 != getsockname(sock, (struct sockaddr *)&sockAddr, (socklen_t *)&addrLen))
		return HI_RETURN_FAIL;
	return sockAddr.sin_addr.s_addr;
}

int hi_get_sock_port(int sock)
{
	struct sockaddr_in sockAddr;
	int addrLen = sizeof(struct sockaddr);
	
	if (0 != getsockname(sock, (struct sockaddr *)&sockAddr, (socklen_t *)&addrLen))
		return HI_RETURN_FAIL;
	return htons(sockAddr.sin_port);
}

int hi_set_sock_attr(int fd, int bReuseAddr, int nSndTimeO, int nRcvTimeO, int nSndBuf, int nRcvBuf)
{
    int err_ret = HI_RETURN_OK;	    
    struct timeval sndTo, rcvTo;

	if (fd <= 0)
		return HI_RETURN_FAIL;
	
    sndTo.tv_sec  = nSndTimeO / 1000;
    sndTo.tv_usec = (nSndTimeO % 1000) * 1000;
	
    rcvTo.tv_sec  = nRcvTimeO / 1000;
    rcvTo.tv_usec = (nRcvTimeO % 1000) * 1000;
	
    if (bReuseAddr != 0 && setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(void *)&bReuseAddr,sizeof(int)) < 0)
        err_ret = HI_RETURN_FAIL;
	
    if (nSndTimeO != 0 && setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,(void  *)&sndTo,sizeof(sndTo)) < 0)
		err_ret = HI_RETURN_FAIL;
    if (nSndTimeO != 0 && setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,(void  *)&rcvTo,sizeof(rcvTo)) < 0)
		err_ret = HI_RETURN_FAIL;
	
    if (nSndBuf != 0 && setsockopt(fd,SOL_SOCKET,SO_SNDBUF,(void  *)&nSndBuf,sizeof(nSndBuf)) < 0)
		err_ret = HI_RETURN_FAIL;
    if (nRcvBuf != 0 && setsockopt(fd,SOL_SOCKET,SO_RCVBUF,(void  *)&nRcvBuf,sizeof(nSndBuf)) < 0)
        err_ret = HI_RETURN_FAIL;
    return err_ret;
}

int hi_set_sock_nodelay(int fd)
{
	int opt = 1;	

	if (fd <= 0)
		return HI_RETURN_FAIL;
	return setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(char *)&opt,sizeof(opt));
}

int hi_set_sock_keepalive(int fd)
{
	int opt = 1;	

	if (fd <= 0)
		return HI_RETURN_FAIL;
	return setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE,(char *)&opt,sizeof(opt));
}

int  hi_set_sock_boardcast(int fd)
{
	int    so_boardcast = 1;
	return setsockopt(fd,SOL_SOCKET,SO_BROADCAST,&so_boardcast,sizeof(so_boardcast));
}

int  hi_set_sock_multicast_noloop(int fd)
{
	int opt=0;
	return setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&opt,  sizeof(char));
}

int  hi_set_sock_multi_membership(int fd, char *multiAddr, int interfaceIp)
{
	int    ret = 0;
	struct ip_mreq	ipmreq;
	
	memset(&ipmreq, 0, sizeof(ipmreq));
	//add multicast membership
	ipmreq.imr_multiaddr.s_addr = inet_addr(multiAddr);
	ipmreq.imr_interface.s_addr = interfaceIp;

	ret = setsockopt(fd, IPPROTO_IP,IP_ADD_MEMBERSHIP,(char *)&ipmreq,
					  sizeof(ipmreq));
	return ret;
}

int hi_set_sock_rm_multi_membership(int fd, char *multiAddr)
{
	int    ret = 0;
	struct ip_mreq	ipmreq;
	
	memset(&ipmreq, 0, sizeof(ipmreq));
	//add multicast membership
	ipmreq.imr_multiaddr.s_addr = inet_addr(multiAddr);
	ipmreq.imr_interface.s_addr = htonl(0);

	ret = setsockopt(fd ,IPPROTO_IP,IP_DROP_MEMBERSHIP,(char *)&ipmreq,
					sizeof(ipmreq));
	return ret;
}

//***********************网络发送相关************************************//
  
int Listen(int fd, int backlog)
{
    char	*ptr = NULL;
	
	if (fd <= 0)
		return HI_RETURN_FAIL;
	
	/*4can override 2nd argument with environment variable */
    if ( (ptr = getenv("LISTENQ")) != NULL)
		backlog = atoi(ptr);
	
    if (listen(fd, backlog) < 0)
	{
		printf("dbg line %d %d\n", fd, backlog);
		return HI_RETURN_FAIL;
    }	
    return HI_RETURN_OK;
}

int hi_tcp_listen(const char *host, const char *serv, int *addrlenp)
{
    int      listenfd, n;
    struct addrinfo hints, *res, *resSave;
	
    bzero(&hints, sizeof (struct addrinfo)) ;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
	
    if ( (n = getaddrinfo (host, serv, &hints, &res)) != 0)
        return HI_RETURN_FAIL;
	
    resSave = res;
    do {
		listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (listenfd < 0)
		{
			sleep(3);	
			continue;            /* error, try next one */
		}
        
		hi_set_sock_attr(listenfd, 1, 0, 0, 0, 0);
		
        if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
            break;               /* success */
		close (listenfd);        /* bind error, close and try next one */
    } while ( (res = res->ai_next) != NULL);
	
    if (res == NULL)            /* errno from final socket () or bind () */
	{
		if (resSave)
		{
			freeaddrinfo (resSave);
			resSave = NULL;
		}
		return HI_RETURN_FAIL;
	}
    Listen (listenfd, LISTENQ);

    if (addrlenp)
		*addrlenp = res->ai_addrlen;     /* return size of protocol address */
	if (resSave)
		freeaddrinfo (resSave);
	
    return (listenfd);
}
   
int hi_set_sock_block(int nSock)
{	
    int bBlock = 0;
	if (nSock <= 0)
		return HI_RETURN_FAIL;
	
    if (ioctl(nSock, FIONBIO, &bBlock) < 0 )
        return HI_RETURN_FAIL;
    return HI_RETURN_OK;
}
   
int hi_set_sock_noblock(int nSock)
{	
    int bNoBlock = 1;
	
	if (nSock <= 0)
		return HI_RETURN_FAIL;
	
    if (ioctl(nSock, FIONBIO, &bNoBlock) < 0 )
        return HI_RETURN_FAIL;
    return HI_RETURN_OK;
}

 
int hi_tcp_block_connect (const char *localHost, const char *localServ, const char *dstHost, const char *dstServ)
{
    int     sockfd, n;
    struct addrinfo hints, *dstRes, *localRes, *resSave;
	
	dstRes = localRes = resSave = NULL;
	
    if (dstHost == NULL || dstServ == NULL)
		return HI_RETURN_FAIL;	
    bzero(&hints, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
	
    if ( (n = getaddrinfo (dstHost, dstServ, &hints, &dstRes)) != 0)
        return HI_RETURN_FAIL;
    resSave = dstRes;
	
    do {
		sockfd = socket (dstRes->ai_family, dstRes->ai_socktype, dstRes->ai_protocol);
		if (sockfd < 0)
		{
			sleep(1);
			continue;            /*ignore this one */
		}   
		
		if (localServ)
		{
			if ( (n = getaddrinfo (localHost, localServ, &hints, &localRes)) != 0)
			{
				if (resSave)
				{
					freeaddrinfo(resSave);
					resSave = NULL;
				}
				return HI_RETURN_FAIL;
			}
			hi_set_sock_attr(sockfd, 1, 0, 0, 0, 0);  //set the sock reuser_addr attribute
			
			if (bind(sockfd, localRes->ai_addr, localRes->ai_addrlen) == 0)
				break;               /* success */
		}
		
		if (connect (sockfd, dstRes->ai_addr, dstRes->ai_addrlen) == 0)
			break;               /* success */
		
		close(sockfd);          /* ignore this one */
    } while ( (dstRes = dstRes->ai_next) != NULL);
    
	if (dstRes == NULL)             /* errno set from final connect() */
		sockfd =  HI_RETURN_FAIL;
	if (resSave)
	{
		freeaddrinfo(resSave);
		resSave = NULL;
	}

	if (localRes)
	{
		freeaddrinfo(localRes);
		localRes = NULL;
	}

    return (sockfd);
}

int hi_tcp_block_recv(int sockfd, char *rcvBuf, int bufSize, int rcvSize)
{
    int nleft = 0;
    int nread = 0;
	int nTryTimes = 0;
    char *ptr = NULL;
	
    ptr  = rcvBuf;
    nleft= rcvSize;

	if  (sockfd <= 0 || rcvBuf == NULL || bufSize <= 0)
		return HI_RETURN_FAIL; 
	
    if (rcvSize <= 0 || bufSize < rcvSize)
        nleft = bufSize;
	
    while(nleft > 0 && nTryTimes < HI_MAX_BLOCK_RECV_TIME) 
    {
        if ((nread = recv(sockfd, ptr, nleft, 0)) < 0)
		{
			if (errno == EINTR)
                nread = 0;
            else
                return HI_RETURN_FAIL;    	
		}
        else if (nread == 0)
        {
			break;
        }
		
		nleft -= nread;
        ptr   += nread;
        ++nTryTimes;
        if (rcvSize <= 0 && nread > 0)
			break;
    }
	
    if (rcvSize <= 0)
		return nread;
    return  rcvSize - nleft;   	     	
}
 
int hi_tcp_block_send(int fd, const void *vptr, int n)
{
    int nleft;
    int nwritten;
    int nTryTimes = 0;
    const char *ptr;

	if  (fd <= 0 || vptr == NULL || n <= 0)
		return HI_RETURN_FAIL; 
	
    ptr = vptr;
    nleft = n;
    while (nleft > 0 && nTryTimes < HI_MAX_BLOCK_SEND_TIME)
    {
        if ( (nwritten = send(fd, ptr, nleft, 0)) <= 0) 
        {
			if (nwritten < 0 && errno == EINTR)
			{
				nwritten = 0;   /* and call write() again */	
				nTryTimes = -1;
			}
			else
				return (HI_RETURN_FAIL);    /* error */
		}
		
		nleft -= nwritten;
		ptr += nwritten;
        ++nTryTimes;
    }
    return (n - nleft);
}

int hi_tcp_block_accept(int fd, struct sockaddr *sa, int *salenptr)
{
	int n = HI_RETURN_FAIL;
//	char szIp[32] = {0};
again:
	if ( (n = accept(fd, sa, (socklen_t *)salenptr)) < 0) 
	{
		if (errno == ECONNABORTED)
			goto again;
	}
	//if (sa != NULL)
	//	HI_SYSLOG(HI_LOG_DEBUG,"RECV REMOTE CONNECT %s\n", 
	//	inet_ntop(AF_INET, &((struct   sockaddr_in*)sa)->sin_addr, szIp, sizeof(szIp)));
	return (n);
}
		
int hi_tcp_noblock_connect(const char *localHost, const char *localServ, const char *dstHost, const char *dstServ, int timeout_ms)
{
    int     sockfd, n;
    struct addrinfo hints, *dstRes, *localRes, *resSave;
    int	    error = 1;
    int	    error_len = sizeof(int);
    fd_set  fset;
    int     ret = 0;
	int     opt = 0;
	
    struct timeval to = {0};
	
	 dstRes = resSave = localRes = NULL;
	
    if (dstHost == NULL || dstServ == NULL)
		return HI_RETURN_FAIL;	
	
    bzero(&hints, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
	
    to.tv_sec = timeout_ms / 1000;
    to.tv_usec= (timeout_ms % 1000)*1000;	
    if ( (n = getaddrinfo (dstHost, dstServ, &hints, &dstRes)) != 0)
        return HI_RETURN_FAIL;
    resSave = dstRes;
	
    do {
		sockfd = socket (dstRes->ai_family, dstRes->ai_socktype, dstRes->ai_protocol);
		if (sockfd < 0)
		{
			sleep(3);     
			continue;            /*ignore this one */
		}   
		
		hi_set_sock_noblock(sockfd);
		opt = 1;
		setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&opt,sizeof(opt));
		
		if (localServ)
		{
			if ( (n = getaddrinfo (localHost, localServ, &hints, &localRes)) != 0)
			{
				if (resSave)
				{
					freeaddrinfo (resSave);
					resSave = NULL;
				}
				close(sockfd); 
				return HI_RETURN_FAIL;
			}
			hi_set_sock_attr(sockfd, 1, 0, 0, 0, 0);  //set the sock reuser_addr attribute
			
			if (bind(sockfd, localRes->ai_addr, localRes->ai_addrlen) == 0)
				break;               /* success */		
		}
		ret = connect (sockfd, dstRes->ai_addr, dstRes->ai_addrlen);
		if (ret == 0)
			break;	
		if (ret < 0 && errno == EINPROGRESS)
		{
			FD_ZERO(&fset);
			FD_SET(sockfd,&fset);
			
			if ((ret = select(sockfd + 1, NULL, &fset, NULL, &to)) > 0)
			{
				if (FD_ISSET(sockfd, &fset) > 0)
				{
					if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char *)&error, (socklen_t *)&error_len) < 0)
						error = 1;
				}
			}
			if (error == 0)
				break;
		}
		
		close(sockfd);          /* ignore this one */
    } while ( (dstRes = dstRes->ai_next) != NULL);
	if (dstRes == NULL)             /* errno set from final connect() */
        sockfd =  HI_RETURN_FAIL;

	if (resSave)
	{
		freeaddrinfo (resSave);
		resSave = NULL;
	}
	if (localRes)
	{
		freeaddrinfo (localRes);
		resSave = NULL;
	}

    return (sockfd);
}

int hi_tcp_noblock_recv(int sockfd, char *rcvBuf, int bufSize, int rcvSize, int timeOut)
{
    int		ret;
    unsigned long	dwRecved = 0;
    unsigned long	nTryTime = 0;
    int             nSize    = rcvSize;
	
	if (sockfd <= 0 || rcvBuf == NULL || bufSize <= 0)
		return -1;
	
    if (rcvSize <= 0 || bufSize < rcvSize)
        nSize = bufSize;	           
	
    while(dwRecved < nSize)
    {
		ret = recv(sockfd, rcvBuf + dwRecved, nSize - dwRecved, 0);
		if(0 == ret)
			return -1;
		else if(ret < 1)
		{
			if(ECONNRESET == errno)
				return -1;
			else if(EWOULDBLOCK == errno  || errno == EINTR || errno == EAGAIN)
			{
				if(nTryTime++ < HI_DEFAULT_RECV_TIMEOUT)
				{				
					usleep(10000);
					continue;
				}
				else
					break;
			}
			
			return -1;
		}
		nTryTime = 0;
		dwRecved += ret;
		if (rcvSize <= 0) //没有指定收数据的长度,则收一次就返回收到的长度
            break;
    }
    return dwRecved;
}

int hi_tcp_noblock_send(int hSock,char *pbuf,int size, int *pBlock, int timeOut)
{
	int  block = 0;	
	int  alllen = size;
	int  sended = 0;
	
	if (hSock < 0 || pbuf == NULL || size <= 0)
		return 0;

	if (pBlock != NULL)
		*pBlock = 0;
	
	while(alllen > 0)
	{	
		sended = send(hSock,pbuf,alllen,0);
		if(0 == sended)
		{
			return HI_RETURN_FAIL;
		}
		else if(sended < 1)
		{
			if(block > HI_DEFAULT_SEND_TIMEOUT)
			{
				return HI_RETURN_FAIL;
			}
			if(errno == EWOULDBLOCK || errno == EINTR || errno == EAGAIN)
			{			
				if(block++ < HI_DEFAULT_SEND_TIMEOUT)
				{
					usleep(1000);
					continue;
				}
				else
				{
					if (pBlock != NULL)	
						*pBlock = 1;
					break;
				}
			}
			return HI_RETURN_FAIL;
		}
		else
		{	
			if (pBlock != NULL)	
				*pBlock = 0;
			pbuf += sended;
			alllen -= sended;
		}
	}
		  
	if(alllen > 0)
		return HI_RETURN_FAIL;
	return size;
}
	
int hi_tcp_noblock_mtu_send(int hSock,char *pbuf,int size, int mtu)
{
	int ret       = size;
	int sendLen   = 0;
	int nBlock    = 0;
	
	if (hSock <= 0 || pbuf == NULL || size <= 0)
		return HI_RETURN_FAIL;

	
	while(size > 0)
	{
		sendLen = size > mtu ? mtu : size;
		if (sendLen != hi_tcp_noblock_send(hSock, pbuf, sendLen, &nBlock, 3000))
			return HI_RETURN_FAIL;
	
		pbuf += sendLen;
		size -= sendLen;
	}

	return ret;
}
	 		 
int hi_tcp_noblock_accept(int fd, struct sockaddr *sa, int *salenptr, struct timeval * to)
{
	int nready = 0;
	int maxfd = fd+1;
	fd_set rset;

	if (fd <= 0 || to == NULL)
		return HI_RETURN_FAIL;
	
	FD_ZERO(&rset);
	FD_SET(fd, &rset);
	
	nready = select(maxfd, &rset, NULL, NULL, to);
	if (nready > 0 && FD_ISSET(fd, &rset))
	{
		return hi_tcp_block_accept(fd, sa, salenptr);
	}
	
	return HI_RETURN_FAIL;
}

int hi_udp_recv(int sockfd, char *rcvBuf, int bufSize, int rcvSize, struct sockaddr *from, int *fromlen)
{
    int		ret;
    unsigned long	dwRecved = 0;
    unsigned long	nTryTime = 0;
    int             nSize    = rcvSize;

    if (rcvSize <= 0 || rcvBuf == NULL || bufSize <= 0)
        nSize = bufSize;	           

    while(dwRecved < nSize)
    {
		ret = recvfrom(sockfd, rcvBuf + dwRecved, nSize - dwRecved, 
						0, from, (socklen_t *)fromlen);
		if(0 == ret)
			return HI_RETURN_FAIL;
		else if(ret < 1)
		{
			if(ECONNRESET == errno)
				return HI_RETURN_FAIL;
			else if(EWOULDBLOCK == errno  || errno == EINTR || errno == EAGAIN)
			{
				if(nTryTime++ < HI_MAX_BLOCK_RECV_TIME)
				{				
					usleep(10000);
					continue;
				}
				else
					break;
			}
			
			return HI_RETURN_FAIL;
		}
		nTryTime = 0;
		dwRecved += ret;

		if (rcvSize <= 0)
            break;
    }
    return dwRecved;
}
	 
int hi_udp_send(int hSock,char *pbuf,int size, struct sockaddr * distAddr)
{
    int  block  = 0;	
    int  alllen = size;
    int  sended = 0;

	if (hSock <= 0 || pbuf == NULL || size <= 0 || distAddr == NULL)
	{
//		HI_SYSLOG(HI_LOG_ERR,"HI_Udp_Send %d %d\n", hSock, size);
		return 0;
	}
	
    while(alllen > 0)
    {	
		sended = sendto(hSock,pbuf,alllen,0, distAddr, sizeof(struct sockaddr));
		if(0 == sended)
		{
            return HI_RETURN_FAIL;
		}
		else if(sended < 1)
		{
			if(block > HI_MAX_BLOCK_SEND_TIME)
			{
				return HI_RETURN_FAIL;
			}
			if(errno == EWOULDBLOCK || errno == EINTR || errno == EAGAIN)
			{			
				if(block++ < HI_MAX_BLOCK_SEND_TIME)
				{
					usleep(1000);
                    continue;
				}
				else
				{
					break;
				}
			}
			return HI_RETURN_FAIL;
		}
		else
		{		
			pbuf += sended;
			alllen -= sended;
		}
    }
    
    if(alllen > 0)
		return HI_RETURN_FAIL;
    return size;
}

unsigned short cal_chksum(unsigned short *addr, int len)
{
	int leave = len;
	int sum   = 0;
	unsigned short *w=addr;
	unsigned short answer=0;
    
	while(leave > 1)
	{
		sum += *w++;
		leave -= 2;
	}
     
	if( leave == 1)
	{       
		*(unsigned char *)(&answer) = *(unsigned char *)w;
		sum += answer;
	}
    
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = ~sum;
    
	return answer;
}

int hi_ping(char *ips, int timeout, int max_times)
{
    struct timeval timeo;
    int sockfd;
    struct sockaddr_in addr;
    struct sockaddr_in from;
    
    struct timeval *tval;
    struct ip 		 *iph;
    struct icmp 	 *icmp;

    char sendpacket[128];
    char recvpacket[128];
    char from_ip[32];
    int n;
    int ping_times = 0;
    int ret = 0;
    pid_t pid;
    int maxfds = 0;
    fd_set readfds;
    
    if (ips == NULL || strcmp(ips, "") == 0)
	return 0;
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
	inet_pton(AF_INET, ips, &addr.sin_addr);  

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0)
    {
		return HI_RETURN_FAIL;
    }
    
    timeo.tv_sec = timeout / 1000000;
    timeo.tv_usec = (timeout % 1000);
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo)) == -1)
    {
    	close(sockfd);
		sleep(5);
        return HI_RETURN_FAIL;
    }
    
    memset(sendpacket, 0, sizeof(sendpacket));
    
    pid=getpid();
    int packsize = 0;

    icmp=(struct icmp*)sendpacket;
    icmp->icmp_type=ICMP_ECHO;
    icmp->icmp_code=0;
    icmp->icmp_cksum=0;
    icmp->icmp_seq=0;
    icmp->icmp_id=pid;
    packsize=8+56;
    tval= (struct timeval *)icmp->icmp_data;
    gettimeofday(tval,NULL);
    icmp->icmp_cksum=cal_chksum((unsigned short *)icmp,packsize);

    n = sendto(sockfd, (char *)&sendpacket, packsize, 0, (struct sockaddr *)&addr, sizeof(addr));
    if (n < 1)
    {
		close(sockfd);
		sleep(5);
        return HI_RETURN_FAIL;
    }
	
    while(ping_times++ < max_times)
    {	
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        maxfds = sockfd + 1;
        n = select(maxfds, &readfds, NULL, NULL, &timeo);
        if (n <= 0)
        {
            ret = HI_RETURN_FAIL;
            continue;
        }
    
        memset(recvpacket, 0, sizeof(recvpacket));
        int fromlen = sizeof(from);
        n = recvfrom(sockfd, recvpacket, sizeof(recvpacket), 0, 
					(struct sockaddr *)&from, (socklen_t *)&fromlen);
        if (n < 1) 
        {
			ret = HI_RETURN_FAIL;
            continue;
        }
        
		inet_ntop(AF_INET, &from.sin_addr, from_ip, sizeof(from_ip));
        if (strcmp(from_ip,ips) != 0)
        {
			ret = HI_RETURN_FAIL;
            continue;
        }
        
        iph = (struct ip *)recvpacket;
    
        icmp=(struct icmp *)(recvpacket + (iph->ip_hl<<2));

        if (icmp->icmp_type == ICMP_ECHOREPLY && icmp->icmp_id == pid)
        {
            ret = HI_RETURN_OK;
            break;
        }  
        else
        {
            ret = HI_RETURN_FAIL;
            continue;
        }
    }
 
    close(sockfd);
    return ret;
}
 
int hi_get_host_ip(int af, char *host)
{
    unsigned long dwIp = 0;
    int           ret  = 0;
    struct addrinfo hints, *res;
    
    if(host == NULL)
		return HI_RETURN_FAIL;
    if(strcmp(host,"255.255.255.255") == 0 || strcmp(host, "") == 0)
        return HI_RETURN_FAIL;
	
    bzero (&hints, sizeof (struct addrinfo));
    hints.ai_flags  = AI_CANONNAME;   /* always return canonical name */
    hints.ai_family = AF_UNSPEC;   /* AF_UNSPEC, AF_INET, AF_INET6, etc. */
    hints.ai_socktype = 0;  /* 0, SOCK_STREAM, SOCK_DGRAM, etc. */
    if ( (ret = getaddrinfo(host, NULL, &hints, &res)) != 0)
        return HI_RETURN_FAIL;
    dwIp =  ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;	
	
    freeaddrinfo(res);

    return dwIp;
}
	
int hi_select(int *fd_array, int fd_num, int fd_type, int time_out)
{
	int maxfd = 0;
	int index = 0;
	int ret   = 0;
	
	fd_set read_fd, *p_read_fd;
	fd_set write_fd, *p_write_fd;
	fd_set except_fd, *p_except_fd;
	struct timeval timeO, *p_time_out;

	if (fd_array == NULL || fd_num < 0)
		return HI_RETURN_FAIL;
	p_time_out = NULL;
	if (time_out > 0)
	{
    	timeO.tv_sec = time_out / 1000;
    	timeO.tv_usec= (time_out % 1000)*1000;	
		p_time_out = &timeO;
	}

	p_read_fd = p_write_fd = p_except_fd = NULL;
	
	if (fd_type & 0x1)
	{
		p_read_fd = &read_fd;
		FD_ZERO(p_read_fd);
	}
	
	if (fd_type & 0x2)
	{
		p_write_fd = &write_fd;
		FD_ZERO(p_write_fd);
	}
	
	if (fd_type & 0x4)
	{
		p_except_fd = &except_fd;
		FD_ZERO(p_except_fd);
	}
	
	for (index = 0; index < fd_num; ++index)
	{
		if(fd_array[index] <= 0)
			continue;
		maxfd = maxfd > fd_array[index] ? maxfd : fd_array[index];

		if (p_read_fd)
			FD_SET(fd_array[index], p_read_fd);
		if (p_write_fd)
			FD_SET(fd_array[index], p_write_fd);
		if (p_except_fd)
			FD_SET(fd_array[index], p_except_fd);
	}
	if (maxfd <= 0)
		return HI_RETURN_FAIL;
	
	maxfd += 1;

	while (1)
	{
		ret = select(maxfd, p_read_fd, p_write_fd, p_except_fd, p_time_out);
		if (ret < 0 && errno == EINTR)
		{
		    continue;
		}    
		else if (ret < 0)
			return HI_RETURN_FAIL;
		else if (ret == 0)
			return HI_RETURN_OK;
		else
		{
			for (index = 0; index < fd_num; ++index)
			{
				if (fd_array[index] <= 0)  //socket error
					continue;
				
				if (p_read_fd)
				{
					if (FD_ISSET(fd_array[index], p_read_fd))
						return fd_array[index] | 0x10000;
				}
				else if (p_write_fd)
				{
					if (FD_ISSET(fd_array[index], p_write_fd))
						return fd_array[index] | 0x20000;
				}
				else if (p_except_fd)
				{
					if (FD_ISSET(fd_array[index], p_except_fd))
						return fd_array[index] | 0x40000;;
				}			
			}
			return HI_RETURN_OK;
		}
	}	
	return HI_RETURN_FAIL;	
}

unsigned long hi_ip_n2a(unsigned long ip, char *ourIp, int len)
{
	if(!ourIp)
		return HI_RETURN_FAIL;
	
	memset(ourIp, 0, len);
	ip = htonl(ip);
	inet_ntop(AF_INET, &ip, ourIp, len);	
	return HI_RETURN_OK;
}

unsigned long hi_ip_a2n(char *pstrIP)
{
	unsigned long ret;
    struct hostent *hostinfo;
    struct sockaddr_in address;

	if (!pstrIP)
		return HI_RETURN_FAIL;
	
	inet_pton(AF_INET, pstrIP, &ret);
		
    if(0xFFFFFFFF == ret)
    {
		if(strcmp(pstrIP,"255.255.255.255") == 0)
		{
		    return 0xFFFFFFFF;
		}
	
		if(!(hostinfo = gethostbyname(pstrIP)))
		{
			return 0;
		}

		address.sin_addr = *((struct in_addr *)(*(hostinfo->h_addr_list)));
		ret = address.sin_addr.s_addr;
    }
	
	return htonl(ret);
}

void  reverse(char   s[])   
{   
	int   c,   i,   j;   

	for   (i   =   0,   j   =   strlen(s)-1;   i   <   j;   i++,   j--)
	{   
		c   =   s[i];   
		s[i]   =   s[j];   
		s[j]   =   c;   
	}   
}   

void   hi_itoa(int   n,   char   s[])   
{   
    int   i,   sign;   

    if ((sign = n) < 0)     /*   record   sign   */   
    	n = -n;                     /*   make   n   positive   */   
    i = 0;   
    do{             /*   generate   digits   in   reverse   order   */   
    	s[i++] = n % 10 + '0';     /*   get   next   digit   */   
    }while ((n /= 10) > 0);         /*   delete   it   */   

	if (sign < 0)   
       s[i++] = '-';   
    s[i] = '\0';   
    reverse(s);   
}

int hi_create_local_tcp_server(char *svrPath)
{
	int server_fd = 0;
	struct sockaddr_un server_address;      /*声明一个UNIX域套接字结构*/
	int server_len= 0;

	if ((server_fd = hi_create_sock(3)) < 0)
		return HI_RETURN_FAIL;
	
	memset(&server_address, 0, sizeof(struct sockaddr_un));

	server_address.sun_family = AF_UNIX;
	server_len = sizeof(server_address);

	if (sizeof(server_address.sun_path) < strlen(svrPath))
		strncpy(server_address.sun_path, svrPath, sizeof(server_address.sun_path)-1);
	else
		strncpy(server_address.sun_path, svrPath, strlen(svrPath));//strlen(svrPath) - 1);

	unlink (server_address.sun_path);       /*删除原有server_socket对象*/	

	if (bind(server_fd,(struct sockaddr*)&server_address, server_len) < 0)
	{
		hi_close_socket(&server_fd);
		return HI_RETURN_FAIL;
	}
		
	if (Listen(server_fd, LISTENQ) == HI_RETURN_FAIL)
	{
		hi_close_socket(&server_fd);
		return HI_RETURN_FAIL;
	}
	
	return server_fd;
}

int hi_local_tcp_connect(char *svrPath)
{
	int client_fd = 0;
	struct sockaddr_un server_address;      /*声明一个UNIX域套接字结构*/
	int server_len= 0;

	if ((client_fd = hi_create_sock(3)) < 0)
		return HI_RETURN_FAIL;

	server_address.sun_family = AF_UNIX;
	server_len = sizeof(server_address);

	if (sizeof(server_address.sun_path) < strlen(svrPath))
		strncpy(server_address.sun_path, svrPath, sizeof(server_address.sun_path)-1);
	else
		strncpy (server_address.sun_path, svrPath, strlen(svrPath));

	if (connect(client_fd, (struct sockaddr *)&server_address, server_len) < 0)
	{
		hi_close_socket(&client_fd);
		return HI_RETURN_FAIL;
	}		

	return client_fd;
}

int hi_local_tcp_send(int fd, char *data, int len)
{
	int ret    = 0;
	int sndCnt = 0;
	int sndSum = 0;
	
	if (fd <= 0 || data == NULL || len == 0)
		return HI_RETURN_FAIL;

	while (len > 0 && sndCnt++ < HI_DEFAULT_LOCAL_TIME)
	{
		if ((ret = write(fd, data, len)) < 0)
			return HI_RETURN_FAIL;
		len    -= ret;
		sndSum += ret;
	}
	return sndSum;
}

int hi_local_tcp_recv(int fd, char *data, int readLen)
{
	int readCnt = 0;
	
	if (fd <= 0 || data == NULL || readLen == 0)
		return HI_RETURN_FAIL;

	if ((readCnt = read(fd, data, readLen)) < 0)
		return HI_RETURN_FAIL;

	return readCnt;
}

int hi_create_local_udp_server(char *svrPath)
{
	int server_fd = 0;
	struct sockaddr_un server_address;      /*声明一个UNIX域套接字结构*/
	int server_len= 0;

	if ((server_fd = hi_create_sock(4)) < 0)
		return HI_RETURN_FAIL;
	
	memset(&server_address, 0, sizeof(struct sockaddr_un));

	server_address.sun_family = AF_UNIX;
	server_len = sizeof(server_address);

	if (sizeof(server_address.sun_path) < strlen(svrPath))
		strncpy(server_address.sun_path, svrPath, sizeof(server_address.sun_path)-1);
	else
		strncpy (server_address.sun_path, svrPath, strlen(svrPath));

	unlink (server_address.sun_path);/*删除原有server_socket对象*/	

	if (bind(server_fd,(struct sockaddr*)&server_address, server_len) < 0)
	{
		hi_close_socket(&server_fd);
		return HI_RETURN_FAIL;
	}
	
	return server_fd;
}

int hi_local_udp_send(int fd, char *svrPath, char *data, int nLen)
{
	struct sockaddr_un  server_address;

	memset(&server_address, 0, sizeof(server_address));
	
	server_address.sun_family = AF_UNIX;
	if (sizeof(server_address.sun_path) < strlen(svrPath))
		strncpy(server_address.sun_path, svrPath, sizeof(server_address.sun_path)-1);
	else
		strncpy (server_address.sun_path, svrPath, strlen(svrPath));

	if (nLen != sendto(fd, data, nLen, 0,(struct sockaddr *)&server_address, 
										 sizeof(struct sockaddr)))
	{
		return HI_RETURN_FAIL;
	}
	
	return nLen;
}

int hi_local_udp_recv(int fd, char *pData, int nLen)
{
	return recvfrom(fd, pData, nLen, 0, NULL, NULL);
}

/*********实现获取设备的公网ip功能**************************/

#define GET_WAN_IP_PACK	 "GET %s HTTP/1.1\r\n"  \
					     "Accept: */*\r\n"	\
					     "Referer: http://%s/\r\n" \
					     "Accept-Language: zh-cn\r\n"	\
					     "Accept-Encoding: gzip, deflate\r\n" \
					     "User-Agent: Mozilla/4.0 (compatible; Win32; WinHttp.WinHttpRequest.5)\r\n"	\
					     "Host:%s\r\n"	\
					     "Connection: Keep-Alive\r\n\r\n"

char webUrl[3][32]     = {"www.ip138.com",
					      "www.ip.cn",
					      "www.ip-adress.com"};

char ipUrl[3][32]      = {"221.5.47.132", 
						  "219.232.228.186",
						  "87.118.124.104"};

char httpAction[3][32] = {"/ip2city.asp", 						
						  "/getip.php?action=getip&ip_url=",
						  "/"};

char startCode[3][32]  = {"您的IP地址是：[",
						  "'>",
						  "My IP address: "};

char endCode[3][32]    = {"] </",
						  "</",
						  "</"};

int hi_get_wan_ip(char *pIp)
{	
	int  socket   = -1;
	int  nUrl     = 0;
	int  sendLen  = 0;
	char *pStart  = NULL;
	char *pEnd    = NULL;
	char szGetBuf[1024 * 10] = {0};
	
	if(NULL == pIp)
		return -1;

    printf("get wan ip!\n");

	for( ;nUrl < 3; nUrl++)
	{
		if((socket = hi_tcp_noblock_connect(NULL, NULL, webUrl[nUrl], "80", 15000)) < 0)
		{
			if ((socket = hi_tcp_noblock_connect(NULL, NULL, ipUrl[nUrl], "80", 15000)) < 0)
				continue;
		}
        printf("connect %s %s ok!\n", webUrl[nUrl], ipUrl[nUrl]);
		
		memset(szGetBuf, 0, sizeof(szGetBuf));
		
		sprintf(szGetBuf, GET_WAN_IP_PACK, httpAction[nUrl], webUrl[nUrl], webUrl[nUrl]);
		sendLen = strlen(szGetBuf);

		if(sendLen != hi_tcp_noblock_send(socket, szGetBuf, sendLen, NULL, 1000))
		{
            printf("send error!\n");
			close(socket);
			continue;
		}

		if (hi_select(&socket, 0x1, 0x1, 15000) <= 0)
		{
            printf("select error!\n");
			close(socket);
			continue;
		}
		
		memset(szGetBuf, 0, sizeof(szGetBuf));
		if((sendLen = hi_tcp_noblock_recv(socket, szGetBuf, sizeof(szGetBuf), 0, 10000)) <= 0)
		{
            printf("recv error!\n");
			close(socket);
			continue;
		}
        else{
            info("%s\n", szGetBuf);
        }
		
		pStart = strstr(szGetBuf, startCode[nUrl]);
		if (pStart != NULL)
			pEnd   = strstr(pStart, endCode[nUrl]);
		if(pStart && pEnd)
		{
			pStart += strlen(startCode[nUrl]);
			if (pIp)
			{
				memcpy(pIp, pStart, pEnd - pStart);
				pIp[pEnd - pStart] = '\0';
			}	

			close(socket);
			return HI_RETURN_OK;
		}
		
		close(socket);
	}
	
	return HI_RETURN_FAIL;
}

