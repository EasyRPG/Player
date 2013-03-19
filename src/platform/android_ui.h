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

#ifndef _ANDROID_UI_H_
#define _ANDROID_UI_H_

// Headers
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
	void DrawScreenText(const std::string &text);
	void DrawScreenText(const std::string &text, int x, int y, Color const& color);
	void DrawScreenText(const std::string &text, Rect const& dst_rect, Color const& color);
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
