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
#include <cassert>
#include <lcf/data.h>
#include "player.h"
#include "game_actors.h"
#include "game_enemyparty.h"
#include "game_message.h"
#include "game_party.h"
#include "game_switches.h"
#include "game_system.h"
#include "game_variables.h"
#include "game_interpreter_battle.h"
#include "game_screen.h"
#include "game_pictures.h"
#include "battle_animation.h"
#include "game_battle.h"
#include <lcf/reader_util.h>
#include "spriteset_battle.h"
#include "output.h"
#include "utils.h"
#include "rand.h"

namespace Game_Battle {
	const lcf::rpg::Troop* troop = nullptr;

	std::string background_name;

	std::unique_ptr<Game_Interpreter_Battle> interpreter;
	/** Contains battle related sprites */
	std::unique_ptr<Spriteset_Battle> spriteset;

	std::unique_ptr<BattleAnimation> animation_actors;
	std::unique_ptr<BattleAnimation> animation_enemies;

	bool battle_running = false;

	struct BattleTest battle_test;
}

namespace {
	int terrain_id;
	lcf::rpg::System::BattleCondition battle_cond = lcf::rpg::System::BattleCondition_none;
	lcf::rpg::System::BattleFormation battle_form = lcf::rpg::System::BattleFormation_terrain;
}

void Game_Battle::Init(int troop_id) {
	// troop_id is guaranteed to be valid
	troop = lcf::ReaderUtil::GetElement(lcf::Data::troops, troop_id);
	assert(troop);
	Game_Battle::battle_running = true;
	Main_Data::game_party->ResetTurns();

	Main_Data::game_enemyparty->ResetBattle(troop_id);
	Main_Data::game_actors->ResetBattle();

	interpreter.reset(new Game_Interpreter_Battle(troop->pages));
	spriteset.reset(new Spriteset_Battle(background_name, terrain_id));
	spriteset->Update();
	animation_actors.reset();
	animation_enemies.reset();


	for (auto* actor: Main_Data::game_party->GetActors()) {
		actor->ResetEquipmentStates(true);
	}
}

void Game_Battle::Quit() {
	if (!IsBattleRunning()) {
		return;
	}

	interpreter.reset();
	spriteset.reset();
	animation_actors.reset();
	animation_enemies.reset();

	Game_Battle::battle_running = false;
	terrain_id = 0;

	std::vector<Game_Battler*> allies;
	Main_Data::game_party->GetBattlers(allies);

	// Remove conditions which end after battle
	for (std::vector<Game_Battler*>::iterator it = allies.begin(); it != allies.end(); it++) {
		(*it)->RemoveBattleStates();
		(*it)->SetBattleAlgorithm(BattleAlgorithmRef());
	}

	Main_Data::game_actors->ResetBattle();
	Main_Data::game_enemyparty->ResetBattle(0);
	Main_Data::game_pictures->OnBattleEnd();
}

void Game_Battle::UpdateAnimation() {
	if (animation_actors) {
		animation_actors->Update();
		if (animation_actors->IsDone()) {
			animation_actors.reset();
		}
	}
	if (animation_enemies) {
		animation_enemies->Update();
		if (animation_enemies->IsDone()) {
			animation_enemies.reset();
		}
	}
}

void Game_Battle::UpdateGraphics() {
	spriteset->Update();
}

bool Game_Battle::CheckWin() {
	return !Main_Data::game_enemyparty->IsAnyActive();
}

bool Game_Battle::CheckLose() {
	// If there are active characters, but all of them are in a state with Restriction "Do Nothing" and 0% recovery probability (including death), it's game over
	// Physical recovery doesn't matter in this case
	for (auto& actor : Main_Data::game_party->GetActors()) {
		if (!actor->IsHidden() && actor->CanActOrRecoverable()) {
			return false;
		}
	}

	return true;
}

Spriteset_Battle& Game_Battle::GetSpriteset() {
	assert(spriteset);
	return *spriteset;
}

int Game_Battle::ShowBattleAnimation(int animation_id, std::vector<Game_Battler*> targets, bool only_sound, int cutoff, bool invert) {
	const lcf::rpg::Animation* anim = lcf::ReaderUtil::GetElement(lcf::Data::animations, animation_id);
	if (!anim) {
		Output::Warning("ShowBattleAnimation Many: Invalid animation ID {}", animation_id);
		return 0;
	}

	const auto main_type = targets.empty() ? Game_Battler::Type_Ally : targets.front()->GetType();
	std::vector<Game_Battler*> alt_targets;

	for (auto iter = targets.begin(); iter != targets.end();) {
		if ((*iter)->GetType() == main_type) {
			++iter;
		} else {
			alt_targets.push_back(*iter);
			iter = targets.erase(iter);
		}
	}

	auto& main_anim = main_type == Game_Battler::Type_Ally ? animation_actors : animation_enemies;
	auto& alt_anim = main_type == Game_Battler::Type_Ally ? animation_enemies : animation_actors;

	main_anim.reset(new BattleAnimationBattle(*anim, std::move(targets), only_sound, cutoff, invert));
	auto main_frames = main_anim->GetFrames();
	main_frames = cutoff >= 0 ? std::min(main_frames, cutoff) : main_frames;

	auto alt_frames = 0;
	if (!alt_targets.empty()) {
		alt_anim.reset(new BattleAnimationBattle(*anim, std::move(alt_targets), only_sound, cutoff, invert));
		auto alt_frames = alt_anim->GetFrames();
		alt_frames = cutoff >= 0 ? std::min(alt_frames, cutoff) : alt_frames;
	}

	return std::max(main_frames, alt_frames);
}

bool Game_Battle::IsBattleAnimationWaiting() {
	return bool(animation_actors) || bool(animation_enemies);
}

void Game_Battle::UpdateAtbGauges() {
	std::vector<Game_Battler*> battlers;
	Main_Data::game_enemyparty->GetBattlers(battlers);
	Main_Data::game_party->GetBattlers(battlers);

	const auto use_2k3e_algo = Player::IsRPG2k3E();

	int sum_agi = 0;
	for (auto* bat: battlers) {
		// RPG_RT uses dead and state restricted battlers to contribute to the sum.
		if (!bat->IsHidden()) {
			sum_agi += bat->GetAgi();
		}
	}
	sum_agi *= 100;

	const int max_atb = Game_Battler::GetMaxAtbGauge();

	for (auto* bat: battlers) {
		// RPG_RT always updates atb for non-hidden enemies, even if they can't act.
		// We don't update ATB for battlers with a pending battle algo
		if (!bat->GetBattleAlgorithm() && bat->Exists() && (bat->CanAct() || bat->GetType() == Game_Battler::Type_Enemy))
		{
			const auto agi = bat->GetAgi();
			auto increment = max_atb / (sum_agi / (agi + 1));
			if (use_2k3e_algo) {
				const auto cur_atb = bat->GetAtbGauge();
				const auto multiplier = std::max(1.0, static_cast<double>(275000 - cur_atb) / 55000.0);
				increment = Utils::RoundTo<int>(multiplier * increment);
			}
			bat->IncrementAtbGauge(increment);
		}
	}
}

void Game_Battle::ChangeBackground(const std::string& name) {
	background_name = name;
}

const std::string& Game_Battle::GetBackground() {
	return background_name;
}

int Game_Battle::GetTurn() {
	return Main_Data::game_party->GetTurns();
}

bool Game_Battle::CheckTurns(int turns, int base, int multiple) {
	if (multiple == 0) {
		return turns >= base && (turns - base) == 0;
	}
	else {
		return turns >= base && (turns - base) % multiple == 0;
	}
}

Game_Interpreter& Game_Battle::GetInterpreter() {
	assert(interpreter);
	return *interpreter;
}

Game_Interpreter_Battle& Game_Battle::GetInterpreterBattle() {
	assert(interpreter);
	return *interpreter;
}

void Game_Battle::SetTerrainId(int id) {
	terrain_id = id;
}

int Game_Battle::GetTerrainId() {
	return terrain_id;
}

void Game_Battle::SetBattleCondition(lcf::rpg::System::BattleCondition cond) {
	battle_cond = cond;
}

lcf::rpg::System::BattleCondition Game_Battle::GetBattleCondition() {
	return battle_cond;
}

void Game_Battle::SetBattleFormation(lcf::rpg::System::BattleFormation form) {
	battle_form = form;
}

lcf::rpg::System::BattleFormation Game_Battle::GetBattleFormation() {
	return battle_form;
}

bool Game_Battle::HasDeathHandler() {
	// RPG Maker Editor always sets both death_handler and death_handler_unused chunks.
	// However, RPG_RT will only trigger death handler based on the death_handler chunk.
	auto& db = lcf::Data::battlecommands;
	return Player::IsRPG2k3() && db.death_handler;
}

int Game_Battle::GetDeathHandlerCommonEvent() {
	auto& db = lcf::Data::battlecommands;
	if (HasDeathHandler()) {
		return db.death_event;
	}
	return 0;
}

TeleportTarget Game_Battle::GetDeathHandlerTeleport() {
	auto& db = lcf::Data::battlecommands;
	if (HasDeathHandler() && db.death_teleport) {
		return TeleportTarget(db.death_teleport_id, db.death_teleport_x, db.death_teleport_y, db.death_teleport_face -1, TeleportTarget::eParallelTeleport);
	}
	return {};
}

const lcf::rpg::Troop* Game_Battle::GetActiveTroop() {
	return IsBattleRunning() ? troop : nullptr;
}

static constexpr double grid_tables[4][8][8] = {
{
// In DynRPG 0x4CC16C
	{ 0.5, },
	{ 0.0, 1.0, },
	{ 0.0, 0.5, 1.0, },
	{ 0.0, 0.33, 0.66, 1.0, },
	{ 0.0, 0.25, 0.5, 0.75, 1.0, },
	{ 0.0, 0.0, 0.5, 0.5, 1.0, 1.0, },
	{ 0.0, 0.25, 0.33, 0.5, 0.66, 0.75, 1.0, },
	{ 0.0, 0.0, 0.33, 0.33, 0.66, 0.66, 1.0, 1.0, },
}, {
// In DynRPG 0x4CC36C
	{ 0.5, },
	{ 0.0, 1.0, },
	{ 0.0, 1.0, 0.5, },
	{ 0.0, 0.75, 0.25, 1.0, },
	{ 0.0, 0.5, 1.0, 0.25, 0.75, },
	{ 0.0, 0.4, 0.8, 0.2, 0.6, 1.0, },
	{ 0.0, 0.33, 0.66, 1.0, 0.25, 0.5, 1.0, },
	{ 0.0, 0.28, 0.56, 0.84, 0.14, 0.42, 0.7, 0.98, }
}, {
// In DynRPG 0x4CC35C
	{ 0.5, },
	{ 0.5, 0.5, },
	{ 0.0, 0.5, 1.0, },
	{ 0.0, 0.0, 1.0, 1.0, },
	{ 0.0, 0.33, 0.5, 0.66, 1.0, },
	{ 0.0, 0.0, 0.5, 0.5, 1.0, 1.0, },
	{ 0.0, 0.25, 0.33, 0.5, 0.66, 0.75, 1.0,},
	{ 0.0, 0.0, 0.33, 0.33, 0.66, 0.66, 1.0, 1.0, }
}, {
// In DynRPG 0x4CC76C
	{ },
	{ 24, },
	{ 48, 48, },
	{ 48, 48, },
	{ 56, 56, 56, 8, 8, },
	{ 56, 56, 56, 8, 8, },
	{ 64, 64, 64, 64, 16, 16, 16, },
	{ 64, 64, 64, 64, 16, 16, 16, 16 },
}};


Point Game_Battle::CalculateBaseGridPosition(
		int party_idx,
		int party_size,
		int table_x,
		int table_y,
		lcf::rpg::System::BattleFormation form,
		int terrain_id)
{
	Point pos;

	assert(party_idx >= 0);
	assert(party_idx < party_size);
	assert(party_size <= 8);

	int grid_top_y = 112;
	double grid_elongation = 392;
	double grid_inclination = 16000;
	if (terrain_id > 0) {
		const auto* terrain = lcf::ReaderUtil::GetElement(lcf::Data::terrains, Game_Battle::GetTerrainId());
		if (terrain) {
			grid_top_y = terrain->grid_top_y;
			grid_elongation = static_cast<double>(terrain->grid_elongation);
			grid_inclination = static_cast<double>(terrain->grid_inclination);
		}
	} else if (form == lcf::rpg::System::BattleFormation_tight) {
		grid_top_y = 132;
		grid_elongation = 196;
		grid_inclination = 24000;
	}

	const auto tdx = grid_tables[table_x][party_size - 1][party_idx];
	const auto tdy = grid_tables[table_y][party_size - 1][party_idx];

	pos.x = static_cast<int>((1.0 - tdx) * (grid_inclination / 1000.0));
	pos.y = grid_top_y + static_cast<int>(std::sin(grid_elongation / 1000.0) * 120.0 * tdy);

	return pos;
}


Point Game_Battle::Calculate2k3BattlePosition(const Game_Enemy& enemy) {
	assert(troop);

	const auto terrain_id = Game_Battle::GetTerrainId();
	const auto cond = Game_Battle::GetBattleCondition();
	const auto form = Game_Battle::GetBattleFormation();
	auto* sprite = enemy.GetBattleSprite();

	int half_height = 0;
	int half_width = 0;
	if (sprite) {
		half_height = sprite->GetHeight() / 2;
		half_width = sprite->GetWidth() / 2;
	}

	// Manual position case
	if (!troop->auto_alignment
			&& cond != lcf::rpg::System::BattleCondition_pincers
			&& cond != lcf::rpg::System::BattleCondition_surround) {
		auto position = enemy.GetOriginalPosition();
		if (cond == lcf::rpg::System::BattleCondition_back) {
			position.x = 320 - position.x;
		}

		// RPG_RT only clamps Y position for enemies
		position.y = Utils::Clamp(position.y, half_height, 240 - half_height);
		return position;
	}

	// Auto position case

	int party_size = 0;
	int idx = 0;
	bool found_myself = false;
	// Position is computed based on this monster's index relative to party size
	// If monster is hidden -> use real party idx / real party size
	// If monster is visible -> use visible party idx / visible party size
	for (auto& e: Main_Data::game_enemyparty->GetEnemies()) {
		if (e == &enemy) {
			found_myself = true;
		}
		if (enemy.IsHidden() || !e->IsHidden()) {
			party_size += 1;
			idx += !(found_myself);
		}
	}

	// RPG_RT has a bug where the pincer table is only used for enemies on terrain battles but not on non terrain battles.
	const int table_y_idx = (cond == lcf::rpg::System::BattleCondition_pincers && terrain_id >= 1) ? 2 : 1;
	const auto grid = CalculateBaseGridPosition(idx, party_size, 0, table_y_idx, form, terrain_id);
	const auto ti = grid_tables[3][party_size - 1][idx];

	Point position;

	const bool is_odd = (idx & 1);

	switch (cond) {
		case lcf::rpg::System::BattleCondition_none:
		case lcf::rpg::System::BattleCondition_initiative:
			position.x = grid.x + ti + half_width;
			break;
		case lcf::rpg::System::BattleCondition_back:
			position.x = 320 - (grid.x + ti + half_width);
			break;
		case lcf::rpg::System::BattleCondition_surround:
			position.x = 160 + (is_odd ? 16 : -16);
			break;
		case lcf::rpg::System::BattleCondition_pincers:
			position.x = grid.x + half_width + 16;
			if (!is_odd) {
				position.x = 320 - position.x;
			}
			break;
	}

	position.y = grid.y - half_height;

	// RPG_RT only clamps Y position for enemies
	position.y = Utils::Clamp(position.y, half_height, 240 - half_height);

	return position;
}

Point Game_Battle::Calculate2k3BattlePosition(const Game_Actor& actor) {
	assert(troop);

	const auto terrain_id = Game_Battle::GetTerrainId();
	const auto cond = Game_Battle::GetBattleCondition();
	const auto form = Game_Battle::GetBattleFormation();
	auto* sprite = actor.GetBattleSprite();

	int half_height = 0;
	int half_width = 0;
	if (sprite) {
		half_height = sprite->GetHeight() / 2;
		half_width = sprite->GetWidth() / 2;
	}

	int row_x_offset = 0;
	if (actor.GetBattleRow() == Game_Actor::RowType::RowType_front) {
		row_x_offset = half_width;
	}

	// Manual position case
	if (lcf::Data::battlecommands.placement == lcf::rpg::BattleCommands::Placement_manual) {
		auto position = actor.GetOriginalPosition();

		if (cond == lcf::rpg::System::BattleCondition_back) {
			position.x = 320 - (position.x + row_x_offset);
		} else {
			position.x = position.x - row_x_offset;
		}

		// RPG_RT doesn't clamp Y for actors
		position.x = Utils::Clamp(position.x, half_width, 320 - half_width);
		return position;
	}

	const auto idx = Main_Data::game_party->GetActorPositionInParty(actor.GetId());
	const auto party_size = Main_Data::game_party->GetBattlerCount();

	const int table_y_idx = (cond == lcf::rpg::System::BattleCondition_surround) ? 2 : 0;
	const auto grid = CalculateBaseGridPosition(idx, party_size, 0, table_y_idx, form, terrain_id);

	Point position;

	const bool is_odd = (idx & 1);

	switch (cond) {
		case lcf::rpg::System::BattleCondition_none:
		case lcf::rpg::System::BattleCondition_initiative:
			position.x = 320 - (grid.x + half_width + row_x_offset);
			break;
		case lcf::rpg::System::BattleCondition_back:
			position.x = grid.x + 2 * half_width - row_x_offset;
			break;
		case lcf::rpg::System::BattleCondition_surround:
			position.x = grid.x + half_width + row_x_offset;
			if (!is_odd) {
				position.x = 320 - position.x;
			}
			break;
		case lcf::rpg::System::BattleCondition_pincers:
			position.x = 160 + (half_width / 2) - row_x_offset;
			break;
	}

	position.y = grid.y - half_height;

	// RPG_RT doesn't clamp Y for actors
	position.x = Utils::Clamp(position.x, half_width, 320 - half_width);

	return position;
}

