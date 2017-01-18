#ifndef NTPCLIENT_H_
#define NTPCLIENT_H_

int g_sysYear;
int g_sysMonth;
int g_sysDay;
int g_sysHour;
int g_sysMinute;
int g_sysSecond;
int g_sysMilliseconds;


#define NTP_STATUS_NORUN    0
#define NTP_STATUS_CONNECT  1
#define NTP_STATUS_FAILED   2
#define NTP_STATUS_SUCCESS  3


typedef struct _set_time_
{
	unsigned int year;
	unsigned int month;
	unsigned int date;
	
	unsigned int hour;
	unsigned int minute;
	unsigned int second;

}_set_time;

typedef struct _ntp_conf
{
	unsigned int  mod;               //1,ntp 2 pc or manul
	unsigned char Server[32];        // NTP Server:
	_set_time     time;
	unsigned char TimeZone;           // TimeZone:  0~25:(GMT-12)~GMT~(GMT+12)
	int           is_ntp_or_sys;      //用来判断是系统时间还是ntp时间
	pthread_mutex_t *lock;
	
}ntp_conf;


extern ntp_conf g_ntp_pra;


int ntpclient(const char *hostname, int ntp_server_port, int time_zone);

int ntpset(char *postvars);
int ntpsetalso(char *postvars);



#endif /* NTPCLIENT_H_ */



