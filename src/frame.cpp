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
#include "cache.h"
#include "bitmap.h"
#include "main_data.h"
#include "frame.h"
#include "drawable_mgr.h"

Frame::Frame() :
	Drawable(Priority_Frame)
{
	if (!lcf::Data::system.frame_name.empty() && lcf::Data::system.show_frame) {
		FileRequestAsync* request = AsyncHandler::RequestFile("Frame", lcf::Data::system.frame_name);
		request->SetGraphicFile(true);
		request_id = request->Bind(&Frame::OnFrameGraphicReady, this);
		request->Start();
	}

	DrawableMgr::Register(this);
}

void Frame::Update() {
	// no-op
}

void Frame::Draw(Bitmap& dst) {
	if (frame_bitmap) {
		dst.Blit(0, 0, *frame_bitmap, frame_bitmap->GetRect(), 255);
	}
}

void Frame::OnFrameGraphicReady(FileRequestResult* result) {
	frame_bitmap = Cache::Frame(result->file);
}
