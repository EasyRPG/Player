
#include <deque>
#include <algorithm>
#include "data.h"
#include "game_party.h"
#include "game_temp.h"
#include "game_switches.h"
#include "game_variables.h"
#include "battle_animation.h"
#include "battle_battler.h"
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
	bool terminate;
	bool allies_flee;

	BattleAnimation* animation;
	std::deque<BattleAnimation*> animations;
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
	terminate = false;
	animation = NULL;
	animations.clear();
	allies_flee = false;
}

void Game_Battle::Quit() {
	delete background;
	background = NULL;

	if (animation != NULL)
		delete animation;
	animation = NULL;

	while (!animations.empty()) {
		delete animations.front();
		animations.pop_front();
	}

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
	for (std::vector<Battle::Enemy>::iterator it = enemies.begin(); it != enemies.end(); it++)
		if (!it->game_enemy->IsDead())
			return false;
	return true;
}

bool Game_Battle::CheckLose() {
	for (std::vector<Battle::Ally>::iterator it = allies.begin(); it != allies.end(); it++)
		if (!it->GetActor()->IsDead())
			return false;
	return true;
}

////////////////////////////////////////////////////////////
void Game_Battle::Terminate() {
	terminate = true;
}

////////////////////////////////////////////////////////////
void Game_Battle::ShowAnimation(int animation_id, bool allies, Battle::Ally* ally, Battle::Enemy* enemy, bool wait) {
	const RPG::Animation* rpg_anim = &Data::animations[animation_id - 1];
	int x, y;

	if (ally != NULL) {
		x = ally->sprite->GetX();
		y = ally->sprite->GetY();
	}
	else if (enemy != NULL) {
		x = enemy->sprite->GetX();
		y = enemy->sprite->GetY();
	}
	else if (allies)
		AlliesCentroid(x, y);
	else
		EnemiesCentroid(x, y);

	BattleAnimation* new_animation = new BattleAnimation(x, y, rpg_anim);

	if (wait) {
		if (animation != NULL)
			delete animation;
		animation = new_animation;
	}
	else
		animations.push_back(new_animation);
}

////////////////////////////////////////////////////////////
void Game_Battle::UpdateAnimations() {
	animation->Update();
	if (animation->IsDone()) {
		delete animation;
		animation = NULL;
	}

	for (std::deque<BattleAnimation*>::iterator it = animations.begin(); it != animations.end(); it++) {
		BattleAnimation* anim = *it;
		if (anim == NULL)
			continue;
		anim->Update();
		if (anim->IsDone()) {
			delete anim;
			*it = NULL;
		}
	}

	std::deque<BattleAnimation*>::iterator end;
	end = std::remove(animations.begin(), animations.end(), (BattleAnimation*) NULL);
	animations.erase(end, animations.end());
}

////////////////////////////////////////////////////////////
bool Game_Battle::IsAnimationWaiting() {
	return animation != NULL;
}

////////////////////////////////////////////////////////////
void Game_Battle::ChangeBackground(const std::string& name) {
	delete background;

	background = new Background(name);
}

////////////////////////////////////////////////////////////
void Game_Battle::EnemyEscape() {
	Battle::Enemy& enemy = GetActiveEnemy();

	enemy.fade = 30;
	enemy.escaped = true;
}

////////////////////////////////////////////////////////////
void Game_Battle::MonsterFlee(int id) {
	SetActiveEnemy(id);
	if (GetActiveEnemy().game_enemy->Exists())
		EnemyEscape();
}

////////////////////////////////////////////////////////////
void Game_Battle::MonstersFlee() {
	for (int i = 0; i < (int) enemies.size(); i++)
		MonsterFlee(i);
}

////////////////////////////////////////////////////////////
bool Game_Battle::CheckTurns(int turns, int base, int multiple) {
	return turns >= base && (turns - base) % multiple == 0;
}

bool Game_Battle::CheckCondition(const RPG::TroopPageCondition& condition) {
	if (condition.switch_a && !Game_Switches[condition.switch_a_id])
		return false;

	if (condition.switch_b && !Game_Switches[condition.switch_b_id])
		return false;

	if (condition.variable && !(Game_Variables[condition.variable_id] >= condition.variable_value))
		return false;

	if (condition.turn && !CheckTurns(GetTurns(), condition.turn_b, condition.turn_a))
		return false;

	if (condition.turn_enemy && !CheckTurns(GetEnemy(condition.turn_enemy_id).GetTurns(),
											condition.turn_enemy_b, condition.turn_enemy_a))
		return false;

	if (condition.turn_actor) {
		Battle::Ally* ally = FindAlly(condition.turn_actor_id);
		if (!ally)
			return false;
		if (!CheckTurns(ally->GetTurns(), condition.turn_actor_b, condition.turn_actor_a))
			return false;
	}

    if (condition.enemy_hp) {
		int hp = GetEnemy(condition.enemy_id).GetActor()->GetHp();
		if (hp < condition.enemy_hp_min || hp > condition.enemy_hp_max)
			return false;
	}

    if (condition.actor_hp) {
		Battle::Ally* ally = FindAlly(condition.actor_id);
		if (!ally)
			return false;
		int hp = ally->GetActor()->GetHp();
		if (hp < condition.actor_hp_min || hp > condition.actor_hp_max)
			return false;
	}

    if (condition.command_actor) {
		Battle::Ally* ally = FindAlly(condition.actor_id);
		if (!ally)
			return false;
		if (ally->last_command != condition.command_id)
			return false;
	}

	return true;
}

void Game_Battle::CheckEvents() {
	std::vector<RPG::TroopPage>::const_iterator it;
	for (it = troop->pages.begin(); it != troop->pages.end(); it++) {
		const RPG::TroopPage& page = *it;
		if (!CheckCondition(page.condition))
			continue;
		// FIXME: create interpreter
	}
}

