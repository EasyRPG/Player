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
#include "game_interpreter_battle.h"
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
#include "scene_settings.h"
#include "scene_debug.h"
#include "game_interpreter.h"
#include "rand.h"
#include "autobattle.h"
#include "enemyai.h"
#include "feature.h"

Scene_Battle::Scene_Battle(const BattleArgs& args)
	: troop_id(args.troop_id),
	allow_escape(args.allow_escape),
	first_strike(args.first_strike),
	on_battle_end(args.on_battle_end)
{
	SetUseSharedDrawables(true);

	Scene::type = Scene::Battle;

	// Face graphic is cleared when battle scene is created.
	// Even if the battle gets interrupted by another scene and never starts.
	Main_Data::game_system->ClearMessageFace();
	Main_Data::game_system->SetBeforeBattleMusic(Main_Data::game_system->GetCurrentBGM());
	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_BeginBattle));
	Main_Data::game_system->BgmPlay(Main_Data::game_system->GetSystemBGM(Main_Data::game_system->BGM_Battle));

	Game_Battle::SetTerrainId(args.terrain_id);
	Game_Battle::ChangeBackground(args.background);
	Game_Battle::SetBattleCondition(args.condition);
	Game_Battle::SetBattleFormation(args.formation);
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

	autobattle_algos.push_back(AutoBattle::CreateAlgorithm(AutoBattle::RpgRtCompat::name));
	autobattle_algos.push_back(AutoBattle::CreateAlgorithm(AutoBattle::RpgRtImproved::name));
	autobattle_algos.push_back(AutoBattle::CreateAlgorithm(AutoBattle::AttackOnly::name));
	enemyai_algos.push_back(EnemyAi::CreateAlgorithm(EnemyAi::RpgRtCompat::name));
	enemyai_algos.push_back(EnemyAi::CreateAlgorithm(EnemyAi::RpgRtImproved::name));

	if (lcf::Data::system.easyrpg_default_actorai == -1 || (Player::debug_flag && !Player::player_config.autobattle_algo.Get().empty())) {
		if (Player::player_config.autobattle_algo.Get().empty()) {
			Player::player_config.autobattle_algo.Set(ToString(autobattle_algos[0]->GetName()));
		}
		for (auto& algo : autobattle_algos) {
			if (algo->GetName() == Player::player_config.autobattle_algo.Get()) {
				default_autobattle_algo = algo->GetId();
				break;
			}
		}
	} else {
		default_autobattle_algo = lcf::Data::system.easyrpg_default_actorai;
	}
	if (lcf::Data::system.easyrpg_default_enemyai == -1 || (Player::debug_flag && !Player::player_config.enemyai_algo.Get().empty())) {
		if (Player::player_config.enemyai_algo.Get().empty()) {
			Player::player_config.enemyai_algo.Set(ToString(enemyai_algos[0]->GetName()));
		}
		for (auto& algo : enemyai_algos) {
			if (algo->GetName() == Player::player_config.enemyai_algo.Get()) {
				default_enemyai_algo = algo->GetId();
				break;
			}
		}
	} else {
		default_enemyai_algo = lcf::Data::system.easyrpg_default_enemyai;
	}

	Output::Debug("Starting battle {} ({}): algos=({}/{})", troop_id, troop->name, autobattle_algos[default_autobattle_algo]->GetName(), enemyai_algos[default_enemyai_algo]->GetName());

	Game_Battle::Init(troop_id);

	CreateUi();

	InitEscapeChance();

	SetState(State_Start);
}

void Scene_Battle::InitEscapeChance() {
	int avg_enemy_agi = Main_Data::game_enemyparty->GetAverageAgility();
	int avg_actor_agi = Main_Data::game_party->GetAverageAgility();

	int base_chance = Utils::RoundTo<int>(100.0 * static_cast<double>(avg_enemy_agi) / static_cast<double>(avg_actor_agi));
	this->escape_chance = Utils::Clamp(150 - base_chance, 0, 100);
}

bool Scene_Battle::TryEscape() {
	if (first_strike || Game_Battle::GetInterpreterBattle().IsForceFleeEnabled() || Rand::PercentChance(escape_chance)) {
		return true;
	}
	escape_chance += 10;
	return false;
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
	Transition::instance().InitShow(Main_Data::game_system->GetTransition(Main_Data::game_system->Transition_BeginBattleShow), this);
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

	transition.InitErase(Main_Data::game_system->GetTransition(Main_Data::game_system->Transition_EndBattleErase), this);
}

void Scene_Battle::DrawBackground(Bitmap& dst) {
	dst.Clear();
}

void Scene_Battle::CreateUi() {
	std::vector<std::string> commands;

	for (auto option: lcf::Data::system.easyrpg_battle_options) {
		battle_options.push_back((BattleOptionType)option);
	}

	// Add all menu items
	for (auto option: battle_options) {
		switch(option) {
		case Battle:
			commands.push_back(ToString(lcf::Data::terms.battle_fight));
			break;
		case AutoBattle:
			commands.push_back(ToString(lcf::Data::terms.battle_auto));
			break;
		case Escape:
			commands.push_back(ToString(lcf::Data::terms.battle_escape));
			break;
		}
	}

	options_window.reset(new Window_Command(commands, option_command_mov));
	options_window->SetHeight(80);
	options_window->SetX(Player::menu_offset_x);
	options_window->SetY(Player::menu_offset_y + MENU_HEIGHT - 80);

	help_window.reset(new Window_Help(Player::menu_offset_x, Player::menu_offset_y, MENU_WIDTH, 32));
	help_window->SetVisible(false);

	item_window.reset(new Window_Item(Player::menu_offset_x, (Player::menu_offset_y + MENU_HEIGHT - 80), MENU_WIDTH, 80));
	item_window->SetHelpWindow(help_window.get());
	item_window->Refresh();
	item_window->SetIndex(0);

	skill_window.reset(new Window_BattleSkill(Player::menu_offset_x, (Player::menu_offset_y + MENU_HEIGHT - 80), MENU_WIDTH, 80));
	skill_window->SetHelpWindow(help_window.get());

	message_window.reset(new Window_Message(Player::menu_offset_x, (Player::menu_offset_y + MENU_HEIGHT - 80), MENU_WIDTH, 80));
	Game_Message::SetWindow(message_window.get());
}

void Scene_Battle::UpdateScreen() {
	Main_Data::game_screen->Update();
	Main_Data::game_pictures->Update(true);
}

void Scene_Battle::UpdateBattlers() {
	std::vector<Game_Battler*> battlers;
	Main_Data::game_enemyparty->GetBattlers(battlers);
	Main_Data::game_party->GetBattlers(battlers);
	for (auto* b : battlers) {
		b->UpdateBattle();
	}
	Game_Battle::UpdateAnimation();
}

void Scene_Battle::UpdateUi() {
	options_window->Update();
	status_window->Update();
	command_window->Update();
	help_window->Update();
	item_window->Update();
	skill_window->Update();
	target_window->Update();

	Game_Message::Update();
}

bool Scene_Battle::UpdateEvents() {
	auto& interp = Game_Battle::GetInterpreterBattle();
	interp.Update();
	status_window->Refresh();

	if (interp.IsForceFleeEnabled()) {
		if (state != State_Escape) {
			SetState(State_Escape);
		}
	}

	auto call = TakeRequestedScene();
	if (call && (call->type == Scene::Gameover || call->type == Scene::Settings)) {
		Scene::Push(std::move(call));
	}

	if (interp.IsAsyncPending()) {
		auto aop = interp.GetAsyncOp();

		if (aop.GetType() == AsyncOp::eTerminateBattle) {
			EndBattle(static_cast<BattleResult>(aop.GetBattleResult()));
			return false;
		}

		if (CheckSceneExit(aop)) {
			return false;
		}
	}

	return true;
}

bool Scene_Battle::UpdateTimers() {
	const int timer1 = Main_Data::game_party->GetTimerSeconds(Game_Party::Timer1);
	const int timer2 = Main_Data::game_party->GetTimerSeconds(Game_Party::Timer2);

	// Screen Effects
	Main_Data::game_party->UpdateTimers();

	// Query Timer before and after update.
	// If it reached zero during update was a running battle timer.
	if ((Main_Data::game_party->GetTimerSeconds(Game_Party::Timer1) == 0 && timer1 > 0) ||
		(Main_Data::game_party->GetTimerSeconds(Game_Party::Timer2) == 0 && timer2 > 0)) {
		EndBattle(BattleResult::Abort);
		return false;
	}
	return true;
}

void Scene_Battle::UpdateGraphics() {
	Game_Battle::UpdateGraphics();
}

bool Scene_Battle::IsWindowMoving() {
	return options_window->IsMovementActive() || status_window->IsMovementActive() || command_window->IsMovementActive();
}

Game_Enemy* Scene_Battle::EnemySelected() {
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
				return nullptr;
			}
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(active_actor, target, *skill, item));
		} else {
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Item>(active_actor, target, *item));
		}
	} else {
		assert("Invalid previous state for enemy selection" && false);
	}

	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
	ActionSelectedCallback(active_actor);
	return target;
}

Game_Actor* Scene_Battle::AllySelected() {
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
				return nullptr;
			}
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Skill>(active_actor, &target, *skill, item));
		} else {
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Item>(active_actor, &target, *item));
		}
	} else {
		assert("Invalid previous state for ally selection" && false);
	}

	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
	ActionSelectedCallback(active_actor);
	return &target;
}

void Scene_Battle::AttackSelected() {
	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));

	if (active_actor->HasAttackAll()) {
		active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(active_actor, Main_Data::game_enemyparty.get()));
		ActionSelectedCallback(active_actor);
	} else {
		SetState(State_SelectEnemyTarget);
	}
}

void Scene_Battle::DefendSelected() {
	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));

	active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Defend>(active_actor));

	ActionSelectedCallback(active_actor);
}

void Scene_Battle::ItemSelected() {
	const lcf::rpg::Item* item = item_window->GetItem();

	if (!item || !item_window->CheckEnable(item->ID)) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
		return;
	}

	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));

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
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
		return;
	}

	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));

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
	if (Feature::HasRpg2kBattleSystem()) {
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
		if (battler->GetBattleAlgorithm()->GetType() != Game_BattleAlgorithm::Type::None) {
			battler->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::None>(battler));
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

	// If we can no longer perform the action (no more items, ran out of SP, etc..)
	if (!battler->GetBattleAlgorithm()->ActionIsPossible()) {
		battler->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::None>(battler));
	}
}

void Scene_Battle::RemoveCurrentAction() {
	battle_actions.front()->SetBattleAlgorithm(nullptr);
	battle_actions.pop_front();
}

void Scene_Battle::ActionSelectedCallback(Game_Battler* for_battler) {
	assert(for_battler->GetBattleAlgorithm() != nullptr);

	if (for_battler->GetBattleAlgorithm() == nullptr) {
		Output::Warning("ActionSelectedCallback: Invalid action for battler {} ({})",
				for_battler->GetId(), for_battler->GetName());
		Output::Warning("Please report a bug!");
	}

	battle_actions.push_back(for_battler);
}

bool Scene_Battle::CallDebug() {
	if (Player::debug_flag) {
		Scene::Push(std::make_shared<Scene_Debug>());
		return true;
	}
	return false;
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

	// For RPG_RT compatibility, wait 30 frames if a battle test ends
	if (Game_Battle::battle_test.enabled) {
		Scene::instance->SetDelayFrames(30);
	}

	if (on_battle_end) {
		on_battle_end(result);
		on_battle_end = {};
	}
}

