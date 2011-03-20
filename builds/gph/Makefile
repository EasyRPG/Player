##################################################################
#                           - Cygwin -                           #
#                      Makefile for Devices                      #
#                        - by Rikku2000 -                        #
##################################################################

# Compiler target file
TARGET = EasyRPG.gpe

# Compiler Device option
BUILD_GP2XWIZ = NO
BUILD_CAANOO = YES

# Compiler headers
INCLUDES = ../../../readers/include ../../src
INCDIR = $(foreach dir, $(INCLUDES), -iquote $(wildcard $(dir)))

# Compiler source
SOURCES = ../../../readers/src ../../src
CPPFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.cpp))
BINFILES = $(foreach dir, $(DATA), $(wildcard $(dir)/*.*))
OBJS = $(addsuffix .o, $(BINFILES)) $(CPPFILES:.cpp=.o)

# Compiler flags
CFLAGS = -O2 -Wall -DGPH -DUSE_SDL `sdl-config --cflags`
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
LDFLAGS = `sdl-config --libs` -lstdc++

# Host compiler and extra flags
ifeq ($(BUILD_GP2XWIZ), YES)
	HOST = arm-gp2x-linux-
	CFLAGS += -DUSE_GP2XWIZ
else
ifeq ($(BUILD_CAANOO), YES)
	HOST = arm-gph-linux-gnueabi-
	CFLAGS += -DUSE_CAANOO
endif
endif

# Start compile
$(OBJS): $(CPPFILES)
	$(HOST)g++ $(INCDIR) $(CXXFLAGS) -c $(CPPFILES) >stdout.txt 2>stderr.txt

$(TARGET):
	$(HOST)g++ -o $(TARGET) $(OBJS) $(LDFLAGS)
	$(HOST)strip $(TARGET)