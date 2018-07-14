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
#include "graphics.h"
#include "sprite_clone.h"
#include "output.h"

// Constructor
SpriteClone::SpriteClone(Sprite* nreference, int nx, int ny, int nz) : reference(nreference), type(TypeClone), x(nx), y(ny), z(nz) {
	if (reference == nullptr)
		Output::Warning("Drawable reference null, it won't draw anything.");
}

// Destructor
SpriteClone::~SpriteClone() {
	Graphics::RemoveDrawable(this);
}

// Draw
void SpriteClone::Draw() {
	if (reference == nullptr)
		return;
	int old_x = reference->GetX();
	reference->SetX(x);
	int old_y = reference->GetY();
	reference->SetY(y);;

	reference->Draw();
	reference->SetX(old_x);
	reference->SetY(old_y);
}

int SpriteClone::GetX() const {
	return x;
}

int SpriteClone::GetY() const {
	return y;
}

int SpriteClone::GetZ() const {
	return z;
}

DrawableType SpriteClone::GetType() const {
	return type;
}
