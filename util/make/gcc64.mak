#  File: util/make/gcc64.mak
#	GCC Tool Definitions, Host Compile and Run, for 64b system
#
#  EEMBC : Technical Advisory Group (TechTAG)
#------------------------------------------------------------------------------
# Copyright (c) 1998-2004 by the EDN Embedded Microprocessor 
# Benchmark Consortium (EEMBC), Inc. All Rights Reserved.
#==============================================================================

# RUN OPTIONS SECTION
# Build or run options (i.e. profiling, simulation)

# Enable profiling with 'yes'. All other strings disable profiling.
ifndef (DO_PROFILE)
DO_PROFILE=no
endif
ifndef (DO_VALGRIND)
DO_VALGRIND=no
endif

# ARCHITECTURE SECTION
# Any specific options (i.e. cpu, fpu)

# SYSTEM ENVIRONMENT SECTION

# Tools Root Directory
TOOLS	= /usr
# For Solaris
#TOOLS	= /usr/local
# For Solaris
# (A shell which supports [ file1 -nt file2 ] for office, consumer)
#SHELL	= /bin/bash

# Tools Executables, Output File Flag and Output File Types

# NOTE :	Spacing between option and values can be compiler dependant.
#		The following is a trick to ensure that a space follows the -o flag. 
#		Do not remove the line continuation backslash or the following blank
#		line.
#		OBJOUT = -o \

# Variable: CC
#	name of the compiler
CC		= $(TOOLS)/bin/gcc
# Solaris: /usr/ccs/bin/as requires space after -o passed from gcc.
#OBJOUT = -o \#
OBJOUT	= -o
COBJT	= -c
CINCD	= -I
CDEFN	= -D
OEXT = .o

AS		= $(TOOLS)/bin/as

LD		= $(TOOLS)/bin/gcc
EXEOUT	= -o
EXE		= .exe

AR		= $(TOOLS)/bin/ar
LIBTYPE	= .a
LIBOUT	= 

# Some Tool Chains require specific perl version. 
# makefile default setting can be overridden here.
#PERL=`which perl`


# COMPILER SECTION

# You may need to override the Environment variable INCLUDE.
# INCLUDE is used by most compilers, and should not 
# be passed to the compiler in the makefile.
INCLUDE = $(TOOLS)/include

# -c             compile but do not link
# -o             specify the output file name
# -march=i486    generate code for the intel 486
# -O0			 Do not optimize
# -O2			 Optimize for speed

COMPILER_FLAGS	= -g -O2 $(CDEFN)NDEBUG $(CDEFN)HOST_EXAMPLE_CODE=1 -std=gnu99  
COMPILER_NOOPT	= -O0 -g $(CDEFN)NDEBUG $(CDEFN)HOST_EXAMPLE_CODE=1
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

# -ansi          Support all ANSI standard C programs. 
#                Turns off most of the GNU extensions
# -pedantic      Issue all the warnings demanded by strict ANSI standard C;
#                reject all programs that use forbidden extensions. 
# -fno-asm       do not allow the 'asm' keyword.  Eg. no inline asembly
# -fsigned-char  use signed characters
WARNING_OPTIONS	=	\
        -Wall -Wno-long-long -fno-asm -fsigned-char 

# Additional include files not in dependancies or system include.
COMPILER_INCLUDES = 
# Override harness thincs, make sure you take care of the harness paths
#THINCS=

#Variable: COMPILER_DEFINES
# Optional - Passed to compiler, here or in makefile to override THCFG defines.
# This flag contains the only difference from gcc for 32b platform, defining size of long and pointer types.
# It is also possible to simply define these in the th_cfg.h file for the platform.
COMPILER_DEFINES += HAVE_SYS_STAT_H=1 USE_NATIVE_PTHREAD=1 GCC_INLINE_MACRO=1 NO_RESTRICT_QUALIFIER=1
COMPILER_DEFINES += EE_SIZEOF_LONG=8 EE_SIZEOF_PTR=8 EE_PTR_ALIGN=8
ifeq ($(DO_MICA),yes)
COMPILER_DEFINES += DO_MICA=1
endif
# For Solaris, and Big Endian Targets, using 0/1 also allows support for
# files that do not have EEMBC includes. (Don't quote the string)
#COMPILER_DEFINES += EE_BIG_ENDIAN=1 EE_LITTLE_ENDIAN=0
COMPILER_DEFS = $(addprefix $(CDEFN),$(COMPILER_DEFINES))
PLATFORM_DEFS = $(addprefix $(CDEFN),$(PLATFORM_DEFINES))

# ASSEMBLER SECTION

ASSEMBLER_FLAGS		= 
ASSEMBLER_INCLUDES	=

# LINKER SECTION
# -lm is optional. Some linkers (linux gcc) do not include math library by default.
LINKER_FLAGS	+=  
ifeq ($(PLATFORM),cygwin)
LINKER_FLAGS	+= -Wl,--stack,33554432 
endif

LINKER_INCLUDES	= 
# Some linkers do not re-scan libraries, and require some libraries 
# to be placed last on the command line to resolve references.
# some linkers require -lrt since they do not include realtime clock functions by default.
ifneq ($(PLATFORM),cygwin)
LINKER_LAST 	+= -lm -lrt -lpthread
endif

# LIBRARIAN SECTION
LIBRARY_FLAGS	= scr

# SIZE SECTION
SIZE	= $(TOOLS)/bin/size
SIZE_FLAGS		= 

# CONTROL SECTION

ifeq ($(DO_PROFILE),yes)
	# Make PG overrides
	DIR_BUILD	= $(DIR_TARGET)/obj-pg
	DIR_IMG		= $(DIR_TARGET)/bin-pg
	DIR_LOG		= $(DIR_TARGET)/logs-pg

	COMPILER_FLAGS += -pg
	LINKER_FLAGS += -pg
else
	ifeq ($(DO_VALGRIND),yes)
		DIR_LOG	= $(DIR_TARGET)/logs-vg
		COMPILER_FLAGS += -g
	endif
endif
ALL_TARGETS		= $(EXTRA_TARGETS_S) mkdir targets run results $(EXTRA_TARGETS_F)

#PGO options
ifeq ($(TRAINING),yes)
 COMPILER_FLAGS	+= -DTRAINING=1 -fprofile-generate
 LINKER_FLAGS += -fprofile-generate
endif

ifeq ($(TRAINING_UNIQUE),yes)
 COMPILER_FLAGS	+= -DTRAINING=1 -DTRAINING_UNIQUE=1 -fprofile-generate
 LINKER_FLAGS += -fprofile-generate
endif

ifeq ($(USEPGO),yes)
 COMPILER_FLAGS	+= -fprofile-use
 LINKER_FLAGS += -fprofile-use
endif

ARFLAGS = $(LIBRARY_FLAGS)
LIBRARY     = $(AR) $(ARFLAGS)
LIBRARY_LAST =


