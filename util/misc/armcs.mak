#  File : util/misc/armcs.mak
#	GCC for ARM, Prism Toolchain Definitions, Host Compile and Run, cross from host to a linux target
#
#  EEMBC : Technical Advisory Group (TechTAG)
#------------------------------------------------------------------------------
# Copyright (c) 1998-2009 by the EDN Embedded Microprocessor 
# Benchmark Consortium (EEMBC), Inc. All Rights Reserved.
#==============================================================================

# RUN OPTIONS SECTION
# Build or run options (i.e. profiling, simulation)

# SYSTEM ENVIRONMENT SECTION

# Tools Root Directory
# This example uses a cross compiler built with crosstool targeting x86 linux.
CODESOURCERY_DIR=d:/dev/tools/arm
APRISM_DIR=d:/dev/tools/A_Prism
TOOLS	= /d/dev/tools/arm
TPREF = arm-none-eabi-
# And this shows compiling to a big endian mips linux
# TOOLS	= /opt/crosstool/gcc-3.3.6-glibc-2.3.2/mips-unknown-linux-gnu
# TPREF = mips-unknown-linux-gnu-
# * remember to set big endian flags below for this setting!
#SHELL	= /bin/bash

# Tools Executables, Output File Flag and Output File Types

# Variable: CC
#	name of the compiler
CC		= $(TOOLS)/bin/$(TPREF)gcc
# Solaris: /usr/ccs/bin/as requires space after -o passed from gcc.
OBJOUT	= -o 
COBJT	= -c 
CINCD	= -I 
CDEFN	= -D 
OEXT = .o

AS		= $(TOOLS)/bin/$(TPREF)as

LD		= $(TOOLS)/bin/$(TPREF)gcc
EXEOUT	= -o 
EXE		= .axf

AR		= $(TOOLS)/bin/$(TPREF)ar
LIBTYPE	= .a
LIBOUT	= 

# Some Tool Chains require specific perl version. 
# makefile default setting can be overridden here.
#PERL=`which perl`

# COMPILER SECTION

INCLUDE = $(TOOLS)/include

COMPILER_FLAGS	= -g -O0 $(CDEFN)NDEBUG $(CDEFN)HOST_EXAMPLE_CODE=1
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
        -Wall

# Additional include files not in dependancies or system include.
COMPILER_INCLUDES = -I "$(APRISM_DIR)/pThreads/libpthread/include"
# Override harness thincs, make sure you take care of the harness paths
#THINCS=
CFLAGS += $(COMPILER_INCLUDES)
#Variable: COMPILER_DEFINES
# Optional - Passed to compiler, here or in makefile to override THCFG defines.
COMPILER_DEFINES += HAVE_SYS_STAT_H=1 USE_NATIVE_PTHREAD=1 GCC_INLINE_MACRO=1
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
PTHREADS="$(APRISM_DIR)/pThreads/pthread.a"
LINKER_FLAGS	+= -T "$(CODESOURCERY_DIR)/arm-none-eabi/lib/generic-hosted.ld" -lm 

LINKER_INCLUDES	= 
# Some linkers do not re-scan libraries, and require some libraries 
# to be placed last on the command line to resolve references.
# some linkers require -lrt since they do not include realtime clock functions by default.
#LINKER_LAST 	+= -lrt
LINKER_LAST	+= $(PTHREADS) -lm

# LIBRARIAN SECTION
LIBRARY_FLAGS	= scr

# SIZE SECTION
SIZE	= $(TOOLS)/bin/$(TPREF)size
SIZE_FLAGS		= 

# CONTROL SECTION

LIBRARY     = $(AR) $(ARFLAGS)
LIBRARY_LAST =

%$(OEXT) : %.c
	SRC_NAME=`cygpath -m $<` OUT_NAME=`cygpath -m $@` && \
	$(CC) $(COBJT) $(CFLAGS) $(WARNING_OPTIONS) $(BENCH_CFLAGS) $(XCFLAGS) $(INC_FLAGS) $$SRC_NAME $(OBJOUT)"$$OUT_NAME"

INC_DIRS_WIN = $(foreach dir,$(INC_DIRS),$(shell cygpath -m $(dir)))
INC_FLAGS = $(addprefix $(CINCD),$(INC_DIRS_WIN) $(COMPILER_INCLUDES))

LIB_TH_NAME = $(shell cygpath -m $(LIB_TH))
MYOBJS_NAME = `cygpath -m $(MYOBJS)`
IMAGE_NAME  ="`cygpath -m $(IMAGE)`"
