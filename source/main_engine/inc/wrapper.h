/*
 * =====================================================================================
 *
 *       Filename:  wrapper.h
 *
 *    Description:  wrapper
 *
 *        Version:  
 *        Created:  
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  
 *        Company:  
 *         Modify:  
 *
 * =====================================================================================
 */
#ifndef __WRAPPER_H__
#define __WRAPPER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <utime.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <ctype.h>
#include <math.h>
#include <inttypes.h>
#include <sched.h>
#include <sys/timeb.h>
#include <netdb.h>
#include <signal.h>
#include <stdarg.h>
#include <termios.h>
#include <sys/syscall.h>


/* ------------ time ----------------- */
time_t os_make_time(int year,int month,int day
        ,int hour,int minute,int second);
void os_local_time(time_t utc
        ,unsigned int *pyear,unsigned int *pmonth,unsigned int *pday
        ,unsigned int *phour,unsigned int *pminute,unsigned int *psecond
        ,unsigned int *pweekday);

char *os_local_time_str(time_t time, char *time_str);

int os_get_days(int year, int month);
int os_set_time(time_t t);
int os_get_rand(int min, int max);
int os_get_time(unsigned int *pyear, unsigned int *pmonth, unsigned int *pday
        		,unsigned int *phour, unsigned int *pminute, unsigned int *psecond);


/* ------------ io && string ----------------- */
ssize_t os_writen(int fd, const void *vptr, size_t n);
ssize_t os_readn(int fd, void *vptr, size_t n);
ssize_t os_readline(int fd, void *vptr, size_t maxlen);

char *os_strsep(char **stringp, const char *delim);

int os_str2number(char *str , uint32_t* pulValue);


/* ------------ mem ----------------- */
void *os_calloc(size_t nmemb, size_t size);
void *os_malloc(size_t size);
void *os_realloc(void *ptr, size_t size);
void os_free(void *ptr);

/* ------------ other ----------------- */
unsigned int os_crc32(const void *buf, unsigned int size);
pid_t os_gettid(void);

/* ------------ com port ----------------- */
int os_set_com_param(int fd, int baut, int parity, int stop, int databit);


/* ------------ net ----------------- */
#define OS_ETH0  "eth0"
#define OS_ETH1  "eth1"
#define OS_PPP0  "ppp0"
#define OS_PPP1  "ppp1"
int os_get_if_index(const char *if_name, int *ifindex);
int os_get_mac_addr(const char *if_name, char *macaddr, size_t len);
int os_get_ip_addr(const char *if_name, char *ipaddr, size_t len);
int os_get_net_mask(const char *if_name, char *netmask, size_t len);
int os_get_net_addr(const char *if_name, char *netaddr, size_t len);
int os_get_broad_cast(const char *if_name, char *broadcast, size_t len);
int os_get_gateway(const char *if_name, char *gateway, size_t len);
int os_get_dns(char *firstdns, char *seconddns, size_t len);
int os_check_mac_addr(const char *macaddr);
int os_check_ip_addr(const char *ipaddr);
int os_set_mac_addr(const char *if_name, const char *macaddr);
int os_set_mac_addr_2(const char *if_name, const char *pmac);
int os_set_ip_addr(const char* if_name, const char *ipaddr);
int os_set_net_mask(const char *if_name, const char *netmask);
int os_set_net_addr(const char *netaddr);
int os_set_broad_cast(const char *if_name, const char *broadcast);
int os_del_gateway(const char *gateway);
int os_set_gateway(const char *if_name, const char *gateway);
int os_set_dns(const char *firstdns, const char *seconddns);
int os_set_auto_dns(void);
int os_get_link_status(const char *if_name);



/* ------------ file ----------------- */

int os_is_file_exist(const char *pathname);
int os_get_file_length(const char *pathname);
int os_get_file_Name(const char *pathname, char *file, size_t len);
int os_get_file_path(const char *pathname, char *path, size_t len);
time_t os_get_file_Atime(const char *pathname);
time_t os_get_file_Mtime(const char *pathname);
int os_is_file_read(const char *pathname);
int os_is_file_write(const char *pathname);
int os_is_file_exec(const char *pathname);
int os_is_link_file(const char *pathname);
int os_is_device_file(const char *pathname);
int os_is_regular_file(const char *pathname);
int os_copy_file(const char *oldpath, const char *newpath, int flag);
int os_rm_file(const char *pathname);
int os_rename_file(const char *oldpath, const char *newpath);
int os_set_file_length(const char *pathname, int newlen);
int os_set_file_mode(const char *pathname, mode_t mode);
int os_read_lock(int fd, off_t offset, int whence, off_t len);
int os_readW_lock(int fd, off_t offset, int whence, off_t len);
int os_write_lock(int fd, off_t offset, int whence, off_t len);
int os_writeW_lock(int fd, off_t offset, int whence, off_t len);
int os_unlock(int fd, off_t offset, int whence, off_t len);
off_t os_seek(int fd, off_t offset, int whence);
off_t os_seek2begin(int fd);
off_t os_seek2end(int fd);
int os_set_Utime(const char *pathname, const struct utimbuf *times);
int os_fsync(int fd);
int os_fflush(FILE *stream);
int os_is_dir_exist(const char *pathname);
int os_is_dir(const char *pathname);
int os_get_dir_name(const char *pathname, char *dirname, size_t len);
int os_get_absolute_dir(const char *pathname, char *dirname, size_t len);
int os_is_dir_empty(const char *pathname);
long long os_get_dir_size(const char *pathname);
int os_get_dir_fileCount(const char *pathname);
int os_mk_dir(const char *pathname, mode_t mode);
int os_set_dir_mode(const char *pathname, mode_t mode);
int os_copy_dir(const char *oldpath, const char *newpath, int flag, long sleeptime);
int os_rename_dir(const char *oldpath, const char *newpath);
int os_rm_dir(const char *pathname);
int os_get_delFileCount(void);
int os_clear_delFileCount(void);

int os_stack_set_fd(FILE *fd);
void os_stack_trace(void);

#endif  //__WRAPPER_H__

