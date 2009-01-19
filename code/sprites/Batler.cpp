/*Batler.cpp, sprite routines.
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
/*

void Batler::init_Batler()//esto es asi porque no se me ocurre aun algo mejor
{
	    
	int i,j
	x = 50;
	y = 50;
	frame = 2;
	dir = 0;
	cols=4;
	rows=6;
	for(i=0;i<cols;i++)
	{
		for(j=0;j<rows;j++)
		{
			animation[i][j] = i+j%4;
		}
	}
}

void Batler::frameupdate()
{
	static long delay=0;
	delay++;
	if(delay==10)
	{
		if(frame==3)
		endanimation=true;
		frame= (frame +1)%4;
		delay=0;
	}
}

int Batler::Get_center_X()
{
	return(x+getw()/2)
}
int Batler::Get_center_Y()
{
	return(y+geth()/2)
}

bool Batler::Move_to_XY(int moves, int posX,int posY)//movimiento de vector
{
	int MyX=Get_center_X();
	int MyY=Get_center_Y();

	if((MyY==posX)&& (MyX==posY))
	{
		return (true);
	}

	if(MyX!=posX)
	{ 
		x=moves+x;
		if(MyY!=posY)
		{
		y=((posY-MyY)/(posX-MyX))*moves+y;
		}
	}
	else
	{
		y=y+moves;
	}
	return (false);
}

bool Batler::Move_to(Batler Bt,int moves)//movimiento de vector
{
	int YourX=Bt.Get_center_X();
	int YourY=Bt.Get_center_Y();
	return ( Move_to_XY(moves,YourX,YourY) );
}

void Batler::draw (SDL_Surface * screen)
{    
	int realframe;
	int w = getw();
	int h = geth();
	realframe=animation [dir][frame];
	SDL_Rect fuente = {(realframe%cols)* w,(realframe/cols) * h, w, h};
	SDL_Rect rect = {x, y, 0, 0};
	SDL_BlitSurface (img, & fuente,	screen, &rect);
}
*/

