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
#include "rpg_animation.h"
#include "output.h"
#include "graphics.h"
#include "filefinder.h"
#include "cache.h"
#include "battle_animation.h"

////////////////////////////////////////////////////////////
Battle::Animation::Animation(int x, int y, const RPG::Animation* animation) :
	x(x), y(y), animation(animation), frame(0), initialized(false) {}

void Battle::Animation::Setup() {

	if (initialized)
		return;

	ID = Graphics::drawable_id++;

	const std::string& name = animation->animation_name;
	Bitmap* graphic;

	if (!FileFinder::FindImage("Battle", name).empty()) {
		large = false;
		graphic = Cache::Battle(name);
	}
	else if (!FileFinder::FindImage("Battle2", name).empty()) {
		large = true;
		graphic = Cache::Battle2(name);
	}
	else {
		Output::Warning("Couldn't find animation: %s", name.c_str());
		screen = NULL;
		return;
	}

	zobj = Graphics::RegisterZObj(GetZ(), ID);
	Graphics::RegisterDrawable(ID, this);

	screen = BitmapScreen::CreateBitmapScreen(graphic);

	initialized = true;
}

Battle::Animation::~Animation() {
	if (!initialized)
		return;

	Graphics::RemoveZObj(ID);
	Graphics::RemoveDrawable(ID);
	if (screen != NULL)
		delete screen;
}

unsigned long Battle::Animation::GetId() const {
	return ID;
}

int Battle::Animation::GetZ() const {
	return 400;
}

DrawableType Battle::Animation::GetType() const {
	return TypeDefault;
}

void Battle::Animation::Draw(int z_order) {
	if (frame >= (int) animation->frames.size())
		return;

	const RPG::AnimationFrame& anim_frame = animation->frames[frame];

	std::vector<RPG::AnimationCellData>::const_iterator it;
	for (it = anim_frame.cells.begin(); it != anim_frame.cells.end(); it++) {
		const RPG::AnimationCellData& cell = *it;
		int sx = cell.ID % 5;
		int sy = cell.ID / 5;
		int size = large ? 128 : 96;
		int zoomed = size * cell.zoom / 100;
		screen->SetSrcRect(Rect(sx * size, sy * size, size, size));
		screen->SetZoomXEffect(cell.zoom / 100.0);
		screen->SetZoomYEffect(cell.zoom / 100.0);
		screen->SetToneEffect(Tone(cell.tone_red, cell.tone_green, cell.tone_blue, cell.tone_gray));
		screen->SetOpacityEffect(255 * (100 - cell.transparency) / 100);
		screen->BlitScreen(x + cell.x - zoomed / 2, y + cell.y - zoomed / 2);
	}
}

void Battle::Animation::Update(int _frame) {
	frame = _frame;
	Setup();
}

