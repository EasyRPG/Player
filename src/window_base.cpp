#include "window_base.h"

Window_Base::Window_Base(int ix, int iy, int iwidth, int iheight)
{
	windowskin_id = Main_Data::game_system->get_windowskin_id();
    windowskin = RPG::Cache.windowskin(windowskin_id);

    x = ix;
    y = iy;
    width = iwidth;
    height = iheight;
    z = 100;
    needs_refresh = true;
}

Window_Base::~Window_Base() 
{
	
}

void Window_Base::dispose() 
{
	Window::dispose();
	if(contents != NULL) {
      contents->dispose();
	}
	if(windowskin != NULL) { // Remove when cache
		windowskin->dispose();
	}
}

void Window_Base::update() {
	Window::update();
	/*if(Main_Data::game_system->get_windowskin_name() != windowskin_name) {
		//windowskin_name = Main_Data::game_system->get_windowskin_name();
		if(windowskin != NULL) {
			windowskin->dispose(); // Remove when cache
		}
		windowskin = new Bitmap(windowskin_name); //RPG::Cache.windowskin(windowskin_name);
	}*/
}

void Window_Base::draw_actor_graphic(Game_Actor* actor, int cx, int cy) {
	
}
void Window_Base::draw_actor_name(Game_Actor* actor, int cx, int cy) {
	
}
void Window_Base::draw_actor_class(Game_Actor* actor, int cx, int cy) {
	
}
void Window_Base::draw_actor_level(Game_Actor* actor, int cx, int cy) {
	
}
std::string Window_Base::make_battler_state_text(Game_Battler* battler, int width, bool need_normal) {
	return "";
}
void Window_Base::draw_actor_state(Game_Actor* actor, int cx, int cy) {
	draw_actor_state(actor, x, y, 120); // 120 is from RMXP, maybe need to be adjusted
}
void Window_Base::draw_actor_state(Game_Actor* actor, int cx, int cy, int width) {
	
}
void Window_Base::draw_actor_exp(Game_Actor* actor, int cx, int cy) {
	
}
void Window_Base::draw_actor_hp(Game_Actor* actor, int cx, int cy) {
	draw_actor_hp(actor, x, y, 144); // 144 is from RMXP, maybe need to be adjusted
}
void Window_Base::draw_actor_hp(Game_Actor* actor, int cx, int cy, int width) {
	
}
void Window_Base::draw_actor_sp(Game_Actor* actor, int cx, int cy) {
	draw_actor_sp(actor, x, y, 144); // 144 is from RMXP, maybe need to be adjusted
}
void Window_Base::draw_actor_sp(Game_Actor* actor, int cx, int cy, int width) {
	
}
void Window_Base::draw_actor_parameter(Game_Actor* actor, int cx, int cy, int type) {
	
}
void Window_Base::draw_item_name(RPG::Item* item, int cx, int cy) {
	
}
