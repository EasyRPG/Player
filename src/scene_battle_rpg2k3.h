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
#include "async_handler.h"

/**
 * Scene_Battle class.
 * Manages the battles.
 */
class Scene_Battle_Rpg2k3 : public Scene_Battle {

public:
	Scene_Battle_Rpg2k3();
	~Scene_Battle_Rpg2k3() override;

	void Update() override;

protected:
	void OnSystem2Ready(FileRequestResult* result);
	void CreateUi() override;

	void CreateBattleTargetWindow();
	void CreateBattleCommandWindow();

	void UpdateCursors();
	void DrawFloatText(int x, int y, int color, const std::string& text);

	void RefreshCommandWindow();

	void SetState(Scene_Battle::State new_state) override;

	bool CheckWin();
	bool CheckLose();
	bool CheckResultConditions();

	void ProcessActions() override;
	bool ProcessBattleAction(Game_BattleAlgorithm::AlgorithmBase* action);
	void ProcessInput() override;

	void OptionSelected();
	void CommandSelected();
	void AttackSelected() override;
	void SubskillSelected();
	void SpecialSelected();

	void Escape();

	void SelectNextActor();

	void ActionSelectedCallback(Game_Battler* for_battler) override;

	void ShowNotification(const std::string& text);

	std::unique_ptr<Sprite> ally_cursor, enemy_cursor;

	struct FloatText {
		std::shared_ptr<Sprite> sprite;
		int remaining_time = 30;
	};

	std::vector<FloatText> floating_texts;

	int battle_action_wait;
	int battle_action_state;
	bool battle_action_need_event_refresh = true;
	int combo_repeat = 1;
	bool play_reflected_anim = false;

	std::unique_ptr<Window_BattleStatus> enemy_status_window;

	std::vector<Game_Battler*> targets;

	int select_target_flash_count = 0;

	FileRequestBinding request_id;
};

#endif
