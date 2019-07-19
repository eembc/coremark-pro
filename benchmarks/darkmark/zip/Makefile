MYNAME = zip
include common.mak
ifndef DIR_TARGET
SHELL=bash
TOPDIR:=$(shell pwd)/../../../
%:
	$(MAKE) -I$(TOPDIR)util/make TARGET=$(TARGET) $@
else	
endif

ZLIB = zlib-1.2.8
KERNEL_DEFINES=-DZLIB_COMPAT_ALL -DZLIB_ANSI 
TOPME = $(TOPDIR)benchmarks/darkmark/zip

S_DIRS = $(addprefix $(TOPME)/,$(ZLIB)) 
INC_DIRS += $(S_DIRS) 
vpath %.c $(TOPME) $(S_DIRS)
ZLIB_SRC=adler32.c crc32.c deflate.c infback.c inffast.c inflate.c inftrees.c \
	trees.c zutil.c compress.c uncompr.c gzclose.c gzlib.c 
SRC_C_FILES = zip_darkmark.c $(ZLIB_SRC)
SRC_FILES = $(basename $(SRC_C_FILES)) 
OBJ_FILES = $(SRC_C_FILES:.c=$(OEXT))

KERNEL_DEFINES=$(CDEFN)MITH_MEMORY_ONLY_VERSION=1

.PHONY: build
build: done.build

done.build: $(OBJ_FILES)
	touch done.build

