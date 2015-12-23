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
	bool SetupFromSave(const std::vector<RPG::SaveEventCommands>& save, int _event_id, int index = 0);

	/**
	 * Generates a SaveEventCommands vector needed for the savefile.
	 *
	 * @return interpreter commands stored in SaveEventCommands
	 */
	std::vector<RPG::SaveEventCommands> GetSaveData() const;

	bool ExecuteCommand();

private:
	bool CommandMessageOptions(RPG::EventCommand const& com);
	bool CommandChangeExp(RPG::EventCommand const& com);
	bool CommandChangeParameters(RPG::EventCommand const& com);
	bool CommandChangeHeroName(RPG::EventCommand const& com);
	bool CommandChangeHeroTitle(RPG::EventCommand const& com);
	bool CommandChangeSpriteAssociation(RPG::EventCommand const& com);
	bool CommandMemorizeLocation(RPG::EventCommand const& com);
	bool CommandRecallToLocation(RPG::EventCommand const& com);
	bool CommandStoreTerrainID(RPG::EventCommand const& com);
	bool CommandStoreEventID(RPG::EventCommand const& com);
	bool CommandMemorizeBGM(RPG::EventCommand const& com);
	bool CommandPlayMemorizedBGM(RPG::EventCommand const& com);
	bool CommandChangeSystemBGM(RPG::EventCommand const& com);
	bool CommandChangeSystemSFX(RPG::EventCommand const& com);
	bool CommandChangeSaveAccess(RPG::EventCommand const& com);
	bool CommandChangeTeleportAccess(RPG::EventCommand const& com);
	bool CommandChangeEscapeAccess(RPG::EventCommand const& com);
	bool CommandChangeMainMenuAccess(RPG::EventCommand const& com);
	bool CommandChangeActorFace(RPG::EventCommand const& com);
	bool CommandTeleport(RPG::EventCommand const& com);
	bool CommandEraseScreen(RPG::EventCommand const& com);
	bool CommandShowScreen(RPG::EventCommand const& com);
	bool CommandShowPicture(RPG::EventCommand const& com);
	bool CommandMovePicture(RPG::EventCommand const& com);
	bool CommandErasePicture(RPG::EventCommand const& com);
	bool CommandWeatherEffects(RPG::EventCommand const& com);
	bool CommandChangeSystemGraphics(RPG::EventCommand const& com);
	bool CommandChangeScreenTransitions(RPG::EventCommand const& com);
	bool CommandChangeEventLocation(RPG::EventCommand const& com);
	bool CommandTradeEventLocations(RPG::EventCommand const& com);
	bool CommandTimerOperation(RPG::EventCommand const& com);
	bool CommandChangePBG(RPG::EventCommand const& com);
	bool CommandJumpToLabel(RPG::EventCommand const& com);
	bool CommandBreakLoop(RPG::EventCommand const& com);
	bool CommandEndLoop(RPG::EventCommand const& com);
	bool CommandOpenShop(RPG::EventCommand const& com);
	bool CommandShowInn(RPG::EventCommand const& com);
	bool CommandEnterHeroName(RPG::EventCommand const& com);
	bool CommandReturnToTitleScreen(RPG::EventCommand const& com);
	bool CommandOpenSaveMenu(RPG::EventCommand const& com);
	bool CommandOpenMainMenu(RPG::EventCommand const& com);
	bool CommandEnemyEncounter(RPG::EventCommand const& com);
	bool CommandTeleportTargets(RPG::EventCommand const& com);
	bool CommandEscapeTarget(RPG::EventCommand const& com);
	bool CommandMoveEvent(RPG::EventCommand const& com);
	bool CommandFlashSprite(RPG::EventCommand const& com);
	bool CommandSpriteTransparency(RPG::EventCommand const& com);
	bool CommandEraseEvent(RPG::EventCommand const& com);
	bool CommandChangeMapTileset(RPG::EventCommand const& com);
	bool CommandCallEvent(RPG::EventCommand const& com);
	bool CommandChangeEncounterRate(RPG::EventCommand const& com);
	bool CommandProceedWithMovement(RPG::EventCommand const& com);
	bool CommandPlayMovie(RPG::EventCommand const& com);
	bool CommandChangeBattleCommands(RPG::EventCommand const& com);
	bool CommandKeyInputProc(RPG::EventCommand const& com);
	bool CommandChangeVehicleGraphic(RPG::EventCommand const& com);
	bool CommandEnterExitVehicle(RPG::EventCommand const& com);
	bool CommandSetVehicleLocation(RPG::EventCommand const& com);
	bool CommandTileSubstitution(RPG::EventCommand const& com);
	bool CommandPanScreen(RPG::EventCommand const& com);
	bool CommandSimulatedAttack(RPG::EventCommand const& com);
	bool CommandConditionalBranch(RPG::EventCommand const& com);
	bool CommandShowBattleAnimation(RPG::EventCommand const& com);
	bool CommandChangeClass(RPG::EventCommand const& com);
	bool CommandHaltAllMovement(RPG::EventCommand const& com);
	bool CommandOpenLoadMenu(RPG::EventCommand const& com);
	bool CommandExitGame(RPG::EventCommand const& com);
	bool CommandToggleAtbMode(RPG::EventCommand const& com);
	bool CommandToggleFullscreen(RPG::EventCommand const& com);

	bool ContinuationOpenShop(RPG::EventCommand const& com);
	bool ContinuationShowInnStart(RPG::EventCommand const& com);
	bool ContinuationShowInnContinue(RPG::EventCommand const& com);
	bool ContinuationShowInnFinish(RPG::EventCommand const& com);
	bool ContinuationEnemyEncounter(RPG::EventCommand const& com);

private:
	void OnChangeSystemGraphicReady(FileRequestResult* result);
	int DecodeInt(std::vector<int>::const_iterator& it);
	const std::string DecodeString(std::vector<int>::const_iterator& it);
	RPG::MoveCommand DecodeMove(std::vector<int>::const_iterator& it);

	static std::vector<Game_Character*> pending;
};

#endif
