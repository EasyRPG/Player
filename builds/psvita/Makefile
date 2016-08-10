TARGET		:=	EasyRPG
BUILD		:=	build
SOURCES		:=	../../src
DATA		:=	data

APP_TITLE   	:=	EasyRPG Player
APP_ID		:=	ERPG00001

CFILES		:=	$(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
CPPFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
BINFILES := $(foreach dir,$(DATA), $(wildcard $(dir)/*.bin))
OBJS     := $(addsuffix .o,$(BINFILES)) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) 

LIBS = -lvorbisfile -logg -lvorbis -lspeexdsp -lsndfile \
	-lvita2d -llcf -licuuc -licui18n -licudata -lpixman-1 \
	-lSceKernel_stub -lSceDisplay_stub -lSceGxm_stub	\
	-lSceSysmodule_stub -lSceCtrl_stub -lm \
	-lScePgf_stub -ljpeg -lfreetype -lc \
	-lScePower_stub -lSceCommonDialog_stub -lpng16 -lz \
	-lmpg123 -lSceAudio_stub

ifeq ($(strip $(LIBLCF_DIR)),)
LIBLCF_DIR := $(TOOLCHAIN_DIR)
endif

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CXX      = $(PREFIX)-g++
CFLAGS  = -Wl,-q -O3 -DPSP2 -DHAVE_LIBSPEEXDSP \
		-DWANT_FASTWAV -DSUPPORT_AUDIO -DHAVE_LIBSNDFILE \
		-DHAVE_OGGVORBIS -DHAVE_MPG123 -DWANT_FMMIDI=1 \
		-I$(VITASDK)/arm-vita-eabi/include/liblcf \
		-I$(VITASDK)/arm-vita-eabi/include/pixman-1 \
		-I$(TOOLCHAIN_DIR)/include \
		-I$(LIBLCF_DIR)/include/liblcf \
		-I$(TOOLCHAIN_DIR)/include/pixman-1 \
		-L$(TOOLCHAIN_DIR)/lib \
		-L$(LIBLCF_DIR)/lib

CXXFLAGS  = $(CFLAGS) -fno-exceptions -std=gnu++11 -fno-rtti
ASFLAGS = $(CFLAGS)

HAVE_ZIP := $(shell command -v zip 2> /dev/null)

all: $(TARGET).vpk

$(TARGET).vpk: $(TARGET).velf
		vita-make-fself $< release/eboot.bin
		vita-mksfoex -s TITLE_ID=$(APP_ID) "$(APP_TITLE)" release/sce_sys/param.sfo
ifdef HAVE_ZIP
		cd release; zip ../$(TARGET).vpk -r ./sce_sys ./eboot.bin
else
		7z a -tzip ./$(TARGET).vpk -r .\release\sce_sys\* .\release\eboot.bin
endif

%.velf: %.elf
	$(PREFIX)-strip -g $<
	vita-elf-create $< $@

$(TARGET).elf: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).velf $(TARGET).elf $(OBJS) param.sfo eboot.bin