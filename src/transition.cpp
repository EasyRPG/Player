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

#include "transition.h"
#include "baseui.h"
#include "bitmap.h"
#include "game_player.h"
#include "graphics.h"
#include "main_data.h"
#include "scene.h"
#include "drawable.h"

Transition::Transition() {
	flash_iterations = 0;
	flash_duration = 0;
	current_frame = -1;
	total_frames = -2;
	black_screen = nullptr;
}

Transition::~Transition() {
	Graphics::RemoveDrawable(this);
}

int Transition::GetZ() const {
	return z;
}

DrawableType Transition::GetType() const {
	return type;
}

void Transition::AppendBefore(Color color, int duration, int iterations) {
	if (!IsActive()) {
		current_frame = 0;
		total_frames = 0;
	}
	flash_color = color;
	flash_duration = std::max(1, duration);
	flash_iterations = std::max(1, iterations);
	total_frames += flash_duration * flash_iterations;
}

void Transition::Init(TransitionType type, Scene *linked_scene, int duration, bool erase) {
	if (!black_screen && DisplayUi) {
		black_screen = Bitmap::Create(DisplayUi->GetWidth(), DisplayUi->GetHeight(), Color(0, 0, 0, 255));
		Graphics::RegisterDrawable(this);
	}

	if (erase && type == TransitionNone) {
		old_frozen_screen = Graphics::SnapToBitmap(z);
		screen1 = old_frozen_screen;
		return;
	}
	else if ((screen_erased && erase) || type == TransitionNone) {
		return;
	}

	frozen_screen = Graphics::SnapToBitmap(z);
	screen1 = erase ? frozen_screen : old_frozen_screen? old_frozen_screen : black_screen;
	screen2 = erase ? black_screen : frozen_screen;

	transition_type = type;
	scene = linked_scene;
	screen_erased = erase;
	old_frozen_screen = nullptr;

	current_frame = 0;
	flash_iterations = 0;
	flash_duration = 0;
	total_frames = transition_type == TransitionErase ? 1 : duration;

	SetAttributesTransitions();
}

void Transition::SetAttributesTransitions() {
	int w, h, beg_i, mid_i, end_i, length;

	zoom_position = std::vector<int>(2);
	random_blocks = std::vector<uint32_t>(DisplayUi->GetWidth() * DisplayUi->GetHeight() / (size_random_blocks * size_random_blocks));
	for (uint32_t i = 0; i < random_blocks.size(); i++) {
		random_blocks[i] = i;
	}

	switch (transition_type) {
	case TransitionRandomBlocks:
		random_block_transition = Bitmap::Create(DisplayUi->GetWidth(), DisplayUi->GetHeight(), true);
		current_blocks_print = 0;
		std::shuffle(random_blocks.begin(), random_blocks.end(), Utils::GetRNG());
		break;
	case TransitionRandomBlocksDown:
	case TransitionRandomBlocksUp:
		random_block_transition = Bitmap::Create(DisplayUi->GetWidth(), DisplayUi->GetHeight(), true);
		current_blocks_print = 0;
		if (transition_type == TransitionRandomBlocksUp) { std::reverse(random_blocks.begin(), random_blocks.end()); }

		w = DisplayUi->GetWidth() / 4;
		h = DisplayUi->GetHeight() / 4;
		length = 10;
		for (int i = 0; i < h - 1; i++) {
			end_i = (i < length ? 2 * i + 1 : i <= h - length ? i + length : (i + h) / 2) * w;
			std::shuffle(random_blocks.begin() + i * w, random_blocks.begin() + end_i, Utils::GetRNG());

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
			zoom_position[0] = std::max(0, std::min(Main_Data::game_player->GetScreenX(), (int)DisplayUi->GetWidth()));
			zoom_position[1] = std::max(0, std::min(Main_Data::game_player->GetScreenY() - 8, (int)DisplayUi->GetHeight()));
		}
		else {
			zoom_position[0] = DisplayUi->GetWidth() / 2;
			zoom_position[1] = DisplayUi->GetHeight() / 2;
		}
		break;
	default:
		// do nothing, keep the compiler happy
		break;
	}
}

void Transition::Draw() {
	if (!IsActive())
		return;

	std::vector<int> z_pos(2), z_size(2), z_length(2);
	int z_min, z_max, z_percent, z_fixed_pos, z_fixed_size;
	uint8_t m_r, m_g, m_b, m_a;
	uint32_t *m_pointer, blocks_to_print;
	int m_size;

	BitmapRef dst = DisplayUi->GetDisplaySurface(), screen_pointer1, screen_pointer2;
	int w = DisplayUi->GetWidth();
	int h = DisplayUi->GetHeight();

	if (current_frame < flash_duration * flash_iterations) {
		Color current_color = Color(flash_color.red, flash_color.green, flash_color.blue, (flash_duration - current_frame % flash_duration) * 255 / flash_duration);
		dst->BlendBlit(0, 0, *screen1, screen1->GetRect(), current_color, 255);
		return;
	}
	else if (total_frames == flash_duration * flash_iterations) {
		return;
	}

	int percentage = (current_frame - flash_duration * flash_iterations) * 100 / (total_frames - flash_duration * flash_iterations);

	switch (transition_type) {
	case TransitionFadeIn:
	case TransitionFadeOut:
		dst->Blit(0, 0, *screen1, screen1->GetRect(), 255);
		dst->Blit(0, 0, *screen2, screen2->GetRect(), 255 * percentage / 100);
		break;
	case TransitionRandomBlocks:
	case TransitionRandomBlocksDown:
	case TransitionRandomBlocksUp:
		blocks_to_print = random_blocks.size() * percentage / 100;

		for (uint32_t i = current_blocks_print; i < blocks_to_print; i++) {
			random_block_transition->Blit(random_blocks[i] % (w / size_random_blocks) * size_random_blocks,
				random_blocks[i] / (w / size_random_blocks) * size_random_blocks, *screen2,
				Rect(random_blocks[i] % (w / size_random_blocks) * size_random_blocks, random_blocks[i] / (w / size_random_blocks) * size_random_blocks,
					size_random_blocks, size_random_blocks), Opacity::opaque);
		}
		dst->Blit(0, 0, *screen1, screen1->GetRect(), Opacity::opaque);
		dst->Blit(0, 0, *random_block_transition, random_block_transition->GetRect(), Opacity::opaque);
		current_blocks_print = blocks_to_print;
		break;
	case TransitionBlindOpen:
		for (int i = 0; i < h / 8; i++) {
			dst->Blit(0, i * 8, *screen1, Rect(0, i * 8, w, 8 - 8 * percentage / 100), 255);
			dst->Blit(0, i * 8 + 8 - 8 * percentage / 100, *screen2, Rect(0, i * 8 + 8 - 8 * percentage / 100, w, 8 * percentage / 100), 255 * percentage / 100);
		}
		break;
	case TransitionBlindClose:
		for (int i = 0; i < h / 8; i++) {
			dst->Blit(0, i * 8 + 8 * percentage / 100, *screen1, Rect(0, i * 8 + 8 * percentage / 100, w, 8 - 8 * percentage / 100), 255);
			dst->Blit(0, i * 8, *screen2, Rect(0, i * 8, w, 8 * percentage / 100), 255);
		}
		break;
	case TransitionVerticalStripesIn:
	case TransitionVerticalStripesOut:
		for (int i = 0; i < h / 6 + 1 - h / 6 * percentage / 100; i++) {
			dst->Blit(0, i * 6 + 3, *screen1, Rect(0, i * 6 + 3, w, 3), 255);
			dst->Blit(0, h - i * 6, *screen1, Rect(0, h - i * 6, w, 3), 255);
		}
		for (int i = 0; i < h / 6 * percentage / 100; i++) {
			dst->Blit(0, i * 6, *screen2, Rect(0, i * 6, w, 3), 255);
			dst->Blit(0, h - 3 - i * 6, *screen2, Rect(0, h - 3 - i * 6, w, 3), 255);
		}
		break;
	case TransitionHorizontalStripesIn:
	case TransitionHorizontalStripesOut:
		for (int i = 0; i < w / 8 + 1 - w / 8 * percentage / 100; i++) {
			dst->Blit(i * 8 + 4, 0, *screen1, Rect(i * 8 + 4, 0, 4, h), 255);
			dst->Blit(w - i * 8, 0, *screen1, Rect(w - i * 8, 0, 4, h), 255);
		}
		for (int i = 0; i < w / 8 * percentage / 100; i++) {
			dst->Blit(i * 8, 0, *screen2, Rect(i * 8, 0, 4, h), 255);
			dst->Blit(w - 4 - i * 8, 0, *screen2, Rect(w - 4 - i * 8, 0, 4, h), 255);
		}
		break;
	case TransitionBorderToCenterIn:
	case TransitionBorderToCenterOut:
		dst->Blit(0, 0, *screen2, screen2->GetRect(), 255);
		dst->Blit((w / 2) * percentage / 100, (h / 2) * percentage / 100, *screen1, Rect((w / 2) * percentage / 100, (h / 2) * percentage / 100, w - w * percentage / 100, h - h * percentage / 100), 255);
		break;
	case TransitionCenterToBorderIn:
	case TransitionCenterToBorderOut:
		dst->Blit(0, 0, *screen1, screen1->GetRect(), 255);
		dst->Blit(w / 2 - (w / 2) * percentage / 100, h / 2 - (h / 2) * percentage / 100, *screen2, Rect(w / 2 - (w / 2) * percentage / 100, h / 2 - (h / 2) * percentage / 100, w * percentage / 100, h * percentage / 100), 255);
		break;
	case TransitionScrollUpIn:
	case TransitionScrollUpOut:
		dst->Blit(0, -h * percentage / 100, *screen1, screen1->GetRect(), 255);
		dst->Blit(0, h - h * percentage / 100, *screen2, screen2->GetRect(), 255);
		break;
	case TransitionScrollDownIn:
	case TransitionScrollDownOut:
		dst->Blit(0, h * percentage / 100, *screen1, screen1->GetRect(), 255);
		dst->Blit(0, -h + h * percentage / 100, *screen2, screen2->GetRect(), 255);
		break;
	case TransitionScrollLeftIn:
	case TransitionScrollLeftOut:
		dst->Blit(-w * percentage / 100, 0, *screen1, screen1->GetRect(), 255);
		dst->Blit(w - w * percentage / 100, 0, *screen2, screen2->GetRect(), 255);
		break;
	case TransitionScrollRightIn:
	case TransitionScrollRightOut:
		dst->Blit(w * percentage / 100, 0, *screen1, screen1->GetRect(), 255);
		dst->Blit(-w + w * percentage / 100, 0, *screen2, screen2->GetRect(), 255);
		break;
	case TransitionVerticalCombine:
	case TransitionVerticalDivision:
		// If TransitionVerticalCombine, invert percentage and screen:
		if (transition_type == TransitionVerticalCombine) { percentage = 100 - percentage; }
		screen_pointer1 = transition_type == TransitionVerticalCombine ? screen2 : screen1;
		screen_pointer2 = transition_type == TransitionVerticalCombine ? screen1 : screen2;

		dst->Blit(0, -(h / 2) * percentage / 100, *screen_pointer1, Rect(0, 0, w, h / 2), 255);
		dst->Blit(0, h / 2 + (h / 2) * percentage / 100, *screen_pointer1, Rect(0, h / 2, w, h / 2), 255);
		dst->Blit(0, h / 2 - (h / 2) * percentage / 100, *screen_pointer2, Rect(0, h / 2 - (h / 2) * percentage / 100, w, h * percentage / 100), 255);
		break;
	case TransitionHorizontalCombine:
	case TransitionHorizontalDivision:
		// If TransitionHorizontalCombine, invert percentage and screen:
		if (transition_type == TransitionHorizontalCombine) { percentage = 100 - percentage; }
		screen_pointer1 = transition_type == TransitionHorizontalCombine ? screen2 : screen1;
		screen_pointer2 = transition_type == TransitionHorizontalCombine ? screen1 : screen2;

		dst->Blit(-(w / 2) * percentage / 100, 0, *screen_pointer1, Rect(0, 0, w / 2, h), 255);
		dst->Blit(w / 2 + (w / 2) * percentage / 100, 0, *screen_pointer1, Rect(w / 2, 0, w / 2, h), 255);
		dst->Blit(w / 2 - (w / 2) * percentage / 100, 0, *screen_pointer2, Rect(w / 2 - (w / 2) * percentage / 100, 0, w * percentage / 100, h), 255);
		break;
	case TransitionCrossCombine:
	case TransitionCrossDivision:
		// If TransitionCrossCombine, invert percentage and screen:
		if (transition_type == TransitionCrossCombine) { percentage = 100 - percentage; }
		screen_pointer1 = transition_type == TransitionCrossCombine ? screen2 : screen1;
		screen_pointer2 = transition_type == TransitionCrossCombine ? screen1 : screen2;

		dst->Blit(-(w / 2) * percentage / 100, -(h / 2) * percentage / 100, *screen_pointer1, Rect(0, 0, w / 2, h / 2), 255);
		dst->Blit(w / 2 + (w / 2) * percentage / 100, -(h / 2) * percentage / 100, *screen_pointer1, Rect(w / 2, 0, w / 2, h / 2), 255);
		dst->Blit(w / 2 + (w / 2) * percentage / 100, h / 2 + (h / 2) * percentage / 100, *screen_pointer1, Rect(w / 2, h / 2, w / 2, h / 2), 255);
		dst->Blit(-(w / 2) * percentage / 100, h / 2 + (h / 2) * percentage / 100, *screen_pointer1, Rect(0, h / 2, w / 2, h / 2), 255);
		dst->Blit(w / 2 - (w / 2) * percentage / 100, 0, *screen_pointer2, Rect(w / 2 - (w / 2) * percentage / 100, 0, w * percentage / 100, h / 2 - (h / 2) * percentage / 100), 255);
		dst->Blit(w / 2 - (w / 2) * percentage / 100, h / 2 + (h / 2) * percentage / 100, *screen_pointer2, Rect(w / 2 - (w / 2) * percentage / 100, h / 2 + (h / 2) * percentage / 100, w * percentage / 100, h / 2 + (h / 2) * percentage / 100), 255);
		dst->Blit(0, h / 2 - (h / 2) * percentage / 100, *screen_pointer2, Rect(0, h / 2 - (h / 2) * percentage / 100, w, h * percentage / 100), 255);
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

		dst->StretchBlit(Rect(0, 0, w, h), *screen_pointer1, Rect(z_pos[0], z_pos[1], z_size[0], z_size[1]), 255);
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
					dst->pixel_format.uint32_to_rgba(*m_pointer, m_r, m_g, m_b, m_a);
					dst->FillRect(Rect(i - ((m_size - w % m_size) % m_size) / 2, j - ((m_size - h % m_size) % m_size) / 2, m_size, m_size), Color(m_r, m_g, m_b, m_a));
				}
		else
			dst->Blit(0, 0, *screen_pointer1, screen_pointer1->GetRect(), 255);
		break;
	case TransitionWaveIn:
	case TransitionWaveOut:
		// If TransitionWaveIn, invert percentage and screen:
		if (transition_type == TransitionWaveIn) { percentage = 100 - percentage; }
		screen_pointer1 = transition_type == TransitionWaveIn ? screen2 : screen1;
		screen_pointer2 = transition_type == TransitionWaveIn ? screen1 : screen2;

		dst->Blit(0, 0, *screen_pointer2, screen_pointer1->GetRect(), 255);
		dst->WaverBlit(0, 0, 1, 1, *screen_pointer1, screen_pointer2->GetRect(), percentage * 2 / 5, percentage * 8, 255);
		break;
	default:
		if (screen_erased)
			DisplayUi->CleanDisplay();
		else
			dst->Blit(0, 0, *screen1, screen1->GetRect(), 255);
		break;
	}
}

void Transition::Update() {
	if (IsActive()) {
		//Update current_frame:
		current_frame++;
	}
	else if (frozen_screen) {
		frozen_screen.reset();
		frozen_screen = nullptr;
	}
}

bool Transition::IsGlobal() const {
	return true;
}

bool Transition::IsActive() {
	return current_frame <= total_frames;
}

bool Transition::IsErased() {
	return screen_erased && !IsActive();
}
