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

#ifndef _GAME_INTERPRETER_H_
#define _GAME_INTERPRETER_H_

#include <map>
#include <string>
#include <vector>
#include "async_handler.h"
#include "game_character.h"
#include "game_actor.h"
#include "rpg_eventcommand.h"
#include "system.h"
#include "command_codes.h"

class Game_Event;
class Game_CommonEvent;

/**
 * Game_Interpreter class
 */
class Game_Interpreter
{
public:
	Game_Interpreter(int _depth = 0, bool _main_flag = false);
#ifndef EMSCRIPTEN
	// No idea why but emscripten will complain about a missing destructor when
	// using virtual here
	virtual
#endif
	~Game_Interpreter();

	void Clear();
	void Setup(
		const std::vector<RPG::EventCommand>& _list,
		int _event_id,
		bool started_by_decision_key = false,
		int dbg_x = -1, int dbg_y = -1
	);

	bool IsRunning() const;
	void Update();

	void SetupStartingEvent(Game_Event* ev);
	void SetupStartingEvent(Game_CommonEvent* ev);
	void InputButton();
	void SetupChoices(const std::vector<std::string>& choices);

	virtual bool ExecuteCommand();

	enum Sizes {
		MaxSize = 9999999,
		MinSize = -9999999
	};

protected:
	friend class Game_Interpreter_Map;

	int depth;
	bool main_flag;

	int loop_count;
	bool wait_messages;

	unsigned int index;
	int map_id;
	unsigned int event_id;
	int wait_count;

	std::unique_ptr<Game_Interpreter> child_interpreter;
	typedef bool (Game_Interpreter::*ContinuationFunction)(RPG::EventCommand const& com);
	ContinuationFunction continuation;

	std::vector<RPG::EventCommand> list;

	int button_timer;
	bool waiting_battle_anim;
	bool updating;
	bool clear_child;

	bool triggered_by_decision_key = false;

	/**
	 * Gets strings for choice selection.
	 * This is just a helper (private) method
	 * to avoid repeating code.
	 */
	std::vector<std::string> GetChoices();

	/**
	 * Calculates operated value.
	 *
	 * @param operation operation (increase: 0, decrease: 1).
	 * @param operand_type operand type (0: set, 1: variable).
	 * @param operand operand (number or var ID).
	 */
	int OperateValue(int operation, int operand_type, int operand);
	Game_Character* GetCharacter(int character_id) const;

	bool SkipTo(int code, int code2 = -1, int min_indent = -1, int max_indent = -1, bool otherwise_end = false);
	void SetContinuation(ContinuationFunction func);

	void CancelMenuCall();

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
	 * Triggers a game over when all party members are dead.
	 */
	void CheckGameOver();

	bool CommandShowMessage(RPG::EventCommand const& com);
	bool CommandMessageOptions(RPG::EventCommand const& com);
	bool CommandChangeFaceGraphic(RPG::EventCommand const& com);
	bool CommandShowChoices(RPG::EventCommand const& com);
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
	bool CommandEnd();

	virtual bool DefaultContinuation(RPG::EventCommand const& com);
	virtual bool ContinuationChoices(RPG::EventCommand const& com);
	virtual bool ContinuationOpenShop(RPG::EventCommand const& com);
	virtual bool ContinuationShowInnStart(RPG::EventCommand const& com);
	virtual bool ContinuationShowInnFinish(RPG::EventCommand const& com);
	virtual bool ContinuationEnemyEncounter(RPG::EventCommand const& com);

	int DecodeInt(std::vector<int>::const_iterator& it);
	const std::string DecodeString(std::vector<int>::const_iterator& it);
	RPG::MoveCommand DecodeMove(std::vector<int>::const_iterator& it);

	void OnChangeSystemGraphicReady(FileRequestResult* result);

	int debug_x;
	int debug_y;

	FileRequestBinding request_id;
};

#endif
