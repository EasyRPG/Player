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

#ifndef EP_BATTLE_MESSAGE_H
#define EP_BATTLE_MESSAGE_H

#include <string>
#include "string_view.h"
#include <lcf/rpg/fwd.h>

class Game_Battler;

namespace BattleMessage {

std::string GetStateInflictMessage(const Game_Battler& target, const lcf::rpg::State& state);

std::string GetStateRecoveryMessage(const Game_Battler& target, const lcf::rpg::State& state);

std::string GetStateAffectedMessage(const Game_Battler& target, const lcf::rpg::State& state);

std::string GetStateAlreadyMessage(const Game_Battler& target, const lcf::rpg::State& state);

std::string GetDeathMessage(const Game_Battler& target);

std::string GetPhysicalFailureMessage(const Game_Battler& source, const Game_Battler& target);

std::string GetSkillFailureMessage(const Game_Battler& source, const Game_Battler& target, const lcf::rpg::Skill& skill);

std::string GetUndamagedMessage(const Game_Battler& target);

std::string GetCriticalHitMessage(const Game_Battler& source, const Game_Battler& target);

std::string GetHpRecoveredMessage(const Game_Battler& target, int value);

std::string GetSpRecoveredMessage(const Game_Battler& target, int value);

std::string GetParameterAbsorbedMessage(const Game_Battler& source, const Game_Battler& target, int value, StringView points);

std::string GetHpAbsorbedMessage(const Game_Battler& source, const Game_Battler& target, int value);

std::string GetSpAbsorbedMessage(const Game_Battler& source, const Game_Battler& target, int value);

std::string GetAtkAbsorbedMessage(const Game_Battler& source, const Game_Battler& target, int value);

std::string GetDefAbsorbedMessage(const Game_Battler& source, const Game_Battler& target, int value);

std::string GetSpiAbsorbedMessage(const Game_Battler& source, const Game_Battler& target, int value);

std::string GetAgiAbsorbedMessage(const Game_Battler& source, const Game_Battler& target, int value);

std::string GetDamagedMessage(const Game_Battler& target, int value);

std::string GetParameterChangeMessage(const Game_Battler& target, int value, StringView points);

std::string GetSpReduceMessage(const Game_Battler& target, int value);

std::string GetAtkChangeMessage(const Game_Battler& target, int value);

std::string GetDefChangeMessage(const Game_Battler& target, int value);

std::string GetSpiChangeMessage(const Game_Battler& target, int value);

std::string GetAgiChangeMessage(const Game_Battler& target, int value);

std::string GetAttributeShiftMessage(const Game_Battler& target, int value, const lcf::rpg::Attribute& attribute);

std::string GetNormalAttackStartMessage2k(const Game_Battler& source);

std::string GetDefendStartMessage2k(const Game_Battler& source);

std::string GetObserveStartMessage2k(const Game_Battler& source);

std::string GetChargeUpStartMessage2k(const Game_Battler& source);

std::string GetSelfDestructStartMessage2k(const Game_Battler& source);

std::string GetEscapeStartMessage2k(const Game_Battler& source);

std::string GetTransformStartMessage(const Game_Battler& source, const lcf::rpg::Enemy& new_enemy);

std::string GetSkillFirstStartMessage2k(const Game_Battler& source, const Game_Battler* target, const lcf::rpg::Skill& skill);

std::string GetSkillSecondStartMessage2k(const Game_Battler& source, const Game_Battler* target, const lcf::rpg::Skill& skill);

std::string GetItemStartMessage2k(const Game_Battler& source, const lcf::rpg::Item& item);

std::string GetDoubleAttackStartMessage2k3(const Game_Battler& source);

std::string GetSkillStartMessage2k3(const Game_Battler& source, const Game_Battler* target, const lcf::rpg::Skill& skill);

std::string GetItemStartMessage2k3(const Game_Battler& source, const lcf::rpg::Item& item);

std::string GetObserveStartMessage2k3(const Game_Battler& source);

std::string GetDefendStartMessage2k3(const Game_Battler& source);

std::string GetChargeUpStartMessage2k3(const Game_Battler& source);

std::string GetSelfDestructStartMessage2k3(const Game_Battler& source);

std::string GetEscapeStartMessage2k3(const Game_Battler& source);

} // namespace BattleMessage

#endif
