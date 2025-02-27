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
#include "game_map.h"
#include "game_party.h"
#include "game_switches.h"
#include "game_variables.h"
#include "game_actor.h"
#include "main_data.h"
#include "player.h"

bool RuntimePatches::EncounterRandomnessAlert::HandleEncounter(int troop_id) {
#ifdef NO_RUNTIME_PATCHES
	// no-op
	(void)troop_id;
	return false;
#endif
	if (auto var_id = Player::game_config.patch_encounter_random_alert_var.Get(); var_id > 0) {
		Main_Data::game_player->SetTotalEncounterRate(0);
		Main_Data::game_player->SetEncounterCalling(false);

		Main_Data::game_variables->Set(var_id, troop_id);
		Game_Map::SetNeedRefreshForVarChange(var_id);

		if (auto switch_id = Player::game_config.patch_encounter_random_alert_sw.Get(); switch_id > 0) {
			Main_Data::game_switches->Set(switch_id, true);
			Game_Map::SetNeedRefreshForSwitchChange(switch_id);
		}
		// Always refresh the map (Original patch does this only for the MEPR variant)
		Game_Map::Refresh();
		return true;
	}
	return false;
}

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

void RuntimePatches::EXPlus::ModifyExpGain(Game_Actor& actor, int& exp_gain) {
#ifdef NO_RUNTIME_PATCHES
	// no-op
	(void)actor;
	(void)exp_gain;
	return false;
#endif
	if (auto base_var_id = Player::game_config.patch_explus_var.Get(); base_var_id > 0) {
		exp_gain *= (100 + Main_Data::game_variables->Get(base_var_id + actor.GetPartyIndex()));
		exp_gain /= 100;
	}
}

void RuntimePatches::EXPlus::StoreActorPosition(int actor_id) {
#ifdef NO_RUNTIME_PATCHES
	// no-op
	(void)actor_id;
	return;
#endif
	if (auto var_id = Player::game_config.patch_explusplus_var.Get(); var_id > 0) {
		Main_Data::game_variables->Set(var_id, Main_Data::game_party->GetActorPositionInParty(actor_id) + 1);
	}
}
