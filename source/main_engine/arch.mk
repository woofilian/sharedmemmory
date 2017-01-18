#--------------------------------------#

#export VR_VERSION = V5.1.4
export VR_VERSION = V0.0.8

ARCH_CPU ?= GM_36S
#ARCH_CPU ?= x86

TMP_SDIR               = $(shell pwd)
TOP_DIR           = $(PWD)/..


export	CURL_INC=$(TOP_DIR)/inc/curl
export	OPENSSL_INC=$(TOP_DIR)/inc/openssl
export	ALSA_INC=$(TOP_DIR)/inc/alsa





ifeq ($(ARCH_CPU),GM_36S)
	export CROSS = arm-unknown-linux-uclibcgnueabi-
	export ARCH_FLAGS =  -g  -O2  -Wall  -fPIC  -D_REENTRENT -D_GNU_SOURCE     -D_GRAIN_ -D_GM_36S_
	export ARCH_LDFLAGS:= -L$(TOP_DIR)/lib/grain  -lcodec -lparam -lstream  -lupgrade  -ls3 -lcurl  -lssl -Wall  -Wshadow -Wextra -lcrypto -lxml2   -llog -lmisc  -lgm -ldana -ldana_video_plus 

endif

ifeq ($(ARCH_CPU),x86)
    export ARCH_FLAGS = -D_ARCH_x86_ -D_ARCH_3520D_ -g -DDEBUG
endif


export CC = $(CROSS)gcc
#export CC = $(CROSS)g++
export AR = $(CROSS)ar
export AS = $(CROSS)as
export LD = $(CROSS)ld
export STRIP = $(CROSS)strip

#ifeq ($(CROSS),)
ifdef CROSS
	export CXX = $(CROSS)g++
else
	export CXX = $(CROSS)g++-uc
endif


export CP = cp
export RM = rm -vf
export INSTALL = cp -rfv
export LN_S = ln -s
