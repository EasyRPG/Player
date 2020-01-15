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

#ifndef EP_GAME_INTERPRETER_MAP_H
#define EP_GAME_INTERPRETER_MAP_H

// Headers
#include <string>
#include <vector>
#include "game_character.h"
#include "rpg_eventcommand.h"
#include "rpg_saveeventexecstate.h"
#include "system.h"
#include "game_interpreter.h"

class Game_Event;
class Game_CommonEvent;

/**
 * Game_Interpreter_Map class
 */
class Game_Interpreter_Map : public Game_Interpreter
{
public:
	using Game_Interpreter::Game_Interpreter;

	/**
	 * Sets up the interpreter with given state.
	 *
	 * @param save event to load.
	 *
	 */
	void SetState(const RPG::SaveEventExecState& save);

	/**
	 * Called when we change maps.
	 */
	void OnMapChange();

	bool ExecuteCommand() override;

private:
	bool CommandRecallToLocation(RPG::EventCommand const& com);
	bool CommandEnemyEncounter(RPG::EventCommand const& com);
	bool CommandVictoryHandler(RPG::EventCommand const& com);
	bool CommandEscapeHandler(RPG::EventCommand const& com);
	bool CommandDefeatHandler(RPG::EventCommand const& com);
	bool CommandEndBattle(RPG::EventCommand const& com);
	bool CommandOpenShop(RPG::EventCommand const& com);
	bool CommandTransaction(RPG::EventCommand const& com);
	bool CommandNoTransaction(RPG::EventCommand const& com);
	bool CommandEndShop(RPG::EventCommand const& com);
	bool CommandShowInn(RPG::EventCommand const& com);
	bool CommandStay(RPG::EventCommand const& com);
	bool CommandNoStay(RPG::EventCommand const& com);
	bool CommandEndInn(RPG::EventCommand const& com);
	bool CommandEnterHeroName(RPG::EventCommand const& com);
	bool CommandTeleport(RPG::EventCommand const& com);
	bool CommandEnterExitVehicle(RPG::EventCommand const& com);
	bool CommandPanScreen(RPG::EventCommand const& com);
	bool CommandShowBattleAnimation(RPG::EventCommand const& com);
	bool CommandFlashSprite(RPG::EventCommand const& com);
	bool CommandProceedWithMovement(RPG::EventCommand const& com);
	bool CommandHaltAllMovement(RPG::EventCommand const& com);
	bool CommandPlayMovie(RPG::EventCommand const& com);
	bool CommandOpenSaveMenu(RPG::EventCommand const& com);
	bool CommandOpenMainMenu(RPG::EventCommand const& com);
	bool CommandOpenLoadMenu(RPG::EventCommand const& com);
	bool CommandToggleAtbMode(RPG::EventCommand const& com);
	bool CommandOpenVideoOptions(RPG::EventCommand const& com);

	void ContinuationShowInnStart(int indent, int choice_result, int price);
	bool ContinuationEnemyEncounter(RPG::EventCommand const& com) override;

	static std::vector<Game_Character*> pending;
};

#endif
