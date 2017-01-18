/*
 * ntpclient.c - NTP client
 *
 * Copyright 1997, 1999, 2000, 2003, 2006, 2007  Larry Doolittle  <larry@doolittle.boa.org>
 * Last hack: December 30, 2007
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License (Version 2,
 *  June 1991) as published by the Free Software Foundation.  At the
 *  time of writing, that license was published by the FSF with the URL
 *  http://www.gnu.org/copyleft/gpl.html, and is incorporated herein by
 *  reference.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  Possible future improvements:
 *      - Write more documentation  :-(
 *      - Support leap second processing
 *      - Support IPv6
 *      - Support multiple (interleaved) servers
 *
 *  Compile with -DPRECISION_SIOCGSTAMP if your machine really has it.
 *  There are patches floating around to add this to Linux, but
 *  usually you only get an answer to the nearest jiffy.
 *  Hint for Linux hacker wannabes: look at the usage of get_fast_time()
 *  in net/core/dev.c, and its definition in kernel/time.c .
 *
 *  If the compile gives you any flak, check below in the section
 *  labelled "XXX fixme - non-automatic build configuration".
 */

#define _POSIX_C_SOURCE 199309

 
#define _BSD_SOURCE
 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>     /* gethostbyname */
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <sys/utsname.h>
#include <sys/time.h>
#include <sys/timex.h>
#include <sys/prctl.h>
#include "ntpclient.h"

typedef unsigned int uint32;
typedef unsigned char uint8;
typedef unsigned short uint16;

#define NTP_TRACE_GREEN			printf
#define NTP_TRACE_RED			printf
#define ERRNO(a,b)				((a << 16) | b)
#define PERROR(a)				{ printf("%s %d Error %X\n",__FUNCTION__,__LINE__,a)	}

// 错误代码采用宏定义
typedef enum {
    NTP_ERROR_ADJTIMEX = 0x1000, 
    NTP_ERROR_CLOCK_SETTIME, 
    NTP_ERROR_SETTIMEOFDAY, 
    NTP_ERROR_GETHOSTBYNAME, 
    NTP_ERROR_SOCKET, 
    NTP_ERROR_BIND, 
    NTP_ERROR_CONNECT, 
    NTP_ERROR_REPLAY, 
    NTP_ERROR_LI, // error checking,see RFC-4330 section 5,FAIL("LI==3")
    NTP_ERROR_VN, // error checking,see RFC-4330 section 5,FAIL("VN<3")
    NTP_ERROR_MODE, // RFC-4330 documents SNTP v4,but we interoperate with NTP v3,FAIL("MODE!=3")
    NTP_ERROR_ORG, // error checking,see RFC-4330 section 5,FAIL("ORG!=sent")
    NTP_ERROR_XMT, // error checking,see RFC-4330 section 5,FAIL("XMT==0")
    NTP_ERROR_DELAY, // error checking,see RFC-4330 section 5,FAIL("abs(DELAY)>65536")
    NTP_ERROR_DISP, // error checking,see RFC-4330 section 5,FAIL("abs(DISP)>65536")
    NTP_ERROR_STRATUM, // error checking,see RFC-4330 section 5,FAIL("STRATUM==0")
    NTP_ERROR_TCRT,
    NTP_ERROR_PORT
} NTP_ERROR;


#define COMP_NENT           0
#define COMP_FLAG           0x4350544E // "NTPC"
#define FLAG_VERIFY(fd)     ((fd)->flag == COMP_FLAG)

/* Default to the RFC-4330 specified value */
#ifndef MIN_INTERVAL
#define MIN_INTERVAL    15
#endif 

typedef uint32_t u32; /* universal for C99 */
 
/* end configuration for host systems */

#define JAN_1970        0x83aa7e80      /* 2208988800 1970 - 1900 in seconds */
#define NTP_PORT        (123)

/* How to multiply by 4294.967296 quickly (and not quite exactly)
 * without using floating point or greater than 32-bit integers.
 * If you want to fix the last 12 microseconds of error, add in
 * (2911*(x))>>28)
 */
#define NTPFRAC(x)      (4294 * (x) + ((1981 * (x)) >> 11))

/* The reverse of the above, needed if we want to set our microsecond
 * clock (via clock_settime) based on the incoming time in NTP format.
 * Basically exact.
 */
#define USEC(x)         (((x) >> 12) - 759 * ((((x) >> 10) + 32768) >> 16))

/* Converts NTP delay and dispersion, apparently in seconds scaled
 * by 65536, to microseconds.  RFC-1305 states this time is in seconds,
 * doesn't mention the scaling.
 * Should somehow be the same as 1000000 * x / 65536
 */
#define sec2u(x)        ((x) * 15.2587890625)

struct ntptime {
    unsigned int coarse;
    unsigned int fine;
};

struct ntp_control {
    u32 time_of_send[2];
    int live;
    int set_clock; /* non-zero presumably needs root privs */
    int probe_count;
    int cycle_time;
    int goodness;
    int cross_check;
    char serv_addr[4];
    int ntp_port;
};

/* prototypes for some local routines */
static void send_packet(int usd, u32 time_sent[2]);
static int rfc1305print(u32 *data, struct ntptime *arrival, struct ntp_control *ntpc, int *error, int time_zone);
/*
static void udp_handle(int usd, char* data, int data_len, struct sockaddr* sa_source, int sa_len);
*/

static int get_current_freq(void)
{
    /* OS dependent routine to get the current value of clock frequency. */
#ifdef __linux__
	struct timex txc;
	int rtval = 0;

	txc.modes = 0;
 
	if ((rtval = adjtimex(&txc)) < 0)
	{
		NTP_TRACE_GREEN("adjtimex error: %d, %s\n", errno, strerror(errno));
		return ERRNO(NTP_ERROR_ADJTIMEX, COMP_NENT);
	} 
	return txc.freq;
#else 
	return 0;
#endif 
}

static int set_freq(int new_freq)
{
    /* OS dependent routine to set a new value of clock frequency. */
#ifdef __linux__
    struct timex txc;
    int rtval = 0;

    txc.modes   = ADJ_FREQUENCY;
    txc.freq    = new_freq;
#if 0
    if ((rtval = __adjtimex(&txc)) < 0) {
        NTP_TRACE_GREEN("__adjtimex error: %d, %s\n", errno, strerror(errno));
#else
    if ((rtval = adjtimex(&txc)) < 0) {
        NTP_TRACE_GREEN("__adjtimex error: %d, %s\n", errno, strerror(errno));
#endif
        return ERRNO(NTP_ERROR_ADJTIMEX, COMP_NENT);
    } 
    return txc.freq;
#else 
    return 0;
#endif 
}

static int set_time(struct ntptime *newtime, int time_zone)
{
	int rtval = 0;
 
	struct timeval tv_set; /* Traditional Linux way to set the system clock */
	tv_set.tv_sec   = newtime->coarse - JAN_1970;// + (time_zone-12) * 60 * 60; /* it would be even better to subtract half the slop */
	tv_set.tv_usec  = USEC(newtime->fine); /* divide xmttime.fine by 4294.967296 */
	printf("----------1----------settimeofday tv_set.tv_sec=%ld\n",tv_set.tv_sec);

	if ((rtval = settimeofday(&tv_set, NULL)) < 0) 
	{
		NTP_TRACE_GREEN("settimeofday error: %d, %s\n", errno, strerror(errno));
		return ERRNO(NTP_ERROR_SETTIMEOFDAY, COMP_NENT);
	}
	//set成功 且获取的时间大于2015/1/1/00:00:00	 //1420041600 北京时间2015/1/1/00:00:00
	if((0 == rtval) && ((unsigned long )tv_set.tv_sec > 1420041600))
    {
    	printf("((0 == rtval) && ((unsigned long )tv_set.tv_sec > 1420041600))\n");
    	return 0;
	}
	return -1;
}

static void ntpc_gettime(u32 *time_coarse, u32 *time_fine)
{
 
	struct timeval now; /* Traditional Linux way to get the system time */
	gettimeofday(&now, NULL);
	*time_coarse    = now.tv_sec + JAN_1970;
	*time_fine      = NTPFRAC(now.tv_usec);
  
}

static void send_packet(int usd, u32 time_sent[2])
{
	u32 data[12];
#define LI 0
#define VN 3
#define MODE 3
#define STRATUM 0
#define POLL 4
#define PREC -6

	if (sizeof(data) != 48)
	{
		NTP_TRACE_GREEN("size error\n");
		return;
	}

	memset(data, 0x00, sizeof(data));
	data[0] = htonl((LI << 30) | (VN << 27) | (MODE << 24) | (STRATUM << 16) | (POLL << 8) | (PREC & 0xff));
	data[1] = htonl(1 << 16); /* Root Delay (seconds) */
	data[2] = htonl(1 << 16); /* Root Dispersion (seconds) */
	ntpc_gettime(time_sent, time_sent + 1);
	data[10] = htonl(time_sent[0]); /* Transmit Timestamp coarse */
	data[11] = htonl(time_sent[1]); /* Transmit Timestamp fine   */
	send(usd, data, 48, 0);
	return;
}

static void get_packet_timestamp(int usd, struct ntptime *udp_arrival_ntp)
{
	ntpc_gettime(&udp_arrival_ntp->coarse, &udp_arrival_ntp->fine);
}

static int check_source(int data_len, struct sockaddr *sa_source, unsigned int sa_len, struct ntp_control *ntpc)
{
    struct sockaddr_in *sa_in = (struct sockaddr_in *)sa_source;
    
    (void)sa_len; /* not used */
    
#ifdef USER_DEBUG
	NTP_TRACE_GREEN("packet of length %d received\n", data_len)
	if (sa_source->sa_family == AF_INET) 
	{
		NTP_TRACE_GREEN("Source: INET Port %d host %s\n", ntohs(sa_in->sin_port), inet_ntoa(sa_in->sin_addr));
	}
	else
	{
		NTP_TRACE_GREEN("Source: Address family %d\n", sa_source->sa_family);
	}
#else
	(void)data_len; /* not used */
#endif
	if (ntpc->ntp_port != (int)ntohs(sa_in->sin_port))
	{
		NTP_TRACE_GREEN("ntp server port error, %d != %d\n", ntpc->ntp_port, ntohs(sa_in->sin_port));
		return ERRNO(NTP_ERROR_PORT, COMP_NENT); /* fault */
	}
	return 0;
}

static double ntpdiff(struct ntptime *start, struct ntptime *stop)
{
	int a = 0;
	unsigned int b = 0;
    
	a = stop->coarse - start->coarse;
	if (stop->fine >= start->fine) 
	{
		b = stop->fine - start->fine;
	} 
	else
	{
		b   = start->fine - stop->fine;
		b   = ~b;
		a  -= 1;
	}
	return (a * 1.e6 + b * (1.e6 / 4294967296.0));
}

int contemplate_data(unsigned int absolute, double skew, double errorbar, int freq);

/* Does more than print, so this name is bogus. It also makes time adjustments, both sudden (-s)
 * and phase-locking (-l). sets *error to the number of microseconds uncertainty in answer
 * returns 0 normally, 1 if the message fails sanity checks
 */
static int rfc1305print(u32 *data, struct ntptime *arrival, struct ntp_control *ntpc, int *error, int time_zone)
{
	/* straight out of RFC-1305 Appendix A */
	int li, vn, mode, stratum, poll, prec, delay, disp, refid, freq, rtval = 0;
	struct ntptime reftime, orgtime, rectime, xmttime;
	double el_time, st_time, skew1, skew2;
 

#define Data(i)  ntohl(((u32 *)data)[i])
	li      = Data(0) >> 30 & 0x03;
	vn      = Data(0) >> 27 & 0x07;
	mode    = Data(0) >> 24 & 0x07;
	stratum = Data(0) >> 16 & 0xff;
	poll    = Data(0) >> 8 & 0xff;
	prec    = Data(0) & 0xff;
	if (prec & 0x80) 
	{
		prec |= 0xffffff00;
	}
	delay           = Data(1);
	disp            = Data(2);
	refid           = Data(3);
	reftime.coarse  = Data(4);
	reftime.fine    = Data(5);
	orgtime.coarse  = Data(6);
	orgtime.fine    = Data(7);
	rectime.coarse  = Data(8);
	rectime.fine    = Data(9);
	xmttime.coarse  = Data(10);
	xmttime.fine    = Data(11);
	
#undef Data

	
	NTP_TRACE_GREEN("LI=%d  VN=%d  Mode=%d  Stratum=%d  Poll=%d  Precision=%d\n", li, vn, mode, stratum, poll, prec);
	NTP_TRACE_GREEN("Delay=%.1f  Dispersion=%.1f  Refid=%u.%u.%u.%u\n", sec2u(delay), sec2u(disp), refid >> 24 & 0xff, refid >> 16 & 0xff, refid >> 8 & 0xff, refid & 0xff);
	NTP_TRACE_GREEN("Reference %u.%.6u\n", reftime.coarse, USEC(reftime.fine));
	NTP_TRACE_GREEN("(sent)    %u.%.6u\n", ntpc->time_of_send[0], USEC(ntpc->time_of_send[1]));
	NTP_TRACE_GREEN("Originate %u.%.6u\n", orgtime.coarse, USEC(orgtime.fine));
	NTP_TRACE_GREEN("Receive   %u.%.6u\n", rectime.coarse, USEC(rectime.fine));
	NTP_TRACE_GREEN("Transmit  %u.%.6u\n", xmttime.coarse, USEC(xmttime.fine));
	NTP_TRACE_GREEN("Our recv  %u.%.6u\n", arrival->coarse, USEC(arrival->fine));
	

	el_time = ntpdiff(&orgtime, arrival); /* elapsed */
	st_time = ntpdiff(&rectime, &xmttime); /* stall */
	skew1   = ntpdiff(&orgtime, &rectime);
	skew2   = ntpdiff(&xmttime, arrival);
	if ((freq = get_current_freq()) < 0)
	{
		NTP_TRACE_GREEN("Total elapsed: %9.2f\n""Server stall:  %9.2f\n""Slop:          %9.2f\n", el_time, st_time, el_time - st_time);
		NTP_TRACE_GREEN("Skew:          %9.2f\n""Frequency:     %9d\n"" day   second     elapsed    stall     skew  dispersion  freq\n", (skew1 - skew2)/2, freq);
		return freq;
	}

	/* error checking, see RFC-4330 section 5 */
 
#define FAIL(x)		{ printf(x);	goto fail; }
 
	if (ntpc->cross_check) 
	{
		if (li == 3) 
		{
			rtval = ERRNO(NTP_ERROR_LI, COMP_NENT);
			FAIL("LI==3\n");
		}

		/* unsynchronized */
		if (vn < 3) 
		{
			rtval = ERRNO(NTP_ERROR_VN, COMP_NENT);
			FAIL("VN<3\n");
		}

		/* RFC-4330 documents SNTP v4, but we interoperate with NTP v3 */
		if (mode != 4)
		{
			rtval = ERRNO(NTP_ERROR_MODE, COMP_NENT);
			FAIL("MODE!=3\n");
		}

		if ((orgtime.coarse != ntpc->time_of_send[0]) || (orgtime.fine != ntpc->time_of_send[1]))
		{
			rtval = ERRNO(NTP_ERROR_GETHOSTBYNAME, COMP_NENT);
			FAIL("ORG!=sent\n");
		}

		if (xmttime.coarse == 0 && xmttime.fine == 0) 
		{
			rtval = ERRNO(NTP_ERROR_GETHOSTBYNAME, COMP_NENT);
			FAIL("XMT==0\n");
		}

		if (delay > 65536 || delay < -65536) 
		{
			rtval = ERRNO(NTP_ERROR_GETHOSTBYNAME, COMP_NENT);
			FAIL("abs(DELAY)>65536\n");
		}

		if (disp > 65536 || disp < -65536) 
		{
			rtval = ERRNO(NTP_ERROR_GETHOSTBYNAME, COMP_NENT);
			FAIL("abs(DISP)>65536\n");
		}

		if (stratum == 0)
		{
			rtval = ERRNO(NTP_ERROR_GETHOSTBYNAME, COMP_NENT);
			FAIL("STRATUM==0\n");
		}
		/* kiss o' death */
#undef FAIL
	}

	/* XXX should I do this if debug flag is set? */
	if (ntpc->set_clock)
	{ 
		/* you'd better be root, or ntpclient will crash! */
		if ((rtval = set_time(&xmttime, time_zone)) < 0)
		{
			return rtval;
		}
	}

	/* Not the ideal order for printing, but we want to be sure to do all the time-sensitive thinking (and time setting)
	* before we start the output, especially fflush() (which could be slow).  Of course, if debug is turned on, speed
	* has gone down the drain anyway. */
	if (ntpc->live)
	{
		int new_freq;
		new_freq = contemplate_data(arrival->coarse, (skew1 - skew2)/2, el_time + sec2u(disp), freq);
		if (new_freq != freq)
		{
			if ((rtval = set_freq(new_freq)) < 0)
			{
				return rtval;
			}
		}
	}
	/*
	NTP_TRACE_GREEN("%d %.5d.%.3d  %8.1f %8.1f  %8.1f %8.1f %9d\n", 
		arrival->coarse/86400, arrival->coarse%86400, arrival->fine/4294967, el_time, st_time, (skew1 - skew2)/2, sec2u(disp), freq);
	*/
	fflush(stdout);
	*error = (int)(el_time - st_time);
	if( 0 == rtval){return 0;}
	return -1;
fail:
 
	//NTP_TRACE_GREEN("%d %.5d.%.3d  rejected packet\n", arrival->coarse/86400, arrival->coarse%86400, arrival->fine/4294967);
 
	return rtval;
}

static int primary_loop(int usd, struct ntp_control *ntpc, int time_zone)
{
	fd_set fds;
	int		bSuccess = 0;
	struct sockaddr sa_xmit;
	int rtval = 0, pack_len = 0, probes_sent = 0, error = 0;
	unsigned int sa_xmit_len;
	struct timeval to;
	struct ntptime udp_arrival_ntp;
	static u32 incoming_word[325];
 
	probes_sent = 0;
	sa_xmit_len = sizeof(sa_xmit);
	to.tv_sec   = 0;
	to.tv_usec  = 0;
	for (;;) 
	{
		FD_ZERO(&fds);
		FD_SET(usd, &fds);
		rtval = select(usd + 1, &fds, NULL, NULL, &to); /* Wait on read or error */
		if ((rtval != 1) || (!FD_ISSET(usd, &fds))) 
		{
			if (rtval < 0)
			{
				if (errno != EINTR)
				{
					printf("select error: %d, %s\n", errno, strerror(errno));
				}
				continue;
			}
			if (to.tv_sec == 0) 
			{
				if ((probes_sent >= ntpc->probe_count) && (ntpc->probe_count != 0)) 
				{
					printf("if ((probes_sent >= ntpc->probe_count) && (ntpc->probe_count != 0))\n");
					return -1;
				}
				send_packet(usd, ntpc->time_of_send);
				++probes_sent;
				to.tv_sec   = ntpc->cycle_time;
				to.tv_usec  = 0;
			}
			continue;
		}
		pack_len    = recvfrom(usd, (char *)incoming_word, sizeof(incoming_word), 0, &sa_xmit, &sa_xmit_len);
		error       = ntpc->goodness;
		if (pack_len < 0) 
		{
			printf("recvfrom error: %d, %s\n", errno, strerror(errno));
		}
        else 
    	{
			if (pack_len > 0 && (unsigned)pack_len < sizeof(incoming_word)) 
			{
				get_packet_timestamp(usd, &udp_arrival_ntp);
				if (check_source(pack_len, &sa_xmit, sa_xmit_len, ntpc) != 0)
				{
					continue;
				}
				if (rfc1305print(incoming_word, &udp_arrival_ntp, ntpc, &error, time_zone) != 0) 
				{
					return -1;
				}
				else
				{
					bSuccess = 1;
					printf("bSuccess=%d\n",bSuccess);
				}
			}
			else
			{
				printf("Ooops. pack_len=%d\n", pack_len);
				fflush(stdout);
			}
    	}

		/* best rollover option: specify -g, -s, and -l. simpler rollover option: specify -s and -l, which triggers a magic -c 1 */
		if ((error < ntpc->goodness && ntpc->goodness != 0) || (probes_sent >= ntpc->probe_count && ntpc->probe_count != 0))
		{
			ntpc->set_clock = 0;
			if (!ntpc->live) 
			{
				break;
			}
		}
	}
	if(bSuccess == 0){return -1;}
	return 0;
}

/******************************************************************************
* 函数介绍: NTP网络校时
* 输入参数: hostname: NTP服务器
*           ntp_server_port: NTP服务器端口号,0-表示默认值,内部默认为123,其他-表示端口号
*           time_zone: 时区
* 输出参数: 无
* 返回值  : 0-成功,<0-失败
 *****************************************************************************/
int ntpclient(const char *hostname, int ntp_server_port, int time_zone)
{
	int usd = 0; 
	int rtval = 0;
	struct ntp_control ntpc;
	struct sockaddr_in sa_rcvr;
	struct sockaddr_in sa_dest;
	struct hostent *ntpserver = NULL;
	if (hostname == NULL) 
	{
		NTP_TRACE_GREEN("Unsupported hostname is NULL\n");
		return -1;
	}

    memset(&ntpc, 0, sizeof(ntpc));
	if (ntp_server_port <= 0) 
	{
		ntpc.ntp_port = NTP_PORT;
	}
	else 
	{
		ntpc.ntp_port = ntp_server_port;
	}
	ntpc.cross_check    = 1;
	ntpc.cycle_time     = 2;//600; /* seconds */
	ntpc.goodness       = 0;
	ntpc.live           = 0;
	ntpc.probe_count    = 1; /* default of 0 means loop forever */
	ntpc.set_clock      = 1;
 
	if (ntpc.set_clock && !ntpc.live && !ntpc.goodness && !ntpc.probe_count)
	{
		ntpc.probe_count = 1;
	}

	/* respect only applicable MUST of RFC-4330 */
	if ((ntpc.probe_count != 1) && (ntpc.cycle_time < MIN_INTERVAL)) 
	{
		ntpc.cycle_time = MIN_INTERVAL;
	}

	//获取服务器IP
	ntpserver = gethostbyname(hostname);
	if (ntpserver == NULL) 
	{
//		printf("NTP gethostbyname %s Failed %d\n",hostname,errno);
		return -1;
	}
	if (ntpserver->h_length != 4)
	{ 
		printf("oops %d\n", ntpserver->h_length);	/* IPv4 only, until I get a chance to test IPv6 */
		return -1;
	}

	//[2011-11-28 18:42:23] 薛长春 准备连接参数
	memset(&sa_rcvr, 0, sizeof(sa_rcvr));
	memset(&sa_dest, 0, sizeof(sa_dest));
	
	sa_rcvr.sin_family      = AF_INET;
	sa_rcvr.sin_addr.s_addr = htonl(INADDR_ANY);
	sa_rcvr.sin_port        = htons(0);

	sa_dest.sin_family		= AF_INET;
	memcpy(&sa_dest.sin_addr.s_addr, ntpserver->h_addr_list[0], 4);
	sa_dest.sin_port		= htons(ntpc.ntp_port);

 
	if ((usd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		printf("socket error: %d, %s\n", errno, strerror(errno));
		return -1;
	}
	
	if ((rtval = bind(usd, (struct sockaddr *)&sa_rcvr, sizeof(sa_rcvr))) < 0) 
	{
		printf("could not bind to udp port 0 error: %d, %s, \n", errno, strerror(errno));
		close(usd);
		return -2;
	}
	
	if ((rtval = connect(usd, (struct sockaddr *)&sa_dest, sizeof(sa_dest))) < 0)
	{
		printf("NTP connect error: %d, %s\n", errno, strerror(errno));
		close(usd);
		return -1;
	}
 
	memcpy(ntpc.serv_addr, &(sa_dest.sin_addr), 4); /* XXX asumes IPv4 */
	rtval = primary_loop(usd, &ntpc, time_zone);
	close(usd); 
	return rtval;
}

int g_nNTPStatus = NTP_STATUS_NORUN;
int g_nNTPSleep = 0;

int DMS_GetNTPStatus(int *lpSleepTime)
{
    if(lpSleepTime)
        *lpSleepTime = g_nNTPSleep;
    return g_nNTPStatus;
}


/*
typedef unsigned int uint32;
typedef unsigned char uint8;
typedef unsigned short uint16;
*/
#define NTP_TRACE_GREEN			printf
#define NTP_TRACE_RED			printf
#define ERRNO(a,b)				((a << 16) | b)
#define PERROR(a)				{ printf("%s %d Error %X\n",__FUNCTION__,__LINE__,a)	}
 


double min_delay = 800.0; /* global, user-changeable, units are microseconds */

#define RING_SIZE       16
#define MAX_CORRECT     250   /* ppm change to system clock */
#define MAX_C           ((MAX_CORRECT) * 65536)

static struct datum {
    unsigned int absolute;
    double skew;
    double errorbar;
    int freq;

    /* s.s.min and s.s.max (skews) are "corrected" to what they would
     * have been if freq had been constant at its current value during
     * the measurements.
     */
    union {
        struct {
            double min;
            double max;
        } s;
        double ss[2];
    } s;

    /*
    double smin;
    double smax;
     */
} d_ring[RING_SIZE];

static struct _seg {
    double slope;
    double offset;
} maxseg[RING_SIZE + 1], minseg[RING_SIZE + 1];

 

static int next_up(int i)
{
    int r = i + 1;

    if (r >= RING_SIZE) {
        r = 0;
    }

    return r;
}

static int next_dn(int i)
{
    int r = i - 1;

    if (r < 0) {
        r = RING_SIZE - 1;
    }

    return r;
}

/* Looks at the line segments that start at point j, that end at
 * all following points (ending at index rp).  The initial point
 * is on curve s0, the ending point is on curve s1.  The curve choice
 * (s.min vs. s.max) is based on the index in ss[].  The scan
 * looks for the largest (sign=0) or smallest (sign=1) slope.
 */
static int search(int rp, int j, int s0, int s1, int sign, struct _seg *answer)
{
    double dt = 0, slope = 0;
    int n = 0, nextj = 0, cinit = 1;

    for (n = next_up(j); n != next_up(rp); n = next_up(n)) {
        NTP_TRACE_GREEN("d_ring[%d].s.ss[%d]=%f d_ring[%d].s.ss[%d]=%f\n",
			n, s0, d_ring[n].s.ss[s0], 
			j, s1, d_ring[j].s.ss[s1]);

        dt      = d_ring[n].absolute - d_ring[j].absolute;
        slope   = (d_ring[n].s.ss[s0] - d_ring[j].s.ss[s1]) / dt;
        NTP_TRACE_GREEN("slope %d%d%d [%d,%d] = %f\n", s0, s1, sign, j, n, slope);

        if (cinit || (slope < answer->slope) ^ sign) {
            answer->slope   = slope;
            answer->offset  = d_ring[n].s.ss[s0] + slope * (d_ring[rp].absolute - d_ring[n].absolute);
            cinit           = 0;
            nextj           = n;
        } 
    }

    return nextj;
}

/* Pseudo-class for finding consistent frequency shift */
#define MIN_INIT 20
static struct _polygon {
    double l_min;
    double r_min;
} df;

static void polygon_reset(void)
{
    df.l_min = MIN_INIT;
    df.r_min = MIN_INIT;
}

static double find_df(int* flag)
{
    if (df.l_min == 0.0) {
        if (df.r_min == 0.0) {
            return 0.0; /* every point was OK */
        }
        else {
            return -df.r_min;
        }
    }
    else {
        if (df.r_min == 0.0) {
            return df.l_min;
        }
        else {
            if (flag) {
                *flag = 1;
            }

            return 0.0; /* some points on each side, * or no data at all */
        }
    }

    return 0.0;
}

/* Finds the amount of delta-f required to move a point onto a
 * target line in delta-f/delta-t phase space.  Any line is OK
 * as long as it's not convex and never returns greater than
 * MIN_INIT. */
static double find_shift(double slope, double offset)
{
    double shift = slope - offset / 600.0;
    double shift2 = slope + 0.3 - offset / 6000.0;
    
    if (shift2 < shift) {
        shift = shift2;
    }
    NTP_TRACE_GREEN("find_shift %f %f -> %f\n", slope, offset, shift);

    if (shift < 0) {
        return 0.0;
    }

    return shift;
}

static void polygon_point(struct _seg *s)
{
    double l = 0.0, r = 0.0;

    l = find_shift(-s->slope, s->offset);
    r = find_shift(s->slope, -s->offset);

    if (l < df.l_min) {
        df.l_min = l;
    }

    if (r < df.r_min) {
        df.r_min = r;
    }
    NTP_TRACE_GREEN("loop %f %f\n", s->slope, s->offset);
    NTP_TRACE_GREEN("constraint left:  %f %f \n", l, df.l_min);
    NTP_TRACE_GREEN("constraint right: %f %f \n", r, df.r_min);
} 

/* Something like linear feedback to be used when we are "close" to
 * phase lock.  Not really used at the moment:  the logic in find_df()
 * never sets the flag. */
static double find_df_center(struct _seg *min, struct _seg *max, double gross_df)
{
    const double crit_time = 1000.0;
    double slope = 0.5 *(max->slope + min->slope) + gross_df;
    double dslope = (max->slope - min->slope);
    double offset = 0.5 *(max->offset + min->offset);
    double doffset = (max->offset - min->offset);
    double delta1 = -offset / 600.0 - slope;
    double delta2 = -offset / 1800.0 - slope;
    double delta = 0.0;
    double factor = crit_time / (crit_time + doffset + dslope * 1200.0);
    
    if (offset < 0 && delta2 > 0) {
        delta = delta2;
    }

    if (offset < 0 && delta1 < 0) {
        delta = delta1;
    }

    if (offset >= 0 && delta1 > 0) {
        delta = delta1;
    }

    if (offset >= 0 && delta2 < 0) {
        delta = delta2;
    }

    if (max->offset < -crit_time || min->offset > crit_time) {
        return 0.0;
    }
    NTP_TRACE_GREEN("find_df_center %f %f\n", delta, factor);

    return (factor * delta);
} 
 
int contemplate_data(unsigned int absolute, double skew, double errorbar, int freq)
{
    /*  Here is the actual phase lock loop.
     *  Need to keep a ring buffer of points to make a rational
     *  decision how to proceed.  if (debug) print a lot.
     */
    static int rp = 0, valid = 0;
    int both_sides_now = 0;
    int j, n, c, max_avail, min_avail, dinit;
    int nextj = 0; /* initialization not needed; but gcc can't figure out my logic */
    double cum;
    struct _seg check, save_min, save_max;
    double last_slope;
    int delta_freq;
    double delta_f;
    int inconsistent = 0, max_imax, max_imin = 0, min_imax, min_imin = 0;
    int computed_freq = freq;

    NTP_TRACE_GREEN("xontemplate %u %.1f %.1f %d\n", absolute, skew, errorbar, freq);

    d_ring[rp].absolute = absolute;
    d_ring[rp].skew     = skew;
    d_ring[rp].errorbar = errorbar - min_delay; /* quick hack to speed things up */
    d_ring[rp].freq     = freq;

    if (valid < RING_SIZE) {
        ++valid;
    }
    if (valid == RING_SIZE) {
        /*
         * Pass 1: correct for wandering freq's */
        cum = 0.0;
        NTP_TRACE_GREEN("\n");

        for (j = rp;; j = n) {
            d_ring[j].s.s.max = d_ring[j].skew - cum + d_ring[j].errorbar;
            d_ring[j].s.s.min = d_ring[j].skew - cum - d_ring[j].errorbar;
            NTP_TRACE_GREEN("hist %d %d %f %f %f\n", j, d_ring[j].absolute - absolute, cum, d_ring[j].s.s.min, d_ring[j].s.s.max);

            n = next_dn(j);
            if (n == rp) {
                break;
            }

            /* Assume the freq change took place immediately after
             * the data was taken; this is valid for the case where
             * this program was responsible for the change.
             */
            cum = cum + (d_ring[j].absolute - d_ring[n].absolute) * (double)(d_ring[j].freq - freq) / 65536;
        }

        /*
         * Pass 2: find the convex down envelope of s.max, composed of
         * line segments in s.max vs. absolute space, which are
         * points in freq vs. dt space.  Find points in order of increasing
         * slope == freq */
        dinit       = 1;
        last_slope  = -2 * MAX_CORRECT;
        for (c = 1, j = next_up(rp);; j = nextj) {
            nextj = search(rp, j, 1, 1, 0, &maxseg[c]);
            search(rp, j, 0, 1, 1, &check);
            if (check.slope < maxseg[c].slope && check.slope > last_slope && (dinit || check.slope < save_min.slope)) {
                dinit = 0;
                save_min = check;
            }
            NTP_TRACE_GREEN("maxseg[%d] = %f *x+ %f\n", c, maxseg[c].slope, maxseg[c].offset);

            last_slope = maxseg[c].slope;
            c++;
            if (nextj == rp) {
                break;
            }
        }

        if (dinit == 1) {
            inconsistent = 1;
        }

        if (dinit == 0) {
            NTP_TRACE_GREEN("mincross %f *x+ %f\n", save_min.slope, save_min.offset);
        }
        max_avail = c;

        /*
         * Pass 3: find the convex up envelope of s.min, composed of
         * line segments in s.min vs. absolute space, which are
         * points in freq vs. dt space.  These points are found in
         * order of decreasing slope. */
        dinit       = 1;
        last_slope  = +2 * MAX_CORRECT;
        for (c = 1, j = next_up(rp);; j = nextj) {
            nextj = search(rp, j, 0, 0, 1, &minseg[c]);
            search(rp, j, 1, 0, 0, &check);
            if (check.slope > minseg[c].slope && check.slope < last_slope && (dinit || check.slope < save_max.slope)) {
                dinit = 0;
                save_max = check;
            }
            NTP_TRACE_GREEN("minseg[%d] = %f *x+ %f\n", c, minseg[c].slope, minseg[c].offset);

            last_slope = minseg[c].slope;
            c++;
            if (nextj == rp) {
                break;
            }
        }

        if (dinit == 1) {
            inconsistent = 1;
        }

        if (dinit == 0) {
            NTP_TRACE_GREEN("maxcross %f *x+ %f\n", save_max.slope, save_max.offset);
        }
        min_avail = c;

        /*
         * Pass 4: splice together the convex polygon that forms
         * the envelope of slope/offset coordinates that are consistent
         * with the observed data.  The order of calls to polygon_point
         * doesn't matter for the frequency shift determination, but
         * the order chosen is nice for visual display. */
        if (!inconsistent) {
            polygon_reset();
            polygon_point(&save_min);
            for (dinit = 1, c = 1; c < max_avail; c++) {
                if (dinit && maxseg[c].slope > save_min.slope) {
                    max_imin = c - 1;
                    maxseg[max_imin] = save_min;
                    dinit = 0;
                }

                if (maxseg[c].slope > save_max.slope) {
                    break;
                }

                if (dinit == 0) {
                    polygon_point(&maxseg[c]);
                }
            }
            if (dinit) {
                NTP_TRACE_GREEN("found maxseg vs. save_min inconsistency\n");
            }

            if (dinit) {
                inconsistent = 1;
            }

            max_imax            = c;
            maxseg[max_imax]    = save_max;

            polygon_point(&save_max);
            for (dinit = 1, c = 1; c < min_avail; c++) {
                if (dinit && minseg[c].slope < save_max.slope) {
                    max_imin = c - 1;
                    minseg[min_imin] = save_max;
                    dinit = 0;
                }

                if (minseg[c].slope < save_min.slope) {
                    break;
                }

                if (dinit == 0) {
                    polygon_point(&minseg[c]);
                }
            }

            if (dinit) {
                NTP_TRACE_GREEN("found minseg vs. save_max inconsistency\n");
            }

            if (dinit) {
                inconsistent = 1;
            }
            min_imax = c;
            minseg[min_imax] = save_max;

            /* not needed for analysis, but shouldn't hurt either */
            polygon_point(&save_min);
        } /* !inconsistent */

        /*
         * Pass 5: decide on a new freq */
        if (inconsistent) {
            NTP_TRACE_GREEN("# inconsistent\n");
        }
        else {
            delta_f = find_df(&both_sides_now);
            NTP_TRACE_GREEN("find_df() = %e\n", delta_f);

            delta_f    += find_df_center(&save_min, &save_max, delta_f);
            delta_freq  = (int)(delta_f * 65536+.5);
            NTP_TRACE_GREEN("delta_f %f  delta_freq %d  bsn %d\n", delta_f, delta_freq, both_sides_now);

            computed_freq -= delta_freq;
            NTP_TRACE_GREEN("# box [( %.3f , %.1f )  ( %.3f , %.1f )]  delta_f %.3f  computed_freq %d\n", 
                save_min.slope, save_min.offset, save_max.slope, save_max.offset, delta_f, computed_freq);

            if (computed_freq < -MAX_C) {
                computed_freq = -MAX_C;
            }

            if (computed_freq > MAX_C) {
                computed_freq = MAX_C;
            }
        }
    }

    rp = (rp + 1) % RING_SIZE;

    return computed_freq;
}









