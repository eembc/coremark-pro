#File: util/make/common.mak
#	Common flags used by the infrastructure
INC_FLAGS = $(addprefix $(CINCD),$(INC_DIRS) $(COMPILER_INCLUDES))
DIR_TARGET = $(TOPDIR)builds/$(PLATFORM)/$(TOOLCHAIN)
DIR_BUILD = $(DIR_TARGET)/obj
DIR_LOG = $(DIR_TARGET)/logs
DIR_IMG = $(DIR_TARGET)/bin
ifdef PGOINI
DIR_BUILD = $(DIR_TARGET)/obj/pgo
DIR_IMG = $(DIR_TARGET)/pgo
DIR_LOG = $(DIR_TARGET)/pgo/logs
endif
DIR_CERT = $(DIR_TARGET)/cert/$(DIRDATESTAMP)
DIR_CERT_TMP = $(DIR_TARGET)/cert/tmp
DIR_DAT = $(DIR_TARGET)/data
DIR_DAT_OUT = $(DIR_IMG)/data
DIR_WLD = $(DIR_BUILD)/workloads
DIR_BENCH = $(DIR_BUILD)/bench
LIB_TH = $(DIR_BUILD)/mith$(LIBTYPE)
LIB_TH_NAME = $(LIB_TH)
IMAGE_NAME = $(IMAGE)
MYOBJS_NAME = $(MYOBJS)
LOG_PROGRESS = $(DIR_LOG)/progress.log
COMMON_DIRS = $(TOPDIR)builds $(DIR_TARGET) $(DIR_BUILD) $(DIR_LOG) $(DIR_TARGET)/mith $(DIR_IMG) $(DIR_BENCH) $(DIR_WLD) $(DIR_DAT) $(DIR_DAT_OUT)
DATESTAMP = $(shell date +%y%j:%H:%M:%S)
DIRDATESTAMP = $(shell date +%y_%m_%d_%H_%M_%S)
RSTAMP := $(shell date +%y%j)
# Flag: INC_DIRS
#	Global directories that should be on the include path 
#	Benchmark specific include directories should be added in the benchmark specific makefile
INC_DIRS = $(TOPDIR)mith/include  $(TOPDIR)mith/al/include
RESLOG = $(DIR_LOG)/$(TARGET).$(TOOLCHAIN).log
WLD_PARSE = $(PERL) $(TOPDIR)util/perl/results_parser.pl

# Flag: BEST_CONCURRENCY 
#	Number of work items that run in parallel during certification step for maximum performance.
#	Should be defined in <platform>.mak, can be overridden in workload specific option file <workload name>.opt

# Flag: OVERLOADED_CONCURRENCY 
#	Number of work items that run in parallel during certification step for overloaded performance.
#	Should be defined in <platform>.mak, can be overridden in workload specific option file <workload name>.opt

# Default concurrency related switches:
#	- CONCURRNECY (set during automated certification process)
#	- XCMD concurrency during regular runs unless command line override
ifndef BEST_CONCURRENCY
BEST_CONCURRENCY_CONTEXTS=-c8
BEST_CONCURRENCY_WORKERS=-w8
endif
ifeq ($(CONCURRNCY),single)
CERT_CMD=-c1 -w1
endif
ifeq ($(CONCURRNCY),best)
CERT_CMD=$(BEST_CONCURRENCY)
endif
# Flag: XCMD
#	Command line flags to send to workloads.
ifndef XCMD
ifndef CERT_CMD
XCMD=
endif
endif

#commands to use
MDIR=mkdir -p
RMDIR=rm -rf
CPDIR=cp -rfu
RM=rm -f
CP=cp -f
CAT=cat
PERL=perl

#common make targets
.PHONY: clean distclean all dolog force_rebuild run results copy_mydata force print-%
#Target: print-%
#	To help in debugging problems with makefiles
print-%:
	@echo $* = $($*)  
	@echo [Defined at $(origin $*)] 

#Target: dolog
#	log date and current step to file, as well as flags used for step
dolog:
	@echo " $(DATESTAMP) $(MYNAME)" >> $(LOG_PROGRESS)
	@echo "  bench-flags: $(BENCHCFLAGS)" >> $(LOG_PROGRESS)

ifndef NEEDLOAD
NEEDLOAD=no
endif

#Target: load
#	load a binary image onto the target if LOAD is defined
#	override this target if one command is not convenient for loading.
#	If overriding, use $(IMAGE) as name of the image to load.
load:
	@if [ "$(NEEDLOAD)" != "no" ] ; then \
	echo "Loading $(IMAGE)" && \
	$(LOAD) $(IMAGE) ;\
	fi

#Target: run
#	run a binary image that has been loaded to the target
run: $(RUN_DEPS)
	@if ( [ "$(WLD_RUN_FLAGS)" != "$(RUN_FLAGS)" ] || [ "$(WLD_CMD_FLAGS)" != "" ] )  ; then echo "Overrides for $(IMAGE):" >> $(LOG_PROGRESS); fi
	@if [ "$(WLD_RUN_FLAGS)" != "$(RUN_FLAGS)" ] ; then echo "RUN_FLAGS=$(RUN_FLAGS)" >> $(LOG_PROGRESS); fi
	@if [ "$(WLD_CMD_FLAGS)" != "" ] ; then echo "WLD_CMD_FLAGS=$(WLD_CMD_FLAGS)" >> $(LOG_PROGRESS); fi
	$(WLD_RUN) $(WLD_RUN_FLAGS) $(IMAGE_NAME) $(CMD_SEP) $(CERT_CMD) $(XCMD) $(WLD_CMD_FLAGS) > $(DIR_LOG)/$(MYNAME).run.log 2>&1
	$(SIZE) $(SIZE_FLAGS) $(IMAGE_NAME) >  $(DIR_LOG)/$(MYNAME).size.log 

#Target: results
#	collect workload summary from a run log
results:
	$(WLD_PARSE) $(RESLOG) $(DIR_LOG)/$(MYNAME).run.log

#Target: copy_mydata
#	Copy data files to target directory
copy_mydata:
	@echo Copy input data.
	for cpdir in $(MYDATA) ; do $(COPY_DATA) $$cpdir $(DIR_DAT) ; done

#Target: copy_profile
#	Copy profile files from location they are in to location they are needed for compile
copy_profile: dirs
	for dn in $(ITEMS) ; do \
		$(CP) $(PROFILE_DIR)/bench/$$dn/$(PROFILE_FILES) $(DIR_BENCH)/$$dn ;\
	done

#Target: compilation .c->.o
#	Command to compile c files
#	
#Flag: CFLAGS
#	Compiler flags for C. 
#	Set in [toolchain].mak file
#
#Flag: WARNING_OPTIONS
#	Compiler options to report warnings
#	Set in [toolchain].mak file
#
#Flag: BENCH_CFLAGS
#	Specific benchmark/workload options
#	Set in the benchmark specific makefile
#
#Flag: XCFLAGS
#	Generic extra options for compile
#	Set in the command line e.g. make XCFLAGS=-g
#	
#Flag: INC_FLAGS
#	Include directories
#	Derived from <INC_DIRS>
#
#Flag: KERNEL_DEFINES
#	Allows a workload to set specific defines for a kernel

#%$(OEXT) : %.c $(EXTRA_DEPS)
#	$(CC) $(COBJT) $(CFLAGS) $(WARNING_OPTIONS) $(BENCH_CFLAGS) $(XCFLAGS) $(KERNEL_DEFINES) $(INC_FLAGS) $< $(OBJOUT) $@

%$(OEXT) : %.c $(EXTRA_DEPS)
	$(CC) $(COBJT) $(CFLAGS) $(WARNING_OPTIONS) $(BENCH_CFLAGS) $(XCFLAGS) $(KERNEL_DEFINES) $(WORKLOAD_DEFINES) $(INC_FLAGS) $< $(OBJOUT) $@

%$(OEXT) : %.cpp $(EXTRA_DEPS)
	$(CC) $(COBJT) $(CFLAGS) $(WARNING_OPTIONS) $(BENCH_CFLAGS) $(XCFLAGS) $(KERNEL_DEFINES) $(WORKLOAD_DEFINES) $(INC_FLAGS) $< $(OBJOUT) $@
	
# allow TARGET and TOOLCHAIN makefiles to override common targets and variables

#TARGET and TOOLCHAIN related vars
# Flag: TARGET
#	TARGET designates the target architecture.
#	It can be supplied on the command line (e.g. make TARGET=cygwin)
#	or in the environment, or can be set in common.mak 
ifndef TARGET
$(error ERROR: Must define TARGET, and util/make/TARGET.mak must exist!)
endif
include $(TARGET).mak

# Flag: TOOLCHAIN
#	TOOLCHAIN designates the target toolchain to use.
#	It can be supplied on the command line (e.g. make TARGET=cygwin TOOLCHAIN=gcc)
#	or in the environment, or can be set in common.mak or in the $TARGET.mak 
ifndef TOOLCHAIN
$(error ERROR: Must define TOOLCHAIN, and util/make/TOOLCHAIN.mak must exist!)
endif
include $(TOOLCHAIN).mak
