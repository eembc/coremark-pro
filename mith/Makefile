MYNAME=mith
include common.mak
MYDIRS=src al/src $(TOOL_AL_DIRS) $(PLATFORM_AL_DIRS)
TOPME=$(TOPDIR)$(MYNAME)
#set the search path for sources. If adding other files (e.g. asm files), make sure to add the relevant extension
#e.g add another line : vpath %.s $(TOPME) , if you placed .s files in a source directory.
vpath %.c $(TOPME)

#flags that are specific to the test harness should be applied here
BENCH_CFLAGS=

#setup sample abstraction layer sources if none were defined
ifndef PLATFORM_AL_SRCS
PLATFORM_AL_SRCS = $(wildcard $(TOPME)/al/src/*.c) 
endif

# Collect all the source files
TH_SRCS := $(wildcard $(TOPME)/src/*.c) $(TOOL_AL_SRCS) $(PLATFORM_AL_SRCS)
TH_HDRS := $(wildcard $(TOPME)/include/*.h) $(wildcard $(TOPME)/al/include/*.h) 
INC_DIRS += $(TOPME)/al/include
ifdef TARGET_EXTRA
-include $(TOPME)/al/$(TARGET_EXTRA)/th_extra.mak
TH_SRCS += $(wildcard $(TOPME)/al/$(TARGET_EXTRA)/*.c)
INC_DIRS += $(TOPME)/al/$(TARGET_EXTRA)
MYDIRS+=al/$(TARGET_EXTRA)
endif
#put all harness object into one directory
TH_SRCS_N = $(notdir  $(TH_SRCS))
TH_OBJS_O = $(TH_SRCS:.c=$(OEXT))
TH_OBJS = $(subst $(TOPME)/,,$(TH_OBJS_O))
EXTRA_DEPS=$(TH_HDRS)

all: build dolog

$(MYDIRS):
	$(MDIR) $@

.PHONY: build 
build: done.build

done.build: $(MYDIRS) $(LIB_TH)
	touch done.build
	
$(LIB_TH): $(TH_OBJS) $(TH_EXTRA_OBJS)
	$(LIBRARY) $(LIBOUT)$(LIB_TH_NAME) $(TH_OBJS) $(TH_EXTRA_OBJS) $(LIBRARY_LAST)

clean:
	$(RM) $(TH_OBJS) $(LIB_TH)

distclean: clean

.PHONY: print-%
print-%:
	@echo $* = $($*)  
	@echo [Defined at $(origin $*)] 

$(TH_OBJS) : $(TH_HDRS)	