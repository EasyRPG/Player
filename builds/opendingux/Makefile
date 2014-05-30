##################################################################
#         Makefile for OpenDingux (Dingoo A320/GCW-Zero)         #
#                            By Shin-NiL                         #
#                     - Based on Rikku2000's -                   #
##################################################################

# Compiler Device option
BUILD_DINGOO = NO
BUILD_GCWZERO = YES

# Host toolchain directory and extra flags
ifeq ($(BUILD_DINGOO), YES)
	TARGET = dingoo/EasyRPG
	TOOLCHAINDIR = /opt/opendingux-toolchain
	SYSROOT = $(TOOLCHAINDIR)
	CFLAGS = -O2 -fomit-frame-pointer -ffunction-sections -ffast-math -G0 -mbranch-likely -std=gnu++11
	LDFLAGS = -Wl,--gc-sections
	PNG_LIBS = -lpng12
	CXXFLAGS = 
else
ifeq ($(BUILD_GCWZERO), YES)
	TARGET = gcw-zero/EasyRPG
	TOOLCHAINDIR = /opt/gcw0-toolchain
	SYSROOT = $(TOOLCHAINDIR)/usr/mipsel-gcw0-linux-uclibc/sysroot
	CFLAGS = -O2 -fomit-frame-pointer -ffunction-sections -ffast-math -G0 -std=gnu++11
	LDFLAGS = -Wl,--gc-sections
	PNG_LIBS = -lpng14
	CXXFLAGS = 
endif
endif

BINDIR = $(TOOLCHAINDIR)/usr/bin
SDL_CONFIG = $(SYSROOT)/usr/bin/sdl-config
PIXMAN_CFLAGS = -I$(SYSROOT)/usr/include/pixman-1
PIXMAN_LIBS =  $(SYSROOT)/usr/lib/libpixman-1.a
FREETYPE_CFLAGS = -I$(SYSROOT)/usr/include/freetype2

# Compiler headers
INCLUDES = ../../src ../../lib/shinonome ../../lib/liblcf/src ../../lib/liblcf/src/generated
INCDIR = $(foreach dir, $(INCLUDES), -I$(wildcard $(dir)))

# Compiler source
SOURCES = ../../lib/liblcf/src ../../src ../../lib/liblcf/src/generated ../../lib/shinonome
CPPFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.cpp))
BINFILES = $(foreach dir, $(DATA), $(wildcard $(dir)/*.*))
OBJS = $(addsuffix .o, $(BINFILES)) $(CPPFILES:.cpp=.o)
OBJS += gothic.o mincho.o


# Compiler flags
CFLAGS += -O2 -Wall -DUSE_SDL -DHAVE_SDL_MIXER `$(SDL_CONFIG) --cflags` $(PIXMAN_CFLAGS) $(FREETYPE_CFLAGS)
CXXFLAGS += $(CFLAGS) -fexceptions -fno-rtti
LDFLAGS += -lstdc++ -lexpat -lSDL -lSDL_gfx -lSDL_ttf -lSDL_image -lSDL_mixer\
          -ljpeg -lz -lfreetype -lvorbisidec -lgcc -lm -lc -lpthread\
          -ldl -liconv `$(SDL_CONFIG) --libs` $(PIXMAN_LIBS) $(PNG_LIBS)

# Host compiler and extra flags
HOST = $(BINDIR)/mipsel-linux-
CFLAGS += -DOPENDINGUX

# Start compile
$(TARGET): $(OBJS)
	$(HOST)gcc $(OBJS) -o $@ $(LDFLAGS)
	$(HOST)strip $(TARGET)

.cpp.o:
	$(HOST)gcc $(INCDIR) $(CXXFLAGS) -c $< -o $@

gothic.o: ../../lib/shinonome/gothic.cxx
	$(HOST)gcc -c ../../lib/shinonome/gothic.cxx

mincho.o: ../../lib/shinonome/mincho.cxx
	$(HOST)gcc -c ../../lib/shinonome/mincho.cxx



# Clean Project
clean:
	rm -f $(OBJS) $(TARGET) 
