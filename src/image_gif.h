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

#pragma once
#include <vector>
#include <cstdint>
#include "bitmap.h"
#include "nsgif.h" 

struct GifFrame {
	BitmapRef bitmap;
	int delay_cs; // Delay in centiseconds
};

class GifDecoder {
public:
	GifDecoder(const uint8_t* data, size_t len);
	~GifDecoder();

	bool IsValid() const { return _valid; }
	int GetWidth() const { return _width; }
	int GetHeight() const { return _height; }
	const std::vector<GifFrame>& GetFrames() const { return _frames; }

private:
	static nsgif_bitmap_t* cb_bitmap_create(int width, int height);
	static void cb_bitmap_destroy(nsgif_bitmap_t* bitmap);
	static uint8_t* cb_bitmap_get_buffer(nsgif_bitmap_t* bitmap);

	bool _valid = false;
	int _width = 0;
	int _height = 0;
	std::vector<GifFrame> _frames;
	nsgif_t* _gif = nullptr;
};
