/*
 * =====================================================================================
 *
 *       Filename:  wrapper.c
 *
 *    Description:  wrapper
 *
 *        Version:  1.0
 *        Created:  2011å¹´08æœˆ27æ—¥ 17æ—¶43åˆ†18ç§’
 *       Revision:  none
 *       Compiler:  gcc
 * 
 *
 * =====================================================================================
 */

#include <stdarg.h>
//#include "log.h"
#include "wrapper.h"

time_t os_make_time(int year,int month,int day
        ,int hour,int minute,int second)
{
    time_t ret;
    struct tm t;
	memset(&t, 0, sizeof(t));
	
    t.tm_year = year-1900;
    t.tm_mon = month-1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = minute;
    t.tm_sec = second;
	
    ret = mktime(&t);
    if(ret < 0) 
    {
    	printf("mktime error! errno:%s\n", strerror(errno));
        return 0;
    }
    return ret;
}

void os_local_time(time_t utc
        ,unsigned int *pyear,unsigned int *pmonth,unsigned int *pday
        ,unsigned int *phour,unsigned int *pminute,unsigned int *psecond
        ,unsigned int *pweekday)
{
    struct timeval tv;
    struct tm tm,*ret;
    if(utc<0)
    {
        utc=0;
    }
    tv.tv_sec=utc;
    tv.tv_usec=0;
    ret=localtime_r(&tv.tv_sec,&tm);
    if(ret == NULL)
    {
        return;
    }
    if(pyear   != NULL) *pyear    = tm.tm_year+1900;
    if(pmonth  != NULL) *pmonth   = tm.tm_mon+1;
    if(pday    != NULL) *pday     = tm.tm_mday;
    if(phour   != NULL) *phour    = tm.tm_hour;
    if(pminute != NULL) *pminute  = tm.tm_min;
    if(psecond != NULL) *psecond  = tm.tm_sec;
    if(pweekday!= NULL) *pweekday = tm.tm_wday;
}

char *os_local_time_str(time_t time, char *time_str)
{   
    if(time_str == NULL)return NULL;

    unsigned int year,mon,day,hour,min,sec,weekday;
	os_local_time(time, &year, &mon, &day, &hour, &min, &sec, &weekday);
    sprintf(time_str, "%04d-%02d-%02d %02d:%02d:%02d weekday:%d"
            , year
            , mon
            , day
            , hour
            , min
            , sec
            , weekday);

    return time_str;
}

int os_set_time(time_t t)
{
    struct timeval tv;

    tv.tv_sec = t;
    tv.tv_usec = 0;

	printf("-----------3---------settimeofday time:%d\n",0);

    if (settimeofday(&tv, NULL) < 0)
    {
        return(-1);
    }

    return(0);
}

int os_get_time(unsigned int *pyear, unsigned int *pmonth, unsigned int *pday
        		,unsigned int *phour, unsigned int *pminute, unsigned int *psecond)
{
	time_t seconds;
	struct tm *p = NULL;
	
	seconds = time((time_t *)NULL);
	p = localtime(&seconds);
	if (NULL == p)
	{
		printf("localtime error!\n");
		return -1;
	}

	if(pyear   != NULL) *pyear   = 1900 + p->tm_year;
	if(pmonth  != NULL) *pmonth  = 1 + p->tm_mon;
	if(pday    != NULL) *pday    = p->tm_mday;
	if(phour   != NULL) *phour   = p->tm_hour;
	if(pminute != NULL) *pminute = p->tm_min;
	if(psecond != NULL) *psecond = p->tm_sec;

	return 0;
}

int os_get_days(int year, int month)
{
    switch (month)
    {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            return(31);	
        case 4:
        case 6:
        case 9:
        case 11:
            return(30);
        case 2:
            if (year%4 == 0)
                return(29);
            else
                return(28);
        default:
            return(-1);
    }
}


ssize_t os_writen(int fd, const void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) 
    {
        if ( (nwritten = write(fd, ptr, nleft)) <= 0) 
        {
            if (errno == EINTR)
                nwritten = 0; /* and call write() again */
            else
                return(-1); /* error */
        }
        if (errno == 5)
            return(-1);
        nleft -= nwritten;
        ptr += nwritten;
    }
    return(n);
}

ssize_t os_readn(int fd, void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nread;
    char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) 
    {
        if ( (nread = read(fd, ptr, nleft)) < 0) 
        {
            if (errno == EINTR)
                nread = 0; /* and call read() again */
            else
                return(-1);
        } 
        else if (nread == 0)
        {
            break; /* EOF */
        }
        nleft -= nread;
        ptr += nread;
    }

    return(n - nleft); /* return >= 0 */
}

ssize_t os_readline(int fd, void *vptr, size_t maxlen)
{
    ssize_t n, rc;
    char c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++)
    {
again:
        if ((rc = read(fd, &c, 1)) == 1)
        {
            *ptr++ = c;
            if (c == '\n')
                break; /* newline is stored, like fgets() */
        }
        else if (rc == 0)
        {
            *ptr = 0;
            return(n - 1); /* EOF, n-1 bytes were read */
        }
        else
        {
            if (errno == EINTR)
                goto again;
            return(-1);
        }
    }
    *ptr = 0; /* null terminate like fgets() */
    return(n);
}

char *os_strsep(char **stringp, const char *delim)
{
    char *p;

    while (1)
    {
        p = strsep(stringp, delim);
        if (p == NULL)
        {
            return NULL;
        }
        else
        {
            if (p[0] == '\0')
                continue;
            else
                break;
        }
    }
    return p;
}



int os_get_rand(int min, int max)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    srand(tv.tv_usec);
    return(min+(int) ((double)(max-min+1)*rand()/(RAND_MAX*1.0)));
}

//static pthread_mutex_t alloc_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_mutex_t alloc_mutex = PTHREAD_MUTEX_INITIALIZER;
static unsigned int alloc_count    = 0;

void *os_calloc(size_t nmemb, size_t size)
{
    void *p;
    pthread_mutex_lock(&alloc_mutex);
    p = calloc(nmemb, size);
    if(p != NULL){alloc_count++;};
    pthread_mutex_unlock(&alloc_mutex);
    return(p);
}

void *os_malloc(size_t size)
{
    void *p;
    pthread_mutex_lock(&alloc_mutex);
    p = malloc(size);
    if(p != NULL){alloc_count++;};
    pthread_mutex_unlock(&alloc_mutex);
    return(p);
}

void os_free(void *ptr)
{
    pthread_mutex_lock(&alloc_mutex);
    if(ptr != NULL)
    {
        free(ptr);
        alloc_count--;
    }
    pthread_mutex_unlock(&alloc_mutex);
}

void *os_realloc(void *ptr, size_t size)
{
    void *p;
    pthread_mutex_lock(&alloc_mutex);
    p = realloc(ptr, size);	
    if(p != NULL){alloc_count++;};
    pthread_mutex_unlock(&alloc_mutex);
    return(p);
}


#define NETWORK_FILE_EXIST 0

#define COMM_ADDRSIZE   20
#define COMM_FILELINE   256
#define COMM_ETHFILE    "/etc/sysconfig/network-scripts/ifcfg-eth1"
#define COMM_GATEWAY    "/etc/sysconfig/network"
#define COMM_RESOLVCONF "/etc/resolv.conf"
#define PROC_ROUTE      "/proc/net/route"

#define ETHTOOL_GLINK   0x0000000a /* Get link status (ethtool_value) */
#define SIOCETHTOOL     0x8946

typedef unsigned long long __u64;
typedef __uint32_t __u32;
typedef __uint16_t __u16;
typedef __uint8_t __u8;

struct ethtool_cmd {
    __u32 cmd;
    __u32 supported; /* Features this interface supports */
    __u32 advertising; /* Features this interface advertises */
    __u16 speed; /* The forced speed, 10Mb, 100Mb, gigabit */
    __u8 duplex; /* Duplex, half or full */
    __u8 port; /* Which connector port */
    __u8 phy_address;
    __u8 transceiver; /* Which transceiver to use */
    __u8 autoneg; /* Enable or disable autonegotiation */
    __u32 maxtxpkt; /* Tx pkts before generating tx int */
    __u32 maxrxpkt; /* Rx pkts before generating rx int */
    __u32 reserved[4];
};

/* for passing single values */
struct ethtool_value {
    __u32 cmd;
    __u32 data;
};

static int _os_mac2int(const char *macaddr, int *p);

int os_get_if_index(const char *if_name, int *ifindex)
{
    int fd;
    /*char buffer[COMM_ADDRSIZE];*/
    struct ifreq ifr;

    if((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) {
        ifr.ifr_addr.sa_family = AF_INET;
        ifr.ifr_name[IFNAMSIZ - 1] = '\0';
        strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
        if (ioctl(fd, SIOCGIFINDEX, &ifr) == 0) {
            *ifindex = ifr.ifr_ifindex;
        }
        else
        {
            close(fd);
            return -2;
        }
    }
    else
    {
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int os_get_mac_addr(const char *if_name, char *macaddr, size_t len)
{
    int fd;
    char buffer[COMM_ADDRSIZE];
    struct ifreq ifr;

    if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0)
    {
        strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
        ifr.ifr_name[IFNAMSIZ - 1] = '\0';
        if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0)
        {
            snprintf(buffer, COMM_ADDRSIZE, "%02x:%02x:%02x:%02x:%02x:%02x",
                    (unsigned char)ifr.ifr_hwaddr.sa_data[0],
                    (unsigned char)ifr.ifr_hwaddr.sa_data[1],
                    (unsigned char)ifr.ifr_hwaddr.sa_data[2],
                    (unsigned char)ifr.ifr_hwaddr.sa_data[3],
                    (unsigned char)ifr.ifr_hwaddr.sa_data[4],
                    (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
        }
        else
        {
            close(fd);
            return(-1);
        }
    }
    else
    {
        return(-1);
    }

    if (strlen(buffer) > len-1)
    {
        return(-1);
    }
    strncpy(macaddr, buffer, len);
    close(fd);
    return(0);
}

int os_get_ip_addr(const char *if_name, char *ipaddr, size_t len)
{
    int fd;
    char buffer[COMM_ADDRSIZE];
    struct ifreq ifr;
    struct sockaddr_in *addr;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
    {
        strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
        ifr.ifr_name[IFNAMSIZ - 1] = '\0';
        if (ioctl(fd, SIOCGIFADDR, &ifr) == 0)
        {
            addr = (struct sockaddr_in *)&(ifr.ifr_addr);
            inet_ntop(AF_INET, &addr->sin_addr, buffer, 20);
        }
        else
        {
            close(fd);
            return(-1);
        }
    }
    else
    {
        perror("os_getIpAddr error :");
        return(-1);
    }

    if (strlen(buffer) > len-1)
    {
        return(-1);
    }
    strncpy(ipaddr, buffer, len);
    close(fd);
    return(0);
}

int os_get_net_mask(const char *if_name, char *netmask, size_t len)
{
    int fd;
    char buffer[COMM_ADDRSIZE];
    struct ifreq ifr;
    struct sockaddr_in *addr;

    if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0)
    {
        strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
        ifr.ifr_name[IFNAMSIZ - 1] = '\0';
        if (ioctl(fd, SIOCGIFNETMASK, &ifr) == 0)
        {
            addr = (struct sockaddr_in *)&(ifr.ifr_addr);
            inet_ntop(AF_INET, &addr->sin_addr, buffer, 20);
        }
        else
        {
            close(fd);
            return(-1);
        }
    }
    else
    {
        return(-1);
    }

    if (strlen(buffer) > len-1)
    {
        return(-1);
    }
    strncpy(netmask, buffer, len);

    close(fd);
    return(0);
}

int os_get_net_addr(const char *if_name, char *netaddr, size_t len)
{
    int m1, m2, m3, m4, i1, i2, i3, i4;
    char ipbuf[COMM_ADDRSIZE], maskbuf[COMM_ADDRSIZE];

    if (os_get_ip_addr(if_name, ipbuf, COMM_ADDRSIZE) < 0)
        return(-1);
    if (os_get_net_mask(if_name, maskbuf, COMM_ADDRSIZE) < 0)
        return(-1);

    sscanf(maskbuf, "%d.%d.%d.%d", &m1, &m2, &m3, &m4);
    sscanf(ipbuf, "%d.%d.%d.%d", &i1, &i2, &i3, &i4);
    snprintf(netaddr, len, "%d.%d.%d.%d", m1&i1, m2&i2, m3&i3, m4&i4);

    return(0);
}

int os_get_broad_cast(const char *if_name, char *broadcast, size_t len)
{
    int fd;
    char buffer[COMM_ADDRSIZE];
    struct ifreq ifr;
    struct sockaddr_in *addr;

    if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0)
    {
        strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
        ifr.ifr_name[IFNAMSIZ - 1] = '\0';
        if (ioctl(fd, SIOCGIFBRDADDR, &ifr) == 0)
        {
            addr = (struct sockaddr_in *)&(ifr.ifr_addr);
            inet_ntop(AF_INET, &addr->sin_addr, buffer, 20);
        }
        else
        {
            close(fd);
            return(-1);
        }
    }
    else
    {
        return(-1);
    }

    if (strlen(buffer) > len-1)
    {
        return(-1);
    }
    strncpy(broadcast, buffer, len);

    close(fd);
    return(0);
}

#define CH_TO_HEX(ch) ((ch)<='9'?(ch)-48:(ch)-'A'+10)

int os_get_gateway(const char *if_name, char *gateway, size_t len)
{
#if NETWORK_FILE_EXIST
    int flag = 0; /*isn't find the GATEWAY?*/
    FILE *fp;
    char buf[COMM_FILELINE], *ptr;

    if((fp = fopen(COMM_GATEWAY, "r")) == NULL)
    {
        return(-1);
    }
    while (fgets(buf, COMM_FILELINE, fp) != NULL)
    {
        if (strncmp(buf, "GATEWAY", 7) == 0)
        {
            sscanf(buf, "GATEWAY=%s", gateway);
            if (gateway[strlen(gateway)-1] == '\n')	
                gateway[strlen(gateway)-1] = '\0';
            flag = 1;
            break;
        }
    }

    fclose(fp);
    if (flag == 0)
    {
        return(-1);
    }
#else
    int i;
    int n[4];
    int fd;
    char name[10], dest[10], dg[10], buf[1024];

    memset(gateway, '\0', len);
    if ((fd = open(PROC_ROUTE, O_RDONLY)) < 0)
    {
        perror("open route");
        return(-1);
    }
    while (os_readline(fd, buf, 1024) > 0)
    {
        if (memcmp(buf, if_name, 4) != 0)
            continue;
        sscanf(buf, "%4s\t%8s\t%8s\t", name, dest, dg);
        if (!strcmp(dest, "00000000"))
        {
            for (i = 0; i < 8; i = i+2)
            {
                n[i/2] = CH_TO_HEX(dg[i])*16+CH_TO_HEX(dg[i+1]);
            }
#ifdef COMM_PPC
            snprintf(gateway, len, "%d.%d.%d.%d", n[0], n[1], n[2], n[3]);
#else
            snprintf(gateway, len, "%d.%d.%d.%d", n[3], n[2], n[1], n[0]);
#endif
            break;
        }
    }
    close(fd);
#endif
    if (gateway[0] == '\0')
        return(-1);

    return(0);
}

int os_get_dns(char *firstdns, char *seconddns, size_t len)
{
    int flag = 0; /*isn't find the DNS?*/
    int fd;
    char buf[COMM_FILELINE], *ptr;

    if(firstdns == NULL)
        return -1;
    if((fd = open(COMM_RESOLVCONF, O_RDONLY)) < 0)
    {
        return(-1);
    }
    while (os_readline(fd, buf, COMM_FILELINE) > 0)
    {//printf("buf=%s\n",buf);
        if (strncmp(buf, "nameserver", 10) == 0)
        {
            ptr = buf;
            os_strsep(&ptr, " ");
            if (flag == 0)
            {
                strncpy(firstdns, os_strsep(&ptr, " "), len);
                if (firstdns[strlen(firstdns)-1] == '\n')
                    firstdns[strlen(firstdns)-1] = '\0';
					//printf("firstdns=%s\n",firstdns);
                flag = 1;
                if (seconddns == NULL)
                    break;
                continue;
            }
            else
            {
                if(seconddns == NULL)
                    return -1;
                strncpy(seconddns, os_strsep(&ptr, " "), len);
                if (seconddns[strlen(seconddns)-1] == '\n')
                    seconddns[strlen(seconddns)-1] = '\0';
				//printf("seconddns=%s\n",seconddns);
                flag = 2;
                break;
            }
        }
    }

    close(fd);
    if (flag == 0)
    {
        return(-1);
    }

    return(flag);
}

int os_set_mac_addr(const char *if_name, const char *macaddr)
{
    int i = 0;
    int fd;
    int imac[6];
    struct ifreq ifr;
    short temp;
#if NETWORK_FILE_EXIST
    int flag = 0; /*COMM_ETHFILE hasn't a line of MACADDR?*/
    FILE *fp, *fp_tmp;
    char tmpfile[COMM_FILELINE], buf[COMM_FILELINE];
#endif
    if (os_check_mac_addr(macaddr) < 0)
    {
        printf("check mac error!\n");
        return(-1);
    }
    if ((fd =socket(AF_INET,SOCK_DGRAM,0))< 0)
    {
        return(-1);
    }
    strncpy(ifr.ifr_name,if_name,IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0)
    {
        printf("ioctl get ifr failed!\n");
        return(-1);
    }
    temp = ifr.ifr_flags;
    /*shutdown interface.*/
    ifr.ifr_flags = temp & (~IFF_UP);
    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0)
    {
        printf("ioctl ifr failed!\n");
        return(-1);
    }
    /*change mac address.*/
    ifr.ifr_hwaddr.sa_family = 1;
    _os_mac2int(macaddr,imac);
    for (i = 0; i < 6; i++)
        ifr.ifr_hwaddr.sa_data[i] = (unsigned char)imac[i];
    if (ioctl(fd,SIOCSIFHWADDR,&ifr) < 0)
    {
        return(-1);
    }
    /*up interface.*/
    ifr.ifr_flags = temp|IFF_UP;
    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0)
    {
        return(-1);
    }
    close(fd);
#if NETWORK_FILE_EXIST
    /*change interface file, and it will take affect when reboot.*/
    if((fp = fopen(COMM_ETHFILE, "r")) == NULL)
    {
        return(-1);
    }
    snprintf(tmpfile, COMM_FILELINE, "%s_%d", COMM_ETHFILE, getpid());
    if((fp_tmp = fopen(tmpfile, "w")) == NULL)
    {
        return(-1);
    }
    while (fgets(buf, COMM_FILELINE, fp) != NULL)
    {
        if (strncmp(buf, "MACADDR", 6) == 0)
        {
            snprintf(buf, COMM_FILELINE, "%s=%s\n", "MACADDR", macaddr);
            flag = 1;
        }
        fputs(buf, fp_tmp);
    }
    if (flag == 0)
    {
        snprintf(buf, COMM_FILELINE, "%s=%s\n", "MACADDR", macaddr);
        fputs(buf, fp_tmp);
    }
    fclose(fp);
    fclose(fp_tmp);
    if (rename(tmpfile, COMM_ETHFILE) < 0)
    {
        return(-1);
    }
#endif
    return(0);
}

int os_set_mac_addr_2(const char *if_name, const char *pmac)
{  
	int ret = 0;
    char shellbuf[128]= {0}; 
	
    sprintf(shellbuf, "ifconfig %s hw ether %s", if_name, pmac);
    ret = system(shellbuf);
	printf("  ret:%d, shellbuf:%s\n", ret, shellbuf);
	
	return 0;
}

int os_set_ip_addr(const char* if_name, const char *ipaddr)
{
    int fd;
    struct ifreq ifr;
    struct sockaddr_in addr;

#if NETWORK_FILE_EXIST
    FILE *fp, *fp_tmp;
    char tmpfile[COMM_FILELINE], buf[COMM_FILELINE];
#endif
    if (os_check_ip_addr(ipaddr) < 0)
    {
    	printf("os_check_ip_addr error!\n");
        return(-1);
    }
    /*set ip address, and take affect instant.*/
    if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("socket error!\n");
        return(-1);
    }
    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    bzero(&addr, sizeof(struct sockaddr_in));
    if (inet_pton(AF_INET, ipaddr, &addr.sin_addr) < 0)
    {
        printf("inet_pton error!\n");
        return(-1);
    }
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    memcpy(&ifr.ifr_addr, &addr, sizeof(struct sockaddr));
    if (ioctl(fd, SIOCSIFADDR, &ifr) < 0)
    {
        printf("ioctl error!\n");
        return(-1);
    }
    close(fd);
	
#if NETWORK_FILE_EXIST
    /*change interface file, and it will take affect when reboot.*/
    if((fp = fopen(COMM_ETHFILE, "r")) == NULL)
    {
    	printf("fopen error!\n");
        return(-1);
    }

    snprintf(tmpfile, COMM_FILELINE, "%s_%d", COMM_ETHFILE, getpid());
    if((fp_tmp = fopen(tmpfile, "w")) == NULL)
    {
    	printf("fopen error!\n");
        return(-1);
    }

    while (fgets(buf, COMM_FILELINE, fp) != NULL)
    {
        if (strncmp(buf, "IPADDR", 6) == 0)
            snprintf(buf, COMM_FILELINE, "%s=%s\n", "IPADDR", ipaddr);
        fputs(buf, fp_tmp);
    }
    fclose(fp);
    fclose(fp_tmp);
    if (rename(tmpfile, COMM_ETHFILE) < 0)
    {
    	printf("rename error!\n");
        return(-1);
    }
#endif

    return(0);
}

int os_set_net_mask(const char *if_name, const char *netmask)
{
    int fd;
    struct ifreq ifr;
    struct sockaddr_in addr;

#if NETWORK_FILE_EXIST
    FILE *fp, *fp_tmp;
    char tmpfile[COMM_FILELINE], buf[COMM_FILELINE];
#endif

    if (os_check_ip_addr(netmask) < 0)
    {
    	printf("os_check_ip_addr error!");
        return(-1);
    }

    /*set netmask, and take affect instant.*/
    if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("socket error!");
        return(-1);
    }
    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    bzero(&addr, sizeof(struct sockaddr_in));
    if (inet_pton(AF_INET, netmask, &addr.sin_addr) < 0)
    {
        printf("inet_pton error!");
        return(-1);
    }
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    memcpy(&ifr.ifr_addr, &addr, sizeof(struct sockaddr));
    if (ioctl(fd, SIOCSIFNETMASK, &ifr) < 0)
    {
        printf("ioctl error!");
        return(-1);
    }
    close(fd);

#if NETWORK_FILE_EXIST
    /*change interface file, and it will take affect when reboot.*/
    if((fp = fopen(COMM_ETHFILE, "r")) == NULL)
    {
        printf("fopen error!");
        return(-1);
    }

    snprintf(tmpfile, COMM_FILELINE, "%s_%d", COMM_ETHFILE, getpid());
    if((fp_tmp = fopen(tmpfile, "w")) == NULL)
    {
        printf("fopen error!");
        return(-1);
    }

    while (fgets(buf, COMM_FILELINE, fp) != NULL)
    {
        if (strncmp(buf, "NETMASK", 6) == 0)
            snprintf(buf, COMM_FILELINE, "%s=%s\n", "NETMASK", netmask);
        fputs(buf, fp_tmp);
    }

    fclose(fp);
    fclose(fp_tmp);
    if (rename(tmpfile, COMM_ETHFILE) < 0)
    {
        printf("rename error!");
        return(-1);
    }
#endif

    return(0);
}

int os_set_net_addr(const char *netaddr)
{
#if NETWORK_FILE_EXIST
    int flag = 0; /*COMM_ETHFILE hasn't a line of NETADDR?*/
    FILE *fp, *fp_tmp;
    char tmpfile[COMM_FILELINE], buf[COMM_FILELINE];

    if((fp = fopen(COMM_ETHFILE, "r")) == NULL)
    {
        return(-1);
    }

    snprintf(tmpfile, COMM_FILELINE, "%s_%d", COMM_ETHFILE, getpid());
    if((fp_tmp = fopen(tmpfile, "w")) == NULL)
    {
        return(-1);
    }

    while (fgets(buf, COMM_FILELINE, fp) != NULL)
    {
        if (strncmp(buf, "NETADDR", 6) == 0)
        {
            snprintf(buf, COMM_FILELINE, "%s=%s\n", "NETADDR", netaddr);
            flag = 1;
        }
        fputs(buf, fp_tmp);
    }

    if (flag == 0)
    {
        snprintf(buf, COMM_FILELINE, "%s=%s\n", "NETADDR", netaddr);
        fputs(buf, fp_tmp);
    }

    fclose(fp);
    fclose(fp_tmp);
    if (rename(tmpfile, COMM_ETHFILE) < 0)
    {
        return(-1);
    }
#endif

    return(0);
}

int os_set_broad_cast(const char *if_name, const char *broadcast)
{
    int fd;
    struct ifreq ifr;
    struct sockaddr_in addr;

#if NETWORK_FILE_EXIST
    int flag = 0; /*COMM_ETHFILE hasn't a line of BROADCAST?*/
    FILE *fp, *fp_tmp;
    char tmpfile[COMM_FILELINE], buf[COMM_FILELINE];
#endif

    if (os_check_ip_addr(broadcast) < 0)
    {
        return(-1);
    }

    /*set broadcast, and take affect instant.*/
    if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        return(-1);
    }
    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    bzero(&addr, sizeof(struct sockaddr_in));
    if (inet_pton(AF_INET, broadcast, &addr.sin_addr) < 0)
    {
        return(-1);
    }
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    memcpy(&ifr.ifr_addr, &addr, sizeof(struct sockaddr));
    if (ioctl(fd, SIOCSIFBRDADDR, &ifr) < 0)
    {
        return(-1);
    }
    close(fd);

#if NETWORK_FILE_EXIST
    /*change interface file, and it will take affect when reboot.*/
    if((fp = fopen(COMM_ETHFILE, "r")) == NULL)
    {
        return(-1);
    }

    snprintf(tmpfile, COMM_FILELINE, "%s_%d", COMM_ETHFILE, getpid());
    if((fp_tmp = fopen(tmpfile, "w")) == NULL)
    {
        return(-1);
    }

    while (fgets(buf, COMM_FILELINE, fp) != NULL)
    {
        if (strncmp(buf, "BROADCAST", 6) == 0)
        {
            snprintf(buf, COMM_FILELINE, "%s=%s\n", "BROADCAST", broadcast);
            flag = 1;
        }
        fputs(buf, fp_tmp);
    }

    if (flag == 0)
    {
        snprintf(buf, COMM_FILELINE, "%s=%s\n", "BROADCAST", broadcast);
        fputs(buf, fp_tmp);
    }

    fclose(fp);
    fclose(fp_tmp);
    if (rename(tmpfile, COMM_ETHFILE) < 0)
    {
        return(-1);
    }
#endif

    return(0);
}

int os_del_gateway(const char *gateway)
{
    char cmdbuf[COMM_FILELINE]="";
    int ret;

    snprintf(cmdbuf, COMM_FILELINE, "route del default gw %s", gateway);
    ret = system(cmdbuf);

    return(0);
}

int os_set_gateway(const char *if_name, const char *gateway)
{
    int ret;
#if NETWORK_FILE_EXIST
    FILE *fp, *fp_tmp;
    char tmpfile[COMM_FILELINE], buf[COMM_FILELINE];
#endif
    char cmdbuf[COMM_FILELINE]="";
    if(gateway == NULL)
        return -1;
#if NETWORK_FILE_EXIST
    if((fp = fopen(COMM_GATEWAY, "r")) == NULL)
    {
        return(-1);
    }
    snprintf(tmpfile, COMM_FILELINE, "%s_%d", COMM_GATEWAY, getpid());
    if((fp_tmp = fopen(tmpfile, "w")) == NULL)
    {
        return(-1);
    }
    while (fgets(buf, COMM_FILELINE, fp) != NULL)
    {
        if (strncmp(buf, "GATEWAY", 7) == 0)
            snprintf(buf, COMM_FILELINE, "%s=%s\n", "GATEWAY", gateway);
        fputs(buf, fp_tmp);
    }
    fclose(fp);
    fclose(fp_tmp);
    if (rename(tmpfile, COMM_GATEWAY) < 0)
    {
        return(-1);
    }
#endif
    snprintf(cmdbuf, COMM_FILELINE, "route add default gw %s dev %s", gateway, if_name);
	ret = system(cmdbuf);	
    return(0);
}

int os_set_dns(const char *firstdns, const char *seconddns)
{
#if 1 /*NETWORK_FILE_EXIST*/
    FILE *fp;
    if(firstdns == NULL)
        return -1;
    unlink(COMM_RESOLVCONF);
    if((fp = fopen(COMM_RESOLVCONF, "w")) == NULL)
    {
        return(-1);
    }
    fprintf(fp, "nameserver %s\n", firstdns);
    if (seconddns != NULL)
        fprintf(fp, "nameserver %s\n", seconddns);
    fclose(fp);
#endif
    return(0);
}

int os_set_auto_dns(void)
{
    unlink(COMM_RESOLVCONF);
    return 0;
}

int os_get_link_status(const char *if_name)
{
    struct ethtool_value edata;
    struct ifreq ifr;
    int fd;
    int err;

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, if_name);

    /* Open control socket. */
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("Cannot get control socket");
        return 70;
    }
    edata.cmd = ETHTOOL_GLINK;
    ifr.ifr_data = (caddr_t)&edata;
    err = ioctl(fd, SIOCETHTOOL, &ifr);
    if (err == 0) {
        close(fd);
        return (edata.data);
    } 
    else if (errno != EOPNOTSUPP) {
        perror("Cannot get link status");
    }
    close(fd);
    return 0;
}

/*some tools function, not in the commlib.*/
int os_check_ip_addr(const char *ipaddr)
{
    int i = 0, flag = 0;
    //char buf[16];
    char buf[16] = {0};

    char *tmp, *ptr;

    if (strlen(ipaddr) > 15)
    {
    	printf("ipaddr > 15\n");
        return(-1);
    }
    //strncpy(buf, ipaddr, 16);
    strncpy(buf, ipaddr, strlen(ipaddr));

    while (buf[i] != '\0')
    {
        switch (buf[i])
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                break;
            case '.':
                flag++;
                break;
            default:
                return(-1);
        }
        i++;
    }
    if (flag != 3)
    {
    	printf("flag error!\n");
        return(-1);
    }

    ptr = buf;
    for (i=0; i<4; i++)
    {
        tmp = os_strsep(&ptr, ".");
        if (tmp == NULL)
        {
        	printf(" tmp == NULL\n");
            return(-1);
        }
        if (atoi(tmp)>255 || strlen(tmp)>3 || (tmp[0]=='0' && strlen(tmp)!=1))
        {
        	printf("tmp error!\n");
            return(-1);
        }
    }

    return(0);
}

int os_check_mac_addr(const char *macaddr)
{
    int i = 0;
    int flag = 0;

    if(macaddr == NULL)
        return -1;
    if (strlen(macaddr) != 17)
        return(-1);
    while (macaddr[i] != '\0')
    {
        switch (macaddr[i])
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
                break;
            case ':':
                flag++;
                break;
            default:
                return(-1);
        }
        i++;
    }
    if (flag != 5)
        return(-1);
    if (macaddr[2] != ':' || macaddr[5] != ':' || macaddr[8] != ':' 
            || macaddr[11] != ':' || macaddr[14] != ':')
        return(-1);
    return(0);
}

static int _os_mac2int(const char *macaddr, int *p)
{
    int i = 0, j = 0;
    int n1 = 0, n2 = 0;

    for (i = 0, j = 0; i < strlen(macaddr); i = i+3, j++)
    {
        if (macaddr[i] >= 48 && macaddr[i] <= 57) /*'0' <--> '9'*/
            n1 = macaddr[i] - 48; /*'0' == 48*/
        else if (macaddr[i] >= 65 && macaddr[i] <= 70) /*'A' <--> 'F'*/
            n1 = macaddr[i] - 55; /*'A' == 65, and in hex 'A' = 10.*/
        else if (macaddr[i] >= 97) /*'a' <--> 'f'*/
            n1 = macaddr[i] - 87; /*'a' == 97, and in hex 'a' = 10.*/

        if (macaddr[i+1] >= 48 && macaddr[i+1] <= 57)
            n2 = macaddr[i+1] - 48;
        else if (macaddr[i+1] >= 65 && macaddr[i+1] <= 70)
            n2 = macaddr[i+1] - 55;
        else if (macaddr[i+1] >= 97)
            n2 = macaddr[i+1] - 87;

        p[j] = n1*16 + n2;
    }

    return(0);
}


/* ------------ com port ----------------- */
static pthread_mutex_t com_serial_mutex = PTHREAD_MUTEX_INITIALIZER;
static int speed_arr[] = {B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1800, B1200, B600};
static int name_arr[]  = {115200, 57600, 38400, 19200, 9600, 4800, 2400, 1800, 1200, 600};
static int setSpeed(int fd, int speed);
static int setParity(int fd, int databits, int parity, int stopbits);

int os_set_com_param(int fd, int baut, int parity, int stop, int databit)
{
    int ret = 0;	
    static int _baut; 
    static int _parity;
    static int _stop;
    static int _databit;
    pthread_mutex_lock(&com_serial_mutex);
    if((_baut != baut) 
        || (_parity != parity) 
        || (_stop   != stop) 
        || (_databit!= databit))
    {
    ret = setSpeed(fd, baut);
    ret = setParity(fd, databit, parity, stop);
        _baut   = baut;
        _parity = parity;
        _stop   = stop;
        _databit= databit;
    };
    pthread_mutex_unlock(&com_serial_mutex);
    
	return ret;
}

static int setSpeed(int fd, int speed)
{
	int i;
	int status;

	struct termios Opt;
	if(tcgetattr(fd, &Opt)!=0)
	{
		perror("SetupSerial error");
		return -1;
	}
	for(i=0; i<sizeof(speed_arr)/sizeof(int); i++)
	{
		if (speed==name_arr[i])
		{
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, speed_arr[i]);
			cfsetospeed(&Opt, speed_arr[i]);
			status=tcsetattr(fd, TCSANOW,&Opt);
			if(0!=status)
			{
				perror("tcsetattr fd error!");
			}
			tcflush(fd, TCIOFLUSH);
		}
	}
	return 0;
}

static int setParity(int fd, int databits, int parity, int stopbits)
{
	struct termios options;

	if (tcgetattr(fd, &options) != 0)
	{
		perror("Setup Serial error!");
		return -1;
	}
	options.c_cflag &= ~CSIZE;
	switch (databits)
	{
	case 5:
		options.c_cflag |= CS5;
		break;
	case 6:
		options.c_cflag |= CS6;
		break;
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr, "Unsupported data size!\n");
		return -1;
	}

	switch (parity)
	{
	case 0: //æ— æ ¡éªŒ
		options.c_cflag &= ~PARENB;
		options.c_iflag &= ~INPCK;
		break;
	case 1: //å¥‡æ ¡éªŒ
		options.c_cflag |= PARENB;
		options.c_cflag |= PARODD;
		options.c_iflag |= INPCK;
		break;
	case 2: //å¶æ ¡éªŒ
		options.c_cflag |= PARENB;
		options.c_cflag &= ~PARODD;
		options.c_iflag |= INPCK;
		break;
	default:
		fprintf(stderr, "Unsupported parity!\n");
		return -1;
	}
	switch (stopbits)
	{
	case 1:
		options.c_cflag &= ~CSTOPB;
		break;
	case 2:
		options.c_cflag |= CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported stop bits!\n");
		return -1;
	}
	options.c_lflag &= ~(ECHO | ICANON | ECHOE | ISIG | IEXTEN);
	options.c_iflag &= ~(IXON | IXOFF | ICRNL);
	options.c_oflag &= ~OPOST;
	tcflush(fd, TCIOFLUSH);
	/* options.c_cc[VTIME]=100;*/  /*SETUP 10s timeout for read if time passed,quit read;*/
	/* options.c_cc[VMIN]=0;*/    /*SETUP min len for read if len passed,quit read;;*/
	if (tcsetattr(fd, TCSANOW, &options) != 0)
	{
		perror("Setup Serial in set_parity error!");
		return -1;
	}

	return 1;
}



/* ------------ file ----------------- */


#define BUCKET_SIZE 8192

static int _os_get_dir_size(const char *pathname, long long *dirsize);
static int _get_dir_file_count(const char *pathname, int *filecount);
static int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len);
static int _del_file_count = 0;

/*Filesystem*/
int os_is_file_exist(const char *pathname)
{	
    int fd;
    struct stat buf;

    if ((fd = open(pathname, O_RDONLY)) < 0)
    {
        return(-1);
    }
    if (fstat(fd, &buf) < 0)
    {
        close(fd);
        return(-1);
    }
    close(fd);

    if (S_ISREG(buf.st_mode))
        return(1);

    return(0); /*not a file, maybe dir/link/fifo etc.*/
}


int os_get_file_length(const char *pathname)
{
    int fd;
    struct stat buf;

    if ((fd = open(pathname, O_RDONLY)) < 0)
    {
        return(-1);
    }
    if (fstat(fd, &buf) < 0)
    {
        close(fd);
        return(-1);
    }
    close(fd);

    if (S_ISREG(buf.st_mode))
        return(buf.st_size);

    return(-1);
}

int os_get_file_Name(const char *pathname, char *file, size_t len)
{
    int fd;
    struct stat buf;

    if ((fd = open(pathname, O_RDONLY)) < 0)
    {
        return(-1);
    }
    if (fstat(fd, &buf) < 0)
    {
        close(fd);
        return(-1);
    }
    close(fd);

    if (S_ISREG(buf.st_mode))
    {
        int i, newpos = -1;

        for (i = 0; *(pathname+i) != '\0'; i++)
        {
            if (*(pathname+i) == '/')
                newpos = i;
        }
        if (newpos == -1)
        {
            if (strlen(pathname) > len-1)
            {
                return(-1);
            }
            strncpy(file, pathname, len);
        }
        else
        {
            if (strlen(pathname+newpos+1) > len-1)
            {
                return(-1);
            }
            strncpy(file, pathname+newpos+1, len);
        }

        return(0);
    }

    return(-1);
}

int os_get_file_path(const char *pathname, char *path, size_t len)
{
    int fd;
    struct stat buf;

    if ((fd = open(pathname, O_RDONLY)) < 0)
    {
        return(-1);
    }
    if (fstat(fd, &buf) < 0)
    {
        close(fd);
        return(-1);
    }
    close(fd);

    if (S_ISREG(buf.st_mode))
    {
        int i;
        int pathlen;
        char tmppath[PATH_MAX];
        const char *ptr = pathname;

        pathlen = strlen(pathname);
        i = pathlen;
        while (pathname[i-1] != '/')
            i--;
        if (pathname[0] == '/')
        {
            strncpy(tmppath, pathname, i-1);
        }
        else if (pathname[0] == '.' && pathname[1] != '.')
        {
            getcwd(tmppath, PATH_MAX);
            strncat(tmppath, ptr+1, i-2);
        }
        else if (pathname[0] == '.' && pathname[1] == '.')
        {
            char oldpath[PATH_MAX];

            getcwd(oldpath, PATH_MAX);
            chdir("..");
            getcwd(tmppath, PATH_MAX);
            chdir(oldpath);
            strncat(tmppath, ptr+2, i-2);
        }
        else
        {
            getcwd(tmppath, PATH_MAX);
            strcat(tmppath, "/");
            strncat(tmppath, pathname, i-1);
        }
        if (strlen(tmppath) > len-1)
        {
            return(-1);
        }
        strncpy(path, tmppath, len);
        return(0);
    }

    return(-1);
}

time_t os_get_file_Atime(const char *pathname)
{
    int fd;
    struct stat buf;

    if ((fd = open(pathname, O_RDONLY)) < 0)
    {
        return(-1);
    }
    if (fstat(fd, &buf) < 0)
    {
        close(fd);
        return(-1);
    }
    close(fd);

    if (S_ISREG(buf.st_mode))
        return(buf.st_atime);

    return(-1);
}

time_t os_get_file_Mtime(const char *pathname)
{
    int fd;
    struct stat buf;

    if ((fd = open(pathname, O_RDONLY)) < 0)
    {
        return(-1);
    }
    if (fstat(fd, &buf) < 0)
    {
        close(fd);
        return(-1);
    }
    close(fd);

    if (S_ISREG(buf.st_mode))
        return(buf.st_mtime);

    return(-1);
}

int os_is_file_read(const char *pathname)
{
    int fd;
    struct stat buf;

    if ((fd = open(pathname, O_RDONLY)) < 0)
    {
        return(-1);
    }
    if (fstat(fd, &buf) < 0)
    {
        close(fd);
        return(-1);
    }
    close(fd);

    if (S_ISREG(buf.st_mode))
    {
        if ((S_IRUSR & buf.st_mode) == S_IRUSR)
            return(1);
        else
            return(0);
    }

    return(-1);
}

int os_is_file_write(const char *pathname)
{
    int fd;
    struct stat buf;

    if ((fd = open(pathname, O_RDONLY)) < 0)
    {
        return(-1);
    }
    if (fstat(fd, &buf) < 0)
    {
        close(fd);
        return(-1);
    }
    close(fd);

    if (S_ISREG(buf.st_mode))
    {
        if ((S_IWUSR & buf.st_mode) == S_IWUSR)
            return(1);
        else
            return(0);
    }

    return(-1);
}

int os_is_file_exec(const char *pathname)
{
    int fd;
    struct stat buf;

    if ((fd = open(pathname, O_RDONLY)) < 0)
    {
        return(-1);
    }
    if (fstat(fd, &buf) < 0)
    {
        close(fd);
        return(-1);
    }
    close(fd);

    if (S_ISREG(buf.st_mode))
    {
        if ((S_IXUSR & buf.st_mode) == S_IXUSR)
            return(1);
        else
            return(0);
    }

    return(-1);
}

int os_is_link_file(const char *pathname)
{
    struct stat buf;

    if (lstat(pathname, &buf) < 0)
    {
        return(-1);
    }

    if (S_ISLNK(buf.st_mode))
        return(1);
    else
        return(0);
}

int os_is_device_file(const char *pathname)
{
    struct stat buf;

    if (lstat(pathname, &buf) < 0)
    {
        return(-1);
    }

    if (S_ISCHR(buf.st_mode) || S_ISBLK(buf.st_mode))
        return(1);
    else
        return(0);
}

int os_is_regular_file(const char *pathname)
{
    struct stat buf;

    if (lstat(pathname, &buf) < 0)
    {
        return(-1);
    }
    if (S_ISREG(buf.st_mode))
        return(1);
    else
        return(0);
}

/*flag:0, cover; 1, not cover.*/
int os_copy_file(const char *oldpath, const char *newpath, int flag)
{
    int fdin, fdout;
    int copylen = 0, leftlen = 0;
    char buf[BUCKET_SIZE];
    struct stat oldbuf, newbuf;	

    if (flag == 1)
    {
        int oldlen, newlen;

        if (os_is_file_exist(newpath) == 1) /*file is exist.*/
        {
            oldlen = os_get_file_length(oldpath);
            newlen = os_get_file_length(newpath);
            if (oldlen < 0 || newlen < 0)
                return(-1);
            if (oldlen == newlen) /*length is same*/
                return(0); /*return.*/
        }
    } /*if size not same or file not exist, copy will go on.*/

    if ((fdin = open(oldpath, O_RDONLY)) < 0) 
    {
        return(-1);
    }
    if (fstat(fdin, &oldbuf) < 0) 
    {
        close(fdin);
        return(-1);
    }
    if ((fdout = open(newpath, O_RDONLY)) > 0) 
    {
        if (fstat(fdout, &newbuf) == 0)
        {
            if (oldbuf.st_ino == newbuf.st_ino) /*oldbuf and newbuf is the same file.*/
            {
                close(fdin);
                close(fdout);
                return(0);
            }
        }
        close(fdout);
    }
    if ((fdout = open(newpath, O_RDWR | O_CREAT | O_TRUNC, oldbuf.st_mode)) < 0) 
    {
        close(fdin);
        return(-1);
    }

    leftlen = oldbuf.st_size;
    while (1)
    {
        if (leftlen > BUCKET_SIZE)
            copylen = BUCKET_SIZE;
        else
            copylen = leftlen;
        if (os_readn(fdin, buf, copylen) < 0)
        {
            close(fdout);
            close(fdin);
            return(-1);
        }
        if (os_writen(fdout, buf, copylen) < 0)
        {
            close(fdout);
            close(fdin);
            return(-1);
        }
        leftlen = leftlen - copylen;
        if (leftlen <= 0)
            break;
    }

    close(fdin);
    close(fdout);

    return(0);		
}

int os_rm_file(const char *pathname)
{
    if (unlink(pathname) < 0)
    {
        return(-1);
    }

    return(0);
}

int os_rename_file(const char *oldpath, const char *newpath)
{
    if (rename(oldpath, newpath) < 0)
    {
        return(-1);
    }

    return(0);
}

int os_set_file_length(const char *pathname, int newlen)
{
    if (truncate(pathname, newlen) < 0)
    {
        return(-1);
    }

    return(0);
}

int os_set_file_mode(const char *pathname, mode_t mode)
{
    if (chmod(pathname, mode) < 0)
    {
        return(-1);
    }

    return(0);
}

int os_read_lock(int fd, off_t offset, int whence, off_t len)
{
    if (lock_reg(fd, F_SETLK, F_RDLCK, offset, whence, len) < 0)
    {
        return(-1);
    }

    return(0);
}

int os_readW_lock(int fd, off_t offset, int whence, off_t len)
{
    if (lock_reg(fd, F_SETLKW, F_RDLCK, offset, whence, len) < 0)
    {
        return(-1);
    }

    return(0);
}

int os_write_lock(int fd, off_t offset, int whence, off_t len)
{
    if (lock_reg(fd, F_SETLK, F_WRLCK, offset, whence, len) < 0)
    {
        return(-1);
    }

    return(0);
}

int os_writeW_lock(int fd, off_t offset, int whence, off_t len)
{
    if (lock_reg(fd, F_SETLKW, F_WRLCK, offset, whence, len) < 0)
    {
        return(-1);
    }

    return(0);
}

int os_unLock(int fd, off_t offset, int whence, off_t len)
{
    if (lock_reg(fd, F_SETLKW, F_UNLCK, offset, whence, len) < 0)
    {
        return(-1);
    }

    return(0);
}

off_t os_seek(int fd, off_t offset, int whence)
{
    if (lseek(fd, offset, whence) < 0)
    {
        return(-1);
    }

    return(0);
}

off_t os_seek2begin(int fd)
{
    if (lseek(fd, 0, SEEK_SET) < 0)
    {
        return(-1);
    }

    return(0);
}

off_t os_seek2end(int fd)
{
    if (lseek(fd, 0, SEEK_END) < 0)
    {
        return(-1);
    }

    return(0);
}

int os_set_Utime(const char *pathname, const struct utimbuf *times)
{
    if (utime(pathname, times) < 0)
    {
        return(-1);
    }

    return(0);
}

int os_fsync(int fd)
{
    if (fsync(fd) < 0)
    {
        return(-1);
    }

    return(0);
}

int os_fflush(FILE *stream)
{
    if (fflush(stream) < 0)
    {
        return(-1);
    }

    return(0);
}

int os_is_dir_exist(const char *pathname)
{
    int fd;
    struct stat buf;

    if ((fd = open(pathname, O_RDONLY)) < 0)
    {
        return(-1);
    }
    if (fstat(fd, &buf) < 0)
    {
        close(fd);
        return(-1);
    }
    close(fd);

    if (S_ISDIR(buf.st_mode))
        return(1);

    return(0); /*not a dir, maybe file/fifo etc.*/
}

int os_is_dir(const char *pathname)
{
    int fd;
    struct stat buf;

    if ((fd = open(pathname, O_RDONLY)) < 0)
    {
        return(-1);
    }
    if (fstat(fd, &buf) < 0)
    {
        close(fd);
        return(-1);
    }
    close(fd);

    if (S_ISDIR(buf.st_mode))
        return(1);
    else
        return(0);
}
#if 1
int os_get_dir_name(const char *pathname, char *dirname, size_t len)
{
    return(0);
}
#endif
int os_get_absolute_dir(const char *pathname, char *dirname, size_t len)
{
    int fd;
    struct stat buf;

    if ((fd = open(pathname, O_RDONLY)) < 0)
    {
        return(-1);
    }
    if (fstat(fd, &buf) < 0)
    {
        close(fd);
        return(-1);
    }
    close(fd);

    if (S_ISDIR(buf.st_mode))
    {
        int pathlen;
        char tmppath[PATH_MAX];
        const char *ptr = pathname;

        pathlen = strlen(pathname);
        if (pathname[0] == '/')
        {
            strncpy(tmppath, pathname, PATH_MAX);
        }
        else if (pathname[0] == '.' && pathname[1] != '.')
        {
            getcwd(tmppath, PATH_MAX);
            strncat(tmppath, ptr+1, PATH_MAX);
        }
        else if (pathname[0] == '.' && pathname[1] == '.')
        {
            char oldpath[PATH_MAX];

            getcwd(oldpath, PATH_MAX);
            chdir("..");
            getcwd(tmppath, PATH_MAX);
            chdir(oldpath);
            strncat(tmppath, ptr+2, PATH_MAX);
        }
        else
        {
            getcwd(tmppath, PATH_MAX);
            strcat(tmppath, "/");
            strncat(tmppath, pathname, PATH_MAX);
        }
        if (strlen(tmppath) > len-1)
        {
            return(-1);
        }
        strncpy(dirname, tmppath, len);
        return(0);
    }

    return(-1);
}

int os_is_dir_empty(const char *pathname)
{
    DIR *dp;
    struct dirent *dirp;
    int flag = 1;

    if ((dp = opendir(pathname)) == NULL)
    {
        return(-1);
    }
    while ((dirp = readdir(dp)) != NULL)
    {
        if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
            continue;
        else
        {
            flag = 0; /*dir is not empty.*/
            break;
        }
    }
    if (closedir(dp) < 0)
    {
        return(-1);
    }

    return(flag);
}

long long os_get_dir_size(const char *pathname)
{
    long long dirsize = 0;

    if (_os_get_dir_size(pathname, &dirsize) < 0)
        return(-1); /*comm_errno has set in _os_get_dir_size().*/

    return(dirsize);	
}

int os_get_dir_fileCount(const char *pathname)
{
    int filecount = 0;	

    if (_get_dir_file_count(pathname, &filecount) < 0)
        return(-1);

    return(filecount);
}

int os_mk_dir(const char *pathname, mode_t mode)
{
    if (mkdir(pathname, mode) < 0)
    {
        return(-1);
    }

    return(0);
}

int os_set_dir_mode(const char *pathname, mode_t mode)
{
    if (chmod(pathname, mode) < 0)
    {
        return(-1);
    }

    return(0);
}

int os_copy_dir(const char *oldpath, const char *newpath, int flag, long sleeptime)
{
    int fdold, fdnew;
    char srcbuf[PATH_MAX], dstbuf[PATH_MAX];
    DIR *dp;
    struct dirent *ep;
    struct stat oldbuf, newbuf;

    /*judge the oldpath and newpath is the same dir.*/
    if ((fdold = open(oldpath, O_RDONLY)) < 0)
    {
        return(-1);
    }
    if (fstat(fdold, &oldbuf) != 0) 
    {
        close(fdold);
        return(-1);
    }
    if ((fdnew = open(newpath, O_RDONLY)) >= 0)
    {
        if (stat(newpath, &newbuf) == 0)
        {
            if (oldbuf.st_ino == newbuf.st_ino)	
            {
                close(fdold);
                close(fdnew);
                return(0);
            }
        }
        close(fdnew);
    }
    close(fdold);

    mkdir(newpath, oldbuf.st_mode); /*if newpath exist, ignore; else create.*/
    if ((dp = opendir(oldpath)) == NULL)
    {
        return(-1);
    }
    while ((ep = readdir(dp)) != NULL) 
    {
        snprintf(srcbuf, PATH_MAX, "%s/%s", oldpath, ep->d_name);
        snprintf(dstbuf, PATH_MAX, "%s/%s", newpath, ep->d_name);
        if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
            continue;
        if (lstat(srcbuf, &oldbuf) != 0) 
        {
            closedir(dp);
            return(-1);
        }
        if (S_ISDIR(oldbuf.st_mode))
        {
            if (os_copy_dir(srcbuf, dstbuf, flag, sleeptime) < 0)
            {
                closedir(dp);
                return(-1);
            }
        }
        else if (S_ISREG(oldbuf.st_mode)) 
        {
            if (os_copy_file(srcbuf, dstbuf, flag) < 0)
            {
                closedir(dp);
                return(-1);
            }
            if (sleeptime != 0)
                usleep(sleeptime);
        }
        else if (S_ISLNK(oldbuf.st_mode)) 
        {
            char tmpbuf[PATH_MAX];
            int len;

            if ((len = readlink(srcbuf, tmpbuf, PATH_MAX)) < 0)	
            {
                closedir(dp);
                return(-1);
            }	
            tmpbuf[len] = '\0';
            unlink(dstbuf); /*ignore old link is exist, unlink and relink it.*/
            if (symlink(tmpbuf, dstbuf) < 0)
            {
                closedir(dp);
                return(-1);
            }
        }
        else
            ; /*others is ignored.*/
    }

    if (closedir(dp) < 0)
    {
        return(-1);
    }
    return(0);
}

int os_rename_dir(const char *oldpath, const char *newpath)
{
    if (rename(oldpath, newpath))
    {
        return(-1);
    }

    return(0);
}

int os_rm_dir(const char *pathname)
{
    char dirbuf[PATH_MAX];
    DIR *dp;
    struct dirent *ep;
    struct stat buf;

    if ((dp = opendir(pathname)) == NULL)
    {
        return(-1);
    }
    while ((ep = readdir(dp)) != NULL)
    {
        snprintf(dirbuf, PATH_MAX, "%s/%s", pathname, ep->d_name);
        if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
            continue;
        if (lstat(dirbuf, &buf) != 0)
        {
            closedir(dp);
            return(-1);
        }
        if (S_ISDIR(buf.st_mode))
        {
            if (os_rm_dir(dirbuf) < -1)
            {
                closedir(dp);
                return(-1);
            }
        }
        else
        {
            if (unlink(dirbuf) < 0)
            {
                closedir(dp);
                return(-1);
            }
            _del_file_count++;
            usleep(200*1000);
        }
    }
    if (closedir(dp) < 0)
    {
        return(-1);
    }
    if (rmdir(pathname) < 0)
    {
        return(-1);
    }

    return(0);
}

int os_get_delFileCount(void)
{
    return(_del_file_count);
}

int os_clear_delFileCount(void)
{
    _del_file_count = 0;
    return(0);
}

/*some tools function, not in the commlib.*/
static int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len)
{
    struct flock lock;

    lock.l_type = type; /* F_RDLCK, F_WRLCK, F_UNLCK */
    lock.l_start = offset; /* byte offset, relative to l_whence */
    lock.l_whence = whence; /* SEET_SET, SEEK_CUR, SEEK_END */
    lock.l_len = len; /* #bytes (0 means to EOF) */

    return(fcntl(fd, cmd, &lock));
}

static int _os_get_dir_size(const char *pathname, long long *dirsize)
{
    char dirbuf[PATH_MAX];
    DIR *dp;
    struct dirent *ep;
    struct stat buf;

    if ((dp = opendir(pathname)) == NULL)
    {
        return(-1);
    }
    while ((ep = readdir(dp)) != NULL)
    {
        snprintf(dirbuf, PATH_MAX, "%s/%s", pathname, ep->d_name);
        if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
            continue;
        if (lstat(dirbuf, &buf) != 0)
        {
            closedir(dp);
            return(-1);
        }
        if (S_ISDIR(buf.st_mode))
        {
            if (_os_get_dir_size(dirbuf, dirsize) < 0)
            {
                closedir(dp);
                return(-1);
            }
        }
        else if (S_ISREG(buf.st_mode))
            *dirsize = *dirsize + buf.st_size;
    }
    if (closedir(dp) < 0)
    {
        return(-1);
    }
    *dirsize = *dirsize + 3*4096; /*dir's size is 4KB, each dir has ".", ".." and self, so 12KB.*/

    return(0);
}

static int _get_dir_file_count(const char *pathname, int *filecount)
{
    char dirbuf[PATH_MAX];
    DIR *dp;
    struct dirent *ep;
    struct stat buf;

    if ((dp = opendir(pathname)) == NULL)
    {
        return(-1);
    }
    while ((ep = readdir(dp)) != NULL)
    {
        snprintf(dirbuf, PATH_MAX, "%s/%s", pathname, ep->d_name);
        if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
            continue;
        if (lstat(dirbuf, &buf) != 0)
        {
            closedir(dp);
            return(-1);
        }
        if (S_ISDIR(buf.st_mode))
        {
            if (_get_dir_file_count(dirbuf, filecount) < 0)
            {
                closedir(dp);
                return(-1);
            }
        }
        else if (S_ISREG(buf.st_mode))
            (*filecount) = (*filecount) + 1;
    }
    if (closedir(dp) < 0)
    {
        return(-1);
    }
    return(0);
}

pid_t os_gettid(void)
{
  return syscall(SYS_gettid);
}

/*****************************************************************************
 Prototype    : _atoul
 Description  : 10½øÖÆ×Ö·û´®×ª»»ÎªÎÞ·ûºÅÊý×Ö¡£
 Input  args  : char *str 10½øÖÆ×Ö·û´®
                ²»½ÓÊÜ·ûºÅ
 Output args  : uint32_t* pulValue, ×ª»»ºóµÄÊý×Ö
 Return value : 0 ×ª»»³É¹¦
                -1 ×ª»»Ê§°Ü
*****************************************************************************/
static int _atoul(char *str, uint32_t * pulValue)
{
    uint32_t ulResult=0;

    while (*str)
    {
        if (isdigit((int)*str))
        {
            /*×î´óÖ§³Öµ½0xFFFFFFFF(4294967295), X * 10 + (*str)-48 <= 4294967295 
              ËùÒÔ£¬ X = 429496729 */
            if ((ulResult<429496729) || ((ulResult==429496729) && (*str<'6')))
            {
                ulResult = ulResult*10 + (*str)-48;
            }
            else
            {
                *pulValue = ulResult;
                return -1;
            }
        }
        else
        {
            *pulValue=ulResult;
            return -1;
        }
        str++;
    }
    *pulValue=ulResult;
    return 0;
}

/*****************************************************************************
 Prototype    : _atoulx
 Description  : 16½øÖÆ×Ö·û´®×ª»»ÎªÎÞ·ûºÅÊý×Ö¡£ÊäÈëµÄ16½øÖÆ×Ö·û´®²»°üÀ¨Ç°×º0x
 Input  args  : char *str 16½øÖÆ×Ö·û´®, ²»°üÀ¨Ç°×º0x. ÈçABCDE
 Output args  : uint32_t* pulValue, ×ª»»ºóµÄÊý×Ö
 Return value : 0 ×ª»»³É¹¦
                -1 ×ª»»Ê§°Ü

*****************************************************************************/
#define ASC2NUM(ch) (ch - '0')
#define HEXASC2NUM(ch) (ch - 'A' + 10)

static int _atoulx(char *str, uint32_t* pulValue)
{
    uint32_t ulResult=0;
    uint8_t ch;

    while (*str)
    {
        ch = toupper(*str);
        if (isdigit(ch) || ((ch >= 'A') && (ch <= 'F' )))
        {
            if (ulResult < 0x10000000)
            {
                ulResult = (ulResult << 4) + ((ch<='9')?(ASC2NUM(ch)):(HEXASC2NUM(ch)));
            }
            else
            {
                *pulValue=ulResult;
                return -1;
            }
        }
        else
        {
            *pulValue=ulResult;
            return -1;
        }
        str++;
    }
    
    *pulValue=ulResult;
    return 0;
}

/*****************************************************************************
 Prototype    : os_str2number
 Description  : 10/16 ½øÖÆ×Ö·û´®×ª»»ÎªÎÞ·ûºÅÊý×Ö¡£
 Input  args  : char *str 
                   10½øÖÆ×Ö·û´®, ²»½ÓÊÜ·ûºÅ
                   16½øÖÆ×Ö·û´®, °üÀ¨Ç°×º0x. Èç0x12345678
                            
 Output args  : uint32_t* pulValue, ×ª»»ºóµÄÊý×Ö
 Return value : ×ª»»³É¹¦:0
                ×ª»»Ê§°Ü:-1
*****************************************************************************/
int os_str2number(char *str , uint32_t* pulValue)
{
    /*ÅÐ¶ÏÊÇ·ñ16½øÖÆµÄ×Ö·û´®*/
    if (*str == '0' && (*(str+1) == 'x' || *(str+1) == 'X'))
    {
        if (*(str+2) == '\0')
        {
            return -1;
        }
        else
        {
            return _atoulx(str+2, pulValue);
        }
    }
    else
    {
        return _atoul(str, pulValue);
    }
}
