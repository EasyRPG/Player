
#include "GameOver_scene.h"


/*GO_Scene::~GO_Scene()
{

}
GO_Scene:: GO_Scene()
{

}
*/

void GO_Scene::init(Audio * theaudio, bool * run,unsigned char * TheScene,Player_Team * TheTeam)
{    
	myteam=TheTeam;
	(*myteam).clear_team();
	(*myteam).clear_obj();
	myaudio=theaudio;
	(*myaudio).musicload("../Music/2003sorrow.mid");
	title.x=0;
	title.y=0;
	title.setimg("../GameOver/gameover.png");
	running=  run;
	NScene=TheScene;
}

void GO_Scene::update(SDL_Surface* Screen)
{
	title.draw(Screen);
}

void GO_Scene::action()
{
	*NScene=0;
}

void GO_Scene::updatekey() 
{
	unsigned char * keyData;
	static int delay=0;
	delay++;
	if(delay==40)
	{
		keyData = SDL_GetKeyState(NULL);
		if ( keyData[SDLK_ESCAPE] ) 
		{
			* running = false;
		}
		if ( keyData[LMK_Z]  ) 
		{
			action();
		}
		delay=0;
	}
}

void GO_Scene::dispose() {
	title.dispose();
}
