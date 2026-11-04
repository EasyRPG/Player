# CMake toolchain file for the PlayStation Portable (PSP).
#
# Requires the pspdev toolchain (https://github.com/pspdev/pspdev), the
# environment variable PSPDEV must point to its install prefix, e.g.
#   cmake -DCMAKE_TOOLCHAIN_FILE=builds/psp/PspToolchain.cmake ...
#
# Libraries (zlib, libpng, pixman, fmt, inih, liblcf) must be installed into
# ${PSPDEV}/psp, see builds/psp/build-deps.sh.

if(NOT DEFINED ENV{PSPDEV})
	message(FATAL_ERROR "PSPDEV is not set. Point it at the pspdev toolchain prefix.")
endif()

set(PSPDEV "$ENV{PSPDEV}")

# Checked by the top-level CMakeLists.txt, like VITA or NINTENDO_3DS
set(PSP TRUE)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR mips)

set(CMAKE_C_COMPILER "${PSPDEV}/bin/psp-gcc")
set(CMAKE_CXX_COMPILER "${PSPDEV}/bin/psp-g++")
set(CMAKE_AR "${PSPDEV}/bin/psp-ar")
set(CMAKE_RANLIB "${PSPDEV}/bin/psp-ranlib")
set(CMAKE_STRIP "${PSPDEV}/bin/psp-strip")

# Only the PSP sysroot may provide headers and libraries
set(CMAKE_FIND_ROOT_PATH "${PSPDEV}/psp" "${PSPDEV}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
