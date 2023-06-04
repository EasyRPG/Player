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
#include "game_map.h"
#include "input.h"
#include "text.h"
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

class MenuItem final : public ConfigParam<StringView> {
public:
    explicit MenuItem(StringView name, StringView description, StringView value) :
		ConfigParam<StringView>(name, description, "", "", value) {
	}
};

Window_Settings::Window_Settings(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight) {
	column_max = 1;
}

void Window_Settings::DrawOption(int index) {
	Rect rect = GetItemRect(index);
	contents->ClearRect(rect);

	auto& option = options[index];

	bool enabled = bool(option.action);
	Font::SystemColor color = enabled ? Font::ColorDefault : Font::ColorDisabled;

	contents->TextDraw(rect, color, option.text);
	contents->TextDraw(rect, color, option.value_text, Text::AlignRight);
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
		case eEngine:
			RefreshEngine();
			break;
		case eLicense:
			RefreshLicense();
			break;
		case eInputButtonCategory:
			RefreshButtonCategory();
			break;
		case eInputListButtonsGame:
		case eInputListButtonsEngine:
		case eInputListButtonsDeveloper:
			RefreshButtonList();
			break;
		default:
			break;
	}

	SetItemMax(options.size());

	if (GetFrame().uimode == eNone || options.empty()) {
		SetIndex(-1);
	}

	CreateContents();

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
void Window_Settings::AddOption(const Param& param,
	Action&& action)
{
	if (!param.IsOptionVisible()) {
		return;
	}
	Option opt;
	opt.text = ToString(param.GetName());
	opt.help = ToString(param.GetDescription());
	opt.value_text = param.ValueToString();
	opt.mode = eOptionNone;
	if (!param.IsLocked()) {
		opt.action = std::forward<Action>(action);
	}
	options.push_back(std::move(opt));
}

template <typename T, typename Action>
void Window_Settings::AddOption(const RangeConfigParam<T>& param,
		Action&& action
	) {
	if (!param.IsOptionVisible()) {
		return;
	}
	Option opt;
	opt.text = ToString(param.GetName());
	opt.help = ToString(param.GetDescription());
	opt.value_text = param.ValueToString();
	opt.mode = eOptionRangeInput;
	opt.current_value = static_cast<int>(param.Get());
	opt.original_value = opt.current_value;
	opt.min_value = param.GetMin();
	opt.max_value = param.GetMax();
	if (!param.IsLocked()) {
		opt.action = std::forward<Action>(action);
	}
	options.push_back(std::move(opt));
}

template <typename T, typename Action, size_t S>
void Window_Settings::AddOption(const EnumConfigParam<T, S>& param,
			Action&& action
	) {
	if (!param.IsOptionVisible()) {
		return;
	}
	Option opt;
	opt.text = ToString(param.GetName());
	opt.help = ToString(param.GetDescription());
	opt.value_text = param.ValueToString();
	opt.mode = eOptionPicker;
	opt.current_value = static_cast<int>(param.Get());
	opt.original_value = opt.current_value;
	int idx = 0;
	for (auto& s: param.GetValues()) {
		if (param.IsValid(static_cast<T>(idx))) {
			opt.options_text.push_back(ToString(s));
			opt.options_index.push_back(idx);
		}
		++idx;
	}

	idx = 0;
	for (auto& s: param.GetDescriptions()) {
		if (param.IsValid(static_cast<T>(idx))) {
			opt.options_help.push_back(ToString(s));
		}
		++idx;
	}
	if (!param.IsLocked()) {
		opt.action = std::forward<Action>(action);
	}
	options.push_back(std::move(opt));
}

void Window_Settings::RefreshVideo() {
	auto cfg = DisplayUi->GetConfig();

	AddOption(cfg.renderer,	[](){});
	AddOption(cfg.fullscreen, [](){ DisplayUi->ToggleFullscreen(); });
	AddOption(cfg.window_zoom, [](){ DisplayUi->ToggleZoom(); });
	AddOption(cfg.vsync, [](){ DisplayUi->ToggleVsync(); });
	AddOption(cfg.fps_limit, [this](){ DisplayUi->SetFrameLimit(GetCurrentOption().current_value); });
	AddOption(cfg.show_fps, [](){ DisplayUi->ToggleShowFps(); });
	AddOption(cfg.fps_render_window, [](){ DisplayUi->ToggleShowFpsOnTitle(); });
	AddOption(cfg.stretch, []() { DisplayUi->ToggleStretch(); });
	AddOption(cfg.scaling_mode, [this](){ DisplayUi->SetScalingMode(static_cast<ScalingMode>(GetCurrentOption().current_value)); });
	AddOption(cfg.touch_ui, [](){ DisplayUi->ToggleTouchUi(); });
	AddOption(cfg.game_resolution, [this]() { DisplayUi->SetGameResolution(static_cast<GameResolution>(GetCurrentOption().current_value)); });
}

void Window_Settings::RefreshAudio() {
	auto cfg = DisplayUi->GetAudio().GetConfig();

	AddOption(cfg.music_volume, [this](){ DisplayUi->GetAudio().BGM_SetGlobalVolume(GetCurrentOption().current_value); });
	AddOption(cfg.sound_volume, [this](){ DisplayUi->GetAudio().SE_SetGlobalVolume(GetCurrentOption().current_value); });
	/*AddOption("Midi Backend", LockedConfigParam<std::string>("Unknown"), "",
			[](){},
			"Which MIDI backend to use");
	AddOption("Midi Soundfont", LockedConfigParam<std::string>("Default"), "",
			[](){},
			"Which MIDI soundfont to use");*/
}

void Window_Settings::RefreshEngine() {
	auto& cfg = Player::player_config;

	// FIXME: Binding &cfg is not needed and generates a warning but requires it
#ifdef _MSC_VER
	AddOption(cfg.settings_autosave, [&cfg](){ cfg.settings_autosave.Toggle(); });
	AddOption(cfg.settings_in_title, [&cfg](){ cfg.settings_in_title.Toggle(); });
	AddOption(cfg.settings_in_menu, [&cfg](){ cfg.settings_in_menu.Toggle(); });
#else
	AddOption(cfg.settings_autosave, [](){ cfg.settings_autosave.Toggle(); });
	AddOption(cfg.settings_in_title, [](){ cfg.settings_in_title.Toggle(); });
	AddOption(cfg.settings_in_menu, [](){ cfg.settings_in_menu.Toggle(); });
#endif
}

void Window_Settings::RefreshLicense() {
	AddOption(MenuItem("EasyRPG Player", "The engine you are using :)", "GPLv3+"), [this](){
		Push(eAbout);
	});
	AddOption(MenuItem("liblcf", "Handles RPG Maker 2000/2003 and EasyRPG projects", "MIT"), [](){});
	AddOption(MenuItem("libpng", "For reading and writing PNG image files", "zlib"), [](){});
	AddOption(MenuItem("zlib", "Implements deflate used in ZIP archives and PNG images", "zlib"), [](){});
	AddOption(MenuItem("Pixman", "Pixel-manipulation library", "MIT"), [](){});
	AddOption(MenuItem("fmtlib", "Text formatting library", "BSD"), [](){});
	// No way to detect them - Used by liblcf
	AddOption(MenuItem("expat", "XML parser", "MIT"), [](){});
	AddOption(MenuItem("ICU", "Unicode library", "ICU"), [](){});
#if USE_SDL == 1
	AddOption(MenuItem("SDL", "Abstraction layer for graphic, audio, input and more", "LGPLv2.1+"), [](){});
#endif
#if USE_SDL == 2
	AddOption(MenuItem("SDL2", "Abstraction layer for graphic, audio, input and more", "zlib"), [](){});
#endif
#ifdef HAVE_FREETYPE
	AddOption(MenuItem("Freetype", "Font parsing and rasterization library", "Freetype"), [](){});
#endif
#ifdef HAVE_HARFBUZZ
	AddOption(MenuItem("Harfbuzz", "Text shaping engine", "MIT"), [](){});
#endif
#ifdef SUPPORT_AUDIO
	// Always shown because the Midi synth is compiled in
	AddOption(MenuItem("FmMidi", "MIDI file parser and Yamaha YM2608 FM synthesizer", "BSD"), [](){});
#ifdef HAVE_LIBMPG123
	AddOption(MenuItem("mpg123", "Decodes MPEG Audio Layer 1, 2 and 3", "LGPLv2.1+"), [](){});
#endif
#ifdef HAVE_LIBSNDFILE
	AddOption(MenuItem("libsndfile", "Decodes sampled audio data (WAV)", "LGPLv2.1+"), [](){});
#endif
#ifdef HAVE_OGGVORBIS
	AddOption(MenuItem("ogg", "Ogg container format library", "BSD"), [](){});
	AddOption(MenuItem("vorbis", "Decodes the free Ogg Vorbis audio codec", "BSD"), [](){});
#endif
#ifdef HAVE_TREMOR
	AddOption(MenuItem("tremor", "Decodes the free Ogg Vorbis audio format", "BSD"), [](){});
#endif
#ifdef HAVE_OPUS
	AddOption(MenuItem("opus", "Decodes the free OPUS audio codec", "BSD"), [](){});
#endif
#ifdef HAVE_WILDMIDI
	AddOption(MenuItem("WildMidi", "MIDI synthesizer", "LGPLv3+"), [](){});
#endif
#ifdef HAVE_FLUIDSYNTH
	AddOption(MenuItem("FluidSynth", "MIDI synthesizer supporting SoundFont 2", "LGPLv2.1+"), [](){});
#endif
#ifdef HAVE_FLUIDLITE
	AddOption(MenuItem("FluidLite", "MIDI synthesizer supporting SoundFont 2 (lite version)", "LGPLv2.1+"), [](){});
#endif
#ifdef HAVE_XMP
	AddOption(MenuItem("xmp-lite", "Module (MOD, S3M, XM and IT) synthesizer", "MIT"), [](){});
#endif
#ifdef HAVE_LIBSPEEXDSP
	AddOption(MenuItem("speexdsp", "Audio resampler", "BSD"), [](){});
#endif
#ifdef HAVE_LIBSAMPLERATE
	AddOption(MenuItem("samplerate", "Audio resampler", "BSD"), [](){});
#endif
#ifdef WANT_DRWAV
	AddOption(MenuItem("dr_wav", "Decodes sampled audio data (WAV)", "MIT-0"), [](){});
#endif
#ifdef HAVE_ALSA
	AddOption(MenuItem("ALSA", "Linux sound support (used for MIDI playback)", "LGPL2.1+"), [](){});
#endif
#endif
	AddOption(MenuItem("rang", "Colors the terminal output", "Unlicense"), [](){});
#ifdef _WIN32
	AddOption(MenuItem("dirent", "Dirent interface for Microsoft Visual Studio", "MIT"), [](){});
#endif
	AddOption(MenuItem("Baekmuk", "Korean font family", "Baekmuk"), [](){});
	AddOption(MenuItem("Shinonome", "Japanese font family", "Public Domain"), [](){});
	AddOption(MenuItem("ttyp0", "ttyp0 font family", "ttyp0"), [](){});
	AddOption(MenuItem("WenQuanYi", "WenQuanYi font family (CJK)", "GPLv2+ with FE"), [](){});
#ifdef EMSCRIPTEN
	AddOption(MenuItem("PicoJSON", "JSON parser/serializer", "BSD"), [](){});
	AddOption(MenuItem("Teenyicons", "Tiny minimal 1px icons", "MIT"), [](){});
#endif
}

void Window_Settings::RefreshInput() {
	Game_ConfigInput& cfg = Input::GetInputSource()->GetConfig();

	AddOption(MenuItem("Key/Button mapping", "Change the keybindings", ""),
		[this]() { Push(eInputButtonCategory); });
	AddOption(cfg.gamepad_swap_ab_and_xy, [&cfg](){ cfg.gamepad_swap_ab_and_xy.Toggle(); Input::ResetTriggerKeys(); });
	AddOption(cfg.gamepad_swap_analog, [&cfg](){ cfg.gamepad_swap_analog.Toggle(); Input::ResetTriggerKeys(); });
	AddOption(cfg.gamepad_swap_dpad_with_buttons, [&cfg](){ cfg.gamepad_swap_dpad_with_buttons.Toggle(); Input::ResetTriggerKeys(); });
}

void Window_Settings::RefreshButtonCategory() {
	AddOption(MenuItem("Game", "Buttons used by games", ""),
		[this]() { Push(eInputListButtonsGame, 0); });
	AddOption(MenuItem("Engine", "Buttons to access engine features", ""),
		[this]() { Push(eInputListButtonsEngine, 1); });
	AddOption(MenuItem("Developer", "Buttons useful for developers", ""),
		[this]() { Push(eInputListButtonsDeveloper, 2); });
}

void Window_Settings::RefreshButtonList() {
	auto& mappings = Input::GetInputSource()->GetButtonMappings();
	auto custom_names = Input::GetInputKeyNames();

	std::vector<Input::InputButton> buttons;
	switch (GetFrame().arg) {
		case 0:
			buttons = {	Input::UP, Input::DOWN, Input::LEFT, Input::RIGHT, Input::DECISION, Input::CANCEL, Input::SHIFT,
				Input::N0, Input::N1, Input::N2, Input::N3, Input::N4, Input::N5, Input::N6, Input::N7, Input::N8, Input::N9,
				Input::PLUS, Input::MINUS, Input::MULTIPLY, Input::DIVIDE, Input::PERIOD, Input::MOUSE_LEFT,
				Input::MOUSE_MIDDLE, Input::MOUSE_RIGHT, Input::SCROLL_UP, Input::SCROLL_DOWN };
			break;
		case 1:
			buttons = {Input::SETTINGS_MENU, Input::TOGGLE_FPS, Input::TOGGLE_FULLSCREEN, Input::TOGGLE_ZOOM,
				Input::TAKE_SCREENSHOT, Input::RESET, Input::FAST_FORWARD, Input::FAST_FORWARD_PLUS,
				Input::PAGE_UP, Input::PAGE_DOWN };
			break;
		case 2:
			buttons = {	Input::DEBUG_MENU, Input::DEBUG_THROUGH, Input::DEBUG_SAVE, Input::DEBUG_ABORT_EVENT,
				Input::SHOW_LOG };
			break;
	}

	for (auto b: buttons) {
		auto button = static_cast<Input::InputButton>(b);

		std::string name = Input::kButtonNames.tag(button);

		// Improve readability of the names
		bool first_letter = true;
		for (size_t i = 0; i < name.size(); ++i) {
			auto& ch = name[i];
			if (ch >= 'A' && ch <= 'Z') {
				if (!first_letter) {
					ch += 32;
				}
				first_letter = false;
			} else if (ch == '_') {
				ch = ' ';
				first_letter = true;
			}
		}

		auto help = Input::kButtonHelp.tag(button);
		std::string value = "";

		// Append as many buttons as fit on the screen, then add ...
		int contents_w = GetContents()->width();
		int name_size = Text::GetSize(*Font::Default(), name).width;
		int value_size = 0;

		for (auto ki = mappings.LowerBound(button); ki != mappings.end() && ki->first == button; ++ki) {
			auto custom_name = std::find_if(custom_names.begin(), custom_names.end(), [&](auto& key_pair) {
				return key_pair.first == ki->second;
			});

			std::string cur_value;
			if (custom_name != custom_names.end()) {
				cur_value = custom_name->second;
			} else {
				cur_value = Input::Keys::kNames.tag(ki->second);
			}

			int cur_value_size = Text::GetSize(*Font::Default(), cur_value + ",").width;

			if (value.empty()) {
				value = cur_value;
			} else if (name_size + value_size + cur_value_size + 24 > contents_w) {
				value += ",…";
				break;
			} else {
				value += "," + cur_value;
			}

			value_size += cur_value_size;
		}

		auto param = MenuItem(name, help, value);
		AddOption(param,
				[this, button](){
				Push(eInputButtonOption, static_cast<int>(button));
			});
	}
}
