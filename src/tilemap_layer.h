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

#ifndef _TILEMAP_LAYER_H_
#define _TILEMAP_LAYER_H_

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include <vector>
#include "drawable.h"
#include "bitmap.h"
#include "sprite.h"

////////////////////////////////////////////////////////////
/// TilemapLayer class
////////////////////////////////////////////////////////////
class TilemapLayer : public Drawable {
public:
    TilemapLayer(int ilayer);
    ~TilemapLayer();

    void Draw();

    Bitmap* GetChipset();
    void SetChipset(Bitmap* nchipset);
    std::vector< std::vector<int> > GetMapData();
    void SetMapData(std::vector< std::vector<int> > nmap_data);
    bool GetVisible();
    void SetVisible(bool nvisible);
    int GetOx();
    void SetOx(int nox);
    int GetOy();
    void SetOy(int noy);

    int GetZ();
    unsigned long GetId();

private:
    Bitmap* chipset;
    std::vector< std::vector<int> > map_data;
    bool visible;
    int ox;
    int oy;

    unsigned long ID;
    int layer;

    std::vector< std::vector<Sprite*> > sprites;
    std::vector<Bitmap*> autotiles;
};

#endif
