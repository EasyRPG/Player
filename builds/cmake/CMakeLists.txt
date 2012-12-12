cmake_minimum_required(VERSION 2.8)

project(EasyRPG_Player CXX C)

# reader
# - ${EASYRPG_READERS_PATH}

set(DEFAULT_EASYRPG_READERS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../lib/readers")
if(NOT EASYRPG_READERS_PATH)
  set(EASYRPG_READERS_PATH "${DEFAULT_EASYRPG_READERS_PATH}")
endif()
find_package(Git REQUIRED)
if(NOT EXISTS ${EASYRPG_READERS_PATH})
  execute_process(COMMAND ${GIT_EXECUTABLE} clone
    "https://github.com/EasyRPG/Readers.git"
    "${EASYRPG_READERS_PATH}")
endif()
add_subdirectory(
  ${EASYRPG_READERS_PATH}/builds/cmake
  ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${EASYRPG_READERS_PATH}/builds/cmake)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../lib)
link_directories(${LIBRARY_OUTPUT_PATH})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Modules")

add_definitions(
  ${PNG_DEFINITIONS}

  -D UNIX=1
  -D USE_SDL=1)

if(CMAKE_GENERATOR MATCHES "Makefile")
  add_definitions(-fno-rtti)
endif()

# shinonome font
set(SHINONOME_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../lib/shinonome")
set(SHINONOME_SRCS
  "${SHINONOME_PATH}/hankaku.cxx" "${SHINONOME_PATH}/latin1.cxx"
  "${SHINONOME_PATH}/gothic.cxx" "${SHINONOME_PATH}/mincho.cxx")
include_directories(${SHINONOME_PATH})
find_package(Ruby)
if(RUBY_FOUND)
  set(SHINONOME_GENERATOR "${SHINONOME_PATH}/generate_cxx_font.rb")
  add_custom_command(
    OUTPUT ${SHINONOME_SRCS} "${SHINONOME_PATH}/shinonome.hxx"
    COMMAND "${RUBY_EXECUTABLE}" "${SHINONOME_GENERATOR}"
    DEPENDS "${SHINONOME_GENERATOR}" WORKING_DIRECTORY "${SHINONOME_PATH}")
endif()

# endianess check
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  include(TestBigEndian)
  test_big_endian(IS_BIG_ENDIAN)
  if(IS_BIG_ENDIAN)
    add_definitions(-D READER_BIG_ENDIAN=1)
  endif()
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  add_definitions(-D _DEBUG=1)
endif()

foreach(i Iconv Expat Freetype JPEG Pixman PNG SDL ZLIB Boost)
  find_package(${i} REQUIRED)

  string(TOUPPER ${i} i)

  include_directories(${${i}_INCLUDE_DIR} ${${i}_INCLUDE_DIRS})
  list(APPEND EASYRPG_PLAYER_LIBRARIES ${${i}_LIBRARY}  ${${i}_LIBRARIES})
endforeach()

# SDL_mixer
find_package(SDL_mixer)
if(SDLMIXER_FOUND)
  include_directories(${SDLMIXER_INCLUDE_DIR})
  list(APPEND EASYRPG_PLAYER_LIBRARIES ${SDLMIXER_LIBRARY})
  add_definitions(-D HAVE_SDL_MIXER=1)
else()
  add_definitions(-DNO_SDL_MIXER)
endif()

# OpenAL
find_package(OpenAL)
if(OPENAL_FOUND)
  find_package(SndFile REQUIRED)
  include_directories(${OPENAL_INCLUDE_DIR} ${SNDFILE_INCLUDE_DIR})
  list(APPEND EASYRPG_PLAYER_LIBRARIES ${OPENAL_LIBRARY} ${SNDFILE_LIBRARIES})
  add_definitions(-D HAVE_OPENAL=1)
endif()

# Lua 5.1
find_package(Lua51)
if(LUA51_FOUND)
  include_directories(${LUA_INCLUDE_DIR})
  list(APPEND EASYRPG_PLAYER_LIBRARIES ${LUA_LIBRARIES})
  add_definitions(-D HAVE_LUA=1)
endif()

# Boost (libraries that need to link)
unset(Boost_FOUND)
find_package(Boost COMPONENTS thread chrono system)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  list(APPEND EASYRPG_PLAYER_LIBRARIES ${Boost_LIBRARIES})
  add_definitions(-D HAVE_BOOST_LIBRARIES)
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../lib/readers/include")

# library
set(EASYRPG_PLAYER_LIBRARIES_ALL
  ${EASYRPG_PLAYER_LIBRARIES} ${PROJECT_NAME}_Static EasyRPG_Reader)

# entry point
set(MAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../../src/main.cpp")

# static library
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../../src SRCS)
list(REMOVE_ITEM SRCS ${MAIN_FILE})
list(APPEND SRCS ${SHINONOME_SRCS})
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
  list(APPEND SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../../resources/player.rc")
endif()
add_library(${PROJECT_NAME}_Static STATIC ${SRCS})

# executable
add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${MAIN_FILE})
add_executable(${PROJECT_NAME}_com ${MAIN_FILE})
foreach(i ${PROJECT_NAME} ${PROJECT_NAME}_com)
  target_link_libraries(${i} ${EASYRPG_PLAYER_LIBRARIES_ALL})
  add_dependencies(${i} EasyRPG_Reader ${i}_Static)
endforeach()

# RTP Table
if(NOT WIN32)
  add_custom_target(
    rtp_table_bom
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/../../resources/add_bom.sh"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/rtp_table.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../src/rtp_table_bom.h"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../../src/rtp_table.h")
  add_dependencies("${PROJECT_NAME}" rtp_table_bom)
endif()

# CPack
set(CPACK_GENERATOR "ZIP" "TGZ")
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  list(APPEND CPACK_GENERATOR "NSIS")
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  list(APPEND CPACK_GENERATOR "DEB")
endif()
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  list(APPEND CPACK_GENERATOR "DMG")
endif()

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION "0.1")
set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/../../resources/player.ico")
set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/../../README)

set(CPACK_DEBIAN_PACKAGE_NAME "easyrpg-player")
set(CPACK_DEBIAN_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://easy-rpg.org/")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "take_cheeze")
set(CPACK_DEBIAN_PACKAGE_DEPENDS
  libexpat1 libfreetype6 libjpeg8 libpixman-1-0 libpng12-0 zlib1g libsdl1.2debian
  libsdl-mixer1.2 libopenal1 libogg0 libflac8 libvorbis0a libvorbisenc2 libsndfile1
  liblua5.1-0 libboost-thread-dev libboost-chrono-dev libboost-system-dev libstdc++6
  libgcc1 libasound2 libpulse0 libxext6 libcaca0 libmikmod2 libvorbisfile3 libmad0
  libjson0 libslang2 libncurses5 libtinfo5 libwrap0 libasyncns0 libxau6 libxdmcp6)

include(CPack)

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME} DESTINATION bin)

# Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
  add_custom_target(player_doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../Doxyfile
    DEPENDS ${SRCS} ${MAIN_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
    COMMENT "Generating API documentation with Doxygen" VERBATIM)
  add_custom_target(doc)
  add_dependencies(doc player_doc reader_doc)
endif()

# test
enable_testing()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src)

set(TEST_GAME_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../lib/TestGame")
if(NOT EXISTS ${TEST_GAME_PATH})
  execute_process(COMMAND ${GIT_EXECUTABLE} clone
    "https://github.com/EasyRPG/TestGame.git"
    "${TEST_GAME_PATH}")
endif()


file(GLOB TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../test/*.cpp)
foreach(i ${TEST_FILES})
  get_filename_component(name "${i}" NAME_WE)
  add_executable(test_${name} ${i})
  target_link_libraries(test_${name} ${EASYRPG_PLAYER_LIBRARIES_ALL})
  add_dependencies(test_${name} ${PROJECT_NAME}_Static)

  add_test(
    NAME test_${name}
    WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
    COMMAND ${EXECUTABLE_OUTPUT_PATH}/test_${name})
  set_tests_properties(test_${name}
    PROPERTIES ENVIRONMENT "RPG_TEST_GAME_PATH=${TEST_GAME_PATH}/TestGame")
endforeach()

file(GLOB LUA_TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../test/lua_script/*.lua)
foreach(i ${LUA_TEST_FILES})
  get_filename_component(name "${i}" NAME_WE)
  add_test(NAME lua_test_${name}
    WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
    COMMAND ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}_com)

  set_tests_properties(lua_test_${name}
    PROPERTIES ENVIRONMENT "RPG_LUABOT_SCRIPT=${i}")
  set_tests_properties(lua_test_${name}
    PROPERTIES ENVIRONMENT "RPG_TEST_GAME_PATH=${TEST_GAME_PATH}/TestGame")
endforeach()
