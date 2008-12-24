
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

