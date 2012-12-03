
#include <deque>
#include <algorithm>
#include "data.h"
#include "game_party.h"
#include "game_temp.h"
#include "game_switches.h"
#include "game_variables.h"
#include "game_interpreter_battle.h"
#include "battle_animation.h"
#include "battle_battler.h"
#include "game_battle.h"
#include <boost/scoped_ptr.hpp>

namespace Game_Battle {
	Battle_Interface* scene;

	const RPG::Troop* troop;
	std::vector<Battle::Ally> allies;
	std::vector<Battle::Enemy> enemies;

	int turn_fragments;
	int target_enemy;
	int target_ally;
	int active_enemy;
	int active_ally;
	bool terminate;
	bool allies_flee;
	int item_id;
	int skill_id;
	int morph_id;
	std::string background_name;
	const RPG::EnemyAction* enemy_action;

	const RPG::TroopPage* script_page;
	boost::scoped_ptr<Game_Interpreter> interpreter;
}

////////////////////////////////////////////////////////////
void Game_Battle::Init(Battle_Interface* _scene) {
	scene = _scene;

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

	background_name = Game_Temp::battle_background;

	active_enemy = -1;
	active_ally = -1;
	target_enemy = -1;
	target_ally = -1;
	terminate = false;
	allies_flee = false;
	item_id = -1;
	skill_id = -1;
	morph_id = -1;

	script_page = NULL;

	interpreter.reset(new Game_Interpreter_Battle());
}

void Game_Battle::Quit() {
	// Remove conditions which end after battle
	for (std::vector<Battle::Ally>::iterator it = allies.begin(); it != allies.end(); it++)
		it->GetActor()->RemoveStates();

	allies.clear();
	enemies.clear();

	interpreter.reset();

	scene = NULL;
}

////////////////////////////////////////////////////////////
Battle_Interface* Game_Battle::GetScene() {
	return scene;
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
void Game_Battle::ChangeBackground(const std::string& name) {
	background_name = name;
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
	if (condition.flags.switch_a && !Game_Switches[condition.switch_a_id])
		return false;

	if (condition.flags.switch_b && !Game_Switches[condition.switch_b_id])
		return false;

	if (condition.flags.variable && !(Game_Variables[condition.variable_id] >= condition.variable_value))
		return false;

	if (condition.flags.turn && !CheckTurns(GetTurns(), condition.turn_b, condition.turn_a))
		return false;

	if (condition.flags.turn_enemy && !CheckTurns(GetEnemy(condition.turn_enemy_id).GetTurns(),
											condition.turn_enemy_b, condition.turn_enemy_a))
		return false;

	if (condition.flags.turn_actor) {
		Battle::Ally* ally = FindAlly(condition.turn_actor_id);
		if (!ally)
			return false;
		if (!CheckTurns(ally->GetTurns(), condition.turn_actor_b, condition.turn_actor_a))
			return false;
	}

    if (condition.flags.enemy_hp) {
		int hp = GetEnemy(condition.enemy_id).GetActor()->GetHp();
		if (hp < condition.enemy_hp_min || hp > condition.enemy_hp_max)
			return false;
	}

    if (condition.flags.actor_hp) {
		Battle::Ally* ally = FindAlly(condition.actor_id);
		if (!ally)
			return false;
		int hp = ally->GetActor()->GetHp();
		if (hp < condition.actor_hp_min || hp > condition.actor_hp_max)
			return false;
	}

    if (condition.flags.command_actor) {
		Battle::Ally* ally = FindAlly(condition.actor_id);
		if (!ally)
			return false;
		if (ally->last_command != condition.command_id)
			return false;
	}

	return true;
}

void Game_Battle::CheckEvents() {
	const RPG::TroopPage* new_page = NULL;
	std::vector<RPG::TroopPage>::const_reverse_iterator it;
	for (it = troop->pages.rbegin(); it != troop->pages.rend(); it++) {
		const RPG::TroopPage& page = *it;
		if (CheckCondition(page.condition)) {
			new_page = &*it;
			break;
		}
	}

	if (new_page != NULL && new_page != script_page)
		interpreter->Setup(new_page->event_commands, 0);
}

////////////////////////////////////////////////////////////
void Game_Battle::Restart() {
	scene->Restart();
	Battle::Ally& ally = GetActiveAlly();
	ally.NextTurn();
	ClearTargetAlly();
	ClearTargetEnemy();
}

////////////////////////////////////////////////////////////
void Game_Battle::SetItem(int id) {
	item_id = id;
}

void Game_Battle::SetSkill(int id) {
	skill_id = id;
}

void Game_Battle::SetMorph(int id) {
	morph_id = id;
}

////////////////////////////////////////////////////////////
bool Game_Battle::Escape() {
	if (Game_Temp::battle_escape_mode != 0) {
		// FIXME: escape probability
		Game_Temp::battle_result = Game_Temp::BattleEscape;
		return true;
	}

	return false;
}

void Game_Battle::Defend() {
	Battle::Ally& ally = GetActiveAlly();
	ally.defending = true;
}

void Game_Battle::Attack() {
	Battle::Ally& ally = GetActiveAlly();
	Battle::Enemy& enemy = GetTargetEnemy();
	AttackEnemy(ally, enemy);
	ally.defending = false;
}

void Game_Battle::UseItem() {
	Battle::Ally& ally = GetActiveAlly();
	const RPG::Item& item = Data::items[item_id - 1];
	UseItem(ally, item);
	ally.defending = false;
}

void Game_Battle::UseSkill() {
	const RPG::Skill& skill = Data::skills[skill_id - 1];
	Battle::Ally& ally = GetActiveAlly();
	UseSkill(ally, skill);
	ally.defending = false;
}

////////////////////////////////////////////////////////////
void Game_Battle::EnemyAttack(void* target) {
	Battle::Enemy& enemy = GetActiveEnemy();
	Battle::Ally& ally = *((Battle::Ally*)target);

	EnemyAttackAlly(enemy, ally);
}

void Game_Battle::EnemyDefend() {
	Battle::Enemy& enemy = GetActiveEnemy();
	const std::string msg = !Data::terms.defending.empty()
		? Data::terms.defending
		: " is defending";
	GetScene()->Message(enemy.rpg_enemy->name + msg);
	enemy.defending = true;
}

void Game_Battle::EnemyObserve() {
	Battle::Enemy& enemy = GetActiveEnemy();
	const std::string msg = !Data::terms.observing.empty()
		? Data::terms.observing
		: " is observing the battle";
	GetScene()->Message(enemy.rpg_enemy->name + msg);
}

void Game_Battle::EnemyCharge() {
	Battle::Enemy& enemy = GetActiveEnemy();
	const std::string msg = !Data::terms.focus.empty()
		? Data::terms.focus
		: " is charging";
	GetScene()->Message(enemy.rpg_enemy->name + msg);
	enemy.charged = true;
}

void Game_Battle::EnemyDestruct() {
	Battle::Enemy& enemy = GetActiveEnemy();
	const std::string msg = !Data::terms.autodestruction.empty()
		? Data::terms.autodestruction
		: "Self-destruct";
	GetScene()->Message(msg);
	enemy.charged = true;
	for (std::vector<Battle::Ally>::iterator it = allies.begin(); it != allies.end(); it++)
		EnemyAttack((void*)&*it);
	enemy.game_enemy->SetHp(0);
}

void Game_Battle::EnemySkill() {
	const RPG::Skill& skill = Data::skills[skill_id - 1];
	Battle::Enemy& enemy = GetActiveEnemy();
	EnemySkill(enemy, skill);
}

void Game_Battle::EnemyTransform() {
	Battle::Enemy& enemy = GetActiveEnemy();
	enemy.Transform(morph_id);
}

////////////////////////////////////////////////////////////
void Game_Battle::EnemyActionDone() {
	if (enemy_action->switch_on)
		Game_Switches[enemy_action->switch_on_id] = true;

	if (enemy_action->switch_off)
		Game_Switches[enemy_action->switch_off_id] = false;
}
