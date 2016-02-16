##################################################################
#         Makefile for OpenDingux (Dingoo A320/GCW-Zero)         #
#                            By Shin-NiL                         #
#                     - Based on Rikku2000's -                   #
##################################################################

# Compiler Device option
BUILD_TARGET ?= gcwzero

# Host toolchain directory and extra flags
ifeq ($(BUILD_TARGET), dingoo)
	TARGET = dingoo/EasyRPG
	TOOLCHAINDIR ?= /opt/opendingux-toolchain
	SYSROOT = $(TOOLCHAINDIR)
	BINDIR = $(TOOLCHAINDIR)/usr/bin
	CFLAGS = -O2 -fomit-frame-pointer -ffunction-sections -ffast-math -G0 -mbranch-likely -std=gnu++11
	LDFLAGS = -Wl,--gc-sections
	PNG_LIBS = -lpng12
	PIXMAN_CFLAGS := -I$(SYSROOT)/usr/include/pixman-1
	PIXMAN_LIBS := $(SYSROOT)/usr/lib/libpixman-1.a
	FREETYPE_CFLAGS := -I$(SYSROOT)/usr/include/freetype2
	ICU_CFLAGS := 
	ICU_LIBS := 
	CXXFLAGS = 
else
ifeq ($(BUILD_TARGET), gcwzero)
	TARGET = gcw-zero/EasyRPG
	TOOLCHAINDIR ?= /opt/gcw0-toolchain
	SYSROOT = $(TOOLCHAINDIR)/usr/mipsel-gcw0-linux-uclibc/sysroot
	BINDIR = $(TOOLCHAINDIR)/usr/bin
	CFLAGS = -O2 -fomit-frame-pointer -ffunction-sections -ffast-math -G0 -std=gnu++11 -DLCF_SUPPORT_ICU
	LDFLAGS = -Wl,--gc-sections
	PKG_CONFIG := $(BINDIR)/pkg-config
	PNG_LIBS := $(shell $(PKG_CONFIG) --libs libpng)
	PIXMAN_CFLAGS := $(shell $(PKG_CONFIG) --static --cflags pixman-1)
	PIXMAN_LIBS := -Wl,-Bstatic $(shell $(PKG_CONFIG) --static --libs pixman-1)
	FREETYPE_CFLAGS := $(shell $(PKG_CONFIG) --cflags freetype2)
	ICU_CFLAGS := $(shell $(PKG_CONFIG) --cflags icu-uc icu-i18n)
	ICU_LIBS := $(shell $(PKG_CONFIG) --libs icu-uc icu-i18n) -Wl,-Bdynamic
	CXXFLAGS = 
else
$(error Unknown device $(BUILD_TARGET)! Valid choices: dingoo, gcwzero)
endif
endif

SDL_CONFIG = $(SYSROOT)/usr/bin/sdl-config

# Compiler headers
INCLUDES = ../../src ../../lib/liblcf/src ../../lib/liblcf/src/generated
INCDIR = $(foreach dir, $(INCLUDES), -I$(wildcard $(dir)))

# Compiler source
SOURCES = ../../lib/liblcf/src ../../src ../../lib/liblcf/src/generated
CPPFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.cpp))
BINFILES = $(foreach dir, $(DATA), $(wildcard $(dir)/*.*))
OBJS = $(addsuffix .o, $(BINFILES)) $(CPPFILES:.cpp=.o)

# Compiler flags
CFLAGS += -Wall -DUSE_SDL -DHAVE_SDL_MIXER `$(SDL_CONFIG) --cflags` $(PIXMAN_CFLAGS) $(FREETYPE_CFLAGS) $(ICU_CFLAGS)
CXXFLAGS += $(CFLAGS) -fexceptions -fno-rtti
LDFLAGS += -lstdc++ -lexpat -lSDL -lSDL_mixer -lz -lfreetype -lvorbisidec -lgcc -lm -lc -lpthread \
           -ldl `$(SDL_CONFIG) --libs` $(PIXMAN_LIBS) $(ICU_LIBS) $(PNG_LIBS)

# Host compiler and extra flags
HOST = $(BINDIR)/mipsel-linux-
CFLAGS += -DOPENDINGUX

# Start compile
$(TARGET): $(OBJS)
	$(HOST)gcc $(OBJS) -o $@ $(LDFLAGS)
	$(HOST)strip $(TARGET)

.cpp.o:
	$(HOST)gcc $(INCDIR) $(CXXFLAGS) -c $< -o $@

# Clean Project
clean:
	rm -f $(OBJS) $(TARGET) 
