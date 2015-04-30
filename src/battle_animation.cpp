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

#include "bitmap.h"
#include "rpg_animation.h"
#include "output.h"
#include "graphics.h"
#include "filefinder.h"
#include "cache.h"
#include "battle_animation.h"
#include "baseui.h"

BattleAnimation::BattleAnimation(int x, int y, const RPG::Animation* animation) :
	x(x), y(y), animation(animation), frame(0)
{
	const std::string& name = animation->animation_name;
	BitmapRef graphic;

	if (name.empty()) return;

	if (!FileFinder::FindImage("Battle", name).empty()) {
		large = false;
		graphic = Cache::Battle(name); // TODO
	}
	else if (!FileFinder::FindImage("Battle2", name).empty()) {
		graphic = Cache::Battle2(name); // TODO
	}
	else {
		Output::Warning("Couldn't find animation: %s", name.c_str());
		return;
	}

	screen = graphic;

	Graphics::RegisterDrawable(this);
}

BattleAnimation::~BattleAnimation() {
	Graphics::RemoveDrawable(this);
}

int BattleAnimation::GetZ() const {
	return 1500;
}

DrawableType BattleAnimation::GetType() const {
	return TypeDefault;
}

void BattleAnimation::Draw() {
	if (!screen) {
		// Initialization failed
		return;
	}

	if (frame >= (int) animation->frames.size())
		return;

	const RPG::AnimationFrame& anim_frame = animation->frames[frame];

	std::vector<RPG::AnimationCellData>::const_iterator it;
	for (it = anim_frame.cells.begin(); it != anim_frame.cells.end(); ++it) {
		const RPG::AnimationCellData& cell = *it;
		int sx = cell.cell_id % 5;
		int sy = cell.cell_id / 5;
		int size = large ? 128 : 96;
		Rect src_rect(sx * size, sy * size, size, size);
		Tone tone(cell.tone_red, cell.tone_green, cell.tone_blue, cell.tone_gray);
		int opacity = 255 * (100 - cell.transparency) / 100;
		double zoom = cell.zoom / 100.0;
		DisplayUi->GetDisplaySurface()->EffectsBlit(
			x + cell.x, y + cell.y,
			size / 2, size / 2,
			*screen, src_rect, 
			opacity, tone,
			zoom, zoom);
	}
}

void BattleAnimation::Update() {
	static bool update = true;
	if (update) {
		frame++;
	}
	update = !update;
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

bool BattleAnimation::IsDone() const {
	return GetFrame() >= GetFrames();
}
