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

#ifndef EP_SCENE_TELEPORT_H
#define EP_SCENE_TELEPORT_H

// Headers
#include "scene.h"
#include "window_teleport.h"

/**
 * Scene Teleport class.
 * Displays the Teleport locations of the teleport skill.
 */
class Scene_Teleport : public Scene {

public:
	/**
	 * Constructor.
	 */
	Scene_Teleport(Game_Actor& actor, const lcf::rpg::Skill& skill);
	Scene_Teleport(const lcf::rpg::Item& item, const lcf::rpg::Skill& skill);

	void Start() override;
	void vUpdate() override;
	void TransitionOut(SceneType next_scene) override;
private:
	std::unique_ptr<Window_Teleport> teleport_window;

	Game_Actor* actor = nullptr;
	const lcf::rpg::Skill* skill = nullptr;
	const lcf::rpg::Item* item = nullptr;
};

#endif
