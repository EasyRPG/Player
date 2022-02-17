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
#include "battle_message.h"
#include "player.h"
#include "utils.h"
#include "algo.h"
#include "game_battler.h"
#include <lcf/rpg/state.h>
#include <lcf/data.h>
#include <lcf/reader_util.h>
#include "feature.h"

namespace BattleMessage {

static std::string GetStateMessage(StringView target_name, StringView message) {
	if (Feature::HasPlaceholders()) {
		return Utils::ReplacePlaceholders(
			message,
			Utils::MakeArray('S'),
			Utils::MakeSvArray(target_name)
		);
	}
	else {
		return ToString(target_name) + ToString(message);
	}
}

std::string GetStateInflictMessage(const Game_Battler& target, const lcf::rpg::State& state) {
	if (target.GetType() == Game_Battler::Type_Ally) {
		return GetStateMessage(target.GetName(), state.message_actor);
	}
	if (target.GetType() == Game_Battler::Type_Enemy) {
		return GetStateMessage(target.GetName(), state.message_enemy);
	}
	return "";
}

std::string GetStateRecoveryMessage(const Game_Battler& target, const lcf::rpg::State& state) {
	return GetStateMessage(target.GetName(), state.message_recovery);
}

std::string GetStateAffectedMessage(const Game_Battler& target, const lcf::rpg::State& state) {
	return GetStateMessage(target.GetName(), state.message_affected);
}

std::string GetStateAlreadyMessage(const Game_Battler& target, const lcf::rpg::State& state) {
	return GetStateMessage(target.GetName(), state.message_already);
}

std::string GetDeathMessage(const Game_Battler& target) {
	const auto* state = lcf::ReaderUtil::GetElement(lcf::Data::states, 1);
	if (state) {
		return GetStateInflictMessage(target, *state);
	}
	return "";
}

static std::string GetActionFailureMessage(StringView source, StringView target, StringView message) {
	if (Feature::HasPlaceholders()) {
		return Utils::ReplacePlaceholders(
			message,
			Utils::MakeArray('S', 'O'),
			Utils::MakeSvArray(source, target)
		);
	}
	else {
		return ToString(target) + ToString(message);
	}
}

std::string GetPhysicalFailureMessage(const Game_Battler& source, const Game_Battler& target) {
	return GetActionFailureMessage(source.GetName(), target.GetName(), lcf::Data::terms.dodge);
}

std::string GetSkillFailureMessage(const Game_Battler& source, const Game_Battler& target, const lcf::rpg::Skill& skill) {
	StringView msg;
	switch (skill.failure_message) {
		case 0:
			msg = lcf::Data::terms.skill_failure_a;
			break;
		case 1:
			msg = lcf::Data::terms.skill_failure_b;
			break;
		case 2:
			msg = lcf::Data::terms.skill_failure_c;
			break;
		case 3:
			msg = lcf::Data::terms.dodge;
			break;
		default:
			break;
	}
	return GetActionFailureMessage(source.GetName(), target.GetName(), msg);
}

std::string GetUndamagedMessage(const Game_Battler& target) {
	StringView name = target.GetName();
	StringView message = (target.GetType() == Game_Battler::Type_Ally)
		? StringView(lcf::Data::terms.actor_undamaged)
		: StringView(lcf::Data::terms.enemy_undamaged);

	if (Feature::HasPlaceholders()) {
		return Utils::ReplacePlaceholders(
			message,
			Utils::MakeArray('S'),
			Utils::MakeSvArray(name)
		);
	}
	else {
		return ToString(name) + ToString(message);
	}
}

std::string GetCriticalHitMessage(const Game_Battler& source, const Game_Battler& target) {
	StringView message = (target.GetType() == Game_Battler::Type_Ally)
		? StringView(lcf::Data::terms.actor_critical)
		: StringView(lcf::Data::terms.enemy_critical);

	if (Feature::HasPlaceholders()) {
		return Utils::ReplacePlaceholders(
			message,
			Utils::MakeArray('S', 'O'),
			Utils::MakeSvArray(source.GetName(), target.GetName())
		);
	}
	else {
		return ToString(message);
	}
}

static std::string GetHpSpRecoveredMessage(const Game_Battler& target, int value, StringView points) {
	if (Feature::HasPlaceholders()) {
		return Utils::ReplacePlaceholders(
			lcf::Data::terms.hp_recovery,
			Utils::MakeArray('S', 'V', 'U'),
			Utils::MakeSvArray(target.GetName(), std::to_string(value), points)
		);
	}
	std::stringstream ss;
	std::string particle, particle2, space = "";

	ss << target.GetName();
	if (Player::IsCP932()) {
		particle = "の";
		particle2 = "が ";
		space += " ";
	}
	else {
		particle = particle2 = " ";
	}
	ss << particle << points << particle2;
	ss << value << space << lcf::Data::terms.hp_recovery;
	return ss.str();
}

std::string GetHpRecoveredMessage(const Game_Battler& target, int value) {
	return GetHpSpRecoveredMessage(target, value, lcf::Data::terms.health_points);
}

std::string GetSpRecoveredMessage(const Game_Battler& target, int value) {
	return GetHpSpRecoveredMessage(target, value, lcf::Data::terms.spirit_points);
}

std::string GetParameterAbsorbedMessage(const Game_Battler& source, const Game_Battler& target, int value, StringView points) {
	const auto target_is_ally = (target.GetType() == Game_Battler::Type_Ally);
	StringView message = target_is_ally
		? StringView(lcf::Data::terms.actor_hp_absorbed)
		: StringView(lcf::Data::terms.enemy_hp_absorbed);

	if (Feature::HasPlaceholders()) {
		return Utils::ReplacePlaceholders(
			message,
			Utils::MakeArray('S', 'O', 'V', 'U'),
			Utils::MakeSvArray(source.GetName(), target.GetName(), std::to_string(value), points)
		);
	}
	std::stringstream ss;
	std::string particle, particle2, space = "";

	ss << target.GetName();

	if (Player::IsCP932()) {
		particle = (target_is_ally ? "は" : "の");
		particle2 = "を ";
		space += " ";
	} else {
		particle = particle2 = " ";
	}
	ss << particle << points << particle2;
	ss << value << space << message;

	return ss.str();
}

std::string GetHpAbsorbedMessage(const Game_Battler& source, const Game_Battler& target, int value) {
	return GetParameterAbsorbedMessage(source, target, value, lcf::Data::terms.health_points);
}

std::string GetSpAbsorbedMessage(const Game_Battler& source, const Game_Battler& target, int value) {
	return GetParameterAbsorbedMessage(source, target, value, lcf::Data::terms.spirit_points);
}

std::string GetAtkAbsorbedMessage(const Game_Battler& source, const Game_Battler& target, int value) {
	return GetParameterAbsorbedMessage(source, target, value, lcf::Data::terms.attack);
}

std::string GetDefAbsorbedMessage(const Game_Battler& source, const Game_Battler& target, int value) {
	return GetParameterAbsorbedMessage(source, target, value, lcf::Data::terms.defense);
}

std::string GetSpiAbsorbedMessage(const Game_Battler& source, const Game_Battler& target, int value) {
	return GetParameterAbsorbedMessage(source, target, value, lcf::Data::terms.spirit);
}

std::string GetAgiAbsorbedMessage(const Game_Battler& source, const Game_Battler& target, int value) {
	return GetParameterAbsorbedMessage(source, target, value, lcf::Data::terms.agility);
}

std::string GetDamagedMessage(const Game_Battler& target, int value) {
	bool target_is_ally = (target.GetType() == Game_Battler::Type_Ally);
	StringView message = target_is_ally
		? StringView(lcf::Data::terms.actor_damaged)
		: StringView(lcf::Data::terms.enemy_damaged);

	if (Feature::HasPlaceholders()) {
		return Utils::ReplacePlaceholders(
			message,
			Utils::MakeArray('S', 'V', 'U'),
			Utils::MakeSvArray(target.GetName(), std::to_string(value), lcf::Data::terms.health_points)
		);
	}
	std::stringstream ss;
	std::string particle, space = "";
	ss << target.GetName();

	if (Player::IsCP932()) {
		particle = (target_is_ally ? "は " : "に ");
		space += " ";
	} else {
		particle = " ";
	}
	ss << particle << value << space << message;
	return ss.str();
}

std::string GetParameterChangeMessage(const Game_Battler& target, int value, StringView points) {
	const bool is_positive = (value >= 0);
	value = std::abs(value);
	if (value == 0) {
		return "";
	}

	StringView message = is_positive
		? StringView(lcf::Data::terms.parameter_increase)
	   	: StringView(lcf::Data::terms.parameter_decrease);


	if (Feature::HasPlaceholders()) {
		return Utils::ReplacePlaceholders(
			message,
			Utils::MakeArray('S', 'V', 'U'),
			Utils::MakeSvArray(target.GetName(), std::to_string(value), points)
		);
	}
	std::stringstream ss;
	std::string particle, particle2, space = "";
	ss << target.GetName();

	if (Player::IsCP932()) {
		particle = "の";
		particle2 = "が ";
		space += " ";
	}
	else {
		particle = particle2 = " ";
	}
	ss << particle << points << particle2 << value << space;
	ss << message;

	return ss.str();
}

std::string GetSpReduceMessage(const Game_Battler& target, int value) {
	return GetParameterChangeMessage(target, -value, lcf::Data::terms.spirit_points);
}

std::string GetAtkChangeMessage(const Game_Battler& target, int value) {
	return GetParameterChangeMessage(target, value, lcf::Data::terms.attack);
}

std::string GetDefChangeMessage(const Game_Battler& target, int value) {
	return GetParameterChangeMessage(target, value, lcf::Data::terms.defense);
}

std::string GetSpiChangeMessage(const Game_Battler& target, int value) {
	return GetParameterChangeMessage(target, value, lcf::Data::terms.spirit);
}

std::string GetAgiChangeMessage(const Game_Battler& target, int value) {
	return GetParameterChangeMessage(target, value, lcf::Data::terms.agility);
}

std::string GetAttributeShiftMessage(const Game_Battler& target, int value, const lcf::rpg::Attribute& attribute) {
	const bool is_positive = (value >= 0);
	value = std::abs(value);
	if (value == 0) {
		return "";
	}
	StringView message = is_positive
		? StringView(lcf::Data::terms.resistance_increase)
		: StringView(lcf::Data::terms.resistance_decrease);
	std::stringstream ss;

	if (Feature::HasPlaceholders()) {
		return Utils::ReplacePlaceholders(
			message,
			Utils::MakeArray('S', 'O'),
			Utils::MakeSvArray(target.GetName(), attribute.name)
		);
	}
	std::string particle, space = "";
	ss << target.GetName();

	if (Player::IsCP932()) {
		particle = "は";
		space += " ";
	}
	else {
		particle = " ";
	}
	ss << particle << attribute.name << space;
	ss << message;

	return ss.str();
}

static std::string GetBasicStartMessage2k(const Game_Battler& source, StringView term) {
	if (Feature::HasPlaceholders()) {
		return Utils::ReplacePlaceholders(
			term,
			Utils::MakeArray('S'),
			Utils::MakeSvArray(source.GetName())
		);
	}
	return ToString(source.GetName()) + ToString(term);
}

std::string GetNormalAttackStartMessage2k(const Game_Battler& source) {
	return GetBasicStartMessage2k(source, lcf::Data::terms.attacking);
}

std::string GetDefendStartMessage2k(const Game_Battler& source) {
	return GetBasicStartMessage2k(source, lcf::Data::terms.defending);
}

std::string GetObserveStartMessage2k(const Game_Battler& source) {
	return GetBasicStartMessage2k(source, lcf::Data::terms.observing);
}

std::string GetChargeUpStartMessage2k(const Game_Battler& source) {
	return GetBasicStartMessage2k(source, lcf::Data::terms.focus);
}

std::string GetSelfDestructStartMessage2k(const Game_Battler& source) {
	return GetBasicStartMessage2k(source, lcf::Data::terms.autodestruction);
}

std::string GetEscapeStartMessage2k(const Game_Battler& source) {
	return GetBasicStartMessage2k(source, lcf::Data::terms.enemy_escape);
}

std::string GetTransformStartMessage(const Game_Battler& source, const lcf::rpg::Enemy& new_enemy) {
	if (Feature::HasPlaceholders()) {
		return Utils::ReplacePlaceholders(
			lcf::Data::terms.enemy_transform,
			Utils::MakeArray('S', 'O'),
			Utils::MakeSvArray(source.GetName(), new_enemy.name)
		);
	}
	return ToString(source.GetName()) + ToString(lcf::Data::terms.enemy_transform);
}

static std::string GetSkillStartMessageGeneric(const Game_Battler& source, const Game_Battler* target, const lcf::rpg::Skill& skill, StringView usage, bool second_message = false) {
	StringView target_name = "???";
	if (target && Algo::IsNormalOrSubskill(skill) && Algo::SkillTargetsOne(skill)) {
		target_name = target->GetName();
	}
	if (Feature::HasPlaceholders()) {
		return Utils::ReplacePlaceholders(
				usage,
				Utils::MakeArray('S', 'O', 'U'),
				Utils::MakeSvArray(source.GetName(), target_name, skill.name)
				);
	}
	if (second_message) {
		return ToString(usage);
	} else {
		return ToString(source.GetName()) + ToString(usage);
	}
}

std::string GetSkillFirstStartMessage2k(const Game_Battler& source, const Game_Battler* target, const lcf::rpg::Skill& skill) {
	return GetSkillStartMessageGeneric(source, target, skill, skill.using_message1);
}

std::string GetSkillSecondStartMessage2k(const Game_Battler& source, const Game_Battler* target, const lcf::rpg::Skill& skill) {
	return GetSkillStartMessageGeneric(source, target, skill, skill.using_message2, true);
}

std::string GetItemStartMessage2k(const Game_Battler& source, const lcf::rpg::Item& item) {
	if (item.easyrpg_using_message != lcf::rpg::Item::kDefaultMessage) {
		return Utils::ReplacePlaceholders(
			item.easyrpg_using_message,
			Utils::MakeArray('S', 'O'),
			Utils::MakeSvArray(source.GetName(), item.name)
		);
	}

	if (Feature::HasPlaceholders()) {
		return Utils::ReplacePlaceholders(
			lcf::Data::terms.use_item,
			Utils::MakeArray('S', 'O'),
			Utils::MakeSvArray(source.GetName(), item.name)
		);
	}
	std::string particle;
	if (Player::IsCP932())
		particle = "は";
	else
		particle = " ";
	return ToString(source.GetName()) + particle + ToString(item.name) + ToString(lcf::Data::terms.use_item);
}

std::string GetDoubleAttackStartMessage2k3(const Game_Battler& source) {
	if (lcf::Data::terms.easyrpg_battle2k3_double_attack == lcf::Data::terms.kDefaultTerm) {
		return "Double Attack";
	}
	return Utils::ReplacePlaceholders(
		lcf::Data::terms.easyrpg_battle2k3_double_attack,
		Utils::MakeArray('S'),
		Utils::MakeSvArray(source.GetName())
	);
}

std::string GetSkillStartMessage2k3(const Game_Battler& source, const Game_Battler* target, const lcf::rpg::Skill& skill) {
	StringView target_name = "???";
	if (target && Algo::IsNormalOrSubskill(skill) && Algo::SkillTargetsOne(skill)) {
		target_name = target->GetName();
	}
	if (skill.easyrpg_battle2k3_message == lcf::rpg::Skill::kDefaultMessage) {
		if (lcf::Data::terms.easyrpg_battle2k3_skill == lcf::Data::terms.kDefaultTerm) {
			return ToString(skill.name);
		}
		return Utils::ReplacePlaceholders(
			lcf::Data::terms.easyrpg_battle2k3_skill,
			Utils::MakeArray('S', 'O', 'U'),
			Utils::MakeSvArray(source.GetName(), target_name, skill.name)
		);
	}
	return Utils::ReplacePlaceholders(
		skill.easyrpg_battle2k3_message,
		Utils::MakeArray('S', 'O', 'U'),
		Utils::MakeSvArray(source.GetName(), target_name, skill.name)
	);
}

std::string GetItemStartMessage2k3(const Game_Battler& source, const lcf::rpg::Item& item) {
	if (item.easyrpg_using_message == lcf::rpg::Item::kDefaultMessage) {
		if (lcf::Data::terms.easyrpg_battle2k3_item == lcf::Data::terms.kDefaultTerm) {
			return ToString(item.name);
		}
		return Utils::ReplacePlaceholders(
			lcf::Data::terms.easyrpg_battle2k3_item,
			Utils::MakeArray('S', 'O'),
			Utils::MakeSvArray(source.GetName(), item.name)
		);
	}
	return Utils::ReplacePlaceholders(
		item.easyrpg_using_message,
		Utils::MakeArray('S', 'O'),
		Utils::MakeSvArray(source.GetName(), item.name)
	);
}

std::string GetObserveStartMessage2k3(const Game_Battler& source) {
	if (lcf::Data::terms.easyrpg_battle2k3_observe == lcf::Data::terms.kDefaultTerm) {
		return "Observe";
	}
	return Utils::ReplacePlaceholders(
		lcf::Data::terms.easyrpg_battle2k3_observe,
		Utils::MakeArray('S'),
		Utils::MakeSvArray(source.GetName())
	);
}

std::string GetDefendStartMessage2k3(const Game_Battler& source) {
	if (lcf::Data::terms.easyrpg_battle2k3_defend == lcf::Data::terms.kDefaultTerm) {
		return "Defend";
	}
	return Utils::ReplacePlaceholders(
		lcf::Data::terms.easyrpg_battle2k3_defend,
		Utils::MakeArray('S'),
		Utils::MakeSvArray(source.GetName())
	);
}

std::string GetChargeUpStartMessage2k3(const Game_Battler& source) {
	if (lcf::Data::terms.easyrpg_battle2k3_charge == lcf::Data::terms.kDefaultTerm) {
		return "Charge Up";
	}
	return Utils::ReplacePlaceholders(
		lcf::Data::terms.easyrpg_battle2k3_charge,
		Utils::MakeArray('S'),
		Utils::MakeSvArray(source.GetName())
	);
}

std::string GetSelfDestructStartMessage2k3(const Game_Battler& source) {
	if (lcf::Data::terms.easyrpg_battle2k3_selfdestruct == lcf::Data::terms.kDefaultTerm) {
		return "Self-Destruct";
	}
	return Utils::ReplacePlaceholders(
		lcf::Data::terms.easyrpg_battle2k3_selfdestruct,
		Utils::MakeArray('S'),
		Utils::MakeSvArray(source.GetName())
	);
}

std::string GetEscapeStartMessage2k3(const Game_Battler& source) {
	if (lcf::Data::terms.easyrpg_battle2k3_escape == lcf::Data::terms.kDefaultTerm) {
		return "Escape";
	}
	return Utils::ReplacePlaceholders(
		lcf::Data::terms.easyrpg_battle2k3_escape,
		Utils::MakeArray('S'),
		Utils::MakeSvArray(source.GetName())
	);
}

} // namespace BattleMessage
