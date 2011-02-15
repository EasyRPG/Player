
#include "data.h"
#include "game_party.h"
#include "game_temp.h"
#include "game_battle.h"

namespace Game_Battle {

	const RPG::Troop* troop;
	std::vector<Battle::Ally> allies;
	std::vector<Battle::Enemy> enemies;
	Background* background;

	bool active;
	int turn_fragments;
	int target_enemy;
	int target_ally;
	int active_enemy;
	int active_ally;
}

////////////////////////////////////////////////////////////
void Game_Battle::Init() {
	active = true;

	if (!Game_Temp::battle_background.empty())
		background = new Background(Game_Temp::battle_background);
	else
		background = new Background(Game_Temp::battle_terrain_id);

	troop = &Data::troops[Game_Temp::battle_troop_id - 1];

	allies.clear();
	const std::vector<Game_Actor*>& actors = Game_Party::GetActors();
	std::vector<Game_Actor*>::const_iterator ai;
	for (ai = actors.begin(); ai != actors.end(); ai++)
		allies.push_back(Battle::Ally(*ai, ai - actors.begin()));

	enemies.clear();
	std::vector<RPG::TroopMember>::const_iterator ei;
	for (ei = troop->members.begin(); ei != troop->members.end(); ei++)
		enemies.push_back(Battle::Enemy(&*ei, ei - troop->members.begin()));

	for (std::vector<Battle::Enemy>::iterator it = enemies.begin(); it != enemies.end(); it++)
		it->CreateSprite();

	for (std::vector<Battle::Ally>::iterator it = allies.begin(); it != allies.end(); it++)
		it->CreateSprite();

	int gauge = Game_Temp::battle_first_strike ? Battle::Battler::gauge_full : 0;
	for (std::vector<Battle::Ally>::iterator it = allies.begin(); it != allies.end(); it++)
		it->gauge = gauge;

	active_enemy = -1;
	active_ally = -1;
	target_enemy = -1;
	target_ally = -1;
}

void Game_Battle::Quit() {
	delete background;
	background = NULL;

	// Remove conditions which end after battle
	for (std::vector<Battle::Ally>::iterator it = allies.begin(); it != allies.end(); it++)
		it->GetActor()->RemoveStates();

	allies.clear();
	enemies.clear();

	active = false;
}

////////////////////////////////////////////////////////////
Battle::Ally* Game_Battle::FindAlly(int actor_id) {
	for (std::vector<Battle::Ally>::iterator it = allies.begin(); it != allies.end(); it++)
		if (it->rpg_actor->ID == actor_id)
			return &*it;
	return NULL;
}

////////////////////////////////////////////////////////////
void Game_Battle::AlliesCentroid(int& x, int& y) {
	x = 0;
	y = 0;
	for (std::vector<Battle::Ally>::iterator it = allies.begin(); it != allies.end(); it++) {
		x += it->rpg_actor->battle_x;
		y += it->rpg_actor->battle_y;
	}
	x /= allies.size();
	y /= allies.size();
}

////////////////////////////////////////////////////////////
void Game_Battle::EnemiesCentroid(int& x, int& y) {
	x = 0;
	y = 0;
	for (std::vector<Battle::Enemy>::iterator it = enemies.begin(); it != enemies.end(); it++) {
		x += it->member->x;
		y += it->member->y;
	}
	x /= allies.size();
	y /= allies.size();
}


////////////////////////////////////////////////////////////
Battle::Ally& Game_Battle::GetAlly(int i) {
	return allies[i];
}

void Game_Battle::SetTargetAlly(int target) {
	target_ally = target;
}

void Game_Battle::ClearTargetAlly() {
	target_ally = -1;
}

Battle::Ally& Game_Battle::GetTargetAlly() {
	return allies[target_ally];
}

bool Game_Battle::HaveTargetAlly() {
	return target_ally >= 0;
}

void Game_Battle::TargetNextAlly() {
	target_ally++;
	target_ally %= allies.size();
}

void Game_Battle::TargetPreviousAlly() {
	target_ally--;
	target_ally += allies.size();
	target_ally %= allies.size();
}

void Game_Battle::SetActiveAlly(int active) {
	active_ally = active;
}

Battle::Ally& Game_Battle::GetActiveAlly() {
	return allies[active_ally];
}

bool Game_Battle::HaveActiveAlly() {
	return active_ally >= 0;
}

void Game_Battle::TargetActiveAlly() {
	target_ally = active_ally;
}

void Game_Battle::TargetRandomAlly() {
	target_ally = rand() % allies.size();
}

////////////////////////////////////////////////////////////
Battle::Enemy& Game_Battle::GetEnemy(int i) {
	return enemies[i];
}

void Game_Battle::SetTargetEnemy(int target) {
	target_enemy = target;
}

void Game_Battle::ClearTargetEnemy() {
	target_enemy = -1;
}

Battle::Enemy& Game_Battle::GetTargetEnemy() {
	return enemies[target_enemy];
}

bool Game_Battle::HaveTargetEnemy() {
	return target_enemy >= 0;
}

void Game_Battle::TargetNextEnemy() {
	target_enemy++;
	target_enemy %= enemies.size();
}

void Game_Battle::TargetPreviousEnemy() {
	target_enemy--;
	target_enemy += enemies.size();
	target_enemy %= enemies.size();
}

void Game_Battle::SetActiveEnemy(int active) {
	active_enemy = active;
}

Battle::Enemy& Game_Battle::GetActiveEnemy() {
	return enemies[active_enemy];
}

void Game_Battle::TargetRandomEnemy() {
	target_enemy = rand() % enemies.size();
}

bool Game_Battle::NextActiveEnemy() {
	for (std::vector<Battle::Enemy>::iterator it = enemies.begin(); it != enemies.end(); it++) {
		if (it->IsReady()) {
			active_enemy = it->ID;
			it->gauge = 0;
			return true;
		}
	}

	return false;
}

////////////////////////////////////////////////////////////
void Game_Battle::ChooseEnemy() {
	if (target_enemy < 0)
		target_enemy = 0;

	if (GetTargetEnemy().game_enemy->Exists())
		return;

	for (int i = 1; i < (int) enemies.size(); i++) {
		TargetNextEnemy();
		if (GetTargetEnemy().game_enemy->Exists())
			break;
	}

	if (!GetTargetEnemy().game_enemy->Exists())
		ClearTargetEnemy();
}

////////////////////////////////////////////////////////////
int Game_Battle::GetActiveActor() {
	Battle::Ally& ally = HaveActiveAlly() ? GetActiveAlly() : GetAlly(0);
	return ally.game_actor->GetId();
}

////////////////////////////////////////////////////////////
int Game_Battle::GetTurns() {
	return turn_fragments / turn_length;
}

////////////////////////////////////////////////////////////
void Game_Battle::Update() {
	turn_fragments++;

	for (std::vector<Battle::Ally>::iterator it = allies.begin(); it != allies.end(); it++) {
		if (it->gauge < Battle::Battler::gauge_full) {
			// FIXME: this should account for agility, paralysis, etc
			it->gauge += it->speed;
		}
	}

	for (std::vector<Battle::Enemy>::iterator it = enemies.begin(); it != enemies.end(); it++) {
		if (it->gauge < Battle::Battler::gauge_full) {
			// FIXME: this should account for agility, paralysis, etc
			it->gauge += it->speed;
		}
	}
}

////////////////////////////////////////////////////////////
bool Game_Battle::HaveCorpse() {
	for (std::vector<Battle::Ally>::iterator it = allies.begin(); it != allies.end(); it++)
		if (it->GetActor()->IsDead())
			return true;
	return false;
}

////////////////////////////////////////////////////////////
bool Game_Battle::CheckWin() {
	for (std::vector<Battle::Enemy>::iterator it = Game_Battle::enemies.begin(); it != Game_Battle::enemies.end(); it++)
		if (!it->game_enemy->IsDead())
			return false;
	return true;
}

bool Game_Battle::CheckLose() {
	for (std::vector<Battle::Ally>::iterator it = Game_Battle::allies.begin(); it != Game_Battle::allies.end(); it++)
		if (!it->GetActor()->IsDead())
			return false;
	return true;
}

////////////////////////////////////////////////////////////
