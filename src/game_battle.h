#include "rpg_troop.h"
#include "battle_battler.h"
#include "background.h"

namespace Game_Battle {
	extern const RPG::Troop* troop;
	extern std::vector<Battle::Ally> allies;
	extern std::vector<Battle::Enemy> enemies;
	extern Background* background;
	extern bool active;
	extern int turn_fragments;

	static const int gauge_full = Battle::Battler::gauge_full;
	static const int turn_length = 333; // frames

	void Init();
	void Quit();

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
	int GetTurns();
	void Update();
	bool HaveCorpse();
	bool CheckWin();
	bool CheckLose();
}

