#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

ifeq ($(strip $(TOOLCHAIN_DIR)),)
$(error "Please point TOOLCHAIN_DIR to your Wii portlibs directory")
endif

ifeq ($(strip $(LIBLCF_DIR)),)
$(warning "LIBLCF_DIR not set. Ignore this warning when liblcf is in TOOLCHAIN_DIR")
LIBLCF_DIR := $(TOOLCHAIN_DIR)
endif

include $(DEVKITPPC)/wii_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# TOPDIR is the directory from where relative paths to source files are resolved
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	boot
BUILD		:=	obj
TOPDIR		:=	$(subst /builds/wii,,$(CURDIR))
SOURCES		:=	src
DATA		:=	data  
INCLUDES	:=	src

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------

CFLAGS		= 	-g -O2 -Wall -Wextra \
				-fomit-frame-pointer -ffast-math \
				-flto -fdata-sections -ffunction-sections \
				-DUSE_SDL -DNDEBUG -DWANT_FMMIDI=2 -DHAVE_MPG123 \
				-DHAVE_LIBSNDFILE -DHAVE_TREMOR -DHAVE_WILDMIDI \
				-DHAVE_XMP -DHAVE_LIBSPEEXDSP -DWANT_FASTWAV \
				-DSUPPORT_AUDIO \
				$(MACHDEP) $(INCLUDE)
				
CXXFLAGS	= 	$(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11

LDFLAGS	=		-g $(MACHDEP) -Wl,-Map,easyrpg-player-wii.map,--gc-sections,-flto

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------

LIBS	:=	-llcf -lexpat -lpixman-1 -lpng -lz \
			-licuuc -licui18n -licudata \
			-lSDL -lvorbisidec -lmpg123 \
			-lsndfile -lWildMidi -lxmp-lite -lspeexdsp \
			-lfat -lwiiuse -lbte -logc -lm -lwiikeyboard

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(TOOLCHAIN_DIR) $(LIBLCF_DIR)

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(TOPDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(TOPDIR)/$(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(TOPDIR)/$(dir)/*.cpp)))
sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(TOPDIR)/$(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(TOPDIR)/$(dir)/*.S)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
					$(sFILES:.s=.o) $(SFILES:.S=.o)

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES), -I$(TOPDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD) \
					-I$(LIBOGC_INC) \
					-I$(TOOLCHAIN_DIR)/include/SDL \
					-I$(TOOLCHAIN_DIR)/include/pixman-1 \
					-I$(TOOLCHAIN_DIR)/include/libxmp-lite \
					-I$(LIBLCF_DIR)/include/liblcf

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
					-L$(LIBOGC_LIB)

export OUTPUT	:=	$(CURDIR)/$(TARGET)
.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).dol

#---------------------------------------------------------------------------------
run:
	wiiload $(TARGET).dol

#---------------------------------------------------------------------------------
echoconf:
	@echo Folder Configuration:
	@echo ---------------------
	@echo TOPDIR: $(TOPDIR)
	@echo
	@echo INCLUDE: $(INCLUDE)
	@echo
	@echo LIBPATHS: $(LIBPATHS)
	@echo

#---------------------------------------------------------------------------------
addr2line:
	@echo "Enter address(es) from stack dump (press ctrl+c to exit):"
	@powerpc-eabi-addr2line -f -i -C -e boot.elf

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

#---------------------------------------------------------------------------------

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
