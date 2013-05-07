##################################################################
#                 Makefile for Dingux/OpenDingux                 #
#                            By Shin-NiL                         #
#                     - Based on Rikku2000's -                   #
##################################################################

# Compiler target file
TARGET = EasyRPG.dge

# Compiler Device option
BUILD_DINGUX = NO
BUILD_OPENDINGUX = YES

# Host toolchain directory and extra flags
ifeq ($(BUILD_DINGUX), YES)
	TOOLCHAINDIR = /opt/mipsel-linux-uclibc
else
ifeq ($(BUILD_OPENDINGUX), YES)
	TOOLCHAINDIR = /opt/opendingux-toolchain
	CFLAGS = -O2 -fomit-frame-pointer -ffunction-sections -ffast-math -G0 -mbranch-likely
	LDFLAGS = -Wl,--gc-sections
	CXXFLAGS = 
endif
endif

BINDIR = $(TOOLCHAINDIR)/usr/bin
SDL_CONFIG = $(BINDIR)/sdl-config

# Compiler headers
INCLUDES = ../../lib/readers/include ../../src
INCDIR = $(foreach dir, $(INCLUDES), -I$(wildcard $(dir)))

# Compiler source
SOURCES = ../../lib/readers/src ../../lib/readers/src/generated ../../src
CPPFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.cpp))
BINFILES = $(foreach dir, $(DATA), $(wildcard $(dir)/*.*))
OBJS = $(addsuffix .o, $(BINFILES)) $(CPPFILES:.cpp=.o)



# Compiler flags
CFLAGS += -O2 -Wall -DUSE_SDL `$(SDL_CONFIG) --cflags`
CXXFLAGS += $(CFLAGS) -fno-exceptions -fno-rtti
LDFLAGS += -lstdc++ -lexpat -lSDL -lSDL_gfx -lSDL_ttf -lSDL_image -lSDL_mixer\
          -ljpeg -lpng12 -lz -lfreetype -lvorbisidec -lgcc -lm -lc -lpthread\
          -ldl `$(SDL_CONFIG) --libs`

# Host compiler and extra flags
HOST = $(BINDIR)/mipsel-linux-
CFLAGS += -DDINGOO

# Start compile
$(TARGET): $(OBJS)
	$(HOST)gcc $(OBJS) -o $@ $(LDFLAGS)
	$(HOST)strip $(TARGET)

.cpp.o:
	$(HOST)gcc $(INCDIR) $(CXXFLAGS) -c $< -o $@

# Clean Project
clean:
	rm -f $(OBJS) $(TARGET)
