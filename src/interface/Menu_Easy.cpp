/*Menu_Easy.cpp, Menu_Easy routines.
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

#include "Menu_Easy.h"
#include "../tools/control.h"

void Menu_Easy::init(Player_Team * TheTeam, bool * run,int X,int Y)
{
	Team=TheTeam;
	desided=false;
	maxindexY=Y;
	maxindexX=X;
	delay=0;
	running=  run;
	cancel=false;
    indexX=0;
    indexY=0;

	system_Cursor.clear();
    system_Cursor.append("Sound/");
    system_Cursor.append(Team->data2.System_dat.Cursor_SFX.Name_of_Sound_effect);
    system_Cursor.append(".wav");

	system_Accept.clear();
    system_Accept.append("Sound/");
    system_Accept.append(Team->data2.System_dat.Accept_SFX.Name_of_Sound_effect);
    system_Accept.append(".wav");

	system_Cancel.clear();
    system_Cancel.append("Sound/");
    system_Cancel.append(Team->data2.System_dat.Cancel_SFX.Name_of_Sound_effect);
    system_Cancel.append(".wav");

}

bool Menu_Easy::desition()
{
	return (desided);
}

void Menu_Easy::restarmenu()
{
	desided=false;
	delay=0;
	indexY=0;
	indexX=0;
	//(*myaudio).freesound();
}

int Menu_Easy::getindexY()
{
	return (indexY);
}

int Menu_Easy::getindexX()
{
	return (indexX);
}

void Menu_Easy::updatekey()
{
int i;
int temp;
temp = Control::pop_LM();
if (desided==false)
        switch (temp)
        {

            default:
                break;
        }

temp = Control::pop_action();

if (desided==false)
        switch (temp)
        {

           case DECISION:
                desided= true;

                i=Team->S_manager.load_sound(system_Accept.c_str());
                Team->S_manager.play_sound(i);
                break;
            case CANCEL:
                i=Team->S_manager.load_sound(system_Cancel.c_str());
                Team->S_manager.play_sound(i);
                cancel=true;
                break;
            case ARROW_UP:
                i=Team->S_manager.load_sound(system_Cursor.c_str());
                Team->S_manager.play_sound(i);
                indexY--;
                break;
            case ARROW_DOWN:
                i=Team->S_manager.load_sound(system_Cursor.c_str());
                Team->S_manager.play_sound(i);
                indexY++;
                break;
            case ARROW_RIGHT:
                i=Team->S_manager.load_sound(system_Cursor.c_str());
                Team->S_manager.play_sound(i);
                indexX++;
                break;
            case ARROW_LEFT:
                i=Team->S_manager.load_sound(system_Cursor.c_str());
                Team->S_manager.play_sound(i);
                indexX--;
                break;
            case SHIFT:
                break;
            case EXIT:
                *running = false;
                break;
            default:
                break;
        }
		if (indexX<0)
		{
			indexX=maxindexX;
		}

		if (indexX>maxindexX)
		{
			indexX=0;
		}

		if (indexY<0)
		{
			indexY=maxindexY;
		}

		if (indexY>maxindexY)
		{
		indexY=0;
		}
}

