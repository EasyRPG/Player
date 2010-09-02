/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
// 
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#ifndef _TILEMAP_H_
#define _TILEMAP_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <vector>
#include "tilemap_layer.h"
#include "bitmap.h"

////////////////////////////////////////////////////////////
/// Tilemap class
////////////////////////////////////////////////////////////
class Tilemap {
public:
	Tilemap();
	~Tilemap();

	Bitmap* GetChipset() const;
	void SetChipset(Bitmap* nchipset);
	std::vector<short> GetMapDataDown() const;
	std::vector<short> GetMapDataUp() const;
	void SetMapDataDown(std::vector<short> down);
	void SetMapDataUp(std::vector<short> up);
	bool GetVisible() const;
	void SetVisible(bool nvisible);
	int GetOx() const;
	void SetOx(int nox);
	int GetOy() const;
	void SetOy(int noy);

private:
	Bitmap* chipset;
	std::vector<short> map_data_down;
	std::vector<short> map_data_up;
	bool visible;
	int ox;
	int oy;

	TilemapLayer* layer_down;
	TilemapLayer* layer_up;
};

#endif
