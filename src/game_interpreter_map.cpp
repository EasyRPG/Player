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

// Headers
#include <boost/bind.hpp>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include "async_handler.h"
#include "audio.h"
#include "game_map.h"
#include "game_battle.h"
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
#include "game_targets.h"
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
#include "reader_util.h"
#include "filefinder.h"
#include "reader_lcf.h"

Game_Interpreter_Map::Game_Interpreter_Map(int depth, bool main_flag) :
	Game_Interpreter(depth, main_flag) {
}

bool Game_Interpreter_Map::SetupFromSave(const std::vector<RPG::SaveEventCommands>& save, int _event_id, int _index) {
	if (_index < (int)save.size()) {
		map_id = Game_Map::GetMapId();
		event_id = _event_id;
		list = save[_index].commands;
		index = save[_index].current_command;

		child_interpreter.reset(new Game_Interpreter_Map());
		bool result = static_cast<Game_Interpreter_Map*>(child_interpreter.get())->SetupFromSave(save, _event_id, _index + 1);
		if (!result) {
			child_interpreter.reset();
		}
		return true;
	}
	return false;
}

// Taken from readers because a kitten is killed when reader_structs is included
static int GetEventCommandSize(const std::vector<RPG::EventCommand>& commands) {
	std::vector<RPG::EventCommand>::const_iterator it;

	int result = 0;
	for (it = commands.begin(); it != commands.end(); ++it) {
		result += LcfReader::IntSize(it->code);
		result += LcfReader::IntSize(it->indent);
		result += LcfReader::IntSize(it->string.size());
		result += ReaderUtil::Recode(it->string, Player::encoding).size();

		int count = it->parameters.size();
		result += LcfReader::IntSize(count);
		for (int i = 0; i < count; i++)
			result += LcfReader::IntSize(it->parameters[i]);
	}
	result += 4; // No idea why but then it fits

	return result;
}

std::vector<RPG::SaveEventCommands> Game_Interpreter_Map::GetSaveData() const {
	std::vector<RPG::SaveEventCommands> save;

	const Game_Interpreter_Map* save_interpreter = this;

	int i = 1;

	if (save_interpreter->list.empty()) {
		return save;
	}

	while (save_interpreter != NULL) {
		RPG::SaveEventCommands save_commands;
		save_commands.commands = save_interpreter->list;
		save_commands.current_command = save_interpreter->index;
		save_commands.commands_size = GetEventCommandSize(save_commands.commands);
		save_commands.ID = i++;
		save.push_back(save_commands);
		save_interpreter = static_cast<Game_Interpreter_Map*>(save_interpreter->child_interpreter.get());
	}

	save.back().ID = event_id;

	save.back().current_command++;

	return save;
}

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

	std::string result = ReaderUtil::Recode(out.str(), Player::encoding);

	return result;
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

/**
 * Execute Command.
 */
bool Game_Interpreter_Map::ExecuteCommand() {
	if (index >= list.size()) {
		return CommandEnd();
	}

	RPG::EventCommand const& com = list[index];

	switch (com.code) {
		case Cmd::MessageOptions:
			return CommandMessageOptions(com);
		case Cmd::ChangeExp:
			return CommandChangeExp(com);
		case Cmd::ChangeParameters:
			return CommandChangeParameters(com);
		case Cmd::ChangeHeroName:
			return CommandChangeHeroName(com);
		case Cmd::ChangeHeroTitle:
			return CommandChangeHeroTitle(com);
		case Cmd::ChangeSpriteAssociation:
			return CommandChangeSpriteAssociation(com);
		case Cmd::MemorizeLocation:
			return CommandMemorizeLocation(com);
		case Cmd::RecallToLocation:
			return CommandRecallToLocation(com);
		case Cmd::StoreTerrainID:
			return CommandStoreTerrainID(com);
		case Cmd::StoreEventID:
			return CommandStoreEventID(com);
		case Cmd::MemorizeBGM:
			return CommandMemorizeBGM(com);
		case Cmd::PlayMemorizedBGM:
			return CommandPlayMemorizedBGM(com);
		case Cmd::ChangeSystemBGM:
			return CommandChangeSystemBGM(com);
		case Cmd::ChangeSystemSFX:
			return CommandChangeSystemSFX(com);
		case Cmd::ChangeSaveAccess:
			return CommandChangeSaveAccess(com);
		case Cmd::ChangeTeleportAccess:
			return CommandChangeTeleportAccess(com);
		case Cmd::ChangeEscapeAccess:
			return CommandChangeEscapeAccess(com);
		case Cmd::ChangeMainMenuAccess:
			return CommandChangeMainMenuAccess(com);
		case Cmd::ChangeActorFace:
			return CommandChangeActorFace(com);
		case Cmd::Teleport:
			return CommandTeleport(com);
		case Cmd::EraseScreen:
			return CommandEraseScreen(com);
		case Cmd::ShowScreen:
			return CommandShowScreen(com);
		case Cmd::ShowPicture:
			return CommandShowPicture(com);
		case Cmd::MovePicture:
			return CommandMovePicture(com);
		case Cmd::ErasePicture:
			return CommandErasePicture(com);
		case Cmd::WeatherEffects:
			return CommandWeatherEffects(com);
		case Cmd::ChangeSystemGraphics:
			return CommandChangeSystemGraphics(com);
		case Cmd::ChangeScreenTransitions:
			return CommandChangeScreenTransitions(com);
		case Cmd::ChangeEventLocation:
			return CommandChangeEventLocation(com);
		case Cmd::TradeEventLocations:
			return CommandTradeEventLocations(com);
		case Cmd::TimerOperation:
			return CommandTimerOperation(com);
		case Cmd::ChangePBG:
			return CommandChangePBG(com);
		case Cmd::Label:
			return true;
		case Cmd::JumpToLabel:
			return CommandJumpToLabel(com);
		case Cmd::Loop:
			return true;
		case Cmd::BreakLoop:
			return CommandBreakLoop(com);
		case Cmd::EndLoop:
			return CommandEndLoop(com);
		case Cmd::MoveEvent:
			return CommandMoveEvent(com);
		case Cmd::OpenShop:
			return CommandOpenShop(com);
		case Cmd::Transaction:
		case Cmd::NoTransaction:
			return SkipTo(Cmd::EndShop);
		case Cmd::EndShop:
			return true;
		case Cmd::ShowInn:
			return CommandShowInn(com);
		case Cmd::Stay:
		case Cmd::NoStay:
			return SkipTo(Cmd::EndInn);
		case Cmd::EndInn:
			return true;
		case Cmd::EnterHeroName:
			return CommandEnterHeroName(com);
		case Cmd::ReturntoTitleScreen:
			return CommandReturnToTitleScreen(com);
		case Cmd::OpenSaveMenu:
			return CommandOpenSaveMenu(com);
		case Cmd::OpenMainMenu:
			return CommandOpenMainMenu(com);
		case Cmd::EnemyEncounter:
			return CommandEnemyEncounter(com);
		case Cmd::VictoryHandler:
		case Cmd::EscapeHandler:
		case Cmd::DefeatHandler:
			return SkipTo(Cmd::EndBattle);
		case Cmd::EndBattle:
			return true;
		case Cmd::TeleportTargets:
			return CommandTeleportTargets(com);
		case Cmd::EscapeTarget:
			return CommandEscapeTarget(com);
		case Cmd::SpriteTransparency:
			return CommandSpriteTransparency(com);
		case Cmd::FlashSprite:
			return CommandFlashSprite(com);
		case Cmd::EraseEvent:
			return CommandEraseEvent(com);
		case Cmd::ChangeMapTileset:
			return CommandChangeMapTileset(com);
		case Cmd::CallEvent:
			return CommandCallEvent(com);
		case Cmd::ChangeEncounterRate:
			return CommandChangeEncounterRate(com);
		case Cmd::ProceedWithMovement:
			return CommandProceedWithMovement(com);
		case Cmd::PlayMovie:
			return CommandPlayMovie(com);
		case Cmd::ChangeBattleCommands:
			return CommandChangeBattleCommands(com);
		case Cmd::KeyInputProc:
			return CommandKeyInputProc(com);
		case Cmd::ChangeVehicleGraphic:
			return CommandChangeVehicleGraphic(com);
		case Cmd::EnterExitVehicle:
			return CommandEnterExitVehicle(com);
		case Cmd::SetVehicleLocation:
			return CommandSetVehicleLocation(com);
		case Cmd::TileSubstitution:
			return CommandTileSubstitution(com);
		case Cmd::PanScreen:
			return CommandPanScreen(com);
		case Cmd::SimulatedAttack:
			return CommandSimulatedAttack(com);
		case Cmd::ShowBattleAnimation:
			return CommandShowBattleAnimation(com);
		case Cmd::ChangeClass:
			return CommandChangeClass(com);
		case Cmd::HaltAllMovement:
			return CommandHaltAllMovement(com);
		case Cmd::ConditionalBranch:
			return CommandConditionalBranch(com);
		case Cmd::ElseBranch:
			return SkipTo(Cmd::EndBranch);
		case Cmd::EndBranch:
			return true;
		case Cmd::OpenLoadMenu:
			return CommandOpenLoadMenu(com);
		case Cmd::ExitGame:
			return CommandExitGame(com);
		case Cmd::ToggleAtbMode:
			return CommandToggleAtbMode(com);
		case Cmd::ToggleFullscreen:
			return CommandToggleFullscreen(com);
		case Cmd::OpenVideoOptions:
			// don't care
			return true;
		default:
			return Game_Interpreter::ExecuteCommand();
	}
}

/**
 * Commands
 */
bool Game_Interpreter_Map::CommandMessageOptions(RPG::EventCommand const& com) { //code 10120
	Game_Message::SetTransparent(com.parameters[0] != 0);
	Game_Message::SetPosition(com.parameters[1]);
	Game_Message::SetPositionFixed(com.parameters[2] == 0);
	Game_Message::SetContinueEvents(com.parameters[3] != 0);
	return true;
}


bool Game_Interpreter_Map::CommandChangeExp(RPG::EventCommand const& com) { // Code 10410
	int value = OperateValue(
		com.parameters[2],
		com.parameters[3],
		com.parameters[4]
	);

	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		actor->ChangeExp(actor->GetExp() + value, com.parameters[5] != 0);
	}

	return true;
}

bool Game_Interpreter_Map::CommandChangeParameters(RPG::EventCommand const& com) { // Code 10430
	int value = OperateValue(
		com.parameters[2],
		com.parameters[4],
		com.parameters[5]
		);

	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		switch (com.parameters[3]) {
			case 0:
				// Max HP
				actor->SetBaseMaxHp(actor->GetBaseMaxHp() + value);
				break;
			case 1:
				// Max MP
				actor->SetBaseMaxSp(actor->GetBaseMaxSp() + value);
				break;
			case 2:
				// Attack
				actor->SetBaseAtk(actor->GetBaseAtk() + value);
				break;
			case 3:
				// Defense
				actor->SetBaseDef(actor->GetBaseDef() + value);
				break;
			case 4:
				// Spirit
				actor->SetBaseSpi(actor->GetBaseSpi() + value);
				break;
			case 5:
				// Agility
				actor->SetBaseAgi(actor->GetBaseAgi() + value);
				break;
		}
	}
	return true;
}

bool Game_Interpreter_Map::CommandChangeHeroName(RPG::EventCommand const& com) { // code 10610
	Game_Actor* actor = Game_Actors::GetActor(com.parameters[0]);
	actor->SetName(com.string);
	return true;
}

bool Game_Interpreter_Map::CommandChangeHeroTitle(RPG::EventCommand const& com) { // code 10620
	Game_Actor* actor = Game_Actors::GetActor(com.parameters[0]);
	actor->SetTitle(com.string);
	return true;
}

bool Game_Interpreter_Map::CommandChangeSpriteAssociation(RPG::EventCommand const& com) { // code 10630
	Game_Actor* actor = Game_Actors::GetActor(com.parameters[0]);
	const std::string &file = com.string;
	int idx = com.parameters[1];
	bool transparent = com.parameters[2] != 0;
	actor->SetSprite(file, idx, transparent);
	Main_Data::game_player->Refresh();
	return true;
}

bool Game_Interpreter_Map::CommandMemorizeLocation(RPG::EventCommand const& com) { // code 10820
	Game_Character *player = Main_Data::game_player.get();
	int var_map_id = com.parameters[0];
	int var_x = com.parameters[1];
	int var_y = com.parameters[2];
	Game_Variables[var_map_id] = Game_Map::GetMapId();
	Game_Variables[var_x] = player->GetX();
	Game_Variables[var_y] = player->GetY();
	Game_Map::SetNeedRefresh(true);
	return true;
}

bool Game_Interpreter_Map::CommandRecallToLocation(RPG::EventCommand const& com) { // Code 10830
	Game_Character *player = Main_Data::game_player.get();
	int var_map_id = com.parameters[0];
	int var_x = com.parameters[1];
	int var_y = com.parameters[2];
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
	Main_Data::game_player->StartTeleport();

	// Parallel events should keep on running in 2k and 2k3, unlike in later versions
	if (!main_flag)
		return true;

	index++;
	return false;
}

bool Game_Interpreter_Map::CommandStoreTerrainID(RPG::EventCommand const& com) { // code 10820
	int x = ValueOrVariable(com.parameters[0], com.parameters[1]);
	int y = ValueOrVariable(com.parameters[0], com.parameters[2]);
	int var_id = com.parameters[3];
	Game_Variables[var_id] = Game_Map::GetTerrainTag(x, y);
	Game_Map::SetNeedRefresh(true);
	return true;
}

bool Game_Interpreter_Map::CommandStoreEventID(RPG::EventCommand const& com) { // code 10920
	int x = ValueOrVariable(com.parameters[0], com.parameters[1]);
	int y = ValueOrVariable(com.parameters[0], com.parameters[2]);
	int var_id = com.parameters[3];
	std::vector<Game_Event*> events;
	Game_Map::GetEventsXY(events, x, y);
	Game_Variables[var_id] = events.size() > 0 ? events.back()->GetId() : 0;
	Game_Map::SetNeedRefresh(true);
	return true;
}

bool Game_Interpreter_Map::CommandMemorizeBGM(RPG::EventCommand const& /* com */) { // code 11530
	Game_System::MemorizeBGM();
	return true;
}

bool Game_Interpreter_Map::CommandPlayMemorizedBGM(RPG::EventCommand const& /* com */) { // code 11540
	Game_System::PlayMemorizedBGM();
	return true;
}

bool Game_Interpreter_Map::CommandChangeSystemBGM(RPG::EventCommand const& com) { //code 10660
	RPG::Music music;
	int context = com.parameters[0];
	music.name = com.string;
	music.fadein = com.parameters[1];
	music.volume = com.parameters[2];
	music.tempo = com.parameters[3];
	music.balance = com.parameters[4];
	Game_System::SetSystemBGM(context, music);
	return true;
}

bool Game_Interpreter_Map::CommandChangeSystemSFX(RPG::EventCommand const& com) { //code 10670
	RPG::Sound sound;
	int context = com.parameters[0];
	sound.name = com.string;
	sound.volume = com.parameters[1];
	sound.tempo = com.parameters[2];
	sound.balance = com.parameters[3];
	Game_System::SetSystemSE(context, sound);
	return true;
}

bool Game_Interpreter_Map::CommandChangeSaveAccess(RPG::EventCommand const& com) { // code 11930
	Game_System::SetAllowSave(com.parameters[0] != 0);
	return true;
}

bool Game_Interpreter_Map::CommandChangeTeleportAccess(RPG::EventCommand const& com) { // code 11820
	Game_System::SetAllowTeleport(com.parameters[0] != 0);
	return true;
}

bool Game_Interpreter_Map::CommandChangeEscapeAccess(RPG::EventCommand const& com) { // code 11840
	Game_System::SetAllowEscape(com.parameters[0] != 0);
	return true;
}

bool Game_Interpreter_Map::CommandChangeMainMenuAccess(RPG::EventCommand const& com) { // code 11960
	Game_System::SetAllowMenu(com.parameters[0] != 0);
	return true;
}

bool Game_Interpreter_Map::CommandChangeActorFace(RPG::EventCommand const& com) { // code 10640
	Game_Actor* actor = Game_Actors::GetActor(com.parameters[0]);
	if (actor != NULL) {
		actor->SetFace(com.string, com.parameters[1]);
		return true;
	}
	return false;
}

bool Game_Interpreter_Map::CommandTeleport(RPG::EventCommand const& com) { // Code 10810
	// TODO: if in battle return true
	if (Main_Data::game_player->IsTeleporting() || Game_Temp::transition_processing ||
		Game_Message::visible) {
			return false;
	}

	int map_id = com.parameters[0];
	int x = com.parameters[1];
	int y = com.parameters[2];

	// RPG2k3 feature
	int direction = com.parameters.size() > 3 ? com.parameters[3] - 1 : -1;

	Main_Data::game_player->ReserveTeleport(map_id, x, y, direction);
	Main_Data::game_player->StartTeleport();

	// Parallel events should keep on running in 2k and 2k3, unlike in later versions
	if (!main_flag)
		return true;

	index++;
	return false;
}

bool Game_Interpreter_Map::CommandEraseScreen(RPG::EventCommand const& com) { // code 11010
	if (Game_Temp::transition_processing || Game_Message::visible)
		return false;

	Game_Temp::transition_processing = true;
	Game_Temp::transition_erase = true;

	switch(com.parameters[0]) {
		case -1:
			Game_Temp::transition_type = (Graphics::TransitionType)Game_System::GetTransition(
				Game_System::Transition_TeleportErase);
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

bool Game_Interpreter_Map::CommandShowScreen(RPG::EventCommand const& com) { // code 11020
	if (Game_Temp::transition_processing || Game_Message::visible)
		return false;

	Game_Temp::transition_processing = true;
	Game_Temp::transition_erase = false;

	switch(com.parameters[0]) {
		case -1:
			Game_Temp::transition_type = (Graphics::TransitionType)Game_System::GetTransition(
				Game_System::Transition_TeleportShow);
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

bool Game_Interpreter_Map::CommandShowPicture(RPG::EventCommand const& com) { // code 11110
	int pic_id = com.parameters[0];
	Game_Picture* picture = Main_Data::game_screen->GetPicture(pic_id);
	std::string const& pic_name = com.string;
	int x = ValueOrVariable(com.parameters[1], com.parameters[2]);
	int y = ValueOrVariable(com.parameters[1], com.parameters[3]);
	bool scrolls = com.parameters[4] > 0;
	int magnify = com.parameters[5];
	int top_trans = com.parameters[6];
	bool use_trans = com.parameters[7] > 0;
	int red = com.parameters[8];
	int green = com.parameters[9];
	int blue = com.parameters[10];
	int saturation = com.parameters[11];
	int effect = com.parameters[12];
	int speed = com.parameters[13];
	int bottom_trans;

	if (Player::IsRPG2k() || Player::IsRPG2k3E()) {
		// RKG2k and RPG2k3 1.10 do not support this option
		bottom_trans = top_trans;
	} else {
		// Corner case when 2k maps are used in 2k3 (pre-1.10) and don't contain this chunk
		size_t param_size = com.parameters.size();
		bottom_trans = param_size > 14 ? com.parameters[14] : top_trans;
	}

	picture->Show(pic_name, use_trans);
	picture->SetFixedToMap(scrolls);

	picture->SetMovementEffect(x, y);
	picture->SetColorEffect(red, green, blue, saturation);
	picture->SetZoomEffect(magnify);
	picture->SetTransparencyEffect(top_trans, bottom_trans);
	picture->SetTransition(0);

	switch (effect) {
		case 0:
			picture->StopEffects();
			break;
		case 1:
			picture->SetRotationEffect(speed);
			break;
		case 2:
			picture->SetWaverEffect(speed);
			break;
	}

	picture->SetTransition(0);

	return true;
}

bool Game_Interpreter_Map::CommandMovePicture(RPG::EventCommand const& com) { // code 11120
	int pic_id = com.parameters[0];
	Game_Picture* picture = Main_Data::game_screen->GetPicture(pic_id);
	int x = ValueOrVariable(com.parameters[1], com.parameters[2]);
	int y = ValueOrVariable(com.parameters[1], com.parameters[3]);
	int magnify = com.parameters[5];
	int top_trans = com.parameters[6];
	int red = com.parameters[8];
	int green = com.parameters[9];
	int blue = com.parameters[10];
	int saturation = com.parameters[11];
	int effect = com.parameters[12];
	int speed = com.parameters[13];
	int tenths = com.parameters[14];
	bool wait = com.parameters[15] != 0;

	int bottom_trans;
	if (Player::IsRPG2k() || Player::IsRPG2k3E()) {
		// RPG2k and RPG2k3 1.10 do not support this option
		bottom_trans = top_trans;
	} else {
		// Corner case when 2k maps are used in 2k3 (pre-1.10) and don't contain this chunk
		size_t param_size = com.parameters.size();
		bottom_trans = param_size > 16 ? com.parameters[16] : top_trans;
	}

	picture->SetMovementEffect(x, y);
	picture->SetColorEffect(red, green, blue, saturation);
	picture->SetZoomEffect(magnify);
	picture->SetTransparencyEffect(top_trans, bottom_trans);
	picture->SetTransition(tenths);

	switch (effect) {
		case 0:
			picture->StopEffects();
			break;
		case 1:
			picture->SetRotationEffect(speed);
			break;
		case 2:
			picture->SetWaverEffect(speed);
			break;
	}

	if (wait)
		SetupWait(tenths);

	return true;
}

bool Game_Interpreter_Map::CommandErasePicture(RPG::EventCommand const& com) { // code 11130
	int pic_id = com.parameters[0];
	Game_Picture* picture = Main_Data::game_screen->GetPicture(pic_id);
	picture->Erase();

	return true;
}

bool Game_Interpreter_Map::CommandWeatherEffects(RPG::EventCommand const& com) { // code 11070
	Game_Screen* screen = Main_Data::game_screen.get();
	int type = com.parameters[0];
	int strength = com.parameters[1];
	screen->SetWeatherEffect(type, strength);
	return true;
}

void Game_Interpreter_Map::OnChangeSystemGraphicReady(FileRequestResult* result) {
	Game_System::SetSystemName(result->file);

	Scene_Map* scene = (Scene_Map*)Scene::Find(Scene::Map).get();

	if (!scene)
		return;

	scene->spriteset->SystemGraphicUpdated();
}

bool Game_Interpreter_Map::CommandChangeSystemGraphics(RPG::EventCommand const& com) { // code 10680
	FileRequestAsync* request = AsyncHandler::RequestFile("System", com.string);
	request->Bind(&Game_Interpreter_Map::OnChangeSystemGraphicReady, this);
	request->SetImportantFile(true);
	request->Start();

	return true;
}

bool Game_Interpreter_Map::CommandChangeScreenTransitions(RPG::EventCommand const& com) { // code 10690
	Game_System::SetTransition(com.parameters[0], com.parameters[1]);
	return true;
}

bool Game_Interpreter_Map::CommandChangeEventLocation(RPG::EventCommand const& com) { // Code 10860
	int event_id = com.parameters[0];
	Game_Character *event = GetCharacter(event_id);
	if (event != NULL) {
		int x = ValueOrVariable(com.parameters[1], com.parameters[2]);
		int y = ValueOrVariable(com.parameters[1], com.parameters[3]);
		event->MoveTo(x, y);
	}
	return true;
}

bool Game_Interpreter_Map::CommandTradeEventLocations(RPG::EventCommand const& com) { // Code 10870
	int event1_id = com.parameters[0];
	int event2_id = com.parameters[1];

	Game_Character *event1 = GetCharacter(event1_id);
	Game_Character *event2 = GetCharacter(event2_id);

	if (event1 != NULL && event2 != NULL) {
		int x1 = event1->GetX();
		int y1 = event1->GetY();

		int x2 = event2->GetX();
		int y2 = event2->GetY();

		event1->MoveTo(x2, y2);
		event2->MoveTo(x1, y1);
	}

	return true;
}

bool Game_Interpreter_Map::CommandTimerOperation(RPG::EventCommand const& com) { // code 10230
	int timer_id = (Player::IsRPG2k()) ? 0 : com.parameters[5];
	int seconds;
	bool visible, battle;

	switch (com.parameters[0]) {
		case 0:
			seconds = ValueOrVariable(com.parameters[1],
									  com.parameters[2]);
			Main_Data::game_party->SetTimer(timer_id, seconds);
			break;
		case 1:
			visible = com.parameters[3] != 0;
			battle = com.parameters[4] != 0;
			Main_Data::game_party->StartTimer(timer_id, visible, battle);
			break;
		case 2:
			Main_Data::game_party->StopTimer(timer_id);
			break;
		default:
			return false;
	}
	return true;
}

bool Game_Interpreter_Map::CommandChangePBG(RPG::EventCommand const& com) { // code 11720
	const std::string& name = com.string;
	Game_Map::SetParallaxName(name);

	bool horz = com.parameters[0] != 0;
	bool vert = com.parameters[1] != 0;
	bool horz_auto = com.parameters[2] != 0;
	int horz_speed = com.parameters[3];
	bool vert_auto = com.parameters[4] != 0;
	int vert_speed = com.parameters[5];
	Game_Map::SetParallaxScroll(horz, vert,
								horz_auto, vert_auto,
								horz_speed, vert_speed);
	return true;
}

bool Game_Interpreter_Map::CommandJumpToLabel(RPG::EventCommand const& com) { // code 12120
	int label_id = com.parameters[0];

	for (int idx = 0; (size_t) idx < list.size(); idx++) {
		if (list[idx].code != Cmd::Label)
			continue;
		if (list[idx].parameters[0] != label_id)
			continue;
		index = idx;
		break;
	}

	return true;
}

bool Game_Interpreter_Map::CommandBreakLoop(RPG::EventCommand const& com) { // code 12220
	return SkipTo(Cmd::EndLoop, Cmd::EndLoop, 0, com.indent - 1, true);
}

bool Game_Interpreter_Map::CommandEndLoop(RPG::EventCommand const& com) { // code 22210
	int indent = com.indent;

	for (int idx = index; idx >= 0; idx--) {
		if (list[idx].indent > indent)
			continue;
		if (list[idx].indent < indent)
			return false;
		if (list[idx].code != Cmd::Loop)
			continue;
		index = idx;
		break;
	}

	return true;
}

bool Game_Interpreter_Map::CommandMoveEvent(RPG::EventCommand const& com) { // code 11330
	int event_id = com.parameters[0];
	Game_Character* event = GetCharacter(event_id);
	if (event != NULL) {
		// If the event is a vehicle in use, push the commands to the player instead
		if (event_id >= Game_Character::CharBoat && event_id <= Game_Character::CharAirship)
			if (static_cast<Game_Vehicle*>(event)->IsInUse())
				event = Main_Data::game_player.get();

		RPG::MoveRoute route;
		int move_freq = com.parameters[1];
		route.repeat = com.parameters[2] != 0;
		route.skippable = com.parameters[3] != 0;

		std::vector<int>::const_iterator it;
		for (it = com.parameters.begin() + 4; it < com.parameters.end(); )
			route.move_commands.push_back(DecodeMove(it));

		event->ForceMoveRoute(route, move_freq);
	}
	return true;
}

bool Game_Interpreter_Map::CommandOpenShop(RPG::EventCommand const& com) { // code 10720

	switch (com.parameters[0]) {
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

	Game_Temp::shop_type = com.parameters[1];
	Game_Temp::shop_handlers = com.parameters[2] != 0;

	Game_Temp::shop_goods.clear();
	std::vector<int>::const_iterator it;
	for (it = com.parameters.begin() + 4; it < com.parameters.end(); ++it)
		Game_Temp::shop_goods.push_back(*it);

	Game_Temp::shop_transaction = false;
	Game_Temp::shop_calling = true;
	SetContinuation(static_cast<ContinuationFunction>(&Game_Interpreter_Map::ContinuationOpenShop));
	return false;
}

bool Game_Interpreter_Map::ContinuationOpenShop(RPG::EventCommand const& /* com */) {
	continuation = NULL;
	if (!Game_Temp::shop_handlers) {
		index++;
		return true;
	}

	if (!SkipTo(Game_Temp::shop_transaction
				? Cmd::Transaction
				: Cmd::NoTransaction,
				Cmd::EndShop)) {
		return false;
	}

	index++;
	return true;
}

bool Game_Interpreter_Map::CommandShowInn(RPG::EventCommand const& com) { // code 10730
	int inn_type = com.parameters[0];
	Game_Temp::inn_price = com.parameters[1];
	Game_Temp::inn_handlers = com.parameters[2] != 0;

	if (Game_Temp::inn_price == 0) {
		// Skip prompt.
		Game_Message::choice_result = 0;
		SetContinuation(static_cast<ContinuationFunction>(&Game_Interpreter_Map::ContinuationShowInnStart));
		return true;
	}

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
	if (Main_Data::game_party->GetGold() < Game_Temp::inn_price)
		Game_Message::choice_disabled.set(0);

	Game_Temp::inn_calling = true;
	Game_Message::choice_result = 4;

	SetContinuation(static_cast<ContinuationFunction>(&Game_Interpreter_Map::ContinuationShowInnStart));
	return true;
}

bool Game_Interpreter_Map::ContinuationShowInnStart(RPG::EventCommand const& /* com */) {
	if (Game_Message::visible) {
		return false;
	}
	continuation = NULL;

	bool inn_stay = Game_Message::choice_result == 0;

	Game_Temp::inn_calling = false;

	if (inn_stay)
		Main_Data::game_party->GainGold(-Game_Temp::inn_price);

	if (inn_stay) {
		// Full heal
		std::vector<Game_Actor*> actors = Main_Data::game_party->GetActors();
		for (Game_Actor* actor : actors) {
			actor->ChangeHp(actor->GetMaxHp());
			actor->SetSp(actor->GetMaxSp());
			actor->RemoveAllStates();
		}
		Graphics::Transition(Graphics::TransitionFadeOut, 36, true);
		Audio().BGM_Fade(800);
		SetContinuation(static_cast<ContinuationFunction>(&Game_Interpreter_Map::ContinuationShowInnContinue));
		return false;
	}

	if (Game_Temp::inn_handlers && !SkipTo(inn_stay ? Cmd::Stay : Cmd::NoStay, Cmd::EndInn))
		return false;
	index++;
	return true;
}

bool Game_Interpreter_Map::ContinuationShowInnContinue(RPG::EventCommand const& /* com */) {
	if (Graphics::IsTransitionPending())
		return false;

	const RPG::Music& bgm_inn = Game_System::GetSystemBGM(Game_System::BGM_Inn);
	// FIXME: Abusing before_battle_music (Which is unused when calling an Inn)
	// Is there also before_inn_music in the savegame?
	Main_Data::game_data.system.before_battle_music = Game_System::GetCurrentBGM();

	Game_System::BgmPlay(bgm_inn);
	
	SetContinuation(static_cast<ContinuationFunction>(&Game_Interpreter_Map::ContinuationShowInnFinish));

	return false;
}

bool Game_Interpreter_Map::ContinuationShowInnFinish(RPG::EventCommand const& /* com */) {
	if (Graphics::IsTransitionPending())
		return false;

	const RPG::Music& bgm_inn = Game_System::GetSystemBGM(Game_System::BGM_Inn);
	if (bgm_inn.name.empty() || bgm_inn.name == "(OFF)" || bgm_inn.name == "(Brak)" || Audio().BGM_PlayedOnce()) {
		Game_System::BgmStop();
		continuation = NULL;
		Graphics::Transition(Graphics::TransitionFadeIn, 36, false);
		Game_System::BgmPlay(Main_Data::game_data.system.before_battle_music);
		index++;
		return false;
	}

	return false;
}

bool Game_Interpreter_Map::CommandEnterHeroName(RPG::EventCommand const& com) { // code 10740
	Game_Temp::hero_name_id = com.parameters[0];
	Game_Temp::hero_name_charset = com.parameters[1];

	if (com.parameters[2] != 0)
		Game_Temp::hero_name = Game_Actors::GetActor(Game_Temp::hero_name_id)->GetName();
	else
		Game_Temp::hero_name.clear();

	Game_Temp::name_calling = true;
	return true;
}

bool Game_Interpreter_Map::CommandReturnToTitleScreen(RPG::EventCommand const& /* com */) { // code 12510
	Game_Temp::to_title = true;
	SetContinuation(&Game_Interpreter::DefaultContinuation);
	return false;
}

bool Game_Interpreter_Map::CommandOpenSaveMenu(RPG::EventCommand const& /* com */) { // code 11910
	Game_Temp::save_calling = true;
	SetContinuation(&Game_Interpreter::DefaultContinuation);
	return false;
}

bool Game_Interpreter_Map::CommandOpenMainMenu(RPG::EventCommand const& /* com */) { // code 11950
	Game_Temp::menu_calling = true;
	SetContinuation(&Game_Interpreter::DefaultContinuation);
	return false;
}

bool Game_Interpreter_Map::CommandEnemyEncounter(RPG::EventCommand const& com) { // code 10710
	Game_Temp::battle_troop_id = ValueOrVariable(com.parameters[0],
												 com.parameters[1]);
	Game_Character *player = Main_Data::game_player.get();
	Game_Battle::SetTerrainId(Game_Map::GetTerrainTag(player->GetX(), player->GetY()));

	switch (com.parameters[2]) {
		case 0:
			break;
		case 1:
			Game_Temp::battle_background = com.string;

			if (Player::IsRPG2k())
				Game_Temp::battle_formation = 0;
			else
				Game_Temp::battle_formation = com.parameters[7];
			break;
		case 2:
			Game_Battle::SetTerrainId(com.parameters[8]);
			Game_Temp::battle_background = Data::terrains[com.parameters[8] - 1].background_name;
			break;
		default:
			return false;
	}
	Game_Temp::battle_escape_mode = com.parameters[3]; // 0 disallow, 1 end event processing, 2 victory/escape custom handler
	Game_Temp::battle_defeat_mode = com.parameters[4]; // 0 game over, 1 victory/defeat custom handler
	Game_Temp::battle_first_strike = com.parameters[5] != 0;

	if (Player::IsRPG2k())
		Game_Battle::SetBattleMode(0);
	else
		Game_Battle::SetBattleMode(com.parameters[6]); // 0 normal, 1 initiative, 2 surround, 3 back attack, 4 pincer

	Game_Temp::battle_result = Game_Temp::BattleVictory;
	Game_Temp::battle_calling = true;

	SetContinuation(static_cast<ContinuationFunction>(&Game_Interpreter_Map::ContinuationEnemyEncounter));
	return false;
}

bool Game_Interpreter_Map::ContinuationEnemyEncounter(RPG::EventCommand const& com) {
	continuation = NULL;

	switch (Game_Temp::battle_result) {
		case Game_Temp::BattleVictory:
			if ((Game_Temp::battle_defeat_mode == 0 && Game_Temp::battle_escape_mode != 2) || !SkipTo(Cmd::VictoryHandler, Cmd::EndBattle)) {
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
					return CommandEndEventProcessing(com);
				case 2:
					if (!SkipTo(Cmd::EscapeHandler, Cmd::EndBattle))
						return false;
					index++;
					return true;
				default:
					return false;
			}
		case Game_Temp::BattleDefeat:
			switch (Game_Temp::battle_defeat_mode) {
				case 0:
					return CommandGameOver(com);
				case 1:
					if (!SkipTo(Cmd::DefeatHandler, Cmd::EndBattle))
						return false;
					index++;
					return true;
				default:
					return false;
			}
		case Game_Temp::BattleAbort:
			if (!SkipTo(Cmd::EndBattle))
				return false;
			index++;
			return true;
		default:
			return false;
	}
}

bool Game_Interpreter_Map::CommandTeleportTargets(RPG::EventCommand const& com) { // code 11810
	int map_id = com.parameters[1];

	if (com.parameters[0] != 0) {
		Game_Targets::RemoveTeleportTarget(map_id);
		return true;
	}

	int x = com.parameters[2];
	int y = com.parameters[3];
	int switch_id = (com.parameters[4] != 0)
		? com.parameters[5]
		: -1;
	Game_Targets::AddTeleportTarget(map_id, x, y, switch_id);
	return true;
}

bool Game_Interpreter_Map::CommandEscapeTarget(RPG::EventCommand const& com) { // code 11830
	int map_id = com.parameters[0];
	int x = com.parameters[1];
	int y = com.parameters[2];
	int switch_id = (com.parameters[3] != 0)
		? com.parameters[4]
		: -1;
	Game_Targets::SetEscapeTarget(map_id, x, y, switch_id);
	return true;
}

bool Game_Interpreter_Map::CommandSpriteTransparency(RPG::EventCommand const& com) { // code 11310
	bool visible = com.parameters[0] != 0;
	Game_Character* player = Main_Data::game_player.get();
	player->SetVisible(visible);

	return true;
}

bool Game_Interpreter_Map::CommandFlashSprite(RPG::EventCommand const& com) { // code 11320
	int event_id = com.parameters[0];
	Color color(com.parameters[1] << 3,
				com.parameters[2] << 3,
				com.parameters[3] << 3,
				com.parameters[4] << 3);
	int tenths = com.parameters[5];
	bool wait = com.parameters[6] > 0;
	Game_Character* event = GetCharacter(event_id);

	if (event != NULL) {
		event->Flash(color, tenths);

		if (wait)
			SetupWait(tenths);
	}

	return true;
}

bool Game_Interpreter_Map::CommandEraseEvent(RPG::EventCommand const& /* com */) { // code 12320
	if (event_id == 0)
		return true;

	tEventHash& events = Game_Map::GetEvents();
	events[event_id]->SetActive(false);

	return true;
}

bool Game_Interpreter_Map::CommandChangeMapTileset(RPG::EventCommand const& com) { // code 11710
	int chipset_id = com.parameters[0];
	Game_Map::SetChipset(chipset_id);

	Scene_Map* scene = (Scene_Map*) Scene::Find(Scene::Map).get();

	if (!scene)
		return true;

	scene->spriteset->ChipsetUpdated();

	return true;
}

bool Game_Interpreter_Map::CommandCallEvent(RPG::EventCommand const& com) { // code 12330
	int evt_id;
	int event_page;

	if (child_interpreter)
		return false;

	clear_child = false;

	child_interpreter.reset(new Game_Interpreter_Map(depth + 1, main_flag));

	switch (com.parameters[0]) {
		case 0: // Common Event
			evt_id = com.parameters[1];
			child_interpreter->Setup(Data::commonevents[evt_id - 1].event_commands, 0, Data::commonevents[evt_id - 1].ID, -2);
			return true;
		case 1: // Map Event
			evt_id = com.parameters[1];
			event_page = com.parameters[2];
			break;
		case 2: // Indirect
			evt_id = Game_Variables[com.parameters[1]];
			event_page = Game_Variables[com.parameters[2]];
			break;
		default:
			return false;
	}

	Game_Event* event = static_cast<Game_Event*>(GetCharacter(evt_id));
	if (event != NULL) {
		RPG::EventPage& page = event->GetEvent().pages[event_page - 1];
		child_interpreter->Setup(page.event_commands, event->GetId(), event->GetX(), event->GetY());
	}

	return true;
}

bool Game_Interpreter_Map::CommandChangeEncounterRate(RPG::EventCommand const& com) { // code 11740
	int steps = com.parameters[0];

	Game_Map::SetEncounterRate(steps);

	return true;
}

bool Game_Interpreter_Map::CommandProceedWithMovement(RPG::EventCommand const& /* com */) { // code 11340
	return !Game_Map::IsAnyMovePending();
}

bool Game_Interpreter_Map::CommandPlayMovie(RPG::EventCommand const& com) { // code 11560
	const std::string& filename = com.string;
	int pos_x = ValueOrVariable(com.parameters[0], com.parameters[1]);
	int pos_y = ValueOrVariable(com.parameters[0], com.parameters[2]);
	int res_x = com.parameters[3];
	int res_y = com.parameters[4];

	Main_Data::game_screen->PlayMovie(filename, pos_x, pos_y, res_x, res_y);

	return true;
}

bool Game_Interpreter_Map::CommandChangeBattleCommands(RPG::EventCommand const& com) { // code 1009
	int actor_id = com.parameters[1];
	Game_Actor* actor = Game_Actors::GetActor(actor_id);
	int cmd_id = com.parameters[2];
	bool add = com.parameters[3] != 0;

	actor->ChangeBattleCommands(add, cmd_id);

	return true;
}

bool Game_Interpreter_Map::CommandKeyInputProc(RPG::EventCommand const& com) { // code 11610
	int var_id = com.parameters[0];
	bool wait = com.parameters[1] != 0;

	// Wait the first frame so that it ignores keys that were pressed before this command started.
	if (wait && button_timer == 0) {
		button_timer++;
		return false;
	}

	bool time = false;
	int time_id = 0;

	bool check_decision = com.parameters[3] != 0;
	bool check_cancel   = com.parameters[4] != 0;
	bool check_numbers  = false;
	bool check_arith    = false;
	bool check_shift    = false;
	bool check_down     = false;
	bool check_left     = false;
	bool check_right    = false;
	bool check_up       = false;
	int result = 0;
	size_t param_size = com.parameters.size();

	// Use a function pointer to check triggered keys if it waits for input and pressed keys otherwise
	bool (*check)(Input::InputButton) = wait ? Input::IsTriggered : Input::IsPressed;

	if (param_size < 6) {
		// For Rpg2k <1.50
		bool check_dir = com.parameters[2] != 0;
		check_down  = check_dir;
		check_left  = check_dir;
		check_right = check_dir;
		check_up    = check_dir;
	} else if (param_size < 11) {
		// For Rpg2k >=1.50
		check_shift = com.parameters[5] != 0;
		check_down  = param_size > 6 ? com.parameters[6] != 0 : false;
		check_left  = param_size > 7 ? com.parameters[7] != 0 : false;
		check_right = param_size > 8 ? com.parameters[8] != 0 : false;
		check_up    = param_size > 9 ? com.parameters[9] != 0 : false;
	} else {
		// For Rpg2k3
		check_numbers  = com.parameters[5] != 0;
		check_arith    = com.parameters[6] != 0;
		time_id        = com.parameters[7];
		time           = com.parameters[8] != 0;
		check_shift    = com.parameters[9] != 0;
		check_down     = com.parameters[10] != 0;
		check_left     = param_size > 11 ? com.parameters[11] != 0 : false;
		check_right    = param_size > 12 ? com.parameters[12] != 0 : false;
		check_up       = param_size > 13 ? com.parameters[13] != 0 : false;
	}

	if (check_down && check(Input::DOWN)) {
		result = 1;
	}
	if (check_left && check(Input::LEFT)) {
		result = 2;
	}
	if (check_right && check(Input::RIGHT)) {
		result = 3;
	}
	if (check_up && check(Input::UP)) {
		result = 4;
	}
	if (check_decision && check(Input::DECISION)) {
		result = 5;
	}
	if (check_cancel && check(Input::CANCEL)) {
		result = 6;
	}
	if (check_shift && check(Input::SHIFT)) {
		result = 7;
	}
	if (check_numbers) {
		for (int i = 0; i < 10; ++i) {
			if (check((Input::InputButton)(Input::N0 + i))) {
				result = 10 + i;
			}
		}
	}
	if (check_arith) {
		for (int i = 0; i < 5; ++i) {
			if (check((Input::InputButton)(Input::PLUS + i))) {
				result = 20 + i;
			}
		}
	}

	Game_Variables[var_id] = result;
	Game_Map::SetNeedRefresh(true);

	if (!wait)
		return true;

	button_timer++;

	if (result == 0)
		return false;

	if (time) {
		// 10 per second
		Game_Variables[time_id] = (int)((float)button_timer / Graphics::GetDefaultFps() * 10);
	}

	button_timer = 0;

	return true;
}

bool Game_Interpreter_Map::CommandChangeVehicleGraphic(RPG::EventCommand const& com) { // code 10650
	Game_Vehicle::Type vehicle_id = (Game_Vehicle::Type) (com.parameters[0]+1);
	Game_Vehicle* vehicle = Game_Map::GetVehicle(vehicle_id);
	const std::string& name = com.string;
	int vehicle_index = com.parameters[1];

	vehicle->SetGraphic(name, vehicle_index);

	return true;
}

bool Game_Interpreter_Map::CommandEnterExitVehicle(RPG::EventCommand const& /* com */) { // code 10840
	Main_Data::game_player->GetOnOffVehicle();

	return true;
}

bool Game_Interpreter_Map::CommandSetVehicleLocation(RPG::EventCommand const& com) { // code 10850
	Game_Vehicle::Type vehicle_id = (Game_Vehicle::Type) (com.parameters[0]+1);
	Game_Vehicle* vehicle = Game_Map::GetVehicle(vehicle_id);
	int map_id = ValueOrVariable(com.parameters[1], com.parameters[2]);
	int x = ValueOrVariable(com.parameters[1], com.parameters[3]);
	int y = ValueOrVariable(com.parameters[1], com.parameters[4]);

	vehicle->SetPosition(map_id, x, y);

	return true;
}

bool Game_Interpreter_Map::CommandTileSubstitution(RPG::EventCommand const& com) { // code 11750
	bool upper = com.parameters[0] != 0;
	int old_id = com.parameters[1];
	int new_id = com.parameters[2];
	Scene_Map* scene = (Scene_Map*) Scene::Find(Scene::Map).get();
	if (!scene)
		return true;

	if (upper)
		scene->spriteset->SubstituteUp(old_id, new_id);
	else
		scene->spriteset->SubstituteDown(old_id, new_id);

	return true;
}

bool Game_Interpreter_Map::CommandPanScreen(RPG::EventCommand const& com) { // code 11060
	int direction;
	int distance;
	int speed;

	if (waiting_pan_screen) {
		waiting_pan_screen = Game_Map::IsPanWaiting();
		return !waiting_pan_screen;
	}

	switch (com.parameters[0]) {
	case 0: // Lock
		Game_Map::LockPan();
		break;
	case 1: // Unlock
		Game_Map::UnlockPan();
		break;
	case 2: // Pan
		direction = com.parameters[1];
		distance = com.parameters[2];
		speed = com.parameters[3];
		waiting_pan_screen = com.parameters[4] != 0;
		Game_Map::StartPan(direction, distance, speed, waiting_pan_screen);
		break;
	case 3: // Reset
		speed = com.parameters[3];
		waiting_pan_screen = com.parameters[4] != 0;
		Game_Map::ResetPan(speed, waiting_pan_screen);
		break;
	}

	return !waiting_pan_screen;
}

bool Game_Interpreter_Map::CommandSimulatedAttack(RPG::EventCommand const& com) { // code 10500
	int atk = com.parameters[2];
	int def = com.parameters[3];
	int spi = com.parameters[4];
	int var = com.parameters[5];

	for (const auto& actor : GetActors(com.parameters[0], com.parameters[1])) {
		actor->ResetBattle();
		int result = atk;
		result -= (actor->GetDef() * def) / 400;
		result -= (actor->GetSpi() * spi) / 800;
		if (var != 0) {
			int rperc = var * 5;
			int rval = rand() % (2 * rperc) - rperc;
			result += result * rval / 100;
		}

		result = std::max(0, result);
		actor->ChangeHp(-result);

		CheckGameOver();

		if (com.parameters[6] != 0) {
			Game_Variables[com.parameters[7]] = result;
			Game_Map::SetNeedRefresh(true);
		}
	}

	return true;
}

bool Game_Interpreter_Map::CommandShowBattleAnimation(RPG::EventCommand const& com) { // code 11210
	if (waiting_battle_anim) {
		waiting_battle_anim = Game_Map::IsBattleAnimationWaiting();
		return !waiting_battle_anim;
	}

	int animation_id = com.parameters[0];
	int evt_id = com.parameters[1];
	waiting_battle_anim = com.parameters[2] > 0;
	bool global = com.parameters[3] > 0;

	if (evt_id == Game_Character::CharThisEvent)
		evt_id = event_id;

	Game_Map::ShowBattleAnimation(animation_id, evt_id, global);

	return !waiting_battle_anim;
}

bool Game_Interpreter_Map::CommandChangeClass(RPG::EventCommand const& com) { // code 1008
	int actor_id = com.parameters[1];
	int class_id = com.parameters[2];
	bool level1 = com.parameters[3] > 0;
	int skill_mode = com.parameters[4]; // no change, replace, add
	int stats_mode = com.parameters[5]; // no change, halve, level 1, current level
	bool show = com.parameters[6] > 0;

	Game_Actor* actor = Game_Actors::GetActor(actor_id);

	int cur_lvl = actor->GetLevel();
	int cur_exp = actor->GetExp();

	switch (stats_mode) {
		case 2:
			actor->SetClass(class_id);
			actor->SetLevel(1);
			actor->SetExp(0);
			break;
		case 3:
			actor->SetClass(class_id);
			break;
	}

	int cur_hp = actor->GetBaseMaxHp();
	int cur_sp = actor->GetBaseMaxSp();
	int cur_atk = actor->GetBaseAtk();
	int cur_def = actor->GetBaseDef();
	int cur_spi = actor->GetBaseSpi();
	int cur_agi = actor->GetBaseAgi();

	switch (stats_mode) {
		case 1:
			cur_hp /= 2;
			cur_sp /= 2;
			cur_atk /= 2;
			cur_def /= 2;
			cur_spi /= 2;
			cur_agi /= 2;
			break;
	}

	actor->SetClass(class_id);
	if (level1) {
		actor->SetLevel(1);
		actor->SetExp(0);
	}
	else {
		actor->SetExp(cur_exp);
		actor->SetLevel(cur_lvl);
	}

	actor->SetBaseMaxHp(cur_hp);
	actor->SetBaseMaxSp(cur_sp);
	actor->SetBaseAtk(cur_atk);
	actor->SetBaseDef(cur_def);
	actor->SetBaseSpi(cur_spi);
	actor->SetBaseAgi(cur_agi);

	int level = actor->GetLevel();

	switch (skill_mode) {
		case 0:
			break;
		case 1:
			while (!actor->GetSkills().empty())
				actor->UnlearnSkill(actor->GetSkills()[0]);
			break;
		case 2:
		{
			const RPG::Class& klass = Data::classes[class_id - 1];
			while (!actor->GetSkills().empty())
				actor->UnlearnSkill(actor->GetSkills()[0]);
			std::vector<RPG::Learning>::const_iterator it;
			for (it = klass.skills.begin(); it != klass.skills.end(); ++it) {
				const RPG::Learning& learn = *it;
				if (level >= learn.level)
					actor->LearnSkill(learn.skill_id);
			}
			break;
		}
	}

	if (show && level > cur_lvl) {
		// TODO
		// Show message increase level
	}

	return true;
}

bool Game_Interpreter_Map::CommandHaltAllMovement(RPG::EventCommand const& /* com */) { // code 11350
	Game_Map::RemoveAllPendingMoves();
	return true;
}

bool Game_Interpreter_Map::CommandOpenLoadMenu(RPG::EventCommand const& com) {
	Game_Temp::load_calling = true;
	return true;
}

bool Game_Interpreter_Map::CommandExitGame(RPG::EventCommand const& com) {
	Player::exit_flag = true;
	return true;
}

bool Game_Interpreter_Map::CommandToggleAtbMode(RPG::EventCommand const& com) {
	Output::Warning("Command Toggle ATB mode not supported");
	return true;
}

bool Game_Interpreter_Map::CommandToggleFullscreen(RPG::EventCommand const& com) {
	DisplayUi->BeginDisplayModeChange();
	DisplayUi->ToggleFullscreen();
	DisplayUi->EndDisplayModeChange();
	return true;
}

/**
 * Conditional Branch
 */
bool Game_Interpreter_Map::CommandConditionalBranch(RPG::EventCommand const& com) { // Code 12010
	bool result = false;
	int value1, value2;
	int actor_id;
	Game_Actor* actor;
	Game_Character* character;

	switch (com.parameters[0]) {
		case 0:
			// Switch
			result = Game_Switches[com.parameters[1]] == (com.parameters[2] == 0);
			break;
		case 1:
			// Variable
			value1 = Game_Variables[com.parameters[1]];
			if (com.parameters[2] == 0) {
				value2 = com.parameters[3];
			} else {
				value2 = Game_Variables[com.parameters[3]];
			}
			switch (com.parameters[4]) {
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
			value1 = Main_Data::game_party->GetTimer(Main_Data::game_party->Timer1);
			value2 = com.parameters[1] * DEFAULT_FPS;
			switch (com.parameters[2]) {
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
			if (com.parameters[2] == 0) {
				// Greater than or equal
				result = (Main_Data::game_party->GetGold() >= com.parameters[1]);
			} else {
				// Less than or equal
				result = (Main_Data::game_party->GetGold() <= com.parameters[1]);
			}
			break;
		case 4:
			// Item
			if (com.parameters[2] == 0) {
				// Having
				result = Main_Data::game_party->GetItemCount(com.parameters[1])
					+ Main_Data::game_party->GetItemCount(com.parameters[1], true) > 0;
			} else {
				// Not having
				result = Main_Data::game_party->GetItemCount(com.parameters[1])
					+ Main_Data::game_party->GetItemCount(com.parameters[1], true) == 0;
			}
			break;
		case 5:
			// Hero
			actor_id = com.parameters[1];
			actor = Game_Actors::GetActor(actor_id);
			switch (com.parameters[2]) {
				case 0:
					// Is actor in party
					result = Main_Data::game_party->IsActorInParty(actor_id);
					break;
				case 1:
					// Name
					result = (actor->GetName() == com.string);
					break;
				case 2:
					// Higher or equal level
					result = (actor->GetLevel() >= com.parameters[3]);
					break;
				case 3:
					// Higher or equal HP
					result = (actor->GetHp() >= com.parameters[3]);
					break;
				case 4:
					// Is skill learned
					result = (actor->IsSkillLearned(com.parameters[3]));
					break;
				case 5:
					// Equipped object
					result = (
						(actor->GetShieldId() == com.parameters[3]) ||
						(actor->GetArmorId() == com.parameters[3]) ||
						(actor->GetHelmetId() == com.parameters[3]) ||
						(actor->GetAccessoryId() == com.parameters[3]) ||
						(actor->GetWeaponId() == com.parameters[3])
					);
					break;
				case 6:
					// Has state
					result = (actor->HasState(com.parameters[3]));
					break;
				default:
					;
			}
			break;
		case 6:
			// Orientation of char
			character = GetCharacter(com.parameters[1]);
			if (character != NULL) {
				result = character->GetSpriteDirection() == com.parameters[2];
			}
			break;
		case 7:
			// Vehicle in use
			result = Game_Map::GetVehicle((Game_Vehicle::Type) (com.parameters[1]+1))->IsInUse();
			break;
		case 8:
			// TODO Key decision initiated this event
			Output::Warning("Branch: Started using Key not implemented");
			break;
		case 9:
			// BGM looped at least once
			result = Audio().BGM_PlayedOnce();
			break;
		case 10:
			value1 = Main_Data::game_party->GetTimer(Main_Data::game_party->Timer2);
			value2 = com.parameters[1] * DEFAULT_FPS;
			switch (com.parameters[2]) {
				case 0:
					result = (value1 >= value2);
					break;
				case 1:
					result = (value1 <= value2);
					break;
			}
			break;
		case 11:
			// RPG Maker 2003 v1.11 features
			switch (com.parameters[1]) {
				case 0:
					// Any savestate available
					result = FileFinder::HasSavegame(*FileFinder::CreateSaveDirectoryTree());
					break;
				case 1:
					// Is Test Play mode?
					result = Player::debug_flag;
					break;
				case 2:
					// Is ATB wait?
					Output::Warning("Branch: Is ATB wait not implemented");
					break;
				case 3:
					// Is Fullscreen active?
					result = DisplayUi->IsFullscreen();
					break;

			}
			break;
		default:
			Output::Warning("Branch %d unsupported", com.parameters[0]);
	}

	if (result)
		return true;

	return SkipTo(Cmd::ElseBranch, Cmd::EndBranch);
}
