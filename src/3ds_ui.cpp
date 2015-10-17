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

#include "system.h"
#ifdef _3DS

// Headers
#include "3ds_ui.h"
#include "color.h"
#include "graphics.h"
#include "keys.h"
#include "output.h"
#include "player.h"
#include "bitmap.h"
#include <iostream>

#include <3ds.h>
#include <sf2d.h>
#include <cstring>

#ifdef SUPPORT_AUDIO
AudioInterface& CtrUi::GetAudio() {
	return *audio_;
}
#endif

CtrUi::CtrUi(int width, int height) :
	BaseUi() {
	frame = 0;
	sf2d_init();
	consoleInit(GFX_BOTTOM, NULL);
	// sf2d_set_clear_color(RGBA8(0x40, 0x40, 0x40, 0xFF));
	current_display_mode.width = width;
	current_display_mode.height = height;
	current_display_mode.bpp = 32;
	const DynamicFormat format(
		32,
		0x000000FF,
		0x0000FF00,
		0x00FF0000,
		0xFF000000,
		PF::Alpha);
	Bitmap::SetFormat(format);
	main_surface = Bitmap::Create(width, height, false, 32);
}

CtrUi::~CtrUi() {
	sf2d_fini();
}

#define TICKS_PER_MSEC 268123.480
static inline double u64_to_double(u64 value) {
	return (((double)(u32)(value >> 32))*0x100000000ULL+(u32)value);
}

uint32_t CtrUi::GetTicks() const {
	double ticks = u64_to_double(svcGetSystemTick());
	u64 usecs = (u64)(ticks/TICKS_PER_MSEC);
	return usecs;
}

void CtrUi::Sleep(uint32_t time) {
	//no-op
}

void CtrUi::BeginDisplayModeChange() {
	// no-op
}

void CtrUi::EndDisplayModeChange() {
	// no-op
}

void CtrUi::Resize(long /*width*/, long /*height*/) {
	// no-op
}

void CtrUi::ToggleFullscreen() {
	// no-op
}

void CtrUi::ToggleZoom() {
	// no-op
}

bool CtrUi::IsFullscreen() {
	return true;
}

void CtrUi::ProcessEvents() {
	hidScanInput();

	keys[Input::Keys::Z] = false;
	keys[Input::Keys::X] = false;
	keys[Input::Keys::N8] = false;
	keys[Input::Keys::F12] = false;
	keys[Input::Keys::RIGHT] = false;
	keys[Input::Keys::LEFT] = false;
	keys[Input::Keys::UP] = false;
	keys[Input::Keys::DOWN] = false;
	keys[Input::Keys::F2] = false;

	uint32_t kDown = hidKeysDown();
	if (kDown & KEY_A) keys[Input::Keys::Z] = true;
	if (kDown & KEY_B) keys[Input::Keys::X] = true;
	if (kDown & KEY_X) keys[Input::Keys::N8] = true;
	if (kDown & KEY_SELECT) keys[Input::Keys::F12] = true;
	if (kDown & KEY_START) Player::exit_flag = true;
	if (kDown & KEY_DRIGHT) keys[Input::Keys::RIGHT] = true;
	if (kDown & KEY_DLEFT) keys[Input::Keys::LEFT] = true;
	if (kDown & KEY_DUP) keys[Input::Keys::UP] = true;
	if (kDown & KEY_DDOWN) keys[Input::Keys::DOWN] = true;
	if (kDown & KEY_L) keys[Input::Keys::F2] = true;
}

void CtrUi::UpdateDisplay() {
	// There's also sf2d_fill_texture, but it shows garbage after the second frame in citra.
	main_texture = sf2d_create_texture_mem_RGBA8(main_surface->pixels(),
	                                             main_surface->GetWidth(), main_surface->GetHeight(), 
	                                             TEXFMT_RGBA8, SF2D_PLACE_RAM);

	sf2d_start_frame(GFX_TOP, GFX_LEFT);
		sf2d_draw_texture(main_texture, 40, 0);
	sf2d_end_frame();
	sf2d_free_texture(main_texture);
	sf2d_swapbuffers();
}

void CtrUi::BeginScreenCapture() {
	CleanDisplay();
}

BitmapRef CtrUi::EndScreenCapture() {
	return Bitmap::Create(*main_surface, main_surface->GetRect());
}

void CtrUi::SetTitle(const std::string& /* title */) {
	// no-op
}

bool CtrUi::ShowCursor(bool /* flag */) {
	return true;
}

#endif
