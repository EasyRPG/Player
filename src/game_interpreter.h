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

#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include "async_handler.h"
#include "game_character.h"
#include "game_actor.h"
#include <lcf/dbarray.h>
#include <lcf/rpg/fwd.h>
#include <lcf/rpg/eventcommand.h>
#include "system.h"
#include <lcf/rpg/saveeventexecstate.h>
#include <lcf/flag_set.h>
#include "async_op.h"

class Game_Event;
class Game_CommonEvent;
class PendingMessage;

/**
 * Game_Interpreter class
 */
class Game_Interpreter
{
public:
	using Cmd = lcf::rpg::EventCommand::Code;

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
			const std::vector<lcf::rpg::EventCommand>& _list,
			int _event_id,
			bool started_by_decision_key = false
	);
	void Push(Game_Event* ev);
	void Push(Game_Event* ev, const lcf::rpg::EventPage* page, bool triggered_by_decision_key);
	void Push(Game_CommonEvent* ev);

	void InputButton();
	void SetupChoices(const std::vector<std::string>& choices, int indent, PendingMessage& pm);

	virtual bool ExecuteCommand();


	/**
	 * Returns a SaveEventExecState needed for the savefile.
	 *
	 * @return interpreter commands stored in SaveEventCommands
	 */
	lcf::rpg::SaveEventExecState GetState() const;

	/** @return Game_Character of the passed event_id */
	Game_Character* GetCharacter(int event_id) const;

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

	/** @return true if wait command (time or key) is active. Used by 2k3 battle system */
	bool IsWaitingForWaitCommand() const;

protected:
	static constexpr int loop_limit = 10000;
	static constexpr int call_stack_limit = 1000;
	static constexpr int subcommand_sentinel = 255;

	const lcf::rpg::SaveEventExecFrame& GetFrame() const;
	lcf::rpg::SaveEventExecFrame& GetFrame();
	const lcf::rpg::SaveEventExecFrame* GetFramePtr() const;
	lcf::rpg::SaveEventExecFrame* GetFramePtr();

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
	void SkipToNextConditional(std::initializer_list<Cmd> codes, int indent);

	/**
	 * Sets up a wait (and closes the message box)
	 */
	void SetupWait(int duration);

	/**
	 * Sets up a wait using frames (and closes the message box)
	 */
	void SetupWaitFrames(int duration);

	/**
	 * Calculates list of actors.
	 *
	 * @param mode 0: party, 1: specific actor, 2: actor referenced by variable.
	 * @param id actor ID (mode = 1) or variable ID (mode = 2).
	 */
	static std::vector<Game_Actor*> GetActors(int mode, int id);
	static int ValueOrVariable(int mode, int val);
	static int ValueOrVariableBitfield(int mode, int shift, int val);

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

	bool CommandOptionGeneric(lcf::rpg::EventCommand const& com, int option_sub_idx, std::initializer_list<Cmd> next);

	bool CommandShowMessage(lcf::rpg::EventCommand const& com);
	bool CommandMessageOptions(lcf::rpg::EventCommand const& com);
	bool CommandChangeFaceGraphic(lcf::rpg::EventCommand const& com);
	bool CommandShowChoices(lcf::rpg::EventCommand const& com);
	bool CommandShowChoiceOption(lcf::rpg::EventCommand const& com);
	bool CommandShowChoiceEnd(lcf::rpg::EventCommand const& com);
	bool CommandInputNumber(lcf::rpg::EventCommand const& com);
	bool CommandControlSwitches(lcf::rpg::EventCommand const& com);
	bool CommandControlVariables(lcf::rpg::EventCommand const& com);
	bool CommandTimerOperation(lcf::rpg::EventCommand const& com);
	bool CommandChangeGold(lcf::rpg::EventCommand const& com);
	bool CommandChangeItems(lcf::rpg::EventCommand const& com);
	bool CommandChangePartyMember(lcf::rpg::EventCommand const& com);
	bool CommandChangeExp(lcf::rpg::EventCommand const& com);
	bool CommandChangeLevel(lcf::rpg::EventCommand const& com);
	bool CommandChangeParameters(lcf::rpg::EventCommand const& com);
	bool CommandChangeSkills(lcf::rpg::EventCommand const& com);
	bool CommandChangeEquipment(lcf::rpg::EventCommand const& com);
	bool CommandChangeHP(lcf::rpg::EventCommand const& com);
	bool CommandChangeSP(lcf::rpg::EventCommand const& com);
	bool CommandChangeCondition(lcf::rpg::EventCommand const& com);
	bool CommandFullHeal(lcf::rpg::EventCommand const& com);
	bool CommandSimulatedAttack(lcf::rpg::EventCommand const& com);
	bool CommandWait(lcf::rpg::EventCommand const& com);
	bool CommandPlayBGM(lcf::rpg::EventCommand const& com);
	bool CommandFadeOutBGM(lcf::rpg::EventCommand const& com);
	bool CommandPlaySound(lcf::rpg::EventCommand const& com);
	bool CommandEndEventProcessing(lcf::rpg::EventCommand const& com);
	bool CommandComment(lcf::rpg::EventCommand const& com);
	bool CommandGameOver(lcf::rpg::EventCommand const& com);
	bool CommandChangeHeroName(lcf::rpg::EventCommand const& com);
	bool CommandChangeHeroTitle(lcf::rpg::EventCommand const& com);
	bool CommandChangeSpriteAssociation(lcf::rpg::EventCommand const& com);
	bool CommandChangeActorFace(lcf::rpg::EventCommand const& com);
	bool CommandChangeVehicleGraphic(lcf::rpg::EventCommand const& com);
	bool CommandChangeSystemBGM(lcf::rpg::EventCommand const& com);
	bool CommandChangeSystemSFX(lcf::rpg::EventCommand const& com);
	bool CommandChangeSystemGraphics(lcf::rpg::EventCommand const& com);
	bool CommandChangeScreenTransitions(lcf::rpg::EventCommand const& com);
	bool CommandMemorizeLocation(lcf::rpg::EventCommand const& com);
	bool CommandSetVehicleLocation(lcf::rpg::EventCommand const& com);
	bool CommandChangeEventLocation(lcf::rpg::EventCommand const& com);
	bool CommandTradeEventLocations(lcf::rpg::EventCommand const& com);
	bool CommandStoreTerrainID(lcf::rpg::EventCommand const& com);
	bool CommandStoreEventID(lcf::rpg::EventCommand const& com);
	bool CommandEraseScreen(lcf::rpg::EventCommand const& com);
	bool CommandShowScreen(lcf::rpg::EventCommand const& com);
	bool CommandTintScreen(lcf::rpg::EventCommand const& com);
	bool CommandFlashScreen(lcf::rpg::EventCommand const& com);
	bool CommandShakeScreen(lcf::rpg::EventCommand const& com);
	bool CommandWeatherEffects(lcf::rpg::EventCommand const& com);
	bool CommandShowPicture(lcf::rpg::EventCommand const& com);
	bool CommandMovePicture(lcf::rpg::EventCommand const& com);
	bool CommandErasePicture(lcf::rpg::EventCommand const& com);
	bool CommandPlayerVisibility(lcf::rpg::EventCommand const& com);
	bool CommandMoveEvent(lcf::rpg::EventCommand const& com);
	bool CommandMemorizeBGM(lcf::rpg::EventCommand const& com);
	bool CommandPlayMemorizedBGM(lcf::rpg::EventCommand const& com);
	bool CommandKeyInputProc(lcf::rpg::EventCommand const& com);
	bool CommandChangeMapTileset(lcf::rpg::EventCommand const& com);
	bool CommandChangePBG(lcf::rpg::EventCommand const& com);
	bool CommandChangeEncounterSteps(lcf::rpg::EventCommand const& com);
	bool CommandTileSubstitution(lcf::rpg::EventCommand const& com);
	bool CommandTeleportTargets(lcf::rpg::EventCommand const& com);
	bool CommandChangeTeleportAccess(lcf::rpg::EventCommand const& com);
	bool CommandEscapeTarget(lcf::rpg::EventCommand const& com);
	bool CommandChangeEscapeAccess(lcf::rpg::EventCommand const& com);
	bool CommandChangeSaveAccess(lcf::rpg::EventCommand const& com);
	bool CommandChangeMainMenuAccess(lcf::rpg::EventCommand const& com);
	bool CommandConditionalBranch(lcf::rpg::EventCommand const& com);
	bool CommandElseBranch(lcf::rpg::EventCommand const& com);
	bool CommandEndBranch(lcf::rpg::EventCommand const& com);
	bool CommandJumpToLabel(lcf::rpg::EventCommand const& com);
	bool CommandLoop(lcf::rpg::EventCommand const& com);
	bool CommandBreakLoop(lcf::rpg::EventCommand const& com);
	bool CommandEndLoop(lcf::rpg::EventCommand const& com);
	bool CommandEraseEvent(lcf::rpg::EventCommand const& com);
	bool CommandCallEvent(lcf::rpg::EventCommand const& com);
	bool CommandReturnToTitleScreen(lcf::rpg::EventCommand const& com);
	bool CommandChangeClass(lcf::rpg::EventCommand const& com);
	bool CommandChangeBattleCommands(lcf::rpg::EventCommand const& com);
	bool CommandExitGame(lcf::rpg::EventCommand const& com);
	bool CommandToggleFullscreen(lcf::rpg::EventCommand const& com);
	bool CommandOpenVideoOptions(lcf::rpg::EventCommand const& com);
	bool CommandManiacGetSaveInfo(lcf::rpg::EventCommand const& com);
	bool CommandManiacSave(lcf::rpg::EventCommand const& com);
	bool CommandManiacLoad(lcf::rpg::EventCommand const& com);
	bool CommandManiacEndLoadProcess(lcf::rpg::EventCommand const& com);
	bool CommandManiacGetMousePosition(lcf::rpg::EventCommand const& com);
	bool CommandManiacSetMousePosition(lcf::rpg::EventCommand const& com);
	bool CommandManiacShowStringPicture(lcf::rpg::EventCommand const& com);
	bool CommandManiacGetPictureInfo(lcf::rpg::EventCommand const& com);
	bool CommandManiacControlVarArray(lcf::rpg::EventCommand const& com);
	bool CommandManiacKeyInputProcEx(lcf::rpg::EventCommand const& com);
	bool CommandManiacRewriteMap(lcf::rpg::EventCommand const& com);
	bool CommandManiacControlGlobalSave(lcf::rpg::EventCommand const& com);
	bool CommandManiacChangePictureId(lcf::rpg::EventCommand const& com);
	bool CommandManiacSetGameOption(lcf::rpg::EventCommand const& com);
	bool CommandManiacCallCommand(lcf::rpg::EventCommand const& com);

	int DecodeInt(lcf::DBArray<int32_t>::const_iterator& it);
	const std::string DecodeString(lcf::DBArray<int32_t>::const_iterator& it);
	lcf::rpg::MoveCommand DecodeMove(lcf::DBArray<int32_t>::const_iterator& it);

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
		eOperators,
		// ManiacPatch
		eMouseLeft,
		eMouseRight,
		eMouseMiddle,
		eMouseScrollDown,
		eMouseScrollUp
	};

	struct KeyInputState {
		lcf::FlagSet<Keys> keys = {};
		int variable = 0;
		int time_variable = 0;
		int wait_frames = 0;
		bool wait = false;
		bool timed = false;

		int CheckInput() const;
		void fromSave(const lcf::rpg::SaveEventExecState& save);
		void toSave(lcf::rpg::SaveEventExecState& save) const;
	};

	bool CheckOperator(int val, int val2, int op) const;
	bool ManiacCheckContinueLoop(int val, int val2, int type, int op) const;

	lcf::rpg::SaveEventExecState _state;
	KeyInputState _keyinput;
	AsyncOp _async_op = {};
};

inline const lcf::rpg::SaveEventExecFrame* Game_Interpreter::GetFramePtr() const {
	return !_state.stack.empty() ? &_state.stack.back() : nullptr;
}

inline lcf::rpg::SaveEventExecFrame* Game_Interpreter::GetFramePtr() {
	return !_state.stack.empty() ? &_state.stack.back() : nullptr;
}

inline const lcf::rpg::SaveEventExecFrame& Game_Interpreter::GetFrame() const {
	auto* frame = GetFramePtr();
	assert(frame);
	return *frame;
}

inline lcf::rpg::SaveEventExecFrame& Game_Interpreter::GetFrame() {
	auto* frame = GetFramePtr();
	assert(frame);
	return *frame;
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
