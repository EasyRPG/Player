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

#include <cmath>
#include "scene_battle_rpg2k3.h"
#include <lcf/rpg/battlecommand.h>
#include <lcf/rpg/battleranimation.h>
#include <lcf/reader_util.h>
#include "drawable.h"
#include "input.h"
#include "output.h"
#include "player.h"
#include "sprite.h"
#include "sprite_enemy.h"
#include "sprite_actor.h"
#include "sprite_weapon.h"
#include "cache.h"
#include "game_actors.h"
#include "game_system.h"
#include "game_party.h"
#include "game_enemy.h"
#include "game_enemyparty.h"
#include "game_message.h"
#include "game_battle.h"
#include "game_interpreter_battle.h"
#include "game_battlealgorithm.h"
#include "game_screen.h"
#include <lcf/reader_util.h>
#include "scene_gameover.h"
#include "utils.h"
#include "font.h"
#include "output.h"
#include "autobattle.h"
#include "enemyai.h"
#include <algorithm>
#include <memory>
#include "feature.h"

//#define EP_DEBUG_BATTLE2K3_STATE_MACHINE

Scene_Battle_Rpg2k3::Scene_Battle_Rpg2k3(const BattleArgs& args) :
	Scene_Battle(args),
	first_strike(args.first_strike)
{
}

void Scene_Battle_Rpg2k3::Start() {
	Scene_Battle::Start();
	InitBattleCondition(Game_Battle::GetBattleCondition());
	CreateEnemySprites();
	CreateActorSprites();

	// We need to wait for actor and enemy graphics to load before we can finish initializing the battle.
	AsyncNext([this]() { Start2(); });
}

void Scene_Battle_Rpg2k3::Start2() {
	InitEnemies();
	InitActors();
	InitAtbGauges();

	// Changed enemy place means we need to recompute Z order
	ResetAllBattlerZ();
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

	if (condition == lcf::rpg::System::BattleCondition_back || condition == lcf::rpg::System::BattleCondition_pincers) {
		first_strike = false;
	}
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
}

void Scene_Battle_Rpg2k3::CreateUi() {
	Scene_Battle::CreateUi();

	CreateBattleTargetWindow();
	CreateBattleStatusWindow();
	CreateBattleCommandWindow();

	RecreateSpWindow(nullptr);

	ally_cursor.reset(new Sprite());
	enemy_cursor.reset(new Sprite());

	if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_gauge) {
		item_window->SetX(Player::menu_offset_x);
		item_window->SetY(Player::menu_offset_y + 64);
		skill_window->SetX(Player::menu_offset_x);
		skill_window->SetY(Player::menu_offset_y + 64);
	}

	if (lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_traditional) {
		int transp = IsTransparent() ? 160 : 255;
		options_window->SetBackOpacity(transp);
		item_window->SetBackOpacity(transp);
		skill_window->SetBackOpacity(transp);
		help_window->SetBackOpacity(transp);
		status_window->SetBackOpacity(transp);
	}

	if (!Cache::System2() && Main_Data::game_system->HasSystem2Graphic()) {
		FileRequestAsync* request = AsyncHandler::RequestFile("System2", Main_Data::game_system->GetSystem2Name());
		request->SetGraphicFile(true);
		request_id = request->Bind(&Scene_Battle_Rpg2k3::OnSystem2Ready, this);
		request->Start();
	} else {
		SetupSystem2Graphics();
	}

	if (lcf::Data::battlecommands.window_size == lcf::rpg::BattleCommands::WindowSize_small) {
		int height = 68;
		int y = Player::screen_height - Player::menu_offset_y - height;

		auto small_window = [&](auto& window) {
			if (window) {
				window->SetHeight(height);
				window->SetY(y);
				window->SetBorderY(5);
				window->SetContents(Bitmap::Create(window->GetWidth() - window->GetBorderX() * 2, window->GetHeight() - window->GetBorderY() * 2));
				window->SetMenuItemHeight(14);
			}
		};

		small_window(options_window);
		small_window(command_window);
		small_window(skill_window);
		small_window(item_window);
		small_window(target_window);

		options_window->Refresh();
		status_window->SetY(y);

		if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_gauge) {
			command_window->SetY(Player::screen_height / 2 - 80 / 2 + 12);
			item_window->SetY(76);
			skill_window->SetY(76);
		}
	}

	ResetWindows(true);
}

void Scene_Battle_Rpg2k3::CreateEnemySprites() {
	for (auto* enemy: Main_Data::game_enemyparty->GetEnemies()) {
		enemy->SetBattleSprite(std::make_unique<Sprite_Enemy>(enemy));
	}
}

void Scene_Battle_Rpg2k3::CreateActorSprites() {
	for (auto* actor: Main_Data::game_party->GetActors()) {
		actor->SetBattleSprite(std::make_unique<Sprite_Actor>(actor));
		actor->SetWeaponSprite(std::make_unique<Sprite_Weapon>(actor));
	}
}

void Scene_Battle_Rpg2k3::ResetAllBattlerZ() {
	for (auto* enemy: Main_Data::game_enemyparty->GetEnemies()) {
		auto* sprite = enemy->GetBattleSprite();
		if (sprite) {
			sprite->ResetZ();
		}
	}

	for (auto* actor: Main_Data::game_party->GetActors()) {
		auto* sprite = actor->GetActorBattleSprite();
		if (sprite) {
			sprite->ResetZ();
			sprite->UpdatePosition();
			sprite->DetectStateChange();
		}
	}
}

void Scene_Battle_Rpg2k3::UpdateAnimations() {
	for (auto it = floating_texts.begin(); it != floating_texts.end();) {
		int &time = it->remaining_time;

		if (time % 2 == 0) {
			int modifier = time <= 10 ? 1 :
						   time < 20 ? 0 :
						   -1;
			it->sprite->SetY(it->sprite->GetY() + modifier);
		}

		--time;
		if (time <= 0) {
			it = floating_texts.erase(it);
		} else {
			++it;
		}
	}

	if (running_away) {
		for (auto& actor: Main_Data::game_party->GetActors()) {
			Point p = actor->GetBattlePosition();
			if (actor->IsDirectionFlipped()) {
				p.x -= 6;
			} else {
				p.x += 6;
			}
			actor->SetBattlePosition(p);
		}
	}

	auto frame_counter = Main_Data::game_system->GetFrameCounter();

	bool ally_set = false;
	if (status_window->GetActive()
			&& lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_traditional)
	{
		auto* actor = Main_Data::game_party->GetActor(status_window->GetIndex());
		if (actor) {
			const auto* sprite = actor->GetBattleSprite();
			if (sprite) {
				static const int frames[] = { 0, 1, 2, 1 };
				int sprite_frame = frames[(frame_counter / 15) % 4];
				ally_cursor->SetSrcRect(Rect(sprite_frame * 16, 16, 16, 16));

				ally_cursor->SetVisible(true);
				ally_cursor->SetX(Player::menu_offset_x + actor->GetBattlePosition().x);
				ally_cursor->SetY(Player::menu_offset_y + actor->GetBattlePosition().y - 40);

				if (frame_counter % 30 == 0) {
					SelectionFlash(actor);
				}
				ally_set = true;
			}
		}
	}
	if (!ally_set) {
		ally_cursor->SetVisible(false);
	}

	bool enemy_set = false;
	if (target_window->GetActive()) {
		std::vector<Game_Battler*> battlers;
		Main_Data::game_enemyparty->GetActiveBattlers(battlers);
		auto idx = target_window->GetIndex();
		if (idx >= 0) {
			auto* enemy = battlers[idx];
			if (enemy) {
				const auto* sprite = enemy->GetBattleSprite();
				if (sprite) {
					static const int frames[] = { 0, 1, 2, 1 };
					int sprite_frame = frames[(frame_counter / 15) % 4];
					enemy_cursor->SetSrcRect(Rect(sprite_frame * 16, 0, 16, 16));

					enemy_cursor->SetVisible(true);
					enemy_cursor->SetX(Player::menu_offset_x + enemy->GetBattlePosition().x + sprite->GetWidth() / 2);
					enemy_cursor->SetY(Player::menu_offset_y + enemy->GetBattlePosition().y);

					std::vector<lcf::rpg::State*> ordered_states = enemy->GetInflictedStatesOrderedByPriority();
					if (ordered_states.size() > 0) {
						help_window->Clear();
						int state_counter = 0;
						for (lcf::rpg::State* state : ordered_states) {
							std::string state_name = fmt::format("{:9s}", state->name);
							help_window->AddText(state_name, state->color, Text::AlignLeft, false);
							if (++state_counter >= 5) break;
						}
						help_window->SetVisible(true);
					} else {
						help_window->SetVisible(false);
					}

					if (sprite_frame % 30 == 0) {
						SelectionFlash(enemy);
					}
					enemy_set = true;
				}
			}
		}
		if (!enemy_set) {
			help_window->Clear();
		}
	}

	if (!enemy_set) {
		enemy_cursor->SetVisible(false);
	}
}

void Scene_Battle_Rpg2k3::DrawFloatText(int x, int y, int color, StringView text) {
	Rect rect = Text::GetSize(*Font::Default(), text);

	BitmapRef graphic = Bitmap::Create(rect.width, rect.height);
	graphic->Clear();
	graphic->TextDraw(-rect.x, -rect.y, color, text);

	std::shared_ptr<Sprite> floating_text = std::make_shared<Sprite>();
	floating_text->SetBitmap(graphic);
	floating_text->SetOx(rect.width / 2);
	floating_text->SetOy(rect.height + 5);
	floating_text->SetX(Player::menu_offset_x + x);
	// Move 5 pixel down because the number "jumps" with the intended y as the peak
	floating_text->SetY(Player::menu_offset_y + y + 5);
	floating_text->SetZ(Priority_Window + y);

	FloatText float_text;
	float_text.sprite = floating_text;

	floating_texts.push_back(float_text);
}

bool Scene_Battle_Rpg2k3::IsTransparent() const {
	return lcf::Data::battlecommands.transparency == lcf::rpg::BattleCommands::Transparency_transparent;
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

void Scene_Battle_Rpg2k3::CreateBattleTargetWindow() {
	auto commands = GetEnemyTargetNames();

	int width = (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_traditional) ? 104 : 136;
	int height = 80;

	target_window.reset(new Window_Command(std::move(commands), width, 4));
	target_window->SetHeight(height);
	target_window->SetX(Player::menu_offset_x);
	target_window->SetY(Player::screen_height - Player::menu_offset_y - height);
	// Above other windows
	target_window->SetZ(Priority_Window + 10);

	if (lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_traditional) {
		int transp = IsTransparent() ? 160 : 255;
		target_window->SetBackOpacity(transp);
	}
}

void Scene_Battle_Rpg2k3::RefreshTargetWindow() {
	// FIXME: Handle live refresh in traditional when the window is always visible
	auto commands = GetEnemyTargetNames();
	target_window->ReplaceCommands(std::move(commands));
	if (!target_window->GetActive()) {
		target_window->SetIndex(-1);
	}
}

void Scene_Battle_Rpg2k3::CreateBattleStatusWindow() {
	int w = MENU_WIDTH;
	int h = 80;
	int x = Player::menu_offset_x;
	int y = Player::screen_height - Player::menu_offset_y - h;

	switch (lcf::Data::battlecommands.battle_type) {
		case lcf::rpg::BattleCommands::BattleType_traditional:
			x = Player::menu_offset_x + target_window->GetWidth();
			w = MENU_WIDTH - target_window->GetWidth();
			break;
		case lcf::rpg::BattleCommands::BattleType_alternative:
			x = Player::menu_offset_x + options_window->GetWidth();
			w = MENU_WIDTH - options_window->GetWidth();
			break;
		case lcf::rpg::BattleCommands::BattleType_gauge:
			x = Player::menu_offset_x + options_window->GetWidth();
			// Default window too small for 4 actors
			w = MENU_WIDTH;
			break;
	}

	status_window.reset(new Window_BattleStatus(x, y, w, h));
	status_window->SetZ(Priority_Window + 1);
}

std::vector<std::string> Scene_Battle_Rpg2k3::GetBattleCommandNames(const Game_Actor* actor) {
	std::vector<std::string> commands;
	if (actor) {
		for (auto* cmd: actor->GetBattleCommands()) {
			commands.push_back(ToString(cmd->name));
		}
	}
	if (Feature::HasRow() && lcf::Data::battlecommands.easyrpg_enable_battle_row_command) {
		commands.push_back(ToString(lcf::Data::terms.row));
	}

	return commands;
}

void Scene_Battle_Rpg2k3::SetBattleCommandsDisable(Window_Command& window, const Game_Actor* actor) {
	if (actor) {
		const auto& cmds = actor->GetBattleCommands();
		for (size_t i = 0; i < cmds.size(); ++i) {
			auto* cmd = cmds[i];
			if (cmd->type == lcf::rpg::BattleCommand::Type_escape && !IsEscapeAllowedFromActorCommand()) {
				window.DisableItem(i);
			} else {
				window.EnableItem(i);
			}
		}
	}
}

void Scene_Battle_Rpg2k3::CreateBattleCommandWindow() {
	auto* actor = Main_Data::game_party->GetActor(0);
	auto commands = GetBattleCommandNames(actor);

	command_window.reset(new Window_Command(std::move(commands), option_command_mov));

	SetBattleCommandsDisable(*command_window, actor);

	int height = 80;

	command_window->SetHeight(height);
	switch (lcf::Data::battlecommands.battle_type) {
		case lcf::rpg::BattleCommands::BattleType_traditional:
			command_window->SetX(Player::menu_offset_x + target_window->GetWidth() - command_window->GetWidth());
			command_window->SetY(Player::screen_height - Player::menu_offset_y - height);
			break;
		case lcf::rpg::BattleCommands::BattleType_alternative:
			command_window->SetX(Player::menu_offset_x + MENU_WIDTH);
			command_window->SetY(Player::screen_height - Player::menu_offset_y - height);
			break;
		case lcf::rpg::BattleCommands::BattleType_gauge:
			command_window->SetX(Player::menu_offset_x);
			command_window->SetY(Player::screen_height / 2 - height / 2);
			break;
	}
	// Above the target window
	command_window->SetZ(Priority_Window + 20);

	if (lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_traditional) {
		int transp = IsTransparent() ? 160 : 255;
		command_window->SetBackOpacity(transp);
	}
}

void Scene_Battle_Rpg2k3::RefreshCommandWindow(const Game_Actor* actor) {
	auto commands = GetBattleCommandNames(actor);
	command_window->ReplaceCommands(std::move(commands));
	SetBattleCommandsDisable(*command_window, actor);
	command_window->SetIndex(-1);
}

void Scene_Battle_Rpg2k3::SetActiveActor(int idx) {
#ifdef EP_DEBUG_BATTLE2K3_STATE_MACHINE
	Output::Debug("Battle2k3 SetActiveActor({}) frame={}", idx, Main_Data::game_system->GetFrameCounter());
#endif
	status_window->SetIndex(idx);
	active_actor = Main_Data::game_party->GetActor(idx);
	auto* display_actor = active_actor ? active_actor : Main_Data::game_party->GetActor(0);
	RefreshCommandWindow(display_actor);
}

void Scene_Battle_Rpg2k3::ResetWindows(bool make_invisible) {
	item_window->SetHelpWindow(nullptr);
	skill_window->SetHelpWindow(nullptr);

	options_window->SetActive(false);
	status_window->SetActive(false);
	command_window->SetActive(false);
	item_window->SetActive(false);
	skill_window->SetActive(false);
	target_window->SetActive(false);
	sp_window->SetActive(false);

	if (!make_invisible) {
		return;
	}

	options_window->SetVisible(false);
	status_window->SetVisible(false);
	command_window->SetVisible(false);
	target_window->SetVisible(false);
	item_window->SetVisible(false);
	skill_window->SetVisible(false);
	help_window->SetVisible(false);
	sp_window->SetVisible(false);
}

void Scene_Battle_Rpg2k3::MoveCommandWindows(int x, int frames) {
	if (lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_traditional) {
		options_window->InitMovement(options_window->GetX(), options_window->GetY(),
				x, options_window->GetY(), frames);

		x += options_window->GetWidth();

		status_window->InitMovement(status_window->GetX(), status_window->GetY(),
				x, status_window->GetY(), frames);

		if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_alternative) {
			x += status_window->GetWidth();
			command_window->InitMovement(command_window->GetX(), command_window->GetY(),
					x, command_window->GetY(), frames);
		}
	}
}

void Scene_Battle_Rpg2k3::SetState(Scene_Battle::State new_state) {
	previous_state = state;
	state = new_state;

	if (new_state == State_SelectActor) {
		auto_battle = false;
	}
	if (new_state == State_AutoBattle) {
		auto_battle = true;
	}

	SetSceneActionSubState(0);

#ifdef EP_DEBUG_BATTLE2K3_STATE_MACHINE
	Output::Debug("Battle2k3 SetState state={} prev={} auto_battle={}", state, previous_state, auto_battle);
#endif
}

void Scene_Battle_Rpg2k3::ReturnToMainBattleState() {
	SetState(auto_battle ? State_AutoBattle : State_SelectActor);
}

void Scene_Battle_Rpg2k3::SetSceneActionSubState(int substate) {
	scene_action_substate = substate;
}

static bool BattlerReadyToAct(const Game_Battler* battler) {
	return battler->IsAtbGaugeFull() && battler->Exists() && battler->CanAct();
}

void Scene_Battle_Rpg2k3::UpdateReadyActors() {
	const auto& actors = Main_Data::game_party->GetActors();
	for (auto actor: actors) {
		auto position = std::find(atb_order.begin(), atb_order.end(), actor->GetId());
		if (BattlerReadyToAct(actor)) {
			if (position == atb_order.end()) {
#ifdef EP_DEBUG_BATTLE2K3_STATE_MACHINE
				Output::Debug("Battle2k3 UpdateReadyActors add={} frame={}", actor->GetId(), Main_Data::game_system->GetFrameCounter());
#endif
				atb_order.push_back(actor->GetId());
			}
		} else {
			if (position != atb_order.end()) {
#ifdef EP_DEBUG_BATTLE2K3_STATE_MACHINE
				Output::Debug("Battle2k3 UpdateReadyActors remove={} frame={}", actor->GetId(), Main_Data::game_system->GetFrameCounter());
#endif
				atb_order.erase(position);
			}
		}
	}
}

int Scene_Battle_Rpg2k3::GetNextReadyActor() {
	if (!atb_order.empty()) {
#ifdef EP_DEBUG_BATTLE2K3_STATE_MACHINE
		Output::Debug("Battle2k3 GetNextReadyActor actor={} frame={}", atb_order.front(), Main_Data::game_system->GetFrameCounter());
#endif
		return Main_Data::game_party->GetActorPositionInParty(atb_order.front());
	}
#ifdef EP_DEBUG_BATTLE2K3_STATE_MACHINE
	Output::Debug("Battle2k3 GetNextReadyActor (none) frame={}", Main_Data::game_system->GetFrameCounter());
#endif
	return -1;
}


bool Scene_Battle_Rpg2k3::IsAtbAccumulating() const {
	if (Game_Battle::IsBattleAnimationWaiting()) {
		return false;
	}

	const bool active_atb = Main_Data::game_system->GetAtbMode() == lcf::rpg::SaveSystem::AtbMode_atb_active;

	switch(state) {
		case State_SelectEnemyTarget:
		case State_SelectAllyTarget:
		case State_SelectItem:
		case State_SelectSkill:
		case State_SelectCommand:
			return active_atb;
		case State_AutoBattle:
		case State_SelectActor:
			return true;
		default:
			break;
	}
	return false;
}

void Scene_Battle_Rpg2k3::CreateEnemyActions() {
	// FIXME: RPG_RT checks animations and event ready flag?
	for (auto* enemy: Main_Data::game_enemyparty->GetEnemies()) {
		if (enemy->IsAtbGaugeFull() && !enemy->GetBattleAlgorithm()) {
			if (!EnemyAi::SetStateRestrictedAction(*enemy)) {
				if (enemy->GetEnemyAi() == -1) {
					enemyai_algos[default_enemyai_algo]->SetEnemyAiAction(*enemy);
				} else {
					enemyai_algos[enemy->GetEnemyAi()]->SetEnemyAiAction(*enemy);
				}
			}
			assert(enemy->GetBattleAlgorithm() != nullptr);
			ActionSelectedCallback(enemy);
#ifdef EP_DEBUG_BATTLE2K3_STATE_MACHINE
			Output::Debug("Battle2k3 ScheduleEnemyAction name={} type={} frame={}", enemy->GetName(), static_cast<int>(enemy->GetBattleAlgorithm()->GetType()), Main_Data::game_system->GetFrameCounter());
#endif
		}
	}
}

void Scene_Battle_Rpg2k3::CreateActorAutoActions() {
	if (state != State_SelectActor
			&& state != State_AutoBattle
			&& state != State_Battle
			) {
		return;
	}

	// FIXME: RPG_RT checks only actor animations?
	for (auto* actor: Main_Data::game_party->GetActors()) {
		if (!BattlerReadyToAct(actor)
				|| actor->GetBattleAlgorithm()
				|| (actor->IsControllable() && state != State_AutoBattle)
				) {
			continue;
		}

		Game_Battler* random_target = nullptr;
		switch (actor->GetSignificantRestriction()) {
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
			actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Normal>(actor, random_target));
		} else {
			if (actor->GetActorAi() == -1) {
				this->autobattle_algos[default_autobattle_algo]->SetAutoBattleAction(*actor);
			} else {
				this->autobattle_algos[actor->GetActorAi()]->SetAutoBattleAction(*actor);
			}
			assert(actor->GetBattleAlgorithm() != nullptr);
		}

		actor->SetLastBattleAction(-1);
		ActionSelectedCallback(actor);
	}
}

bool Scene_Battle_Rpg2k3::UpdateAtb() {
	if (Game_Battle::GetInterpreter().IsRunning() || Game_Message::IsMessageActive()) {
		return true;
	}
	if (IsAtbAccumulating()) {
		// FIXME: If one monster can act now, he gets his battle algo set, and we abort updating atb for other monsters
		Game_Battle::UpdateAtbGauges();
	}

	CreateEnemyActions();
	CreateActorAutoActions();

	return true;
}

bool Scene_Battle_Rpg2k3::IsBattleActionPending() const {
	return !battle_actions.empty();
}

bool Scene_Battle_Rpg2k3::UpdateBattleState() {
	if (resume_from_debug_scene) {
		resume_from_debug_scene = false;
		return true;
	}

	UpdateScreen();
	// FIXME: RPG_RT updates actors first, and this goes into doing CBA actor battle actions initiated last frame
	UpdateBattlers();

	UpdateUi();

	const auto battle_ending = (state == State_Victory || state == State_Defeat);

	if (!battle_ending) {
		// FIXME: Interpreter also blocked by an RPG_RT continueBattle flag. What is this flag?
		if (!Game_Battle::IsBattleAnimationWaiting()) {
			if (!UpdateEvents()) {
				return false;
			}
		}
	}

	// FIXME: Update Panorama

	if (!battle_ending) {
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

		CheckBattleEndConditions();
		UpdateAtb();
	}
	return true;
}

void Scene_Battle_Rpg2k3::vUpdate() {
	const auto process_scene = UpdateBattleState();

	while (process_scene) {
		// Something ended the battle.
		if (Scene::instance.get() != this) {
			break;
		}

		if (IsWindowMoving()) {
			break;
		}

		if (Game_Message::IsMessageActive()) {
			break;
		}

		if (state != State_Victory && state != State_Defeat && Game_Battle::GetInterpreter().IsRunning()) {
			break;
		}

		if (!CheckWait()) {
			break;
		}

		if (ProcessSceneAction() == SceneActionReturn::eWaitTillNextFrame) {
			break;
		}
	}

	UpdateAnimations();
	UpdateGraphics();
}

void Scene_Battle_Rpg2k3::NextTurn(Game_Battler* battler) {
	Main_Data::game_party->IncTurns();
	battler->NextBattleTurn();
	Game_Battle::GetInterpreterBattle().ResetPagesExecuted();
}

bool Scene_Battle_Rpg2k3::CheckBattleEndConditions() {
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


bool Scene_Battle_Rpg2k3::CheckBattleEndAndScheduleEvents(EventTriggerType tt, Game_Battler* source) {
	auto& interp = Game_Battle::GetInterpreterBattle();

	if (interp.IsRunning()) {
		return false;
	}

	if (tt == EventTriggerType::eAfterBattleAction
			&& (Game_Message::IsMessageActive() || interp.IsWaitingForWaitCommand())) {
		return true;
	}

	if (CheckBattleEndConditions()) {
		return false;
	}

	lcf::rpg::TroopPageCondition::Flags flags;
	switch (tt) {
		case EventTriggerType::eBeforeBattleAction:
			flags.turn = flags.turn_actor = flags.turn_enemy = flags.command_actor = true;
			break;
		case EventTriggerType::eAfterBattleAction:
			flags.switch_a = flags.switch_b = flags.variable = flags.fatigue = flags.enemy_hp = flags.actor_hp = true;
			break;
		case EventTriggerType::eAll:
			for (auto& ff: flags.flags) ff = true;
			break;
	}

	int page = interp.ScheduleNextPage(flags, source);
#ifdef EP_DEBUG_BATTLE2K3_STATE_MACHINE
	if (page) {
		Output::Debug("Battle2k3 ScheduleNextEventPage Scheduled Page {} frame={}", page, Main_Data::game_system->GetFrameCounter());
	} else {
		Output::Debug("Battle2k3 ScheduleNextEventPage No Events to Run frame={}", Main_Data::game_system->GetFrameCounter());
	}
#else
	(void)page;
#endif
	RefreshTargetWindow();

	return !interp.IsRunning();
}

Scene_Battle_Rpg2k3::SceneActionReturn Scene_Battle_Rpg2k3::ProcessSceneAction() {
#ifdef EP_DEBUG_BATTLE2K3_STATE_MACHINE
	static int last_state = -1;
	static int last_substate = -1;
	if (state != last_state || scene_action_substate != last_substate) {
		int actor_id = active_actor ? active_actor->GetId() : 0;
		StringView actor_name = active_actor ? StringView(active_actor->GetName()) : "Null";
		Output::Debug("Battle2k3 ProcessSceneAction({}, {}) actor={}({}) frames={} auto_battle={}", state, scene_action_substate, actor_name, actor_id, Main_Data::game_system->GetFrameCounter(), auto_battle);
		last_state = state;
		last_substate = scene_action_substate;
	}
#endif

	// If actor was killed or event removed from the party, immediately cancel out of menu states
	if (active_actor && !active_actor->Exists()) {
		status_window->Refresh();
		SetActiveActor(-1);
		if (state != State_Battle) {
			ReturnToMainBattleState();
		}
	}

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

Scene_Battle_Rpg2k3::SceneActionReturn Scene_Battle_Rpg2k3::ProcessSceneActionStart() {
	enum SubState {
		eStartMessage,
		eSpecialMessage,
		eUpdateBattlers,
		eUpdateEvents,
	};

	if (scene_action_substate == eStartMessage) {
		ResetWindows(true);

		if (!lcf::Data::terms.battle_start.empty()) {
			ShowNotification(ToString(lcf::Data::terms.battle_start));
			SetWait(10, 80);
		}
		SetSceneActionSubState(eSpecialMessage);
		return SceneActionReturn::eContinueThisFrame;
	}

	if (scene_action_substate == eSpecialMessage) {
		EndNotification();
		const auto cond = Game_Battle::GetBattleCondition();
		if ((!lcf::Data::terms.special_combat.empty() || !lcf::Data::terms.easyrpg_battle2k3_special_combat_back.empty()) && (cond != lcf::rpg::System::BattleCondition_none || first_strike)) {
			if (!lcf::Data::terms.special_combat.empty() && (cond == lcf::rpg::System::BattleCondition_initiative || cond == lcf::rpg::System::BattleCondition_surround || (cond == lcf::rpg::System::BattleCondition_none && first_strike))) {
				ShowNotification(ToString(lcf::Data::terms.special_combat));
			}
			if (!lcf::Data::terms.easyrpg_battle2k3_special_combat_back.empty() && (cond == lcf::rpg::System::BattleCondition_back || cond == lcf::rpg::System::BattleCondition_pincers)) {
				ShowNotification(ToString(lcf::Data::terms.easyrpg_battle2k3_special_combat_back));
			}
			SetWait(30, 70);
		}
		SetSceneActionSubState(eUpdateBattlers);
		return SceneActionReturn::eContinueThisFrame;
	}

	if (scene_action_substate == eUpdateBattlers) {
		EndNotification();
		UpdateEnemiesDirection();
		UpdateActorsDirection();
		SetSceneActionSubState(eUpdateEvents);
		return SceneActionReturn::eContinueThisFrame;
	}

	if (scene_action_substate == eUpdateEvents) {
		if (!CheckBattleEndAndScheduleEvents(EventTriggerType::eAll, nullptr)) {
			return SceneActionReturn::eContinueThisFrame;
		}

		SetState(State_SelectOption);
		return SceneActionReturn::eContinueThisFrame;
	}

	return SceneActionReturn::eWaitTillNextFrame;
}

Scene_Battle_Rpg2k3::SceneActionReturn Scene_Battle_Rpg2k3::ProcessSceneActionFightAutoEscape() {
	enum SubState {
		eBegin,
		eWaitInput,
		ePreActor,
	};

	if (scene_action_substate == eBegin) {
		ResetWindows(true);
		target_window->SetIndex(-1);

		if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_traditional || ((std::find(battle_options.begin(), battle_options.end(), AutoBattle) == battle_options.end()) && !IsEscapeAllowedFromOptionWindow())) {
			if (lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_traditional) MoveCommandWindows(Player::menu_offset_x - options_window->GetWidth(), 1);
			SetState(State_SelectActor);
			return SceneActionReturn::eContinueThisFrame;
		}

		options_window->SetActive(true);

		if (IsEscapeAllowedFromOptionWindow()) {
			auto it = std::find(battle_options.begin(), battle_options.end(), Escape);
			if (it != battle_options.end()) {
				options_window->EnableItem(std::distance(battle_options.begin(), it));
			}
		} else {
			auto it = std::find(battle_options.begin(), battle_options.end(), Escape);
			if (it != battle_options.end()) {
				options_window->DisableItem(std::distance(battle_options.begin(), it));
			}
		}

		options_window->SetVisible(true);
		status_window->SetVisible(true);
		if (lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_gauge) {
			command_window->SetVisible(true);
		}
		SetActiveActor(-1);
		RefreshCommandWindow(Main_Data::game_party->GetActor(0));
		status_window->Refresh();
		command_window->SetIndex(-1);

		if (previous_state != State_Start) {
			MoveCommandWindows(Player::menu_offset_x, 8);
		}

		SetSceneActionSubState(eWaitInput);
		return SceneActionReturn::eContinueThisFrame;
	}

	if (scene_action_substate == eWaitInput) {
		if (Input::IsTriggered(Input::DECISION)) {
			if (message_window->IsVisible()) {
				return SceneActionReturn::eWaitTillNextFrame;
			}
			switch (battle_options[options_window->GetIndex()]) {
				case Battle: // Battle
					Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
					MoveCommandWindows(Player::menu_offset_x - options_window->GetWidth(), 8);
					SetState(State_SelectActor);
					break;
				case AutoBattle: // Auto Battle
					MoveCommandWindows(Player::menu_offset_x - options_window->GetWidth(), 8);
					SetState(State_AutoBattle);
					Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
					break;
				case Escape: // Escape
					if (IsEscapeAllowedFromOptionWindow()) {
						Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
						SetState(State_Escape);
					} else {
						Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
					}
					break;
			}
		}
		return SceneActionReturn::eWaitTillNextFrame;
	}

	return SceneActionReturn::eWaitTillNextFrame;
}

Scene_Battle_Rpg2k3::SceneActionReturn Scene_Battle_Rpg2k3::ProcessSceneActionActor() {
	enum SubState {
		eBegin,
		eWaitInput,
		eWaitActor,
	};

	if (scene_action_substate == eBegin) {
		ResetWindows(true);
		target_window->SetIndex(-1);

		status_window->SetVisible(true);
		command_window->SetIndex(-1);

		if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_traditional) {
			status_window->SetChoiceMode(Window_BattleStatus::ChoiceMode_None);
			target_window->SetVisible(true);

			SetSceneActionSubState(eWaitActor);
			return SceneActionReturn::eContinueThisFrame;
		}

		status_window->SetChoiceMode(Window_BattleStatus::ChoiceMode_Ready);

		if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_alternative) {
			command_window->SetVisible(true);
		}

		if (lcf::Data::battlecommands.easyrpg_sequential_order) {
			SetSceneActionSubState(eWaitActor);
			return SceneActionReturn::eContinueThisFrame;
		}

		SetSceneActionSubState(eWaitInput);
	}

	if (scene_action_substate == eWaitInput) {
		UpdateReadyActors();

		auto* selected_actor = Main_Data::game_party->GetActor(status_window->GetIndex());
		if (selected_actor == nullptr || !BattlerReadyToAct(selected_actor)) {
			// If current selection is no longer valid, force a new selection
			const auto idx = GetNextReadyActor();
			if (idx != status_window->GetIndex()) {
				SetActiveActor(idx);
			}
		} else if (selected_actor != active_actor) {
			// If selection changed due to player input
			SetActiveActor(status_window->GetIndex());
		}
		status_window->SetActive(active_actor != nullptr);

		if (lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_alternative) {
			command_window->SetVisible(status_window->GetActive());
		}
	}

	if (scene_action_substate == eWaitActor) {
		if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_traditional) {
			UpdateReadyActors();
			SetActiveActor(GetNextReadyActor());
		}
	}

	// If any battler is waiting to attack, immediately interrupt and do the attack.
	if (IsBattleActionPending()) {
		SetState(State_Battle);
		return SceneActionReturn::eContinueThisFrame;
	}

	if (scene_action_substate == eWaitInput) {
		if (Input::IsTriggered(Input::CANCEL)) {
			SetActiveActor(-1);
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
			SetState(State_SelectOption);
			return SceneActionReturn::eWaitTillNextFrame;
		}

		if (status_window->GetActive() && status_window->GetIndex() >= 0) {
			if (Input::IsTriggered(Input::DECISION)) {
				command_window->SetIndex(0);
				SetState(State_SelectCommand);
				return SceneActionReturn::eWaitTillNextFrame;
			}
		}

		return SceneActionReturn::eWaitTillNextFrame;
	}

	if (scene_action_substate == eWaitActor) {
		const auto idx = GetNextReadyActor();
		SetActiveActor(idx);
		if (idx >= 0) {
			command_window->SetIndex(0);
			SetState(State_SelectCommand);
			return SceneActionReturn::eWaitTillNextFrame;
		}

		if (lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_traditional) {
			if (Input::IsTriggered(Input::CANCEL)) {
				SetActiveActor(-1);
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
				SetState(State_SelectOption);
				return SceneActionReturn::eWaitTillNextFrame;
			}
		}

		return SceneActionReturn::eWaitTillNextFrame;
	}

	return SceneActionReturn::eWaitTillNextFrame;
}

Scene_Battle_Rpg2k3::SceneActionReturn Scene_Battle_Rpg2k3::ProcessSceneActionAutoBattle() {
	enum SubState {
		eBegin,
		eWaitInput,
	};

	if (scene_action_substate == eBegin) {
		ResetWindows(true);
		target_window->SetIndex(-1);

		status_window->SetVisible(true);
		command_window->SetIndex(-1);
		status_window->SetChoiceMode(Window_BattleStatus::ChoiceMode_None);
		if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_alternative) {
			command_window->SetVisible(true);
		}
		SetActiveActor(-1);

		SetSceneActionSubState(eWaitInput);
	}

	// If any battler is waiting to attack, immediately interrupt and do the attack.
	if (IsBattleActionPending()) {
		SetState(State_Battle);
		return SceneActionReturn::eContinueThisFrame;
	}

	if (scene_action_substate == eWaitInput) {
		if (Input::IsTriggered(Input::CANCEL)) {
			SetActiveActor(-1);
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
			SetState(State_SelectOption);
			return SceneActionReturn::eWaitTillNextFrame;
		}
		return SceneActionReturn::eWaitTillNextFrame;
	}

	return SceneActionReturn::eWaitTillNextFrame;
}

Scene_Battle_Rpg2k3::SceneActionReturn Scene_Battle_Rpg2k3::ProcessSceneActionCommand() {
	assert(active_actor != nullptr);
	enum SubState {
		eBegin,
		eWaitInput,
	};

	if (scene_action_substate == eBegin) {
		ResetWindows(true);
		target_window->SetIndex(-1);

		status_window->SetVisible(true);
		command_window->SetVisible(true);
		if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_traditional) {
			target_window->SetVisible(true);
		}
		command_window->SetActive(true);

		SetSceneActionSubState(eWaitInput);
	}

	// If any battler is waiting to attack, immediately interrupt and do the attack.
	if (Main_Data::game_system->GetAtbMode() == lcf::rpg::SaveSystem::AtbMode_atb_active && IsBattleActionPending()) {
		SetState(State_Battle);
		return SceneActionReturn::eContinueThisFrame;
	}

	if (scene_action_substate == eWaitInput) {
		if (Input::IsTriggered(Input::DECISION)) {
			int index = command_window->GetIndex();
			// Row command always uses the last index
			if (!Feature::HasRow() || !lcf::Data::battlecommands.easyrpg_enable_battle_row_command || index < command_window->GetRowMax() - 1) {
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
						case lcf::rpg::BattleCommand::Type_escape:
							EscapeSelected();
							break;
						case lcf::rpg::BattleCommand::Type_item:
							Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
							SetState(State_SelectItem);
							break;
						case lcf::rpg::BattleCommand::Type_skill:
							Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
							skill_window->SetSubsetFilter(0);
							RecreateSpWindow(active_actor);
							SetState(State_SelectSkill);
							break;
						case lcf::rpg::BattleCommand::Type_special:
							Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
							SpecialSelected();
							break;
						case lcf::rpg::BattleCommand::Type_subskill:
							Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
							SubskillSelected(command->ID);
							break;
					}
				}
			} else {
				active_actor->SetLastBattleAction(-1);
				// FIXME: Verify how battle interpreter runs with row command
				RowSelected();
			}
			return SceneActionReturn::eWaitTillNextFrame;
		}
		if (lcf::Data::battlecommands.battle_type != lcf::rpg::BattleCommands::BattleType_traditional) {
			if (Input::IsTriggered(Input::CANCEL)) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
				SetState(State_SelectOption);

				return SceneActionReturn::eWaitTillNextFrame;
			}
		}
		return SceneActionReturn::eWaitTillNextFrame;
	}
	return SceneActionReturn::eWaitTillNextFrame;
}

Scene_Battle_Rpg2k3::SceneActionReturn Scene_Battle_Rpg2k3::ProcessSceneActionItem() {
	assert(active_actor != nullptr);
	enum SubState {
		eBegin,
		eWaitInput,
	};

	if (scene_action_substate == eBegin) {
		ResetWindows(true);
		item_window->SetVisible(true);
		item_window->SetActive(true);
		item_window->SetActor(active_actor);

		item_window->SetHelpWindow(help_window.get());
		help_window->SetVisible(true);

		item_window->Refresh();

		if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_gauge) {
			status_window->SetVisible(true);
		}

		SetSceneActionSubState(eWaitInput);
	}

	if (scene_action_substate == eWaitInput) {
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

Scene_Battle_Rpg2k3::SceneActionReturn Scene_Battle_Rpg2k3::ProcessSceneActionSkill() {
	assert(active_actor != nullptr);
	enum SubState {
		eBegin,
		eWaitInput,
	};

	const auto actor_index = Main_Data::game_party->GetActorPositionInParty(active_actor->GetId());

	if (scene_action_substate == eBegin) {
		ResetWindows(true);

		skill_window->SetActive(true);
		skill_window->SetActor(active_actor->GetId());
		if (previous_state == State_SelectCommand) {
			skill_window->RestoreActorIndex(actor_index);
		}

		skill_window->SetVisible(true);
		skill_window->SetHelpWindow(help_window.get());
		help_window->SetVisible(true);
		if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_traditional) {
			sp_window->SetVisible(true);
		}
		if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_gauge) {
			status_window->SetVisible(true);
		}

		SetSceneActionSubState(eWaitInput);
	}

	skill_window->SaveActorIndex(actor_index);

	if (scene_action_substate == eWaitInput) {
		if (Input::IsTriggered(Input::DECISION)) {
			SkillSelected();
			skill_window->SaveActorIndex(actor_index);
			return SceneActionReturn::eWaitTillNextFrame;
		}
		if (Input::IsTriggered(Input::CANCEL)) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
			SetState(State_SelectCommand);
			skill_window->SaveActorIndex(actor_index);
			return SceneActionReturn::eWaitTillNextFrame;
		}
	}
	return SceneActionReturn::eWaitTillNextFrame;
}

Scene_Battle_Rpg2k3::SceneActionReturn Scene_Battle_Rpg2k3::ProcessSceneActionEnemyTarget() {
	assert(active_actor != nullptr);
	enum SubState {
		eBegin,
		eWaitInput,
	};

	if (scene_action_substate == eBegin) {
		RefreshTargetWindow();
		target_window->SetIndex(0);

		switch (lcf::Data::battlecommands.battle_type) {
			case lcf::rpg::BattleCommands::BattleType_traditional:
				ResetWindows(false);
				command_window->SetVisible(false);
				target_window->SetVisible(true);
				break;
			case lcf::rpg::BattleCommands::BattleType_alternative:
				ResetWindows(true);
				status_window->SetVisible(true);
				command_window->SetVisible(true);
				break;
			case lcf::rpg::BattleCommands::BattleType_gauge:
				ResetWindows(true);
				status_window->SetVisible(true);
				break;
		}

		target_window->SetActive(true);

		SetSceneActionSubState(eWaitInput);
	}

	if (scene_action_substate == eWaitInput) {
		if (Input::IsTriggered(Input::DECISION)) {
			auto* actor = active_actor;
			// active_actor gets reset after the next call, so save it.
			auto* enemy = EnemySelected();
			if (enemy) {
				FaceTarget(*actor, *enemy);
			}
			target_window->SetIndex(-1);
			return SceneActionReturn::eWaitTillNextFrame;
		}
		if (Input::IsTriggered(Input::CANCEL)) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
			SetState(previous_state);
			target_window->SetIndex(-1);
			return SceneActionReturn::eWaitTillNextFrame;
		}
	}
	return SceneActionReturn::eWaitTillNextFrame;
}

Scene_Battle_Rpg2k3::SceneActionReturn Scene_Battle_Rpg2k3::ProcessSceneActionAllyTarget() {
	assert(active_actor != nullptr);
	enum SubState {
		eBegin,
		eWaitInput,
	};

	if (scene_action_substate == eBegin) {
		switch (lcf::Data::battlecommands.battle_type) {
			case lcf::rpg::BattleCommands::BattleType_traditional:
				ResetWindows(false);
				status_window->SetVisible(true);
				break;
			case lcf::rpg::BattleCommands::BattleType_alternative:
				ResetWindows(true);
				status_window->SetVisible(true);
				command_window->SetVisible(true);
				command_window->SetIndex(-1);
				break;
			case lcf::rpg::BattleCommands::BattleType_gauge:
				ResetWindows(true);
				status_window->SetVisible(true);
				break;
		}

		status_window->SetActive(true);

		SetSceneActionSubState(eWaitInput);
	}

	if (scene_action_substate == eWaitInput) {
		if (Input::IsTriggered(Input::DECISION)) {
			AllySelected();
			return SceneActionReturn::eWaitTillNextFrame;
		}
		if (Input::IsTriggered(Input::CANCEL)) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Cancel));
			status_window->SetIndex(Main_Data::game_party->GetActorPositionInParty(active_actor->GetId()));
			SetState(previous_state);
			return SceneActionReturn::eWaitTillNextFrame;
		}
	}
	return SceneActionReturn::eWaitTillNextFrame;
}

Scene_Battle_Rpg2k3::SceneActionReturn Scene_Battle_Rpg2k3::ProcessSceneActionBattle() {
	enum SubState {
		eBegin,
		ePreAction,
		eBattleAction,
		ePostEvents,
		ePost,
	};

	if (scene_action_substate == eBegin) {
		ResetWindows(false);

		SetSceneActionSubState(ePreAction);
	}

	if (scene_action_substate == ePreAction) {
		if (battle_actions.empty()) {
			SetSceneActionSubState(ePost);
			return SceneActionReturn::eContinueThisFrame;
		}

		auto* battler = battle_actions.front();
		// If we will start a new battle action, first check for state changes
		// such as death, paralyze, confuse, etc..
		PrepareBattleAction(battler);

		pending_battle_action = battler->GetBattleAlgorithm();
		SetBattleActionState(BattleActionState_Begin);

		NextTurn(battler);

#ifdef EP_DEBUG_BATTLE2K3_STATE_MACHINE
		Output::Debug("Battle2k3 StartBattleAction battler={} frame={} auto_battle={}", battler->GetName(), Main_Data::game_system->GetFrameCounter(), auto_battle);
#endif
		SetSceneActionSubState(eBattleAction);
	}

	if (scene_action_substate == eBattleAction) {
		auto rc = ProcessBattleAction(pending_battle_action.get());
		// If interpreter or something else changed the battle state, cleanup before we abort.
		if (state != State_Battle) {
			pending_battle_action = {};
			RemoveCurrentAction();
		}
		if (rc == BattleActionReturn::eContinue) {
			return SceneActionReturn::eContinueThisFrame;
		}
		if (rc == BattleActionReturn::eWait) {
			return SceneActionReturn::eWaitTillNextFrame;
		}

		auto* battler = pending_battle_action->GetSource();
		assert(battler != active_actor);

		pending_battle_action = {};
		RemoveCurrentAction();

		// If battle ended, quit now
		if (CheckBattleEndConditions()) {
			return SceneActionReturn::eContinueThisFrame;
		}

		// Try next battler
		SetSceneActionSubState(ePreAction);
		return SceneActionReturn::eContinueThisFrame;
	}

	if (scene_action_substate == ePost) {
		// If the selected actor acted, or if they were killed / removed, then cancel out of their menus
		if (active_actor == nullptr || !active_actor->Exists()) {
			ReturnToMainBattleState();
		} else {
			SetState(previous_state);
		}
		return SceneActionReturn::eWaitTillNextFrame;
	}

	return SceneActionReturn::eWaitTillNextFrame;
}

Scene_Battle_Rpg2k3::SceneActionReturn Scene_Battle_Rpg2k3::ProcessSceneActionVictory() {
	enum SubState {
		eCBAInit,
		eCBAMove,
		eBegin,
		ePreMessage,
		eMessages,
		eEnd,
	};

	if (scene_action_substate == eCBAInit) {
		for (auto* actor: Main_Data::game_party->GetActors()) {
			auto* sprite = actor->GetActorBattleSprite();
			if (actor->Exists() && sprite) {
				sprite->SetNormalAttacking(false);
				auto* weapon = actor->GetWeaponSprite();
				if (weapon) {
					weapon->StopAttack();
				}
			}
		}

		if (cba_action != nullptr && cba_direction_back) {
			CBAInit();
			SetSceneActionSubState(eCBAMove);
		} else {
			SetSceneActionSubState(eBegin);
		}
		return SceneActionReturn::eWaitTillNextFrame;
	}

	if (scene_action_substate == eCBAMove) {
		CBAMove();
		if (cba_move_frame >= cba_num_move_frames) {
			SetSceneActionSubState(eBegin);
		}
		return SceneActionReturn::eWaitTillNextFrame;
	}

	if (scene_action_substate == eBegin) {
		ResetWindows(true);
		status_window->SetVisible(true);

		if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_traditional) {
			status_window->SetChoiceMode(Window_BattleStatus::ChoiceMode_None);
			target_window->SetVisible(true);
		}
		if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_alternative) {
			command_window->SetVisible(true);
		}
		battle_end_timer = 60;
		SetSceneActionSubState(ePreMessage);
	}

	if (scene_action_substate == ePreMessage) {
		if (battle_end_timer > 0) {
			--battle_end_timer;
			return SceneActionReturn::eContinueThisFrame;
		}

		for (auto* actor: Main_Data::game_party->GetActors()) {
			auto* sprite = actor->GetActorBattleSprite();
			if (actor->Exists() && sprite) {
				actor->SetIsDefending(false);
				sprite->SetAnimationState(Sprite_Actor::AnimationState_Victory);
			}
		}
		Main_Data::game_system->BgmPlay(Main_Data::game_system->GetSystemBGM(Main_Data::game_system->BGM_Victory));
		SetWait(30, 30);
		SetSceneActionSubState(eMessages);
		return SceneActionReturn::eContinueThisFrame;
	}

	if (scene_action_substate == eMessages) {
		int exp = Main_Data::game_enemyparty->GetExp();
		int money = Main_Data::game_enemyparty->GetMoney();
		std::vector<int> drops;
		Main_Data::game_enemyparty->GenerateDrops(drops);

		auto pm = PendingMessage();
		pm.SetEnableFace(false);

		pm.PushLine(ToString(lcf::Data::terms.victory) + Player::escape_symbol + "|");
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
		for (auto& item_id: drops) {
			const lcf::rpg::Item* item = lcf::ReaderUtil::GetElement(lcf::Data::items, item_id);
			// No Output::Warning needed here, reported later when the item is added
			StringView item_name = item ? StringView(item->name) : StringView("??? BAD ITEM ???");

			ss.str("");
			ss << item_name << space << lcf::Data::terms.item_recieved;
			pm.PushLine(ss.str());
			pm.PushPageEnd();
		}

		for (auto* actor: Main_Data::game_party->GetActors()) {
			if (actor->Exists()) {
				actor->ChangeExp(actor->GetExp() + exp, &pm);
			}
		}

		Main_Data::game_party->GainGold(money);
		for (auto& item: drops) {
			Main_Data::game_party->AddItem(item, 1);
		}

		message_window->SetHeight(32);
		message_window->SetMaxLinesPerPage(1);
		Game_Message::SetPendingMessage(std::move(pm));

		status_window->Refresh();

		SetSceneActionSubState(eEnd);
		return SceneActionReturn::eContinueThisFrame;
	}

	if (scene_action_substate == eEnd) {
		EndBattle(BattleResult::Victory);
		return SceneActionReturn::eContinueThisFrame;
	}

	return SceneActionReturn::eWaitTillNextFrame;
}

Scene_Battle_Rpg2k3::SceneActionReturn Scene_Battle_Rpg2k3::ProcessSceneActionDefeat() {
	enum SubState {
		eBegin,
		ePreMessage,
		eMessages,
		eEnd,
	};

	if (scene_action_substate == eBegin) {
		ResetWindows(true);
		status_window->SetVisible(true);

		if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_traditional) {
			status_window->SetChoiceMode(Window_BattleStatus::ChoiceMode_None);
			target_window->SetVisible(true);
		}
		if (lcf::Data::battlecommands.battle_type == lcf::rpg::BattleCommands::BattleType_alternative) {
			command_window->SetVisible(true);
		}

		battle_end_timer = 60;
		SetSceneActionSubState(ePreMessage);
	}

	if (scene_action_substate == ePreMessage) {
		if (battle_end_timer > 0) {
			--battle_end_timer;
			return SceneActionReturn::eContinueThisFrame;
		}
		Main_Data::game_system->BgmPlay(Main_Data::game_system->GetSystemBGM(Main_Data::game_system->BGM_GameOver));
		SetWait(60, 60);
		SetSceneActionSubState(eMessages);
		return SceneActionReturn::eContinueThisFrame;
	}

	if (scene_action_substate == eMessages) {
		Main_Data::game_system->SetMessagePositionFixed(true);
		Main_Data::game_system->SetMessagePosition(0);
		Main_Data::game_system->SetMessageTransparent(false);

		auto pm = PendingMessage();
		pm.SetEnableFace(false);
		pm.PushLine(ToString(lcf::Data::terms.defeat));

		message_window->SetHeight(32);
		message_window->SetMaxLinesPerPage(1);
		Game_Message::SetPendingMessage(std::move(pm));

		SetSceneActionSubState(eEnd);
		return SceneActionReturn::eContinueThisFrame;
	}

	if (scene_action_substate == eEnd) {
		EndBattle(BattleResult::Defeat);
		return SceneActionReturn::eContinueThisFrame;
	}

	return SceneActionReturn::eWaitTillNextFrame;
}

Scene_Battle_Rpg2k3::SceneActionReturn Scene_Battle_Rpg2k3::ProcessSceneActionEscape() {
	enum SubState {
		eBegin,
		eFailure,
		eSuccess,
	};

	if (scene_action_substate == eBegin) {
		if (previous_state == State_SelectOption || TryEscape()) {
			// There is no success text for escape in 2k3, however 2k3 still waits the same as if there was.
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Escape));
			for (auto& actor: Main_Data::game_party->GetActors()) {
				auto* sprite = actor->GetActorBattleSprite();
				if (sprite) {
					sprite->SetAnimationState(Sprite_Actor::AnimationState_WalkingRight);
				}
			}
			running_away = true;
			SetSceneActionSubState(eSuccess);
		} else {
			SetSceneActionSubState(eFailure);
			ShowNotification(ToString(lcf::Data::terms.escape_failure));
		}
		SetWait(10, 30);
		return SceneActionReturn::eContinueThisFrame;
	}

	if (scene_action_substate == eFailure) {
		EndNotification();
		ReturnToMainBattleState();
		return SceneActionReturn::eContinueThisFrame;
	}

	if (scene_action_substate == eSuccess) {
		EndNotification();
		EndBattle(BattleResult::Escape);
		return SceneActionReturn::eContinueThisFrame;
	}

	return SceneActionReturn::eWaitTillNextFrame;
}

static int AdjustPoseForDirection(const Game_Battler* battler, int pose) {
	if (battler->IsDirectionFlipped()) {
		switch (pose) {
			case lcf::rpg::BattlerAnimation::Pose_AttackRight:
				return lcf::rpg::BattlerAnimation::Pose_AttackLeft;
			case lcf::rpg::BattlerAnimation::Pose_AttackLeft:
				return lcf::rpg::BattlerAnimation::Pose_AttackRight;
			case lcf::rpg::BattlerAnimation::Pose_WalkRight:
				return lcf::rpg::BattlerAnimation::Pose_WalkLeft;
			case lcf::rpg::BattlerAnimation::Pose_WalkLeft:
				return lcf::rpg::BattlerAnimation::Pose_WalkRight;
		}
	}
	return pose;
}

void Scene_Battle_Rpg2k3::SetBattleActionState(BattleActionState state) {
	battle_action_state = state;
}

Scene_Battle_Rpg2k3::BattleActionReturn Scene_Battle_Rpg2k3::ProcessBattleAction(Game_BattleAlgorithm::AlgorithmBase* action) {
	// End any notification started by battle action
	EndNotification();
	auto* source = action->GetSource();

	if (action == nullptr) {
		return BattleActionReturn::eFinished;
	}

	// Immediately quit for dead actors no move. Prevents any animations or delays.
	if (action->GetType() == Game_BattleAlgorithm::Type::None && action->GetSource()->IsDead()) {
		return BattleActionReturn::eFinished;
	}

	if (Game_Battle::IsBattleAnimationWaiting() && !(action->GetType() == Game_BattleAlgorithm::Type::Normal && source->GetType() == Game_Battler::Type_Enemy)) {
		return BattleActionReturn::eWait;
	}

	if (source->GetType() == Game_Battler::Type_Ally) {
		auto* sprite = static_cast<Game_Actor*>(source)->GetActorBattleSprite();
		if (sprite && !sprite->IsIdling()) {
			switch (battle_action_state) {
				case BattleActionState_CBAMove:
				case BattleActionState_StartAnimation:
				case BattleActionState_CBARangedWeaponInit:
				case BattleActionState_CBARangedWeaponMove:
				case BattleActionState_Animation:
					break;
				default:
					return BattleActionReturn::eWait;
			}
		}
	}

#ifdef EP_DEBUG_BATTLE2K3_STATE_MACHINE
	static int last_state = -1;
	if (battle_action_state != last_state) {
		auto* source = action->GetSource();
		Output::Debug("Battle2k3 ProcessBattleAction({}, {}) actor={}({}) frames={} auto_battle={}", action->GetSource()->GetName(), battle_action_state,
				source->GetName(), source->GetId(),
				Main_Data::game_system->GetFrameCounter(), auto_battle);
		last_state = battle_action_state;
	}
#endif

	switch (battle_action_state) {
		case BattleActionState_Begin:
			return ProcessBattleActionBegin(action);
		case BattleActionState_PreEvents:
			return ProcessBattleActionPreEvents(action);
		case BattleActionState_Conditions:
			return ProcessBattleActionConditions(action);
		case BattleActionState_Notify:
			return ProcessBattleActionNotify(action);
		case BattleActionState_Combo:
			return ProcessBattleActionCombo(action);
		case BattleActionState_StartAlgo:
			return ProcessBattleActionStartAlgo(action);
		case BattleActionState_CBAInit:
			return ProcessBattleActionCBAInit(action);
		case BattleActionState_CBAMove:
			return ProcessBattleActionCBAMove(action);
		case BattleActionState_StartAnimation:
			return ProcessBattleActionStartAnimation(action);
		case BattleActionState_CBARangedWeaponInit:
			return ProcessBattleActionCBARangedWeaponInit(action);
		case BattleActionState_CBARangedWeaponMove:
			return ProcessBattleActionCBARangedWeaponMove(action);
		case BattleActionState_Animation:
			return ProcessBattleActionAnimation(action);
		case BattleActionState_AnimationReflect:
			return ProcessBattleActionAnimationReflect(action);
		case BattleActionState_FinishPose:
			return ProcessBattleActionFinishPose(action);
		case BattleActionState_Execute:
			return ProcessBattleActionExecute(action);
		case BattleActionState_SwitchEvents:
			return ProcessBattleActionSwitchEvents(action);
		case BattleActionState_Apply:
			return ProcessBattleActionApply(action);
		case BattleActionState_PostAction:
			return ProcessBattleActionPostAction(action);
		case BattleActionState_PostEvents:
			return ProcessBattleActionPostEvents(action);
		case BattleActionState_Finished:
			return ProcessBattleActionFinished(action);
	}

	assert(false && "Invalid BattleActionState!");

	return BattleActionReturn::eFinished;
}

Scene_Battle_Rpg2k3::BattleActionReturn Scene_Battle_Rpg2k3::ProcessBattleActionBegin(Game_BattleAlgorithm::AlgorithmBase* action) {
	auto* source = action->GetSource();

	// Emulate an RPG_RT bug where whenver actors attack, the damage and evasion calculations are performed
	// as if the enemies are in the front row.
	if (source->GetType() == Game_Battler::Type_Ally) {
		if (action->GetType() == Game_BattleAlgorithm::Type::Normal) {
			static_cast<Game_BattleAlgorithm::Normal*>(action)->SetTreatEnemiesAsIfInFrontRow(true);
		} else if (action->GetType() == Game_BattleAlgorithm::Type::Skill) {
			static_cast<Game_BattleAlgorithm::Skill*>(action)->SetTreatEnemiesAsIfInFrontRow(true);
		}
	}

	// Setup enemy targets
	// FIXME: This is not 100% bug compatible with RPG_RT but pretty close
	// See: https://github.com/EasyRPG/Player/issues/2405#issuecomment-716298981
	if (source->GetType() == Game_Battler::Type_Ally) {
		auto& interp = Game_Battle::GetInterpreterBattle();
		auto* actor = static_cast<Game_Actor*>(source);
		interp.SetCurrentActingActorId(actor->GetId());

		if (action->GetType() == Game_BattleAlgorithm::Type::Normal
				|| action->GetType() == Game_BattleAlgorithm::Type::Skill
				|| action->GetType() == Game_BattleAlgorithm::Type::Item)
		{
			auto* original_target = action->GetOriginalSingleTarget();

			if (original_target && original_target->GetType() == Game_Battler::Type_Enemy) {
				auto* enemy = static_cast<Game_Enemy*>(original_target);
				interp.SetCurrentEnemyTargetIndex(Main_Data::game_enemyparty->GetEnemyPositionInParty(enemy));
				interp.SetCurrentActionTargetsSingleEnemy(true);
			} else {
				interp.SetCurrentActionTargetsSingleEnemy(false);
			}
		}
	}
	// Enemy doesn't change the values, and inherits whatever the last actor did...
	// Defend, row, etc.. is similar..

	SetBattleActionState(BattleActionState_PreEvents);
	return BattleActionReturn::eContinue;
}


Scene_Battle_Rpg2k3::BattleActionReturn Scene_Battle_Rpg2k3::ProcessBattleActionPreEvents(Game_BattleAlgorithm::AlgorithmBase* action) {
	auto* source = action->GetSource();

	// RPG_RT always runs the interpreter before starting the action.
	if (!CheckBattleEndAndScheduleEvents(EventTriggerType::eBeforeBattleAction, source)) {
		return BattleActionReturn::eContinue;
	}

	// If any battle animation is running for any reason, RPG_RT waits until the animation finishes.
	// This also means that the interpreter can run again.
	if (Game_Battle::IsBattleAnimationWaiting()) {
		return BattleActionReturn::eWait;
	}

	// If the event made the current action ususable, such as MP loss or silence etc..
	PrepareBattleAction(source);
	pending_battle_action = source->GetBattleAlgorithm();
	action = pending_battle_action.get();

	// Now perform filtering. RPG_RT will run events but will early abort the battle algo if any of the following conditions hold.
	// FIXME: RPG_RT doesn't actually check hidden (maybe it's impossible?) But we do it here for extensions.
	// FIXME: RPG_RT doesn't check for dead enemies, only actors. Why?
	if (source->IsHidden()
			|| !source->IsInParty()
			|| !source->CanActOrRecoverable()
			) {
		return BattleActionReturn::eFinished;
	}

	// RPG_RT cancels all enemy actions when first_strike flag is still active. This is different than
	// initiative / surround, unless the flag is set for those too.
	if (source->GetType() == Game_Battler::Type_Enemy
			&& !source->Exists()
			&& first_strike) {
		return BattleActionReturn::eFinished;
	}

	if (source->GetType() == Game_Battler::Type_Enemy) {
		if (action->GetType() != Game_BattleAlgorithm::Type::None
				&& action->GetType() != Game_BattleAlgorithm::Type::DoNothing) {
			if (Player::IsEnglish() || Player::IsPatchDynRpg()) {
				Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Game_System::SFX_EnemyAttacks));
			}
			source->Flash(31, 31,31, 32, 48);
		}
	}

	SetBattleActionState(BattleActionState_Conditions);
	return BattleActionReturn::eContinue;
}

Scene_Battle_Rpg2k3::BattleActionReturn Scene_Battle_Rpg2k3::ProcessBattleActionConditions(Game_BattleAlgorithm::AlgorithmBase* action) {
	(void)action;

	std::vector<Game_Battler*> battlers;
	Main_Data::game_party->GetActiveBattlers(battlers);
	Main_Data::game_enemyparty->GetActiveBattlers(battlers);

	for (auto* b : battlers) {
		b->BattleStateHeal();
		int damageTaken = b->ApplyConditions();
		if (damageTaken != 0) {
			DrawFloatText(
					b->GetBattlePosition().x,
					b->GetBattlePosition().y,
					damageTaken < 0 ? Font::ColorDefault : Font::ColorHeal,
					std::to_string(std::abs(damageTaken)));
		}
		if (b->GetType() == Game_Battler::Type_Ally) {
			auto* sprite = static_cast<Game_Actor*>(b)->GetActorBattleSprite();
			if (sprite) {
				sprite->DetectStateChange();
			}
		}
	}

	status_window->Refresh();

	SetBattleActionState(BattleActionState_Notify);
	return BattleActionReturn::eContinue;
}

Scene_Battle_Rpg2k3::BattleActionReturn Scene_Battle_Rpg2k3::ProcessBattleActionNotify(Game_BattleAlgorithm::AlgorithmBase* action) {
	std::string notification = action->GetStartMessage(0);
	ShowNotification(notification);
	if (!notification.empty()) {
		if (action->GetType() == Game_BattleAlgorithm::Type::Skill) {
			SetWait(15, 50);
		} else {
			SetWait(10, 40);
		}
	}

	SetBattleActionState(BattleActionState_Combo);
	return BattleActionReturn::eContinue;
}

Scene_Battle_Rpg2k3::BattleActionReturn Scene_Battle_Rpg2k3::ProcessBattleActionCombo(Game_BattleAlgorithm::AlgorithmBase* action) {
	auto* source = action->GetSource();
	if (source->GetType() == Game_Battler::Type_Ally) {
		auto* actor = static_cast<Game_Actor*>(source);
		auto combo_cmd = actor->GetBattleComboCommand();
		auto combo_times = actor->GetBattleComboTimes();

		if (combo_times > 1 && combo_cmd >= 0 && combo_cmd == actor->GetLastBattleAction()) {
			auto* cmd = lcf::ReaderUtil::GetElement(lcf::Data::battlecommands.commands, combo_cmd);
			if (cmd && (cmd->type == lcf::rpg::BattleCommand::Type_attack
						|| cmd->type == lcf::rpg::BattleCommand::Type_skill
						|| cmd->type == lcf::rpg::BattleCommand::Type_subskill))
			{
				// RPG_RT doesn't allow combo for item or other actions other than attack and skills.
				action->ApplyComboHitsMultiplier(combo_times);
			}
		}
	}
	SetBattleActionState(BattleActionState_StartAlgo);
	return BattleActionReturn::eContinue;
}

Scene_Battle_Rpg2k3::BattleActionReturn Scene_Battle_Rpg2k3::ProcessBattleActionStartAlgo(Game_BattleAlgorithm::AlgorithmBase* action) {
	const auto is_target_party = action->GetOriginalPartyTarget() != nullptr;
	auto* source = action->GetSource();

	action->Start();

	// Drop out of the battle state machine to process actor escape.
	if (action->GetType() == Game_BattleAlgorithm::Type::Escape && source->GetType() == Game_Battler::Type_Ally) {
		SetState(State_Escape);
		return BattleActionReturn::eContinue;
	}

	// FIXME: This needs to be attached to the monster target window.
	// Counterexample is weapon with attack all, engine still makes you target a specific enemy,
	// even though your weapon will hit all enemies.
	if (action->GetSource()->GetType() == Game_Battler::Type_Ally
			&& !is_target_party
			&& action->GetTarget()
			&& action->GetTarget()->GetType() == Game_Battler::Type_Enemy)
	{
		auto* actor = static_cast<Game_Actor*>(action->GetSource());
		FaceTarget(*actor, *action->GetTarget());
	}

	if (action->GetCurrentRepeat() == 0 && action->GetCBAMovement() != lcf::rpg::BattlerAnimationItemSkill::Movement_none && source->GetType() == Game_Battler::Type_Ally) {
		cba_action = action;
		cba_direction_back = false;
		SetBattleActionState(BattleActionState_CBAInit);
	} else {
		SetBattleActionState(BattleActionState_StartAnimation);
	}
	return BattleActionReturn::eWait;
}

Scene_Battle_Rpg2k3::BattleActionReturn Scene_Battle_Rpg2k3::ProcessBattleActionCBAInit(Game_BattleAlgorithm::AlgorithmBase*) {
	CBAInit();

	SetBattleActionState(BattleActionState_CBAMove);
	return BattleActionReturn::eWait;
}

Scene_Battle_Rpg2k3::BattleActionReturn Scene_Battle_Rpg2k3::ProcessBattleActionCBAMove(Game_BattleAlgorithm::AlgorithmBase*) {
	CBAMove();

	if (cba_move_frame >= cba_num_move_frames) {
		if (cba_direction_back) {
			SetBattleActionState(BattleActionState_PostAction);
		} else {
			SetBattleActionState(BattleActionState_StartAnimation);
		}
	}
	return BattleActionReturn::eWait;
}

Scene_Battle_Rpg2k3::BattleActionReturn Scene_Battle_Rpg2k3::ProcessBattleActionStartAnimation(Game_BattleAlgorithm::AlgorithmBase* action) {
	auto* source = action->GetSource();
	bool ranged_weapon = false;

	if (source->GetType() == Game_Battler::Type_Ally) {
		auto* actor = static_cast<Game_Actor*>(source);
		auto* sprite = actor->GetActorBattleSprite();
		if (sprite) {
			const auto pose = AdjustPoseForDirection(action->GetSource(), action->GetSourcePose());
			if (pose != lcf::rpg::BattlerAnimation::Pose_Idle) {
				auto action_state = static_cast<Sprite_Actor::AnimationState>(pose + 1);

				if (action->GetType() == Game_BattleAlgorithm::Type::Normal) {
					sprite->SetNormalAttacking(true);
					auto* weapon = actor->GetWeaponSprite();
					int weapon_animation_id = 0;
					if (weapon) {
						auto* weapon_animation_data = action->GetWeaponAnimationData();
						if (weapon_animation_data) {
							if (weapon_animation_data->type == lcf::rpg::BattlerAnimationItemSkill::AnimType_weapon) {
								if (weapon_animation_data->weapon_animation_id >= 0) {
									weapon->SetWeaponAnimation(weapon_animation_data->weapon_animation_id + 1);
									weapon->StartAttack(action->GetSourcePose() == lcf::rpg::BattlerAnimation::Pose_AttackLeft);
								}
							} else {
								if (weapon_animation_data->type == lcf::rpg::BattlerAnimationItemSkill::AnimType_battle) {
									weapon_animation_id = weapon_animation_data->battle_animation_id;
								}
							}
							if (weapon_animation_data->ranged) {
								ranged_weapon = true;
							}
						}
					}
					sprite->SetAnimationState(
							action_state,
							Sprite_Actor::LoopState_WaitAfterFinish,
							weapon_animation_id);
				} else {
					sprite->SetAnimationState(
							action_state,
							Sprite_Actor::LoopState_WaitAfterFinish);
				}
			} else {
				sprite->SetAnimationLoop(Sprite_Actor::LoopState_DefaultAnimationAfterFinish);
			}
		}
	}

	if (ranged_weapon) {
		SetBattleActionState(BattleActionState_CBARangedWeaponInit);
	} else {
		SetBattleActionState(BattleActionState_Animation);
	}
	return BattleActionReturn::eWait;
}

Scene_Battle_Rpg2k3::BattleActionReturn Scene_Battle_Rpg2k3::ProcessBattleActionCBARangedWeaponInit(Game_BattleAlgorithm::AlgorithmBase* action) {
	auto* source = action->GetSource();
	cba_ranged_weapon_move_frame = 0;
	cba_ranged.clear();

	if (source->GetType() == Game_Battler::Type_Ally) {
		auto* actor = static_cast<Game_Actor*>(source);

		if (action->GetType() == Game_BattleAlgorithm::Type::Normal) {
			auto* weapon_animation_data = action->GetWeaponAnimationData();
			if (weapon_animation_data) {
				cba_num_ranged_weapon_move_frames = (weapon_animation_data->ranged_speed + 1) * 20;

				// The ranged weapon animation targets the original single target
				// if the weapon has the "Attack All" flag set and the ranged attack
				// range is set to "Single Enemy"
				if (action->GetWeaponData()->attack_all) {
					if (action->GetWeaponData()->ranged_target == lcf::rpg::Item::Target_single) {
						cba_ranged.emplace_back(*action->GetOriginalSingleTarget(), nullptr);
					} else if (action->GetWeaponData()->ranged_target == lcf::rpg::Item::Target_center) {
						std::vector<Game_Battler*> enemies;
						Main_Data::game_enemyparty->GetActiveBattlers(enemies);
						int x = 0;
						int y = 0;
						for (Game_Battler* enemy : enemies) {
							x += enemy->GetBattlePosition().x;
							y += enemy->GetBattlePosition().y;
						}
						if (enemies.size() > 0) {
							x /= enemies.size();
							y /= enemies.size();
						}
						cba_ranged_center = Point(x, y);

						// This is needed to make the ranged weapon animation appear
						// even if the real target of the animation is the center
						cba_ranged.emplace_back(*action->GetTarget(), nullptr);
					} else if (action->GetWeaponData()->ranged_target == lcf::rpg::Item::Target_simultaneous) {
						std::vector<Game_Battler*> battlers;
						Main_Data::game_enemyparty->GetActiveBattlers(battlers);
						for (auto& b: battlers) {
							cba_ranged.emplace_back(*b, nullptr);
						}
					}
				} else {
					assert(action->GetTarget());
					cba_ranged.emplace_back(*action->GetTarget(), nullptr);
				}

				for (auto& it: cba_ranged) {
					std::unique_ptr<Sprite_Weapon> cba_ranged_weapon = std::make_unique<Sprite_Weapon>(actor);
					cba_ranged_weapon->SetWeaponAnimation(weapon_animation_data->ranged_animation_id + 1);
					cba_ranged_weapon->SetRanged(true);
					cba_ranged_weapon->StartAttack(action->GetSourcePose() == lcf::rpg::BattlerAnimation::Pose_AttackLeft);
					cba_ranged_weapon->Update();
					it.second = std::move(cba_ranged_weapon);
				}
			}
		}
	}

	SetBattleActionState(BattleActionState_CBARangedWeaponMove);
	return BattleActionReturn::eWait;
}

Scene_Battle_Rpg2k3::BattleActionReturn Scene_Battle_Rpg2k3::ProcessBattleActionCBARangedWeaponMove(Game_BattleAlgorithm::AlgorithmBase* action) {
	auto* source = action->GetSource();

	if (cba_ranged_weapon_move_frame < cba_num_ranged_weapon_move_frames) {
		cba_ranged_weapon_move_frame++;
		for (auto& it: cba_ranged) {
			int offset_x = 0;
			int offset_y = 0;
			if (action->GetWeaponData()->ranged_target == lcf::rpg::Item::Target_center && action->GetWeaponData()->attack_all) {
				offset_x = cba_ranged_center.x - source->GetBattlePosition().x;
				offset_y = cba_ranged_center.y - source->GetBattlePosition().y;
			} else {
				auto& battler = it.first;
				offset_x = battler.GetBattlePosition().x - source->GetBattlePosition().x;
				offset_y = battler.GetBattlePosition().y - source->GetBattlePosition().y;
			}
			auto& weapon = it.second;
			assert(weapon);
			weapon->SetX(source->GetBattlePosition().x + (offset_x * cba_ranged_weapon_move_frame / cba_num_ranged_weapon_move_frames));
			weapon->SetY(source->GetBattlePosition().y + (offset_y * cba_ranged_weapon_move_frame / cba_num_ranged_weapon_move_frames));
			weapon->Update();
		}
	}

	if (cba_ranged_weapon_move_frame >= cba_num_ranged_weapon_move_frames) {
		for (auto& it: cba_ranged) {
			auto& weapon = it.second;
			weapon->StopAttack();
			weapon = nullptr;
		}
		SetBattleActionState(BattleActionState_Animation);
	}

	return BattleActionReturn::eWait;
}

Scene_Battle_Rpg2k3::BattleActionReturn Scene_Battle_Rpg2k3::ProcessBattleActionAnimation(Game_BattleAlgorithm::AlgorithmBase* action) {
	const auto anim_id = action->GetAnimationId(0);
	if (anim_id) {
		action->PlayAnimation(anim_id, false, -1, CheckAnimFlip(action->GetSource()));
	}
	if (action->GetStartSe()) {
		Main_Data::game_system->SePlay(*action->GetStartSe());
	}

	if (action->GetCBAMovement() != lcf::rpg::BattlerAnimationItemSkill::Movement_none) {
		cba_direction_back = true;
	}

	if (action->ReflectTargets()) {
		SetBattleActionState(BattleActionState_AnimationReflect);
	} else {
		SetBattleActionState(BattleActionState_Execute);
	}
	return BattleActionReturn::eContinue;
}

Scene_Battle_Rpg2k3::BattleActionReturn Scene_Battle_Rpg2k3::ProcessBattleActionAnimationReflect(Game_BattleAlgorithm::AlgorithmBase* action) {
	const auto anim_id = action->GetAnimationId(0);
	if (anim_id) {
		assert(action->GetReflectTarget());
		action->PlayAnimation(anim_id, false, -1, CheckAnimFlip(action->GetReflectTarget()));
	}
	SetBattleActionState(BattleActionState_FinishPose);
	return BattleActionReturn::eContinue;
}

Scene_Battle_Rpg2k3::BattleActionReturn Scene_Battle_Rpg2k3::ProcessBattleActionFinishPose(Game_BattleAlgorithm::AlgorithmBase* action) {
	auto* source = action->GetSource();
	if (source->GetType() == Game_Battler::Type_Ally) {
		auto* sprite = static_cast<Game_Actor*>(source)->GetActorBattleSprite();
		if (sprite) {
			sprite->SetAnimationLoop(Sprite_Actor::LoopState_DefaultAnimationAfterFinish);
		}
	}

	SetBattleActionState(BattleActionState_Execute);
	return BattleActionReturn::eContinue;
}

Scene_Battle_Rpg2k3::BattleActionReturn Scene_Battle_Rpg2k3::ProcessBattleActionExecute(Game_BattleAlgorithm::AlgorithmBase* action) {
	if (!action->IsCurrentTargetValid()) {
		if (action->GetCBAMovement() != lcf::rpg::BattlerAnimationItemSkill::Movement_none) {
			SetBattleActionState(BattleActionState_CBAInit);
		} else {
			SetBattleActionState(BattleActionState_PostAction);
		}
		return BattleActionReturn::eContinue;
	}

	auto* source = action->GetSource();
	if (source->GetType() == Game_Battler::Type_Ally) {
		auto* sprite = static_cast<Game_Actor*>(source)->GetActorBattleSprite();
		if (sprite) {
			sprite->SetAnimationLoop(Sprite_Actor::LoopState_DefaultAnimationAfterFinish);
		}
	}

	action->Execute();
	action->ApplyCustomEffect();
	action->ApplySwitchEffect();

	if (action->GetAffectedSwitch() > 0) {
		SetBattleActionState(BattleActionState_SwitchEvents);
	} else {
		SetBattleActionState(BattleActionState_Apply);
	}
	return BattleActionReturn::eContinue;
}

Scene_Battle_Rpg2k3::BattleActionReturn Scene_Battle_Rpg2k3::ProcessBattleActionSwitchEvents(Game_BattleAlgorithm::AlgorithmBase* action) {
	// RPG_RT always runs the interpreter before starting the action.
	if (!CheckBattleEndAndScheduleEvents(EventTriggerType::eAfterBattleAction, action->GetSource())) {
		return BattleActionReturn::eContinue;
	}

	SetBattleActionState(BattleActionState_Apply);
	return BattleActionReturn::eContinue;
}

Scene_Battle_Rpg2k3::BattleActionReturn Scene_Battle_Rpg2k3::ProcessBattleActionApply(Game_BattleAlgorithm::AlgorithmBase* action) {
	auto* source = action->GetSource();
	auto* target = action->GetTarget();

	Sprite_Actor* target_sprite = nullptr;
	if (target->GetType() == Game_Battler::Type_Ally) {
		target_sprite = static_cast<Game_Actor*>(target)->GetActorBattleSprite();
	}

	const bool was_dead = target->IsDead();


	const bool was_absorb_hp = action->IsAbsorbHp();
	// Emulates an RPG_RT bug where inverted absorb damage doesn't absorb hp anymore.
	// This bug only affects hp, not sp.
	if (action->IsAbsorbHp() && action->GetAffectedHp() > 0) {
		action->SetIsAbsorbHp(false);
	}

	action->ApplyHpEffect();

	// Emulates an RPG_RT bug where damage which is reversed into healing due to negative attributes is applied twice.
	// The displayed numbers are normal, but the actual effect is doubled.
	if (!action->IsPositive() && !was_absorb_hp && action->IsAffectHp() && action->GetAffectedHp() > 0) {
		action->ApplyHpEffect();
	}

	action->ApplySpEffect();
	action->ApplyAtkEffect();
	action->ApplyDefEffect();
	action->ApplySpiEffect();
	action->ApplyAgiEffect();
	action->ApplyStateEffects();
	action->ApplyAttributeShiftEffects();

	if (action->IsSuccess() && action->IsAffectHp() && action->GetAffectedHp() <= 0) {
		if (target->GetType() == Game_Battler::Type_Enemy) {
			auto* enemy = static_cast<Game_Enemy*>(target);
			enemy->SetBlinkTimer();
		} else if (action->GetAffectedHp() < 0) {
			if (!target->IsDead()) {
				target_sprite->SetAnimationState(Sprite_Actor::AnimationState_Damage, Sprite_Actor::LoopState_DefaultAnimationAfterFinish);
			}
		}
	}

	if (!was_dead && target->GetType() == Game_Battler::Type_Ally && target->IsDead()) {
		target_sprite->SetAnimationState(Sprite_Actor::AnimationState_Dead, Sprite_Actor::LoopState_WaitAfterFinish);
	}

	if (action->IsSuccess() && target->GetType() == Game_Battler::Type_Enemy) {
		auto* enemy = static_cast<Game_Enemy*>(target);
		if (!was_dead && enemy->IsDead()) {
			Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_EnemyKill));
			enemy->SetDeathTimer();
			RefreshTargetWindow();
		}
	}

	if (target_sprite) {
		target_sprite->DetectStateChange();
	}

	if (action->IsSuccess()) {
		if (action->IsCriticalHit()) {
			Main_Data::game_screen->FlashOnce(28, 28, 28, 20, 8);
		}
		if (action->IsAffectHp()) {
			const auto hp = action->GetAffectedHp();
			if (hp != 0 || (!action->IsPositive() && !action->IsAbsorbHp())) {
				DrawFloatText(
						target->GetBattlePosition().x,
						target->GetBattlePosition().y,
						hp > 0 ? Font::ColorHeal : Font::ColorDefault,
						std::to_string(std::abs(hp)));

				if (action->IsAbsorbHp()) {
					DrawFloatText(
							source->GetBattlePosition().x,
							source->GetBattlePosition().y,
							hp > 0 ? Font::ColorDefault : Font::ColorHeal,
							std::to_string(std::abs(hp)));
				}
			}

			if (!action->IsPositive() && !action->IsAbsorbHp()) {
				if (target->GetType() == Game_Battler::Type_Ally) {
					Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_AllyDamage));
				} else {
					Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_EnemyDamage));
				}
			}
		}
	} else {
		auto* se = action->GetFailureSe();
		if (se) {
			Main_Data::game_system->SePlay(*se);
		}
		DrawFloatText(
				target->GetBattlePosition().x,
				target->GetBattlePosition().y,
				0,
				lcf::Data::terms.miss);
	}

	status_window->Refresh();

	// Repeat on next target
	if (action->TargetNext()) {
		SetBattleActionState(BattleActionState_Execute);
		return BattleActionReturn::eContinue;
	}

	// If action does multiple attacks, repeat again.
	if (action->RepeatNext(false)) {
		SetBattleActionState(BattleActionState_StartAlgo);
		return BattleActionReturn::eContinue;
	}

	if (source->GetType() == Game_Battler::Type_Ally) {
		if (action->GetType() == Game_BattleAlgorithm::Type::Normal) {
			auto* actor = static_cast<Game_Actor*>(source);
			auto* source_sprite = actor->GetActorBattleSprite();
			if (source_sprite) {
				source_sprite->SetNormalAttacking(false);
				auto* weapon = actor->GetWeaponSprite();
				if (weapon) {
					auto* weapon_animation_data = action->GetWeaponAnimationData();
					if (weapon_animation_data) {
						if (weapon_animation_data->type == lcf::rpg::BattlerAnimationItemSkill::AnimType_weapon) {
							weapon->StopAttack();
						}
					}
				}
			}
		}
	}

	if (action->GetCBAMovement() != lcf::rpg::BattlerAnimationItemSkill::Movement_none) {
		SetBattleActionState(BattleActionState_CBAInit);
	} else {
		SetBattleActionState(BattleActionState_PostAction);
	}
	return BattleActionReturn::eContinue;
}

Scene_Battle_Rpg2k3::BattleActionReturn Scene_Battle_Rpg2k3::ProcessBattleActionPostAction(Game_BattleAlgorithm::AlgorithmBase* action) {
	auto* source = action->GetSource();
	action->ProcessPostActionSwitches();

	// RPG_RT bug: Final interpreter call is only done for normal and skill for actors
	if (source->GetType() == Game_Battler::Type_Enemy
			|| action->GetType() == Game_BattleAlgorithm::Type::Normal
			|| action->GetType() == Game_BattleAlgorithm::Type::Skill) {
		SetBattleActionState(BattleActionState_PostEvents);
	} else {
		SetBattleActionState(BattleActionState_Finished);
	}

	return BattleActionReturn::eContinue;
}

Scene_Battle_Rpg2k3::BattleActionReturn Scene_Battle_Rpg2k3::ProcessBattleActionPostEvents(Game_BattleAlgorithm::AlgorithmBase* action) {
	// RPG_RT always runs the interpreter before starting the action.
	if (!CheckBattleEndAndScheduleEvents(EventTriggerType::eAfterBattleAction, action->GetSource())) {
		return BattleActionReturn::eContinue;
	}

	SetBattleActionState(BattleActionState_Finished);
	return BattleActionReturn::eContinue;
}

Scene_Battle_Rpg2k3::BattleActionReturn Scene_Battle_Rpg2k3::ProcessBattleActionFinished(Game_BattleAlgorithm::AlgorithmBase* action) {
	(void)action;
	first_strike = false;

	return BattleActionReturn::eFinished;
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

void Scene_Battle_Rpg2k3::AttackSelected() {
	// RPG_RT still requires you to select an enemy target, even if your weapon has attack all.
	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
	SetState(State_SelectEnemyTarget);
}

void Scene_Battle_Rpg2k3::SubskillSelected(int command) {
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
	RecreateSpWindow(active_actor);
}

void Scene_Battle_Rpg2k3::SpecialSelected() {
	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));

	active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::DoNothing>(active_actor));

	ActionSelectedCallback(active_actor);
}

void Scene_Battle_Rpg2k3::EscapeSelected() {
	if (!IsEscapeAllowedFromActorCommand()) {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
		return;
	}
	Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Decision));
	active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::Escape>(active_actor));
	ActionSelectedCallback(active_actor);
}

void Scene_Battle_Rpg2k3::RowSelected() {
	// Switching rows is only possible if in back row or
	// if at least 2 party members are in front row
	int current_row = active_actor->GetBattleRow();
	int front_row_battlers = 0;
	if (current_row == static_cast<int>(active_actor->IsDirectionFlipped())) {
		for (auto& actor: Main_Data::game_party->GetActors()) {
			if (actor->GetBattleRow() == static_cast<int>(actor->IsDirectionFlipped())) front_row_battlers++;
		}
	}
	if (current_row != static_cast<int>(active_actor->IsDirectionFlipped()) || front_row_battlers >= 2) {
		if (active_actor->GetBattleRow() == Game_Actor::RowType::RowType_front) {
			active_actor->SetBattleRow(Game_Actor::RowType::RowType_back);
		} else {
			active_actor->SetBattleRow(Game_Actor::RowType::RowType_front);
		}
		active_actor->SetBattlePosition(Game_Battle::Calculate2k3BattlePosition(*active_actor));
		active_actor->SetBattleAlgorithm(std::make_shared<Game_BattleAlgorithm::DoNothing>(active_actor));
		ActionSelectedCallback(active_actor);
	} else {
		Main_Data::game_system->SePlay(Main_Data::game_system->GetSystemSE(Main_Data::game_system->SFX_Buzzer));
	}
}

void Scene_Battle_Rpg2k3::ActionSelectedCallback(Game_Battler* for_battler) {
	for_battler->SetAtbGauge(0);

	if (for_battler == active_actor) {
		auto idx = GetNextReadyActor();
		SetActiveActor(idx);
	}

	Scene_Battle::ActionSelectedCallback(for_battler);

	if (for_battler->GetType() == Game_Battler::Type_Ally) {
		ReturnToMainBattleState();
	}

#ifdef EP_DEBUG_BATTLE2K3_STATE_MACHINE
	Output::Debug("Battle2k3 ScheduleAction {} name={} type={} frame={}",
			((for_battler->GetType() == Game_Battler::Type_Ally) ? "Actor" : "Enemy"),
			for_battler->GetName(), static_cast<int>(for_battler->GetBattleAlgorithm()->GetType()), Main_Data::game_system->GetFrameCounter());
#endif
}

void Scene_Battle_Rpg2k3::ShowNotification(std::string text) {
	if (text.empty()) {
		return;
	}
	help_window->SetVisible(true);
	help_window->SetText(std::move(text), Font::ColorDefault, Text::AlignLeft, false);
}

void Scene_Battle_Rpg2k3::EndNotification() {
	help_window->SetVisible(false);
}

bool Scene_Battle_Rpg2k3::CheckAnimFlip(Game_Battler* battler) {
	if (Main_Data::game_system->GetInvertAnimations()) {
		return battler->IsDirectionFlipped() ^ (battler->GetType() == Game_Battler::Type_Enemy);
	}
	return false;
}

void Scene_Battle_Rpg2k3::SetWait(int min_wait, int max_wait) {
        battle_action_wait = max_wait;
        battle_action_min_wait = max_wait - min_wait;
}

bool Scene_Battle_Rpg2k3::CheckWait() {
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

void Scene_Battle_Rpg2k3::OnPartyChanged(Game_Actor* actor, bool added) {
	if (!added) {
		actor->SetBattleSprite(nullptr);
		return;
	}

	actor->SetBattleSprite(std::make_unique<Sprite_Actor>(actor));

	// RPG_RT only does this when actors added to party
	// Wait until sprites loaded
	AsyncNext([this]() {
			InitActors();
			ResetAllBattlerZ();
			});
}

void Scene_Battle_Rpg2k3::OnEventHpChanged(Game_Battler* battler, int hp) {
	DrawFloatText(
			battler->GetBattlePosition().x,
			battler->GetBattlePosition().y,
			hp < 0 ? Font::ColorDefault : Font::ColorHeal,
			std::to_string(std::abs(hp)));
}

void Scene_Battle_Rpg2k3::RecreateSpWindow(Game_Battler* battler) {
	bool small_window = (lcf::Data::battlecommands.window_size == lcf::rpg::BattleCommands::WindowSize_small);
	int spwindow_size = 60;
	int spwindow_height = (small_window ? 20 : 32);
	if (battler && battler->MaxSpValue() >= 1000) {
		spwindow_size = 72;
	}
	sp_window = std::make_unique<Window_ActorSp>(Player::screen_width - Player::menu_offset_x - spwindow_size, (small_window ? Player::menu_offset_y + 154 : Player::menu_offset_y + 136), spwindow_size, spwindow_height);
	sp_window->SetVisible(false);
	sp_window->SetBorderY(small_window ? 2 : 8);
	sp_window->SetContents(Bitmap::Create(sp_window->GetWidth() - sp_window->GetBorderX() / 2, sp_window->GetHeight() - sp_window->GetBorderY() * 2));
	sp_window->SetZ(Priority_Window + 2);
	if (battler) {
		sp_window->SetBattler(*battler);
	}
}

void Scene_Battle_Rpg2k3::CBAInit() {
	auto* source = cba_action->GetSource();
	cba_move_frame = 0;

	auto* actor = static_cast<Game_Actor*>(source);
	auto* sprite = actor->GetActorBattleSprite();
	if (!cba_direction_back) {
		cba_start_pos = source->GetBattlePosition();
		if (sprite) {
			sprite->SetAnimationState(Sprite_Actor::AnimationState_WalkingLeft);
		}
		if (cba_action->GetCBAMovement() == lcf::rpg::BattlerAnimationItemSkill::Movement_move) {
			auto* target = cba_action->GetTarget();
			if (target != nullptr) {
				auto* enemy = static_cast<Game_Enemy*>(target);
				auto* enemysprite = enemy->GetEnemyBattleSprite();
				if (enemysprite) {
					cba_end_pos = Point(target->GetBattlePosition().x + (source->IsDirectionFlipped() ? -(enemysprite->GetWidth() / 2) : enemysprite->GetWidth() / 2), target->GetBattlePosition().y);
				}
			}
		}
	} else {
		if (sprite) {
			if (cba_action->GetType() == Game_BattleAlgorithm::Type::Normal || cba_action->GetCBAMovement() == lcf::rpg::BattlerAnimationItemSkill::Movement_move) {
				sprite->SetAnimationState(Sprite_Actor::AnimationState_WalkingRight);
			} else {
				sprite->SetAnimationState(Sprite_Actor::AnimationState_WalkingLeft);
			}
		}
	}

	if (cba_action->GetCBAMovement() != lcf::rpg::BattlerAnimationItemSkill::Movement_none) {
		sprite->SetAfterimageAmount(cba_action->GetCBAAfterimage() == lcf::rpg::BattlerAnimationItemSkill::Afterimage_add ? 3 : 0);
	}
}

void Scene_Battle_Rpg2k3::CBAMove() {
	auto* source = cba_action->GetSource();

	if (cba_move_frame < cba_num_move_frames) {
		// RPG_RT increments the frame counter twice per frame,
		// so we emulate this behavior here
		cba_move_frame += 2;
		int frame = (cba_direction_back ? std::max(0, cba_num_move_frames - cba_move_frame) : std::min(cba_num_move_frames, cba_move_frame));
		int move_dir_mult = (source->IsDirectionFlipped() ? 1 : -1);
		int offset_x = 0;
		int offset_y = 0;
		if (cba_action->GetCBAMovement() == lcf::rpg::BattlerAnimationItemSkill::Movement_step || cba_action->GetCBAMovement() == lcf::rpg::BattlerAnimationItemSkill::Movement_jump) {
			offset_x = 25 * move_dir_mult * frame / cba_num_move_frames;
		}
		if (cba_action->GetCBAMovement() == lcf::rpg::BattlerAnimationItemSkill::Movement_jump) {
			offset_y = -25 * sin(M_PI * frame / cba_num_move_frames) / 2;
		}
		if (cba_action->GetCBAMovement() == lcf::rpg::BattlerAnimationItemSkill::Movement_move) {
			offset_x = (cba_end_pos.x - cba_start_pos.x) * frame / cba_num_move_frames;
			offset_y = (cba_end_pos.y - cba_start_pos.y) * frame / cba_num_move_frames;
		}
		source->SetBattlePosition(Point(cba_start_pos.x + offset_x, cba_start_pos.y + offset_y));

		if (source->GetType() == Game_Battler::Type_Ally) {
			auto* sprite = static_cast<Game_Actor*>(source)->GetActorBattleSprite();
			if (sprite) {
				sprite->ResetZ();
			}
		}
	}

	if (cba_move_frame >= cba_num_move_frames) {
		auto* actor = static_cast<Game_Actor*>(source);
		auto* sprite = actor->GetActorBattleSprite();
		if (sprite) {
			sprite->DoAfterimageFade();
		}
		if (cba_direction_back) {
			if (sprite) {
				sprite->DoIdleAnimation();
			}
			cba_action = nullptr;
		}
	}
}
