/*
 * main.c -- Main program for the GoAhead WebServer (LINUX version)
 *
 * Copyright (c) GoAhead Software Inc., 1995-2010. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 *
 */

/******************************** Description *********************************/

/*
 *	Main program for for the GoAhead WebServer.
 */

/********************************* Includes ***********************************/

#include	"uemf.h"
#include	"wsIntrn.h"
#include	<signal.h>
#include	<unistd.h>
#include	<sys/types.h>

#ifdef WEBS_SSL_SUPPORT
#include	"websSSL.h"
#endif

#ifdef USER_MANAGEMENT_SUPPORT
#include	"um.h"
void	formDefineUserMgmt(void);
#endif


/*********************************** Locals ***********************************/
/*
 *	Change configuration here
 */

static char_t		*rootWeb = T("/tmp/webs");			/* Root web directory */
static char_t		*password = T("");				/* Security password */
static int			port = WEBS_DEFAULT_PORT;		/* Server port */
static int			retries = 5;					/* Server port retries */
static int			finished = 0;					/* Finished flag */
static char_t		*gopid = T("/var/run/goahead.pid");	/* pid file */

/****************************** Forward Declarations **************************/

static int	writeGoPid(void);
static int 	initWebs();
static int	aspTest(int eid, webs_t wp, int argc, char_t **argv);
static void formTest(webs_t wp, char_t *path, char_t *query);
static void formUploadFileTest(webs_t wp, char_t *path, char_t *query); // add by gyr 2011.10.15
static int  websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
				int arg, char_t *url, char_t *path, char_t *query);
static void	sigintHandler(int);
#ifdef B_STATS
static void printMemStats(int handle, char_t *fmt, ...);
static void memLeaks();
#endif

/*********************************** Code *************************************/
/*
 *	Main -- entry point from LINUX
 */

int main(int argc, char** argv)
{
	int i;

/*
 *	Initialize the memory allocator. Allow use of malloc and start
 *	with a 60K heap.  For each page request approx 8KB is allocated.
 *	60KB allows for several concurrent page requests.  If more space
 *	is required, malloc will be used for the overflow.
 */
	bopen(NULL, (60 * 1024), B_USE_MALLOC);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, sigintHandler);
	signal(SIGTERM, sigintHandler);
	if (writeGoPid() < 0)
		return -1;

/*
 *	Initialize the web server
 */
	if (initWebs() < 0) {
		printf("initWebs error.\n");	// added by gyr 2011.09.17
		return -1;
	}

#ifdef WEBS_SSL_SUPPORT
	websSSLOpen();
/*	websRequireSSL("/"); */	/* Require all files be served via https */
#endif

/*
 *	Basic event loop. SocketReady returns true when a socket is ready for
 *	service. SocketSelect will block until an event occurs. SocketProcess
 *	will actually do the servicing.
 */
	finished = 0;
	while (!finished) {
		if (socketReady(-1) || socketSelect(-1, 1000)) {
			socketProcess(-1);
		}
		websCgiCleanup();
		emfSchedProcess();
	}

#ifdef WEBS_SSL_SUPPORT
	websSSLClose();
#endif

#ifdef USER_MANAGEMENT_SUPPORT
	umClose();
#endif

/*
 *	Close the socket module, report memory leaks and close the memory allocator
 */
	websCloseServer();
	socketClose();
#ifdef B_STATS
	memLeaks();
#endif
	bclose();
	return 0;
}

/******************************************************************************/
/*
 *	Write pid to the pid file
 */
int writeGoPid(void)
{
	FILE *fp;

	fp = fopen(gopid, "w+");
	if (NULL == fp) {
		error(E_L, E_LOG, T("goahead.c: cannot open pid file"));
		return (-1);
	}
	fprintf(fp, "%d", getpid());
    fclose(fp);
	return 0;
}
/*
 *	Exit cleanly on interrupt
 */
static void sigintHandler(int unused)
{
	finished = 1;
}

/******************************************************************************/
/*
 *	Initialize the web server.
 */

static int initWebs()
{
	struct hostent	*hp;
	struct in_addr	intaddr;
	char			host[128];
	char			*cp;
	char_t			wbuf[128];

/*
 *	Initialize the socket subsystem
 */
	socketOpen();

#ifdef USER_MANAGEMENT_SUPPORT
/*
 *	Initialize the User Management database
 */
	umOpen();
	umRestore(T("umconfig.txt"));
#endif

/*
 *	Define the local Ip address, host name, default home page and the
 *	root web directory.
 */
 #if 0
 /* del by gyr 2011.09.17*/
	if (gethostname(host, sizeof(host)) < 0) {
		error(E_L, E_LOG, T("Can't get hostname"));
		printf("initWebs::Can't get hostname.\n");		
		return -1;
	}
	if ((hp = gethostbyname(host)) == NULL) {
		error(E_L, E_LOG, T("Can't get host address"));
		printf("initWebs::Can't get hostname...\n");	
		return -1;
	}
	memcpy((char *) &intaddr, (char *) hp->h_addr_list[0],
		(size_t) hp->h_length);
/**/
#else
	intaddr.s_addr = inet_addr(T("192.168.20.147"));		
#endif

/*
 *	Set rootWeb as the root web. Modify this to suit your needs
 *	A "-demo" option to the command line will set a webdemo root
 */
/*
 *	Configure the web server options before opening the web server
 */
	websSetDefaultDir(rootWeb);
	cp = inet_ntoa(intaddr);
	ascToUni(wbuf, cp, min(strlen(cp) + 1, sizeof(wbuf)));
	websSetIpaddr(wbuf);
	ascToUni(wbuf, host, min(strlen(host) + 1, sizeof(wbuf)));
	websSetHost(wbuf);

/*
 *	Configure the web server options before opening the web server
 */
	websSetDefaultPage(T("asptest.asp"));
	websSetPassword(password);

/*
 *	Open the web server on the given port. If that port is taken, try
 *	the next sequential port for up to "retries" attempts.
 */
	websOpenServer(port, retries);

/*
 * 	First create the URL handlers. Note: handlers are called in sorted order
 *	with the longest path handler examined first. Here we define the security
 *	handler, forms handler and the default web page handler.
 */
	websUrlHandlerDefine(T(""), NULL, 0, websSecurityHandler,
		WEBS_HANDLER_FIRST);
	websUrlHandlerDefine(T("/goform"), NULL, 0, websFormHandler, 0);
	websUrlHandlerDefine(T("/cgi-bin"), NULL, 0, websCgiHandler, 0);
	websUrlHandlerDefine(T(""), NULL, 0, websDefaultHandler,
		WEBS_HANDLER_LAST);

/*
 *	Now define two test procedures. Replace these with your application
 *	relevant ASP script procedures and form functions.
 */
	websAspDefine(T("aspTest"), aspTest);
	websFormDefine(T("formTest"), formTest);
	websFormDefine(T("formUploadFileTest"), formUploadFileTest);

	FW_FormDefine();
	
	formDefineIPcamAPI();
	
/*
 *	Create the Form handlers for the User Management pages
 */
#ifdef USER_MANAGEMENT_SUPPORT
	formDefineUserMgmt();
#endif

/*
 *	Create a handler for the default home page
 */
	websUrlHandlerDefine(T("/"), NULL, 0, websHomePageHandler, 0);

	return 0;
}

/******************************************************************************/
/*
 *	Test Javascript binding for ASP. This will be invoked when "aspTest" is
 *	embedded in an ASP page. See web/asp.asp for usage. Set browser to
 *	"localhost/asp.asp" to test.
 */

static int aspTest(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t	*name, *address;

	if (ejArgs(argc, argv, T("%s %s"), &name, &address) < 2) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	return websWrite(wp, T("Name: %s, Address %s"), name, address);
}

/******************************************************************************/
/*
 *	Test form for posted data (in-memory CGI). This will be called when the
 *	form in web/forms.asp is invoked. Set browser to "localhost/forms.asp" to test.
 */

static void formTest(webs_t wp, char_t *path, char_t *query)
{
	char_t	*name, *address;

	name = websGetVar(wp, T("name"), T("Joe Smith"));
	address = websGetVar(wp, T("address"), T("1212 Milky Way Ave."));

	websHeader(wp);
	websWrite(wp, T("<body><h2>Name: %s, Address: %s</h2>\n"), name, address);
	websFooter(wp);
	websDone(wp, 200);
}

/******************************************************************************/
/*
 *	Home page handler
 */

static int websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
	int arg, char_t *url, char_t *path, char_t *query)
{
/*
 *	If the empty or "/" URL is invoked, redirect default URLs to the home page
 */
	if (*url == '\0' || gstrcmp(url, T("/")) == 0) {
		websRedirect(wp, WEBS_DEFAULT_HOME);
		return 1;
	}
	return 0;
}

/******************************************************************************/

#ifdef B_STATS
static void memLeaks()
{
	int		fd;

	if ((fd = gopen(T("leak.txt"), O_CREAT | O_TRUNC | O_WRONLY, 0666)) >= 0) {
		bstats(fd, printMemStats);
		close(fd);
	}
}

/******************************************************************************/
/*
 *	Print memory usage / leaks
 */

static void printMemStats(int handle, char_t *fmt, ...)
{
	va_list		args;
	char_t		buf[256];

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	write(handle, buf, strlen(buf));
}
#endif


/******************************************************************************/
/*
 * for test html upload file to web server
 * add by gyr 2011.10.15
 */

static void formUploadFileTest(webs_t wp, char_t *path, char_t *query)
{
    FILE *       fp;
    char_t *     fn;
    char_t *     bn = NULL;
    int          locWrite;
    int          numLeft;
    int          numWrite;

	printf("\n...................formUploadFileTest...................\n\n");

    a_assert(websValid(wp));
    websHeader(wp);

    fn = websGetVar(wp, T("filename"), T(""));
    if (fn != NULL && *fn != '\0') {
        if ((int)(bn = gstrrchr(fn, '/') + 1) == 1) {
            if ((int)(bn = gstrrchr(fn, '\\') + 1) == 1) {
                bn = fn;
            }
        }
    }

	printf("fn=%s, bn=%s  .......\n", fn, bn);

    websWrite(wp, T("Filename = %s<br>Size = %d bytes<br>"), bn, wp->lenPostData);

    if ((fp = fopen((bn == NULL ? "upldForm.bin" : bn), "w+b")) == NULL) {
        websWrite(wp, T("File open failed!<br>"));
    } else {
        locWrite = 0;
        numLeft = wp->lenPostData;
        while (numLeft > 0) {
            numWrite = fwrite(&(wp->postData[locWrite]), sizeof(*(wp->postData)), numLeft, fp);
            if (numWrite < numLeft) {
                websWrite(wp, T("File write failed.<br>  ferror=%d locWrite=%d numLeft=%d numWrite=%d Size=%d bytes<br>"), ferror(fp), locWrite, numLeft, numWrite, wp->lenPostData);
            break;
            }
            locWrite += numWrite;
            numLeft -= numWrite;
        }

        if (numLeft == 0) {
            if (fclose(fp) != 0) {
                websWrite(wp, T("File close failed.<br>  errno=%d locWrite=%d numLeft=%d numWrite=%d Size=%d bytes<br>"), errno, locWrite, numLeft, numWrite, wp->lenPostData);
            } else {
                websWrite(wp, T("File Size Written = %d bytes<br>"), wp->lenPostData);
            }
        } else {
            websWrite(wp, T("numLeft=%d locWrite=%d Size=%d bytes<br>"), numLeft, locWrite, wp->lenPostData);
        }
    }

    websFooter(wp);
    websDone(wp, 200);

}

/******************************************************************************/
