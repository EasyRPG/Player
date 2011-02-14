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
	void Setup(std::vector<RPG::EventCommand>& _list, int _event_id, int dbg_x = -1, int dbg_y = -1);
	bool IsRunning() const;
	void Update();

	void SetupStartingEvent(Game_Event* ev);
	void SetupStartingEvent(Game_CommonEvent* ev);
	void InputButton();
	void SetupChoices(const std::vector<std::string>& choices);

	virtual bool ExecuteCommand();
	virtual void EndMoveRoute(RPG::MoveRoute* route);

	////////////////////////////////////////////////////////////
	/// Enumeration of codes
	////////////////////////////////////////////////////////////
	enum CommandCodes {
		CallCommonEvent			=  1005,
		ForceFlee				=  1006,
		EnableCombo				=  1007,

		ChangeClass				=  1008,
		ChangeBattleCommands	=  1009,
		ShowMessage				= 10110,
		MessageOptions			= 10120,
		ChangeFaceGraphic		= 10130,
		ShowChoice				= 10140,
		InputNumber				= 10150,
		ControlSwitches			= 10210,
		ControlVars				= 10220,
		TimerOperation			= 10230,
		ChangeGold				= 10310,
		ChangeItems				= 10320,
		ChangePartyMembers		= 10330,
		ChangeExp				= 10410,
		ChangeLevel				= 10420,
		ChangeParameters		= 10430,
		ChangeSkills			= 10440,
		ChangeEquipment			= 10450,
		ChangeHP				= 10460,
		ChangeSP				= 10470,
		ChangeCondition			= 10480,
		FullHeal				= 10490,
		SimulatedAttack			= 10500,
		ChangeHeroName			= 10610,
		ChangeHeroTitle			= 10620,
		ChangeSpriteAssociation = 10630,
		ChangeActorFace			= 10640,
		ChangeVehicleGraphic	= 10650,
		ChangeSystemBGM			= 10660,
		ChangeSystemSFX			= 10670,
		ChangeSystemGraphics	= 10680,
		ChangeScreenTransitions = 10690,
		EnemyEncounter			= 10710,
		OpenShop				= 10720,
		ShowInn					= 10730,
		EnterHeroName			= 10740,
		Teleport				= 10810,
		MemorizeLocation		= 10820,
		RecallToLocation		= 10830,
		EnterExitVehicle		= 10840,
		SetVehicleLocation		= 10850,
		ChangeEventLocation		= 10860,
		TradeEventLocations		= 10870,
		StoreTerrainID			= 10910,
		StoreEventID			= 10920,
		EraseScreen				= 11010,
		ShowScreen				= 11020,
		TintScreen				= 11030,
		FlashScreen				= 11040,
		ShakeScreen				= 11050,
		PanScreen				= 11060,
		WeatherEffects			= 11070,
		ShowPicture				= 11110,
		MovePicture				= 11120,
		ErasePicture			= 11130,
		ShowBattleAnimation		= 11210,
		SpriteTransparency		= 11310,
		FlashSprite				= 11320,
		MoveEvent				= 11330,
		ProceedWithMovement		= 11340,
		HaltAllMovement			= 11350,
		Wait					= 11410,
		PlayBGM					= 11510,
		FadeOutBGM				= 11520,
		MemorizeBGM				= 11530,
		PlayMemorizedBGM		= 11540,
		PlaySound				= 11550,
		PlayMovie				= 11560,
		KeyInputProc			= 11610,
		ChangeMapTileset		= 11710,
		ChangePBG				= 11720,
		ChangeEncounterRate		= 11740,
		TileSubstitution		= 11750,
		TeleportTargets			= 11810,
		ChangeTeleportAccess	= 11820,
		EscapeTarget			= 11830,
		ChangeEscapeAccess		= 11840,
		OpenSaveMenu			= 11910,
		ChangeSaveAccess		= 11930,
		OpenMainMenu			= 11950,
		ChangeMainMenuAccess	= 11960,
		ConditionalBranch		= 12010,
		Label					= 12110,
		JumpToLabel				= 12120,
		Loop					= 12210,
		BreakLoop				= 12220,
		EndEventProcessing		= 12310,
		EraseEvent				= 12320,
		CallEvent				= 12330,
		Comment					= 12410,
		GameOver				= 12420,
		ReturntoTitleScreen		= 12510,

		ChangeMonsterHP			= 13110,
		ChangeMonsterMP			= 13120,
		ChangeMonsterCondition	= 13130,
		ShowHiddenMonster		= 13150,
		ChangeBattleBG			= 13210,
		ShowBattleAnimation_B	= 13260,
		ConditionalBranch_B		= 13310,
		TerminateBattle			= 13410,

		ShowMessage_2			= 20110,
		ShowChoiceOption		= 20140,
		ShowChoiceEnd			= 20141,
		VictoryHandler			= 20710,
		EscapeHandler			= 20711,
		DefeatHandler			= 20712,
		EndBattle				= 20713,
		Transaction				= 20720,
		NoTransaction			= 20721,
		EndShop					= 20722,
		Stay					= 20730,
		NoStay					= 20731,
		EndInn					= 20732,
		ElseBranch				= 22010,
		EndBranch				= 22011,
		EndLoop					= 22210,
		Comment_2				= 22410
	};

	enum Sizes {
		MaxSize = 9999999,
		MinSize = -9999999
	};

	enum CharsID {
		CharPlayer		= 10001,
		CharBoat		= 10002,
		CharShip		= 10003,
		CharAirship		= 10004,
		CharThisEvent	= 10005
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

	bool CommandShowMessage();
	bool CommandChangeFaceGraphic();
	bool CommandShowChoices();
	bool CommandInputNumber();
	bool CommandControlSwitches();
	bool CommandControlVariables();
	bool CommandChangeGold();
	bool CommandChangeItems();
	bool CommandChangePartyMember();
	bool CommandChangeLevel();
	bool CommandChangeSkills();
	bool CommandChangeEquipment();
	bool CommandChangeHP();
	bool CommandChangeSP();
	bool CommandChangeCondition();
	bool CommandFullHeal();
	bool CommandTintScreen();
	bool CommandFlashScreen();
	bool CommandShakeScreen();
	bool CommandWait();
	bool CommandPlayBGM();
	bool CommandFadeOutBGM();
	bool CommandPlaySound();
	bool CommandEndEventProcessing();
	bool CommandGameOver();

	void CommandEnd();

	virtual bool DefaultContinuation();
	virtual bool ContinuationChoices();
	virtual bool ContinuationOpenShop();
	virtual bool ContinuationShowInn();
	virtual bool ContinuationEnemyEncounter();

	int debug_x;
	int debug_y;

	bool teleport_pending;
};

#endif
