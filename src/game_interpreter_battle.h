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

	bool IsValidPage(int page_id) const;

	bool HasPageExecuted(int page_id) const;
	void SetHasPageExecuted(int page_id, bool value);

	bool CanPageRun(int page_id) const;
	void SetCanPageRun(int page_id, bool value);

	void ResetAllPagesExecuted();
	int GetNumPages() const;

	static bool AreConditionsMet(const lcf::rpg::TroopPageCondition& condition);
	void ResetPagesExecuted(const Game_Battler* Battler);

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
private:
	Span<const lcf::rpg::TroopPage> pages;
	std::vector<bool> pages_state;
};

inline int Game_Interpreter_Battle::GetNumPages() const {
	return static_cast<int>(pages_state.size() / 2);
}

inline bool Game_Interpreter_Battle::IsValidPage(int page_id) const {
	return page_id >= 1 && page_id <= GetNumPages();
}

inline bool Game_Interpreter_Battle::HasPageExecuted(int page_id) const {
	assert(IsValidPage(page_id));
	return pages_state[(page_id - 1) * 2];
}

inline void Game_Interpreter_Battle::SetHasPageExecuted(int page_id, bool value) {
	assert(IsValidPage(page_id));
	pages_state[(page_id - 1) * 2] = value;
}

inline bool Game_Interpreter_Battle::CanPageRun(int page_id) const {
	assert(IsValidPage(page_id));
	return pages_state[(page_id - 1) * 2 + 1];
}

inline void Game_Interpreter_Battle::SetCanPageRun(int page_id, bool value) {
	assert(IsValidPage(page_id));
	pages_state[(page_id - 1) * 2 + 1] = value;
}

inline void Game_Interpreter_Battle::ResetAllPagesExecuted() {
	for (int i = 0; i < GetNumPages(); ++i) {
		SetHasPageExecuted(i + 1, false);
	}
}

#endif
