/*CActor.h, EasyRPG player CActor class declaration file.
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

#ifndef CACTOR_H
#define CACTOR_H

#include "../tools/control.h"
#include "../sprites/chipset.h"
#include "../attributes/Player_Team.h"


struct mot
{
	sll distance;
	sll delta;
	unsigned char direction;
};


class CActor:public Chara {

unsigned char * NScene;
Chipset * World;
std:: vector <Chara> * NPC;
Player_Team * myteam;

// Methods
public:
    void set_dir(int the_dir);
    int get_dir();
    bool tried_to_talk;
	void MoveOnInput();
	bool npc_colision(int x, int y);
    bool npc_subcolision(int id);
	int Min(int value, int max);
	sll Minf(float value, float max);
	int Clamp(int value, int min, int max);
	sll Clampf(float value, float min, float max);
    void setposXY(int x,int y,Chipset * the_World,std:: vector <Chara> * Charas_nps,unsigned char *TheScene, Player_Team *TheTeam);
    unsigned char  flags;
	unsigned char state;
	mot Cmotion;
	int GridX;///aparte de la X  y Y  tenemos la poscion con referencia bloques.
	int GridY;
	sll realX;
	sll realY;
	bool outofarea;
};

#endif /* CActor */
