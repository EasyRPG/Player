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

#ifndef _SCENE_ACTORTARGET_H_
#define _SCENE_ACTORTARGET_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "scene.h"
#include "window_actortarget.h"
#include "window_help.h"
#include "window_targetstatus.h"

////////////////////////////////////////////////////////////
/// Scene ActorTarget class.
/// Manages using of Items and Spells.
////////////////////////////////////////////////////////////
class Scene_ActorTarget : public Scene {

public:
	////////////////////////////////////////////////////////
	/// Constructor.
	/// @param item_id : Item id of item to use
	/// @param item_index : Index of item in Scene_Item
	////////////////////////////////////////////////////////
	Scene_ActorTarget(int item_id, int item_index);

	////////////////////////////////////////////////////////
	/// Constructor.
	/// @param skill_id : Skill id of skill to use
	/// @param actor_index: Index of the spell caster in party
	/// @param skill_index : Index of skill in Scene_Skill
	////////////////////////////////////////////////////////
	Scene_ActorTarget(int skill_id, int actor_index, int skill_index);

	void Start();
	void Update();
	void Terminate();

	////////////////////////////////////////////////////////
	/// Update function used when an item will be used.
	////////////////////////////////////////////////////////
	void UpdateItem();

	////////////////////////////////////////////////////////
	/// Update function used when a skill will be used.
	////////////////////////////////////////////////////////
	void UpdateSkill();

private:
	/// Contains the actors of the party
	Window_ActorTarget* target_window;
	/// Contains the name of the item/skill that will be used
	Window_Help* help_window;
	/// Contains quantity/cost of item/spell
	Window_TargetStatus* status_window;

	/// id of item/skill to use
	int id;
	/// index of item/skill in previous scene
	int index;
	/// index of spell caster in party (only for skills)
	int actor_index;
	/// True if item, false if skill
	bool use_item;
};

#endif
