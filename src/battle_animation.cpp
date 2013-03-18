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
#include "rpg_animation.h"
#include "output.h"
#include "graphics.h"
#include "filefinder.h"
#include "cache.h"
#include "battle_animation.h"
#include "bitmap_screen.h"

BattleAnimation::BattleAnimation(int x, int y, const RPG::Animation* animation) :
	x(x), y(y), animation(animation), frame(0), initialized(false), visible(false),
	ID(Graphics::drawable_id++) {
}

BattleAnimation::~BattleAnimation() {
	SetVisible(false);
}

void BattleAnimation::Setup() {
	if (initialized)
		return;

	const std::string& name = animation->animation_name;
	BitmapRef graphic;

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
		screen.reset();
		return;
	}

	screen = BitmapScreen::Create(graphic);

	initialized = true;
}

unsigned long BattleAnimation::GetId() const {
	return ID;
}

int BattleAnimation::GetZ() const {
	return 400;
}

DrawableType BattleAnimation::GetType() const {
	return TypeDefault;
}

void BattleAnimation::Draw(int /* z_order */) {
	if (frame >= (int) animation->frames.size())
		return;

	Setup();

	const RPG::AnimationFrame& anim_frame = animation->frames[frame];

	std::vector<RPG::AnimationCellData>::const_iterator it;
	for (it = anim_frame.cells.begin(); it != anim_frame.cells.end(); it++) {
		const RPG::AnimationCellData& cell = *it;
		int sx = cell.cell_id % 5;
		int sy = cell.cell_id / 5;
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

void BattleAnimation::Update() {
	frame++;
}

void BattleAnimation::SetFrame(int _frame) {
	frame = _frame;
}

int BattleAnimation::GetFrame() const {
	return frame;
}

int BattleAnimation::GetFrames() const {
	return animation->frames.size();
}

void BattleAnimation::SetVisible(bool _visible) {
	if (visible == _visible)
		return;

	visible = _visible;

	if (visible) {
		zobj = Graphics::RegisterZObj(GetZ(), ID);
		Graphics::RegisterDrawable(ID, this);
	}
	else {
		Graphics::RemoveZObj(ID);
		Graphics::RemoveDrawable(ID);
	}
}

bool BattleAnimation::GetVisible() {
	return visible;
}

bool BattleAnimation::IsDone() const {
	return GetFrame() >= GetFrames();
}
