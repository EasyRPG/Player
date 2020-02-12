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

#ifndef EP_GAME_INTERPRETER_H
#define EP_GAME_INTERPRETER_H

#include <map>
#include <string>
#include <vector>
#include "async_handler.h"
#include "game_character.h"
#include "game_actor.h"
#include "rpg_eventcommand.h"
#include "system.h"
#include "command_codes.h"
#include "rpg_saveeventexecstate.h"
#include "flag_set.h"
#include "async_op.h"

class Game_Event;
class Game_CommonEvent;
class PendingMessage;

namespace RPG {
	class EventPage;
}

/**
 * Game_Interpreter class
 */
class Game_Interpreter
{
public:
	static Game_Interpreter& GetForegroundInterpreter();

	Game_Interpreter(bool _main_flag = false);
#ifndef EMSCRIPTEN
	// No idea why but emscripten will complain about a missing destructor when
	// using virtual here
	virtual
#endif
	~Game_Interpreter();

	void Clear();

	bool IsRunning() const;
	int GetLoopCount() const;
	bool ReachedLoopLimit() const;

	void Update(bool reset_loop_count=true);

	void Push(
			const std::vector<RPG::EventCommand>& _list,
			int _event_id,
			bool started_by_decision_key = false
	);
	void Push(Game_Event* ev);
	void Push(Game_CommonEvent* ev);

	void InputButton();
	void SetupChoices(const std::vector<std::string>& choices, int indent, PendingMessage& pm);

	virtual bool ExecuteCommand();


	/**
	 * Returns a SaveEventExecState needed for the savefile.
	 *
	 * @return interpreter commands stored in SaveEventCommands
	 */
	RPG::SaveEventExecState GetState() const;

	/** @return the event_id of the current frame */
	int GetCurrentEventId() const;

	/** @return the event_id used by "ThisEvent" in commands */
	int GetThisEventId() const;

	/** @return the event_id of the event at the base of the call stack */
	int GetOriginalEventId() const;

	/** Return true if the interpreter is waiting for an async operation and needs to be resumed */
	bool IsAsyncPending();

	/** Return true if the interpreter is waiting for an async operation and needs to be resumed */
	AsyncOp GetAsyncOp() const;

protected:
	static constexpr int loop_limit = 10000;
	static constexpr int call_stack_limit = 1000;
	static constexpr int subcommand_sentinel = 255;

	const RPG::SaveEventExecFrame* GetFrame() const;
	RPG::SaveEventExecFrame* GetFrame();

	bool main_flag;

	int loop_count = 0;

	/**
	 * Gets strings for choice selection.
	 * This is just a helper (private) method
	 * to avoid repeating code.
	 *
	 * @param max_num_choices The maximum number of choices to allow. Any choice with id >= this value (cancel handler) is ignored.
	 */
	std::vector<std::string> GetChoices(int max_num_choices);

	/**
	 * Calculates operated value.
	 *
	 * @param operation operation (increase: 0, decrease: 1).
	 * @param operand_type operand type (0: set, 1: variable).
	 * @param operand operand (number or var ID).
	 */
	int OperateValue(int operation, int operand_type, int operand);
	Game_Character* GetCharacter(int character_id) const;

	/**
	 * Skips to the next option in a chain of conditional commands.
	 * Works by skipping until we hit the end or the next command
	 * with com.indent <= indent.
	 * The <= protects against broken game code which terminates without
	 * a proper conditional.
	 *
	 * @param codes which codes to check.
	 * @param indent the indentation level to check
	 */
	void SkipToNextConditional(std::initializer_list<int> codes, int indent);

	/**
	 * Sets up a wait (and closes the message box)
	 */
	void SetupWait(int duration);

	/**
	 * Calculates list of actors.
	 *
	 * @param mode 0: party, 1: specific actor, 2: actor referenced by variable.
	 * @param id actor ID (mode = 1) or variable ID (mode = 2).
	 */
	static std::vector<Game_Actor*> GetActors(int mode, int id);
	static int ValueOrVariable(int mode, int val);

	/**
	 * When current frame finishes executing we pop the stack
	 */
	bool OnFinishStackFrame();

	/**
	 * Triggers a game over when all party members are dead.
	 *
	 * @return true if game over was started.
	 */
	bool CheckGameOver();

	bool CommandOptionGeneric(RPG::EventCommand const& com, int option_sub_idx, std::initializer_list<int> next);

	bool CommandShowMessage(RPG::EventCommand const& com);
	bool CommandMessageOptions(RPG::EventCommand const& com);
	bool CommandChangeFaceGraphic(RPG::EventCommand const& com);
	bool CommandShowChoices(RPG::EventCommand const& com);
	bool CommandShowChoiceOption(RPG::EventCommand const& com);
	bool CommandShowChoiceEnd(RPG::EventCommand const& com);
	bool CommandInputNumber(RPG::EventCommand const& com);
	bool CommandControlSwitches(RPG::EventCommand const& com);
	bool CommandControlVariables(RPG::EventCommand const& com);
	bool CommandTimerOperation(RPG::EventCommand const& com);
	bool CommandChangeGold(RPG::EventCommand const& com);
	bool CommandChangeItems(RPG::EventCommand const& com);
	bool CommandChangePartyMember(RPG::EventCommand const& com);
	bool CommandChangeExp(RPG::EventCommand const& com);
	bool CommandChangeLevel(RPG::EventCommand const& com);
	bool CommandChangeParameters(RPG::EventCommand const& com);
	bool CommandChangeSkills(RPG::EventCommand const& com);
	bool CommandChangeEquipment(RPG::EventCommand const& com);
	bool CommandChangeHP(RPG::EventCommand const& com);
	bool CommandChangeSP(RPG::EventCommand const& com);
	bool CommandChangeCondition(RPG::EventCommand const& com);
	bool CommandFullHeal(RPG::EventCommand const& com);
	bool CommandSimulatedAttack(RPG::EventCommand const& com);
	bool CommandWait(RPG::EventCommand const& com);
	bool CommandPlayBGM(RPG::EventCommand const& com);
	bool CommandFadeOutBGM(RPG::EventCommand const& com);
	bool CommandPlaySound(RPG::EventCommand const& com);
	bool CommandEndEventProcessing(RPG::EventCommand const& com);
	bool CommandGameOver(RPG::EventCommand const& com);
	bool CommandChangeHeroName(RPG::EventCommand const& com);
	bool CommandChangeHeroTitle(RPG::EventCommand const& com);
	bool CommandChangeSpriteAssociation(RPG::EventCommand const& com);
	bool CommandChangeActorFace(RPG::EventCommand const& com);
	bool CommandChangeVehicleGraphic(RPG::EventCommand const& com);
	bool CommandChangeSystemBGM(RPG::EventCommand const& com);
	bool CommandChangeSystemSFX(RPG::EventCommand const& com);
	bool CommandChangeSystemGraphics(RPG::EventCommand const& com);
	bool CommandChangeScreenTransitions(RPG::EventCommand const& com);
	bool CommandMemorizeLocation(RPG::EventCommand const& com);
	bool CommandSetVehicleLocation(RPG::EventCommand const& com);
	bool CommandChangeEventLocation(RPG::EventCommand const& com);
	bool CommandTradeEventLocations(RPG::EventCommand const& com);
	bool CommandStoreTerrainID(RPG::EventCommand const& com);
	bool CommandStoreEventID(RPG::EventCommand const& com);
	bool CommandEraseScreen(RPG::EventCommand const& com);
	bool CommandShowScreen(RPG::EventCommand const& com);
	bool CommandTintScreen(RPG::EventCommand const& com);
	bool CommandFlashScreen(RPG::EventCommand const& com);
	bool CommandShakeScreen(RPG::EventCommand const& com);
	bool CommandWeatherEffects(RPG::EventCommand const& com);
	bool CommandShowPicture(RPG::EventCommand const& com);
	bool CommandMovePicture(RPG::EventCommand const& com);
	bool CommandErasePicture(RPG::EventCommand const& com);
	bool CommandSpriteTransparency(RPG::EventCommand const& com);
	bool CommandMoveEvent(RPG::EventCommand const& com);
	bool CommandMemorizeBGM(RPG::EventCommand const& com);
	bool CommandPlayMemorizedBGM(RPG::EventCommand const& com);
	bool CommandKeyInputProc(RPG::EventCommand const& com);
	bool CommandChangeMapTileset(RPG::EventCommand const& com);
	bool CommandChangePBG(RPG::EventCommand const& com);
	bool CommandChangeEncounterRate(RPG::EventCommand const& com);
	bool CommandTileSubstitution(RPG::EventCommand const& com);
	bool CommandTeleportTargets(RPG::EventCommand const& com);
	bool CommandChangeTeleportAccess(RPG::EventCommand const& com);
	bool CommandEscapeTarget(RPG::EventCommand const& com);
	bool CommandChangeEscapeAccess(RPG::EventCommand const& com);
	bool CommandChangeSaveAccess(RPG::EventCommand const& com);
	bool CommandChangeMainMenuAccess(RPG::EventCommand const& com);
	bool CommandConditionalBranch(RPG::EventCommand const& com);
	bool CommandElseBranch(RPG::EventCommand const& com);
	bool CommandEndBranch(RPG::EventCommand const& com);
	bool CommandJumpToLabel(RPG::EventCommand const& com);
	bool CommandBreakLoop(RPG::EventCommand const& com);
	bool CommandEndLoop(RPG::EventCommand const& com);
	bool CommandEraseEvent(RPG::EventCommand const& com);
	bool CommandCallEvent(RPG::EventCommand const& com);
	bool CommandReturnToTitleScreen(RPG::EventCommand const& com);
	bool CommandChangeClass(RPG::EventCommand const& com);
	bool CommandChangeBattleCommands(RPG::EventCommand const& com);
	bool CommandExitGame(RPG::EventCommand const& com);
	bool CommandToggleFullscreen(RPG::EventCommand const& com);

	int DecodeInt(std::vector<int32_t>::const_iterator& it);
	const std::string DecodeString(std::vector<int32_t>::const_iterator& it);
	RPG::MoveCommand DecodeMove(std::vector<int32_t>::const_iterator& it);

	void SetSubcommandIndex(int indent, int idx);
	uint8_t& ReserveSubcommandIndex(int indent);
	int GetSubcommandIndex(int indent) const;

	void ForegroundTextPush(PendingMessage pm);
	void EndEventProcessing();

	FileRequestBinding request_id;
	enum class Keys {
		eDown,
		eLeft,
		eRight,
		eUp,
		eDecision,
		eCancel,
		eShift,
		eNumbers,
		eOperators
	};

	struct KeyInputState {
		FlagSet<Keys> keys = {};
		int variable = 0;
		int time_variable = 0;
		int wait_frames = 0;
		bool wait = false;
		bool timed = false;

		int CheckInput() const;
		void fromSave(const RPG::SaveEventExecState& save);
		void toSave(RPG::SaveEventExecState& save) const;
	};

	RPG::SaveEventExecState _state;
	KeyInputState _keyinput;
	AsyncOp _async_op = {};
};

inline const RPG::SaveEventExecFrame* Game_Interpreter::GetFrame() const {
	return !_state.stack.empty() ? &_state.stack.back() : nullptr;
}

inline RPG::SaveEventExecFrame* Game_Interpreter::GetFrame() {
	return !_state.stack.empty() ? &_state.stack.back() : nullptr;
}

inline int Game_Interpreter::GetCurrentEventId() const {
	return !_state.stack.empty() ? _state.stack.back().event_id : 0;
}

inline int Game_Interpreter::GetOriginalEventId() const {
	return !_state.stack.empty() ? _state.stack.front().event_id : 0;
}

inline int Game_Interpreter::GetLoopCount() const {
	return loop_count;
}

inline bool Game_Interpreter::IsAsyncPending() {
	return GetAsyncOp().IsActive();
}

inline AsyncOp Game_Interpreter::GetAsyncOp() const {
	return _async_op;
}

#endif
