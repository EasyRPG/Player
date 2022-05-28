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

// Headers
#include "ui.h"
#include "color.h"
#include "graphics.h"
#include "keys.h"
#include "output.h"
#include "player.h"
#include "bitmap.h"
#include <iostream>
#include <sys/iosupport.h>

#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>
#include <cstring>
#include <stdio.h>

#include "keyboard_t3x.h"

#ifdef SUPPORT_AUDIO
#include "audio.h"
AudioInterface& CtrUi::GetAudio() {
	return *audio_;
}
#endif

namespace {
	int touch_x, touch_y;
	int touch_state = 0; // 0 not, 1 touched, 2 touched before, wait clear
	Tex3DS_SubTexture subt3x;
	constexpr int button_width = 80;
	constexpr int button_height = 60;
	constexpr int width_pow2 = 512;
	constexpr int height_pow2 = 256;
	u32* main_buffer;
}

CtrUi::CtrUi(int width, int height, const Game_ConfigVideo& cfg) : BaseUi(cfg)
{
	SetIsFullscreen(true);

	show_touchscreen = true;

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	top_screen = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	bottom_screen = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	current_display_mode.width = width;
	current_display_mode.height = height;
	current_display_mode.bpp = 32;

	const DynamicFormat format(
		32,
		0x00FF0000,
		0x0000FF00,
		0x000000FF,
		0xFF000000,
		PF::NoAlpha);
	Bitmap::SetFormat(Bitmap::ChooseFormat(format));

	main_buffer = (u32*)linearAlloc((width_pow2*height_pow2*4));
	main_surface = Bitmap::Create(main_buffer, width, height, width_pow2*4, format);

	// default for both screens
	subt3x.width = width_pow2;
	subt3x.height = height_pow2;
	subt3x.left = 0.0f;
	subt3x.top = 1.0f;
	subt3x.right = 1.0f;
	subt3x.bottom = 0.0f;

	C3D_Tex* tex = (C3D_Tex*)malloc(sizeof(C3D_Tex));
	C3D_TexInit(tex, width_pow2, height_pow2, GPU_RGB8);
	memset(tex->data, 0, tex->size);
	tex->border = 0xFFFFFFFF;
	C3D_TexSetWrap(tex, GPU_CLAMP_TO_BORDER, GPU_CLAMP_TO_BORDER);
	top_image.tex = tex;
	top_image.subtex = &subt3x;

	// FIXME: Add option to settings Ui
	if (cfg.stretch_width.Get()) {
		C3D_TexSetFilter(top_image.tex, GPU_LINEAR, GPU_LINEAR);
	} else {
		C3D_TexSetFilter(top_image.tex, GPU_NEAREST, GPU_NEAREST);
	}

#ifdef SUPPORT_AUDIO
	audio_.reset(new CtrAudio());
#endif

#ifndef _DEBUG
	C3D_Tex* keyb_tex = (C3D_Tex*)malloc(sizeof(C3D_Tex));
	Tex3DS_Texture keyb_t3x = Tex3DS_TextureImport(keyboard_t3x, keyboard_t3x_size, keyb_tex, nullptr, false);
	Tex3DS_TextureFree(keyb_t3x);
	bottom_image.tex = keyb_tex;
	bottom_image.subtex = &subt3x;
#endif
}

CtrUi::~CtrUi() {
	C3D_TexDelete(top_image.tex);
	free(top_image.tex);

#ifndef _DEBUG
	C3D_TexDelete(bottom_image.tex);
	free(bottom_image.tex);
#endif

	C2D_Fini();
	C3D_Fini();
}

void CtrUi::ProcessEvents() {
	if (!aptMainLoop())
		Player::Exit();

	hidScanInput();
	u32 input = hidKeysHeld();
	keys[Input::Keys::JOY_DPAD_UP] = (input & KEY_DUP);
	keys[Input::Keys::JOY_DPAD_DOWN] = (input & KEY_DDOWN);
	keys[Input::Keys::JOY_DPAD_RIGHT] = (input & KEY_DRIGHT);
	keys[Input::Keys::JOY_DPAD_LEFT] = (input & KEY_DLEFT);
	keys[Input::Keys::JOY_A] = (input & KEY_A);
	keys[Input::Keys::JOY_B] = (input & KEY_B);
	keys[Input::Keys::JOY_X] = (input & KEY_X);
	keys[Input::Keys::JOY_Y] = (input & KEY_Y);
	keys[Input::Keys::JOY_SHOULDER_LEFT] = (input & KEY_L);
	keys[Input::Keys::JOY_SHOULDER_RIGHT] = (input & KEY_R);
	keys[Input::Keys::JOY_BACK] = (input & KEY_SELECT);
	keys[Input::Keys::JOY_START] = (input & KEY_START);

#if defined(USE_JOYSTICK_AXIS) && defined(SUPPORT_JOYSTICK_AXIS)
	// CirclePad support
	circlePosition circlepad;
	hidCircleRead(&circlepad);

	auto normalize = [](int value) {
		return static_cast<float>(value) / 0x9C;
	};

	analog_input.primary.x = normalize(circlepad.dx);
	analog_input.primary.y = normalize(circlepad.dy);
#endif

#ifndef _DEBUG
	// Touchscreen support
	u32 keys_tbl[16] = {
		Input::Keys::N7, Input::Keys::N8, Input::Keys::N9, Input::Keys::KP_DIVIDE,
		Input::Keys::N4, Input::Keys::N5, Input::Keys::N6, Input::Keys::KP_DIVIDE,
		Input::Keys::N1, Input::Keys::N2, Input::Keys::N3, Input::Keys::KP_MULTIPLY,
		Input::Keys::N0, Input::Keys::KP_PERIOD, Input::Keys::KP_ADD, Input::Keys::KP_SUBTRACT
	};

	if (touch_state == 1) {
		// Touch finished, do final redraw in UpdateDisplay
		touch_state = 2;
	}

	for (int i = 0; i < 16; i++)
		keys[keys_tbl[i]] = false;

	if (input & KEY_TOUCH) {
		if(show_touchscreen) {
			touch_state = 1;
			touchPosition pos;
			hidTouchRead(&pos);
			u8 col = pos.px / button_width;
			u8 row = pos.py / button_height;

			if (row == 0 && col == 3) { // Turn off touchscreen for top right button
				if (R_SUCCEEDED(gspLcdInit())) {
					show_touchscreen = false;
					GSPLCD_PowerOffBacklight(GSPLCD_SCREEN_BOTTOM);
					gspLcdExit();
				}

			} else {
				keys[keys_tbl[col + (row * 4)]] = true;
			}

			touch_x = pos.px;
			touch_y = pos.py;
		} else {
			if (R_SUCCEEDED(gspLcdInit())) {
				show_touchscreen = true;
				GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_BOTTOM);
				gspLcdExit();
			}
		}
	}
#endif
}

static __attribute__((always_inline, optimize(3))) inline u32 NDS3D_Reverse32(u32 val)
{
	__asm("ROR %0, %1, #24" : "=r" (val) : "r" (val));

	return val;
}

void CtrUi::UpdateDisplay() {
	// rotate ARGB buffer to RGBA buffer
	// required because pixman has no fast-paths for non AXXX buffers
	u32* line = main_buffer;
	for (int i = 0; i <= 240; ++i) {
		for (int j = 0; j <= 320; ++j) {
			u32* val = line + j;
			*val = NDS3D_Reverse32(*val);
		}
		line += width_pow2;
	}

	GSPGPU_FlushDataCache(main_buffer, (width_pow2*height_pow2*4));

	// Using RGB8 as output format is faster and improves framerate ¯\_(ツ)_/¯
	const u32 flags = (GX_TRANSFER_FLIP_VERT(0) |
		GX_TRANSFER_OUT_TILED(1) |
		GX_TRANSFER_RAW_COPY(0) |
		GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) |
		GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) |
		GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO));

	// Doing this after FrameBegin corrupts the output, probably because this
	// is asynchronous and FrameBegin will block until it finishes
	C3D_SyncDisplayTransfer(
		(u32*)main_buffer,
		GX_BUFFER_DIM(width_pow2, height_pow2),
		(u32*)top_image.tex->data,
		GX_BUFFER_DIM(width_pow2, height_pow2),
		flags
	 );

	C3D_FrameBegin(0);

	// top screen
	C2D_SceneBegin(top_screen);
	C2D_TargetClear(top_screen, C2D_Color32f(0, 0, 0, 1));

	// FIXME: All video options must be accesible during runtime
	/*if (cfg.stretch_width) {
		C2D_DrawImageAt(top_image, 0, 0, 0.5f, NULL, 1.25f, 1.0f);
	} else {
		C2D_DrawImageAt(top_image, 40, 0, 0.5f, NULL);
	}*/
	C2D_DrawImageAt(top_image, 40, 0, 0.5f, NULL);

#ifndef _DEBUG
	// bottom screen
	C2D_SceneBegin(bottom_screen);

	// More low hanging fruit optimisation:
	// Only refresh the bottom when a touch happens and one frame after
	static bool once = false;
	if (!once || touch_state == 2) {
		C2D_TargetClear(bottom_screen, C2D_Color32f(0, 0, 0, 1));
		C2D_DrawImageAt(bottom_image, 0, 0, 0.5f, NULL);
		touch_state = 0;
		once = true;
	}

	if (touch_state == 1) {
		C2D_TargetClear(bottom_screen, C2D_Color32f(0, 0, 0, 1));
		C2D_DrawImageAt(bottom_image, 0, 0, 0.5f, NULL);

		u32 gray = C2D_Color32f(0.8f, 0.8f, 0.8f, 1);
		u32 white = C2D_Color32f(1, 1, 1, 1);

		// "circle" cursor
		C2D_DrawRectSolid(touch_x - 1, touch_y, 0.5f, 3, 1, gray);
		C2D_DrawRectSolid(touch_x, touch_y - 1, 0.5f, 1, 3, gray);

		// get touched button
		u8 col = touch_x / button_width;
		u8 row = touch_y / button_height;
		u8 pos_x = col * button_width;
		u8 pos_y = row * button_height;

		// darkened button with outline
		C2D_DrawRectSolid(pos_x + 2, pos_y + 2, 0.5f, button_width - 2, button_height - 2, C2D_Color32f(0, 0, 0, 0.2f));
		C2D_DrawRectSolid(pos_x + button_width - 2, pos_y, 0.5f, 2, button_height, white); // right
		C2D_DrawRectSolid(pos_x, pos_y + button_height - 2, 0.5f, button_width, 2, white); // bottom
		C2D_DrawRectSolid(pos_x, pos_y, 0.5f, button_width, 2, gray); // top
		C2D_DrawRectSolid(pos_x, pos_y, 0.5f, 2, button_height, gray); // left
	}
#endif

	C3D_FrameEnd(0);
}

bool CtrUi::LogMessage(const std::string &message) {
	std::string m = std::string("[" GAME_TITLE "] ") + message + "\n";

	// HLE in citra emulator
	svcOutputDebugString(m.c_str(), m.length());

#ifdef _DEBUG
	// log additionally to bottom console
	return false;
#else
	return true;
#endif
}
