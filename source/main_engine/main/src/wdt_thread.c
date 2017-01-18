/* standard C headers */
#include <stdlib.h>
#include <stdio.h>


/* POSIX's headers */
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <pthread.h>


/* for Linux watchdog ioctl */
#include <linux/watchdog.h>

#define WDT_DEVICE_FILE         "/dev/watchdog"
#define TIMEOUT	20

static int g_watchdog_fd = -1;
void * thread_wdt(void *arg)
{
    int fd = -1;
    int timeout = 0;

    g_watchdog_fd = open(WDT_DEVICE_FILE, O_RDWR);
    if (!g_watchdog_fd)
    {
        printf("Error in file open WDT device file(%s)...\n", WDT_DEVICE_FILE);

        return NULL;
    }

    ioctl(g_watchdog_fd, WDIOC_GETTIMEOUT, &timeout);
    printf("default timeout %d sec.\n", timeout);

    printf("We reset timeout as %d sec.\n", TIMEOUT);
    timeout = TIMEOUT;
    ioctl(g_watchdog_fd, WDIOC_SETTIMEOUT, &timeout);
    ioctl(g_watchdog_fd, WDIOC_GETTIMEOUT, &timeout);
    if (timeout != TIMEOUT)
    {
        printf("WDT timeout reset error.\n");

        return NULL;
    }

    //printf("Please notice if system reboot after %d sec.\n", TIMEOUT);

    while (1)
    {
        ioctl(g_watchdog_fd, WDIOC_KEEPALIVE, 0);
        sleep(5);
    }

    return NULL;
}
int watchdog_disable_dog()
{
    int ret;
    int val = WDIOS_DISABLECARD;
    ret = ioctl(g_watchdog_fd, WDIOC_SETOPTIONS, &val);
    if(ret < 0)
    {
        printf("WDIOF_SETTIMEOUT error %s\n", strerror(errno));
        return ret;
    }
    return 0;
}
int watchdog_server_start()
{
    pthread_t watchdog_id = 0;
    pthread_create(&watchdog_id, 0, thread_wdt, NULL);

}
