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
#include "game_constants.h"
#include "game_variables.h"
#include "output.h"
#include "player.h"

std::array<Game_Variables::Var_t, 2> Game_Constants::GetVariableLimits() {
	Game_Variables::Var_t min_var = lcf::Data::system.easyrpg_variable_min_value;
	TryGetOverriddenConstant(ConstantType::MinVarLimit, min_var);
	if (min_var == 0) {
		if (!Player::IsPatchManiac() || Player::game_config.patch_maniac.Get() == 2) {
			min_var = Player::IsRPG2k3() ? Game_Variables::min_2k3 : Game_Variables::min_2k;
		} else {
			min_var = std::numeric_limits<Game_Variables::Var_t>::min();
		}
	}
	Game_Variables::Var_t max_var = lcf::Data::system.easyrpg_variable_max_value;
	TryGetOverriddenConstant(ConstantType::MaxVarLimit, max_var);
	if (max_var == 0) {
		if (!Player::IsPatchManiac() || Player::game_config.patch_maniac.Get() == 2) {
			max_var = Player::IsRPG2k3() ? Game_Variables::max_2k3 : Game_Variables::max_2k;
		} else {
			max_var = std::numeric_limits<Game_Variables::Var_t>::max();
		}
	}

	return {min_var, max_var};
}

int32_t Game_Constants::MaxActorHpValue() {
	auto val = lcf::Data::system.easyrpg_max_actor_hp;
	TryGetOverriddenConstant(ConstantType::MaxActorHP, val);
	if (val == -1) {
		return Player::IsRPG2k() ? 999 : 9'999;
	}
	return val;
}

int32_t Game_Constants::MaxActorSpValue() {
	auto val = lcf::Data::system.easyrpg_max_actor_sp;
	TryGetOverriddenConstant(ConstantType::MaxActorSP, val);
	if (val == -1) {
		return 999;
	}
	return val;
}

int32_t Game_Constants::MaxEnemyHpValue() {
	auto val = lcf::Data::system.easyrpg_max_enemy_hp;
	if (val == -1) {
		// Upper bound is an editor limit, not enforced by the engine
		return std::numeric_limits<int32_t>::max();
	}
	return val;
}

int32_t Game_Constants::MaxEnemySpValue() {
	auto val = lcf::Data::system.easyrpg_max_enemy_sp;
	if (val == -1) {
		// Upper bound is an editor limit, not enforced by the engine
		return std::numeric_limits<int32_t>::max();
	}
	return val;
}

int32_t Game_Constants::MaxStatBaseValue() {
	auto val = lcf::Data::system.easyrpg_max_stat_base_value;
	TryGetOverriddenConstant(ConstantType::MaxStatBaseValue, val);
	if (val == -1) {
		return 999;
	}
	return val;
}

int32_t Game_Constants::MaxStatBattleValue() {
	auto val = lcf::Data::system.easyrpg_max_stat_battle_value;
	TryGetOverriddenConstant(ConstantType::MaxStatBattleValue, val);
	if (val == -1) {
		return 9'999;
	}
	return val;
}

int32_t Game_Constants::MaxDamageValue() {
	auto val = lcf::Data::system.easyrpg_max_damage;
	TryGetOverriddenConstant(ConstantType::MaxDamageValue, val);
	if (val == -1) {
		return (Player::IsRPG2k() ? 999 : 9'999);
	}
	return val;
}

int32_t Game_Constants::MaxExpValue() {
	auto val = lcf::Data::system.easyrpg_max_exp;
	TryGetOverriddenConstant(ConstantType::MaxExpValue, val);
	if (val == -1) {
		return Player::IsRPG2k() ? 999'999 : 9'999'999;
	}
	return val;
}

int32_t Game_Constants::MaxLevel() {
	auto max_level = Player::IsRPG2k() ? max_level_2k : max_level_2k3;
	if (TryGetOverriddenConstant(ConstantType::MaxLevel, max_level)) {
		return max_level;
	}
	if (lcf::Data::system.easyrpg_max_level > -1) {
		max_level = lcf::Data::system.easyrpg_max_level;
	}
	return max_level;
}

int32_t Game_Constants::MaxGoldValue() {
	int32_t max_gold = 999'999;
	if (TryGetOverriddenConstant(ConstantType::MaxGoldValue, max_gold)) {
		return max_gold;
	}
	return max_gold;
}

int32_t Game_Constants::MaxItemCount() {
	int32_t max_item_count = (lcf::Data::system.easyrpg_max_item_count == -1 ? 99 : lcf::Data::system.easyrpg_max_item_count);;
	TryGetOverriddenConstant(ConstantType::MaxItemCount, max_item_count);
	return max_item_count;
}

int32_t Game_Constants::MaxSaveFiles() {
	int32_t max_savefiles = Utils::Clamp<int32_t>(lcf::Data::system.easyrpg_max_savefiles, 3, 99);
	TryGetOverriddenConstant(ConstantType::MaxSaveFiles, max_savefiles);
	return max_savefiles;
}

bool Game_Constants::TryGetOverriddenConstant(ConstantType const_type, int32_t& out_value) {
	auto it = constant_overrides.find(const_type);
	if (it != constant_overrides.end()) {
		out_value = (*it).second;
	}
	return it != constant_overrides.end();
}

void Game_Constants::OverrideGameConstant(ConstantType const_type, int32_t value) {
	constant_overrides[const_type] = value;
}

void Game_Constants::PrintActiveOverrides() {
	std::vector<std::tuple<std::string, int32_t>> overridden_constants;

	auto it = kConstantType.begin();
	int32_t value;
	while (it != kConstantType.end()) {
		auto const_type = static_cast<ConstantType>((*it).value);
		if (!TryGetOverriddenConstant(const_type, value)) {
			++it;
			continue;
		}
		overridden_constants.push_back(std::make_tuple((*it).name, value));
		++it;
	}

	if (!overridden_constants.empty()) {
		std::string out = "Overridden Game Constants: ";
		bool first = true;
		for (const auto& p : overridden_constants) {
			if (!first) {
				out += ", ";
			}
			out += fmt::format("{}: {}", std::get<std::string>(p), std::get<int32_t>(p));
			first = false;
		}
		Output::DebugStr(out);
	}
}
