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
#include "bitmap.h"
#include "game_actors.h"
#include "window_face.h"

Window_Face::Window_Face(int ix, int iy, int iwidth, int iheight) :
	Window_Base(ix, iy, iwidth, iheight), actor_id(1) {

	SetContents(Bitmap::Create(width - 16, height - 16));
}

void Window_Face::Refresh() {
	contents->Clear();
	DrawActorFace(Game_Actors::GetActor(actor_id), 0, 0);
}

void Window_Face::Set(int id) {
	actor_id = id;
	Refresh();
}
