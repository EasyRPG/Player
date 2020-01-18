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
#include "data.h"
#include "player.h"
#include "game_actors.h"
#include "game_enemyparty.h"
#include "game_message.h"
#include "game_party.h"
#include "game_temp.h"
#include "game_switches.h"
#include "game_system.h"
#include "game_variables.h"
#include "game_interpreter_battle.h"
#include "game_screen.h"
#include "game_pictures.h"
#include "battle_animation.h"
#include "game_battle.h"
#include "reader_util.h"
#include "spriteset_battle.h"
#include "output.h"

namespace Game_Battle {
	const RPG::Troop* troop;

	bool terminate;
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
	int battle_mode;
	int target_enemy_index;
	bool need_refresh;
	std::vector<bool> page_can_run;

	std::function<bool(const RPG::TroopPage&)> last_event_filter;
}

void Game_Battle::Init() {
	interpreter.reset(new Game_Interpreter_Battle());
	spriteset.reset(new Spriteset_Battle());
	spriteset->Update();
	animation.reset();

	Game_Battle::battle_running = true;
	Main_Data::game_party->ResetTurns();
	terminate = false;
	escape_fail_count = 0;
	target_enemy_index = 0;
	need_refresh = false;

	// troop_id is guaranteed to be valid
	troop = ReaderUtil::GetElement(Data::troops, Game_Temp::battle_troop_id);
	page_executed.resize(troop->pages.size());
	std::fill(page_executed.begin(), page_executed.end(), false);
	page_can_run.resize(troop->pages.size());
	std::fill(page_can_run.begin(), page_can_run.end(), false);

	RefreshEvents([](const RPG::TroopPage&) {
		return false;
	});

	Main_Data::game_party->ResetBattle();

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
	Game_Temp::battle_background = "";
	SetTerrainId(0);

	std::vector<Game_Battler*> allies;
	Main_Data::game_party->GetBattlers(allies);

	// Remove conditions which end after battle
	for (std::vector<Game_Battler*>::iterator it = allies.begin(); it != allies.end(); it++) {
		(*it)->RemoveBattleStates();
		(*it)->SetBattleAlgorithm(BattleAlgorithmRef());
	}

	Main_Data::game_party->IncBattleCount();
	switch (Game_Temp::battle_result) {
		case Game_Temp::BattleVictory: Main_Data::game_party->IncWinCount(); break;
		case Game_Temp::BattleEscape: Main_Data::game_party->IncRunCount(); break;
		case Game_Temp::BattleDefeat: Main_Data::game_party->IncDefeatCount(); break;
		case Game_Temp::BattleAbort: break;
	}

	page_executed.clear();
	page_can_run.clear();

	Main_Data::game_party->ResetBattle();
	Main_Data::game_pictures->OnBattleEnd();
}

void Game_Battle::RunEvents() {
	interpreter->Update();
	if (interpreter->IsAsyncPending()) {
		terminate = true;
		return;
	}
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
	Main_Data::game_pictures->UpdateGraphics(true);

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

void Game_Battle::Terminate() {
	Game_Temp::battle_result = Game_Temp::BattleAbort;
	terminate = true;
}

bool Game_Battle::CheckWin() {
	return !Main_Data::game_enemyparty->IsAnyActive();
}

bool Game_Battle::CheckLose() {
	if (!Main_Data::game_party->IsAnyActive())
		return true;

	// If there are active characters, but all of them are in a state with Restriction "Do Nothing" and 0% recovery probability, it's game over
	// Physical recovery doesn't matter in this case
	int character_number = 0;
	std::vector<Game_Battler*> actors;

	Main_Data::game_party->GetActiveBattlers(actors);
	for (auto actor : actors) {
		for (auto id_state : actor->GetInflictedStates()) {
			RPG::State *state = ReaderUtil::GetElement(Data::states, id_state);
			if (state->restriction == RPG::State::Restriction_do_nothing && state->auto_release_prob == 0) {
				++character_number;
				break;
			}
		}
	}

	return character_number == static_cast<int>(actors.size());
}

Spriteset_Battle& Game_Battle::GetSpriteset() {
	assert(spriteset);
	return *spriteset;
}

int Game_Battle::ShowBattleAnimation(int animation_id, std::vector<Game_Battler*> targets, bool only_sound, int cutoff) {
	const RPG::Animation* anim = ReaderUtil::GetElement(Data::animations, animation_id);
	if (!anim) {
		Output::Warning("ShowBattleAnimation Many: Invalid animation ID %d", animation_id);
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
		for (const RPG::TroopPage& page : troop->pages) {
			const RPG::TroopPageCondition& condition = page.condition;

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

void Game_Battle::UpdateGauges() {
	std::vector<Game_Battler*> battlers;
	Main_Data::game_enemyparty->GetActiveBattlers(battlers);
	Main_Data::game_party->GetActiveBattlers(battlers);

	int max_agi = 1;

	for (std::vector<Game_Battler*>::const_iterator it = battlers.begin();
		it != battlers.end(); ++it) {
		max_agi = std::max(max_agi, (*it)->GetAgi());
	}

	// Only affects how fast the gauges move, can be safely clamped
	max_agi = Utils::Clamp(max_agi, 1, 1000);

	for (std::vector<Game_Battler*>::const_iterator it = battlers.begin();
		it != battlers.end(); ++it) {
		if (!(*it)->GetBattleAlgorithm() && (*it)->CanAct()) {
			(*it)->UpdateGauge(1000 / max_agi);
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

bool Game_Battle::AreConditionsMet(const RPG::TroopPageCondition& condition) {
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
	RefreshEvents([](const RPG::TroopPage&) {
		return false;
	});

	return true;
}

void Game_Battle::RefreshEvents() {
	RefreshEvents([](const RPG::TroopPage&) {
		return true;
	});
}

void Game_Battle::RefreshEvents(std::function<bool(const RPG::TroopPage&)> predicate) {
	for (const auto& it : troop->pages) {
		const RPG::TroopPage& page = it;
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

bool Game_Battle::IsEscapeAllowed() {
	return Game_Temp::battle_escape_mode != 0;
}

bool Game_Battle::IsTerminating() {
	return terminate;
}

Game_Interpreter& Game_Battle::GetInterpreter() {
	assert(interpreter);
	return *interpreter;
}

void Game_Battle::SetTerrainId(int terrain_id_) {
	terrain_id = terrain_id_;
}

int Game_Battle::GetTerrainId() {
	return terrain_id;
}

void Game_Battle::SetBattleMode(int battle_mode_) {
	battle_mode = battle_mode_;
}

int Game_Battle::GetBattleMode() {
	return battle_mode;
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
	auto& db = Data::battlecommands;
	return Player::IsRPG2k3() && db.death_handler;
}

int Game_Battle::GetDeathHandlerCommonEvent() {
	auto& db = Data::battlecommands;
	if (HasDeathHandler()) {
		return db.death_event;
	}
	return 0;
}

TeleportTarget Game_Battle::GetDeathHandlerTeleport() {
	auto& db = Data::battlecommands;
	if (HasDeathHandler() && db.death_teleport) {
		return TeleportTarget(db.death_teleport_id, db.death_teleport_x, db.death_teleport_y, db.death_teleport_face -1, TeleportTarget::eParallelTeleport);
	}
	return {};
}
