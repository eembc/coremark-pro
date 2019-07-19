MYNAME = linpack
include common.mak

TOPME = $(TOPDIR)benchmarks/fp/linpack
S_DIRS = $(addprefix $(TOPME)/,ref) 
vpath %.c $(TOPME) $(S_DIRS)
INC_DIRS += $(TOPME)

OBJ_FILES = linpack$(OEXT) 

ifndef PRECISION
PRECISION=DP
endif
ifeq ($(PRECISION),DP)
OBJ_FILES+=inputs_f64$(OEXT)
KERNEL_CFLAGS+=-DDP=1
endif 
ifeq ($(PRECISION),SP)
OBJ_FILES+=inputs_f32$(OEXT)
KERNEL_CFLAGS+=-DSP=1
endif 

.PHONY: build
build: done.build

done.build: $(OBJ_FILES)
	touch done.build


