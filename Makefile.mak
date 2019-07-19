#Title: Makefile.mak
#	Main makefile for MITH
#
# Note: 
# to define specific workload groups, define the following
# <GROUP>_WORKLOADS = <dir1> <dir2> ...
# build-<GROUP> : <copy from build-wld>
include suite.mak
include common.mak
ifndef TAG
SVN_VERSION = $(shell svn info 2>/dev/null| grep Revision | sed -e "s/.*vision: //" | sed -e "s/\.//" || 'nosvninfo')
TAG = ${VTAG_BASE}.${SVN_VERSION}
#TAG = $(shell svn info 2>/dev/null| grep Revision | sed -e "s/.*vision: /${VTAG_BASE}/" | sed -e "s/\.//" || date +%y%m%d)
endif

export

#Target: all
#	Create all required dirs, build all images, and run all workloads
all: $(COMMON_DIRS) build run-all

$(COMMON_DIRS):
	@$(MDIR) $@

#Target: build
#	Build all workloads
.PHONY: build 
build: build-all

COMPILED_LISTS := $(wildcard workloads/sets/workload-defs.*) $(wildcard workloads/sets/workload-joins.*)
include $(COMPILED_LISTS)

VALID_WORKLOADS := $(wildcard workloads/*/Makefile)
WORKLOADS=$(dir $(VALID_WORKLOADS))
WORKLOAD_NAMES=$(subst workloads/,,$(WORKLOADS))
build-all: $(COMMON_DIRS) log-flags build-mith
	@echo "Starting build process for $(WORKLOAD_NAMES)"
	@for WLD in $(WORKLOAD_NAMES) ; do \
		$(MDIR) $(DIR_BUILD)/workloads/$$WLD && \
		$(MAKE) -C $(DIR_BUILD)/workloads/$$WLD -f $(TOPDIR)workloads/$${WLD}/Makefile build $(POST_BUILD);\
	done

.PHONY: $(LIB_TH)
$(LIB_TH): $(COMMON_DIRS)
	$(MDIR) $(DIR_BUILD)/mith
	$(MAKE) -C $(DIR_BUILD)/mith -f $(TOPDIR)mith/Makefile all

#Target: build-mith
#	Build the Multi Instance Test Harness library
.PHONY: build-mith
build-mith: $(LIB_TH)

#Target: wbuild-<workload dir name>
#	Build just the workload in a specific directory
.PHONY: wbuild-%
wbuild-%:  $(COMMON_DIRS) log-flags build-mith
	@for WLD in workloads/$* ; do \
		$(MDIR) $(DIR_BUILD)/$$WLD && \
		$(MAKE) -C $(DIR_BUILD)/$$WLD -f $(TOPDIR)$${WLD}/Makefile build $(POST_BUILD);\
	done
	
#Target: copy-profile-<workload dir name>
#	For each workload, copy the profile for the work items, and then the profile for the workload itself.
#	The <copy_profile> target is set in <common.mak> and relies on the workload definition file.
.PHONY: copy-profile-%
copy-profile-%:
	for WLD in workloads/$* ; do \
		$(MDIR) $(DIR_BUILD)/$$WLD && \
		$(MAKE) PROFILE_DIR=$(DIR_BUILD)/pgo -C $(DIR_BUILD)/$$WLD -f $(TOPDIR)$${WLD}/Makefile copy_profile && \
		$(CP) $(DIR_BUILD)/pgo/$$WLD/$(PROFILE_FILES) $(DIR_BUILD)/$$WLD ;\
	done

#Target: pgoinibuild-<workload dir name>
#	Build just the workload in a specific directory, preparing to generate profile data
#	Note - The profile files need to end up where they are accessible to the compiler for pgouse-build!
.PHONY: pgoinibuild-%
pgoinibuild-%:  
	$(MAKE) PGOINI=1 wbuild-$*

#Target: pgousebuild-<workload dir name>
#	Build just the workload in a specific directory, using profile data.
#	First makes sure that a profile training run is done, and copying over the profile data.
.PHONY: pgousebuild-%
pgousebuild-%: pgoinirun-% copy-profile-%
	$(MAKE) PGOUSE=1 wbuild-$*

#Target: run-all
#	Run all workloads, and collect results
run-all: build-all $(EXTRA_TARGETS_S) 
	@echo "$(DATESTAMP) Starting Run" >> $(LOG_PROGRESS)
	[ -f $(RESLOG) ] || $(CAT) util/perl/headings.txt > $(RESLOG)
	@echo "#Results for run started at $(SDATESTAMP) XCMD=$(XCMD)" >> $(RESLOG)
	for WLD in $(WORKLOADS) ; do \
		$(MAKE) -C $(DIR_IMG) -f $(TOPDIR)$${WLD}Makefile load &&	\
		$(MAKE) -C $(DIR_IMG) -f $(TOPDIR)$${WLD}Makefile run  &&	\
		$(MAKE) -C $(DIR_LOG) -f $(TOPDIR)$${WLD}Makefile results	\
		;\
	done

#Target: wrun-<workload dir name>
#	Run just the workload in a specific directory
.PHONY: wrun-%
wrun-%: wbuild-% $(EXTRA_TARGETS_S) 
	@echo "$(DATESTAMP) Starting Run" >> $(LOG_PROGRESS)
	[ -f $(RESLOG) ] || $(CAT) util/perl/headings.txt > $(RESLOG)
	@echo "#Results for run started at $(SDATESTAMP) XCMD=$(XCMD)" >> $(RESLOG)
	for WLD in workloads/$* ; do \
		$(MAKE) -C $(DIR_IMG) -f $(TOPDIR)$${WLD}/Makefile load &&	\
		$(MAKE) -C $(DIR_IMG) -f $(TOPDIR)$${WLD}/Makefile run	;	\
		$(MAKE) -C $(DIR_LOG) -f $(TOPDIR)$${WLD}/Makefile results	\
		;\
	done

.PHONY: wshell-%
wshell-%: wbuild-% $(EXTRA_TARGETS_S) 
	echo "echo \"$*$(EXE) \$$OPTS\"" ${SCRIPTNAME}
	echo "$*$(EXE) \$$OPTS > ./\$$SNAME/$*.log" ${SCRIPTNAME}
	echo "TIME=\`grep workloads ./\$$SNAME/$*.log | sed -e \"s/.*=//\"\` && echo \"$*, \$$TIME\" >> \$$SNAME.csv" ${SCRIPTNAME}
	
#Target: pgoinirun-<workload dir name>
#	Run just the workload(s) to generate profile data
#	For cross compile platforms if using gcc, make sure that the exact folder structure for the obj files exists, 
#	or use GCOV_PREFIX and GCOV_PREFIX_STRIP to get the profile in a more convenient location.
#	May want to override the <run> target in the platform definition, or the copy_profile target in this file for this to work depending on toolchain requirements.
.PHONY: pgoinirun-%
pgoinirun-%: pgoinibuild-% 
	$(MAKE) PGOINI=1 wrun-$*


#Target: pgouserun-<workload dir name>
#	This is a regular run, just here to get the dependencies for profile build correctly
.PHONY: pgouserun-%
pgouserun-%: pgousebuild-% 
	$(MAKE) PGOUSE=1 wrun-$*

#Target: run-list-<list name>
#	Run just the workloads in the list.
#
#  	Example: make run-list-RC1 will run all the workloads that are part of the RC1 workload set.
.PHONY: run-list-%
run-list-%:
	for WLD in $($*) ; do \
		$(MAKE) wrun-$${WLD} ;\
	done

.PHONY: shell-list-% 
shell-list-%:
	echo "#!/bin/bash" > $(DIR_IMG)/run-$*.sh
	chmod +x $(DIR_IMG)/run-$*.sh
	echo "if [ \"$1\" == \"-h\" ] ; then echo \"Usage: \$$0 <name> <options>\" && echo \" Name: Name for the results file and details folder.\" && echo \" Options: options to pass to all workloads,\" && echo \"  e.g. -v0 -i100 to run each workload with no verification for 100 iterations. \"&& exit ; fi" >> $(DIR_IMG)/run-$*.sh 
	echo "if [ \"\$$#\" == \"0\" ]; then export SNAME=summary ; else export SNAME=\$$1 && shift ; fi" >> $(DIR_IMG)/run-$*.sh
	cp util/perl/cert_mark.pl $(DIR_IMG)
	echo "echo \"NOTICE: This is an EEMBC benchmark. \"" >> $(DIR_IMG)/run-$*.sh
	echo "echo \"* Use and distribution of this benchmark and any results are subject to the EEMBC license agreement.\"" >> $(DIR_IMG)/run-$*.sh
	echo "export PATH=./bin/:./:\$$PATH" >> $(DIR_IMG)/run-$*.sh
	echo "echo \"Name,It/s\" > \$$SNAME.csv" >> $(DIR_IMG)/run-$*.sh
	echo "echo \"CFLAGS: $(CFLAGS)\" >> \$$SNAME.csv" >> $(DIR_IMG)/run-$*.sh
	echo "echo \"XCFLAGS: $(XCFLAGS)\" >> \$$SNAME.csv" >> $(DIR_IMG)/run-$*.sh
	echo "cat /proc/cpuinfo >> \$$SNAME.csv" >> $(DIR_IMG)/run-$*.sh
	echo "cat /proc/version >> \$$SNAME.csv" >> $(DIR_IMG)/run-$*.sh
	echo "echo \"\`date\`,performance runs\" >> \$$SNAME.csv" >> $(DIR_IMG)/run-$*.sh
	echo "if [ -z \"\$$1\" ] ; then export OPTS=\"-v0\" ; else export OPTS=\"\$$@\" ; fi" >> $(DIR_IMG)/run-$*.sh
	echo "mkdir -p ./\$$SNAME" >> $(DIR_IMG)/run-$*.sh
	for WLD in $($*) ; do \
		$(MAKE) wshell-$${WLD} SCRIPTNAME=">> $(DIR_IMG)/run-$*.sh" ;\
	done
	echo "echo \"NOTICE: This is an EEMBC benchmark. \" >> \$$SNAME.csv" >> $(DIR_IMG)/run-$*.sh
	echo "echo \"* Use and distribution of this benchmark and any results are subject to the EEMBC license agreement. \" >> \$$SNAME.csv" >> $(DIR_IMG)/run-$*.sh
	
#Target: pgorun-list-<list name>
#	Run just the workloads in the list, using pgo feedback optimizations.
#
#  	Example: make pgorun-list-RC1 will run all the workloads that are part of the RC1 workload set, setting dependencies such that the benchmarks will be built with profile guided optimizations.
.PHONY: pgorun-list-%
pgorun-list-%:
	for WLD in $($*) ; do \
		$(MAKE) pgouserun-$${WLD} ;\
	done
	
#Target: build-list-<list name>
#	build just the workloads in the list.
#
#  	Example: make build-list-RC1 will build all the workloads that are part of the RC1 workload set.
build-list-%:
	for WLD in $($*) ; do \
		$(MAKE) wbuild-$${WLD} ;\
	done

#Target: build-list-<list name>
#	build just the workloads in the list.
#
#  	Example: make build-list-RC1 will build all the workloads that are part of the RC1 workload set.
pgobuild-list-%:
	for WLD in $($*) ; do \
		$(MAKE) pgousebuild-$${WLD} ;\
	done
	
#Target: clean
#	Clean the build directory
clean: 
	@$(RMDIR) $(DIR_IMG) $(DIR_BUILD) $(DIR_CERT) $(DIR_TARGET)/mith

#Target: disclean
#	Clean project directories, preparing for release
distclean: clean
	@$(RMDIR) $(TOPDIR)builds
	@$(RM) `find . -name \*.ncb`
	@$(RM) `find . -name \*.suo`
	@$(RMDIR) `find . -name Debug`
	@$(RMDIR) `find . -name debug`
	@$(RMDIR) `find . -name Release`

DOCFILES_base = docs/images docs/affinity docs/notes docs/Marks_for_MultiBench-*
DOCFILES = docs/multibench_primer.ppt docs/html $(DOCFILES_base)
	
mith-%: $(COMMON_DIRS)
	$(MAKE) -C $(DIR_TARGET)/mith -f $(TOPDIR)mith/Makefile $*

log-flags: $(COMMON_DIRS)
	@echo "$(DATESTAMP) Starting build" >> $(LOG_PROGRESS)
	@echo "  flags: $(CFLAGS) $(XCFLAGS)" >> $(LOG_PROGRESS)

#Certification Targets
#=====================
.PHONY: csv 

#Target: csv
#	Convert the log file to csv format (can be loaded in excel).
csv:
	cat $(DIR_LOG)/$(TARGET).$(TOOLCHAIN).log | grep -v "#" | sed -e "s/\t/,/g" > $(DIR_LOG)/$(TARGET).$(TOOLCHAIN).csv

#Target: certify-all
#	Build, run and collect results for certification procedure
#	See <certify-process> for details
certify-all: build-all
	@echo "Starting certifcation process for $(WORKLOAD_NAMES)"
	$(MAKE) DIR_CERT=$(DIR_CERT) -f Makefile.mak certify-process

#Target: certify-list-<list name>
#	Performs certification steps just the workloads in the list.
#	See <certify-process> for details
#
#  	Example: make certify-list-RC1 will run certification steps for all the workloads that are part of the RC1 workload set.
certify-list-%: 
	$(RMDIR) $(DIR_CERT)
	$(MDIR) $(DIR_CERT_TMP)
	$(MAKE) certify-all DIR_CERT=$(DIR_CERT) WORKLOAD_NAMES="$($*-contexts)" BEST_CONCURRENCY=$(BEST_CONCURRENCY_CONTEXTS) && echo "$(DIR_CERT)" > $(DIR_CERT_TMP)/contexts-folder.txt
	$(CP) $(DIR_LOG)/$(TARGET).$(TOOLCHAIN).log $(DIR_CERT_TMP)
	$(MAKE) certify-all DIR_CERT=$(DIR_CERT) WORKLOAD_NAMES="$($*-workers)" BEST_CONCURRENCY=$(BEST_CONCURRENCY_WORKERS) && echo "$(DIR_CERT)" > $(DIR_CERT_TMP)/workers-folder.txt
	$(CAT) $(DIR_CERT_TMP)/$(TARGET).$(TOOLCHAIN).log >> $(DIR_LOG)/$(TARGET).$(TOOLCHAIN).log
	$(CP) $(DIR_CERT_TMP)/*folder.txt $(DIR_LOG)
	$(PERL) $(TOPDIR)/util/perl/cert_mark.pl -i $(DIR_CERT)/$(TARGET).$(TOOLCHAIN).log -s $(MULTIBENCH_SUITE) > $(DIR_CERT)/$(TARGET).$(TOOLCHAIN).mark
	$(RMDIR) $(DIR_CERT_TMP)

#Target: pgocertify-list-<list name>
#	Performs certification steps just the workloads in the list, using profile guided optimizations.
#	This target will first build and run workloads with a profile training data set, then build the actual benchmarks for certification with profile feedback optimizations enabled.
#	See <certify-process> for details
#
#  	Example: make pgocertify-list-RC1 will run certification steps for all the workloads that are part of the RC1 workload set, using profile guided optimizations when compiling.
pgocertify-list-%: 
	$(MAKE) pgobuild-list-$*
	$(RM) $(RESLOG)
	$(MAKE) certify-list-$*
	
#Target: certify-process
#	Actual certification process, requires DIR_CERT to be defined on call to make sure all results go to the same dir.
#	For each workload to be certified, performs the following steps
#	- Run once with verification turned on, and save results
#	- Run 9 times with verification turned off, and calculate median result
#	- Backup all results to certifiaction folder
#	- Repeat all steps once for concurrency type in 1,  <BEST_CONCURRENCY> 
#	- Calculate marks
certify-process:
	for CONTYPE in single best ; do \
		echo "	Gathering results for $${CONTYPE}";\
		$(MDIR) $(DIR_CERT)/$${CONTYPE}/verify ;\
		$(MDIR) $(DIR_CERT)/$${CONTYPE}/perf ;\
		echo "$(DATESTAMP) Starting Run $${CONTYPE}" >> $(LOG_PROGRESS) ;\
		[ -f $(RESLOG) ] || $(CAT) util/perl/headings.txt > $(RESLOG) ;\
		for WLD in $(WORKLOAD_NAMES) ; do \
			$(MAKE) DIR_CERT=$(DIR_CERT)/$${CONTYPE} CONCURRNCY=$${CONTYPE} -f Makefile.mak wcertify-$${WLD} ;\
			echo "	Saving results for $${WLD}";\
			$(CAT) $(DIR_CERT)/$${CONTYPE}/perf/logs/progress.log >> $(DIR_CERT)/$${CONTYPE}/progress.log &&\
			$(RM) $(DIR_CERT)/$${CONTYPE}/perf/logs/progress.log ;\
			$(CAT) $(DIR_CERT)/$${CONTYPE}/perf/logs/$(TARGET).$(TOOLCHAIN).log >> $(DIR_CERT)/$${CONTYPE}/$(TARGET).$(TOOLCHAIN).log &&\
			$(RM) $(DIR_CERT)/$${CONTYPE}/perf/logs/$(TARGET).$(TOOLCHAIN).log;\
		done ;\
		$(CAT) $(DIR_CERT)/$${CONTYPE}/progress.log >> $(DIR_CERT)/progress.log ;\
		$(CAT) $(DIR_CERT)/$${CONTYPE}/$(TARGET).$(TOOLCHAIN).log >> $(DIR_CERT)/$(TARGET).$(TOOLCHAIN).log ;\
	done
	$(PERL) $(TOPDIR)/util/perl/cert_mark.pl -i $(DIR_CERT)/$(TARGET).$(TOOLCHAIN).log -s $(MULTIBENCH_SUITE) > $(DIR_CERT)/$(TARGET).$(TOOLCHAIN).mark
	$(CP) $(DIR_CERT)/$(TARGET).$(TOOLCHAIN).log $(DIR_LOG) 
	$(CP) $(DIR_CERT)/$(TARGET).$(TOOLCHAIN).mark $(DIR_LOG)
	$(CP) $(DIR_CERT)/progress.log $(DIR_LOG)
	@$(CAT) $(DIR_CERT)/$(TARGET).$(TOOLCHAIN).mark

#Target: wcertify-%
#	Build, run and collect results for certification procedure on specific workloads
wcertify-%: wbuild-%
	$(RM) $(DIR_LOG)/*
	$(MDIR) $(DIR_CERT)/verify
	$(MDIR) $(DIR_CERT)/perf
	@echo "$(DATESTAMP) Starting Run" >> $(LOG_PROGRESS)
	[ -f $(RESLOG) ] || $(CAT) util/perl/headings.txt > $(RESLOG)
	@echo "#Results for verification run started at $(SDATESTAMP) XCMD=$(XCMD)" >> $(RESLOG)
	@echo "	Verification run for $*"
	for WLD in workloads/$* ; do \
		$(MAKE) -C $(DIR_IMG) -f $(TOPDIR)$${WLD}/Makefile load && \
		$(MAKE) XCMD="$(XCMD) -v1" -C $(DIR_IMG) -f $(TOPDIR)$${WLD}/Makefile run && \
		$(MAKE) -C $(DIR_LOG) -f $(TOPDIR)$${WLD}/Makefile results && \
		$(MDIR) $(DIR_CERT)/verify/$${WLD} && \
		$(CPDIR) $(DIR_LOG) $(DIR_CERT)/verify/$${WLD} \
		;\
	done
	@echo "#Results for performance runs started at $(SDATESTAMP) XCMD=$(XCMD)" >> $(RESLOG)
	@echo "	Performance runs for $*"
	for ii in 1 2 3 ; do \
	for WLD in workloads/$* ; do \
		$(MAKE) -C $(DIR_IMG) -f $(TOPDIR)$${WLD}/Makefile load &&	\
		$(MAKE) XCMD="$(XCMD) -v0" -C $(DIR_IMG) -f $(TOPDIR)$${WLD}/Makefile run &&	\
		$(MAKE) -C $(DIR_LOG) -f $(TOPDIR)$${WLD}/Makefile results	\
		;\
	done; done
	@echo "#Median for final result $*" >> $(RESLOG)
	$(PERL) $(TOPDIR)/util/perl/cert_median.pl $(RESLOG) $(CONCURRNCY) >> $(RESLOG)
	$(CPDIR) $(DIR_LOG) $(DIR_CERT)/perf 

#foreach workload
#	Foreach work item
#	Concat work item benchmark key to workload.key file
#	for ITEM in $$(ITEMS) ; do
#		[ -f release/items/$$(ITEM).tgz ] || \
#		tar -vzcf release/items/$$(ITEM).tgz --exclude=.svn --exclude=Ankh.Load benchmarks/$$(ITEM) && \
#		ccencrypt -k keys/$$(ITEM) release/items/$$(ITEM).tgz	
#	ccencrypt -K workload key
#tgz benchmarks and workloads
#tgz harness 
release-pack: distclean
	$(RMDIR) release/items
	$(MDIR) release/items
	$(RMDIR) release/keys
	$(MDIR) release/keys
	$(MDIR) release/keys/all
	$(RM) tar.log
	@for WLD_DIR in $(WORKLOAD_NAMES) ; do \
		WLD=`basename $$WLD_DIR` &&\
		echo "Prepare $$WLD"; \
		ITEMS=`$(PERL) $(TOPDIR)/util/perl/itemex.pl workloads/$${WLD}/Makefile` && \
		echo " ITEMS: $$ITEMS" && \
		for ITEM in $$ITEMS ; do \
			FNAME=`echo $$ITEM | sed -e  "s/\//-/g"` && \
			[ -f release/items/$${FNAME}.tgz.cpt ] || \
			( echo " Prepare $$ITEM" &&\
			  { { [ -f keys/$${ITEM}.data ] && DATA_FOLDER="`cat keys/$${ITEM}.data`" ; } || echo " No data folder for $$ITEM" ; } &&\
			  echo "  adding $$ITEM and $$DATA_FOLDER" &&\
			  tar -vzcf release/items/$${FNAME}.tgz --exclude=.svn --exclude=\*.Shay --exclude=Ankh.Load benchmarks/$${ITEM} $$DATA_FOLDER >> tar.log && \
			  ccencrypt -k keys/$${ITEM}.keys release/items/$${FNAME}.tgz &&\
			  cat keys/$${ITEM}.keys > release/keys/all/$${FNAME}.keys ) ;\
			echo "" >> release/keys/all/$${WLD}.allkeys ;\
			echo "Item:$${ITEM}" >> release/keys/$${WLD}.allkeys ;\
			echo "" >> release/keys/$${WLD}.allkeys ;\
			cat keys/$${ITEM}.keys >> release/keys/$${WLD}.allkeys ;\
		done ;\
		{ { [ -f keys/$${WLD}.data ] && DATA_FOLDER="`cat keys/$${WLD}.data`" ; } || { echo " No extra data for $$WLD" && DATA_FOLDER="" ; } } &&\
		echo "  adding $$WLD and $$DATA_FOLDER" &&\
		tar -vzcf release/items/$${WLD}.tgz --exclude=.svn --exclude=Ankh.Load --exclude=\*.Shay workloads/$${WLD} $$DATA_FOLDER >> tar.log && \
		ccencrypt -k keys/$${WLD}.keys release/items/$${WLD}.tgz ;\
		echo "" >> release/keys/all/$${WLD}.allkeys ;\
		echo "Workload:$${WLD}" >> release/keys/all/$${WLD}.allkeys ;\
		cat keys/$${WLD}.keys >> release/keys/all/$${WLD}.allkeys ;\
		cat keys/$${WLD}.keys > release/keys/all/$${WLD}.keys ;\
	done
	tar -vzcf release/items/architect.tgz util/wld util/bin workloads/xml benchmarks/xml docs/mith_workload_creator_primer.ppt --exclude=.svn --exclude=Ankh.Load --exclude=\*.xls >> tar.log
	ccencrypt -k keys/architect.keys release/items/architect.tgz ;\
	tar -vzcf release/mith_lib_${TAG}.tgz mith Makefile.mak util/make util/perl util/shell util/misc docs workloads/sets --exclude=.svn --exclude=Ankh.Load --exclude=\*.xls >> tar.log
	cd release && total=0 && tgn=0 && unset TN[$tgn] && echo "splitting to totals under 100M" && \
	for f in `ls -1 items/*.cpt` ; do \
		FS=`stat -c%s $$f` && (( total+=$$FS )) && \
		if (( total > 100000000 )) ; then (( total=0 , tgn+=1 )) && unset TN[$$tgn]; fi && \
		TN[$$tgn]="$${TN[$$tgn]} $$f" \
	; done && \
	for (( i=0; i<=$$tgn; i++ )) ; do \
		echo "files in TN $$i" ;\
		tar -vzcf mith_wld_${TAG}_part$${i}.tgz $${TN[$$i]} > ../tar.log ; \
	done
	cd release && md5sum mith_lib_${TAG}.tgz mith_wld_${TAG}*.tgz > mith_${TAG}.md5
	echo "TAG = $(TAG)" > release/Makefile
	cat Makefile >> release/Makefile
	cp README.TXT release
	$(MAKE) pack-keys
	$(RM) tar.log
	$(RMDIR) release/items
	$(MDIR) release/$(TAG) && mv release/*_$(TAG)* release/$(TAG)
	
.PHONY:	pack-keys
pack-keys:
	$(MDIR) release/keys
	cp keys/architect.keys release/keys
	for set in $(SET_DEFS) ; do $(MAKE) packme-$$set ; done
	for f in keys/*.pack ; do \
		N=`basename $$f` && \
		$(MDIR) release/keys/$$N && echo "Prepare keys for pack $$f" &&\
		for tn in `cat $$f` ; do [ "$$tn" != "" ] && bn=`basename $$tn` && echo "  - $$bn" && \
			( cp `find release/keys -name $$bn -print -quit` release/keys/$$N || echo "Could not find $$bn" ) \
		; done && \
		pushd release && zip -r -j keys/$$N.zip keys/$$N && rm -rf keys/$$N && popd \
	; done
	pushd release && zip -r keys.zip keys && popd
	
release-pack-%:
	@$(MAKE) -f Makefile.mak WORKLOAD_NAMES="$($*)" release-pack

.PHONY: packme-%
packme-%:
	rm -f keys/$*.pack
	for f in $($*) ; do echo $$f.keys >> keys/$*.pack ; done
	for f in $($*) ; do echo $$f.allkeys >> keys/$*.pack ; done		
	for f in $($*-kernels) ; do [[ "$$f" =~ ".h" ]] || echo $$f.keys | sed -e  "s/\//-/g" >> keys/$*.pack ; done		

.PHONY: release
release: distclean
	@echo "creating release with TAG:$(TAG) use TAG=<stamp> to override tag."
	$(MDIR) release
	@tar -vzcf release/mith_tools_${TAG}.tgz util/wld util/bin workloads/xml benchmarks/xml docs/mith_workload_creator_primer.ppt --exclude=.svn --exclude=Ankh.Load 
	@tar -vzcf release/mith_lib_${TAG}.tgz mith Make* util/make util/perl util/shell util/misc $(DOCFILES) --exclude=.svn --exclude=Ankh.Load --exclude=\*.xls --exclude=\*.Shay
	@tar -vzcf release/mith_wld_${TAG}.tgz benchmarks workloads --exclude=.svn --exclude=Ankh.Load --exclude=xml --exclude=\*.Shay
	@cd release && md5sum mith_lib_${TAG}.tgz mith_wld_${TAG}.tgz > mith_${TAG}.md5

# The file 'suite.mak' sets a variable in the makefile
# telling make which suit is being run. This is kind of a
# hack since the same code is used for all suites, but it
# is a way of solving the problem of telling cert_mark.pl
# which workload to score. Replace this file with an empty
# file after making the tarball.
# TODO it would be nice if the makefile created a REDIST
# TODO area first, and then tar'd that, rather than tarring
# TODO the work area. Then we could set things up the way
# TODO we want based on workload, but I understand Shay's
# TODO attempt at trying to keep everything generic. Kinda
# TODO breaking that here.
release-tag-%: distclean
	$(MDIR) release
	$(MDIR) release/${TAG_BASE}
	$(RM) release/${TAG_BASE}/*
	@echo "MULTIBENCH_SUITE=$*" > util/make/suite.mak
	@echo "Creating tar for $*"
	@tar \
		-hzcv \
		-f release/${TAG_BASE}/${TAG_BASE}${TAG}.tgz \
		--transform=s,^,${TAG_BASE}${TAG}/, \
		docs/RELEASE_NOTES-$*.txt \
		mith \
		Make* \
		util/make \
		util/perl \
		util/shell \
		util/misc \
		workloads/sets \
		$($*-dirs) \
		--exclude=.svn \
		--exclude=Ankh.Load \
		--exclude=\*.Shay \
		> release/tar-$*.log \
		|| echo "Error packing lib"
	$(RM) changes.log
	$(RM) util/make/suite.mak
	touch util/make/suite.mak

# the expansion should install ON TOP of its parent
# can we call the base name in the workload*.defs file instead?
release-expansion-%: distclean
	$(MDIR) release
	$(MDIR) release/${TAG_BASE}
	$(RM) release/${TAG_BASE}/*
	@echo "Creating expansion tar for $*"
	@tar \
		-hvzc \
		-f release/${TAG_BASE}/${TAG_BASE}${TAG}.tgz \
		--transform=s,_expansion,, \
		docs/RELEASE_NOTES-$*.txt \
		$($*-dirs) \
		--exclude=.svn \
		--exclude=Ankh.Load \
		--exclude=\*.Shay \
		> release/tar-$*.log \
		|| echo "Error packing lib"

ifndef TAG_BASE
release-list-%: distclean
	$(MAKE) -f Makefile.mak release-tag-$* TAG_BASE=$(TAGBASE-$*)
else
release-list-%: distclean
	$(MAKE) -f Makefile.mak release-tag-$* TAG_BASE=$(TAG_BASE)
endif
	
ndrelease-list-%: distclean
	$(MDIR) release
	@tar -vzcf release/mith_lib_${TAG}.tgz mith Make* util/make util/perl util/shell util/misc --exclude=.svn --exclude=Ankh.Load --exclude=\*.Shay > tar.log || echo "Error packing lib"
	tar -vzcf release/mith_wld_${TAG}.tgz --exclude=.svn --exclude=Ankh.Load --exclude=\*.Shay workloads/sets $($*-dirs) > tar.log || echo "Error packing workloads"
	@cd release && md5sum mith_lib_${TAG}.tgz mith_wld_${TAG}.tgz > mith_${TAG}.md5
	
.PHONY: print-%
print-%:
	@echo $* = $($*)  
	@echo [Defined at $(origin $*)] 
	
