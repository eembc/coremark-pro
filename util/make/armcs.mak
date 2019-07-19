#  File : util/make/gcc-cross-linux.mak
#	GCC Tool Definitions, Host Compile and Run, cross from host to a linux target
#
#  EEMBC : Technical Advisory Group (TechTAG)
#------------------------------------------------------------------------------
# Copyright (c) 1998-2007 by the EDN Embedded Microprocessor 
# Benchmark Consortium (EEMBC), Inc. All Rights Reserved.
#==============================================================================

# RUN OPTIONS SECTION
# Build or run options (i.e. profiling, simulation)

# SYSTEM ENVIRONMENT SECTION

# Tools Root Directory
TOOLS = /opt/arm-2011.03/arm-none-linux-gnueabi
TPREF = /opt/arm-2011.03/bin/arm-none-linux-gnueabi-
# And this shows compiling to a big endian mips linux
# TOOLS	= /opt/crosstool/gcc-3.3.6-glibc-2.3.2/mips-unknown-linux-gnu
# TPREF = mips-unknown-linux-gnu-
# * remember to set big endian flags below for this setting!
#SHELL	= /bin/bash

# Tools Executables, Output File Flag and Output File Types

# Variable: CC
#	name of the compiler
CC		= $(TPREF)gcc
# Solaris: /usr/ccs/bin/as requires space after -o passed from gcc.
OBJOUT	= -o 
COBJT	= -c 
CINCD	= -I 
CDEFN	= -D 
OEXT = .o

AS		= $(TPREF)as

LD		= $(TPREF)gcc
EXEOUT	= -o 
EXE		= .exe

AR		= $(TPREF)ar
LIBTYPE	= .a
LIBOUT	=  

# Some Tool Chains require specific perl version. 
# makefile default setting can be overridden here.
#PERL=`which perl`

# COMPILER SECTION

INCLUDE = $(TOOLS)/include

COMPILER_FLAGS	= -g $(CDEFN)NDEBUG $(CDEFN)HOST_EXAMPLE_CODE=1 -static -O3 -mfpu=neon -mfloat-abi=softfp -O3 -ffast-math $(CDEFN)RESTRICT=__restrict
COMPILER_NOOPT	= -O0 $(CDEFN)NDEBUG $(CDEFN)HOST_EXAMPLE_CODE=1
COMPILER_DEBUG	= -O0 -g $(CDEFN)HOST_EXAMPLE_CODE=1 -DBMDEBUG=1 -DTHDEBUG=1
PACK_OPTS = 

#Variable: CFLAGS 
#	Options for the compiler.
ifdef DDB
 CFLAGS = $(COMPILER_DEBUG) $(COMPILER_DEFS) $(PLATFORM_DEFS) $(PACK_OPTS)
else
 ifdef DDN
  CFLAGS = $(COMPILER_NOOPT) $(COMPILER_DEFS) $(PLATFORM_DEFS) $(PACK_OPTS) 
 else
  CFLAGS = $(COMPILER_FLAGS) $(COMPILER_DEFS) $(PLATFORM_DEFS) $(PACK_OPTS)
 endif
endif
ifdef DDT
 CFLAGS += -DTHDEBUG=1
endif

WARNING_OPTIONS	=	\
        -Wall -Wno-long-long -fno-asm -fsigned-char 

# Additional include files not in dependancies or system include.
COMPILER_INCLUDES = 
# Override harness thincs, make sure you take care of the harness paths
#THINCS=

#Variable: COMPILER_DEFINES
# Optional - Passed to compiler, here or in makefile to override THCFG defines.
COMPILER_DEFINES += HAVE_SYS_STAT_H=1 USE_NATIVE_PTHREAD=1 GCC_INLINE_MACRO=1 NO_RESTRICT_QUALIFIER=1
# For Big Endian Targets, using 0/1 also allows support for
# files that do not have EEMBC includes. (Don't quote the string)
#COMPILER_DEFINES += EE_BIG_ENDIAN=1 EE_LITTLE_ENDIAN=0
COMPILER_DEFS = $(addprefix $(CDEFN),$(COMPILER_DEFINES))
PLATFORM_DEFS = $(addprefix $(CDEFN),$(PLATFORM_DEFINES))

# ASSEMBLER SECTION

ASSEMBLER_FLAGS		= 
ASSEMBLER_INCLUDES	=

# LINKER SECTION
# -lm is optional. Some linkers (linux gcc) do not include math library by default.
LINKER_FLAGS	+= -lm -lpthread 

LINKER_INCLUDES	= 
# Some linkers do not re-scan libraries, and require some libraries 
# to be placed last on the command line to resolve references.
# some linkers require -lrt since they do not include realtime clock functions by default.
LINKER_LAST 	+= -lrt

# LIBRARIAN SECTION
LIBRARY_FLAGS	= scr

# SIZE SECTION
SIZE	= $(TPREF)size
SIZE_FLAGS		= 

# CONTROL SECTION

ARFLAGS = $(LIBRARY_FLAGS)
LIBRARY     = $(AR) $(ARFLAGS)
LIBRARY_LAST =


# Target: Custom build rule
# For VC, transform path names to windows version of path
# Adding a rule:
# >%$(OEXT) : %.c
#	This new rules will be invoked to transform C files to objects.
#	>SRC_NAME=`cygpath -w $<` OUT_NAME=`cygpath -w $@` && \
#	First, make sure we have a path name that VC will accept
#	>$(CC) $(COBJT) $(CFLAGS) $(WARNING_OPTIONS) $(BENCH_CFLAGS) $(XCFLAGS) $(INC_FLAGS) $$SRC_NAME $(OBJOUT)"$$OUT_NAME"
#	Then compile SRC_NAME.
#	Note the use of $$ to make sure we get the value from the command line rather then the make file.
#
%$(OEXT) : %.c
	SRC_NAME=`cygpath -w $<` OUT_NAME=`cygpath -w $@` && \
	$(CC) $(COBJT) $(CFLAGS) $(WARNING_OPTIONS) $(BENCH_CFLAGS) $(XCFLAGS) $(INC_FLAGS) $$SRC_NAME $(OBJOUT)"$$OUT_NAME"

INC_DIRS_WIN = $(foreach dir,$(INC_DIRS),$(shell cygpath -m $(dir)))
INC_FLAGS = $(addprefix $(CINCD),$(INC_DIRS_WIN) $(COMPILER_INCLUDES))


LIB_TH_NAME = $(shell cygpath -m $(LIB_TH))
MYOBJS_NAME = `cygpath -m $(MYOBJS)`

# Target: Custom image name
IMAGE_NAME  ="`cygpath -m $(IMAGE)`"
UIMAGE_NAME  ="`cygpath -u $(IMAGE)`"
BASE_NAME = "'basename $(IMAGE)'"

#use -v1 for debug runs
RUN_FLAGS     = -v0 -i10
WLD_RUN_FLAGS = -v0 -i10
RUN           =

#-------------------------------------------- Added for Cross-compiled system runs ------------------------
# Cross compile from a cygwin host to a linux target.
#PLATFORM=csl441BLD202_O3_cortex-a8_neon
PLATFORM=csl452BLD-41_O3_static
#USER=qview
#HOST=localhost
#DIR_IMG = /home/qview/jflynn

BEST_CONCURRENCY=8
OVERLOAD_CONCURRENCY=32

# Flag: LOAD
#	In this sample we use scp, with the localhost as our target.
#
#	To make this work, the authorized_keys in our .ssh directory should be a copy of the id_rsa file, 
#	and the known_hosts should be updated with the fingerprint of the host.
#
#	Copy both files to the .ssh folder in your home directory on the target machine, and you should be 
#	able to ssh back and forth.
#
#	To update the known hosts, and test the login, execute from the shell:
#	> ssh <username>@<hostname>
#	The first time will cause the known_hosts to be updated. 
#	After this ssh <hostname> should login direct to prompt.
#	The name of the linux machine targeted in this sample is: alt
#
#	The function doscp will do the following:
#	- make a folder to copy to, matching the directory strcture on the current machine
#		* make sure you have permissions on the target to do that.
#		for example, if your host starts the build in /home/eembc/multibench,
#		make sure that the directory /home/eembc/multibench exists on the target, and that you have write permission there.
#	- copy the file to the directory on the target machine
#
#	Other options to load files to the target machine:
#	- Share the folder via nfs/samba on the target
#	- Share the folder via nfs/samba on the host
#	- Copy files via ftp
#	- Send files via serial link
#	- Build an image containing the files as a ram based file system, and flash that image
#	- Any other method that will allow the binary and data files to reach the target

LOAD = function doscp() { ssh -p 8001 $(USER)@$(HOST) mkdir -p `dirname $$1` ; scp -P 8001 $$1 $(USER)@$(HOST):$$1 ; cp $$1 $(DIR_TARGET)/$(MYNAME)$(EXE) ; } && doscp 
NEEDLOAD = no


# Flag: RUN
#	In this sample we use ssh for execution, with localhost as our target
#
#	See the comment regarding <LOAD> for setting this up.
#
#	Using ssh localhost preceding the binary name and run flags will cause 
#	the binary to be executed on the remote target via ssh, and the output 
#	to be output to the screen.
#
#	Note: 
#	We know the binary will be at the correct path since we placed it there using the LOAD command.
RUN = function dorun() { ssh -p 8001 $(USER)@$(HOST) "cd $(DIR_IMG) && $$@" ; } && dorun 

PLATFORM_DEFINES = 

# Flag: CMD_SEP
#	Use CMD_SEP if a separator is required before run flags (e.g. --)
CMD_SEP=

#FLAG: COPY_DATA
#	How should data be copied so that it is available for the executable?
#
#	Common ways include: 
#	- scp <target_ip>:<target_dir>(if target allows ssh connection)
#	- invoking a special command line tool to transfer the data
#	- copy (for file systems that do not support links)
#
#	In this case we will use scp.
#
#	For a linux machine, we could also have shared the build/data folder via samba or nfs,
#	and done a direct copy.
COPY_DATA=function doscp() { ssh -p 8001 $(USER)@$(HOST) mkdir -p $$2 ; scp -P8001 -rp $$1 $(USER)@$(HOST):$$2 ; } && doscp 

#Target: setup_out_dir
#	Setup output directory. 
#	Added to the build steps at start by defining EXTRA_TARGETS_S
EXTRA_TARGETS_S = setup_out_dir
setup_out_dir:
	ssh -p 8001 $(USER)@$(HOST) mkdir -p $(DIR_DAT_OUT)

