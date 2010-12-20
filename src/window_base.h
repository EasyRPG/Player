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

#ifndef _WINDOW_BASE_H_
#define _WINDOW_BASE_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include "window.h"
#include "game_actor.h"
#include "main_data.h"

////////////////////////////////////////////////////////////
/// Window Base class
////////////////////////////////////////////////////////////
class Window_Base : public Window {
public:
	Window_Base(int ix, int iy, int iwidth, int iheight);
	virtual ~Window_Base();

	void Update();
	void DrawActorGraphic(Game_Actor* actor, int cx, int cy);
	void DrawActorName(Game_Actor* actor, int cx, int cy);
	void DrawActorTitle(Game_Actor* actor, int cx, int cy);
	void DrawActorClass(Game_Actor* actor, int cx, int cy);
	void DrawActorLevel(Game_Actor* actor, int cx, int cy);
	void DrawActorState(Game_Actor* actor, int cx, int cy);
	void DrawActorState(Game_Actor* actor, int cx, int cy, int cwidth);
	void DrawActorExp(Game_Actor* actor, int cx, int cy);
	void DrawActorHp(Game_Actor* actor, int cx, int cy);
	void DrawActorHp(Game_Actor* actor, int cx, int cy, int cwidth);
	void DrawActorSp(Game_Actor* actor, int cx, int cy);
	void DrawActorSp(Game_Actor* actor, int cx, int cy, int cwidth);
	void DrawActorParameter(Game_Actor* actor, int cx, int cy, int type);
	void DrawEquipmentType(Game_Actor* actor, int cx, int cy, int type);
	void DrawItemName(RPG::Item* item, int cx, int cy, bool enabled = true);
	void DrawSkillName(RPG::Skill* skill, int cx, int cy, bool enabled = true);

private:
	std::string windowskin_name;
};

#endif
