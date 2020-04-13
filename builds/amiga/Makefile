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
PLATFORM_CXXFLAGS = -D_GLIBCXX_USE_C99
TOOLCHAIN_DIR := /gg/usr
SDL_LIBS = -L$(TOOLCHAIN_DIR)/local/lib -lSDL_mixer -lSDL
SDL_CFLAGS = -I$(TOOLCHAIN_DIR)/local/include/SDL -I$(TOOLCHAIN_DIR)/local/include
PIXMAN_LIBS = -lpixman
PNG_LIBS =  -lpng_shared
MPG123_LIBS = -lmpg123
endif
ifeq ($(OSTYPE), AmigaOS)
PREFIX  = ppc-amigaos
CC      = $(PREFIX)-gcc
CXX     = $(PREFIX)-g++
PLATFORM_CXXFLAGS = -D_GLIBCXX_USE_C99_STDINT_TR1 -D_GLIBCXX_USE_C99_STDIO
TOOLCHAIN_DIR := /usr/local/amiga/ppc-amigaos/SDK/local/newlib
SDL_LIBS = -lSDL_mixer -lmikmod -lFLAC -lmodplug -lsmpeg -lSDL -athread=native
SDL_CFLAGS = -I$(TOOLCHAIN_DIR)/include/SDL
PIXMAN_LIBS = -lpixman-1
PNG_LIBS = -lpng -lz
MPG123_LIBS = -lmpg123
endif

ICU_LIBS = -licuuc -licui18n -licudata

LIBS = -llcf -lexpat \
	$(PIXMAN_LIBS) $(PNG_LIBS) \
	-lfreetype $(MPG123_LIBS) \
	-lvorbisfile -lvorbis -logg -lWildMidi \
	$(ICU_LIBS) \
	$(SDL_LIBS)

ifeq ($(strip $(LIBLCF_DIR)),)
LIBLCF_DIR := $(TOOLCHAIN_DIR)
endif

CFLAGS  = -g -O2 -fno-math-errno
CXXFLAGS = $(CFLAGS) $(PLATFORM_CXXFLAGS) -std=gnu++14 -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
CPPFLAGS = -DUSE_SDL -DHAVE_SDL_MIXER -DHAVE_FREETYPE -DSUPPORT_AUDIO \
		-DHAVE_MPG123 -DHAVE_OGGVORBIS -DHAVE_WILDMIDI -DWANT_FASTWAV \
		-I$(TOOLCHAIN_DIR)/include \
		$(SDL_CFLAGS) \
		-I$(LIBLCF_DIR)/include/liblcf \
		-I$(TOOLCHAIN_DIR)/include/pixman-1 \
		-I$(TOOLCHAIN_DIR)/include/freetype2 \
		-I$(TOOLCHAIN_DIR)/include/libxmp-lite \
		-I$(TOOLCHAIN_DIR)/include/opus

LDFLAGS = -L$(TOOLCHAIN_DIR)/lib \
		-L$(LIBLCF_DIR)/lib

all: $(TARGET).elf

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(LIBS) -o $@
	$(PREFIX)-strip -g -o $(basename $@)-stripped.elf $@

clean:
	rm -rf $(TARGET).elf $(TARGET)-stripped.elf $(OBJS)
