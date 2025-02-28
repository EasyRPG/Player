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

#include "game_map.h"
#include "game_party.h"
#include "game_switches.h"
#include "game_variables.h"
#include "game_actor.h"
#include "game_battler.h"
#include "main_data.h"
#include "player.h"
#include "output.h"

namespace {
	template<int C>
	void LockPatchArguments(std::array<RuntimePatches::PatchArg, C> const& patch_args) {
		for (auto& patch_arg : patch_args) {
			patch_arg.config_param->Lock(0);
		}
	}

	template<int C>
	bool ParsePatchArguments(CmdlineParser& cp, CmdlineArg arg, std::array<RuntimePatches::PatchArg, C> const& patch_args) {
		if (arg.ArgIsOff()) {
			for (auto& patch_arg : patch_args) {
				patch_arg.config_param->Set(0);
			}
			return true;
		}
		if (arg.ArgIsOn()) {
			for (auto& patch_arg : patch_args) {
				patch_arg.config_param->Set(patch_arg.default_value);
			}

			std::string value;
			if (arg.ParseValue(0, value)) {
				std::string::iterator it;
				int i = 0, pos = 0, new_pos;
				while ((new_pos = value.rfind(',', pos)) >= 0) {
					int v = std::atoi(value.substr(pos, new_pos - pos).c_str());
					patch_args[i++].config_param->Set(v);
					if (i == static_cast<int>(patch_args.size())) {
						break;
					}
					pos = new_pos + 1;
				}
				return true;
			}

			bool parsed;
			long li_value = 0;
			do {
				parsed = false;
				for (int i = 0; i < static_cast<int>(patch_args.size()); ++i) {
					if (cp.ParseNext(arg, 1, patch_args[i].cmd_arg)) {
						parsed = true;
						if (arg.ParseValue(0, li_value)) {
							patch_args[i].config_param->Set(li_value);
						}
					}
				}
			} while (parsed);

			return true;
		}
		return false;
	}

	template<int C>
	bool ParsePatchFromIni(lcf::INIReader& ini, std::array<RuntimePatches::PatchArg, C> const& patch_args) {
		bool patch_override = false;
		for (auto& patch_arg : patch_args) {
			patch_override |= patch_arg.config_param->FromIni(ini);
		}
		return patch_override;
	}

	template<int C>
	void PrintPatch(std::vector<std::string>& patches, std::array<RuntimePatches::PatchArg, C> const& patch_args) {
		assert(patch_args.size() > 0);

		bool is_set = false;
		for (auto& patch_arg : patch_args) {
			if (patch_arg.config_param->Get() > 0) {
				is_set = true;
				break;
			}
		}
		if (!is_set) {
			return;
		}

		if (patch_args.size() == 1) {
			patches.push_back(fmt::format("{} ({})", patch_args[0].config_param->GetName(), patch_args[0].config_param->Get()));
			return;
		}

		std::string out = fmt::format("{} (", patch_args[0].config_param->GetName());
		for (int i = 0; i < static_cast<int>(patch_args.size()); ++i) {
			if (i > 0) {
				out += ", ";
			}
			out += fmt::format("{}", patch_args[i].config_param->Get());
		}
		out += ")";
		patches.push_back(out);
	}
}

void RuntimePatches::LockPatchesAsDiabled() {
#ifndef NO_RUNTIME_PATCHES
	LockPatchArguments<2>(EncounterRandomnessAlert::patch_args);
	LockPatchArguments<11>(MonSca::patch_args);
	LockPatchArguments<2>(EXPlus::patch_args);
	LockPatchArguments<2>(GuardRevamp::patch_args);
#endif
}

bool RuntimePatches::ParseFromCommandLine(CmdlineParser& cp) {
#ifndef NO_RUNTIME_PATCHES
	CmdlineArg arg;
	if (cp.ParseNext(arg, 1, { "--patch-encounter-alert", "--no-patch-encounter-alert" })) {
		return ParsePatchArguments<2>(cp, arg, EncounterRandomnessAlert::patch_args);
	}
	if (cp.ParseNext(arg, 1, { "--patch-monsca", "--no-patch-monsca" })) {
		return ParsePatchArguments<11>(cp, arg, MonSca::patch_args);
	}
	if (cp.ParseNext(arg, 1, { "--patch-explus", "--no-patch-explus" })) {
		return ParsePatchArguments<2>(cp, arg, EXPlus::patch_args);
	}
	if (cp.ParseNext(arg, 1, { "--patch-guardrevamp", "--no-patch-guardrevamp" })) {
		return ParsePatchArguments<2>(cp, arg, GuardRevamp::patch_args);
	}
#else
	(void)cp;
#endif
	return false;
}

bool RuntimePatches::ParseFromIni(lcf::INIReader& ini) {
#ifndef NO_RUNTIME_PATCHES
	bool patch_override = false;
	patch_override |= ParsePatchFromIni<2>(ini, EncounterRandomnessAlert::patch_args);
	patch_override |= ParsePatchFromIni<11>(ini, MonSca::patch_args);
	patch_override |= ParsePatchFromIni<2>(ini, EXPlus::patch_args);
	patch_override |= ParsePatchFromIni<2>(ini, GuardRevamp::patch_args);
	return patch_override;
#else
	(void)ini;
	return false;
#endif
}

void RuntimePatches::DetermineActivePatches(std::vector<std::string>& patches) {
#ifndef NO_RUNTIME_PATCHES
	PrintPatch<2>(patches, EncounterRandomnessAlert::patch_args);
	PrintPatch<11>(patches, MonSca::patch_args);
	PrintPatch<2>(patches, EXPlus::patch_args);
	PrintPatch<2>(patches, GuardRevamp::patch_args);
#else
	(void)patches;
#endif
}

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

bool RuntimePatches::GuardRevamp::OverrideDamageAdjustment(int& dmg, const Game_Battler& target) {
#ifdef NO_RUNTIME_PATCHES
	// no-op
	(void)dmg;
	(void)target;
	return false;
#endif
	auto rate_normal = Player::game_config.patch_guardrevamp_normal.Get();
	auto rate_strong = Player::game_config.patch_guardrevamp_strong.Get();

	if ((rate_normal > 0 || rate_strong > 0) && target.IsDefending()) {
		if (!target.HasStrongDefense()) {
			if (rate_normal == 0) {
				return false;
			}
			dmg *= rate_normal;
		} else {
			if (rate_strong == 0) {
				return false;
			}
			dmg *= rate_strong;
		}
		dmg /= 100;
		return true;
	}
	return false;
}
