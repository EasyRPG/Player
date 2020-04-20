TARGET  := EasyRPG
BUILD   := build
SOURCES := ../../src ../../src/platform/psvita

APP_TITLE := EasyRPG Player
APP_ID    := ERPG00001

CPPFILES := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
OBJS     := $(CPPFILES:.cpp=.o)

LIBS = -llcf -lexpat -lvita2d \
	-lpixman-1 -lpng16 -lz -licuuc -licui18n -licudata \
	-lfreetype -lharfbuzz -lfreetype -lxmp-lite -lmpg123 -lsndfile \
	-lvorbisfile -lvorbis -logg -lWildMidi -lspeexdsp -lopusfile -lopus \
	-lpthread -lSceLibKernel_stub -lSceDisplay_stub -lSceGxm_stub \
	-lvitashaders -lSceSysmodule_stub -lSceCtrl_stub -lSceTouch_stub \
	-lScePgf_stub -lScePower_stub -lSceCommonDialog_stub -lSceAudio_stub

ifeq ($(strip $(LIBLCF_DIR)),)
LIBLCF_DIR := $(TOOLCHAIN_DIR)
endif

PREFIX  = arm-vita-eabi
CXX     = $(PREFIX)-g++

ifneq ($(strip $(shell command -v ccache 2> /dev/null)),)
CCACHE = ccache
endif

CFLAGS  = -Wl,-q -g -O3 -fno-math-errno
CXXFLAGS = $(CFLAGS) -fno-exceptions -std=gnu++14 -fno-rtti
ASFLAGS = $(CFLAGS)
CPPFLAGS = -DPSP2 -DHAVE_FREETYPE -DHAVE_HARFBUZZ -DSUPPORT_AUDIO \
		-DHAVE_MPG123 -DHAVE_LIBSNDFILE -DHAVE_OGGVORBIS \
		-DHAVE_XMP -DHAVE_WILDMIDI -DHAVE_OPUS \
		-DHAVE_LIBSPEEXDSP -DWANT_FMMIDI -DWANT_FASTWAV \
		-I../../src \
		-I$(TOOLCHAIN_DIR)/include \
		-I$(LIBLCF_DIR)/include/liblcf \
		-I$(TOOLCHAIN_DIR)/include/pixman-1 \
		-I$(TOOLCHAIN_DIR)/include/freetype2 \
		-I$(TOOLCHAIN_DIR)/include/libxmp-lite \
		-I$(TOOLCHAIN_DIR)/include/opus

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

$(TARGET).velf: $(TARGET).elf
	$(PREFIX)-strip -g -o $(TARGET)-stripped.elf $<
	vita-elf-create $(TARGET)-stripped.elf $@

%.o: %.cpp
	@echo "[CXX] $<"
	@$(CCACHE) $(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	@echo "[CXXLD] $@"
	@$(CCACHE) $(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).velf $(TARGET).elf $(TARGET)-stripped.elf $(OBJS) param.sfo eboot.bin
