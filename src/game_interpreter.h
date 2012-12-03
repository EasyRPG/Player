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
#include <boost/scoped_ptr.hpp>

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

	boost::scoped_ptr<Game_Interpreter> child_interpreter;
	typedef bool (Game_Interpreter::*ContinuationFunction)(RPG::EventCommand const& com);
	ContinuationFunction continuation;

	std::vector<RPG::EventCommand> list;

	int button_timer;
	bool active;

	// Helper function
	void GetStrings(std::vector<std::string>& ret_val);

	int OperateValue(int operation, int operand_type, int operand);
	Game_Character* GetCharacter(int character_id);

	bool SkipTo(int code, int code2 = -1, int min_indent = -1, int max_indent = -1);
	void SetContinuation(ContinuationFunction func);

	void CancelMenuCall();

	static std::vector<Game_Actor*> GetActors(int mode, int id);
	static int ValueOrVariable(int mode, int val);

	////////////////////////////////////////////////////////
	/// Closes the Message Window
	////////////////////////////////////////////////////////
	void CloseMessageWindow();

	bool CommandShowMessage(RPG::EventCommand const& com);
	bool CommandChangeFaceGraphic(RPG::EventCommand const& com);
	bool CommandShowChoices(RPG::EventCommand const& com);
	bool CommandInputNumber(RPG::EventCommand const& com);
	bool CommandControlSwitches(RPG::EventCommand const& com);
	bool CommandControlVariables(RPG::EventCommand const& com);
	bool CommandChangeGold(RPG::EventCommand const& com);
	bool CommandChangeItems(RPG::EventCommand const& com);
	bool CommandChangePartyMember(RPG::EventCommand const& com);
	bool CommandChangeLevel(RPG::EventCommand const& com);
	bool CommandChangeSkills(RPG::EventCommand const& com);
	bool CommandChangeEquipment(RPG::EventCommand const& com);
	bool CommandChangeHP(RPG::EventCommand const& com);
	bool CommandChangeSP(RPG::EventCommand const& com);
	bool CommandChangeCondition(RPG::EventCommand const& com);
	bool CommandFullHeal(RPG::EventCommand const& com);
	bool CommandTintScreen(RPG::EventCommand const& com);
	bool CommandFlashScreen(RPG::EventCommand const& com);
	bool CommandShakeScreen(RPG::EventCommand const& com);
	bool CommandWait(RPG::EventCommand const& com);
	bool CommandPlayBGM(RPG::EventCommand const& com);
	bool CommandFadeOutBGM(RPG::EventCommand const& com);
	bool CommandPlaySound(RPG::EventCommand const& com);
	bool CommandEndEventProcessing(RPG::EventCommand const& com);
	bool CommandGameOver(RPG::EventCommand const& com);

	bool CommandEnd();

	virtual bool DefaultContinuation(RPG::EventCommand const& com);
	virtual bool ContinuationChoices(RPG::EventCommand const& com);
	virtual bool ContinuationOpenShop(RPG::EventCommand const& com);
	virtual bool ContinuationShowInn(RPG::EventCommand const& com);
	virtual bool ContinuationEnemyEncounter(RPG::EventCommand const& com);

	int debug_x;
	int debug_y;

	bool teleport_pending;
};

#endif
