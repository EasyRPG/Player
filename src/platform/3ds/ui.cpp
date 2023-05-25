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
#include "battery_t3x.h"

#ifdef SUPPORT_AUDIO
#include "audio.h"
AudioInterface& CtrUi::GetAudio() {
	return *audio_;
}
#endif

#ifndef _DEBUG
using namespace std::chrono_literals;
#endif

namespace {
	Point touch_pos;
	enum class screen_state { off, keep, touched, refresh };
	screen_state bottom_state = screen_state::refresh;
	Tex3DS_SubTexture subt3x;
	constexpr int button_width = 80;
	constexpr int button_height = 60;
	constexpr int width_pow2 = 512;
	constexpr int height_pow2 = 256;
	constexpr int z = 0.5f;
	u32* screen_buffer;
	BitmapRef screen_surface; // contains screen_surface, linear heap allocated
	aptHookCookie cookie;
#ifndef _DEBUG
	struct _batt {
		u8 level = 0;
		bool adapter = false;
		C2D_SpriteSheet sheet;
		C2D_Image image;
		bool warned = false;
	};
	struct _batt battery;
	Game_Clock::time_point info_tick;
#endif
}

static void _aptHook(APT_HookType hook, void*) {
	switch (hook) {
		case APTHOOK_ONSUSPEND:
			Output::Debug("app is going to suspend.");
			Player::Pause();
			break;
		case APTHOOK_ONRESTORE:
			Output::Debug("app has been restored.");
			Player::Resume();
			break;
		case APTHOOK_ONSLEEP:
			Output::Debug("3ds is going to sleep.");
			Player::Pause();
			break;
		case APTHOOK_ONWAKEUP:
			Output::Debug("3ds is waking up.");
			Player::Resume();
			break;
		case APTHOOK_ONEXIT:
			Output::Debug("3ds is going to home menu...");
			Player::Exit();
			break;
		default:
			Output::Debug("Unhandled 3ds hook.");
			break;
	}
}

void CtrUi::ToggleBottomScreen(bool state) {
	// no need to access service
	if ((state && bottom_state != screen_state::off) ||
		(!state && bottom_state == screen_state::off))
		return;

	if (R_SUCCEEDED(gspLcdInit())) {
		if (state) {
			GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_BOTTOM);
			bottom_state = screen_state::refresh;
			Output::Debug("Turned Backlight on.");
		} else {
			GSPLCD_PowerOffBacklight(GSPLCD_SCREEN_BOTTOM);
			bottom_state = screen_state::off;
			Output::Debug("Turned Backlight off.");
		}
		gspLcdExit();
	} else {
		Output::Warning("Could not turn Backlight state.");
	}
}

CtrUi::CtrUi(int width, int height, const Game_Config& cfg) : BaseUi(cfg)
{
	SetIsFullscreen(true);
	aptHook(&cookie, _aptHook, 0);
	ptmuInit();

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	top_screen = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

	current_display_mode.width = width;
	current_display_mode.height = height;
	current_display_mode.bpp = 32;

	const auto format = format_B8G8R8A8_n().format();
	Bitmap::SetFormat(Bitmap::ChooseFormat(format));
	main_surface = Bitmap::Create(width, height, Color(0, 0, 0, 255));

	const auto screen_format = format_A8B8G8R8_n().format();
	screen_buffer = (u32*)linearAlloc((width_pow2*height_pow2*4));
	screen_surface = Bitmap::Create(screen_buffer, width, height, width_pow2*4, screen_format);

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

	if (vcfg.stretch.Get()) {
		C3D_TexSetFilter(top_image.tex, GPU_LINEAR, GPU_LINEAR);
	} else {
		C3D_TexSetFilter(top_image.tex, GPU_NEAREST, GPU_NEAREST);
	}

#ifdef SUPPORT_AUDIO
	audio_ = std::make_unique<CtrAudio>(cfg.audio);
#endif

#ifndef _DEBUG
	bottom_screen = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	C3D_Tex* keyb_tex = (C3D_Tex*)malloc(sizeof(C3D_Tex));
	Tex3DS_Texture keyb_t3x = Tex3DS_TextureImport(keyboard_t3x, keyboard_t3x_size, keyb_tex, nullptr, false);
	Tex3DS_TextureFree(keyb_t3x);
	bottom_image.tex = keyb_tex;
	bottom_image.subtex = &subt3x;

	battery.sheet = C2D_SpriteSheetLoadFromMem(battery_t3x, battery_t3x_size);
	battery.image = C2D_SpriteSheetGetImage(battery.sheet, 0);

	ToggleBottomScreen(vcfg.touch_ui.Get());

	// force refresh
	info_tick = Game_Clock::now() - 10s;
#endif
}

CtrUi::~CtrUi() {
	C3D_TexDelete(top_image.tex);
	free(top_image.tex);

#ifndef _DEBUG
	C3D_TexDelete(bottom_image.tex);
	free(bottom_image.tex);

	C2D_SpriteSheetFree(battery.sheet);
#endif

	C2D_Fini();
	C3D_Fini();

	ptmuExit();
	aptUnhook(&cookie);

	// Always turn bottom screen on
	ToggleBottomScreen(true);
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
	analog_input.primary.y = -normalize(circlepad.dy);
#endif

#ifndef _DEBUG
	// Touchscreen support
	u32 keys_tbl[16] = {
		Input::Keys::N7, Input::Keys::N8, Input::Keys::N9, Input::Keys::NONE,
		Input::Keys::N4, Input::Keys::N5, Input::Keys::N6, Input::Keys::KP_DIVIDE,
		Input::Keys::N1, Input::Keys::N2, Input::Keys::N3, Input::Keys::KP_MULTIPLY,
		Input::Keys::N0, Input::Keys::KP_PERIOD, Input::Keys::KP_ADD, Input::Keys::KP_SUBTRACT
	};

	for (int i = 0; i < 16; i++)
		keys[keys_tbl[i]] = false;

	if (input & KEY_TOUCH) {
		if(bottom_state != screen_state::off) {
			bottom_state = screen_state::touched;
			touchPosition pos;
			hidTouchRead(&pos);
			u8 col = pos.px / button_width;
			u8 row = pos.py / button_height;

			// Turn off touchscreen for top right button
			if (row == 0 && col == 3) {
				ToggleBottomScreen(false);
			} else {
				keys[keys_tbl[col + (row * 4)]] = true;
			}

			touch_pos.x = pos.px;
			touch_pos.y = pos.py;
		} else {
			ToggleBottomScreen(true);
		}
	}

	// info display, query every 10s
	auto t = Game_Clock::now();
	auto s = std::chrono::duration_cast<std::chrono::seconds>(t - info_tick);
	if (s > 10s) {
		info_tick = t;

		struct _batt old = battery;
		PTMU_GetAdapterState(&battery.adapter);
		PTMU_GetBatteryLevel(&battery.level); // 0-5

		if (!battery.warned && !battery.adapter && !battery.level) {
			Output::Warning("Your 3DS may run out of battery soon.");
			battery.warned = true;
		}

		if (battery.adapter) {
			battery.warned = false;
			battery.image = C2D_SpriteSheetGetImage(battery.sheet, 6);
		} else {
			battery.image = C2D_SpriteSheetGetImage(battery.sheet, battery.level);
		}

		// only refresh for battery, when something changed
		if ((old.adapter != battery.adapter
			|| old.level != battery.level) &&
			bottom_state == screen_state::keep) {
			bottom_state = screen_state::refresh;
		}
	}
#endif
}

void CtrUi::UpdateDisplay() {
	// convert ARGB buffer to RGBA buffer
	// required because pixman has no fast-paths for non AXXX buffers and 3DS wants RGBA
	screen_surface->BlitFast(0, 0, *main_surface, main_surface->GetRect(), Opacity::Opaque());

	GSPGPU_FlushDataCache(screen_buffer, (width_pow2*height_pow2*4));

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
		(u32*)screen_buffer,
		GX_BUFFER_DIM(width_pow2, height_pow2),
		(u32*)top_image.tex->data,
		GX_BUFFER_DIM(width_pow2, height_pow2),
		flags
	 );

	C3D_FrameBegin(0);

	// top screen
	C2D_SceneBegin(top_screen);
	C2D_TargetClear(top_screen, C2D_Color32f(0, 0, 0, 1));

	if (vcfg.stretch.Get()) {
		C2D_DrawImageAt(top_image, 0, 0, z, nullptr, 1.25f, 1.0f);
	} else {
		C2D_DrawImageAt(top_image, 40, 0, z);
	}

	// bottom screen
#ifndef _DEBUG
	// darkened button with outline
	auto draw_button = [this](Point p, int w, int h) {
		u8 col = p.x / w;
		u8 row = p.y / h;
		int x = col * w;
		int y = row * h;
		int b = 2; // border width
		constexpr u32 white = C2D_Color32f(1, 1, 1, 1);
		constexpr u32 darkgray = C2D_Color32f(0.5f, 0.5f, 0.5f, 1);
		constexpr u32 lightgray = C2D_Color32f(0.8f, 0.8f, 0.8f, 1);
		constexpr u32 transblack = C2D_Color32f(0, 0, 0, 0.3f);

		C2D_DrawRectSolid(x + b, y + b, z, w - b * 2, h - b * 2, transblack); // inner layer
		C2D_DrawRectangle(x, y, z, w, b, darkgray, lightgray, darkgray, lightgray); // top
		C2D_DrawRectangle(x, y, z, b, h, darkgray, darkgray, lightgray, lightgray); // left
		C2D_DrawRectangle(x + w - b, y, z, b, h, lightgray, lightgray, white, white); // right
		C2D_DrawRectangle(x, y + h - b, z, w, b, lightgray, white, lightgray, white); // bottom
	};

	// "circle" cursor
	auto draw_cursor = [this](Point p) {
		constexpr u32 green = C2D_Color32f(0.1f, 0.7f, 0.1f, 0.9f);
		C2D_DrawRectSolid(p.x-1, p.y, z, 3, 1, green);
		C2D_DrawRectSolid(p.x, p.y-1, z, 1, 3, green);
		// real circle, requires state change, so slower
		//C2D_DrawCircleSolid(p.x, p.y, z, 2, green);
	};

	/* More low hanging fruit optimisation: Only refresh the bottom screen
	 * when there is something to draw (a touch happens, battery level changed,
	 * etc) and one frame after
	 */
	if (bottom_state > screen_state::keep) {
		C2D_SceneBegin(bottom_screen);

		C2D_TargetClear(bottom_screen, C2D_Color32f(0, 0, 0, 1));
		C2D_DrawImageAt(bottom_image, 0, 0, z);

		if (bottom_state == screen_state::touched) {
			draw_button(touch_pos, button_width, button_height);
			draw_cursor(touch_pos);

			// Touch finished, do final redraw
			bottom_state = screen_state::refresh;
		} else if (bottom_state == screen_state::refresh) {
			// nothing new anymore, keep framebuffer
			bottom_state = screen_state::keep;
		}

		// image is 41*12, position with 2 pixels border at bottom right
		C2D_DrawImageAt(battery.image, 320-41-2, 240-12-2, z);
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

void CtrUi::ToggleStretch() {
	vcfg.stretch.Toggle();

	if (vcfg.stretch.Get()) {
		C3D_TexSetFilter(top_image.tex, GPU_LINEAR, GPU_LINEAR);
	} else {
		C3D_TexSetFilter(top_image.tex, GPU_NEAREST, GPU_NEAREST);
	}
}

void CtrUi::ToggleTouchUi() {
	ToggleBottomScreen(bottom_state == screen_state::off);
}

void CtrUi::vGetConfig(Game_ConfigVideo& cfg) const {
	cfg.renderer.Lock("3DS Citro (Software)");

	cfg.stretch.SetOptionVisible(true);
	cfg.touch_ui.SetOptionVisible(true);
	cfg.touch_ui.SetName("Backlight");
	cfg.touch_ui.SetDescription("Toggle the backlight of the bottom screen");
	cfg.touch_ui.Set(bottom_state != screen_state::off);
}
