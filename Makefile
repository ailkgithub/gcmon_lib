#-----------------------------------------
# OS: target operation system, currently support: Linux, Solaris
#-----------------------------------------
OS = Linux

#-----------------------------------------
# gcmon compile mode support: debug, release
#-----------------------------------------
TYPE = release

#-----------------------------------------
# 32bit or 64bit operating system
#-----------------------------------------
BIT = 64

#-----------------------------------------
# output dirs
#-----------------------------------------
OUT_DIR = output

#-----------------------------------------
# include dirs and source dirs
#-----------------------------------------
INC_DIR = ./gcmon/include
SRC_DIR = ./gcmon

#-----------------------------------------
# gcmon library properties
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

CPPFLAGS := $(_PIC) -I$(INC_DIR) -I$(INC_DIR)/hotspot/linux -I$(SRC_DIR)

ifeq ($(OS), Linux)
    CPPFLAGS+= -DLINUX
else
ifeq ($(OS), Solaris)
    CPPFLAGS+= -DSOLARIS
endif
endif

ifeq ($(TYPE), release)
    CPPFLAGS+= -DNDEBUG -O3
else
ifeq ($(TYPE), debug)
    CPPFLAGS+= -ggdb -DDEBUG
endif
endif

ifeq ($(BIT), 64)
    CPPFLAGS+= -DOS_64BIT -m64
else
ifeq ($(BIT), 32)
    CPPFLAGS+= -m32
endif
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

all : $(OBJS)
	mkdir -p $(OUT_DIR)/$(OS)$(BIT)
	ar crv $(OUT_DIR)/$(OS)$(BIT)/$(GCMON_NAME).a $(OBJS)
	ranlib $(OUT_DIR)/$(OS)$(BIT)/$(GCMON_NAME).a  

	$(CC) $(CPPFLAGS) $(_MODE) $(OBJS) -o $(OUT_DIR)/$(OS)$(BIT)/$(GCMON_NAME).so

clean : 
	rm -rf $(OBJS)
	rm -rf $(OUT_DIR)
	