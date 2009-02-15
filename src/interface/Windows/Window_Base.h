/*Window_Base.h, EasyRPG player Window_Base class declaration file.
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

#ifndef WINDOW_BASE_H_
#define WINDOW_BASE_H_

#include "../../sprites/Sistem.h"
#include "../../tools/font.h"
#include "../../sprites/sprite.h"
#include <vector>
#include <string>
#include <bitset>

class Window_Base  {

protected:
	Sistem System;
	Sprite tapiz;
	Font fuente;
	int pos_X,pos_Y;
	int Size_X,Size_Y;
	std:: vector <Sprite> Vtext_Sprite;
	std:: vector <Sprite*> V_Sprite;//appuntadores a sprites
	Sprite text, sha_text;
	char stringBuffer[255];
	int falla;
	bool disposing;



public:
	//Window_Base();		///constructor
	//~Window_Base();	///destructor
	bool visible;
	void dispose();
	void init(int SizeX, int SizeY, int PosX, int PosY, const std::string& SysIMg);
	void draw(SDL_Surface* Screen);
	void add_text(std::string ctext, int x, int y);
	void add_sprite(Sprite * the_sprite, int x, int y);

};

#endif
