MYNAME = fft_radix2
include common.mak

TOPME = $(TOPDIR)benchmarks/fp/fft_radix2
ifeq ($(PRECISION),SP)
REF_FOLDER=ref-sp
else
REF_FOLDER=ref
endif

S_DIRS += $(addprefix $(TOPME)/,$(REF_FOLDER)) 
INC_DIRS += $(TOPME)
include $(TOPME)/preset.mak
vpath %.c $(TOPME) $(S_DIRS)

OBJ_FILES = $(addsuffix $(OEXT),$(SRC_FILES))

.PHONY: build
build: done.build

done.build: $(OBJ_FILES)
	touch done.build


