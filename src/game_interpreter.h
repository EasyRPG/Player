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

#ifndef _GAME_INTERPRETER_H_
#define _GAME_INTERPRETER_H_

#include <map>
#include <string>
#include <vector>
#include "game_character.h"
#include "game_actor.h"
#include "rpg_eventcommand.h"
#include "system.h"
#include "command_codes.h"

class Game_Event;
class Game_CommonEvent;

////////////////////////////////////////////////////////////
/// Game_Interpreter class
////////////////////////////////////////////////////////////
class Game_Interpreter
{
public:
	Game_Interpreter(int _depth = 0, bool _main_flag = false);
	virtual ~Game_Interpreter();

	void Clear();
	void Setup(const std::vector<RPG::EventCommand>& _list, int _event_id, int dbg_x = -1, int dbg_y = -1);
	bool IsRunning() const;
	void Update();

	void SetupStartingEvent(Game_Event* ev);
	void SetupStartingEvent(Game_CommonEvent* ev);
	void InputButton();
	void SetupChoices(const std::vector<std::string>& choices);

	virtual bool ExecuteCommand();
	virtual void EndMoveRoute(RPG::MoveRoute* route);

	enum Sizes {
		MaxSize = 9999999,
		MinSize = -9999999
	};

protected:
	friend class Game_Interpreter_Map;

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
	bool (Game_Interpreter::*continuation)();

	std::vector<RPG::EventCommand> list;

	int button_timer;
	bool active;

	// Helper function
	void GetStrings(std::vector<std::string>& ret_val);

	int OperateValue(int operation, int operand_type, int operand);
	Game_Character* GetCharacter(int character_id);

	bool SkipTo(int code, int code2 = -1, int min_indent = -1, int max_indent = -1);
	void SetContinuation(bool (Game_Interpreter::*func)());

	void CancelMenuCall();

	static std::vector<Game_Actor*> GetActors(int mode, int id);
	static int ValueOrVariable(int mode, int val);

	////////////////////////////////////////////////////////
	/// Closes the Message Window
	////////////////////////////////////////////////////////
	void CloseMessageWindow();

	template<Cmd::CommandCodes Code>
	bool Command(RPG::EventCommand const& com);

	typedef bool (Game_Interpreter::*CommandPointer)(RPG::EventCommand const&);
	class CodeTable {
	private:
		static std::map<unsigned, CommandPointer> code2func_;
		static std::map<unsigned, std::string> code2str_;
		static std::map<std::string, unsigned> str2code_;

		CodeTable(CodeTable const&);
	public:
		CodeTable();

		static std::string const& ToString(unsigned code);
		static Cmd::CommandCodes ToCode(char const* str);

		static CommandPointer GetCommand(unsigned code);
		// static void Erase(Cmd::CommandCodes code);
	} codeTable_;

	virtual bool DefaultContinuation();
	virtual bool ContinuationChoices();
	virtual bool ContinuationOpenShop();
	virtual bool ContinuationShowInn();
	virtual bool ContinuationEnemyEncounter();

	int debug_x;
	int debug_y;

	bool teleport_pending;
};

#define PP_enum(VAL, CODE) template<> bool Game_Interpreter::Command<Cmd::VAL>(RPG::EventCommand const& com);
PP_rpgEnumCommandCode(PP_enum)
#undef PP_enum

#endif
