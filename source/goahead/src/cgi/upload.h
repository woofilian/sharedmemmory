#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>
#include <errno.h>
#include <stdint.h>
#include <flash_api.h>

typedef enum
{
	IMAGE_TYPE_LINUX,
	IMAGE_TYPE_FS,
	IMAGE_TYPE_APP,
	IMAGE_TYPE_ALL
}IMAGE_TYPE;

#define REFRESH_TIMEOUT		"60000"		/* 60000 = 60 secs*/
#define RFC_ERROR "RFC1867 error"


#define PARTITION_LINUX  "LINUX"
#define PARTITION_FS 	 "FS"
#define PARTITION_APP 	 "APP"
#define PARTITION_ALL 	 "ALL"



#define LINUX_ASP 	"upload_kernel.asp"
#define FS_ASP 		"upload_rootfs.asp"
#define APP_ASP 	"upload_firmware.asp"
#define ALL_ASP 	"upload_all.asp"



#define IH_MAGIC    0x27051956
#define IH_NMLEN    32

//#define IH_NMLEN    (32-4-4)
typedef struct image_header {
    uint32_t    ih_magic;   /* off=0  Image Header Magic Number    */
    uint32_t    ih_hcrc;    /* off=4  Image Header CRC Checksum    */
    uint32_t    ih_time;    /* off=8  Image Creation Timestamp */
    uint32_t    ih_size;    /* off=12  Image Data Size      */
    uint32_t    ih_load;    /* off=16  Data  Load  Address      */
    uint32_t    ih_ep;      /* off=20  Entry Point Address      */
    uint32_t    ih_dcrc;    /* off=24  Image Data CRC Checksum  */
    uint8_t     ih_os;      /* off=28  Operating System     */
    uint8_t     ih_arch;    /* off=29  CPU architecture     */
    uint8_t     ih_type;    /* off=30  Image Type           */
    uint8_t     ih_comp;    /* off=31  Compression Type     */
    uint8_t     ih_name[IH_NMLEN];  /* off=32  Image Name       */
	//uint32_t    ih_ver;     /* off=56  upgrade version, avoid downgrade*/
	//uint32_t	ih_count;	/* off=60  Kernel Part Size		*/
} image_header_t;

#if 1
static int CRC_check(char *src, int len, char *name)
{
	int  data_len;
	char *data;
	unsigned char *ptr;
	unsigned long checksum;
	image_header_t *hdr;
	image_header_t tmpH;
	
	if ((unsigned)len < sizeof(image_header_t)) {
		fprintf (stderr, "Bad size: not a valid image\n");
		return -1;
	}

	/*
	 *  handle Header CRC32
	 */
	memcpy(&tmpH, src, sizeof(tmpH));
	hdr=&tmpH;
    if (ntohl(hdr->ih_magic) != IH_MAGIC) {
		fprintf (stderr, "Bad Magic Number: not a valid image\n");
		return -1;
	}

	if(strcmp(hdr->ih_name, name) != 0)
	{
		fprintf (stderr, "Bad image: not a valid image, need %s, but get %s\n", name, hdr->ih_name);
		return -1;
	}
	
    checksum = ntohl(hdr->ih_hcrc);
    hdr->ih_hcrc = htonl(0);	/* clear for re-calculation */
    if (crc32 (0, hdr, sizeof(image_header_t)) != checksum) {
		fprintf (stderr, "*** Bad image:  has bad header checksum!\n");
		return -1;
    }

	ptr = src;
	data = (char *)(ptr + sizeof(image_header_t));
    data_len  = ntohl(hdr->ih_size);
	if(data_len != len-sizeof(image_header_t))
	{
		fprintf (stderr, "Bad image: data corrupted, len(%d != %d)\n", data_len, len-sizeof(image_header_t));
		return -1;
	}

	//ptr = (unsigned char *) data;
	//fprintf (stderr, "%02x-%02x-%02x-%02x\n", ptr[0], ptr[1], ptr[2], ptr[3]);

    if (crc32 (0, data, data_len) != ntohl(hdr->ih_dcrc)) {
		fprintf (stderr, "Bad image: corrupted data!, data_len=%d, count crc=%x, keeped crc=%x\n", data_len, crc32(0, data, data_len), ntohl(hdr->ih_dcrc));
		return -1;
    }	

	
	return 0;
}
#endif

static void javascriptUpdate(int success, IMAGE_TYPE imageType)
{
    printf("<script language=\"JavaScript\" type=\"text/javascript\">");
    if(success){
        printf(" \
function refresh_all(){	\
  top.location.href = \"http://\"+location.hostname+\"/apcam/adm/%s\"; \
} \
function update(){ \
  self.setTimeout(\"refresh_all()\", %s);\
}", imageType==IMAGE_TYPE_LINUX?LINUX_ASP:imageType==IMAGE_TYPE_FS?FS_ASP:imageType==IMAGE_TYPE_APP?APP_ASP:ALL_ASP
, REFRESH_TIMEOUT);
    }else{
        printf("function update(){ parent.menu.setLockMenu(0);}");
    }
    printf("</script>");
}

static void webFoot(void)
{
    printf("</body></html>\n");
}

static int upGradeDo(char *fwname, IMAGE_TYPE imageType)
{
	char fwType[256];
	char fwUpName[256];
	int iret=0;
	unsigned char *buf=0;
	unsigned char *objbuf=0;
	unsigned int flen=0;
	struct stat statbuf;  
    stat(fwname, &statbuf);  
    int size=statbuf.st_size;
fprintf(stderr, " go in upGradeDo... 1, size=%d\n", size);	
	FILE* fd = fopen(fwname,"rb");
    if(fd && fwname)
    {
        fseek(fd,0,SEEK_END);
        flen = ftell(fd);
        fseek(fd,0,SEEK_SET);
fprintf(stderr, " go in upGradeDo... 2, flen=%d\n", flen); 
sleep(2);
        buf = (unsigned char *)malloc(flen);
		if( buf )
		    fread(buf,flen,1,fd);

		fclose(fd);
        objbuf = buf;

        unsigned char *pBegin, *pEnd;
	    unsigned char *boundary;
        unsigned int rlen=0, boundary_len;

        //get boundary length
    	pBegin = (unsigned char *)memmem(buf, flen, "\r\n", 2);
    	if(pBegin == NULL)
    	{
    		printf("Error: can not find boundary end!<br>");
			goto GIVEUP_UPGRADE_AND_QUIT;
    		//return -1;   	
    	}
    	
    	boundary_len = pBegin - buf;
    	//get upload file length
    	pBegin = (unsigned char *)memmem(pBegin, flen-(pBegin-buf), "\r\n\r\n", 4);
		if(pBegin == NULL)
    	{
    		printf("Error: can not find boundary end!<br>");
			goto GIVEUP_UPGRADE_AND_QUIT; 	
    	}
		
    	pBegin += 4;
    	pEnd   = (unsigned char *)memmem(pBegin, flen-(pBegin-buf), buf, boundary_len);
		if(pBegin == NULL)
    	{
    		printf("Error: can not find boundary end!<br>");
			goto GIVEUP_UPGRADE_AND_QUIT;
    	}
		
    	pEnd -= 2; //skip  \r\n
    	flen = pEnd-pBegin;
        objbuf = pBegin;
fprintf(stderr, " go in upGradeDo... 3, imageType=%d\n", imageType);

		iret = -1;
		if(imageType == IMAGE_TYPE_FS)
		{
			#if 0
			if(CRC_check(pBegin, pEnd-pBegin, "rootfs")<0)
			{
	    		printf("Error: CRC error!<br>");
				goto GIVEUP_UPGRADE_AND_QUIT;
	    	}
			#endif

			pBegin+=64;//skip the image header for fs
			fprintf(stderr, "filelen=%d, buf=%x, pBegin=%x, pEnd=%x, rlen=%d, boundary_len=%d, fwlen=%d\n", flen, buf, pBegin, pEnd, rlen, boundary_len, pEnd-pBegin);
			fprintf(stderr, "    RootFS start writing!!!!!!!!!!, len=%d\n", pEnd-pBegin);
			iret=flash_write_byname(PARTITION_FS, (char*)pBegin, pEnd-pBegin);
			if(iret >= 0)
			{
				fprintf(stderr, " FS  write finished!!! iret=%d\n", iret);
				goto FINISH_UPGRADE_AND_QUIT;
			}
		}
		else if(imageType == IMAGE_TYPE_LINUX)
		{
		#if 0
			if(CRC_check(pBegin, pEnd-pBegin, "Linux-3.10.0-InfoTMIC+")<0)
			{
	    		printf("Error: CRC error!<br>");
				goto GIVEUP_UPGRADE_AND_QUIT;
	    	}
	    #endif	
			fprintf(stderr, "filelen=%d, buf=%x, pBegin=%x, pEnd=%x, rlen=%d, boundary_len=%d, fwlen=%d\n", flen, buf, pBegin, pEnd, rlen, boundary_len, pEnd-pBegin);
			fprintf(stderr, "     Linux start writing!!!!!!!!!!, len=%d\n", pEnd-pBegin);
			fprintf(stderr, "     %02x-%02x-%02x-%02x\n", pBegin[0], pBegin[1], pBegin[2], pBegin[3]);
			iret=flash_write_byname(PARTITION_LINUX, (char*)pBegin, pEnd-pBegin);
			if(iret >= 0)
			{
				fprintf(stderr, " Linux  write finished!!! iret=%d\n", iret);
				goto FINISH_UPGRADE_AND_QUIT;
			}
		}
		else if(imageType == IMAGE_TYPE_APP)
		{
			fprintf(stderr, "filelen=%d, buf=%x, pBegin=%x, pEnd=%x, rlen=%d, boundary_len=%d, fwlen=%d\n", flen, buf, pBegin, pEnd, rlen, boundary_len, pEnd-pBegin);
			fprintf(stderr, "     App-FW start writing!!!!!!!!!!, len=%d\n", pEnd-pBegin);
			iret=flash_write_byname(PARTITION_APP, (char*)pBegin, pEnd-pBegin);
			if(iret >=0)
			{
				fprintf(stderr, " APP  write finished!!! iret=%d\n", iret);
				goto FINISH_UPGRADE_AND_QUIT;
			}
		}
		else if(imageType == IMAGE_TYPE_ALL)
        {
            fprintf(stderr, "filelen=%d, buf=%x, pBegin=%x, pEnd=%x, rlen=%d, boundary_len=%d, fwlen=%d\n", flen, buf, pBegin, pEnd, rlen, boundary_len, pEnd-pBegin);
            fprintf(stderr, "     Burnin-ALL writing!!!!!!!!!!, len=%d\n", pEnd-pBegin);
            //iret=flash_write_byname(PARTITION_ALL, (char*)pBegin, pEnd-pBegin);
            if(iret >= 0)
            {
                fprintf(stderr, " ALL  write finished!!! iret=%d\n", iret);
                goto FINISH_UPGRADE_AND_QUIT;
            }
        }
    }
    else
    {
    	printf("%x, errno=%d, %s<br>", fwname, errno, strerror(errno));
    }
	
GIVEUP_UPGRADE_AND_QUIT:
	fprintf(stderr, "   write failed!!! ,reboot, iret=%d\n", iret);
	printf("failed...rebooting");
	free(buf);
	javascriptUpdate(0, imageType);
	return -1;
FINISH_UPGRADE_AND_QUIT:
	
	printf("Done...rebooting");
	javascriptUpdate(1, imageType);
	free(buf);
	return 0;
}

char *getTitle(IMAGE_TYPE imageType)
{
	if(imageType==IMAGE_TYPE_LINUX)
	{
		return "Upload Linux Kernel";
	}
	else if(imageType==IMAGE_TYPE_FS)
	{
		return "Upload Root FileSystem";
	}
	else if(imageType==IMAGE_TYPE_APP)
	{
		return "Upload Firmware";
	}
	else
	{
		return "Upload ALL";
	}
}

static int upgrade(IMAGE_TYPE imageType)
{
	char *filename = getenv("UPLOAD_FILENAME");

	if(imageType==IMAGE_TYPE_FS || imageType==IMAGE_TYPE_ALL)
	{
		system("cp -f /gm/bin/busybox /var/busybox");
	}
	system("killall Mainengine");
	system("killall -9 Mainengine");
	
	printf(
"\
Server: %s\n\
Pragma: no-cache\n\
Content-type: text/html\n",
getenv("SERVER_SOFTWARE"));

    printf("\n\
<html>\n\
<head>\n\
<TITLE>%s</TITLE>\n\
<link rel=stylesheet href=/style/normal_ws.css type=text/css>\n\
<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">\n\
</head>\n\
<body onload=\"update()\"> <h1>%s</h1>", getTitle(imageType), getTitle(imageType));

	upGradeDo(filename, imageType);

	webFoot();

	if(imageType==IMAGE_TYPE_FS || imageType==IMAGE_TYPE_ALL)
		system("(sleep 3; /var/busybox reboot) &");
	else
		system("(sleep 3; reboot) &");

	return 0;
}

