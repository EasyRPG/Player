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

#include <cstdlib>
#include <iostream>
#include <sstream>
#include "game_interpreter.h"
#include "audio.h"
#include "game_map.h"
#include "game_event.h"
#include "game_player.h"
#include "game_temp.h"
#include "game_switches.h"
#include "game_variables.h"
#include "game_party.h"
#include "game_actors.h"
#include "game_system.h"
#include "game_message.h"
#include "game_picture.h"
#include "spriteset_map.h"
#include "sprite_character.h"
#include "scene_map.h"
#include "scene.h"
#include "graphics.h"
#include "input.h"
#include "main_data.h"
#include "output.h"
#include "player.h"
#include "util_macro.h"

////////////////////////////////////////////////////////////
/// Enumeration of codes
////////////////////////////////////////////////////////////
enum CommandCodes {
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

////////////////////////////////////////////////////////////
/// Forward declarations
///////////////////////////////////////////////////////////
Game_Interpreter::Game_Interpreter(int _depth, bool _main_flag) {
	depth = _depth;
	main_flag = _main_flag;

	if (depth > 100) {
		Output::Warning("Too many event calls (over 9000)");
	}

	Clear();
}

Game_Interpreter::~Game_Interpreter() {
}

////////////////////////////////////////////////////////////
/// Clear
////////////////////////////////////////////////////////////
void Game_Interpreter::Clear() {
	map_id = 0;						// map ID when starting up
	event_id = 0;					// event ID
	//Game_Message::message_waiting = false;	// waiting for message to end
	move_route_waiting = false;		// waiting for move completion
	button_input_variable_id = 0;	// button input variable ID
	wait_count = 0;					// wait count
	child_interpreter = NULL;		// child interpreter for common events, etc
	continuation = NULL;			// function to execute to resume command
	button_timer = 0;
}

////////////////////////////////////////////////////////////
/// Is Interpreter Running
////////////////////////////////////////////////////////////
bool Game_Interpreter::IsRunning() const {
	return !list.empty();
}

////////////////////////////////////////////////////////////
/// Setup
////////////////////////////////////////////////////////////
void Game_Interpreter::Setup(std::vector<RPG::EventCommand>& _list, int _event_id) {

	Clear();

	map_id = Game_Map::GetMapId();
	event_id = _event_id;
	list = _list;
	index = 0;

	CancelMenuCall();
}

void Game_Interpreter::CancelMenuCall() {
	// TODO
}

void Game_Interpreter::SetContinuation(bool (Game_Interpreter::*func)()) {
	continuation = func;
}

////////////////////////////////////////////////////////////
/// Update
////////////////////////////////////////////////////////////
void Game_Interpreter::Update() {

	loop_count = 0;

	for (;;) {
		loop_count++;

		if (loop_count > 100) {
			Graphics::Update(); // Freeze prevention
			loop_count = 0;
		}

		/* If map is different than event startup time
		set event_id to 0 */
		if (Game_Map::GetMapId() != map_id) {
			event_id = 0;
		}
		
		/* If there's any active child interpreter, update it */
		if (child_interpreter != NULL) {

			child_interpreter->Update();

			if (!child_interpreter->IsRunning()) {
				delete child_interpreter;
				child_interpreter = NULL;
			}

			// If child interpreter still exists
			if (child_interpreter != NULL) {
				return;
			}
		}

		if (Game_Message::message_waiting) {
			return;
		}

		// If waiting for a move to end
		if (move_route_waiting) {
			if (Main_Data::game_player->GetMoveRouteForcing()) {
				return;
			}
				
			Game_Event* g_event;
			for (size_t i = 0; i < Game_Map::GetEvents().size(); i++) {
				g_event = Game_Map::GetEvents()[i];

				if (g_event->GetMoveRouteForcing()) {
					return;
				}
			}
			move_route_waiting = false;
		}

		if (button_input_variable_id > 0) {
			InputButton();
			return;
		}

		if (wait_count > 0) {
			wait_count--;
			return;
		}

		if (Game_Temp::forcing_battler != NULL) {
			return;
		}

		if (Game_Temp::battle_calling ||
			Game_Temp::shop_calling ||
//			Game_Temp::inn_calling ||
			Game_Temp::name_calling ||
			Game_Temp::menu_calling ||
			Game_Temp::save_calling ||
			Game_Temp::to_title ||
			Game_Temp::gameover) {
			
			return;
		}

		if (continuation) {
			bool result = (this->*continuation)();
			continuation = NULL;
			if (result)
				continue;
			else
				return;
		}

		if (list.empty()) {
			if (main_flag) {
				SetupStartingEvent();
			}

			if (list.empty()) {
				return;
			}
		}

		if (!ExecuteCommand()) {
			return;
		}

		index++;
	} // for
}

////////////////////////////////////////////////////////////
/// Setup Starting Event
////////////////////////////////////////////////////////////
void Game_Interpreter::SetupStartingEvent() {

	if (Game_Map::GetNeedRefresh()) {
		Game_Map::Refresh();
	}

	if (Game_Temp::common_event_id > 0) {
		Setup(Data::commonevents[Game_Temp::common_event_id].event_commands, 0);
		Game_Temp::common_event_id = 0;
		return;
	}

	Game_Event* _event;
	for (tEventHash::iterator i = Game_Map::GetEvents().begin(); i != Game_Map::GetEvents().end(); i++) {
		_event = i->second;
		
		if (_event->GetStarting()) {
			_event->ClearStarting();
			Setup(_event->GetList(), _event->GetId());
			return;
		}
	}

	RPG::CommonEvent* common_event;
	for (size_t i = 0; i < Data::commonevents.size(); i++) {
		common_event = &Data::commonevents[i];

		// If trigger is auto run, and condition switch is ON
		if ( (common_event->trigger == 1) &&
			Game_Switches[common_event->switch_id]) {
			Setup(common_event->event_commands, 0);
			return;
		}
	}
}

////////////////////////////////////////////////////////////
/// Skip to Command
////////////////////////////////////////////////////////////
bool Game_Interpreter::SkipTo(int code, int code2, int min_indent, int max_indent) {
	if (code2 < 0)
		code2 = code;
	if (min_indent < 0)
		min_indent = list[index].indent;
	if (max_indent < 0)
		max_indent = list[index].indent;

	for (int idx = index; (size_t) idx < list.size(); idx++) {
		if (list[idx].indent < min_indent)
			return false;
		if (list[idx].indent > max_indent)
			continue;
		if (list[idx].code != code &&
			list[idx].code != code2)
			continue;
		index = idx;
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////
/// Execute Command
////////////////////////////////////////////////////////////
bool Game_Interpreter::ExecuteCommand() {
	
	if (index >= list.size()) {
		CommandEnd();
		return true;
	}
	
	switch (list[index].code) {
		case ShowMessage:
			return CommandShowMessage();
		case ShowChoice: 
			return CommandShowChoices();
		case ShowChoiceOption:
			return SkipTo(ShowChoiceEnd);
		case ShowChoiceEnd:
			return true;
		case InputNumber: 
			return CommandInputNumber();
		case MessageOptions: 
			return CommandMessageOptions();
		case ChangeFaceGraphic: 
			return CommandChangeFaceGraphic();
		case ChangeExp: 
			return CommandChangeExp();
		case ChangeGold: 
			return CommandChangeGold();
		case ChangeItems: 
			return CommandChangeItems();
		case ChangeLevel: 
			return CommandChangeLevel();
		case ConditionalBranch: 
			return CommandConditionalBranch();
		case ElseBranch:
			return SkipTo(EndBranch);
		case EndBranch:
			return true;
		case ControlSwitches: 
			return CommandControlSwitches();
		case ControlVars: 
			return CommandControlVariables();
		case ChangeParameters:
			return CommandChangeParameters();
		case ChangeSkills:
			return CommandChangeSkills();
		case ChangeEquipment:
			return CommandChangeEquipment();
		case ChangeHP:
			return CommandChangeHP();
		case ChangeSP:
			return CommandChangeSP();
		case ChangeCondition:
			return CommandChangeCondition();
		case ChangePartyMembers:
			return CommandChangePartyMember();
		case FullHeal:
			return CommandFullHeal();
		case ChangeHeroName:
			return CommandChangeHeroName();
		case ChangeHeroTitle:
			return CommandChangeHeroTitle();
		case ChangeSpriteAssociation:
			return CommandChangeSpriteAssociation();
		case MemorizeLocation:
			return CommandMemorizeLocation();
		case RecallToLocation:
			return CommandRecallToLocation();
		case StoreTerrainID:
			return CommandStoreTerrainID();
		case StoreEventID:
			return CommandStoreEventID();
		case PlayBGM:
			return CommandPlayBGM();
		case FadeOutBGM:
			return CommandFadeOutBGM();
		case MemorizeBGM:
			return CommandMemorizeBGM();
		case PlayMemorizedBGM:
			return CommandPlayMemorizedBGM();
		case PlaySound:
			return CommandPlaySound();
		case ChangeSystemBGM:
			return CommandChangeSystemBGM();
		case ChangeSystemSFX:
			return CommandChangeSystemSFX();
		case Wait:
			return CommandWait();
		case ChangeSaveAccess:
			return CommandChangeSaveAccess();
		case ChangeTeleportAccess:
			return CommandChangeTeleportAccess();
		case ChangeEscapeAccess:
			return CommandChangeEscapeAccess();
		case ChangeMainMenuAccess:
			return CommandChangeMainMenuAccess();
		case ChangeActorFace:
			return CommandChangeActorFace();
		case Teleport:
			return CommandTeleport();
		case EraseScreen:
			return CommandEraseScreen();
		case ShowScreen:
			return CommandShowScreen();
		case Comment:
		case Comment_2:
			return true;
		case ShowPicture:
			return CommandShowPicture();
		case MovePicture:
			return CommandMovePicture();
		case ErasePicture:
			return CommandErasePicture();
		case TintScreen:
			return CommandTintScreen();
		case FlashScreen:
			return CommandFlashScreen();
		case ShakeScreen:
			return CommandShakeScreen();
		case WeatherEffects:
			return CommandWeatherEffects();
		case ChangeSystemGraphics:
			return CommandChangeSystemGraphics();
		case ChangeScreenTransitions:
			return CommandChangeScreenTransitions();
		case ChangeEventLocation:
			return CommandChangeEventLocation();
		case TradeEventLocations:
			return CommandTradeEventLocations();
		case TimerOperation:
			return CommandTimerOperation();
		case ChangePBG:
			return CommandChangePBG();
		case Label:
			return true;
		case JumpToLabel:
			return CommandJumpToLabel();
		case Loop:
			return true;
		case BreakLoop:
			return CommandBreakLoop();
		case EndLoop:
			return CommandEndLoop();
		case EndEventProcessing:
			return CommandEndEventProcessing();
		case OpenShop:
			return CommandOpenShop();
		case Transaction:
		case NoTransaction:
			return SkipTo(EndShop);
		case EndShop:
			return true;
		case ShowInn:
			return CommandShowInn();
		case Stay:
		case NoStay:
			return SkipTo(EndInn);
		case EndInn:
			return true;
		case EnterHeroName:
			return CommandEnterHeroName();
		case GameOver:
			return CommandGameOver();
		case ReturntoTitleScreen:
			return CommandReturntoTitleScreen();
		case OpenSaveMenu:
			return CommandOpenSaveMenu();
		case OpenMainMenu:
			return CommandOpenMainMenu();
		case EnemyEncounter:
			return CommandEnemyEncounter();
		case VictoryHandler:
		case EscapeHandler:
		case DefeatHandler:
			return SkipTo(EndBattle);
		case EndBattle:
			return true;
		case TeleportTargets:
			return CommandTeleportTargets();
		case EscapeTarget:
			return CommandEscapeTarget();
		case SpriteTransparency:
			return CommandSpriteTransparency();
		case FlashSprite:
			return CommandFlashSprite();
		case EraseEvent:
			return CommandEraseEvent();
		case ChangeMapTileset:
			return CommandChangeMapTileset();
		case CallEvent:
			return CommandCallEvent();
		case ChangeEncounterRate:
			return CommandChangeEncounterRate();
		case ProceedWithMovement:
			return CommandProceedWithMovement();
		case PlayMovie:
			return CommandPlayMovie();
		case ChangeBattleCommands:
			return CommandChangeBattleCommands();
		case KeyInputProc:
			return CommandKeyInputProc();
		case ChangeVehicleGraphic:
			return CommandChangeVehicleGraphic();
		case EnterExitVehicle:
			return CommandEnterExitVehicle();
		case SetVehicleLocation:
			return CommandSetVehicleLocation();
		default:
			return true;
	}
}

bool Game_Interpreter::CommandTeleport() { // Code 10810
	// TODO: if in battle return true
	if (Main_Data::game_player->IsTeleporting()) {
			return false;
	}

	int map_id = list[index].parameters[0];
	int x = list[index].parameters[1];
	int y = list[index].parameters[2];

	Main_Data::game_player->ReserveTeleport(map_id, x, y);
	index++;

	return false;
}

bool Game_Interpreter::CommandChangeActorFace() {
	Game_Actor* actor = Game_Actors::GetActor(list[index].parameters[0]);
	if (actor != NULL) {
		actor->SetFace(list[index].string, list[index].parameters[1]);
		return true;
	}
	return false;
}

bool Game_Interpreter::CommandChangeSaveAccess() { // code 11930
	Game_System::save_disabled = list[index].parameters[0] == 0;
	return true;
}

bool Game_Interpreter::CommandChangeTeleportAccess() { // code 11820
	Game_System::teleport_disabled = list[index].parameters[0] == 0;
	return true;
}

bool Game_Interpreter::CommandChangeEscapeAccess() { // code 11840
	Game_System::escape_disabled = list[index].parameters[0] == 0;
	return true;
}

bool Game_Interpreter::CommandChangeMainMenuAccess() { // code 11960
	Game_System::main_menu_disabled = list[index].parameters[0] == 0;
	return true;
}

bool Game_Interpreter::CommandWait() {
	if (Player::engine == Player::EngineRpg2k) {
		wait_count = list[index].parameters[0] * DEFAULT_FPS / 10;
	} else {
		switch (list[index].parameters[1]) {
			case 0:
				wait_count = list[index].parameters[0] * DEFAULT_FPS / 10;
				break;
			default:
				// TODO: wait until key pressed
				wait_count = 0;
				break;
		}
	}
	return true;
}

////////////////////////////////////////////////////////////
/// Input Button
////////////////////////////////////////////////////////////
void Game_Interpreter::InputButton() {
	Input::InputButton n = Input::BUTTON_COUNT;
	
	if (Input::IsTriggered(Input::UP)) {
		n = Input::UP;
	} else {
		if (Input::IsTriggered(Input::DOWN)) {
			n = Input::DOWN;
		} else {
			if (Input::IsTriggered(Input::LEFT)) {
				n = Input::LEFT;
			} else {
				if (Input::IsTriggered(Input::RIGHT)) {
					n = Input::RIGHT;
				} else {
					if (Input::IsTriggered(Input::DECISION)) {
						n = Input::DECISION;
					} else {
						if (Input::IsTriggered(Input::CANCEL)) {
							n = Input::CANCEL;
						} else {
							if (Input::IsTriggered(Input::SHIFT)) {
								n = Input::SHIFT;
							}
						}
					}
				}
			}
		}
	} // end first if

	// If a button was pressed
	if (n != Input::BUTTON_COUNT) {
		// Set variable
		Game_Variables[button_input_variable_id] = n;
		Game_Map::SetNeedRefresh(true);
		button_input_variable_id = 0;
	}
}

void Game_Interpreter::CommandEnd() {
	if (Game_Message::visible) {
		Game_Message::visible = false;
		Game_Message::FullClear();
	}
	list.clear();

	if ((main_flag) && (event_id > 0)) {
		Game_Map::GetEvents()[event_id]->Unlock();
	}
}

/////////////////////////////////////////////
/// Get Strings for choice selection
/// This is just a helper (private) method
/// to avoid repeating code
/////////////////////////////////////////////
void Game_Interpreter::GetStrings(std::vector<std::string>& ret_val) {
	// Let's find the choices
	int current_indent = list[index + 1].indent;
	unsigned int index_temp = index + 1;
	std::vector<std::string> s_choices;
	while ( index_temp < list.size() ) {
		if ( (list[index_temp].code == ShowChoiceOption) && (list[index_temp].indent == current_indent) ) {
			// Choice found
			s_choices.push_back(list[index_temp].string);
		}
		// If found end of show choice command
		if ( ( (list[index_temp].code == ShowChoiceEnd) && (list[index_temp].indent == current_indent) ) ||
			// Or found Cancel branch
			( (list[index_temp].code == ShowChoiceOption) && (list[index_temp].indent == current_indent) &&
			(list[index_temp].string == "") ) ) {
			
			break;
		}
		// Move on to the next command
		index_temp++;
	}
	ret_val.swap(s_choices);
}

////////////////////////////////////////////////////////////
/// Command Show Message
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandShowMessage() { // Code ShowMessage
	// If there's a text already, return immediately
	if (!Game_Message::texts.empty()) {
		return false;
	}
	unsigned int line_count = 0;

	Game_Message::message_waiting = true;

	// Set first line
	Game_Message::texts.push_back(list[index].string);
	line_count++;

	for (;;) {
		// If next event command is the following parts of the message
		if ( index < list.size() - 1 && list[index+1].code == ShowMessage_2 ) {
			// Add second (another) line
			line_count++;
			Game_Message::texts.push_back(list[index+1].string);
		} else {
			// If next event command is show choices
			std::vector<std::string> s_choices;
			if ( (index < list.size() - 1) && (list[index+1].code == ShowChoice) ) {
				GetStrings(s_choices);
				// If choices fit on screen
				if (s_choices.size() <= (4 - line_count)) {
					index++;
					Game_Message::choice_start = line_count;
					Game_Message::choice_cancel_type = list[index].parameters[0];
					SetupChoices(s_choices);
				}
			} else if ((index < list.size() - 1) && (list[index+1].code == InputNumber) ) {
				// If next event command is input number
				// If input number fits on screen
				if (line_count < 4) {
					index++;
					Game_Message::num_input_start = line_count;
					Game_Message::num_input_digits_max = list[index].parameters[0];
					Game_Message::num_input_variable_id = list[index].parameters[1];
				}
			}

			return true;
		}
		index++;
	} // End for
}

////////////////////////////////////////////////////////////
/// Setup Choices
////////////////////////////////////////////////////////////
void Game_Interpreter::SetupChoices(const std::vector<std::string>& choices) {
	Game_Message::choice_start = Game_Message::texts.size();
	Game_Message::choice_max = choices.size();
	Game_Message::choice_disabled.reset();

	// Set choices to message text
	unsigned int i;
	for (i = 0; i < choices.size(); i++) {
		Game_Message::texts.push_back(choices[i]);
	}

	SetContinuation(&Game_Interpreter::ContinuationChoices);
}

bool Game_Interpreter::ContinuationChoices() {
	int indent = list[index].indent;
	for (;;) {
		if (!SkipTo(ShowChoiceOption, ShowChoiceEnd, indent, indent))
			return false;
		int which = list[index].parameters[0];
		index++;
		if (which > Game_Message::choice_result)
			return false;
		if (which < Game_Message::choice_result)
			continue;
		break;
	}

	return true;
}

////////////////////////////////////////////////////////////
/// Command Show choices
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandShowChoices() { // Code ShowChoice
	if (!Game_Message::texts.empty()) {
		return false;
	}

	Game_Message::message_waiting = true;

	// Choices setup
	std::vector<std::string> choices;
	Game_Message::choice_cancel_type = list[index].parameters[0];
	GetStrings(choices);
	SetupChoices(choices);

	return true;
}

////////////////////////////////////////////////////////////
/// Command control switches
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandControlSwitches() { // Code ControlSwitches
	int i;
	switch (list[index].parameters[0]) {
		case 0:
		case 1:
			// Single and switch range
			for (i = list[index].parameters[1]; i <= list[index].parameters[2]; i++) {
				if (list[index].parameters[3] != 2) {
					Game_Switches[i] = list[index].parameters[3] == 0;
				} else {
					Game_Switches[i] = !Game_Switches[i];
				}
			}
			break;
		case 2:
			// Switch from variable
			if (list[index].parameters[3] != 2) {
				Game_Switches[list[index].parameters[2]] = list[index].parameters[3] == 0;
			} else {
				Game_Switches[list[index].parameters[2]] = !Game_Switches[list[index].parameters[2]];
			}
			break;
		default:
			return false;
	}
	Game_Map::SetNeedRefresh(true);
	return true;
}

////////////////////////////////////////////////////////////
/// Command control vars
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandControlVariables() { // Code ControlVars
	int i, value = 0;
	Game_Actor* actor;
	Game_Character* character;

	switch (list[index].parameters[4]) {
		case 0:
			// Constant
			value = list[index].parameters[5];
			break;
		case 1:
			// Var A ops B
			value = Game_Variables[list[index].parameters[5]];
			break;
		case 2:
			// Number of var A ops B
			value = Game_Variables[Game_Variables[list[index].parameters[5]]];
			break;
		case 3:
			// Random between range
			int a, b;
			a = max(list[index].parameters[5], list[index].parameters[6]);
			b = min(list[index].parameters[5], list[index].parameters[6]);
			value = rand() % (a-b+1)+b;
			break;
		case 4:
			// Items
			switch (list[index].parameters[6]) {
				case 0:
					// Number of items posessed
					value = Game_Party::ItemNumber(list[index].parameters[5]);
					break;
				case 1:
					// Number of heroes that have the item equipped
					std::vector<Game_Actor*>::iterator j;
					for (j = Game_Party::GetActors().begin(); j != Game_Party::GetActors().end(); j++) {
						if ( ((*j)->GetWeaponId() == list[index].parameters[6]) ||
							((*j)->GetShieldId() == list[index].parameters[6]) ||
							((*j)->GetArmorId() == list[index].parameters[6]) ||
							((*j)->GetHelmetId() == list[index].parameters[6]) ||
							((*j)->GetAccessoryId() == list[index].parameters[6]) ) {
								value++;
						}
					}
					break;
			}
			break;
		case 5:
			// Hero
			actor = Game_Actors::GetActor(list[index].parameters[5]);
			if (actor != NULL) {
				switch (list[index].parameters[6]) {
					case 0:
						// Level
						value = actor->GetLevel();
						break;
					case 1:
						// Experience
						value = actor->GetExp();
						break;
					case 2:
						// Current HP
						value = actor->GetHp();
						break;
					case 3:
						// Current MP
						value = actor->GetSp();
						break;
					case 4:
						// Max HP
						value = actor->GetMaxHp();
						break;
					case 5:
						// Max MP
						value = actor->GetMaxSp();
						break;
					case 6:
						// Attack
						value = actor->GetAtk();
						break;
					case 7:
						// Defense
						value = actor->GetDef();
						break;
					case 8:
						// Intelligence
						value = actor->GetSpi();
						break;
					case 9:
						// Agility
						value = actor->GetAgi();
						break;
					case 10:
						// Weapon ID
						value = actor->GetWeaponId();
						break;
					case 11:
						// Shield ID
						value = actor->GetShieldId();
						break;
					case 12:
						// Armor ID
						value = actor->GetArmorId();
						break;
					case 13:
						// Helmet ID
						value = actor->GetHelmetId();
						break;
					case 14:
						// Accesory ID
						value = actor->GetAccessoryId();
						break;
				}
			}
			break;
		case 6:
			// Characters
			if (list[index].parameters[6] != 0){
				character = GetCharacter(list[index].parameters[5]);
			} else {
				// Special case for Player Map ID
				character = NULL;
				value = Game_Map::GetMapId();
			}
			// Other cases
			if (character != NULL) {
				switch (list[index].parameters[6]) {
					case 1:
						// X Coordinate
						value = character->GetX();
						break;
					case 2:
						// Y Coordinate
						value = character->GetY();
						break;
					case 3:
						// TODO Orientation
						// Needs testing
						value = character->GetDirection();
						break;
					case 4:
						// Screen X
						value = character->GetScreenX();
						break;
					case 5:
						// Screen Y
						value = character->GetScreenY();
				}
			}
			break;
		case 7:
			// More
			switch (list[index].parameters[5]) {
				case 0:
					// Gold
					value = Game_Party::GetGold();
					break;
				case 1:
					value = Game_System::ReadTimer(Game_System::Timer1);
					break;
				case 2:
					// Number of heroes in party
					value = Game_Party::GetActors().size();
					break;
				case 3:
					// Number of saves
					value = Game_System::save_count;
					break;
				case 4:
					// Number of battles
					value = Game_Party::GetBattleCount();
					break;
				case 5:
					// Number of wins
					value = Game_Party::GetWinCount();
					break;
				case 6:
					// Number of defeats
					value = Game_Party::GetDefeatCount();
					break;
				case 7:
					// Number of flights (aka run away)
					value = Game_Party::GetRunCount();
					break;
				case 8:
					// TODO MIDI performance position (wtf is this?)
					break;
				case 9:
					value = Game_System::ReadTimer(Game_System::Timer2);
					break;
			}
			break;
		default:
			;
	}

	switch (list[index].parameters[0]) {
		case 0:
		case 1:
			// Single and Var range
			for (i = list[index].parameters[1]; i <= list[index].parameters[2]; i++) {
				switch (list[index].parameters[3]) {
					case 0:
						// Assignement
						Game_Variables[i] = value;
						break;
					case 1:
						// Addition
						Game_Variables[i] += value;
						break;
					case 2:
						// Subtraction
						Game_Variables[i] -= value;
						break;
					case 3:
						// Multiplication
						Game_Variables[i] *= value;
						break;
					case 4:
						// Division
						if (value != 0) {
							Game_Variables[i] /= value;
						}
						break;
					case 5:
						// Module
						if (value != 0) {
							Game_Variables[i] %= value;
						}
				}
				if (Game_Variables[i] > MaxSize) {
					Game_Variables[i] = MaxSize;
				}
				if (Game_Variables[i] < MinSize) {
					Game_Variables[i] = MinSize;
				}
			}
			break;

		case 2:
			switch (list[index].parameters[3]) {
				case 0:
					// Assignement
					Game_Variables[list[index].parameters[1]] = value;
					break;
				case 1:
					// Addition
					Game_Variables[list[index].parameters[1]] += value;
					break;
				case 2:
					// Subtraction
					Game_Variables[list[index].parameters[1]] -= value;
					break;
				case 3:
					// Multiplication
					Game_Variables[list[index].parameters[1]] *= value;
					break;
				case 4:
					// Division
					if (value != 0) {
						Game_Variables[list[index].parameters[1]] /= value;
					}
					break;
				case 5:
					// Module
					if (value != 0) {
						Game_Variables[list[index].parameters[1]] %= value;
					}
			}
			if (Game_Variables[list[index].parameters[1]] > MaxSize) {
				Game_Variables[list[index].parameters[1]] = MaxSize;
			}
			if (Game_Variables[list[index].parameters[1]] < MinSize) {
				Game_Variables[list[index].parameters[1]] = MinSize;
			}
	}
	Game_Map::SetNeedRefresh(true);
	return true;
}

////////////////////////////////////////////////////////////
/// * Calculate Operated Value
///     operation    : operation (increase: 0, decrease: 1)
///     operand_type : operand type (0: set, 1: variable)
///     operand      : operand (number or var ID)
////////////////////////////////////////////////////////////
int Game_Interpreter::OperateValue(int operation, int operand_type, int operand) {
	int value = 0;

	if (operand_type == 0) {
		value = operand;
	} else {
		value = Game_Variables[operand];
	}

	// Reverse sign of value if operation is substract
	if (operation == 1) {
		value = -value;
	}

	return value;
}

////////////////////////////////////////////////////////////
/// * Calculate List of Actors
///     mode : 0: party, 1: specific actor, 2: actor referenced by variable
///     id   : actor ID (mode = 1) or variable ID (mode = 2)
////////////////////////////////////////////////////////////
static std::vector<Game_Actor*> GetActors(int mode, int id) {
	std::vector<Game_Actor*> actors;

	switch (mode) {
	case 0:
		// Party
		for (std::vector<Game_Actor*>::iterator i = Game_Party::GetActors().begin(); 
			 i != Game_Party::GetActors().end(); 
			 i++) {
			actors.push_back(Game_Actors::GetActor((*i)->GetId()));
		}
		break;
	case 1:
		// Hero
		actors.push_back(Game_Actors::GetActor(id));
		break;
	case 2:
		// Var hero
		actors.push_back(Game_Actors::GetActor(Game_Variables[id]));
		break;
	}

	return actors;
}

////////////////////////////////////////////////////////////
/// Get Character
////////////////////////////////////////////////////////////
Game_Character* Game_Interpreter::GetCharacter(int character_id) {

	switch (character_id) {
		case CharPlayer:
			// Player/Hero
			return Main_Data::game_player;
		case CharBoat:
			return Game_Map::GetVehicle(Game_Vehicle::Boat);
		case CharShip:
			return Game_Map::GetVehicle(Game_Vehicle::Ship);
		case CharAirship:
			return Game_Map::GetVehicle(Game_Vehicle::Airship);
		case CharThisEvent:
			// This event
			return (Game_Map::GetEvents().empty()) ? NULL : Game_Map::GetEvents()[event_id];
		default:
			// Other events
			return (Game_Map::GetEvents().empty()) ? NULL : Game_Map::GetEvents()[character_id];
	}
}

////////////////////////////////////////////////////////////
/// Change Gold
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandChangeGold() { // Code 10310
	int value;
	value = OperateValue(
		list[index].parameters[0], 
		list[index].parameters[1],
		list[index].parameters[2]
	);

	Game_Party::GainGold(value);

	// Continue
	return true;
}

////////////////////////////////////////////////////////////
/// Change Items
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandChangeItems() { // Code 10320
	int value;
	value = OperateValue(
		list[index].parameters[0],
		list[index].parameters[3],
		list[index].parameters[4]
	);

	if (list[index].parameters[1] == 0) {
		// Item by const number
		Game_Party::GainItem(list[index].parameters[2], value);
	} else {
		// Item by variable
		Game_Party::GainItem(
			Game_Variables[list[index].parameters[2]],
			value
		);
	}
	Game_Map::SetNeedRefresh(true);
	// Continue
	return true;
}

////////////////////////////////////////////////////////////
/// Input Number
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandInputNumber() {
	if (!Game_Message::texts.empty()) {
		return false;
	}

	Game_Message::message_waiting = true;

	Game_Message::texts.clear();
	Game_Message::num_input_start = 0;
	Game_Message::num_input_variable_id = list[index].parameters[1];
	Game_Message::num_input_digits_max = list[index].parameters[0];
	
	// Continue
	return true;
}

////////////////////////////////////////////////////////////
/// Change Face Graphic
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandChangeFaceGraphic() { // Code 10130
	Game_Message::face_name = list[index].string;
	Game_Message::face_index = list[index].parameters[0];
	Game_Message::face_left_position = list[index].parameters[1] == 0;
	Game_Message::face_flipped = list[index].parameters[2] != 0;
	return true;
}

////////////////////////////////////////////////////////////
/// Change Party Member
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandChangePartyMember() { // Code 10330
	Game_Actor* actor;
	int id;

	if (list[index].parameters[1] == 0) {
		id = list[index].parameters[2];
	} else {
		id = Game_Variables[list[index].parameters[2]];
	}

	actor = Game_Actors::GetActor(id);

	if (actor != NULL) {

		if (list[index].parameters[0] == 0) {
			// Add members
			Game_Party::AddActor(id);

		} else {
			// Remove members
			Game_Party::RemoveActor(id);
		}
	}

	Game_Map::SetNeedRefresh(true);

	// Continue
	return true;
}

////////////////////////////////////////////////////////////
/// Change Experience
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandChangeExp() { // Code 10410
	std::vector<Game_Actor*> actors = GetActors(list[index].parameters[0],
												list[index].parameters[1]);
	int value = OperateValue(
		list[index].parameters[2],
		list[index].parameters[3],
		list[index].parameters[4]
	);

	for (std::vector<Game_Actor*>::iterator i = actors.begin(); 
		 i != actors.end(); 
		 i++) {
		Game_Actor* actor = *i;
		actor->SetExp(actor->GetExp() + value);
	}

	if (list[index].parameters[5] != 0) {
		// TODO
		// Show message increase level
	} else {
		// Don't show message increase level
	}

	// Continue
	return true;
}


bool Game_Interpreter::CommandChangeLevel() { // Code 10420
	std::vector<Game_Actor*> actors = GetActors(list[index].parameters[0],
												list[index].parameters[1]);
	int value = OperateValue(
		list[index].parameters[2],
		list[index].parameters[3],
		list[index].parameters[4]
	);

	for (std::vector<Game_Actor*>::iterator i = actors.begin(); 
		 i != actors.end(); 
		 i++) {
		Game_Actor* actor = *i;
		actor->ChangeLevel(actor->GetLevel() + value);
	}

	if (list[index].parameters[5] != 0) {
		// TODO
		// Show message increase level
	} else {
		// Don't show message increase level
	}

	// Continue
	return true;
}

bool Game_Interpreter::CommandChangeParameters() { // Code 10430
	std::vector<Game_Actor*> actors = GetActors(list[index].parameters[0],
												list[index].parameters[1]);
	int value = OperateValue(
		list[index].parameters[2],
		list[index].parameters[4],
		list[index].parameters[5]
		);

	for (std::vector<Game_Actor*>::iterator i = actors.begin(); 
		 i != actors.end(); 
		 i++) {
		Game_Actor* actor = *i;
		switch (list[index].parameters[3]) {
			case 0:
				// Max HP
				actor->SetMaxHp(actor->GetMaxHp() + value);
				break;
			case 1:
				// Max MP
				actor->SetMaxSp(actor->GetMaxSp() + value);
				break;
			case 2:
				// Attack
				actor->SetAtk(actor->GetAtk() + value);
				break;
			case 3:
				// Defense
				actor->SetDef(actor->GetDef() + value);
				break;
			case 4:
				// Spirit
				actor->SetSpi(actor->GetSpi() + value);
				break;
			case 5:
				// Agility
				actor->SetAgi(actor->GetAgi() + value);
				break;
		}	
	}
	return true;
}

static int ValueOrVariable(int mode, int val) {
	switch (mode) {
		case 0:
			return val;
		case 1:
			return Game_Variables[val];
		default:
			return -1;
	}
}

bool Game_Interpreter::CommandChangeSkills() { // Code 10440
	std::vector<Game_Actor*> actors = GetActors(list[index].parameters[0],
												list[index].parameters[1]);
	bool remove = list[index].parameters[2] != 0;
	int skill_id = ValueOrVariable(list[index].parameters[3],
								   list[index].parameters[4]);

	for (std::vector<Game_Actor*>::iterator i = actors.begin(); 
		 i != actors.end(); 
		 i++) {
		Game_Actor* actor = *i;
		if (remove)
			actor->UnlearnSkill(skill_id);
		else
			actor->LearnSkill(skill_id);
	}

	return true;
}

bool Game_Interpreter::CommandChangeEquipment() { // Code 10450
	std::vector<Game_Actor*> actors = GetActors(list[index].parameters[0],
												list[index].parameters[1]);
	int item_id;
	int type;
	int slot;

	switch (list[index].parameters[2]) {
		case 0:
			item_id = ValueOrVariable(list[index].parameters[3],
									  list[index].parameters[4]);
			type = Data::items[item_id - 1].type;
			switch (type) {
				case RPG::Item::Type_weapon:
				case RPG::Item::Type_shield:
				case RPG::Item::Type_armor:
				case RPG::Item::Type_helmet:
				case RPG::Item::Type_accessory:
					slot = type - 1;
				default:
					return true;
			}
			break;
		case 1:
			item_id = 0;
			slot = list[index].parameters[3];
			break;
		default:
			return false;
	}

	for (std::vector<Game_Actor*>::iterator i = actors.begin(); 
		 i != actors.end(); 
		 i++) {
		Game_Actor* actor = *i;
		actor->ChangeEquipment(slot, item_id, false);
	}

	return true;
}

bool Game_Interpreter::CommandChangeHP() { // Code 10460
	std::vector<Game_Actor*> actors = GetActors(list[index].parameters[0],
												list[index].parameters[1]);
	bool remove = list[index].parameters[2] != 0;
	int amount = ValueOrVariable(list[index].parameters[3],
								 list[index].parameters[4]);
	bool lethal = list[index].parameters[5] != 0;

	if (remove)
		amount = -amount;

	for (std::vector<Game_Actor*>::iterator i = actors.begin(); 
		 i != actors.end(); 
		 i++) {
		Game_Actor* actor = *i;
		int hp = actor->GetHp() + amount;
		if (hp < 0)
			hp = lethal ? 0 : 1;
		actor->SetHp(hp);
	}

	return true;
}

bool Game_Interpreter::CommandChangeSP() { // Code 10470
	std::vector<Game_Actor*> actors = GetActors(list[index].parameters[0],
												list[index].parameters[1]);
	bool remove = list[index].parameters[2] != 0;
	int amount = ValueOrVariable(list[index].parameters[3],
								 list[index].parameters[4]);

	if (remove)
		amount = -amount;

	for (std::vector<Game_Actor*>::iterator i = actors.begin(); 
		 i != actors.end(); 
		 i++) {
		Game_Actor* actor = *i;
		int sp = actor->GetSp() + amount;
		if (sp < 0)
			sp = 0;
		actor->SetSp(sp);
	}

	return true;
}

bool Game_Interpreter::CommandChangeCondition() { // Code 10480
	std::vector<Game_Actor*> actors = GetActors(list[index].parameters[0],
												list[index].parameters[1]);
	bool remove = list[index].parameters[2] != 0;
	int state_id = list[index].parameters[3];

	for (std::vector<Game_Actor*>::iterator i = actors.begin(); 
		 i != actors.end(); 
		 i++) {
		Game_Actor* actor = *i;
		if (remove)
			actor->RemoveState(state_id);
		else
			actor->AddState(state_id);
	}

	return true;
}

bool Game_Interpreter::CommandFullHeal() { // Code 10490
	std::vector<Game_Actor*> actors = GetActors(list[index].parameters[0],
												list[index].parameters[1]);

	for (std::vector<Game_Actor*>::iterator i = actors.begin(); 
		 i != actors.end(); 
		 i++) {
		Game_Actor* actor = *i;
		actor->SetHp(actor->GetMaxHp());
		actor->SetSp(actor->GetMaxSp());
		actor->RemoveAllStates();
	}

	return true;
}

bool Game_Interpreter::CommandChangeHeroName() { // code 10610
	Game_Actor* actor = Game_Actors::GetActor(list[index].parameters[0]);
	actor->SetName(list[index].string);
	return true;
}

bool Game_Interpreter::CommandChangeHeroTitle() { // code 10620
	Game_Actor* actor = Game_Actors::GetActor(list[index].parameters[0]);
	actor->SetTitle(list[index].string);
	return true;
}

bool Game_Interpreter::CommandChangeSpriteAssociation() { // code 10630
	Game_Actor* actor = Game_Actors::GetActor(list[index].parameters[0]);
	const std::string &file = list[index].string;
	int idx = list[index].parameters[1];
	bool transparent = list[index].parameters[2] != 0;
	actor->SetSprite(file, idx, transparent);
	return true;
}

bool Game_Interpreter::CommandMemorizeLocation() { // code 10820
	Game_Character *player = GetCharacter(CharPlayer);
	int var_map_id = list[index].parameters[0];
	int var_x = list[index].parameters[1];
	int var_y = list[index].parameters[2];
 	Game_Variables[var_map_id] = Game_Map::GetMapId();
	Game_Variables[var_x] = player->GetX();
	Game_Variables[var_y] = player->GetY();
	return true;
}

bool Game_Interpreter::CommandRecallToLocation() { // Code 10830
	Game_Character *player = GetCharacter(CharPlayer);
	int var_map_id = list[index].parameters[0];
	int var_x = list[index].parameters[1];
	int var_y = list[index].parameters[2];
	int map_id = Game_Variables[var_map_id];
	int x = Game_Variables[var_x];
	int y = Game_Variables[var_y];

	if (map_id == Game_Map::GetMapId()) {
		player->MoveTo(x, y);
		return true;
	};

	if (Main_Data::game_player->IsTeleporting() || 
		Game_Message::visible) {
			return false;
	}

	Main_Data::game_player->ReserveTeleport(map_id, x, y);
	index++;

	return false;
}

bool Game_Interpreter::CommandStoreTerrainID() { // code 10820
	int x = ValueOrVariable(list[index].parameters[0], list[index].parameters[1]);
	int y = ValueOrVariable(list[index].parameters[0], list[index].parameters[2]);
	int var_id = list[index].parameters[3];
 	Game_Variables[var_id] = Game_Map::GetTerrainTag(x, y);
	return true;
}

bool Game_Interpreter::CommandStoreEventID() { // code 10920
	int x = ValueOrVariable(list[index].parameters[0], list[index].parameters[1]);
	int y = ValueOrVariable(list[index].parameters[0], list[index].parameters[2]);
	int var_id = list[index].parameters[3];
	std::vector<Game_Event*> events;
	Game_Map::GetEventsXY(events, x, y);
 	Game_Variables[var_id] = events.size() > 0 ? events[0]->GetId() : 0;
	return true;
}

bool Game_Interpreter::CommandPlayBGM() { // code 11510
	RPG::Music music;
	music.name = list[index].string;
	music.fadein = list[index].parameters[0];
	music.volume = list[index].parameters[1];
	music.tempo = list[index].parameters[2];
	music.balance = list[index].parameters[3];
	Game_System::BgmPlay(music);
	return true;
}

bool Game_Interpreter::CommandFadeOutBGM() { // code 11520
	int fadeout = list[index].parameters[0];
	Audio::BGM_Fade(fadeout);
	return true;
}

bool Game_Interpreter::CommandMemorizeBGM() { // code 11530
	Game_System::memorized_bgm = Game_System::current_bgm;
	return true;
}

bool Game_Interpreter::CommandPlayMemorizedBGM() { // code 11540
	Game_System::BgmPlay(Game_System::memorized_bgm);
	return true;
}

bool Game_Interpreter::CommandPlaySound() { // code 11550
	RPG::Sound sound;
	sound.name = list[index].string;
	sound.volume = list[index].parameters[0];
	sound.tempo = list[index].parameters[1];
	sound.balance = list[index].parameters[2];
	Game_System::SePlay(sound);
	return true;
}

////////////////////////////////////////////////////////////
/// Conditional Branch
////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandConditionalBranch() { // Code 12010
	bool result = false;
	int value1, value2;
	Game_Actor* actor;
	Game_Character* character;

	switch (list[index].parameters[0]) {
		case 0:
			// Switch
			result = Game_Switches[list[index].parameters[1]] == (list[index].parameters[2] == 0);
			break;
		case 1:
			// Variable
			value1 = Game_Variables[list[index].parameters[1]];
			if (list[index].parameters[2] == 0) {
				value2 = list[index].parameters[3];
			} else {
				value2 = Game_Variables[list[index].parameters[3]];
			}
			switch (list[index].parameters[4]) {
				case 0:
					// Equal to
					result = (value1 == value2);
					break;
				case 1:
					// Greater than or equal
					result = (value1 >= value2);
					break;
				case 2:
					// Less than or equal
					result = (value1 <= value2);
					break;
				case 3:
					// Greater than
					result = (value1 > value2);
					break;
				case 4:
					// Less than
					result = (value1 < value2);
					break;
				case 5:
					// Different
					result = (value1 != value2);
					break;
			}
			break;
		case 2:
			value1 = Game_System::ReadTimer(Game_System::Timer1);
			value2 = list[index].parameters[1] * DEFAULT_FPS;
			switch (list[index].parameters[2]) {
				case 0:
					result = (value1 >= value2);
					break;
				case 1:
					result = (value1 <= value2);
					break;
			}
			break;
		case 3:
			// Gold
			if (list[index].parameters[2] == 0) {
				// Greater than or equal
				result = (Game_Party::GetGold() >= list[index].parameters[1]);
			} else {
				// Less than or equal
				result = (Game_Party::GetGold() >= list[index].parameters[1]);
			}
			break;
		case 4:
			// Item
			result = (Game_Party::ItemNumber(list[index].parameters[1]) > 0);
			break;
		case 5:
			// Hero
			actor = Game_Actors::GetActor(list[index].parameters[1]);
			switch (list[index].parameters[2]) {
				case 0:
					// Is actor in party
					result = Game_Party::IsActorInParty(actor);
					break;
				case 1:
					// Name
					result = (actor->GetName() ==  list[index].string);
					break;
				case 2:
					// Higher or equal level
					result = (actor->GetLevel() >= list[index].parameters[3]);
					break;
				case 3:
					// Higher or equal HP
					//result = (actor->hp >= list[index].parameters[3]);
					break;
				case 4:
					// Can learn skill
					result = (actor->IsSkillLearned(list[index].parameters[3]));
					break;
				case 5:
					// Equipped object
					result = ( 
						(actor->GetShieldId() == list[index].parameters[3]) ||
						(actor->GetArmorId() == list[index].parameters[3]) ||
						(actor->GetHelmetId() == list[index].parameters[3]) ||
						(actor->GetAccessoryId() == list[index].parameters[3]) ||
						(actor->GetWeaponId() == list[index].parameters[3])
					);
					break;
				case 6:
					// Has state
					result = (actor->HasState(list[index].parameters[3]));
					break;
				default:
					;
			}
			break;
		case 6:
			// Orientation of char
			character = GetCharacter(list[index].parameters[3]);
			if (character != NULL) {
				switch (list[index].parameters[2]) {
					case 0:
						// Up 8
						result = (character->GetDirection() == 8);
						break;
					case 1:
						// Right 6
						result = (character->GetDirection() == 6);
						break;
					case 2:
						// Down 2
						result = (character->GetDirection() == 2);
						break;
					case 3:
						// Left 4
						result = (character->GetDirection() == 4);
						break;
				}
			}
			break;
		case 7:
			// TODO On vehicle
			break;
		case 8:
			// TODO Key decision initiated this event
			break;
		case 9:
			// TODO BGM Playing
			break;
		case 10:
			value1 = Game_System::ReadTimer(Game_System::Timer2);
			value2 = list[index].parameters[1] * DEFAULT_FPS;
			switch (list[index].parameters[2]) {
				case 0:
					result = (value1 >= value2);
					break;
				case 1:
					result = (value1 <= value2);
					break;
			}
			break;
	}

	if (result)
		return true;

	return SkipTo(ElseBranch, EndBranch);
}

////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandEraseScreen() {
	if (Game_Temp::transition_processing) return false;

	Game_Temp::transition_processing = true;
	Game_Temp::transition_erase = true;

	switch(list[index].parameters[0]) {
		case -1:
			Game_Temp::transition_type = Graphics::TransitionNone;
			return true;
		case 0:
			Game_Temp::transition_type = Graphics::TransitionFadeOut;
			return true;
		case 1:
			Game_Temp::transition_type = Graphics::TransitionRandomBlocks;
			return true;
		case 2:
			Game_Temp::transition_type = Graphics::TransitionRandomBlocksUp;
			return true;
		case 3:
			Game_Temp::transition_type = Graphics::TransitionRandomBlocksDown;
			return true;
		case 4:
			Game_Temp::transition_type = Graphics::TransitionBlindClose;
			return true;
		case 5:
			Game_Temp::transition_type = Graphics::TransitionVerticalStripesOut;
			return true;
		case 6:
			Game_Temp::transition_type = Graphics::TransitionHorizontalStripesOut;
			return true;
		case 7:
			Game_Temp::transition_type = Graphics::TransitionBorderToCenterOut;
			return true;
		case 8:
			Game_Temp::transition_type = Graphics::TransitionCenterToBorderOut;
			return true;
		case 9:
			Game_Temp::transition_type = Graphics::TransitionScrollUpOut;
			return true;
		case 10:
			Game_Temp::transition_type = Graphics::TransitionScrollDownOut;
			return true;
		case 11:
			Game_Temp::transition_type = Graphics::TransitionScrollLeftOut;
			return true;
		case 12:
			Game_Temp::transition_type = Graphics::TransitionScrollRightOut;
			return true;
		case 13:
			Game_Temp::transition_type = Graphics::TransitionVerticalDivision;
			return true;
		case 14:
			Game_Temp::transition_type = Graphics::TransitionHorizontalDivision;
			return true;
		case 15:
			Game_Temp::transition_type = Graphics::TransitionCrossDivision;
			return true;
		case 16:
			Game_Temp::transition_type = Graphics::TransitionZoomIn;
			return true;
		case 17:
			Game_Temp::transition_type = Graphics::TransitionMosaicOut;
			return true;
		case 18:
			Game_Temp::transition_type = Graphics::TransitionWaveOut;
			return true;
		case 19:
			Game_Temp::transition_type = Graphics::TransitionErase;
			return true;
		default:
			Game_Temp::transition_type = Graphics::TransitionNone;
			return true;
	}
}

////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandShowScreen() {
	if (Game_Temp::transition_processing) return false;

	Game_Temp::transition_processing = true;
	Game_Temp::transition_erase = false;

	switch(list[index].parameters[0]) {
		case -1:
			Game_Temp::transition_type = Graphics::TransitionNone;
			return true;
		case 0:
			Game_Temp::transition_type = Graphics::TransitionFadeIn;
			return true;
		case 1:
			Game_Temp::transition_type = Graphics::TransitionRandomBlocks;
			return true;
		case 2:
			Game_Temp::transition_type = Graphics::TransitionRandomBlocksUp;
			return true;
		case 3:
			Game_Temp::transition_type = Graphics::TransitionRandomBlocksDown;
			return true;
		case 4:
			Game_Temp::transition_type = Graphics::TransitionBlindOpen;
			return true;
		case 5:
			Game_Temp::transition_type = Graphics::TransitionVerticalStripesIn;
			return true;
		case 6:
			Game_Temp::transition_type = Graphics::TransitionHorizontalStripesIn;
			return true;
		case 7:
			Game_Temp::transition_type = Graphics::TransitionBorderToCenterIn;
			return true;
		case 8:
			Game_Temp::transition_type = Graphics::TransitionCenterToBorderIn;
			return true;
		case 9:
			Game_Temp::transition_type = Graphics::TransitionScrollUpIn;
			return true;
		case 10:
			Game_Temp::transition_type = Graphics::TransitionScrollDownIn;
			return true;
		case 11:
			Game_Temp::transition_type = Graphics::TransitionScrollLeftIn;
			return true;
		case 12:
			Game_Temp::transition_type = Graphics::TransitionScrollRightIn;
			return true;
		case 13:
			Game_Temp::transition_type = Graphics::TransitionVerticalCombine;
			return true;
		case 14:
			Game_Temp::transition_type = Graphics::TransitionHorizontalCombine;
			return true;
		case 15:
			Game_Temp::transition_type = Graphics::TransitionCrossCombine;
			return true;
		case 16:
			Game_Temp::transition_type = Graphics::TransitionZoomOut;
			return true;
		case 17:
			Game_Temp::transition_type = Graphics::TransitionMosaicIn;
			return true;
		case 18:
			Game_Temp::transition_type = Graphics::TransitionWaveIn;
			return true;
		case 19:
			Game_Temp::transition_type = Graphics::TransitionErase;
			return true;
		default:
			Game_Temp::transition_type = Graphics::TransitionNone;
			return true;
	}
}

////////////////////////////////////////////////////////////
bool Game_Interpreter::CommandShowPicture() { // code 11110
	int pic_id = list[index].parameters[0];
	Picture& picture = Main_Data::game_screen->GetPicture(pic_id);
	std::string& pic_name = list[index].string;
	int x = ValueOrVariable(list[index].parameters[1], list[index].parameters[2]);
	int y = ValueOrVariable(list[index].parameters[1], list[index].parameters[3]);
	bool scrolls = list[index].parameters[4] > 0;
	int magnify = list[index].parameters[5];
	int top_trans = list[index].parameters[6];
	bool use_trans = list[index].parameters[7] > 0;
	int red = list[index].parameters[8];
	int green = list[index].parameters[9];
	int blue = list[index].parameters[10];
	int saturation = list[index].parameters[11];
	int effect = list[index].parameters[12];
	int speed = list[index].parameters[13];
	int bottom_trans;

	if (Player::engine == Player::EngineRpg2k) {
		// Rpg2k does not support this option
		bottom_trans = top_trans;
	} else {
		bottom_trans = list[index].parameters[14];
	}

	picture.Show(pic_name);
	picture.UseTransparent(use_trans);
	picture.Scrolls(scrolls);

	picture.Move(x, y);
	picture.Color(red, green, blue, saturation);
	picture.Magnify(magnify);
	picture.Transparency(top_trans, bottom_trans);
	picture.Transition(0);

	switch (effect) {
		case 0:
			picture.StopEffects();
			break;
		case 1:
			picture.Rotate(speed);
			break;
		case 2:
			picture.Waver(speed);
			break;
	}

	return true;
}

bool Game_Interpreter::CommandMovePicture() { // code 11120
	int pic_id = list[index].parameters[0];
	Picture& picture = Main_Data::game_screen->GetPicture(pic_id);
	int x = ValueOrVariable(list[index].parameters[1], list[index].parameters[2]);
	int y = ValueOrVariable(list[index].parameters[1], list[index].parameters[3]);
	int magnify = list[index].parameters[5];
	int top_trans = list[index].parameters[6];
	int red = list[index].parameters[8];
	int green = list[index].parameters[9];
	int blue = list[index].parameters[10];
	int saturation = list[index].parameters[11];
	int effect = list[index].parameters[12];
	int speed = list[index].parameters[13];
	int tenths = list[index].parameters[14];
	bool wait = list[index].parameters[15] != 0;

	int bottom_trans;
	if (Player::engine == Player::EngineRpg2k) {
		// Rpg2k does not support this option
		bottom_trans = top_trans;
	} else {
		bottom_trans = list[index].parameters[16];
	}

	picture.Move(x, y);
	picture.Color(red, green, blue, saturation);
	picture.Magnify(magnify);
	picture.Transparency(top_trans, bottom_trans);
	picture.Transition(tenths);

	switch (effect) {
		case 0:
			picture.StopEffects();
			break;
		case 1:
			picture.Rotate(speed);
			break;
		case 2:
			picture.Waver(speed);
			break;
	}

	if (wait)
		wait_count = tenths * DEFAULT_FPS / 10;

	return true;
}

bool Game_Interpreter::CommandErasePicture() { // code 11130
	int pic_id = list[index].parameters[0];
	Picture& picture = Main_Data::game_screen->GetPicture(pic_id);
	picture.Erase();

	return true;
}

bool Game_Interpreter::CommandChangeSystemGraphics() { // code 10680
	Game_System::SetSystemName(list[index].string);
	return true;
}

bool Game_Interpreter::CommandChangeScreenTransitions() { // code 10690
	static const int fades[2][21] = {
		{
			Graphics::TransitionFadeOut,
			Graphics::TransitionRandomBlocksUp,
			Graphics::TransitionBorderToCenterOut,
			Graphics::TransitionCenterToBorderOut,
			Graphics::TransitionBlindClose,
			Graphics::TransitionVerticalStripesOut,
			Graphics::TransitionHorizontalStripesOut,
			Graphics::TransitionBorderToCenterOut,
			Graphics::TransitionCenterToBorderOut,
			Graphics::TransitionScrollUpOut,
			Graphics::TransitionScrollDownOut,
			Graphics::TransitionScrollLeftOut,
			Graphics::TransitionScrollRightOut,
			Graphics::TransitionVerticalDivision,
			Graphics::TransitionHorizontalDivision,
			Graphics::TransitionCrossDivision,
			Graphics::TransitionZoomIn,
			Graphics::TransitionMosaicOut,
			Graphics::TransitionWaveOut,
			Graphics::TransitionErase,
			Graphics::TransitionNone
		},
		{
			Graphics::TransitionFadeIn,
			Graphics::TransitionRandomBlocksDown,
			Graphics::TransitionBorderToCenterIn,
			Graphics::TransitionCenterToBorderIn,
			Graphics::TransitionBlindOpen,
			Graphics::TransitionVerticalStripesIn,
			Graphics::TransitionHorizontalStripesIn,
			Graphics::TransitionBorderToCenterIn,
			Graphics::TransitionCenterToBorderIn,
			Graphics::TransitionScrollUpIn,
			Graphics::TransitionScrollDownIn,
			Graphics::TransitionScrollLeftIn,
			Graphics::TransitionScrollRightIn,
			Graphics::TransitionVerticalCombine,
			Graphics::TransitionHorizontalCombine,
			Graphics::TransitionCrossCombine,
			Graphics::TransitionZoomOut,
			Graphics::TransitionMosaicIn,
			Graphics::TransitionWaveIn,
			Graphics::TransitionErase,
			Graphics::TransitionNone,
		}
	};
	int which = list[index].parameters[0];
	int trans = fades[which % 2][list[index].parameters[1]];
	Game_System::SetTransition(which, trans);
	return true;
}

bool Game_Interpreter::CommandChangeEventLocation() { // Code 10860
	int event_id = list[index].parameters[0];
	Game_Character *event = GetCharacter(event_id);
	int x = ValueOrVariable(list[index].parameters[1], list[index].parameters[2]);
	int y = ValueOrVariable(list[index].parameters[1], list[index].parameters[3]);
	event->MoveTo(x, y);
	return true;
}

bool Game_Interpreter::CommandTradeEventLocations() { // Code 10870
	int event1_id = list[index].parameters[0];
	Game_Character *event1 = GetCharacter(event1_id);
	int x1 = event1->GetX();
	int y1 = event1->GetY();
	int event2_id = list[index].parameters[1];
	Game_Character *event2 = GetCharacter(event2_id);
	int x2 = event2->GetX();
	int y2 = event2->GetY();

	event1->MoveTo(x2, y2);
	event2->MoveTo(x1, y1);

	return true;
}

bool Game_Interpreter::CommandTintScreen() { // code 11030
	Game_Screen* screen = Main_Data::game_screen;
	int r = list[index].parameters[0];
	int g = list[index].parameters[1];
	int b = list[index].parameters[2];
	int s = list[index].parameters[3];
	int tenths = list[index].parameters[4];
	bool wait = list[index].parameters[5] != 0;

	screen->TintScreen(r, g, b, s, tenths);

	if (wait)
		wait_count = tenths * DEFAULT_FPS / 10;

	return true;
}

bool Game_Interpreter::CommandFlashScreen() { // code 11040
	Game_Screen* screen = Main_Data::game_screen;
	int r = list[index].parameters[0];
	int g = list[index].parameters[1];
	int b = list[index].parameters[2];
	int s = list[index].parameters[3];
	int tenths = list[index].parameters[4];
	bool wait = list[index].parameters[5] != 0;

	switch (list[index].parameters[6]) {
		case 0:
			screen->FlashOnce(r, g, b, s, tenths);
			if (wait)
				wait_count = tenths * DEFAULT_FPS / 10;
			break;
		case 1:
			screen->FlashBegin(r, g, b, s, tenths);
			break;
		case 2:
			screen->FlashEnd();
			break;
	}

	return true;
}

bool Game_Interpreter::CommandShakeScreen() { // code 11050
	Game_Screen* screen = Main_Data::game_screen;
	int strength = list[index].parameters[0];
	int speed = list[index].parameters[1];
	int tenths = list[index].parameters[2];
	bool wait = list[index].parameters[3] != 0;

	switch (list[index].parameters[4]) {
		case 0:
			screen->ShakeOnce(strength, speed, tenths);
			if (wait)
				wait_count = tenths * DEFAULT_FPS / 10;
			break;
		case 1:
			screen->ShakeBegin(strength, speed);
			break;
		case 2:
			screen->ShakeEnd();
			break;
	}

	return true;
}

bool Game_Interpreter::CommandWeatherEffects() { // code 11070
	Game_Screen* screen = Main_Data::game_screen;
	int type = list[index].parameters[0];
	int strength = list[index].parameters[1];
	screen->Weather(type, strength);
	return true;
}

bool Game_Interpreter::CommandMessageOptions() { //code 10120
	Game_Message::background = list[index].parameters[0] == 0;
	Game_Message::position = list[index].parameters[1];
	Game_Message::fixed_position = list[index].parameters[2] != 0;
	Game_Message::dont_halt = list[index].parameters[3] != 0;
	return true;
}

bool Game_Interpreter::CommandChangeSystemBGM() { //code 10660
	RPG::Music music;
	int context = list[index].parameters[0];
	music.name = list[index].string;
	music.fadein = list[index].parameters[1];
	music.volume = list[index].parameters[2];
	music.tempo = list[index].parameters[3];
	music.balance = list[index].parameters[4];
	Game_System::SetSystemBGM(context, music);
	return true;
}

bool Game_Interpreter::CommandChangeSystemSFX() { //code 10670
	RPG::Sound sound;
	int context = list[index].parameters[0];
	sound.name = list[index].string;
	sound.volume = list[index].parameters[1];
	sound.tempo = list[index].parameters[2];
	sound.balance = list[index].parameters[3];
	Game_System::SetSystemSE(context, sound);
	return true;
}

bool Game_Interpreter::CommandTimerOperation() { // code 10230
	int timer_id = list[index].parameters[5];
	int seconds;
	bool visible, battle;

	switch (list[index].parameters[0]) {
		case 0:
			seconds = ValueOrVariable(list[index].parameters[1],
									  list[index].parameters[2]);
			Game_System::SetTimer(timer_id, seconds);
			break;
		case 1:
			visible = list[index].parameters[3] != 0;
			battle = list[index].parameters[4] != 0;
			Game_System::StartTimer(timer_id, visible, battle);
		case 2:
			Game_System::StopTimer(timer_id);
			break;
		default:
			return false;
	}
	return true;
}

bool Game_Interpreter::CommandChangePBG() { // code 11720
	const std::string& name = list[index].string;
	Game_Map::SetParallaxName(name);

	bool horz = list[index].parameters[0] != 0;
	bool horz_auto = list[index].parameters[1] != 0;
	int horz_speed = list[index].parameters[2];
	bool vert = list[index].parameters[3] != 0;
	bool vert_auto = list[index].parameters[4] != 0;
	int vert_speed = list[index].parameters[5];
	Game_Map::SetParallaxScroll(horz, vert,
								horz_auto, vert_auto,
								horz_speed, vert_speed);
	return true;
}

bool Game_Interpreter::CommandJumpToLabel() { // code 12120
	int label_id = list[index].parameters[0];

	for (int idx = 0; (size_t) idx < list.size(); idx++) {
		if (list[idx].code != Label)
			continue;
		if (list[idx].parameters[0] != label_id)
			continue;
		index = idx;
		break;
	}

	return true;
}

bool Game_Interpreter::CommandBreakLoop() { // code 12220
	return SkipTo(EndLoop, EndLoop, 0, list[index].indent - 1);
}

bool Game_Interpreter::CommandEndLoop() { // code 22210
	int indent = list[index].indent;

	for (int idx = index; idx >= 0; idx--) {
		if (list[idx].indent > indent)
			continue;
		if (list[idx].indent < indent)
			return false;
		if (list[idx].code != Loop)
			continue;
		index = idx;
		break;
	}

	return true;
}

bool Game_Interpreter::CommandEndEventProcessing() { // code 12310
	index = list.size();
	return true;
}

bool Game_Interpreter::CommandOpenShop() { // code 10720

	switch (list[index].parameters[0]) {
		case 0:
			Game_Temp::shop_buys = true;
			Game_Temp::shop_sells = true;
			break;
		case 1:
			Game_Temp::shop_buys = true;
			Game_Temp::shop_sells = false;
			break;
		case 2:
			Game_Temp::shop_buys = false;
			Game_Temp::shop_sells = true;
			break;
		default:
			return false;
	}

	Game_Temp::shop_type = list[index].parameters[1];
	Game_Temp::shop_handlers = list[index].parameters[2] != 0;

	Game_Temp::shop_goods.clear();
	std::vector<int>::const_iterator it;
	for (it = list[index].parameters.begin() + 4; it < list[index].parameters.end(); it++)
		Game_Temp::shop_goods.push_back(*it);

	Game_Temp::shop_transaction = false;
	Game_Temp::shop_calling =  true;
	SetContinuation(&Game_Interpreter::ContinuationOpenShop);
	return false;
}

bool Game_Interpreter::ContinuationOpenShop() {
	if (!Game_Temp::shop_handlers)
		return true;

	if (!SkipTo(Game_Temp::shop_transaction
				? Transaction
				: NoTransaction,
				EndShop))
		return false;
	index++;
	return true;
}

bool Game_Interpreter::CommandShowInn() { // code 10730
	int inn_type = list[index].parameters[0];
	Game_Temp::inn_price = list[index].parameters[1];
	Game_Temp::inn_handlers = list[index].parameters[2] != 0;

	Game_Message::message_waiting = true;

	Game_Message::texts.clear();

	std::ostringstream out;

	switch (inn_type) {
		case 0:
			out << Data::terms.inn_a_greeting_1
				<< " " << Game_Temp::inn_price
				<< " " << Data::terms.gold
				<< Data::terms.inn_a_greeting_2;
			Game_Message::texts.push_back(out.str());
			Game_Message::texts.push_back(Data::terms.inn_a_greeting_3);
			break;
		case 1:
			out << Data::terms.inn_b_greeting_1
				<< " " << Game_Temp::inn_price
				<< " " << Data::terms.gold
				<< Data::terms.inn_b_greeting_2;
			Game_Message::texts.push_back(out.str());
			Game_Message::texts.push_back(Data::terms.inn_b_greeting_3);
			break;
		default:
			return false;
	}

	Game_Message::choice_start = Game_Message::texts.size();

	switch (inn_type) {
		case 0:
			Game_Message::texts.push_back(Data::terms.inn_a_accept);
			Game_Message::texts.push_back(Data::terms.inn_a_cancel);
			break;
		case 1:
			Game_Message::texts.push_back(Data::terms.inn_b_accept);
			Game_Message::texts.push_back(Data::terms.inn_b_cancel);
			break;
		default:
			return false;
	}

	Game_Message::choice_max = 2;
	Game_Message::choice_disabled.reset();
	if (Game_Party::GetGold() < Game_Temp::inn_price)
		Game_Message::choice_disabled.set(0);

	Game_Temp::inn_calling =  true;
	Game_Message::choice_result = 4;

	SetContinuation(&Game_Interpreter::ContinuationShowInn);
	return false;
}

bool Game_Interpreter::ContinuationShowInn() {
	bool inn_stay = Game_Message::choice_result == 0;

	Game_Temp::inn_calling =  false;

	if (inn_stay)
		Game_Party::GainGold(-Game_Temp::inn_price);

	if (!Game_Temp::inn_handlers) {
		if (inn_stay) {
			// Full heal
			for (std::vector<Game_Actor*>::iterator i = Game_Party::GetActors().begin(); 
				 i != Game_Party::GetActors().end(); 
				 i++) {
				Game_Actor* actor = Game_Actors::GetActor((*i)->GetId());
				actor->SetHp(actor->GetMaxHp());
				actor->SetSp(actor->GetMaxSp());
				actor->RemoveAllStates();
			}
		}
		index++;
		return true;
	}

	if (!SkipTo(inn_stay ? Stay : NoStay, EndInn))
		return false;
	index++;
	return true;
}

bool Game_Interpreter::DefaultContinuation() {
	index++;
	return true;
}

bool Game_Interpreter::CommandEnterHeroName() { // code 10740
	Game_Temp::hero_name_id = list[index].parameters[0];
	Game_Temp::hero_name_charset = list[index].parameters[1];
	
	if (list[index].parameters[2] != 0)
		Game_Temp::hero_name = Game_Actors::GetActor(Game_Temp::hero_name_id)->GetName();
	else
		Game_Temp::hero_name.clear();

	Game_Temp::name_calling =  true;
	return true;
}

bool Game_Interpreter::CommandGameOver() { // code 12420
	Game_Temp::gameover =  true;
	SetContinuation(&Game_Interpreter::DefaultContinuation);
	return false;
}

bool Game_Interpreter::CommandReturntoTitleScreen() { // code 12510
	Game_Temp::to_title =  true;
	SetContinuation(&Game_Interpreter::DefaultContinuation);
	return false;
}

bool Game_Interpreter::CommandOpenSaveMenu() { // code 11910
	Game_Temp::save_calling =  true;
	SetContinuation(&Game_Interpreter::DefaultContinuation);
	return false;
}

bool Game_Interpreter::CommandOpenMainMenu() { // code 11950
	Game_Temp::menu_calling =  true;
	SetContinuation(&Game_Interpreter::DefaultContinuation);
	return false;
}

bool Game_Interpreter::CommandEnemyEncounter() { // code 10710
	Game_Temp::battle_troop_id = ValueOrVariable(list[index].parameters[0],
												 list[index].parameters[1]);
	Game_Character *player;
	switch (list[index].parameters[2]) {
		case 0:
			player = GetCharacter(CharPlayer);
			Game_Temp::battle_terrain_id = Game_Map::GetTerrainTag(player->GetX(), player->GetY());
			Game_Temp::battle_background = "";
			break;
		case 1:
			Game_Temp::battle_terrain_id = 0;
			Game_Temp::battle_background = list[index].string;
			Game_Temp::battle_formation = list[index].parameters[7];
			break;
		case 2:
			Game_Temp::battle_terrain_id = list[index].parameters[8];
			Game_Temp::battle_background = "";
			break;
		default:
			return false;
	}
	Game_Temp::battle_escape_mode = list[index].parameters[3]; // disallow, end event processing, custom handler
	Game_Temp::battle_defeat_mode = list[index].parameters[4]; // game over, custom handler
	Game_Temp::battle_first_strike = list[index].parameters[5] != 0;
	Game_Temp::battle_mode = list[index].parameters[6]; // normal, initiative, surround, back attack, pincer
	Game_Temp::battle_result = Game_Temp::BattleVictory;

	Game_Temp::battle_calling = true;
	SetContinuation(&Game_Interpreter::ContinuationEnemyEncounter);
	return false;
}

bool Game_Interpreter::ContinuationEnemyEncounter() {
	switch (Game_Temp::battle_result) {
		case Game_Temp::BattleVictory:
			return SkipTo(VictoryHandler, EndBattle);
		case Game_Temp::BattleEscape:
			switch (Game_Temp::battle_escape_mode) {
				case 0:	// disallowed - shouldn't happen
					return true;
				case 1:
					return CommandEndEventProcessing();
				case 2:
					if (!SkipTo(EscapeHandler, EndBattle))
						return false;
					index++;
					return true;
				default:
					return false;
			}
		case Game_Temp::BattleDefeat:
			switch (Game_Temp::battle_defeat_mode) {
				case 0:
					return CommandGameOver();
				case 1:
					if (!SkipTo(DefeatHandler, EndBattle))
						return false;
					index++;
					return true;
				default:
					return false;
			}
		default:
			return false;
	}
}

bool Game_Interpreter::CommandTeleportTargets() { // code 11810
	int map_id = list[index].parameters[1];

	if (list[index].parameters[0] != 0) {
		Game_System::RemoveTeleportTarget(map_id);
		return true;
	}

	int x = list[index].parameters[2];
	int y = list[index].parameters[3];
	int switch_id = (list[index].parameters[4] != 0)
		? list[index].parameters[5]
		: -1;
	Game_System::AddTeleportTarget(map_id, x, y, switch_id);
	return true;
}

bool Game_Interpreter::CommandEscapeTarget() { // code 11830
	int map_id = list[index].parameters[0];
	int x = list[index].parameters[1];
	int y = list[index].parameters[2];
	int switch_id = (list[index].parameters[3] != 0)
		? list[index].parameters[4]
		: -1;
	Game_System::SetEscapeTarget(map_id, x, y, switch_id);
	return true;
}

static int decode_int(std::vector<int>::const_iterator& it)
{
	int value = 0;

	for (;;) {
		int x = *it++;
		value <<= 7;
		value |= x & 0x7F;
		if (!(x & 0x80))
			break;
	}

	return value;
}

static const std::string decode_str(std::vector<int>::const_iterator& it)
{
	std::ostringstream out;
	int len = decode_int(it);

	for (int i = 0; i < len; i++)
		out << (char) *it++;

	return out.str();
}

static RPG::MoveCommand decode_move(std::vector<int>::const_iterator& it)
{
	RPG::MoveCommand cmd;
	cmd.command_id = *it++;

	switch (cmd.command_id) {
		case 32:	// Switch ON
		case 33:	// Switch OFF
			cmd.parameter_a = decode_int(it);
			break;
		case 34:	// Change Graphic
			cmd.parameter_string = decode_str(it);
			cmd.parameter_a = decode_int(it);
			break;
		case 35:	// Play Sound Effect
			cmd.parameter_string = decode_str(it);
			cmd.parameter_a = decode_int(it);
			cmd.parameter_b = decode_int(it);
			cmd.parameter_c = decode_int(it);
			break;
	}

	return cmd;
}

std::vector<RPG::MoveRoute*> pending;

bool Game_Interpreter::CommandMoveEvent() { // code 11330
	int event_id = list[index].parameters[0];
	Game_Character* event = GetCharacter(event_id);

	RPG::MoveRoute* route = new RPG::MoveRoute;
	int move_freq = list[index].parameters[1];
	route->repeat = list[index].parameters[2] != 0;
	route->skippable = list[index].parameters[3] != 0;

    std::vector<int>::const_iterator it;
	for (it = list[index].parameters.begin() + 4; it < list[index].parameters.end(); )
	    route->move_commands.push_back(decode_move(it));

	event->ForceMoveRoute(route, move_freq, this);
	pending.push_back(route);
	return true;
}

void Game_Interpreter::EndMoveRoute(RPG::MoveRoute* route) {
	std::vector<RPG::MoveRoute*>::iterator it;
	for (it = pending.begin(); it != pending.end(); it++)
		if (*it == route)
			break;

	if (it != pending.end())
		pending.erase(it);
}

bool Game_Interpreter::CommandFlashSprite() { // code 11320
	int event_id = list[index].parameters[0];
	Color color(list[index].parameters[1],
				list[index].parameters[2],
				list[index].parameters[3],
				list[index].parameters[4]);
	int tenths = list[index].parameters[5];
	bool wait = list[index].parameters[6] > 0;
	Game_Character* event = GetCharacter(event_id);

	Scene_Map* scene = (Scene_Map*) Scene::Find(Scene::Map);
	if (!scene)
		return true;

	Sprite_Character* sprite = scene->spriteset->FindCharacter(event);
	if (!sprite)
		return true;

	sprite->Flash(color, tenths * DEFAULT_FPS / 10);

	if (wait)
		wait_count = tenths * DEFAULT_FPS / 10;

	return true;
}

bool Game_Interpreter::CommandSpriteTransparency() { // code 11310
	bool visible = list[index].parameters[0] != 0;
	Game_Character* player = Main_Data::game_player;

	Scene_Map* scene = (Scene_Map*) Scene::Find(Scene::Map);
	if (!scene)
		return true;

	Sprite_Character* sprite = scene->spriteset->FindCharacter(player);
	if (!sprite)
		return true;

	sprite->SetVisible(visible);

	return true;
}

bool Game_Interpreter::CommandEraseEvent() { // code 12320
	if (event_id == 0)
		return true;

	tEventHash& events = Game_Map::GetEvents();
	events.erase(event_id);

	return true;
}

bool Game_Interpreter::CommandChangeMapTileset() { // code 11710
	int chipset_id = list[index].parameters[0];
	Game_Map::SetChipset(chipset_id);

	Scene_Map* scene = (Scene_Map*) Scene::Find(Scene::Map);
	if (!scene)
		return true;
	scene->spriteset->ChipsetUpdated();

	return true;
}

bool Game_Interpreter::CommandCallEvent() { // code 12330
	int event_id;
	int event_page;

	if (child_interpreter != NULL)
		return false;

	child_interpreter = new Game_Interpreter(depth + 1);

	switch (list[index].parameters[0]) {
		case 0: // Common Event
			event_id = list[index].parameters[1];
			child_interpreter->Setup(Data::commonevents[event_id - 1].event_commands, 0);
			return true;
		case 1: // Map Event
			event_id = list[index].parameters[1];
			event_page = list[index].parameters[2];
			break;
		case 2: // Indirect
			event_id = Game_Variables[list[index].parameters[1]];
			event_page = Game_Variables[list[index].parameters[2]];
			break;
		default:
			return false;
	}

	Game_Event* event = Game_Map::GetEvents()[event_id];
	RPG::EventPage& page = event->GetEvent().pages[event_page - 1];
	child_interpreter->Setup(page.event_commands, event_id);

	return true;
}

bool Game_Interpreter::CommandChangeEncounterRate() { // code 11740
	int steps = list[index].parameters[0];

	Game_Map::SetEncounterStep(steps);

	return true;
}

bool Game_Interpreter::CommandProceedWithMovement() { // code 11340
	return pending.empty();
}

bool Game_Interpreter::CommandPlayMovie() { // code 11560
	const std::string& filename = list[index].string;
	int pos_x = ValueOrVariable(list[index].parameters[0], list[index].parameters[1]);
	int pos_y = ValueOrVariable(list[index].parameters[0], list[index].parameters[2]);
	int res_x = list[index].parameters[3];
	int res_y = list[index].parameters[4];

	Main_Data::game_screen->PlayMovie(filename, pos_x, pos_y, res_x, res_y);

	return true;
}

bool Game_Interpreter::CommandChangeBattleCommands() { // code 1009
	int actor_id = list[index].parameters[1];
	Game_Actor* actor = Game_Actors::GetActor(actor_id);
	int cmd_id = list[index].parameters[2];
	bool add = list[index].parameters[3] != 0;

	actor->ChangeBattleCommands(add, cmd_id);

	return true;
}

bool Game_Interpreter::CommandKeyInputProc() { // code 11610
	int var_id = list[index].parameters[0];
	bool wait = list[index].parameters[1] != 0;
	int time_id = list[index].parameters[7];
	bool time = list[index].parameters[8] != 0;
	bool check_decision = list[index].parameters[ 3] != 0;
	bool check_cancel   = list[index].parameters[ 4] != 0;
	bool check_numbers  = list[index].parameters[ 5] != 0;
	bool check_arith    = list[index].parameters[ 6] != 0;
	bool check_shift    = list[index].parameters[ 9] != 0;
	bool check_down     = list[index].parameters[10] != 0;
	bool check_left     = list[index].parameters[11] != 0;
	bool check_right    = list[index].parameters[12] != 0;
	bool check_up       = list[index].parameters[13] != 0;
	int result = 0;

	if (check_down && Input::IsTriggered(Input::DOWN))
		result = 1;
	if (check_left && Input::IsTriggered(Input::LEFT))
		result = 2;
	if (check_right && Input::IsTriggered(Input::RIGHT))
		result = 3;
	if (check_up && Input::IsTriggered(Input::UP))
		result = 4;
	if (check_decision && Input::IsTriggered(Input::DECISION))
		result = 5;
	if (check_cancel && Input::IsTriggered(Input::CANCEL))
		result = 6;
	if (check_shift && Input::IsTriggered(Input::SHIFT))
		result = 7;
	if (check_numbers)
		for (int i = 0; i < 10; i++)
			if (Input::IsTriggered((Input::InputButton)(Input::N0 + i)))
				result = 10 + i;
	if (check_arith)
		for (int i = 0; i < 5; i++)
			if (Input::IsTriggered((Input::InputButton)(Input::PLUS + i)))
				result = 20 + i;

	Game_Variables[var_id] = result;

	if (!wait)
		return true;

	button_timer++;

	if (result == 0)
		return false;

	if (time)
		Game_Variables[time_id] = button_timer;

	button_timer = 0;

	return true;
}

bool Game_Interpreter::CommandChangeVehicleGraphic() { // code 10650
	Game_Vehicle::Type vehicle_id = (Game_Vehicle::Type) list[index].parameters[0];
	Game_Vehicle* vehicle = Game_Map::GetVehicle(vehicle_id);
	const std::string& name = list[index].string;
	int vehicle_index = list[index].parameters[1];

	vehicle->SetGraphic(name, vehicle_index);

	return true;
}

bool Game_Interpreter::CommandEnterExitVehicle() { // code 10840
	Main_Data::game_player->GetOnOffVehicle();

	return true;
}

bool Game_Interpreter::CommandSetVehicleLocation() { // code 10850
	Game_Vehicle::Type vehicle_id = (Game_Vehicle::Type) list[index].parameters[0];
	Game_Vehicle* vehicle = Game_Map::GetVehicle(vehicle_id);
	int map_id = ValueOrVariable(list[index].parameters[1], list[index].parameters[2]);
	int x = ValueOrVariable(list[index].parameters[1], list[index].parameters[3]);
	int y = ValueOrVariable(list[index].parameters[1], list[index].parameters[4]);

	vehicle->SetPosition(map_id, x, y);

	return true;
}

