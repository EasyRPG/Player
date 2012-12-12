TARGET = Player.exe
SOURCES = ../../src ../../lib/readers/src ../../lib/readers/src/generated
SHINONOME_SRCS = \
	../../lib/shinonome/gothic.cxx \
	../../lib/shinonome/mincho.cxx \
	../../lib/shinonome/hankaku.cxx
CPPFILES = $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp)) ${SHINONOME_SRCS}
OBJ_DIR=build
OBJS = $(foreach f,${CPPFILES}, ${OBJ_DIR}/$(abspath ${f}).o)
INCLUDES = ../../lib/shinonome ../../lib/readers/include ../../src

CFLAGS = \
	-g \
	-I ${HOME_BUILD_PATH}/include \
	$(foreach dir,$(INCLUDES), -I $(CURDIR)/$(dir)) \
	-O2 -Wall -Wextra \
	-D USE_SDL -D USE_PIXMAN_BITMAP -D NO_SDL_MIXER \
	-D UNICODE -D _UNICODE -DWIN32 \
	-I ${HOME_BUILD_PATH}/include/pixman-1 \
	$(shell ${HOME_BUILD_PATH}/bin/sdl-config --cflags) \
	$(shell ${HOME_BUILD_PATH}/bin/freetype-config --cflags) \
	-DHAVE_OPENAL=1 -DHAVE_LUA=1 -DHABE_BOOST_LIBRARIES=1  -DAL_LIBTYPE_STATIC=1 \

CXXFLAGS = $(CFLAGS) -fexceptions -std=gnu++0x
ASFLAGS = $(CFLAGS)

LDFLAGS = -flto -mwindows -static-libgcc -static-libstdc++

LIBS += \
	-L ${HOME_BUILD_PATH}/lib \
	-lexpat -lpixman-1 -lpng -ljpeg -lSDLmain \
	$(shell ${HOME_BUILD_PATH}/bin/sdl-config --libs) \
	$(shell ${HOME_BUILD_PATH}/bin/freetype-config --libs) \
	 -lz -lm -lOpenAL32 -llua \
	-lsndfile -lvorbis -lvorbisenc -lvorbisfile -lFLAC -logg \
	-lole32 -loleaut32 -lwinmm -lws2_32 \

MINGW_PREFIX =i686-w64-mingw32

CC =${CCACHE} ${MINGW_PREFIX}-gcc
CXX=${CCACHE} ${MINGW_PREFIX}-g++

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

${OBJ_DIR}/%.cxx.o : %.cxx
	mkdir -p $(dir $@)
	${CXX}  ${CXXFLAGS} -c $< -o $@

SHINONOME_GENERATOR=../../lib/shinonome/generate_cxx_font.rb
${SHINONOME_SRCS} : ${SHINONOME_GENERATOR}
	ruby ${SHINONOME_GENERATOR}

${OBJ_DIR}/player.o :../../resources/player.rc ../../resources/player.ico ../../resources/player.xml
	${MINGW_PREFIX}-windres -i $< --input-format=rc -o $@ -O coff
