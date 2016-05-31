#ifdef USE_LIBRETRO
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>
#include <math.h>

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

static const unsigned AUDIO_SAMPLERATE = 44100.0;

static retro_environment_t retro_environment;
static retro_input_poll_t retro_input_poll;


RETRO_CALLCONV void retro_time_update(retro_usec_t usec) {
    LibretroUi::time_in_microseconds += usec;
}

RETRO_CALLCONV void retro_write_audio(void) {
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

    retro_environment = cb;


    cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_content);
    #ifdef USE_LIBRETRO_AUDIO
      //Currently audio is only supported via EasyRPGs own implementation (no control from retro api)
      cb(RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK, &audio_callback_definition);
    #endif
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
    //unused
}

RETRO_API void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) {
    LibretroAudio::SetRetroAudioCallback(cb);
}

RETRO_API void retro_set_input_poll(retro_input_poll_t cb) {
    retro_input_poll = cb;
}

RETRO_API void retro_set_input_state(retro_input_state_t cb) {
    LibretroUi::SetRetroInputStateCallback(cb);
}

static void reinit_easy_rpg(void){
	Player::Init(0, 0);
    Graphics::Init();
    Input::Init();
}

/* Library global initialization/deinitialization. */
RETRO_API void retro_init(void) {
    LibretroAudio::SetNumberOfSamplesPerFrame(AUDIO_SAMPLERATE / Graphics::GetDefaultFps());
    LibretroAudio::SetOutputSampleRate(AUDIO_SAMPLERATE);

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
    static const char lib_name[] = "EasyRPG";
    static const char lib_version[] = PLAYER_VERSION;
    static const char lib_extensions[] = "rpg_rt.ini";
    memset(info, 0, sizeof(*info));
    info->library_name = lib_name;
    info->library_version = lib_version;
    info->need_fullpath = true;
    info->valid_extensions = lib_extensions;
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
    info->geometry.aspect_ratio = SCREEN_TARGET_WIDTH / SCREEN_TARGET_HEIGHT;
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
    //Not used
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
    retro_input_poll();

    if (!Player::exit_flag) {
        Player::MainLoop();
		if(!DisplayUi){ //Only occurs when the function Player::Exit() was called from within the game
			Player::exit_flag=true;
			retro_environment(RETRO_ENVIRONMENT_SHUTDOWN, 0);			
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
    //not used
}

RETRO_API void retro_cheat_set(unsigned index, bool enabled, const char *code) {
    //not used
}

/* Loads a game. */
RETRO_API bool retro_load_game(const struct retro_game_info *game) {

    enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
    glsm_ctx_params_t params = {0};
#endif

    if (!retro_environment(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt)) {
        log_cb(RETRO_LOG_INFO, "XRGB8888 is not supported.\n");
        return false;
    }

#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
    params.context_reset         = LibretroUi::ResetRetroGLContext;
    params.context_destroy       = LibretroUi::DestroyRetroGLContext;
    params.environ_cb        	= retro_environment;
    params.stencil               = false;
    params.imm_vbo_draw          = NULL;
    params.imm_vbo_disable       = NULL;
    params.framebuffer_lock      = LibretroUi::LockRetroGLFramebuffer;

    if (!glsm_ctl(GLSM_CTL_STATE_CONTEXT_INIT, &params)){
       return false;
 }
#endif

    std::string rpg_rt_ini_path = "";
    if (game != 0) { std::string rpg_rt_ini_path = game->path; }
    Player::exit_flag = false;
	
	if(!DisplayUi){ //If player was exited before -> reiinitialize
		reinit_easy_rpg();
	}
	
    size_t end = rpg_rt_ini_path.find_last_of("/\\");
    if (end != std::string::npos) {
        Main_Data::SetProjectPath(rpg_rt_ini_path.substr(0, end));
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
	if(Player::exit_flag!=true){
		Player::exit_flag = true;
		Player::MainLoop(); //Execute one mainloop to do the exiting
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