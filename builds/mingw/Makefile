TARGET = Player.exe
SOURCES = ../../src ../../lib/readers/src ../../lib/readers/src/generated
SHINONOME_SRCS = \
	../../lib/shinonome/gothic.cxx \
	../../lib/shinonome/mincho.cxx \
	../../lib/shinonome/hankaku.cxx
CPPFILES = $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp)) ${SHINONOME_SRCS}
OBJ_DIR=build
OBJS = $(foreach f,${CPPFILES}, ${OBJ_DIR}/$(abspath ${f}).o)
INCLUDES = ../../lib/readers/include ../../src

CFLAGS = \
	-g \
	-I ${HOME_MINGW_PATH}/include \
	$(foreach dir,$(INCLUDES), -I $(CURDIR)/$(dir)) \
	-O2 -Wall -Wextra \
	-D USE_SDL -D USE_PIXMAN_BITMAP -D NO_SDL_MIXER \
	-D UNICODE -D _UNICODE -DWIN32 \
	-I ${HOME_MINGW_PATH}/include/pixman-1 \
	$(shell ${HOME_MINGW_PATH}/bin/sdl-config --cflags) \
	$(shell ${HOME_MINGW_PATH}/bin/freetype-config --cflags) \

CXXFLAGS = $(CFLAGS) -fno-exceptions
ASFLAGS = $(CFLAGS)

LDFLAGS = -flto -mno-mingw -mwindows -static-libgcc -static-libstdc++

LIBS += \
	-L ${HOME_MINGW_PATH}/lib \
	-lexpat -lpixman-1 -lpng -ljpeg -lSDLmain \
	$(shell ${HOME_MINGW_PATH}/bin/sdl-config --libs) \
	$(shell ${HOME_MINGW_PATH}/bin/freetype-config --libs) \
	 -lz -lm \

MINGW_PREFIX =i686-w64-mingw32

CC =ccache ${MINGW_PREFIX}-gcc
CXX=ccache ${MINGW_PREFIX}-g++

OBJS += ${OBJ_DIR}/player.o

.PHONY:all
all : ${TARGET}

.PHONY:clean
clean :
	rm -rf ${OBJ_DIR}

${TARGET} : ${OBJS}
	${CXX} ${CXXFLAGS} ${LDFLAGS} ${OBJS} ${LIBS} -o ${TARGET}

${OBJ_DIR}/%.cpp.o : %.cpp
	mkdir -p $(dir $@)
	${CXX}  ${CXXFLAGS} -c $< -o $@

SHINONOME_GENERATOR=../../lib/shinonome/generate_cxx_font.rb
${SHINONOME_SRCS} : ${SHINONOME_GENERATOR}
	ruby ${SHINONOME_GENERATOR}

${OBJ_DIR}/player.o :../../resources/player.rc ../../resources/player.ico ../../resources/player.xml
	${MINGW_PREFIX}-windres -i $< --input-format=rc -o $@ -O coff
