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
#include <lcf/rpg/battlecommand.h>
#include "input.h"
#include "output.h"
#include "player.h"
#include "sprite.h"
#include "cache.h"
#include "game_system.h"
#include "game_party.h"
#include "game_enemy.h"
#include "game_enemyparty.h"
#include "game_message.h"
#include "game_battle.h"
#include "game_battlealgorithm.h"
#include "game_screen.h"
#include <lcf/reader_util.h>
#include "scene_gameover.h"
#include "utils.h"
#include "font.h"
#include "output.h"

Scene_Battle_Rpg2k3::Scene_Battle_Rpg2k3(const BattleArgs& args) :
	Scene_Battle(args),
	first_strike(args.first_strike)
{
}

void Scene_Battle_Rpg2k3::Start() {
	Scene_Battle::Start();
	InitBattleCondition(Game_Battle::GetBattleCondition());

	// We need to wait for actor and enemy graphics to load before we can finish initializing the battle.
	AsyncNext([this]() { Start2(); });
}

void Scene_Battle_Rpg2k3::Start2() {
	InitEnemies();
	InitActors();
	InitAtbGauges();

	// Changed enemy place means we need to recompute Z order
	Game_Battle::GetSpriteset().ResetAllBattlerZ();
}

void Scene_Battle_Rpg2k3::InitBattleCondition(lcf::rpg::System::BattleCondition condition) {
	if (condition == lcf::rpg::System::BattleCondition_pincers
			&& (lcf::Data::battlecommands.placement == lcf::rpg::BattleCommands::Placement_manual
				|| Main_Data::game_enemyparty->GetVisibleBattlerCount() <= 1))
	{
		condition = lcf::rpg::System::BattleCondition_back;
	}

	if (condition == lcf::rpg::System::BattleCondition_surround
			&& (lcf::Data::battlecommands.placement == lcf::rpg::BattleCommands::Placement_manual
				|| Main_Data::game_party->GetVisibleBattlerCount() <= 1))
	{
		condition = lcf::rpg::System::BattleCondition_initiative;
	}

	Game_Battle::SetBattleCondition(condition);
}

void Scene_Battle_Rpg2k3::InitEnemies() {
	const auto& enemies = Main_Data::game_enemyparty->GetEnemies();
	const auto cond = Game_Battle::GetBattleCondition();

	// PLACEMENT AND DIRECTION
	for (int real_idx = 0, visible_idx = 0; real_idx < static_cast<int>(enemies.size()); ++real_idx) {
		auto& enemy = *enemies[real_idx];
		const auto idx = enemy.IsHidden() ? real_idx : visible_idx;

		enemy.SetBattlePosition(Game_Battle::Calculate2k3BattlePosition(enemy));

		switch(cond) {
			case lcf::rpg::System::BattleCondition_none:
				enemy.SetDirectionFlipped(false);
				break;
			case lcf::rpg::System::BattleCondition_initiative:
			case lcf::rpg::System::BattleCondition_back:
			case lcf::rpg::System::BattleCondition_surround:
				enemy.SetDirectionFlipped(true);
				break;
			case lcf::rpg::System::BattleCondition_pincers:
				enemy.SetDirectionFlipped(!(idx & 1));
				break;
		}

		visible_idx += !enemy.IsHidden();
	}
}

void Scene_Battle_Rpg2k3::InitActors() {
	const auto& actors = Main_Data::game_party->GetActors();
	const auto cond = Game_Battle::GetBattleCondition();

	// ROW ADJUSTMENT
	// If all actors in the front row have battle loss conditions,
	// all back row actors forced to the front row.
	// FIXME: Does this happen mid battle too?
	bool force_front_row = true;
	for (auto& actor: actors) {
		if (actor->GetBattleRow() == Game_Actor::RowType::RowType_front
				&& !actor->IsHidden()
				&& actor->CanActOrRecoverable()) {
			force_front_row = false;
		}
	}
	if (force_front_row) {
		for (auto& actor: actors) {
			actor->SetBattleRow(Game_Actor::RowType::RowType_front);
		}
	}

	// PLACEMENT AND DIRECTION
	for (int idx = 0; idx < static_cast<int>(actors.size()); ++idx) {
		auto& actor = *actors[idx];

		actor.SetBattlePosition(Game_Battle::Calculate2k3BattlePosition(actor));

		if (cond == lcf::rpg::System::BattleCondition_surround) {
			actor.SetDirectionFlipped(idx & 1);
		} else {
			actor.SetDirectionFlipped(false);
		}
	}
}

Scene_Battle_Rpg2k3::~Scene_Battle_Rpg2k3() {
}

void Scene_Battle_Rpg2k3::InitAtbGauge(Game_Battler& battler, int preempt_atb, int ambush_atb) {
	if (battler.IsHidden() || !battler.CanActOrRecoverable()) {
		return;
	}

	switch(Game_Battle::GetBattleCondition()) {
		case lcf::rpg::System::BattleCondition_initiative:
		case lcf::rpg::System::BattleCondition_surround:
			battler.SetAtbGauge(preempt_atb);
			break;
		case lcf::rpg::System::BattleCondition_back:
		case lcf::rpg::System::BattleCondition_pincers:
			battler.SetAtbGauge(ambush_atb);
			break;
		case lcf::rpg::System::BattleCondition_none:
			if (first_strike || battler.HasPreemptiveAttack()) {
				battler.SetAtbGauge(preempt_atb);
			} else {
				battler.SetAtbGauge(Game_Battler::GetMaxAtbGauge() / 2);
			}
			break;
	}
}

void Scene_Battle_Rpg2k3::InitAtbGauges() {
	for (auto& enemy: Main_Data::game_enemyparty->GetEnemies()) {
		InitAtbGauge(*enemy, 0, Game_Battler::GetMaxAtbGauge());
	}
	for (auto& actor: Main_Data::game_party->GetActors()) {
		InitAtbGauge(*actor, Game_Battler::GetMaxAtbGauge(), 0);
	}
}

template <typename O, typename M, typename C>
static bool CheckFlip(const O& others, const M& me, bool prefer_flipped, C&& cmp) {
	for (auto& other: others) {
			if (!other->IsHidden() && cmp(other->GetBattlePosition().x, me.GetBattlePosition().x)) {
				return prefer_flipped;
			}
		}
		return !prefer_flipped;
	}

void Scene_Battle_Rpg2k3::UpdateEnemiesDirection() {
	const auto& enemies = Main_Data::game_enemyparty->GetEnemies();
	const auto& actors = Main_Data::game_party->GetActors();

	for (int real_idx = 0, visible_idx = 0; real_idx < static_cast<int>(enemies.size()); ++real_idx) {
		auto& enemy = *enemies[real_idx];
		const auto idx = enemy.IsHidden() ? real_idx : visible_idx;

		switch(Game_Battle::GetBattleCondition()) {
			case lcf::rpg::System::BattleCondition_none:
			case lcf::rpg::System::BattleCondition_initiative:
				enemy.SetDirectionFlipped(CheckFlip(actors, enemy, false, std::greater_equal<>()));
				break;
			case lcf::rpg::System::BattleCondition_back:
				enemy.SetDirectionFlipped(CheckFlip(actors, enemy, true, std::less_equal<>()));
				break;
			case lcf::rpg::System::BattleCondition_surround:
			case lcf::rpg::System::BattleCondition_pincers:
				enemy.SetDirectionFlipped(!(idx & 1));
				break;
		}

		visible_idx += !enemy.IsHidden();
	}
}

void Scene_Battle_Rpg2k3::UpdateActorsDirection() {
	const auto& actors = Main_Data::game_party->GetActors();
	const auto& enemies = Main_Data::game_enemyparty->GetEnemies();

	for (int idx = 0; idx < static_cast<int>(actors.size()); ++idx) {
		auto& actor = *actors[idx];

		switch(Game_Battle::GetBattleCondition()) {
			case lcf::rpg::System::BattleCondition_none:
			case lcf::rpg::System::BattleCondition_initiative:
				actor.SetDirectionFlipped(CheckFlip(enemies, actor, false, std::less_equal<>()));
				break;
			case lcf::rpg::System::BattleCondition_back:
				actor.SetDirectionFlipped(CheckFlip(enemies, actor, true, std::greater_equal<>()));
				break;
			case lcf::rpg::System::BattleCondition_surround:
			case lcf::rpg::System::BattleCondition_pincers:
				actor.SetDirectionFlipped(idx & 1);
				break;
		}
	}
}

void Scene_Battle_Rpg2k3::FaceTarget(Game_Actor& source, const Game_Battler& target) {
	const auto sx = source.GetBattlePosition().x;
	const auto tx = target.GetBattlePosition().x;
	const bool flipped = source.IsDirectionFlipped();
	if ((flipped && tx < sx) || (!flipped && tx > sx)) {
		source.SetDirectionFlipped(1 - flipped);
	}
}

void Scene_Battle_Rpg2k3::Update() {
	// FIXME: RPG_RT sets initial directions on start, displays any
	// battle start messages, and then monsters may turn to face the party
	// and the party turns to face the monsters.
	// This only happens once on battle start. Until we refactor 2k3 state machine,
	// we emulate the behavior with this bool.
	// Monster directions never change during battle, but actors can.
	if (!initial_directions_updated) {
		UpdateEnemiesDirection();
		UpdateActorsDirection();
		initial_directions_updated = true;
	}

	switch (state) {
		case State_SelectActor:
		case State_AutoBattle: {
			if (!IsWindowMoving()) {

				if (battle_actions.empty()) {
					Game_Battle::UpdateAtbGauges();
				}

				int old_state = state;
				SelectNextActor();

				if (old_state == state && battle_actions.empty()) {
					// No actor got the turn
					std::vector<Game_Battler*> enemies;
					Main_Data::game_enemyparty->GetActiveBattlers(enemies);

					for (auto* battler: enemies) {
						if (battler->IsAtbGaugeFull() && !battler->GetBattleAlgorithm()) {
							auto* enemy = static_cast<Game_Enemy*>(battler);
							const auto* action = enemy->ChooseRandomAction();
							if (action) {
								CreateEnemyAction(enemy, action);
							}
							//FIXME: Do we need to handle invalid actions or empty action list here?
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

	//enemy_status_window->Update();
}

void Scene_Battle_Rpg2k3::OnSystem2Ready(FileRequestResult* result) {
	Cache::SetSystem2Name(result->file);

	SetupSystem2Graphics();
}

void Scene_Battle_Rpg2k3::SetupSystem2Graphics() {
	BitmapRef system2 = Cache::System2();
	if (!system2) {
		return;
	}

	ally_cursor->SetBitmap(system2);
	ally_cursor->SetZ(Priority_Window);
	ally_cursor->SetVisible(false);

	enemy_cursor->SetBitmap(system2);
	enemy_cursor->SetZ(Priority_Window);
	enemy_cursor->SetVisible(false);

	enemy_status_window->Refresh();
}

void Scene_Battle_Rpg2k3::CreateUi() {
	Scene_Battle::CreateUi();

	CreateBattleTargetWindow();
	CreateBattleCommandWindow();

	enemy_status_window.reset(new Window_BattleStatus(0, 0, SCREEN_TARGET_WIDTH - option_command_mov, 80, true));
	enemy_status_window->SetVisible(false);
	sp_window.reset(new Window_ActorSp(SCREEN_TARGET_WIDTH - 60, 136, 60, 32));
	sp_window->SetVisible(false);
	sp_window->SetZ(Priority_Window + 1);

	ally_cursor.reset(new Sprite());
	enemy_cursor.reset(new Sprite());

	if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_gauge) {
		item_window->SetY(64);
		skill_window->SetY(64);

		// Default window too small for 4 actors
		status_window.reset(new Window_BattleStatus(0, SCREEN_TARGET_HEIGHT - 80, SCREEN_TARGET_WIDTH, 80));
	}

	if (lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_traditional) {
		int transp = lcf::Data::battlecommands.transparency == lcf::rpg::BattleCommands::Transparency_transparent ? 128 : 255;
		options_window->SetBackOpacity(transp);
		item_window->SetBackOpacity(transp);
		skill_window->SetBackOpacity(transp);
		help_window->SetBackOpacity(transp);
		status_window->SetBackOpacity(transp);
		enemy_status_window->SetBackOpacity(transp);
	}

	if (!Cache::System2() && Game_System::HasSystem2Graphic()) {
		FileRequestAsync* request = AsyncHandler::RequestFile("System2", Game_System::GetSystem2Name());
		request->SetGraphicFile(true);
		request_id = request->Bind(&Scene_Battle_Rpg2k3::OnSystem2Ready, this);
		request->Start();
	} else {
		SetupSystem2Graphics();
	}
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

		if (ally_index >= 0 && lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_traditional) {
			ally_cursor->SetVisible(true);
			Main_Data::game_party->GetBattlers(actors);
			Game_Battler* actor = actors[ally_index];
			Sprite_Battler* sprite = Game_Battle::GetSpriteset().FindBattler(actor);
			ally_cursor->SetX(actor->GetBattlePosition().x);
			ally_cursor->SetY(actor->GetBattlePosition().y - sprite->GetHeight() / 2);
			static const int frames[] = { 0, 1, 2, 1 };
			int frame = frames[(cycle / 15) % 4];
			ally_cursor->SetSrcRect(Rect(frame * 16, 16, 16, 16));

			if (cycle % 30 == 0) {
				SelectionFlash(actor);
			}
		}

		if (enemy_index >= 0) {
			enemy_cursor->SetVisible(true);
			actors.clear();
			Main_Data::game_enemyparty->GetActiveBattlers(actors);
			const Game_Battler* actor = actors[enemy_index];
			const Sprite_Battler* sprite = Game_Battle::GetSpriteset().FindBattler(actor);
			enemy_cursor->SetX(actor->GetBattlePosition().x + sprite->GetWidth() / 2 + 2);
			enemy_cursor->SetY(actor->GetBattlePosition().y - enemy_cursor->GetHeight() / 2);
			static const int frames[] = { 0, 1, 2, 1 };
			int frame = frames[(cycle / 15) % 4];
			enemy_cursor->SetSrcRect(Rect(frame * 16, 0, 16, 16));

			if (state == State_SelectEnemyTarget) {
				auto states = actor->GetInflictedStates();

				help_window->SetVisible(!states.empty());
				help_window->Clear();
				BitmapRef contents = help_window->GetContents();

				int text_width = 0;
				for (auto state_id : states) {
					// States are sanitized in Game_Battler
					const lcf::rpg::State* state = lcf::ReaderUtil::GetElement(lcf::Data::states, state_id);
					std::string name = state->name;
					int color = state->color;
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

	if (lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_traditional) {
		int transp = lcf::Data::battlecommands.transparency == lcf::rpg::BattleCommands::Transparency_transparent ? 128 : 255;
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
		const std::vector<const lcf::rpg::BattleCommand*> bcmds = actor->GetBattleCommands();
		int i = 0;
		for (const lcf::rpg::BattleCommand* command : bcmds) {
			commands.push_back(command->name);

			if (!IsEscapeAllowedFromActorCommand() && command->type == lcf::rpg::BattleCommand::Type_escape) {
				disabled_items.push_back(i);
			}
			++i;
		}
	}

	command_window.reset(new Window_Command(commands, option_command_mov));

	for (std::vector<int>::iterator it = disabled_items.begin(); it != disabled_items.end(); ++it) {
		command_window->DisableItem(*it);
	}

	command_window->SetHeight(80);
	if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_gauge) {
		command_window->SetX(0);
		command_window->SetY(SCREEN_TARGET_HEIGHT / 2 - 80 / 2);
	}
	else {
		command_window->SetX(SCREEN_TARGET_WIDTH - option_command_mov);
		command_window->SetY(SCREEN_TARGET_HEIGHT - 80);
	}

	if (lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_traditional) {
		int transp = lcf::Data::battlecommands.transparency == lcf::rpg::BattleCommands::Transparency_transparent ? 128 : 255;
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
		Game_Battle::RefreshEvents([](const lcf::rpg::TroopPage& page) {
			const lcf::rpg::TroopPageCondition::Flags& flag = page.condition.flags;
			return flag.turn || flag.turn_actor || flag.turn_enemy ||
				   flag.switch_a || flag.switch_b || flag.variable ||
				   flag.fatigue;

		});
		break;
	case State_SelectOption:
		options_window->SetActive(true);
		if (IsEscapeAllowedFromOptionWindow()) {
			options_window->EnableItem(2);
		} else {
			options_window->DisableItem(2);
		}

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
		if (previous_state == State_SelectCommand) {
			skill_window->RestoreActorIndex(actor_index);
		}
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
	sp_window->SetVisible(false);

	if (previous_state == State_SelectSkill) {
		skill_window->SaveActorIndex(actor_index);
	}

	switch (state) {
	case State_Start:
		break;
	case State_SelectOption:
		options_window->SetVisible(true);
		options_window->SetX(0);
		status_window->SetVisible(true);
		status_window->SetX(option_command_mov);
		status_window->SetIndex(-1);
		status_window->Refresh();
		if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_alternative) {
			command_window->SetX(SCREEN_TARGET_WIDTH);
			command_window->SetIndex(-1);
		}
		break;
	case State_AutoBattle:
	case State_SelectActor:
		options_window->SetVisible(true);
		options_window->SetX(-option_command_mov);
		status_window->SetVisible(true);
		status_window->SetX(0);
		status_window->SetChoiceMode(Window_BattleStatus::ChoiceMode_None);
		command_window->SetIndex(-1);
		command_window->SetX(SCREEN_TARGET_WIDTH - option_command_mov);
		if (lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_gauge) {
			command_window->SetVisible(true);
		}
		break;
	case State_SelectCommand:
		if (lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_traditional) {
			options_window->SetVisible(true);
		}
		status_window->SetVisible(true);
		command_window->SetVisible(true);
		break;
	case State_SelectEnemyTarget:
		status_window->SetVisible(true);
		target_window->SetActive(true);

		if (lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_gauge) {
			command_window->SetVisible(true);
		}

		if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_traditional) {
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
		if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_traditional) {
			sp_window->SetVisible(true);
		}
		break;
	case State_Victory:
	case State_Defeat:
		status_window->SetVisible(true);
		if (lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_gauge) {
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

	// If SelectOption <-> SelectCommand => Display Movement:
	if (lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_traditional) {
		if ((previous_state == State_SelectActor || previous_state == State_AutoBattle || previous_state == State_SelectCommand) && state == State_SelectOption) {
			options_window->InitMovement(options_window->GetX() - option_command_mov, options_window->GetY(),
				options_window->GetX(), options_window->GetY(), option_command_time);

			status_window->InitMovement(status_window->GetX() - option_command_mov, status_window->GetY(),
				status_window->GetX(), status_window->GetY(), option_command_time);

			if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_alternative) {
				command_window->SetVisible(true);
				command_window->InitMovement(command_window->GetX() - option_command_mov, command_window->GetY(),
					command_window->GetX(), command_window->GetY(), option_command_time);
			}
		}
		else if (previous_state == State_SelectOption && (state == State_SelectActor || state == State_AutoBattle)) {
			options_window->SetVisible(true);
			options_window->InitMovement(options_window->GetX() + option_command_mov, options_window->GetY(),
				options_window->GetX(), options_window->GetY(), option_command_time);

			status_window->InitMovement(status_window->GetX() + option_command_mov, status_window->GetY(),
				status_window->GetX(), status_window->GetY(), option_command_time);

			if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_alternative) {
				command_window->InitMovement(command_window->GetX() + option_command_mov, command_window->GetY(),
					command_window->GetX(), command_window->GetY(), option_command_time);
			}
		}
	}
}

void Scene_Battle_Rpg2k3::ProcessActions() {
	if (Main_Data::game_party->GetBattlerCount() == 0) {
		EndBattle(BattleResult::Victory);
		return;
	}

	if (!battle_actions.empty()) {
		auto* battler = battle_actions.front();
		if (!battle_action_pending) {
			// If we will start a new battle action, first check for state changes
			// such as death, paralyze, confuse, etc..
			PrepareBattleAction(battler);

		}
		auto* alg = battler->GetBattleAlgorithm().get();
		battle_action_pending = true;
		if (ProcessBattleAction(alg)) {
			battle_action_pending = false;
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

	if (help_window->IsVisible() && message_timer > 0) {
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
			++select_target_flash_count;

			if (select_target_flash_count == 60) {
				SelectionFlash(target);
				select_target_flash_count = 0;
			}
			break;
		}
		case State_Victory:
			EndBattle(BattleResult::Victory);
			break;
		case State_Defeat:
			EndBattle(BattleResult::Defeat);
			break;
		case State_Escape:
			Escape();
			break;
		default:
			break;
	}
}

bool Scene_Battle_Rpg2k3::ProcessBattleAction(Game_BattleAlgorithm::AlgorithmBase* action) {
	// Immediately quit for dead actors no move. Prevents any animations or delays.
	if (action->GetType() == Game_BattleAlgorithm::Type::NoMove && action->GetSource()->IsDead()) {
		return true;
	}

	if (Game_Battle::IsBattleAnimationWaiting()) {
		return false;
	}

	if (play_reflect_anim) {
		play_reflect_anim = false;
		action->PlayAnimation(false, CheckAnimFlip(action->GetFirstOriginalTarget()));
		return false;
	}

	Sprite_Battler* source_sprite;
	source_sprite = Game_Battle::GetSpriteset().FindBattler(action->GetSource());

	if (source_sprite && !source_sprite->IsIdling()) {
		return false;
	}

	bool is_target_party = false;

	switch (battle_action_state) {
	case BattleActionState_Execute:
		if (battle_action_need_event_refresh) {
			action->GetSource()->NextBattleTurn();

			// The internal state turn counter increments for all every turn
			std::vector<Game_Battler*> battler;
			Main_Data::game_party->GetActiveBattlers(battler);
			Main_Data::game_enemyparty->GetActiveBattlers(battler);

			for (auto& b : battler) {
				b->BattleStateHeal();
			}

			NextTurn(action->GetSource());
			battle_action_need_event_refresh = false;

			// Next turn events must run before the battle animation is played
			Game_Battle::RefreshEvents([](const lcf::rpg::TroopPage& page) {
				const lcf::rpg::TroopPageCondition::Flags& flag = page.condition.flags;
				return flag.turn || flag.turn_actor || flag.turn_enemy ||
					   flag.command_actor;
			});

			return false;
		}

		// FIXME: This gets cleared after calling TargetFirst() so we query it now.
		// Refactor this to be less brittle.
		// FIXME: This bool should be locally scoped here, but that requires refactoring this switch statement.
		is_target_party = action->IsTargetingParty();

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

		if (action->GetSource()->GetType() == Game_Battler::Type_Ally
				&& !is_target_party
				&& action->GetTarget()
				&& action->GetTarget()->GetType() == Game_Battler::Type_Enemy)
		{
			auto* actor = static_cast<Game_Actor*>(action->GetSource());
			FaceTarget(*actor, *action->GetTarget());
		}

		//Output::Debug("Action: {}", action->GetSource()->GetName());

		action->Execute();

		if (source_sprite) {
			SelectionFlash(action->GetSource());
			source_sprite->SetAnimationState(
				action->GetSourceAnimationState(),
				Sprite_Battler::LoopState_WaitAfterFinish);
		}

		if (action->OriginalTargetsSet()) {
			play_reflect_anim = true;
			action->PlayAnimation(true, CheckAnimFlip(action->GetSource()));
		} else {
			action->PlayAnimation(false, CheckAnimFlip(action->GetSource()));
		}

		{
			std::vector<Game_Battler*> battlers;
			Main_Data::game_party->GetActiveBattlers(battlers);
			Main_Data::game_enemyparty->GetActiveBattlers(battlers);

			if (combo_repeat == 1) {
				for (auto &b : battlers) {
					int damageTaken = b->ApplyConditions();
					if (damageTaken != 0) {
						DrawFloatText(
								b->GetBattlePosition().x,
								b->GetBattlePosition().y,
								damageTaken < 0 ? Font::ColorDefault : Font::ColorHeal,
								std::to_string(damageTaken < 0 ? -damageTaken : damageTaken));
					}
				}
			}

			if (action->GetStartSe()) {
				Game_System::SePlay(*action->GetStartSe());
			}
		}

		battle_action_state = BattleActionState_ResultPush;
		break;
	case BattleActionState_ResultPush:
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
					if (action->IsCriticalHit()) {
						Main_Data::game_screen->FlashOnce(28, 28, 28, 20, 8);
					}
					if (action->GetAffectedHp() != -1) {
						DrawFloatText(
							target->GetBattlePosition().x,
							target->GetBattlePosition().y,
							action->IsPositive() ? Font::ColorHeal : Font::ColorDefault,
							std::to_string(action->GetAffectedHp()));
					}
				} else {
					DrawFloatText(
						target->GetBattlePosition().x,
						target->GetBattlePosition().y,
						0,
						lcf::Data::terms.miss);
				}

				targets.push_back(target);
			}

			status_window->Refresh();
		} while (action->TargetNext());

		//FIXME: Figure out specific logic for 2k3 and remove GetResultSe() method.
		//This method is no longer used in 2k battle system.
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
			battle_action_state = BattleActionState_Execute;
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

			battle_action_state = BattleActionState_Execute;
			// Count how often we have to repeat
			++combo_repeat;
			return false;
		}

		// Must loop another time otherwise the event update happens during
		// SelectActor which updates the gauge
		battle_action_need_event_refresh = true;

		Game_Battle::RefreshEvents([](const lcf::rpg::TroopPage& page) {
			const lcf::rpg::TroopPageCondition::Flags& flag = page.condition.flags;
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
			if (!message_window->IsVisible()) {
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
		case State_SelectItem:
		case State_SelectSkill:
			SetState(State_SelectCommand);
			break;
		case State_SelectEnemyTarget:
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

	if (Input::IsTriggered(Input::DEBUG_MENU)) {
		this->CallDebug();
	}
}

bool Scene_Battle_Rpg2k3::IsEscapeAllowedFromOptionWindow() const {
	auto cond = Game_Battle::GetBattleCondition();

	return Scene_Battle::IsEscapeAllowed() && (Game_Battle::GetTurn() == 0)
		&& (first_strike || cond == lcf::rpg::System::BattleCondition_initiative || cond == lcf::rpg::System::BattleCondition_surround);
}

bool Scene_Battle_Rpg2k3::IsEscapeAllowedFromActorCommand() const {
	auto cond = Game_Battle::GetBattleCondition();

	return Scene_Battle::IsEscapeAllowed() && cond != lcf::rpg::System::BattleCondition_pincers;
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
			if (IsEscapeAllowedFromOptionWindow()) {
				Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
				Escape(true);
			} else {
				Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
			}
			break;
	}
}

void Scene_Battle_Rpg2k3::CommandSelected() {
	const lcf::rpg::BattleCommand* command = active_actor->GetBattleCommands()[command_window->GetIndex()];

	switch (command->type) {
	case lcf::rpg::BattleCommand::Type_attack:
		AttackSelected();
		break;
	case lcf::rpg::BattleCommand::Type_defense:
		DefendSelected();
		break;
	case lcf::rpg::BattleCommand::Type_escape:
		if (!IsEscapeAllowedFromActorCommand()) {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
		}
		else {
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
			active_actor->SetAtbGauge(0);
			SetState(State_Escape);
		}
		break;
	case lcf::rpg::BattleCommand::Type_item:
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
		SetState(State_SelectItem);
		break;
	case lcf::rpg::BattleCommand::Type_skill:
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
		skill_window->SetSubsetFilter(0);
		sp_window->SetBattler(*active_actor);
		SetState(State_SelectSkill);
		break;
	case lcf::rpg::BattleCommand::Type_special:
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
		SpecialSelected();
		break;
	case lcf::rpg::BattleCommand::Type_subskill:
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
	int subskill = lcf::rpg::Skill::Type_subskill;

	const std::vector<const lcf::rpg::BattleCommand*> bcmds = active_actor->GetBattleCommands();
	// Get ID of battle command
	int command_id = bcmds[command_window->GetIndex()]->ID - 1;

	// Loop through all battle commands smaller then that ID and count subsets
	int i = 0;
	for (lcf::rpg::BattleCommand& cmd : lcf::Data::battlecommands.commands) {
		if (i >= command_id) {
			break;
		}

		if (cmd.type == lcf::rpg::BattleCommand::Type_subskill) {
			++subskill;
		}
		++i;
	}

	// skill subset is 4 (Type_subskill) + counted subsets
	skill_window->SetSubsetFilter(subskill);
	SetState(State_SelectSkill);
	sp_window->SetBattler(*active_actor);
}

void Scene_Battle_Rpg2k3::SpecialSelected() {
	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));

	active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::NoMove>(active_actor));

	ActionSelectedCallback(active_actor);
}

void Scene_Battle_Rpg2k3::Escape(bool force_allow) {
	if (force_allow || TryEscape()) {
		// There is no success text for escape in 2k3
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Escape));
		EndBattle(BattleResult::Escape);
		return;
	}

	SetState(State_SelectActor);
	ShowNotification(lcf::Data::terms.escape_failure);
}

bool Scene_Battle_Rpg2k3::CheckWin() {
	if (Game_Battle::CheckWin()) {
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

		auto pm = PendingMessage();
		pm.SetEnableFace(false);

		pm.PushLine(lcf::Data::terms.victory + Player::escape_symbol + "|");
		pm.PushPageEnd();

		std::string space = Player::IsRPG2k3E() ? " " : "";

		std::stringstream ss;
		if (exp > 0) {
			ss << exp << space << lcf::Data::terms.exp_received;
			pm.PushLine(ss.str());
			pm.PushPageEnd();
		}
		if (money > 0) {
			ss.str("");
			ss << lcf::Data::terms.gold_recieved_a << " " << money << lcf::Data::terms.gold << lcf::Data::terms.gold_recieved_b;
			pm.PushLine(ss.str());
			pm.PushPageEnd();
		}
		for (std::vector<int>::iterator it = drops.begin(); it != drops.end(); ++it) {
			const lcf::rpg::Item* item = lcf::ReaderUtil::GetElement(lcf::Data::items, *it);
			// No Output::Warning needed here, reported later when the item is added
			std::string item_name = "??? BAD ITEM ???";
			if (item) {
				item_name = item->name;
			}

			ss.str("");
			ss << item_name << space << lcf::Data::terms.item_recieved;
			pm.PushLine(ss.str());
			pm.PushPageEnd();
		}

		message_window->SetHeight(32);
		Game_Message::SetPendingMessage(std::move(pm));

		Game_System::BgmPlay(Game_System::GetSystemBGM(Game_System::BGM_Victory));

		// Update attributes
		std::vector<Game_Battler*> ally_battlers;
		Main_Data::game_party->GetActiveBattlers(ally_battlers);

		pm.PushPageEnd();

		for (std::vector<Game_Battler*>::iterator it = ally_battlers.begin();
			it != ally_battlers.end(); ++it) {
				Game_Actor* actor = static_cast<Game_Actor*>(*it);
				actor->ChangeExp(actor->GetExp() + exp, &pm);
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
		SetState(State_Defeat);

		message_window->SetHeight(32);
		Game_Message::SetPositionFixed(true);
		Game_Message::SetPosition(0);
		Game_Message::SetTransparent(false);

		auto pm = PendingMessage();
		pm.SetEnableFace(false);
		pm.PushLine(lcf::Data::terms.defeat);

		Game_System::BgmPlay(Game_System::GetSystemBGM(Game_System::BGM_GameOver));
		Game_Message::SetPendingMessage(std::move(pm));

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
	std::vector<Game_Battler*> actors;
	Main_Data::game_party->GetBattlers(actors);

	int i = 0;
	for (auto* battler: actors) {
		if (battler->IsAtbGaugeFull() && !battler->GetBattleAlgorithm() && battle_actions.empty()) {
			actor_index = i;
			active_actor = static_cast<Game_Actor*>(battler);

			// Handle automatic attack
			Game_Battler* random_target = nullptr;

			if (active_actor->CanAct()) {
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
			}

			if (random_target || auto_battle || active_actor->GetAutoBattle()) {
				if (!random_target) {
					random_target = Main_Data::game_enemyparty->GetRandomActiveBattler();
				}

				// ToDo: Auto battle logic is dumb
				active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(active_actor, random_target));
				battle_actions.push_back(active_actor);
				active_actor->SetAtbGauge(0);

				return;
			}

			// Handle manual attack
			status_window->SetIndex(actor_index);

			if (active_actor->GetBattleCommands().empty()) {
				// Skip actors with only row command
				// FIXME: Actually support row command ;)
				NextTurn(active_actor);
				active_actor->SetAtbGauge(0);
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
	for_battler->SetAtbGauge(0);

	if (for_battler->GetType() == Game_Battler::Type_Ally) {
		const lcf::rpg::BattleCommand* command = static_cast<Game_Actor*>(for_battler)->GetBattleCommands()[command_window->GetIndex()];
		for_battler->SetLastBattleAction(command->ID);
		status_window->SetIndex(-1);
	}

	ally_cursor->SetVisible(false);
	enemy_cursor->SetVisible(false);

	Scene_Battle::ActionSelectedCallback(for_battler);

	// First strike escape bonus cancelled on actor non-escape action.
	first_strike = false;
}

void Scene_Battle_Rpg2k3::ShowNotification(const std::string& text) {
	if (text.empty()) {
		return;
	}
	help_window->SetVisible(true);
	message_timer = 60;
	help_window->SetText(text);
}

bool Scene_Battle_Rpg2k3::CheckAnimFlip(Game_Battler* battler) {
	if (Game_System::GetInvertAnimations()) {
		return battler->IsDirectionFlipped() ^ battler->GetType() == Game_Battler::Type_Enemy;
	}
	return false;
}
