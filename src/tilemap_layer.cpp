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
#include "tilemap_layer.h"
#include "player.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
TilemapLayer::TilemapLayer(int ilayer) {
    chipset = NULL;
    visible = true;
    ox = 0;
    oy = 0;

    layer = ilayer;

    int lines = (int)(Player::GetWidth() / 16.0f + 1.5);
    int columns = (int)(Player::GetHeight() / 16.0f + 1.5);

    for (int i = 0; i < lines; i++) {
        for (int j = 0; j < columns; j++) {
            Sprite* sprite = new Sprite();
            sprite->SetX(i * 16);
            sprite->SetY(j * 16);
            if (ilayer > 1) {
                int z = j * 16 + 16;
                if (j == 0) z += 16;
                sprite->SetZ(z);
            }
            sprites[i][j] = sprite;
        }
    }
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
TilemapLayer::~TilemapLayer() {
    for (unsigned int i = 0; i < sprites.size(); i++) {
        for (unsigned int j = 0; i < sprites[i].size(); j++) {
            delete sprites[i][j];
        }
    }
}

////////////////////////////////////////////////////////////
/// Draw
////////////////////////////////////////////////////////////
void TilemapLayer::Draw() {
    /*for (int i = 0; i < sprites.size(); i++) {
        for (int j = 0; j < sprites[i].size(); j++) {
            sprite->SetBitmap(tilemap->GetChipset());
        }
    }*/
}

////////////////////////////////////////////////////////////
/// Properties
////////////////////////////////////////////////////////////
Bitmap* TilemapLayer::GetChipset() {
    return chipset;
}
void TilemapLayer::SetChipset(Bitmap* nchipset) {
    chipset = nchipset;
}
std::vector<short> TilemapLayer::GetMapData() {
    return map_data;
}
void TilemapLayer::SetMapData(std::vector<short> nmap_data) {
    map_data = nmap_data;
}
bool TilemapLayer::GetVisible() {
    return visible;
}
void TilemapLayer::SetVisible(bool nvisible) {
    visible = nvisible;
}
int TilemapLayer::GetOx() {
    return ox;
}
void TilemapLayer::SetOx(int nox) {
    ox = nox;
}
int TilemapLayer::GetOy() {
    return oy;
}
void TilemapLayer::SetOy(int noy) {
    oy = noy;
}

////////////////////////////////////////////////////////////
/// Get z
////////////////////////////////////////////////////////////
int TilemapLayer::GetZ() {
    return 0;
}

////////////////////////////////////////////////////////////
/// Get id
////////////////////////////////////////////////////////////
unsigned long TilemapLayer::GetId() {
    return 0;
}
