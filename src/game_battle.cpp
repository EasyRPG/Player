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

// Headers
#include <deque>
#include <algorithm>
#include "data.h"
#include "game_actors.h"
#include "game_enemyparty.h"
#include "game_party.h"
#include "game_temp.h"
#include "game_switches.h"
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

	const RPG::TroopPage* script_page;

	boost::scoped_ptr<Game_Interpreter> interpreter;
	/** Contains battle related sprites */
	boost::scoped_ptr<Spriteset_Battle> spriteset;
}

void Game_Battle::Init() {
	interpreter.reset(new Game_Interpreter_Battle(0, true));
	spriteset.reset(new Spriteset_Battle());

	Game_Temp::battle_running = true;
	turn = 0;

	troop = &Data::troops[Game_Temp::battle_troop_id - 1];
}

void Game_Battle::Quit() {
	interpreter.reset();
	spriteset.reset();

	Game_Temp::battle_running = false;
}

void Game_Battle::Update() {
	interpreter->Update();
	spriteset->Update();
}

void Game_Battle::Terminate() {
	terminate = true;
}

Spriteset_Battle& Game_Battle::GetSpriteset() {
	return *spriteset;
}

void Game_Battle::NextTurn() {
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

/*
void Game_Battle::Quit() {
// Remove conditions which end after battle
for (std::vector<Battle::Ally>::iterator it = allies.begin(); it != allies.end(); it++)
it->GetActor()->RemoveStates();

interpreter.reset();

scene = NULL;
}
*/

int Game_Battle::GetTurn() {
	return turn;

	//return turn_fragments / turn_length;
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

	/*
	TODO RPG 2k3
	if (condition.flags.turn_enemy && !CheckTurns(GetEnemy(condition.turn_enemy_id).GetTurns(),
	condition.turn_enemy_b, condition.turn_enemy_a))
	return false;

	if (condition.flags.turn_actor) {
	Battle::Ally* ally = FindAlly(condition.turn_actor_id);
	if (!ally)
	return false;
	if (!CheckTurns(ally->GetTurns(), condition.turn_actor_b, condition.turn_actor_a))
	return false;
	}*/

	if (condition.flags.enemy_hp) {
		Game_Battler& enemy = (*Main_Data::game_enemyparty)[condition.enemy_id];
		int hp = enemy.GetHp();
		if (hp < condition.enemy_hp_min || hp > condition.enemy_hp_max)
			return false;
	}

	if (condition.flags.actor_hp) {
		Game_Actor* actor = Game_Actors::GetActor(condition.actor_id);
		int hp = actor->GetHp();
		if (hp < condition.actor_hp_min || hp > condition.actor_hp_max)
			return false;
	}
	/*
	TODO RPG2k3

	if (condition.flags.command_actor) {
	Battle::Ally* ally = FindAlly(condition.actor_id);
	if (!ally)
	return false;
	if (ally->last_command != condition.command_id)
	return false;
	}*/

	return true;
}

void Game_Battle::UpdateEvents() {
	const RPG::TroopPage* new_page = NULL;
	std::vector<RPG::TroopPage>::const_reverse_iterator it;
	for (it = troop->pages.rbegin(); it != troop->pages.rend(); it++) {
		const RPG::TroopPage& page = *it;
		if (AreConditionsMet(page.condition)) {
			new_page = &*it;
			break;
		}
	}

	if (new_page != NULL && new_page != script_page)
		interpreter->Setup(new_page->event_commands, 0);
}
