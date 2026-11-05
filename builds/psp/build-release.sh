#!/usr/bin/env bash
# Builds EasyRPG Player for the PlayStation Portable and packages the release
# zip (EBOOT.PBP + README). Meant to run inside the pspdev container, see
# .github/workflows/psp.yml.
#
# Usage: PSPDEV=/usr/local/pspdev builds/psp/build-release.sh
#
# Environment:
#   PSP_DEPS_BUILD   set to 0 to skip building the dependencies (default: 1)
#   PSP_RELEASE_TAG  tag name used in the zip name; empty for dev builds
#   PSP_GIT_SHA      commit id appended to the zip name of dev builds
#   PSP_CMAKE_ARGS   extra CMake arguments, e.g. -DPLAYER_PSP_AUDIO_STATS=ON

set -euo pipefail

export PSPDEV="${PSPDEV:-/usr/local/pspdev}"

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
TOOLCHAIN="${ROOT}/builds/psp/PspToolchain.cmake"
BUILD_DIR="${ROOT}/build-psp"
DIST_DIR="${ROOT}/dist"
JOBS="${JOBS:-$(nproc)}"

export PATH="${PSPDEV}/bin:${PATH}"

# Git inside the container does not own the mounted checkout
git config --global --add safe.directory '*' || true

if [[ "${PSP_DEPS_BUILD:-1}" == "1" ]]; then
	"${ROOT}/builds/psp/build-deps.sh"
fi

GENERATOR=()
if command -v ninja >/dev/null 2>&1; then
	GENERATOR=(-G Ninja)
fi

cmake -S "${ROOT}" -B "${BUILD_DIR}" "${GENERATOR[@]}" \
	-DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN}" \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_INSTALL_PREFIX="${PSPDEV}/psp" \
	-DBUILD_SHARED_LIBS=OFF \
	-DPLAYER_BUILD_LIBLCF=OFF \
	-DPLAYER_WITH_FREETYPE=OFF \
	-DPLAYER_WITH_HARFBUZZ=OFF \
	-DPLAYER_WITH_LHASA=OFF \
	-DPLAYER_WITH_NLOHMANN_JSON=OFF \
	-DPLAYER_WITH_MPG123=OFF \
	-DPLAYER_WITH_LIBSNDFILE=OFF \
	-DPLAYER_WITH_OGGVORBIS=OFF \
	-DPLAYER_WITH_OPUS=OFF \
	-DPLAYER_WITH_WILDMIDI=OFF \
	-DPLAYER_WITH_FLUIDSYNTH=OFF \
	-DPLAYER_WITH_XMP=OFF \
	-DPLAYER_WITH_BAEKMUK=OFF \
	-DPLAYER_WITH_WQY=OFF \
	-DPLAYER_VERSIONED_PACKAGES=OFF \
	${PSP_CMAKE_ARGS:-}

cmake --build "${BUILD_DIR}" --parallel "${JOBS}"

EBOOT="${BUILD_DIR}/psp/EBOOT.PBP"
if [[ ! -f "${EBOOT}" ]]; then
	echo "EBOOT.PBP was not produced" >&2
	exit 1
fi

VERSION="$(sed -n 's/^project(EasyRPG_Player VERSION \([0-9.]*\).*/\1/p' "${ROOT}/CMakeLists.txt" | head -n1)"
NAME="EasyRPG-Player-${VERSION}-psp"
if [[ -n "${PSP_RELEASE_TAG:-}" ]]; then
	NAME="EasyRPG-Player-${PSP_RELEASE_TAG}-psp"
elif [[ -n "${PSP_GIT_SHA:-}" ]]; then
	NAME="${NAME}-${PSP_GIT_SHA:0:7}"
fi

STAGE="${BUILD_DIR}/package/EasyRPG"
rm -rf "${BUILD_DIR}/package"
mkdir -p "${STAGE}" "${DIST_DIR}"
cp "${EBOOT}" "${STAGE}/EBOOT.PBP"
cp "${ROOT}/COPYING" "${STAGE}/COPYING.txt"

cat > "${STAGE}/README.txt" <<EOF
EasyRPG Player ${VERSION} for PlayStation Portable
==================================================

Installation
------------
1. Copy the "EasyRPG" folder to ms0:/PSP/GAME/ on the memory stick.
2. Copy your RPG Maker 2000/2003 game (the folder containing RPG_RT.ldb or
   RPG_RT.lmt, plus its Map/Music/Picture/... folders) into
   ms0:/PSP/GAME/EasyRPG/ next to EBOOT.PBP.
3. Start "EasyRPG Player" from the Game menu of the XMB.

The game is always started from the folder that contains EBOOT.PBP.

Controls
--------
  D-Pad / Left analog stick  move and select
  Cross        cancel
  Circle       decision
  Square       fast forward (hold)
  Triangle     shift
  L / R        N1 / N2 (game keys)
  Start        settings menu
  Select       back

Limitations of this build
-------------------------
* Only windows-1252 text is supported (no Japanese / Shift-JIS encoding).
* Video is software rendered at 1:1 scale, centered on the 480x272 screen.
* Game data must fit in the PSP's memory; large games may run out of memory.

Source code and bug reports: https://github.com/EasyRPG/Player
EOF

(
	cd "${BUILD_DIR}/package"
	rm -f "${DIST_DIR}/${NAME}.zip"
	if command -v zip >/dev/null 2>&1; then
		zip -r -9 "${DIST_DIR}/${NAME}.zip" EasyRPG
	else
		echo "zip is required to create the release archive" >&2
		exit 1
	fi
)

echo "Release package: ${DIST_DIR}/${NAME}.zip"
ls -l "${DIST_DIR}/${NAME}.zip"
