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

#include <algorithm>
#include <sstream>
#include "input.h"
#include "player.h"
#include "sprite.h"
#include "game_battler.h"
#include "game_system.h"
#include "game_temp.h"
#include "game_party.h"
#include "game_enemy.h"
#include "game_enemyparty.h"
#include "game_message.h"
#include "game_battle.h"
#include "game_battlealgorithm.h"
#include "battle_animation.h"
#include "reader_util.h"
#include "scene_battle_rpg2k.h"
#include "scene_battle.h"
#include "scene_gameover.h"
#include "output.h"

Scene_Battle_Rpg2k::Scene_Battle_Rpg2k() : Scene_Battle()
{
}

Scene_Battle_Rpg2k::~Scene_Battle_Rpg2k() {
}

void Scene_Battle_Rpg2k::Update() {
	battle_message_window->Update();

	if (message_window->GetVisible() && !message_box_got_visible) {
		battle_message_window->Clear();
		message_box_got_visible = true;
	} else if (!message_window->GetVisible() && message_box_got_visible) {
		message_box_got_visible = false;
	}

	Scene_Battle::Update();
}

void Scene_Battle_Rpg2k::CreateUi() {
	Scene_Battle::CreateUi();

	CreateBattleTargetWindow();
	CreateBattleCommandWindow();

	battle_message_window.reset(new Window_BattleMessage(0, (SCREEN_TARGET_HEIGHT - 80), SCREEN_TARGET_WIDTH, 80));

	if (!Game_Battle::IsEscapeAllowed()) {
		options_window->DisableItem(2);
	}
}

void Scene_Battle_Rpg2k::CreateBattleTargetWindow() {
	std::vector<std::string> commands;
	std::vector<Game_Battler*> enemies;
	Main_Data::game_enemyparty->GetActiveBattlers(enemies);

	for (std::vector<Game_Battler*>::iterator it = enemies.begin();
		it != enemies.end(); ++it) {
		commands.push_back((*it)->GetName());
	}

	target_window.reset(new Window_Command(commands, 136, 4));
	target_window->SetHeight(80);
	target_window->SetY(SCREEN_TARGET_HEIGHT-80);
	// Above other windows
	target_window->SetZ(Priority_Window + 10);
}

void Scene_Battle_Rpg2k::CreateBattleCommandWindow() {
	std::vector<std::string> commands;
	commands.push_back(Data::terms.command_attack);
	commands.push_back(Data::terms.command_skill);
	commands.push_back(Data::terms.command_defend);
	commands.push_back(Data::terms.command_item);

	command_window.reset(new Window_Command(commands, 76));
	command_window->SetHeight(80);
	command_window->SetX(SCREEN_TARGET_WIDTH - option_command_mov);
	command_window->SetY(SCREEN_TARGET_HEIGHT-80);
}

void Scene_Battle_Rpg2k::RefreshCommandWindow() {
	command_window->SetItemText(1, active_actor->GetSkillName());
}

void Scene_Battle_Rpg2k::SetState(Scene_Battle::State new_state) {
	previous_state = state;
	state = new_state;

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
		select_target_flash_count = 0;
		break;
	case State_SelectAllyTarget:
		status_window->SetActive(true);
		break;
	case State_Battle:
		// no-op
		break;
	case State_SelectItem:
		item_window->SetActive(true);
		item_window->SetActor(active_actor);
		item_window->Refresh();
		break;
	case State_SelectSkill:
		skill_window->SetActive(true);
		skill_window->SetActor(active_actor->GetId());
		if (previous_state == State_SelectCommand)
			skill_window->RestoreActorIndex(actor_index - 1);
		break;
	case State_Victory:
	case State_Defeat:
		battle_message_window->Clear();
		break;
	case State_Escape:
		battle_message_window->SetActive(true);
		break;
	}

	if (state != State_SelectEnemyTarget && state != State_SelectAllyTarget) {
		options_window->SetVisible(false);
		status_window->SetVisible(false);
		command_window->SetVisible(false);
		target_window->SetVisible(false);
		battle_message_window->SetVisible(false);
		item_window->SetVisible(false);
		skill_window->SetVisible(false);
		help_window->SetVisible(false);

	}

	if (previous_state == State_SelectSkill) {
		skill_window->SaveActorIndex(actor_index - 1);
	}

	switch (state) {
	case State_Start:
		battle_message_window->SetVisible(true);
		break;
	case State_SelectOption:
		options_window->SetVisible(true);
		options_window->SetX(0);
		status_window->SetVisible(true);
		status_window->SetX(option_command_mov);
		status_window->SetIndex(-1);
		command_window->SetX(SCREEN_TARGET_WIDTH);
		command_window->SetIndex(-1);
		status_window->Refresh();
		move_screen = true;
		break;
	case State_SelectActor:
		SelectNextActor();
		break;
	case State_AutoBattle:
		SetState(State_SelectActor);
		break;
	case State_SelectCommand:
		options_window->SetX(-option_command_mov);
		status_window->SetVisible(true);
		status_window->SetX(0);
		command_window->SetVisible(true);
		command_window->SetX(SCREEN_TARGET_WIDTH - option_command_mov);
		if (previous_state == State_SelectActor) {
			command_window->SetIndex(0);
		}
		break;
	case State_SelectEnemyTarget:
		target_window->SetActive(true);
		target_window->SetVisible(true);
		target_window->SetIndex(0);
		break;
	case State_SelectAllyTarget:
		status_window->SetVisible(true);
		status_window->SetX(0);
		status_window->SetIndex(0);
		break;
	case State_Battle:
		battle_message_window->SetVisible(true);
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
		battle_message_window->SetVisible(true);
		break;
	case State_Escape:
		battle_message_window->SetVisible(true);
		break;
	}

	// If SelectOption <-> SelectCommand => Display Movement:
	if (state == State_SelectOption && previous_state == State_SelectCommand) {
		options_window->InitMovement(options_window->GetX() - option_command_mov, options_window->GetY(),
			options_window->GetX(), options_window->GetY(), option_command_time);

		status_window->InitMovement(status_window->GetX() - option_command_mov, status_window->GetY(),
			status_window->GetX(), status_window->GetY(), option_command_time);

		command_window->SetVisible(true);
		command_window->InitMovement(command_window->GetX() - option_command_mov, command_window->GetY(),
			command_window->GetX(), command_window->GetY(), option_command_time);
	}
	else if (state == State_SelectCommand && move_screen) {
		move_screen = false;
		options_window->SetVisible(true);
		options_window->InitMovement(options_window->GetX() + option_command_mov, options_window->GetY(),
			options_window->GetX(), options_window->GetY(), option_command_time);

		status_window->InitMovement(status_window->GetX() + option_command_mov, status_window->GetY(),
			status_window->GetX(), status_window->GetY(), option_command_time);

		command_window->InitMovement(command_window->GetX() + option_command_mov, command_window->GetY(),
			command_window->GetX(), command_window->GetY(), option_command_time);
	}
}

void Scene_Battle_Rpg2k::NextTurn() {
	Scene_Battle::NextTurn(nullptr);

	auto_battle = false;
}

void Scene_Battle_Rpg2k::ProcessActions() {
	switch (state) {
	case State_Start:
		if (DisplayMonstersInMessageWindow()) {
			Game_Battle::RefreshEvents();
			SetState(State_Battle);
		}
		break;
	case State_SelectOption:
		// No Auto battle/Escape when all actors are sleeping or similar
		if (!Main_Data::game_party->IsAnyControllable()) {
			SelectNextActor();
		}
		break;
	case State_SelectActor:
	case State_AutoBattle:
		CheckResultConditions();

		if (help_window->GetVisible() && message_timer > 0) {
			message_timer--;
			if (message_timer <= 0)
				help_window->SetVisible(false);
		}

		break;
	case State_Battle:
		// If no battle action is running, we need to check for battle events which could have
		// triggered win/loss.
		if (!battle_action_pending && CheckResultConditions()) {
			return;
		}
		if (!battle_actions.empty()) {
			auto* battler = battle_actions.front();
			if (!battle_action_pending) {
				// If we will start a new battle action, first check for state changes
				// such as death, paralyze, confuse, etc..
				PrepareBattleAction(battler);

				// If we can no longer perform the action (no more items, ran out of SP, etc..)
				if (!battler->GetBattleAlgorithm()->ActionIsPossible()) {
					battler->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::NoMove>(battler));
					battler->SetCharged(false);
				}
			}
			auto* alg = battler->GetBattleAlgorithm().get();

			if (ProcessBattleAction(alg)) {
				battle_action_pending = false;
				RemoveCurrentAction();
				battle_message_window->Clear();
				Game_Battle::RefreshEvents();

				if (CheckResultConditions()) {
					return;
				}
			}
		} else {
			// Everybody acted
			actor_index = 0;

			// Go right into next turn if no actors controllable.
			if (!Main_Data::game_party->IsAnyControllable()) {
				SelectNextActor();
			} else {
				SetState(State_SelectOption);
			}
		}
		break;
	case State_SelectEnemyTarget: {
		std::vector<Game_Battler*> enemies;
		Main_Data::game_enemyparty->GetActiveBattlers(enemies);

		Game_Enemy* target = static_cast<Game_Enemy*>(enemies[target_window->GetIndex()]);
		Sprite_Battler* sprite = Game_Battle::GetSpriteset().FindBattler(target);
		if (sprite) {
			++select_target_flash_count;

			if (select_target_flash_count == 60) {
				sprite->Flash(Color(255, 255, 255, 100), 15);
				select_target_flash_count = 0;
			}
		}
		break;
	}
	case State_Victory:
		Scene::Pop();
		break;
	case State_Defeat:
		if (Player::battle_test_flag || Game_Temp::battle_defeat_mode != 0) {
			Scene::Pop();
		}
		else {
			Scene::Push(std::make_shared<Scene_Gameover>());
		}
		break;
	case State_Escape:
		Escape();
		break;
	default:
		break;
	}
}

bool Scene_Battle_Rpg2k::ProcessBattleAction(Game_BattleAlgorithm::AlgorithmBase* action) {
	if (!battle_action_pending) {
		// First time we are called, do initialization.
		battle_action_wait = 0;
		battle_action_state = BattleActionState_ConditionHeal;
		battle_action_start_index = 0;
		battle_action_results_index = 0;

		battle_action_pending = true;
	}

	const bool wait = battle_action_wait > 0;
	if (battle_action_wait > 0 && !Input::IsPressed(Input::CANCEL)) {
		--battle_action_wait;
		if (Input::IsPressed(Input::DECISION) && battle_action_wait > 0) {
			--battle_action_wait;
		}
	}

	if (Game_Battle::IsBattleAnimationWaiting()) {
		return false;
	}
	if (action->HasAnimationPlayed() && action->GetSecondAnimation() != nullptr && !action->HasSecondAnimationPlayed()) {
		action->PlaySecondAnimation();
		return false;
	}

	if (wait) {
		return false;
	}

	switch (battle_action_state) {
		case BattleActionState_ConditionHeal:
			return ProcessActionConditionHeal(action);
		case BattleActionState_Usage1:
			return ProcessActionUsage1(action);
		case BattleActionState_Usage2:
			return ProcessActionUsage2(action);
		case BattleActionState_Animation:
			return ProcessActionAnimation(action);
		case BattleActionState_Execute:
			return ProcessActionExecute(action);
		case BattleActionState_Apply:
			return ProcessActionApply(action);
		case BattleActionState_Results:
			return ProcessActionResults(action);
		case BattleActionState_Death:
			return ProcessActionDeath(action);
		case BattleActionState_Finished:
			return ProcessActionFinished(action);
	}

	assert(false && "Invalid BattleActionState!");

	return true;
}

bool Scene_Battle_Rpg2k::ProcessActionConditionHeal(Game_BattleAlgorithm::AlgorithmBase* action) {
	battle_action_state = BattleActionState_ConditionHeal;
	if (action->IsFirstAttack()) {
		battle_message_window->Clear();

		auto* src = action->GetSource();

		if (src->Exists()) {
			auto* source_sprite = Game_Battle::GetSpriteset().FindBattler(action->GetSource());
			if (source_sprite) {
				source_sprite->Flash(Color(255, 255, 255, 100), 15);
			}
		}

		std::vector<int16_t> states_to_heal = src->NextBattleTurn();
		src->ApplyConditions();

		const RPG::State* pri_state = nullptr;
		bool pri_was_healed = false;
		for (size_t id = 1; id <= Data::states.size(); ++id) {
			auto was_healed = std::find(states_to_heal.begin(), states_to_heal.end(), id) != states_to_heal.end();
			if (!was_healed && !src->HasState(id)) {
				continue;
			}

			auto* state = ReaderUtil::GetElement(Data::states, id);
			if (!pri_state || state->priority >= pri_state->priority) {
				pri_state = state;
				pri_was_healed = was_healed;
			}
		}

		if (pri_state != nullptr) {
			const auto& msg = pri_was_healed
				? pri_state->message_recovery
				: pri_state->message_affected;

			// RPG_RT behavior:
			// If state was healed, always prints.
			// If state is inflicted, only prints if msg not empty.
			if (pri_was_healed || !msg.empty()) {
				battle_message_window->PushWithSubject(msg, action->GetSource()->GetName());
				battle_action_wait = GetDelayForWindow();

				battle_action_state = BattleActionState_Usage1;
				return ProcessBattleAction(action);
			}
		}
	}

	return ProcessActionUsage1(action);
}


bool Scene_Battle_Rpg2k::ProcessActionUsage1(Game_BattleAlgorithm::AlgorithmBase* action) {
	battle_action_state = BattleActionState_Usage1;

	action->TargetFirst();
	if (!action->IsTargetValid()) {
		if (!action->GetTarget()) {
			// No target but not a target-only action.
			// Maybe a bug report will help later
			Output::Warning("Battle: BattleAction without valid target.");
			return true;
		}

		action->SetTarget(action->GetTarget()->GetParty().GetNextActiveBattler(action->GetTarget()));

		if (!action->IsTargetValid()) {
			// Nothing left to target, abort
			return true;
		}
	}

	battle_message_window->Clear();

	if (action->HasStartMessage()) {
		battle_message_window->Push(action->GetStartMessage());
		battle_message_window->ScrollToEnd();

		if (action->HasSecondStartMessage()) {
			battle_action_wait = GetDelayForWindow();

			battle_action_state = BattleActionState_Usage2;
			return ProcessBattleAction(action);
		}
	}

	return ProcessActionUsage2(action);
}

bool Scene_Battle_Rpg2k::ProcessActionUsage2(Game_BattleAlgorithm::AlgorithmBase* action) {
	battle_action_state = BattleActionState_Usage2;

	if (action->HasSecondStartMessage()) {
		battle_message_window->Push(action->GetSecondStartMessage());
		battle_message_window->ScrollToEnd();
	}

	return ProcessActionAnimation(action);
}

bool Scene_Battle_Rpg2k::ProcessActionAnimation(Game_BattleAlgorithm::AlgorithmBase* action) {
	battle_action_state = BattleActionState_Animation;

	battle_action_start_index = battle_message_window->GetLineCount();

	if (action->GetStartSe()) {
		Game_System::SePlay(*action->GetStartSe());
	}

	if (action->GetTarget() && action->GetAnimation()) {
		if (action->GetTarget()->GetType() == Game_Battler::Type_Enemy) {
			action->PlayAnimation();
		} else {
			action->PlaySoundAnimation(false, 20);
		}
	}

	if (action->GetSource()->GetType() == Game_Battler::Type_Enemy) {
		if (action->GetType() == Game_BattleAlgorithm::Type::Escape) {
			auto* source_sprite = Game_Battle::GetSpriteset().FindBattler(action->GetSource());
			source_sprite->SetAnimationState(
					Sprite_Battler::AnimationState_Dead,
					Sprite_Battler::LoopState_DefaultAnimationAfterFinish);
		}

		if (action->GetType() == Game_BattleAlgorithm::Type::SelfDestruct) {
			auto* source_sprite = Game_Battle::GetSpriteset().FindBattler(action->GetSource());
			source_sprite->SetAnimationState(
					Sprite_Battler::AnimationState_SelfDestruct,
					Sprite_Battler::LoopState_DefaultAnimationAfterFinish);
		}
	}


	// Wait for last start message and animations.
	if (action->GetSource()->Exists()) {
		battle_action_state = BattleActionState_Execute;
		if (action->GetType() != Game_BattleAlgorithm::Type::NoMove) {
			battle_action_wait = GetDelayForWindow();
		} else {
			battle_action_wait = 1;
		}
		return ProcessBattleAction(action);
	}

	return ProcessActionExecute(action);
}

bool Scene_Battle_Rpg2k::ProcessActionExecute(Game_BattleAlgorithm::AlgorithmBase* action) {
	battle_action_state = BattleActionState_Execute;

	action->Execute();

	// Wait for critical hit message.
	if (action->IsSuccess() && action->IsCriticalHit()) {
		battle_message_window->Push(action->GetCriticalHitMessage());
		battle_message_window->ScrollToEnd();
		battle_action_wait = GetDelayForWindow();

		battle_action_state = BattleActionState_Apply;
		return ProcessBattleAction(action);
	}

	return ProcessActionApply(action);
}

bool Scene_Battle_Rpg2k::ProcessActionApply(Game_BattleAlgorithm::AlgorithmBase* action) {
	battle_action_state = BattleActionState_Apply;

	battle_result_messages.clear();
	action->GetResultMessages(battle_result_messages);
	battle_result_messages_it = battle_result_messages.begin();

	action->Apply();

	battle_action_results_index = battle_message_window->GetLineCount();

	auto* target = action->GetTarget();
	if (target) {
		auto* target_sprite = Game_Battle::GetSpriteset().FindBattler(target);
		if (action->IsSuccess() && target_sprite && !action->IsPositive() && !action->IsAbsorb() && action->GetAffectedHp() > -1) {
			target_sprite->SetAnimationState(Sprite_Battler::AnimationState_Damage);
		}
		if (action->IsSuccess() && target->GetType() == Game_Battler::Type_Ally && !action->IsPositive() && !action->IsAbsorb() && action->GetAffectedHp() > 0) {
			// FIXME: How does this interact with map shake?
			Main_Data::game_screen->ShakeOnce(5, 5, 6);
		}
	}

	if (action->GetResultSe()) {
		Game_System::SePlay(*action->GetResultSe());
	}

	return ProcessActionResults(action);
}

bool Scene_Battle_Rpg2k::ProcessActionResults(Game_BattleAlgorithm::AlgorithmBase* action) {
	battle_action_state = BattleActionState_Results;

	auto* source = action->GetSource();
	auto* target = action->GetTarget();

	if (battle_result_messages_it == battle_result_messages.end()) {
		if (action->IsLethal()) {
			return ProcessActionDeath(action);
		}
		return ProcessActionFinished(action);
	}

	if (battle_action_results_index != battle_message_window->GetLineCount()) {
		battle_message_window->PopUntil(battle_action_results_index);
		battle_message_window->ScrollToEnd();
		// One frame of emptyness between results
		return false;
	}

	battle_message_window->Push(*battle_result_messages_it);
	battle_message_window->ScrollToEnd();
	battle_action_wait = GetDelayForWindow();
	++battle_result_messages_it;

	return false;
}

bool Scene_Battle_Rpg2k::ProcessActionDeath(Game_BattleAlgorithm::AlgorithmBase* action) {
	battle_action_state = BattleActionState_Death;
	if (action->GetTarget()) {
		auto* target_sprite = Game_Battle::GetSpriteset().FindBattler(action->GetTarget());
		battle_message_window->Push(action->GetDeathMessage());
		battle_message_window->ScrollToEnd();
		battle_action_wait = GetDelayForWindow();

		battle_action_state = BattleActionState_Finished;

		if (action->GetDeathSe()) {
			Game_System::SePlay(*action->GetDeathSe());
		}
		if (target_sprite) {
			target_sprite->DetectStateChange();
		}

		return false;
	}

	return ProcessActionFinished(action);
}

bool Scene_Battle_Rpg2k::ProcessActionFinished(Game_BattleAlgorithm::AlgorithmBase* action) {
	battle_action_state = BattleActionState_Finished;

	if (action->GetTarget()) {
		auto* target_sprite = Game_Battle::GetSpriteset().FindBattler(action->GetTarget());
		if (action->GetTarget() && target_sprite && !target_sprite->IsIdling()) {
			return false;
		}
	}

	if (action->TargetNext()) {
		// Clear the console for the next target
		battle_message_window->PopUntil(battle_action_start_index);
		battle_message_window->ScrollToEnd();

		// 1 frame delay before starting next target.
		battle_action_state = BattleActionState_Execute;
		return false;
	}

	battle_message_window->Clear();
	return true;
}

void Scene_Battle_Rpg2k::ProcessInput() {
	if (IsWindowMoving()) {
		return;
	}
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
			SetState(State_SelectCommand);
			SelectNextActor();
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
			AllySelected();
			break;
		case State_Battle:
			// no-op
			break;
		case State_SelectItem:
			ItemSelected();
			break;
		case State_SelectSkill:
			SkillSelected();
			break;
		case State_Victory:
		case State_Defeat:
		case State_Escape:
			// no-op
			break;
		}
	}

	if (Input::IsTriggered(Input::CANCEL)) {
		switch (state) {
		case State_Start:
		case State_SelectOption:
			// no-op
			break;
		case State_SelectActor:
		case State_AutoBattle:
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
			SetState(State_SelectOption);
			break;
		case State_SelectCommand:
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
			--actor_index;
			SelectPreviousActor();
			break;
		case State_SelectItem:
		case State_SelectSkill:
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
			SetState(State_SelectCommand);
			break;
		case State_SelectEnemyTarget:
		case State_SelectAllyTarget:
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
			SetState(previous_state);
			break;
		case State_Battle:
			// no-op
			break;
		case State_Victory:
		case State_Defeat:
		case State_Escape:
			// no-op
			break;
		}
	}
	if (Input::IsTriggered(Input::DEBUG_MENU)) {
		this->CallDebug();
	}
}

void Scene_Battle_Rpg2k::OptionSelected() {
	switch (options_window->GetIndex()) {
		case 0: // Battle
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
			CreateBattleTargetWindow();
			auto_battle = false;
			SetState(State_SelectActor);
			break;
		case 1: // Auto Battle
			auto_battle = true;
			SetState(State_AutoBattle);
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
			break;
		case 2: // Escape
			if (!Game_Battle::IsEscapeAllowed()) {
				Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
			}
			else {
				Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
				SetState(State_Escape);
			}
			break;
	}
}

void Scene_Battle_Rpg2k::CommandSelected() {

	switch (command_window->GetIndex()) {
		case 0: // Attack
			AttackSelected();
			break;
		case 1: // Skill
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
			SetState(State_SelectSkill);
			break;
		case 2: // Defense
			DefendSelected();
			break;
		case 3: // Item
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
			SetState(State_SelectItem);
			break;
		default:
			// no-op
			break;
	}
}

void Scene_Battle_Rpg2k::Escape() {
	if (begin_escape) {
		battle_message_window->Clear();

		Game_BattleAlgorithm::Escape escape_alg = Game_BattleAlgorithm::Escape(&(*Main_Data::game_party)[0]);

		escape_success = escape_alg.Execute();
		escape_alg.Apply();

		battle_result_messages.clear();
		escape_alg.GetResultMessages(battle_result_messages);

		battle_message_window->Push(battle_result_messages[0]);
		begin_escape = false;
	}
	else {
		if (Input::IsPressed(Input::DECISION)) {
			++escape_counter;
		}

		++escape_counter;

		if (escape_counter > 60) {
			begin_escape = true;
			escape_counter = 0;

			if (escape_success) {
				Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Escape));

				Game_Temp::battle_result = Game_Temp::BattleEscape;

				Scene::Pop();
			}
			else {
				SetState(State_Battle);
				NextTurn();

				CreateEnemyActions();
				CreateExecutionOrder();
				Game_Battle::RefreshEvents();
			}
		}
	}
}

void Scene_Battle_Rpg2k::SelectNextActor() {
	std::vector<Game_Actor*> allies = Main_Data::game_party->GetActors();

	if ((size_t)actor_index == allies.size()) {
		// All actor actions decided, player turn ends
		SetState(State_Battle);
		NextTurn();

		if (!Game_Temp::battle_first_strike || Game_Battle::GetTurn() > 1) {
			CreateEnemyActions();
		}
		CreateExecutionOrder();
		Game_Battle::RefreshEvents();

		return;
	}

	active_actor = allies[actor_index];
	status_window->SetIndex(actor_index);
	actor_index++;

	Game_Battler* random_target = NULL;

	if (!active_actor->CanAct()) {
		active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::NoMove>(active_actor));
		battle_actions.push_back(active_actor);
		SelectNextActor();
		return;
	}

	switch (active_actor->GetSignificantRestriction()) {
		case RPG::State::Restriction_attack_ally:
			random_target = Main_Data::game_party->GetRandomActiveBattler();
			break;
		case RPG::State::Restriction_attack_enemy:
			random_target = Main_Data::game_enemyparty->GetRandomActiveBattler();
			break;
		default:
			break;
	}

	if (random_target) {
		// RPG_RT doesn't support "Attack All" weapons when battler is confused or provoked.
		active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(active_actor, random_target));
		battle_actions.push_back(active_actor);

		SelectNextActor();
		return;
	}

	if (auto_battle || active_actor->GetAutoBattle()) {
		if (active_actor->HasAttackAll()) {
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(active_actor,
						Main_Data::game_enemyparty.get()));
		} else {
			active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(active_actor,
						Main_Data::game_enemyparty->GetRandomActiveBattler()));
		}
		battle_actions.push_back(active_actor);

		SelectNextActor();
		return;
	}

	SetState(Scene_Battle::State_SelectCommand);
}

void Scene_Battle_Rpg2k::SelectPreviousActor() {
	std::vector<Game_Actor*> allies = Main_Data::game_party->GetActors();

	if (allies[0] == active_actor) {
		SetState(State_SelectOption);
		actor_index = 0;
		return;
	}

	actor_index--;
	active_actor = allies[actor_index];

	if (active_actor->IsDead()) {
		SelectPreviousActor();
		return;
	}

	battle_actions.back()->SetBattleAlgorithm(std::shared_ptr<Game_BattleAlgorithm::AlgorithmBase>());
	battle_actions.pop_back();

	if (!active_actor->IsControllable()) {
		SelectPreviousActor();
		return;
	}

	SetState(State_SelectActor);
}

void Scene_Battle_Rpg2k::CreateExecutionOrder() {
	// Define random Agility. Must be done outside of the sort function because of the "strict weak ordering" property, so the sort is consistent
	for (auto battler : battle_actions) {
		int battle_order = battler->GetAgi() + Utils::GetRandomNumber(0, battler->GetAgi() / 4 + 3);
		if (battler->GetBattleAlgorithm()->GetType() == Game_BattleAlgorithm::Type::Normal && battler->HasPreemptiveAttack()) {
			// This is an arbitrarily large number to separate preemptive vs non-preemptive battlers
			battle_order += 100000;
		}
		battler->SetBattleOrderAgi(battle_order);
	}
	std::sort(battle_actions.begin(), battle_actions.end(),
			[](Game_Battler* l, Game_Battler* r) {
			return l->GetBattleOrderAgi() > r->GetBattleOrderAgi();
			});
}

void Scene_Battle_Rpg2k::CreateEnemyActions() {
	std::vector<Game_Battler*> enemies;
	Main_Data::game_enemyparty->GetActiveBattlers(enemies);

	for (Game_Battler* battler : enemies) {
		if (!battler->CanAct()) {
			battler->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::NoMove>(battler));
			ActionSelectedCallback(battler);
			continue;
		}

		const RPG::EnemyAction* action = static_cast<Game_Enemy*>(battler)->ChooseRandomAction();
		if (action) {
			CreateEnemyAction(static_cast<Game_Enemy*>(battler), action);
		}
	}
}

int Scene_Battle_Rpg2k::GetDelayForWindow() {
	if (Player::IsRPG2kE()) {
		return 40;
	}
	else {
		return 60 / 2;
	}
}

int Scene_Battle_Rpg2k::GetDelayForLine() {
	if (Player::IsRPG2kE()) {
		return 60 / 10;
	}
	else {
		return 60 / 7;
	}
}

bool Scene_Battle_Rpg2k::DisplayMonstersInMessageWindow() {
	if (encounter_message_first_monster) {
		std::vector<Game_Battler *> visible_enemies;
		// First time entered, initialize.
		Main_Data::game_enemyparty->GetActiveBattlers(visible_enemies);

		for (auto& enemy: visible_enemies) {
			// Format and wordwrap all messages, then pull them out and push them back 1 at a time.
			battle_message_window->PushWithSubject(Data::terms.encounter, enemy->GetName());
		}

		battle_result_messages = battle_message_window->GetLines();
		battle_result_messages_it = battle_result_messages.begin();
		battle_message_window->Clear();

		encounter_message_wait = 0;
		encounter_message_first_strike = false;
		encounter_message_first_monster = false;
	}

	if (encounter_message_wait > 0) {
		if (Input::IsPressed(Input::CANCEL)) {
			return false;
		}
		--encounter_message_wait;
		if (Input::IsPressed(Input::DECISION) && encounter_message_wait > 0) {
			--encounter_message_wait;
		}
		return false;
	}

	if (battle_result_messages_it == battle_result_messages.end()) {
		battle_message_window->Clear();
		if (Game_Temp::battle_first_strike && !encounter_message_first_strike) {
			battle_message_window->Push(Data::terms.special_combat);
			encounter_message_wait = GetDelayForWindow();
			encounter_message_first_strike = true;
			assert(encounter_message_wait > 0);
			return DisplayMonstersInMessageWindow();;
		}
		else {
			//reset static vars
			encounter_message_first_strike = false;
			encounter_message_first_monster = true;
			battle_result_messages.clear();
			battle_result_messages_it = battle_result_messages.end();
			return true;
		}
	}

	if (battle_message_window->IsPageFilled()) {
		battle_message_window->Clear();
	}

	battle_message_window->Push(*battle_result_messages_it);
	++battle_result_messages_it;

	if (battle_result_messages_it == battle_result_messages.end() ||
			battle_message_window->IsPageFilled()) {
		encounter_message_wait = GetDelayForWindow();
	}
	else {
		encounter_message_wait = GetDelayForLine();
	}

	assert(encounter_message_wait > 0);
	return DisplayMonstersInMessageWindow();
}

void Scene_Battle_Rpg2k::PushExperienceGainedMessage(int exp) {
	std::stringstream ss;
	if (Player::IsRPG2kE()) {
		ss << exp;
		Game_Message::texts.push_back(
			Utils::ReplacePlaceholders(
				Data::terms.exp_received,
				{'V', 'U'},
				{ss.str(), Data::terms.exp_short}
			) + Player::escape_symbol + "."
		);
	}
	else {
		ss << exp << Data::terms.exp_received << Player::escape_symbol << ".";
		Game_Message::texts.push_back(ss.str());
	}
}

void Scene_Battle_Rpg2k::PushGoldReceivedMessage(int money) {
	std::stringstream ss;

	if (Player::IsRPG2kE()) {
		ss << money;
		Game_Message::texts.push_back(
			Utils::ReplacePlaceholders(
				Data::terms.gold_recieved_a,
				{'V', 'U'},
				{ss.str(), Data::terms.gold}
			) + Player::escape_symbol + "."
		);
	}
	else {
		ss << Data::terms.gold_recieved_a << " " << money << Data::terms.gold << Data::terms.gold_recieved_b << Player::escape_symbol << ".";
		Game_Message::texts.push_back(ss.str());
	}
}

void Scene_Battle_Rpg2k::PushItemRecievedMessages(std::vector<int> drops) {
	std::stringstream ss;

	for (std::vector<int>::iterator it = drops.begin(); it != drops.end(); ++it) {
		const RPG::Item* item = ReaderUtil::GetElement(Data::items, *it);
		// No Output::Warning needed here, reported later when the item is added
		std::string item_name = "??? BAD ITEM ???";
		if (item) {
			item_name = item->name;
		}

		if (Player::IsRPG2kE()) {
			Game_Message::texts.push_back(
				Utils::ReplacePlaceholders(
					Data::terms.item_recieved,
					{'S'},
					{item_name}
				) + Player::escape_symbol + "."
			);
		}
		else {
			ss.str("");
			ss << item_name << Data::terms.item_recieved << Player::escape_symbol << ".";
			Game_Message::texts.push_back(ss.str());
		}
	}
}

bool Scene_Battle_Rpg2k::CheckWin() {
	if (Game_Battle::CheckWin()) {
		Game_Temp::battle_result = Game_Temp::BattleVictory;
		SetState(State_Victory);

		int exp = Main_Data::game_enemyparty->GetExp();
		int money = Main_Data::game_enemyparty->GetMoney();
		std::vector<int> drops;
		Main_Data::game_enemyparty->GenerateDrops(drops);

		Game_Message::is_word_wrapped = Player::IsRPG2kE();
		Game_Message::texts.push_back(Data::terms.victory + Player::escape_symbol + "|");

		std::stringstream ss;
		if (exp > 0) {
			PushExperienceGainedMessage(exp);
		}
		if (money > 0) {
			PushGoldReceivedMessage(money);
		}
		PushItemRecievedMessages(drops);

		Game_System::BgmPlay(Game_System::GetSystemBGM(Game_System::BGM_Victory));

		// Update attributes
		std::vector<Game_Battler*> ally_battlers;
		Main_Data::game_party->GetActiveBattlers(ally_battlers);

		for (std::vector<Game_Battler*>::iterator it = ally_battlers.begin();
			it != ally_battlers.end(); ++it) {
				Game_Actor* actor = static_cast<Game_Actor*>(*it);
				actor->ChangeExp(actor->GetExp() + exp, true);
		}
		Main_Data::game_party->GainGold(money);
		for (std::vector<int>::iterator it = drops.begin(); it != drops.end(); ++it) {
			Main_Data::game_party->AddItem(*it, 1);
		}

		return true;
	}

	return false;
}

bool Scene_Battle_Rpg2k::CheckLose() {
	if (Game_Battle::CheckLose()) {
		Game_Temp::battle_result = Game_Temp::BattleDefeat;
		SetState(State_Defeat);

		Game_Message::SetPositionFixed(true);
		Game_Message::SetPosition(2);
		Game_Message::SetTransparent(false);

		Game_Message::is_word_wrapped = Player::IsRPG2kE();
		Game_Message::texts.push_back(Data::terms.defeat);

		Game_System::BgmPlay(Game_System::GetSystemBGM(Game_System::BGM_GameOver));

		return true;
	}

	return false;
}

bool Scene_Battle_Rpg2k::CheckResultConditions() {
	return CheckLose() || CheckWin();
}

