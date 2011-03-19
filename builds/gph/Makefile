TARGET = EasyRPG.gpe

BUILD_GP2XWIZ = NO
BUILD_CAANOO = YES

INCLUDES = -I../../../readers/include

SOURCES = ../../../readers/src ../../src
CPPFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.cpp))
OBJS = $(CPPFILES:.o=.cpp)

CFLAGS = -O2 -Wall -DGPH -DUSE_SDL -DUSE_JOYSTICK `sdl-config --cflags`
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LDFLAGS = `sdl-config --libs` -lstdc++

ifeq ($(BUILD_GP2XWIZ), YES)
	HOST = arm-gp2x-linux-
	CFLAGS += -DUSE_WIZ
else
ifeq ($(BUILD_CAANOO), YES)
	HOST = arm-gph-linux-gnueabi-
	CFLAGS += -DUSE_CAANOO
endif
endif

$(TARGET):
	$(HOST)g++ $(CFLAGS) $(INCLUDES) $(OBJS) -o $(TARGET) $(LDFLAGS)
	$(HOST)strip $(TARGET)
