MYNAME = sha
include common.mak

TOPME = $(TOPDIR)benchmarks/darkmark/sha

INC_DIRS += $(TOPME)
vpath %.c $(TOPME) 

SRC_FILES = sha256 shabench
OBJ_FILES = $(addsuffix $(OEXT),$(SRC_FILES))

.PHONY: build
build: done.build

done.build: $(KERNEL_DIRS) $(OBJ_FILES)
	touch done.build


