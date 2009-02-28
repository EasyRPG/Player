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

void Menu_Easy::init(Sound_Manager * theaudio, bool * run,int X,int Y)
{
	myaudio=theaudio;
	desided=false;
	maxindexY=Y;
	maxindexX=X;
	delay=0;
	running=  run;
	cancel=false;
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
                i=myaudio->load_sound("Sound/Decision2.wav");
                myaudio->play_sound(i);
                break;
            case CANCEL:
                i=myaudio->load_sound("Sound/Cancelar.wav");
                myaudio->play_sound(i);
                cancel=true;
                break;
            case ARROW_UP:
                i=myaudio->load_sound("Sound/Cursor1.wav");
                myaudio->play_sound(i);
                indexY--;
                break;
            case ARROW_DOWN:
                i=myaudio->load_sound("Sound/Cursor1.wav");
                myaudio->play_sound(i);
                indexY++;
                break;
            case ARROW_RIGHT:
                i=myaudio->load_sound("Sound/Cursor1.wav");
                myaudio->play_sound(i);
                indexX++;
                break;
            case ARROW_LEFT:
                i=myaudio->load_sound("Sound/Cursor1.wav");
                myaudio->play_sound(i);
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

