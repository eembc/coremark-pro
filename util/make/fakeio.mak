#  File: util/make/gcc.mak
#	GCC Tool Definitions, Host Compile and Run
#

# RUN OPTIONS SECTION
# Build or run options (i.e. profiling, simulation)

# Enable profiling with 'yes'. All other strings disable profiling.
ifndef (DO_PROFILE)
DO_PROFILE=no
endif
ifndef (DO_VALGRIND)
DO_VALGRIND=no
endif
ifndef (DO_MICA)
DO_MICA=no
endif

# ARCHITECTURE SECTION
# Any specific options (i.e. cpu, fpu)

# SYSTEM ENVIRONMENT SECTION

# Variable: TOOLS
#	Tools Root Directory
TOOLS	= /usr
# For Solaris
#TOOLS	= /usr/local

# Tools Executables, Output File Flag and Output File Types

# NOTE :	Spacing between option and values can be compiler dependant.
#		The following is a trick to ensure that a space follows the -o flag. 
#		Do not remove the line continuation backslash or the following blank
#		line.

# Variable: CC
#	name of the compiler
CC		= $(TOOLS)/bin/gcc
# Solaris : /usr/ccs/bin/as requires space after -o passed from gcc.
#OBJOUT = -o \#
OBJOUT	= -o
COBJT	= -c
CINCD	= -I
CDEFN	= -D
OEXT = .o

# Variable: CC
#	name of the assembler (if needed)
AS		= $(TOOLS)/bin/as

# Variable: CC
#	name of the linker
LD		= $(TOOLS)/bin/gcc
EXEOUT	= -o
EXE		= .exe

# Variable: CC
#	name of the librarian
AR		= $(TOOLS)/bin/ar
LIBTYPE	= .a
LIBOUT	= 

# Some Tool Chains require specific perl version. 
# makefile default setting can be overridden here.
#PERL=`which perl`


# COMPILER SECTION

# Variable: INCLUDE
# You may need to override the Environment variable INCLUDE.
# INCLUDE is used by most compilers, and should not 
# be passed to the compiler in the makefile.
INCLUDE = $(TOOLS)/include

# -c             compile but do not link
# -o             specify the output file name
# -march=i486    generate code for the intel 486
# -O0			 Do not optimize
# -O2			 Optimize for speed

COMPILER_FLAGS	= -O2 $(CDEFN)NDEBUG $(CDEFN)HOST_EXAMPLE_CODE=1 -std=gnu99 -mpc64
COMPILER_NOOPT	= -g -O0 $(CDEFN)NDEBUG $(CDEFN)HOST_EXAMPLE_CODE=1 -mpc64
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
        -Wall -Wno-long-long -fsigned-char -Wno-unused

# Additional include files not in dependancies or system include.
COMPILER_INCLUDES = 
# Override harness thincs, make sure you take care of the harness paths
#THINCS=

#Variable: COMPILER_DEFINES
# Optional - Passed to compiler, here or in makefile to override THCFG defines.
COMPILER_DEFINES += HAVE_SYS_STAT_H=1 USE_NATIVE_PTHREAD=1 GCC_INLINE_MACRO=1 NO_RESTRICT_QUALIFIER=1 FAKE_FILEIO=1
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

# Variable: LINKER_LAST
#	Add libraries that need to be last on the linker command line here.
# Some linkers do not re-scan libraries, and require some libraries 
# to be placed last on the command line to resolve references.
# some linkers require -lrt since they do not include realtime clock functions by default.
ifeq (,$(findstring lpthread,$(LINKER_LAST)))
 LINKER_LAST	+= -lm -lpthread 
 ifeq ($(PLATFORM),cygwin)
  LINKER_FLAGS	+= -Wl,--stack,33554432 
 else
  LINKER_LAST 	+= -lrt
 endif
endif

LINKER_INCLUDES	= 

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

#FLAG: PGOINI
#	If set, this flag will cause the code to be compiled for generating profile information.
#	In addition, the flag -pgo=1 is added to the command line to select the training data set.
ifdef PGOINI
 CFLAGS	+= -DTRAINING=1 -fprofile-generate
 LINKER_FLAGS += -fprofile-generate
 CERT_CMD += -pgo=1
endif

#FLAG: PGOUSE
#	If set, this flag will cause the compiler to use profile information generated by a previous run.
#	Please note - gcc profile feedback generation is far from fault free, especially in a parallel environment. This is for expert users only.
#	If using gcc 4.3 or above, you can add -Wcoverage-mismatch to compiler flags to igore profile errors istead of failing the build.
ifdef PGOUSE
 COMPILER_FLAGS	+= -fprofile-use 
 LINKER_FLAGS += -fprofile-use
endif

PROFILE_FILES = *.gc??

ARFLAGS = $(LIBRARY_FLAGS)
LIBRARY     = $(AR) $(ARFLAGS)
LIBRARY_LAST =


