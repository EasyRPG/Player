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
#include "game_interpreter_battle.h"
#include "game_message.h"
#include "game_battle.h"
#include "game_battlealgorithm.h"
#include "battle_animation.h"
#include "reader_util.h"
#include "scene_battle_rpg2k.h"
#include "scene_battle.h"
#include "scene_gameover.h"
#include "output.h"

Scene_Battle_Rpg2k::Scene_Battle_Rpg2k() : Scene_Battle(),
battle_action_wait(0),
battle_action_state(BattleActionState_ConditionHeal)
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
	
	// If an event that changed status finishes without displaying a message window,
	// we need this so it can update automatically the status_window
	if (Game_Battle::GetInterpreter().finished_not_updated)
		status_window->Refresh();

	if (Game_Battle::GetInterpreter().IsRunning() && !interpreter_activated && !battle_message_window->GetVisible()) {
		battle_message_window->SetVisible(true);
		interpreter_activated = true;
	}
	else if (!Game_Battle::GetInterpreter().IsRunning() && interpreter_activated) {
		battle_message_window->SetVisible(false);
		interpreter_activated = false;
	}
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
	command_window->SetItemText(1, !active_actor->IsSkillRenamed() ? Data::terms.command_skill : active_actor->GetSkillName());
}

void Scene_Battle_Rpg2k::SetState(Scene_Battle::State new_state) {
	previous_state = state;
	if (new_state != State_SelectEnemyTarget && new_state != State_SelectAllyTarget)
		state = new_state;

	options_window->SetActive(false);
	status_window->SetActive(false);
	command_window->SetActive(false);
	item_window->SetActive(false);
	skill_window->SetActive(false);
	target_window->SetActive(false);
	battle_message_window->SetActive(false);

	if (new_state != State_SelectEnemyTarget && new_state != State_SelectAllyTarget) {
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
				skill_window->SetIndex(0);
			break;
		case State_Victory:
		case State_Defeat:
			battle_message_window->Clear();
			break;
		case State_Escape:
			battle_message_window->SetActive(true);
			break;
		}
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
		options_window->SetX(0);
		status_window->SetVisible(true);
		status_window->SetX(option_command_mov);
		status_window->SetIndex(-1);
		command_window->SetX(SCREEN_TARGET_WIDTH);
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

	// If Select:
	if (new_state == State_SelectEnemyTarget) {
		state = new_state;
		select_target_flash_count = 0;
		target_window->SetActive(true);
		target_window->SetVisible(true);
	} else if (new_state == State_SelectAllyTarget) {
		state = new_state;
		status_window->SetActive(true);
		status_window->SetVisible(true);
		status_window->SetX(0);
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
			SetState(State_SelectOption);
			CheckResultConditions();
		}
		break;
	case State_SelectOption:
		if (last_turn_check < Game_Battle::GetTurn()) {
			// Handle end of a battle caused by an event that ran on battle
			// start or at the end of a turn.
			CheckResultConditions();

			last_turn_check = Game_Battle::GetTurn();
		}

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
		if (!battle_actions.empty()) {
			Game_BattleAlgorithm::AlgorithmBase* alg = battle_actions.front()->GetBattleAlgorithm().get();

			if (ProcessBattleAction(alg)) {
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
			for (auto ally : Main_Data::game_party->GetActors()) {
				ally->SetDefending(false);
			}
			
			std::vector<Game_Battler*> enemy_battlers;
			Main_Data::game_enemyparty->GetBattlers(enemy_battlers);
			for (auto enemy : enemy_battlers) {
				enemy->SetDefending(false);
			}

			SetState(State_SelectOption);
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
	/**
     * Order of execution of BattleActionState:
	 * ConditionHeal > Execute > Apply > (ResultPop > ResultPush) > Death > Finished.
	 **/

	if (Game_Battle::IsBattleAnimationWaiting() && !Game_Battle::IsBattleAnimationOnlySound()) {
		return false;
	}
	else if (action->HasAnimationPlayed() && action->GetSecondAnimation() != nullptr && !action->HasSecondAnimationPlayed()) {
		action->PlaySecondAnimation();
		return false;
	}

	int critical_hit, default_result_lines;
	Sprite_Battler* source_sprite;
	Sprite_Battler* target_sprite;

	if (Input::IsPressed(Input::DECISION)) {
		--battle_action_wait;
	}

	if (battle_action_wait > 0) {
		if (--battle_action_wait) {
			return false;
		}
		else if (battle_message_window->GetHiddenLineCount()) {
			if (battle_message_window->IsPageFilled()) {
				if (battle_message_window->NextPage()) {
					battle_action_wait = GetDelayForLine();
					battle_message_window->ShowHiddenLines(1);
				}
			}
			else {
				if (battle_message_window->GetLineCount()) {
					battle_message_window->ShowHiddenLines(1);
					if (battle_message_window->IsPageFilled() &&
							battle_message_window->GetHiddenLineCount()) {
						battle_action_wait = GetDelayForLine();
					}
					else {
						battle_action_wait = GetDelayForWindow();
					}
				}
			}
			return false;
		}
	}

	if (Input::IsPressed(Input::CANCEL)) {
		return false;
	}

	battle_action_wait = action->GetType() == "NoMove" ? 0 : GetDelayForWindow();

	switch (battle_action_state) {
	case BattleActionState_ConditionHeal:
		battle_action_wait = 0;
		if (action->IsFirstAttack()) {
			battle_message_window->Clear();
			source_sprite = Game_Battle::GetSpriteset().FindBattler(action->GetSource());
			if (source_sprite) {
				source_sprite->Flash(Color(255, 255, 255, 100), 15);
				source_sprite->SetAnimationState(
					action->GetSourceAnimationState(),
					Sprite_Battler::LoopState_DefaultAnimationAfterFinish);
			}

			const RPG::State* highest_state = nullptr;
			std::string message_to_print;
			std::vector<int16_t> states_to_heal = action->GetSource()->NextBattleTurn();
			std::vector<int16_t> states_remaining = action->GetSource()->GetInflictedStates();
			action->GetSource()->ApplyConditions();

			for (auto state_id : states_to_heal) {
				const RPG::State* state = ReaderUtil::GetElement(Data::states, state_id);
				if (!state->message_recovery.empty() && (!highest_state || state->priority >= highest_state->priority)) {
					highest_state = state;
					message_to_print = state->message_recovery;
				}
			}
			for (auto state_id : states_remaining) {
				const RPG::State* state = ReaderUtil::GetElement(Data::states, state_id);
				if (!state->message_affected.empty() && (!highest_state || state->priority >= highest_state->priority)) {
					highest_state = state;
					message_to_print = state->message_affected;
				}
			}
			if (highest_state != nullptr) {
				battle_message_window->PushWithSubject(message_to_print, action->GetSource()->GetName());
				battle_action_wait = GetDelayForWindow() * 3 / 2;
			}
		}

		battle_action_state = BattleActionState_Execute;

		break;
		case BattleActionState_Execute:
			if (action->IsFirstAttack()) {
				action->TargetFirst();
			}

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

			action->Execute();

			battle_message_window->Clear();
			battle_message_window->Push(action->GetStartMessage());

			battle_result_messages.clear();
			battle_result_order.clear();
			action->GetResultMessages(battle_result_messages, battle_result_order);
			battle_result_messages_it = battle_result_messages.begin();
			battle_result_order_it = battle_result_order.begin();

			if (!action->IsSecondStartMessage())
				battle_action_wait = 0;

			battle_action_state = BattleActionState_Apply;

			if (!action->IsFirstAttack()) {
				battle_action_wait = 0;
				return ProcessBattleAction(action);
			}

			break;
		case BattleActionState_Apply:
			if (action->IsSecondStartMessage())
				battle_message_window->Push(action->GetSecondStartMessage());

			if (action->IsFirstAttack()) {

				if (action->GetTarget() &&
					!(action->GetSource()->GetSignificantRestriction() == RPG::State::Restriction_attack_ally
						&& action->GetSource()->GetType() == Game_Battler::Type_Enemy)) {
					if (action->GetTarget()->GetType() == Game_Battler::Type_Enemy) {
						action->PlayAnimation();
					}
					else if (action->GetTarget()->GetType() == Game_Battler::Type_Ally
						&& (action->GetType() == "Skill" || action->GetSource()->GetSignificantRestriction() == RPG::State::Restriction_attack_ally)) {
						action->PlaySoundAnimation(false, 20);
					}
				}
			}

			if (action->IsFirstAttack() && action->GetStartSe()) {
				Game_System::SePlay(*action->GetStartSe());
			}

			action->Apply();
			battle_action_state = BattleActionState_ResultPop;

			if (action->GetSourceAnimationStateApply() != NULL) {
				source_sprite = Game_Battle::GetSpriteset().FindBattler(action->GetSource());
				source_sprite->SetAnimationState(
					action->GetSourceAnimationStateApply(),
					Sprite_Battler::LoopState_DefaultAnimationAfterFinish);
			}

			if (!action->IsFirstAttack()) {
				battle_action_wait = 0;
				return ProcessBattleAction(action);
			}

			break;
		case BattleActionState_ResultPop:
			battle_action_wait = std::min<int>(GetDelayForLine() / 2, battle_action_wait);

			if (battle_result_messages_it != battle_result_messages.end()) {
				critical_hit = action->IsCriticalHit() && action->IsSuccess() ? 1 : 0;
				default_result_lines = 1 + (action->IsSecondStartMessage() ? 1 : 0) + critical_hit;
				while (battle_message_window->GetLineCount() > (default_result_lines + *battle_result_order_it))
					battle_message_window->Pop();
			}

			battle_action_state = BattleActionState_ResultPush;

			return ProcessBattleAction(action);
		case BattleActionState_ResultPush:
			critical_hit = action->IsCriticalHit() && action->IsSuccess() ? 1 : 0;

			if (battle_result_messages_it != battle_result_messages.end()) {

				// Animation and Sound when hurt (only when HP damage):
				target_sprite = Game_Battle::GetSpriteset().FindBattler(action->GetTarget());
				if (battle_result_messages_it == battle_result_messages.begin() + critical_hit) {
					if (action->IsSuccess() && target_sprite && !action->IsPositive() && !action->IsAbsorb() && action->GetAffectedHp() > -1) {
						target_sprite->SetAnimationState(Sprite_Battler::AnimationState_Damage);
					}
					if (action->IsSuccess() && action->GetTarget()->GetType() == Game_Battler::Type_Ally && !action->IsPositive() && !action->IsAbsorb() && action->GetAffectedHp() > 0) {
						Main_Data::game_screen->ShakeOnce(2, 16, 1);
					}

					if (action->GetResultSe()) {
						Game_System::SePlay(*action->GetResultSe());
					}
				} else {
					if (target_sprite) {
						target_sprite->SetAnimationState(Sprite_Battler::AnimationState_Idle);
					}
				}

				// Push message, next iteration:
				battle_message_window->Push(*battle_result_messages_it);
				++battle_result_messages_it;
				++battle_result_order_it;

				// Only goes here if it's the first message of a critic hit:
				if (battle_result_messages_it == battle_result_messages.begin() + critical_hit) {
					battle_action_wait = GetDelayForLine() * 2;
				}
			}

			// When it finishes
			if (battle_result_messages_it == battle_result_messages.end()) {
				battle_action_wait = 0;
				battle_action_state = BattleActionState_Death;
			}
			else {
				battle_action_state = BattleActionState_ResultPop;
			}

			break;
		case BattleActionState_Death:
			if (action->GetTarget()) {
				target_sprite = Game_Battle::GetSpriteset().FindBattler(action->GetTarget());
				if (action->GetTarget()->IsDead() && action->GetDeathSe()) {
					Game_System::SePlay(*action->GetDeathSe());
				}
				if (target_sprite) {
					target_sprite->DetectStateChange();
				}
			}
			if (battle_result_messages.empty())
				battle_action_wait = GetDelayForLine();

			battle_action_state = BattleActionState_Finished;

			break;
		case BattleActionState_Finished:
			battle_action_wait = 0;
			if (action->GetTarget()) {
				target_sprite = Game_Battle::GetSpriteset().FindBattler(action->GetTarget());
				if (action->GetTarget() && target_sprite && !target_sprite->IsIdling()) {
					return false;
				}
			}

			battle_action_state = BattleActionState_ConditionHeal;
			return !action->TargetNext();
	}

	return false;
}

void Scene_Battle_Rpg2k::ProcessInput() {
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
		battle_result_order.clear();
		escape_alg.GetResultMessages(battle_result_messages, battle_result_order);

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
				CreateEnemyActions();
				CreateExecutionOrder();
				UpdateBattlerAction(battle_actions.front());

				NextTurn();
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
		if (!Game_Temp::battle_first_strike || Game_Battle::GetTurn() > 0) {
			CreateEnemyActions();
		}
		CreateExecutionOrder();
		UpdateBattlerAction(battle_actions.front());

		NextTurn();
		Game_Battle::RefreshEvents();

		return;
	}

	active_actor = allies[actor_index];
	status_window->SetIndex(actor_index);
	actor_index++;

	if (active_actor->IsDead()) {
		SelectNextActor();
		return;
	}

	Game_Battler* random_target = NULL;

	if (active_actor->CanAct()) {
		switch (active_actor->GetSignificantRestriction()) {
		case RPG::State::Restriction_attack_ally:
			random_target = Main_Data::game_party->GetRandomActiveBattler();
			break;
		case RPG::State::Restriction_attack_enemy:
			random_target = Main_Data::game_enemyparty->GetRandomActiveBattler();
			break;
		}
	}
	else {
		active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::NoMove>(active_actor));
		battle_actions.push_back(active_actor);
		SelectNextActor();
		return;
	}

	if (random_target || auto_battle || active_actor->GetAutoBattle()) {
		if (!random_target) {
			random_target = Main_Data::game_enemyparty->GetRandomActiveBattler();
		}

		// ToDo: Auto battle logic is dumb
		active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(active_actor, random_target));
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

	if (active_actor->GetAutoBattle()) {
		SelectPreviousActor();
		return;
	}

	SetState(State_SelectActor);
}

static bool BattlerSort(Game_Battler* first, Game_Battler* second) {
	if (first->HasPreemptiveAttack() && second->HasPreemptiveAttack()) {
		return first->GetRandomOrderAgi() > second->GetRandomOrderAgi();
	}

	if (first->HasPreemptiveAttack()) {
		return true;
	}

	if (second->HasPreemptiveAttack()) {
		return false;
	}

	return first->GetRandomOrderAgi() > second->GetRandomOrderAgi();
}

void Scene_Battle_Rpg2k::CreateExecutionOrder() {
	// Define random Agility. Must be done outside of the sort function because of the "strict weak ordering" property, so the sort is consistent
	for (auto battler : battle_actions) {
		battler->SetRandomOrderAgi();
	}
	std::sort(battle_actions.begin(), battle_actions.end(), BattlerSort);
	for (auto battler : battle_actions) {
		battler->GetBattleAlgorithm()->ApplyFirst();
	}
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

void Scene_Battle_Rpg2k::SetWaitForEnemyAppearanceMessages() {
	if ((enemy_iterator == visible_enemies.end() &&
			!battle_message_window->GetHiddenLineCount()) ||
			battle_message_window->IsPageFilled()) {
		encounter_message_sleep_until = Player::GetFrames() + GetDelayForWindow();
	}
	else {
		encounter_message_sleep_until = Player::GetFrames() + GetDelayForLine();
	}
}

bool Scene_Battle_Rpg2k::DisplayMonstersInMessageWindow() {
	if (encounter_message_first_monster) {
		Main_Data::game_enemyparty->GetActiveBattlers(visible_enemies);
		enemy_iterator = visible_enemies.begin();
		encounter_message_first_monster = false;
	}

	if (Input::IsPressed(Input::DECISION)) {
		--encounter_message_sleep_until;
	}

	if (encounter_message_sleep_until > -1) {
		if (Player::GetFrames() >= encounter_message_sleep_until) {
			// Sleep over
			encounter_message_sleep_until = -1;
		} else {
			return false;
		}
	}

	if (battle_message_window->GetHiddenLineCount() > 0) {
		if (battle_message_window->IsPageFilled()) {
			battle_message_window->NextPage();
		}
		else {
			battle_message_window->ShowHiddenLines(1);
		}
		SetWaitForEnemyAppearanceMessages();
		return false;
	}

	if (enemy_iterator == visible_enemies.end()) {
		battle_message_window->Clear();
		if (Game_Temp::battle_first_strike && !encounter_message_first_strike) {
			battle_message_window->Push(Data::terms.special_combat);
			encounter_message_sleep_until = Player::GetFrames() + 60;
			encounter_message_first_strike = true;
			return false;
		}
		else {
			//reset static vars
			encounter_message_first_strike = false;
			encounter_message_first_monster = true;
			return true;
		}
	}

	if (battle_message_window->IsPageFilled()) {
		battle_message_window->NextPage();
	}
	battle_message_window->PushWithSubject(Data::terms.encounter, (*enemy_iterator)->GetName());
	++enemy_iterator;

	SetWaitForEnemyAppearanceMessages();

	return false;
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
			) + "\\."
		);
	}
	else {
		ss << exp << Data::terms.exp_received << "\\.";
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
			) + "\\."
		);
	}
	else {
		ss << Data::terms.gold_recieved_a << " " << money << Data::terms.gold << Data::terms.gold_recieved_b << "\\.";
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
				) + "\\."
			);
		}
		else {
			ss.str("");
			ss << item_name << Data::terms.item_recieved << "\\.";
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
		Game_Message::texts.push_back(Data::terms.victory + "\\|");

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
