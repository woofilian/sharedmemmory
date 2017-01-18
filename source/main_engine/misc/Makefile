#20101015 by maohw
#=============================================#

-include ../arch.mk

# ============= 输出目录定义 ===========# 
#EXP_TARGET_DIR: 如果 EXP_TARGET_DIR不存在，则为 ./lib
#EXP_INC_DIR:    如果 EXP_INC_DIR不存在，则为 ./inc

#EXP_TARGET_DIR ?= $(PWD)/lib
#EXP_INC_DIR    ?= $(PWD)/inc

EXP_TARGET_DIR ?= ../../lib
EXP_INC_DIR    ?= ../../inc

CXXABI_PATH ?= -I$(PWD)/inc
#CXXABI_PATH ?= -I/opt_3520/hisi-linux/x86-arm/gcc-3.4.3-uClibc-0.9.28/include/c++/3.4.3/ -I/opt_3520/hisi-linux/x86-arm/gcc-3.4.3-uClibc-0.9.28/include/c++/3.4.3/arm-hisi-linux/

# ============= demo compile option ===========#
DEMO_TARGET := miscdemo
DEMO_SRC    := ./demo
ifdef CROSS
DEMO_CFLAGS :=  $(ARCH_FLAGS) -Wall -I./ -I$(EXP_INC_DIR) -DMODULE_NAME="\"\\\"miscdemo\"\\\""
else
DEMO_CFLAGS := -g -Wall -I./ -I$(EXP_INC_DIR) -DMODULE_NAME="\"\\\"miscdemo\"\\\""
endif
DEMO_CPPFLAGS:=$(DEMO_CFLAGS)
DEMO_LDFLAGS:= -rdynamic -ldl -lstdc++ -lpthread -L./ -L$(EXP_TARGET_DIR) -lmisc $(EX_LDFLAGS)
# ---------------------------------------------#
DEMO_MAKE_PARAM := -C $(DEMO_SRC) \
		TARGET=$(DEMO_TARGET)\
		EXPORT_TARGET_DIR=$(EXP_TARGET_DIR)\
		CFLAGS="$(DEMO_CFLAGS)"\
		CXXFLAGS="$(DEMO_CPPFLAGS)"\
		LDFLAGS="$(DEMO_LDFLAGS)"


# ============= lib compile option ===========#

LIB_TARGET  := libmisc.a
EXP_LIB_INC := wrapper.h \
			   hi_net_api.h

LIB_SRC     := ./src
ifdef CROSS
LIB_CFLAGS  := $(ARCH_FLAGS) -Wall -I$(EXP_INC_DIR) $(CXXABI_PATH) \
	-DMODULE_NAME="\"\\\"misc\"\\\""
else
LIB_CFLAGS  := -g -Wall -I$(EXP_INC_DIR) $(CXXABI_PATH) -DMODULE_NAME="\"\\\"misc\"\\\""
endif
LIB_CPPFLAGS:=$(LIB_CFLAGS)
LIB_LDFLAGS := -lpthread -L./
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
#	make $(LIVE555_ENV_PARAM)
#	cp  $(LIVE555_ENV_PATH)/libbasic_env.a  ./src/$(EXP_TARGET_DIR)
#	cp  $(LIVE555_ENV_PATH)/basic_env.h      ./src/$(EXP_INC_DIR)
#	make $(DEMO_MAKE_PARAM)

clean:
	make clean NODEP=yes $(LIB_MAKE_PARAM)
#	make clean NODEP=yes $(LIVE555_ENV_PARAM)
#	make clean NODEP=yes $(DEMO_MAKE_PARAM)

