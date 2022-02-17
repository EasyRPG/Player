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

// Headers
#include "feature.h"
#include "player.h"
#include <lcf/data.h>

bool Feature::HasRpg2kBattleSystem() {
	if (Player::IsRPG2k()) {
		return true;
	}

	return lcf::Data::system.easyrpg_use_rpg2k_battle_system;
}

bool Feature::HasRpg2k3BattleSystem() {
	return !HasRpg2kBattleSystem();
}

bool Feature::HasRow() {
	if (HasRpg2k3BattleSystem() && !lcf::Data::battlecommands.easyrpg_disable_row_feature) {
		return true;
	}

	return false;
}

bool Feature::HasPlaceholders() {
	if (Player::IsRPG2kE()) {
		return true;
	}

	return Player::IsRPG2k3() && HasRpg2kBattleSystem() && lcf::Data::system.easyrpg_battle_use_rpg2ke_strings;
}
