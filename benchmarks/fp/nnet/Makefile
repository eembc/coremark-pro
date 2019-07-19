MYNAME = nnet
include common.mak

TOPME = $(TOPDIR)benchmarks/fp/nnet
ifeq ($(PRECISION),SP)
S_DIRS = $(addprefix $(TOPME)/,ref-sp) 
else
S_DIRS = $(addprefix $(TOPME)/,ref) 
endif
vpath %.c $(TOPME) $(S_DIRS)
INC_DIRS += $(TOPME)

OBJ_FILES = nnet$(OEXT) letters$(OEXT) 1letter$(OEXT)

.PHONY: build
build: done.build

done.build: $(OBJ_FILES)
	touch done.build


