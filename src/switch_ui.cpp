/**
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

#ifdef SWITCH

// Headers
#include "switch_ui.h"
#include "color.h"
#include "graphics.h"
#include "keys.h"
#include "output.h"
#include "player.h"
#include "bitmap.h"
#include <iostream>
#include <sys/iosupport.h>

#include <switch.h>
#include <cstring>
#include <stdio.h>

#include "build/touch_ui_png.h"

#ifdef SUPPORT_AUDIO
#include "audio_switch.h"
AudioInterface& NxUi::GetAudio() {
	return *audio_;
}
#endif

namespace {
	const double ticks_per_msec = 19200.0f;
}

static const devoptab_t dotab_null = {
	"null", 0, NULL, NULL, NULL, NULL, NULL, NULL
};

NxUi::NxUi(int width, int height) :
	BaseUi() {

	fullscreen = false;

	gfxInitDefault();

	current_display_mode.width = width;
	current_display_mode.height = height;
	current_display_mode.bpp = 32;
	const DynamicFormat format(
		32,
		0x000000FF,
		0x0000FF00,
		0x00FF0000,
		0xFF000000,
		PF::NoAlpha);
	Bitmap::SetFormat(Bitmap::ChooseFormat(format));
	main_surface = Bitmap::Create(width, height, true, 32);

	touch_ui = Bitmap::Create(touch_ui_png, touch_ui_png_size, false);

	#ifdef SUPPORT_AUDIO
	audio_.reset(new NxAudio());
	#endif

}

NxUi::~NxUi() {
	gfxExit();
}

void NxUi::Sleep(uint32_t time) {
	u64 nsecs = time * 1000000;
	svcSleepThread(nsecs);
}

static inline double u64_to_double(u64 value) {
	return (((double)(u32)(value >> 32)) * 0x100000000ULL + (u32)value);
}

uint32_t NxUi::GetTicks() const {
	double ticks = u64_to_double(svcGetSystemTick());
	u64 msecs = (u64)(ticks / ticks_per_msec);
	return msecs;
}
void NxUi::BeginDisplayModeChange() {
	// no-op
}

void NxUi::EndDisplayModeChange() {
	// no-op
}

void NxUi::Resize(long /*width*/, long /*height*/) {
	// no-op
}

void NxUi::ToggleFullscreen() {
	// no-op
}

void NxUi::ToggleZoom() {
	// no-op
}

bool NxUi::IsFullscreen() {
	return true;
}

void NxUi::ProcessEvents() {
	hidScanInput();

	u32 input = hidKeysHeld(CONTROLLER_P1_AUTO);
	keys[Input::Keys::UP] = (input & KEY_DUP);
	keys[Input::Keys::DOWN] = (input & KEY_DDOWN);
	keys[Input::Keys::RIGHT] = (input & KEY_DRIGHT);
	keys[Input::Keys::LEFT] = (input & KEY_DLEFT);
	keys[Input::Keys::Z] = (input & KEY_A);
	keys[Input::Keys::X] = (input & KEY_B);
	keys[Input::Keys::X] = (input & KEY_X);
	keys[Input::Keys::LSHIFT] = (input & KEY_Y);
	keys[Input::Keys::F2] = (input & KEY_L);
	keys[Input::Keys::F] = (input & KEY_R);
	keys[Input::Keys::F12] = (input & KEY_MINUS);
	keys[Input::Keys::ESCAPE] = (input & KEY_PLUS);

	// cycle through GUI layouts
	if (!update_ui) {
		input = hidKeysDown(CONTROLLER_P1_AUTO);
		update_ui = (input & KEY_ZL);
		if (update_ui) {
			ui_mode = (ui_mode + 1) % 3;
		}
	}

	// Touch handling
	static const int touch_left[] = {
		Input::Keys::N1,
		Input::Keys::N2,
		Input::Keys::N3,
		Input::Keys::N4,
		Input::Keys::N5,
		Input::Keys::N6,
		Input::Keys::N7,
		Input::Keys::N8
	};

	static const int touch_right[] = {
		Input::Keys::ESCAPE,
		Input::Keys::N9,
		Input::Keys::N0,
		Input::Keys::PERIOD,
		Input::Keys::ADD,
		Input::Keys::SUBTRACT,
		Input::Keys::MULTIPLY,
		Input::Keys::DIVIDE
	};

	for (int i = 0; i < 8; ++i) {
		keys[touch_left[i]] = false;
		keys[touch_right[i]] = false;
	}

	for (uint32_t i = 0; i < hidTouchCount(); ++i) {
		touchPosition pos;
		hidTouchRead(&pos, i);

		if (pos.px < 160) {
			// shouldn't happen
			if (pos.py < 720) {
				keys[touch_left[pos.py / (720 / 8)]] = true;
			}
		} else if (pos.px >= 1280 - 160) {
			if (pos.py < 720) {
				keys[touch_right[pos.py / (720 / 8)]] = true;
			}
		}
	}
}

void NxUi::UpdateDisplay() {
	uint32_t w, h;
	uint8_t* fb = gfxGetFramebuffer(&w, &h);

	const DynamicFormat format(
		32,
		0x000000FF,
		0x0000FF00,
		0x00FF0000,
		0xFF000000,
		PF::NoAlpha);
	BitmapRef framebuffer = Bitmap::Create(fb, w, h, w * 4, format);

	if (update_ui) {
		if (ui_mode == 0) {
			// Touch-Ui + 4:3 output
			// Blit touch ui once
			framebuffer->Blit(0, 0, *touch_ui, framebuffer->GetRect(), Opacity::opaque);
		} else if (ui_mode == 1) {
			// Delete whole screen once
			framebuffer->Clear();
		}
		update_ui = false;
	}

	Rect dst_rect(160, 0, 960, 720);
	if (ui_mode == 2) {
		// Render stretched without touch ui
		dst_rect = framebuffer->GetRect();
	}
	framebuffer->StretchBlit(dst_rect, *main_surface, main_surface->GetRect(), Opacity::opaque);

	gfxFlushBuffers();
	gfxSwapBuffers();
	gfxWaitForVsync();
}

void NxUi::BeginScreenCapture() {
	CleanDisplay();
}

BitmapRef NxUi::EndScreenCapture() {
	return Bitmap::Create(*main_surface, main_surface->GetRect());
}

void NxUi::SetTitle(const std::string& /* title */) {
	// no-op
}

bool NxUi::ShowCursor(bool /* flag */) {
	return true;
}

#endif
