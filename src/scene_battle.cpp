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

#include "bitmap.h"
#include "input.h"
#include "output.h"
#include "player.h"
#include "transition.h"
#include "game_battlealgorithm.h"
#include "game_message.h"
#include "game_system.h"
#include "game_temp.h"
#include "game_party.h"
#include "game_enemy.h"
#include "game_enemyparty.h"
#include "game_battle.h"
#include "battle_animation.h"
#include "reader_util.h"
#include "scene_battle.h"
#include "scene_battle_rpg2k.h"
#include "scene_battle_rpg2k3.h"
#include "scene_gameover.h"

Scene_Battle::Scene_Battle() :
	actor_index(0),
	active_actor(NULL),
	skill_item(NULL)
{
	Scene::type = Scene::Battle;
}

Scene_Battle::~Scene_Battle() {
	Game_Battle::Quit();
}

void Scene_Battle::Start() {
	if (Player::battle_test_flag) {
		Game_Temp::battle_troop_id = Player::battle_test_troop_id;
	}

	const RPG::Troop* troop = ReaderUtil::GetElement(Data::troops, Game_Temp::battle_troop_id);

	if (!troop) {
		const char* error_msg = "Invalid Monster Party ID %d";
		if (Player::battle_test_flag) {
			Output::Error(error_msg, Game_Temp::battle_troop_id);
		}
		else {
			Output::Warning(error_msg, Game_Temp::battle_troop_id);
		}
		Game_Temp::battle_result = Game_Temp::BattleVictory;
		Scene::Pop();
		return;
	}

	// Game_Temp::battle_troop_id is valid during the whole battle
	Output::Debug("Starting battle %d (%s)", Game_Temp::battle_troop_id, troop->name.c_str());

	if (Player::battle_test_flag) {
		InitBattleTest();
	} else {
		Main_Data::game_enemyparty.reset(new Game_EnemyParty());
		Main_Data::game_enemyparty->Setup(Game_Temp::battle_troop_id);
	}

	Game_Battle::Init();

	cycle = 0;
	auto_battle = false;
	enemy_action = NULL;

	CreateUi();

	Game_System::BgmPlay(Game_System::GetSystemBGM(Game_System::BGM_Battle));

	SetState(State_Start);
}

void Scene_Battle::TransitionIn() {
	Graphics::GetTransition().Init((Transition::TransitionType)Game_System::GetTransition(Game_System::Transition_BeginBattleShow), this, 32);
}

void Scene_Battle::TransitionOut() {
	if (Player::exit_flag || Player::battle_test_flag || Scene::instance->type == Scene::Title) {
		Scene::TransitionOut();
	}
	else {
		Graphics::GetTransition().Init((Transition::TransitionType)Game_System::GetTransition(Game_System::Transition_EndBattleErase), this, 32, true);
	}
}

void Scene_Battle::DrawBackground() {
	DisplayUi->CleanDisplay();
}

void Scene_Battle::CreateUi() {
	std::vector<std::string> commands;
	commands.push_back(Data::terms.battle_fight);
	commands.push_back(Data::terms.battle_auto);
	commands.push_back(Data::terms.battle_escape);
	options_window.reset(new Window_Command(commands, option_command_mov));
	options_window->SetHeight(80);
	options_window->SetY(SCREEN_TARGET_HEIGHT - 80);

	help_window.reset(new Window_Help(0, 0, SCREEN_TARGET_WIDTH, 32));
	help_window->SetVisible(false);

	item_window.reset(new Window_Item(0, (SCREEN_TARGET_HEIGHT-80), SCREEN_TARGET_WIDTH, 80));
	item_window->SetHelpWindow(help_window.get());
	item_window->Refresh();
	item_window->SetIndex(0);

	skill_window.reset(new Window_Skill(0, (SCREEN_TARGET_HEIGHT-80), SCREEN_TARGET_WIDTH, 80));
	skill_window->SetHelpWindow(help_window.get());

	status_window.reset(new Window_BattleStatus(0, (SCREEN_TARGET_HEIGHT-80), SCREEN_TARGET_WIDTH - option_command_mov, 80));

	message_window.reset(new Window_Message(0, (SCREEN_TARGET_HEIGHT - 80), SCREEN_TARGET_WIDTH, 80));
}

void Scene_Battle::Update() {
	Scene_Battle::State changed_state;

	message_window->Update();

	// Query Timer before and after update.
	// If it reached zero during update was a running battle timer.
	int timer1 = Main_Data::game_party->GetTimer(Game_Party::Timer1);
	int timer2 = Main_Data::game_party->GetTimer(Game_Party::Timer2);
	Main_Data::game_party->UpdateTimers();
	if ((Main_Data::game_party->GetTimer(Game_Party::Timer1) == 0 && timer1 > 0) ||
		(Main_Data::game_party->GetTimer(Game_Party::Timer2) == 0 && timer2 > 0)) {
		Scene::Pop();
	}

	do {
		bool events_finished = Game_Battle::UpdateEvents();
		changed_state = state;

		if (Game_Temp::gameover) {
			Game_Temp::gameover = false;
			Scene::Push(std::make_shared<Scene_Gameover>());
		}

		if (!Game_Message::visible && events_finished) {
			options_window->Update();
			status_window->Update();
			command_window->Update();
			help_window->Update();
			item_window->Update();
			skill_window->Update();
			target_window->Update();

			if (!IsWindowMoving()) {
				ProcessActions();
				ProcessInput();
			}
		}
	} while (changed_state != state && state == State_SelectOption);

	Game_Battle::Update();

	Main_Data::game_screen->Update();

	if (Game_Battle::IsTerminating()) {
		Scene::Pop();
	}
}

bool Scene_Battle::IsWindowMoving() {
	return options_window->IsMovementActive() || status_window->IsMovementActive() || command_window->IsMovementActive();
}

void Scene_Battle::InitBattleTest()
{
	Game_Temp::battle_troop_id = Player::battle_test_troop_id;
	Game_Temp::battle_background = Data::system.battletest_background;
	Game_Battle::SetTerrainId(Data::system.battletest_terrain);

	Main_Data::game_party->SetupBattleTestMembers();

	Main_Data::game_enemyparty.reset(new Game_EnemyParty());
	Main_Data::game_enemyparty->Setup(Game_Temp::battle_troop_id);
}

void Scene_Battle::NextTurn(Game_Battler* battler) {
	Game_Battle::NextTurn(battler);
}

void Scene_Battle::SetAnimationState(Game_Battler* target, int new_state) {
	Sprite_Battler* target_sprite = Game_Battle::GetSpriteset().FindBattler(target);
	if (target_sprite) {
		target_sprite->SetAnimationState(new_state);
	}
}

void Scene_Battle::EnemySelected() {
	std::vector<Game_Battler*> enemies;
	Main_Data::game_enemyparty->GetActiveBattlers(enemies);

	Game_Enemy* target = static_cast<Game_Enemy*>(enemies[target_window->GetIndex()]);

	if (previous_state == State_SelectCommand) {
		const RPG::Item* item = active_actor->GetEquipment(RPG::Item::Type_weapon);
		const RPG::Item* item2 = active_actor->HasTwoWeapons() ? active_actor->GetEquipment(RPG::Item::Type_weapon + 1) : nullptr;
		if ((item && item->dual_attack) || (item2 && item2->dual_attack)) {
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::NormalDual>(active_actor, target));
		}
		else {
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(active_actor, target));
		}
		
	} else if (previous_state == State_SelectSkill || (previous_state == State_SelectItem && skill_item)) {
		if (skill_item) {
			const RPG::Skill* skill = ReaderUtil::GetElement(Data::skills, skill_item->skill_id);
			if (!skill) {
				Output::Warning("EnemySelected: Item %d references invalid skill %d", skill_item->ID, skill_item->skill_id);
				return;
			}
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(active_actor, target, *skill, skill_item));
		} else {
			active_actor->SetBattleAlgorithm(
					std::make_shared<Game_BattleAlgorithm::Skill>(active_actor, target, *skill_window->GetSkill()));
		}
	} else if (previous_state == State_SelectItem) {
		active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Item>(active_actor, target, *item_window->GetItem()));
	} else {
		assert("Invalid previous state for enemy selection" && false);
	}

	for (int i = 0; i < Main_Data::game_enemyparty->GetBattlerCount(); ++i) {
		if (&(*Main_Data::game_enemyparty)[i] == target) {
			Game_Battle::SetEnemyTargetIndex(i);
		}
	}

	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
	ActionSelectedCallback(active_actor);
}

void Scene_Battle::AllySelected() {
	Game_Actor& target = (*Main_Data::game_party)[status_window->GetIndex()];

	if (previous_state == State_SelectSkill || (previous_state == State_SelectItem && skill_item)) {
		if (skill_item) {
			const RPG::Skill* skill = ReaderUtil::GetElement(Data::skills, skill_item->skill_id);
			if (!skill) {
				Output::Warning("AllySelected: Item %d references invalid skill %d", skill_item->ID, skill_item->skill_id);
				return;
			}
		}

		active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(active_actor, &target,
			skill_item ? *ReaderUtil::GetElement(Data::skills, skill_item->skill_id) : *skill_window->GetSkill(), skill_item));
	} else if (previous_state == State_SelectItem) {
		active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Item>(active_actor, &target, *item_window->GetItem()));
	} else {
		assert("Invalid previous state for ally selection" && false);
	}

	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
	ActionSelectedCallback(active_actor);
}

void Scene_Battle::AttackSelected() {
	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));

	const RPG::Item* item = active_actor->GetEquipment(RPG::Item::Type_weapon);
	const RPG::Item* item2 = active_actor->HasTwoWeapons() ? active_actor->GetEquipment(RPG::Item::Type_weapon + 1) : nullptr;
	if ((item && item->attack_all) || (item2 && item2->attack_all)) {
		if ((item && item->dual_attack) || (item2 && item2->dual_attack)) {
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::NormalDual>(active_actor, Main_Data::game_enemyparty.get()));
		}
		else {
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(active_actor, Main_Data::game_enemyparty.get()));
		}
		ActionSelectedCallback(active_actor);
	} else {
		SetState(State_SelectEnemyTarget);
	}
}

void Scene_Battle::DefendSelected() {
	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));

	active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Defend>(active_actor));

	ActionSelectedCallback(active_actor);
}

void Scene_Battle::ItemSelected() {
	const RPG::Item* item = item_window->GetItem();

	skill_item = nullptr;

	if (!item || !item_window->CheckEnable(item->ID)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
		return;
	}

	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));

	switch (item->type) {
		case RPG::Item::Type_normal:
		case RPG::Item::Type_book:
		case RPG::Item::Type_material:
			assert(false);
			return;
		case RPG::Item::Type_weapon:
		case RPG::Item::Type_shield:
		case RPG::Item::Type_armor:
		case RPG::Item::Type_helmet:
		case RPG::Item::Type_accessory:
		case RPG::Item::Type_special: {
			const RPG::Skill* skill = ReaderUtil::GetElement(Data::skills, item->skill_id);
			if (!skill) {
				Output::Warning("ItemSelected: Item %d references invalid skill %d", item->ID, item->skill_id);
				return;
			}
			skill_item = item;
			AssignSkill(skill);
			break;
		}
		case RPG::Item::Type_medicine:
			if (item->entire_party) {
				active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Item>(active_actor, Main_Data::game_party.get(), *item_window->GetItem()));
				ActionSelectedCallback(active_actor);
			} else {
				SetState(State_SelectAllyTarget);
				status_window->SetChoiceMode(Window_BattleStatus::ChoiceMode_All);
			}
			break;
		case RPG::Item::Type_switch:
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Item>(active_actor, *item_window->GetItem()));
			ActionSelectedCallback(active_actor);
			break;
	}
}

void Scene_Battle::SkillSelected() {
	const RPG::Skill* skill = skill_window->GetSkill();

	skill_item = NULL;

	if (!skill || !skill_window->CheckEnable(skill->ID)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
		return;
	}

	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));

	AssignSkill(skill);
}

void Scene_Battle::AssignSkill(const RPG::Skill* skill) {
	switch (skill->type) {
		case RPG::Skill::Type_teleport:
		case RPG::Skill::Type_escape:
		case RPG::Skill::Type_switch: {
			if (skill_item) {
				const RPG::Skill *skill = ReaderUtil::GetElement(Data::skills, skill_item->skill_id);
				if (!skill) {
					Output::Warning("AssignSkill: Item %d references invalid skill %d", skill_item->ID, skill_item->skill_id);
					return;
				}
			}

			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(active_actor, skill_item ? *ReaderUtil::GetElement(Data::skills, skill_item->skill_id) : *skill_window->GetSkill(), skill_item));
			ActionSelectedCallback(active_actor);
			return;
		}
		case RPG::Skill::Type_normal:
		case RPG::Skill::Type_subskill:
		default:
			break;
	}

	switch (skill->scope) {
		case RPG::Skill::Scope_enemies:
		case RPG::Skill::Scope_self:
		case RPG::Skill::Scope_party: {
			if (skill_item) {
				const RPG::Skill *skill = ReaderUtil::GetElement(Data::skills, skill_item->skill_id);
				if (!skill) {
					Output::Warning("AssignSkill: Item %d references invalid skill %d", skill_item->ID, skill_item->skill_id);
					return;
				}
			}
		}
		default:
			break;
	}

	switch (skill->scope) {
		case RPG::Skill::Scope_enemy:
			SetState(State_SelectEnemyTarget);
			break;
		case RPG::Skill::Scope_ally:
			SetState(State_SelectAllyTarget);
			status_window->SetChoiceMode(Window_BattleStatus::ChoiceMode_All);
			break;
		case RPG::Skill::Scope_enemies:
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(
					active_actor, Main_Data::game_enemyparty.get(), skill_item ? *ReaderUtil::GetElement(Data::skills, skill_item->skill_id) : *skill_window->GetSkill(), skill_item));
			ActionSelectedCallback(active_actor);
			break;
		case RPG::Skill::Scope_self:
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(
					active_actor, active_actor, skill_item ? *ReaderUtil::GetElement(Data::skills, skill_item->skill_id) : *skill_window->GetSkill(), skill_item));
			ActionSelectedCallback(active_actor);
			break;
		case RPG::Skill::Scope_party:
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(
					active_actor, Main_Data::game_party.get(), skill_item ? *ReaderUtil::GetElement(Data::skills, skill_item->skill_id) : *skill_window->GetSkill(), skill_item));
			ActionSelectedCallback(active_actor);
			break;
	}
}

std::shared_ptr<Scene_Battle> Scene_Battle::Create()
{
	if (Player::IsRPG2k()) {
		return std::make_shared<Scene_Battle_Rpg2k>();
	}
	else {
		return std::make_shared<Scene_Battle_Rpg2k3>();
	}
}

void Scene_Battle::UpdateBattlerAction(Game_Battler* battler) {
	BattleAlgorithmRef alg = battler->GetBattleAlgorithm();
	if (!battler->CanAct()) {
		battler->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::NoMove>(battler));
		battler->SetCharged(false);
	}
	else if (battler->GetSignificantRestriction() == RPG::State::Restriction_attack_ally) {
		Game_Battler *target = battler->GetType() == Game_Battler::Type_Enemy ?
			Main_Data::game_enemyparty->GetRandomActiveBattler() :
			Main_Data::game_party->GetRandomActiveBattler();

		battler->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(battler, target));
		battler->SetCharged(false);
	}
	else if (battler->GetSignificantRestriction() == RPG::State::Restriction_attack_enemy) {
		Game_Battler *target = battler->GetType() == Game_Battler::Type_Ally ?
			Main_Data::game_enemyparty->GetRandomActiveBattler() :
			Main_Data::game_party->GetRandomActiveBattler();

		battler->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(battler, target));
		battler->SetCharged(false);
	}
	else if (alg->GetType() == "Skill" && alg->GetItem() == nullptr && static_cast<Game_BattleAlgorithm::Skill*>(alg.get())->GetSpCost() > battler->GetSp()) {
		battler->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::NoMove>(battler));
	}
	else if (alg->GetItem() != nullptr && Main_Data::game_party->GetItemCount(alg->GetItem()->ID, false) == 0) {
		battler->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::NoMove>(battler));
	}
}

void Scene_Battle::CreateEnemyAction(Game_Enemy* enemy, const RPG::EnemyAction* action) {
	switch (action->kind) {
		case RPG::EnemyAction::Kind_basic:
			CreateEnemyActionBasic(enemy, action);
			break;
		case RPG::EnemyAction::Kind_skill:
			CreateEnemyActionSkill(enemy, action);
			break;
		case RPG::EnemyAction::Kind_transformation:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Transform>(enemy, action->enemy_id));
			if (action->switch_on) {
				enemy->GetBattleAlgorithm()->SetSwitchEnable(action->switch_on_id);
			}
			if (action->switch_off) {
				enemy->GetBattleAlgorithm()->SetSwitchEnable(action->switch_off_id);
			}
			ActionSelectedCallback(enemy);
	}
}

void Scene_Battle::CreateEnemyActionBasic(Game_Enemy* enemy, const RPG::EnemyAction* action) {
	if (action->kind != RPG::EnemyAction::Kind_basic) {
		return;
	}

	switch (action->basic) {
		case RPG::EnemyAction::Basic_attack:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(enemy, Main_Data::game_party->GetRandomActiveBattler()));
			break;
		case RPG::EnemyAction::Basic_dual_attack:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::NormalDual>(enemy, Main_Data::game_party->GetRandomActiveBattler()));
			break;
		case RPG::EnemyAction::Basic_defense:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Defend>(enemy));
			break;
		case RPG::EnemyAction::Basic_observe:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Observe>(enemy));
			break;
		case RPG::EnemyAction::Basic_charge:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Charge>(enemy));
			break;
		case RPG::EnemyAction::Basic_autodestruction:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::SelfDestruct>(enemy, Main_Data::game_party.get()));
			break;
		case RPG::EnemyAction::Basic_escape:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Escape>(enemy));
			break;
		case RPG::EnemyAction::Basic_nothing:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::NoMove>(enemy));
			break;
	}

	if (action->switch_on) {
		enemy->GetBattleAlgorithm()->SetSwitchEnable(action->switch_on_id);
	}
	if (action->switch_off) {
		enemy->GetBattleAlgorithm()->SetSwitchEnable(action->switch_off_id);
	}

	ActionSelectedCallback(enemy);
}

void Scene_Battle::RemoveCurrentAction() {
	battle_actions.front()->SetBattleAlgorithm(std::shared_ptr<Game_BattleAlgorithm::AlgorithmBase>());
	battle_actions.pop_front();

	if (!battle_actions.empty()) {
		if (battle_actions.front()->IsDead()) {
			// No zombies allowed ;)
			RemoveCurrentAction();
		}
		else {
			UpdateBattlerAction(battle_actions.front());
		}
	}

}

void Scene_Battle::CreateEnemyActionSkill(Game_Enemy* enemy, const RPG::EnemyAction* action) {
	if (action->kind != RPG::EnemyAction::Kind_skill) {
		return;
	}

	RPG::Skill* skill = ReaderUtil::GetElement(Data::skills, action->skill_id);
	if (!skill) {
		Output::Warning("CreateEnemyAction: Enemy can't use invalid skill %d", action->skill_id);
		return;
	}


	switch (skill->type) {
		case RPG::Skill::Type_teleport:
		case RPG::Skill::Type_escape:
			// FIXME: Can enemy use this?
			return;
		case RPG::Skill::Type_switch:
		case RPG::Skill::Type_normal:
		case RPG::Skill::Type_subskill:
		default:
			break;
		}

	switch (skill->scope) {
		case RPG::Skill::Scope_enemy:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(enemy, Main_Data::game_party->GetRandomActiveBattler(), *skill));
			break;
		case RPG::Skill::Scope_ally:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(enemy, Main_Data::game_enemyparty->GetRandomActiveBattler(), *skill));
			break;
		case RPG::Skill::Scope_enemies:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(enemy, Main_Data::game_party.get(), *skill));
			break;
		case RPG::Skill::Scope_self:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(enemy, enemy, *skill));
			break;
		case RPG::Skill::Scope_party:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(enemy, Main_Data::game_enemyparty.get(), *skill));
			break;
	}

	if (action->switch_on) {
		enemy->GetBattleAlgorithm()->SetSwitchEnable(action->switch_on_id);
	}
	if (action->switch_off) {
		enemy->GetBattleAlgorithm()->SetSwitchDisable(action->switch_off_id);
	}

	ActionSelectedCallback(enemy);
}

void Scene_Battle::ActionSelectedCallback(Game_Battler* for_battler) {
	battle_actions.push_back(for_battler);

	if (for_battler->GetType() == Game_Battler::Type_Ally) {
		SetState(State_SelectActor);
	}
}
