#=============================================#
export WORK_PATH					?= $(PWD)/..
-include ../arch.mk
-include	../param.mk
# ============= 输出目录定义 ===========# 
#EXP_TARGET_DIR: 如果 EXP_TARGET_DIR不存在，则为 ./lib
#EXP_INC_DIR:    如果 EXP_INC_DIR不存在，则为 ./inc

#EXP_TARGET_DIR ?= $(PWD)/lib
#EXP_INC_DIR    ?= $(PWD)/inc
#export WORK_DIR		:= $(shell pwd)
#@echo "=======current work path: $WORK_DIR====="
#=========================================
ifeq ($(ARCH_CPU),AMBA_S2L)
EXP_TARGET_DIR ?= ../../lib/amba
endif

ifeq ($(ARCH_CPU),GM_36S)
EXP_TARGET_DIR ?= ../../lib/grain
endif


EXP_INC_DIR    ?= ../../inc

CXXABI_PATH ?= -I$(EXP_INC_DIR)


# ============= lib compile option ===========#

LIB_TARGET  := libcodec.a
EXP_LIB_INC := codeclib.h

LIB_SRC     := ./src

SHMLIB_PATH := ../../../libshmav/ 

ifdef CROSS
LIB_CFLAGS  := $(ARCH_FLAGS) -Wall -I$(EXP_INC_DIR) -I$(CXXABI_PATH) -I$(SHMLIB_PATH) $(LOCAL_CFLAGS) \-DMODULE_NAME="\"\\\"codecdemo\"\\\""
else
LIB_CFLAGS  := -g -Wall -I$(EXP_INC_DIR)  -I$(CXXABI_PATH) -DMODULE_NAME="\"\\\"codecdemo\"\\\""
endif
LIB_CPPFLAGS:=$(LIB_CFLAGS)
LIB_LDFLAGS := -lpthread -L./  -L$(SHMLIB_PATH) -lshmav   -L$(EXP_TARGET_DIR)-llog  -lavi -lmisc  -L$(EXP_TARGET_DIR)/grain -lgm  -lAED
# ---------------------------------------------#
LIB_MAKE_PARAM := -C $(LIB_SRC) \
		TARGET=$(LIB_TARGET)\
		EXPORT_INC_FILE="$(EXP_LIB_INC)"\
		EXPORT_TARGET_DIR=$(EXP_TARGET_DIR)\
		EXPORT_INC_DIR=$(EXP_INC_DIR)\
		CFLAGS="$(LIB_CFLAGS)"\
		CXXFLAGS="$(LIB_CFLAGS)"\
		LDFLAGS="$(LIB_LDFLAGS)"S

.PHONY: default clean

default:
	make $(LIB_MAKE_PARAM)
#	make $(DEMO_MAKE_PARAM)

clean:
	make clean NODEP=yes $(LIB_MAKE_PARAM)
#	make clean NODEP=yes $(DEMO_MAKE_PARAM)

