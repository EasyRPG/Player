#include "message.h"

Player_Team *myTeam = NULL;

message_options CMessage::opt =
{
    false,
    DOWN,
    false
};

//bool CMessage::is_visible = false;

//void CMessage::init(const std::string& sys)
CMessage::CMessage(const std::string& sys)
{
    const int SizeX = 320;
    const int SizeY = 80;
    const int PosX = 0;
    const int PosY = 160;
    done = false;
	System.init_Sistem();
	System.setimg(sys);
	tapiz.set_surface(System.Exdraw(SizeX,SizeY));
	tapiz.x = PosX;
	tapiz.y = PosY;
	fuente.init_Font();
	pos_X = PosX;
	pos_Y = PosY * opt.place;
	Size_X = SizeX;
	Size_Y = SizeY;
	disposing = false;
	visible = false;

	blink = 0;
	cursor_visible = false;
	next = false;
    type_set.set('c');
    type_set.set('s');
    type_set.set('n');
}

void CMessage::draw(SDL_Surface *dst)
{
    unsigned int i;
	if (visible)
	{
		if (!disposing)
		{
            tapiz.y = pos_Y;
            if (!opt.transparent) tapiz.draw(dst);
			for (i = 0; i < Vtext_Sprite.size(); i++)
			{
				Vtext_Sprite[i].draw(dst);
			}

			for (i = 0; i < V_Sprite.size(); i++)
			{
				V_Sprite[i]->draw(dst);
			}
            if(done)
            draw_blink(dst);
		}
	}
}



void CMessage::draw_blink(SDL_Surface *dst)
{
   SDL_Rect clip =
	{
	    43,
	    17,
	    10,
	     6
    };
    SDL_Rect pos =
    {
        155,
        pos_Y+73,
        10,
         6
    };
	if (blink >= 30)
	{
        if (cursor_visible) SDL_BlitSurface(System.get_img(), &clip, dst, &pos);
        blink++;
	}
    else
    {
        blink++;
    }
    if (blink >= 60) blink = 0;
}

void CMessage::clean() //NO BORRAR clean no destruye el objeto, solo limpia el texto
{
    int i, tp;
	tp = Vtext_Sprite.size();
	for (i = 0; i < tp; i ++)
	{
	Vtext_Sprite[i].dispose();
	}
	Vtext_Sprite.clear();
    pos_Y = 80 * opt.place;
}

CMessage::~CMessage()
{
    int i, tp;
	tp = Vtext_Sprite.size();
	for (i = 0; i < tp; i ++)
	{
        Vtext_Sprite[i].dispose();
	}

	Vtext_Sprite.clear();
    tapiz.dispose();
}

void CMessage::add_text(const std::string& ctext, int line)
{
	text.x = pos_X+9;
	text.y = pos_Y+9+(15*line);
	sha_text.x = text.x+1;
	sha_text.y = text.y+1;
	std::string s_tmp;
    int l = ctext.length();
    int i;
    bool state_control = false;
    Uint8 state = 0;
    char type = 0;
    int n = 0;
    int n_color = 0;
    int lost_space = 0;
    SDL_Surface *text_tmp = fuente.create_font_surface(FONT_WIDTH*l, 15);
    SDL_Surface *shadow = fuente.create_font_surface(FONT_WIDTH*l, 15);

    for (i = 0; i < l; i++)
    {
        if (state_control)
        {
            lost_space++;
            switch (state)
            {
                case 0:
                    type = ctext[i];
                    if (!type_set.test(type))
                    {
                        n = 0;
                        state_control = false;
                    }
                    state++;
                    break;
                case 1:
                    if (ctext[i] != '[')
                    {
                        n = 0;
                        state_control = false;
                    }
                    state++;
                    break;
                case 2:
                    if (ctext[i] == ']')
                    {
                        switch (type)
                        {
                            case 'c':
                                (n < 20) ? n_color = n : n_color = 0;
                                break;
                            case 'n':
                                if(myTeam) {
                                    fuente.blit_background(text_tmp, n_color, System.get_img(), i-lost_space);
                                    fuente.blit_shadow(shadow, System.get_img(), i-lost_space);
                                    s_tmp.append( myTeam->get_name(n) );
                                }
                                break;
                            default:
                                break;
                        }
                        n = 0;
                        state_control = false;
                    }
                    else
                    {
                        /* Begin reading number */
                        if (isdigit(ctext[i])) n = n*10 + (ctext[i] - '0'); //Improvised atoi :p
                        else state_control = false;
                    }
                    break;

                default:
                    break;
            }
        }
        else
        {
            state = 0;
            switch (ctext[i])
            {
                case '\\':
                    lost_space++;
                    state_control = true;
                    break;

                default:
                    fuente.blit_background(text_tmp, n_color, System.get_img(), i-lost_space);
                    fuente.blit_shadow(shadow, System.get_img(), i-lost_space);
                    s_tmp.push_back(ctext[i]);
            }
        }
    }
    fuente.blit_font(text_tmp, &s_tmp, i-lost_space, 0);
    fuente.blit_font(shadow, &s_tmp, i-lost_space, 0);

	text.set_surface(text_tmp);
	sha_text.set_surface(shadow);

	Vtext_Sprite.push_back(sha_text);
	Vtext_Sprite.push_back(text);

	cursor_visible = true;
}
