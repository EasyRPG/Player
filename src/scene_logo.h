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

#ifndef _SCENE_LOGO_H_
#define _SCENE_LOGO_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "scene.h"
#include "sprite.h"
#include "system.h"

////////////////////////////////////////////////////////////
/// Scene Logo class.
/// Displays the shiny EasyRPG logo on startup.
////////////////////////////////////////////////////////////
class Scene_Logo : public Scene {

public:
	////////////////////////////////////////////////////////
	/// Constructor.
	////////////////////////////////////////////////////////
	Scene_Logo();

	void Start();
	void PerformTransition();
	void Update();
	void Terminate();

private:
	Sprite* logo;
	Bitmap* logo_img;
	int frame_counter;
};

#endif
