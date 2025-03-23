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

#ifndef EP_GAME_RUNTIMEPATCHES_H
#define EP_GAME_RUNTIMEPATCHES_H

#include "game_config_game.h"
#include "cmdline_parser.h"
#include "player.h"
#include "input.h"

class Game_Actor;
class Game_Battler;
class Game_Enemy;

// When this compile flag is set, all of the evaluation logic for these patches
// will be disabled, by simply voiding any calls to their function hooks.
//#define NO_RUNTIME_PATCHES

namespace RuntimePatches {
	struct PatchArg {
		ConfigParam<int>* config_param = nullptr;
		char const* cmd_arg;
		int const default_value = 0;

		constexpr PatchArg(ConfigParam<int>& config_param, char const* cmd_arg, int const default_value)
			: config_param(&config_param), cmd_arg(cmd_arg), default_value(default_value) {
		}
	};

	void LockPatchesAsDiabled();

	bool ParseFromCommandLine(CmdlineParser& cp);

	bool ParseFromIni(lcf::INIReader& ini);

	void DetermineActivePatches(std::vector<std::string>& patches);

	/**
	 * Support for RPG_RT patch 'Encounter Randomness Alert'.
	 * This patch skips the normal battle startup logic whenever a random
	 * encounter would be triggered.
	 * Instead a switch (default: S[1018]) is set to ON and the troop ID
	 * is stored into a variable (default: V[3355]).
	 *
	 * This implementation always triggers a page-refresh for all
	 * events on the current map.
	 */
	namespace EncounterRandomnessAlert {
		constexpr std::array<PatchArg, 2> patch_args = { {
			{ Player::game_config.patch_encounter_random_alert_sw, "-sw", 1018},
			{ Player::game_config.patch_encounter_random_alert_var, "-var", 3355 }
		} };

		/**
		 * Sets the configured switch & variable according to ERA's rules.
		 * @return if normal battle processing should be skipped.
		 */
		bool HandleEncounter(int troop_id);
	}

	/**
	 * Support for RPG_RT patch 'MonSca' & 'MonScaPlus'.
	 * This patch scales the default battle parameters of an enemy
	 * based on the contents of some in-game variables.
	 * (Default: V[1001] - V[1010])
	 * 
	 * When a switch is set (default: S[1001]) to ON, an alternative
	 * scaling formula, based on the average party level, is used.
	 *
	 * Default formula:     val = val * V[...] / 1000
	 * Alternative formula: val = val * avg_level * V[...] / 1000
	 *
	 * Variant 'MonScaPlus':
	 * If set, the variable IDs used for scaling will be offset
	 * by the enemy's troop index.
	 *  -> V[base_var_id + troop_index]
	 */
	namespace MonSca {
		constexpr std::array<PatchArg, 12> patch_args = { {
			{ Player::game_config.patch_monsca_maxhp, "-maxhp", 1001 },
			{ Player::game_config.patch_monsca_maxsp, "-maxsp", 1002 },
			{ Player::game_config.patch_monsca_atk, "-atk", 1003 },
			{ Player::game_config.patch_monsca_def, "-def", 1004 },
			{ Player::game_config.patch_monsca_spi, "-spi", 1005 },
			{ Player::game_config.patch_monsca_agi, "-agi", 1006 },
			{ Player::game_config.patch_monsca_exp, "-exp", 1007 },
			{ Player::game_config.patch_monsca_gold, "-gold", 1008 },
			{ Player::game_config.patch_monsca_item, "-item", 1009 },
			{ Player::game_config.patch_monsca_droprate, "-droprate", 1010 },
			{ Player::game_config.patch_monsca_levelscaling, "-lvlscale", 1001 },
			{ Player::game_config.patch_monsca_plus, "-plus", 0 }
		} };

		/** Scales an enemies's maximum HP stat, based on the value of variable V[1001] */
		void ModifyMaxHp(Game_Enemy const& enemy, int32_t& val);
		/** Scales an enemies's maximum SP stat, based on the value of variable V[1002] */
		void ModifyMaxSp(Game_Enemy const& enemy, int32_t& val);
		/** Scales an enemies's attack stat, based on the value of variable V[1003] */
		void ModifyAtk(Game_Enemy const& enemy, int32_t& val);
		/** Scales an enemies's defense stat, based on the value of variable V[1004] */
		void ModifyDef(Game_Enemy const& enemy, int32_t& val);
		/** Scales an enemies's spirit stat, based on the value of variable V[1005] */
		void ModifySpi(Game_Enemy const& enemy, int32_t& val);
		/** Scales an enemies's agility stat, based on the value of variable V[1006] */
		void ModifyAgi(Game_Enemy const& enemy, int32_t& val);
		/** Scales the experience points gained by defating an enemy, based on the value of variable V[1007] */
		void ModifyExpGained(Game_Enemy const& enemy, int& val);
		/** Scales the money gained by defating an enemy, based on the value of variable V[1008] */
		void ModifyMoneyGained(Game_Enemy const& enemy, int& val);
		/**
		 * Modifies the item dropped by defating an enemy, based on the value of variable V[1009]
		 * In contrast to other modifers of this patch, this skips the normal formula and just
		 * adds the variable value to the result.
		 */
		void ModifyItemGained(Game_Enemy const& enemy, int& item_id);
		/** Scales the item drop rate of an enemy, based on the value of variable V[1010] */
		void ModifyItemDropRate(Game_Enemy const& enemy, int& val);
	}

	/**
	 * Support for RPG_RT patch 'EXPlus' & 'EXPLus[+]'.
	 * This patch allows to individually boost the 4 party members'
	 * gained experience inside battles by applying an extra percentage
	 * based on the values of in-game variables.
	 *  (default: V[3333] for party member #1; the amounts for other
	 *   party members are read from the subsequent 3 variables)
	 *
	 * If the '[+]' option is enabled, a side effect is added to one
	 * of the Actor clauses of 'CommandConditionalBranch':
	 *  Whenever this command is used to check for the existence of
	 *  an actor in the current party, the current party slot (1-4)
	 *  of this actor is set to an in-game variable. (default: V[3332])
	 */
	namespace EXPlus {
		constexpr std::array<PatchArg, 2> patch_args = { {
			{ Player::game_config.patch_explus_var, "-var", 3333},
			{ Player::game_config.patch_explusplus_var, "-var", 3332 }
		} };

		/**
		 * Boosts the gained experience points which would be added to
		 * the given actor's stats by an extra amount which is calculated
		 * from the value of the in-game variable V[X + party_index - 1].
		 */
		void ModifyExpGain(Game_Actor& actor, int& exp_gain);

		/**
		 * Store's the current party position of the given actor inside
		 * the configured in-game variable for the '[+]' variant of
		 * te EXPlus patch.
		 */
		void StoreActorPosition(int actor_id);
	}

	/**
	 * Support for RPG_RT patch 'GuardRevamp'.
	 * This patch changes the way the damage adjustment is calculated
	 * whenever the target of an attack is defending.
	 *
	 * Normally this calculation is done by simply dividing the damage
	 * in half for normal defense situations, and by quartering it
	 * when the target has the 'strong defense' attribute.
	 * With 'GuardRevamp' enabled, this is changed to a percentage
	 * calculation, allowing for more granular control over the output.
	 * The given default values of '50%', and '25%' would provide
	 * the same results in most situations.
	 */
	namespace GuardRevamp {
		constexpr std::array<PatchArg, 2> patch_args = { {
			{ Player::game_config.patch_guardrevamp_normal, "-normal", 50},
			{ Player::game_config.patch_guardrevamp_strong, "-strong", 25 }
		} };

		/**
		 * Adjusts the damage value taken by the given battler according
		 * the GuardRevamp patches' rules.
		 * @return if normal damage calculation should skipped.
		 */
		bool OverrideDamageAdjustment(int& dmg, const Game_Battler& target);
	}

	namespace VirtualKeys {
		constexpr Input::Keys::InputKey VirtualKeyToInputKey(uint32_t key_id);
	}
}

constexpr Input::Keys::InputKey RuntimePatches::VirtualKeys::VirtualKeyToInputKey(uint32_t key_id) {
	// see https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
	switch (key_id) {
#if defined(USE_MOUSE) && defined(SUPPORT_MOUSE)
		case 0x1: return Input::Keys::MOUSE_LEFT;
		case 0x2: return Input::Keys::MOUSE_RIGHT;
		case 0x4: return Input::Keys::MOUSE_MIDDLE;
		case 0x5: return Input::Keys::MOUSE_XBUTTON1;
		case 0x6: return Input::Keys::MOUSE_XBUTTON2;
#endif
		case 0x8: return Input::Keys::BACKSPACE;
		case 0x9: return Input::Keys::TAB;
		case 0xD: return Input::Keys::RETURN;
		case 0x10: return Input::Keys::SHIFT;
		case 0x11: return Input::Keys::CTRL;
		case 0x12: return Input::Keys::ALT;
		case 0x13: return Input::Keys::PAUSE;
		case 0x14: return Input::Keys::CAPS_LOCK;
		case 0x1B: return Input::Keys::ESCAPE;
		case 0x20: return Input::Keys::SPACE;
		case 0x21: return Input::Keys::PGUP;
		case 0x22: return Input::Keys::PGDN;
		case 0x23: return Input::Keys::ENDS;
		case 0x24: return Input::Keys::HOME;
		case 0x25: return Input::Keys::LEFT;
		case 0x26: return Input::Keys::UP;
		case 0x27: return Input::Keys::RIGHT;
		case 0x28: return Input::Keys::DOWN;
		case 0x2D: return Input::Keys::INSERT;
		case 0x2E: return Input::Keys::DEL;
		case 0x30: return Input::Keys::N0;
		case 0x31: return Input::Keys::N1;
		case 0x32: return Input::Keys::N2;
		case 0x33: return Input::Keys::N3;
		case 0x34: return Input::Keys::N4;
		case 0x35: return Input::Keys::N5;
		case 0x36: return Input::Keys::N6;
		case 0x37: return Input::Keys::N7;
		case 0x38: return Input::Keys::N8;
		case 0x39: return Input::Keys::N9;
		case 0x41: return Input::Keys::A;
		case 0x42: return Input::Keys::B;
		case 0x43: return Input::Keys::C;
		case 0x44: return Input::Keys::D;
		case 0x45: return Input::Keys::E;
		case 0x46: return Input::Keys::F;
		case 0x47: return Input::Keys::G;
		case 0x48: return Input::Keys::H;
		case 0x49: return Input::Keys::I;
		case 0x4A: return Input::Keys::J;
		case 0x4B: return Input::Keys::K;
		case 0x4C: return Input::Keys::L;
		case 0x4D: return Input::Keys::M;
		case 0x4E: return Input::Keys::N;
		case 0x4F: return Input::Keys::O;
		case 0x50: return Input::Keys::P;
		case 0x51: return Input::Keys::Q;
		case 0x52: return Input::Keys::R;
		case 0x53: return Input::Keys::S;
		case 0x54: return Input::Keys::T;
		case 0x55: return Input::Keys::U;
		case 0x56: return Input::Keys::V;
		case 0x57: return Input::Keys::W;
		case 0x58: return Input::Keys::X;
		case 0x59: return Input::Keys::Y;
		case 0x5A: return Input::Keys::Z;
		case 0x60: return Input::Keys::KP0;
		case 0x61: return Input::Keys::KP1;
		case 0x62: return Input::Keys::KP2;
		case 0x63: return Input::Keys::KP3;
		case 0x64: return Input::Keys::KP4;
		case 0x65: return Input::Keys::KP5;
		case 0x66: return Input::Keys::KP6;
		case 0x67: return Input::Keys::KP7;
		case 0x68: return Input::Keys::KP8;
		case 0x69: return Input::Keys::KP9;
		case 0x6A: return Input::Keys::KP_MULTIPLY;
		case 0x6B: return Input::Keys::KP_ADD;
		case 0x6D: return Input::Keys::KP_SUBTRACT;
		case 0x6E: return Input::Keys::KP_PERIOD;
		case 0x6F: return Input::Keys::KP_DIVIDE;
		case 0x70: return Input::Keys::F1;
		case 0x71: return Input::Keys::F2;
		case 0x72: return Input::Keys::F3;
		case 0x73: return Input::Keys::F4;
		case 0x74: return Input::Keys::F5;
		case 0x75: return Input::Keys::F6;
		case 0x76: return Input::Keys::F7;
		case 0x77: return Input::Keys::F8;
		case 0x78: return Input::Keys::F9;
		case 0x79: return Input::Keys::F10;
		case 0x7A: return Input::Keys::F11;
		case 0x7B: return Input::Keys::F12;
		case 0x90: return Input::Keys::NUM_LOCK;
		case 0x91: return Input::Keys::SCROLL_LOCK;
		case 0xA0: return Input::Keys::LSHIFT;
		case 0xA1: return Input::Keys::RSHIFT;
		case 0xA2: return Input::Keys::LCTRL;
		case 0xA3: return Input::Keys::RCTRL;

		default: return Input::Keys::NONE;
	}
}

#endif
