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
	 * is stored into a variable (default: V[3355).
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
	 * When a switch is set (default: S[1001) to ON, an alternative
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
		void ModifyMaxHp(Game_Enemy const& enemy, int& val);
		/** Scales an enemies's maximum SP stat, based on the value of variable V[1002] */
		void ModifyMaxSp(Game_Enemy const& enemy, int& val);
		/** Scales an enemies's attack stat, based on the value of variable V[1003] */
		void ModifyAtk(Game_Enemy const& enemy, int& val);
		/** Scales an enemies's defense stat, based on the value of variable V[1004] */
		void ModifyDef(Game_Enemy const& enemy, int& val);
		/** Scales an enemies's spirit stat, based on the value of variable V[1005] */
		void ModifySpi(Game_Enemy const& enemy, int& val);
		/** Scales an enemies's agility stat, based on the value of variable V[1006] */
		void ModifyAgi(Game_Enemy const& enemy, int& val);
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
}
#endif
