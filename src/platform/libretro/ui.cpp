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
#include "clock.h"
#include "bitmap.h"
#include "color.h"
#include "filefinder.h"
#include "graphics.h"
#include "input.h"
#include "keys.h"
#include "main_data.h"
#include "options.h"
#include "output.h"
#include "player.h"
#include "scene.h"
#include "utils.h"

#include <cstring>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstdarg>
#include <string>
#include <cmath>

namespace Options {
	const char* debug_mode = "easyrpg_debug_mode";
}

#ifdef SUPPORT_AUDIO
#include "audio.h"
AudioInterface& LibretroUi::GetAudio() {
	return *audio_;
}
#endif

retro_environment_t LibretroUi::environ_cb = nullptr;
retro_input_poll_t LibretroUi::input_poll_cb = nullptr;
bool LibretroUi::player_exit_called = false;
Game_ConfigInput LibretroUi::cfg_input;

#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
static Input::Keys::InputKey RetroJKey2InputKey(int button_index);
#endif

// libretro needs an upper limit for the framebuffer set in av_info
const int fb_max_width = 1920;
const int fb_max_height = 1080;

LibretroUi::LibretroUi(int width, int height, const Game_Config& cfg) : BaseUi(cfg)
{
	// Handled by libretro
	// FIXME: There is currently no callback from libretro telling us whether or not fullscreen is enabled.
	SetIsFullscreen(true);

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

	main_surface = Bitmap::Create(current_display_mode.width,
		current_display_mode.height,
		false,
		current_display_mode.bpp
	);

	#ifdef SUPPORT_AUDIO
	audio_ = std::make_unique<LibretroAudio>(cfg.audio);
	#endif

	cfg_input = cfg.input;

	UpdateVariables();
}

void LibretroUi::UpdateDisplay() {
	if (UpdateWindow == nullptr) {
		return;
	}

	UpdateWindow(main_surface->pixels(), current_display_mode.width, current_display_mode.height, main_surface->pitch());
}

bool LibretroUi::vChangeDisplaySurfaceResolution(int new_width, int new_height) {
	if (new_width > fb_max_width || new_height > fb_max_height) {
		Output::Warning("ChangeDisplaySurfaceResolution: {}x{} is too large", new_width, new_height);
		return false;
	}

	BitmapRef new_main_surface = Bitmap::Create(new_width, new_height, false, current_display_mode.bpp);

	if (!new_main_surface) {
		Output::Warning("ChangeDisplaySurfaceResolution Bitmap::Create failed");
		return false;
	}

	retro_game_geometry geom = {};
	geom.base_width = new_width;
	geom.base_height = new_height;
	if (!LibretroUi::environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &geom)) {
		Output::Warning("ChangeDisplaySurfaceResolution SET_GEOMETRY failed");
		return false;
	}

	main_surface = new_main_surface;

	current_display_mode.width = new_width;
	current_display_mode.height = new_height;

	return true;
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

	auto check_pressed = [this] (int button_id) {
		keys[RetroJKey2InputKey(button_id)] = CheckInputState(0, RETRO_DEVICE_JOYPAD, 0, button_id) != 0;
	};

	check_pressed(RETRO_DEVICE_ID_JOYPAD_UP);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_DOWN);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_LEFT);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_RIGHT);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_A);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_B);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_Y);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_X);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_L);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_R);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_L3);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_R3);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_START);
	check_pressed(RETRO_DEVICE_ID_JOYPAD_SELECT);

#	if defined(USE_JOYSTICK_AXIS) && defined(SUPPORT_JOYSTICK_AXIS)
	auto normalize = [](int value) {
		return static_cast<float>(value) / 32768.f;
	};

	analog_input.primary.x = normalize(CheckInputState(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X));
	analog_input.primary.y = normalize(CheckInputState(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y));
	analog_input.secondary.x = normalize(CheckInputState(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X));
	analog_input.secondary.y = normalize(CheckInputState(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y));
	analog_input.trigger_left = normalize(CheckInputState(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_BUTTON, RETRO_DEVICE_ID_JOYPAD_L2));
	analog_input.trigger_right = normalize(CheckInputState(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_BUTTON, RETRO_DEVICE_ID_JOYPAD_R2));
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

void LibretroUi::UpdateVariables() {
	static struct retro_variable debug = { Options::debug_mode, nullptr };

	LibretroUi::environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &debug);
	Player::debug_flag = strcmp(debug.value, "Enabled") == 0;
}

#if defined(USE_JOYSTICK) && defined(SUPPORT_JOYSTICK)
Input::Keys::InputKey RetroJKey2InputKey(int button_index) {
	switch (button_index) {
		case RETRO_DEVICE_ID_JOYPAD_UP		: return Input::Keys::JOY_DPAD_UP;
		case RETRO_DEVICE_ID_JOYPAD_DOWN	: return Input::Keys::JOY_DPAD_DOWN;
		case RETRO_DEVICE_ID_JOYPAD_LEFT	: return Input::Keys::JOY_DPAD_LEFT;
		case RETRO_DEVICE_ID_JOYPAD_RIGHT	: return Input::Keys::JOY_DPAD_RIGHT;
		case RETRO_DEVICE_ID_JOYPAD_A		: return Input::Keys::JOY_A;
		case RETRO_DEVICE_ID_JOYPAD_B		: return Input::Keys::JOY_B;
		case RETRO_DEVICE_ID_JOYPAD_X		: return Input::Keys::JOY_X;
		case RETRO_DEVICE_ID_JOYPAD_Y		: return Input::Keys::JOY_Y;
		case RETRO_DEVICE_ID_JOYPAD_START	: return Input::Keys::JOY_START;
		case RETRO_DEVICE_ID_JOYPAD_SELECT	: return Input::Keys::JOY_BACK;
		case RETRO_DEVICE_ID_JOYPAD_L		: return Input::Keys::JOY_SHOULDER_LEFT;
		case RETRO_DEVICE_ID_JOYPAD_R		: return Input::Keys::JOY_SHOULDER_RIGHT;
		case RETRO_DEVICE_ID_JOYPAD_L3		: return Input::Keys::JOY_LSTICK;
		case RETRO_DEVICE_ID_JOYPAD_R3		: return Input::Keys::JOY_RSTICK;

		default : return Input::Keys::NONE;
	}
}
#endif

void LibretroUi::vGetConfig(Game_ConfigVideo& cfg) const {
	cfg.renderer.Lock("Libretro (Software)");
	cfg.game_resolution.SetOptionVisible(true);
}

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

	static struct retro_log_callback logging;

	LibretroUi::environ_cb = cb;

	cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_content);
	cb(RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK, &audio_callback_definition);
	cb(RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK, &frame_time_definition);

	if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
		log_cb = logging.log;
	else
		log_cb = fallback_log;

	struct retro_variable variables[] = {
		{ Options::debug_mode, "Debug menu and walk through walls; Disabled|Enabled" },
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
	std::vector<std::string> args;

	Player::Init(args);

	Input::Init(LibretroUi::cfg_input, "", "");
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
	info->library_name = GAME_TITLE;
	info->library_version = PLAYER_VERSION_FULL;
	info->need_fullpath = true;
	info->valid_extensions = "ldb|zip|easyrpg";
	info->block_extract = true;
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
	info->geometry.max_width = fb_max_width;
	info->geometry.max_height = fb_max_height;
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

/* Loads a game. */
RETRO_API bool retro_load_game(const struct retro_game_info* game) {
	enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;

	if (!game)
		return false;

	if (!LibretroUi::environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt)) {
		log_cb(RETRO_LOG_INFO, "XRGB8888 is not supported.\n");
		return false;
	}

	Output::IgnorePause(true);

	std::string game_path = game->path;

	// Convert RetroArch archive paths to paths our VFS understands
	game_path = Utils::ReplaceAll(game_path, ".zip#", ".zip/");
	game_path = Utils::ReplaceAll(game_path, ".easyrpg#", ".easyrpg/");

	auto fs = FileFinder::Root().Create(game_path);
	if (!fs) {
		std::tie(game_path, std::ignore) = FileFinder::GetPathAndFilename(game_path);
		fs = FileFinder::Root().Create(game_path);
		if (!fs || !FileFinder::IsValidProject(fs)) {
			log_cb(RETRO_LOG_ERROR, "Unsupported game %s\n", game_path.c_str());
			return false;
		}
	}
	FileFinder::SetGameFilesystem(fs);

	init_easy_rpg();

	Player::Run();

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
