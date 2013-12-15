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
#include <boost/lexical_cast.hpp>
#include "scene_battle_rpg2k3.h"
#include "scene_battle.h"
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
#include "game_interpreter_battle.h"
#include "game_message.h"
#include "game_switches.h"
#include "game_battle.h"
#include "game_battlealgorithm.h"
#include "battle_battler.h"
#include "battle_animation.h"
#include "scene_gameover.h"

Scene_Battle_Rpg2k3::Scene_Battle_Rpg2k3() : Scene_Battle(),
	battle_action_wait(30),
	battle_action_state(BattleActionState_Start)
{
}

Scene_Battle_Rpg2k3::~Scene_Battle_Rpg2k3() {
}

void Scene_Battle_Rpg2k3::Update() {
	switch (state) {
	case State_SelectActor: {
		Game_Battle::UpdateGauges();

		if (status_window->GetIndex() == -1) {
			if (status_window->ChooseActiveCharacter() != -1) {
				RefreshCommandWindow();
			}
		}

		std::vector<Game_Enemy*> enemies = Main_Data::game_enemyparty->GetAliveEnemies();

		for (std::vector<Game_Enemy*>::iterator it = enemies.begin();
			it != enemies.end(); ++it) {
			if ((*it)->IsGaugeFull() && !(*it)->GetBattleAlgorithm()) {
				const RPG::EnemyAction* action = (*it)->ChooseRandomAction();
				if (action) {
					CreateEnemyAction(*it, action);
					(*it)->SetGauge(0);
				}
			}
		}

		break;
	}
	default:;
	}

	for (std::vector<FloatText>::iterator it = floating_texts.begin();
		it != floating_texts.end();) {
		(*it).second -= 1;
		if ((*it).second <= 0) {
			it = floating_texts.erase(it);
		}
		else {
			++it;
		}
	}

	Scene_Battle::Update();
}

void Scene_Battle_Rpg2k3::CreateCursors() {
	BitmapRef system2 = Cache::System2(Data::system.system2_name);

	ally_cursor.reset(new Sprite());
	ally_cursor->SetBitmap(system2);
	ally_cursor->SetSrcRect(Rect(0, 16, 16, 16));
	ally_cursor->SetZ(999);
	ally_cursor->SetVisible(false);

	enemy_cursor.reset(new Sprite());
	enemy_cursor->SetBitmap(system2);
	enemy_cursor->SetSrcRect(Rect(0, 0, 16, 16));
	enemy_cursor->SetZ(999);
	enemy_cursor->SetVisible(false);
}


void Scene_Battle_Rpg2k3::UpdateCursors() {
	/*if (Game_Battle::HaveActiveAlly()) {
		const Battle::Ally& ally = state == State_SelectAllyTarget && Game_Battle::HaveTargetAlly()
			? Game_Battle::GetTargetAlly()
			: Game_Battle::GetActiveAlly();
		ally_cursor->SetVisible(true);
		ally_cursor->SetX(ally.rpg_actor->battle_x - ally_cursor->GetWidth() / 2);
		ally_cursor->SetY(ally.rpg_actor->battle_y - ally.sprite->GetHeight() / 2 - ally_cursor->GetHeight() - 2);
		static const int frames[] = {0,1,2,1};
		int frame = frames[(cycle / 15) % 4];
		ally_cursor->SetSrcRect(Rect(frame * 16, 16, 16, 16));
	}
	else
		ally_cursor->SetVisible(false);

	if (state == State_SelectEnemyTarget && Game_Battle::HaveTargetEnemy()) {
		const Battle::Enemy& enemy = Game_Battle::GetTargetEnemy();
		enemy_cursor->SetVisible(true);
		enemy_cursor->SetX(enemy.member->x + enemy.sprite->GetWidth() / 2 + 2);
		enemy_cursor->SetY(enemy.member->y - enemy_cursor->GetHeight() / 2);
		static const int frames[] = {0,1,2,1};
		int frame = frames[(cycle / 15) % 4];
		enemy_cursor->SetSrcRect(Rect(frame * 16, 0, 16, 16));
	}
	else
		enemy_cursor->SetVisible(false);*/
}

void Scene_Battle_Rpg2k3::DrawFloatText(int x, int y, int color, const std::string& text, int _duration) {
	Rect rect = Font::Default()->GetSize(text);

	BitmapRef graphic = Bitmap::Create(rect.width, rect.height);
	graphic->Clear();
	graphic->TextDraw(-rect.x, -rect.y, color, text);

	Sprite* floating_text = new Sprite();
	floating_text->SetBitmap(graphic);
	floating_text->SetOx(rect.width / 2);
	floating_text->SetOy(rect.height + 5);
	floating_text->SetX(x);
	floating_text->SetY(y);
	floating_text->SetZ(500 + y);

	FloatText float_text = FloatText(EASYRPG_SHARED_PTR<Sprite>(floating_text), _duration);

	floating_texts.push_back(float_text);
}

void Scene_Battle_Rpg2k3::CreateBattleOptionWindow() {
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

void Scene_Battle_Rpg2k3::CreateBattleTargetWindow() {
	std::vector<std::string> commands;
	std::vector<Game_Enemy*> enemies = Main_Data::game_enemyparty->GetAliveEnemies();

	for (std::vector<Game_Enemy*>::iterator it = enemies.begin();
		it != enemies.end(); ++it) {
			commands.push_back((*it)->GetName());
	}

	target_window.reset(new Window_Command(commands, 136, 4));
	target_window->SetHeight(80);
	target_window->SetY(160);
	target_window->SetZ(200);
}

void Scene_Battle_Rpg2k3::CreateBattleCommandWindow() {
	std::vector<std::string> commands;

	Game_Actor& actor = (*Main_Data::game_party)[actor_index];

	const std::vector<uint32_t>& bcmds = actor.GetBattleCommands();
	std::vector<uint32_t>::const_iterator it;
	for (it = bcmds.begin(); it != bcmds.end(); it++) {
		uint32_t bcmd = *it;
		if (bcmd <= 0 || bcmd > Data::battlecommands.commands.size())
			break;
		const RPG::BattleCommand& command = Data::battlecommands.commands[bcmd - 1];
		commands.push_back(command.name);
	}

	command_window.reset(new Window_Command(commands, 76));

	command_window->SetHeight(80);
	command_window->SetX(320 - 76);
	command_window->SetY(160);
}

void Scene_Battle_Rpg2k3::CreateBattleMessageWindow() {
	message_window.reset(new Window_Message(0, 160, 320, 80));
	message_window->SetZ(300);
}

void Scene_Battle_Rpg2k3::RefreshCommandWindow() {
	CreateBattleCommandWindow();
	command_window->SetActive(false);
}

void Scene_Battle_Rpg2k3::SetState(Scene_Battle::State new_state) {
	previous_state = state;
	state = new_state;
	if (state == State_SelectActor && auto_battle)
		state = State_AutoBattle;

	options_window->SetActive(false);
	status_window->SetActive(false);
	command_window->SetActive(false);
	item_window->SetActive(false);
	skill_window->SetActive(false);
	target_window->SetActive(false);

	switch (state) {
	case State_Start:
		break;
	case State_SelectOption:
		options_window->SetActive(true);
		break;
	case State_SelectActor:
		status_window->SetActive(true);
		break;
	case State_AutoBattle:
		// no-op
		break;
	case State_SelectCommand:
		RefreshCommandWindow();
		command_window->SetActive(true);
		break;
	case State_SelectEnemyTarget:
	case State_Battle:
		// no-op
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
	case State_TryEscape:
		// no-op
		break;
	}

	options_window->SetVisible(false);
	status_window->SetVisible(false);
	command_window->SetVisible(false);
	item_window->SetVisible(false);
	skill_window->SetVisible(false);
	help_window->SetVisible(false);
	target_window->SetVisible(false);

	switch (state) {
	case State_Start:
		break;
	case State_SelectOption:
		options_window->SetVisible(true);
		status_window->SetVisible(true);
		status_window->SetX(76);
		status_window->SetIndex(-1);
		status_window->Refresh();
		break;
	case State_AutoBattle:
		// no-op
		break;
	case State_SelectActor:
		command_window->SetIndex(-1);
	case State_SelectCommand:
		status_window->SetVisible(true);
		command_window->SetVisible(true);
		status_window->SetX(0);
		break;
	case State_SelectEnemyTarget:
		status_window->SetVisible(true);
		command_window->SetVisible(true);
		target_window->SetActive(true);
		target_window->SetVisible(true);
		break;
	case State_SelectAllyTarget:
		status_window->SetVisible(true);
		status_window->SetX(0);
		command_window->SetVisible(true);
		break;
	case State_AllyAction:
	case State_EnemyAction:
	case State_Battle:
		// no-op
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
		command_window->SetVisible(true);
		status_window->SetX(0);
		break;
	case State_TryEscape:
		// no-op
		break;
	}
}

void Scene_Battle_Rpg2k3::ProcessActions() {
	if (!battle_actions.empty()) {
		if (battle_actions.front()->IsDead()) {
			// No zombies allowed ;)
			RemoveCurrentAction();
		}
		else if (ProcessBattleAction(battle_actions.front()->GetBattleAlgorithm().get())) {
			RemoveCurrentAction();
			if (CheckWin() ||
				CheckLose() ||
				CheckAbort() ||
				CheckFlee()) {
					return;
			}
		}
	} else {
		NextTurn();
		//actor_index = 0;
		//SetState(State_SelectOption);
	}

	switch (state) {
	case State_Start:
		SetState(State_SelectOption);
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

		break;
	case State_Battle:
		if (!battle_actions.empty()) {
			if (battle_actions.front()->IsDead()) {
				// No zombies allowed ;)
				RemoveCurrentAction();
			}
			else if (ProcessBattleAction(battle_actions.front()->GetBattleAlgorithm().get())) {
				RemoveCurrentAction();
			}
		} else {
			if (CheckWin() ||
				CheckLose() ||
				CheckAbort() ||
				CheckFlee()) {
				return;
			}

			NextTurn();
			actor_index = 0;
			SetState(State_SelectOption);
		}
		break;
	case State_SelectEnemyTarget: {
		static int flash_count = 0;

		Game_Enemy* target = static_cast<Game_Enemy*>(Main_Data::game_enemyparty->GetAliveEnemies()[target_window->GetIndex()]);
		Sprite_Battler* sprite = Game_Battle::GetSpriteset().FindBattler(target);
		if (sprite) {
			++flash_count;

			if (flash_count == 60) {
				sprite->Flash(Color(255, 255, 255, 100), 15);
				flash_count = 0;
			}
		}
		break;
	}
	case State_AllyAction:
	case State_EnemyAction:
	default:
		break;
	}
}

bool Scene_Battle_Rpg2k3::ProcessBattleAction(Game_BattleAlgorithm::AlgorithmBase* action) {
	if (Main_Data::game_screen->IsBattleAnimationWaiting()) {
		return false;
	}

	Sprite_Battler* source_sprite;
	source_sprite = Game_Battle::GetSpriteset().FindBattler(action->GetSource());

	if (source_sprite && !source_sprite->IsIdling()) {
		return false;
	}

	bool first = true;

	switch (battle_action_state) {
	case BattleActionState_Start:
		if (!action->IsDeadTargetValid()) {
			action->SetTarget(action->GetTarget()->GetParty().GetNextAliveBattler(action->GetTarget()));
		}

		//printf("Action: %s\n", action->GetSource()->GetName().c_str());

		action->Execute();

		if (action->GetAnimation()) {
			Main_Data::game_screen->ShowBattleAnimation(
				action->GetAnimation()->ID,
				action->GetTarget()->GetBattleX(),
				action->GetTarget()->GetBattleY(),
				false);
		}

		if (source_sprite) {
			source_sprite->Flash(Color(255, 255, 255, 100), 15);
			source_sprite->SetAnimationState(Sprite_Battler::LeftHand, Sprite_Battler::IdleAnimationAfterFinish);
		}

		if (action->GetStartSe()) {
			Game_System::SePlay(*action->GetStartSe());
		}

		battle_action_state = BattleActionState_Result;
		break;
	case BattleActionState_Result:
		do {
			if (first) {
				first = false;
			} else {
				action->Execute();
			}

			Sprite_Battler* target_sprite = Game_Battle::GetSpriteset().FindBattler(action->GetTarget());
			if (target_sprite) {
				target_sprite->SetAnimationState(Sprite_Battler::Damage, Sprite_Battler::IdleAnimationAfterFinish);
			}

			action->Apply();

			DrawFloatText(
				action->GetTarget()->GetBattleX(),
				action->GetTarget()->GetBattleY(),
				0,
				action->GetSuccess() ? boost::lexical_cast<std::string>(*action->GetAffectedHp()) : Data::terms.miss,
				30);

			status_window->Refresh();
		} while (action->TargetNext());

		if (action->GetResultSe()) {
			Game_System::SePlay(*action->GetResultSe());
		}

		battle_action_wait = 30;

		battle_action_state = BattleActionState_Finished;

		break;
	case BattleActionState_Finished:
		if (battle_action_wait--) {
			return false;
		}
		battle_action_wait = 30;

		Sprite_Battler* target_sprite = Game_Battle::GetSpriteset().FindBattler(action->GetTarget());

		if (action->GetTarget()->IsDead()) {
			if (action->GetDeathSe()) {
				Game_System::SePlay(*action->GetDeathSe());
			}

			if (target_sprite) {
				target_sprite->SetAnimationState(Sprite_Battler::Dead);
			}
		} else {
			if (target_sprite) {
				target_sprite->SetAnimationState(Sprite_Battler::Idle);
			}
		}

		// Reset variables
		battle_action_state = BattleActionState_Start;

		return true;
	}

	return false;
}

void Scene_Battle_Rpg2k3::ProcessInput() {
	if (Input::IsTriggered(Input::DECISION)) {
		switch (state) {
		case State_Start:
			// no-op
			break;
		case State_SelectOption:
			// Interpreter message boxes pop up in this state
			if (!message_window->GetVisible()) {
				OptionSelected();
			}
			break;
		case State_SelectActor:
			if (status_window->GetIndex() >= 0) {
				Game_System::SePlay(Data::system.decision_se);
				SelectNextActor();
			}
			break;
		case State_AutoBattle:
			// no-op
			break;
		case State_SelectCommand:
			CommandSelected();
			break;
		case State_SelectEnemyTarget:
			EnemySelected();
			break;
		case State_SelectAllyTarget:
			//TargetDone();
			break;
		case State_SelectItem:
			ItemSelected();
			break;
		case State_SelectSkill:
			SkillSelected();
			break;
		case State_AllyAction:
		case State_EnemyAction:
		case State_Battle:
			// no-op
			break;
		case State_Victory:
			Scene::Pop();
			break;
		case State_Defeat:
			if (Player::battle_test_flag || Game_Temp::battle_defeat_mode != 0) {
				Scene::Pop();
			} else {
				Scene::Push(EASYRPG_MAKE_SHARED<Scene_Gameover>());
			}
			break;
		case State_TryEscape:
			// no-op
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
			--actor_index;
			SetState(State_SelectActor);
			//SelectPreviousActor();
			break;
		case State_SelectEnemyTarget:
		case State_SelectItem:
		case State_SelectSkill:
			SetState(State_SelectCommand);
			break;
		case State_SelectAllyTarget:
			SetState(State_SelectItem);
			break;
		case State_Battle:
		case State_AllyAction:
		case State_EnemyAction:
			// no-op
			break;
		case State_Victory:
		case State_Defeat:
			Scene::Pop();
			break;
		case State_TryEscape:
			// no-op
			break;
		}
	}
}

void Scene_Battle_Rpg2k3::OptionSelected() {
	Game_System::SePlay(Data::system.decision_se);

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
}

void Scene_Battle_Rpg2k3::CommandSelected() {
	Game_System::SePlay(Data::system.decision_se);

	const RPG::BattleCommand& command = 
		Data::battlecommands.commands[active_actor->GetBattleCommands()[command_window->GetIndex()] - 1];

	switch (command.type) {
	case RPG::BattleCommand::Type_attack:
		AttackSelected();
		break;
	case RPG::BattleCommand::Type_defense:
		DefendSelected();
		break;
	case RPG::BattleCommand::Type_escape:
		//EscapeSelected();
		break;
	case RPG::BattleCommand::Type_item:
		SetState(State_SelectItem);
		break;
	case RPG::BattleCommand::Type_skill:
		SetState(State_SelectSkill);
		break;
	case RPG::BattleCommand::Type_special:
		//SpecialSelected()
		break;
	case RPG::BattleCommand::Type_subskill:
		//SubskillSelected()
		break;
	}
}

void Scene_Battle_Rpg2k3::AttackSelected() {
	Game_System::SePlay(Data::system.decision_se);

	CreateBattleTargetWindow();

	SetState(State_SelectEnemyTarget);
}

void Scene_Battle_Rpg2k3::DefendSelected() {
	Game_System::SePlay(Data::system.decision_se);
}

void Scene_Battle_Rpg2k3::ItemSelected() {
	Game_System::SePlay(Data::system.decision_se);
}

void Scene_Battle_Rpg2k3::SkillSelected() {
	RPG::Skill* skill = skill_window->GetSkill();

	if (!skill || !active_actor->IsSkillUsable(skill->ID)) {
		Game_System::SePlay(Data::system.buzzer_se);
		return;
	}

	Game_System::SePlay(Data::system.decision_se);

	switch (skill->type) {
	case RPG::Skill::Type_teleport:
	case RPG::Skill::Type_escape:
	case RPG::Skill::Type_switch:
		//BeginSkill();
		return;
	case RPG::Skill::Type_normal:
	default:
		break;
	}

	switch (skill->scope) {
	case RPG::Skill::Scope_enemy:
		SetState(State_SelectEnemyTarget);
		break;
	case RPG::Skill::Scope_ally:
		SetState(State_SelectAllyTarget);
		break;
	case RPG::Skill::Scope_enemies:
		active_actor->SetBattleAlgorithm(EASYRPG_MAKE_SHARED<Game_BattleAlgorithm::Skill>(active_actor, Main_Data::game_enemyparty.get(), *skill_window->GetSkill()));
		battle_actions.push_back(active_actor);
		SetState(State_SelectActor);
		break;
	case RPG::Skill::Scope_self:
		active_actor->SetBattleAlgorithm(EASYRPG_MAKE_SHARED<Game_BattleAlgorithm::Skill>(active_actor, active_actor, *skill_window->GetSkill()));
		battle_actions.push_back(active_actor);
		SetState(State_SelectActor);
		break;
	case RPG::Skill::Scope_party: {
		active_actor->SetBattleAlgorithm(EASYRPG_MAKE_SHARED<Game_BattleAlgorithm::Skill>(active_actor, Main_Data::game_party.get(), *skill_window->GetSkill()));
		battle_actions.push_back(active_actor);
		SetState(State_SelectActor);
		break;
								  }
	}
}

void Scene_Battle_Rpg2k3::EnemySelected() {
	Game_Enemy* target = static_cast<Game_Enemy*>(Main_Data::game_enemyparty->GetAliveEnemies()[target_window->GetIndex()]);

	switch (previous_state) {
	case State_SelectCommand:
		active_actor->SetBattleAlgorithm(EASYRPG_MAKE_SHARED<Game_BattleAlgorithm::Normal>(active_actor, target));
		battle_actions.push_back(active_actor);
		break;
	case State_SelectSkill:
		active_actor->SetBattleAlgorithm(EASYRPG_MAKE_SHARED<Game_BattleAlgorithm::Skill>(active_actor, target, *skill_window->GetSkill()));
		battle_actions.push_back(active_actor);
		break;
	case State_SelectItem:
		{
			//active_actor->SetBattleAlgorithm(EASYRPG_MAKE_SHARED<Game_BattleAlgorithm::Item>(active_actor, target, *item_window->GetItem()));
			// battle_actions.push_back(active_actor);
			// Todo
			break;
		}
	default:
		assert("Invalid previous state for enemy selection" && false);
	}

	active_actor->SetGauge(0);
	status_window->SetIndex(-1);

	SetState(State_SelectActor);
}

bool Scene_Battle_Rpg2k3::CheckWin() {
	if (!Main_Data::game_enemyparty->IsAnyAlive()) {
		Game_Temp::battle_result = Game_Temp::BattleVictory;
		SetState(State_Victory);

		int exp = Main_Data::game_enemyparty->GetExp();
		int money = Main_Data::game_enemyparty->GetMoney();

		Game_Message::texts.push_back(Data::terms.victory + "\f");

		std::stringstream ss;
		ss << exp << Data::terms.exp_received << "\f";
		Game_Message::texts.push_back(ss.str());

		ss.str("");
		ss << Data::terms.gold_recieved_a << " " << money << Data::terms.gold << Data::terms.gold_recieved_b << "\f";
		Game_Message::texts.push_back(ss.str());

		message_window->SetHeight(32);
		Game_Message::fixed_position = true;
		Game_Message::position = 0;
		Game_Message::message_waiting = true;

		Game_System::BgmPlay(Data::system.battle_end_music);

		// Update attributes
		std::vector<Game_Battler*> ally_battlers;
		Main_Data::game_party->GetAliveBattlers(ally_battlers);

		for (std::vector<Game_Battler*>::iterator it = ally_battlers.begin();
			it != ally_battlers.end(); ++it) {
				Game_Actor* actor = static_cast<Game_Actor*>(*it);
				actor->ChangeExp(actor->GetExp() + exp, true);
		}
		Main_Data::game_party->GainGold(money);

		return true;
	}

	return false;
}

bool Scene_Battle_Rpg2k3::CheckLose() {
	if (!Main_Data::game_party->IsAnyAlive()) {
		Game_Temp::battle_result = Game_Temp::BattleDefeat;
		SetState(State_Defeat);

		message_window->SetHeight(32);
		Game_Message::fixed_position = true;
		Game_Message::position = 0;
		Game_Message::message_waiting = true;

		Game_Message::texts.push_back(Data::terms.defeat);

		Game_System::BgmPlay(Data::system.gameover_music);

		return true;
	}

	return false;
}

bool Scene_Battle_Rpg2k3::CheckAbort() {
	/*if (!Game_Battle::terminate)
		return;
	Game_Temp::battle_result = Game_Temp::BattleAbort;
	Scene::Pop();*/

	return false;
}

bool Scene_Battle_Rpg2k3::CheckFlee() {
	/*if (!Game_Battle::allies_flee)
		return;
	Game_Battle::allies_flee = false;
	Game_Temp::battle_result = Game_Temp::BattleEscape;
	Scene::Pop();*/

	return false;
}

void Scene_Battle_Rpg2k3::SelectNextActor() {
	if (status_window->GetIndex() < 0) {
		return;
	}

	active_actor = &(*Main_Data::game_party)[status_window->GetIndex()];
	actor_index = status_window->GetIndex();

	RefreshCommandWindow();

	SetState(Scene_Battle::State_SelectCommand);
}
