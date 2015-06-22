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
#include "scene_battle.h"

#include <boost/scoped_ptr.hpp>

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
	void OnSystem2Ready(FileRequestResult* result);
	void CreateCursors();
	void UpdateCursors();
	void DrawFloatText(int x, int y, int color, const std::string& text, int _duration);

	void RefreshCommandWindow();

	void SetState(Scene_Battle::State new_state);

	bool CheckWin();
	bool CheckLose();
	bool CheckAbort();
	bool CheckFlee();
	bool CheckResultConditions();

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
	void SubskillSelected();

	void Escape();

	void SelectNextActor();

	void ActionSelectedCallback(Game_Battler* for_battler);

	void ShowNotification(const std::string& text);

	boost::scoped_ptr<Sprite> ally_cursor, enemy_cursor;
	std::vector<FloatText> floating_texts;

	int battle_action_wait;
	int battle_action_state;

	boost::scoped_ptr<Window_BattleStatus> enemy_status_window;
};

#endif
