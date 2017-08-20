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
#include "game_character.h"
#include "rpg_eventcommand.h"
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
	Game_Interpreter_Battle(int _depth = 0, bool _main_flag = false);

	bool ExecuteCommand() override;
private:
	bool CommandCallCommonEvent(RPG::EventCommand const& com);
	bool CommandForceFlee(RPG::EventCommand const& com);
	bool CommandEnableCombo(RPG::EventCommand const& com);
	bool CommandChangeMonsterHP(RPG::EventCommand const& com);
	bool CommandChangeMonsterMP(RPG::EventCommand const& com);
	bool CommandChangeMonsterCondition(RPG::EventCommand const& com);
	bool CommandShowHiddenMonster(RPG::EventCommand const& com);
	bool CommandChangeBattleBG(RPG::EventCommand const& com);
	bool CommandShowBattleAnimation(RPG::EventCommand const& com);
	bool CommandTerminateBattle(RPG::EventCommand const& com);
	bool CommandConditionalBranchBattle(RPG::EventCommand const& com);
};

#endif
