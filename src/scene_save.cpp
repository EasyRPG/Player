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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <sstream>
#include "data.h"
#include "filefinder.h"
#include "lsd_reader.h"
#include "scene_save.h"
#include "scene_file.h"

////////////////////////////////////////////////////////////
Scene_Save::Scene_Save() :
	Scene_File(Data::terms.save_game_message) {
	Scene::type = Scene::Save;
}

////////////////////////////////////////////////////////////
void Scene_Save::Action(int index) {
	std::stringstream ss;
	ss << "Save" << (index <= 8 ? "0" : "") << (index + 1) << ".lsd";

#ifndef _WIN32
	// Get the case insensitive filename to make sure that only one savefile
	// for every slot exists
	std::string file = FileFinder::FindDefault(".", ss.str());
	if (file.empty())
		file = ss.str();
#else
	std::string file = ss.str();
#endif

	LSD_Reader::Save(file, Main_Data::game_data);
}
