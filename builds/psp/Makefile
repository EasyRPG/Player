TARGET = EasyRPG
SOURCES = ../../src ../../lib/liblcf/src ../../lib/liblcf/src/generated
CPPFILES = $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
BINFILES = $(foreach dir,$(DATA), $(wildcard $(dir)/*.*))
OBJS = $(addsuffix .o,$(BINFILES)) $(CPPFILES:.cpp=.o)
INCLUDES = ../../lib/liblcf/include ../../src

INCDIR = $(foreach dir,$(INCLUDES), -iquote $(CURDIR)/$(dir))
CFLAGS = \
	-O2 -G0 -Wall -Wextra -D PSP \
	-D USE_SDL -D USE_PIXMAN_BITMAP
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS = -flto -fpic -fwhopr

EXTRA_TARGETS = EBOOT.PBP
EXTRA_CLEAN = my-clean

PSP_EBOOT_TITLE = EasyRPG
PSP_EBOOT_ICON = "144x80.png"
PSP_EBOOT_PIC1 = "480x272.png"
PSP_DIR_NAME = easyrpg

PSPSDK=$(shell psp-config --pspsdk-path)
PSPBIN = $(PSPSDK)/../bin
CFLAGS += \
	-I $(shell psp-config --psp-prefix)/include/pixman-1 \
	$(shell $(PSPBIN)/sdl-config --cflags) \
	$(shell $(PSPBIN)/freetype-config --cflags)
LIBS += \
	-lexpat -lpixman-1 \
	-lSDL_image -lpng -ljpeg \
	$(shell $(PSPBIN)/freetype-config --libs) \
	$(shell $(PSPBIN)/sdl-config --libs) \
	 -lz -lstdc++ -lm -lc \

#	-lSDL_mixer -lmikmod -lvorbisfile -lvorbis -lvorbisenc -logg \

include $(PSPSDK)/lib/build.mak

CC=ccache psp-gcc
CXX=ccache psp-g++
