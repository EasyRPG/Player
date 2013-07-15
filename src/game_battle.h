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
#include "rpg_troop.h"
#include "battle_battler.h"

class Spriteset_Battle;

namespace Game_Battle {
	/**
	 * Initialize Game_Battle.
	 */
	void Init();

	/**
	 * Quits (frees) Game_Battle.
	 */
	void Quit();

	/**
	 * Updates the battle state.
	 */
	void Update();

	/**
	 * Gets the game interpreter.
	 *
	 * @return the game interpreter.
	 */
	Game_Interpreter& GetInterpreter();

	Spriteset_Battle& GetSpriteset();

	bool AreConditionsMet(const RPG::TroopPageCondition& condition);
	void UpdateEvents();
	void NextTurn();

	static int turn;

	//// Old battle stuff
	extern const RPG::Troop* troop;
	extern std::vector<Battle::Ally> allies;
	extern std::vector<Battle::Enemy> enemies;
	extern bool active;
	extern int turn_fragments;
	extern bool terminate;
	extern bool allies_flee;
	extern std::string background_name;
	extern const RPG::EnemyAction* enemy_action;

	static const int gauge_full = Battle::Battler::gauge_full;
	static const int turn_length = 333; // frames

	//void Quit();

	Battle::Ally* FindAlly(int actor_id);
	void AlliesCentroid(int& x, int& y);
	void EnemiesCentroid(int& x, int& y);

	Battle::Ally& GetAlly(int i);
	void SetTargetAlly(int target);
	void ClearTargetAlly();
	Battle::Ally& GetTargetAlly();
	bool HaveTargetAlly();
	void TargetNextAlly();
	void TargetPreviousAlly();
	void SetActiveAlly(int active);
	Battle::Ally& GetActiveAlly();
	bool HaveActiveAlly();
	void TargetActiveAlly();
	void TargetRandomAlly();

	Battle::Enemy& GetEnemy(int i);
	void SetTargetEnemy(int target);
	void ClearTargetEnemy();
	Battle::Enemy& GetTargetEnemy();
	bool HaveTargetEnemy();
	void TargetNextEnemy();
	void TargetPreviousEnemy();
	void SetActiveEnemy(int active);
	Battle::Enemy& GetActiveEnemy();
	void TargetRandomEnemy();
	bool NextActiveEnemy();

	void ChooseEnemy();
	int GetActiveActor();
	int GetTurn();
	//void Update();
	bool HaveCorpse();
	bool CheckWin();
	bool CheckLose();
	void Terminate();

	void ChangeBackground(const std::string& name);

	void EnemyEscape();

	void MonsterFlee(int id);
	void MonstersFlee();

	bool CheckTurns(int turns, int base, int multiple);

	void Restart();

	void SetItem(int id);
	void SetSkill(int id);
	void SetMorph(int id);

	bool Escape();
	void Defend();
	void Attack();
	void UseItem();
	void UseSkill();
	/*
	void EnemyAttack(void* target);
	void EnemyDefend();
	void EnemyObserve();
	void EnemyCharge();
	void EnemyDestruct();
	void EnemySkill();
	void EnemyTransform();

	void EnemyActionDone();
	*/
	void AttackEnemy(Battle::Ally& ally, Battle::Enemy& enemy);
	void UseItem(Battle::Ally& ally, const RPG::Item& item);
	void UseItemAlly(Battle::Ally& ally, const RPG::Item& item, Battle::Ally& target);
	void UseSkill(Battle::Ally& ally, const RPG::Skill& skill);
	void UseSkillAlly(Battle::Battler& user, const RPG::Skill& skill, Battle::Battler& target);
	void UseSkillEnemy(Battle::Battler& user, const RPG::Skill& skill, Battle::Battler& target);
	bool EnemyActionValid(const RPG::EnemyAction& action, Battle::Enemy& enemy);
	void EnemyAttackAlly(Battle::Enemy& enemy, Battle::Ally& ally);
	void EnemySkill(Battle::Enemy& enemy, const RPG::Skill& skill);
	const RPG::EnemyAction* ChooseEnemyAction(Battle::Enemy& enemy);
}

