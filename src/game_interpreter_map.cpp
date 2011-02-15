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
#include "game_interpreter_map.h"

///////////////////////////////////////////////////////////
Game_Interpreter_Map::Game_Interpreter_Map(int depth, bool main_flag) :
	Game_Interpreter(depth, main_flag) {
}

Game_Interpreter_Map::~Game_Interpreter_Map() {
	std::vector<pending_move_route>::iterator it;
	for (it = pending.begin(); it != pending.end(); it++) {
		(*it).second->DetachMoveRouteOwner(this);
	}
}

///////////////////////////////////////////////////////////
int Game_Interpreter_Map::DecodeInt(std::vector<int>::const_iterator& it) {
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

const std::string Game_Interpreter_Map::DecodeString(std::vector<int>::const_iterator& it)
{
	std::ostringstream out;
	int len = DecodeInt(it);

	for (int i = 0; i < len; i++)
		out << (char) *it++;

	return out.str();
}

RPG::MoveCommand Game_Interpreter_Map::DecodeMove(std::vector<int>::const_iterator& it)
{
	RPG::MoveCommand cmd;
	cmd.command_id = *it++;

	switch (cmd.command_id) {
	case 32:	// Switch ON
	case 33:	// Switch OFF
		cmd.parameter_a = DecodeInt(it);
		break;
	case 34:	// Change Graphic
		cmd.parameter_string = DecodeString(it);
		cmd.parameter_a = DecodeInt(it);
		break;
	case 35:	// Play Sound Effect
		cmd.parameter_string = DecodeString(it);
		cmd.parameter_a = DecodeInt(it);
		cmd.parameter_b = DecodeInt(it);
		cmd.parameter_c = DecodeInt(it);
		break;
	}

	return cmd;
}

///////////////////////////////////////////////////////////
void Game_Interpreter_Map::EndMoveRoute(RPG::MoveRoute* route) {
	std::vector<pending_move_route>::iterator it;
	for (it = pending.begin(); it != pending.end(); it++) {
		if ((*it).first == route) {
			break;
		}
	}

	if (it != pending.end()) {
		pending.erase(it);
	}
}

////////////////////////////////////////////////////////////
/// Execute Command
////////////////////////////////////////////////////////////
bool Game_Interpreter_Map::ExecuteCommand() {
	
	if (index >= list.size()) {
		CommandEnd();
		return true;
	}
	
	switch (list[index].code) {
		case MessageOptions: 
			return CommandMessageOptions();
		case ChangeExp: 
			return CommandChangeExp();
		case ChangeParameters:
			return CommandChangeParameters();
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
		case MemorizeBGM:
			return CommandMemorizeBGM();
		case PlayMemorizedBGM:
			return CommandPlayMemorizedBGM();
		case ChangeSystemBGM:
			return CommandChangeSystemBGM();
		case ChangeSystemSFX:
			return CommandChangeSystemSFX();
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
		case ShowPicture:
			return CommandShowPicture();
		case MovePicture:
			return CommandMovePicture();
		case ErasePicture:
			return CommandErasePicture();
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
		case MoveEvent:
			return CommandMoveEvent();
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
		case ReturntoTitleScreen:
			return CommandReturnToTitleScreen();
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
		case ProceedWithMovement: // FIXME: Causes a hang
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
		case TileSubstitution:
			return CommandTileSubstitution();
		case PanScreen:
			return CommandPanScreen();
		case SimulatedAttack:
			return CommandSimulatedAttack();
		case ShowBattleAnimation:
			return CommandShowBattleAnimation();
		case ConditionalBranch: 
			return CommandConditionalBranch();
		case ElseBranch:
			return SkipTo(EndBranch);
		case EndBranch:
			return true;
		default:
			return Game_Interpreter::ExecuteCommand();
	}
}

///////////////////////////////////////////////////////////
/// Commands
///////////////////////////////////////////////////////////
bool Game_Interpreter_Map::CommandMessageOptions() { //code 10120
	Game_Message::background = list[index].parameters[0] == 0;
	Game_Message::position = list[index].parameters[1];
	Game_Message::fixed_position = list[index].parameters[2] == 0;
	Game_Message::dont_halt = list[index].parameters[3] != 0;
	return true;
}


bool Game_Interpreter_Map::CommandChangeExp() { // Code 10410
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

bool Game_Interpreter_Map::CommandChangeParameters() { // Code 10430
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

bool Game_Interpreter_Map::CommandChangeHeroName() { // code 10610
	Game_Actor* actor = Game_Actors::GetActor(list[index].parameters[0]);
	actor->SetName(list[index].string);
	return true;
}

bool Game_Interpreter_Map::CommandChangeHeroTitle() { // code 10620
	Game_Actor* actor = Game_Actors::GetActor(list[index].parameters[0]);
	actor->SetTitle(list[index].string);
	return true;
}

bool Game_Interpreter_Map::CommandChangeSpriteAssociation() { // code 10630
	Game_Actor* actor = Game_Actors::GetActor(list[index].parameters[0]);
	const std::string &file = list[index].string;
	int idx = list[index].parameters[1];
	bool transparent = list[index].parameters[2] != 0;
	actor->SetSprite(file, idx, transparent);
	return true;
}

bool Game_Interpreter_Map::CommandMemorizeLocation() { // code 10820
	Game_Character *player = GetCharacter(CharPlayer);
	int var_map_id = list[index].parameters[0];
	int var_x = list[index].parameters[1];
	int var_y = list[index].parameters[2];
 	Game_Variables[var_map_id] = Game_Map::GetMapId();
	Game_Variables[var_x] = player->GetX();
	Game_Variables[var_y] = player->GetY();
	return true;
}

bool Game_Interpreter_Map::CommandRecallToLocation() { // Code 10830
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

bool Game_Interpreter_Map::CommandStoreTerrainID() { // code 10820
	int x = ValueOrVariable(list[index].parameters[0], list[index].parameters[1]);
	int y = ValueOrVariable(list[index].parameters[0], list[index].parameters[2]);
	int var_id = list[index].parameters[3];
 	Game_Variables[var_id] = Game_Map::GetTerrainTag(x, y);
	return true;
}

bool Game_Interpreter_Map::CommandStoreEventID() { // code 10920
	int x = ValueOrVariable(list[index].parameters[0], list[index].parameters[1]);
	int y = ValueOrVariable(list[index].parameters[0], list[index].parameters[2]);
	int var_id = list[index].parameters[3];
	std::vector<Game_Event*> events;
	Game_Map::GetEventsXY(events, x, y);
 	Game_Variables[var_id] = events.size() > 0 ? events[0]->GetId() : 0;
	return true;
}

bool Game_Interpreter_Map::CommandMemorizeBGM() { // code 11530
	Game_System::memorized_bgm = Game_System::current_bgm;
	return true;
}

bool Game_Interpreter_Map::CommandPlayMemorizedBGM() { // code 11540
	Game_System::BgmPlay(Game_System::memorized_bgm);
	return true;
}

bool Game_Interpreter_Map::CommandChangeSystemBGM() { //code 10660
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

bool Game_Interpreter_Map::CommandChangeSystemSFX() { //code 10670
	RPG::Sound sound;
	int context = list[index].parameters[0];
	sound.name = list[index].string;
	sound.volume = list[index].parameters[1];
	sound.tempo = list[index].parameters[2];
	sound.balance = list[index].parameters[3];
	Game_System::SetSystemSE(context, sound);
	return true;
}

bool Game_Interpreter_Map::CommandChangeSaveAccess() { // code 11930
	Game_System::save_disabled = list[index].parameters[0] == 0;
	return true;
}

bool Game_Interpreter_Map::CommandChangeTeleportAccess() { // code 11820
	Game_System::teleport_disabled = list[index].parameters[0] == 0;
	return true;
}

bool Game_Interpreter_Map::CommandChangeEscapeAccess() { // code 11840
	Game_System::escape_disabled = list[index].parameters[0] == 0;
	return true;
}

bool Game_Interpreter_Map::CommandChangeMainMenuAccess() { // code 11960
	Game_System::main_menu_disabled = list[index].parameters[0] == 0;
	return true;
}

bool Game_Interpreter_Map::CommandChangeActorFace() {
	Game_Actor* actor = Game_Actors::GetActor(list[index].parameters[0]);
	if (actor != NULL) {
		actor->SetFace(list[index].string, list[index].parameters[1]);
		return true;
	}
	return false;
}

bool Game_Interpreter_Map::CommandTeleport() { // Code 10810
	// TODO: if in battle return true
	if (Main_Data::game_player->IsTeleporting()) {
			return false;
	}

	int map_id = list[index].parameters[0];
	int x = list[index].parameters[1];
	int y = list[index].parameters[2];
	// FIXME: RPG2K3 => facing direction = list[index].parameters[3]

	Main_Data::game_player->ReserveTeleport(map_id, x, y);
	teleport_pending = true;

	if (Game_Message::visible) {
		Game_Message::visible = false;
		Game_Message::FullClear();
	}

	index++;

	return false;
}

bool Game_Interpreter_Map::CommandEraseScreen() {
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

bool Game_Interpreter_Map::CommandShowScreen() {
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

bool Game_Interpreter_Map::CommandShowPicture() { // code 11110
	int pic_id = list[index].parameters[0];
	Picture* picture = Main_Data::game_screen->GetPicture(pic_id);
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

	picture->Show(pic_name);
	picture->UseTransparent(use_trans);
	picture->Scrolls(scrolls);

	picture->Move(x, y);
	picture->Color(red, green, blue, saturation);
	picture->Magnify(magnify);
	picture->Transparency(top_trans, bottom_trans);
	picture->Transition(0);

	switch (effect) {
		case 0:
			picture->StopEffects();
			break;
		case 1:
			picture->Rotate(speed);
			break;
		case 2:
			picture->Waver(speed);
			break;
	}

	return true;
}

bool Game_Interpreter_Map::CommandMovePicture() { // code 11120
	int pic_id = list[index].parameters[0];
	Picture* picture = Main_Data::game_screen->GetPicture(pic_id);
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

	picture->Move(x, y);
	picture->Color(red, green, blue, saturation);
	picture->Magnify(magnify);
	picture->Transparency(top_trans, bottom_trans);
	picture->Transition(tenths);

	switch (effect) {
		case 0:
			picture->StopEffects();
			break;
		case 1:
			picture->Rotate(speed);
			break;
		case 2:
			picture->Waver(speed);
			break;
	}

	if (wait)
		wait_count = tenths * DEFAULT_FPS / 10;

	return true;
}

bool Game_Interpreter_Map::CommandErasePicture() { // code 11130
	int pic_id = list[index].parameters[0];
	Picture* picture = Main_Data::game_screen->GetPicture(pic_id);
	picture->Erase();

	return true;
}

bool Game_Interpreter_Map::CommandWeatherEffects() { // code 11070
	Game_Screen* screen = Main_Data::game_screen;
	int type = list[index].parameters[0];
	int strength = list[index].parameters[1];
	screen->Weather(type, strength);
	return true;
}

bool Game_Interpreter_Map::CommandChangeSystemGraphics() { // code 10680
	Game_System::SetSystemName(list[index].string);
	return true;
}

bool Game_Interpreter_Map::CommandChangeScreenTransitions() { // code 10690
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

bool Game_Interpreter_Map::CommandChangeEventLocation() { // Code 10860
	int event_id = list[index].parameters[0];
	Game_Character *event = GetCharacter(event_id);
	int x = ValueOrVariable(list[index].parameters[1], list[index].parameters[2]);
	int y = ValueOrVariable(list[index].parameters[1], list[index].parameters[3]);
	event->MoveTo(x, y);
	return true;
}

bool Game_Interpreter_Map::CommandTradeEventLocations() { // Code 10870
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

bool Game_Interpreter_Map::CommandTimerOperation() { // code 10230
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

bool Game_Interpreter_Map::CommandChangePBG() { // code 11720
	const std::string& name = list[index].string;
	Game_Map::SetParallaxName(name);

	bool horz = list[index].parameters[0] != 0;
	bool vert = list[index].parameters[1] != 0;
	bool horz_auto = list[index].parameters[2] != 0;
	int horz_speed = list[index].parameters[3];
	bool vert_auto = list[index].parameters[4] != 0;
	int vert_speed = list[index].parameters[5];
	Game_Map::SetParallaxScroll(horz, vert,
								horz_auto, vert_auto,
								horz_speed, vert_speed);
	return true;
}

bool Game_Interpreter_Map::CommandJumpToLabel() { // code 12120
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

bool Game_Interpreter_Map::CommandBreakLoop() { // code 12220
	return SkipTo(EndLoop, EndLoop, 0, list[index].indent - 1);
}

bool Game_Interpreter_Map::CommandEndLoop() { // code 22210
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

bool Game_Interpreter_Map::CommandMoveEvent() { // code 11330
	int event_id = list[index].parameters[0];
	Game_Character* event = GetCharacter(event_id);

	RPG::MoveRoute* route = new RPG::MoveRoute;
	int move_freq = list[index].parameters[1];
	route->repeat = list[index].parameters[2] != 0;
	route->skippable = list[index].parameters[3] != 0;

	std::vector<int>::const_iterator it;
	for (it = list[index].parameters.begin() + 4; it < list[index].parameters.end(); )
		route->move_commands.push_back(DecodeMove(it));

	event->ForceMoveRoute(route, move_freq, this);
	pending.push_back(pending_move_route(route, event));
	return true;
}

bool Game_Interpreter_Map::CommandOpenShop() { // code 10720

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
	CloseMessageWindow();
	Game_Temp::shop_calling = true;
	SetContinuation(static_cast<bool (Game_Interpreter::*)()>(&Game_Interpreter_Map::ContinuationOpenShop));
	return false;
}

bool Game_Interpreter_Map::ContinuationOpenShop() {
	if (!Game_Temp::shop_handlers) {
		index++;
		return true;
	}

	if (!SkipTo(Game_Temp::shop_transaction
				? Transaction
				: NoTransaction,
				EndShop)) {
		return false;
	}

	index++;
	return true;
}

bool Game_Interpreter_Map::CommandShowInn() { // code 10730
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

	CloseMessageWindow();
	Game_Temp::inn_calling = true;
	Game_Message::choice_result = 4;

	SetContinuation(static_cast<bool (Game_Interpreter::*)()>(&Game_Interpreter_Map::ContinuationShowInn));
	return false;
}

bool Game_Interpreter_Map::ContinuationShowInn() {
	bool inn_stay = Game_Message::choice_result == 0;

	Game_Temp::inn_calling = false;

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

bool Game_Interpreter_Map::CommandEnterHeroName() { // code 10740
	Game_Temp::hero_name_id = list[index].parameters[0];
	Game_Temp::hero_name_charset = list[index].parameters[1];
	
	if (list[index].parameters[2] != 0)
		Game_Temp::hero_name = Game_Actors::GetActor(Game_Temp::hero_name_id)->GetName();
	else
		Game_Temp::hero_name.clear();

	CloseMessageWindow();
	Game_Temp::name_calling = true;
	return true;
}

bool Game_Interpreter_Map::CommandReturnToTitleScreen() { // code 12510
	CloseMessageWindow();
	Game_Temp::to_title = true;
	SetContinuation(&Game_Interpreter::DefaultContinuation);
	return false;
}

bool Game_Interpreter_Map::CommandOpenSaveMenu() { // code 11910
	CloseMessageWindow();
	Game_Temp::save_calling = true;
	SetContinuation(&Game_Interpreter::DefaultContinuation);
	return false;
}

bool Game_Interpreter_Map::CommandOpenMainMenu() { // code 11950
	CloseMessageWindow();
	Game_Temp::menu_calling = true;
	SetContinuation(&Game_Interpreter::DefaultContinuation);
	return false;
}

bool Game_Interpreter_Map::CommandEnemyEncounter() { // code 10710
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
			if (Player::engine == Player::EngineRpg2k3) {
				Game_Temp::battle_formation = list[index].parameters[7];
			}
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

	if (Player::engine == Player::EngineRpg2k3)
		Game_Temp::battle_mode = list[index].parameters[6]; // normal, initiative, surround, back attack, pincer
	else
		Game_Temp::battle_mode = 0;

	Game_Temp::battle_result = Game_Temp::BattleVictory;

	CloseMessageWindow();
	Game_Temp::battle_calling = true;

	SetContinuation(static_cast<bool (Game_Interpreter::*)()>(&Game_Interpreter_Map::ContinuationEnemyEncounter));
	return false;
}

bool Game_Interpreter_Map::ContinuationEnemyEncounter() {
	switch (Game_Temp::battle_result) {
		case Game_Temp::BattleVictory:
			if (!SkipTo(VictoryHandler, EndBattle)) {
				// Was an event battle with no handlers
				index++;
				return false;
			}
			index++;
			return true;
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
		case Game_Temp::BattleAbort:
			if (!SkipTo(EndBattle))
				return false;
			index++;
			return true;
		default:
			return false;
	}
}

bool Game_Interpreter_Map::CommandTeleportTargets() { // code 11810
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

bool Game_Interpreter_Map::CommandEscapeTarget() { // code 11830
	int map_id = list[index].parameters[0];
	int x = list[index].parameters[1];
	int y = list[index].parameters[2];
	int switch_id = (list[index].parameters[3] != 0)
		? list[index].parameters[4]
		: -1;
	Game_System::SetEscapeTarget(map_id, x, y, switch_id);
	return true;
}

bool Game_Interpreter_Map::CommandSpriteTransparency() { // code 11310
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

bool Game_Interpreter_Map::CommandFlashSprite() { // code 11320
	int event_id = list[index].parameters[0];
	Color color(list[index].parameters[1] << 3,
				list[index].parameters[2] << 3,
				list[index].parameters[3] << 3,
				list[index].parameters[4] << 3);
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

bool Game_Interpreter_Map::CommandEraseEvent() { // code 12320
	if (event_id == 0)
		return true;

	tEventHash& events = Game_Map::GetEvents();
	events[event_id]->SetDisabled(true);

	return true;
}

bool Game_Interpreter_Map::CommandChangeMapTileset() { // code 11710
	int chipset_id = list[index].parameters[0];
	Game_Map::SetChipset(chipset_id);

	Scene_Map* scene = (Scene_Map*) Scene::Find(Scene::Map);

	if (!scene)
		return true;

	scene->spriteset->ChipsetUpdated();

	return true;
}

bool Game_Interpreter_Map::CommandCallEvent() { // code 12330
	int event_id;
	int event_page;

	if (child_interpreter != NULL)
		return false;

	child_interpreter = new Game_Interpreter_Map(depth + 1);

	switch (list[index].parameters[0]) {
		case 0: // Common Event
			event_id = list[index].parameters[1];
			child_interpreter->Setup(Data::commonevents[event_id - 1].event_commands, 0, Data::commonevents[event_id - 1].ID, -2);
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

	Game_Event* event = Game_Map::GetEvents().find(event_id)->second;
	RPG::EventPage& page = event->GetEvent().pages[event_page - 1];
	child_interpreter->Setup(page.event_commands, event_id, event->GetX(), event->GetY());

	return true;
}

bool Game_Interpreter_Map::CommandChangeEncounterRate() { // code 11740
	int steps = list[index].parameters[0];

	Game_Map::SetEncounterStep(steps);

	return true;
}

bool Game_Interpreter_Map::CommandProceedWithMovement() { // code 11340
	return pending.empty();
}

bool Game_Interpreter_Map::CommandPlayMovie() { // code 11560
	const std::string& filename = list[index].string;
	int pos_x = ValueOrVariable(list[index].parameters[0], list[index].parameters[1]);
	int pos_y = ValueOrVariable(list[index].parameters[0], list[index].parameters[2]);
	int res_x = list[index].parameters[3];
	int res_y = list[index].parameters[4];

	Main_Data::game_screen->PlayMovie(filename, pos_x, pos_y, res_x, res_y);

	return true;
}

bool Game_Interpreter_Map::CommandChangeBattleCommands() { // code 1009
	int actor_id = list[index].parameters[1];
	Game_Actor* actor = Game_Actors::GetActor(actor_id);
	int cmd_id = list[index].parameters[2];
	bool add = list[index].parameters[3] != 0;

	actor->ChangeBattleCommands(add, cmd_id);

	return true;
}

bool Game_Interpreter_Map::CommandKeyInputProc() { // code 11610
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

bool Game_Interpreter_Map::CommandChangeVehicleGraphic() { // code 10650
	Game_Vehicle::Type vehicle_id = (Game_Vehicle::Type) list[index].parameters[0];
	Game_Vehicle* vehicle = Game_Map::GetVehicle(vehicle_id);
	const std::string& name = list[index].string;
	int vehicle_index = list[index].parameters[1];

	vehicle->SetGraphic(name, vehicle_index);

	return true;
}

bool Game_Interpreter_Map::CommandEnterExitVehicle() { // code 10840
	Main_Data::game_player->GetOnOffVehicle();

	return true;
}

bool Game_Interpreter_Map::CommandSetVehicleLocation() { // code 10850
	Game_Vehicle::Type vehicle_id = (Game_Vehicle::Type) list[index].parameters[0];
	Game_Vehicle* vehicle = Game_Map::GetVehicle(vehicle_id);
	int map_id = ValueOrVariable(list[index].parameters[1], list[index].parameters[2]);
	int x = ValueOrVariable(list[index].parameters[1], list[index].parameters[3]);
	int y = ValueOrVariable(list[index].parameters[1], list[index].parameters[4]);

	vehicle->SetPosition(map_id, x, y);

	return true;
}

bool Game_Interpreter_Map::CommandTileSubstitution() { // code 11750
	bool upper = list[index].parameters[0] != 0;
	int old_id = list[index].parameters[1];
	int new_id = list[index].parameters[2];
	Scene_Map* scene = (Scene_Map*) Scene::Find(Scene::Map);
	if (!scene)
		return true;

	if (upper)
		scene->spriteset->SubstituteUp(old_id, new_id);
	else
		scene->spriteset->SubstituteDown(old_id, new_id);

	return true;
}

bool Game_Interpreter_Map::CommandPanScreen() { // code 11060
	int direction;
	int distance;
	int speed;
	bool wait = false;

	if (active)
		return !Game_Map::IsPanWaiting();

	switch (list[index].parameters[0]) {
	case 0: // Lock
		Game_Map::LockPan();
		break;
	case 1: // Unlock
		Game_Map::UnlockPan();
		break;
	case 2: // Pan
		direction = list[index].parameters[1];
		distance = list[index].parameters[2];
		speed = list[index].parameters[3];
		wait = list[index].parameters[4] != 0;
		Game_Map::StartPan(direction, distance, speed, wait);
		break;
	case 3: // Reset
		speed = list[index].parameters[3];
		wait = list[index].parameters[4] != 0;
		Game_Map::ResetPan(speed, wait);
		break;
	}

	return !wait;
}

bool Game_Interpreter_Map::CommandSimulatedAttack() { // code 10500
	std::vector<Game_Actor*> actors = GetActors(list[index].parameters[0],
												list[index].parameters[1]);
	int atk = list[index].parameters[2];
	int def = list[index].parameters[3];
	int spi = list[index].parameters[4];
	int var = list[index].parameters[5];

	for (std::vector<Game_Actor*>::iterator i = actors.begin(); 
		 i != actors.end(); 
		 i++) {
		Game_Actor* actor = *i;
		int result = atk;
		result -= (actor->GetDef() * def) / 400;
		result -= (actor->GetSpi() * spi) / 800;
		if (var != 0) {
			int rperc = var * 5;
			int rval = rand() % (2 * rperc) - rperc;
			result += result * rval / 100;
		}

		result = std::max(0, result);

		int hp = actor->GetHp() - result;
		hp = std::max(0, hp);
		actor->SetHp(hp);

		if (list[index].parameters[6] != 0)
			Game_Variables[list[index].parameters[7]] = result;
	}

	return true;
}

bool Game_Interpreter_Map::CommandShowBattleAnimation() { // code 11210
	if (active)
		return !Main_Data::game_screen->IsBattleAnimationWaiting();

	int animation_id = list[index].parameters[0];
	int event_id = list[index].parameters[1];
	Game_Character *event = GetCharacter(event_id);
	bool wait = list[index].parameters[2] > 0;
	bool global = list[index].parameters[3] > 0;

	Main_Data::game_screen->ShowBattleAnimation(animation_id, event, global);

	return !wait;
}

////////////////////////////////////////////////////////////
/// Conditional Branch
////////////////////////////////////////////////////////////
bool Game_Interpreter_Map::CommandConditionalBranch() { // Code 12010
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
				result = (Game_Party::GetGold() <= list[index].parameters[1]);
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
					result = (actor->GetName() == list[index].string);
					break;
				case 2:
					// Higher or equal level
					result = (actor->GetLevel() >= list[index].parameters[3]);
					break;
				case 3:
					// Higher or equal HP
					result = (actor->GetHp() >= list[index].parameters[3]);
					break;
				case 4:
					// Is skill learned
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
			character = GetCharacter(list[index].parameters[1]);
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

