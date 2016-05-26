#ifndef _EASYRPG_LIBRETROUI_H_
#define _EASYRPG_LIBRETROUI_H_

// Headers
#include "baseui.h"
#include "color.h"
#include "rect.h"
#include "system.h"

#ifdef USE_LIBRETRO
#include "libretro.h"
#endif
/**
 * LibretroUi class.
 */
class LibretroUi : public BaseUi {
public:
	/**
	 * Constructor.
	 *
	 * @param width window client width.
	 * @param height window client height.
	 * @param title window title.
	 * @param fullscreen start in fullscreen flag.
	 */
	LibretroUi(int width, int height);

	/**
	 * Destructor.
	 */
	~LibretroUi();

	/**
	 * Inherited from BaseUi.
	 */
	/** @{ */

	void BeginDisplayModeChange() override;
	void EndDisplayModeChange() override;
	void Resize(long width, long height) override;
	void ToggleFullscreen() override;
	void ToggleZoom() override;
	void UpdateDisplay() override;
	void SetTitle(const std::string &title) override;
	bool ShowCursor(bool flag) override;

	void ProcessEvents() override;

	bool IsFullscreen() override;

	uint32_t GetTicks() const override;
	void Sleep(uint32_t time_milli) override;
#ifdef SUPPORT_AUDIO
	AudioInterface& GetAudio();
	std::unique_ptr<AudioInterface> audio_;
#endif

	/** @} */
#ifdef USE_LIBRETRO
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
public:
	static void ResetRetroGLContext(void);
	static void DestroyRetroGLContext(void);
	static bool LockRetroGLFramebuffer(void *data);
private:	
	static bool retro_gl_framebuffer_ready;
	
#endif
public:
	static void SetRetroVideoCallback(retro_video_refresh_t cb);
	static void SetRetroInputStateCallback(retro_input_state_t cb); 
	static retro_usec_t time_in_microseconds;
private:
	static retro_video_refresh_t UpdateWindow;
	static retro_input_state_t CheckInputState;
	
#endif
	
};

#endif
