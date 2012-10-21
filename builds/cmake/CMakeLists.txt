cmake_minimum_required(VERSION 2.8)

project(EasyRPG_Player CXX C)

# reader
# - ${EASYRPG_READERS_PATH}

set(DEFAULT_EASYRPG_READERS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../lib/readers")
if(NOT EASYRPG_READERS_PATH)
  set(EASYRPG_READERS_PATH "${DEFAULT_EASYRPG_READERS_PATH}")
endif()
if(NOT EXISTS ${EASYRPG_READERS_PATH})
  find_package(Git REQUIRED)
  execute_process(COMMAND ${GIT_EXECUTABLE} clone
    "https://github.com/EasyRPG/Readers.git"
    "${EASYRPG_READERS_PATH}")
endif()
add_subdirectory(${EASYRPG_READERS_PATH}/builds/cmake ${EASYRPG_READERS_PATH}/builds/cmake)

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

if(USE_OPENAL)
  find_package(OpenAL REQUIRED)
  include_directories(${OPENAL_INCLUDE_DIR})
  add_definitions(-D USE_OPENAL=1)
endif()

# endianess check
include(TestBigEndian)
test_big_endian(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
  add_definitions(-D READER_BIG_ENDIAN=1)
endif()

if(APPLE)
  add_definitions(-D COCOA=1)
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  add_definitions(-D _DEBUG=1)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  # find gtk
  find_package(GTK2)
  if(GTK2_FOUND)
    add_definitions(-D GTK=1)
    include_directories(${GTK2_INCLUDE_DIRS})
    list(APPEND EASYRPG_PLAYER_LIBRARIES ${GTK2_LIBRARIES})
  endif()
endif()

foreach(i Iconv Expat Freetype JPEG Pixman PNG SDL SDL_mixer ZLIB Boost)
  find_package(${i} REQUIRED)

  string(TOUPPER ${i} i)
  string(REPLACE "SDL_" "SDL" sdl ${i})

  include_directories(
    ${${i}_INCLUDE_DIR} ${${i}_INCLUDE_DIRS}
    ${${sdl}_INCLUDE_DIR} ${${sdl}_INCLUDE_DIRS})
  list(APPEND EASYRPG_PLAYER_LIBRARIES
    ${${i}_LIBRARY}  ${${i}_LIBRARIES}
    ${${sdl}_LIBRARY}  ${${sdl}_LIBRARIES})
endforeach()
if(USE_OPENAL)
  list(APPEND EASYRPG_PLAYER_LIBRARIES
    avformat avcodec avutil avdevice
    ${OPENAL_LIBRARY})
endif()

find_package(Boost COMPONENTS filesystem system)
if(Boost_FOUND)
  list(APPEND EASYRPG_PLAYER_LIBRARIES ${Boost_LIBRARIES})
else()
  add_definitions(-DEASYRPG_BOOST_ONLY_HEADER)
  if(CMAKE_GENERATOR MATCHES "Makefile")
    add_definitions(-fno-exceptions)
  endif()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../lib/readers/include)

# library
set(EASYRPG_PLAYER_LIBRARIES_ALL
  ${EASYRPG_PLAYER_LIBRARIES} ${PROJECT_NAME}_Static EasyRPG_Reader)

# entry point
set(MAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../../src/main.cpp")

# static library
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../../src SRCS)
list(REMOVE_ITEM SRCS ${MAIN_FILE})
add_library(${PROJECT_NAME}_Static STATIC ${SRCS})

add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main.cpp)
target_link_libraries(${PROJECT_NAME} ${EASYRPG_PLAYER_LIBRARIES_ALL})

add_dependencies(${PROJECT_NAME} EasyRPG_Reader ${PROJECT_NAME}_Static)

set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURC_DIR}/../../resources/player.ico")
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

file(GLOB TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../test/*.cpp)
foreach(i ${TEST_FILES})
  get_filename_component(name ${i} NAME_WE)
  add_executable(test_${name} ${i})
  target_link_libraries(test_${name} ${EASYRPG_PLAYER_LIBRARIES_ALL})
  add_dependencies(test_${name} ${PROJECT_NAME}_Static)

  add_test(
    NAME test_${name}
    WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
    COMMAND ${EXECUTABLE_OUTPUT_PATH}/test_${name})
endforeach()
