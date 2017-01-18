#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>

#include "flash_api.h"

struct mtd_info_user {
	u_char type;
	u_int32_t flags;
	u_int32_t size;
	u_int32_t erasesize;
	u_int32_t oobblock;
	u_int32_t oobsize;
	u_int32_t ecctype;
	u_int32_t eccsize;
};

struct erase_info_user {
	u_int32_t start;
	u_int32_t length;
};

/************************************
* dev:    size   erasesize  name
* mtd0: 00070000 00010000 "UBOOT"
* mtd1: 00200000 00010000 "LINUX"
* mtd2: 00480000 00010000 "FS"
* mtd3: 00100000 00010000 "CONFIG"
* mtd4: 00800000 00010000 "APP"
* mtd5: 01000000 00010000 "ALL"
************************************/
#define NSBOOT_OFFSET 	 0	
#define NSBOOT_SIZE   	(64 * 1024)
#define UBOOT_OFFSET 	(NSBOOT_SIZE)
#define UBOOT_SIZE   	(448 * 1024)
#define KERNEL_OFFSET	(NSBOOT_SIZE + UBOOT_SIZE)	
#define KERNEL_SIZE		(2048 * 1024)
#define ROOTFS_OFFSET	(KERNEL_OFFSET + KERNEL_SIZE)
#define ROOTFS_SIZE		(4608 * 1024)
#define CONFIG_OFFSET	(ROOTFS_OFFSET + ROOTFS_SIZE)
#define CONFIG_SIZE		(1024 * 1024)
#define APP_OFFSET		(CONFIG_OFFSET + CONFIG_SIZE)
#define APP_SIZE		(8192 * 1024)

//#define MEMGETINFO	_IOR('M', 1, struct mtd_info_user)
//#define MEMERASE	_IOW('M', 2, struct erase_info_user)

#define MEMGETINFO      _IOR('M', 1, struct mtd_info_user)
#define MEMERASE        _IOW('M', 2, struct erase_info_user)


int mtd_open(const char *name, int flags)
{
	FILE *fp;
	char dev[80];
	int i, ret;

	if ((fp = fopen("/proc/mtd", "r"))) 
	{
		while (fgets(dev, sizeof(dev), fp)) 
		{
			if (sscanf(dev, "mtd%d:", &i) && strstr(dev, name)) 
			{
				snprintf(dev, sizeof(dev), "/dev/mtd/%d", i);
				if ((ret = open(dev, flags)) < 0) 
				{
					snprintf(dev, sizeof(dev), "/dev/mtd%d", i);
					ret = open(dev, flags);
				}
				fclose(fp);
				return ret;
			}
		}
		fclose(fp);
	}
	return -1;
}


int flash_read_byname(char *part_name, char *buf, off_t from, int len)
{
	int fd, ret;
	struct mtd_info_user info;

	fd = mtd_open(part_name, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Could not open mtd device for read, %s\n", part_name);
		return -1;
	}

	if (ioctl(fd, MEMGETINFO, &info)) {
		fprintf(stderr, "Could not get mtd device info, %s\n", part_name);
		close(fd);
		return -1;
	}
	
	if (len > info.size) {
		fprintf(stderr, "%s: Too many bytes - %d > %d bytes\n", part_name, len, info.erasesize);
		close(fd);
		return -1;
	}

	close(fd);
	fd = mtd_open(part_name, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Could not open mtd block device, %s\n", part_name);
		return -1;
	}

	lseek(fd, from, SEEK_SET);
	ret = read(fd, buf, len);
	if (ret == -1) {
		fprintf(stderr, "Reading from mtd failed, %s\n", part_name);
		close(fd);
		return -1;
	}

	close(fd);
	return ret;
}

//#define min(x,y) ({ typeof(x) _x = (x); typeof(y) _y = (y); (void) (&_x == &_y); _x < _y ? _x : _y; })

int flash_write_byname(char *part_name, char *buf, int len)
{
	int fd, ret = 0, wlen;
	char *bak = NULL;
	char *pbuf= NULL;
	struct mtd_info_user info;
	struct erase_info_user ei;

	fd = mtd_open(part_name, O_RDWR | O_SYNC);
	if (fd < 0) {
		fprintf(stderr, "Could not open mtd device for write, %s\n", part_name);
		return -1;
	}

	if (ioctl(fd, MEMGETINFO, &info) != 0) {
		fprintf(stderr, "Could not get mtd device info, %s\n", part_name);
		close(fd);
		return -1;
	}
	if (len > info.size) {
		fprintf(stderr, "%s, Too many bytes: %d > %d bytes\n", part_name, len, info.erasesize);
		close(fd);
		return -1;
	}

	ei.start = 0;
	ei.length = info.size;
#if 0
	if(strcmp(part_name, "LINUX")==0)
	{
		printf("upgrade linux kernel....1, offset=%d, erase size=%d\n", to, ei.length);
		printf("upgrade linux kernel....2, offset=%d, erase size=%d\n", KERNEL_OFFSET, KERNEL_SIZE);
		if(to==KERNEL_OFFSET)
		{
			fprintf(stderr, "upgrade linux kernel...., offset=%d, erase size=%d\n", to, ei.length);
			ei.length = KERNEL_SIZE;
		}
	}
	if(strcmp(part_name, "FS")==0)
	{
		printf("upgrade root filesystem....1, offset=%d, erase size=%d\n", to, ei.length);
		printf("upgrade root filesystem....2, offset=%d, erase size=%d\n", ROOTFS_OFFSET, ROOTFS_SIZE);
		if(to==ROOTFS_OFFSET)
		{
			fprintf(stderr, "upgrade root filesystem...., offset=%d, erase size=%d\n", to, ei.length);
			ei.length = ROOTFS_SIZE;
		}
	}
	if(strcmp(part_name, "APP")==0)
	{
		printf("upgrade app firmware....1, offset=%d, erase size=%d\n", to, ei.length);
		printf("upgrade app firmware....2, offset=%d, erase size=%d\n", APP_OFFSET, APP_SIZE);
		if(to==APP_OFFSET)
		{
			fprintf(stderr, "upgrade app firmware...., offset=%d, erase size=%d\n", to, ei.length);
			ei.length = APP_SIZE;
		}
	}

	if (len > ei.length) {
		fprintf(stderr, "%s, Too many bytes: %d > %d bytes\n", part_name, len, ei.length);
		close(fd);
		return -1;
	}
#endif

	

	//fprintf(stderr, "len=%x, erasesize=%x\n", len, ei.length);
	if((ret = ioctl(fd, MEMERASE, &ei)) < 0) {
		fprintf(stderr, "Erase block failed.\n");
		return -1;
	}

	//fprintf(stderr, "start write.....\n");
	pbuf = buf;
	// write device
	while(len>0) {
		wlen = (len > info.erasesize ? info.erasesize:len);
		ret = write(fd, pbuf, wlen);
		
		if(ret == -1)
		{
			if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINPROGRESS)
			{
				perror("flash_write_byname");
				continue;
			}
		}
		len -= wlen;
		pbuf += wlen;
	}
	//fprintf(stderr, "write finished!!!\n");
	
	close(fd);

	sync();
	return ret;
}

int flash_write_byname_fullblock(char *part_name, char *buf, off_t to, int len)
{
	int fd, ret = 0, wlen;
	char *bak = NULL;
	char *pbuf= NULL;
	struct mtd_info_user info;
	struct erase_info_user ei;

	fd = mtd_open(part_name, O_RDWR | O_SYNC);
	if (fd < 0) {
		fprintf(stderr, "Could not open mtd device for write, %s\n", part_name);
		return -1;
	}

	if (ioctl(fd, MEMGETINFO, &info)) {
		fprintf(stderr, "Could not get mtd device info, %s\n", part_name);
		close(fd);
		return -1;
	}

	ei.start = to;
	ei.length = info.size;
	lseek(fd, to, SEEK_SET);
	
	//for burning don't care fore the len, just use the block size
	if (len > ei.length) {
		fprintf(stderr, "!!!!!!! Warnning: %s, Too many bytes: %d > %d bytes, but will still continue!!!!\n", part_name, len, ei.length);
		len = ei.length;
		//close(fd);
		//return -1;
	}
	
	fprintf(stderr, "len=%x, erasesize=%x\n", len, ei.length);
	if((ret = ioctl(fd, MEMERASE, &ei)) < 0) {
		fprintf(stderr, "Erase block failed.\n");
		return -1;
	}
	
	fprintf(stderr, "start write.....\n");
	pbuf = buf;
	// write device
	while(len>0) {
		wlen = (len > info.erasesize ? info.erasesize:len);
		ret = write(fd, pbuf, wlen);
		
		if(ret == -1)
		{
			if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINPROGRESS)
			{
				perror("flash_write_byname");
				continue;
			}
		}
		len -= wlen;
		pbuf += wlen;
	}

	fprintf(stderr, "write finished!!!\n");
	close(fd);

	sync();
	return ret;
}


#if 0
int flash_read(char *buf, off_t from, int len)
{
	return flash_read_byname("config", buf, from, len);
}

int flash_write(char *buf, off_t to, int len)
{
	return flash_write_byname("config", buf, to, len);
}

// read oem info from flash
int flash_read_oem(char *buf, off_t from, int len)
{
	if(flash_read_byname("oem", buf, from, len)<0)
		return -1;
	unsigned long *pCRC = (unsigned long *) buf;
	return (crc32(0, (unsigned char *)(buf+sizeof(unsigned long)), len-sizeof(unsigned long)) == *pCRC);
}

int flash_write_oem(char *buf, off_t to, int len)
{
	unsigned long *pCRC = (unsigned long *) buf;
	*pCRC = (unsigned long)crc32(0, (unsigned char *)(buf+sizeof(unsigned long)), len-sizeof(unsigned long));
	return flash_write_byname("oem", buf, to, len);
}


int flash_read_image_header(char *buf, int len, int kernelIdx)
{
	if(buf==NULL ) return -1;
	if(kernelIdx == 2)
	{
		return flash_read_byname("Kernel0", buf, 0, len);
	}
	else
	{
		return flash_read_byname("Kernel", buf, 0, len);
	}
}

int flash_read_bootloader_header(char *buf, int len)
{
	if(buf==NULL ) return -1;
	return flash_read_byname("Bootloader", buf, 0x30000 - len, len);
}

int flash_write_bootloader_header(char *buf, int len)
{
	if(buf==NULL ) return -1;
	return flash_write_byname("Bootloader", buf, 0x30000 - len, len);
}

int flash_write_bootloader(char *buf, int len)
{
	if(buf==NULL || len <=0 ) return -1;
	return flash_write_byname("Bootloader", buf, 0, len);
}


int flash_write_other(char *buf, int len)
{
	if(buf==NULL || len <=0 ) return -1;
	return flash_write_byname("Other", buf, 0, len);
}
#endif

