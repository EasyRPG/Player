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

#ifdef USE_LIBRETRO

// Headers
#include "libretro_ui.h"
#include "libretro_clock.h"
#include "bitmap.h"
#include "color.h"
#include "graphics.h"
#include "input.h"
#include "keys.h"
#include "main_data.h"
#include "options.h"
#include "output.h"
#include "player.h"
#include "scene.h"
#include "version.h"

#include <cstring>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>
#include <math.h>

#ifdef SUPPORT_AUDIO
#include "libretro_audio.h"
AudioInterface& LibretroUi::GetAudio() {
	return *audio_;
}
#endif

retro_environment_t LibretroUi::environ_cb = nullptr;
retro_input_poll_t LibretroUi::input_poll_cb = nullptr;
bool LibretroUi::player_exit_called = false;

#if defined(USE_KEYBOARD) && defined(SUPPORT_KEYBOARD)
static Input::Keys::InputKey RetroKey2InputKey(int retrokey);
#endif

#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
static Input::Keys::InputKey RetroJKey2InputKey(int button_index);
static int button_remapper[7];
#endif

enum PadInputState {
	PadInputState_RetroPad = 1,
	PadInputState_Keyboard = 2
};

LibretroUi::LibretroUi(int width, int height, const Game_ConfigVideo& cfg) : BaseUi(cfg)
{
	// Handled by libretro
	// FIXME: There is currently no callback from libretro telling us whether or not fullscreen is enabled.
	SetIsFullscreen(false);

	current_display_mode.width = width;
	current_display_mode.height = height;
	current_display_mode.bpp = 32;

	// libretro always owns main loop
	SetFrameRateSynchronized(true);

	const DynamicFormat format(
		32,
		0x00FF0000,
		0x0000FF00,
		0x000000FF,
		0xFF000000,
		PF::NoAlpha);

	Bitmap::SetFormat(Bitmap::ChooseFormat(format));
	main_surface.reset();
	main_surface = Bitmap::Create(current_display_mode.width,
		current_display_mode.height,
		false,
		current_display_mode.bpp
	);

	#ifdef SUPPORT_AUDIO
	audio_.reset(new LibretroAudio());
	#endif

	UpdateVariables();
}

void LibretroUi::ToggleFullscreen() {
	// no-op
}

void LibretroUi::ToggleZoom() {
	// no-op
}

void LibretroUi::UpdateDisplay() {
	if (UpdateWindow == nullptr) {
		return;
	}

	UpdateWindow(main_surface->pixels(), current_display_mode.width, current_display_mode.height, main_surface->pitch());
}

void LibretroUi::SetTitle(const std::string &title){
	// no-op
}

bool LibretroUi::ShowCursor(bool flag) {
	return false;
}

void LibretroUi::ProcessEvents() {
#	if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
	if (CheckInputState == nullptr) {
		return;
	}

	LibretroUi::input_poll_cb();

	bool any_var_changed;
	LibretroUi::environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &any_var_changed);
	if (any_var_changed) {
		UpdateVariables();
	}

	if ((keyboard_retropad_state & PadInputState_RetroPad) == 0) {
		return;
	}

	auto check_pressed = [this] (int button_id) {
		keys[RetroJKey2InputKey(button_id)] = CheckInputState(0, RETRO_DEVICE_JOYPAD, 0, button_id) != 0;
	};

#	if defined(USE_JOYSTICK_HAT) && defined(SUPPORT_JOYSTICK_HAT)
	check_pressed(RETRO_DEVICE_ID_JOYPAD_UP);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_DOWN);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_LEFT);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_RIGHT);
#	endif

	check_pressed(RETRO_DEVICE_ID_JOYPAD_A);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_B);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_Y);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_START);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_SELECT);

	check_pressed(RETRO_DEVICE_ID_JOYPAD_X);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_L);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_R);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_L2);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_R2);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_L3);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_R3);

#	if defined(USE_JOYSTICK_AXIS) && defined(SUPPORT_JOYSTICK_AXIS)
	int16_t axis = CheckInputState(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X);
	if (axis < -JOYSTICK_AXIS_SENSIBILITY) {
		keys[Input::Keys::JOY_AXIS_X_LEFT] = true;
		keys[Input::Keys::JOY_AXIS_X_RIGHT] = false;
	} else if (axis > JOYSTICK_AXIS_SENSIBILITY) {
		keys[Input::Keys::JOY_AXIS_X_LEFT] = false;
		keys[Input::Keys::JOY_AXIS_X_RIGHT] = true;
	} else {
		keys[Input::Keys::JOY_AXIS_X_LEFT] = false;
		keys[Input::Keys::JOY_AXIS_X_RIGHT] = false;
	}

	axis = CheckInputState(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y);
	if (axis < -JOYSTICK_AXIS_SENSIBILITY) {
		keys[Input::Keys::JOY_AXIS_Y_UP] = true;
		keys[Input::Keys::JOY_AXIS_Y_DOWN] = false;
	} else if (axis > JOYSTICK_AXIS_SENSIBILITY) {
		keys[Input::Keys::JOY_AXIS_Y_UP] = false;
		keys[Input::Keys::JOY_AXIS_Y_DOWN] = true;
	} else {
		keys[Input::Keys::JOY_AXIS_Y_UP] = false;
		keys[Input::Keys::JOY_AXIS_Y_DOWN] = false;
	}
#	endif
#	endif
}

retro_video_refresh_t LibretroUi::UpdateWindow = nullptr;
void LibretroUi::SetRetroVideoCallback(retro_video_refresh_t cb) {
	UpdateWindow = cb;
}

retro_input_state_t LibretroUi::CheckInputState = nullptr;
void LibretroUi::SetRetroInputStateCallback(retro_input_state_t cb) {
	CheckInputState = cb;
}

void LibretroUi::UpdateKeyboardCallback(bool down, unsigned keycode) {
	if ((keyboard_retropad_state & PadInputState_Keyboard) == PadInputState_Keyboard) {
		keys[RetroKey2InputKey(keycode)] = down;
	}
}

void LibretroUi::UpdateVariables() {
	static const char* none = "None (See Core Options)";

	static struct retro_variable debug = { "Debug Mode", nullptr };
	static struct retro_variable input = { "Input", nullptr };
	static struct retro_variable variables[] = {
		{ "RetroPad X", nullptr },
		{ "RetroPad L", nullptr },
		{ "RetroPad R", nullptr },
		{ "RetroPad L2", nullptr },
		{ "RetroPad R2", nullptr },
		{ "RetroPad L3", nullptr },
		{ "RetroPad R3", nullptr }
	};

	static const char buttons[][24] = {
		"0", "1", "2", "3", "4",
		"5", "6", "7", "8", "9",
		"+", "-", "*", "/", ".",
		"Open Debug Menu", "Walk through walls"
	};

	LibretroUi::environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &debug);
	Player::debug_flag = strcmp(debug.value, "Enabled") == 0;

	LibretroUi::environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &input);
	if (strcmp(input.value, "Use Both") == 0) {
		keyboard_retropad_state = PadInputState_Keyboard | PadInputState_RetroPad;
	} else if (strcmp(input.value, "Only Keyboard") == 0) {
		keyboard_retropad_state = PadInputState_Keyboard;
	} else if (strcmp(input.value, "Only RetroPad") == 0) {
		keyboard_retropad_state = PadInputState_RetroPad;
	}

	// Button remapping from settings
	for (int i = 0; i < 7; ++i) {
		LibretroUi::environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &variables[i]);
		button_remapper[i] = Input::Keys::NONE;
		for (int j = 0; j < 17; ++j) {
			if (strcmp(variables[i].value, buttons[j]) == 0) {
				button_remapper[i] = Input::Keys::JOY_10 + j;
				break;
			}
		}
		if (button_remapper[i] == Input::Keys::NONE) {
			variables[i].value = none;
		}
	}

	struct retro_input_descriptor desc[] = {
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "Confirm" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "Cancel" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y, "Shift" },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Reset" },

		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X, variables[0].value },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L, variables[1].value },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R, variables[2].value },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2, variables[3].value },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2, variables[4].value },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L3, variables[5].value },
		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R3, variables[6].value },

		{ 0 }
	};

	LibretroUi::environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, &desc);
}

#if defined(USE_KEYBOARD) && defined(SUPPORT_KEYBOARD)
Input::Keys::InputKey RetroKey2InputKey(int retrokey) {
	switch (retrokey) {
		case RETROK_BACKSPACE	: return Input::Keys::BACKSPACE;
		case RETROK_TAB			: return Input::Keys::TAB;
		case RETROK_CLEAR		: return Input::Keys::CLEAR;
		case RETROK_RETURN		: return Input::Keys::RETURN;
		case RETROK_PAUSE		: return Input::Keys::PAUSE;
		case RETROK_ESCAPE		: return Input::Keys::ESCAPE;
		case RETROK_SPACE		: return Input::Keys::SPACE;
		case RETROK_PAGEUP		: return Input::Keys::PGUP;
		case RETROK_PAGEDOWN	: return Input::Keys::PGDN;
		case RETROK_END			: return Input::Keys::ENDS;
		case RETROK_HOME		: return Input::Keys::HOME;
		case RETROK_LEFT		: return Input::Keys::LEFT;
		case RETROK_UP			: return Input::Keys::UP;
		case RETROK_RIGHT		: return Input::Keys::RIGHT;
		case RETROK_DOWN		: return Input::Keys::DOWN;
		case RETROK_PRINT		: return Input::Keys::SNAPSHOT;
		case RETROK_INSERT		: return Input::Keys::INSERT;
		case RETROK_DELETE		: return Input::Keys::DEL;
		case RETROK_LSHIFT		: return Input::Keys::LSHIFT;
		case RETROK_RSHIFT		: return Input::Keys::RSHIFT;
		case RETROK_LCTRL		: return Input::Keys::LCTRL;
		case RETROK_RCTRL		: return Input::Keys::RCTRL;
		case RETROK_LALT		: return Input::Keys::LALT;
		case RETROK_RALT		: return Input::Keys::RALT;
		case RETROK_0			: return Input::Keys::N0;
		case RETROK_1			: return Input::Keys::N1;
		case RETROK_2			: return Input::Keys::N2;
		case RETROK_3			: return Input::Keys::N3;
		case RETROK_4			: return Input::Keys::N4;
		case RETROK_5			: return Input::Keys::N5;
		case RETROK_6			: return Input::Keys::N6;
		case RETROK_7			: return Input::Keys::N7;
		case RETROK_8			: return Input::Keys::N8;
		case RETROK_9			: return Input::Keys::N9;
		case RETROK_a			: return Input::Keys::A;
		case RETROK_b			: return Input::Keys::B;
		case RETROK_c			: return Input::Keys::C;
		case RETROK_d			: return Input::Keys::D;
		case RETROK_e			: return Input::Keys::E;
		case RETROK_f			: return Input::Keys::F;
		case RETROK_g			: return Input::Keys::G;
		case RETROK_h			: return Input::Keys::H;
		case RETROK_i			: return Input::Keys::I;
		case RETROK_j			: return Input::Keys::J;
		case RETROK_k			: return Input::Keys::K;
		case RETROK_l			: return Input::Keys::L;
		case RETROK_m			: return Input::Keys::M;
		case RETROK_n			: return Input::Keys::N;
		case RETROK_o			: return Input::Keys::O;
		case RETROK_p			: return Input::Keys::P;
		case RETROK_q			: return Input::Keys::Q;
		case RETROK_r			: return Input::Keys::R;
		case RETROK_s			: return Input::Keys::S;
		case RETROK_t			: return Input::Keys::T;
		case RETROK_u			: return Input::Keys::U;
		case RETROK_v			: return Input::Keys::V;
		case RETROK_w			: return Input::Keys::W;
		case RETROK_x			: return Input::Keys::X;
		case RETROK_y			: return Input::Keys::Y;
		case RETROK_z			: return Input::Keys::Z;
		case RETROK_MENU		: return Input::Keys::MENU;
		case RETROK_KP0			: return Input::Keys::KP0;
		case RETROK_KP1			: return Input::Keys::KP1;
		case RETROK_KP2			: return Input::Keys::KP2;
		case RETROK_KP3			: return Input::Keys::KP3;
		case RETROK_KP4			: return Input::Keys::KP4;
		case RETROK_KP5			: return Input::Keys::KP5;
		case RETROK_KP6			: return Input::Keys::KP6;
		case RETROK_KP7			: return Input::Keys::KP7;
		case RETROK_KP8			: return Input::Keys::KP8;
		case RETROK_KP9			: return Input::Keys::KP9;
		case RETROK_KP_MULTIPLY	: return Input::Keys::MULTIPLY;
		case RETROK_KP_PLUS		: return Input::Keys::ADD;
		case RETROK_KP_ENTER	: return Input::Keys::RETURN;
		case RETROK_KP_MINUS	: return Input::Keys::SUBTRACT;
		case RETROK_KP_PERIOD	: return Input::Keys::PERIOD;
		case RETROK_KP_DIVIDE	: return Input::Keys::DIVIDE;
		case RETROK_F1			: return Input::Keys::F1;
		case RETROK_F2			: return Input::Keys::F2;
		case RETROK_F3			: return Input::Keys::F3;
		case RETROK_F4			: return Input::Keys::F4;
		case RETROK_F5			: return Input::Keys::F5;
		case RETROK_F6			: return Input::Keys::F6;
		case RETROK_F7			: return Input::Keys::F7;
		case RETROK_F8			: return Input::Keys::F8;
		case RETROK_F9			: return Input::Keys::F9;
		case RETROK_F10			: return Input::Keys::F10;
		case RETROK_F11			: return Input::Keys::F11;
		case RETROK_F12			: return Input::Keys::F12;
		case RETROK_CAPSLOCK	: return Input::Keys::CAPS_LOCK;
		case RETROK_NUMLOCK		: return Input::Keys::NUM_LOCK;
		case RETROK_SCROLLOCK	: return Input::Keys::SCROLL_LOCK;

		default					: return Input::Keys::NONE;
	}
}
#endif

#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
Input::Keys::InputKey RetroJKey2InputKey(int button_index) {
	switch (button_index) {
		case RETRO_DEVICE_ID_JOYPAD_UP		: return Input::Keys::JOY_0;
		case RETRO_DEVICE_ID_JOYPAD_DOWN	: return Input::Keys::JOY_1;
		case RETRO_DEVICE_ID_JOYPAD_LEFT	: return Input::Keys::JOY_2;
		case RETRO_DEVICE_ID_JOYPAD_RIGHT	: return Input::Keys::JOY_3;
		case RETRO_DEVICE_ID_JOYPAD_A		: return Input::Keys::JOY_4;
		case RETRO_DEVICE_ID_JOYPAD_B		: return Input::Keys::JOY_5;
		case RETRO_DEVICE_ID_JOYPAD_Y		: return Input::Keys::JOY_6;
		case RETRO_DEVICE_ID_JOYPAD_START	: return Input::Keys::JOY_7;
		case RETRO_DEVICE_ID_JOYPAD_SELECT	: return Input::Keys::JOY_8;

		case RETRO_DEVICE_ID_JOYPAD_X		: return (Input::Keys::InputKey)button_remapper[0];
		case RETRO_DEVICE_ID_JOYPAD_L		: return (Input::Keys::InputKey)button_remapper[1];
		case RETRO_DEVICE_ID_JOYPAD_R		: return (Input::Keys::InputKey)button_remapper[2];
		case RETRO_DEVICE_ID_JOYPAD_L2		: return (Input::Keys::InputKey)button_remapper[3];
		case RETRO_DEVICE_ID_JOYPAD_R2		: return (Input::Keys::InputKey)button_remapper[4];
		case RETRO_DEVICE_ID_JOYPAD_L3		: return (Input::Keys::InputKey)button_remapper[5];
		case RETRO_DEVICE_ID_JOYPAD_R3		: return (Input::Keys::InputKey)button_remapper[6];

		default : return Input::Keys::NONE;
	}
}
#endif

/* libretro api implementation */
static const unsigned AUDIO_SAMPLERATE = 48000;

RETRO_CALLCONV void retro_time_update(retro_usec_t usec) {
	LibretroClock::time_in_microseconds += usec;
}

RETRO_CALLCONV void retro_write_audio() {
	if (DisplayUi) {
		LibretroAudio::AudioThreadCallback();
	}
}

RETRO_CALLCONV void retro_enable_audio(bool enabled) {
	LibretroAudio::EnableAudio(enabled);
}

RETRO_CALLCONV void retro_keyboard_event(bool down, unsigned keycode, uint32_t, uint16_t) {
	if (DisplayUi) {
		static_cast<LibretroUi*>(DisplayUi.get())->UpdateKeyboardCallback(down, keycode);
	}
}

static void fallback_log(enum retro_log_level level, const char *fmt, ...) {
	(void) level;
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
}

static retro_log_printf_t log_cb = fallback_log;

/* Sets callbacks. retro_set_environment() is guaranteed to be called
 * before retro_init().
 *
 * The rest of the set_* functions are guaranteed to have been called
 * before the first call to retro_run() is made. */
RETRO_API void retro_set_environment(retro_environment_t cb) {
	bool no_content = false;

	static retro_frame_time_callback frame_time_definition = {
		retro_time_update,
		1000000 / Game_Clock::GetTargetGameFps()
	};

	static retro_audio_callback audio_callback_definition = {
		retro_write_audio,
		retro_enable_audio
	};

	static retro_keyboard_callback keyboard_callback_definition = {
		retro_keyboard_event
	};

	static struct retro_log_callback logging;

	LibretroUi::environ_cb = cb;

	cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_content);
	cb(RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK, &audio_callback_definition);
	cb(RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK, &frame_time_definition);
	cb(RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK, &keyboard_callback_definition);

	if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
		log_cb = logging.log;
	else
		log_cb = fallback_log;

#	define EP_RETRO_OPTIONS "None|0|1|2|3|4|5|6|7|8|9|+|-|*|/|.|Open Debug Menu|Walk through walls"

	struct retro_variable variables[] = {
		{ "RetroPad X", "Button mapping of RetroPad X; " EP_RETRO_OPTIONS },
		{ "RetroPad L", "Button mapping of RetroPad L; " EP_RETRO_OPTIONS },
		{ "RetroPad R", "Button mapping of RetroPad R; " EP_RETRO_OPTIONS },
		{ "RetroPad L2", "Button mapping of RetroPad L2; " EP_RETRO_OPTIONS },
		{ "RetroPad R2", "Button mapping of RetroPad R2; " EP_RETRO_OPTIONS },
		{ "RetroPad L3", "Button mapping of RetroPad L3; " EP_RETRO_OPTIONS },
		{ "RetroPad R3", "Button mapping of RetroPad R3; " EP_RETRO_OPTIONS },
		{ "Input", "Keyboard and RetroPad; Use Both|Only Keyboard|Only RetroPad" },
		{ "Debug Mode", "Debug menu and walk through walls; Disabled|Enabled" },
		{ nullptr, nullptr }
	};
	cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);
}

RETRO_API void retro_set_video_refresh(retro_video_refresh_t cb) {
	LibretroUi::SetRetroVideoCallback(cb);
}

RETRO_API void retro_set_audio_sample(retro_audio_sample_t cb) {
	// unused
}

RETRO_API void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) {
	LibretroAudio::SetRetroAudioCallback(cb);
}

RETRO_API void retro_set_input_poll(retro_input_poll_t cb) {
	LibretroUi::input_poll_cb = cb;
}

RETRO_API void retro_set_input_state(retro_input_state_t cb) {
	LibretroUi::SetRetroInputStateCallback(cb);
}

static void init_easy_rpg() {
	Player::exit_flag = false;
	LibretroUi::player_exit_called = false;

	Player::Init(0, nullptr);

	auto buttons = Input::GetDefaultButtonMappings();
	auto directions = Input::GetDefaultDirectionMappings();

	Input::Init(std::move(buttons), std::move(directions), "", "");
}

/* Library global initialization/deinitialization. */
RETRO_API void retro_init() {
	// no-op, handled in retro_load_game
}

RETRO_API void retro_deinit() {
	// no-op, handled in retro_unload_game
}

/* Must return RETRO_API_VERSION. Used to validate ABI compatibility
 * when the API is revised. */
RETRO_API unsigned retro_api_version() {
	return RETRO_API_VERSION;
}

/* Gets statically known system info. Pointers provided in *info
 * must be statically allocated.
 * Can be called at any time, even before retro_init(). */
RETRO_API void retro_get_system_info(struct retro_system_info* info) {
	memset(info, 0, sizeof(*info));
	info->library_name = "EasyRPG Player";
	#ifndef GIT_VERSION
	#define GIT_VERSION ""
	#endif
	info->library_version = PLAYER_VERSION GIT_VERSION;
	info->need_fullpath = true;
	info->valid_extensions = SUFFIX_LDB;
}

/* Gets information about system audio/video timings and geometry.
 * Can be called only after retro_load_game() has successfully completed.
 * NOTE: The implementation of this function might not initialize every
 * variable if needed.
 * E.g. geom.aspect_ratio might not be initialized if core doesn't
 * desire a particular aspect ratio. */
RETRO_API void retro_get_system_av_info(struct retro_system_av_info* info) {
	info->geometry.base_width = SCREEN_TARGET_WIDTH;
	info->geometry.base_height = SCREEN_TARGET_HEIGHT;
	info->geometry.max_width = SCREEN_TARGET_WIDTH;
	info->geometry.max_height = SCREEN_TARGET_HEIGHT;
	info->geometry.aspect_ratio = 0.0f;
	info->timing.fps = Game_Clock::GetTargetGameFps();
	info->timing.sample_rate = AUDIO_SAMPLERATE;
}

/* Sets device to be used for player 'port'.
 * By default, RETRO_DEVICE_JOYPAD is assumed to be plugged into all
 * available ports.
 * Setting a particular device type is not a guarantee that libretro cores
 * will only poll input based on that particular device type. It is only a
 * hint to the libretro core when a core cannot automatically detect the
 * appropriate input device type on its own. It is also relevant when a
 * core can change its behavior depending on device type. */
RETRO_API void retro_set_controller_port_device(unsigned port, unsigned device) {
	// Not used
}

/* Resets the current game. */
RETRO_API void retro_reset(void) {
	Player::reset_flag = true;
}

/* Runs the game for one video frame.
 * During retro_run(), input_poll callback must be called at least once.
 *
 * If a frame is not rendered for reasons where a game "dropped" a frame,
 * this still counts as a frame, and retro_run() should explicitly dupe
 * a frame if GET_CAN_DUPE returns true.
 * In this case, the video callback can take a NULL argument for data.
 */

RETRO_API void retro_run() {
	Player::MainLoop();

	if (!DisplayUi) {
		// Player::Exit was called, send shutdown request to the frontend
		LibretroUi::player_exit_called = true;
		// This closes the whole frontend and not just the core, but there is no API for core unloading...
		LibretroUi::environ_cb(RETRO_ENVIRONMENT_SHUTDOWN, nullptr);
	}
}

static void extract_directory(char *buf, const char *path, size_t size) {
	strncpy(buf, path, size - 1);
	buf[size - 1] = '\0';

	char *base = strrchr(buf, '/');
	if (!base)
		base = strrchr(buf, '\\');

	if (base)
		*base = '\0';
	else
		buf[0] = '\0';
}

/* Loads a game. */
RETRO_API bool retro_load_game(const struct retro_game_info* game) {
	char parent_dir[1024];
	enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;

	if (!game)
		return false;

	if (!LibretroUi::environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt)) {
		log_cb(RETRO_LOG_INFO, "XRGB8888 is not supported.\n");
		return false;
	}

	extract_directory(parent_dir, game->path, sizeof(parent_dir));

	init_easy_rpg();

	log_cb(RETRO_LOG_INFO, "parent dir is: %s\n", parent_dir );

	if (parent_dir[0] != '\0') {
		Main_Data::SetProjectPath(parent_dir);
		Player::Run();
	} else {
		Main_Data::SetProjectPath(".");
		Player::Run();
	}
	return true;
}

/* Unloads a currently loaded game. */
RETRO_API void retro_unload_game() {
	// Workaround a crash on Windows & Android because the callbacks are invoked after the DLL/SO was unloaded
	static retro_audio_callback no_audio_callback_definition = {
		nullptr,
		nullptr
	};
	LibretroUi::environ_cb(RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK, &no_audio_callback_definition);

	if (!LibretroUi::player_exit_called) {
		// Shutdown requested by the frontend and not via Title scene
		Player::Exit();
	}
}

// unused stuff required by libretro api
// this looks like features only emulators use but they say that libretro is
// not a emulator only API :P

/* Returns the amount of data the implementation requires to serialize
 * internal state (save states).
 * Between calls to retro_load_game() and retro_unload_game(), the
 * returned size is never allowed to be larger than a previous returned
 * value, to ensure that the frontend can allocate a save state buffer once.
 */
RETRO_API size_t retro_serialize_size() {
	// no-op
	return 0;
}

/* Serializes internal state. If failed, or size is lower than
 * retro_serialize_size(), it should return false, true otherwise. */
RETRO_API bool retro_serialize(void *data, size_t size) {
	// no-op
	return false;
}

RETRO_API bool retro_unserialize(const void *data, size_t size) {
	// no-op
	return false;
}

RETRO_API void retro_cheat_reset(void) {
	// not used
}

RETRO_API void retro_cheat_set(unsigned index, bool enabled, const char *code) {
	// not used
}

/* Loads a "special" kind of game. Should not be used,
 * except in extreme cases. */
RETRO_API bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info) {
	// no-op
	return false;
}

/* Gets region of game. */
RETRO_API unsigned retro_get_region() {
	// no-op
	return RETRO_REGION_NTSC;
}

/* Gets region of memory. */
RETRO_API void* retro_get_memory_data(unsigned id) {
	// no-op
	return nullptr;
}

RETRO_API size_t retro_get_memory_size(unsigned id) {
	// no-op
	return 0;
}

#endif
