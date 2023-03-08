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

#ifndef EP_BACKGROUND_H
#define EP_BACKGROUND_H

// Headers
#include <string>
#include "system.h"
#include "drawable.h"
#include "async_handler.h"
#include "tone.h"

class Background : public Drawable {
public:
	Background(const std::string& name);
	Background(int terrain_id);

	void Draw(Bitmap& dst) override;
	void Update();
	Tone GetTone() const;
	void SetTone(Tone tone);

private:
	static void Update(int& rate, int& value);
	static int Scale(int x);

	void OnBackgroundGraphicReady(FileRequestResult* result);
	void OnForegroundFrameGraphicReady(FileRequestResult* result);

	Tone tone_effect;
	Tone current_tone;
	BitmapRef bg_bitmap;
	int bg_hscroll = 0;
	int bg_vscroll = 0;
	int bg_x = 0;
	int bg_y = 0;
	BitmapRef fg_bitmap;
	int fg_hscroll = 0;
	int fg_vscroll = 0;
	int fg_x = 0;
	int fg_y = 0;

	FileRequestBinding fg_request_id;
	FileRequestBinding bg_request_id;
};

inline Tone Background::GetTone() const {
	return tone_effect;
}

inline void Background::SetTone(Tone tone) {
	tone_effect = tone;
}

#endif
