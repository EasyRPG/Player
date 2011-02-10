/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#ifndef _SCENE_BATTLE_H_
#define _SCENE_BATTLE_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "scene.h"
#include "background.h"
#include "window_help.h"
#include "window_item.h"
#include "window_skill.h"
#include "window_battlecommand.h"
#include "window_battlestatus.h"

////////////////////////////////////////////////////////////
/// Scene_Battle class.
/// Manages the battles.
////////////////////////////////////////////////////////////
class Scene_Battle : public Scene {

public:
	Scene_Battle();
	~Scene_Battle();

	void Start();
	void Update();

	enum State {
		Options,
		Battle,
		AutoBattle,
		Command,
		Item,
		Skill
	};

	void SetState(State state);
	int GetActiveActor();

private:
	State state;
	int cycle;

	Window_Help* help_window;
	Window_BattleCommand* options_window;
	Window_BattleStatus* status_window;
	Window_BattleCommand* command_window;
	Window_Item* item_window;
	Window_Skill* skill_window;
	Background* background;

	const RPG::Troop* troop;
	std::vector<Sprite*> enemy_sprites;
	std::vector<Sprite*> actor_sprites;
};

#endif
