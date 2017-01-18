#=============================================#

-include ../arch.mk

# ============= 输出目录定义 ===========# 
#EXP_TARGET_DIR: 如果 EXP_TARGET_DIR不存在，则为 ./lib
#EXP_INC_DIR:    如果 EXP_INC_DIR不存在，则为 ./inc

#EXP_TARGET_DIR ?= $(PWD)/lib
#EXP_INC_DIR    ?= $(PWD)/inc

EXP_TARGET_DIR ?= ../../lib/grain/
EXP_INC_DIR    ?= ../../inc

# ============= demo compile option ===========#
DEMO_TARGET := storage
DEMO_SRC    := ./demo

ifdef CROSS
DEMO_CFLAGS :=  $(ARCH_FLAGS) -Wall -I./ -I$(EXP_INC_DIR) -DMODULE_NAME="\"\\\"storage\"\\\""
else
DEMO_CFLAGS := -g -Wall -I./ -I$(EXP_INC_DIR) -DMODULE_NAME="\"\\\"storage\"\\\""
endif

DEMO_CPPFLAGS:=$(DEMO_CFLAGS)
DEMO_LDFLAGS:= -lpthread -L./ -L$(EXP_TARGET_DIR) -lstorage -lstream -llog  -lmisc  $(EX_LDFLAGS)
# ---------------------------------------------#
DEMO_MAKE_PARAM := -C $(DEMO_SRC) \
		TARGET=$(DEMO_TARGET)\
		EXPORT_TARGET_DIR=$(EXP_TARGET_DIR)\
		CFLAGS="$(DEMO_CFLAGS)"\
		CXXFLAGS="$(DEMO_CPPFLAGS)"\
		LDFLAGS="$(DEMO_LDFLAGS)"



# ============= lib compile option ===========#

LIB_TARGET  := libstorage.a
EXP_LIB_INC := storage.h
LIB_SRC     := ./src

ifdef CROSS
LIB_CFLAGS  :=  $(ARCH_FLAGS) -Wall -I$(EXP_INC_DIR) \
	-DMODULE_NAME="\"\\\"storage\"\\\""
else
LIB_CFLAGS  := -g -Wall -I$(EXP_INC_DIR) -DMODULE_NAME="\"\\\"storage\"\\\""
endif

LIB_CPPFLAGS:=$(LIB_CFLAGS)
LIB_LDFLAGS := -lpthread -L./ -lmisc $(EX_LDFLAGS)
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

