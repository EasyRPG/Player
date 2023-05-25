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
#include "sprite_enemy.h"
#include "game_battler.h"
#include "game_system.h"
#include "game_party.h"
#include "game_enemy.h"
#include "game_enemyparty.h"
#include "game_message.h"
#include "game_battle.h"
#include "game_battlealgorithm.h"
#include "game_screen.h"
#include "battle_animation.h"
#include <lcf/reader_util.h>
#include "scene_battle_rpg2k.h"
#include "scene_battle.h"
#include "scene_gameover.h"
#include "game_interpreter_battle.h"
#include "output.h"
#include "rand.h"
#include "autobattle.h"
#include "enemyai.h"
#include "battle_message.h"
#include "feature.h"

Scene_Battle_Rpg2k::Scene_Battle_Rpg2k(const BattleArgs& args) :
	Scene_Battle(args)
{
}

Scene_Battle_Rpg2k::~Scene_Battle_Rpg2k() {
}

void Scene_Battle_Rpg2k::Start() {
	Scene_Battle::Start();
	CreateEnemySprites();
}

void Scene_Battle_Rpg2k::CreateUi() {
	Scene_Battle::CreateUi();

	status_window.reset(new Window_BattleStatus(Player::menu_offset_x, (Player::screen_height - Player::menu_offset_y - 80), MENU_WIDTH - option_command_mov, 80));

	CreateBattleTargetWindow();
	CreateBattleCommandWindow();

	battle_message_window.reset(new Window_BattleMessage(Player::menu_offset_x, (Player::screen_height - Player::menu_offset_y - 80), MENU_WIDTH, 80));

	if (!IsEscapeAllowed()) {
		auto it = std::find(battle_options.begin(), battle_options.end(), Escape);
		if (it != battle_options.end()) {
			options_window->DisableItem(std::distance(battle_options.begin(), it));
		}
	}

	SetCommandWindows(Player::menu_offset_x);

	ResetWindows(true);
	battle_message_window->SetVisible(true);
}

void Scene_Battle_Rpg2k::CreateEnemySprites() {
	for (auto* enemy: Main_Data::game_enemyparty->GetEnemies()) {
		auto sprite = std::make_unique<Sprite_Enemy>(enemy);
		sprite->SetVisible(true);
		enemy->SetBattleSprite(std::move(sprite));
	}
}

static std::vector<std::string> GetEnemyTargetNames() {
	std::vector<std::string> commands;

	std::vector<Game_Battler*> enemies;
	Main_Data::game_enemyparty->GetActiveBattlers(enemies);

	for (auto& enemy: enemies) {
		commands.push_back(ToString(enemy->GetName()));
	}

	return commands;
}

void Scene_Battle_Rpg2k::CreateBattleTargetWindow() {
	auto commands = GetEnemyTargetNames();
	target_window.reset(new Window_Command(std::move(commands), 136, 4));
	target_window->SetHeight(80);
	target_window->SetX(Player::menu_offset_x);
	target_window->SetY(Player::screen_height - Player::menu_offset_y - 80);
	// Above other windows
	target_window->SetZ(Priority_Window + 10);
}

void Scene_Battle_Rpg2k::RefreshTargetWindow() {
	auto commands = GetEnemyTargetNames();
	target_window->ReplaceCommands(std::move(commands));
}

std::vector<std::string> Scene_Battle_Rpg2k::GetBattleCommandNames2k3(const Game_Actor* actor) {
	std::vector<std::string> commands;
	if (actor) {
		for (auto* cmd: actor->GetBattleCommands()) {
			if (cmd->type != lcf::rpg::BattleCommand::Type_escape) {
				commands.push_back(ToString(cmd->name));
			}
		}
	}

	return commands;
}

void Scene_Battle_Rpg2k::CreateBattleCommandWindow() {
	std::vector<std::string> commands = {
		ToString(lcf::Data::terms.command_attack),
		ToString(lcf::Data::terms.command_skill),
		ToString(lcf::Data::terms.command_defend),
		ToString(lcf::Data::terms.command_item)
	};

	command_window.reset(new Window_Command(std::move(commands), 76));
	command_window->SetHeight(80);
	command_window->SetX(Player::screen_width - Player::menu_offset_x - option_command_mov);
	command_window->SetY(Player::screen_height - Player::menu_offset_y - 80);
}

void Scene_Battle_Rpg2k::RefreshCommandWindow() {
	if (Player::IsRPG2k3() && Feature::HasRpg2kBattleSystem() && !lcf::Data::system.easyrpg_use_rpg2k_battle_commands) {
		int index = command_window->GetIndex();
		auto commands = GetBattleCommandNames2k3(active_actor);
		command_window->ReplaceCommands(std::move(commands));
		command_window->SetIndex(index);
	} else {
		command_window->SetItemText(1, active_actor->GetSkillName());
	}
}

void Scene_Battle_Rpg2k::SetState(Scene_Battle::State new_state) {
	previous_state = state;
	state = new_state;

	SetSceneActionSubState(0);
}

bool Scene_Battle_Rpg2k::UpdateBattleState() {
	if (resume_from_debug_scene) {
		resume_from_debug_scene = false;
		return true;
	}

	UpdateScreen();
	UpdateBattlers();
	UpdateUi();
	battle_message_window->Update();

	if (!UpdateEvents()) {
		return false;
	}

	if (!UpdateTimers()) {
		return false;
	}

	if (Input::IsTriggered(Input::DEBUG_MENU)) {
		if (this->CallDebug()) {
			// Set this flag so that when we return and run update again, we resume exactly from after this point.
			resume_from_debug_scene = true;
			return false;
		}
	}
	return true;
}

void Scene_Battle_Rpg2k::vUpdate() {
	const auto process_scene = UpdateBattleState();

	while (process_scene) {
		// Something ended the battle.
		if (Scene::instance.get() != this) {
			break;
		}

		if (IsWindowMoving()) {
			break;
		}

		if (Game_Message::IsMessageActive() || Game_Battle::GetInterpreter().IsRunning()) {
			break;
		}

		if (!CheckWait()) {
			break;
		}

		if (ProcessSceneAction() == SceneActionReturn::eWaitTillNextFrame) {
			break;
		}
	}

	Game_Battle::UpdateGraphics();
}

void Scene_Battle_Rpg2k::SetSceneActionSubState(int substate) {
	scene_action_substate = substate;
}

void Scene_Battle_Rpg2k::NextTurn() {
	Main_Data::game_party->IncTurns();
	Game_Battle::GetInterpreterBattle().ResetPagesExecuted();
}

bool Scene_Battle_Rpg2k::CheckBattleEndAndScheduleEvents() {
	if (CheckBattleEndConditions()) {
		return false;
	}

	auto& interp = Game_Battle::GetInterpreterBattle();

	int page = interp.ScheduleNextPage(nullptr);
#ifdef EP_DEBUG_BATTLE2K_STATE_MACHINE
	if (page) {
		Output::Debug("Battle2k ScheduleNextEventPage Scheduled Page {} frame={}", page, Main_Data::game_system->GetFrameCounter());
	} else {
		Output::Debug("Battle2k ScheduleNextEventPage No Events to Run frame={}", Main_Data::game_system->GetFrameCounter());
	}
#else
	(void)page;
#endif

	return !interp.IsRunning();
}


Scene_Battle_Rpg2k::SceneActionReturn Scene_Battle_Rpg2k::ProcessSceneAction() {
#ifdef EP_DEBUG_BATTLE2K_STATE_MACHINE
	static int last_state = -1;
	static int last_substate = -1;
	if (state != last_state || scene_action_substate != last_substate) {
		Output::Debug("Battle2k ProcessSceneAction({},{}) frames={}", state, scene_action_substate, Main_Data::game_system->GetFrameCounter());
		last_state = state;
		last_substate = scene_action_substate;
	}
#endif
	switch (state) {
		case State_Start:
			return ProcessSceneActionStart();
		case State_SelectOption:
			return ProcessSceneActionFightAutoEscape();
		case State_SelectActor:
			return ProcessSceneActionActor();
		case State_AutoBattle:
			return ProcessSceneActionAutoBattle();
		case State_SelectCommand:
			return ProcessSceneActionCommand();
		case State_SelectItem:
			return ProcessSceneActionItem();
		case State_SelectSkill:
			return ProcessSceneActionSkill();
		case State_SelectEnemyTarget:
			return ProcessSceneActionEnemyTarget();
		case State_SelectAllyTarget:
			return ProcessSceneActionAllyTarget();
		case State_Battle:
			return ProcessSceneActionBattle();
		case State_Victory:
			return ProcessSceneActionVictory();
		case State_Defeat:
			return ProcessSceneActionDefeat();
		case State_Escape:
			return ProcessSceneActionEscape();
	}
	assert(false && "Invalid SceneActionState!");
	return SceneActionReturn::eWaitTillNextFrame;
}

Scene_Battle_Rpg2k::SceneActionReturn Scene_Battle_Rpg2k::ProcessSceneActionStart() {
	enum SubState {
		eBegin,
		eDisplayMonsters,
		eFirstStrike,
		eClear
	};

	if (scene_action_substate == eBegin) {
		ResetWindows(true);
		battle_message_window->SetVisible(true);

		std::vector<Game_Battler *> visible_enemies;
		// First time entered, initialize.
		Main_Data::game_enemyparty->GetActiveBattlers(visible_enemies);

		for (auto& enemy: visible_enemies) {
			// Format and wordwrap all messages, then pull them out and push them back 1 at a time.
			battle_message_window->PushWithSubject(lcf::Data::terms.encounter, enemy->GetName());
		}

		battle_result_messages = battle_message_window->GetLines();
		battle_result_messages_it = battle_result_messages.begin();
		battle_message_window->Clear();

		if (!visible_enemies.empty()) {
			SetWait(4, 4);
		}
		SetSceneActionSubState(eDisplayMonsters);
		return SceneActionReturn::eContinueThisFrame;
	}

	if (scene_action_substate == eDisplayMonsters) {
		if (battle_result_messages_it == battle_result_messages.end()) {
			SetSceneActionSubState(eFirstStrike);
			return SceneActionReturn::eContinueThisFrame;
		}

		if (battle_message_window->IsPageFilled()) {
			battle_message_window->Clear();
			SetWait(4, 4);
			return SceneActionReturn::eContinueThisFrame;
		}

		battle_message_window->Push(*battle_result_messages_it);
		++battle_result_messages_it;

		if (battle_result_messages_it == battle_result_messages.end() ||
				battle_message_window->IsPageFilled()) {
			SetWait(30, 70);
		}
		else {
			SetWait(8, 8);
		}

		return SceneActionReturn::eContinueThisFrame;
	}

	if (scene_action_substate == eFirstStrike) {
		battle_message_window->Clear();
		battle_result_messages.clear();
		battle_result_messages_it = battle_result_messages.end();

		if (first_strike) {
			battle_message_window->Push(lcf::Data::terms.special_combat);
			SetWait(30, 70);
		}

		SetSceneActionSubState(eClear);

		return SceneActionReturn::eContinueThisFrame;
	}

	if (scene_action_substate == eClear) {
		battle_message_window->Clear();
		SetState(State_SelectOption);
	}

	return SceneActionReturn::eContinueThisFrame;
}

void Scene_Battle_Rpg2k::ResetWindows(bool make_invisible) {
	options_window->SetActive(false);
	status_window->SetActive(false);
	command_window->SetActive(false);
	item_window->SetActive(false);
	skill_window->SetActive(false);
	target_window->SetActive(false);
	battle_message_window->SetActive(false);

	if (!make_invisible) {
		return;
	}

	options_window->SetVisible(false);
	status_window->SetVisible(false);
	command_window->SetVisible(false);
	target_window->SetVisible(false);
	battle_message_window->SetVisible(false);
	item_window->SetVisible(false);
	skill_window->SetVisible(false);
	help_window->SetVisible(false);
}

void Scene_Battle_Rpg2k::SetCommandWindows(int x) {
	options_window->SetX(x);
	x += options_window->GetWidth();
	status_window->SetX(x);
	x += status_window->GetWidth();
	command_window->SetX(x);
}

void Scene_Battle_Rpg2k::MoveCommandWindows(int x, int frames) {
	options_window->InitMovement(options_window->GetX(), options_window->GetY(),
			x, options_window->GetY(), frames);

	x += options_window->GetWidth();
	status_window->InitMovement(status_window->GetX(), status_window->GetY(),
			x, status_window->GetY(), frames);

	x += status_window->GetWidth();
	command_window->InitMovement(command_window->GetX(), command_window->GetY(),
			x, command_window->GetY(), frames);
}

Scene_Battle_Rpg2k::SceneActionReturn Scene_Battle_Rpg2k::ProcessSceneActionFightAutoEscape() {
	enum SubState {
		eBegin,
		eCheckEvents,
		eMoveWindow,
		eWaitForInput,
	};

	if (scene_action_substate == eBegin) {
		ResetWindows(true);
		battle_message_window->SetVisible(true);

		SetSceneActionSubState(eCheckEvents);
	}

	if (scene_action_substate == eCheckEvents) {
		if (!CheckBattleEndAndScheduleEvents()) {
			return SceneActionReturn::eContinueThisFrame;
		}

		// No Auto battle/Escape when all actors are sleeping or similar
		if (!Main_Data::game_party->IsAnyControllable()) {
			SetState(State_SelectActor);
			return SceneActionReturn::eContinueThisFrame;
		}

		SetSceneActionSubState(eMoveWindow);
	}

	if (scene_action_substate == eMoveWindow) {
		options_window->SetVisible(true);
		status_window->SetVisible(true);
		status_window->SetIndex(-1);
		command_window->SetIndex(-1);
		command_window->SetVisible(true);
		battle_message_window->SetVisible(false);
		status_window->Refresh();

		if (previous_state == State_SelectCommand) {
			MoveCommandWindows(Player::menu_offset_x, 8);
		} else {
			SetCommandWindows(Player::menu_offset_x);
		}
		SetSceneActionSubState(eWaitForInput);
		// Prevent that DECISION from a closed message triggers a battle option in eWaitForInput
		Input::ResetTriggerKeys();
		return SceneActionReturn::eContinueThisFrame;
	}

	if (scene_action_substate == eWaitForInput) {
		options_window->SetActive(true);

		if (Input::IsTriggered(Input::DECISION)) {
			if (!message_window->IsVisible()) {
				switch (battle_options[options_window->GetIndex()]) {
					case Battle: // Battle
						Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
						RefreshTargetWindow();
						target_window->SetVisible(false);
						SetState(State_SelectActor);
						break;
					case AutoBattle: // Auto Battle
						SetState(State_AutoBattle);
						Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
						break;
					case Escape: // Escape
						if (!IsEscapeAllowed()) {
							Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
						}
						else {
							Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
							SetState(State_Escape);
						}
						break;
				}
			}
			return SceneActionReturn::eWaitTillNextFrame;
		}
	}
	return SceneActionReturn::eWaitTillNextFrame;
}

Scene_Battle_Rpg2k::SceneActionReturn Scene_Battle_Rpg2k::ProcessSceneActionActor() {
	SelectNextActor(false);
	return SceneActionReturn::eContinueThisFrame;
}

Scene_Battle_Rpg2k::SceneActionReturn Scene_Battle_Rpg2k::ProcessSceneActionAutoBattle() {
	SelectNextActor(true);
	return SceneActionReturn::eContinueThisFrame;
}

Scene_Battle_Rpg2k::SceneActionReturn Scene_Battle_Rpg2k::ProcessSceneActionCommand() {
	enum SubState {
		eMoveWindow,
		eWaitForInput,
	};

	if (scene_action_substate == eMoveWindow) {
		RefreshCommandWindow();

		ResetWindows(true);

		options_window->SetVisible(true);
		status_window->SetVisible(true);
		command_window->SetVisible(true);
		if (previous_state == State_SelectActor) {
			command_window->SetIndex(0);
		}

		MoveCommandWindows(Player::menu_offset_x - options_window->GetWidth(), 8);
		SetSceneActionSubState(eWaitForInput);
		return SceneActionReturn::eContinueThisFrame;
	}

	if (scene_action_substate == eWaitForInput) {
		command_window->SetActive(true);
		if (Input::IsTriggered(Input::DECISION)) {
			if (Player::IsRPG2k3() && Feature::HasRpg2kBattleSystem() && !lcf::Data::system.easyrpg_use_rpg2k_battle_commands) {
				int index = command_window->GetIndex();
				const auto* command = active_actor->GetBattleCommand(index);

				if (command) {
					active_actor->SetLastBattleAction(command->ID);
					switch (command->type) {
						case lcf::rpg::BattleCommand::Type_attack:
							AttackSelected();
							break;
						case lcf::rpg::BattleCommand::Type_defense:
							DefendSelected();
							break;
						case lcf::rpg::BattleCommand::Type_item:
							Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
							SetState(State_SelectItem);
							break;
						case lcf::rpg::BattleCommand::Type_skill:
							Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
							skill_window->SetSubsetFilter(0);
							SetState(State_SelectSkill);
							break;
						case lcf::rpg::BattleCommand::Type_special:
							Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
							SpecialSelected2k3();
							break;
						case lcf::rpg::BattleCommand::Type_subskill:
							Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
							SubskillSelected2k3(command->ID);
							break;
					}
				}
			} else {
				switch (command_window->GetIndex()) {
					case 0: // Attack
						AttackSelected();
						break;
					case 1: // Skill
						Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
						SetState(State_SelectSkill);
						break;
					case 2: // Defense
						DefendSelected();
						break;
					case 3: // Item
						Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
						SetState(State_SelectItem);
						break;
					default:
						// no-op
						break;
				}
			}
			return SceneActionReturn::eWaitTillNextFrame;
		}
		if (Input::IsTriggered(Input::CANCEL)) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
			--actor_index;
			SelectPreviousActor();
			return SceneActionReturn::eWaitTillNextFrame;
		}
	}
	return SceneActionReturn::eWaitTillNextFrame;
}

Scene_Battle_Rpg2k::SceneActionReturn Scene_Battle_Rpg2k::ProcessSceneActionItem() {
	enum SubState {
		eBegin,
		eWaitForInput,
	};

	if (scene_action_substate == eBegin) {
		ResetWindows(true);

		item_window->SetVisible(true);
		item_window->SetActive(true);
		item_window->SetActor(active_actor);
		item_window->Refresh();
		item_window->SetHelpWindow(help_window.get());
		help_window->SetVisible(true);

		SetSceneActionSubState(eWaitForInput);
	}

	if (scene_action_substate == eWaitForInput) {
		if (Input::IsTriggered(Input::DECISION)) {
			ItemSelected();
			return SceneActionReturn::eWaitTillNextFrame;
		}
		if (Input::IsTriggered(Input::CANCEL)) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
			SetState(State_SelectCommand);
			return SceneActionReturn::eWaitTillNextFrame;
		}
	}
	return SceneActionReturn::eWaitTillNextFrame;
}

Scene_Battle_Rpg2k::SceneActionReturn Scene_Battle_Rpg2k::ProcessSceneActionSkill() {
	enum SubState {
		eBegin,
		eWaitForInput,
	};

	if (scene_action_substate == eBegin) {
		ResetWindows(true);

		skill_window->SetActive(true);
		skill_window->SetActor(active_actor->GetId());
		if (previous_state == State_SelectCommand) {
			skill_window->RestoreActorIndex(actor_index - 1);
		}
		skill_window->SetVisible(true);
		skill_window->SetHelpWindow(help_window.get());
		help_window->SetVisible(true);

		SetSceneActionSubState(eWaitForInput);
	}

	if (scene_action_substate == eWaitForInput) {
		if (Input::IsTriggered(Input::DECISION)) {
			skill_window->SaveActorIndex(actor_index - 1);
			SkillSelected();
			return SceneActionReturn::eWaitTillNextFrame;
		}
		if (Input::IsTriggered(Input::CANCEL)) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
			skill_window->SaveActorIndex(actor_index - 1);
			SetState(State_SelectCommand);
			return SceneActionReturn::eWaitTillNextFrame;
		}
	}
	return SceneActionReturn::eWaitTillNextFrame;
}

Scene_Battle_Rpg2k::SceneActionReturn Scene_Battle_Rpg2k::ProcessSceneActionEnemyTarget() {
	enum SubState {
		eBegin,
		eWaitForInput,
	};

	std::vector<Game_Battler*> enemies;
	Main_Data::game_enemyparty->GetActiveBattlers(enemies);
	Game_Enemy* target = static_cast<Game_Enemy*>(enemies[target_window->GetIndex()]);

	if (scene_action_substate == eBegin) {
		select_target_flash_count = 0;
		ResetWindows(false);

		target_window->SetVisible(true);
		target_window->SetActive(true);
		target_window->SetVisible(true);
		target_window->SetIndex(0);

		SetSceneActionSubState(eWaitForInput);
	}

	++select_target_flash_count;

	if (select_target_flash_count == 60) {
		SelectionFlash(target);
		select_target_flash_count = 0;
	}

	if (scene_action_substate == eWaitForInput) {
		if (Input::IsTriggered(Input::DECISION)) {
			EnemySelected();
			return SceneActionReturn::eWaitTillNextFrame;
		}
		if (Input::IsTriggered(Input::CANCEL)) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
			SetState(previous_state);
			return SceneActionReturn::eWaitTillNextFrame;
		}
	}
	return SceneActionReturn::eWaitTillNextFrame;
}

Scene_Battle_Rpg2k::SceneActionReturn Scene_Battle_Rpg2k::ProcessSceneActionAllyTarget() {
	enum SubState {
		eBegin,
		eWaitForInput,
	};

	if (scene_action_substate == eBegin) {
		ResetWindows(false);
		status_window->SetActive(true);
		status_window->SetVisible(true);
		status_window->SetIndex(0);

		SetSceneActionSubState(eWaitForInput);
	}

	if (scene_action_substate == eWaitForInput) {
		if (Input::IsTriggered(Input::DECISION)) {
			AllySelected();
			return SceneActionReturn::eWaitTillNextFrame;
		}
		if (Input::IsTriggered(Input::CANCEL)) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
			SetState(previous_state);
			return SceneActionReturn::eWaitTillNextFrame;
		}
	}
	return SceneActionReturn::eWaitTillNextFrame;
}

Scene_Battle_Rpg2k::SceneActionReturn Scene_Battle_Rpg2k::ProcessSceneActionBattle() {
	enum SubState {
		eBegin,
		ePreAction,
		eBattleAction,
		ePost,
	};

	if (scene_action_substate == eBegin) {
		ResetWindows(true);
		battle_message_window->SetVisible(true);

		SetSceneActionSubState(ePreAction);
	}

	if (scene_action_substate == ePreAction) {
		// Remove actions for battlers who were killed or removed from the battle.
		while (!battle_actions.empty() && !battle_actions.front()->Exists()) {
			RemoveCurrentAction();
		}

		// Check for end battle, and run events before action
		// This happens before each battler acts and also right after the last battler acts.
		if (!CheckBattleEndAndScheduleEvents()) {
			return SceneActionReturn::eContinueThisFrame;
		}

		if (battle_actions.empty()) {
			SetSceneActionSubState(ePost);
			return SceneActionReturn::eContinueThisFrame;
		}

		auto* battler = battle_actions.front();
		// If we will start a new battle action, first check for state changes
		// such as death, paralyze, confuse, etc..
		PrepareBattleAction(battler);
		pending_battle_action = battler->GetBattleAlgorithm();

#ifdef EP_DEBUG_BATTLE2K_STATE_MACHINE
		Output::Debug("Battle2k StartBattleAction battler={} frame={}", battler->GetName(), Main_Data::game_system->GetFrameCounter());
#endif

		// Initialize battle state
		battle_action_wait = 0;
		SetBattleActionState(BattleActionState_Begin);
		battle_action_start_index = 0;
		battle_action_results_index = 0;
		battle_action_dmg_index = 0;
		battle_action_substate_index = 0;
		pending_message = {};

		SetSceneActionSubState(eBattleAction);
	}

	if (scene_action_substate == eBattleAction) {
		if (ProcessBattleAction(pending_battle_action.get()) == BattleActionReturn::eContinue) {
			return SceneActionReturn::eContinueThisFrame;
		}

		pending_battle_action = nullptr;
		RemoveCurrentAction();
		battle_message_window->Clear();

		SetSceneActionSubState(ePreAction);
		return SceneActionReturn::eContinueThisFrame;
	}

	if (scene_action_substate == ePost) {
		// Everybody acted
		actor_index = 0;
		first_strike = false;

		SetState(State_SelectOption);
		return SceneActionReturn::eWaitTillNextFrame;
	}
	return SceneActionReturn::eWaitTillNextFrame;
}

Scene_Battle_Rpg2k::SceneActionReturn Scene_Battle_Rpg2k::ProcessSceneActionVictory() {
	enum SubState {
		eBegin = 0,
		eEnd = 1,
	};

	if (scene_action_substate == eBegin) {
		ResetWindows(true);
		battle_message_window->Clear();
		battle_message_window->SetVisible(true);

		int exp = Main_Data::game_enemyparty->GetExp();
		int money = Main_Data::game_enemyparty->GetMoney();
		std::vector<int> drops;
		Main_Data::game_enemyparty->GenerateDrops(drops);

		auto pm = PendingMessage();
		pm.SetEnableFace(false);

		pm.SetWordWrapped(Feature::HasPlaceholders());
		pm.PushLine(ToString(lcf::Data::terms.victory) + Player::escape_symbol + "|");

		std::stringstream ss;
		if (exp > 0) {
			PushExperienceGainedMessage(pm, exp);
		}
		if (money > 0) {
			PushGoldReceivedMessage(pm, money);
		}
		PushItemRecievedMessages(pm, drops);

		Main_Data::game_system->BgmPlay(Main_Data::game_system->GetSystemBGM(Main_Data::game_system->BGM_Victory));

		// Update attributes
		std::vector<Game_Battler*> ally_battlers;
		Main_Data::game_party->GetActiveBattlers(ally_battlers);

		pm.PushPageEnd();

		for (auto& ally: ally_battlers) {
			Game_Actor* actor = static_cast<Game_Actor*>(ally);
			actor->ChangeExp(actor->GetExp() + exp, &pm);
		}
		Main_Data::game_party->GainGold(money);
		for (auto& item: drops) {
			Main_Data::game_party->AddItem(item, 1);
		}

		Game_Message::SetPendingMessage(std::move(pm));

		SetSceneActionSubState(eEnd);
		return SceneActionReturn::eContinueThisFrame;
	}

	EndBattle(BattleResult::Victory);
	return SceneActionReturn::eWaitTillNextFrame;
}

Scene_Battle_Rpg2k::SceneActionReturn Scene_Battle_Rpg2k::ProcessSceneActionDefeat() {
	enum SubState {
		eBegin = 0,
		eEnd = 1,
	};

	if (scene_action_substate == eBegin) {
		ResetWindows(true);
		battle_message_window->Clear();
		battle_message_window->SetVisible(true);

		Main_Data::game_system->SetMessagePositionFixed(true);
		Main_Data::game_system->SetMessagePosition(2);
		Main_Data::game_system->SetMessageTransparent(false);

		auto pm = PendingMessage();
		pm.SetEnableFace(false);

		pm.SetWordWrapped(Feature::HasPlaceholders());

		pm.PushLine(ToString(lcf::Data::terms.defeat));

		Main_Data::game_system->BgmPlay(Main_Data::game_system->GetSystemBGM(Main_Data::game_system->BGM_GameOver));

		Game_Message::SetPendingMessage(std::move(pm));
		SetSceneActionSubState(eEnd);

		return SceneActionReturn::eContinueThisFrame;
	}

	EndBattle(BattleResult::Defeat);
	return SceneActionReturn::eWaitTillNextFrame;
}

Scene_Battle_Rpg2k::SceneActionReturn Scene_Battle_Rpg2k::ProcessSceneActionEscape() {
	enum SubState {
		eBegin = 0,
		eSuccess = 1,
		eFailure = 2,
	};

	if (scene_action_substate == eBegin) {
		ResetWindows(true);
		battle_message_window->Clear();
		battle_message_window->SetVisible(true);

		auto next_ss = TryEscape() ? eSuccess : eFailure;

		if (next_ss == eSuccess) {
			battle_message_window->Push(lcf::Data::terms.escape_success);
		} else {
			battle_message_window->Push(lcf::Data::terms.escape_failure);
		}
		SetWait(10, 60);
		SetSceneActionSubState(next_ss);
		return SceneActionReturn::eContinueThisFrame;
	}

	if (scene_action_substate == eSuccess) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Escape));

		EndBattle(BattleResult::Escape);
		return SceneActionReturn::eContinueThisFrame;
	}

	if (scene_action_substate == eFailure) {
		SetState(State_Battle);
		NextTurn();

		CreateEnemyActions();
		CreateExecutionOrder();
		return SceneActionReturn::eContinueThisFrame;
	}
	return SceneActionReturn::eWaitTillNextFrame;
}

void Scene_Battle_Rpg2k::SetBattleActionState(BattleActionState state) {
	battle_action_state = state;
	SetBattleActionSubState(0);
}

void Scene_Battle_Rpg2k::SetBattleActionSubState(int substate, bool reset_index) {
	battle_action_substate = substate;
	if (reset_index) {
		battle_action_substate_index = 0;
	}
}

Scene_Battle_Rpg2k::BattleActionReturn Scene_Battle_Rpg2k::ProcessBattleAction(Game_BattleAlgorithm::AlgorithmBase* action) {
	if (action == nullptr) {
		Output::Warning("ProcessBattleAction: Invalid battle action");
		Output::Warning("Please report a bug!");
		return BattleActionReturn::eFinished;
	}

#ifdef EP_DEBUG_BATTLE2K_STATE_MACHINE
	static int last_state = -1;
	static int last_substate = -1;
	static int last_substate_index = -1;
	if (battle_action_state != last_state || battle_action_substate != last_substate || battle_action_substate_index != last_substate_index) {
		Output::Debug("Battle2k ProcessBattleAction({}, {},{},{}) frames={}", action->GetSource()->GetName(), battle_action_state, battle_action_substate, battle_action_substate_index, Main_Data::game_system->GetFrameCounter());
		last_state = battle_action_state;
		last_substate = battle_action_substate;
		last_substate_index = battle_action_substate_index;
	}
#endif

	switch (battle_action_state) {
		case BattleActionState_Begin:
			return ProcessBattleActionBegin(action);
		case BattleActionState_Usage:
			return ProcessBattleActionUsage(action);
		case BattleActionState_Animation:
			return ProcessBattleActionAnimation(action);
		case BattleActionState_AnimationReflect:
			return ProcessBattleActionAnimationReflect(action);
		case BattleActionState_Execute:
			return ProcessBattleActionExecute(action);
		case BattleActionState_Critical:
			return ProcessBattleActionCritical(action);
		case BattleActionState_Apply:
			return ProcessBattleActionApply(action);
		case BattleActionState_Failure:
			return ProcessBattleActionFailure(action);
		case BattleActionState_Damage:
			return ProcessBattleActionDamage(action);
		case BattleActionState_Params:
			return ProcessBattleActionParamEffects(action);
		case BattleActionState_States:
			return ProcessBattleActionStateEffects(action);
		case BattleActionState_Attributes:
			return ProcessBattleActionAttributeEffects(action);
		case BattleActionState_Finished:
			return ProcessBattleActionFinished(action);
	}

	assert(false && "Invalid BattleActionState!");

	return BattleActionReturn::eFinished;
}

Scene_Battle_Rpg2k::BattleActionReturn Scene_Battle_Rpg2k::ProcessBattleActionBegin(Game_BattleAlgorithm::AlgorithmBase* action) {
	enum SubState {
		eBegin = 0,
		eShowMessage,
		ePost,
	};

	auto* src = action->GetSource();

	if (battle_action_substate == eBegin) {
		assert(src->Exists());
		battle_message_window->Clear();

		bool show_message = false;
		src->NextBattleTurn();

		std::vector<int16_t> states_to_heal = src->BattleStateHeal();
		src->ApplyConditions();

		const lcf::rpg::State* pri_state = nullptr;
		bool pri_was_healed = false;
		for (size_t id = 1; id <= lcf::Data::states.size(); ++id) {
			auto was_healed = std::find(states_to_heal.begin(), states_to_heal.end(), id) != states_to_heal.end();
			if (!was_healed && !src->HasState(id)) {
				continue;
			}

			auto* state = lcf::ReaderUtil::GetElement(lcf::Data::states, id);
			if (!pri_state || state->priority >= pri_state->priority) {
				pri_state = state;
				pri_was_healed = was_healed;
			}
		}

		if (pri_state != nullptr) {
			StringView msg = pri_was_healed
				? pri_state->message_recovery
				: pri_state->message_affected;

			// RPG_RT behavior:
			// If state was healed, always prints.
			// If state is inflicted, only prints if msg not empty.
			if (pri_was_healed || !msg.empty()) {
				show_message = true;
				pending_message = ToString(msg);
			}
		}

		if (action->GetType() != Game_BattleAlgorithm::Type::None || show_message) {
			action->GetSource()->Flash(31, 31, 31, 10, 10);
		}

		if (show_message) {
			SetWait(4,4);
			SetBattleActionSubState(eShowMessage);
			return BattleActionReturn::eContinue;
		}
		battle_action_substate = ePost;
	}

	if (battle_action_substate == eShowMessage) {
		battle_message_window->PushWithSubject(std::move(pending_message), action->GetSource()->GetName());
		SetWait(20, 60);
		pending_message.clear();
		SetBattleActionSubState(ePost);
		return BattleActionReturn::eContinue;
	}

	if (battle_action_substate == ePost) {
		battle_message_window->Clear();

		if (action->GetType() == Game_BattleAlgorithm::Type::None) {
			SetBattleActionState(BattleActionState_Finished);
			return BattleActionReturn::eContinue;
		}

		SetWait(4,4);
	}

	SetBattleActionState(BattleActionState_Usage);
	return BattleActionReturn::eContinue;
}


Scene_Battle_Rpg2k::BattleActionReturn Scene_Battle_Rpg2k::ProcessBattleActionUsage(Game_BattleAlgorithm::AlgorithmBase* action) {
	enum SubState {
		eBegin = 0,
		eMessages,
		eLastMessage,
	};

	if (battle_action_substate == eBegin) {
		action->Start();
		battle_message_window->Clear();

		pending_message = action->GetStartMessage(0);

		SetBattleActionSubState(eMessages);
	}

	if (battle_action_substate == eMessages) {
		if (!pending_message.empty()) {
			battle_message_window->Push(std::move(pending_message));
			battle_message_window->ScrollToEnd();

			pending_message = action->GetStartMessage(++battle_action_substate_index);

			if (!pending_message.empty()) {
				SetWaitForUsage(action->GetType(), 0);
				return BattleActionReturn::eContinue;
			}
		}

		SetBattleActionSubState(eLastMessage);
	}

	if (battle_action_substate == eLastMessage) {
		battle_action_start_index = battle_message_window->GetLineCount();

		auto* se = action->GetStartSe();
		if (se) {
			Main_Data::game_system->SePlay(*se);
		}
	}

	SetBattleActionState(BattleActionState_Animation);
	return BattleActionReturn::eContinue;
}

Scene_Battle_Rpg2k::BattleActionReturn Scene_Battle_Rpg2k::ProcessBattleActionAnimation(Game_BattleAlgorithm::AlgorithmBase* action) {
	return ProcessBattleActionAnimationImpl(action, false);
}

Scene_Battle_Rpg2k::BattleActionReturn Scene_Battle_Rpg2k::ProcessBattleActionAnimationReflect(Game_BattleAlgorithm::AlgorithmBase* action) {
	return ProcessBattleActionAnimationImpl(action, true);
}

Scene_Battle_Rpg2k::BattleActionReturn Scene_Battle_Rpg2k::ProcessBattleActionAnimationImpl(Game_BattleAlgorithm::AlgorithmBase* action, bool reflect) {
	int frames = 0;
	while(1) {
		const int cur_anim = action->GetAnimationId(battle_action_substate_index);
		++battle_action_substate_index;
		int next_anim = 0;

		if (cur_anim) {
			if (action->GetTarget()->GetType() == Game_Battler::Type_Enemy) {
				frames = action->PlayAnimation(cur_anim);
			} else {
				frames = action->PlayAnimation(cur_anim, true, 40);
			}
			next_anim = action->GetAnimationId(battle_action_substate_index);
		}

		if (!next_anim) {
			break;
		}

		if (frames) {
			SetWait(frames, frames);
			return BattleActionReturn::eContinue;
		}
	}

	if (!reflect) {
		// Wait for last start message and last animation.
		SetWaitForUsage(action->GetType(), frames);

		// EasyRPG extension: Support 2k3 reflect feature in 2k battle system.
		if (action->ReflectTargets()) {
			SetBattleActionState(BattleActionState_AnimationReflect);
			return BattleActionReturn::eContinue;
		}
	} else {
		// Wait for reflected animation - no message.
		SetWait(frames, frames);
	}

	SetBattleActionState(BattleActionState_Execute);
	return BattleActionReturn::eContinue;
}

Scene_Battle_Rpg2k::BattleActionReturn Scene_Battle_Rpg2k::ProcessBattleActionExecute(Game_BattleAlgorithm::AlgorithmBase* action) {
	action->Execute();
	if (action->GetType() == Game_BattleAlgorithm::Type::Normal
			|| action->GetType() == Game_BattleAlgorithm::Type::Skill
			|| action->GetType() == Game_BattleAlgorithm::Type::SelfDestruct) {
		if (action->GetType() != Game_BattleAlgorithm::Type::Skill) {
			SetWait(4,4);
		}
		if (action->IsSuccess() && action->IsCriticalHit()) {
			SetBattleActionState(BattleActionState_Critical);
			return BattleActionReturn::eContinue;
		}
	}
	SetBattleActionState(BattleActionState_Apply);
	return BattleActionReturn::eContinue;
}

Scene_Battle_Rpg2k::BattleActionReturn Scene_Battle_Rpg2k::ProcessBattleActionCritical(Game_BattleAlgorithm::AlgorithmBase* action) {
	battle_message_window->Push(BattleMessage::GetCriticalHitMessage(*action->GetSource(), *action->GetTarget()));
	battle_message_window->ScrollToEnd();
	SetWait(10, 30);

	SetBattleActionState(BattleActionState_Apply);
	return BattleActionReturn::eContinue;
}

Scene_Battle_Rpg2k::BattleActionReturn Scene_Battle_Rpg2k::ProcessBattleActionApply(Game_BattleAlgorithm::AlgorithmBase* action) {
	action->ApplyCustomEffect();
	action->ApplySwitchEffect();

	battle_action_results_index = battle_message_window->GetLineCount();

	if (!action->IsSuccess()) {
		SetBattleActionState(BattleActionState_Failure);
		return BattleActionReturn::eContinue;
	}

	auto* target = action->GetTarget();

	if (!target) {
		SetBattleActionState(BattleActionState_Finished);
		return BattleActionReturn::eContinue;
	}

	SetBattleActionState(BattleActionState_Damage);
	return BattleActionReturn::eContinue;
}

Scene_Battle_Rpg2k::BattleActionReturn Scene_Battle_Rpg2k::ProcessBattleActionFailure(Game_BattleAlgorithm::AlgorithmBase* action) {
	enum SubState {
		eBegin = 0,
		eProcess,
	};

	if (battle_action_substate == eBegin) {
		SetWait(4,4);
		SetBattleActionSubState(eProcess);
		return BattleActionReturn::eContinue;
	}

	auto* se = action->GetFailureSe();
	if (se) {
		Main_Data::game_system->SePlay(*se);
	}

	const auto& fail_msg = action->GetFailureMessage();
	battle_message_window->Push(fail_msg);
	battle_message_window->ScrollToEnd();
	SetWait(20, 60);

	SetBattleActionState(BattleActionState_Finished);
	return BattleActionReturn::eContinue;
}

Scene_Battle_Rpg2k::BattleActionReturn Scene_Battle_Rpg2k::ProcessBattleActionDamage(Game_BattleAlgorithm::AlgorithmBase* action) {
	enum SubState {
		eBegin = 0,
		eMessage,
		eApply,
		ePreStates,
		eStates,
		ePost,
	};

	if (battle_action_substate == eBegin) {
		if (!action->IsAffectHp() || action->GetAffectedHp() > 0 || ((action->IsPositive() || action->IsAbsorbHp()) && action->GetAffectedHp() == 0)) {
			SetBattleActionState(BattleActionState_Params);
			return BattleActionReturn::eContinue;
		}

		SetWait(4,4);
		SetBattleActionSubState(eMessage);
		return BattleActionReturn::eContinue;
	}

	if (battle_action_substate == eMessage) {
		auto* target = action->GetTarget();
		assert(target);
		auto dmg = action->GetAffectedHp();

		if (!action->IsAbsorbHp()) {
			if (target->GetType() == Game_Battler::Type_Ally) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_AllyDamage));
				if (dmg < 0) {
					Main_Data::game_screen->ShakeOnce(3, 5, 8);
				}
			} else {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_EnemyDamage));
			}
			if (target->GetType() == Game_Battler::Type_Enemy) {
				static_cast<Game_Enemy*>(target)->SetBlinkTimer();
			}
		}

		std::string msg;
		if (action->IsAbsorbHp()) {
			msg = BattleMessage::GetHpAbsorbedMessage(*action->GetTarget(), *target, -dmg);
		} else {
			if (dmg == 0) {
				msg = BattleMessage::GetUndamagedMessage(*target);
			} else {
				msg = BattleMessage::GetDamagedMessage(*target, -dmg);
			}
		}

		battle_message_window->Push(msg);
		battle_message_window->ScrollToEnd();
		if (action->IsAbsorbHp()) {
			SetWait(20, 60);
		} else {
			SetWait(20, 40);
		}

		SetBattleActionSubState(eApply);
		return BattleActionReturn::eContinue;
	}

	if (battle_action_substate == eApply) {
		// Hp damage is delayed until after the message, so that the target death animation
		// occurs at the right time.
		action->ApplyHpEffect();

		auto* target = action->GetTarget();
		assert(target);
		if (target->IsDead()) {
			ProcessBattleActionDeath(action);
		}

		battle_action_dmg_index = battle_message_window->GetLineCount();

		SetBattleActionSubState(ePreStates);
		return BattleActionReturn::eContinue;
	}

	if (battle_action_substate == ePreStates) {
		auto* target = action->GetTarget();
		const auto& states = action->GetStateEffects();
		auto& idx = battle_action_substate_index;
		for (;idx < static_cast<int>(states.size()); ++idx) {
			auto& se = states[idx];
			auto* state = lcf::ReaderUtil::GetElement(lcf::Data::states, se.state_id);
			if (!state || se.effect != Game_BattleAlgorithm::StateEffect::HealedByAttack) {
				continue;
			}
			action->ApplyStateEffect(se);
			pending_message = BattleMessage::GetStateRecoveryMessage(*target, *state);
			++battle_action_substate_index;

			battle_message_window->PopUntil(battle_action_dmg_index);
			battle_message_window->ScrollToEnd();
			SetWait(4,4);

			SetBattleActionSubState(eStates, false);
			return BattleActionReturn::eContinue;
		}
		SetBattleActionSubState(ePost);
		return BattleActionReturn::eContinue;
	}

	if (battle_action_substate == eStates) {
		battle_message_window->Push(pending_message);
		battle_message_window->ScrollToEnd();
		SetWait(20, 40);

		SetBattleActionSubState(ePreStates, false);
		return BattleActionReturn::eContinue;
	}

	if (battle_action_substate == ePost) {
		SetWait(0, 10);
	}

	SetBattleActionState(BattleActionState_Params);
	return BattleActionReturn::eContinue;
}

Scene_Battle_Rpg2k::BattleActionReturn Scene_Battle_Rpg2k::ProcessBattleActionParamEffects(Game_BattleAlgorithm::AlgorithmBase* action) {
	enum SubState {
		ePreHp,
		eHp,
		ePreSp,
		eSp,
		ePreAtk,
		eAtk,
		ePreDef,
		eDef,
		ePreSpi,
		eSpi,
		ePreAgi,
		eAgi,
		eDone
	};

	const auto next_state = BattleActionState_States;
	auto* source = action->GetSource();
	auto* target = action->GetTarget();

	// All of the "Pre" states are even numbers, so catch all Pre here.
	if ((battle_action_substate & 1) == 0) {
		pending_message.clear();

		auto checkNext = [&]() {
			if (pending_message.empty()) {
				SetBattleActionSubState(battle_action_substate + 2);
			}
		};

		if (battle_action_substate == ePreHp) {
			// Damage is handled by Damage state, so only check healing here.
			if (action->GetAffectedHp() > 0 && !action->IsRevived()) {
				auto hp = action->ApplyHpEffect();
				pending_message = BattleMessage::GetHpRecoveredMessage(*target, hp);
			}
			checkNext();
		}

		if (battle_action_substate == ePreSp) {
			auto sp = action->ApplySpEffect();
			if (action->IsAbsorbSp()) {
				pending_message = BattleMessage::GetSpAbsorbedMessage(*source, *target, -sp);
			} else {
				if (sp > 0) {
					pending_message = BattleMessage::GetSpRecoveredMessage(*target, sp);
				}
				if (sp < 0) {
					pending_message = BattleMessage::GetSpReduceMessage(*target, -sp);
				}
			}
			checkNext();
		}

		if (battle_action_substate == ePreAtk) {
			auto atk = action->ApplyAtkEffect();
			if (atk != 0) {
				if (action->IsAbsorbAtk()) {
					pending_message = BattleMessage::GetAtkAbsorbedMessage(*source, *target, -atk);
				} else {
					pending_message = BattleMessage::GetAtkChangeMessage(*target, atk);
				}
			}
			checkNext();
		}

		if (battle_action_substate == ePreDef) {
			auto def = action->ApplyDefEffect();
			if (def != 0) {
				if (action->IsAbsorbDef()) {
					pending_message = BattleMessage::GetDefAbsorbedMessage(*source, *target, -def);
				} else {
					pending_message = BattleMessage::GetDefChangeMessage(*target, def);
				}
			}
			checkNext();
		}

		if (battle_action_substate == ePreSpi) {
			auto spi = action->ApplySpiEffect();
			if (spi != 0) {
				if (action->IsAbsorbSpi()) {
					pending_message = BattleMessage::GetSpiAbsorbedMessage(*source, *target, -spi);
				} else {
					pending_message = BattleMessage::GetSpiChangeMessage(*target, spi);
				}
			}
			checkNext();
		}

		if (battle_action_substate == ePreAgi) {
			auto agi = action->ApplyAgiEffect();
			if (agi != 0) {
				if (action->IsAbsorbAgi()) {
					pending_message = BattleMessage::GetAgiAbsorbedMessage(*source, *target, -agi);
				} else {
					pending_message = BattleMessage::GetAgiChangeMessage(*target, agi);
				}
			}
			checkNext();
		}

		if (!pending_message.empty()) {
			battle_message_window->PopUntil(battle_action_results_index);
			battle_message_window->ScrollToEnd();
			SetWait(4,4);

			SetBattleActionSubState(battle_action_substate + 1, false);
			return BattleActionReturn::eContinue;
		}
	}

	// Use >= here so that the each "pre" stage above can just call
	// checkNext() to increment +2 without worrying about overflowing
	// past eDone.
	if (battle_action_substate >= eDone) {
		SetBattleActionState(next_state);
		return BattleActionReturn::eContinue;
	}

	// All of the normal states are odd numbers.
	if ((battle_action_substate & 1) != 0) {
		battle_message_window->Push(pending_message);
		battle_message_window->ScrollToEnd();
		SetWait(20, 60);

		SetBattleActionSubState(battle_action_substate + 1);
		return BattleActionReturn::eContinue;
	}

	SetBattleActionState(next_state);
	return BattleActionReturn::eContinue;
}

Scene_Battle_Rpg2k::BattleActionReturn Scene_Battle_Rpg2k::ProcessBattleActionStateEffects(Game_BattleAlgorithm::AlgorithmBase* action) {
	enum SubState {
		eApply,
		ePreWait,
		eMessage,
		eDone
	};

	const auto next_state = BattleActionState_Attributes;

	auto* target = action->GetTarget();

	if (battle_action_substate == eApply) {
		pending_message.clear();

		const auto was_dead = target->IsDead();
		const auto& states = action->GetStateEffects();
		auto& idx = battle_action_substate_index;

		if (idx >= static_cast<int>(states.size())) {
			SetBattleActionState(next_state);
			return BattleActionReturn::eContinue;
		}

		for (;idx < (int)states.size(); ++idx) {
			auto& se = states[idx];
			// Already applied earlier after damage
			if (se.effect == Game_BattleAlgorithm::StateEffect::HealedByAttack) {
				++idx;
				return BattleActionReturn::eContinue;
			}

			auto* state = lcf::ReaderUtil::GetElement(lcf::Data::states, se.state_id);
			if (!state) {
				continue;
			}

			action->ApplyStateEffect(se);
			switch (se.effect) {
				case Game_BattleAlgorithm::StateEffect::Inflicted:
					pending_message = BattleMessage::GetStateInflictMessage(*target, *state);
					break;
				case Game_BattleAlgorithm::StateEffect::Healed:
					pending_message = BattleMessage::GetStateRecoveryMessage(*target, *state);
					break;
				case Game_BattleAlgorithm::StateEffect::AlreadyInflicted:
					pending_message = BattleMessage::GetStateAlreadyMessage(*target, *state);
					break;
				default:
					break;
			}

			if ((!was_dead && target->IsDead()) || !pending_message.empty()) {
				break;
			}
		}

		battle_message_window->PopUntil(battle_action_results_index);
		battle_message_window->ScrollToEnd();

		// If we were killed by state
		if (!was_dead && target->IsDead()) {
			ProcessBattleActionDeath(action);
			SetBattleActionState(next_state);
			// FIXES an RPG_RT bug where RPG_RT does an extra SetWait(4,4), SetWait(20,60) on death state infliction
			return BattleActionReturn::eContinue;
		}
		SetBattleActionSubState(ePreWait, false);
		return BattleActionReturn::eContinue;
	}

	if (battle_action_substate == ePreWait) {
		SetWait(4,4);
		SetBattleActionSubState(eMessage, false);
		return BattleActionReturn::eContinue;
	}

	if (battle_action_substate == eMessage) {
		battle_message_window->Push(pending_message);
		battle_message_window->ScrollToEnd();
		SetWait(20, 60);

		// Process the next state
		++battle_action_substate_index;
		SetBattleActionSubState(eApply, false);
		return BattleActionReturn::eContinue;
	}

	SetBattleActionState(next_state);
	return BattleActionReturn::eContinue;
}

Scene_Battle_Rpg2k::BattleActionReturn Scene_Battle_Rpg2k::ProcessBattleActionAttributeEffects(Game_BattleAlgorithm::AlgorithmBase* action) {
	enum SubState {
		eApply,
		eMessage,
	};

	const auto next_state = BattleActionState_Finished;

	// All of the "Pre" states are even numbers, so catch all Pre here.
	if (battle_action_substate == eApply) {
		pending_message.clear();

		const auto& attrs = action->GetShiftedAttributes();
		auto& idx = battle_action_substate_index;
		if (idx >= static_cast<int>(attrs.size())) {
			SetBattleActionState(next_state);
			return BattleActionReturn::eContinue;
		}

		for (;idx < (int)attrs.size(); ++idx) {
			auto& ae = attrs[battle_action_substate_index];
			auto shifted = action->ApplyAttributeShiftEffect(ae);
			if (shifted != 0) {
				auto* attr = lcf::ReaderUtil::GetElement(lcf::Data::attributes, ae.attr_id);
				pending_message = BattleMessage::GetAttributeShiftMessage(*action->GetTarget(), shifted, *attr);
				break;
			}
		}

		battle_message_window->PopUntil(battle_action_results_index);
		battle_message_window->ScrollToEnd();
		SetWait(4,4);

		SetBattleActionSubState(eMessage, false);
		return BattleActionReturn::eContinue;
	}

	// All of the normal states are odd numbers.
	if (battle_action_substate == eMessage) {
		battle_message_window->Push(pending_message);
		battle_message_window->ScrollToEnd();
		SetWait(20, 60);

		++battle_action_substate_index;
		SetBattleActionSubState(eApply, false);
		return BattleActionReturn::eContinue;
	}

	SetBattleActionState(next_state);
	return BattleActionReturn::eContinue;
}

void Scene_Battle_Rpg2k::ProcessBattleActionDeath(Game_BattleAlgorithm::AlgorithmBase* action) {
	auto* target = action->GetTarget();
	assert(target);

	battle_message_window->Push(BattleMessage::GetDeathMessage(*action->GetTarget()));
	battle_message_window->ScrollToEnd();
	SetWait(36, 60);

	if (target->GetType() == Game_Battler::Type_Enemy) {
		static_cast<Game_Enemy*>(target)->SetDeathTimer();
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_EnemyKill));
	}
}

Scene_Battle_Rpg2k::BattleActionReturn Scene_Battle_Rpg2k::ProcessBattleActionFinished(Game_BattleAlgorithm::AlgorithmBase* action) {
	if (action->RepeatNext(true) || action->TargetNext()) {
		// Clear the console for the next target
		battle_message_window->PopUntil(battle_action_start_index);
		battle_message_window->ScrollToEnd();

		SetBattleActionState(BattleActionState_Execute);
		return BattleActionReturn::eContinue;
	}

	battle_message_window->Clear();
	action->ProcessPostActionSwitches();
	return BattleActionReturn::eFinished;
}

void Scene_Battle_Rpg2k::SelectNextActor(bool auto_battle) {
	std::vector<Game_Actor*> allies = Main_Data::game_party->GetActors();

	if ((size_t)actor_index == allies.size()) {
		// All actor actions decided, player turn ends
		SetState(State_Battle);
		NextTurn();

		CreateEnemyActions();
		CreateExecutionOrder();

		return;
	}

	active_actor = allies[actor_index];
	status_window->SetIndex(actor_index);
	actor_index++;

	Game_Battler* random_target = NULL;

	if (!active_actor->CanAct()) {
		active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::None>(active_actor));
		battle_actions.push_back(active_actor);
		SelectNextActor(auto_battle);
		return;
	}

	switch (active_actor->GetSignificantRestriction()) {
		case lcf::rpg::State::Restriction_attack_ally:
			random_target = Main_Data::game_party->GetRandomActiveBattler();
			break;
		case lcf::rpg::State::Restriction_attack_enemy:
			random_target = Main_Data::game_enemyparty->GetRandomActiveBattler();
			break;
		default:
			break;
	}

	if (random_target) {
		// RPG_RT doesn't support "Attack All" weapons when battler is confused or provoked.
		active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(active_actor, random_target));
		battle_actions.push_back(active_actor);

		SelectNextActor(auto_battle);
		return;
	}

	if (auto_battle || active_actor->GetAutoBattle()) {
		if (active_actor->GetActorAi() == -1) {
			this->autobattle_algos[default_autobattle_algo]->SetAutoBattleAction(*active_actor);
		} else {
			this->autobattle_algos[active_actor->GetActorAi()]->SetAutoBattleAction(*active_actor);
		}
		assert(active_actor->GetBattleAlgorithm() != nullptr);
		battle_actions.push_back(active_actor);

		SelectNextActor(auto_battle);
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

	battle_actions.back()->SetBattleAlgorithm(nullptr);
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
		int battle_order = battler->GetAgi() + Rand::GetRandomNumber(0, battler->GetAgi() / 4 + 3);
		if (battler->GetBattleAlgorithm()->GetType() == Game_BattleAlgorithm::Type::Normal && battler->HasPreemptiveAttack()) {
			// RPG_RT sets this value
			battle_order += 9999;
		}
		battler->SetBattleOrderAgi(battle_order);
	}
	std::sort(battle_actions.begin(), battle_actions.end(),
			[](Game_Battler* l, Game_Battler* r) {
			return l->GetBattleOrderAgi() > r->GetBattleOrderAgi();
			});

	for (const auto& battler : battle_actions) {
		if (std::count(battle_actions.begin(), battle_actions.end(), battler) > 1) {
			Output::Warning("CreateExecutionOrder: Battler {} ({}) has multiple battle actions", battler->GetId(), battler->GetName());
			Output::Warning("Please report a bug!");
			break;
		}
	}
}

void Scene_Battle_Rpg2k::CreateEnemyActions() {
	if (first_strike) {
		return;
	}

	for (auto* enemy : Main_Data::game_enemyparty->GetEnemies()) {
		if (!EnemyAi::SetStateRestrictedAction(*enemy)) {
			if (enemy->GetEnemyAi() == -1) {
				enemyai_algos[default_enemyai_algo]->SetEnemyAiAction(*enemy);
			} else {
				enemyai_algos[enemy->GetEnemyAi()]->SetEnemyAiAction(*enemy);
			}
		}
		assert(enemy->GetBattleAlgorithm() != nullptr);
		ActionSelectedCallback(enemy);
	}
}

void Scene_Battle_Rpg2k::SubskillSelected2k3(int command) {
	auto idx = command - 1;
	// Resolving a subskill battle command to skill id
	int subskill = lcf::rpg::Skill::Type_subskill;

	// Loop through all battle commands smaller then that ID and count subsets
	for (int i = 0; i < static_cast<int>(lcf::Data::battlecommands.commands.size()); ++i) {
		auto& cmd = lcf::Data::battlecommands.commands[i];
		if (i >= idx) {
			break;
		}
		if (cmd.type == lcf::rpg::BattleCommand::Type_subskill) {
			++subskill;
		}
	}

	// skill subset is 4 (Type_subskill) + counted subsets
	skill_window->SetSubsetFilter(subskill);
	SetState(State_SelectSkill);
}

void Scene_Battle_Rpg2k::SpecialSelected2k3() {
	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));

	active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::DoNothing>(active_actor));

	ActionSelectedCallback(active_actor);
}

void Scene_Battle_Rpg2k::ActionSelectedCallback(Game_Battler* for_battler) {
	Scene_Battle::ActionSelectedCallback(for_battler);

	if (for_battler->GetType() == Game_Battler::Type_Ally) {
		SetState(State_SelectActor);
	}
}

void Scene_Battle_Rpg2k::SetWait(int min_wait, int max_wait) {
#if defined(EP_DEBUG_BATTLE2K_MESSAGE) || defined(EP_DEBUG_BATTLE2K_STATE_MACHINE)
	Output::Debug("Battle2k Wait({},{}) frame={}", min_wait, max_wait, Main_Data::game_system->GetFrameCounter());
#endif
	battle_action_wait = max_wait;
	battle_action_min_wait = max_wait - min_wait;
}

void Scene_Battle_Rpg2k::SetWaitForUsage(Game_BattleAlgorithm::Type type, int anim_frames) {
	int min_wait = 0;
	int max_wait = 0;
	switch (type) {
		case Game_BattleAlgorithm::Type::Normal:
			min_wait = 20;
			max_wait = 40;
			break;
		case Game_BattleAlgorithm::Type::Escape:
			min_wait = 36;
			max_wait = 60;
			break;
		case Game_BattleAlgorithm::Type::None:
		case Game_BattleAlgorithm::Type::DoNothing:
			min_wait = max_wait = 0;
			break;
		default:
			min_wait = 20;
			max_wait = 60;
			break;
	}
	SetWait(std::max(min_wait, anim_frames), std::max(max_wait, anim_frames));
}

bool Scene_Battle_Rpg2k::CheckWait() {
	if (battle_action_wait > 0) {
		if (Input::IsPressed(Input::CANCEL)) {
			return false;
		}
		--battle_action_wait;
		if (battle_action_wait > battle_action_min_wait) {
			return false;
		}
		if (!Input::IsPressed(Input::DECISION)
			&& !Input::IsPressed(Input::SHIFT)
			&& battle_action_wait > 0) {
			return false;
		}
		battle_action_wait = 0;
	}
	return true;
}

void Scene_Battle_Rpg2k::PushExperienceGainedMessage(PendingMessage& pm, int exp) {
	if (Feature::HasPlaceholders()) {
		pm.PushLine(
			Utils::ReplacePlaceholders(
				lcf::Data::terms.exp_received,
				Utils::MakeArray('V', 'U'),
				Utils::MakeSvArray(std::to_string(exp), lcf::Data::terms.exp_short)
			) + Player::escape_symbol + "."
		);
	}
	else {
		std::stringstream ss;
		ss << exp << lcf::Data::terms.exp_received << Player::escape_symbol << ".";
		pm.PushLine(ss.str());
	}
}

void Scene_Battle_Rpg2k::PushGoldReceivedMessage(PendingMessage& pm, int money) {

	if (Feature::HasPlaceholders()) {
		pm.PushLine(
			Utils::ReplacePlaceholders(
				lcf::Data::terms.gold_recieved_a,
				Utils::MakeArray('V', 'U'),
				Utils::MakeSvArray(std::to_string(money), lcf::Data::terms.gold)
			) + Player::escape_symbol + "."
		);
	}
	else {
		std::stringstream ss;
		ss << lcf::Data::terms.gold_recieved_a << " " << money << lcf::Data::terms.gold << lcf::Data::terms.gold_recieved_b << Player::escape_symbol << ".";
		pm.PushLine(ss.str());
	}
}

void Scene_Battle_Rpg2k::PushItemRecievedMessages(PendingMessage& pm, std::vector<int> drops) {
	std::stringstream ss;

	for (std::vector<int>::iterator it = drops.begin(); it != drops.end(); ++it) {
		const lcf::rpg::Item* item = lcf::ReaderUtil::GetElement(lcf::Data::items, *it);
		// No Output::Warning needed here, reported later when the item is added
		StringView item_name = item ? StringView(item->name) : StringView("??? BAD ITEM ???");

		if (Feature::HasPlaceholders()) {
			pm.PushLine(
				Utils::ReplacePlaceholders(
					lcf::Data::terms.item_recieved,
					Utils::MakeArray('S'),
					Utils::MakeSvArray(item_name)
				) + Player::escape_symbol + "."
			);
		}
		else {
			ss.str("");
			ss << item_name << lcf::Data::terms.item_recieved << Player::escape_symbol << ".";
			pm.PushLine(ss.str());
		}
	}
}

bool Scene_Battle_Rpg2k::CheckBattleEndConditions() {
	if (state == State_Defeat || Game_Battle::CheckLose()) {
		if (state != State_Defeat) {
			SetState(State_Defeat);
		}
		return true;
	}

	if (state == State_Victory || Game_Battle::CheckWin()) {
		if (state != State_Victory) {
			SetState(State_Victory);
		}
		return true;
	}

	return false;
}

