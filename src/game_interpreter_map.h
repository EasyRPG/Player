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

#ifndef _GAME_INTERPRETER_MAP_H_
#define _GAME_INTERPRETER_MAP_H_

// Headers
#include <string>
#include <vector>
#include "game_character.h"
#include "rpg_eventcommand.h"
#include "rpg_saveeventcommands.h"
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
	Game_Interpreter_Map(int _depth = 0, bool _main_flag = false);

	/**
	* Parses a SaveEventCommand to create an interpreter.
	*
	* @param save event to load.
	* @param index index in the event list.
	*
	* @return If the setup was successful (fails when index out of range)
	*/
	bool SetupFromSave(const std::vector<RPG::SaveEventCommands>& save, int index = 0);

	/**
	 * Generates a SaveEventCommands vector needed for the savefile.
	 *
	 * @return interpreter commands stored in SaveEventCommands
	 */
	std::vector<RPG::SaveEventCommands> GetSaveData() const;

	bool ExecuteCommand() override;

private:
	bool CommandRecallToLocation(RPG::EventCommand const& com);
	bool CommandEnemyEncounter(RPG::EventCommand const& com);
	bool CommandOpenShop(RPG::EventCommand const& com);
	bool CommandShowInn(RPG::EventCommand const& com);
	bool CommandEnterHeroName(RPG::EventCommand const& com);
	bool CommandTeleport(RPG::EventCommand const& com);
	bool CommandEnterExitVehicle(RPG::EventCommand const& com);
	bool CommandPanScreen(RPG::EventCommand const& com);
	bool CommandShowPicture(RPG::EventCommand const& com);
	bool CommandMovePicture(RPG::EventCommand const& com);
	bool CommandErasePicture(RPG::EventCommand const& com);
	bool CommandShowBattleAnimation(RPG::EventCommand const& com);
	bool CommandFlashSprite(RPG::EventCommand const& com);
	bool CommandProceedWithMovement(RPG::EventCommand const& com);
	bool CommandHaltAllMovement(RPG::EventCommand const& com);
	bool CommandPlayMovie(RPG::EventCommand const& com);
	bool CommandOpenSaveMenu(RPG::EventCommand const& com);
	bool CommandOpenMainMenu(RPG::EventCommand const& com);
	bool CommandOpenLoadMenu(RPG::EventCommand const& com);
	bool CommandToggleAtbMode(RPG::EventCommand const& com);

	bool ContinuationOpenShop(RPG::EventCommand const& com) override;
	bool ContinuationShowInnStart(RPG::EventCommand const& com) override;
	bool ContinuationShowInnContinue(RPG::EventCommand const& com);
	bool ContinuationShowInnFinish(RPG::EventCommand const& com) override;
	bool ContinuationEnemyEncounter(RPG::EventCommand const& com) override;

	static std::vector<Game_Character*> pending;
};

#endif
