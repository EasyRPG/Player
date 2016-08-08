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

#ifdef PSP2

// Headers
#include "psp2_ui.h"
#include "color.h"
#include "graphics.h"
#include "keys.h"
#include "output.h"
#include "player.h"
#include "bitmap.h"
#include <iostream>
#include <psp2/rtc.h>
#include <psp2/ctrl.h>
#include <psp2/types.h>
#include <psp2/display.h>
#include <psp2/gxm.h>
#include <psp2/power.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <cstring>
#include <stdio.h>

#ifdef SUPPORT_AUDIO
#include "audio_psp2.h"
AudioInterface& Psp2Ui::GetAudio() {
	return *audio_;
}
#endif

int _newlib_heap_size_user = 192 * 1024 * 1024;

Psp2Ui::Psp2Ui(int width, int height) :
	BaseUi() {
	
	starttick = sceKernelGetProcessTimeWide() / 1000;
	frame = 0;
	zoom_state = 0;
	trigger_state = false;
	vita2d_init();
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
	main_texture = vita2d_create_empty_texture_format(
												width, height,
												SCE_GXM_TEXTURE_FORMAT_A8B8G8R8);
	Bitmap::SetFormat(Bitmap::ChooseFormat(format));
	main_surface = Bitmap::Create(vita2d_texture_get_datap(main_texture),width, height, vita2d_texture_get_stride(main_texture), format);
	
	#ifdef SUPPORT_AUDIO
		audio_.reset(new Psp2Audio());
	#endif
	
	scePowerSetArmClockFrequency(444);
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
	
}

Psp2Ui::~Psp2Ui() {
	vita2d_free_texture(main_texture);
	vita2d_fini();
}

void Psp2Ui::Sleep(uint32_t time) {
	uint64_t usecs = time*1000;
	sceKernelDelayThread(usecs);
}

uint32_t Psp2Ui::GetTicks() const {
	return (sceKernelGetProcessTimeWide() / 1000 - starttick);
}
void Psp2Ui::BeginDisplayModeChange() {
	// no-op
}

void Psp2Ui::EndDisplayModeChange() {
	// no-op
}

void Psp2Ui::Resize(long /*width*/, long /*height*/) {
	// no-op
}

void Psp2Ui::ToggleFullscreen() {
	// no-op
}

void Psp2Ui::ToggleZoom() {
	// no-op
}

bool Psp2Ui::IsFullscreen() {
	return true;
}

void Psp2Ui::ProcessEvents() {
	
	SceCtrlData input;
	sceCtrlPeekBufferPositive(0, &input, 1);
	keys[Input::Keys::Z] = (input.buttons & SCE_CTRL_CROSS);
	keys[Input::Keys::X] = (input.buttons & SCE_CTRL_CIRCLE);
	keys[Input::Keys::N8] = (input.buttons & SCE_CTRL_TRIANGLE);
	keys[Input::Keys::LSHIFT] = (input.buttons & SCE_CTRL_SQUARE);
	keys[Input::Keys::F12] = (input.buttons & SCE_CTRL_SELECT);
	keys[Input::Keys::ESCAPE] = (input.buttons & SCE_CTRL_START);
	keys[Input::Keys::RIGHT] = (input.buttons & SCE_CTRL_RIGHT);
	keys[Input::Keys::LEFT] = (input.buttons & SCE_CTRL_LEFT);
	keys[Input::Keys::UP] = (input.buttons & SCE_CTRL_UP);
	keys[Input::Keys::DOWN] = (input.buttons & SCE_CTRL_DOWN);
	keys[Input::Keys::F2] = (input.buttons & SCE_CTRL_LTRIGGER);
	
	//Resolution changing support
	bool old_state = trigger_state;
	trigger_state = (input.buttons & SCE_CTRL_RTRIGGER);
	if ((trigger_state != old_state) && trigger_state) zoom_state = ((zoom_state + 1) % 3);
	
	// Left analog support
	keys[Input::Keys::JOY_AXIS_X_LEFT] = (input.lx < 50);
	keys[Input::Keys::JOY_AXIS_X_RIGHT] = (input.lx > 170);
	keys[Input::Keys::JOY_AXIS_Y_DOWN] = (input.ly > 170);
	keys[Input::Keys::JOY_AXIS_Y_UP] = (input.ly < 50);
	
	// Right analog support for extra buttons
	keys[Input::Keys::N1] = (input.ry > 170);
	keys[Input::Keys::N3] = (input.ry < 50);
	keys[Input::Keys::N5] = (input.rx > 170);
	keys[Input::Keys::N9] = (input.rx < 50);
	
}

void Psp2Ui::UpdateDisplay() {
	vita2d_start_drawing();
	vita2d_clear_screen();
	switch (zoom_state){
		case 0: // 640x480
			vita2d_draw_texture_scale(main_texture, 160, 32, 2.0, 2.0);
			break;
		case 1: // 725x544
			vita2d_draw_texture_scale(main_texture, 117, 0, 2.266, 2.266);
			break;
		case 2: // 960x544
			vita2d_draw_texture_scale(main_texture, 0, 0, 3, 2.266);
			break;
	}
	vita2d_end_drawing();
	vita2d_wait_rendering_done();
	vita2d_swap_buffers();
}

void Psp2Ui::BeginScreenCapture() {
	CleanDisplay();
}

BitmapRef Psp2Ui::EndScreenCapture() {
	return Bitmap::Create(*main_surface, main_surface->GetRect());
}

void Psp2Ui::SetTitle(const std::string& /* title */) {
	// no-op
}

bool Psp2Ui::ShowCursor(bool /* flag */) {
	return true;
}

#endif
