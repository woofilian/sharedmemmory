#include <unistd.h>
#include <sys/reboot.h>

#include "webs.h"

//#define PARTITION_LINUX  "LINUX"
//#define PARTITION_FS "root"
//#define PARTITION_USER "USER0" 
//#define PARTITION_BOOT "UBOOT"

#define PARTITION_LINUX "LINUX"
#define PARTITION_FS 	"FS"
#define PARTITION_APP 	"APP"



static int SD_fwUpGrade(char *fwname, char *result, int rslen)
{
	char fwType[256];
	char fwUpName[256];
	int iret=0;
	unsigned char *buf=0;
	unsigned char *objbuf=0;
	unsigned int flen=0;
	
	int fd = fopen(fwname,"rb");
    if(fd>0 && fwname)
    {
        fseek(fd,0,SEEK_END);
        flen = ftell(fd);
        fseek(fd,0,SEEK_SET);
        buf = (unsigned char *)malloc(flen);
		if( buf )
		    fread(buf,flen,1,fd);

		fclose(fd);
        objbuf = buf;

        printf("rlen=%d\n", rslen);

        unsigned char *pBegin, *pEnd;
	    unsigned char *boundary;
        unsigned int rlen=0, boundary_len;

        //get boundary length
    	pBegin       = (unsigned char *)memmem(buf, flen, "\r\n", 2);
    	if(pBegin == NULL)
    	{
    		snprintf(result, rslen, "Error: can not find boundary end!");
			goto GIVEUP_UPGRADE_AND_QUIT;
    		//return -1;   	
    	}
    	
    	boundary_len = pBegin - buf;

    	//get firmware type
    	pBegin       = (unsigned char *)memmem(pBegin, flen-(pBegin-buf), "\r\n\r\n", 4);
    	if(pBegin == NULL)
    	{
    		snprintf(result, rslen, "Error: can not find firmware type head!");
			goto GIVEUP_UPGRADE_AND_QUIT;
    		//return -1;   	
    	}
    	
    	pBegin       += 4;
    	pEnd         = (unsigned char *)memmem(pBegin, flen-(pBegin-buf), "\r\n", 2);
    	if(pEnd == NULL)
    	{
    		snprintf(result, rslen, "Error: can not find firmware type tail!");
			goto GIVEUP_UPGRADE_AND_QUIT;
    		//return -1;   	
    	}
    	else if(pEnd-pBegin > sizeof(fwType))
    	{
    		snprintf(result, rslen, "firmware type is too long, limited 256 bytes");
			goto GIVEUP_UPGRADE_AND_QUIT;
    		//return -1;
    	}
    	else
    	{
    		memcpy(fwType, pBegin, pEnd-pBegin);
    		fwType[pEnd-pBegin]=0;
    		printf("fw type=%s\n", fwType);
    	}

		// get upload firmware name
    	pBegin = pEnd + 2;
    	pBegin = (unsigned char *)memmem(pBegin, flen-(pBegin-buf), "filename=", sizeof("filename=")-1);
		if(pBegin != NULL)
    		pBegin += sizeof("filename=")-1;
		
    	pEnd   = (unsigned char *)memmem(pBegin, flen-(pBegin-buf), "\r\n", 2);
		if(pEnd == NULL)
		{
			snprintf(result, rslen, "Error: can not find firmware name!");
			goto GIVEUP_UPGRADE_AND_QUIT;
		}
    	else if(pEnd-pBegin > sizeof(fwUpName))
    	{
    		snprintf(result, rslen, "firmware upload name is too long, limited 256 bytes");
            goto GIVEUP_UPGRADE_AND_QUIT;
			//return -1;	
    	}
    	else
    	{
    		memcpy(fwUpName, pBegin+1, pEnd-pBegin-2); //skip the ""
    		fwUpName[pEnd-pBegin-2]=0;
    		printf("fw name=%s\n", fwUpName);
    	}

    	if(strcasestr(fwUpName, fwType) == NULL)
		{
			printf("ttttttttttttttttttttttttttttt\n");
			snprintf(result, rslen, "Error: firmware doesn't match the firmware type!, fwUpName=%s, fwType=%s", fwUpName, fwType);
			printf("result=%s\n", result);
			goto GIVEUP_UPGRADE_AND_QUIT;
			//return -1;
		}

    	//get upload file length
    	pBegin       = (unsigned char *)memmem(pBegin, flen-(pBegin-buf), "\r\n\r\n", 4);
    	pBegin       += 4;
    	pEnd   = (unsigned char *)memmem(pBegin, flen-(pBegin-buf), buf, boundary_len);
    	pEnd -= 2; //skip  \r\n
    	flen = pEnd-pBegin;
        objbuf = pBegin;
    	printf("filelen=%d, buf=%x, pBegin=%x, pEnd=%x, rlen=%d, boundary_len=%d, fwlen=%d\n", flen, buf, pBegin, pEnd, rlen, boundary_len, pEnd-pBegin);

		snprintf(result, rslen, "<h2> upload %s success! </h2><br>", fwType);
		if(strcasecmp(fwType, "fs")==0)
		{
			//flash_write_byname(PARTITION_FS, (char*)pBegin, 0, pEnd-pBegin);
		}
		else if(strcasecmp(fwType, "user")==0)
		{
			//flash_write_byname(PARTITION_USER, (char*)pBegin, 0, pEnd-pBegin);
		}
		else if(strcasecmp(fwType, "linux")==0)
		{
			//flash_write_byname(PARTITION_LINUX, (char*)pBegin, 0, pEnd-pBegin);
		}
		else
		{
			snprintf(result, rslen, "<h2> unknow fw type: %s </h2><br>", fwType);
		}

		goto FINISH_UPGRADE_AND_QUIT;
        //return 0;
    }
    else
    {
    	snprintf(result, rslen, "%x, errno=%d, %s", fwname, errno, strerror(errno));
    	return -1;
    }
	
GIVEUP_UPGRADE_AND_QUIT:
	free(buf);
	return -1;
FINISH_UPGRADE_AND_QUIT:
	free(buf);
	return 0;
}

static void FW_upload(webs_t wp, char_t *path, char_t *query)
{
	char *filename=websGetVar(wp, "filename", "unkonw");
	char *fwType=websGetVar(wp, "fwType", "12");
	char result[512];
	printf("filename = %s, fwType=%s\n", filename, fwType);
	if(wp->cgiStdin == NULL)
	{
		printf("storename: %s\n", wp->cgiStdin);
		snprintf(result, sizeof(result), "%s", "Error: can not find the file!");
	}
	else
	{
		SD_fwUpGrade(wp->cgiStdin, result, sizeof(result));
	}

	printf("%s\n", result);
	websHeader(wp);
	websWrite(wp, T("%s"), result);
	websFooter(wp);
	websDone(wp, 200);

	//sleep(3);
	//system("sleep 3; reboot");
}

void FW_FormDefine(void)
{
	websFormDefine(T("uploadFW"), FW_upload);
}


