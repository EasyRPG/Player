/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EP_TILEMAP_H
#define EP_TILEMAP_H

// Headers
#include <vector>
#include "system.h"
#include "tilemap_layer.h"
#include "tone.h"

/**
 * Tilemap class
 */
class Tilemap {
public:
	Tilemap();

	BitmapRef const& GetChipset() const;
	void SetChipset(BitmapRef const& nchipset);

	const std::vector<short>& GetMapDataDown() const;
	void SetMapDataDown(std::vector<short> down);

	const std::vector<short>& GetMapDataUp() const;
	void SetMapDataUp(std::vector<short> up);

	const std::vector<unsigned char>& GetPassableUp() const;
	void SetPassableUp(std::vector<unsigned char> up);

	const std::vector<unsigned char>& GetPassableDown() const;
	void SetPassableDown(std::vector<unsigned char> down);

	bool IsVisible() const;
	void SetVisible(bool nvisible);
	int GetOx() const;
	void SetOx(int nox);
	int GetOy() const;
	void SetOy(int noy);
	int GetWidth() const;
	void SetWidth(int nwidth);
	int GetHeight() const;
	void SetHeight(int nheight);
	int GetAnimationSpeed() const;
	void SetAnimationSpeed(int speed);
	int GetAnimationType() const;
	void SetAnimationType(int type);
	void OnSubstituteDown();
	void OnSubstituteUp();
	void SetFastBlitDown(bool fast);
	void SetTone(Tone tone);

private:
	TilemapLayer layer_down, layer_up;
};

inline const std::vector<short>& Tilemap::GetMapDataDown() const {
	return layer_down.GetMapData();
}

inline void Tilemap::SetMapDataDown(std::vector<short> down) {
	layer_down.SetMapData(std::move(down));
}

inline const std::vector<short>& Tilemap::GetMapDataUp() const {
	return layer_up.GetMapData();
}

inline void Tilemap::SetMapDataUp(std::vector<short> up) {
	layer_up.SetMapData(std::move(up));
}

inline const std::vector<unsigned char>& Tilemap::GetPassableDown() const {
	return layer_down.GetPassable();
}

inline void Tilemap::SetPassableDown(std::vector<unsigned char> down) {
	layer_down.SetPassable(std::move(down));
}

inline const std::vector<unsigned char>& Tilemap::GetPassableUp() const {
	return layer_up.GetPassable();
}

inline void Tilemap::SetPassableUp(std::vector<unsigned char> up) {
	layer_up.SetPassable(std::move(up));
}

#endif
