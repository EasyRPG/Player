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
#include "player.h"
#include "window_command.h"
#include "window_varlist.h"
#include "window_numberinput.h"
#include "bitmap.h"
#include "game_temp.h"
#include "game_party.h"
#include "game_player.h"
#include "data.h"
#include "output.h"

namespace {
struct PrevIndex {
	int main_range_index = 0;

	struct IndexSet {
		int range_index = 0;
		int range_page = 0;
		int range_page_index = 0;
	};

	IndexSet sw;
	IndexSet var;
	IndexSet item;
	IndexSet troop;
	IndexSet map;
};

static PrevIndex prev = {};

} //namespace

Scene_Debug::Scene_Debug() {
	Scene::type = Scene::Debug;
}

void Scene_Debug::ResetPrevIndicies() {
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
		if (range_window->GetActive())
			if (mode == eMain) {
				prev.main_range_index = range_index;
				Scene::Pop();
			} else {
				if (mode == eSwitch) {
					prev.sw.range_index = range_index;
					prev.sw.range_page = range_page;
					range_index = 2;
				} else if (mode == eVariable) {
					prev.var.range_index = range_index;
					prev.var.range_page = range_page;
					range_index = 3;
				} else if (mode == eItem) {
					prev.item.range_index = range_index;
					prev.item.range_page = range_page;
					range_index = 5;
				} else if (mode == eBattle) {
					prev.troop.range_index = range_index;
					prev.troop.range_page = range_page;
					range_index = 6;
				} else if (mode == eMap) {
					prev.map.range_index = range_index;
					prev.map.range_page = range_page;
					range_index = 7;
				} else if (mode == eFullHeal) {
					range_index = 8;
				} else {
					range_index = 0;
				}
				range_page = 0;
				mode = eMain;
				range_window->SetIndex(range_index);
				var_window->SetMode(Window_VarList::eNone);
				UpdateRangeListWindow();
			}
		else if (var_window->GetActive()) {
			if (mode == eSwitch) {
				prev.sw.range_page_index = var_window->GetIndex();
			} else if (mode == eVariable) {
				prev.var.range_page_index = var_window->GetIndex();
			} else if (mode == eItem) {
				prev.item.range_page_index = var_window->GetIndex();
			} else if (mode == eBattle) {
				prev.troop.range_page_index = var_window->GetIndex();
			} else if (mode == eMap) {
				prev.map.range_page_index = var_window->GetIndex();
			} else if (mode == eFullHeal) {
				range_index = 8;
				range_page = 0;
				mode = eMain;
				range_window->SetIndex(range_index);
				var_window->SetMode(Window_VarList::eNone);
				UpdateRangeListWindow();
			}
			var_window->SetActive(false);
			range_window->SetActive(true);
			var_window->Refresh();
		} else if (numberinput_window->GetActive()) {
			if (mode == eMapY) {
				numberinput_window->SetNumber(pending_map_x);
				mode = eMapX;
				UpdateRangeListWindow();
			} else {
				numberinput_window->SetVisible(false);
				numberinput_window->SetActive(false);
				if (mode == eMapX) {
					mode = eMap;
					UpdateRangeListWindow();
				}
				if (var_window->GetMode() != Window_VarList::eNone) {
					var_window->SetActive(true);
					var_window->Refresh();
				} else {
					range_index = 0;
					if (mode == eGold) {
						range_index = 4;
					}
					mode = eMain;
					range_window->SetActive(true);
					range_window->SetIndex(range_index);
					UpdateRangeListWindow();
				}
			}
		}
	} else if (Input::IsTriggered(Input::DECISION)) {
		var_window->Refresh();
		if (range_window->GetActive()) {
			if (mode == eMain) {
				switch (range_window->GetIndex()) {
					case 0:
						if (Game_Temp::battle_running) {
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
						range_index = prev.sw.range_index;
						range_page = prev.sw.range_page;
						mode = eSwitch;
						var_window->SetMode(Window_VarList::eSwitch);
						var_window->UpdateList(range_page * 100 + range_index * 10 + 1);
						range_window->SetIndex(range_index);
						UpdateRangeListWindow();
						var_window->Refresh();
						break;
					case 3:
						Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
						range_index = prev.var.range_index;
						range_page = prev.var.range_page;
						mode = eVariable;
						var_window->SetMode(Window_VarList::eVariable);
						var_window->UpdateList(range_page * 100 + range_index * 10 + 1);
						range_window->SetIndex(range_index);
						UpdateRangeListWindow();
						var_window->Refresh();
						break;
					case 4:
						Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
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
						break;
					case 5:
						Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
						range_index = prev.item.range_index;
						range_page = prev.item.range_page;
						mode = eItem;
						var_window->SetMode(Window_VarList::eItem);
						var_window->UpdateList(range_page * 100 + range_index * 10 + 1);
						range_window->SetIndex(range_index);
						UpdateRangeListWindow();
						var_window->Refresh();
						break;
					case 6:
						if (Game_Temp::battle_running) {
							Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
						} else {
							Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
							range_index = prev.troop.range_index;
							range_page = prev.troop.range_page;
							var_window->SetIndex(prev.troop.range_page_index);
							mode = eBattle;
							var_window->SetMode(Window_VarList::eTroop);
							var_window->UpdateList(range_page * 100 + range_index * 10 + 1);
							range_window->SetIndex(range_index);
							UpdateRangeListWindow();
							var_window->Refresh();
						}
						break;
					case 7:
						if (Game_Temp::battle_running) {
							Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
						} else {
							Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
							range_index = prev.map.range_index;
							range_page = prev.map.range_page;
							mode = eMap;
							var_window->SetMode(Window_VarList::eMap);
							var_window->UpdateList(range_page * 100 + range_index * 10 + 1);
							range_window->SetIndex(range_index);
							UpdateRangeListWindow();
							var_window->Refresh();
						}
						break;
					case 8:
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
						break;
					default:
						break;
				}
			} else {
				range_window->SetActive(false);
				var_window->SetActive(true);
				if (mode == eSwitch) {
					var_window->SetIndex(prev.sw.range_page_index);
				} else if (mode == eVariable) {
					var_window->SetIndex(prev.var.range_page_index);
				} else if (mode == eItem) {
					var_window->SetIndex(prev.item.range_page_index);
				} else if (mode == eBattle) {
					var_window->SetIndex(prev.troop.range_page_index);
				} else if (mode == eMap) {
					var_window->SetIndex(prev.map.range_page_index);
				}
			}
		} else if (var_window->GetActive()) {
			switch (mode) {
				case eSwitch:
					if (Game_Switches.IsValid(GetIndex())) {
						Game_Switches.Flip(GetIndex());
					}
					break;
				case eVariable:
					if (Game_Variables.IsValid(GetIndex())) {
						var_window->SetActive(false);
						numberinput_window->SetNumber(Game_Variables.Get(GetIndex()));
						numberinput_window->SetShowOperator(true);
						numberinput_window->SetVisible(true);
						numberinput_window->SetActive(true);
						numberinput_window->SetMaxDigits(7);
						numberinput_window->Refresh();
					}
					break;
				case eItem:
					if (GetIndex() <= Data::items.size()) {
						var_window->SetActive(false);
						numberinput_window->SetNumber(Main_Data::game_party->GetItemCount(GetIndex()));
						numberinput_window->SetShowOperator(false);
						numberinput_window->SetVisible(true);
						numberinput_window->SetActive(true);
						numberinput_window->SetMaxDigits(2);
						numberinput_window->Refresh();
					}
					break;
				case eBattle:
					if (GetIndex() <= Data::troops.size()) {
						Scene::PopUntil(Scene::Map);
						if (Scene::instance) {
							prev.main_range_index = 6;
							prev.troop.range_index = range_index;
							prev.troop.range_page = range_page;
							prev.troop.range_page_index = var_window->GetIndex();

							Game_Character *player = Main_Data::game_player.get();
							Game_Battle::SetTerrainId(Game_Map::GetTerrainTag(player->GetX(), player->GetY()));
							Game_Map::SetupBattle();
							Game_Temp::battle_troop_id = GetIndex();
							Game_Temp::battle_formation = 0;
							Game_Temp::battle_escape_mode = 2;
							Game_Temp::battle_defeat_mode = 1;
							Game_Temp::battle_first_strike = 0;
							Game_Temp::battle_result = Game_Temp::BattleVictory;
							Game_Battle::SetBattleMode(0);
							static_cast<Scene_Map*>(Scene::instance.get())->CallBattle();
						}
					}
					break;
				case eMap:
					{
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
					break;
				case eFullHeal:
					{
						int id = GetIndex();
						auto actors = Main_Data::game_party->GetActors();
						if (id <= 1) {
							for (auto& actor: actors) {
								actor->FullHeal();
							}
						} else {
							int idx = id - 2;
							if (idx < (int)actors.size()) {
								actors[idx]->FullHeal();
							}
						}
						var_window->UpdateList(1);
					}
					break;
				default:
					break;
			}
			var_window->Refresh();
		} else if (numberinput_window->GetActive()) {
			if (mode == eVariable) {
				Game_Variables.Set(GetIndex(), numberinput_window->GetNumber());
			} else if (mode == eGold) {
				auto delta = numberinput_window->GetNumber() - Main_Data::game_party->GetGold();
				Main_Data::game_party->GainGold(delta);
				range_index = 4;
			} else if (mode == eItem) {
				auto delta = numberinput_window->GetNumber() - Main_Data::game_party->GetItemCount(GetIndex());
				Main_Data::game_party->AddItem(GetIndex(), delta);
			} else if (mode == eMapX) {
				pending_map_x = numberinput_window->GetNumber();
				numberinput_window->SetNumber(pending_map_y);
				mode = eMapY;
				UpdateRangeListWindow();
			} else if (mode == eMapY) {
				int pending_map_y = numberinput_window->GetNumber();
				Scene::PopUntil(Scene::Map);
				if (Scene::instance) {
					Main_Data::game_player->ReserveTeleport(pending_map_id, pending_map_x, pending_map_y, -1);

					// FIXME: Fixes emscripten, but this should be done in Continue/Resume in scene_map
					FileRequestAsync* request = Game_Map::RequestMap(pending_map_id);
					request->SetImportantFile(true);
					request->Start();
				}
			}
			if (mode != eMapY) {
				numberinput_window->SetActive(false);
				numberinput_window->SetVisible(false);
				if (var_window->GetVisible()) {
					var_window->SetActive(true);
					var_window->Refresh();
				} else {
					mode = eMain;
					range_window->SetIndex(4);
					range_window->SetActive(true);
					UpdateRangeListWindow();
				}
			}
		}
		Game_Map::SetNeedRefresh(Game_Map::Refresh_All);
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
					if (!battle_ok && Game_Temp::battle_running) {
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
				while (i < 10) {
					addItem(i++, "", true);
				}
				return;
			}
			break;
		case eSwitch:
		case eVariable:
		case eItem:
		case eBattle:
		case eMap:
			{
				const char* prefix = "???";
				switch (mode) {
					case eSwitch:
						prefix = "Sw[";
						break;
					case eVariable:
						prefix = "Vr[";
						break;
					case eItem:
						prefix = "It[";
						break;
					case eBattle:
						prefix = "Tp[";
						break;
					case eMap:
						prefix = "Mp[";
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
			num_elements = Game_Switches.GetSize();
			break;
		case eVariable:
			num_elements = Game_Variables.GetSize();
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

