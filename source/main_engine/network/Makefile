#=============================================#

-include ../arch.mk

# ============= 输出目录定义 ===========# 
#EXP_TARGET_DIR: 如果 EXP_TARGET_DIR不存在，则为 ./lib
#EXP_INC_DIR:    如果 EXP_INC_DIR不存在，则为 ./inc

#EXP_TARGET_DIR ?= $(PWD)/lib
#EXP_INC_DIR    ?= $(PWD)/inc

EXP_TARGET_DIR ?= ../../lib

EXP_INC_DIR  ?= ../../inc



ifeq ($(VR_VERSION),)
$(error O_O O_O O_O request VR_VERSION=V0.0.x O_O O_O O_O)
else
DVR_VER_CFLAGS = -g -Wall -D_VR_VERSION="\"\\\"$(VR_VERSION)\"\\\""
endif


# ============= demo compile option ===========#
DEMO_TARGET := netdemo
DEMO_SRC    := ./demo

ifdef CROSS
DEMO_CFLAGS :=  $(ARCH_FLAGS) -Wall -I./ -I$(EXP_INC_DIR)  -DMODULE_NAME="\"\\\"netdemo\"\\\""
else
DEMO_CFLAGS := -g -Wall -I./ -I$(EXP_INC_DIR) -DMODULE_NAME="\"\\\"netdemo\"\\\""
endif

DEMO_CPPFLAGS:=$(DEMO_CFLAGS)
DEMO_LDFLAGS:= -lpthread -L./   -L$(EXP_TARGET_DIR)  -ladapt -lnet $(ARCH_LDFLAGS) -lpthread   -ldl -lm -lrt
# ---------------------------------------------#
DEMO_MAKE_PARAM := -C $(DEMO_SRC) \
		TARGET=$(DEMO_TARGET)\
		EXPORT_TARGET_DIR=$(EXP_TARGET_DIR)\
		CFLAGS="$(DEMO_CFLAGS)"\
		CXXFLAGS="$(DEMO_CPPFLAGS)"\
		LDFLAGS="$(DEMO_LDFLAGS)"


# ============= lib compile option ===========#

LIB_TARGET  := libnet.a
EXP_LIB_INC := netlib.h 
LIB_SRC     := ./src

ifdef CROSS
LIB_CFLAGS  := $(ARCH_FLAGS) -Wall $(DVR_VER_CFLAGS) -I$(EXP_INC_DIR) -I$(JSDK_INC_PATH) \
	-DMODULE_NAME="\"\\\"net\"\\\""
else
LIB_CFLAGS  := -g -Wall $(DVR_VER_CFLAGS) -I$(EXP_INC_DIR)  -DMODULE_NAME="\"\\\"net\"\\\""
endif

LIB_CPPFLAGS:=$(LIB_CFLAGS)
LIB_LDFLAGS := -lpthread -L./   -L$(EXP_TARGET_DIR)  -ladapt -llog   -lmisc
# ---------------------------------------------#
LIB_MAKE_PARAM := -C $(LIB_SRC) \
		TARGET=$(LIB_TARGET)\
		EXPORT_INC_FILE="$(EXP_LIB_INC)"\
		EXPORT_TARGET_DIR=$(EXP_TARGET_DIR)\
		EXPORT_INC_DIR=$(EXP_INC_DIR)\
		CFLAGS="$(LIB_CFLAGS)"\
		CXXFLAGS="$(LIB_CFLAGS)"\
		LDFLAGS="$(LIB_LDFLAGS)"

.PHONY: default clean

default:
	make $(LIB_MAKE_PARAM)
#	make $(DEMO_MAKE_PARAM)
	
clean:
	make clean NODEP=yes $(LIB_MAKE_PARAM)
#	make clean NODEP=yes $(DEMO_MAKE_PARAM)

