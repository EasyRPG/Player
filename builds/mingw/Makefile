TARGET = Player.exe
SOURCES = ../../src ../../lib/readers/src ../../lib/readers/src/generated
CPPFILES = $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
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

CC=ccache i686-w64-mingw32-gcc
CXX=ccache i686-w64-mingw32-g++

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

${OBJ_DIR}/player.o :../../resources/player.rc ../../resources/player.ico ../../resources/player.xml
	i686-w64-mingw32-windres -i $< --input-format=rc -o $@ -O coff
