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

#ifndef _SCENE_H_
#define _SCENE_H_

////////////////////////////////////////////////////////////
/// Scene types
////////////////////////////////////////////////////////////
#define SCENE_NULL 0
#define SCENE_TITLE 1
#define SCENE_MAP 2
#define SCENE_MENU 3
#define SCENE_ITEM 4
#define SCENE_SKILL 5
#define SCENE_EQUIP 6
#define SCENE_STATUS 7
#define SCENE_FILE 8
#define SCENE_SAVE 9
#define SCENE_LOAD 10
#define SCENE_END 11
#define SCENE_BATTLE 12
#define SCENE_SHOP 13
#define SCENE_NAME 14
#define SCENE_GAMEOVER 15
#define SCENE_DEBUG 16

////////////////////////////////////////////////////////////
/// Scene virtual class
////////////////////////////////////////////////////////////
class Scene {
public:
	virtual ~Scene() {};

	virtual void MainFunction() =0;
};

#endif
