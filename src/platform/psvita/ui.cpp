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
#include "color.h"
#include "game_config.h"
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
#include <psp2/touch.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/clib.h>
#include <vita2d.h>
#include <cstring>
#include <stdio.h>

#ifdef SUPPORT_AUDIO
#include "audio.h"
AudioInterface& Psp2Ui::GetAudio() {
	return *audio_;
}
#endif

namespace {
	vita2d_texture* gpu_texture;
	vita2d_texture* next_texture;
	vita2d_texture* main_texture;
	vita2d_texture* touch_texture;
	bool is_pstv;
	SceUID GPU_Thread;
	SceUID GPU_Mutex, GPU_Cleanup_Mutex;

	const int touch_left[] = {
		Input::Keys::N1, Input::Keys::N2, Input::Keys::N3, Input::Keys::N4,
		Input::Keys::N5, Input::Keys::N6, Input::Keys::N7, Input::Keys::N8
	};

	const int touch_right[] = {
		Input::Keys::JOY_TOUCH, Input::Keys::N9, Input::Keys::N0,
		Input::Keys::KP_PERIOD, Input::Keys::KP_ADD, Input::Keys::KP_SUBTRACT,
		Input::Keys::KP_MULTIPLY, Input::Keys::KP_DIVIDE
	};

	constexpr int SCREEN_WIDTH = 960;
	constexpr int SCREEN_HEIGHT = 544;

	constexpr int touch_buttons_width = 117;
	constexpr int touch_buttons_height = SCREEN_HEIGHT/8;
	constexpr int touch_buttons_right_x = SCREEN_WIDTH - touch_buttons_width;
	std::vector<bool> touched_buttons(16, false);

	Game_ConfigVideo* vcfg_ref;
}

static int renderThread(unsigned int args, void* arg){
	for (;;){

		sceKernelWaitSema(GPU_Mutex, 1, nullptr);
		memcpy(vita2d_texture_get_datap(gpu_texture),
			vita2d_texture_get_datap(next_texture),
			vita2d_texture_get_stride(gpu_texture)*240);
		sceKernelSignalSema(GPU_Mutex, 1);

		sceKernelWaitSema(GPU_Cleanup_Mutex, 1, nullptr);

		if (main_texture == nullptr) sceKernelExitDeleteThread(0); // Exit procedure

		vita2d_start_drawing();

		vita2d_clear_screen();
		if (!is_pstv && touch_texture && vcfg_ref->touch_ui.Get() && !vcfg_ref->touch_ui.IsLocked()) {
			vita2d_draw_texture(touch_texture, 0, 0);
		}

		if (vcfg_ref->scaling_mode.Get() == ScalingMode::Nearest) {
			if (!vcfg_ref->stretch.Get()) {
				// 725x544 (scaled)
				vita2d_draw_texture_scale(gpu_texture, 117, 0, 2.266, 2.266);
			} else {
				// 960x544 (full-stretched)
				vita2d_draw_texture_scale(gpu_texture, 0, 0, 3, 2.266);
			}
		} else if (vcfg_ref->scaling_mode.Get() == ScalingMode::Integer) {
			if (!vcfg_ref->stretch.Get()) {
				// 640x480 (doubled)
				vita2d_draw_texture_scale(gpu_texture, 160, 32, 2.0, 2.0);
			} else {
				// 960x480 (doubled & stretched)
				vita2d_draw_texture_scale(gpu_texture, 0, 32, 3.0, 2.0);
			}
		}

		// draw touched keys
		if (!is_pstv && touch_texture && !vcfg_ref->stretch.Get()) {
			for (int i = 0; i < 16; ++i) {
				if (touched_buttons[i]) {
					vita2d_draw_rectangle(i < 8 ? 0 : touch_buttons_right_x,
						touch_buttons_height*(i < 8 ? i : i - 8), touch_buttons_width,
						touch_buttons_height, RGBA8(200, 200, 200, 128));
				}
			}
		}

		vita2d_end_drawing();
		vita2d_wait_rendering_done();
		vita2d_swap_buffers();
		sceKernelSignalSema(GPU_Cleanup_Mutex, 1);
	}
}

Psp2Ui::Psp2Ui(int width, int height, const Game_Config& cfg) : BaseUi(cfg)
{
	vcfg_ref = &vcfg;

	SetIsFullscreen(true);

	is_pstv = sceKernelIsPSVitaTV();
	vita2d_init();
	vita2d_set_vblank_wait(cfg.video.vsync.Get());
	gpu_texture = vita2d_create_empty_texture_format(width, height,
		SCE_GXM_TEXTURE_FORMAT_A8B8G8R8);
	vita2d_texture_set_alloc_memblock_type(SCE_KERNEL_MEMBLOCK_TYPE_USER_RW);
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
	main_texture = vita2d_create_empty_texture_format(width, height,
		SCE_GXM_TEXTURE_FORMAT_A8B8G8R8);
	next_texture = vita2d_create_empty_texture_format(width, height,
		SCE_GXM_TEXTURE_FORMAT_A8B8G8R8);
	touch_texture = vita2d_load_PNG_file("app0:/touch_ui.png");
	Bitmap::SetFormat(Bitmap::ChooseFormat(format));
	main_surface = Bitmap::Create(vita2d_texture_get_datap(main_texture),
		width, height, vita2d_texture_get_stride(main_texture), format);

	#ifdef SUPPORT_AUDIO
	audio_ = std::make_unique<Psp2Audio>(cfg.audio);
	#endif

	scePowerSetArmClockFrequency(444);
	scePowerSetBusClockFrequency(222);
	scePowerSetGpuClockFrequency(222);
	scePowerSetGpuXbarClockFrequency(222);

	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
	sceCtrlSetSamplingModeExt(SCE_CTRL_MODE_ANALOG_WIDE);
	if (!is_pstv) {
		sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
	}

	GPU_Mutex = sceKernelCreateSema("GPU Mutex", 0, 1, 1, nullptr);
	GPU_Cleanup_Mutex = sceKernelCreateSema("GPU Cleanup Mutex", 0, 1, 1, nullptr);
	GPU_Thread = sceKernelCreateThread("GPU Thread", &renderThread,
		0x10000100, 0x10000, 0, 0, nullptr);
	sceKernelStartThread(GPU_Thread, 0, nullptr);

	// shiny green bar for ds4 controllers
	if (is_pstv) {
		sceCtrlSetLightBar(1, 0x54, 0x92, 0x36);
	}

	vcfg.touch_ui.SetLocked(vcfg.stretch.Get());
}

Psp2Ui::~Psp2Ui() {
	sceKernelWaitSema(GPU_Cleanup_Mutex, 1, nullptr);
	vita2d_free_texture(main_texture);
	main_texture = nullptr;
	sceKernelSignalSema(GPU_Cleanup_Mutex, 1);
	sceKernelWaitThreadEnd(GPU_Thread, nullptr, nullptr);
	vita2d_free_texture(next_texture);
	vita2d_free_texture(gpu_texture);
	vita2d_free_texture(touch_texture);
	sceKernelDeleteSema(GPU_Mutex);
	sceKernelDeleteSema(GPU_Cleanup_Mutex);
	vita2d_fini();
}

void Psp2Ui::ProcessEvents() {
	SceCtrlData input;
	SceTouchData touch;

	// reset touch keys
	for (int i = 0; i < 8; ++i) {
		keys[touch_left[i]] = false;
		keys[touch_right[i]] = false;
		touched_buttons[i] = false;
		touched_buttons[i+8] = false;
	}

	sceCtrlPeekBufferPositiveExt2(is_pstv ? 1 : 0, &input, 1);
	keys[Input::Keys::JOY_A] = (input.buttons & SCE_CTRL_CIRCLE);
	keys[Input::Keys::JOY_B] = (input.buttons & SCE_CTRL_CROSS);
	keys[Input::Keys::JOY_X] = (input.buttons & SCE_CTRL_TRIANGLE);
	keys[Input::Keys::JOY_Y] = (input.buttons & SCE_CTRL_SQUARE);
	keys[Input::Keys::JOY_BACK] = (input.buttons & SCE_CTRL_SELECT);
	keys[Input::Keys::JOY_START] = (input.buttons & SCE_CTRL_START);
	keys[Input::Keys::JOY_DPAD_RIGHT] = (input.buttons & SCE_CTRL_RIGHT);
	keys[Input::Keys::JOY_DPAD_LEFT] = (input.buttons & SCE_CTRL_LEFT);
	keys[Input::Keys::JOY_DPAD_UP] = (input.buttons & SCE_CTRL_UP);
	keys[Input::Keys::JOY_DPAD_DOWN] = (input.buttons & SCE_CTRL_DOWN);
	keys[Input::Keys::JOY_SHOULDER_LEFT] = (input.buttons & SCE_CTRL_L1);
	keys[Input::Keys::JOY_SHOULDER_RIGHT] = (input.buttons & SCE_CTRL_R1);

	// Analog support
	auto normalize = [](int value) {
		return static_cast<float>(value - 127) / 128.f;
	};

	analog_input.primary.x = normalize(input.lx);
	analog_input.primary.y = normalize(input.ly);
	analog_input.secondary.x = normalize(input.rx);
	analog_input.secondary.y = normalize(input.ry);

	// Touchpad support
	if (!vcfg.touch_ui.IsLocked() && vcfg.touch_ui.Get() && !is_pstv) {
		sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1);
		for (int i = 0; i < touch.reportNum; ++i) {
			int xpos = touch.report[i].x / 2;
			int ypos = touch.report[i].y / 2;
			if (ypos < SCREEN_HEIGHT) {
				int btn = ypos / touch_buttons_height;
				if (xpos < touch_buttons_width) {
					keys[touch_left[btn]] = true;
					touched_buttons[btn] = true;
				} else if (xpos >= touch_buttons_right_x) {
					keys[touch_right[btn]] = true;
					touched_buttons[btn+8] = true;
				}
			}
		}
	}
}

void Psp2Ui::UpdateDisplay() {
	sceKernelWaitSema(GPU_Mutex, 1, nullptr);
	memcpy(vita2d_texture_get_datap(next_texture),
		vita2d_texture_get_datap(main_texture),
		vita2d_texture_get_stride(main_texture)*240);
	sceKernelSignalSema(GPU_Mutex, 1);
}

bool Psp2Ui::LogMessage(const std::string &message) {
	// HLE in psp2shell
	sceClibPrintf("[%s] %s\n", GAME_TITLE, message.c_str());

	// skip useless stderr output
	return true;
}

void Psp2Ui::SetScalingMode(ScalingMode mode) {
	vcfg.scaling_mode.Set(mode);
}

void Psp2Ui::ToggleStretch() {
	vcfg.stretch.Toggle();
	vcfg.touch_ui.SetLocked(vcfg.stretch.Get());
}

void Psp2Ui::ToggleTouchUi() {
	vcfg.touch_ui.Toggle();
}

void Psp2Ui::ToggleVsync() {
	vcfg.vsync.Toggle();
	vita2d_set_vblank_wait(vcfg.vsync.Get());
}

void Psp2Ui::vGetConfig(Game_ConfigVideo& cfg) const {
	cfg.renderer.Lock("Vita (Software)");

	cfg.vsync.SetOptionVisible(true);
	cfg.fps_limit.SetOptionVisible(true);
	cfg.scaling_mode.SetOptionVisible(true);
	cfg.scaling_mode.RemoveFromValidSet(ScalingMode::Bilinear);
	cfg.stretch.SetOptionVisible(true);
	cfg.touch_ui.SetOptionVisible(!is_pstv);

	if (cfg.stretch.Get()) {
		cfg.touch_ui.SetLocked(true);
		cfg.touch_ui.SetDescription("Not available when stretch is enabled");
	}
}
