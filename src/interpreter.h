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
#include <vector>
#include "rpg_eventcommand.h"

////////////////////////////////////////////////////////////
/// Interpreter class
////////////////////////////////////////////////////////////
class Interpreter
{
public:
	Interpreter(int _depth = 0, bool _main_flag = false);
	~Interpreter();

	void Clear();
	void Setup(std::vector<RPG::EventCommand> _list, int _event_id);
	bool IsRunning();
	void Update();

	void SetupStartingEvent();
	bool ExecuteCommand();
	void InputButton();
	void SetupChoices(const std::vector<std::string>& choices);

	bool CommandShowMessage();
	bool CommandShowChoices();
	bool CommandControlSwitches();
	bool CommandControlVariables();

private:
	int depth;
	bool main_flag;

	int loop_count;

	// May change this for a custom variable in game_temp
	bool message_waiting;

	bool move_route_waiting;

	int button_input_variable_id;
	unsigned int index;
	unsigned int map_id;
	unsigned int event_id;
	int wait_count;

	Interpreter* child_interpreter;

	std::vector<RPG::EventCommand> list;
	std::map<int, bool> branch;

	// Helper function
	std::vector<std::string> GetStrings();
};

#endif
