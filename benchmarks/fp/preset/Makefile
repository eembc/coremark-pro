MYNAME = fp_preset
include common.mak

TOPME = $(TOPDIR)benchmarks/fp/preset
vpath %.c $(TOPME) 
INC_DIRS += $(TOPME)

OBJ_FILES = fake_0$(OEXT) \
	fake_1$(OEXT) \
	fake_2$(OEXT) \
	fake_3$(OEXT) \
	fake_4$(OEXT) \
	fake_5$(OEXT) \
	fake_6$(OEXT) \

.PHONY: build
build: done.build

done.build: $(OBJ_FILES)
	touch done.build


