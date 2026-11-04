#!/usr/bin/env bash
# Cross-compiles the libraries EasyRPG Player needs for the PlayStation Portable
# and installs them into ${PSPDEV}/psp.
#
# The pspdev image already ships zlib, libpng, pixman and fmt in ${PSPDEV}/psp,
# so only inih and liblcf are built here. Missing libraries are built too.
#
# Usage: PSPDEV=/usr/local/pspdev builds/psp/build-deps.sh
#
# Needs: git, cmake, make or ninja, the pspdev toolchain. meson is only needed
# when pixman is missing from the sysroot.

set -euo pipefail

export PSPDEV="${PSPDEV:-/usr/local/pspdev}"

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
TOOLCHAIN="${ROOT}/builds/psp/PspToolchain.cmake"
PREFIX="${PSPDEV}/psp"
WORK="${PSP_DEPS_WORK:-${ROOT}/psp-deps}"
JOBS="${JOBS:-$(nproc)}"

# Pinned versions
ZLIB_REF=v1.3.1
LIBPNG_REF=v1.6.43
PIXMAN_VERSION=0.44.2
FMT_REF=10.2.1
INIH_REF=r58
LIBLCF_REF="${LIBLCF_REF:-master}"  # same branch the stable CI builds (PLAYER_BUILD_LIBLCF_BRANCH)

mkdir -p "${WORK}"

# Generator: Ninja when available, the default one otherwise
GENERATOR=()
if command -v ninja >/dev/null 2>&1; then
	GENERATOR=(-G Ninja)
fi

log() {
	echo "==> $1"
}

fetch_git() { # <url> <ref> <name>
	if [[ ! -d "${WORK}/$3" ]]; then
		git clone --depth 1 --branch "$2" "$1" "${WORK}/$3"
	fi
}

cmake_lib() { # <name> <source dir> <extra cmake args...>
	local name=$1 src=$2
	shift 2
	cmake -S "${src}" -B "${WORK}/build-${name}" "${GENERATOR[@]}" \
		-DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN}" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX="${PREFIX}" \
		-DBUILD_SHARED_LIBS=OFF \
		"$@"
	cmake --build "${WORK}/build-${name}" --parallel "${JOBS}"
	cmake --install "${WORK}/build-${name}"
}

# True when the sysroot already has the library and its header
have_sysroot() { # <library file> <header>
	[[ -f "${PREFIX}/lib/$1" && -f "${PREFIX}/include/$2" ]]
}

# zlib
if have_sysroot libz.a zlib.h; then
	log "zlib: already in ${PREFIX}"
else
	log "zlib ${ZLIB_REF}"
	fetch_git https://github.com/madler/zlib.git "${ZLIB_REF}" zlib
	cmake_lib zlib "${WORK}/zlib" -DZLIB_BUILD_TESTING=OFF
fi

# libpng (needs zlib)
if have_sysroot libpng.a png.h; then
	log "libpng: already in ${PREFIX}"
else
	log "libpng ${LIBPNG_REF}"
	fetch_git https://github.com/pnggroup/libpng.git "${LIBPNG_REF}" libpng
	cmake_lib libpng "${WORK}/libpng" \
		-DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF -DPNG_TOOLS=OFF
fi

# pixman (meson only). No threads, no SIMD: the Allegrex CPU has neither.
if have_sysroot libpixman-1.a pixman-1/pixman.h; then
	log "pixman: already in ${PREFIX}"
else
	log "pixman ${PIXMAN_VERSION}"
	if [[ ! -d "${WORK}/pixman-${PIXMAN_VERSION}" ]]; then
		curl -fsSL "https://cairographics.org/releases/pixman-${PIXMAN_VERSION}.tar.gz" \
			| tar -xz -C "${WORK}"
	fi
	cat > "${WORK}/psp-cross.ini" <<EOF
[binaries]
c = '${PSPDEV}/bin/psp-gcc'
cpp = '${PSPDEV}/bin/psp-g++'
ar = '${PSPDEV}/bin/psp-ar'
strip = '${PSPDEV}/bin/psp-strip'

[host_machine]
system = 'generic'
cpu_family = 'mips'
cpu = 'allegrex'
endian = 'little'

[properties]
needs_exe_wrapper = true
EOF
	meson setup "${WORK}/build-pixman" "${WORK}/pixman-${PIXMAN_VERSION}" \
		--cross-file "${WORK}/psp-cross.ini" \
		--prefix "${PREFIX}" \
		--default-library static \
		--buildtype release \
		-Dopenmp=disabled -Dtls=disabled -Dgtk=disabled -Dlibpng=disabled \
		-Dtests=disabled -Ddemos=disabled \
		-Dmmx=disabled -Dsse2=disabled -Dssse3=disabled -Dvmx=disabled \
		-Darm-simd=disabled -Dneon=disabled -Da64-neon=disabled \
		-Dmips-dspr2=disabled -Dloongson-mmi=disabled
	ninja -C "${WORK}/build-pixman" -j "${JOBS}"
	ninja -C "${WORK}/build-pixman" install
fi

# fmt
if have_sysroot libfmt.a fmt/format.h; then
	log "fmt: already in ${PREFIX}"
else
	log "fmt ${FMT_REF}"
	fetch_git https://github.com/fmtlib/fmt.git "${FMT_REF}" fmt
	cmake_lib fmt "${WORK}/fmt" -DFMT_TEST=OFF -DFMT_DOC=OFF
fi

# inih: upstream has no CMake file at this tag. liblcf needs ini.c and the
# INIReader C++ wrapper, so a small CMake project builds and installs them.
log "inih ${INIH_REF}"
fetch_git https://github.com/benhoyt/inih.git "${INIH_REF}" inih
mkdir -p "${WORK}/inih-cmake"
cat > "${WORK}/inih-cmake/CMakeLists.txt" <<EOF
cmake_minimum_required(VERSION 3.16)
project(inih C CXX)
add_library(inih STATIC "${WORK}/inih/ini.c" "${WORK}/inih/cpp/INIReader.cpp")
target_include_directories(inih PUBLIC "${WORK}/inih" "${WORK}/inih/cpp")
install(TARGETS inih ARCHIVE DESTINATION lib)
install(FILES "${WORK}/inih/ini.h" "${WORK}/inih/cpp/INIReader.h" DESTINATION include)
EOF
cmake_lib inih "${WORK}/inih-cmake" -DINI_INITIAL_ALLOC=200

# liblcf: no ICU (only windows-1252 text), no XML (expat) and no tools.
log "liblcf ${LIBLCF_REF}"
fetch_git https://github.com/EasyRPG/liblcf.git "${LIBLCF_REF}" liblcf
cmake_lib liblcf "${WORK}/liblcf" \
	-DLIBLCF_WITH_ICU=OFF \
	-DLIBLCF_WITH_XML=OFF \
	-DLIBLCF_WITH_INI=ON \
	-DLIBLCF_ENABLE_TOOLS=OFF \
	-DLIBLCF_ENABLE_TESTS=OFF \
	-DLIBLCF_UPDATE_MIMEDB=OFF \
	-DLIBLCF_ENABLE_INSTALL=ON

echo "PSP dependencies installed into ${PREFIX}"
