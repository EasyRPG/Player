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
#include "data.h"
#include "game_actors.h"
#include "game_enemyparty.h"
#include "game_message.h"
#include "game_party.h"
#include "game_temp.h"
#include "game_switches.h"
#include "game_system.h"
#include "game_variables.h"
#include "game_interpreter_battle.h"
#include "battle_animation.h"
#include "game_battle.h"
#include "spriteset_battle.h"
#include <boost/scoped_ptr.hpp>

namespace Game_Battle {
	const RPG::Troop* troop;

	bool terminate;
	std::string background_name;

	boost::scoped_ptr<Game_Interpreter> interpreter;
	/** Contains battle related sprites */
	boost::scoped_ptr<Spriteset_Battle> spriteset;

	boost::scoped_ptr<BattleAnimation> animation;

	int escape_fail_count;
}

namespace {
	int turn;
	bool message_is_fixed;
	int message_position;
	bool message_is_transparent;
	std::vector<bool> page_executed;
	int terrain_id;
	int battle_mode;
}

void Game_Battle::Init() {
	interpreter.reset(new Game_Interpreter_Battle(0, true));
	spriteset.reset(new Spriteset_Battle());
	animation.reset();

	Game_Temp::battle_running = true;
	turn = 0;
	escape_fail_count = 0;

	troop = &Data::troops[Game_Temp::battle_troop_id - 1];
	page_executed.resize(troop->pages.size());

	message_is_fixed = Game_Message::IsPositionFixed();
	message_position = Game_Message::GetPosition();

	Main_Data::game_party->ResetBattle();
}

void Game_Battle::Quit() {
	interpreter.reset();
	spriteset.reset();
	animation.reset();

	Game_Temp::battle_running = false;

	std::vector<Game_Battler*> allies;
	Main_Data::game_party->GetBattlers(allies);

	// Remove conditions which end after battle
	for (std::vector<Game_Battler*>::iterator it = allies.begin(); it != allies.end(); it++) {
		(*it)->RemoveBattleStates();
		(*it)->SetBattleAlgorithm(BattleAlgorithmRef());
	}

	Main_Data::game_party->ResetBattle();

	Game_Message::SetPositionFixed(message_is_fixed);
	Game_Message::SetPosition(message_position);
	Game_Message::SetTransparent(message_is_transparent);
}

void Game_Battle::Update() {
	interpreter->Update();
	spriteset->Update();
	if (animation) {
		animation->Update();
		if (animation->IsDone()) {
			animation.reset();
		}
	}
}

void Game_Battle::Terminate() {
	// Prevent gameover in cause some battle interpreter event toggled that
	// flag, checked again on map.
	Game_Temp::gameover = false;
}

Spriteset_Battle& Game_Battle::GetSpriteset() {
	return *spriteset;
}

void Game_Battle::ShowBattleAnimation(int animation_id, Game_Battler* target, bool flash) {
	Main_Data::game_data.screen.battleanim_id = animation_id;

	const RPG::Animation& anim = Data::animations[animation_id - 1];
	animation.reset(new BattleAnimationBattlers(anim, *target, flash));
}

void Game_Battle::ShowBattleAnimation(int animation_id, const std::vector<Game_Battler*>& targets, bool flash) {
	Main_Data::game_data.screen.battleanim_id = animation_id;

	const RPG::Animation& anim = Data::animations[animation_id - 1];
	animation.reset(new BattleAnimationBattlers(anim, targets, flash));
}

bool Game_Battle::IsBattleAnimationWaiting() {
	return bool(animation);
}

void Game_Battle::NextTurn(Game_Battler* battler) {
	if (battler == nullptr) {
		std::fill(page_executed.begin(), page_executed.end(), false);
	} else {
		std::vector<RPG::TroopPage>::const_iterator it;
		for (it = troop->pages.begin(); it != troop->pages.end(); ++it) {
			const RPG::TroopPage& page = *it;
			const RPG::TroopPageCondition& condition = page.condition;

			// Reset pages without actor/enemy condition each turn
			if (!condition.flags.turn_actor &&
				!condition.flags.turn_enemy) {
				page_executed[(*it).ID - 1] = false;
			}

			// Reset pages of specific actor after that actors turn
			if (page_executed[(*it).ID - 1]) {
				if (battler->GetType() == Game_Battler::Type_Ally &&
					condition.flags.turn_actor &&
					Game_Actors::GetActor(condition.turn_actor_id) == battler) {
					page_executed[(*it).ID - 1] = false;
				}
			}

			// Reset pages of specific enemy after that enemies turn
			if (battler->GetType() == Game_Battler::Type_Enemy &&
				condition.flags.turn_enemy &&
				(&((*Main_Data::game_enemyparty)[condition.turn_enemy_id]) == battler)) {
				page_executed[(*it).ID - 1] = false;
			}
		}
	}

	++turn;
}

void Game_Battle::UpdateGauges() {
	std::vector<Game_Battler*> battlers;
	Main_Data::game_enemyparty->GetBattlers(battlers);
	Main_Data::game_party->GetBattlers(battlers);

	int max_agi = 1;

	for (std::vector<Game_Battler*>::const_iterator it = battlers.begin();
		it != battlers.end(); ++it) {
		max_agi = std::max(max_agi, (*it)->GetAgi());
	}

	for (std::vector<Game_Battler*>::const_iterator it = battlers.begin();
		it != battlers.end(); ++it) {
		if (!(*it)->GetBattleAlgorithm()) {
			(*it)->UpdateGauge(1000 / max_agi);
		}
	}
}

void Game_Battle::ChangeBackground(const std::string& name) {
	background_name = name;
}

int Game_Battle::GetTurn() {
	return turn;
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
	if (condition.flags.switch_a && !Game_Switches[condition.switch_a_id])
		return false;

	if (condition.flags.switch_b && !Game_Switches[condition.switch_b_id])
		return false;

	if (condition.flags.variable && !(Game_Variables[condition.variable_id] >= condition.variable_value))
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
		Game_Battler& enemy = (*Main_Data::game_enemyparty)[condition.enemy_id - 1];
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
		condition.command_id != Game_Actors::GetActor(condition.turn_actor_id)->GetLastBattleAction())
		return false;

	return true;
}

bool Game_Battle::UpdateEvents() {
	if (interpreter->IsRunning()) {
		return false;
	}

	const RPG::TroopPage* new_page = NULL;
	std::vector<RPG::TroopPage>::const_iterator it;
	for (it = troop->pages.begin(); it != troop->pages.end(); ++it) {
		const RPG::TroopPage& page = *it;
		if (!page_executed[(*it).ID - 1] && AreConditionsMet(page.condition)) {
			new_page = &*it;
			page_executed[(*it).ID - 1] = true;
			break;
		}
	}

	if (new_page != NULL) {
		interpreter->Setup(new_page->event_commands, 0);
	}

	return new_page == NULL;
}

bool Game_Battle::IsEscapeAllowed() {
	if (Game_Temp::battle_escape_mode == -1) {
		return Game_System::GetAllowEscape();
	}
	else {
		return !!Game_Temp::battle_escape_mode;
	}
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
	// Fixme: Workaround for battle test
	// Has options loose/tight formation which hardcodes to specific
	// x/y values and ignores terrain
	return terrain_id <= 0 ? 1 : terrain_id;
}

void Game_Battle::SetBattleMode(int battle_mode_) {
	battle_mode = battle_mode_;
}

int Game_Battle::GetBattleMode() {
	return battle_mode;
}
