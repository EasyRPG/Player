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

# set(USE_OPENAL 1)

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

# OpenAL
if(USE_OPENAL)
  find_package(OpenAL REQUIRED)
  include_directories(${OPENAL_INCLUDE_DIR})
  add_definitions(-D USE_OPENAL=1)
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
if(USE_OPENAL)
  list(APPEND EASYRPG_PLAYER_LIBRARIES
    avformat avcodec avutil avdevice
    ${OPENAL_LIBRARY})
endif()
find_package(SDL_mixer)
if(SDLMIXER_FOUND)
  include_directories(${SDLMIXER_INCLUDE_DIR})
  list(APPEND EASYRPG_PLAYER_LIBRARIES ${SDLMIXER_LIBRARY})
else()
  add_definitions(-DNO_SDL_MIXER)
endif()

find_package(Lua51)
if(LUA51_FOUND)
  include_directories(${LUA_INCLUDE_DIR})
  list(APPEND EASYRPG_PLAYER_LIBRARIES ${LUA_LIBRARIES})
  add_definitions(-D HAVE_LUA=1)
endif()

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

set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/../../resources/player.ico")
set(CPACK_GENERATOR "DEB" "DMG" "RPM" "NSIS")
include(CPack)

# if(APPLE)
#   message(STATUS "${EASYRPG_PLAYER_LIBRARIES}")
#   include(BundleUtilities)
#   set(APP_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../bin/${PROJECT_NAME}.app")
#   verify_app(${APP_DIRECTORY})
#   copy_and_fixup_bundle("" ${APP_DIRECTORY} ${EASYRPG_PLAYER_LIBRARIES} "")
# endif()

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
