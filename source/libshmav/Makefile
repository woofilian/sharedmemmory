CROSS_COMPILE=arm-unknown-linux-uclibcgnueabi-

LD=$(CROSS_COMPILE)ld
CC=$(CROSS_COMPILE)gcc
CPP=$(CROSS_COMPILE)g++
CXX=$(CROSS_COMPILE)g++
AR=$(CROSS_COMPILE)ar
AS=$(CROSS_COMPILE)as
NM=$(CROSS_COMPILE)nm
CPP        = $(CC) -E
STRIP   = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
STRIPTOOL=$(CROSS_COMPILE)strip
LN=ln
RM=rm -rf

LIBSHMAV=libshmav.a
LIBSHMAV_SHARED=libshmav.so

#INSTALL_DIR=../../../output/lib/

SRCS  := $(wildcard *.cpp)
OBJS  := $(SRCS:.cpp=.o)
HEADS := $(wildcard *.h)

all : $(LIBSHMAV) $(LIBSHMAV_SHARED)

$(OBJS): %.o : %.cpp
	$(CXX) -fPIC -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(LIBSHMAV_SHARED): $(LIBSHMAV)
	$(CC) $(OBJS)  -shared -o -fPIC -o $(LIBSHMAV_SHARED) 
#	cp -f $(LIBSHMAV_SHARED)  $(INSTALL_DIR)

$(LIBSHMAV) ar-target: $(OBJS)
	$(AR) -q $(LIBSHMAV) $(OBJS)
#cp -f  $(LIBSHMAV) $(INSTALL_DIR)

$(OBJS): $(HEADERS) Makefile

clean:
	$(RM) *.[oa] $(LIBSHMAV_SHARED)*  $(INSTALL_DIR)/$(LIBSHMAV_SHARED)  $(INSTALL_DIR)/$(LIBSHMAV)
