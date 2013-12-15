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

#ifndef _SCENE_BATTLE_RPG2K3_H_
#define _SCENE_BATTLE_RPG2K3_H_

// Headers
#include <deque>
#include "scene_battle.h"
#include "rpg_troopmember.h"
#include "rpg_actor.h"
#include "rpg_enemy.h"
#include "game_actor.h"
#include "game_enemy.h"
#include "scene.h"
#include "background.h"
#include "drawable.h"
#include "zobj.h"
#include "window_help.h"
#include "window_item.h"
#include "window_skill.h"
#include "window_battleoption.h"
#include "window_message.h"

#include "window_command.h"
#include "window_battlemessage.h"
#include "battle_battler.h"
#include "battle_animation.h"
#include <boost/scoped_ptr.hpp>

namespace Battle {
class Action;
class SpriteAction;
}

typedef std::pair<EASYRPG_SHARED_PTR<Sprite>, int> FloatText;

/**
 * Scene_Battle class.
 * Manages the battles.
 */
class Scene_Battle_Rpg2k3 : public Scene_Battle {

public:
	Scene_Battle_Rpg2k3();
	~Scene_Battle_Rpg2k3();

	void Update();

protected:

	void CreateCursors();
	void UpdateCursors();
	void DrawFloatText(int x, int y, int color, const std::string& text, int _duration);

	void RefreshCommandWindow();

	void SetState(Scene_Battle::State new_state);

	bool CheckWin();
	bool CheckLose();
	bool CheckAbort();
	bool CheckFlee();

	void CreateBattleOptionWindow();
	void CreateBattleTargetWindow();
	void CreateBattleCommandWindow();
	void CreateBattleMessageWindow();

	void ProcessActions();
	bool ProcessBattleAction(Game_BattleAlgorithm::AlgorithmBase* action);
	void ProcessInput();

	void OptionSelected();
	void CommandSelected();
	void AttackSelected();
	void DefendSelected();
	void ItemSelected();
	void SkillSelected();
	void EnemySelected();

	void SelectNextActor();

	boost::scoped_ptr<Sprite> ally_cursor, enemy_cursor;
	std::vector<FloatText> floating_texts;

	int battle_action_wait;
	int battle_action_state;
};

#endif
