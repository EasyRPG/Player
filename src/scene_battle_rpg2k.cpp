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
#include <algorithm>
#include <sstream>
#include "rpg_battlecommand.h"
#include "input.h"
#include "output.h"
#include "player.h"
#include "sprite.h"
#include "graphics.h"
#include "filefinder.h"
#include "cache.h"
#include "game_system.h"
#include "game_temp.h"
#include "game_party.h"
#include "game_enemy.h"
#include "game_enemyparty.h"
#include "game_switches.h"
#include "game_battle.h"
#include "battle_battler.h"
#include "battle_animation.h"
#include "battle_actions.h"
#include "scene_battle_rpg2k.h"
#include "scene_battle.h"

////////////////////////////////////////////////////////////
Scene_Battle_Rpg2k::Scene_Battle_Rpg2k() : Scene_Battle(),
actor_index(0),
active_actor(NULL)
{
}

////////////////////////////////////////////////////////////
Scene_Battle_Rpg2k::~Scene_Battle_Rpg2k() {
}

void Scene_Battle_Rpg2k::Start() {
	if (Player::battle_test_flag) {
		if (Player::battle_test_troop_id <= 0) {
			Output::Error("Invalid Monster Party Id");
		} else {
			InitBattleTest();
		}
	}

	//Game_Battle::Init(this);

	spriteset.reset(new Spriteset_Battle());
	CreateWindows();

	SetState(State_Start);
}

void Scene_Battle_Rpg2k::Update() {
	options_window->Update();
	status_window->Update();
	command_window->Update();
	help_window->Update();
	item_window->Update();
	skill_window->Update();
	target_window->Update();
	battle_message_window->Update();

	ProcessActions();
	ProcessInput();

	spriteset->Update();
	/*DoAuto();

	UpdateBackground();
	UpdateCursors();
	UpdateSprites();
	UpdateFloaters();
	UpdateAnimations();*/
}

void Scene_Battle_Rpg2k::Terminate() {

}

void Scene_Battle_Rpg2k::InitBattleTest() {
	Game_Temp::battle_troop_id = Player::battle_test_troop_id;
	Game_Temp::battle_background = Data::system.battletest_background;

	Game_EnemyParty::Setup(Game_Temp::battle_troop_id);
}

void Scene_Battle_Rpg2k::CreateWindows() {
	CreateBattleOptionWindow();
	CreateBattleTargetWindow();
	CreateBattleCommandWindow();
	CreateBattleMessageWindow();

	help_window.reset(new Window_Help(0, 0, 320, 32));
	item_window.reset(new Window_Item(0, 160, 320, 80));
	item_window->SetHelpWindow(help_window.get());
	item_window->Refresh();
	item_window->SetIndex(0);

	skill_window.reset(new Window_Skill(0, 160, 320, 80));
	skill_window->SetHelpWindow(help_window.get());

	status_window.reset(new Window_BattleStatus_Rpg2k(0, 160, 320 - 76, 80));
}

void Scene_Battle_Rpg2k::CreateBattleOptionWindow() {
	std::vector<std::string> commands;
	commands.push_back(Data::terms.battle_fight);
	commands.push_back(Data::terms.battle_auto);
	commands.push_back(Data::terms.battle_escape);

	options_window.reset(new Window_Command(commands, 76));
	options_window->SetHeight(80);
	options_window->SetY(160);
	// TODO: Auto Battle not implemented
	options_window->DisableItem(1);
}

void Scene_Battle_Rpg2k::CreateBattleTargetWindow() {
	std::vector<std::string> commands;
	std::vector<Game_Battler*> enemies = Game_EnemyParty::GetAliveEnemies();

	for (std::vector<Game_Battler*>::iterator it = enemies.begin();
		it != enemies.end(); ++it) {
		commands.push_back((*it)->GetName());
	}

	target_window.reset(new Window_Command(commands, 136, 4));
	target_window->SetHeight(80);
	target_window->SetY(160);
	target_window->SetZ(200);
}

void Scene_Battle_Rpg2k::CreateBattleCommandWindow() {
	std::vector<std::string> commands;
	commands.push_back(Data::terms.command_attack);
	commands.push_back(Data::terms.command_skill);
	commands.push_back(Data::terms.command_defend);
	commands.push_back(Data::terms.command_item);

	command_window.reset(new Window_Command(commands, 76));
	command_window->SetHeight(80);
	command_window->SetX(320 - 76);
	command_window->SetY(160);
}


void Scene_Battle_Rpg2k::CreateBattleMessageWindow() {
	battle_message_window.reset(new Window_BattleMessage(0, 160, 320, 80));

	battle_message_window->SetZ(300);
}

void Scene_Battle_Rpg2k::RefreshCommandWindow() {
	std::string skill_name = active_actor->GetSkillName();
	command_window->SetItemText(1,
		skill_name.empty() ? Data::terms.command_skill : skill_name);
}

void Scene_Battle_Rpg2k::SetState(Scene_Battle::State new_state) {
	target_state = state;
	state = new_state;
	if (state == State_SelectActor && auto_battle)
		state = State_AutoBattle;

	options_window->SetActive(false);
	status_window->SetActive(false);
	command_window->SetActive(false);
	item_window->SetActive(false);
	skill_window->SetActive(false);
	target_window->SetActive(false);
	battle_message_window->SetActive(false);

	switch (state) {
	case State_Start:
		battle_message_window->SetActive(true);
		break;
	case State_SelectOption:
		options_window->SetActive(true);
		break;
	case State_SelectActor:
		status_window->SetActive(true);
		break;
	case State_AutoBattle:
		break;
	case State_SelectCommand:
		command_window->SetActive(true);
		RefreshCommandWindow();
		break;
	case State_SelectEnemyTarget:
		break;
	case State_SelectAllyTarget:
		status_window->SetActive(true);
		break;
	case State_SelectItem:
		item_window->SetActive(true);
		//item_window->SetActor(Game_Battle::GetActiveActor());
		item_window->Refresh();
		break;
	case State_SelectSkill:
		skill_window->SetActive(true);
		skill_window->SetActor(active_actor->GetId());
		skill_window->SetIndex(0);
		break;
	case State_AllyAction:
	case State_EnemyAction:
	case State_Victory:
	case State_Defeat:
		break;
	}

	options_window->SetVisible(false);
	status_window->SetVisible(false);
	command_window->SetVisible(false);
	item_window->SetVisible(false);
	skill_window->SetVisible(false);
	help_window->SetVisible(false);
	target_window->SetVisible(false);
	battle_message_window->SetVisible(false);

	switch (state) {
	case State_Start:
		battle_message_window->SetVisible(true);
		break;
	case State_SelectOption:
		options_window->SetVisible(true);
		status_window->SetVisible(true);
		status_window->SetX(76);
		status_window->SetIndex(-1);
		break;
	case State_SelectActor:
		ActivateNextActor();
		break;
	case State_AutoBattle:
		// no-op
		break;
	case State_SelectCommand:
		status_window->SetVisible(true);
		command_window->SetVisible(true);
		status_window->SetX(0);
		break;
	case State_SelectEnemyTarget:
		CreateBattleTargetWindow();
		status_window->SetVisible(true);
		command_window->SetVisible(true);
		target_window->SetActive(true);
		target_window->SetVisible(true);
		break;
	case State_SelectAllyTarget:
	case State_AllyAction:
	case State_EnemyAction:
		status_window->SetVisible(true);
		status_window->SetX(0);
		command_window->SetVisible(true);
		break;
	case State_SelectItem:
		item_window->SetVisible(true);
		item_window->SetHelpWindow(help_window.get());
		help_window->SetVisible(true);
		break;
	case State_SelectSkill:
		skill_window->SetVisible(true);
		skill_window->SetHelpWindow(help_window.get());
		help_window->SetVisible(true);
		break;
	case State_Victory:
	case State_Defeat:
		status_window->SetVisible(true);
		status_window->SetX(0);
		command_window->SetVisible(true);
		help_window->SetVisible(true);
		break;
	}
}

void Scene_Battle_Rpg2k::ProcessActions() {
	switch (state) {
	case State_Start:
		if (DisplayMonstersInMessageWindow()) {
			SetState(State_SelectOption);
		}
		break;
	case State_SelectActor:
	case State_AutoBattle:
		Game_Battle::Update();

		CheckWin();
		CheckLose();
		CheckAbort();
		CheckFlee();

		if (help_window->GetVisible() && message_timer > 0) {
			message_timer--;
			if (message_timer <= 0)
				help_window->SetVisible(false);
		}

		while (Game_Battle::NextActiveEnemy())
			EnemyAction();

		break;
	case State_AllyAction:
	case State_EnemyAction:
		/*if (!actions.empty()) {
			Battle::Action* action = actions.front();
			if ((*action)()) {
				delete action;
				actions.pop_front();
			}
		}*/
	default:
		break;
	}
}

void Scene_Battle_Rpg2k::ProcessInput() {
	if (Input::IsTriggered(Input::DECISION)) {
		Game_System::SePlay(Data::system.decision_se);
		switch (state) {
		case State_Start:
			// no-op
			break;
		case State_SelectOption:
			switch (options_window->GetIndex()) {
			case 0: // Battle
				auto_battle = false;
				SetState(State_SelectActor);
				break;
			case 1: // Auto Battle
				auto_battle = true;
				Output::Post("Auto Battle not implemented yet.\nSorry :)");
				//SetState(State_SelectActor);
				break;
			case 2: // Escape
				//Escape();
				break;
			}
			break;
		case State_SelectActor:
			SetState(State_SelectCommand);
			ActivateNextActor();
			break;
		case State_AutoBattle:
			// no-op
			break;
		case State_SelectCommand:
			CommandSelected();
			break;
		case State_SelectEnemyTarget:
		case State_SelectAllyTarget:
			//TargetDone();
			break;
		case State_SelectItem:
			Item();
			break;
		case State_SelectSkill:
			Skill();
			break;
		case State_AllyAction:
		case State_EnemyAction:
			break;
		case State_Victory:
		case State_Defeat:
			Scene::Pop();
			break;
		}
	}

	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Data::system.cancel_se);
		switch (state) {
		case State_Start:
		case State_SelectOption:
			// no-op
			break;
		case State_SelectActor:
		case State_AutoBattle:
			SetState(State_SelectOption);
			break;
		case State_SelectCommand:
			SetState(State_SelectOption);
			break;
		case State_SelectEnemyTarget:
		case State_SelectItem:
		case State_SelectSkill:
			SetState(State_SelectCommand);
			break;
		case State_SelectAllyTarget:
			SetState(State_SelectItem);
			break;
		case State_AllyAction:
		case State_EnemyAction:
			// no-op
			break;
		case State_Victory:
		case State_Defeat:
			Scene::Pop();
			break;
		}
	}

	/*if (state == State_SelectEnemyTarget && Game_Battle::HaveTargetEnemy()) {
		if (Input::IsRepeated(Input::DOWN))
			Game_Battle::TargetNextEnemy();
		if (Input::IsRepeated(Input::UP))
			Game_Battle::TargetPreviousEnemy();
		Game_Battle::ChooseEnemy();
	}

	if (state == State_SelectAllyTarget && Game_Battle::HaveTargetAlly()) {
		if (Input::IsRepeated(Input::DOWN))
			Game_Battle::TargetNextAlly();
		if (Input::IsRepeated(Input::UP))
			Game_Battle::TargetPreviousAlly();
	}*/
}

void Scene_Battle_Rpg2k::CommandSelected() {
	switch (command_window->GetIndex()) {
		case 0: // Attack
			SetState(State_SelectEnemyTarget);
			break;
		case 1: // Skill
			SetState(State_SelectSkill);
			break;
		case 2: // Defense
			// Defend();
			break;
		case 3: // Item
			SetState(State_SelectItem);
			break;
		default:
			// no-op
			break;
	}
}

void Scene_Battle_Rpg2k::Attack() {

}
	
void Scene_Battle_Rpg2k::Defend() {

}

void Scene_Battle_Rpg2k::Item() {

}
	
void Scene_Battle_Rpg2k::Skill() {

}

void Scene_Battle_Rpg2k::ActivateNextActor() {
	std::vector<Game_Actor*> allies = Game_Party::GetActors();

	for (size_t i = 0; i < allies.size(); ++i) {
		if ((size_t)actor_index == allies.size()) {
			// ToDo Start Battle
			SetState(Scene_Battle::State_SelectActor);
			return;
		}

		active_actor = allies[actor_index];
		status_window->SetIndex(actor_index);
		actor_index++;

		if (active_actor->GetAutoBattle()) {
			// Automatic stuff
		}
		break;
	}

	SetState(Scene_Battle::State_SelectCommand);
}

void Scene_Battle_Rpg2k::ActivatePreviousActor() {
}

bool Scene_Battle_Rpg2k::DisplayMonstersInMessageWindow() {
	static int tick_count = 0;
	static int next_monster = 0;
	static int tick_limit = 8;
	++tick_count;

	if (tick_count == tick_limit) {
		const boost::ptr_vector<Game_Battler>& enemies = Game_EnemyParty::GetEnemies();
		if ((size_t)next_monster < enemies.size()) {
			battle_message_window->AddMessage(enemies[next_monster].GetName() + Data::terms.encounter);
			tick_count = 0;
			next_monster++;
			tick_limit = 8;

			if (battle_message_window->AllLinesFilled() || (size_t)next_monster == enemies.size()) {
				tick_limit = 15;
			}
		} else {
			return true;
		}
	}
	return false;
}
