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
#include "window_actorsp.h"
#include "bitmap.h"
#include "font.h"

Window_ActorSp::Window_ActorSp(Scene* parent, int ix, int iy, int iwidth, int iheight) :
	Window_Base(parent, ix, iy, iwidth, iheight) {

	SetContents(Bitmap::Create(width - 16, height - 16));

	contents->Clear();
}

void Window_ActorSp::SetBattler(const Game_Battler& battler) {
	int cx = 0;

	int digits = (battler.MaxSpValue() >= 1000) ? 4 : 3;

	int color = Font::ColorDefault;
	if (battler.GetMaxSp() != 0 && battler.GetSp() <= battler.GetMaxSp() / 4) {
		color = Font::ColorCritical;
	}

	contents->Clear();

	// Draw current Sp
	contents->TextDraw(cx + digits * 6, 2, color, std::to_string(battler.GetSp()), Text::AlignRight);

	// Draw /
	cx += digits * 6;
	contents->TextDraw(cx, 2, Font::ColorDefault, "/");

	// Draw Max Sp
	cx += 6;
	contents->TextDraw(cx + digits * 6, 2, Font::ColorDefault, std::to_string(battler.GetMaxSp()), Text::AlignRight);
}
