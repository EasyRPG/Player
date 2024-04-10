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
#include <lcf/rpg/eventcommand.h>
#include <lcf/rpg/saveeventexecstate.h>
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
	void SetState(const lcf::rpg::SaveEventExecState& save);

	/**
	 * Called when we change maps.
	 */
	void OnMapChange();

	bool ExecuteCommand(lcf::rpg::EventCommand const& com) override;

private:
	bool CommandRecallToLocation(lcf::rpg::EventCommand const& com);
	bool CommandEnemyEncounter(lcf::rpg::EventCommand const& com);
	bool CommandVictoryHandler(lcf::rpg::EventCommand const& com);
	bool CommandEscapeHandler(lcf::rpg::EventCommand const& com);
	bool CommandDefeatHandler(lcf::rpg::EventCommand const& com);
	bool CommandEndBattle(lcf::rpg::EventCommand const& com);
	bool CommandOpenShop(lcf::rpg::EventCommand const& com);
	bool CommandTransaction(lcf::rpg::EventCommand const& com);
	bool CommandNoTransaction(lcf::rpg::EventCommand const& com);
	bool CommandEndShop(lcf::rpg::EventCommand const& com);
	bool CommandShowInn(lcf::rpg::EventCommand const& com);
	bool CommandStay(lcf::rpg::EventCommand const& com);
	bool CommandNoStay(lcf::rpg::EventCommand const& com);
	bool CommandEndInn(lcf::rpg::EventCommand const& com);
	bool CommandEnterHeroName(lcf::rpg::EventCommand const& com);
	bool CommandTeleport(lcf::rpg::EventCommand const& com);
	bool CommandEnterExitVehicle(lcf::rpg::EventCommand const& com);
	bool CommandPanScreen(lcf::rpg::EventCommand const& com);
	bool CommandShowBattleAnimation(lcf::rpg::EventCommand const& com);
	bool CommandFlashSprite(lcf::rpg::EventCommand const& com);
	bool CommandProceedWithMovement(lcf::rpg::EventCommand const& com);
	bool CommandHaltAllMovement(lcf::rpg::EventCommand const& com);
	bool CommandPlayMovie(lcf::rpg::EventCommand const& com);
	bool CommandOpenSaveMenu(lcf::rpg::EventCommand const& com);
	bool CommandOpenMainMenu(lcf::rpg::EventCommand const& com);
	bool CommandOpenLoadMenu(lcf::rpg::EventCommand const& com);
	bool CommandToggleAtbMode(lcf::rpg::EventCommand const& com);

	bool CommandEasyRpgTriggerEventAt(lcf::rpg::EventCommand const& com);

	AsyncOp ContinuationShowInnStart(int indent, int choice_result, int price);

	static std::vector<Game_Character*> pending;
};

#endif
