#=============================================#
-include ../arch.mk

EXP_TARGET_DIR ?= ../../lib
EXP_INC_DIR    ?= ../../inc

# ============= lib compile option ===========#

LIB_TARGET  := libdana.a
EXP_LIB_INC := dana.h   
LIB_SRC     := ./src

SHMLIB_PATH := ../../libshmav/
ifdef CROSS
LIB_CFLAGS  := $(ARCH_FLAGS) -Wall $(DVR_VER_CFLAGS) -I$(EXP_INC_DIR)  -I$(JSDK_INC_PATH) -I$(SHMLIB_PATH) -lpthread   -ldl -lrt -lm  -lstdc++	-lresolv  -llog -ldana_video_plus
else
LIB_CFLAGS  := -g -Wall $(DVR_VER_CFLAGS) -I$(EXP_INC_DIR)  -I$(LOCAL_INC_DIR) -I$(SHMLIB_PATH) -I$(LOCAL_INC_DIR)/curl -lpthread   -ldl -lrt -lm  -lstdc++	-lresolv  -llog -ldana_video_plus
endif

LIB_CPPFLAGS:=$(LIB_CFLAGS)
LIB_LDFLAGS :=   -L./  -L$(EXP_TARGET_DIR) $(ARCH_LDFLAGS)  -L$(SHMLIB_PATH) -lshmav -lpthread   -ldl -lrt -lm  -lstdc++	-lresolv 

#-ladapt -llog   -lmisc
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
	
clean:
	make clean NODEP=yes $(LIB_MAKE_PARAM)

