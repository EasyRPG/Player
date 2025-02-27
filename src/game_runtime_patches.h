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

class Game_Actor;
namespace RuntimePatches {
	/**
	 * Support for RPG_RT patch 'Encounter Randomness Alert'.
	 * This patch skips the normal battle startup logic whenever a random
	 * encounter would be triggered and.
	 * Instead a switch (default: S[1018]) is set to ON and the troop ID
	 * is stored into a variable (default: V[3355).
	 *
	 * This implementation always triggers a page-refresh for all
	 * events on the current map.
	 */
	namespace EncounterRandomnessAlert {
		/**
		 * Sets the configured switch & variable according to ERA큦 rules.
		 * @return if normal battle processing should be skipped.
		 */
		bool HandleEncounter(int troop_id);
	}

	/**
	 * Support for RPG_RT patch 'MonSca'.
	 * This patch scales the default battle parameters of an enemy
	 * based on the contents of some in-game variables.
	 * (Default: V[1001] - V[1010])
	 * 
	 * When a switch is set (default: S[1001) to ON, an alternative
	 * scaling formular, based on the average party level is used.
	 *
	 * Default formula:     val = val * V[...] / 1000
	 * Alternative formula: val = val * avg_level * V[...] / 1000
	 */
	namespace MonSca {
		/** Scales an enemies큦 maximum HP stat, based on the value of variable V[1001] */
		void ModifyMaxHp(int& val);
		/** Scales an enemies큦 maximum SP stat, based on the value of variable V[1002] */
		void ModifyMaxSp(int& val);
		/** Scales an enemies큦 attack stat, based on the value of variable V[1003] */
		void ModifyAtk(int& val);
		/** Scales an enemies큦 defense stat, based on the value of variable V[1004] */
		void ModifyDef(int& val);
		/** Scales an enemies큦 spirit stat, based on the value of variable V[1005] */
		void ModifySpi(int& val);
		/** Scales an enemies큦 agility stat, based on the value of variable V[1006] */
		void ModifyAgi(int& val);
		/** Scales the experience points gained by defating an enemy, based on the value of variable V[1007] */
		void ModifyExpGained(int& val);
		/** Scales the money gained by defating an enemy, based on the value of variable V[1008] */
		void ModifyMoneyGained(int& val);
		/**
		 * Modifies the item dropped by defating an enemy, based on the value of variable V[1009]
		 * In contrast to other modifers of this patch, this skips the normal formula and just
		 * adds the variable value to the result.
		 */
		void ModifyItemGained(int& item_id);
		/** Scales the item drop rate of an enemy, based on the value of variable V[1010] */
		void ModifyItemDropRate(int& val);
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
}
#endif
