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
#include <lcf/rpg/animation.h>
#include "output.h"
#include "game_battle.h"
#include "game_system.h"
#include "game_screen.h"
#include "game_map.h"
#include "main_data.h"
#include "filefinder.h"
#include "cache.h"
#include "battle_animation.h"
#include "baseui.h"
#include "spriteset_battle.h"
#include "player.h"
#include "options.h"
#include "drawable_mgr.h"

BattleAnimation::BattleAnimation(const lcf::rpg::Animation& anim, bool only_sound, int cutoff) :
	animation(anim), only_sound(only_sound)
{
	num_frames = GetRealFrames() * 2;
	if (cutoff >= 0 && cutoff < num_frames) {
		num_frames = cutoff;
	}

	SetZ(Priority_BattleAnimation);

	StringView name = animation.animation_name;
	BitmapRef graphic;

	if (name.empty()) return;

	if (animation.large) {
		FileRequestAsync* request = AsyncHandler::RequestFile("Battle2", name);
		request->SetGraphicFile(true);
		request_id = request->Bind(&BattleAnimation::OnBattle2SpriteReady, this);
		request->Start();
	} else {
		FileRequestAsync* request = AsyncHandler::RequestFile("Battle", name);
		request->SetGraphicFile(true);
		request_id = request->Bind(&BattleAnimation::OnBattleSpriteReady, this);
		request->Start();
	}
}

void BattleAnimation::Update() {
	if (!IsDone() && (frame & 1) == 0) {
		// Lookup any timed SFX (SE/flash/shake) data for this frame
		for (auto& timing: animation.timings) {
			if (timing.frame == GetRealFrame() + 1) {
				ProcessAnimationTiming(timing);
			}
		}
	}

	UpdateScreenFlash();
	UpdateTargetFlash();

	SetFlashEffect(Main_Data::game_screen->GetFlashColor());

	frame++;
}

void BattleAnimation::OnBattleSpriteReady(FileRequestResult* result) {
	BitmapRef bitmap = Cache::Battle(result->file);
	SetBitmap(bitmap);
	SetSrcRect(Rect(0, 0, 0, 0));
}

void BattleAnimation::OnBattle2SpriteReady(FileRequestResult* result) {
	BitmapRef bitmap = Cache::Battle2(result->file);
	SetBitmap(bitmap);
	SetSrcRect(Rect(0, 0, 0, 0));
}

void BattleAnimation::DrawAt(Bitmap& dst, int x, int y) {
	if (IsDone()) {
		return;
	}

	const lcf::rpg::AnimationFrame& anim_frame = animation.frames[GetRealFrame()];

	std::vector<lcf::rpg::AnimationCellData>::const_iterator it;
	for (it = anim_frame.cells.begin(); it != anim_frame.cells.end(); ++it) {
		const lcf::rpg::AnimationCellData& cell = *it;

		if (!cell.valid) {
			// Skip unused cells (they are created by deleting cells in the
			// animation editor, resulting in gaps)
			continue;
		}

		SetX(invert ? x - cell.x : cell.x + x);
		SetY(cell.y + y);
		int sx = cell.cell_id % 5;
		if (invert) sx = 4 - sx;
		int sy = cell.cell_id / 5;
		int size = animation.large ? 128 : 96;
		SetSrcRect(Rect(sx * size, sy * size, size, size));
		SetOx(size / 2);
		SetOy(size / 2);
		SetTone(Tone(cell.tone_red * 128 / 100,
			cell.tone_green * 128 / 100,
			cell.tone_blue * 128 / 100,
			cell.tone_gray * 128 / 100));
		SetOpacity(255 * (100 - cell.transparency) / 100);
		SetZoomX(cell.zoom / 100.0);
		SetZoomY(cell.zoom / 100.0);
		SetFlipX(invert);
		Sprite::Draw(dst);
	}

	if (anim_frame.cells.empty()) {
		// Draw an empty sprite when no cell is available in the animation
		SetSrcRect(Rect(0, 0, 0, 0));
		Sprite::Draw(dst);
	}
}

void BattleAnimation::ProcessAnimationFlash(const lcf::rpg::AnimationTiming& timing) {
	if (IsOnlySound()) {
		return;
	}

	if (timing.flash_scope == lcf::rpg::AnimationTiming::FlashScope_target) {
		target_flash_timing = &timing - animation.timings.data();
	} else if (timing.flash_scope == lcf::rpg::AnimationTiming::FlashScope_screen) {
		screen_flash_timing = &timing - animation.timings.data();
	}
}

void BattleAnimation::ProcessAnimationTiming(const lcf::rpg::AnimationTiming& timing) {
	// Play the SE.
	Main_Data::game_system->SePlay(timing.se);
	if (IsOnlySound()) {
		return;
	}

	// Flash.
	ProcessAnimationFlash(timing);

	// Shake (only happens in battle).
	if (Game_Battle::IsBattleRunning()) {
		switch (timing.screen_shake) {
		case lcf::rpg::AnimationTiming::ScreenShake_nothing:
			break;
		case lcf::rpg::AnimationTiming::ScreenShake_target:
			// FIXME: Estimate, see below for screen shake.
			ShakeTargets(3, 5, 32);
			break;
		case lcf::rpg::AnimationTiming::ScreenShake_screen:
			Game_Screen* screen = Main_Data::game_screen.get();
			// FIXME: This is not proven accurate. Screen captures show that
			// the shake effect lasts for 16 animation frames (32 real frames).
			// The maximum offset observed was 6 or 7, which makes these numbers
			// seem reasonable.
			screen->ShakeOnce(3, 5, 32);
			break;
		}
	}
}

static int CalculateFlashPower(int frames, int power) {
	// This algorithm was determined numerically by measuring the flash
	// power for each frame of battle animation flashs.
	int f = 7 - ((frames + 1) / 2);
	return std::min(f * power / 6, 31);
}

void BattleAnimation::UpdateFlashGeneric(int timing_idx, int& r, int& g, int& b, int& p) {
	r = 0; g = 0; b = 0; p = 0;

	if (timing_idx >= 0) {
		auto& timing = animation.timings[timing_idx];
		int start_frame = (timing.frame - 1) * 2;
		int delta_frames = GetFrame() - start_frame;
		if (delta_frames <= 10) {
			r = timing.flash_red;
			g = timing.flash_green;
			b = timing.flash_blue;
			p = CalculateFlashPower(delta_frames, timing.flash_power);
		}
	}
}

void BattleAnimation::UpdateScreenFlash() {
	int r, g, b, p;
	UpdateFlashGeneric(screen_flash_timing, r, g, b, p);
	Main_Data::game_screen->FlashOnce(r, g, b, p, 0);
}

void BattleAnimation::UpdateTargetFlash() {
	int r, g, b, p;
	UpdateFlashGeneric(target_flash_timing, r, g, b, p);
	FlashTargets(r, g, b, p);
}

// For handling the vertical position.
// (The first argument should be an lcf::rpg::Animation::Position,
// but the position member is an int, so take an int.)
static int CalculateOffset(int pos, int target_height) {
	switch (pos) {
	case lcf::rpg::Animation::Position_down:
		return target_height / 2;
	case lcf::rpg::Animation::Position_up:
		return -(target_height / 2);
	default:
		return 0;
	}
}

/////////

BattleAnimationMap::BattleAnimationMap(const lcf::rpg::Animation& anim, Game_Character& target, bool global) :
	BattleAnimation(anim), target(target), global(global)
{
}

void BattleAnimationMap::Draw(Bitmap& dst) {
	if (IsOnlySound()) {
		return;
	}

	if (global) {
		DrawGlobal(dst);
	} else {
		DrawSingle(dst);
	}
}

void BattleAnimationMap::DrawGlobal(Bitmap& dst) {
	auto rect = Main_Data::game_screen->GetScreenEffectsRect();

	for (int y = -1; y < 2; ++y) {
		for (int x = -1; x < 2; ++x) {
			DrawAt(dst, rect.width * x + rect.x, rect.height * y + rect.y);
		}
	}
}

void BattleAnimationMap::DrawSingle(Bitmap& dst) {
	//If animation is targeted on the screen
	if (animation.scope == lcf::rpg::Animation::Scope_screen) {
		DrawAt(dst, Player::screen_width / 2, Player::screen_height / 2);
		return;
	}
	const int character_height = 24;
	int vertical_center = target.GetScreenY(false, false) - character_height / 2;
	int offset = CalculateOffset(animation.position, character_height);
	DrawAt(dst, target.GetScreenX(), vertical_center + offset);
}

void BattleAnimationMap::FlashTargets(int r, int g, int b, int p) {
	target.Flash(r, g, b, p, 0);
}

void BattleAnimationMap::ShakeTargets(int /* str */, int /* spd */, int /* time */) {
}

/////////

BattleAnimationBattle::BattleAnimationBattle(const lcf::rpg::Animation& anim, std::vector<Game_Battler*> battlers, bool only_sound, int cutoff_frame, bool set_invert) :
	BattleAnimation(anim, only_sound, cutoff_frame), battlers(std::move(battlers))
{
	invert = set_invert;
}

void BattleAnimationBattle::Draw(Bitmap& dst) {
	if (IsOnlySound())
		return;
	if (animation.scope == lcf::rpg::Animation::Scope_screen) {
		DrawAt(dst, Player::menu_offset_x + (Player::screen_width / 2), Player::menu_offset_y + (Player::screen_height / 3));
		return;
	}

	for (auto* battler: battlers) {
		const Sprite_Battler* sprite = battler->GetBattleSprite();
		int offset = 0;
		if (sprite) {
			if (sprite->GetBitmap()) {
				offset = CalculateOffset(animation.position, sprite->GetHeight());
			} else {
				offset = CalculateOffset(animation.position, GetAnimationCellHeight() / 2);
			}
		}
		DrawAt(dst, Player::menu_offset_x + battler->GetBattlePosition().x, Player::menu_offset_y + battler->GetBattlePosition().y + offset);
	}
}
void BattleAnimationBattle::FlashTargets(int r, int g, int b, int p) {
	for (auto& battler: battlers) {
		battler->Flash(r, g, b, p, 0);
	}
}

void BattleAnimationBattle::ShakeTargets(int str, int spd, int time) {
	for (auto& battler: battlers) {
		battler->ShakeOnce(str, spd, time);
	}
}

BattleAnimationBattler::BattleAnimationBattler(const lcf::rpg::Animation& anim, std::vector<Game_Battler*> battlers, bool only_sound, int cutoff_frame, bool set_invert) :
	BattleAnimation(anim, only_sound, cutoff_frame), battlers(std::move(battlers))
{
	invert = set_invert;
}

void BattleAnimationBattler::Draw(Bitmap& dst) {
	if (IsOnlySound())
		return;
	if (animation.scope == lcf::rpg::Animation::Scope_screen) {
		DrawAt(dst, Player::menu_offset_x + Player::screen_width / 2, Player::menu_offset_y + Player::screen_height / 3);
		return;
	}

	for (auto* battler: battlers) {
		SetFlashEffect(battler->GetFlashColor());
		// Game_Battler::GetDisplayX() and Game_Battler::GetDisplayX() already add MENU_OFFSET
		DrawAt(dst, battler->GetDisplayX(), battler->GetDisplayY());
	}
}

void BattleAnimationBattler::FlashTargets(int r, int g, int b, int p) {
	for (auto& battler: battlers) {
		battler->Flash(r, g, b, p, 0);
	}
}

void BattleAnimationBattler::ShakeTargets(int str, int spd, int time) {
	for (auto& battler: battlers) {
		battler->ShakeOnce(str, spd, time);
	}
}

void BattleAnimationBattler::ProcessAnimationTiming(const lcf::rpg::AnimationTiming& timing) {
	// Play the SE.
	Main_Data::game_system->SePlay(timing.se);
	if (IsOnlySound()) {
		return;
	}

	// Flash.
	ProcessAnimationFlash(timing);
}

void BattleAnimationBattler::ProcessAnimationFlash(const lcf::rpg::AnimationTiming& timing) {
	if (IsOnlySound()) {
		return;
	}

	if (timing.flash_scope == lcf::rpg::AnimationTiming::FlashScope_screen) {
		target_flash_timing = &timing - animation.timings.data();
	}
}

void BattleAnimationBattler::UpdateScreenFlash() {
	int r, g, b, p;
	UpdateFlashGeneric(screen_flash_timing, r, g, b, p);
	if (r > 0 || g > 0 || b > 0 || p > 0) {
		Main_Data::game_screen->FlashOnce(r, g, b, p, 0);
	}
}

void BattleAnimationBattler::UpdateTargetFlash() {
	int r, g, b, p;
	UpdateFlashGeneric(target_flash_timing, r, g, b, p);
	if (r > 0 || g > 0 || b > 0 || p > 0) {
		FlashTargets(r, g, b, p);
	}
}

void BattleAnimation::SetFrame(int frame) {
	// Reset pending flash.
	int real_frame = frame / 2;
	screen_flash_timing = -1;
	target_flash_timing = -1;
	for (auto& timing: animation.timings) {
		if (timing.frame > real_frame + 1) {
			break;
		}
		ProcessAnimationFlash(timing);
	}

	this->frame = frame;
}

void BattleAnimation::SetInvert(bool inverted) {
	invert = inverted;
}
