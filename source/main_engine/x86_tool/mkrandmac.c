#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <fcntl.h>  
#include <sys/types.h>  
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>

/* 
 *  * make a true rand 
 *   */  
static inline unsigned int _new_rand ()  
{  
#if 1
    int fd;  
    unsigned int n = 0;  

    fd = open ("/dev/urandom", O_RDONLY);  
    if (fd > 0){  
        read (fd, &n, sizeof (n));  
    }  
    close (fd); 
    return n; 
#endif
}


/* 
 *  * change rand number to char 
 *   */  
static unsigned char randomchar(void)  
{  
   // unsigned int rand;  
    unsigned char a;  

    a =(unsigned char)rand();//new_rand();  
    if (a < 'A')  
        a = a % 10 + 48;  
    else if (a < 'F')  
        a = a % 6 + 65;  
    else if (a < 'a' || a > 'f')  
        a = a % 6 + 97;  

    return a;  
}  
/* 
 *  * product one mac addr 
 *   */  
static void product_one_mac(void)  
{  
    char mac[18]={'0','0',};  
    int i;  
    char shellbuf[128];

    /*set mac addr */  
    for (i=3; i<17; i++){  
        usleep(10);  
        mac[i] = randomchar();  
    }  
    mac[2] = mac[5] = mac[8] = mac[11] = mac[14] = ':';  
    mac[17] = '\0';  

    sprintf(shellbuf, "ifconfig eth0 hw ether %s", mac);
    printf("%s\n", shellbuf);  
    system(shellbuf);
}

int main(int argc, char *argv[])  
{  
    int i;  
    struct timespec tp;

   // srand(time(NULL));
    clock_gettime(CLOCK_MONOTONIC, &tp);
    srand(tp.tv_nsec+tp.tv_sec + _new_rand()+ time(NULL));
    if(argc == 1){  
        product_one_mac();  
        return 0;  
    }  
    for(i = 0; i < atoi(argv[1]); i++)  
        product_one_mac();  

    return 0;  
}  
