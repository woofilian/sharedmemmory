#20101015 by maohw
#=============================================#
#TARGET: 			输出目标文件名，如果文件名中包含 .a 则输出为静态库，否则输出 elf格式可执行文件。
#EXPORT_INC_FILE:	输出头文件名，只有当输出目标为库(.a)的时候，才需要同时输出头文件(.h)。
#EXPORT_TARGET_DIR: 存放输出目标文件的目录。
#EXPORT_INC_DIR:    存放输出头文件的目录。
#CFLAGS:			编译选项。
#LDFLAGS:			链接选项。
#=============================================#

SRCPATH:=$(shell pwd | xargs basename)

SOURCES:=$(shell find . -type f -name "*.c" -o -name "*.cpp")
HSOURCE:=$(shell find . -type f -name "*.h")

SRC_C   = $(filter-out %.cpp,$(SOURCES))
SRC_CXX = $(filter-out %.c,$(SOURCES))

OBJECTS:=$(addsuffix .o, $(basename $(SOURCES)))

OBJ_C  :=$(addsuffix .o, $(basename $(SRC_C)))
OBJ_CXX:=$(addsuffix .o, $(basename $(SRC_CXX)))


DEPS    = $(OBJECTS:.o=.d)
DEP_C	= $(OBJ_C:.o=.d)
DEP_CXX	= $(OBJ_CXX:.o=.d)


COMPILE.c   := $(CC)   $(CFLAGS) -c
COMPILE.cxx := $(CXX)  $(CXXFLAGS) -c
LINK.c      := $(CC)   $(CFLAGS)  $(LDFLAGS)
LINK.cxx    := $(CXX)  $(CXXFLAGS) $(LDFLAGS)


DEP_OPT = $(shell if `$(CC) --version | grep "GCC" >/dev/null`; then \
		  echo "-MM -MP"; else echo "-M"; fi )
DEPEND      = $(CC)  $(DEP_OPT)  $(CFLAGS)
DEPEND.d	=$(subst -g ,,$(DEPEND))


ifneq ($(SRC_CXX),)
	__LINK := $(LINK.cxx)
else
	__LINK := $(LINK.c)
endif


ifndef TARGET
	ERROR0 := "TARGET IS NULL!" 
	ERR_F := 1
endif

ifndef ERR_F
TARGET_IS_LIB := $(findstring .a;,$(TARGET);)
endif

ifndef ERR_F
	ifneq ($(TARGET_IS_LIB),)
		TARGET_LIB := $(TARGET)
		ifndef EXPORT_INC_FILE
			ERROR1 := "EXPORT_INC_FILE IS NULL!"
			ERR_F := 1
		endif
	endif
	ifeq ($(TARGET_IS_LIB),)
		TARGET_ELF := $(TARGET) 
	endif
endif

ifdef ERR_F
define ERROR
	$(ERROR0)
	$(ERROR1)
endef
endif


.PHONY: default clean

ifndef ERR_F
deflaut:$(TARGET_ELF) $(TARGET_LIB)
else
deflaut:
	$(error $(ERROR))
endif

ifneq ($(TARGET_IS_LIB),)
$(TARGET_LIB):$(OBJECTS)
	@echo "========= is lib:$(TARGET_LIB) =========="
	$(AR) -r $@ $(OBJECTS) 
	$(INSTALL) $(TARGET) $(EXPORT_TARGET_DIR)	
	$(INSTALL) $(EXPORT_INC_FILE) $(EXPORT_INC_DIR)	
endif

ifeq ($(TARGET_IS_LIB),)
$(TARGET_ELF):$(OBJECTS)
	@echo "========= is bin:$(TARGET_LIB) =========="
	$(__LINK) -o $@  $(OBJECTS) $(LDFLAGS)
	#$(INSTALL) $(TARGET) $(EXPORT_TARGET_DIR)	
endif

ifndef NODEP
ifneq ($(DEPS),)
-include $(DEPS)
endif
endif

$(OBJ_C): %.o: %.c
	@echo "======== compile.c: $< -o $@=======" 
	$(COMPILE.c) $< -o $@

$(OBJ_CXX):%.o:%.cpp
	@echo "======== compile.cpp:$(OBJ_CXX) =======" 
	$(COMPILE.cxx) $< -o $@


$(DEP_C): %.d:%.c
	@echo -n $(dir $<) > $@
	@$(DEPEND.d) $< >> $@

$(DEP_CXX): %.d:%.cpp
	@echo -n $(dir $<) > $@
	@$(DEPEND.d) $< >> $@




clean:	
	-$(RM) $(TARGET) $(OBJECTS) $(DEPS) -rf




