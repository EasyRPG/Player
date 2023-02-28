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
			return 32;
		case TransitionCutIn:
		case TransitionCutOut:
		case TransitionNone:
			return 0;
		default:
			return 40;
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

	current_frame = 0;
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
			zoom_position[0] = std::max(0, std::min(Main_Data::game_player->GetScreenX(), (int)Player::screen_width));
			zoom_position[1] = std::max(0, std::min(Main_Data::game_player->GetScreenY() - 8, (int)Player::screen_height));
		}
		else {
			zoom_position[0] = Player::screen_width / 2;
			zoom_position[1] = Player::screen_height / 2;
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
	int z_min, z_max, z_percent, z_fixed_pos, z_fixed_size;
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

	int percentage = (current_frame) * 100 / (total_frames);

	switch (transition_type) {
	case TransitionFadeIn:
	case TransitionFadeOut:
		dst.Blit(0, 0, *screen1, screen1->GetRect(), 255);
		dst.Blit(0, 0, *screen2, screen2->GetRect(), 255 * percentage / 100);
		break;
	case TransitionRandomBlocks:
	case TransitionRandomBlocksDown:
	case TransitionRandomBlocksUp:
		blocks_to_print = random_blocks.size() * percentage / 100;

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
			dst.Blit(0, i * 8, *screen1, Rect(0, i * 8, w, 8 - 8 * percentage / 100), 255);
			dst.Blit(0, i * 8 + 8 - 8 * percentage / 100, *screen2, Rect(0, i * 8 + 8 - 8 * percentage / 100, w, 8 * percentage / 100), 255);
		}
		break;
	case TransitionBlindClose:
		for (int i = 0; i < h / 8; i++) {
			dst.Blit(0, i * 8 + 8 * percentage / 100, *screen1, Rect(0, i * 8 + 8 * percentage / 100, w, 8 - 8 * percentage / 100), 255);
			dst.Blit(0, i * 8, *screen2, Rect(0, i * 8, w, 8 * percentage / 100), 255);
		}
		break;
	case TransitionVerticalStripesIn:
	case TransitionVerticalStripesOut:
		for (int i = 0; i < h / 6 + 1 - h / 6 * percentage / 100; i++) {
			dst.Blit(0, i * 6 + 3, *screen1, Rect(0, i * 6 + 3, w, 3), 255);
			dst.Blit(0, h - i * 6, *screen1, Rect(0, h - i * 6, w, 3), 255);
		}
		for (int i = 0; i < h / 6 * percentage / 100; i++) {
			dst.Blit(0, i * 6, *screen2, Rect(0, i * 6, w, 3), 255);
			dst.Blit(0, h - 3 - i * 6, *screen2, Rect(0, h - 3 - i * 6, w, 3), 255);
		}
		break;
	case TransitionHorizontalStripesIn:
	case TransitionHorizontalStripesOut:
		for (int i = 0; i < w / 8 + 1 - w / 8 * percentage / 100; i++) {
			dst.Blit(i * 8 + 4, 0, *screen1, Rect(i * 8 + 4, 0, 4, h), 255);
			dst.Blit(w - i * 8, 0, *screen1, Rect(w - i * 8, 0, 4, h), 255);
		}
		for (int i = 0; i < w / 8 * percentage / 100; i++) {
			dst.Blit(i * 8, 0, *screen2, Rect(i * 8, 0, 4, h), 255);
			dst.Blit(w - 4 - i * 8, 0, *screen2, Rect(w - 4 - i * 8, 0, 4, h), 255);
		}
		break;
	case TransitionBorderToCenterIn:
	case TransitionBorderToCenterOut:
		dst.Blit(0, 0, *screen2, screen2->GetRect(), 255);
		dst.Blit((w / 2) * percentage / 100, (h / 2) * percentage / 100, *screen1, Rect((w / 2) * percentage / 100, (h / 2) * percentage / 100, w - w * percentage / 100, h - h * percentage / 100), 255);
		break;
	case TransitionCenterToBorderIn:
	case TransitionCenterToBorderOut:
		dst.Blit(0, 0, *screen1, screen1->GetRect(), 255);
		dst.Blit(w / 2 - (w / 2) * percentage / 100, h / 2 - (h / 2) * percentage / 100, *screen2, Rect(w / 2 - (w / 2) * percentage / 100, h / 2 - (h / 2) * percentage / 100, w * percentage / 100, h * percentage / 100), 255);
		break;
	case TransitionScrollUpIn:
	case TransitionScrollUpOut:
		dst.Blit(0, -h * percentage / 100, *screen1, screen1->GetRect(), 255);
		dst.Blit(0, h - h * percentage / 100, *screen2, screen2->GetRect(), 255);
		break;
	case TransitionScrollDownIn:
	case TransitionScrollDownOut:
		dst.Blit(0, h * percentage / 100, *screen1, screen1->GetRect(), 255);
		dst.Blit(0, -h + h * percentage / 100, *screen2, screen2->GetRect(), 255);
		break;
	case TransitionScrollLeftIn:
	case TransitionScrollLeftOut:
		dst.Blit(-w * percentage / 100, 0, *screen1, screen1->GetRect(), 255);
		dst.Blit(w - w * percentage / 100, 0, *screen2, screen2->GetRect(), 255);
		break;
	case TransitionScrollRightIn:
	case TransitionScrollRightOut:
		dst.Blit(w * percentage / 100, 0, *screen1, screen1->GetRect(), 255);
		dst.Blit(-w + w * percentage / 100, 0, *screen2, screen2->GetRect(), 255);
		break;
	case TransitionVerticalCombine:
	case TransitionVerticalDivision:
		// If TransitionVerticalCombine, invert percentage and screen:
		if (transition_type == TransitionVerticalCombine) { percentage = 100 - percentage; }
		screen_pointer1 = transition_type == TransitionVerticalCombine ? screen2 : screen1;
		screen_pointer2 = transition_type == TransitionVerticalCombine ? screen1 : screen2;

		dst.Blit(0, -(h / 2) * percentage / 100, *screen_pointer1, Rect(0, 0, w, h / 2), 255);
		dst.Blit(0, h / 2 + (h / 2) * percentage / 100, *screen_pointer1, Rect(0, h / 2, w, h / 2), 255);
		dst.Blit(0, h / 2 - (h / 2) * percentage / 100, *screen_pointer2, Rect(0, h / 2 - (h / 2) * percentage / 100, w, h * percentage / 100), 255);
		break;
	case TransitionHorizontalCombine:
	case TransitionHorizontalDivision:
		// If TransitionHorizontalCombine, invert percentage and screen:
		if (transition_type == TransitionHorizontalCombine) { percentage = 100 - percentage; }
		screen_pointer1 = transition_type == TransitionHorizontalCombine ? screen2 : screen1;
		screen_pointer2 = transition_type == TransitionHorizontalCombine ? screen1 : screen2;

		dst.Blit(-(w / 2) * percentage / 100, 0, *screen_pointer1, Rect(0, 0, w / 2, h), 255);
		dst.Blit(w / 2 + (w / 2) * percentage / 100, 0, *screen_pointer1, Rect(w / 2, 0, w / 2, h), 255);
		dst.Blit(w / 2 - (w / 2) * percentage / 100, 0, *screen_pointer2, Rect(w / 2 - (w / 2) * percentage / 100, 0, w * percentage / 100, h), 255);
		break;
	case TransitionCrossCombine:
	case TransitionCrossDivision:
		// If TransitionCrossCombine, invert percentage and screen:
		if (transition_type == TransitionCrossCombine) { percentage = 100 - percentage; }
		screen_pointer1 = transition_type == TransitionCrossCombine ? screen2 : screen1;
		screen_pointer2 = transition_type == TransitionCrossCombine ? screen1 : screen2;

		dst.Blit(-(w / 2) * percentage / 100, -(h / 2) * percentage / 100, *screen_pointer1, Rect(0, 0, w / 2, h / 2), 255);
		dst.Blit(w / 2 + (w / 2) * percentage / 100, -(h / 2) * percentage / 100, *screen_pointer1, Rect(w / 2, 0, w / 2, h / 2), 255);
		dst.Blit(w / 2 + (w / 2) * percentage / 100, h / 2 + (h / 2) * percentage / 100, *screen_pointer1, Rect(w / 2, h / 2, w / 2, h / 2), 255);
		dst.Blit(-(w / 2) * percentage / 100, h / 2 + (h / 2) * percentage / 100, *screen_pointer1, Rect(0, h / 2, w / 2, h / 2), 255);
		dst.Blit(w / 2 - (w / 2) * percentage / 100, 0, *screen_pointer2, Rect(w / 2 - (w / 2) * percentage / 100, 0, w * percentage / 100, h / 2 - (h / 2) * percentage / 100), 255);
		dst.Blit(w / 2 - (w / 2) * percentage / 100, h / 2 + (h / 2) * percentage / 100, *screen_pointer2, Rect(w / 2 - (w / 2) * percentage / 100, h / 2 + (h / 2) * percentage / 100, w * percentage / 100, h / 2 + (h / 2) * percentage / 100), 255);
		dst.Blit(0, h / 2 - (h / 2) * percentage / 100, *screen_pointer2, Rect(0, h / 2 - (h / 2) * percentage / 100, w, h * percentage / 100), 255);
		break;
	case TransitionZoomIn:
	case TransitionZoomOut:
		// If TransitionZoomOut, invert percentage and screen:
		if (transition_type == TransitionZoomOut) { percentage = 100 - percentage; }
		screen_pointer1 = transition_type == TransitionZoomOut ? screen2 : screen1;

		// X Coordinate: [0]   Y Coordinate: [1]
		z_length[0] = w;
		z_length[1] = h;
		percentage = percentage <= 97 ? percentage : 97;

		for (int i = 0; i < 2; i++) {
			z_min = z_length[i] / 4;
			z_max = z_length[i] * 3 / 4;
			z_pos[i] = std::max(z_min, std::min((int)zoom_position[i], z_max)) * percentage / 100;
			z_size[i] = z_length[i] * (100 - percentage) / 100;

			z_percent = (zoom_position[i] < z_min) ? (100 * zoom_position[i] / z_min - 100) :
				(zoom_position[i] > z_max) ? (100 * (zoom_position[i] - z_max) / (z_length[i] - z_max)) : 0;

			if (z_percent != 0 && percentage > 0) {
				z_fixed_pos = z_pos[i] * std::abs(z_percent) / percentage;
				z_fixed_size = z_length[i] * (100 - std::abs(z_percent)) / 100;
				z_pos[i] += percentage < std::abs(z_percent) ? (z_percent > 0 ? 1 : 0) * (z_length[i] - z_size[i]) - z_pos[i] :
					(z_percent > 0 ? z_length[i] - z_fixed_pos - z_fixed_size : -z_fixed_pos);
			}
		}

		dst.StretchBlit(Rect(0, 0, w, h), *screen_pointer1, Rect(z_pos[0], z_pos[1], z_size[0], z_size[1]), 255);
		break;
	case TransitionMosaicIn:
	case TransitionMosaicOut:
		// If TransitionMosaicIn, invert percentage and screen:
		if (transition_type == TransitionMosaicIn) { percentage = 100 - percentage; }
		screen_pointer1 = transition_type == TransitionMosaicIn ? screen2 : screen1;

		m_size = (percentage + 1) * 4 / 10;
		if (m_size > 1)
			for (int i = 0; i < w; i += m_size)
				for (int j = 0; j < h; j += m_size) {
					m_pointer = static_cast<uint32_t *>(screen_pointer1->pixels()) + j * w + i;
					m_pointer += ((i == 0 ? 1 : 0) + (j == 0 ? w : 0)) * (m_size - 1);
					dst.pixel_format.uint32_to_rgba(*m_pointer, m_r, m_g, m_b, m_a);
					dst.FillRect(Rect(i - ((m_size - w % m_size) % m_size) / 2, j - ((m_size - h % m_size) % m_size) / 2, m_size, m_size), Color(m_r, m_g, m_b, m_a));
				}
		else
			dst.Blit(0, 0, *screen_pointer1, screen_pointer1->GetRect(), 255);
		break;
	case TransitionWaveIn:
	case TransitionWaveOut:
		{
			// If TransitionWaveIn, invert percentage and screen:
			auto p = (transition_type == TransitionWaveIn) ? 100 - percentage : percentage;
			auto& screen = (transition_type == TransitionWaveIn) ? *screen2 : *screen1;
			auto depth = p * 40 / 100;
			auto phase = p * 5 * M_PI / 100.0 + M_PI;
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

