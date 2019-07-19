# File: util/make/linux.mak
# Optional specific files for specialized run and results extraction
PLATFORM=linux

ifndef TOOLCHAIN
TOOLCHAIN=gcc
endif

# Flag: LOAD
#	Use LOAD to define command used for loading the image to the target
LOAD =

# Flag: RUN
#	Use this flag to define a command needed to run the image
#
#	Example: if you need to run the image with a simulator, set this flag to point to the simulator executable
RUN			=
# Flag: RUN_FLAGS
#	Use this flag to supply flags before the image name and parameters
#
#	Example: if you need want to run the image using gdb, you probably need to set this flag to --args
RUN_FLAGS	=

ifeq ($(DO_PROFILE),yes)
	RUN_FLAGS			= 
	RUN = function dorun() { \
	BENCHNAME=`basename $$1 .exe` && \
	$$@ &&\
	gprof $$1 gmon.out > $(DIR_LOG)/$$BENCHNAME.gprof && \
	mv gmon.out $(DIR_LOG)/$$BENCHNAME.gmon ;\
	} && dorun 
endif
ifeq ($(DO_MICA),yes)
	RUN_FLAGS			= 
	RUN = function dorun() { \
	BENCHNAME=`basename $$1 .exe` && \
	pin -t micas -controller-start-int3 1 -controller-stop-int3 2 -- $$@ &&\
	mkdir -p $(DIR_LOG)/mica/$$BENCHNAME && mv *pin.out $(DIR_LOG)/mica/$$BENCHNAME ;\
	} && dorun 
endif
ifeq ($(DO_VALGRIND),yes)
	RUN_FLAGS		= 
	RUN			= valgrind --log-fd=1 --tool=memcheck -v --leak-check=yes --show-reachable=yes
endif

#Flag: PLATFORM_DEFINES 
#	Use PLATFORM_DEFINES to set platform specific compiler flags. E.g. set the timer resolution to millisecs with TIMER_RES_DIVIDER=1000
PLATFORM_DEFINES = 

# Flag: CMD_SEP
#	Use CMD_SEP if a separator is required before run flags (e.g. --)
CMD_SEP=
# Flag: XCMD
#	Define XCMD on command line to pass flag at run time to workloads, or here if you want to override the default in <util/make/common.mak>

#FLAG: COPY_DATA
#	How should data be copied so that it is available for the executable?
#	Common ways include: 
#	- scp <target_ip>:<target_dir>(if target allows ssh connection)
#	- invoking a special command line tool to transfer the data
#	- copy (for file systems that do not support links)
#	- link (for file systems where a simple symbolic link will work)
COPY_DATA=cp -Ru 
