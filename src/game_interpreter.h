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

#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include <map>
#include <string>
#include <vector>
#include "game_character.h"
#include "rpg_eventcommand.h"
#include "system.h"

////////////////////////////////////////////////////////////
/// Game_Interpreter class
////////////////////////////////////////////////////////////
class Game_Interpreter
{
public:
	Game_Interpreter(int _depth = 0, bool _main_flag = false);
	~Game_Interpreter();

	void Clear();
	void Setup(std::vector<RPG::EventCommand>& _list, int _event_id);
	bool IsRunning() const;
	void Update();

	void SetupStartingEvent();
	bool ExecuteCommand();
	void InputButton();
	void SetupChoices(const std::vector<std::string>& choices);

private:
	int depth;
	bool main_flag;

	int loop_count;

	bool move_route_waiting;

	int button_input_variable_id;
	unsigned int index;
	int map_id;
	unsigned int event_id;
	int wait_count;

	Game_Interpreter* child_interpreter;

	std::vector<RPG::EventCommand> list;
	/// Contains the results of branches
	/// Note: Difference to RGSS:
	/// RGSS uses bool for this and uses a ruby feature to get three states:
	/// true, false and (if element is missing) nil.
	/// This is not possible under C++, the mapping is: -1 false, 0 nil, 1 true
	std::map<int, int8> branch;

	// Helper function
	void GetStrings(std::vector<std::string>& ret_val);

	int OperateValue(int operation, int operand_type, int operand);
	Game_Character* GetCharacter(int character_id);

	void CancelMenuCall();

	bool CommandShowMessage();
	bool CommandShowChoices();
	bool CommandChangeFaceGraphic();
	bool CommandInputNumber();
	bool CommandMessageOptions();
	bool CommandControlSwitches();
	bool CommandControlVariables();
	bool CommandChangeGold();
	bool CommandChangeItems();
	bool CommandChangePartyMember();
	bool CommandConditionalBranch();
	bool CommandElseBranch();
	bool CommandChangeExp();
	bool CommandChangeLevel();
	bool CommandChangeParameters();
	bool CommandChangeSkills();
	bool CommandChangeEquipment();
	bool CommandChangeHP();
	bool CommandChangeSP();
	bool CommandChangeCondition();
	bool CommandFullHeal();
	bool CommandChangeHeroName();
	bool CommandChangeHeroTitle();
	bool CommandChangeSpriteAssociation();
	bool CommandMemorizeLocation();
	bool CommandRecallToLocation();
	bool CommandStoreTerrainID();
	bool CommandStoreEventID();
	bool CommandPlayBGM();
	bool CommandFadeOutBGM();
	bool CommandMemorizeBGM();
	bool CommandPlayMemorizedBGM();
	bool CommandPlaySound();
	bool CommandChangeSystemBGM();
	bool CommandChangeSystemSFX();
	bool CommandChangeSaveAccess();
	bool CommandChangeTeleportAccess();
	bool CommandChangeEscapeAccess();
	bool CommandChangeMainMenuAccess();
	bool CommandChangeActorFace();
	bool CommandSkip();
	bool CommandWait();
	bool CommandTeleport();
	bool CommandEraseScreen();
	bool CommandShowScreen();
	bool CommandShowPicture();
	bool CommandMovePicture();
	bool CommandErasePicture();
	bool CommandTintScreen();
	bool CommandFlashScreen();
	bool CommandShakeScreen();
	bool CommandWeatherEffects();
	bool CommandChangeSystemGraphics();
	bool CommandChangeEventLocation();
	bool CommandTradeEventLocations();
	bool CommandTimerOperation();

	void CommandEnd();
};

#endif
