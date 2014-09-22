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

ifeq (gcc, $(CC))
  _PIC = -fpic
  ifeq ($(BIT), 64)
    _MODE = -shared -m64
  endif
  ifeq ($(BIT), 32)
    _MODE = -shared -m32
  endif
endif

ifeq (Solaris, $(OS))
  CPPFLAGS := $(_OTHER) $(_PIC) -I$(INC_DIR) -I$(INC_DIR)/hotspot/linux -I$(SRC_DIR) -DSOLARIS -m64
endif

ifeq (Linux, $(OS))
    CPPFLAGS := $(_PIC) -I$(INC_DIR) -I$(INC_DIR)/hotspot/linux -I$(SRC_DIR) -DLINUX
  ifeq ($(BIT), 64)
     CPPFLAGS+= -DOS_64BIT -m64 
    _MODE = -shared -m64
  endif
  ifeq ($(BIT), 32)
    CPPFLAGS+= -m32
        _MODE = -shared -m32
  endif
endif

ifeq ($(TYPE), debug)
  CPPFLAGS+= -ggdb -DDEBUG
else
  ifeq ($(TYPE), release)       
      CPPFLAGS+= -DNDEBUG -O3 
  endif
endif

OBJS = $(patsubst %.c,%.o,$(wildcard gcmon/src/args/*.c))    \
       $(patsubst %.c,%.o,$(wildcard gcmon/src/gcmon/*.c))  \
       $(patsubst %.c,%.o,$(wildcard gcmon/src/list/*.c))   \
       $(patsubst %.c,%.o,$(wildcard gcmon/src/os/*.c))     \
       $(patsubst %.c,%.o,$(wildcard gcmon/src/perf/*.c))    \
       $(patsubst %.c,%.o,$(wildcard gcmon/src/rbtree/*.c))   \
       $(patsubst %.c,%.o,$(wildcard gcmon/src/sample/*.c))   \
       $(patsubst %.c,%.o,$(wildcard gcmon/src/share/*.c)) 

all : $(OBJS)
	mkdir -p $(OUT_DIR)/$(OS)/$(BIT)
	ar crv $(OUT_DIR)/$(OS)/$(BIT)/$(GCMON_NAME).a $(OBJS)
	ranlib $(OUT_DIR)/$(OS)/$(BIT)/$(GCMON_NAME).a  

	$(CC) -O2 $(_PIC) $(_MODE) $(OBJS) $(LIBS) -o $(OUT_DIR)/$(OS)/$(BIT)/$(GCMON_NAME).so

clean : 
	rm -rf $(OBJS)
	rm -rf $(OUT_DIR)
	