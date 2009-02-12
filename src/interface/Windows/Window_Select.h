/*Window_Select.h, EasyRPG player Window_Select class declaration file.
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

#ifndef WINDOW_SELECT_H_
#define WINDOW_SELECT_H_

#include "../../sprites/Sistem.h"
#include "../Menu_Easy.h"
#include "../../sprites/Sistem.h"
#include "../../tools/font.h"
#include "../../sprites/sprite.h"
#include <vector>
#include <string>

class Window_Select   {

protected:

	Sistem System;
	Sprite tapiz;
	Sprite cursor;
	Font fuente;

	std:: vector <Sprite> Vtext_Sprite;
	int pos_X,pos_Y;
	int Init_text_X;
	int Size_X,Size_Y,Size_of_Block;
	int Comand_X,Comand_Y;
	std:: vector <std::string> * My_vector;
	std:: vector <Sprite> command_sprites;
	std:: vector <Sprite> sha_command_sprites;
	bool disposing;
	Sprite text, sha_text;

public:
    Menu_Easy menu;
	//Window_Select();		///constructor
	//~Window_Select();		///destructor
	bool visible;
	bool on_use;
	void updatekey();
	int getindexX();
	int getindexY();
	void restarmenu();
	bool desition();
	void dispose();
	void init(Audio * theaudio, bool * run,int ComandX,int ComandY,int SizeX,int SizeY,int PosX,int PosY, const char *SysIMg);
	void draw(SDL_Surface* Screen);
	void add_text(std::string ctext, int x, int y);
	void set_posx_text(int x);
	void setComands(std:: vector <std::string> * str_Vec);
};

#endif
