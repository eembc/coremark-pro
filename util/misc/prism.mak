# file: util/misc/prism.mak
# Optional specific files for specialized run and results
# Cross compile from a cygwin host to a prism ARM target.
PLATFORM=prism

BEST_CONCURRENCY=1
OVERLOAD_CONCURRENCY=1

TOOLCHAIN=armcs

# Flag: LOAD
LOAD = 

# Flag: RUN
#	Use the toolchain simulator
RUN = /d/dev/tools/arm/bin/arm-none-eabi-run.exe -m 33554432
RUN_FLAGS =

PLATFORM_DEFINES = 

# Flag: CMD_SEP
#	Use CMD_SEP if a separator is required before run flags (e.g. --)
CMD_SEP=

#FLAG: COPY_DATA
#	How should data be copied so that it is available for the executable?
COPY_DATA=cp -Ru 

TARGET_EXTRA=prism