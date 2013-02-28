#ifndef _ANDROID_UI_H_
#define _ANDROID_UI_H_

#include "baseui.h"
#include "keys.h"
#include <boost/scoped_ptr.hpp>

#include <vector>
#include <map>

#include <EGL/egl.h>
#include <GLES/gl.h> // OpenGL ES 1

struct android_app;

struct AInputQueue;
struct ASensor;
struct ASensorManager;
struct ASensorEventQueue;

struct ALAudio;

struct AndroidUi : public BaseUi {
	AndroidUi(android_app* app);

	void BeginDisplayModeChange();
	void EndDisplayModeChange();
	void Resize(long width, long height);
	void ToggleFullscreen();
	void ToggleZoom();
	void ProcessEvents();
	void UpdateDisplay();
	void BeginScreenCapture();
	BitmapRef EndScreenCapture();
	void SetTitle(const std::string &title);
	bool ShowCursor(bool flag);
	bool IsFullscreen();
	uint32_t GetTicks() const;
	void Sleep(uint32_t time_milli);
	AudioInterface& GetAudio();

private:
	boost::scoped_ptr<ALAudio> audio_;

	android_app* const app_;
	ASensorManager* const sensor_man_;
	ASensor const* const sensor_;
	ASensorEventQueue* const sensor_queue_;

	EGLDisplay diplay;
	EGLSurface surface_;
	EGLContext ctx_;
	unsigned width_, height_;

	struct Button {
		unsigned x, y;
		unsigned size;
		std::string txt;
		Input::Keys::InputKey key;
	};
	std::vector<Button> button_list_;
}; // struct AndroidUi

#endif // _ANDROID_UI_H_
