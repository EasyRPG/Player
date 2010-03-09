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
#include "tilemap.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
Tilemap::Tilemap() {
    chipset = NULL;
    visible = true;
    ox = 0;
    oy = 0;
    
    layer_down = new TilemapLayer(0);
    layer_up = new TilemapLayer(1);
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
Tilemap::~Tilemap() {
    delete layer_down;
    delete layer_up;
}

////////////////////////////////////////////////////////////
/// Properties
////////////////////////////////////////////////////////////
Bitmap* Tilemap::GetChipset() {
    return chipset;
}
void Tilemap::SetChipset(Bitmap* nchipset) {
    chipset = nchipset;
    layer_down->SetChipset(nchipset);
    layer_up->SetChipset(nchipset);
}
std::vector< std::vector<int> > Tilemap::GetMapData() {
    return map_data;
}
void Tilemap::SetMapData(std::vector< std::vector<int> > nmap_data) {
    map_data = nmap_data;
    layer_down->SetMapData(nmap_data);
    layer_up->SetMapData(nmap_data);
}
bool Tilemap::GetVisible() {
    return visible;
}
void Tilemap::SetVisible(bool nvisible) {
    visible = nvisible;
    layer_down->SetVisible(nvisible);
    layer_up->SetVisible(nvisible);
}
int Tilemap::GetOx() {
    return ox;
}
void Tilemap::SetOx(int nox) {
    ox = nox;
    layer_down->SetOx(nox);
    layer_up->SetOx(nox);
}
int Tilemap::GetOy() {
    return oy;
}
void Tilemap::SetOy(int noy) {
    oy = noy;
    layer_down->SetOy(noy);
    layer_up->SetOy(noy);
}
