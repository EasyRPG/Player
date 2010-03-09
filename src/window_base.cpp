//////////////////////////////////////////////////////////////////////////////////
/// This file is part of EasyRPG Player.
/// 
/// EasyRPG Player is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
/// 
/// EasyRPG Player is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
/// 
/// You should have received a copy of the GNU General Public License
/// along with EasyRPG Player.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include "window_base.h"
#include "cache.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Window_Base::Window_Base(int ix, int iy, int iwidth, int iheight) {
    windowskin_name = Main_Data::game_system->GetSystemName();
    windowskin = Cache::System(windowskin_name);

    SetX(ix);
    SetY(iy);
    SetWidth(iwidth);
    SetHeight(iheight);
    SetZ(100);
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Window_Base::~Window_Base() {
}

////////////////////////////////////////////////////////////
/// Update
////////////////////////////////////////////////////////////
void Window_Base::Update() {
    Window::Update();
    if (Main_Data::game_system->GetSystemName() != windowskin_name) {
        windowskin_name = Main_Data::game_system->GetSystemName();
        windowskin = Cache::System(windowskin_name);
    }
}

////////////////////////////////////////////////////////////
/// Draw helpers
////////////////////////////////////////////////////////////
void Window_Base::DrawActorGraphic(Game_Actor* actor, int cx, int cy) {
    
}
void Window_Base::DrawActorName(Game_Actor* actor, int cx, int cy) {
    
}
void Window_Base::DrawActorTitle(Game_Actor* actor, int cx, int cy) {
    
}
void Window_Base::DrawActorClass(Game_Actor* actor, int cx, int cy) {
    
}
void Window_Base::DrawActorLevel(Game_Actor* actor, int cx, int cy) {
    
}
void Window_Base::DrawActorState(Game_Actor* actor, int cx, int cy) {
    DrawActorState(actor, x, y, 120); // 120 is from RMXP, maybe need to be adjusted
}
void Window_Base::DrawActorState(Game_Actor* actor, int cx, int cy, int width) {
    
}
void Window_Base::DrawActorExp(Game_Actor* actor, int cx, int cy) {
    
}
void Window_Base::DrawActorHp(Game_Actor* actor, int cx, int cy) {
    DrawActorHp(actor, x, y, 144); // 144 is from RMXP, maybe need to be adjusted
}
void Window_Base::DrawActorHp(Game_Actor* actor, int cx, int cy, int width) {
    
}
void Window_Base::DrawActorSp(Game_Actor* actor, int cx, int cy) {
    DrawActorSp(actor, x, y, 144); // 144 is from RMXP, maybe need to be adjusted
}
void Window_Base::DrawActorSp(Game_Actor* actor, int cx, int cy, int width) {
    
}
void Window_Base::DrawActorParameter(Game_Actor* actor, int cx, int cy, int type) {
    
}
void Window_Base::DrawItemName(RPG::Item* item, int cx, int cy) {
    
}
