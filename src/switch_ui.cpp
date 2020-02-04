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

#ifdef __SWITCH__

// Headers
#include "switch_ui.h"
#include "color.h"
#include "graphics.h"
#include "keys.h"
#include "output.h"
#include "player.h"
#include "bitmap.h"

#include <switch.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <glad/glad.h>

#include <cstring>
#include <cstdio>
#include <iostream>
#include <unistd.h>

#include "touch_ui_png.h"

#ifdef SUPPORT_AUDIO
#include "audio_switch.h"

AudioInterface& NxUi::GetAudio() {
	return *audio_;
}
#endif

namespace {
	const double ticks_per_msec = 19200.0f;

	int nxlinkSocket = -1;

	EGLDisplay eglDisplay;
	EGLContext eglContext;
	EGLSurface eglSurface;

	const GLfloat _offsets[] = {
		0.f, 0.f,
		1.f, 0.f,
		1.f, 1.f,
		0.f, 1.f
	};

	const GLchar* const vertexShaderSource = R"glsl(
		#version 330 core
		precision mediump float;
		attribute vec2 offset;
		uniform vec2 dims;
		varying vec2 texCoord;

		void main() {
			vec2 scaledOffset = offset * dims;
			gl_Position = vec4(scaledOffset.x * 2.0 - dims.x, scaledOffset.y * -2.0 + dims.y, 0.0, 1.0);
			texCoord = offset;
		}
	)glsl";

	const GLchar* const fragmentShaderSource = R"glsl(
		#version 330 core
		precision mediump float;
		varying vec2 texCoord;
		uniform sampler2D tex;

		void main() {
			gl_FragColor = vec4(texture2D(tex, texCoord).rgb, 1.0);
		}
	)glsl";

	GLuint shaderProgramm;
	GLuint shaderVAO;
	GLuint shaderVBO;
	GLuint texLocation;
	GLuint dimsLocation;
	GLuint textures[2];

	const int touch_left[] = {
		Input::Keys::N1, Input::Keys::N2, Input::Keys::N3, Input::Keys::N4,
		Input::Keys::N5, Input::Keys::N6, Input::Keys::N7, Input::Keys::N8
	};

	const int touch_right[] = {
		Input::Keys::ESCAPE, Input::Keys::N9, Input::Keys::N0,
		Input::Keys::PERIOD, Input::Keys::ADD, Input::Keys::SUBTRACT,
		Input::Keys::MULTIPLY, Input::Keys::DIVIDE
	};
}

extern "C" void userAppInit() {
	if (R_FAILED(socketInitializeDefault()))
		return;

	nxlinkSocket = nxlinkStdio();
	if (nxlinkSocket < 0)
		socketExit();
}

extern "C" void userAppExit() {
	if (nxlinkSocket >= 0) {
		close(nxlinkSocket);
		socketExit();
		nxlinkSocket = -1;
	}
}

static void initEgl() {
	eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (!eglDisplay) {
		Output::Error("Could not connect to display! error: %d", eglGetError());
		return;
	}

	eglInitialize(eglDisplay, nullptr, nullptr);

	if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE) {
		eglTerminate(eglDisplay);
		eglDisplay = nullptr;
		Output::Error("Could not set OpenGL API! error: %d", eglGetError());
		return;
	}

	EGLConfig config;
	EGLint numConfigs;
	static const EGLint framebufferAttributeList[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
		EGL_RED_SIZE,     8,
		EGL_GREEN_SIZE,   8,
		EGL_BLUE_SIZE,    8,
		EGL_ALPHA_SIZE,   8,
		EGL_NONE
	};
	eglChooseConfig(eglDisplay, framebufferAttributeList, &config, 1, &numConfigs);
	if (numConfigs == 0) {
		eglTerminate(eglDisplay);
		eglDisplay = nullptr;
		Output::Error("No EGL config found! error: %d", eglGetError());
		return;
	}

	eglSurface = eglCreateWindowSurface(eglDisplay, config, nwindowGetDefault(),
		nullptr);
	if (!eglSurface) {
		eglTerminate(eglDisplay);
		eglDisplay = nullptr;
		Output::Error("EGL Surface creation failed! error: %d", eglGetError());
		return;
	}

	static const EGLint contextAttributeList[] = {
		EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,
		EGL_CONTEXT_MAJOR_VERSION_KHR, 4,
		EGL_CONTEXT_MINOR_VERSION_KHR, 3,
		EGL_NONE
	};
	eglContext = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT,
		contextAttributeList);
	if (!eglContext) {
		eglDestroySurface(eglDisplay, eglSurface);
		eglSurface = nullptr;
		eglTerminate(eglDisplay);
		eglDisplay = nullptr;
		Output::Error("EGL Context creation failed! error: %d", eglGetError());
		return;
	}

	eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
}

static void deinitEgl() {
	if (eglDisplay) {
		eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (eglContext) {
			eglDestroyContext(eglDisplay, eglContext);
			eglContext = nullptr;
		}
		if (eglSurface) {
			eglDestroySurface(eglDisplay, eglSurface);
			eglSurface = nullptr;
		}
		eglTerminate(eglDisplay);
		eglDisplay = nullptr;
	}
}

static GLuint createAndCompileShader(GLenum type, const char* source) {
	GLint success;
	GLchar msg[512];

	GLuint handle = glCreateShader(type);
	if (!handle) {
		Output::Error("%u: cannot create shader", type);
		return 0;
	}
	glShaderSource(handle, 1, &source, nullptr);
	glCompileShader(handle);
	glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE) {
		glGetShaderInfoLog(handle, sizeof(msg), nullptr, msg);
		Output::Error("%u: %s\n", type, msg);
		glDeleteShader(handle);
		return 0;
	}

	return handle;
}

NxUi::NxUi(int width, int height) :
	BaseUi() {
#if 1
	setenv("MESA_NO_ERROR", "1", 1);
#else
	// shader/gl debugging
	setenv("EGL_LOG_LEVEL", "debug", 1);
	setenv("MESA_VERBOSE", "all", 1);
	setenv("NOUVEAU_MESA_DEBUG", "1", 1);

	setenv("NV50_PROG_OPTIMIZE", "0", 1);
	setenv("NV50_PROG_DEBUG", "1", 1);
	setenv("NV50_PROG_CHIPSET", "0x120", 1);
#endif

	nwindowSetDimensions(nwindowGetDefault(), 1920, 1080);
	initEgl();
	gladLoadGL();

	fullscreen = false;
	current_display_mode.width = width;
	current_display_mode.height = height;
	current_display_mode.bpp = 32;
	const DynamicFormat format(
		32,
		0x000000FF,
		0x0000FF00,
		0x00FF0000,
		0xFF000000,
		PF::NoAlpha);
	Bitmap::SetFormat(Bitmap::ChooseFormat(format));
	main_surface = Bitmap::Create(width, height, true, 32);
	touch_ui = Bitmap::Create(touch_ui_png, touch_ui_png_size, false);

	GLint vsh = createAndCompileShader(GL_VERTEX_SHADER, vertexShaderSource);
	GLint fsh = createAndCompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
	shaderProgramm = glCreateProgram();
	glAttachShader(shaderProgramm, vsh);
	glAttachShader(shaderProgramm, fsh);
	glLinkProgram(shaderProgramm);
	GLint success;
	glGetProgramiv(shaderProgramm, GL_LINK_STATUS, &success);
	if (success == GL_FALSE) {
		char buf[512];
		glGetProgramInfoLog(shaderProgramm, sizeof(buf), nullptr, buf);
		Output::Warning("Shader link error: %s", buf);
	}
	glDeleteShader(vsh);
	glDeleteShader(fsh);

	glGenTextures(2, textures);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, touch_ui->GetWidth(),
		touch_ui->GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE,
		touch_ui->pixels());

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, NULL);

	texLocation = glGetUniformLocation(shaderProgramm, "tex");
	dimsLocation = glGetUniformLocation(shaderProgramm, "dims");
	GLuint offsetLocation = glGetAttribLocation(shaderProgramm, "offset");

	glGenBuffers(1, &shaderVBO);
	glGenVertexArrays(1, &shaderVAO);
	glBindVertexArray(shaderVAO);
	glBindBuffer(GL_ARRAY_BUFFER, shaderVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_offsets), _offsets, GL_STATIC_DRAW);
	glVertexAttribPointer(offsetLocation, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(offsetLocation);

	glUseProgram(shaderProgramm);

#ifdef SUPPORT_AUDIO
	audio_.reset(new NxAudio());
#endif
}

NxUi::~NxUi() {
	glBindVertexArray(0);
	glUseProgram(0);

	glDeleteTextures(2, textures);
	glDeleteProgram(shaderProgramm);
	glDeleteBuffers(1, &shaderVBO);
	glDeleteVertexArrays(1, &shaderVAO);

	deinitEgl();
}

void NxUi::BeginDisplayModeChange() {
	// no-op
}

void NxUi::EndDisplayModeChange() {
	// no-op
}

void NxUi::Resize(long /*width*/, long /*height*/) {
	// no-op
}

void NxUi::ToggleFullscreen() {
	// no-op
}

void NxUi::ToggleZoom() {
	// no-op
}

bool NxUi::IsFullscreen() {
	return true;
}

void NxUi::ProcessEvents() {
	hidScanInput();

	u32 input = hidKeysHeld(CONTROLLER_P1_AUTO);
	keys[Input::Keys::UP] = (input & KEY_DUP);
	keys[Input::Keys::DOWN] = (input & KEY_DDOWN);
	keys[Input::Keys::RIGHT] = (input & KEY_DRIGHT);
	keys[Input::Keys::LEFT] = (input & KEY_DLEFT);
	keys[Input::Keys::Z] = (input & KEY_A);
	keys[Input::Keys::X] = (input & KEY_B);
	keys[Input::Keys::X] = (input & KEY_X);
	keys[Input::Keys::LSHIFT] = (input & KEY_Y);
	keys[Input::Keys::F2] = (input & KEY_L);
	keys[Input::Keys::F] = (input & KEY_R);
	keys[Input::Keys::F12] = (input & KEY_MINUS);
	keys[Input::Keys::ESCAPE] = (input & KEY_PLUS);

	// cycle through GUI layouts
	input = hidKeysDown(CONTROLLER_P1_AUTO);
	if (input & KEY_ZL)
		ui_mode = (ui_mode + 1) % 3;

	// do not handle touch when not displaying buttons
	if (ui_mode != 0)
		return;

	for (int i = 0; i < 8; ++i) {
		keys[touch_left[i]] = false;
		keys[touch_right[i]] = false;
	}
	for (uint32_t i = 0; i < hidTouchCount(); ++i) {
		touchPosition pos;
		hidTouchRead(&pos, i);

		if (pos.px < 160) {
			// shouldn't happen
			if (pos.py < 720) {
				keys[touch_left[pos.py / (720 / 8)]] = true;
			}
		} else if (pos.px >= 1280 - 160) {
			if (pos.py < 720) {
				keys[touch_right[pos.py / (720 / 8)]] = true;
			}
		}
	}
}

void NxUi::UpdateDisplay() {
	float aspectX = 1.0f;
	float aspectY = 1.0f;

	// full res when docked
	int width = 1280, height = 720;
	if (appletGetOperationMode() == AppletOperationMode_Docked) {
		width = 1920;
		height = 1080;

		// disable touch input
		if (ui_mode == 0)
			ui_mode++;
	}
	nwindowSetCrop(nwindowGetDefault(), 0, 0, width, height);
	glViewport(0, 1080-height, width, height);

	// clear
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// touch texture
	if (ui_mode == 0) {
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(texLocation, 0);
		glBindTexture(GL_TEXTURE_2D, textures[0]);
		glUniform2f(dimsLocation, aspectX, aspectY);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}

	// screen texture
	glActiveTexture(GL_TEXTURE1);
	glUniform1i(texLocation, 1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, main_surface->GetWidth(),
		main_surface->GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE,
		main_surface->pixels());
	if (ui_mode != 2)
		aspectX = 0.75f;
	glUniform2f(dimsLocation, aspectX, aspectY);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// display
	eglSwapBuffers(eglDisplay, eglSurface);
}

void NxUi::BeginScreenCapture() {
	CleanDisplay();
}

BitmapRef NxUi::EndScreenCapture() {
	return Bitmap::Create(*main_surface, main_surface->GetRect());
}

void NxUi::SetTitle(const std::string& /* title */) {
	// no-op
}

bool NxUi::ShowCursor(bool /* flag */) {
	return true;
}

#endif
