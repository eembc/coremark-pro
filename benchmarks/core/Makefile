MYNAME = core
include common.mak

TOPME = $(TOPDIR)benchmarks/core
vpath %.c $(TOPME)
vpath %.h $(TOPME)

INC_DIRS += $(TOPDIR)mith/al/include 
KERNEL_DEFINES=$(CDEFN)USE_CRC_TABLE=1 

.PHONY: build
build: done.build

SRC_FILES = core_mith core_matrix core_list_join core_state core_util core_portme
OBJ_FILES = $(addsuffix $(OEXT),$(SRC_FILES))
C_FILES = $(addsuffix .c,$(SRC_FILES))
H_FILES = coremark.h core_portme.h core_mith.h

$(OBJ_FILES): $(H_FILES)

done.build: $(OBJ_FILES)
	touch done.build


