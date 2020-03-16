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
#include <vector>
#include <sstream>
#include <iomanip>
#include "baseui.h"
#include "cache.h"
#include "input.h"
#include "game_variables.h"
#include "game_switches.h"
#include "game_map.h"
#include "game_system.h"
#include "game_battle.h"
#include "scene_debug.h"
#include "scene_load.h"
#include "scene_save.h"
#include "scene_map.h"
#include "scene_battle.h"
#include "player.h"
#include "window_command.h"
#include "window_varlist.h"
#include "window_numberinput.h"
#include "bitmap.h"
#include "game_party.h"
#include "game_player.h"
#include "data.h"
#include "output.h"
#include "transition.h"

struct Scene_Debug::IndexSet {
	int range_index = 0;
	int range_page = 0;
	int range_page_index = 0;
};

struct Scene_Debug::PrevIndex {
	int main_range_index = 0;

	IndexSet sw;
	IndexSet var;
	IndexSet item;
	IndexSet troop;
	IndexSet map;
	IndexSet event;
};

Scene_Debug::PrevIndex Scene_Debug::prev = {};

Scene_Debug::Scene_Debug() {
	Scene::type = Scene::Debug;
}

void Scene_Debug::ResetPrevIndices() {
	prev = {};
}

void Scene_Debug::Start() {
	CreateRangeWindow();
	CreateVarListWindow();
	CreateNumberInputWindow();

	range_index = prev.main_range_index;
	range_window->SetIndex(range_index);

	range_window->SetActive(true);
	var_window->SetActive(false);
	var_window->Refresh();
}

void Scene_Debug::Update() {
	range_window->Update();
	if (range_index != range_window->GetIndex()){
		range_index = range_window->GetIndex();
		var_window->UpdateList(range_page * 100 + range_index * 10 + 1);
		var_window->Refresh();
	}
	var_window->Update();

	if (numberinput_window->GetActive())
		numberinput_window->Update();

	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
		switch (mode) {
			case eMain:
				prev.main_range_index = range_index;
				Scene::Pop();
				break;
			case eSwitch:
				CancelListOption(prev.sw, 2);
				break;
			case eSwitchSelect:
				CancelListOptionSelect(eSwitch, prev.sw);
				break;
			case eVariable:
				CancelListOption(prev.var, 3);
				break;
			case eVariableSelect:
				CancelListOptionSelect(eVariable, prev.var);
				break;
			case eVariableValue:
				CancelListOptionValue(eVariableSelect);
				break;
			case eGold:
				ReturnToMain(4);
				break;
			case eItem:
				CancelListOption(prev.item, 5);
				break;
			case eItemSelect:
				CancelListOptionSelect(eItem, prev.item);
				break;
			case eItemValue:
				CancelListOptionValue(eItemSelect);
				break;
			case eBattle:
				CancelListOption(prev.troop, 6);
				break;
			case eBattleSelect:
				CancelListOptionSelect(eBattle, prev.troop);
				break;
			case eMap:
				CancelListOption(prev.map, 7);
				break;
			case eMapSelect:
				CancelListOptionSelect(eMap, prev.map);
				break;
			case eMapX:
				CancelListOptionValue(eMapSelect);
				break;
			case eMapY:
				CancelMapSelectY();
				break;
			case eFullHeal:
				ReturnToMain(8);
				break;
			case eCallEvent:
				CancelListOption(prev.event, 9);
				break;
			case eCallEventSelect:
				CancelListOptionSelect(eCallEvent, prev.event);
				break;
			}
	} else if (Input::IsTriggered(Input::DECISION)) {
		switch (mode) {
			case eMain:
				EnterFromMain();
				break;
			case eSwitch:
				EnterFromListOption(eSwitchSelect, prev.sw);
				break;
			case eSwitchSelect:
				DoSwitch();
				break;
			case eVariable:
				EnterFromListOption(eVariableSelect, prev.var);
				break;
			case eVariableSelect:
				if (Main_Data::game_variables->IsValid(GetIndex())) {
					EnterFromListOptionToValue(eVariableValue, Main_Data::game_variables->Get(GetIndex()), 7, true);
				}
				break;
			case eVariableValue:
				DoVariable();
				break;
			case eGold:
				DoGold();
				break;
			case eItem:
				EnterFromListOption(eItemSelect, prev.item);
				break;
			case eItemSelect:
				if (GetIndex() <= static_cast<int>(Data::items.size())) {
					EnterFromListOptionToValue(eItemValue, Main_Data::game_party->GetItemCount(GetIndex()), 2, false);
				}
				break;
			case eItemValue:
				DoItem();
				break;
			case eBattle:
				EnterFromListOption(eBattleSelect, prev.troop);
				break;
			case eMap:
				EnterFromListOption(eMapSelect, prev.map);
				break;
			case eBattleSelect:
				DoBattle();
				break;
			case eMapSelect:
				EnterMapSelectX();
				break;
			case eMapX:
				EnterMapSelectY();
				break;
			case eMapY:
				DoMap();
				break;
			case eFullHeal:
				DoFullHeal();
				break;
			case eCallEvent:
				EnterFromListOption(eCallEventSelect, prev.event);
				break;
			case eCallEventSelect:
				DoCallEvent();
				break;
		}
		Game_Map::SetNeedRefresh(true);
	} else if (range_window->GetActive() && Input::IsRepeated(Input::RIGHT)) {
		if (range_page < GetLastPage()) {
			++range_page;
		} else {
			range_page = 0;
		}
		var_window->UpdateList(range_page * 100 + range_index * 10 + 1);
		UpdateRangeListWindow();
		var_window->Refresh();
	} else if (range_window->GetActive() && Input::IsRepeated(Input::LEFT)) {
		if (range_page > 0) {
			--range_page;
		} else {
			range_page = GetLastPage();
		}
		var_window->UpdateList(range_page * 100 + range_index * 10 + 1);
		UpdateRangeListWindow();
		var_window->Refresh();
	}
}

void Scene_Debug::CreateRangeWindow() {

	std::vector<std::string> ranges;
	for (int i = 0; i < 10; i++)
		ranges.push_back("");
	range_window.reset(new Window_Command(ranges, 96));

	range_window->SetHeight(176);
	range_window->SetY(32);
	UpdateRangeListWindow();
}

void Scene_Debug::UpdateRangeListWindow() {
	switch (mode) {
		case eMain:
			{
				auto addItem = [&](int idx, const char* name, bool battle_ok) {
					range_window->SetItemText(idx, name);
					if (!battle_ok && Game_Battle::IsBattleRunning()) {
						range_window->DisableItem(idx);
					}
				};

				int i = 0;
				addItem(i++, "Save", false);
				addItem(i++, "Load", true);
				addItem(i++, "Switches", true);
				addItem(i++, "Variables", true);
				addItem(i++, Data::terms.gold.c_str(), true);
				addItem(i++, "Items", true);
				addItem(i++, "Battle", false);
				addItem(i++, "Map", false);
				addItem(i++, "Full Heal", true);
				addItem(i++, "Call Event", true);
				while (i < 10) {
					addItem(i++, "", true);
				}
				return;
			}
			break;
		case eSwitch:
		case eSwitchSelect:
		case eVariable:
		case eVariableSelect:
		case eVariableValue:
		case eItem:
		case eItemSelect:
		case eBattle:
		case eBattleSelect:
		case eMap:
		case eMapSelect:
		case eCallEvent:
		case eCallEventSelect:
			{
				const char* prefix = "???";
				switch (mode) {
					case eSwitch:
					case eSwitchSelect:
						prefix = "Sw[";
						break;
					case eVariable:
					case eVariableSelect:
					case eVariableValue:
						prefix = "Vr[";
						break;
					case eItem:
					case eItemSelect:
						prefix = "It[";
						break;
					case eBattle:
					case eBattleSelect:
						prefix = "Tp[";
						break;
					case eMap:
					case eMapSelect:
						prefix = "Mp[";
						break;
					case eCallEvent:
					case eCallEventSelect:
						prefix = "Ce[";
						break;
					default:
						break;
				}
				std::stringstream ss;
				for (int i = 0; i < 10; i++){
					ss.str("");
					ss  << prefix
						<< std::setfill('0')
						<< std::setw(4)
						<< (range_page * 100 + i * 10 + 1)
						<< "-"
						<< std::setw(4)
						<< (range_page * 100 + i * 10 + 10) <<
						"]";
					range_window->SetItemText(i, ss.str());
				}
			}
			break;
		case eGold:
			range_window->SetItemText(0, Data::terms.gold);
			for (int i = 1; i < 10; i++){
				range_window->SetItemText(i, "");
			}
			break;
		case eMapX:
		case eMapY:
			range_window->SetItemText(0, std::string("Map: ") + std::to_string(pending_map_id));
			if (mode == eMapX) {
				range_window->SetItemText(1, "X: ");
				range_window->SetItemText(2, "");
			} else {
				range_window->SetItemText(1, "X: " + std::to_string(pending_map_x));
				range_window->SetItemText(2, "Y: ");
			}
			for (int i = 3; i < 10; i++){
				range_window->SetItemText(i, "");
			}
			break;
		case eFullHeal:
			range_window->SetItemText(0, "Full Heal");
			for (int i = 1; i < 10; i++){
				range_window->SetItemText(i, "");
			}
			break;
		default:
			break;
	}
}

void Scene_Debug::CreateVarListWindow() {
	std::vector<std::string> vars;
	for (int i = 0; i < 10; i++)
		vars.push_back("");
	var_window.reset(new Window_VarList(vars));
	var_window->SetX(range_window->GetWidth());
	var_window->SetY(range_window->GetY());
	var_window->SetVisible(false);

	var_window->UpdateList(range_page * 100 + range_index * 10 + 1);
}

void Scene_Debug::CreateNumberInputWindow() {
	numberinput_window.reset(new Window_NumberInput(105, 104,
		Player::IsRPG2k() ? 12 * 7 + 16 : 12 * 8 + 16, 32));
	numberinput_window->SetVisible(false);
	numberinput_window->SetOpacity(255);
	numberinput_window->SetShowOperator(true);
}

int Scene_Debug::GetIndex() {
	return (range_page * 100 + range_index * 10 + var_window->GetIndex() + 1);
}


int Scene_Debug::GetLastPage() {
	size_t num_elements = 0;
	switch (mode) {
		case eSwitch:
			num_elements = Main_Data::game_switches->GetSize();
			break;
		case eVariable:
			num_elements = Main_Data::game_variables->GetSize();
			break;
		case eItem:
			num_elements = Data::items.size();
			break;
		case eBattle:
			num_elements = Data::troops.size();
			break;
		case eMap:
			num_elements = Data::treemap.maps.size() > 0 ? Data::treemap.maps.back().ID : 0;
			break;
		default: break;
	}

	if (num_elements > 0) {
		return (num_elements - 1) / 100;
	}
	return 0;
}

void Scene_Debug::SetupListOption(Mode m, Window_VarList::Mode winmode, const IndexSet& idx) {
	mode = m;
	range_index = idx.range_index;
	range_page = idx.range_page;
	var_window->SetMode(winmode);
	var_window->UpdateList(range_page * 100 + range_index * 10 + 1);
	range_window->SetIndex(range_index);
	UpdateRangeListWindow();
	var_window->Refresh();
}

void Scene_Debug::UseRangeWindow() {
}

void Scene_Debug::UseVarWindow() {
}

void Scene_Debug::UseNumberWindow() {
}

void Scene_Debug::EnterFromMain() {
	switch (range_window->GetIndex()) {
		case 0:
			if (Game_Battle::IsBattleRunning()) {
				Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
			} else {
				Scene::PopUntil(Scene::Map);
				Scene::Push(std::make_shared<Scene_Save>());
			}
			break;
		case 1:
			Scene::Push(std::make_shared<Scene_Load>());
			break;
		case 2:
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
			SetupListOption(eSwitch, Window_VarList::eSwitch, prev.sw);
			break;
		case 3:
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
			SetupListOption(eVariable, Window_VarList::eVariable, prev.var);
			break;
		case 4:
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
			EnterGold();
			break;
		case 5:
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
			SetupListOption(eItem, Window_VarList::eItem, prev.item);
			break;
		case 6:
			if (Game_Battle::IsBattleRunning()) {
				Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
			} else {
				Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
				SetupListOption(eBattle, Window_VarList::eTroop, prev.troop);
			}
			break;
		case 7:
			if (Game_Battle::IsBattleRunning()) {
				Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
			} else {
				Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
				SetupListOption(eMap, Window_VarList::eMap, prev.map);
			}
			break;
		case 8:
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
			EnterFullHeal();
			break;
		case 9:
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
			SetupListOption(eCallEvent, Window_VarList::eCommonEvent, prev.event);
		default:
			break;
	}
}

void Scene_Debug::EnterFromListOption(Mode m, const IndexSet& idx) {
	range_window->SetActive(false);
	var_window->SetActive(true);
	mode = m;
	var_window->SetIndex(idx.range_page_index);
	var_window->Refresh();
}

void Scene_Debug::EnterFromListOptionToValue(Mode m, int init_value, int digits, bool show_operator) {
	mode = m;
	var_window->SetActive(false);
	var_window->Refresh();
	numberinput_window->SetNumber(init_value);
	numberinput_window->SetShowOperator(show_operator);
	numberinput_window->SetVisible(true);
	numberinput_window->SetActive(true);
	numberinput_window->SetMaxDigits(digits);
	numberinput_window->Refresh();
}

void Scene_Debug::EnterGold() {
	mode = eGold;
	range_window->SetActive(false);
	range_index = 0;
	range_window->SetIndex(range_index);
	numberinput_window->SetNumber(Main_Data::game_party->GetGold());
	numberinput_window->SetShowOperator(false);
	numberinput_window->SetVisible(true);
	numberinput_window->SetActive(true);
	numberinput_window->SetMaxDigits(7);
	numberinput_window->Refresh();
	UpdateRangeListWindow();
}

void Scene_Debug::EnterMapSelectX() {
	auto map_id = GetIndex();
	auto iter = std::lower_bound(Data::treemap.maps.begin(), Data::treemap.maps.end(), map_id,
			[](const RPG::MapInfo& l, int r) { return l.ID < r; });
	if (iter != Data::treemap.maps.end()
			&& iter->ID == map_id
			&& iter->type == RPG::TreeMap::MapType_map
	   ) {

		prev.main_range_index = 7;
		prev.map.range_index = range_index;
		prev.map.range_page = range_page;
		prev.map.range_page_index = var_window->GetIndex();

		var_window->SetActive(false);
		pending_map_id = map_id;
		pending_map_x = 0;
		pending_map_y = 0;
		numberinput_window->SetNumber(pending_map_x);
		numberinput_window->SetShowOperator(false);
		numberinput_window->SetVisible(true);
		numberinput_window->SetActive(true);
		numberinput_window->SetMaxDigits(4);
		numberinput_window->Refresh();
		mode = eMapX;
		UpdateRangeListWindow();
	}
}

void Scene_Debug::EnterMapSelectY() {
	pending_map_x = numberinput_window->GetNumber();
	numberinput_window->SetNumber(pending_map_y);
	mode = eMapY;
	UpdateRangeListWindow();
}

void Scene_Debug::EnterFullHeal() {
	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
	mode = eFullHeal;
	var_window->SetMode(Window_VarList::eHeal);
	var_window->UpdateList(1);
	UpdateRangeListWindow();
	var_window->Refresh();
	var_window->SetActive(true);
	range_window->SetActive(false);
	range_index = 0;
	range_window->SetIndex(range_index);
	range_page = 0;
}

void Scene_Debug::CancelListOption(IndexSet& idx, int from_idx) {
	idx.range_index = range_index;
	idx.range_page = range_page;

	ReturnToMain(from_idx);
}

void Scene_Debug::CancelListOptionSelect(Mode m, IndexSet& idx) {
	mode = m;
	idx.range_page_index = var_window->GetIndex();
	var_window->SetActive(false);
	range_window->SetActive(true);
	var_window->Refresh();
}

void Scene_Debug::CancelListOptionValue(Mode m) {
	mode = m;
	numberinput_window->SetActive(false);
	numberinput_window->SetVisible(false);
	var_window->SetActive(true);
	var_window->Refresh();
}

void Scene_Debug::CancelMapSelectY() {
	numberinput_window->SetNumber(pending_map_x);
	mode = eMapX;
	UpdateRangeListWindow();
}

void Scene_Debug::ReturnToMain(int from_idx) {
	numberinput_window->SetActive(false);
	numberinput_window->SetVisible(false);
	var_window->SetActive(false);
	//var_window->SetVisible(false);
	var_window->SetMode(Window_VarList::eNone);

	mode = eMain;
	range_index = from_idx;
	range_page = 0;
	range_window->SetIndex(range_index);
	range_window->SetActive(true);
	UpdateRangeListWindow();
}


void Scene_Debug::DoSwitch() {
	if (Main_Data::game_switches->IsValid(GetIndex())) {
		Main_Data::game_switches->Flip(GetIndex());
		Game_Map::SetNeedRefresh(true);

		var_window->Refresh();
	}
}

void Scene_Debug::DoVariable() {
	Main_Data::game_variables->Set(GetIndex(), numberinput_window->GetNumber());
	Game_Map::SetNeedRefresh(true);

	var_window->Refresh();

	CancelListOptionValue(eVariableSelect);
}

void Scene_Debug::DoGold() {
	auto delta = numberinput_window->GetNumber() - Main_Data::game_party->GetGold();
	Main_Data::game_party->GainGold(delta);

	ReturnToMain(4);
}

void Scene_Debug::DoItem() {
	auto delta = numberinput_window->GetNumber() - Main_Data::game_party->GetItemCount(GetIndex());
	Main_Data::game_party->AddItem(GetIndex(), delta);

	Game_Map::SetNeedRefresh(true);

	var_window->Refresh();

	CancelListOptionValue(eItemSelect);
}

void Scene_Debug::DoBattle() {
	if (GetIndex() <= static_cast<int>(Data::troops.size())) {
		Scene::PopUntil(Scene::Map);
		if (Scene::instance) {
			prev.main_range_index = 6;
			prev.troop.range_index = range_index;
			prev.troop.range_page = range_page;
			prev.troop.range_page_index = var_window->GetIndex();

			BattleArgs args;
			args.troop_id = GetIndex();
			args.first_strike = false;
			args.allow_escape = true;

			Game_Map::SetupBattle(args);

			Scene::Push(Scene_Battle::Create(std::move(args)));
		}
	}
}

void Scene_Debug::DoMap() {
	int pending_map_y = numberinput_window->GetNumber();
	Scene::PopUntil(Scene::Map);
	if (Scene::instance) {
		Main_Data::game_player->ReserveTeleport(pending_map_id, pending_map_x, pending_map_y, -1, TeleportTarget::eSkillTeleport);
	}
}

void Scene_Debug::DoFullHeal() {
	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_UseItem));
	int id = GetIndex();
	auto actors = Main_Data::game_party->GetActors();
	if (id <= 1) {
		for (auto& actor: actors) {
			actor->FullHeal();
		}
	} else {
		int idx = id - 2;
		if (idx < static_cast<int>(actors.size())) {
			actors[idx]->FullHeal();
		}
	}
	var_window->UpdateList(1);
	var_window->Refresh();
}

void Scene_Debug::DoCallEvent() {
	if (GetIndex() > static_cast<int>(Data::commonevents.size())) {
		return;
	}

	auto& ce = Game_Map::GetCommonEvents()[GetIndex() - 1];

	if (Game_Battle::IsBattleRunning()) {
		Game_Battle::GetInterpreter().Push(&ce);
		Scene::PopUntil(Scene::Battle);
		Output::Debug("Debug Scene Forced execution of common event %d on the battle foreground interpreter.", ce.GetIndex());
	} else {
		Game_Map::GetInterpreter().Push(&ce);
		Scene::PopUntil(Scene::Map);
		Output::Debug("Debug Scene Forced execution of common event %d on the map foreground interpreter.", ce.GetIndex());
	}
}

void Scene_Debug::TransitionIn(SceneType /* prev_scene */) {
	Transition::instance().InitShow(Transition::TransitionCutIn, this);
}

void Scene_Debug::TransitionOut(SceneType /* next_scene */) {
	Transition::instance().InitErase(Transition::TransitionCutOut, this);
}
