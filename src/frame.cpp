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

// Headers
#include <string>
#include <vector>
#include "baseui.h"
#include "cache.h"
#include "bitmap.h"
#include "graphics.h"
#include "main_data.h"
#include "frame.h"

Frame::Frame() {
	if (!Data::system.frame_name.empty() && Data::system.show_frame) {
		FileRequestAsync* request = AsyncHandler::RequestFile("Frame", Data::system.frame_name);
		request_id = request->Bind(&Frame::OnFrameGraphicReady, this);
		request->Start();
	}

	Graphics::RegisterDrawable(this);
}

Frame::~Frame() {
	Graphics::RemoveDrawable(this);
}

int Frame::GetZ() const {
	return z;
}

DrawableType Frame::GetType() const {
	return type;
}

void Frame::Update() {
	// no-op
}

void Frame::Draw() {
	if (frame_bitmap) {
		BitmapRef dst = DisplayUi->GetDisplaySurface();
		dst->Blit(0, 0, *frame_bitmap, frame_bitmap->GetRect(), 255);
	}
}

void Frame::OnFrameGraphicReady(FileRequestResult* result) {
	frame_bitmap = Cache::Frame(result->file);
}
