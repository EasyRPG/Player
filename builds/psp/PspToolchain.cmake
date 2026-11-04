# CMake toolchain file for the PlayStation Portable (PSP).
#
# Requires the pspdev toolchain (https://github.com/pspdev/pspdev), the
# environment variable PSPDEV must point to its install prefix, e.g.
#   cmake -DCMAKE_TOOLCHAIN_FILE=builds/psp/PspToolchain.cmake ...
#
# The pspdev-provided toolchain file sets up the compilers, the PSPSDK include
# and library paths, and the create_pbp_file() helper used for EBOOT.PBP.
# Libraries (zlib, libpng, pixman, fmt, inih, liblcf) are installed into
# ${PSPDEV}/psp, see builds/psp/build-deps.sh.

if(NOT DEFINED ENV{PSPDEV})
	message(FATAL_ERROR "PSPDEV is not set. Point it at the pspdev toolchain prefix.")
endif()

include("$ENV{PSPDEV}/psp/share/pspdev.cmake")

# Checked by the top-level CMakeLists.txt, like VITA or NINTENDO_3DS
set(PSP TRUE)
