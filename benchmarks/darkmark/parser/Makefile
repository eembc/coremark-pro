MYNAME = parser
include common.mak

KERNEL_DEFINES=-DEZXML_NOMMAP=1  
TOPME = $(TOPDIR)benchmarks/darkmark/parser

INC_DIRS += $(TOPME)
vpath %.c $(TOPME)
SRC_C_FILES = parser.c ezxml.c 
SRC_FILES = $(basename $(SRC_C_FILES)) 
OBJ_FILES = $(SRC_C_FILES:.c=$(OEXT))

.PHONY: build
build: done.build

done.build: $(OBJ_FILES)
	touch done.build

