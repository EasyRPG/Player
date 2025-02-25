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

#ifndef EP_EXE_PATCHES_H
#define EP_EXE_PATCHES_H

#include "exe_buildinfo.h"
#include "exe_shared.h"

namespace EXE::Patches {
	using namespace BuildInfo;
	using KnownPatches = EXE::Shared::KnownPatches;

	template<typename... Args>
	constexpr patch_segment_data patch_segment(Args... args) {
		return patch_segment_data{ static_cast<const uint8_t>(args)... };
	}

	using patch_detection = std::pair<KnownPatches, PatchDetectionInfo>;
	template<size_t S>
	using patch_detection_map = std::array<patch_detection, S>;

	constexpr patch_detection_map<8> patches_RM2K_107 = {{
		{ KnownPatches::UnlockPics,        { 0x082B4D, patch_segment(0x90, 0x90, 0x90, 0x90, 0x90) } },
		{ KnownPatches::CommonThisEvent,   { 0x084F4C, patch_segment(0xE9, 0xAF, 0xBC, 0xFA, 0xFF) } },
		{ KnownPatches::BreakLoopFix,      { 0x08A62A, patch_segment(0xE8, 0x85, 0xE8, 0xFF, 0xFF) } },
		{ KnownPatches::AutoEnterPatch,    { 0x06D420, patch_segment(0x75, 0x75) } },
		{ KnownPatches::BetterAEP,         { 0x06D75F, patch_segment(0xEB, 0x07), 0x096CA4 } },
		{ KnownPatches::PicPointer,        { 0x087BE6, patch_segment(0xE8, 0xE9, 0xBF, 0xF9, 0xFF) } },
		{ KnownPatches::PicPointer_R,      { 0x087BE6, patch_segment(0xE8, 0x51, 0xC0, 0xF9, 0xFF) } },
		{ KnownPatches::DirectMenu,        { 0x078FC8, patch_segment(0xE9, 0x13, 0x92, 0xFB, 0xFF), 0x0321B5 } }
	}};

	constexpr patch_detection_map<4> patches_RM2K_110 = {{
		{ KnownPatches::CommonThisEvent,   { 0x084E5C, patch_segment(0xE9, 0x33, 0xBD, 0xFA, 0xFF) } },
		{ KnownPatches::BreakLoopFix,      { 0x08A73A, patch_segment(0xE8, 0x85, 0xE8, 0xFF, 0xFF) } },
		{ KnownPatches::PicPointer_R,      { 0x023B28, patch_segment(0xE8, 0xD5, 0xC0, 0xF9, 0xFF) } },
		{ KnownPatches::DirectMenu,        { 0x078ED8, patch_segment(0xE9, 0x93, 0x92, 0xFB, 0xFF), 0x032145 } }
	}};
	
	constexpr patch_detection_map<3> patches_RM2K_150 = {{
		{ KnownPatches::CommonThisEvent,   { 0x089EAC, patch_segment(0xE9, 0xDF, 0x6D, 0xFA, 0xFF) } },
		{ KnownPatches::BreakLoopFix,      { 0x08F72A, patch_segment(0xE8, 0x8D, 0xE8, 0xFF, 0xFF) } },
		{ KnownPatches::DirectMenu,        { 0x07DE30, patch_segment(0xE9, 0x37, 0x44, 0xFB, 0xFF), 0x032241 } }
	}};

	constexpr patch_detection_map<4> patches_RM2K_151 = {{
		{ KnownPatches::CommonThisEvent,   { 0x089FC8, patch_segment(0xE9, 0x07, 0x6D, 0xFA, 0xFF) } },
		{ KnownPatches::BreakLoopFix,      { 0x08F846, patch_segment(0xE88DE8FFFF) } },
		{ KnownPatches::PicPointer_R,      { 0x08CD82, patch_segment(0xE8, 0x89, 0x6F, 0xF9, 0xFF) } },
		{ KnownPatches::DirectMenu,        { 0x07DF4C, patch_segment(0xE9, 0x5F, 0x43, 0xFB, 0xFF), 0x032285 } }
	}};

	constexpr patch_detection_map<2> patches_RM2K_160 = {{
		{ KnownPatches::CommonThisEvent,   { 0x088838, patch_segment(0xE9, 0xEB, 0x85, 0xFA, 0xFF) } },
		{ KnownPatches::BreakLoopFix,      { 0x08E098, patch_segment(0xE8, 0xAF, 0xE8, 0xFF, 0xFF) } }
	}};

	constexpr patch_detection_map<2> patches_RM2K_161 = {{
		{ KnownPatches::CommonThisEvent,   { 0x089208, patch_segment(0xE9, 0x2B, 0x80, 0xFA, 0xFF) } },
		{ KnownPatches::BreakLoopFix,      { 0x08EAA4, patch_segment(0xE8, 0xAF, 0xE8, 0xFE, 0xFF) } }
	}};
	
	constexpr patch_detection_map<4> patches_RM2K_162 = {{
		{ KnownPatches::CommonThisEvent,   { 0x08B8D0, patch_segment(0xE9, 0x63, 0x59, 0xFA, 0xFF) } },
		{ KnownPatches::BreakLoopFix,      { 0x09116C, patch_segment(0xE8, 0xAF, 0xE8, 0xFF, 0xFF) } },
		{ KnownPatches::PicPointer_R,      { 0x08E6CA, patch_segment(0xE8, 0xA5, 0x5B, 0xF9, 0xFF) } },
		{ KnownPatches::DirectMenu,        { 0x07F848, patch_segment(0xE9, 0xC7, 0x2F, 0xFB, 0xFF), 0x0327E9 } }
	}};
	
	constexpr patch_detection_map<8> patches_RM2K3_1080 = {{
		{ KnownPatches::UnlockPics,        { 0x0B12FA, patch_segment(0x90, 0x90, 0x90, 0x90, 0x90) } },
		{ KnownPatches::CommonThisEvent,   { 0x0AB670, patch_segment(0xE9, 0x8B, 0x76, 0xF9, 0xFF) } },
		{ KnownPatches::BreakLoopFix,      { 0x0B1E4B, patch_segment(0xE8, 0x04, 0xE2, 0xFF, 0xFF) } },
		{ KnownPatches::AutoEnterPatch,    { 0x08FAC0, patch_segment(0x75, 0x3D) } },
		{ KnownPatches::BetterAEP,         { 0x08FDA7, patch_segment(0xEB, 0x07), 0x0C91A4 } },
		{ KnownPatches::PicPointer,        { 0x0AEB1E, patch_segment(0xE8, 0x01, 0x5F, 0xF8, 0xFF) } },
		{ KnownPatches::PicPointer_R,      { 0x0AEB1E, patch_segment(0xE8, 0x65, 0x5F, 0xF8, 0xFF) } },
		{ KnownPatches::DirectMenu,        { 0x0A0422, patch_segment(0xE9, 0xE2, 0x5E, 0xFA, 0xFF), 0x00462DE } }
	}};

	constexpr patch_detection_map<4> patches_RM2K3_1091 = {{
		{ KnownPatches::CommonThisEvent,   { 0x0B4008, patch_segment(0xE9, 0xF3, 0xEC, 0xF8, 0xFF) } },
		{ KnownPatches::BreakLoopFix,      { 0x0BA7E3, patch_segment(0xE8, 0x04, 0xE2, 0xFF, 0xFF) } },
		{ KnownPatches::PicPointer_R,      { 0x0B74B6, patch_segment(0xE8, 0xCD, 0xD5, 0xF7, 0xFF) } },
		{ KnownPatches::DirectMenu,        { 0x09F756, patch_segment(0xE9, 0xAE, 0x6B, 0xFA, 0xFF), 0x00462DE } }
	}};
}

#endif
