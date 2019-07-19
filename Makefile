#this file only exists to set global make options
SHELL=bash
PERL=perl
TOPDIR:=$(shell pwd)/
SDATESTAMP := `date +%y%j:%H:%M:%S`
ifdef REBUILD
FORCE_REBUILD=force_rebuild
endif
ifdef SELECT_PRESET
FORCE_REBUILD=force_rebuild
endif
export 
ifndef TARGET
# Try auto detection for common self hosted platforms
	UNAME=$(shell if [[ `uname 2> /dev/null` ]] ; then uname ; fi)
	ifneq (,$(findstring CYGWIN,$(UNAME)))
		DEFAULT_TARGET=cygwin
	endif
	ifneq (,$(findstring Linux,$(UNAME)))
		MACHINE=$(shell uname -m)
		ifneq (,$(findstring 64,$(MACHINE)))
			DEFAULT_TARGET=linux64
		else
			ifneq (,$(findstring x86,$(MACHINE)))
				DEFAULT_TARGET=linux32-x86
			else
				DEFAULT_TARGET=linux
			endif
		endif
	endif
else
	DEFAULT_TARGET=$(TARGET)
endif
ifndef DEFAULT_TARGET
$(error ERROR: Must define TARGET, and util/make/TARGET.mak must exist!)
endif

#Phony target to force "all" rule to be invoked if no target on command line.
.PHONY: all
all: check_target 
	$(MAKE) -I$(TOPDIR)util/make -f Makefile.mak all

.PHONY: release-%
release-%: 
	$(MAKE) -I$(TOPDIR)util/make -f Makefile.mak TARGET=$(DEFAULT_TARGET) release-$*

.PHONY: release
release: 
	$(MAKE) -I$(TOPDIR)util/make -f Makefile.mak TARGET=$(DEFAULT_TARGET) release

# TODO ptorelli this is way out of date...
.PHONY: help
help: 
	@echo "Valid Targets:"
	@echo "help - This message."
	@echo "all - build and run all workloads (must define TARGET=<target>)"
	@echo "wbuild-<workload name> - build a specific workload"
	@echo "wrun-<workload name> - run a specific workload"
	@echo "tool - Run the workload creation tool (x86 only)"
	@echo "create-workloads - Parse xml in workloads/xml folder"
	@echo "release - Create release tarballs (RSTAMP=<tag> for tag)."

.PHONY: distclean
distclean: 
	$(MAKE) -I$(TOPDIR)util/make -f Makefile.mak TARGET=$(DEFAULT_TARGET) distclean

kernels_with_docs = md5 oa/rotatev2 video/x264 networking/ippktcheck networking/tcp \
	filters_v2/rgbcmyk03 networking/ipres video/mp2decode automotive/aifftr01 \
	automotive/basefp01 automotive/iirflt01 automotive/idctrn01 \
	automotive/a2time01 automotive/bitmnp01 automotive/cacheb01 \
	automotive/aifirf01 automotive/aiifft01 automotive/canrdr01 \
	automotive/matrix01 automotive/pntrch01 automotive/puwmod01 \
	automotive/rspeed01 automotive/tblook01 automotive/ttsprk01
	
workload_docs = -i workloads
kernel_docs=$(addprefix -i benchmarks/,$(kernels_with_docs))
kernel_doc=$(addprefix benchmarks/,$(kernels_with_docs))
	
.PHONY: workloads/sets/workload-defs%
workloads/sets/workload-defs%:

COMPILED_LISTS := $(wildcard workloads/sets/workload-defs.*) $(wildcard workloads/sets/workload-joins.*)
include $(COMPILED_LISTS)

public_docs_filter = -xi workloads/template --exclude-source util/wld/workload_parser.pl
	
.PHONY: tool
tool:
	util/bin/architect.exe > tool.log 2>&1 &
	
#Target: create-workloads
#	Create new workloads from unused workload definition files in the workloads/xml folder
.PHONY: create-workloads
create-workloads:
	@cd workloads && \
	for xml in xml/*.xml ; do \
		name=`basename $$xml .xml` && \
		if [ ! -d $$name ] ; then \
			mkdir $$name && (cd $$name && $(PERL) ../../util/wld/workload_parser.pl ../$$xml && cd ..); \
		fi ;\
	done

#Target: recreate-workloads
#	Create or recreate workloads from workload definition files in the workloads/xml folder
.PHONY: recreate-workloads
recreate-workloads:
	cd workloads && \
	for xml in xml/*.xml ; do \
		name=`basename $$xml .xml` && \
		mkdir -p $$name && (cd $$name && $(PERL) ../../util/wld/workload_parser.pl ../$$xml && cd ..) && \
		if [ -f xml/$$name.txt ] ; then cp xml/$$name.txt $$name ; fi \
	; done

#Target: wcreate-workload
#	Force create specific workloads from workload definition files in the workloads/xml folder
.PHONY: wcreate-%
wcreate-%:
	cd workloads && \
	for xml in xml/$*.xml ; do \
		name=`basename $$xml .xml` && \
		mkdir -p $$name && \
			(cd $$name && echo "../../util/wld/workload_parser.pl ../$$xml" && \
			perl ../../util/wld/workload_parser.pl ../$$xml && cd ..); \
	done

.PHONY: wcreate-jni-%
wcreate-jni-%:
	cd workloads && \
	for xml in xml/$*.xml ; do \
		name=`basename $$xml .xml` && \
		mkdir -p $$name/jni && \
			(cd $$name && echo "../../util/wld/workload_parser.pl -jni ../$$xml" && \
			perl ../../util/wld/workload_parser.pl -jni ../$$xml && cd ..); \
	done

.PHONY: create-list-% print-lists printval-%
create-list-%:
	for WLD in $($*) ; do \
		$(MAKE) wcreate-$${WLD} ;\
	done
	
print-lists:
	@echo "Available sets:"
	@for sn in $(SET_DEFS) ; do echo "- $$sn" && $(MAKE) printval-SET_DESC_$$sn | grep Description ; done;
	@echo ""
	@echo "Use build-list-<set> to build, run-list-<set> to run"
	
printval-%:
	@[ "$($*)" != "" ] && echo "  Description - $($*)" || true


.PHONY: check_target
check_target:
	@[ "$(TARGET)" != "" ] || ( echo "Must set TARGET=<target> and util/make/<TARGET>.mak must exist!" && false )

#add a target to test for a new toolchain/platform and create files. perhaps interactive.	
.PHONY: configure
configure:
	echo "NYI"
	
.PHONY: check_files
check_files:
	@if test -f check.md5 ; then (md5sum --status -c check.md5 || ( md5sum -c check.md5 | grep -v OK && false ) ) ; fi

#unpack items folder
#foreach keyfile provided
#	decrypt file
#	unpack file
#foreach leftover encrypted file (ignore case when none)
#	warn about license
#cleanup
#unpack harness
.PHONY: install
install: check_programs check_keys
	@[ -f mith_${RSTAMP}.md5 ] || (echo "Missing required file [mith_${RSTAMP}.md5] to check integrity of the release." && false )
	@echo checking integrity of downloaded files
	@( md5sum -c mith_${RSTAMP}.md5 > md5.log 2>md5.err && rm md5.log md5.err ) || ( echo "ERROR!!! Bad download or missing files!!!" && cat md5.log && false )
	@echo unpacking item repository...
	@for f in mith_wld_${RSTAMP}*.tgz ; do tar -vzxf $$f > tar.list ; done
	@echo decrypting registered items...
	@for KEY in keys/*.keys ; do \
		MYKEY=`basename $$KEY .keys` && \
		ccdecrypt -f -k $$KEY items/$${MYKEY}.tgz.cpt ;\
	done
	@echo "Unpacking registered items..."
	@for TB in items/*.tgz ; do tar -vzxf $$TB >> tar.list ; done
	@for CPT in items/*.tgz.cpt ; do \
		( [ "$$CPT" != "items/*.tgz.cpt" ] && ITEM=`basename $$CPT .tgz.cpt` && \
		echo "* Please contact EEMBC to license the $$ITEM component of Multibench." ) || echo "All workloads decrypted correctly."  \
	; done
	@rm -rf items
	@echo unpacking harness...
	@tar -vzxf mith_lib_${RSTAMP}.tgz >> tar.list
	@rm tar.list

.PHONY: check_keys
check_keys:
	echo "detected $MACHTYPE"
	@[ -d keys ] || (echo "Could not find any keys! Please contact markus.levy@eembc.org for decryption keys" && false)

.PHONY: check_programs
check_programs:
	@tar --help > err.log 2>&1 || (echo "Could not find tar to unpack the distribution. Please install tar (see http://www.gnu.org/software/tar/ for source)." && false)
	@ccdecrypt --help > err.log 2>&1 || (echo "Could not find ccdecrypt to decrypt the distribution. Please install ccdecrypt (see http://ccrypt.sourceforge.net/ for source)." && false)
	@basename Makefile > err.log 2>&1 || (echo "Could not find basename utility. Please install the basename utility." && false)

	
%: check_files
	$(MAKE) -I$(TOPDIR)util/make -f Makefile.mak TARGET=$(DEFAULT_TARGET) $@

LICENSE_FP_BASE=/c/ut/license_fp
LICENSE_FP_C=$(LICENSE_FP_BASE).c
LICENSE_ALL_BASE=/c/ut/license
LICENSE_ALL_C=$(LICENSE_FP_BASE).c
	
.PHONY: update_license_fp
update_license_fp:	$(LICENSE_FP_C) 
	export LICENSE_FILE=$(LICENSE_FP_C) && \
	export LICENSE_LEN=`wc -l $$LICENSE_FILE | cut -f1 -d ' '` && \
	for f in $(fp1-dirs) ; do \
		for fn in `find $$f -type f -name \*.[ch]` ; do \
			/c/ut/add_license.sh $$fn ; \
		done ; \
	done && \	
	echo "DONE [$$LICENSE_FILE] [$$LICENSE_LEN]"

.PHONY: update_license_all
update_license_all:	$(LICENSE_ALL_C) 
	export LICENSE_FILE=$(LICENSE_ALL_C) && \
	export LICENSE_LEN=`wc -l $$LICENSE_FILE | cut -f1 -d ' '` && \
	for f in workloads ; do \
		for fn in `find $$f -type f -name \*.[ch]` ; do \
			/c/ut/add_license.sh $$fn ; \
			perl util/perl/stripper.pl \
			  "Copyright [\w\(\)\s\-]* by the EDN Embedded Microprocessor" \
			  "Other Copyright Notice" \
			  $$fn > /c/tmp/stripped.c && mv /c/tmp/stripped.c $$fn && echo "- stripped $$fn" || echo "- strip not needed for $$fn" ; \
		done ; \
	done && \
	echo "DONE [$$LICENSE_FILE] [$$LICENSE_LEN]"

.PHONY: print-files-%
print-files-%:
	for f in $@-dirs ; do find $f -type f -name \*.[ch]; done
	
check:
	md5sum -c coremarkpro.md5
