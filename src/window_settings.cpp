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
#include <sstream>
#include <utility>
#include "window_settings.h"
#include "game_config.h"
#include "input_buttons.h"
#include "keys.h"
#include "output.h"
#include "baseui.h"
#include "bitmap.h"
#include "player.h"
#include "system.h"
#include "audio.h"


Window_Settings::Window_Settings(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight) {
	column_max = 1;
}

void Window_Settings::UpdateMode() {
	auto& frame = GetFrame();
	auto mode = frame.uimode;

	if (mode == eInputRemap) {
		const Input::KeyStatus* keys;
		int& started = frame.scratch;
		int& cancel_timer = frame.scratch2;
		// Delay button reading on startup until 0 keys are pressed
		// Prevents that CONFIRM is directly detected as pressed key
		// on some platforms
		if (started == 0) {
			keys = &Input::GetAllRawPressed();
			if (keys->count() != 0) {
				return;
			}
			started = 1;
		}

		// Determine if a CANCEL key is for aborting or for mapping
		// depending on how long the key was held
		if (Input::IsPressed(Input::CANCEL)) {
			++cancel_timer;
			if (cancel_timer == 30) {
				Pop();
			}
			return;
		} else if (Input::IsReleased(Input::CANCEL)) {
			keys = &Input::GetAllRawReleased();
		} else {
			keys = &Input::GetAllRawPressed();
		}
		if (keys->count() != 1) {
			return;
		}
		for (size_t i = 0; i < keys->size(); ++i) {
			if ((*keys)[i]) {
				auto button = static_cast<Input::InputButton>(GetFrame().arg);
				auto& mappings = Input::GetInputSource()->GetButtonMappings();
				mappings.Add({button, static_cast<Input::Keys::InputKey>(i) });
				Pop();
				break;
			}
		}
	}
}

void Window_Settings::DrawOption(int index) {
	Rect rect = GetItemRect(index);
	contents->ClearRect(rect);

	auto& option = options[index];

	bool enabled = bool(option.action);
	Font::SystemColor color = enabled ? Font::ColorDefault : Font::ColorDisabled;

	contents->TextDraw(rect.x, rect.y, color, option.text);
}

Window_Settings::StackFrame& Window_Settings::GetFrame(int n) {
	auto i = stack_index - n;
	assert(i >= 0 && i < static_cast<int>(stack.size()));
	return stack[i];
}

const Window_Settings::StackFrame& Window_Settings::GetFrame(int n) const {
	auto i = stack_index - n;
	assert(i >= 0 && i < static_cast<int>(stack.size()));
	return stack[i];
}

int Window_Settings::GetStackSize() const {
	return stack_index + 1;
}

void Window_Settings::Push(UiMode ui, int arg) {
	SavePosition();

	++stack_index;
	assert(stack_index < static_cast<int>(stack.size()));
	stack[stack_index] = { ui, arg, 0, 0};

	Refresh();
	RestorePosition();
}

void Window_Settings::Pop() {
	SavePosition();
	--stack_index;
	assert(stack_index >= 0);

	Refresh();
	RestorePosition();
}

void Window_Settings::SavePosition() {
	auto mode = GetFrame().uimode;
	if (mode != eNone) {
		auto& mem = memory[mode - 1];
		mem.index = index;
		mem.top_row = GetTopRow();
	}
}

void Window_Settings::RestorePosition() {
	auto mode = GetFrame().uimode;
	if (mode != eNone) {
		auto& mem = memory[mode - 1];
		index = mem.index;
		SetTopRow(mem.top_row);
	}
}

Window_Settings::UiMode Window_Settings::GetMode() const {
	return GetFrame().uimode;
}

void Window_Settings::Refresh() {
	options.clear();

	switch (GetFrame().uimode) {
		case eNone:
		case eMain:
			break;
		case eInput:
			RefreshInput();
			break;
		case eVideo:
			RefreshVideo();
			break;
		case eAudio:
			RefreshAudio();
			break;
		case eLicense:
			RefreshLicense();
			break;
		case eInputButton:
			RefreshInputButton();
			break;
		case eInputRemap:
			RefreshInputRemap();
			break;
	}

	item_max = options.size();

	CreateContents();

	if (GetFrame().uimode == eNone || options.empty()) {
		SetIndex(-1);
	} else {
		SetIndex(index);
	}

	contents->Clear();

	for (int i = 0; i < item_max; ++i) {
		DrawOption(i);
	}
}

void Window_Settings::UpdateHelp() {
	if (index >= 0 && index < static_cast<int>(options.size())) {
		help_window->SetText(options[index].help);
	} else {
		help_window->SetText("");
	}
}

template <typename Param, typename Action>
void Window_Settings::AddOption(const std::string& prefix,
		const Param& param,
		Action&& action,
		const std::string& help)
{
	if (!param.Enabled()) {
		return;
	}
	Option opt;
	opt.text = prefix;
	opt.help = help;
	if (!param.Locked()) {
		opt.action = std::forward<Action>(action);
	}
	options.push_back(std::move(opt));
}

template <typename Param, typename Action>
void Window_Settings::AddOption(const Param& param,
		Action&& action)
{
	if (!param.Enabled()) {
		return;
	}
	Option opt;
	opt.text = ToString(param.GetName());
	opt.help = ToString(param.GetDescription());
	if (!param.Locked()) {
		opt.action = std::forward<Action>(action);
	}
	options.push_back(std::move(opt));
}

void Window_Settings::RefreshVideo() {
	auto cfg = DisplayUi->GetConfig();

	AddOption(cfg.renderer,	[](){});
	AddOption(cfg.fullscreen, [](){ DisplayUi->ToggleFullscreen(); });
	AddOption(cfg.window_zoom, [](){ DisplayUi->ToggleZoom(); });
	AddOption(cfg.vsync, [](){});
	AddOption(cfg.fps_limit, [](){});
	AddOption(cfg.show_fps, [](){ DisplayUi->ToggleShowFps(); });
	AddOption(cfg.scaling_mode, [](){ /*DisplayUi->SetScalingMode();*/ });
}

void Window_Settings::RefreshAudio() {
	auto cfg = DisplayUi->GetAudio().GetConfig();

	AddOption(cfg.music_volume,	[](){});
	AddOption(cfg.sound_volume, [](){});
	/*AddOption("Midi Backend", LockedConfigParam<std::string>("Unknown"), "",
			[](){},
			"Which MIDI backend to use");
	AddOption("Midi Soundfont", LockedConfigParam<std::string>("Default"), "",
			[](){},
			"Which MIDI soundfont to use");*/
}

void Window_Settings::RefreshLicense() {
	AddOption("EasyRPG Player", ConfigParam<std::string>("", "", "GPLv3+"), [](){}, "The engine you are using :)");
	AddOption("liblcf", ConfigParam<std::string>("", "", "MIT"), [](){}, "Handles RPG Maker 2000/2003 and EasyRPG projects");
	AddOption("libpng", ConfigParam<std::string>("", "", "zlib"), [](){}, "For reading and writing PNG image files");
	AddOption("zlib", ConfigParam<std::string>("", "", "zlib"), [](){}, "Implements deflate used in ZIP archives and PNG images");
	AddOption("Pixman", ConfigParam<std::string>("", "", "MIT"), [](){}, "Pixel-manipulation library");
	AddOption("fmtlib", ConfigParam<std::string>("", "", "BSD"), [](){}, "Text formatting library");
	// No way to detect them - Used by liblcf
	AddOption("expat", ConfigParam<std::string>("", "", "MIT"), [](){}, "XML parser");
	AddOption("ICU", ConfigParam<std::string>("", "", "ICU"), [](){}, "Unicode library");
#if USE_SDL == 1
	AddOption("SDL", ConfigParam<std::string>("", "", "LGPLv2.1+"), [](){}, "Abstraction layer for graphic, audio, input and more");
#endif
#if USE_SDL == 2
	AddOption("SDL2", ConfigParam<std::string>("", "", "zlib"), [](){}, "Abstraction layer for graphic, audio, input and more");
#endif
#ifdef HAVE_FREETYPE
	AddOption("Freetype", ConfigParam<std::string>("", "", "Freetype"), [](){}, "Font parsing and rasterization library");
#endif
#ifdef HAVE_HARFBUZZ
	AddOption("Harfbuzz", ConfigParam<std::string>("", "", "MIT"), [](){}, "Text shaping engine");
#endif
#ifdef SUPPORT_AUDIO
	// Always shown because the Midi synth is compiled in
	AddOption("FmMidi", ConfigParam<std::string>("", "", "BSD"), [](){}, "MIDI file parser and Yamaha YM2608 FM synthesizer");
#ifdef HAVE_LIBMPG123
	AddOption("mpg123", ConfigParam<std::string>("", "", "LGPLv2.1+"), [](){}, "Decodes MPEG Audio Layer 1, 2 and 3");
#endif
#ifdef HAVE_LIBSNDFILE
	AddOption("libsndfile", ConfigParam<std::string>("", "", "LGPLv2.1+"), [](){}, "Decodes sampled audio data (WAV)");
#endif
#ifdef HAVE_OGGVORBIS
	AddOption("ogg", ConfigParam<std::string>("", "", "BSD"), [](){}, "Ogg container format library");
	AddOption("vorbis", ConfigParam<std::string>("", "", "BSD"), [](){}, "Decodes the free Ogg Vorbis audio codec");
#endif
#ifdef HAVE_TREMOR
	AddOption("tremor", ConfigParam<std::string>("", "", "BSD"), [](){}, "Decodes the free Ogg Vorbis audio format");
#endif
#ifdef HAVE_OPUS
	AddOption("opus", ConfigParam<std::string>("", "", "BSD"), [](){}, "Decodes the free OPUS audio codec");
#endif
#ifdef HAVE_WILDMIDI
	AddOption("WildMidi", ConfigParam<std::string>("", "", "LGPLv3+"), [](){}, "MIDI synthesizer");
#endif
#ifdef HAVE_FLUIDSYNTH
	AddOption("FluidSynth", ConfigParam<std::string>("", "", "LGPLv2.1+"), [](){}, "MIDI synthesizer supporting SoundFont 2");
#endif
#ifdef HAVE_FLUIDLITE
	AddOption("FluidLite", ConfigParam<std::string>("", "", "LGPLv2.1+"), [](){}, "MIDI synthesizer supporting SoundFont 2 (lite version)");
#endif
#ifdef HAVE_XMP
	AddOption("xmp-lite", ConfigParam<std::string>("", "", "MIT"), [](){}, "Module (MOD, S3M, XM and IT) synthesizer");
#endif
#ifdef HAVE_LIBSPEEXDSP
	AddOption("speexdsp", ConfigParam<std::string>("", "", "BSD"), [](){}, "Audio resampler");
#endif
#ifdef HAVE_LIBSAMPLERATE
	AddOption("samplerate", ConfigParam<std::string>("", "", "BSD"), [](){}, "Audio resampler");
#endif
#ifdef WANT_DRWAV
	AddOption("dr_wav", ConfigParam<std::string>("", "", "MIT-0"), [](){}, "Decodes sampled audio data (WAV)");
#endif
#ifdef HAVE_ALSA
	AddOption("ALSA", ConfigParam<std::string>("", "", "LGPL2.1+"), [](){}, "Linux sound support (used for MIDI playback)");
#endif
#endif
	AddOption("rang", ConfigParam<std::string>("", "", "Unlicense"), [](){}, "Colors the terminal output");
#ifdef _WIN32
	AddOption("dirent", ConfigParam<std::string>("", "", "MIT"), [](){}, "Dirent interface for Microsoft Visual Studio");
#endif
	AddOption("Baekmuk", ConfigParam<std::string>("", "", "Baekmuk"), [](){}, "Korean font family");
	AddOption("Shinonome", ConfigParam<std::string>("", "", "Public Domain"), [](){}, "Japanese font family");
	AddOption("ttyp0", ConfigParam<std::string>("", "", "ttyp0"), [](){}, "ttyp0 font family");
	AddOption("WenQuanYi", ConfigParam<std::string>("", "", "GPLv2+ with FE"), [](){}, "WenQuanYi font family (CJK)");
#ifdef EMSCRIPTEN
	AddOption("PicoJSON", ConfigParam<std::string>("", "", "BSD"), [](){}, "JSON parser/serializer");
	AddOption("Teenyicons", ConfigParam<std::string>("", "", "MIT"), [](){}, "Tiny minimal 1px icons");
#endif
}

void Window_Settings::RefreshInput() {
	auto& mappings = Input::GetInputSource()->GetButtonMappings();
	for (int i = 0; i < Input::BUTTON_COUNT; ++i) {
		auto button = static_cast<Input::InputButton>(i);

		auto name = Input::kButtonNames.tag(button);
		auto help = Input::kButtonHelp.tag(button);

		std::stringstream ss;
		for (auto ki = mappings.LowerBound(button); ki != mappings.end() && ki->first == button;++ki) {
			auto key = static_cast<Input::Keys::InputKey>(ki->second);
			auto kname = Input::Keys::kNames.tag(key);
			ss << kname << " ";
		}

		auto param = ConfigParam<std::string>(name, help, ss.str());
		std::string prefix = std::string("Key ") + name;
		AddOption(param,
				[this, button, prefix](){
				Push(eInputButton, static_cast<int>(button));
			});
	}
}

void Window_Settings::RefreshInputButton() {
	auto button = static_cast<Input::InputButton>(GetFrame().arg);

	auto& mappings = Input::GetInputSource()->GetButtonMappings();

	auto name = Input::kButtonNames.tag(button);
	auto help = Input::kButtonHelp.tag(button);
	std::string key_label = fmt::format("{}: {}", name, help);

	// Protect buttons where unmapping makes the Player unusable
	auto def_mappings = Input::GetDefaultButtonMappings();
	auto protected_buttons = std::array<std::pair<Input::InputButton, Input::Keys::InputKey>, 7> {{
		{Input::InputButton::UP,            Input::Keys::InputKey::NONE},
		{Input::InputButton::DOWN,          Input::Keys::InputKey::NONE},
		{Input::InputButton::LEFT,          Input::Keys::InputKey::NONE},
		{Input::InputButton::RIGHT,         Input::Keys::InputKey::NONE},
		{Input::InputButton::DECISION,      Input::Keys::InputKey::NONE},
		{Input::InputButton::CANCEL,        Input::Keys::InputKey::NONE},
		{Input::InputButton::SETTINGS_MENU, Input::Keys::InputKey::NONE}
	}};
	for (auto& p: protected_buttons) {
		for (auto ki = def_mappings.LowerBound(button); ki != def_mappings.end() && ki->first == button; ++ki) {
			p.second = ki->second;
			break;
		}
	}

	std::stringstream ss;
	for (auto ki = mappings.LowerBound(button); ki != mappings.end() && ki->first == button;++ki) {
		auto key = static_cast<Input::Keys::InputKey>(ki->second);
		auto kname = Input::Keys::kNames.tag(key);

		if (std::find(protected_buttons.begin(), protected_buttons.end(), *ki) != protected_buttons.end()) {
			AddOption(kname, LockedConfigParam<std::string>("", "", ""), [](){
			}, key_label);
		} else {
			AddOption(kname, ConfigParam<std::string>("", "", ""), [=]() mutable {
				Input::GetInputSource()->GetButtonMappings().Remove({button, key});
				Refresh();
			}, key_label);
		}
	}

	AddOption("<Add new>", ConfigParam<std::string>("", "", ""), [=](){
		Push(eInputRemap, static_cast<int>(button));
	}, key_label);
	AddOption("<Reset>", ConfigParam<std::string>("", "", ""), [=](){
		Input::ResetDefaultMapping(button);
		Refresh();
	}, key_label);
}

void Window_Settings::RefreshInputRemap() {
	auto button = static_cast<Input::InputButton>(GetFrame().arg);
	auto name = Input::kButtonNames.tag(button);

	AddOption("Press the new key", ConfigParam<std::string>("", "", ""), [](){}, fmt::format("Remapping {}", name));
	AddOption("Hold CANCEL to abort", ConfigParam<std::string>("", "", ""), [](){}, "");
}
