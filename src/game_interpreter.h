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

#ifndef _H_INTERPRETER
#define _H_INTERPRETER

#include <map>
#include <string>
#include <vector>
#include "game_character.h"
#include "rpg_eventcommand.h"

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
	bool IsRunning();
	void Update();

	void SetupStartingEvent();
	bool ExecuteCommand();
	void InputButton();
	void SetupChoices(const std::vector<std::string>& choices);

private:
	int depth;
	bool main_flag;

	int loop_count;

	// May change this for a custom variable in game_temp
	bool message_waiting;

	bool move_route_waiting;

	int button_input_variable_id;
	unsigned int index;
	int map_id;
	unsigned int event_id;
	int wait_count;

	Game_Interpreter* child_interpreter;

	std::vector<RPG::EventCommand> list;
	std::map<int, bool> branch;

	// Helper function
	std::vector<std::string> GetStrings();

	int OperateValue(int operation, int operand_type, int operand);
	Game_Character* GetCharacter(int character_id);

	void CancelMenuCall();

	bool CommandShowMessage();
	bool CommandShowChoices();
	bool CommandChangeFaceGraphic();
	bool CommandInputNumber();
	bool CommandControlSwitches();
	bool CommandControlVariables();
	bool CommandChangeGold();
	bool CommandChangeItems();
	bool CommandChangePartyMember();
	bool CommandConditionalBranch();
	bool CommandChangeExp();
	bool CommandChangeLevel();
	bool CommandChangeParameters();
	bool CommandSkip();
	bool CommandWait();

	void CommandEnd();
};

#endif
