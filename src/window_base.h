#include <string>
#include "window.h"
#include "main_data.h"

class Window_Base : public Window {
	public :
		Window_Base(int ix, int iy, int iwidth, int iheight);
		~Window_Base();
		
		void dispose();
		void update();
		void draw_actor_graphic(Game_Actor* actor, int cx, int cy);
		void draw_actor_name(Game_Actor* actor, int cx, int cy);
		void draw_actor_class(Game_Actor* actor, int cx, int cy);
		void draw_actor_level(Game_Actor* actor, int cx, int cy);
		std::string make_battler_state_text(Game_Battler* battler, int cwidth, bool need_normal);
		void draw_actor_state(Game_Actor* actor, int cx, int cy);
		void draw_actor_state(Game_Actor* actor, int cx, int cy, int cwidth);
		void draw_actor_exp(Game_Actor* actor, int cx, int cy);
		void draw_actor_hp(Game_Actor* actor, int cx, int cy);
		void draw_actor_hp(Game_Actor* actor, int cx, int cy, int cwidth);
		void draw_actor_sp(Game_Actor* actor, int cx, int cy);
		void draw_actor_sp(Game_Actor* actor, int cx, int cy, int cwidth);
		void draw_actor_parameter(Game_Actor* actor, int cx, int cy, int type);
		void draw_item_name(RPG::Item* item, int cx, int cy);
		
		std::string windowskin_name;
};
