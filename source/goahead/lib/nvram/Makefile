#
# (C) Copyright 2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

###########################################
CROSS_COMPILE   =arm-unknown-linux-uclibcgnueabi-
AS      = $(CROSS_COMPILE)as
LD      = $(CROSS_COMPILE)ld
CC      = $(CROSS_COMPILE)gcc
CPP     = $(CC) -E
AR      = $(CROSS_COMPILE)ar
NM      = $(CROSS_COMPILE)nm
STRIP   = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
############################################

CFLAGS += -DUSE_HOSTCC -fPIC

MAJOR_VERSION=0
MINOR_VERSION=9
SUBLEVEL=28

#INSTALL_DIR=../libs
LIBNVRAM=libnvram.a
LIBNVRAM_SHARED=libnvram.so
LIBNVRAM_SHARED_FULLNAME=libnvram-$(MAJOR_VERSION).$(MINOR_VERSION).$(SUBLEVEL).so

OBJS	:= crc32.o flash_api.o
HEADERS := flash_api.h


#all: $(LIBNVRAM)  $(LIBNVRAM_SHARED)
all: $(LIBNVRAM)

$(OBJS): %.o : %.c
	echo $(CC)
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIP) -x -R .note -R .comment $*.o

#$(LIBNVRAM_SHARED): 
#$(LD) $(LDFLAGS) -o $(LIBNVRAM_SHARED_FULLNAME) --whole-archive $(LIBNVRAM) $(LIBGCC);
#	$(CC) $(OBJS)  -shared -o -fPIC -o  $(LIBNVRAM_SHARED_FULLNAME) 
#	cp    -f  $(LIBNVRAM_SHARED_FULLNAME) $(INSTALL_DIR)/libnvram.so
#	$(LN) -sf $(INSTALL_DIR)/$(LIBNVRAM_SHARED_FULLNAME) $(INSTALL_DIR)/$(LIBNVRAM_SHARED)
#	$(LN) -sf $(INSTALL_DIR)/$(LIBNVRAM_SHARED_FULLNAME) $(INSTALL_DIR)/$(LIBNVRAM_SHARED).$(MAJOR_VERSION)
#	cp    -f  $(LIBNVRAM_SHARED_FULLNAME) $(OUT_NFS_DIR)/libnvram.so

$(LIBNVRAM) ar-target: $(OBJS)
	$(AR) -q $(LIBNVRAM) $(OBJS)

$(OBJS): $(HEADERS) Makefile

clean:
	$(RM) *.[oa] $(LIBNVRAM_SHARED)* $(LIBNVRAM_SHARED_FULLNAME)* 

