#-----------------------------------------
# target operation system, currently support: Linux, Solaris
#-----------------------------------------
ifndef OS
    OS = Linux
endif


#-----------------------------------------
# gcmon compile mode support: debug, release
#-----------------------------------------
ifndef TYPE
    TYPE = release
endif


#-----------------------------------------
# 32bit or 64bit operating system: 32, 64
#-----------------------------------------
ifndef BIT
    BIT = 64
endif


#-----------------------------------------
# output dir
#-----------------------------------------
ifndef OUT_DIR
    OUT_DIR = output
endif


#-----------------------------------------
# include dirs and source dirs
#-----------------------------------------
INC_DIR = ./gcmon/include
SRC_DIR = ./gcmon

#-----------------------------------------
# gcmon library name
#-----------------------------------------
GCMON_NAME = libgcmon

#-----------------------------------------
# CC options: gcc
#-----------------------------------------
CC = gcc

ifeq ($(CC), gcc)
    _PIC = -fpic
    _MODE = -shared
endif

CFLAGS = $(_PIC) -I$(INC_DIR) -I$(INC_DIR)/hotspot/linux -I$(SRC_DIR)

ifeq ($(OS), Linux)
    CFLAGS += -DLINUX
endif
ifeq ($(OS), Solaris)
    CFLAGS += -DSOLARIS
endif

ifeq ($(TYPE), release)
    CFLAGS += -DNDEBUG -O3
endif
ifeq ($(TYPE), debug)
    CFLAGS += -ggdb -DDEBUG
endif

ifeq ($(BIT), 64)
    CFLAGS += -DOS_64BIT -m64
endif
ifeq ($(BIT), 32)
    CFLAGS += -m32
endif

OBJS = $(patsubst %.c,%.o,$(wildcard gcmon/src/args/*.c))    \
       $(patsubst %.c,%.o,$(wildcard gcmon/src/gcmon/*.c))   \
       $(patsubst %.c,%.o,$(wildcard gcmon/src/list/*.c))    \
       $(patsubst %.c,%.o,$(wildcard gcmon/src/os/*.c))      \
       $(patsubst %.c,%.o,$(wildcard gcmon/src/perf/*.c))    \
       $(patsubst %.c,%.o,$(wildcard gcmon/src/rbtree/*.c))  \
       $(patsubst %.c,%.o,$(wildcard gcmon/src/sample/*.c))  \
       $(patsubst %.c,%.o,$(wildcard gcmon/src/ana/*.c))     \
       $(patsubst %.c,%.o,$(wildcard gcmon/src/file/*.c))    \
       $(patsubst %.c,%.o,$(wildcard gcmon/src/share/*.c))   

TARGET_DIR = $(OUT_DIR)/$(OS)$(BIT)/$(TYPE)

all : $(OBJS)
	mkdir -p $(TARGET_DIR)
	ar crv $(TARGET_DIR)/$(GCMON_NAME).a $(OBJS)
	ranlib $(TARGET_DIR)/$(GCMON_NAME).a  

	$(CC) $(CFLAGS) $(_MODE) $(OBJS) -o $(TARGET_DIR)/$(GCMON_NAME).so
	
clean : 
	rm -rf $(OBJS)
	rm -rf $(OUT_DIR)
	