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
#include "ui.h"
#include "bitmap.h"
#include "color.h"
#include "game_config.h"
#include "keys.h"
#include "output.h"
#include "player.h"
#include <cstring>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <psppower.h>

#ifdef SUPPORT_AUDIO
#include "audio.h"
AudioInterface& PspUi::GetAudio() {
	return *audio_;
}
#endif

namespace {
	// Native resolution of the PSP screen
	constexpr int SCREEN_WIDTH = 480;
	constexpr int SCREEN_HEIGHT = 272;
	// Row length of the framebuffers in pixels (VRAM is laid out in 512 pixel lines)
	constexpr int LINE_SIZE = 512;

	// VRAM lives at physical 0x04000000, 0x44000000 is the same memory uncached
	constexpr uintptr_t VRAM_CACHED = 0x04000000;
	constexpr uintptr_t VRAM_UNCACHED = 0x44000000;
	constexpr uintptr_t FRAMEBUFFER_SIZE = LINE_SIZE * SCREEN_HEIGHT * 4;
	constexpr uintptr_t FRAMEBUFFER_OFFSET[2] = { 0, FRAMEBUFFER_SIZE };

	uint32_t* VramUncached(int index) {
		return reinterpret_cast<uint32_t*>(VRAM_UNCACHED + FRAMEBUFFER_OFFSET[index]);
	}

	void* VramCached(int index) {
		return reinterpret_cast<void*>(VRAM_CACHED + FRAMEBUFFER_OFFSET[index]);
	}
}

PspUi::PspUi(int width, int height, const Game_Config& cfg) : BaseUi(cfg),
	frame_width(width), frame_height(height)
{
	SetIsFullscreen(true);

	current_display_mode.width = width;
	current_display_mode.height = height;
	current_display_mode.bpp = 32;

	// Pixels are stored as 0xAABBGGRR, which is the layout of the PSP's 8888 framebuffer
	const DynamicFormat format(
		32,
		0x000000FF,
		0x0000FF00,
		0x00FF0000,
		0xFF000000,
		PF::Alpha);
	Bitmap::SetFormat(Bitmap::ChooseFormat(format));

	frame_pixels.assign(static_cast<size_t>(width) * height, 0);
	main_surface = Bitmap::Create(frame_pixels.data(), width, height, width * 4, format);

	// Clear both framebuffers so the letterbox border is black
	for (int i = 0; i < 2; ++i) {
		std::memset(VramUncached(i), 0, FRAMEBUFFER_SIZE);
	}
	// Buffer 0 is shown first, so frames are drawn into buffer 1 (back_buffer)
	sceDisplaySetFrameBuf(VramCached(0), LINE_SIZE, PSP_DISPLAY_PIXEL_FORMAT_8888,
		PSP_DISPLAY_SETBUF_IMMEDIATE);

	#ifdef SUPPORT_AUDIO
	audio_ = std::make_unique<PspAudio>(cfg.audio);
	#endif

	// Run the CPU at its maximum speed
	scePowerSetClockFrequency(333, 333, 166);

	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
}

PspUi::~PspUi() {
	// Stop the audio thread before the frame it reads from goes away
#ifdef SUPPORT_AUDIO
	audio_.reset();
#endif
	main_surface = nullptr;
}

bool PspUi::ProcessEvents() {
	SceCtrlData pad = {};
	sceCtrlPeekBufferPositive(&pad, 1);
	const unsigned int buttons = pad.Buttons;

	auto pressed = [buttons](unsigned int button) {
		return (buttons & button) != 0;
	};

	// Face buttons; the default mappings in psp_input_buttons.cpp use these names
	keys[Input::Keys::JOY_A] = pressed(PSP_CTRL_CROSS);
	keys[Input::Keys::JOY_B] = pressed(PSP_CTRL_CIRCLE);
	keys[Input::Keys::JOY_X] = pressed(PSP_CTRL_SQUARE);
	keys[Input::Keys::JOY_Y] = pressed(PSP_CTRL_TRIANGLE);

	keys[Input::Keys::JOY_BACK] = pressed(PSP_CTRL_SELECT);
	keys[Input::Keys::JOY_START] = pressed(PSP_CTRL_START);

	keys[Input::Keys::JOY_DPAD_UP] = pressed(PSP_CTRL_UP);
	keys[Input::Keys::JOY_DPAD_DOWN] = pressed(PSP_CTRL_DOWN);
	keys[Input::Keys::JOY_DPAD_LEFT] = pressed(PSP_CTRL_LEFT);
	keys[Input::Keys::JOY_DPAD_RIGHT] = pressed(PSP_CTRL_RIGHT);

	keys[Input::Keys::JOY_SHOULDER_LEFT] = pressed(PSP_CTRL_LTRIGGER);
	keys[Input::Keys::JOY_SHOULDER_RIGHT] = pressed(PSP_CTRL_RTRIGGER);

	// The PSP only has one analog stick
	auto normalize = [](unsigned char value) {
		return static_cast<float>(static_cast<int>(value) - 127) / 128.f;
	};
	analog_input.primary.x = normalize(pad.Lx);
	analog_input.primary.y = normalize(pad.Ly);
	analog_input.secondary.x = 0.f;
	analog_input.secondary.y = 0.f;

	return true;
}

void PspUi::UpdateDisplay() {
	// Wait for vblank: this makes the buffer we are about to write no longer scanned out
	sceDisplayWaitVblankStart();

	const int x_off = (SCREEN_WIDTH - frame_width) / 2;
	const int y_off = (SCREEN_HEIGHT - frame_height) / 2;

	uint32_t* dst = VramUncached(back_buffer) + y_off * LINE_SIZE + x_off;
	const uint32_t* src = frame_pixels.data();
	const size_t row_bytes = static_cast<size_t>(frame_width) * 4;

	for (int y = 0; y < frame_height; ++y) {
		std::memcpy(dst, src, row_bytes);
		dst += LINE_SIZE;
		src += frame_width;
	}

	// Show the freshly drawn buffer on the next vblank and draw into the other one next time
	sceDisplaySetFrameBuf(VramCached(back_buffer), LINE_SIZE, PSP_DISPLAY_PIXEL_FORMAT_8888,
		PSP_DISPLAY_SETBUF_NEXTFRAME);
	back_buffer ^= 1;
}

void PspUi::SetScalingMode(ConfigEnum::ScalingMode) {
	// Only 1:1 is supported, the frame is always centered
}

void PspUi::ToggleStretch() {
	// Not supported
}

void PspUi::ToggleTouchUi() {
	// The PSP has no touchscreen
}

void PspUi::ToggleVsync() {
	// Frames are always synchronized with vblank
}

void PspUi::vGetConfig(Game_ConfigVideo& cfg) const {
	cfg.renderer.Lock("PSP (Software)");

	cfg.fps_limit.SetOptionVisible(true);
	cfg.vsync.SetOptionVisible(false);
	cfg.scaling_mode.SetOptionVisible(false);
	cfg.stretch.SetOptionVisible(false);
	cfg.touch_ui.SetOptionVisible(false);
}
