MYNAME = cjpeg
include common.mak

TOPME = $(TOPDIR)benchmarks/consumer_v2/$(MYNAME)
MYDATA = $(TOPDIR)benchmarks/libbmp
S_DIRS += $(addprefix $(TOPME)/,data) 
include $(TOPME)/preset.mak

vpath %.c $(TOPME) $(S_DIRS)

#this item needs direct access to al_smp layer
INC_DIRS += $(TOPDIR)mith/al/include $(TOPME)/../common $(TOPME)/data


.PHONY: build
build: done.build

OBJ_FILES = $(addsuffix $(OEXT),$(SRC_FILES))

done.build: $(OBJ_FILES) copy_mydata
	touch done.build


