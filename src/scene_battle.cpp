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
#include "game_party.h"
#include "game_enemy.h"
#include "game_enemyparty.h"
#include "game_battle.h"
#include "game_screen.h"
#include "game_pictures.h"
#include "battle_animation.h"
#include <lcf/reader_util.h>
#include "scene_battle.h"
#include "scene_battle_rpg2k.h"
#include "scene_battle_rpg2k3.h"
#include "scene_gameover.h"
#include "scene_debug.h"
#include "game_interpreter.h"

Scene_Battle::Scene_Battle(const BattleArgs& args)
	: troop_id(args.troop_id),
	allow_escape(args.allow_escape),
	on_battle_end(args.on_battle_end)
{
	SetUseSharedDrawables(true);

	Scene::type = Scene::Battle;

	// Face graphic is cleared when battle scene is created.
	// Even if the battle gets interrupted by another scene and never starts.
	Game_Message::ClearFace();
	Game_System::SetBeforeBattleMusic(Game_System::GetCurrentBGM());
	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_BeginBattle));
	Game_System::BgmPlay(Game_System::GetSystemBGM(Game_System::BGM_Battle));

	Game_Battle::SetTerrainId(args.terrain_id);
	Game_Battle::ChangeBackground(args.background);
	Game_Battle::SetBattleCondition(args.condition);
}

Scene_Battle::~Scene_Battle() {
	Game_Battle::Quit();
}

void Scene_Battle::Start() {
	if (Scene::Find(Scene::Map) == nullptr) {
		// Battletest mode - need to initialize screen
		Main_Data::game_screen->InitGraphics();
		Main_Data::game_pictures->InitGraphics();
	}

	// RPG_RT will cancel any active screen flash from the map, including
	// wiping out all flash LSD chunks.
	Main_Data::game_screen->FlashOnce(0, 0, 0, 0, 0);

	const lcf::rpg::Troop* troop = lcf::ReaderUtil::GetElement(lcf::Data::troops, troop_id);

	if (!troop) {
		Output::Warning("Invalid Monster Party ID {}", troop_id);
		EndBattle(BattleResult::Victory);
		return;
	}

	Output::Debug("Starting battle {} ({})", troop_id, troop->name);

	if (Game_Battle::battle_test.enabled) {
		Main_Data::game_party->SetupBattleTestMembers();
	}
	Main_Data::game_enemyparty.reset(new Game_EnemyParty());
	Main_Data::game_enemyparty->Setup(troop_id);

	Game_Battle::Init(troop_id);

	// Update picture visibility
	Main_Data::game_pictures->UpdateGraphics(true);

	cycle = 0;
	auto_battle = false;
	enemy_action = NULL;

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
	Transition::instance().InitShow(Game_System::GetTransition(Game_System::Transition_BeginBattleShow), this);
}

void Scene_Battle::TransitionOut(SceneType next_scene) {
	auto& transition = Transition::instance();

	if (next_scene == Scene::Debug) {
		transition.InitErase(Transition::TransitionCutOut, this);
		return;
	}

	if (next_scene == Scene::Null || next_scene == Scene::Title) {
		Scene::TransitionOut(next_scene);
		return;
	}

	transition.InitErase(Game_System::GetTransition(Game_System::Transition_EndBattleErase), this);
}

void Scene_Battle::DrawBackground(Bitmap& dst) {
	dst.Clear();
}

void Scene_Battle::CreateUi() {
	std::vector<std::string> commands;
	commands.push_back(lcf::Data::terms.battle_fight);
	commands.push_back(lcf::Data::terms.battle_auto);
	commands.push_back(lcf::Data::terms.battle_escape);
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

	const int timer1 = Main_Data::game_party->GetTimerSeconds(Game_Party::Timer1);
	const int timer2 = Main_Data::game_party->GetTimerSeconds(Game_Party::Timer2);

	// Update Battlers
	std::vector<Game_Battler*> battlers;
	Main_Data::game_party->GetBattlers(battlers);
	Main_Data::game_enemyparty->GetBattlers(battlers);
	for (auto* b : battlers) {
		b->UpdateBattle();
	}

	// Screen Effects
	Game_Message::Update();
	Main_Data::game_party->UpdateTimers();
	Main_Data::game_screen->Update();
	Main_Data::game_pictures->Update(true);
	Game_Battle::UpdateAnimation();

	// Query Timer before and after update.
	// If it reached zero during update was a running battle timer.
	if ((Main_Data::game_party->GetTimerSeconds(Game_Party::Timer1) == 0 && timer1 > 0) ||
		(Main_Data::game_party->GetTimerSeconds(Game_Party::Timer2) == 0 && timer2 > 0)) {
		EndBattle(BattleResult::Abort);
		return;
	}

	bool events_finished = Game_Battle::UpdateEvents();

	auto call = TakeRequestedScene();
	if (call && call->type == Scene::Gameover) {
		Scene::Push(std::move(call));
	}

	if (!Game_Message::IsMessageActive() && events_finished) {
		ProcessActions();
		ProcessInput();
	}
	UpdateCursors();

	auto& interp = Game_Battle::GetInterpreter();

	bool events_running = interp.IsRunning();
	interp.Update();

	UpdateGraphics();
	if (events_running && !interp.IsRunning()) {
		// If an event that changed status finishes without displaying a message window,
		// we need this so it can update automatically the status_window
		status_window->Refresh();
	}
	if (interp.IsAsyncPending()) {
		auto aop = interp.GetAsyncOp();

		if (aop.GetType() == AsyncOp::eTerminateBattle) {
			EndBattle(static_cast<BattleResult>(aop.GetBattleResult()));
			return;
		}

		if (CheckSceneExit(aop)) {
			return;
		}
	}
}

void Scene_Battle::UpdateGraphics() {
	Game_Battle::UpdateGraphics();
}

bool Scene_Battle::IsWindowMoving() {
	return options_window->IsMovementActive() || status_window->IsMovementActive() || command_window->IsMovementActive();
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
		if (item->type == lcf::rpg::Item::Type_special
				|| (item->use_skill && (item->type == lcf::rpg::Item::Type_weapon
						|| item->type == lcf::rpg::Item::Type_shield
						|| item->type == lcf::rpg::Item::Type_armor
						|| item->type == lcf::rpg::Item::Type_helmet
						|| item->type == lcf::rpg::Item::Type_accessory)))
		{
			const lcf::rpg::Skill* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, item->skill_id);
			if (!skill) {
				Output::Warning("EnemySelected: Item {} references invalid skill {}", item->ID, item->skill_id);
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
	Game_Actor& target = (*Main_Data::game_party)[status_window->GetIndex()];

	if (previous_state == State_SelectSkill) {
		active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(active_actor, &target, *skill_window->GetSkill()));
	} else if (previous_state == State_SelectItem) {
		auto* item = item_window->GetItem();
		assert(item);
		if (item->type == lcf::rpg::Item::Type_special
				|| (item->use_skill && (item->type == lcf::rpg::Item::Type_weapon
						|| item->type == lcf::rpg::Item::Type_shield
						|| item->type == lcf::rpg::Item::Type_armor
						|| item->type == lcf::rpg::Item::Type_helmet
						|| item->type == lcf::rpg::Item::Type_accessory)))
		{
			const lcf::rpg::Skill* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, item->skill_id);
			if (!skill) {
				Output::Warning("AllySelected: Item {} references invalid skill {}", item->ID, item->skill_id);
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
	const lcf::rpg::Item* item = item_window->GetItem();

	if (!item || !item_window->CheckEnable(item->ID)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
		return;
	}

	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));

	switch (item->type) {
		case lcf::rpg::Item::Type_normal:
		case lcf::rpg::Item::Type_book:
		case lcf::rpg::Item::Type_material:
			assert(false);
			return;
		case lcf::rpg::Item::Type_weapon:
		case lcf::rpg::Item::Type_shield:
		case lcf::rpg::Item::Type_armor:
		case lcf::rpg::Item::Type_helmet:
		case lcf::rpg::Item::Type_accessory:
		case lcf::rpg::Item::Type_special: {
			const lcf::rpg::Skill* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, item->skill_id);
			if (!skill) {
				Output::Warning("ItemSelected: Item {} references invalid skill {}", item->ID, item->skill_id);
				return;
			}
			AssignSkill(skill, item);
			break;
		}
		case lcf::rpg::Item::Type_medicine:
			if (item->entire_party) {
				active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Item>(active_actor, Main_Data::game_party.get(), *item_window->GetItem()));
				ActionSelectedCallback(active_actor);
			} else {
				SetState(State_SelectAllyTarget);
				status_window->SetChoiceMode(Window_BattleStatus::ChoiceMode_All);
			}
			break;
		case lcf::rpg::Item::Type_switch:
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Item>(active_actor, *item_window->GetItem()));
			ActionSelectedCallback(active_actor);
			break;
	}
}

void Scene_Battle::SkillSelected() {
	const lcf::rpg::Skill* skill = skill_window->GetSkill();

	if (!skill || !skill_window->CheckEnable(skill->ID)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
		return;
	}

	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));

	AssignSkill(skill, nullptr);
}

void Scene_Battle::AssignSkill(const lcf::rpg::Skill* skill, const lcf::rpg::Item* item) {
	switch (skill->type) {
		case lcf::rpg::Skill::Type_teleport:
		case lcf::rpg::Skill::Type_escape:
		case lcf::rpg::Skill::Type_switch: {
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(active_actor, *skill, item));
			ActionSelectedCallback(active_actor);
			return;
		}
		case lcf::rpg::Skill::Type_normal:
		case lcf::rpg::Skill::Type_subskill:
		default:
			break;
	}

	switch (skill->scope) {
		case lcf::rpg::Skill::Scope_enemy:
			SetState(State_SelectEnemyTarget);
			break;
		case lcf::rpg::Skill::Scope_ally:
			SetState(State_SelectAllyTarget);
			status_window->SetChoiceMode(Window_BattleStatus::ChoiceMode_All);
			break;
		case lcf::rpg::Skill::Scope_enemies:
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(
					active_actor, Main_Data::game_enemyparty.get(), *skill, item));
			ActionSelectedCallback(active_actor);
			break;
		case lcf::rpg::Skill::Scope_self:
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(
					active_actor, active_actor, *skill, item));
			ActionSelectedCallback(active_actor);
			break;
		case lcf::rpg::Skill::Scope_party:
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(
					active_actor, Main_Data::game_party.get(), *skill, item));
			ActionSelectedCallback(active_actor);
			break;
	}
}

std::shared_ptr<Scene_Battle> Scene_Battle::Create(const BattleArgs& args)
{
	if (Player::IsRPG2k()) {
		return std::make_shared<Scene_Battle_Rpg2k>(args);
	}
	else {
		return std::make_shared<Scene_Battle_Rpg2k3>(args);
	}
}

void Scene_Battle::PrepareBattleAction(Game_Battler* battler) {
	if (battler->GetBattleAlgorithm() == nullptr) {
		return;
	}

	if (!battler->CanAct()) {
		if (battler->GetBattleAlgorithm()->GetType() != Game_BattleAlgorithm::Type::NoMove) {
			battler->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::NoMove>(battler));
		}
		return;
	}

	if (battler->GetSignificantRestriction() == lcf::rpg::State::Restriction_attack_ally) {
		Game_Battler *target = battler->GetType() == Game_Battler::Type_Enemy ?
			Main_Data::game_enemyparty->GetRandomActiveBattler() :
			Main_Data::game_party->GetRandomActiveBattler();

		battler->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(battler, target));
		return;
	}

	if (battler->GetSignificantRestriction() == lcf::rpg::State::Restriction_attack_enemy) {
		Game_Battler *target = battler->GetType() == Game_Battler::Type_Ally ?
			Main_Data::game_enemyparty->GetRandomActiveBattler() :
			Main_Data::game_party->GetRandomActiveBattler();

		battler->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(battler, target));
		return;
	}

	// If we had a state restriction previously but were recovered, we do nothing for this round.
	if (battler->GetBattleAlgorithm()->GetSourceRestrictionWhenStarted() != lcf::rpg::State::Restriction_normal) {
		if (battler->GetBattleAlgorithm()->GetType() != Game_BattleAlgorithm::Type::NoMove) {
			battler->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::NoMove>(battler));
		}
		return;
	}

	// If we can no longer perform the action (no more items, ran out of SP, etc..)
	if (!battler->GetBattleAlgorithm()->ActionIsPossible()) {
		battler->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::NoMove>(battler));
	}
}

void Scene_Battle::CreateEnemyAction(Game_Enemy* enemy, const lcf::rpg::EnemyAction* action) {
	switch (action->kind) {
		case lcf::rpg::EnemyAction::Kind_basic:
			CreateEnemyActionBasic(enemy, action);
			break;
		case lcf::rpg::EnemyAction::Kind_skill:
			CreateEnemyActionSkill(enemy, action);
			break;
		case lcf::rpg::EnemyAction::Kind_transformation:
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

void Scene_Battle::CreateEnemyActionBasic(Game_Enemy* enemy, const lcf::rpg::EnemyAction* action) {
	if (action->kind != lcf::rpg::EnemyAction::Kind_basic) {
		return;
	}

	switch (action->basic) {
		case lcf::rpg::EnemyAction::Basic_attack:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(enemy, Main_Data::game_party->GetRandomActiveBattler()));
			break;
		case lcf::rpg::EnemyAction::Basic_dual_attack:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(enemy, Main_Data::game_party->GetRandomActiveBattler()));
			enemy->GetBattleAlgorithm()->SetRepeat(2);
			break;
		case lcf::rpg::EnemyAction::Basic_defense:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Defend>(enemy));
			break;
		case lcf::rpg::EnemyAction::Basic_observe:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Observe>(enemy));
			break;
		case lcf::rpg::EnemyAction::Basic_charge:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Charge>(enemy));
			break;
		case lcf::rpg::EnemyAction::Basic_autodestruction:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::SelfDestruct>(enemy, Main_Data::game_party.get()));
			break;
		case lcf::rpg::EnemyAction::Basic_escape:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Escape>(enemy));
			break;
		case lcf::rpg::EnemyAction::Basic_nothing:
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

void Scene_Battle::CreateEnemyActionSkill(Game_Enemy* enemy, const lcf::rpg::EnemyAction* action) {
	if (action->kind != lcf::rpg::EnemyAction::Kind_skill) {
		return;
	}

	lcf::rpg::Skill* skill = lcf::ReaderUtil::GetElement(lcf::Data::skills, action->skill_id);
	if (!skill) {
		Output::Warning("CreateEnemyAction: Enemy can't use invalid skill {}", action->skill_id);
		return;
	}

	switch (skill->scope) {
		case lcf::rpg::Skill::Scope_enemy:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(enemy, Main_Data::game_party->GetRandomActiveBattler(), *skill));
			break;
		case lcf::rpg::Skill::Scope_ally:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(enemy, Main_Data::game_enemyparty->GetRandomActiveBattler(), *skill));
			break;
		case lcf::rpg::Skill::Scope_enemies:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(enemy, Main_Data::game_party.get(), *skill));
			break;
		case lcf::rpg::Skill::Scope_self:
			enemy->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(enemy, enemy, *skill));
			break;
		case lcf::rpg::Skill::Scope_party:
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

	if (for_battler->GetBattleAlgorithm() == nullptr) {
		Output::Warning("ActionSelectedCallback: Invalid action for battler {} ({})",
				for_battler->GetId(), for_battler->GetName());
		Output::Warning("Please report a bug!");
	}

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

void Scene_Battle::SelectionFlash(Game_Battler* battler) {
	if (battler) {
		battler->Flash(31, 31, 31, 24, 16);
	}
}

void Scene_Battle::EndBattle(BattleResult result) {
	assert(Scene::instance.get() == this && "EndBattle called multiple times!");

	Main_Data::game_party->IncBattleCount();
	switch (result) {
		case BattleResult::Victory: Main_Data::game_party->IncWinCount(); break;
		case BattleResult::Escape: Main_Data::game_party->IncRunCount(); break;
		case BattleResult::Defeat: Main_Data::game_party->IncDefeatCount(); break;
		case BattleResult::Abort: break;
	}

	Scene::Pop();

	if (on_battle_end) {
		on_battle_end(result);
		on_battle_end = {};
	}
}

