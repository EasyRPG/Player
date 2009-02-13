#include "message.h"
message_options CMessage::opt =
{
    false,
    DOWN,
    false
};

bool CMessage::is_visible = false;

CMessage::CMessage(const std::string& sys)
{
    const int SizeX = 320;
    const int SizeY = 80;
    const int PosX = 0;
    const int PosY = 80 * opt.place;
	System.init_Sistem();
	System.setimg(sys);
	tapiz.set_surface(System.Exdraw(SizeX,SizeY));
	tapiz.x = PosX;
	tapiz.y = PosY;
	fuente.init_Font();
	pos_X = PosX;
	pos_Y = PosY;
	Size_X = SizeX;
	Size_Y = SizeY;
	disposing = false;
	visible = true;

    type_set.set('c');
    type_set.set('s');
}

void CMessage::add_text(const std::string& ctext, int line)
{
    int foo;
    foo = line;
	text.x = pos_X+9;
	text.y = pos_Y+9;

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

    int lost_space=0;

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

}
