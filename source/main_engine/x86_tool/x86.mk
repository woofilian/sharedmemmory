#---------------------------------------#

#DVR_VERSION 指定版本号
export DVR_VERSION = V0.0.1

#ARCH_CPU 指定CPU类型
#ARCH_CPU ?= AMBA_S2L
ARCH_CPU ?= x86

#此处指定各个需要的文件路径
TMP_SDIR               = $(shell pwd)
TOP_DIR           = $(PWD)/..


export	AMBA_INC=$(TOP_DIR)/inc/amba
export	FREETYPE_INC=$(TOP_DIR)/inc/freetype
export	CURL_INC=$(TOP_DIR)/inc/curl
export	OPENSSL_INC=$(TOP_DIR)/inc/openssl


#export	AMBA_LIB=$(TOP_DIR)/lib/amba
#---------------------------------------#
#export EX_LDFLAGS = -ldmallocth

#CROSS:	指定使用的编译工具前缀, 如果CROSS不存在，则为空。

ifeq ($(ARCH_CPU),AMBA_S2L)
		AMBARELLA_APP_CFLAGS	+= $(call cc-option,-mno-unaligned-access,)
		AMBARELLA_APP_CFLAGS += -mthumb -march=armv7-a -mtune=cortex-a9 -mlittle-endian -mfloat-abi=hard -mfpu=neon -Wa,-mimplicit-it=thumb

#		export CROSS = /usr/local/bk_20150303_linaro-multilib-2013.09-gcc4.8/bin/arm-linux-gnueabihf-
		export CROSS = arm-ambarella-linux-uclibcgnueabihf-
#    export ARCH_FLAGS = $(AMBARELLA_APP_CFLAGS) --O3 -Wall -fPIC -D_REENTRENT -D_GNU_SOURCE  -D_AMBARELLA_
#debug
	export ARCH_FLAGS = $(AMBARELLA_APP_CFLAGS) -g -O0 -Wall -fPIC -D_REENTRENT -D_GNU_SOURCE   -D_AMBARELLA_  -D_AMBA_S2L_

#定义和芯片相关的依赖库
	export ARCH_FLAGS += -I$(TOP_DIR)/inc/amba  -I$(TOP_DIR)/inc/amba/arch_s2l -I$(FREETYPE_INC) -I$(CURL_INC) -I$(OPENSSL_INC)
	export ARCH_LDFLAGS:= -L$(TOP_DIR)/lib/amba     -lrtmp -lcurl -lssl -lcrypto   -limg_algo_s2l -limg_dsp_s2l_hf -lfreetype  -lchrome -lturbojpeg -lfreetype  -lz

#		export ARCH_FLAGS = -marm -march=armv6k -mtune=arm1136j-s -mlittle-endian -msoft-float    -march=armv6k -mfloat-abi=soft   -fPIC  -D_REENTRENT -D_GNU_SOURCE
#   -O0 -Wall -fPIC 
#		-O3 -Wall -fPIC
endif




ifeq ($(ARCH_CPU),x86)
    export ARCH_FLAGS = -D_ARCH_x86_ -D_GRAIN_ -g -DDEBUG
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

#指定使用的shell命令。

export CP = cp
export RM = rm -vf
export INSTALL = cp -rfv
export LN_S = ln -s
