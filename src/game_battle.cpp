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

namespace Game_Battle {
	const lcf::rpg::Troop* troop = nullptr;

	std::string background_name;

	std::unique_ptr<Game_Interpreter> interpreter;
	/** Contains battle related sprites */
	std::unique_ptr<Spriteset_Battle> spriteset;

	std::unique_ptr<BattleAnimation> animation;

	bool battle_running = false;
	int escape_fail_count;

	struct BattleTest battle_test;
}

namespace {
	std::vector<bool> page_executed;
	int terrain_id;
	lcf::rpg::System::BattleCondition battle_cond = lcf::rpg::System::BattleCondition_none;
	lcf::rpg::System::BattleFormation battle_form = lcf::rpg::System::BattleFormation_terrain;
	int target_enemy_index;
	bool need_refresh;
	std::vector<bool> page_can_run;

	std::function<bool(const lcf::rpg::TroopPage&)> last_event_filter;
}

void Game_Battle::Init(int troop_id) {
	Main_Data::game_enemyparty->ResetBattle(troop_id);
	interpreter.reset(new Game_Interpreter_Battle());
	spriteset.reset(new Spriteset_Battle(background_name, terrain_id));
	spriteset->Update();
	animation.reset();

	Game_Battle::battle_running = true;
	Main_Data::game_party->ResetTurns();
	escape_fail_count = 0;
	target_enemy_index = 0;
	need_refresh = false;

	// troop_id is guaranteed to be valid
	troop = lcf::ReaderUtil::GetElement(lcf::Data::troops, troop_id);
	page_executed.resize(troop->pages.size());
	std::fill(page_executed.begin(), page_executed.end(), false);
	page_can_run.resize(troop->pages.size());
	std::fill(page_can_run.begin(), page_can_run.end(), false);

	RefreshEvents([](const lcf::rpg::TroopPage&) {
		return false;
	});

	Game_Actors::ResetBattle();

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
	animation.reset();

	Game_Battle::battle_running = false;
	terrain_id = 0;

	std::vector<Game_Battler*> allies;
	Main_Data::game_party->GetBattlers(allies);

	// Remove conditions which end after battle
	for (std::vector<Game_Battler*>::iterator it = allies.begin(); it != allies.end(); it++) {
		(*it)->RemoveBattleStates();
		(*it)->SetBattleAlgorithm(BattleAlgorithmRef());
	}

	page_executed.clear();
	page_can_run.clear();

	Game_Actors::ResetBattle();
	Main_Data::game_enemyparty->ResetBattle(0);
	Main_Data::game_pictures->OnBattleEnd();
}

void Game_Battle::UpdateAnimation() {
	if (animation) {
		animation->Update();
		if (animation->IsDone()) {
			animation.reset();
		}
	}
}

void Game_Battle::UpdateGraphics() {
	spriteset->Update();
	Main_Data::game_screen->UpdateGraphics();

	if (need_refresh) {
		need_refresh = false;
		std::vector<Game_Battler*> battlers;
		Main_Data::game_party->GetBattlers(battlers);
		Main_Data::game_enemyparty->GetBattlers(battlers);
		for (Game_Battler* b : battlers) {
			Sprite_Battler* spr = spriteset->FindBattler(b);
			if (spr) {
				spr->DetectStateChange();
			}
		}
	}
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

int Game_Battle::ShowBattleAnimation(int animation_id, std::vector<Game_Battler*> targets, bool only_sound, int cutoff) {
	const lcf::rpg::Animation* anim = lcf::ReaderUtil::GetElement(lcf::Data::animations, animation_id);
	if (!anim) {
		Output::Warning("ShowBattleAnimation Many: Invalid animation ID {}", animation_id);
		return 0;
	}

	animation.reset(new BattleAnimationBattle(*anim, std::move(targets), only_sound, cutoff));
	auto frames = animation->GetFrames();
	return cutoff >= 0 ? std::min(frames, cutoff) : frames;
}

bool Game_Battle::IsBattleAnimationWaiting() {
	return bool(animation);
}

void Game_Battle::NextTurn(Game_Battler* battler) {
	if (battler == nullptr) {
		std::fill(page_executed.begin(), page_executed.end(), false);
	} else {
		for (const lcf::rpg::TroopPage& page : troop->pages) {
			const lcf::rpg::TroopPageCondition& condition = page.condition;

			// Reset pages without actor/enemy condition each turn
			if (!condition.flags.turn_actor &&
				!condition.flags.turn_enemy &&
				!condition.flags.command_actor) {
				page_executed[page.ID - 1] = false;
			}

			// Reset pages of specific actor after that actors turn
			if (page_executed[page.ID - 1]) {
				if (battler->GetType() == Game_Battler::Type_Ally &&
						((condition.flags.turn_actor && Game_Actors::GetActor(condition.turn_actor_id) == battler) ||
						(condition.flags.command_actor && Game_Actors::GetActor(condition.command_actor_id) == battler))) {
					page_executed[page.ID - 1] = false;
				}
			}

			// Reset pages of specific enemy after that enemies turn
			if (battler->GetType() == Game_Battler::Type_Enemy &&
				condition.flags.turn_enemy &&
				(&((*Main_Data::game_enemyparty)[condition.turn_enemy_id]) == battler)) {
				page_executed[page.ID - 1] = false;
			}
		}
	}

	Main_Data::game_party->IncTurns();
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
		if (bat->Exists() && (bat->CanAct() || bat->GetType() == Game_Battler::Type_Enemy))
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

int Game_Battle::GetEscapeFailureCount() {
	return escape_fail_count;
}

void Game_Battle::IncEscapeFailureCount() {
	++escape_fail_count;
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

bool Game_Battle::AreConditionsMet(const lcf::rpg::TroopPageCondition& condition) {
	if (!condition.flags.switch_a &&
		!condition.flags.switch_b &&
		!condition.flags.variable &&
		!condition.flags.turn &&
		!condition.flags.turn_enemy &&
		!condition.flags.turn_actor &&
		!condition.flags.fatigue &&
		!condition.flags.enemy_hp &&
		!condition.flags.actor_hp &&
		!condition.flags.command_actor
		) {
		// Pages without trigger are never run
		return false;
	}

	if (condition.flags.switch_a && !Main_Data::game_switches->Get(condition.switch_a_id))
		return false;

	if (condition.flags.switch_b && !Main_Data::game_switches->Get(condition.switch_b_id))
		return false;

	if (condition.flags.variable && !(Main_Data::game_variables->Get(condition.variable_id) >= condition.variable_value))
		return false;

	if (condition.flags.turn && !CheckTurns(GetTurn(), condition.turn_b, condition.turn_a))
		return false;

	if (condition.flags.turn_enemy &&
		!CheckTurns((*Main_Data::game_enemyparty)[condition.turn_enemy_id].GetBattleTurn(),	condition.turn_enemy_b, condition.turn_enemy_a))
		return false;

	if (condition.flags.turn_actor &&
		!CheckTurns(Game_Actors::GetActor(condition.turn_actor_id)->GetBattleTurn(), condition.turn_actor_b, condition.turn_actor_a))
		return false;

	if (condition.flags.fatigue) {
		int fatigue = Main_Data::game_party->GetFatigue();
		if (fatigue < condition.fatigue_min || fatigue > condition.fatigue_max)
			return false;
	}

	if (condition.flags.enemy_hp) {
		Game_Battler& enemy = (*Main_Data::game_enemyparty)[condition.enemy_id];
		int hp = enemy.GetHp();
		int hpmin = enemy.GetMaxHp() * condition.enemy_hp_min / 100;
		int hpmax = enemy.GetMaxHp() * condition.enemy_hp_max / 100;
		if (hp < hpmin || hp > hpmax)
			return false;
	}

	if (condition.flags.actor_hp) {
		Game_Actor* actor = Game_Actors::GetActor(condition.actor_id);
		int hp = actor->GetHp();
		int hpmin = actor->GetMaxHp() * condition.actor_hp_min / 100;
		int hpmax = actor->GetMaxHp() * condition.actor_hp_max / 100;
		if (hp < hpmin || hp > hpmax)
			return false;
	}

	if (condition.flags.command_actor &&
		condition.command_id != Game_Actors::GetActor(condition.command_actor_id)->GetLastBattleAction())
		return false;

	return true;
}

bool Game_Battle::UpdateEvents() {
	const auto battle_end = Game_Battle::CheckWin() || Game_Battle::CheckLose();

	// 2k3 battle interupts events immediately when battle end conditions occur.
	if (Player::IsRPG2k3() && battle_end) {
		return true;
	}

	if (interpreter->IsRunning()) {
		return false;
	}

	// 2k battle end conditions wait for interpreter to finish.
	if (Player::IsRPG2k() && battle_end) {
		return true;
	}

	// Check if another page can run now or if a page that could run can no longer run.
	RefreshEvents(last_event_filter);

	for (const auto& page : troop->pages) {
		if (page_can_run[page.ID - 1]) {
			interpreter->Clear();
			interpreter->Push(page.event_commands, 0);
			page_can_run[page.ID - 1] = false;
			page_executed[page.ID - 1] = true;
			return false;
		}
	}

	// No event can run anymore, cancel the interpreter calling until
	// the battle system wants to run events again.
	RefreshEvents([](const lcf::rpg::TroopPage&) {
		return false;
	});

	return true;
}

void Game_Battle::RefreshEvents() {
	RefreshEvents([](const lcf::rpg::TroopPage&) {
		return true;
	});
}

void Game_Battle::RefreshEvents(std::function<bool(const lcf::rpg::TroopPage&)> predicate) {
	for (const auto& it : troop->pages) {
		const lcf::rpg::TroopPage& page = it;
		if (!page_executed[page.ID - 1] && AreConditionsMet(page.condition)) {
			if (predicate(it)) {
				page_can_run[it.ID - 1] = true;
			}
		} else {
			page_can_run[it.ID - 1] = false;
		}
	}

	last_event_filter = predicate;
}

Game_Interpreter& Game_Battle::GetInterpreter() {
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

void Game_Battle::SetEnemyTargetIndex(int target_enemy) {
	target_enemy_index = target_enemy;
}

int Game_Battle::GetEnemyTargetIndex() {
	return target_enemy_index;
}

void Game_Battle::SetNeedRefresh(bool refresh) {
	need_refresh = refresh;
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
	auto* sprite = Game_Battle::GetSpriteset().FindBattler(&enemy);

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
		if (e.get() == &enemy) {
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
	auto* sprite = Game_Battle::GetSpriteset().FindBattler(&actor);

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
