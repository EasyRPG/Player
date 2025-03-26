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
#include "span.h"

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

	template<size_t S>
	using patch_addresses = std::array<PatchArg, S> const;

	namespace BetterAEP {
		constexpr patch_addresses<1> addr_rm2k_107 = {{ { 0x096CA4, 3350 } }};
		constexpr patch_addresses<1> addr_rm2k3_1080 = {{ { 0x0C91A4, 3350 } }};
	}

	namespace DirectMenu {
		constexpr patch_addresses<1> addr_rm2k_107 = {{ { 0x0321B5, 3326 } }};
		constexpr patch_addresses<1> addr_rm2k_110 = {{ { 0x032145, 3326 } }};
		constexpr patch_addresses<1> addr_rm2k_150 = {{ { 0x032241, 3326 } }};
		constexpr patch_addresses<1> addr_rm2k_151 = {{ { 0x032285, 3326 } }};
		constexpr patch_addresses<1> addr_rm2k_162 = {{ { 0x0327E9, 3326 } }};
		constexpr patch_addresses<1> addr_rm2k3_1080 = {{ { 0x00462DE, 3326 } }};
		constexpr patch_addresses<1> addr_rm2k3_1090 = {{ { 0x00462DE, 3326 } }};
	}

	namespace MonSca {
		constexpr patch_addresses<11> addr_rm2k_107 = {{
			{ 0x7E947, 1001 }, // MaxHP modifier -> V[x]
			{ 0x7E952, 1002 }, // MaxSP modifier -> V[x]
			{ 0x7E95D, 1003 }, // ATk modifier -> V[x]
			{ 0x7E968, 1004 }, // Def modifier -> V[x]
			{ 0x7E973, 1005 }, // Spi modifier -> V[x]
			{ 0x7E97E, 1006 }, // Agi modifier -> V[x]
			{ 0x7E989, 1007 }, // EXP modifier -> V[x]
			{ 0x7E994, 1008 }, // Gold modifier -> V[x]
			{ 0x7E99F, 1009 }, // Item-ID modifier -> V[x]
			{ 0x7E9AC, 1010 }, // Item-Drop modifier -> V[x]
			{ 0x7ED2D, 1001 } // Use alternate formula  -> Sw[x]
		}};
		constexpr patch_addresses<11> addr_rm2k3_1080 = {{
			{ 0xBCB76, 1001 },
			{ 0xBCB81, 1002 },
			{ 0xBCB8C, 1003 },
			{ 0xBCB97, 1004 },
			{ 0xBCBA2, 1005 },
			{ 0xBCBAD, 1006 },
			{ 0xBCBB8, 1007 },
			{ 0xBCBC3, 1008 },
			{ 0xBCBCE, 1009 },
			{ 0xBCBDB, 1010 },
			{ 0xBD10B, 1001 }
		}};
	}

	namespace MonScaPlus {
		constexpr patch_addresses<11> addr_rm2k3_1080 = {{
			{ 0xBCB76, 1000 },
			{ 0xBCB81, 1008 },
			{ 0xBCB8C, 1016 },
			{ 0xBCB97, 1024 },
			{ 0xBCBA2, 1032 },
			{ 0xBCBAD, 1040 },
			{ 0xBCBB8, 1048 },
			{ 0xBCBC3, 1056 },
			{ 0xBCBCE, 1064 },
			{ 0xBCBDB, 1072 },
			{ 0xBD117, 1001 }
		}};
	}

	namespace EXPlus {
		// EXP modifier for Party Actors  -> V[x] - V[x+3]
		constexpr patch_addresses<1> addr_rm2k_107 = {{ { 0x073B92, 3333 } }};
		constexpr patch_addresses<1> addr_rm2k3_1080 = {{ { 0x0990D2, 3333 } }};
	}

	namespace EXPlusPlus {
		// Party position of last actor checked via "IsActorInParty"  -> V[x]
		constexpr patch_addresses<1> addr_rm2k_107 = {{ { 0x088974, 3332 } }};
		constexpr patch_addresses<1> addr_rm2k3_1080 = {{ { 0x0AFAB8, 3332 } }};
	}

	constexpr patch_detection_map<11> patches_RM2K_107 = { {
		{ KnownPatches::UnlockPics,        { 0x082B4D, patch_segment(0x90, 0x90, 0x90, 0x90, 0x90) } },
		{ KnownPatches::CommonThisEvent,   { 0x084F4C, patch_segment(0xE9, 0xAF, 0xBC, 0xFA, 0xFF) } },
		{ KnownPatches::BreakLoopFix,      { 0x08A62A, patch_segment(0xE8, 0x85, 0xE8, 0xFF, 0xFF) } },
		{ KnownPatches::AutoEnterPatch,    { 0x06D420, patch_segment(0x75, 0x75) } },
		{ KnownPatches::BetterAEP,         { 0x06D75F, patch_segment(0xEB, 0x07), BetterAEP::addr_rm2k_107 } },
		{ KnownPatches::PicPointer,        { 0x087BE6, patch_segment(0xE8, 0xE9, 0xBF, 0xF9, 0xFF) } },
		{ KnownPatches::PicPointer_R,      { 0x087BE6, patch_segment(0xE8, 0x51, 0xC0, 0xF9, 0xFF) } },
		{ KnownPatches::DirectMenu,        { 0x078FC8, patch_segment(0xE9, 0x13, 0x92, 0xFB, 0xFF), DirectMenu::addr_rm2k_107 } },
		{ KnownPatches::MonSca,            { 0x07E880, patch_segment(0x50, 0x8B, 0x40, 0x14, 0x89), MonSca::addr_rm2k_107 } },
		{ KnownPatches::EXPlus,            { 0x07394D, patch_segment(0x50, 0xEB, 0x09), EXPlus::addr_rm2k_107 } },
		{ KnownPatches::EXPlusPlus,        { 0x088BD5, patch_segment(0xE9, 0x91, 0xFD), EXPlusPlus::addr_rm2k_107 } }
	}};

	constexpr patch_detection_map<4> patches_RM2K_110 = {{
		{ KnownPatches::CommonThisEvent,   { 0x084E5C, patch_segment(0xE9, 0x33, 0xBD, 0xFA, 0xFF) } },
		{ KnownPatches::BreakLoopFix,      { 0x08A73A, patch_segment(0xE8, 0x85, 0xE8, 0xFF, 0xFF) } },
		{ KnownPatches::PicPointer_R,      { 0x023B28, patch_segment(0xE8, 0xD5, 0xC0, 0xF9, 0xFF) } },
		{ KnownPatches::DirectMenu,        { 0x078ED8, patch_segment(0xE9, 0x93, 0x92, 0xFB, 0xFF), DirectMenu::addr_rm2k_110 } }
	}};
	
	constexpr patch_detection_map<3> patches_RM2K_150 = {{
		{ KnownPatches::CommonThisEvent,   { 0x089EAC, patch_segment(0xE9, 0xDF, 0x6D, 0xFA, 0xFF) } },
		{ KnownPatches::BreakLoopFix,      { 0x08F72A, patch_segment(0xE8, 0x8D, 0xE8, 0xFF, 0xFF) } },
		{ KnownPatches::DirectMenu,        { 0x07DE30, patch_segment(0xE9, 0x37, 0x44, 0xFB, 0xFF), DirectMenu::addr_rm2k_150 } }
	}};

	constexpr patch_detection_map<4> patches_RM2K_151 = {{
		{ KnownPatches::CommonThisEvent,   { 0x089FC8, patch_segment(0xE9, 0x07, 0x6D, 0xFA, 0xFF) } },
		{ KnownPatches::BreakLoopFix,      { 0x08F846, patch_segment(0xE8, 0x8D, 0xE8, 0xFF, 0xFF) } },
		{ KnownPatches::PicPointer_R,      { 0x08CD82, patch_segment(0xE8, 0x89, 0x6F, 0xF9, 0xFF) } },
		{ KnownPatches::DirectMenu,        { 0x07DF4C, patch_segment(0xE9, 0x5F, 0x43, 0xFB, 0xFF), DirectMenu::addr_rm2k_151 } }
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
		{ KnownPatches::DirectMenu,        { 0x07F848, patch_segment(0xE9, 0xC7, 0x2F, 0xFB, 0xFF), DirectMenu::addr_rm2k_162 } }
	}};
	
	constexpr patch_detection_map<12> patches_RM2K3_1080 = {{
		{ KnownPatches::UnlockPics,        { 0x0B12FA, patch_segment(0x90, 0x90, 0x90, 0x90, 0x90) } },
		{ KnownPatches::CommonThisEvent,   { 0x0AB670, patch_segment(0xE9, 0x8B, 0x76, 0xF9, 0xFF) } },
		{ KnownPatches::BreakLoopFix,      { 0x0B1E4B, patch_segment(0xE8, 0x04, 0xE2, 0xFF, 0xFF) } },
		{ KnownPatches::AutoEnterPatch,    { 0x08FAC0, patch_segment(0x75, 0x3D) } },
		{ KnownPatches::BetterAEP,         { 0x08FDA7, patch_segment(0xEB, 0x07), BetterAEP::addr_rm2k3_1080 } },
		{ KnownPatches::PicPointer,        { 0x0AEB1E, patch_segment(0xE8, 0x01, 0x5F, 0xF8, 0xFF) } },
		{ KnownPatches::PicPointer_R,      { 0x0AEB1E, patch_segment(0xE8, 0x65, 0x5F, 0xF8, 0xFF) } },
		{ KnownPatches::DirectMenu,        { 0x0A0422, patch_segment(0xE9, 0xE2, 0x5E, 0xFA, 0xFF), DirectMenu::addr_rm2k3_1080 } },
		{ KnownPatches::MonSca,            { 0x0BCA31, patch_segment(0x50, 0x8B, 0x40, 0x14, 0x89), MonSca::addr_rm2k3_1080 } },
		{ KnownPatches::MonScaPlus,        { 0x0BD109, patch_segment(0x5A, 0x0F, 0xAF, 0xC2, 0x99), MonScaPlus::addr_rm2k3_1080 } },
		{ KnownPatches::EXPlus,            { 0x098E8D, patch_segment(0x50, 0xEB, 0x09), EXPlus::addr_rm2k3_1080 } },
		{ KnownPatches::EXPlusPlus,        { 0x0AFD19, patch_segment(0xE9, 0x91, 0xFD), EXPlusPlus::addr_rm2k3_1080 } }
	}};

	constexpr patch_detection_map<4> patches_RM2K3_1091 = {{
		{ KnownPatches::CommonThisEvent,   { 0x0B4008, patch_segment(0xE9, 0xF3, 0xEC, 0xF8, 0xFF) } },
		{ KnownPatches::BreakLoopFix,      { 0x0BA7E3, patch_segment(0xE8, 0x04, 0xE2, 0xFF, 0xFF) } },
		{ KnownPatches::PicPointer_R,      { 0x0B74B6, patch_segment(0xE8, 0xCD, 0xD5, 0xF7, 0xFF) } },
		{ KnownPatches::DirectMenu,        { 0x09F756, patch_segment(0xE9, 0xAE, 0x6B, 0xFA, 0xFF), DirectMenu::addr_rm2k3_1090 } }
	}};

	inline Span<patch_detection const> GetPatchesForBuildVersion(EXE::BuildInfo::KnownEngineBuildVersions build_version) {
		switch (build_version) {
			case EXE::BuildInfo::RM2K_20001227:
				return EXE::Patches::patches_RM2K_107;
			case EXE::BuildInfo::RM2K_20010505:
				return EXE::Patches::patches_RM2K_110;
			case EXE::BuildInfo::RM2K_20030327:
				return EXE::Patches::patches_RM2K_150;
			case EXE::BuildInfo::RM2K_20030625:
				return EXE::Patches::patches_RM2K_151;
			case EXE::BuildInfo::RM2KE_160:
				return EXE::Patches::patches_RM2K_160;
			case EXE::BuildInfo::RM2KE_161:
				return EXE::Patches::patches_RM2K_161;
			case EXE::BuildInfo::RM2KE_162:
				return EXE::Patches::patches_RM2K_162;
			case EXE::BuildInfo::RM2K3_1080_1080:
				return EXE::Patches::patches_RM2K3_1080;
			case EXE::BuildInfo::RM2K3_1091_1091:
				return EXE::Patches::patches_RM2K3_1091;
			default:
				break;
		}
		return Span<patch_detection const>();
	}
}

#endif
