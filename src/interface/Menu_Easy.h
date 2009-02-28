/*Menu_Easy.h, EasyRPG player Menu_Easy class declaration file.
    Copyright (C) 2007 EasyRPG Project <http://easyrpg.sourceforge.net/>.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef MENU_EASY_H_
#define MENU_EASY_H_

#include "../tools/audio.h"
#include "../tools/Sound_Manager.h"
#include "../tools/key.h"
#include "../attributes/Player_Team.h"

typedef struct menucomand
{
	int des1;
	int des2;
	int selected_moster;
} MC;

class Menu_Easy {

protected:
	int indexX,indexY;
	int maxindexY,maxindexX;
	Player_Team * Team;
	bool * running;
	bool desided;
	int delay;
    std::string system_Cursor;
    std::string system_Accept;
    std::string system_Cancel;
public:
	//Menu_Easy();		///constructor
	//~Menu_Easy();		///destructor
	bool cancel;
	bool desition();
	int getindexY();
	int getindexX();
	void updatekey();
	void restarmenu();
	void init(Player_Team * TheTeam, bool * run,int X,int Y);
};

#endif
