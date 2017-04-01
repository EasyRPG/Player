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
	LIBMPG123_CFLAGS := 
	LIBMPG123_LIBS := 
	LIBSNDFILE_CFLAGS :=
	LIBSNDFILE_LIBS :=
	CXXFLAGS = 
	SDL_CONFIG = $(SYSROOT)/usr/bin/sdl-config
	LIBSDLMIXER_LIBS = -lSDL_mixer
else
ifeq ($(BUILD_TARGET), gcwzero)
	TARGET = gcw-zero/EasyRPG
	TOOLCHAINDIR ?= /opt/gcw0-toolchain
	SYSROOT = $(TOOLCHAINDIR)/usr/mipsel-gcw0-linux-uclibc/sysroot
	BINDIR = $(TOOLCHAINDIR)/usr/bin
	CFLAGS = -Ofast -fomit-frame-pointer -fdata-sections -ffunction-sections -mips32r2 -mmxu -std=gnu++11 -DLCF_SUPPORT_ICU 
	CFLAGS += -DGCW_ZERO -DWANT_FMMIDI=2 -DHAVE_MPG123 -DHAVE_LIBSNDFILE -DWANT_FASTWAV
	LDFLAGS = -Wl,--as-needed,--gc-sections -flto
	PKG_CONFIG := $(BINDIR)/pkg-config
	PNG_LIBS := $(shell $(PKG_CONFIG) --libs libpng)
	PIXMAN_CFLAGS := $(shell $(PKG_CONFIG) --static --cflags pixman-1)
	PIXMAN_LIBS := -Wl,-Bstatic $(shell $(PKG_CONFIG) --static --libs pixman-1)
	FREETYPE_CFLAGS := $(shell $(PKG_CONFIG) --cflags freetype2)
	ICU_CFLAGS := $(shell $(PKG_CONFIG) --cflags icu-uc icu-i18n)
	ICU_LIBS := $(shell $(PKG_CONFIG) --libs icu-uc icu-i18n) -Wl,-Bdynamic
	LIBMPG123_CFLAGS := $(shell $(PKG_CONFIG) --cflags libmpg123)
	LIBMPG123_LIBS :=  $(shell $(PKG_CONFIG) --static --libs libmpg123)
	LIBSNDFILE_CFLAGS := $(shell $(PKG_CONFIG) --cflags sndfile)
	LIBSNDFILE_LIBS := $(shell $(PKG_CONFIG) --static --libs sndfile)
	CXXFLAGS = 
	SDL_CONFIG = $(SYSROOT)/usr/bin/sdl2-config
	LIBSDLMIXER_LIBS = -lSDL2_mixer
else
$(error Unknown device $(BUILD_TARGET)! Valid choices: dingoo, gcwzero)
endif
endif

# Compiler headers
INCLUDES = ../../src ../../lib/liblcf/src ../../lib/liblcf/src/generated
INCDIR = $(foreach dir, $(INCLUDES), -I$(wildcard $(dir)))

# Compiler source
SOURCES = ../../lib/liblcf/src ../../src ../../lib/liblcf/src/generated
CPPFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.cpp))
BINFILES = $(foreach dir, $(DATA), $(wildcard $(dir)/*.*))
OBJS = $(addsuffix .o, $(BINFILES)) $(CPPFILES:.cpp=.o)

# Compiler flags
CFLAGS += -Wall -DUSE_SDL -DHAVE_SDL_MIXER -DSUPPORT_AUDIO `$(SDL_CONFIG) --cflags` $(PIXMAN_CFLAGS) $(FREETYPE_CFLAGS) $(ICU_CFLAGS) $(LIBMPG123_CFLAGS) $(LIBSNDFILE_CFLAGS)
CXXFLAGS += $(CFLAGS) -fno-rtti -fno-exceptions
LDFLAGS += -lstdc++ -lexpat -lz -lfreetype -lvorbisidec -lgcc -lm -lc -lpthread \
           -ldl `$(SDL_CONFIG) --libs` $(LIBSDLMIXER_LIBS) $(PIXMAN_LIBS) $(ICU_LIBS) $(PNG_LIBS) $(LIBMPG123_LIBS) $(LIBSNDFILE_LIBS)

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
