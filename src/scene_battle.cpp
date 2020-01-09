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
#include "graphics.h"
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
#include "scene_debug.h"
#include "game_interpreter.h"

Scene_Battle::Scene_Battle() :
	actor_index(0),
	active_actor(NULL)
{
	Scene::type = Scene::Battle;
	Game_Temp::battle_result = Game_Temp::BattleAbort;
}

Scene_Battle::~Scene_Battle() {
	Game_Battle::Quit();
}

void Scene_Battle::Start() {
	// RPG_RT will cancel any active screen flash from the map, including
	// wiping out all flash LSD chunks.
	Main_Data::game_screen->FlashOnce(0, 0, 0, 0, 0);

	if (Game_Battle::battle_test.enabled) {
		Game_Temp::battle_troop_id = Game_Battle::battle_test.troop_id;
	}

	const RPG::Troop* troop = ReaderUtil::GetElement(Data::troops, Game_Temp::battle_troop_id);

	if (!troop) {
		const char* error_msg = "Invalid Monster Party ID %d";
		if (Game_Battle::battle_test.enabled) {
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

	if (Game_Battle::battle_test.enabled) {
		InitBattleTest();
	} else {
		Main_Data::game_enemyparty.reset(new Game_EnemyParty());
		Main_Data::game_enemyparty->Setup(Game_Temp::battle_troop_id);
	}

	Game_Battle::Init();

	cycle = 0;
	auto_battle = false;
	enemy_action = NULL;

	Game_System::BgmPlay(Game_System::GetSystemBGM(Game_System::BGM_Battle));

	CreateUi();

	SetState(State_Start);
}

void Scene_Battle::Continue(SceneType /* prev_scene */) {
	Game_Message::SetWindow(message_window.get());

	// Debug scene / other scene could have changed party status.
	status_window->Refresh();
}

void Scene_Battle::TransitionIn(SceneType prev_scene) {
	if (prev_scene == Scene::Debug) {
		Scene::TransitionIn(prev_scene);
		return;
	}
	Graphics::GetTransition().Init((Transition::TransitionType)Game_System::GetTransition(Game_System::Transition_BeginBattleShow), this, 32);
}

void Scene_Battle::TransitionOut(SceneType next_scene) {
	if (Player::exit_flag
			|| Game_Battle::battle_test.enabled
			|| next_scene == Scene::Debug || next_scene == Scene::Title) {
		Scene::TransitionOut(next_scene);
		return;
	}
	Graphics::GetTransition().Init((Transition::TransitionType)Game_System::GetTransition(Game_System::Transition_EndBattleErase), this, 32, true);
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

	skill_window.reset(new Window_BattleSkill(0, (SCREEN_TARGET_HEIGHT-80), SCREEN_TARGET_WIDTH, 80));
	skill_window->SetHelpWindow(help_window.get());

	status_window.reset(new Window_BattleStatus(0, (SCREEN_TARGET_HEIGHT-80), SCREEN_TARGET_WIDTH - option_command_mov, 80));

	message_window.reset(new Window_Message(0, (SCREEN_TARGET_HEIGHT - 80), SCREEN_TARGET_WIDTH, 80));
	Game_Message::SetWindow(message_window.get());
}

void Scene_Battle::Update() {
	options_window->Update();
	status_window->Update();
	command_window->Update();
	help_window->Update();
	item_window->Update();
	skill_window->Update();
	target_window->Update();
	Game_Message::Update();

	// Query Timer before and after update.
	// If it reached zero during update was a running battle timer.
	int timer1 = Game_Data::GetParty().GetTimerSeconds(Game_Party::Timer1);
	int timer2 = Game_Data::GetParty().GetTimerSeconds(Game_Party::Timer2);
	Game_Data::GetParty().UpdateTimers();
	if ((Game_Data::GetParty().GetTimerSeconds(Game_Party::Timer1) == 0 && timer1 > 0) ||
		(Game_Data::GetParty().GetTimerSeconds(Game_Party::Timer2) == 0 && timer2 > 0)) {
		Scene::Pop();
	}

	bool events_finished = Game_Battle::UpdateEvents();

	if (GetRequestedScene() == Gameover) {
		SetRequestedScene(Null);
		Scene::Push(std::make_shared<Scene_Gameover>());
	}

	if (!Game_Message::IsMessageVisible() && events_finished) {
		ProcessActions();
		ProcessInput();
	}

	auto& interp = Game_Battle::GetInterpreter();

	bool events_running = interp.IsRunning();
	Game_Battle::Update();
	if (events_running && !interp.IsRunning()) {
		// If an event that changed status finishes without displaying a message window,
		// we need this so it can update automatically the status_window
		status_window->Refresh();
	}
	if (interp.IsAsyncPending()) {
		auto aop = interp.GetAsyncOp();
		if (CheckSceneExit(aop)) {
			return;
		}

		// Note: ShowScreen / HideScreen is ignored.
	}

	if (Game_Battle::IsTerminating()) {
		Scene::Pop();
	}
}

bool Scene_Battle::IsWindowMoving() {
	return options_window->IsMovementActive() || status_window->IsMovementActive() || command_window->IsMovementActive();
}

void Scene_Battle::InitBattleTest()
{
	Game_Temp::battle_troop_id = Game_Battle::battle_test.troop_id;
	Game_Data::GetParty().SetupBattleTestMembers();

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
		active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(active_actor, target));
	} else if (previous_state == State_SelectSkill) {
		active_actor->SetBattleAlgorithm(
				std::make_shared<Game_BattleAlgorithm::Skill>(active_actor, target, *skill_window->GetSkill()));
	} else if (previous_state == State_SelectItem) {
		auto* item = item_window->GetItem();
		assert(item);
		if (item->type == RPG::Item::Type_special
				|| (item->use_skill && (item->type == RPG::Item::Type_weapon
						|| item->type == RPG::Item::Type_shield
						|| item->type == RPG::Item::Type_armor
						|| item->type == RPG::Item::Type_helmet
						|| item->type == RPG::Item::Type_accessory)))
		{
			const RPG::Skill* skill = ReaderUtil::GetElement(Data::skills, item->skill_id);
			if (!skill) {
				Output::Warning("EnemySelected: Item %d references invalid skill %d", item->ID, item->skill_id);
				return;
			}
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(active_actor, target, *skill, item));
		} else {
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Item>(active_actor, target, *item));
		}
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
	Game_Actor& target = (Game_Data::GetParty())[status_window->GetIndex()];

	if (previous_state == State_SelectSkill) {
		active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(active_actor, &target, *skill_window->GetSkill()));
	} else if (previous_state == State_SelectItem) {
		auto* item = item_window->GetItem();
		assert(item);
		if (item->type == RPG::Item::Type_special
				|| (item->use_skill && (item->type == RPG::Item::Type_weapon
						|| item->type == RPG::Item::Type_shield
						|| item->type == RPG::Item::Type_armor
						|| item->type == RPG::Item::Type_helmet
						|| item->type == RPG::Item::Type_accessory)))
		{
			const RPG::Skill* skill = ReaderUtil::GetElement(Data::skills, item->skill_id);
			if (!skill) {
				Output::Warning("AllySelected: Item %d references invalid skill %d", item->ID, item->skill_id);
				return;
			}
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(active_actor, &target, *skill, item));
		} else {
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Item>(active_actor, &target, *item));
		}
	} else {
		assert("Invalid previous state for ally selection" && false);
	}

	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
	ActionSelectedCallback(active_actor);
}

void Scene_Battle::AttackSelected() {
	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));

	if (active_actor->HasAttackAll()) {
		active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(active_actor, Main_Data::game_enemyparty.get()));
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
			AssignSkill(skill, item);
			break;
		}
		case RPG::Item::Type_medicine:
			if (item->entire_party) {
				active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Item>(active_actor, &Game_Data::GetParty(), *item_window->GetItem()));
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

	if (!skill || !skill_window->CheckEnable(skill->ID)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
		return;
	}

	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));

	AssignSkill(skill, nullptr);
}

void Scene_Battle::AssignSkill(const RPG::Skill* skill, const RPG::Item* item) {
	switch (skill->type) {
		case RPG::Skill::Type_teleport:
		case RPG::Skill::Type_escape:
		case RPG::Skill::Type_switch: {
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(active_actor, *skill, item));
			ActionSelectedCallback(active_actor);
			return;
		}
		case RPG::Skill::Type_normal:
		case RPG::Skill::Type_subskill:
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
					active_actor, Main_Data::game_enemyparty.get(), *skill, item));
			ActionSelectedCallback(active_actor);
			break;
		case RPG::Skill::Scope_self:
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(
					active_actor, active_actor, *skill, item));
			ActionSelectedCallback(active_actor);
			break;
		case RPG::Skill::Scope_party:
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(
					active_actor, &Game_Data::GetParty(), *skill, item));
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

void Scene_Battle::PrepareBattleAction(Game_Battler* battler) {
	if (battler->GetBattleAlgorithm() == nullptr) {
		return;
	}

	if (!battler->CanAct()) {
		if (battler->GetBattleAlgorithm()->GetType() != Game_BattleAlgorithm::Type::NoMove) {
			battler->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::NoMove>(battler));
			battler->SetCharged(false);
		}
		return;
	}

	if (battler->GetSignificantRestriction() == RPG::State::Restriction_attack_ally) {
		Game_Battler *target = battler->GetType() == Game_Battler::Type_Enemy ?
			Main_Data::game_enemyparty->GetRandomActiveBattler() :
			Game_Data::GetParty().GetRandomActiveBattler();

		battler->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(battler, target));
		battler->SetCharged(false);
		return;
	}

	if (battler->GetSignificantRestriction() == RPG::State::Restriction_attack_enemy) {
		Game_Battler *target = battler->GetType() == Game_Battler::Type_Ally ?
			Main_Data::game_enemyparty->GetRandomActiveBattler() :
			Game_Data::GetParty().GetRandomActiveBattler();

		battler->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(battler, target));
		battler->SetCharged(false);
		return;
	}

	// If we had a state restriction previously but were recovered, we do nothing for this round.
	if (battler->GetBattleAlgorithm()->GetSourceRestrictionWhenStarted() != RPG::State::Restriction_normal) {
		if (battler->GetBattleAlgorithm()->GetType() != Game_BattleAlgorithm::Type::NoMove) {
			battler->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::NoMove>(battler));
			battler->SetCharged(false);
		}
		return;
	}

	// If we can no longer perform the action (no more items, ran out of SP, etc..)
	if (!battler->GetBattleAlgorithm()->ActionIsPossible()) {
		battler->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::NoMove>(battler));
		battler->SetCharged(false);
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
				enemy->GetBattleAlgorithm()->SetSwitchDisable(action->switch_off_id);
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
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(enemy, Game_Data::GetParty().GetRandomActiveBattler()));
			break;
		case RPG::EnemyAction::Basic_dual_attack:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(enemy, Game_Data::GetParty().GetRandomActiveBattler()));
			enemy->GetBattleAlgorithm()->SetRepeat(2);
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
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::SelfDestruct>(enemy, &Game_Data::GetParty()));
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

void Scene_Battle::RemoveActionsForNonExistantBattlers() {
	auto iter = std::remove_if(battle_actions.begin(), battle_actions.end(),
			[](Game_Battler* b) {
				if (!b->Exists()) {
					b->SetBattleAlgorithm(nullptr);
					return true;
				}
				return false;
			});
	battle_actions.erase(iter, battle_actions.end());
}

void Scene_Battle::RemoveCurrentAction() {
	battle_actions.front()->SetBattleAlgorithm(nullptr);
	battle_actions.pop_front();
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

	switch (skill->scope) {
		case RPG::Skill::Scope_enemy:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(enemy, Game_Data::GetParty().GetRandomActiveBattler(), *skill));
			break;
		case RPG::Skill::Scope_ally:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(enemy, Main_Data::game_enemyparty->GetRandomActiveBattler(), *skill));
			break;
		case RPG::Skill::Scope_enemies:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(enemy, &Game_Data::GetParty(), *skill));
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
	assert(for_battler->GetBattleAlgorithm() != nullptr);
	battle_actions.push_back(for_battler);

	if (for_battler->GetType() == Game_Battler::Type_Ally) {
		SetState(State_SelectActor);
	}
}

void Scene_Battle::CallDebug() {
	if (Player::debug_flag) {
		Scene::Push(std::make_shared<Scene_Debug>());
	}
}

