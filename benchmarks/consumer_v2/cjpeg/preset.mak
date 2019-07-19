SRC_BASE =	bmark_lite bm_lib cdjpeg cjpeg \
	filedata jcapimin jcapistd jccoefct \
	jccolor jcdctmgr jchuff jcinit \
	jcmainct jcmarker jcmaster jcomapi \
	jcparam jcprepct jcsample jdatadst \
	jerror jfdctint jmemansi jmemmgr \
	jutils rdbmp 

DATA_BASE = \
	DavidAndDogs_bmp \
	door_bmp \
	DragonFly_bmp \
	EEMBCGroupShotMiami_bmp \
	eye_bmp \
	Galileo_bmp \
	goose_bmp \
	hall_bmp \
	Mandrake_bmp \
	MarsFormerLakes_bmp \
	Rose256_bmp \
	window_bmp 

ifdef PRECISION
KERNEL_DEFINES+=$(CDEFN)$(PRECISION)
KERNEL_OBJ_FOLDER=$(PRECISION)/
endif

ifndef SELECT_PRESET_NAME
SRC_FILES = $(SRC_BASE) 
KERNEL_DEFINES+=-DUSE_PRESET=0
else
DATA_SINGLE=$(SELECT_PRESET_NAME)
ifneq ($(USE_PRESET),FALSE)
BO = $(DIR_BENCH)/consumer_v2
SRC_FILES = $(addprefix $(OBJ_HEADER), $(SRC_BASE) $(DATA_SINGLE) )
#Exception, for PGO purposes, the pgo dataset should be built into the object.
ifneq ("$(SELECT_PRESET_NAME)","goose_bmp")
SRC_FILES += $(addprefix $(OBJ_HEADER), goose_bmp )
endif
KERNEL_OBJS_PATH=$(addprefix $(BO)/cjpeg/$(KERNEL_OBJ_FOLDER), $(SRC_FILES)) 
KERNEL_OBJS=$(addsuffix $(OEXT),$(KERNEL_OBJS_PATH))
KERNEL_DEFINES+=-DUSE_PRESET=1 -DSELECT_PRESET_ID=$(SELECT_PRESET_ID)
else
SRC_FILES = $(SRC_BASE) 
KERNEL_DEFINES+=-DUSE_PRESET=1
endif

endif
