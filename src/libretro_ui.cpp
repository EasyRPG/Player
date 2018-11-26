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
#include "color.h"
#include "graphics.h"
#include "keys.h"
#include "output.h"
#include "player.h"
#include "bitmap.h"

#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include <glsm/glsm.h>
#endif

#include <cstring>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>
#include <math.h>

#ifdef SUPPORT_AUDIO
#include "audio_libretro.h"
AudioInterface& LibretroUi::GetAudio() {
	return *audio_;
}
#endif

retro_usec_t LibretroUi::time_in_microseconds = 0;

LibretroUi::LibretroUi(int width, int height) {
	current_display_mode.width = width;
	current_display_mode.height = height;
	current_display_mode.bpp = 32;
	#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
	// RGBA
	const DynamicFormat format(
		32,
		0x000000FF,
		0x0000FF00,
		0x00FF0000,
		0xFF000000,
		PF::NoAlpha);
	#else
	// BGRA
	const DynamicFormat format(
		32,
		0x00FF0000,
		0x0000FF00,
		0x000000FF,
		0xff000000,
		PF::NoAlpha);
	#endif
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
}

void LibretroUi::BeginDisplayModeChange() {
	// no-op
}

void LibretroUi::EndDisplayModeChange() {
	// no-op
}

void LibretroUi::Resize(long width, long height) {
	// no-op
}

void LibretroUi::ToggleFullscreen() {
	// no-op
}

void LibretroUi::ToggleZoom() {
	// no-op
}

void LibretroUi::UpdateDisplay() {
	if (UpdateWindow == 0) {
		return;
	}

#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
	if (!retro_gl_framebuffer_ready) {
		UpdateWindow(NULL, SCREEN_TARGET_WIDTH, SCREEN_TARGET_HEIGHT, 0);
		return;
	}
	glsm_ctl(GLSM_CTL_STATE_BIND, NULL);

	GLuint TextureID;
	glViewport(0, 0, current_display_mode.width, current_display_mode.height);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glDepthFunc(GL_LEQUAL);

	glGenTextures(1,&TextureID);

	glBindTexture(GL_TEXTURE_2D,TextureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,current_display_mode.width,current_display_mode.height,0,GL_RGBA,GL_UNSIGNED_BYTE,main_surface->pixels());

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0,current_display_mode.width, 0.0, current_display_mode.height, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glLoadIdentity();
	glDisable(GL_LIGHTING);

	glBegin(GL_QUADS);
	glColor3f(1,1,1);
	glTexCoord2f(0,0); glVertex3f(0,current_display_mode.height,0);
	glTexCoord2f(1,0); glVertex3f(current_display_mode.width,current_display_mode.height,0);
	glTexCoord2f(1,1); glVertex3f(current_display_mode.width,0,0);
	glTexCoord2f(0,1); glVertex3f(0,0,0);
	glEnd();

	glDisable(GL_TEXTURE_2D);

	glPopMatrix();


	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glDeleteTextures(1,&TextureID);

	glsm_ctl(GLSM_CTL_STATE_UNBIND, NULL);

	UpdateWindow(RETRO_HW_FRAME_BUFFER_VALID, SCREEN_TARGET_WIDTH, SCREEN_TARGET_HEIGHT, 0);
#else
	UpdateWindow(main_surface->pixels(), current_display_mode.width, current_display_mode.height, main_surface->pitch());
#endif
}
void LibretroUi::SetTitle(const std::string &title){

}
bool LibretroUi::ShowCursor(bool flag) {
	return false;
}

void LibretroUi::ProcessEvents() {
	if (CheckInputState == 0) {
		return;
	}

	keys[Input::Keys::UP]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_UP) != 0);       //UP
	keys[Input::Keys::DOWN]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_DOWN) != 0);   //DOWN
	keys[Input::Keys::LEFT]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_LEFT) != 0);   //LEFT
	keys[Input::Keys::RIGHT]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_RIGHT) != 0); //RIGHT
	keys[Input::Keys::SPACE]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_A) != 0);    //DECISION
	keys[Input::Keys::RETURN]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_START) != 0); //DECISION
	keys[Input::Keys::AC_BACK]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_B) != 0);   //CHANCEL
	/*keys[Input::Keys::Z]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_X) != 0);
	keys[Input::Keys::Z]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_Y) != 0);
	keys[Input::Keys::Z]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_SELECT) != 0);
	keys[Input::Keys::Z]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_L) != 0);
	keys[Input::Keys::Z]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_R) != 0);
	keys[Input::Keys::Z]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_L2) != 0);
	keys[Input::Keys::Z]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_R2) != 0);
	keys[Input::Keys::Z]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_L3) != 0);
	keys[Input::Keys::Z]=(CheckInputState(0,RETRO_DEVICE_JOYPAD,0,RETRO_DEVICE_ID_JOYPAD_R3) != 0);*/
}

bool LibretroUi::IsFullscreen() {
	return false;
}

uint32_t LibretroUi::GetTicks() const {
	// Despite it's name this function should obviously return the amount of milliseconds since the game started.
	return (uint32_t)(time_in_microseconds/1000);
}
void LibretroUi::Sleep(uint32_t){
	// Sleep is not needed libretro will ensure 60 fps
}

#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
bool LibretroUi::retro_gl_framebuffer_ready = false;
void LibretroUi::ResetRetroGLContext(void) {
	glsm_ctl(GLSM_CTL_STATE_CONTEXT_RESET, NULL);

	if (!glsm_ctl(GLSM_CTL_STATE_SETUP, NULL))
		return;

	retro_gl_framebuffer_ready=true;
}

void LibretroUi::DestroyRetroGLContext(void){
}

bool LibretroUi::LockRetroGLFramebuffer(void *data){
	if (retro_gl_framebuffer_ready)
		return false;
	return true;
}
#endif

retro_video_refresh_t LibretroUi::UpdateWindow = 0;
void LibretroUi::SetRetroVideoCallback(retro_video_refresh_t cb) {
	UpdateWindow = cb;
}

retro_input_state_t LibretroUi::CheckInputState = 0;
void LibretroUi::SetRetroInputStateCallback(retro_input_state_t cb) {
	CheckInputState = cb;
}

////////////////

#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include <glsm/glsm.h>
#endif

#include <libretro.h>

#include "player.h"
#include "graphics.h"
#include "input.h"
#include "main_data.h"
#include "version.h"
#include "options.h"
#include "libretro_ui.h"
#include "audio_libretro.h"
#include "scene.h"

static const unsigned AUDIO_SAMPLERATE = 44100.0;

retro_environment_t environ_cb;
static retro_input_poll_t  poll_cb;

RETRO_CALLCONV void retro_time_update(retro_usec_t usec) {
	LibretroUi::time_in_microseconds += usec;
}

RETRO_CALLCONV void retro_write_audio(void) {
	if (DisplayUi)
		LibretroAudio::AudioThreadCallback();
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
		1000000 / Graphics::GetDefaultFps()
	};

	static retro_audio_callback audio_callback_definition = {
		retro_write_audio,
		retro_enable_audio
	};

	static struct retro_log_callback logging;

	environ_cb = cb;

	cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_content);
	cb(RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK, &audio_callback_definition);
	cb(RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK, &frame_time_definition);

	if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
		log_cb = logging.log;
	else
		log_cb = fallback_log;
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
	poll_cb = cb;
}

RETRO_API void retro_set_input_state(retro_input_state_t cb) {
	LibretroUi::SetRetroInputStateCallback(cb);
}

static void reinit_easy_rpg(void) {
	Player::Init(0, 0);
	Input::Init("", "");
}

/* Library global initialization/deinitialization. */
RETRO_API void retro_init(void) {
	reinit_easy_rpg();
}

RETRO_API void retro_deinit(void) {
	Player::Exit();
}

/* Must return RETRO_API_VERSION. Used to validate ABI compatibility
 * when the API is revised. */
RETRO_API unsigned retro_api_version(void) {
	return RETRO_API_VERSION;
}

/* Gets statically known system info. Pointers provided in *info
 * must be statically allocated.
 * Can be called at any time, even before retro_init(). */
RETRO_API void retro_get_system_info(struct retro_system_info *info) {
	memset(info, 0, sizeof(*info));
	info->library_name = "EasyRPG";
	#ifndef GIT_VERSION
	#define GIT_VERSION ""
	#endif
	info->library_version = PLAYER_VERSION GIT_VERSION;
	info->need_fullpath = true;
	info->valid_extensions = "ini";
}

/* Gets information about system audio/video timings and geometry.
 * Can be called only after retro_load_game() has successfully completed.
 * NOTE: The implementation of this function might not initialize every
 * variable if needed.
 * E.g. geom.aspect_ratio might not be initialized if core doesn't
 * desire a particular aspect ratio. */
RETRO_API void retro_get_system_av_info(struct retro_system_av_info *info) {
	info->geometry.base_width = SCREEN_TARGET_WIDTH;
	info->geometry.base_height = SCREEN_TARGET_HEIGHT;
	info->geometry.max_width = SCREEN_TARGET_WIDTH;
	info->geometry.max_height = SCREEN_TARGET_HEIGHT;
	info->geometry.aspect_ratio = 0.0f;
	info->timing.fps = Graphics::GetDefaultFps();
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

static void check_variables(bool first_time_startup) {
}

/* Runs the game for one video frame.
 * During retro_run(), input_poll callback must be called at least once.
 *
 * If a frame is not rendered for reasons where a game "dropped" a frame,
 * this still counts as a frame, and retro_run() should explicitly dupe
 * a frame if GET_CAN_DUPE returns true.
 * In this case, the video callback can take a NULL argument for data.
 */

RETRO_API void retro_run(void) {
	poll_cb();

	if (!Player::exit_flag) {
		Player::MainLoop();
		if (!DisplayUi) { // Only occurs when the function Player::Exit() was called from within the game
			Player::exit_flag=true;
			environ_cb(RETRO_ENVIRONMENT_SHUTDOWN, 0);
		}
	}
}

/* Returns the amount of data the implementation requires to serialize
 * internal state (save states).
 * Between calls to retro_load_game() and retro_unload_game(), the
 * returned size is never allowed to be larger than a previous returned
 * value, to ensure that the frontend can allocate a save state buffer once.
 */
RETRO_API size_t

retro_serialize_size(void) {
	return 0;
}

/* Serializes internal state. If failed, or size is lower than
 * retro_serialize_size(), it should return false, true otherwise. */
RETRO_API bool retro_serialize(void *data, size_t size) {
	return false;
}

RETRO_API bool retro_unserialize(const void *data, size_t size) {
	return false;
}

RETRO_API void retro_cheat_reset(void) {
	// not used
}

RETRO_API void retro_cheat_set(unsigned index, bool enabled, const char *code) {
	// not used
}

static void extract_directory(char *buf, const char *path, size_t size)
{
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
RETRO_API bool retro_load_game(const struct retro_game_info *game)
{
   char parent_dir[1024];
	enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
	glsm_ctx_params_t params = {0};
#endif

	if (!game)
		return false;

	if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt)) {
		log_cb(RETRO_LOG_INFO, "XRGB8888 is not supported.\n");
		return false;
	}

#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
	params.context_reset		 = LibretroUi::ResetRetroGLContext;
	params.context_destroy	   = LibretroUi::DestroyRetroGLContext;
	params.environ_cb			   = environ_cb;
	params.stencil			   = false;
	params.imm_vbo_draw		  = NULL;
	params.imm_vbo_disable	   = NULL;
	params.framebuffer_lock	  = LibretroUi::LockRetroGLFramebuffer;

	if (!glsm_ctl(GLSM_CTL_STATE_CONTEXT_INIT, &params)){
		return false;
	}
#endif

	if (game != 0)
	   extract_directory(parent_dir, game->path, sizeof(parent_dir));

	Player::exit_flag = false;

	if (!DisplayUi) { // If player was exited before -> reiinitialize
		reinit_easy_rpg();
	}

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

/* Loads a "special" kind of game. Should not be used,
 * except in extreme cases. */
RETRO_API bool retro_load_game_special(
		unsigned game_type,
		const struct retro_game_info *info, size_t num_info
) { //not used
	return false;
}

/* Unloads a currently loaded game. */
RETRO_API void retro_unload_game(void) {
	if (!Player::exit_flag){
		Player::exit_flag = true;
		Player::MainLoop(); // Execute one mainloop to do the exiting
	}
	Main_Data::Cleanup();
}

/* Gets region of game. */
RETRO_API unsigned retro_get_region(void) {
	return RETRO_REGION_NTSC;
}

/* Gets region of memory. */
RETRO_API void *retro_get_memory_data(unsigned id) {
	return 0;
}

RETRO_API size_t retro_get_memory_size(unsigned id) {
	return 0;
}

#endif
