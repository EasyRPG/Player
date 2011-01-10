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

#ifndef _SCENE_FILE_H_
#define _SCENE_FILE_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <vector>
#include "scene.h"
#include "window_savefile.h"
#include "window_filetitle.h"

////////////////////////////////////////////////////////////
/// Scene_Item class
////////////////////////////////////////////////////////////
class Scene_File : public Scene {

public:
	////////////////////////////////////////////////////////
	/// Constructor.
	/// @param message : title message
	////////////////////////////////////////////////////////
	Scene_File(const std::string& message);

	void Start();
	void Update();
	void Terminate();

	virtual void Action(int index) = 0;

protected:
	void Refresh();

	int index;
	int top_index;
	Window_FileTitle* title_window;
	std::vector<Window_SaveFile*> file_windows;
	std::string message;
};

#endif
