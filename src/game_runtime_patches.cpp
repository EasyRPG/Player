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
#include "game_runtime_patches.h"

#include "game_config.h"
#include "game_party.h"
#include "game_switches.h"
#include "game_variables.h"
#include "main_data.h"
#include "player.h"

namespace RuntimePatches::MonSca {
#ifndef NO_RUNTIME_PATCHES
	bool UseLevelBasedFormula() {
		auto switch_id = Player::game_config.patch_monsca_levelscaling.Get();
		return switch_id > 0 && Main_Data::game_switches->Get(switch_id);
	}

	void ApplyScaling(int& val, int mod) {
		if (mod == 0) {
			return;
		}
		if (UseLevelBasedFormula()) {
			mod *= Main_Data::game_party->GetAverageLevel();
		}
		val *= mod;
		val /= 1000;
	}
#endif
}

void RuntimePatches::MonSca::ModifyMaxHp(int& val) {
#ifdef NO_RUNTIME_PATCHES
	// no-op
	(void)val;
	return;
#endif
	if (auto var_id = Player::game_config.patch_monsca_maxhp.Get(); var_id > 0) {
		ApplyScaling(val, Main_Data::game_variables->Get(var_id));
	}
}

void RuntimePatches::MonSca::ModifyMaxSp(int& val) {
#ifdef NO_RUNTIME_PATCHES
	// no-op
	(void)val;
	return;
#endif
	if (auto var_id = Player::game_config.patch_monsca_maxsp.Get(); var_id > 0) {
		ApplyScaling(val, Main_Data::game_variables->Get(var_id));
	}
}

void RuntimePatches::MonSca::ModifyAtk(int& val) {
#ifdef NO_RUNTIME_PATCHES
	// no-op
	(void)val;
	return;
#endif
	if (auto var_id = Player::game_config.patch_monsca_atk.Get(); var_id > 0) {
		ApplyScaling(val, Main_Data::game_variables->Get(var_id));
	}
}

void RuntimePatches::MonSca::ModifyDef(int& val) {
#ifdef NO_RUNTIME_PATCHES
	// no-op
	(void)val;
	return;
#endif
	if (auto var_id = Player::game_config.patch_monsca_def.Get(); var_id > 0) {
		ApplyScaling(val, Main_Data::game_variables->Get(var_id));
	}
}

void RuntimePatches::MonSca::ModifySpi(int& val) {
#ifdef NO_RUNTIME_PATCHES
	// no-op
	(void)val;
	return;
#endif
	if (auto var_id = Player::game_config.patch_monsca_spi.Get(); var_id > 0) {
		ApplyScaling(val, Main_Data::game_variables->Get(var_id));
	}
}

void RuntimePatches::MonSca::ModifyAgi(int& val) {
#ifdef NO_RUNTIME_PATCHES
	// no-op
	(void)val;
	return;
#endif
	if (auto var_id = Player::game_config.patch_monsca_agi.Get(); var_id > 0) {
		ApplyScaling(val, Main_Data::game_variables->Get(var_id));
	}
}

void RuntimePatches::MonSca::ModifyExpGained(int& val) {
#ifdef NO_RUNTIME_PATCHES
	// no-op
	(void)val;
	return;
#endif
	if (auto var_id = Player::game_config.patch_monsca_exp.Get(); var_id > 0) {
		ApplyScaling(val, Main_Data::game_variables->Get(var_id));
	}
}

void RuntimePatches::MonSca::ModifyMoneyGained(int& val) {
#ifdef NO_RUNTIME_PATCHES
	// no-op
	(void)val;
	return;
#endif
	if (auto var_id = Player::game_config.patch_monsca_gold.Get(); var_id > 0) {
		ApplyScaling(val, Main_Data::game_variables->Get(var_id));
	}
}

void RuntimePatches::MonSca::ModifyItemGained(int& item_id) {
#ifdef NO_RUNTIME_PATCHES
	// no-op
	(void)item_id;
	return;
#endif
	if (auto var_id = Player::game_config.patch_monsca_item.Get(); var_id > 0) {
		item_id += Main_Data::game_variables->Get(var_id);
	}
}

void RuntimePatches::MonSca::ModifyItemDropRate(int& val) {
#ifdef NO_RUNTIME_PATCHES
	// no-op
	(void)val;
	return;
#endif
	if (auto var_id = Player::game_config.patch_monsca_droprate.Get(); var_id > 0) {
		ApplyScaling(val, Main_Data::game_variables->Get(var_id));
	}
}
