#!/usr/bin/env bash
# Cross-compiles the libraries EasyRPG Player needs for the PlayStation Portable
# and installs them into ${PSPDEV}/psp.
#
# Usage: PSPDEV=/usr/local/pspdev builds/psp/build-deps.sh
#
# Needs: git, cmake, ninja, meson, curl, tar and the pspdev toolchain.

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
LIBLCF_REF="${LIBLCF_REF:-0.8.1}"

mkdir -p "${WORK}"

log() {
	echo "::group::$1" 2>/dev/null || echo "==> $1"
}

endgroup() {
	echo "::endgroup::" 2>/dev/null || true
}

fetch_git() { # <url> <ref> <name>
	if [[ ! -d "${WORK}/$3" ]]; then
		git clone --depth 1 --branch "$2" "$1" "${WORK}/$3"
	fi
}

cmake_lib() { # <name> <extra cmake args...>
	local name=$1
	shift
	cmake -S "${WORK}/${name}" -B "${WORK}/build-${name}" -G Ninja \
		-DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN}" \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_INSTALL_PREFIX="${PREFIX}" \
		-DBUILD_SHARED_LIBS=OFF \
		"$@"
	cmake --build "${WORK}/build-${name}" --parallel "${JOBS}"
	cmake --install "${WORK}/build-${name}"
}

# zlib
log "zlib ${ZLIB_REF}"
fetch_git https://github.com/madler/zlib.git "${ZLIB_REF}" zlib
cmake_lib zlib -DZLIB_BUILD_TESTING=OFF
endgroup

# libpng (needs zlib)
log "libpng ${LIBPNG_REF}"
fetch_git https://github.com/pnggroup/libpng.git "${LIBPNG_REF}" libpng
cmake_lib libpng \
	-DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF -DPNG_TOOLS=OFF
endgroup

# pixman (meson only). No threads, no SIMD: the Allegrex CPU has neither.
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
endgroup

# fmt
log "fmt ${FMT_REF}"
fetch_git https://github.com/fmtlib/fmt.git "${FMT_REF}" fmt
cmake_lib fmt -DFMT_TEST=OFF -DFMT_DOC=OFF
endgroup

# inih (liblcf needs it to read the Player's ini files)
log "inih ${INIH_REF}"
fetch_git https://github.com/benhoyt/inih.git "${INIH_REF}" inih
cmake_lib inih -DINIH_TESTS=OFF
endgroup

# liblcf: no ICU (only windows-1252 text), no XML (expat) and no tools.
log "liblcf ${LIBLCF_REF}"
fetch_git https://github.com/EasyRPG/liblcf.git "${LIBLCF_REF}" liblcf
cmake_lib liblcf \
	-DLIBLCF_WITH_ICU=OFF \
	-DLIBLCF_WITH_XML=OFF \
	-DLIBLCF_WITH_INI=ON \
	-DLIBLCF_ENABLE_TOOLS=OFF \
	-DLIBLCF_ENABLE_TESTS=OFF \
	-DLIBLCF_UPDATE_MIMEDB=OFF \
	-DLIBLCF_ENABLE_INSTALL=ON
endgroup

echo "PSP dependencies installed into ${PREFIX}"
