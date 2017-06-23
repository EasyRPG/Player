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

#include "scene_battle_rpg2k3.h"
#include "rpg_battlecommand.h"
#include "input.h"
#include "output.h"
#include "player.h"
#include "sprite.h"
#include "cache.h"
#include "game_system.h"
#include "game_temp.h"
#include "game_party.h"
#include "game_enemy.h"
#include "game_enemyparty.h"
#include "game_message.h"
#include "game_battle.h"
#include "game_battlealgorithm.h"
#include "scene_gameover.h"
#include "utils.h"
#include "font.h"

Scene_Battle_Rpg2k3::Scene_Battle_Rpg2k3() : Scene_Battle(),
	battle_action_wait(30),
	battle_action_state(BattleActionState_Start)
{
}

Scene_Battle_Rpg2k3::~Scene_Battle_Rpg2k3() {
}

void Scene_Battle_Rpg2k3::Update() {
	switch (state) {
		case State_SelectActor:
		case State_AutoBattle: {
			if (battle_actions.empty()) {
				Game_Battle::UpdateGauges();
			}

			int old_state = state;
			SelectNextActor();

			if (old_state == state && battle_actions.empty()) {
				// No actor got the turn
				std::vector<Game_Battler*> enemies;
				Main_Data::game_enemyparty->GetActiveBattlers(enemies);

				for (std::vector<Game_Battler*>::iterator it = enemies.begin();
					it != enemies.end(); ++it) {
					if ((*it)->IsGaugeFull() && !(*it)->GetBattleAlgorithm()) {
						Game_Enemy* enemy = static_cast<Game_Enemy*>(*it);
						const RPG::EnemyAction* action = enemy->ChooseRandomAction();
						if (action) {
							CreateEnemyAction(enemy, action);
						}
					}
				}
			}

			break;
		}
		default:;
	}

	for (std::vector<FloatText>::iterator it = floating_texts.begin();
		it != floating_texts.end();) {
		int &time = (*it).remaining_time;

		if (time % 2 == 0) {
			int modifier = time <= 10 ? 1 :
						   time < 20 ? 0 :
						   -1;
			(*it).sprite->SetY((*it).sprite->GetY() + modifier);
		}

		--time;
		if (time <= 0) {
			it = floating_texts.erase(it);
		}
		else {
			++it;
		}
	}

	Scene_Battle::Update();
	UpdateCursors();

	//enemy_status_window->Update();
}

void Scene_Battle_Rpg2k3::OnSystem2Ready(FileRequestResult* result) {
	BitmapRef system2 = Cache::System2(result->file);

	ally_cursor->SetBitmap(system2);
	ally_cursor->SetZ(Priority_Window);
	ally_cursor->SetVisible(false);

	enemy_cursor->SetBitmap(system2);
	enemy_cursor->SetZ(Priority_Window);
	enemy_cursor->SetVisible(false);
}

void Scene_Battle_Rpg2k3::CreateUi() {
	Scene_Battle::CreateUi();

	CreateBattleTargetWindow();
	CreateBattleCommandWindow();

	// No escape. FIXME: Only enabled when party has initiative.
	options_window->DisableItem(2);

	enemy_status_window.reset(new Window_BattleStatus(0, 0, SCREEN_TARGET_WIDTH - 76, 80, true));
	enemy_status_window->SetVisible(false);

	ally_cursor.reset(new Sprite());
	enemy_cursor.reset(new Sprite());

	if (Data::battlecommands.battle_type == RPG::BattleCommands::BattleType_gauge) {
		item_window->SetY(64);
		skill_window->SetY(64);

		// Default window too small for 4 actors
		status_window.reset(new Window_BattleStatus(0, SCREEN_TARGET_HEIGHT - 80, SCREEN_TARGET_WIDTH, 80));
	}

	if (Data::battlecommands.battle_type != RPG::BattleCommands::BattleType_traditional) {
		int transp = Data::battlecommands.transparency == RPG::BattleCommands::Transparency_transparent ? 128 : 255;
		options_window->SetBackOpacity(transp);
		item_window->SetBackOpacity(transp);
		skill_window->SetBackOpacity(transp);
		help_window->SetBackOpacity(transp);
		status_window->SetBackOpacity(transp);
		enemy_status_window->SetBackOpacity(transp);
	}

	FileRequestAsync* request = AsyncHandler::RequestFile("System2", Data::system.system2_name);
	request_id = request->Bind(&Scene_Battle_Rpg2k3::OnSystem2Ready, this);
	request->Start();
}

void Scene_Battle_Rpg2k3::UpdateCursors() {
	if (state == State_SelectActor ||
		state == State_SelectCommand ||
		state == State_SelectAllyTarget ||
		state == State_SelectEnemyTarget) {

		int ally_index = status_window->GetIndex();
		int enemy_index = target_window->GetIndex();

		if (state != State_SelectEnemyTarget) {
			enemy_index = -1;
			enemy_cursor->SetVisible(false);
		}

		std::vector<Game_Battler*> actors;

		if (ally_index >= 0) {
			ally_cursor->SetVisible(true);
			Main_Data::game_party->GetBattlers(actors);
			const Game_Battler* actor = actors[ally_index];
			Sprite_Battler* sprite = Game_Battle::GetSpriteset().FindBattler(actor);
			ally_cursor->SetX(actor->GetBattleX());
			ally_cursor->SetY(actor->GetBattleY() - sprite->GetHeight() / 2);
			static const int frames[] = { 0, 1, 2, 1 };
			int frame = frames[(cycle / 15) % 4];
			ally_cursor->SetSrcRect(Rect(frame * 16, 16, 16, 16));

			if (cycle % 60 == 0) {
				sprite->Flash(Color(255, 255, 255, 100), 15);
			}
		}

		if (enemy_index >= 0) {
			enemy_cursor->SetVisible(true);
			actors.clear();
			Main_Data::game_enemyparty->GetActiveBattlers(actors);
			const Game_Battler* actor = actors[enemy_index];
			const Sprite_Battler* sprite = Game_Battle::GetSpriteset().FindBattler(actor);
			enemy_cursor->SetX(actor->GetBattleX() + sprite->GetWidth() / 2 + 2);
			enemy_cursor->SetY(actor->GetBattleY() - enemy_cursor->GetHeight() / 2);
			static const int frames[] = { 0, 1, 2, 1 };
			int frame = frames[(cycle / 15) % 4];
			enemy_cursor->SetSrcRect(Rect(frame * 16, 0, 16, 16));

			if (state == State_SelectEnemyTarget) {
				auto states = actor->GetInflictedStates();

				help_window->SetVisible(!states.empty());
				BitmapRef contents = help_window->GetContents();
				contents->Clear();

				int text_width = 0;
				for (auto state : states) {
					std::string name = Data::states[state - 1].name;
					int color = Data::states[state - 1].color;
					FontRef font = Font::Default();
					contents->TextDraw(text_width, 2, color, name, Text::AlignLeft);
					text_width += font->GetSize(name + "  ").width;
				}
			}
		}

		++cycle;
	}
	else {
		ally_cursor->SetVisible(false);
		enemy_cursor->SetVisible(false);
		cycle = 0;
	}
}

void Scene_Battle_Rpg2k3::DrawFloatText(int x, int y, int color, const std::string& text) {
	Rect rect = Font::Default()->GetSize(text);

	BitmapRef graphic = Bitmap::Create(rect.width, rect.height);
	graphic->Clear();
	graphic->TextDraw(-rect.x, -rect.y, color, text);

	std::shared_ptr<Sprite> floating_text = std::make_shared<Sprite>();
	floating_text->SetBitmap(graphic);
	floating_text->SetOx(rect.width / 2);
	floating_text->SetOy(rect.height + 5);
	floating_text->SetX(x);
	// Move 5 pixel down because the number "jumps" with the intended y as the peak
	floating_text->SetY(y + 5);
	floating_text->SetZ(Priority_Window + y);

	FloatText float_text;
	float_text.sprite = floating_text;

	floating_texts.push_back(float_text);
}

void Scene_Battle_Rpg2k3::CreateBattleTargetWindow() {
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

	if (Data::battlecommands.battle_type != RPG::BattleCommands::BattleType_traditional) {
		int transp = Data::battlecommands.transparency == RPG::BattleCommands::Transparency_transparent ? 128 : 255;
		target_window->SetBackOpacity(transp);
	}
}

void Scene_Battle_Rpg2k3::CreateBattleCommandWindow() {
	std::vector<std::string> commands;
	std::vector<int> disabled_items;

	Game_Actor* actor;

	if (!active_actor && Main_Data::game_party->GetBattlerCount() > 0) {
		actor = &(*Main_Data::game_party)[0];
	}
	else {
		actor = active_actor;
	}

	if (actor) {
		const std::vector<const RPG::BattleCommand*> bcmds = actor->GetBattleCommands();
		int i = 0;
		for (const RPG::BattleCommand* command : bcmds) {
			commands.push_back(command->name);

			if (!Game_Battle::IsEscapeAllowed() && command->type == RPG::BattleCommand::Type_escape) {
				disabled_items.push_back(i);
			}
			++i;
		}
	}

	command_window.reset(new Window_Command(commands, 76));

	for (std::vector<int>::iterator it = disabled_items.begin(); it != disabled_items.end(); ++it) {
		command_window->DisableItem(*it);
	}

	command_window->SetHeight(80);
	if (Data::battlecommands.battle_type == RPG::BattleCommands::BattleType_gauge) {
		command_window->SetX(0);
		command_window->SetY(SCREEN_TARGET_HEIGHT / 2 - 80 / 2);
	}
	else {
		command_window->SetX(SCREEN_TARGET_WIDTH - 76);
		command_window->SetY(SCREEN_TARGET_HEIGHT - 80);
	}

	if (Data::battlecommands.battle_type != RPG::BattleCommands::BattleType_traditional) {
		int transp = Data::battlecommands.transparency == RPG::BattleCommands::Transparency_transparent ? 128 : 255;
		command_window->SetBackOpacity(transp);
	}
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
		Game_Battle::RefreshEvents([](const RPG::TroopPage& page) {
			const RPG::TroopPageCondition::Flags& flag = page.condition.flags;
			return flag.turn || flag.turn_actor || flag.turn_enemy ||
				   flag.switch_a || flag.switch_b || flag.variable ||
				   flag.fatigue;

		});
		break;
	case State_SelectOption:
		options_window->SetActive(true);
		break;
	case State_SelectActor:
		// no-op
		break;
	case State_AutoBattle:
		// no-op
		break;
	case State_SelectCommand:
		RefreshCommandWindow();
		command_window->SetActive(true);
		break;
	case State_SelectEnemyTarget:
		CreateBattleTargetWindow();
		select_target_flash_count = 0;
		break;
	case State_Battle:
		// no-op
		break;
	case State_SelectAllyTarget:
		status_window->SetActive(true);
		break;
	case State_SelectItem:
		item_window->SetActive(true);
		item_window->SetActor(active_actor);
		item_window->Refresh();
		break;
	case State_SelectSkill:
		skill_window->SetActive(true);
		skill_window->SetActor(active_actor->GetId());
		skill_window->SetIndex(0);
		break;
	case State_Victory:
	case State_Defeat:
	case State_Escape:
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
	case State_SelectActor:
		command_window->SetIndex(-1);
		status_window->SetVisible(true);
		status_window->SetX(0);
		status_window->SetChoiceMode(Window_BattleStatus::ChoiceMode_None);
		if (Data::battlecommands.battle_type != RPG::BattleCommands::BattleType_gauge) {
			command_window->SetVisible(true);
		}
		break;
	case State_SelectCommand:
		status_window->SetVisible(true);
		command_window->SetVisible(true);
		status_window->SetX(0);
		break;
	case State_SelectEnemyTarget:
		status_window->SetVisible(true);
		target_window->SetActive(true);

		if (Data::battlecommands.battle_type != RPG::BattleCommands::BattleType_gauge) {
			command_window->SetVisible(true);
		}

		if (Data::battlecommands.battle_type == RPG::BattleCommands::BattleType_traditional) {
			target_window->SetVisible(true);
		}
		break;
	case State_SelectAllyTarget:
		status_window->SetVisible(true);
		status_window->SetX(0);
		command_window->SetVisible(true);
		break;
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
		if (Data::battlecommands.battle_type != RPG::BattleCommands::BattleType_gauge) {
			command_window->SetVisible(true);
		}
		status_window->SetX(0);
		break;
	case State_Escape:
		status_window->SetVisible(true);
		command_window->SetVisible(true);
		status_window->SetX(0);
		break;
	}
}

void Scene_Battle_Rpg2k3::ProcessActions() {
	if (Main_Data::game_party->GetBattlerCount() == 0) {
		Game_Temp::battle_result = Game_Temp::BattleVictory;
		Scene::Pop();
	}

	if (!battle_actions.empty()) {
		Game_Battler* action = battle_actions.front();
		if (action->IsDead()) {
			// No zombies allowed ;)
			RemoveCurrentAction();
			battle_action_state = BattleActionState_Start;
		}
		else if (ProcessBattleAction(action->GetBattleAlgorithm().get())) {
			RemoveCurrentAction();
			if (CheckResultConditions()) {
				return;
			}
		}
	} else {
		if (CheckResultConditions()) {
			return;
		}
	}

	if (help_window->GetVisible() && message_timer > 0) {
		message_timer--;
		if (message_timer <= 0)
			help_window->SetVisible(false);
	}

	switch (state) {
		case State_Start:
			SetState(State_SelectOption);
			break;
		case State_SelectActor:
		case State_AutoBattle:
		case State_Battle:
			// no-op
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

bool Scene_Battle_Rpg2k3::ProcessBattleAction(Game_BattleAlgorithm::AlgorithmBase* action) {
	if (Game_Battle::IsBattleAnimationWaiting()) {
		return false;
	}

	if (play_reflected_anim) {
		action->PlayAnimation(true);
		play_reflected_anim = false;
		return false;
	}

	Sprite_Battler* source_sprite;
	source_sprite = Game_Battle::GetSpriteset().FindBattler(action->GetSource());

	if (source_sprite && !source_sprite->IsIdling()) {
		return false;
	}

	switch (battle_action_state) {
	case BattleActionState_Start:
		if (battle_action_need_event_refresh) {
			action->GetSource()->NextBattleTurn();
			NextTurn(action->GetSource());
			battle_action_need_event_refresh = false;

			// Next turn events must run before the battle animation is played
			Game_Battle::RefreshEvents([](const RPG::TroopPage& page) {
				const RPG::TroopPageCondition::Flags& flag = page.condition.flags;
				return flag.turn || flag.turn_actor || flag.turn_enemy ||
					   flag.command_actor;
			});

			return false;
		}

		action->TargetFirst();

		if (combo_repeat == 1) {
			ShowNotification(action->GetStartMessage());
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

		//printf("Action: %s\n", action->GetSource()->GetName().c_str());

		action->Execute();

		if (source_sprite) {
			source_sprite->Flash(Color(255, 255, 255, 100), 15);
			source_sprite->SetAnimationState(
				action->GetSourceAnimationState(),
				Sprite_Battler::LoopState_WaitAfterFinish);
		}

		action->PlayAnimation();
		play_reflected_anim = action->IsReflected();

		{
			std::vector<Game_Battler*> battlers;
			Main_Data::game_party->GetActiveBattlers(battlers);
			Main_Data::game_enemyparty->GetActiveBattlers(battlers);

			if (combo_repeat == 1) {
				for (auto &b : battlers) {
					int damageTaken = b->ApplyConditions();
					if (damageTaken != 0) {
						DrawFloatText(
								b->GetBattleX(),
								b->GetBattleY(),
								damageTaken < 0 ? Font::ColorDefault : Font::ColorHeal,
								Utils::ToString(damageTaken < 0 ? -damageTaken : damageTaken));
					}
				}
			}

			if (action->GetStartSe()) {
				Game_System::SePlay(*action->GetStartSe());
			}
		}

		battle_action_state = BattleActionState_Result;
		break;
	case BattleActionState_Result:
		if (source_sprite) {
			source_sprite->SetAnimationLoop(Sprite_Battler::LoopState_DefaultAnimationAfterFinish);
		}

		do {
			if (!action->IsFirstAttack()) {
				action->Execute();
			}

			Sprite_Battler* target_sprite = Game_Battle::GetSpriteset().FindBattler(action->GetTarget());
			if (action->IsSuccess() && !action->IsPositive() && target_sprite) {
				target_sprite->SetAnimationState(Sprite_Battler::AnimationState_Damage, Sprite_Battler::LoopState_DefaultAnimationAfterFinish);
			}

			Game_Battler* target = action->GetTarget();

			action->Apply();

			if (target) {
				if (action->IsSuccess()) {
					if (action->GetAffectedHp() != -1) {
						DrawFloatText(
							target->GetBattleX(),
							target->GetBattleY(),
							action->IsPositive() ? Font::ColorHeal : Font::ColorDefault,
							Utils::ToString(action->GetAffectedHp()));
					}

					target->BattlePhysicalStateHeal(action->GetPhysicalDamageRate());
				} else {
					DrawFloatText(
						target->GetBattleX(),
						target->GetBattleY(),
						0,
						Data::terms.miss);
				}

				targets.push_back(target);
			}

			status_window->Refresh();
		} while (action->TargetNext());

		if (action->GetResultSe()) {
			Game_System::SePlay(*action->GetResultSe());
		}

		battle_action_wait = 30;

		battle_action_state = BattleActionState_Finished;

		break;
	case BattleActionState_Finished:
		if (battle_action_need_event_refresh) {
			battle_action_wait = 30;
			battle_action_need_event_refresh = true;

			// Reset variables
			battle_action_state = BattleActionState_Start;
			targets.clear();
			combo_repeat = 1;

			return true;
		}

		if (battle_action_wait--) {
			return false;
		}

		{
			std::vector<Game_Battler*>::const_iterator it;

			for (it = targets.begin(); it != targets.end(); ++it) {
				Sprite_Battler* target_sprite = Game_Battle::GetSpriteset().FindBattler(*it);

				if ((*it)->IsDead()) {
					if (action->GetDeathSe()) {
						Game_System::SePlay(*action->GetDeathSe());
					}
				}

				if (target_sprite) {
					target_sprite->DetectStateChange();
				}
			}
		}

		// Check if a combo is enabled and redo the whole action in that case
		int combo_command_id;
		int combo_times;

		action->GetSource()->GetBattleCombo(combo_command_id, combo_times);
		if (action->GetSource()->GetLastBattleAction() == combo_command_id &&
			combo_times > combo_repeat) {
			// TODO: Prevent combo when the combo is a skill and needs more SP
			// then available

			battle_action_state = BattleActionState_Start;
			// Count how often we have to repeat
			++combo_repeat;
			return false;
		}

		// Must loop another time otherwise the event update happens during
		// SelectActor which updates the gauge
		battle_action_need_event_refresh = true;

		Game_Battle::RefreshEvents([](const RPG::TroopPage& page) {
			const RPG::TroopPageCondition::Flags& flag = page.condition.flags;
			return flag.switch_a || flag.switch_b || flag.variable ||
				   flag.fatigue || flag.actor_hp || flag.enemy_hp;
		});

		return false;
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
			// no-op
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
		case State_SelectItem:
			ItemSelected();
			break;
		case State_SelectSkill:
			SkillSelected();
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

	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
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
			active_actor->SetLastBattleAction(-1);
			SetState(State_SelectOption);
			break;
		case State_SelectEnemyTarget:
		case State_SelectItem:
		case State_SelectSkill:
			SetState(State_SelectCommand);
			break;
		case State_SelectAllyTarget:
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

void Scene_Battle_Rpg2k3::OptionSelected() {
	switch (options_window->GetIndex()) {
		case 0: // Battle
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
			auto_battle = false;
			SetState(State_SelectActor);
			break;
		case 1: // Auto Battle
			auto_battle = true;
			SetState(State_AutoBattle);
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
			break;
		case 2: // Escape
			// FIXME : Only enabled when party has initiative.
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
			//SetState(State_Escape);
			break;
	}
}

void Scene_Battle_Rpg2k3::CommandSelected() {
	const RPG::BattleCommand* command = active_actor->GetBattleCommands()[command_window->GetIndex()];

	switch (command->type) {
	case RPG::BattleCommand::Type_attack:
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
		AttackSelected();
		break;
	case RPG::BattleCommand::Type_defense:
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
		DefendSelected();
		break;
	case RPG::BattleCommand::Type_escape:
		if (!Game_Battle::IsEscapeAllowed()) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
		}
		else {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
			SetState(State_Escape);
		}
		break;
	case RPG::BattleCommand::Type_item:
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
		SetState(State_SelectItem);
		break;
	case RPG::BattleCommand::Type_skill:
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
		skill_window->SetSubsetFilter(0);
		SetState(State_SelectSkill);
		break;
	case RPG::BattleCommand::Type_special:
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
		SpecialSelected();
		break;
	case RPG::BattleCommand::Type_subskill:
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
		SubskillSelected();
		break;
	}
}

void Scene_Battle_Rpg2k3::AttackSelected() {
	Scene_Battle::AttackSelected();
}

void Scene_Battle_Rpg2k3::SubskillSelected() {
	// Resolving a subskill battle command to skill id
	int subskill = RPG::Skill::Type_subskill;

	const std::vector<const RPG::BattleCommand*> bcmds = active_actor->GetBattleCommands();
	// Get ID of battle command
	int command_id = bcmds[command_window->GetIndex()]->ID - 1;

	// Loop through all battle commands smaller then that ID and count subsets
	int i = 0;
	for (RPG::BattleCommand& cmd : Data::battlecommands.commands) {
		if (i >= command_id) {
			break;
		}

		if (cmd.type == RPG::BattleCommand::Type_subskill) {
			++subskill;
		}
		++i;
	}

	// skill subset is 4 (Type_subskill) + counted subsets
	skill_window->SetSubsetFilter(subskill);
	SetState(State_SelectSkill);
}

void Scene_Battle_Rpg2k3::SpecialSelected() {
	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));

	active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::NoMove>(active_actor));

	ActionSelectedCallback(active_actor);
}

void Scene_Battle_Rpg2k3::Escape() {
	Game_BattleAlgorithm::Escape escape_alg = Game_BattleAlgorithm::Escape(active_actor);
	active_actor->SetGauge(0);

	bool escape_success = escape_alg.Execute();
	escape_alg.Apply();

	if (!escape_success) {
		std::vector<std::string> battle_result_messages;
		escape_alg.GetResultMessages(battle_result_messages);
		SetState(State_SelectActor);
		ShowNotification(battle_result_messages[0]);
	}
	else {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Escape));
		Game_Temp::battle_result = Game_Temp::BattleEscape;

		// ToDo: Run away animation
		Scene::Pop();
	}
}

bool Scene_Battle_Rpg2k3::CheckWin() {
	if (Game_Battle::CheckWin()) {
		Game_Temp::battle_result = Game_Temp::BattleVictory;
		SetState(State_Victory);

		std::vector<Game_Battler*> battlers;
		Main_Data::game_party->GetActiveBattlers(battlers);
		for (std::vector<Game_Battler*>::const_iterator it = battlers.begin(); it != battlers.end(); ++it) {
			Sprite_Battler* sprite = Game_Battle::GetSpriteset().FindBattler(*it);
			if (sprite) {
				sprite->SetAnimationState(Sprite_Battler::AnimationState_Victory);
			}
		}

		int exp = Main_Data::game_enemyparty->GetExp();
		int money = Main_Data::game_enemyparty->GetMoney();
		std::vector<int> drops;
		Main_Data::game_enemyparty->GenerateDrops(drops);

		Game_Message::texts.push_back(Data::terms.victory);

		std::string space = Player::IsRPG2k3E() ? " " : "";

		std::stringstream ss;
		ss << exp << space << Data::terms.exp_received;
		Game_Message::texts.push_back(ss.str());
		if (money > 0) {
			ss.str("");
			ss << Data::terms.gold_recieved_a << " " << money << Data::terms.gold << Data::terms.gold_recieved_b;
			Game_Message::texts.push_back(ss.str());
		}
		for(std::vector<int>::iterator it = drops.begin(); it != drops.end(); ++it) {
			ss.str("");
			ss << Data::items[*it - 1].name << space << Data::terms.item_recieved;
			Game_Message::texts.push_back(ss.str());
		}

		message_window->SetHeight(32);
		Game_Message::message_waiting = true;

		Game_System::BgmPlay(Game_System::GetSystemBGM(Game_System::BGM_Victory));

		// Update attributes
		std::vector<Game_Battler*> ally_battlers;
		Main_Data::game_party->GetActiveBattlers(ally_battlers);

		for (std::vector<Game_Battler*>::iterator it = ally_battlers.begin();
			it != ally_battlers.end(); ++it) {
				Game_Actor* actor = static_cast<Game_Actor*>(*it);
				actor->ChangeExp(actor->GetExp() + exp, true);
		}

		for (std::string& str : Game_Message::texts) {
			// FIXME: We really need a more sane API for injecting breaks after each line

			if (!str.empty() && str[str.size() - 1] != '\f') {
				str += '\f';
			}
		}

		Main_Data::game_party->GainGold(money);
		for (std::vector<int>::iterator it = drops.begin(); it != drops.end(); ++it) {
			Main_Data::game_party->AddItem(*it, 1);
		}

		return true;
	}

	return false;
}

bool Scene_Battle_Rpg2k3::CheckLose() {
	if (Game_Battle::CheckLose()) {
		Game_Temp::battle_result = Game_Temp::BattleDefeat;
		SetState(State_Defeat);

		message_window->SetHeight(32);
		Game_Message::SetPositionFixed(true);
		Game_Message::SetPosition(0);
		Game_Message::SetTransparent(false);
		Game_Message::message_waiting = true;

		Game_Message::texts.push_back(Data::terms.defeat);

		Game_System::BgmPlay(Game_System::GetSystemBGM(Game_System::BGM_GameOver));

		return true;
	}

	return false;
}

bool Scene_Battle_Rpg2k3::CheckResultConditions() {
	if (state == State_Defeat || state == State_Victory) {
		return false;
	}

	return CheckLose() || CheckWin();
}

void Scene_Battle_Rpg2k3::SelectNextActor() {
	std::vector<Game_Battler*> battler;
	Main_Data::game_party->GetBattlers(battler);

	int i = 0;
	for (std::vector<Game_Battler*>::iterator it = battler.begin();
		it != battler.end(); ++it) {

		if ((*it)->IsGaugeFull() && !(*it)->GetBattleAlgorithm() && battle_actions.empty()) {
			actor_index = i;
			active_actor = static_cast<Game_Actor*>(*it);

			// Handle automatic attack
			Game_Battler* random_target = nullptr;

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

			if (random_target || auto_battle || active_actor->GetAutoBattle()) {
				if (!random_target) {
					random_target = Main_Data::game_enemyparty->GetRandomActiveBattler();
				}

				// ToDo: Auto battle logic is dumb
				active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(active_actor, random_target));
				battle_actions.push_back(active_actor);
				active_actor->SetGauge(0);

				return;
			}

			// Handle manual attack
			status_window->SetIndex(actor_index);

			if (active_actor->GetBattleCommands().empty()) {
				// Skip actors with only row command
				// FIXME: Actually support row command ;)
				NextTurn(active_actor);
				active_actor->SetGauge(0);
				return;
			}

			RefreshCommandWindow();

			SetState(Scene_Battle::State_SelectCommand);

			return;
		}

		++i;
	}
}

void Scene_Battle_Rpg2k3::ActionSelectedCallback(Game_Battler* for_battler) {
	for_battler->SetGauge(0);

	if (for_battler->GetType() == Game_Battler::Type_Ally) {
		const RPG::BattleCommand* command = static_cast<Game_Actor*>(for_battler)->GetBattleCommands()[command_window->GetIndex()];
		for_battler->SetLastBattleAction(command->ID);
		status_window->SetIndex(-1);
	}

	ally_cursor->SetVisible(false);
	enemy_cursor->SetVisible(false);

	Scene_Battle::ActionSelectedCallback(for_battler);
}

void Scene_Battle_Rpg2k3::ShowNotification(const std::string& text) {
	if (text.empty()) {
		return;
	}
	help_window->SetVisible(true);
	message_timer = 60;
	help_window->SetText(text);
}
