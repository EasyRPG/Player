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
INCLUDES = ../../lib/readers/include ../../src
INCDIR = $(foreach dir, $(INCLUDES), -iquote $(wildcard $(dir)))

# Compiler source
SOURCES = ../../lib/readers/src ../../src
CPPFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.cpp))
BINFILES = $(foreach dir, $(DATA), $(wildcard $(dir)/*.*))
OBJS = $(addsuffix .o, $(BINFILES)) $(CPPFILES:.cpp=.o)

# Compiler flags
CFLAGS = -O2 -Wall -DGPH -DUSE_SDL `sdl-config --cflags`
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
LDFLAGS = -lexpat `sdl-config --libs`

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
$(TARGET): $(OBJS)
	$(HOST)gcc $(OBJS) -o $@ $(LDFLAGS)
	$(HOST)strip $(TARGET)

.cpp.o:
	$(HOST)gcc $(INCDIR) $(CXXFLAGS) -c $< -o $@

# Clean Project
clean:
	rm -f $(OBJS) $(TARGET)
