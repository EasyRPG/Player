/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#ifndef _GAME_INTERPRETER_MAP_H_
#define _GAME_INTERPRETER_MAP_H_

#include <map>
#include <string>
#include <vector>
#include "game_character.h"
#include "rpg_eventcommand.h"
#include "system.h"
#include "game_interpreter.h"

class Game_Event;
class Game_CommonEvent;

////////////////////////////////////////////////////////////
/// Game_Interpreter_Map class
////////////////////////////////////////////////////////////
class Game_Interpreter_Map : public Game_Interpreter
{
	friend class Game_Interpreter;
public:
	Game_Interpreter_Map(int _depth = 0, bool _main_flag = false);
	~Game_Interpreter_Map();

	bool ExecuteCommand();

	void EndMoveRoute(RPG::MoveRoute* route);

private:
	/*
	bool CommandMessageOptions();
	bool CommandChangeExp();
	bool CommandChangeParameters();
	bool CommandChangeHeroName();
	bool CommandChangeHeroTitle();
	bool CommandChangeSpriteAssociation();
	bool CommandMemorizeLocation();
	bool CommandRecallToLocation();
	bool CommandStoreTerrainID();
	bool CommandStoreEventID();
	bool CommandMemorizeBGM();
	bool CommandPlayMemorizedBGM();
	bool CommandChangeSystemBGM();
	bool CommandChangeSystemSFX();
	bool CommandChangeSaveAccess();
	bool CommandChangeTeleportAccess();
	bool CommandChangeEscapeAccess();
	bool CommandChangeMainMenuAccess();
	bool CommandChangeActorFace();
	bool CommandTeleport();
	bool CommandEraseScreen();
	bool CommandShowScreen();
	bool CommandShowPicture();
	bool CommandMovePicture();
	bool CommandErasePicture();
	bool CommandWeatherEffects();
	bool CommandChangeSystemGraphics();
	bool CommandChangeScreenTransitions();
	bool CommandChangeEventLocation();
	bool CommandTradeEventLocations();
	bool CommandTimerOperation();
	bool CommandChangePBG();
	bool CommandJumpToLabel();
	bool CommandBreakLoop();
	bool CommandEndLoop();
	bool CommandOpenShop();
	bool CommandShowInn();
	bool CommandEnterHeroName();
	bool CommandReturnToTitleScreen();
	bool CommandOpenSaveMenu();
	bool CommandOpenMainMenu();
	bool CommandEnemyEncounter();
	bool CommandTeleportTargets();
	bool CommandEscapeTarget();
	bool CommandMoveEvent();
	bool CommandFlashSprite();
	bool CommandSpriteTransparency();
	bool CommandEraseEvent();
	bool CommandChangeMapTileset();
	bool CommandCallEvent();
	bool CommandChangeEncounterRate();
	bool CommandProceedWithMovement();
	bool CommandPlayMovie();
	bool CommandChangeBattleCommands();
	bool CommandKeyInputProc();
	bool CommandChangeVehicleGraphic();
	bool CommandEnterExitVehicle();
	bool CommandSetVehicleLocation();
	bool CommandTileSubstitution();
	bool CommandPanScreen();
	bool CommandSimulatedAttack();
	bool CommandConditionalBranch();
	bool CommandShowBattleAnimation();
	bool CommandChangeClass();
	bool CommandHaltAllMovement();
	*/

	bool ContinuationOpenShop();
	bool ContinuationShowInn();
	bool ContinuationEnemyEncounter();

private:

	int DecodeInt(std::vector<int>::const_iterator& it);
	const std::string DecodeString(std::vector<int>::const_iterator& it);
	RPG::MoveCommand DecodeMove(std::vector<int>::const_iterator& it);

	typedef std::pair<RPG::MoveRoute*,Game_Character*> pending_move_route;
	std::vector<pending_move_route> pending;
};

#endif

