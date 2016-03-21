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
#include <stdio.h>

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
	#ifndef NO_DEBUG
	printf("Debug console started...\n");
	#endif
	sf2d_set_clear_color(RGBA8(0x40, 0x40, 0x40, 0xFF));
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
	main_surface = Bitmap::Create(width, height, false, 32);
	main_texture = sf2d_create_texture_mem_RGBA8(main_surface->pixels(),
	                                             main_surface->GetWidth(), main_surface->GetHeight(), 
	                                             TEXFMT_RGBA8, SF2D_PLACE_RAM);
												 
	#ifdef SUPPORT_AUDIO
		audio_.reset(new CtrAudio());
	#endif
}

CtrUi::~CtrUi() {
	sf2d_free_texture(main_texture);
	sf2d_fini();
}

#define TICKS_PER_MSEC 268123.480
static inline double u64_to_double(u64 value) {
	return (((double)(u32)(value >> 32))*0x100000000ULL+(u32)value);
}

void CtrUi::Sleep(uint32_t time) {
	u64 nsecs = time*1000000;
	svcSleepThread(nsecs);
}

uint32_t CtrUi::GetTicks() const {
	double ticks = u64_to_double(svcGetSystemTick());
	u64 usecs = (u64)(ticks/TICKS_PER_MSEC);
	return usecs;
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
	u32 input = hidKeysHeld();
	keys[Input::Keys::Z] = (input & KEY_A);
	keys[Input::Keys::X] = (input & KEY_B);
	keys[Input::Keys::N8] = (input & KEY_X);
	keys[Input::Keys::F12] = (input & KEY_SELECT);
	Player::exit_flag = (input & KEY_START);
	keys[Input::Keys::RIGHT] = (input & KEY_DRIGHT);
	keys[Input::Keys::LEFT] = (input & KEY_DLEFT);
	keys[Input::Keys::UP] = (input & KEY_DUP);
	keys[Input::Keys::DOWN] = (input & KEY_DDOWN);
	keys[Input::Keys::F2] = (input & KEY_L);
	
	//CirclePad support
	circlePosition circlepad;
	hidCircleRead(&circlepad);
	
	if (circlepad.dy > 25) keys[Input::Keys::UP] = true;
	else if (circlepad.dy < -25) keys[Input::Keys::DOWN] = true;
	else if (circlepad.dx > 25) keys[Input::Keys::RIGHT] = true;
	else if (circlepad.dx < -25) keys[Input::Keys::LEFT] = true;
	
	
}

void CtrUi::UpdateDisplay() {
	main_texture->tiled = 0;
	sf2d_fill_texture_from_RGBA8(main_texture, main_surface->pixels(),
	                                             main_surface->GetWidth(), main_surface->GetHeight()
	                            );
	sf2d_start_frame(GFX_TOP, GFX_LEFT);
	sf2d_draw_texture(main_texture, 40, 0);
	sf2d_end_frame();
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
