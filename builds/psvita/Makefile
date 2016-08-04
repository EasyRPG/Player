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

LIBS = -lvita2d -llcf -licuuc -licui18n -licudata -lpixman-1 \
	-lSceKernel_stub -lSceDisplay_stub -lSceGxm_stub	\
	-lSceSysmodule_stub -lSceCtrl_stub -lSceTouch_stub -lm -lSceNet_stub \
	-lSceNetCtl_stub -lScePgf_stub -ljpeg -lfreetype -lc \
	-lScePower_stub -lSceCommonDialog_stub -lpng16 -lz \
	-lmpg123 -lSceAudio_stub

		
PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CXX      = $(PREFIX)-g++
CFLAGS  = -Wl,-q -O3 -DPSP2 -DHAVE_SPEEXDSP \
		-DSUPPORT_AUDIO -DUSE_CACHE \
		-DHAVE_MPG123 -DWANT_FMMIDI=1 \
		-I$(VITASDK)/arm-vita-eabi/include/liblcf \
		-I$(VITASDK)/arm-vita-eabi/include/pixman-1
CXXFLAGS  = $(CFLAGS) -fno-exceptions -std=gnu++11
ASFLAGS = $(CFLAGS)

all: $(TARGET).vpk

$(TARGET).vpk: $(TARGET).velf
	vita-make-fself $< .\release\eboot.bin
	vita-mksfoex -s TITLE_ID=$(APP_ID) "$(APP_TITLE)" param.sfo
	cp -f param.sfo ./release/sce_sys/param.sfo
	
	#------------ Comment this if you don't have 7zip ------------------
	7z a -tzip ./$(TARGET).vpk -r .\release\sce_sys\* .\release\eboot.bin 
	#-------------------------------------------------------------------
	
%.velf: %.elf
	$(PREFIX)-strip -g $<
	vita-elf-create $< $@

$(TARGET).elf: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).velf $(TARGET).elf $(OBJS) param.sfo eboot.bin