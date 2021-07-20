TARGET  := EasyRPG
BUILD   := build
SOURCES := ../../src
OSTYPE  := $(shell uname -s)

CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
CPPFILES := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
OBJS     := $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)

ifeq ($(OSTYPE), MorphOS)
PREFIX  = ppc-morphos
CC      = $(PREFIX)-gcc -noixemul
CXX     = $(PREFIX)-g++ -noixemul
PLATFORM_CXXFLAGS = -D_GLIBCXX_USE_C99 -D_NO_PPCINLINE -D__MORPHOS_SHAREDLIBS
TOOLCHAIN_DIR := /gg/usr/local
SDL_LIBS = -lSDL2
SDL_CFLAGS = -I$(TOOLCHAIN_DIR)/include/SDL2 -DUSE_SDL=2
PIXMAN_CFLAGS = -I/gg/os-include/pixman-1
#PIXMAN_LIBS = -lpixman
PNG_LIBS =  -lpng_shared
MPG123_CFLAGS = -DHAVE_LIBMPG123
MPG123_LIBS = -lmpg123
EXPAT_LIBS =
FREETYPE_CFLAGS = -DHAVE_FREETYPE -I$(TOOLCHAIN_DIR)/include/freetype2
FREETYPE_LIBS = -lfreetype
WILDMIDI_CFLAGS = -DHAVE_LIBWILDMIDI
WILDMIDI_LIBS = -lWildMidi
FMT_LIBS = -lfmt
#OGGVORBIS_CFLAGS = -DHAVE_OGGVORBIS
#OGGVORBIS_LIBS =
endif
ifeq ($(OSTYPE), AmigaOS)
PREFIX  = ppc-amigaos
CC      = $(PREFIX)-gcc
CXX     = $(PREFIX)-g++
PLATFORM_CXXFLAGS = -D_GLIBCXX_USE_C99_STDINT_TR1 -D_GLIBCXX_USE_C99_STDIO -Iamigaos4
TOOLCHAIN_DIR := /usr/local/amiga/ppc-amigaos/SDK/local/newlib
SDL_LIBS = -lSDL -athread=native
SDL_CFLAGS = -I$(TOOLCHAIN_DIR)/include/SDL -DUSE_SDL=1
PIXMAN_CFLAGS = -I$(TOOLCHAIN_DIR)/include/pixman-1
PIXMAN_LIBS = -lpixman-1
PNG_LIBS = -lpng -lz
MPG123_CFLAGS = -DHAVE_LIBMPG123
MPG123_LIBS = -lmpg123
EXPAT_LIBS = -lexpat
FREETYPE_CFLAGS = -DHAVE_FREETYPE -I$(TOOLCHAIN_DIR)/include/freetype2
FREETYPE_LIBS = -lfreetype
WILDMIDI_CFLAGS = -DHAVE_LIBWILDMIDI
WILDMIDI_LIBS = -lWildMidi
OGGVORBIS_CFLAGS = -DHAVE_OGGVORBIS
OGGVORBIS_LIBS = -lvorbisfile -lvorbis -logg
FMT_LIBS = -lfmt
endif

ICU_LIBS = -licuuc -licui18n -licudata

ifeq ($(strip $(LIBLCF_DIR)),)
LIBLCF_DIR := $(TOOLCHAIN_DIR)
endif
LCF_CFLAGS = -I$(LIBLCF_DIR)/include
LCF_LIBS = -L$(LIBLCF_DIR)/lib -llcf

LIBS = $(LCF_LIBS) \
	$(EXPAT_LIBS) \
	$(PIXMAN_LIBS) \
	$(PNG_LIBS) \
	$(FREETYPE_LIBS) \
	$(WILDMIDI_LIBS) \
	$(ICU_LIBS) \
	$(SDL_LIBS) \
	$(OGGVORBIS_LIBS) \
	$(FMT_LIBS) \
	$(MPG123_LIBS)

CFLAGS  = -g -O2 -fno-math-errno
CXXFLAGS = $(CFLAGS) $(PLATFORM_CXXFLAGS) -std=gnu++14 -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
CPPFLAGS =  -DSUPPORT_AUDIO -DWANT_DRWAV -DWANT_FMMIDI \
		-I$(TOOLCHAIN_DIR)/include \
		$(LCF_CFLAGS) \
		$(SDL_CFLAGS) \
		$(PIXMAN_CFLAGS) \
		$(FREETYPE_CFLAGS) \
		$(MPG123_CFLAGS) \
		$(OGGVORBIS_CFLAGS)

LDFLAGS = -L$(TOOLCHAIN_DIR)/lib

all: $(TARGET).elf

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LIBS) -o $@
	$(PREFIX)-strip -g -o $(basename $@)-stripped.elf $@

clean:
	rm -rf $(TARGET).elf $(TARGET)-stripped.elf $(OBJS)
