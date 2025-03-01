/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EP_GAME_DYNRPG_LOADER_H
#define EP_GAME_DYNRPG_LOADER_H

#include <vector>
#include <map>
#include "exe_buildinfo.h"
#include "exe_shared.h"
#include "filefinder.h"

#define DYNRPG_INI_NAME "DynRPG.ini"
#define DYNRPG_INI_SECTION_QUICKPATCHES "QuickPatches"

#define DYNRPG_FOLDER_PATCHES "DynPatches"
#define DYNRPG_FOLDER_PLUGINS "DynPlugins"

namespace DynRpg_Loader {
	constexpr std::array<uint8_t, 5> magic_ips = { 0x50, 0x41, 0x54, 0x43, 0x48 }; // "PATCH"
	constexpr EXE::Shared::PatchSetupInfo invalid_patch = { static_cast<EXE::Shared::KnownPatches>(-1), 0 };

	std::vector<EXE::Shared::PatchSetupInfo> DetectRuntimePatches(EXE::BuildInfo::KnownEngineBuildVersions build_version = EXE::BuildInfo::KnownEngineBuildVersions::RM2K3_1080_1080);

	EXE::Shared::PatchSetupInfo ReadIPS(std::string const& item_name, Filesystem_Stream::InputStream& is);

	void ApplyQuickPatches(EXE::Shared::EngineCustomization& engine_customization, EXE::BuildInfo::KnownEngineBuildVersions build_version = EXE::BuildInfo::KnownEngineBuildVersions::RM2K3_1080_1080);

	struct QuickPatchAddress {
		uint32_t address = 0;
		uint8_t byte_count = 0;
		long patch_val = 0;
	};

	using DynRpgQuickPatch = std::vector<QuickPatchAddress>;

	DynRpgQuickPatch ParseQuickPatch(std::string const& line, int code_size);
}

#endif
