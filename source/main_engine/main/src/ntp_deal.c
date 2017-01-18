/* *********************************************************************
 * 	Description :
 *		ntp_conf.c
 *		NTP Configuration CGI. 
 *		Set the NTP server or PC clock for adjusting system time.
 *		
 *
 * 	Authors : 
 *		novi Hu <noviblue@163.com> 
 * 	
 *	Copyright(c) 2006 Information & Communication Research Laboratories,
 * 	Industrial Technology Research Institute (http://www.nvi.com)
 * 	All rights reserved.
 *
 ************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <linux/rtc.h>
#include <fcntl.h>
#include "ntpclient.h"
#include "log.h"
#include "sdk_struct.h"
#include "adapt.h"

#define NTP_CONFIG_PATH    			"/mnt/mtd/sysconfig/ntp_conf"

// Debug information print
#define DEBUG	0

ntp_conf g_ntp_pra;
char filename[128];
char getParaValues[256];
//extern	int gettimeofday(struct timeval *tv, struct timezone *tz);
//extern	int settimeofday(const struct timeval *tv , const struct timezone *tz);
extern	struct hostent  *gethostbyname(const char *hostname);
int		realtime = 0;

extern int network_is_ok;
//int need_update_time =0;
extern sdk_Network_Info g_network_info;


static int update_time_success = 0;

unsigned int write_rtc(struct rtc_time *rtctime);


#define NTP_VERSION 		0xe3
#define NTP_DEFAULT_PORT	"123"
#define SEC_IN_YEAR 		31556926
#define UNIX_OFFSET 		2208988800UL
#define VN_BITMASK(byte) 	((byte & 0x3f) >> 3)
#define LI_BITMASK(byte) 	(byte >> 6)
#define MODE_BITMASK(byte) 	(byte & 0x7)
#define ENDIAN_SWAP32(data)  	((data >> 24) | /* right shift 3 bytes */ \				
	((data & 0x00ff0000) >> 8) | /* right shift 1 byte */ \			        
	((data & 0x0000ff00) << 8) | /* left shift 1 byte */ \				
	((data & 0x000000ff) << 24)) /* left shift 3 bytes */
	
#define MAX_RETRY_COUNT		5

struct ntpPacket 
{	
	uint8_t flags;	
	uint8_t stratum;	
	uint8_t poll;	
	uint8_t precision;	
	uint32_t root_delay;	
	uint32_t root_dispersion;	
	uint8_t referenceID[4];	
	uint32_t ref_ts_sec;	
	uint32_t ref_ts_frac;	
	uint32_t origin_ts_sec;	
	uint32_t origin_ts_frac;	
	uint32_t recv_ts_sec;	
	uint32_t recv_ts_frac;	
	uint32_t trans_ts_sec;	
	uint32_t trans_ts_frac;
} __attribute__((__packed__)); /* this is not strictly necessary, structure follows alignment rules */


int sync_time_with_server(const char *server_addr) {
//	char *server = server_addr;
	const char *port = NTP_DEFAULT_PORT;
	struct addrinfo hints, *res, *ap; /* address info structs */
	socklen_t addrlen = sizeof(struct sockaddr_storage);

	struct ntpPacket packet;
//	uint8_t *ptr = (uint8_t *)(&packet); /* to read raw bytes */

	int server_sock; /* send through this socket */
	int error; /* error checking */
//	int i;
	unsigned int recv_secs;

	time_t total_secs;
	struct tm *now;

	fd_set read_fds;
	struct timeval timeout;
	int results;
	char cmd[50];

	memset(&packet, 0, sizeof(struct ntpPacket));
	packet.flags = NTP_VERSION;

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_DGRAM;

	fprintf(stderr, "start sync time with server %s\n", server_addr);

	/* fill our address structs for ntp server */
	error = getaddrinfo(server_addr, port, &hints, &res);
	/* error checking */
	if (error != 0) {
		fprintf(stderr, "getaddrinfo() error: %s", gai_strerror(error));
		return -1;
	}

	/* loop through results */
	for (ap = res; ap != NULL; ap = ap->ai_next) {
		server_sock = socket(ap->ai_family, ap->ai_socktype, ap->ai_protocol);
		if (server_sock == -1)
			continue;
		break;
	}
	if (ap == NULL) {
		fprintf(stderr, "socket() error\n");
		return -1;
	}

//	fprintf(stderr, "%lu - %lu \n", sizeof(struct sockaddr), sizeof(struct sockaddr_in));
//	struct sockaddr_in *s = (struct sockaddr_in *)ap->ai_addr;
//	fprintf(stderr, " %d - 0x%x - 0x%x - %s\n", s->sin_family, ntohs(s->sin_port), ntohl(s->sin_addr.s_addr), s->sin_zero);

	error = sendto(server_sock, &packet, sizeof(struct ntpPacket), 0, ap->ai_addr, addrlen);
	if (error == -1) {
		fprintf(stderr, "sendto() error\n");
		return -1;
	}

	FD_ZERO(&read_fds);
	FD_SET(server_sock, &read_fds);

	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	results = select(server_sock + 1, &read_fds, NULL, NULL, &timeout);
	if (results < 0) {
		fprintf(stderr, "select error\n");
	}
	else if (results == 0) {
		fprintf(stderr, "select time out\n");
	}
	else {
		if (FD_ISSET(server_sock, &read_fds)) {
			error = recvfrom(server_sock, &packet, sizeof(struct ntpPacket), 0, ap->ai_addr, &addrlen);
			if (error == -1) {
				fprintf(stderr, "recvfrom() error\n");
				return -1;
			}

			freeaddrinfo(res); /* all done */

			/* print raw bytes */
//			for (i = 0; i < sizeof(struct ntpPacket); i++) {
//				if (i != 0 && i % 8 == 0)
//					printf("\n");
//				printf("0x%2x ", ptr[i]);
//			}
//			printf("\n");

			/* correct for right endianess */
//			packet.root_delay = ENDIAN_SWAP32(packet.root_delay);
//			packet.root_dispersion = ENDIAN_SWAP32(packet.root_dispersion);
//			packet.ref_ts_sec = ENDIAN_SWAP32(packet.ref_ts_sec);
//			packet.ref_ts_frac = ENDIAN_SWAP32(packet.ref_ts_frac);
//			packet.origin_ts_sec = ENDIAN_SWAP32(packet.origin_ts_sec);
//			packet.origin_ts_frac = ENDIAN_SWAP32(packet.origin_ts_frac);
			packet.recv_ts_sec = ENDIAN_SWAP32(packet.recv_ts_sec);
//			packet.recv_ts_frac = ENDIAN_SWAP32(packet.recv_ts_frac);
//			packet.trans_ts_sec = ENDIAN_SWAP32(packet.trans_ts_sec);
//			packet.trans_ts_frac = ENDIAN_SWAP32(packet.trans_ts_frac);

			/* print raw data */
//			printf("LI: %u\n", LI_BITMASK(packet.flags));
//			printf("VN: %u\n", VN_BITMASK(packet.flags));
//			printf("Mode: %u\n", MODE_BITMASK(packet.flags));
//			printf("stratum: %u\n", packet.stratum);
//			printf("poll: %u\n", packet.poll);
//			printf("precision: %u\n", packet.precision);
//			printf("root delay: %u\n", packet.root_delay);
//			printf("root dispersion: %u\n", packet.root_dispersion);
//			printf("reference ID: %u.", packet.referenceID[0]);
//			printf("%u.", packet.referenceID[1]);
//			printf("%u.", packet.referenceID[2]);
//			printf("%u\n", packet.referenceID[3]);
//			printf("reference timestamp: %u.", packet.ref_ts_sec);
//			printf("%u\n", packet.ref_ts_frac);
//			printf("origin timestamp: %u.", packet.origin_ts_sec);
//			printf("%u\n", packet.origin_ts_frac);
//			printf("receive timestamp: %u.", packet.recv_ts_sec);
//			printf("%u\n", packet.recv_ts_frac);
//			printf("transmit timestamp: %u.", packet.trans_ts_sec);
//			printf("%u\n", packet.trans_ts_frac);

			/* print date with receive timestamp */
			recv_secs = packet.recv_ts_sec - UNIX_OFFSET; /* convert to unix time */
			total_secs = recv_secs;
//			printf("Unix time: %u\n", (unsigned int)total_secs);
			now = localtime(&total_secs);
//			printf("%02d/%02d/%d %02d:%02d:%02d\n", now->tm_mday, now->tm_mon+1,
//						now->tm_year+1900, now->tm_hour, now->tm_min, now->tm_sec);

			sprintf(cmd, "date -s %04d%02d%02d%02d%02d.%02d", now->tm_year+1900, now->tm_mon+1,
					now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
			printf("set system time with comand \"%s\"\n", cmd);
			system(cmd);
		}
	}
	
	int updatedone = 0;
	struct  timeval    tv;
    struct  timezone   tz;
    gettimeofday(&tv,&tz);
	if(tv.tv_sec > 1483159995)
		updatedone = 1;

	return updatedone;
}

int sync_time() 
{
	//const char * server[5] = { "time.windows.com", "time-a.nist.gov",
	//		"time-b.nist.gov", "time-c.nist.gov", "time.nist.gov", };
	const char * server[MAX_RETRY_COUNT] = {"stdtime.gov.hk", "tw.pool.ntp.org", 
		"time.windows.com", "asia.pool.ntp.org", "time.nist.gov"};
	int i = 0;
	int result = 0;

	while (i < MAX_RETRY_COUNT) 
	{
		result = sync_time_with_server(server[i]);
		if (result > 0)
			break;
		i++;
	}

	return result;
}



int get_ntp_zone(void)
{
	return g_ntp_pra.TimeZone;
}
time_t get_nowtime()
{
	time_t timep;
	time(&timep);
	timep = timep+(g_ntp_pra.TimeZone-12)*3600;
	///printf("get time now..........TimeZone=%d\n",g_ntp_pra.TimeZone);
	return timep;
}
int save_realtimetofile(time_t times)
{
	char savecmd[100];
	sprintf(savecmd,"echo %d > /mnt/mtd/realtime_conf",(int)times);
	system(savecmd);
	return 0;
}

int SetSystemTime(char *dt)
{
	 struct rtc_time tm;
	 struct tm _tm;
	 struct timeval tv;
	 time_t timep;
	 //// int fd;
	 sscanf(dt, "%d-%d-%d %d:%d:%d", &tm.tm_year,
	        &tm.tm_mon, &tm.tm_mday,&tm.tm_hour,
	        &tm.tm_min, &tm.tm_sec);
	//Set SYS Time
	_tm.tm_sec = tm.tm_sec;
	_tm.tm_min = tm.tm_min;
	_tm.tm_hour = tm.tm_hour;
	_tm.tm_mday = tm.tm_mday;
	_tm.tm_mon = tm.tm_mon - 1;
	_tm.tm_year = tm.tm_year - 1900;

	timep = mktime(&_tm);
	tv.tv_sec = timep;
	tv.tv_usec = 0;
	
	printf("-------2-------------settimeofday time:%d\n",0);
 	if(settimeofday(&tv, (struct timezone *) 0) < 0)
 	{
		printf("SetSystemTime:: Set system datatime error!\n");
		return -1;
 	}
 	else
 	{
		///printf("Set system datatime successfully!\n");
		realtime = timep;
		save_realtimetofile(realtime);//save real time to file
		return 0;
 	}
}


char *yyyymmddhhmmss(char *str,unsigned int str_size)
{
	char buf[128];
	char str_mon[4],str_mday[3],str_year[5],str_hour[3],str_min[3],str_sec[3],str_day[3];
	struct tm tmx;
	int i;
	time_t timep;
		
	char str_mon_array[][4]={
		"Jan",
		"Feb",
		"Mar",
		"Apr",
		"May",
		"Jun",
		"Jul",
		"Aug",
		"Sep",
		"Oct",
	    "Nov",
	    "Dec"};


	memset(str,0x0,str_size);
	tmx.tm_mon=1;

	time(&timep);
	if(timep - realtime>180)
	{
		realtime = timep;
		save_realtimetofile(realtime);//save real time to file
	}
	timep = timep+(g_ntp_pra.TimeZone-12)*3600;
	strcpy(buf,asctime(gmtime(&timep)));

	sscanf(buf,"%3s %3s  %2s %2s:%2s:%2s %4s",(char *)&str_day,(char *)&str_mon,(char *)&str_mday,(char *)&str_hour,(char *)&str_min,(char *)&str_sec,(char *)&str_year);

	for(i=0;i<12;i++){
		if(!strcmp(str_mon_array[i],str_mon)){
			tmx.tm_mon=i;
			break;
		}
	}

	tmx.tm_mday=atoi(str_mday);
	tmx.tm_year=atoi(str_year);
	tmx.tm_hour=atoi(str_hour);
	tmx.tm_min=atoi(str_min);
	tmx.tm_sec=atoi(str_sec);

	g_sysSecond = tmx.tm_sec;
	g_sysMinute = tmx.tm_min;
	g_sysHour = tmx.tm_hour;
	g_sysDay = tmx.tm_mday;
	g_sysMonth = tmx.tm_mon+1;
	g_sysYear = tmx.tm_year;

	snprintf(str,str_size,"%04d-%02d-%02d %02d:%02d:%02d"
	,tmx.tm_year,tmx.tm_mon+1,tmx.tm_mday
	,tmx.tm_hour,tmx.tm_min,tmx.tm_sec	
	);

	return str;
}

//add by jy
char *mmddyyyymmss(char *str,unsigned int str_size)
{
	char buf[128];
	char str_mon[4],str_mday[3],str_year[5],str_hour[3],str_min[3],str_sec[3],str_day[3];
	struct tm tmx;
	int i;
	time_t timep;
		
	char str_mon_array[][4]={
		"Jan",
		"Feb",
		"Mar",
		"Apr",
		"May",
		"Jun",
		"Jul",
		"Aug",
		"Sep",
		"Oct",
	    "Nov",
	    "Dec"};


	memset(str,0x0,str_size);
	tmx.tm_mon=1;

	time(&timep);
	if(timep - realtime>180)
	{
		realtime = timep;
		save_realtimetofile(realtime);//save real time to file
	}
	timep = timep+(g_ntp_pra.TimeZone-12)*3600;
	strcpy(buf,asctime(gmtime(&timep)));

	//printf("buf=%s\n",buf);//Sun Jan  4 02:16:10 1970

	sscanf(buf,"%3s %3s  %2s %2s:%2s:%2s %4s",(char *)&str_day,(char *)&str_mon,(char *)&str_mday,(char *)&str_hour,(char *)&str_min,(char *)&str_sec,(char *)&str_year);

	/*
		fprintf(stdout,"buf=%s\n",buf);
		fprintf(stdout,"str_mon=%s\n",str_mon);
		fprintf(stdout,"str_mday=%s\n",str_mday);
		fprintf(stdout,"str_year=%s\n",str_year);
		fprintf(stdout,"str_hour=%s\n",str_hour);
		fprintf(stdout,"str_min=%s\n",str_min);
		fprintf(stdout,"str_sec=%s\n",str_sec);
	*/

	for(i=0;i<12;i++){
		if(!strcmp(str_mon_array[i],str_mon)){
			tmx.tm_mon=i;
			break;
		}
	}

	tmx.tm_mday=atoi(str_mday);
	tmx.tm_year=atoi(str_year);
	tmx.tm_hour=atoi(str_hour);
	tmx.tm_min=atoi(str_min);
	tmx.tm_sec=atoi(str_sec);

	g_sysSecond = tmx.tm_sec;
	g_sysMinute = tmx.tm_min;
	g_sysHour = tmx.tm_hour;
	g_sysDay = tmx.tm_mday;
	g_sysMonth = tmx.tm_mon+1;
	g_sysYear = tmx.tm_year;

	snprintf(str,str_size,"%02d-%02d-%04d %02d:%02d:%02d"
	,tmx.tm_mon+1,tmx.tm_mday,tmx.tm_year
	,tmx.tm_hour,tmx.tm_min,tmx.tm_sec	
	);

	return str;
}

char *ddmmyyyymmss(char *str,unsigned int str_size)
{
	char buf[128];
	char str_mon[4],str_mday[3],str_year[5],str_hour[3],str_min[3],str_sec[3],str_day[3];
	struct tm tmx;
	int i;
	time_t timep;
		
	char str_mon_array[][4]={
		"Jan",
		"Feb",
		"Mar",
		"Apr",
		"May",
		"Jun",
		"Jul",
		"Aug",
		"Sep",
		"Oct",
	    "Nov",
	    "Dec"};


	memset(str,0x0,str_size);
	tmx.tm_mon=1;

	time(&timep);
	if(timep - realtime>180)
	{
		realtime = timep;
		save_realtimetofile(realtime);//save real time to file
	}
	timep = timep+(g_ntp_pra.TimeZone-12)*3600;
	strcpy(buf,asctime(gmtime(&timep)));

	//printf("buf=%s\n",buf);//Sun Jan  4 02:16:10 1970

	sscanf(buf,"%3s %3s  %2s %2s:%2s:%2s %4s",(char *)&str_day,(char *)&str_mon,(char *)&str_mday,(char *)&str_hour,(char *)&str_min,(char *)&str_sec,(char *)&str_year);

	/*
		fprintf(stdout,"buf=%s\n",buf);
		fprintf(stdout,"str_mon=%s\n",str_mon);
		fprintf(stdout,"str_mday=%s\n",str_mday);
		fprintf(stdout,"str_year=%s\n",str_year);
		fprintf(stdout,"str_hour=%s\n",str_hour);
		fprintf(stdout,"str_min=%s\n",str_min);
		fprintf(stdout,"str_sec=%s\n",str_sec);
	*/

	for(i=0;i<12;i++){
		if(!strcmp(str_mon_array[i],str_mon)){
			tmx.tm_mon=i;
			break;
		}
	}

	tmx.tm_mday=atoi(str_mday);
	tmx.tm_year=atoi(str_year);
	tmx.tm_hour=atoi(str_hour);
	tmx.tm_min=atoi(str_min);
	tmx.tm_sec=atoi(str_sec);

	g_sysSecond = tmx.tm_sec;
	g_sysMinute = tmx.tm_min;
	g_sysHour = tmx.tm_hour;
	g_sysDay = tmx.tm_mday;
	g_sysMonth = tmx.tm_mon+1;
	g_sysYear = tmx.tm_year;

	snprintf(str,str_size,"%02d-%02d-%04d %02d:%02d:%02d"
	,tmx.tm_mday,tmx.tm_mon+1,tmx.tm_year
	,tmx.tm_hour,tmx.tm_min,tmx.tm_sec	
	);

	return str;
}

//end


int InitSysTime()
{
	system("hwclock -s");
	return 1;
}

int GetNTPConfValues(ntp_conf *CONF_NTP)
{
	return 1;
}

 int SetNTPConfValues(ntp_conf CONF_NTP)
 {
	FILE *temp_fp;
	char renamecmd[128];
	

	if (!(temp_fp=fopen("temp_ntp","w"))) // open file pointer
	{ 
		printf("temp_ntp open failed...\n");
		return 0;
	}
	
	//printf("start write cont to file...\n");
	fprintf(temp_fp, "Server %s\n", CONF_NTP.Server);
	fprintf(temp_fp, "mod %d\n", CONF_NTP.mod);
	fprintf(temp_fp, "TimeZone %d\n", CONF_NTP.TimeZone);
	///printf("after write cont to file !\n");
	fflush(temp_fp);
	fclose(temp_fp);
	sprintf(renamecmd,"cp temp_ntp %s",NTP_CONFIG_PATH);
	system(renamecmd);
	
	return 1;
}


// Function: ntpset
int ntpset(char *postvars)
{		
	if(postvars!=NULL) 
	{
		ntp_conf *CONF_NTP_ = (ntp_conf *)	postvars;
		if (CONF_NTP_->mod == 1)
		{
				int i = 0;
				i = ntpclient("0.asia.pool.ntp.org"/*"time.windows.com"*/,123,0);
				printf("--------------------------------------------------i(%d)\n", i);
		        if(0 == i)
		        {
					return 0;

				}
				else
				{
					return 1;
				}
		}
		else if (CONF_NTP_->mod == 2 || CONF_NTP_->mod == 3)
		{
			char time_[100];
			sprintf(time_,"%d-%d-%d  %d:%d:%d",CONF_NTP_->time.year,
			        CONF_NTP_->time.month, CONF_NTP_->time.date,CONF_NTP_->time.hour,
			        CONF_NTP_->time.minute,CONF_NTP_->time.second);
			
			printf("::befire set time :%s\n",time_);
			if (SetSystemTime(time_) < 0)
				return -1;
			
			printf("::After set time:%d-%d-%d  %d:%d:%d\n",CONF_NTP_->time.year,
			CONF_NTP_->time.month, CONF_NTP_->time.date,CONF_NTP_->time.hour,
			CONF_NTP_->time.minute,CONF_NTP_->time.second);
			update_time_success = 1;
		}
		else
		{
			//printf("Please select a time synchronized method\n");
			return -1;
		}
		return 		0;
	} // end of "if we got post"

	return -1;
}


#define TIME_FILE_PATH "/mnt/mtd/run/timestamp.txt"
void * thread_write_time(void * arg)
{
	while(1)
	{
		if( 1 == update_time_success )
		{
			system_ex("date '+%s' > /mnt/mtd/run/timestamp.txt");
		}
		usleep(60 * 1000 * 1000); //每隔1分钟存一次
	}
	
}
void * thread_ntp_update (void * arg)
{
	int timeupdatedone = 0;
	int result = 0;
	
	while(1)
	{
		if(2 == g_network_info.ucNetworkState)
		{
			if(timeupdatedone == 0)
			{
				//system("/mnt/mtd/timeUpdate.sh &");
				//system("ntpdate stdtime.gov.hk tw.pool.ntp.org time.windows.com asia.pool.ntp.org time.nist.gov &");
				result = sync_time();
				
				struct  timeval    tv;
        		struct  timezone   tz;
        		gettimeofday(&tv,&tz);
				
				if((tv.tv_sec > 1483159995) && result > 0)
				{
					timeupdatedone = 1;
				}
			}
		}

		sleep(1); 
	}
#if 0
	int ret;
	unsigned int count = 0;
	if(NULL == strstr((char*)g_ntp_pra.Server,"."))
	{
		strcpy((char*)g_ntp_pra.Server,"0.asia.pool.ntp.org"/*"time.nuri.net"*/);
	}
	printf("thread_ntp_update\n");
	while(1)
	{	
		if(1 == g_network_info.ucNetworkState) //网络正常时才更新
		{
			if(1 == g_ntp_pra.mod ) 
			{  //ntpset返回0 表示成功
				if (0 == (ret = ntpset((char *)&g_ntp_pra))) //更新成功
				{
					pthread_mutex_lock(&g_ntp_pra.lock);
					g_ntp_pra.mod = 0;
					pthread_mutex_unlock(&g_ntp_pra.lock);
					update_time_success = 1;
				}
				else
				{
					update_time_success = 0;
					usleep(1 *1000 * 1000); //休息10秒 继续更新
				}
			}
			else if(0 == g_ntp_pra.mod ) //已经更新成功了 
			{
				usleep(1 * 1000 * 1000);
				count++ ;
				if(count > 12*60*60 ) //已经更新成功了 12个小时更新一次用来校验
				{
					pthread_mutex_lock(&g_ntp_pra.lock);
					g_ntp_pra.mod = 1;
					pthread_mutex_unlock(&g_ntp_pra.lock);
					count = 0;
				}
				
			}
		}
		else if(0 == g_network_info.ucNetworkState)
		{
			usleep(1 * 1000 * 1000);
		}
	}
#endif
}
int ntp_update_thread(void)
{
	int ret = -1,i = 0;
	pthread_t thread_id;
	pthread_t thread_id_write_time;
	sdk_date_t sys_time;
	memset(&sys_time,0,sizeof(sdk_date_t));
	adapt_param_get_systime(&sys_time);
	printf("==================ntp_update_thread time_zone=%d======================\n",sys_time.tzone);

	g_ntp_pra.TimeZone = sys_time.tzone;
	g_ntp_pra.mod = 1;
	g_ntp_pra.is_ntp_or_sys = 1;
	ret=pthread_create(&thread_id, NULL,(void *) thread_ntp_update,NULL);
	if (ret < 0)
	{
		printf("ntp_update_thread\n");
		return -1;
	}
	pthread_detach(thread_id);
	
	//创建一个用于写时间戳到文件的线程
	//ret=pthread_create(&thread_id_write_time, NULL,(void *) thread_write_time,NULL);
    //pthread_detach(thread_id_write_time);
	return 0;
}


#if 0
/************************************
以下代码为RTC 所准备
设置时tm_year-1900 tm_mon-1
获取时tm_year+1900 tm_mon+1

************************************/
#define RTC_NODE "/dev/rtc0"
unsigned int read_rtc(struct rtc_time *rtctime)
{
int ret = -1;
int fd = -1;
	fd = open(RTC_NODE,0);
	if(fd > 0 )	
	{	
		ret = ioctl(fd,RTC_RD_TIME,rtctime);	
		if(-1 == ret)	
		{		
			printf("ioctl RD fail!\n");
		}		
		close(fd);	
		/*printf("read rtc time %04d-%02d-%02d %02d:%02d:%02d\n",	
		rtctime->tm_year,rtctime->tm_mon, rtctime->tm_mday,	
		rtctime->tm_hour,rtctime->tm_min,rtctime->tm_sec);	*/
		return 0;	
	}	
return 1;
}
unsigned int write_rtc(struct rtc_time *rtctime)
{
int ret = -1;
int fd = -1;
	fd = open(RTC_NODE,0);	
	if(fd > 0 )
	{   
		ret = ioctl(fd,RTC_SET_TIME,rtctime);       
		if(-1 == ret)     
		{         
			printf("ioctl SET fail!\n");       
		}       
		close(fd);      
		/*printf("set time %04d-%02d-%02d %02d:%02d:%02d\n",  
			rtctime->tm_year,rtctime->tm_mon, rtctime->tm_mday,   
			rtctime->tm_hour,rtctime->tm_min,rtctime->tm_sec);	*/
		return 0;
	}	
	return -1;
}
int power_read_rtc()
{
	struct rtc_time time;
	struct tm _tm;
	time_t timep;
	struct timeval tv; 
	memset(&time,0,sizeof(time));
	read_rtc(&time);
	printf("power read time %04d-%02d-%02d %02d:%02d:%02d\n",     
	time.tm_year+1900,time.tm_mon+1,time.tm_mday,      
	time.tm_hour,time.tm_min,time.tm_sec);

	_tm.tm_sec 	= time.tm_sec;
	_tm.tm_min 	= time.tm_min;
	_tm.tm_hour = time.tm_hour;
	_tm.tm_mday = time.tm_mday;
	_tm.tm_mon 	= time.tm_mon;
	_tm.tm_year = time.tm_year;
	timep = mktime(&_tm);
	printf("form RTC read time=%ld\n",timep);
	 //1420041600 北京时间2015/1/1/00:00:00
	 if(timep > 1420041600)
	 {
		tv.tv_sec = timep;
		tv.tv_usec = 0;
		if(settimeofday(&tv, (struct timezone *) 0) < 0)
		{
			printf("SetSystemTime:: Set system datatime error!\n");
			need_update_time = 1;
			//return -1;
		}
		else
		{
			need_update_time = 0;
		}
		
	 }
	 else
	 {
		need_update_time = 1;
	 }
	return 0;
}


//test rtc
#if 0
int main()
{
struct rtc_time time;
memset(&time,0,sizeof(time));
time.tm_year=2016 - 1900;
time.tm_mon=4 - 1;
time.tm_mday=4;
time.tm_hour=4;
time.tm_min=4;
time.tm_sec=4;
write_rtc(&time);
sleep(2);
memset(&time,0,sizeof(time));
read_rtc(&time);
printf("read time %04d-%02d-%02d %02d:%02d:%02d\n",     
	time.tm_year+1900,time.tm_mon+1,time.tm_mday,      
	time.tm_hour,time.tm_min,time.tm_sec);}
#endif

#endif

int power_read_time_form_file()
{
	unsigned long timestamp = 0;
	unsigned char timestamp_arr[32];
	FILE *fp = NULL;
	struct timeval tv; 
	memset(timestamp_arr,0,sizeof(timestamp_arr));
	fp = fopen(TIME_FILE_PATH,"r");
	if(NULL != fp)
	{
		fread(timestamp_arr,1,sizeof(timestamp_arr),fp);
		timestamp = atol(timestamp_arr);
		printf("read timestamp=%ld\n",timestamp);
	}
	fclose(fp);
	 //1420041600 北京时间2015/1/1/00:00:00
	 if(timestamp > 1420041600)
	 {
		tv.tv_sec = timestamp;
		tv.tv_usec = 0;
		if(settimeofday(&tv, (struct timezone *) 0) < 0)
		{
			printf("SetSystemTime:: Set system datatime error!\n");
			return -1;
		}
		
	 }
	return 0;
}

