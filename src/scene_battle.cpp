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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <algorithm>
#include "input.h"
#include "output.h"
#include "player.h"
#include "sprite.h"
#include "game_system.h"
#include "game_temp.h"
#include "game_party.h"
#include "scene_battle.h"

////////////////////////////////////////////////////////////
Scene_Battle::Scene_Battle() {
	Scene::type = Scene::Battle;
}

////////////////////////////////////////////////////////////
void Scene_Battle::Start() {
	if (Player::battle_test_flag) {
		if (Player::battle_test_troop_id <= 0) {
			Output::Error("Invalid Monster Party Id");
		}
	}

	help_window = new Window_Help(0, 0, 320, 32);
	help_window->SetVisible(false);

	std::vector<std::string> options;
	options.push_back(Data::terms.battle_fight);
	options.push_back(Data::terms.battle_auto);
	options.push_back(Data::terms.battle_escape);

	options_window = new Window_BattleCommand(0, 172, 76, 68, options);

	status_window = new Window_BattleStatus();

	std::vector<std::string> commands;
	commands.push_back(Data::terms.command_attack);
	commands.push_back(Data::terms.command_defend);
	commands.push_back(Data::terms.command_item);
	commands.push_back(Data::terms.command_skill);

	command_window = new Window_BattleCommand(244, 172, 76, 68, commands);

	skill_window = new Window_Skill(0, 172, 320, 68);
	skill_window->SetVisible(false);
	skill_window->SetHelpWindow(help_window);

	item_window = new Window_Item(0, 172, 320, 68);
	item_window->SetVisible(false);
	item_window->SetHelpWindow(help_window);

	SetState(Options);

	std::fill(status_window->gauges, status_window->gauges+4, 0);
}

////////////////////////////////////////////////////////////
void Scene_Battle::SetState(Scene_Battle::State _state) {
	state = _state;

	options_window->SetActive(false);
	status_window->SetActive(false);
	command_window->SetActive(false);
	item_window->SetActive(false);
	skill_window->SetActive(false);

	switch (state) {
		case Options:
			options_window->SetActive(true);
			break;
		case Battle:
			status_window->SetActive(true);
			break;
		case AutoBattle:
			break;
		case Command:
			command_window->SetActive(true);
			break;
		case Item:
			item_window->SetActive(true);
			break;
		case Skill:
			skill_window->SetActive(true);
			skill_window->SetActor(GetActiveActor());
			break;
	}

	options_window->SetVisible(false);
	status_window->SetVisible(false);
	command_window->SetVisible(false);
	item_window->SetVisible(false);
	skill_window->SetVisible(false);
	help_window->SetVisible(false);

	switch (state) {
		case Options:
			options_window->SetVisible(true);
			status_window->SetVisible(true);
			status_window->SetX(76);
			break;
		case Battle:
		case AutoBattle:
		case Command:
			status_window->SetVisible(true);
			status_window->SetX(0);
			command_window->SetVisible(true);
			break;
		case Item:
			item_window->SetVisible(true);
			help_window->SetVisible(true);
			break;
		case Skill:
			skill_window->SetVisible(true);
			help_window->SetVisible(true);
			break;
	}
}

////////////////////////////////////////////////////////////
void Scene_Battle::Update() {
	options_window->Update();
	status_window->Update();
	command_window->Update();

	if (Input::IsTriggered(Input::DECISION)) {
		Game_System::SePlay(Data::system.decision_se);
		switch (state) {
			case Options:
				switch (options_window->GetIndex()) {
					case 0:
						SetState(Battle);
						break;
					case 1:
						SetState(AutoBattle);
						break;
					case 2:
						if (Game_Temp::battle_escape_mode != 0) {
							Game_Temp::battle_result = Game_Temp::BattleEscape;
							Scene::Pop();
						}
						break;
				}
				break;
			case Battle:
				if (status_window->GetActiveCharacter() >= 0)
					SetState(Command);
				break;
			case AutoBattle:
				// no-op
				break;
			case Command:
				switch (command_window->GetIndex()) {
					case 0:
						// FIXME: attack
						break;
					case 1:
						// FIXME: defend
						break;
					case 2:
						SetState(Item);
						break;
					case 3:
						SetState(Skill);
						break;
				}
				break;
			case Item:
				// FIXME: use item
				SetState(Battle);
				break;
			case Skill:
				// FIXME: use skill
				SetState(Battle);
				break;
		}
	}

	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Data::system.cancel_se);
		switch (state) {
			case Options:
				Scene::Pop();
				break;
			case Battle:
			case AutoBattle:
				SetState(Options);
				break;
			case Command:
				SetState(Battle);
				break;
			case Item:
			case Skill:
				SetState(Command);
				break;
		}
	}

	const std::vector<Game_Actor*>& actors = Game_Party::GetActors();

	switch (state) {
		case Battle:
		case AutoBattle:
			for (int i = 0; i < (int) actors.size(); i++) {
				if (status_window->gauges[i] < Window_BattleStatus::gauge_full)
					status_window->SetTimeGauge(i, status_window->gauges[i] + 1);
			}
			break;
		default:
			break;
	}
}

////////////////////////////////////////////////////////////
int Scene_Battle::GetActiveActor() {
	int idx = status_window->GetActiveCharacter();
	if (idx < 0)
		return idx;
	
	const std::vector<Game_Actor*>& actors = Game_Party::GetActors();
	return actors[idx]->GetId();
}

