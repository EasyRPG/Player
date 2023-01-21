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
#include "audio.h"

using namespace std::chrono_literals;

AudioInterface& NxUi::GetAudio() {
	return *audio_;
}
#endif

namespace {
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
		Input::Keys::JOY_TOUCH, Input::Keys::N9, Input::Keys::N0,
		Input::Keys::KP_PERIOD, Input::Keys::KP_ADD, Input::Keys::KP_SUBTRACT,
		Input::Keys::KP_MULTIPLY, Input::Keys::KP_DIVIDE
	};

	bool is_docked = false;
}

static void initEgl() {
	eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (!eglDisplay) {
		Output::Error("Could not connect to display! error: {}", eglGetError());
		return;
	}

	eglInitialize(eglDisplay, nullptr, nullptr);

	if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE) {
		eglTerminate(eglDisplay);
		eglDisplay = nullptr;
		Output::Error("Could not set OpenGL API! error: {}", eglGetError());
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
		Output::Error("No EGL config found! error: {}", eglGetError());
		return;
	}

	eglSurface = eglCreateWindowSurface(eglDisplay, config, nwindowGetDefault(),
		nullptr);
	if (!eglSurface) {
		eglTerminate(eglDisplay);
		eglDisplay = nullptr;
		Output::Error("EGL Surface creation failed! error: {}", eglGetError());
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
		Output::Error("EGL Context creation failed! error: {}", eglGetError());
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
		Output::Error("{}: cannot create shader", type);
		return 0;
	}
	glShaderSource(handle, 1, &source, nullptr);
	glCompileShader(handle);
	glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE) {
		glGetShaderInfoLog(handle, sizeof(msg), nullptr, msg);
		Output::Error("{}: {}\n", type, msg);
		glDeleteShader(handle);
		return 0;
	}

	return handle;
}

static AppletHookCookie applet_hook_cookie;

static void appletHookCallback(AppletHookType hook, void* param) {
	bool show_message = (bool *)param;
	// HOS sends one focus event on application start (but not in applet mode), we ignore this
	static bool first_time = true;

	switch (hook) {
		case AppletHookType_OnExitRequest:
			Output::Warning("Got close request from home menu.");
			Player::Exit();
			break;

		case AppletHookType_OnFocusState:
			if (appletGetFocusState() == AppletFocusState_InFocus) {
				if (first_time) {
					first_time = false;
					break;
				}
				if (show_message)
					Output::Debug("Gained Focus, resuming execution.");
				Player::Resume();
				appletSetFocusHandlingMode(AppletFocusHandlingMode_NoSuspend);
			} else {
				if (first_time)
					first_time = false;
				if (show_message)
					Output::Debug("Lost focus, pausing execution.");
				Player::Pause();
				// allow suspend
				appletSetFocusHandlingMode(AppletFocusHandlingMode_SuspendHomeSleepNotify);
			}
			break;

		case AppletHookType_OnOperationMode:
			int width, height;
			if (appletGetOperationMode() == AppletOperationMode_Console) {
				if (show_message)
					Output::Debug("Switched from handheld to docked mode.");

				is_docked = true;

				// full res
				width = 1920;
				height = 1080;
			} else {
				if (show_message)
					Output::Debug("Switched from docked to handheld mode.");

				is_docked = false;

				// half res
				width = 1280;
				height = 720;
			}
			nwindowSetCrop(nwindowGetDefault(), 0, 0, width, height);
			glViewport(0, 1080-height, width, height);
			break;

		default:
			break;
	}
}

NxUi::NxUi(int width, int height, const Game_Config& cfg) : BaseUi(cfg) {
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
	SetIsFullscreen(true);

	auto ver = hosversionGet();
	Output::Debug("Running on Nintendo Switch ({}.{}.{}{} CFW)",
		HOSVER_MAJOR(ver), HOSVER_MINOR(ver), HOSVER_MICRO(ver),
		(hosversionIsAtmosphere() ? ", Atmosphere" : ", unknown/no"));

	appletHook(&applet_hook_cookie, appletHookCallback, (void *)true);
	appletSetFocusHandlingMode(AppletFocusHandlingMode_NoSuspend);

	nwindowSetDimensions(nwindowGetDefault(), 1920, 1080);
	initEgl();
	gladLoadGL();

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
		Output::Warning("Shader link error: {}", buf);
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

	// manually check once, if in handheld or docked mode
	appletHookCallback(AppletHookType_OnOperationMode, (void *)false);

#ifdef SUPPORT_AUDIO
	audio_ = std::make_unique<NxAudio>(cfg.audio);
#endif

	padConfigureInput(1, HidNpadStyleSet_NpadStandard);
	padInitializeDefault(&pad);
	hidInitializeTouchScreen();

	vcfg.touch_ui.SetLocked(vcfg.stretch.Get());
}

NxUi::~NxUi() {
	glBindVertexArray(0);
	glUseProgram(0);

	glDeleteTextures(2, textures);
	glDeleteProgram(shaderProgramm);
	glDeleteBuffers(1, &shaderVBO);
	glDeleteVertexArrays(1, &shaderVAO);

	deinitEgl();

	appletSetFocusHandlingMode(AppletFocusHandlingMode_SuspendHomeSleep);
	appletUnhook(&applet_hook_cookie);
}

void NxUi::ProcessEvents() {
	// handle system events
	appletMainLoop();

	// idle, when out of focus
	while(appletGetFocusState() != AppletFocusState_InFocus) {
		Game_Clock::SleepFor(10ms);
	}

	padUpdate(&pad);

	// reset touch keys
	for (int i = 0; i < 8; ++i) {
		keys[touch_left[i]] = false;
		keys[touch_right[i]] = false;
	}

	u64 input = padGetButtons(&pad);
	keys[Input::Keys::JOY_DPAD_UP] = (input & HidNpadButton_Up);
	keys[Input::Keys::JOY_DPAD_DOWN] = (input & HidNpadButton_Down);
	keys[Input::Keys::JOY_DPAD_RIGHT] = (input & HidNpadButton_Right);
	keys[Input::Keys::JOY_DPAD_LEFT] = (input & HidNpadButton_Left);
	keys[Input::Keys::JOY_A] = (input & HidNpadButton_A);
	keys[Input::Keys::JOY_B] = (input & HidNpadButton_B);
	keys[Input::Keys::JOY_X] = (input & HidNpadButton_X);
	keys[Input::Keys::JOY_Y] = (input & HidNpadButton_Y);
	keys[Input::Keys::JOY_LSTICK] = (input & HidNpadButton_StickL);
	keys[Input::Keys::JOY_RSTICK] = (input & HidNpadButton_StickR);
	keys[Input::Keys::JOY_SHOULDER_LEFT] = (input & (HidNpadButton_L|HidNpadButton_AnySL)) > 0;
	keys[Input::Keys::JOY_SHOULDER_RIGHT] = (input & (HidNpadButton_R|HidNpadButton_AnySR)) > 0;
	keys[Input::Keys::JOY_BACK] = (input & HidNpadButton_Minus);
	keys[Input::Keys::JOY_START] = (input & HidNpadButton_Plus);

	HidAnalogStickState analog_left = padGetStickPos(&pad, 0);
	HidAnalogStickState analog_right = padGetStickPos(&pad, 1);

	auto normalize = [](int value) {
		return static_cast<float>(value) / JOYSTICK_MAX;
	};

	analog_input.primary.x = normalize(analog_left.x);
	analog_input.primary.y = -normalize(analog_left.y);
	analog_input.secondary.x = normalize(analog_right.x);
	analog_input.secondary.y = -normalize(analog_right.y);
	analog_input.trigger_left = (input & HidNpadButton_ZL) ? Input::AnalogInput::kMaxValue : 0.f;
	analog_input.trigger_right = (input & HidNpadButton_ZR) ? Input::AnalogInput::kMaxValue : 0.f;

	// do not handle touch when not displaying buttons or no touch happened
	if (is_docked || vcfg.touch_ui.IsLocked() || !vcfg.touch_ui.Get() || !hidGetTouchScreenStates(&touch, 1))
		return;

	for (int32_t i = 0; i < touch.count; ++i) {
		if (touch.touches[i].x < 160) {
			// shouldn't happen
			if (touch.touches[i].y < 720) {
				keys[touch_left[touch.touches[i].y / (720 / 8)]] = true;
			}
		} else if (touch.touches[i].x >= 1280 - 160) {
			if (touch.touches[i].y < 720) {
				keys[touch_right[touch.touches[i].y / (720 / 8)]] = true;
			}
		}
	}
}

void NxUi::UpdateDisplay() {
	float aspectX = 1.0f;
	float aspectY = 1.0f;

	// clear
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// touch texture
	if (!is_docked && !vcfg.touch_ui.IsLocked() && vcfg.touch_ui.Get()) {
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
	if (!vcfg.stretch.Get())
		aspectX = 0.75f;
	glUniform2f(dimsLocation, aspectX, aspectY);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// display
	eglSwapBuffers(eglDisplay, eglSurface);
}

bool NxUi::LogMessage(const std::string &message) {
	std::string m = std::string("[" GAME_TITLE "] ") + message + "\n";

	// HLE in yuzu emulator
	svcOutputDebugString(m.c_str(), m.length());

	// additional to nxlink server
	if(envHasArgv())
		return false;
	else
		return true;
}

void NxUi::ToggleStretch() {
	vcfg.stretch.Toggle();
	vcfg.touch_ui.SetLocked(vcfg.stretch.Get());
}

void NxUi::ToggleTouchUi() {
	vcfg.touch_ui.Toggle();
}

void NxUi::vGetConfig(Game_ConfigVideo& cfg) const {
	cfg.renderer.Lock("Switch NX (Software)");
	cfg.stretch.SetOptionVisible(true);
	cfg.touch_ui.SetOptionVisible(true);

	if (is_docked) {
		cfg.touch_ui.SetLocked(true);
		cfg.touch_ui.SetDescription("Only available in handheld mode");
	} else if (cfg.stretch.Get()) {
		cfg.touch_ui.SetLocked(true);
		cfg.touch_ui.SetDescription("Not available when stretch is enabled");
	}
}
