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

#ifndef EP_GAME_INTERPRETER_BATTLE_H
#define EP_GAME_INTERPRETER_BATTLE_H

// Headers
#include <map>
#include <string>
#include <vector>
#include <cassert>
#include "game_character.h"
#include <lcf/rpg/eventcommand.h>
#include <lcf/rpg/trooppagecondition.h>
#include "system.h"
#include "game_interpreter.h"

class Game_Event;
class Game_CommonEvent;

/**
 * Game_Interpreter_Battle class.
 */
class Game_Interpreter_Battle : public Game_Interpreter
{
public:
	explicit Game_Interpreter_Battle(Span<const lcf::rpg::TroopPage> pages);

	int GetNumPages() const;

	bool IsValidPage(int page_id) const;
	bool HasPageExecuted(int page_id) const;

	static bool AreConditionsMet(const lcf::rpg::TroopPageCondition& condition, Game_Battler* source);

	int ScheduleNextPage(Game_Battler* source);
	int ScheduleNextPage(lcf::rpg::TroopPageCondition::Flags required_conditions, Game_Battler* source);
	void ResetPagesExecuted();

	void SetCurrentEnemyTargetIndex(int idx);
	void SetCurrentActionTargetsSingleEnemy(bool value);
	void SetCurrentActingActorId(int id);

	bool IsForceFleeEnabled() const;

	bool ExecuteCommand() override;
private:
	bool CommandCallCommonEvent(lcf::rpg::EventCommand const& com);
	bool CommandForceFlee(lcf::rpg::EventCommand const& com);
	bool CommandEnableCombo(lcf::rpg::EventCommand const& com);
	bool CommandChangeMonsterHP(lcf::rpg::EventCommand const& com);
	bool CommandChangeMonsterMP(lcf::rpg::EventCommand const& com);
	bool CommandChangeMonsterCondition(lcf::rpg::EventCommand const& com);
	bool CommandShowHiddenMonster(lcf::rpg::EventCommand const& com);
	bool CommandChangeBattleBG(lcf::rpg::EventCommand const& com);
	bool CommandShowBattleAnimation(lcf::rpg::EventCommand const& com);
	bool CommandTerminateBattle(lcf::rpg::EventCommand const& com);
	bool CommandConditionalBranchBattle(lcf::rpg::EventCommand const& com);
	bool CommandElseBranchBattle(lcf::rpg::EventCommand const& com);
	bool CommandEndBranchBattle(lcf::rpg::EventCommand const& com);

	bool CommandManiacControlBattle(lcf::rpg::EventCommand const& com);
	bool CommandManiacControlAtbGauge(lcf::rpg::EventCommand const& com);
	bool CommandManiacChangeBattleCommandEx(lcf::rpg::EventCommand const& com);
	bool CommandManiacGetBattleInfo(lcf::rpg::EventCommand const& com);

private:
	Span<const lcf::rpg::TroopPage> pages;
	std::vector<bool> executed;
	int target_enemy_index = -1;
	int current_actor_id = 0;
	bool targets_single_enemy = false;
	bool force_flee_enabled = false;
};

inline int Game_Interpreter_Battle::GetNumPages() const {
	return static_cast<int>(pages.size());
}

inline bool Game_Interpreter_Battle::IsValidPage(int page_id) const {
	return page_id >= 1 && page_id <= GetNumPages();
}

inline bool Game_Interpreter_Battle::HasPageExecuted(int page_id) const {
	assert(IsValidPage(page_id));
	return executed[page_id - 1];
}

inline void Game_Interpreter_Battle::ResetPagesExecuted() {
	std::fill(executed.begin(), executed.end(), false);
}

inline void Game_Interpreter_Battle::SetCurrentEnemyTargetIndex(int idx) {
	target_enemy_index = idx;
}

inline void Game_Interpreter_Battle::SetCurrentActionTargetsSingleEnemy(bool value) {
	targets_single_enemy = value;
}

inline void Game_Interpreter_Battle::SetCurrentActingActorId(int id) {
	current_actor_id = id;
}

inline bool Game_Interpreter_Battle::IsForceFleeEnabled() const {
	return force_flee_enabled;
}

#endif
