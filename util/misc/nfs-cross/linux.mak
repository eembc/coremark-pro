# Optional specific files for specialized run and results
PLATFORM=linux

ifndef TOOLCHAIN
TOOLCHAIN=gcc
endif

# CONTROL SECTION
# Flag: LOAD
#	Use LOAD to define command used for loading the image to the target
LOAD = perl $(TOPDIR)client.pl -load

ifeq ($(DO_PROFILE),yes)
	RUN_FLAGS			= $(RESULTS)
	RUN				= $(TCDIR)/gprof.sh
else
	ifeq ($(DO_VALGRIND),yes)
		RUN_FLAGS		= 
		RUN			= valgrind --log-fd=1 --tool=memcheck -v --leak-check=yes --show-reachable=yes
	else
		RUN_FLAGS		= 
		RUN			=  perl $(TOPDIR)client.pl -run
	endif
endif

PLATFORM_DEFINES = 

# Flag: CMD_SEP
#	Use CMD_SEP if a separator is required before run flags (e.g. --)
CMD_SEP=
# Flag: XCMD
#	Define XCMD on command line to pass flag at run time to workloads

#FLAG: COPY_DATA
#	How should data be copied so that it is available for the executable?
#	Common ways include: 
#	- scp <target_ip>:<target_dir>(if target allows ssh connection)
#	- invoking a special command line tool to transfer the data
#	- copy (for file systems that do not support links)
#	- link (for file systems where a simple symbolic link will work)
COPY_DATA=cp -Ru 
