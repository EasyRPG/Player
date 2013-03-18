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

#ifndef _SCENE_STATUS_H_
#define _SCENE_STATUS_H_

// Headers
#include "scene.h"
#include "window_actorinfo.h"
#include "window_actorstatus.h"
#include "window_equip.h"
#include "window_equipstatus.h"
#include "window_gold.h"
#include <boost/scoped_ptr.hpp>

/**
 * Scene Status class.
 * Displays status information about a party member.
 */
class Scene_Status : public Scene {
public:
	/**
	 * Constructor.
	 *
	 * @param actor_index party index of the actor.
	 */
	Scene_Status(int actor_index);

	void Start();
	void Update();

private:
	int actor_index;

	boost::scoped_ptr<Window_ActorInfo> actorinfo_window;
	boost::scoped_ptr<Window_ActorStatus> actorstatus_window;
	boost::scoped_ptr<Window_Gold> gold_window;
	boost::scoped_ptr<Window_EquipStatus> equipstatus_window;
	boost::scoped_ptr<Window_Equip> equip_window;
};

#endif
