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

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "tilemap_layer.h"
#include "player.h"
#include "graphics.h"

////////////////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////////////////
TilemapLayer::TilemapLayer(int ilayer) {
	chipset = NULL;
	visible = true;
	ox = 0;
	oy = 0;
	width = 0;
	height = 0;

	layer = ilayer;

	int lines = (int)(Player::GetWidth() / 16.0f + 1.5f);
	int columns = (int)(Player::GetHeight() / 16.0f + 1.5f);

	sprites.resize(lines);

	for (int i = 0; i < lines; i++) {
		sprites[i].resize(columns);
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

	ID = Graphics::ID++;
	Graphics::drawable_list.push_back(this);
	Graphics::drawable_list.sort(Graphics::SortDrawable);
}

////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
TilemapLayer::~TilemapLayer() {
	for (unsigned int i = 0; i < sprites.size(); i++) {
		for (unsigned int j = 0; j < sprites[i].size(); j++) {
			delete sprites[i][j];
		}
	}
	Graphics::RemoveDrawable(ID);
}

////////////////////////////////////////////////////////////
/// Draw
////////////////////////////////////////////////////////////
void TilemapLayer::Draw() {
	if (!visible) return;

	int tiles_x = (int)ceil(Player::GetWidth() / 16.0);
	int tiles_y = (int)ceil(Player::GetHeight() / 16.0);

	for (unsigned int i = 0; i < sprites.size(); i++) {
		if (i == sprites.size() - 1 && ox % 16 == 0) continue;
		for (unsigned int j = 0; j < sprites[i].size(); j++) {
			if (j == sprites[i].size() - 1 && oy % 16 == 0) continue;

			int map_x = ox / 16 + i;
			int map_y = oy / 16 + j;

			TileData tile = data_cache[map_x][map_y];
			
			sprites[i][j]->SetZ(tile.z);
			sprites[i][j]->SetOx(ox % 16);
			sprites[i][j]->SetOy(oy % 16);

			if (layer == 0) {
				if (tile.id >= 5000 && tile.id <= 5143) {
					Rect rect;
					rect.x = 192 + ((tile.id - 5000) % 6) * 16 + ((tile.id - 5000) / 96) * 16;
					rect.y = (((tile.id - 5000) / 6) % 16) * 16;
					rect.width = 16;
					rect.height = 16;

					sprites[i][j]->SetSrcRect(rect);
				} else {
					sprites[i][j]->SetVisible(false);
				}
			} else {
				if (tile.id >= 10000 && tile.id <= 10143) {
					Rect rect;
					if (tile.id < 10048) {
						rect.x = 288 + ((tile.id - 10000) % 6) * 16;
						rect.y = 128 + ((tile.id - 10000) / 6) * 16;
					} else {
						rect.x = 384 + ((tile.id - 10048) % 6) * 16;
						rect.y = ((tile.id - 10048) / 6) * 16;
					}
					rect.width = 16;
					rect.height = 16;

					sprites[i][j]->SetSrcRect(rect);
				} else {
					sprites[i][j]->SetVisible(false);
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////
/// Properties
////////////////////////////////////////////////////////////
Bitmap* TilemapLayer::GetChipset() const {
	return chipset;
}
void TilemapLayer::SetChipset(Bitmap* nchipset) {
	if (chipset != nchipset) {
		for (int i = 0; i < sprites.size(); i++) {
			for (int j = 0; j < sprites[i].size(); j++) {
				sprites[i][j]->SetBitmap(nchipset);
			}
		}
	}
	chipset = nchipset;
}
std::vector<short> TilemapLayer::GetMapData() const {
	return map_data;
}
void TilemapLayer::SetMapData(std::vector<short> nmap_data) {
	if (map_data != nmap_data) {
		data_cache.resize(width);
		for (int x = 0; x < width; x++) {
			data_cache[x].resize(height);
			for (int y = 0; y < height; y++) {
				TileData tile;
				tile.id = nmap_data[x + y * width];
				tile.z = 0;
				if (tile.id >= 5000 && tile.id <= 5143) {
					if (passable[18 + tile.id - 5000] & (1 << 4)) {
						tile.z += 16;
						if (layer == 1) tile.z += 16;
					}
				} else if (tile.id >= 10000 && tile.id <= 10143) {
					if (passable[18 + tile.id - 10000] & (1 << 4)) {
						tile.z += 16;
						if (layer == 1) tile.z += 16;
					}
				}
				data_cache[x][y] = tile;
			}
		}
	}
	map_data = nmap_data;
}
std::vector<unsigned char> TilemapLayer::GetPassable() const {
	return passable;
}
void TilemapLayer::SetPassable(std::vector<unsigned char> npassable) {
	passable = npassable;
}
bool TilemapLayer::GetVisible() const {
	return visible;
}
void TilemapLayer::SetVisible(bool nvisible) {
	if (visible != nvisible) {
		for (int i = 0; i < sprites.size(); i++) {
			for (int j = 0; j < sprites[i].size(); j++) {
				sprites[i][j]->SetVisible(nvisible);
			}
		}
	}
	visible = nvisible;
}
int TilemapLayer::GetOx() const {
	return ox;
}
void TilemapLayer::SetOx(int nox) {
	ox = nox;
}
int TilemapLayer::GetOy() const {
	return oy;
}
void TilemapLayer::SetOy(int noy) {
	oy = noy;
}
int TilemapLayer::GetWidth() const {
	return width;
}
void TilemapLayer::SetWidth(int nwidth) {
	width = nwidth;
}
int TilemapLayer::GetHeight() const {
	return height;
}
void TilemapLayer::SetHeight(int nheight) {
	height = nheight;
}

////////////////////////////////////////////////////////////
/// Get z
////////////////////////////////////////////////////////////
int TilemapLayer::GetZ() const {
	return -1;
}

////////////////////////////////////////////////////////////
/// Get id
////////////////////////////////////////////////////////////
unsigned long TilemapLayer::GetId() const {
	return ID;
}
