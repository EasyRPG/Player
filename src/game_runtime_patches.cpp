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

#include <cmath>
#include "game_map.h"
#include "game_party.h"
#include "game_switches.h"
#include "game_variables.h"
#include "game_actor.h"
#include "game_battler.h"
#include "game_enemy.h"
#include "main_data.h"
#include "player.h"
#include "output.h"
#include "input.h"
#include "scene.h"
#include "scene_load.h"
#include "baseui.h"

namespace {
	template<size_t C>
	void LockPatchArguments(std::array<RuntimePatches::PatchArg, C> const& patch_args) {
		for (auto& patch_arg : patch_args) {
			patch_arg.config_param.Lock(0);
		}
	}

	template<size_t C>
	bool ParsePatchArguments(CmdlineParser& cp, CmdlineArg arg, std::array<RuntimePatches::PatchArg, C> const& patch_args) {
		if (arg.ArgIsOff()) {
			for (auto& patch_arg : patch_args) {
				patch_arg.config_param.Set(0);
			}
			return true;
		}
		if (arg.ArgIsOn()) {
			for (auto& patch_arg : patch_args) {
				patch_arg.config_param.Set(patch_arg.default_value);
			}
			bool parsed;
			long li_value = 0;
			do {
				parsed = false;
				for (int i = 0; i < static_cast<int>(patch_args.size()); ++i) {
					if (cp.ParseNext(arg, 1, patch_args[i].cmd_arg)) {
						parsed = true;
						if (arg.ParseValue(0, li_value)) {
							patch_args[i].config_param.Set(li_value);
						}
					}
				}
			} while (parsed);

			return true;
		}
		return false;
	}

	template<size_t C>
	bool ParsePatchFromIni(lcf::INIReader& ini, std::array<RuntimePatches::PatchArg, C> const& patch_args) {
		bool patch_override = false;
		for (auto& patch_arg : patch_args) {
			patch_override |= patch_arg.config_param.FromIni(ini);
		}
		return patch_override;
	}

	template<size_t C>
	void PrintPatch(std::vector<std::string>& patches, std::array<RuntimePatches::PatchArg, C> const& patch_args) {
		assert(patch_args.size() > 0);

		bool is_set = false;
		for (auto& patch_arg : patch_args) {
			if (patch_arg.config_param.Get() > 0) {
				is_set = true;
				break;
			}
		}
		if (!is_set) {
			return;
		}

		if (patch_args.size() == 1) {
			patches.push_back(fmt::format("{} ({})", patch_args[0].config_param.GetName(), patch_args[0].config_param.Get()));
			return;
		}

		std::string out = fmt::format("{} (", patch_args[0].config_param.GetName());
		for (int i = 0; i < static_cast<int>(patch_args.size()); ++i) {
			if (i > 0) {
				out += ", ";
			}
			out += fmt::format("{}", patch_args[i].config_param.Get());
		}
		out += ")";

		patches.push_back(out);
	}
}

void RuntimePatches::LockPatchesAsDiabled() {
	LockPatchArguments(EncounterRandomnessAlert::patch_args);
	LockPatchArguments(MonSca::patch_args);
	LockPatchArguments(EXPlus::patch_args);
	LockPatchArguments(GuardRevamp::patch_args);
}

bool RuntimePatches::ParseFromCommandLine(CmdlineParser& cp) {
	CmdlineArg arg;
	if (cp.ParseNext(arg, 1, { "--patch-encounter-alert", "--no-patch-encounter-alert" })) {
		return ParsePatchArguments(cp, arg, EncounterRandomnessAlert::patch_args);
	}
	if (cp.ParseNext(arg, 1, { "--patch-monsca", "--no-patch-monsca" })) {
		return ParsePatchArguments(cp, arg, MonSca::patch_args);
	}
	if (cp.ParseNext(arg, 1, { "--patch-explus", "--no-patch-explus" })) {
		return ParsePatchArguments(cp, arg, EXPlus::patch_args);
	}
	if (cp.ParseNext(arg, 1, { "--patch-guardrevamp", "--no-patch-guardrevamp" })) {
		return ParsePatchArguments(cp, arg, GuardRevamp::patch_args);
	}
	return false;
}

bool RuntimePatches::ParseFromIni(lcf::INIReader& ini) {
	bool patch_override = false;
	patch_override |= ParsePatchFromIni(ini, EncounterRandomnessAlert::patch_args);
	patch_override |= ParsePatchFromIni(ini, MonSca::patch_args);
	patch_override |= ParsePatchFromIni(ini, EXPlus::patch_args);
	patch_override |= ParsePatchFromIni(ini, GuardRevamp::patch_args);
	return patch_override;
}

void RuntimePatches::DetermineActivePatches(std::vector<std::string>& patches) {
	PrintPatch(patches, EncounterRandomnessAlert::patch_args);
	PrintPatch(patches, MonSca::patch_args);
	PrintPatch(patches, EXPlus::patch_args);
	PrintPatch(patches, GuardRevamp::patch_args);
}

void RuntimePatches::OnVariableChanged(int variable_id) {
	if (EP_UNLIKELY(Player::game_config.patch_powermode.Get())) {
		PowerMode2003::HandleVariableHooks(variable_id);
	}
}
void RuntimePatches::OnVariableChanged(std::initializer_list<int> variable_ids) {
	if (EP_UNLIKELY(Player::game_config.patch_powermode.Get())) {
		for (int var_id : variable_ids) {
			PowerMode2003::HandleVariableHooks(var_id);
		}
	}
}

void RuntimePatches::OnVariableRangeChanged(int start_id, int end_id) {
	if (EP_UNLIKELY(Player::game_config.patch_powermode.Get())) {
		for (int var_id = start_id; var_id <= end_id; ++var_id) {
			PowerMode2003::HandleVariableHooks(var_id);
		}
	}
}

bool RuntimePatches::EncounterRandomnessAlert::HandleEncounter(int troop_id) {
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
	bool UseLevelBasedFormula() {
		auto switch_id = Player::game_config.patch_monsca_levelscaling.Get();
		return switch_id > 0 && Main_Data::game_switches->Get(switch_id);
	}

	int GetVariableId(Game_Enemy const& enemy, int var_id) {
		if (Player::game_config.patch_monsca_plus.Get() > 0) {
			return var_id + enemy.GetTroopMemberId();
		}
		return var_id;
	}

	template<typename T>
	void ApplyScaling(Game_Enemy const& enemy, T& val, int var_id) {
		int mod = Main_Data::game_variables->Get(GetVariableId(enemy, var_id));
		if (mod == 0) {
			return;
		}
		if (UseLevelBasedFormula()) {
			mod *= Main_Data::game_party->GetAverageLevel();
		}
		val *= mod;
		val /= 1000;
	}
}

void RuntimePatches::MonSca::ModifyMaxHp(Game_Enemy const& enemy, int32_t& val) {
	if (auto var_id = Player::game_config.patch_monsca_maxhp.Get(); var_id > 0) {
		ApplyScaling(enemy, val, var_id);
	}
}

void RuntimePatches::MonSca::ModifyMaxSp(Game_Enemy const& enemy, int32_t& val) {
	if (auto var_id = Player::game_config.patch_monsca_maxsp.Get(); var_id > 0) {
		ApplyScaling(enemy, val, var_id);
	}
}

void RuntimePatches::MonSca::ModifyAtk(Game_Enemy const& enemy, int32_t& val) {
	if (auto var_id = Player::game_config.patch_monsca_atk.Get(); var_id > 0) {
		ApplyScaling(enemy, val, var_id);
	}
}

void RuntimePatches::MonSca::ModifyDef(Game_Enemy const& enemy, int32_t& val) {
	if (auto var_id = Player::game_config.patch_monsca_def.Get(); var_id > 0) {
		ApplyScaling(enemy, val, var_id);
	}
}

void RuntimePatches::MonSca::ModifySpi(Game_Enemy const& enemy, int32_t& val) {
	if (auto var_id = Player::game_config.patch_monsca_spi.Get(); var_id > 0) {
		ApplyScaling(enemy, val, var_id);
	}
}

void RuntimePatches::MonSca::ModifyAgi(Game_Enemy const& enemy, int32_t& val) {
	if (auto var_id = Player::game_config.patch_monsca_agi.Get(); var_id > 0) {
		ApplyScaling(enemy, val, var_id);
	}
}

void RuntimePatches::MonSca::ModifyExpGained(Game_Enemy const& enemy, int& val) {
	if (auto var_id = Player::game_config.patch_monsca_exp.Get(); var_id > 0) {
		ApplyScaling(enemy, val, var_id);
	}
}

void RuntimePatches::MonSca::ModifyMoneyGained(Game_Enemy const& enemy, int& val) {
	if (auto var_id = Player::game_config.patch_monsca_gold.Get(); var_id > 0) {
		ApplyScaling(enemy, val, var_id);
	}
}

void RuntimePatches::MonSca::ModifyItemGained(Game_Enemy const& enemy, int& item_id) {
	if (auto var_id = Player::game_config.patch_monsca_item.Get(); var_id > 0) {
		 item_id += Main_Data::game_variables->Get(GetVariableId(enemy, var_id));
	}
}

void RuntimePatches::MonSca::ModifyItemDropRate(Game_Enemy const& enemy, int& val) {
	if (auto var_id = Player::game_config.patch_monsca_droprate.Get(); var_id > 0) {
		ApplyScaling(enemy, val, var_id);
	}
}

void RuntimePatches::EXPlus::ModifyExpGain(Game_Actor& actor, int& exp_gain) {
	if (auto base_var_id = Player::game_config.patch_explus_var.Get(); base_var_id > 0) {
		exp_gain *= (100 + Main_Data::game_variables->Get(base_var_id + actor.GetPartyIndex()));
		exp_gain /= 100;
	}
}

void RuntimePatches::EXPlus::StoreActorPosition(int actor_id) {
	if (auto var_id = Player::game_config.patch_explusplus_var.Get(); var_id > 0) {
		Main_Data::game_variables->Set(var_id, Main_Data::game_party->GetActorPositionInParty(actor_id) + 1);
	}
}

bool RuntimePatches::GuardRevamp::OverrideDamageAdjustment(int& dmg, const Game_Battler& target) {
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

namespace RuntimePatches::PowerMode2003 {
	void HandleKeyboard() {
#if !defined(SUPPORT_KEYBOARD)
		Output::Warning("PowerMode2003: Keyboard input is not supported on this platform");
		return;
#endif
		int vk_id = Main_Data::game_variables->Get(PM_VAR_KEY);
		if (vk_id == 0) {
			// check the whole keyboard for any key press
			for (int i = 1; i < Input::Keys::KEYS_COUNT; ++i) {
				auto input_key = static_cast<Input::Keys::InputKey>(i);
				if (Input::IsRawKeyPressed(input_key) && (vk_id = RuntimePatches::VirtualKeys::InputKeyToVirtualKey(input_key))) {
					break;
				}
			}
			Main_Data::game_variables->Set(PM_VAR_KEY, vk_id);
		} else if (vk_id > 0 && vk_id <= 255) {
			// check a single key
			auto input_key = RuntimePatches::VirtualKeys::VirtualKeyToInputKey(vk_id);
			int result = 0;
			if (input_key == Input::Keys::NONE) {
				Output::Debug("PowerMode2003: Unsupported keycode {}", vk_id);
			}
			if (!Input::IsRawKeyPressed(input_key)) {
				Main_Data::game_variables->Set(PM_VAR_KEY, 0);
			}
		} else {
			Main_Data::game_variables->Set(PM_VAR_KEY, 0);
		}
	}

	void HandleFloatComputation() {
		auto input1 = static_cast<float>(Main_Data::game_variables->Get(PM_VAR_FVALUE1));

		// Some versions of the documentation for this patch have the target ids swapped.
		// (SIN -> FVALUE1, COS -> FVALUE2, TAN -> FVALUE1)
		//
		// But the only known RPG_RT versions of this patch actually save them like follows..:
		switch (Main_Data::game_variables->Get(PM_VAR_FCODE)) {
			case 1:
			{
				auto v_sin = std::sin(input1 * M_PI / 180);
				auto v_cos = std::cos(input1 * M_PI / 180);
				Main_Data::game_variables->Set(PM_VAR_FVALUE2, static_cast<Game_Variables::Var_t>(v_sin * 1'000'000));
				Main_Data::game_variables->Set(PM_VAR_FVALUE1, static_cast<Game_Variables::Var_t>(v_cos * 1'000'000));
				break;
			}
			case 2:
			{
				auto v_tan = std::tan(input1 * M_PI / 180);
				Main_Data::game_variables->Set(PM_VAR_FVALUE2, static_cast<Game_Variables::Var_t>(v_tan * 1'000'000));
				break;
			}
			case 3:
			{
				float v_int;
				float v_fract = std::modf(std::sqrt(input1), &v_int);
				Main_Data::game_variables->Set(PM_VAR_FVALUE1, static_cast<Game_Variables::Var_t>(v_int));
				Main_Data::game_variables->Set(PM_VAR_FVALUE2, static_cast<Game_Variables::Var_t>(v_fract * 1'000'000));
				break;
			}
			case 4:
			{
				auto input2 = static_cast<float>(Main_Data::game_variables->Get(PM_VAR_FVALUE2));
				float v_int;
				float v_fract = std::modf(input1 / input2, &v_int);
				Main_Data::game_variables->Set(PM_VAR_FVALUE1, static_cast<Game_Variables::Var_t>(v_int));
				Main_Data::game_variables->Set(PM_VAR_FVALUE2, static_cast<Game_Variables::Var_t>(v_fract * 1'000'000));
				break;
			}
			default:
				break;
		}
	}
}

void RuntimePatches::PowerMode2003::Init() {
	if (Player::game_config.patch_powermode.Get()) {
		Player::game_config.new_game.Set(true);
		Main_Data::game_variables->Set(PM_VAR_CR0, FileFinder::HasSavegame() ? 1 : 0);
	}
}

void RuntimePatches::PowerMode2003::HandleVariableHooks(int var_id) {
	switch (var_id) {
		case PM_VAR_CR0:
		{
			int op = Main_Data::game_variables->Get(PM_VAR_CR0);
			if (op == 255 && FileFinder::HasSavegame()) {
				Scene::instance->SetRequestedScene(std::make_shared<Scene_Load>());
			} else if (op == 254) {
				Player::exit_flag = true;
			}
			Main_Data::game_variables->Set(PM_VAR_CR0, FileFinder::HasSavegame() ? 1 : 0);
			break;
		}
		case PM_VAR_MCOORDY:
		{
			Point mouse_pos = Input::GetMousePosition();
			Main_Data::game_variables->Set(PM_VAR_MCOORDX, mouse_pos.x);
			Main_Data::game_variables->Set(PM_VAR_MCOORDY, mouse_pos.y);
			Game_Map::SetNeedRefreshForVarChange(PM_VAR_MCOORDX);
			break;
		}
		case PM_VAR_KEY:
			HandleKeyboard();
			break;
		case PM_VAR_FCODE:
			HandleFloatComputation();
			Game_Map::SetNeedRefreshForVarChange(PM_VAR_FVALUE1);
			Game_Map::SetNeedRefreshForVarChange(PM_VAR_FVALUE2);
			break;
		default:
			return;
	}
}
