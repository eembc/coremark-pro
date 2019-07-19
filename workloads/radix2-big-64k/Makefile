MYNAME = radix2-big-64k
include common.mak


export SELECT_PRESET_ID=3
export SELECT_PRESET_NAME=data3_big

ifndef SELECT_PRESET_ID
USE_PRESET=FALSE
endif
MYDIR = radix2-big-64k
IMAGE = $(DIR_IMG)/radix2-big-64k$(EXE)
ITEMS = \
	fp/fft_radix2 \

radix2-big-64k$(OEXT): radix2-big-64k.c

MYOBJD = radix2-big-64k$(OEXT)
MYOBJS_WORKLOAD = radix2-big-64k$(OEXT)

ifdef PRECISION
ifeq ($(PRECISION),DP)
MAKEFLAGS+= WORKLOAD_DEFINES=-DUSE_FP64
endif
ifeq ($(PRECISION),SP)
MAKEFLAGS+= WORKLOAD_DEFINES=-DUSE_FP32
endif
KBUILD=$(addsuffix /$(PRECISION),$(ITEMS))
KMPATH=../
else
KBUILD=$(ITEMS)
endif
MYDIRS = $(addprefix $(DIR_BENCH)/,$(KBUILD))
S_DIRS = $(addprefix $(TOPDIR)benchmarks,$(ITEMS))
MYOBJD += $(addsuffix /done.build,$(MYDIRS))
MYOBJS = $(MYOBJS_WORKLOAD) $(addsuffix /*$(OEXT),$(MYDIRS))
TOPME = $(TOPDIR)workloads/$(MYDIR)
vpath %.c $(TOPME) $(S_DIRS)
vpath %$(LIBTYPE) $(TOPME) $(MYDIRS)
INC_DIRS += $(TOPDIR)mith/al/include 

ifdef SELECT_PRESET
ifneq ($(USE_PRESET),FALSE)
ITEMS+=$(SELECT_PRESET) 
preset_defines=$(foreach item,$(ITEMS),$(TOPDIR)benchmarks/$(item)/preset.mak )
-include $(preset_defines)
MYOBJS = $(MYOBJS_WORKLOAD) $(KERNEL_OBJS)
endif
endif

include $(TOPME)/$(MYNAME).opt

.PHONY: dirs
dirs: $(MYDIRS)

.PHONY: build
build: done.build $(DO_DATA_COPY)

done.build: $(IMAGE)
	@echo " $(DATESTAMP) $(MYNAME)" >> $(LOG_PROGRESS)
	@echo "  wld-flags: $(BENCHCFLAGS)" >> $(LOG_PROGRESS)
	touch done.build

$(IMAGE): $(MYDIRS) $(MYOBJD) $(LIB_TH)
	$(LD) $(LINKER_FLAGS) $(EXEOUT)$(IMAGE_NAME) $(MYOBJS_NAME) $(BENCH_LIBS) $(LIB_TH_NAME) $(LINKER_LAST)
	
$(MYDIRS):
	$(MDIR) $@

$(DIR_BENCH)/%/done.build: $(FORCE_REBUILD)
	cd $(DIR_BENCH)/$* && $(MAKE) -f $(TOPDIR)benchmarks/$*/$(KMPATH)Makefile build

