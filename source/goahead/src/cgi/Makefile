CROSS_COMPILE=arm-unknown-linux-uclibcgnueabi-
CC=$(CROSS_COMPILE)gcc
STRIP=$(CROSS_COMPILE)strip

GOAHEAD_DIR=../..
MTD_NVRAM_DIR=$(GOAHEAD_DIR)/lib/nvram/
CFLAGS += -I$(MTD_NVRAM_DIR)
LDFLAGS += -L$(MTD_NVRAM_DIR) -lnvram
LDFLAGS += -lpthread

CGI_BIN_LINIX=upload_linux.cgi
CGI_SRC_LINUX=$(CGI_BIN_LINIX).c

CGI_BIN_FS=upload_fs.cgi
CGI_SRC_FS=$(CGI_BIN_FS).c

CGI_BIN_APP=upload_app.cgi
CGI_SRC_APP=$(CGI_BIN_APP).c

CGI_BIN_ALL=upload_all.cgi
CGI_SRC_ALL=$(CGI_BIN_ALL).c

WEBS_CGI_DIR=../webs/cgi-bin/

all:
	$(CC) -o $(CGI_BIN_LINIX) $(CGI_SRC_LINUX) $(CFLAGS) $(LDFLAGS)
	$(CC) -o $(CGI_BIN_FS)    $(CGI_SRC_FS)    $(CFLAGS) $(LDFLAGS)
	$(CC) -o $(CGI_BIN_APP)   $(CGI_SRC_APP)   $(CFLAGS) $(LDFLAGS)
	$(CC) -o $(CGI_BIN_ALL)   $(CGI_SRC_ALL)   $(CFLAGS) $(LDFLAGS)
#	$(STRIP)  $(CGI_BIN_LINIX)
#	$(STPIP)  $(CGI_BIN_FS)
#	$(STPIP)  $(CGI_BIN_APP)
	cp -f $(CGI_BIN_LINIX)  $(WEBS_CGI_DIR)
	cp -f $(CGI_BIN_FS)     $(WEBS_CGI_DIR)
	cp -f $(CGI_BIN_APP)    $(WEBS_CGI_DIR)
	cp -f $(CGI_BIN_ALL)    $(WEBS_CGI_DIR)
	#cp $(CGI_BIN_LINIX) /tftpboot/share
	#cp $(CGI_BIN_FS) /tftpboot/share
	#cp $(CGI_BIN_APP) /tftpboot/share
	#cp $(CGI_BIN_ALL) /tftpboot/share

clean:
	rm *.cgi
