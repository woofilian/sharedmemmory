#=============================================#
-include ../arch.mk

# ============= 输出目录定义 ===========# 
#EXP_TARGET_DIR: 如果 EXP_TARGET_DIR不存在，则为 ./lib
#EXP_INC_DIR:    如果 EXP_INC_DIR不存在，则为 ./inc

#EXP_TARGET_DIR ?= $(PWD)/lib
#EXP_INC_DIR    ?= $(PWD)/inc

EXP_TARGET_DIR ?= ../../lib
EXP_INC_DIR    ?= ../../inc


# ============= lib compile option ===========#
ifeq ($(CROSS),)
	uClibcpp_LDFLAGS:= -rdynamic -ldl 		-lstdc++
	HISI_LIB_PATH:=
	HISI_LDFLAGS:=
	MAIN_DEFINE := -D_NO_CODEC
else
	#uClibcpp_LDFLAGS:= -L/opt/hisi-linux/x86-arm/gcc-3.4.3-uClibc-0.9.28/uClibc++/0.2.0/lib/ -luClibc++		
	uClibcpp_LDFLAGS:= -rdynamic -ldl -lstdc++

				   
	MAIN_DEFINE :=
	COMM_LIB_LD := 
endif

SHMLIB_PATH := ../../../libshmav/
LIB_LDFLAGS := -L./ -L$(EXP_TARGET_DIR)  -L$(SHMLIB_PATH) -lshmav -ladapt  -lnet -lrtsp  -lstorage\
			 $(EX_LDFLAGS) $(ARCH_LDFLAGS) $(uClibcpp_LDFLAGS) \
			-lpthread   -ldl -lm -lrt -lcrypt -lstdc++

ifeq ($(VR_VERSION),)
$(error O_O O_O O_O request VR_VERSION=V0.0.x O_O O_O O_O)
else
DVR_VER_CFLAGS = -g -Wall -D_VR_VERSION="\"\\\"$(VR_VERSION)\"\\\""
endif

MAIN_INC_PATH:=$(PWD)/src

#LIB_TARGET  := main.$(VR_VERSION).exe
LIB_TARGET  := Mainengine
EXP_LIB_INC := main.h

LIB_SRC     := ./src
LIB_CFLAGS  :=  $(ARCH_FLAGS) -Wall $(DVR_VER_CFLAGS) -I$(MAIN_INC_PATH) $(MAIN_DEFINE)\
	-I$(EXP_INC_DIR) \
	-I$(JSDK_INC_PATH) \
	-DMODULE_NAME="\"\\\"main\"\\\""

LIB_CPPFLAGS:=$(LIB_CFLAGS)
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
	@echo  "========TMP_SDIR:$(TMP_SDIR)"
	@echo  "========TOP_DIR:$(TOP_DIR)"
	cd $(PWD)/src/;$(RM) main.o
	make $(LIB_MAKE_PARAM)
#---------- install main -----------------#
	cd $(PWD)/src/;$(INSTALL) $(LIB_TARGET) $(EXP_TARGET_DIR)
	cd $(PWD)/src/;$(INSTALL) $(EXP_LIB_INC) $(EXP_INC_DIR)
#make $(DEMO_MAKE_PARAM)
	
	cd $(PWD)/src/;$(CROSS)strip $(LIB_TARGET); $(INSTALL) $(LIB_TARGET) ../../bin
	
clean:
	make clean NODEP=yes $(LIB_MAKE_PARAM)
#make clean NODEP=yes $(DEMO_MAKE_PARAM)
	rm -rf src/$(LIB_TARGET)

