#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "webs.h"
#include "uemf.h"


#define VERSION_INI "/etc/ver"

static void SD_RestoreDefault(webs_t wp, char_t *path, char_t *query)
{
	char_t result[4096]={'\0'};
	int rlen=sizeof(result);
	
	//do restore factory default...	
	websHeader(wp);
	websWrite(wp, T("<h2>%s </h2><br>\n"), result);
	websFooter(wp);
	websDone(wp, 200);

	sleep(1);
	//system("reboot -f");
}


static void SD_succeedAccess(webs_t wp, char_t *path, char_t *query)
{
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websWrite(wp,T("success"));
	websDone(wp, 200);
}


static int SD_aspPrint(int eid, webs_t wp, int argc, char_t **argv)
{
	char *querystring;
    
    if(ejArgs(argc,argv,"%s",&querystring)==1)
    {
        return websWrite(wp,"%s", querystring);
    }

    return 1;
}

/* instead of goahead */
static int SD_getWebserverName(int eid,webs_t wp,int argc,char_t **argv)
{
    char* value;
	char csTmp[256];
	
	if(value==NULL)value="Camera";
	else if(value[0]==0)value="Camera";
	return websWrite(wp,T("%s"),value);
}

static int SD_getASP(int eid,webs_t wp,int argc,char_t **argv)
{
	char result[256]={'\0'};
	char datebuf[64]={'\0'};
	char ipcInfo[64]={'\0'};
	char ver[64]={0};
	char *pstr;

	FILE *fp = fopen(VERSION_INI, "r");
	if(fp != NULL)
	{
		fgets(datebuf, sizeof(datebuf), fp);
		fclose(fp);
		if((pstr=strchr(datebuf, '_')) != NULL)
		{
			*pstr = '\0';
			sscanf(datebuf, "%s", ver);
		}
	}
	
	//tmp define
	strncpy(ipcInfo, "CGS-D02", sizeof(ipcInfo));

	snprintf(result, sizeof(result), "<br>%s-%s<br>%s--%s", ipcInfo, ver, __TIME__, __DATE__);
	return websWrite(wp,T("%s"), result);
}


/************************************************************
define cgi web for camera application
*/
void formDefineIPcamAPI()
{
	printf("========= Goahead API Define ==========\n");
	websFormDefine(T("restoreDefault"), SD_RestoreDefault);
	websFormDefine(T("succeedAccess"), SD_succeedAccess);

	websAspDefine(T("getASP"), SD_getASP);
	websAspDefine(T("aspPrint"), SD_aspPrint);
	websAspDefine(T("getWebserverName"),SD_getWebserverName);
    
}

