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

#include "statustext_overlay.h"
#include "bitmap.h"
#include "font.h"
#include "drawable_mgr.h"
#include "player.h"

using namespace std::chrono_literals;

static constexpr auto delay = 1s;

StatusTextOverlay::StatusTextOverlay() :
	Drawable(Priority_Overlay + 100, Drawable::Flags::Global)
{
	DrawableMgr::Register(this);
}

void StatusTextOverlay::ShowText(const std::string& statustext) {
	text = statustext;
	last_update_time = Game_Clock::GetFrameTime();
	show = true;
	dirty = true;
}

void StatusTextOverlay::Update() {
	if (!show)
		return;
	auto now = Game_Clock::GetFrameTime();
	auto dt = now - last_update_time;
	if (dt < delay) {
		return;
	}
	last_update_time = now;
	show = false;
	dirty = true;
}

void StatusTextOverlay::Draw(Bitmap& dst) {
	if (dirty) {
		if (show) {
			Rect rect = Text::GetSize(*Font::DefaultBitmapFont(), text);
			if (!statustext_bitmap || statustext_bitmap->GetWidth() != rect.width + 1) {
				statustext_bitmap = Bitmap::Create(rect.width + 1, rect.height - 1, true);
			}

			statustext_bitmap->Clear();
			statustext_bitmap->Fill(Color(0, 0, 0, 128));
			Text::Draw(*statustext_bitmap, 1, 0, *Font::DefaultBitmapFont(), Color(255, 255, 255, 255), text);

			statustext_rect = Rect(0, 0, rect.width + 1, rect.height - 1);
		} else if (statustext_bitmap) {
			statustext_bitmap->Clear();
		}
		dirty = false;
	}
	if (show && statustext_bitmap) {
		dst.Blit(Player::screen_width / 2 - (statustext_bitmap->GetWidth() / 2),
			Player::screen_height * 0.725, *statustext_bitmap, statustext_rect, 255);
	}
}
