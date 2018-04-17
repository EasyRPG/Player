TARGET  := EasyRPG
BUILD   := build
SOURCES := ../../src
DATA    := data

APP_TITLE := EasyRPG Player
APP_ID    := ERPG00001

CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
CPPFILES := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
BINFILES := $(foreach dir,$(DATA), $(wildcard $(dir)/*.bin))
OBJS     := $(addsuffix .o,$(BINFILES)) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) 

LIBS = -llcf -licuuc -licui18n -licudata \
	-lpixman-1 -lpng16 -lz -lfreetype -lvita2d \
	-lvorbisfile -lvorbis -logg -lspeexdsp -lsndfile -lmpg123 -lWildMidi \
	-lpthread -lSceLibKernel_stub -lSceDisplay_stub -lSceGxm_stub \
	-lvitashaders -lSceSysmodule_stub -lSceCtrl_stub -lSceTouch_stub \
	-lScePgf_stub -lScePower_stub -lSceCommonDialog_stub -lSceAudio_stub

ifeq ($(strip $(LIBLCF_DIR)),)
LIBLCF_DIR := $(TOOLCHAIN_DIR)
endif

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CXX     = $(PREFIX)-g++

CFLAGS  = -Wl,-q -g -O3
CXXFLAGS = $(CFLAGS) -fno-exceptions -std=gnu++11 -fno-rtti
ASFLAGS = $(CFLAGS)
CPPFLAGS = -DPSP2 -DSUPPORT_AUDIO \
		-DHAVE_LIBSPEEXDSP -DHAVE_LIBSNDFILE \
		-DHAVE_OGGVORBIS -DHAVE_MPG123 -DHAVE_WILDMIDI \
		-DWANT_FMMIDI -DWANT_FASTWAV \
		-I$(VITASDK)/arm-vita-eabi/include/liblcf \
		-I$(VITASDK)/arm-vita-eabi/include/pixman-1 \
		-I$(TOOLCHAIN_DIR)/include \
		-I$(LIBLCF_DIR)/include/liblcf \
		-I$(TOOLCHAIN_DIR)/include/pixman-1
LDFLAGS = -L$(TOOLCHAIN_DIR)/lib \
		-L$(LIBLCF_DIR)/lib

HAVE_ZIP := $(shell command -v zip 2> /dev/null)

all: $(TARGET).vpk

$(TARGET).vpk: $(TARGET).velf
		vita-make-fself -s $< release/eboot.bin
		vita-mksfoex -s TITLE_ID=$(APP_ID) -d ATTRIBUTE2=12 "$(APP_TITLE)" release/sce_sys/param.sfo
ifdef HAVE_ZIP
		cd release; zip ../$(TARGET).vpk -r ./sce_sys ./eboot.bin
else
		7z a -tzip ./$(TARGET).vpk -r .\release\sce_sys\* .\release\eboot.bin
endif

%.velf: %.elf
	$(PREFIX)-strip -g -o $(basename $<)-stripped.elf $<
	vita-elf-create $(basename $<)-stripped.elf $@

%.o: %.cpp
	@echo "[CXX] $<"
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	@echo "[CXXLD] $@"
	@$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).velf $(TARGET).elf $(TARGET)-stripped.elf $(OBJS) param.sfo eboot.bin