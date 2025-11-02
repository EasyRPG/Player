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
#include <algorithm>
#include <vector>
#include <array>
#include <cmath>

#include "player.h"
#include "transition.h"
#include "async_handler.h"
#include "bitmap.h"
#include "game_player.h"
#include "graphics.h"
#include "main_data.h"
#include "scene.h"
#include "scene_map.h"
#include "spriteset_map.h"
#include "baseui.h"
#include "drawable.h"
#include "drawable_mgr.h"
#include "output.h"
#include "rand.h"

int Transition::GetDefaultFrames(Transition::Type type)
{
	switch (type) {
		case TransitionFadeIn:
		case TransitionFadeOut:
			return 35;
		case TransitionCutIn:
		case TransitionCutOut:
			return 1;
		case TransitionNone:
			return 0;
		default:
			return 41;
	}
	return 0;
}

Transition::Transition() : Drawable(Priority_Transition, Drawable::Flags::Global)
{
	DrawableMgr::Register(this);
}

void Transition::PrependFlashes(int r, int g, int b, int p, int duration, int iterations) {
	flash.red = r;
	flash.green = g;
	flash.blue = b;
	flash.current_level = 0.0;
	flash.time_left = 0;

	flash_power = p;
	flash_duration = std::max(1, duration);
	flash_iterations = std::max(1, iterations);
}

void Transition::Init(Type type, Scene *linked_scene, int duration, bool next_erase) {
	// Triggering multiple transitions on a single frame is a bug.
	assert(!IsActive());

	if (duration < 0) {
		duration = GetDefaultFrames(type);
	}
	transition_type = type;
	scene = linked_scene;

	// Don't skip Frame 0 (Update is called before Draw).
	current_frame = -1;
	flash = {};
	flash_power = 0;
	flash_iterations = 0;
	flash_duration = 0;
	total_frames = 0;
	from_erase = to_erase;

	SetVisible(false);

	// Erase transitions are skipped entirely if screen already erased.
	if (type != TransitionNone && from_erase && next_erase) {
		transition_type = TransitionNone;
		return;
	}

	screen1.reset();
	screen2.reset();

	// Show Screen, the current frame is captured immediately
	if (!next_erase) {
		screen1 = DisplayUi->CaptureScreen();
	}

	// Total frames and erased have to be set *after* the above drawing code.
	// Otherwise IsActive() / IsErasedNotActive() will mess up drawing.
	total_frames = duration;

	// TransitionNone is neither a Show or Erase, it just waits and does nothing.
	// Screen state is not changed.
	if (type == TransitionNone) {
		return;
	}

	to_erase = next_erase;

	SetAttributesTransitions();
}

void Transition::SetAttributesTransitions() {
	int w, h, beg_i, mid_i, end_i, length;

	zoom_position = std::vector<int>(2);
	random_blocks = std::vector<uint32_t>(Player::screen_width * Player::screen_height / (size_random_blocks * size_random_blocks));
	mosaic_random_offset.resize(total_frames);

	for (uint32_t i = 0; i < random_blocks.size(); i++) {
		random_blocks[i] = i;
	}

	switch (transition_type) {
	case TransitionRandomBlocks:
		random_block_transition = Bitmap::Create(Player::screen_width, Player::screen_height, true);
		current_blocks_print = 0;
		std::shuffle(random_blocks.begin(), random_blocks.end(), Rand::GetRNG());
		break;
	case TransitionRandomBlocksDown:
	case TransitionRandomBlocksUp:
		random_block_transition = Bitmap::Create(Player::screen_width, Player::screen_height, true);
		current_blocks_print = 0;
		if (transition_type == TransitionRandomBlocksUp) { std::reverse(random_blocks.begin(), random_blocks.end()); }

		w = Player::screen_width / 4;
		h = Player::screen_height / 4;
		length = 10;
		for (int i = 0; i < h - 1; i++) {
			end_i = (i < length ? 2 * i + 1 : i <= h - length ? i + length : (i + h) / 2) * w;
			std::shuffle(random_blocks.begin() + i * w, random_blocks.begin() + end_i, Rand::GetRNG());

			beg_i = i * w + (i % 2 == 0 ? 0 : 2);
			mid_i = i * w + (i % 2 == 0 ? 1 : 3) + (i > h * 2 / 3 ? 3 : 0);
			if (transition_type == TransitionRandomBlocksDown) {
				std::partial_sort(random_blocks.begin() + beg_i, random_blocks.begin() + mid_i, random_blocks.begin() + end_i);
			}
			else { std::partial_sort(random_blocks.begin() + beg_i, random_blocks.begin() + mid_i, random_blocks.begin() + end_i, std::greater<uint32_t>()); }
		}
		break;
	case TransitionZoomIn:
	case TransitionZoomOut:
		if (scene != nullptr && scene->type == Scene::Map) {
			auto map = static_cast<Scene_Map*>(scene);

			zoom_position[0] = std::max(0, std::min(Main_Data::game_player->GetScreenX() + map->spriteset->GetRenderOx(), (int)Player::screen_width));
			zoom_position[1] = std::max(0, std::min(Main_Data::game_player->GetScreenY() - 8 + map->spriteset->GetRenderOy(), (int)Player::screen_height));
		}
		else {
			zoom_position[0] = Player::screen_width / 2;
			zoom_position[1] = Player::screen_height / 2;
		}
		break;
	case TransitionMosaicIn:
	case TransitionMosaicOut:
		for (int i = 0; i < total_frames; ++i) {
			// by default i 0..40 for scale 1..41
			mosaic_random_offset[i] = Rand::GetRandomNumber(0, i);
		}
		break;
	default:
		// do nothing, keep the compiler happy
		break;
	}
}

void Transition::Draw(Bitmap& dst) {
	if (!IsActive())
		return;

	std::vector<int> z_pos(2), z_size(2), z_length(2);
	int z_min, z_max, z_frame, z_fixed_pos, z_fixed_size;
	uint8_t m_r, m_g, m_b, m_a;
	uint32_t *m_pointer, blocks_to_print;
	int m_size;

	BitmapRef screen_pointer1, screen_pointer2;
	int w = dst.GetWidth();
	int h = dst.GetHeight();

	if (flash_iterations > 0) {
		auto color = Flash::MakeColor(flash.red, flash.green, flash.blue, flash.current_level);
		dst.BlendBlit(0, 0, *screen1, screen1->GetRect(), color, 255);
		return;
	}
	
	int tf_off = total_frames - 1;

	switch (transition_type) {
	case TransitionFadeIn:
	case TransitionFadeOut:
		dst.Blit(0, 0, *screen1, screen1->GetRect(), 255);
		dst.Blit(0, 0, *screen2, screen2->GetRect(), 255 * (current_frame + 1) / (total_frames - 2);
		break;
	case TransitionRandomBlocks:
	case TransitionRandomBlocksDown:
	case TransitionRandomBlocksUp:
		blocks_to_print = random_blocks.size() * (current_frame + 1) / tf_off;

		for (uint32_t i = current_blocks_print; i < blocks_to_print; i++) {
			random_block_transition->Blit(random_blocks[i] % (w / size_random_blocks) * size_random_blocks,
				random_blocks[i] / (w / size_random_blocks) * size_random_blocks, *screen2,
				Rect(random_blocks[i] % (w / size_random_blocks) * size_random_blocks, random_blocks[i] / (w / size_random_blocks) * size_random_blocks,
					size_random_blocks, size_random_blocks), Opacity::Opaque());
		}
		dst.Blit(0, 0, *screen1, screen1->GetRect(), Opacity::Opaque());
		dst.Blit(0, 0, *random_block_transition, random_block_transition->GetRect(), Opacity::Opaque());
		current_blocks_print = blocks_to_print;
		break;
	case TransitionBlindOpen:
		for (int i = 0; i < h / 8; i++) {
			dst.Blit(0, i * 8, *screen1, Rect(0, i * 8, w, 8 - (current_frame + 5) / 5), 255);
			dst.Blit(0, i * 8 + 8 - (current_frame + 5) / 5, *screen2, Rect(0, i * 8 + 8 - (current_frame + 5) / 5, w, (current_frame + 5) / 5), 255);
		}
		break;
	case TransitionBlindClose:
		for (int i = 0; i < h / 8; i++) {
			dst.Blit(0, i * 8 + (current_frame + 5) / 5, *screen1, Rect(0, i * 8 + (current_frame + 5) / 5, w, 8 - (current_frame + 5) / 5), 255);
			dst.Blit(0, i * 8, *screen2, Rect(0, i * 8, w, (current_frame + 5) / 5), 255);
		}
		break;
	case TransitionVerticalStripesIn:
	case TransitionVerticalStripesOut:
		for (int i = 0; i < tf_off - (current_frame + 1); i++) {
			dst.Blit(0, i * 6 + 3, *screen1, Rect(0, i * 6 + 3, w, 3), 255);
			dst.Blit(0, h - i * 6, *screen1, Rect(0, h - i * 6, w, 3), 255);
		}
		for (int i = 0; i < current_frame + 1; i++) {
			dst.Blit(0, i * 6, *screen2, Rect(0, i * 6, w, 3), 255);
			dst.Blit(0, h - 3 - i * 6, *screen2, Rect(0, h - 3 - i * 6, w, 3), 255);
		}
		break;
	case TransitionHorizontalStripesIn:
	case TransitionHorizontalStripesOut:
		for (int i = 0; i < tf_off - (current_frame + 1); i++) {
			dst.Blit(i * 8 + 4, 0, *screen1, Rect(i * 8 + 4, 0, 4, h), 255);
			dst.Blit(w - i * 8, 0, *screen1, Rect(w - i * 8, 0, 4, h), 255);
		}
		for (int i = 0; i < current_frame + 1; i++) {
			dst.Blit(i * 8, 0, *screen2, Rect(i * 8, 0, 4, h), 255);
			dst.Blit(w - 4 - i * 8, 0, *screen2, Rect(w - 4 - i * 8, 0, 4, h), 255);
		}
		break;
	case TransitionBorderToCenterIn:
	case TransitionBorderToCenterOut:
		dst.Blit(0, 0, *screen2, screen2->GetRect(), 255);
		dst.Blit((w / 2) * current_frame / tf_off, (h / 2) * current_frame / tf_off, *screen1, Rect((w / 2) * current_frame / tf_off, (h / 2) * current_frame / tf_off, w - w * current_frame / tf_off, h - h * current_frame / tf_off), 255);
		break;
	case TransitionCenterToBorderIn:
	case TransitionCenterToBorderOut:
		dst.Blit(0, 0, *screen1, screen1->GetRect(), 255);
		dst.Blit(w / 2 - (w / 2) * current_frame / tf_off, h / 2 - (h / 2) * current_frame / tf_off, *screen2, Rect(w / 2 - (w / 2) * current_frame / tf_off, h / 2 - (h / 2) * current_frame / tf_off, w * current_frame / tf_off, h * current_frame / tf_off), 255);
		break;
	case TransitionScrollUpIn:
	case TransitionScrollUpOut:
		dst.Blit(0, -h * current_frame / tf_off, *screen1, screen1->GetRect(), 255);
		dst.Blit(0, h - h * current_frame / tf_off, *screen2, screen2->GetRect(), 255);
		break;
	case TransitionScrollDownIn:
	case TransitionScrollDownOut:
		dst.Blit(0, h * current_frame / tf_off, *screen1, screen1->GetRect(), 255);
		dst.Blit(0, -h + h * current_frame / tf_off, *screen2, screen2->GetRect(), 255);
		break;
	case TransitionScrollLeftIn:
	case TransitionScrollLeftOut:
		dst.Blit(-w * current_frame / tf_off, 0, *screen1, screen1->GetRect(), 255);
		dst.Blit(w - w * current_frame / tf_off, 0, *screen2, screen2->GetRect(), 255);
		break;
	case TransitionScrollRightIn:
	case TransitionScrollRightOut:
		dst.Blit(w * current_frame / tf_off, 0, *screen1, screen1->GetRect(), 255);
		dst.Blit(-w + w * current_frame / tf_off, 0, *screen2, screen2->GetRect(), 255);
		break;
	case TransitionVerticalCombine:
	case TransitionVerticalDivision: {
		// If TransitionVerticalCombine, invert current_frame and screen:
		int ver_cf = transition_type == TransitionVerticalCombine ? tf_off - current_frame : current_frame;
		screen_pointer1 = transition_type == TransitionVerticalCombine ? screen2 : screen1;
		screen_pointer2 = transition_type == TransitionVerticalCombine ? screen1 : screen2;

		dst.Blit(0, -(h / 2) * ver_cf / tf_off, *screen_pointer1, Rect(0, 0, w, h / 2), 255);
		dst.Blit(0, h / 2 + (h / 2) * ver_cf / tf_off, *screen_pointer1, Rect(0, h / 2, w, h / 2), 255);
		dst.Blit(0, h / 2 - (h / 2) * ver_cf / tf_off, *screen_pointer2, Rect(0, h / 2 - (h / 2) * ver_cf / tf_off, w, h * ver_cf / tf_off), 255);
		break;
	}
	case TransitionHorizontalCombine:
	case TransitionHorizontalDivision: {
		// If TransitionHorizontalCombine, invert current_frame and screen:
		int hor_cf = transition_type == TransitionHorizontalCombine ? tf_off - current_frame : current_frame;
		screen_pointer1 = transition_type == TransitionHorizontalCombine ? screen2 : screen1;
		screen_pointer2 = transition_type == TransitionHorizontalCombine ? screen1 : screen2;

		dst.Blit(-(w / 2) * hor_cf / tf_off, 0, *screen_pointer1, Rect(0, 0, w / 2, h), 255);
		dst.Blit(w / 2 + (w / 2) * hor_cf / tf_off, 0, *screen_pointer1, Rect(w / 2, 0, w / 2, h), 255);
		dst.Blit(w / 2 - (w / 2) * hor_cf / tf_off, 0, *screen_pointer2, Rect(w / 2 - (w / 2) * hor_cf / tf_off, 0, w * hor_cf / tf_off, h), 255);
		break;
	}
	case TransitionCrossCombine:
	case TransitionCrossDivision: {
		// If TransitionCrossCombine, invert current_frame and screen:
		int cross_cf = transition_type == TransitionCrossCombine ? tf_off - current_frame : current_frame;
		screen_pointer1 = transition_type == TransitionCrossCombine ? screen2 : screen1;
		screen_pointer2 = transition_type == TransitionCrossCombine ? screen1 : screen2;

		dst.Blit(-(w / 2) * cross_cf / tf_off, -(h / 2) * cross_cf / tf_off, *screen_pointer1, Rect(0, 0, w / 2, h / 2), 255);
		dst.Blit(w / 2 + (w / 2) * cross_cf / tf_off, -(h / 2) * cross_cf / tf_off, *screen_pointer1, Rect(w / 2, 0, w / 2, h / 2), 255);
		dst.Blit(w / 2 + (w / 2) * cross_cf / tf_off, h / 2 + (h / 2) * cross_cf / tf_off, *screen_pointer1, Rect(w / 2, h / 2, w / 2, h / 2), 255);
		dst.Blit(-(w / 2) * cross_cf / tf_off, h / 2 + (h / 2) * cross_cf / tf_off, *screen_pointer1, Rect(0, h / 2, w / 2, h / 2), 255);
		dst.Blit(w / 2 - (w / 2) * cross_cf / tf_off, 0, *screen_pointer2, Rect(w / 2 - (w / 2) * cross_cf / tf_off, 0, w * cross_cf / tf_off, h / 2 - (h / 2) * cross_cf / tf_off), 255);
		dst.Blit(w / 2 - (w / 2) * cross_cf / tf_off, h / 2 + (h / 2) * cross_cf / tf_off, *screen_pointer2, Rect(w / 2 - (w / 2) * cross_cf / tf_off, h / 2 + (h / 2) * cross_cf / tf_off, w * cross_cf / tf_off, h / 2 + (h / 2) * cross_cf / tf_off), 255);
		dst.Blit(0, h / 2 - (h / 2) * cross_cf / tf_off, *screen_pointer2, Rect(0, h / 2 - (h / 2) * cross_cf / tf_off, w, h * cross_cf / tf_off), 255);
		break;
	}
	case TransitionZoomIn:
	case TransitionZoomOut: {
		// If TransitionZoomOut, invert current_frame and screen:
		int z_cf = transition_type == TransitionZoomOut ? tf_off - current_frame : current_frame;
		screen_pointer1 = transition_type == TransitionZoomOut ? screen2 : screen1;

		// X Coordinate: [0]   Y Coordinate: [1]
		z_length[0] = w;
		z_length[1] = h;
		z_cf = z_cf <= total_frames - 2 ? z_cf : total_frames - 2;

		for (int i = 0; i < 2; i++) {
			z_min = z_length[i] / 4;
			z_max = z_length[i] * 3 / 4;
			z_pos[i] = std::max(z_min, std::min((int)zoom_position[i], z_max)) * z_cf / tf_off;
			z_size[i] = z_length[i] * (tf_off - z_cf) / tf_off;

			z_frame = (zoom_position[i] < z_min) ? (tf_off * zoom_position[i] / z_min - tf_off) :
				(zoom_position[i] > z_max) ? (tf_off * (zoom_position[i] - z_max) / (z_length[i] - z_max)) : 0;

			if (z_frame != 0 && z_cf > 0) {
				z_fixed_pos = z_pos[i] * std::abs(z_frame) / z_cf;
				z_fixed_size = z_length[i] * (tf_off - std::abs(z_frame)) / tf_off;
				z_pos[i] += z_cf < std::abs(z_frame) ? (z_frame > 0 ? 1 : 0) * (z_length[i] - z_size[i]) - z_pos[i] :
					(z_frame > 0 ? z_length[i] - z_fixed_pos - z_fixed_size : -z_fixed_pos);
			}
		}

		dst.StretchBlit(Rect(0, 0, w, h), *screen_pointer1, Rect(z_pos[0], z_pos[1], z_size[0], z_size[1]), 255);
		break;
	}
	case TransitionMosaicIn:
	case TransitionMosaicOut: {
		// Goes from scale 1 to 41 (current_frame is 0 - 40)
		// If TransitionMosaicIn, invert scale and screen:
		int32_t rand;
		if (transition_type == TransitionMosaicIn) {
			m_size = total_frames - current_frame;
			screen_pointer1 = screen2;
			rand = mosaic_random_offset[total_frames - current_frame - 1];
		} else {
			m_size = current_frame + 1;
			screen_pointer1 = screen1;
			rand = mosaic_random_offset[current_frame];
		}

		// The offset defines where at (X,Y) the pixel is picked for scaling (nearest neighbour)
		// The pixel is usually initially out of bounds
		// in this case the nearest pixel of the image is choosen (edge handling = extend)
		int off = (m_size / 2);

		for (int row = 0; row < h + rand; ++row) {
			int src_row = std::clamp(((row + off) / m_size) * m_size - off, 0, h - 1);

			for (int col = 0; col < w + rand; ++col) {
				int src_col = std::clamp(((col + off) / m_size) * m_size - off, 0, w - 1);
				m_pointer = static_cast<uint32_t*>(screen_pointer1->pixels()) + src_row * w + src_col;
				dst.pixel_format.uint32_to_rgba(*m_pointer, m_r, m_g, m_b, m_a);

				Rect r(col - rand, row - rand, 1, 1);
				dst.FillRect(r, Color(m_r, m_g, m_b, 255));
			}
		}
		break;
	}
	case TransitionWaveIn:
	case TransitionWaveOut:
		{
			// If TransitionWaveIn, invert depth, phase and screen:
			auto p = (transition_type == TransitionWaveIn) ? total_frames - current_frame : current_frame + 1;
			auto& screen = (transition_type == TransitionWaveIn) ? *screen2 : *screen1;
			auto depth = p;
			auto phase = p * 5 * M_PI / tf_off + M_PI;
			dst.FillRect(Rect(0, 0, w, h), Color(0, 0, 0, 255));
			dst.WaverBlit(0, 0, 1, 1, screen, screen.GetRect(), depth, phase, Opacity::Opaque());
		}
		break;
	case TransitionCutIn:
		dst.Blit(0, 0, *screen2, screen2->GetRect(), Opacity::Opaque());
		break;
	case TransitionCutOut:
		dst.Blit(0, 0, *screen1, screen1->GetRect(), Opacity::Opaque());
		break;
	case TransitionNone:
		break;
	}
}

void Transition::Update() {
	if (!IsActive()) {
		return;
	}

	// FIXME: Break this dependency on DisplayUI
	if (transition_type != TransitionNone && !screen2) {
		// Wait for all graphics to load before drawing screens.
		if (FromErase() && AsyncHandler::IsGraphicFilePending()) {
			return;
		}

		if (scene) {
			scene->UpdateGraphics();
		}

		if (!screen1) {
			// erase -> erase is ingored
			// any -> erase - screen1 was drawn in init.
			assert(ToErase() && !FromErase());
			screen1 =  Bitmap::Create(Player::screen_width, Player::screen_height, false);
			Graphics::LocalDraw(*screen1, std::numeric_limits<Drawable::Z_t>::min(), GetZ() - 1);
		}
		if (ToErase()) {
			screen2 = Bitmap::Create(Player::screen_width, Player::screen_height, Color(0, 0, 0, 255));
		} else {
			screen2 =  Bitmap::Create(Player::screen_width, Player::screen_height, false);
			Graphics::LocalDraw(*screen2, std::numeric_limits<Drawable::Z_t>::min(), GetZ() - 1);
		}
	}

	SetVisible(true);

	if (flash_iterations > 0) {
		if (flash.time_left > 0) {
			Flash::Update(flash.current_level, flash.time_left);
			if (flash.time_left > 0) {
				return;
			}
			--flash_iterations;
		}
		if (flash_iterations > 0) {
			flash.current_level = flash_power;
			flash.time_left = flash_duration;
			return;
		}
	}
	//Update current_frame:
	current_frame++;
}

