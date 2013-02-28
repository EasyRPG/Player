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
#include "message_overlay.h"
#include "player.h"
#include "graphics.h"
#include "bitmap.h"

////////////////////////////////////////////////////////////
MessageOverlay::MessageOverlay() :
	type(TypeMessageOverlay),
	ID(Graphics::drawable_id++),
	bitmap(NULL),
	z(100000),
	ox(0),
	oy(0),
	text_height(12),
	message_max(10),
	dirty(false),
	counter(0) {
	
	bitmap_screen = BitmapScreen::Create();
	black = Bitmap::Create(DisplayUi->GetWidth(), text_height, Color());

	bitmap = Bitmap::Create(DisplayUi->GetWidth(), text_height * message_max, true);
	bitmap_screen->SetBitmap(bitmap);
	//zobj = Graphics::RegisterZObj(100000, ID);
	Graphics::RegisterDrawable(ID, this);
}

////////////////////////////////////////////////////////////
MessageOverlay::~MessageOverlay() {
	Graphics::RemoveZObj(ID);
	Graphics::RemoveDrawable(ID);
}

bool MessageOverlay::IsGlobal() const { return true; }

////////////////////////////////////////////////////////////
void MessageOverlay::Draw(int z_order) {
	++counter;
	if (counter > 120) {
		counter = 0;
		if (!messages.empty()) {
			messages.pop_front();
			dirty = true;
		}
	} else {
		if (!messages.empty()) {
			bitmap_screen->BlitScreen(ox, oy);
		}
	}

	if (!dirty) return;

	bitmap->Clear();
	for (size_t i = 0; i < messages.size(); ++i) {
		bitmap->Blit(0, i * text_height, black, black->GetRect(), 128);
		bitmap->TextDraw(2, i * text_height, bitmap->GetWidth(), text_height,
			Font::ColorDefault, messages[i]);
	}

	bitmap_screen->BlitScreen(ox, oy);

	dirty = false;
}

int MessageOverlay::GetZ() const {
	return z;
}

DrawableType MessageOverlay::GetType() const {
	return type;
}

unsigned long MessageOverlay::GetId() const {
	return ID;
}

////////////////////////////////////////////////////////////
void MessageOverlay::AddMessage(const std::string& message) {
	messages.push_back(message);
	if (messages.size() > 5) {
		messages.pop_front();
	}
	dirty = true;
}