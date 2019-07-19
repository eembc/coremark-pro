MYNAME = loops
include common.mak

TOPME = $(TOPDIR)benchmarks/fp/loops
ifeq ($(PRECISION),SP)
S_DIRS = $(addprefix $(TOPME)/,ref-sp) 
else
S_DIRS = $(addprefix $(TOPME)/,ref) 
endif
vpath %.c $(TOPME) $(S_DIRS)
INC_DIRS += $(TOPME)

OBJ_FILES = loops$(OEXT) 1k$(OEXT) 10k$(OEXT) 100k$(OEXT) 1kdot$(OEXT) 10kdot$(OEXT) 100kdot$(OEXT) 100$(OEXT) 32$(OEXT)

PRECISION=DP

.PHONY: build
build: done.build

done.build: $(OBJ_FILES)
	touch done.build


