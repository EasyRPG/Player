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
#include "game_system.h"
#include "input_buttons.h"
#include "keys.h"
#include "output.h"
#include "baseui.h"
#include "bitmap.h"
#include "player.h"
#include "system.h"
#include "audio.h"
#include "audio_midi.h"
#include "audio_generic_midiout.h"

#ifdef EMSCRIPTEN
#  include "platform/emscripten/interface.h"
#endif

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

	auto& option = GetFrame().options[index];

	bool enabled = bool(option.action);
	Font::SystemColor color = enabled ? option.color : Font::ColorDisabled;

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
	stack[stack_index] = { ui, arg, 0, 0, {}};

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
	GetFrame().options.clear();

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
		case eAudioMidi:
			RefreshAudioMidi();
			break;
		case eAudioSoundfont:
			RefreshAudioSoundfont();
			break;
		case eEngine:
			RefreshEngine();
			break;
		case eEngineFont1:
			RefreshEngineFont(false);
			break;
		case eEngineFont2:
			RefreshEngineFont(true);
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

	SetItemMax(GetFrame().options.size());

	if (GetFrame().uimode == eNone || GetFrame().options.empty()) {
		SetIndex(-1);
	}

	CreateContents();

	contents->Clear();

	for (int i = 0; i < item_max; ++i) {
		DrawOption(i);
	}
}

void Window_Settings::UpdateHelp() {
	if (index >= 0 && index < static_cast<int>(GetFrame().options.size())) {
		help_window->SetText(GetFrame().options[index].help);
		if (help_window2) {
			help_window2->SetText(GetFrame().options[index].help2);
			help_window2->SetVisible(!GetFrame().options[index].help2.empty());
		}
	} else {
		help_window->SetText("");
		if (help_window2) {
			help_window2->SetVisible(false);
		}
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
	GetFrame().options.push_back(std::move(opt));
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
	GetFrame().options.push_back(std::move(opt));
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
	GetFrame().options.push_back(std::move(opt));
}

void Window_Settings::RefreshVideo() {
	auto cfg = DisplayUi->GetConfig();

	AddOption(cfg.renderer,	[](){});
	AddOption(cfg.fullscreen, [](){ DisplayUi->ToggleFullscreen(); });
	AddOption(cfg.window_zoom, [](){ DisplayUi->ToggleZoom(); });
	AddOption(cfg.fps, [this](){ DisplayUi->SetShowFps(static_cast<ConfigEnum::ShowFps>(GetCurrentOption().current_value)); });
	AddOption(cfg.vsync, [](){ DisplayUi->ToggleVsync(); });
	AddOption(cfg.fps_limit, [this](){ DisplayUi->SetFrameLimit(GetCurrentOption().current_value); });
	AddOption(cfg.stretch, []() { DisplayUi->ToggleStretch(); });
	AddOption(cfg.scaling_mode, [this](){ DisplayUi->SetScalingMode(static_cast<ConfigEnum::ScalingMode>(GetCurrentOption().current_value)); });
	AddOption(cfg.pause_when_focus_lost, [cfg]() mutable { DisplayUi->SetPauseWhenFocusLost(cfg.pause_when_focus_lost.Toggle()); });
	AddOption(cfg.touch_ui, [](){ DisplayUi->ToggleTouchUi(); });
	AddOption(cfg.game_resolution, [this]() { DisplayUi->SetGameResolution(static_cast<ConfigEnum::GameResolution>(GetCurrentOption().current_value)); });
}

void Window_Settings::RefreshAudio() {
	auto cfg = Audio().GetConfig();

	AddOption(cfg.music_volume, [this](){ Audio().BGM_SetGlobalVolume(GetCurrentOption().current_value); });
	AddOption(cfg.sound_volume, [this](){ Audio().SE_SetGlobalVolume(GetCurrentOption().current_value); });
	if (cfg.fluidsynth_midi.IsOptionVisible() || cfg.wildmidi_midi.IsOptionVisible() || cfg.native_midi.IsOptionVisible() || cfg.fmmidi_midi.IsOptionVisible()) {
		AddOption(MenuItem("MIDI drivers", "Configure MIDI playback", ""), [this]() { Push(eAudioMidi); });
	}
	AddOption(cfg.soundfont, [this](){ Push(eAudioSoundfont); });
}

void Window_Settings::RefreshAudioMidi() {
	auto cfg = Audio().GetConfig();

	bool used = false;

	if (cfg.fluidsynth_midi.IsOptionVisible()) {
		AddOption(cfg.fluidsynth_midi, []() { Audio().SetFluidsynthEnabled(Audio().GetConfig().fluidsynth_midi.Toggle()); });
		if (!MidiDecoder::CheckFluidsynth(GetFrame().options.back().help2)) {
			GetFrame().options.back().text += " [Not working]";
			GetFrame().options.back().color = Font::ColorKnockout;
		} else if (cfg.fluidsynth_midi.Get()) {
			GetFrame().options.back().text += " [In use]";
			used = true;
		}
	}

	if (cfg.wildmidi_midi.IsOptionVisible()) {
		AddOption(cfg.wildmidi_midi, []() { Audio().SetWildMidiEnabled(Audio().GetConfig().wildmidi_midi.Toggle()); });
		if (!MidiDecoder::CheckWildMidi(GetFrame().options.back().help2)) {
			GetFrame().options.back().text += " [Not working]";
			GetFrame().options.back().color = Font::ColorKnockout;
		} else if (cfg.wildmidi_midi.Get() && !used) {
			GetFrame().options.back().text += " [In use]";
			used = true;
		}
	}

	if (cfg.native_midi.IsOptionVisible()) {
		AddOption(cfg.native_midi, []() { Audio().SetNativeMidiEnabled(Audio().GetConfig().native_midi.Toggle()); });
		auto midi_out = Audio().CreateAndGetMidiOut();
		if (!midi_out || !midi_out->IsInitialized(GetFrame().options.back().help2)) {
			GetFrame().options.back().text += " [Not working]";
			GetFrame().options.back().color = Font::ColorKnockout;
		} else if (cfg.native_midi.Get() && !used) {
			GetFrame().options.back().text += " [In use]";
			used = true;
		}
	}

	if (cfg.fmmidi_midi.IsOptionVisible()) {
		AddOption(cfg.fmmidi_midi, []() {});
		if (!used) {
			GetFrame().options.back().text += " [In use]";
		}
	}

	AddOption(MenuItem("> Information <", "The first active and working option is used for MIDI", ""), [](){});
	GetFrame().options.back().help2 = "Changes take effect when a new MIDI file is played";
}

void Window_Settings::RefreshAudioSoundfont() {
	auto fs = Game_Config::GetSoundfontFilesystem();

	if (!fs) {
		Pop();
	}

	fs.ClearCache();

	auto acfg = Audio().GetConfig();
	AddOption(MenuItem("<Autodetect>", "Attempt to find a suitable soundfont automatically", acfg.soundfont.Get().empty() ? "[x]" : ""), [this]() {
		Audio().SetFluidsynthSoundfont({});
		Pop();
	});

	auto list = fs.ListDirectory();
	assert(list);

	std::string sf_lower = Utils::LowerCase(Audio().GetFluidsynthSoundfont());
	for (const auto& item: *list) {
		if (item.second.type == DirectoryTree::FileType::Regular && (StringView(item.first).ends_with(".sf2") || StringView(item.first).ends_with(".soundfont"))) {
			AddOption(MenuItem(item.second.name, "Use this custom soundfont", StringView(sf_lower).ends_with(item.first) ? "[x]" : ""), [this, fs, item]() {
				Audio().SetFluidsynthSoundfont(FileFinder::MakePath(fs.GetFullPath(), item.second.name));
				Pop();
			});
		}
	}

	for (auto& opt: GetFrame().options) {
		opt.help2 = "Changes take effect when a new MIDI file is played";
	}

#ifdef EMSCRIPTEN
	AddOption(MenuItem("<Upload Soundfont>", "Provide a soundfont from your system", ""), [fs]() { Emscripten_Interface::UploadSoundfont(); });
#elif defined(SUPPORT_FILE_BROWSER)
	AddOption(MenuItem("<Open Soundfont directory>", "Open the soundfont directory in a file browser", ""), [fs]() { DisplayUi->OpenURL(fs.GetFullPath()); });
#endif
}

#ifdef __clang__
// FIXME: Binding &cfg in the lambdas below is not needed and generates a warning in clang but MSVC requires it
#pragma clang diagnostic ignored "-Wunused-lambda-capture"
#endif

void Window_Settings::RefreshEngine() {
	auto& cfg = Player::player_config;
	cfg.Hide();

	AddOption(cfg.font1, [this, &cfg]() {
		font_size.Set(cfg.font1_size.Get());
		Push(eEngineFont1);
		GetFrame().scratch = -1;
	});
	if (cfg.font1.IsOptionVisible()) {
		if (cfg.font1.IsLocked()) {
			GetFrame().options.back().help = "This game uses a custom font";
		}
		if (Main_Data::game_system->GetFontId() == lcf::rpg::System::Font_gothic) {
			GetFrame().options.back().text += " [In use]";
		}
	}

	AddOption(cfg.font2, [this, &cfg]() {
		font_size.Set(cfg.font2_size.Get());
		Push(eEngineFont2);
		GetFrame().scratch = -1;
	});
	if (cfg.font2.IsOptionVisible()) {
		if (cfg.font2.IsLocked()) {
			GetFrame().options.back().help = "This game uses a custom font";
		}
		if (Main_Data::game_system->GetFontId() == lcf::rpg::System::Font_mincho) {
			GetFrame().options.back().text += " [In use]";
		}
	}

	AddOption(cfg.show_startup_logos, [this, &cfg](){ cfg.show_startup_logos.Set(static_cast<ConfigEnum::StartupLogos>(GetCurrentOption().current_value)); });
	AddOption(cfg.settings_autosave, [&cfg](){ cfg.settings_autosave.Toggle(); });
	AddOption(cfg.settings_in_title, [&cfg](){ cfg.settings_in_title.Toggle(); });
	AddOption(cfg.settings_in_menu, [&cfg](){ cfg.settings_in_menu.Toggle(); });
	AddOption(cfg.lang_select_on_start, [this, &cfg]() { cfg.lang_select_on_start.Set(static_cast<ConfigEnum::StartupLangSelect>(GetCurrentOption().current_value)); });
	AddOption(cfg.lang_select_in_title, [&cfg](){ cfg.lang_select_in_title.Toggle(); });
	AddOption(cfg.log_enabled, [&cfg]() { cfg.log_enabled.Toggle(); });
	AddOption(cfg.screenshot_scale, [this, &cfg](){ cfg.screenshot_scale.Set(GetCurrentOption().current_value); });

	GetFrame().options.back().help2 = fmt::format("Screenshot size: {}x{}",
		Player::screen_width * cfg.screenshot_scale.Get(), Player::screen_height * cfg.screenshot_scale.Get());
}

void Window_Settings::RefreshEngineFont(bool mincho) {
	auto fs = Game_Config::GetFontFilesystem();

	if (!fs) {
		Pop();
	}

	fs.ClearCache();

	auto& cfg = Player::player_config;

	auto& setting = mincho ? cfg.font2 : cfg.font1;

	auto set_help2 = [this]() {
		GetFrame().options.back().help2 = ToString(sample_text.GetDescriptions()[static_cast<int>(sample_text.Get())]);
	};

	AddOption(MenuItem("<Built-in Font>", "Use the built-in pixel font", setting.Get().empty() ? "[x]" : ""), [this, &setting, mincho]() {
		Font::SetDefault(nullptr, mincho);
		setting.Set("");
		Pop();
	});
	set_help2();

	std::string font_lower = Utils::LowerCase(Font::Default(mincho)->GetName());

	auto list = fs.ListDirectory();
	assert(list);
	for (const auto& item: *list) {
		bool is_font = std::any_of(FileFinder::FONTS_TYPES.begin(), FileFinder::FONTS_TYPES.end(), [&item](const auto& ext) {
			return StringView(item.first).ends_with(ext);
		});

		if (item.second.type == DirectoryTree::FileType::Regular && is_font) {
			AddOption(MenuItem(item.second.name, "Use this font", StringView(font_lower).ends_with(item.first) ? "[x]" : ""), [=, &cfg, &setting]() mutable {
				if (Input::IsTriggered(Input::LEFT) || Input::IsRepeated(Input::LEFT)) {
					if (font_size.Get() == font_size.GetMin()) {
						font_size.Set(font_size.GetMax());
					} else {
						font_size.Set(font_size.Get() - 1);
					}
					return;
				} else if (Input::IsTriggered(Input::RIGHT) || Input::IsRepeated(Input::RIGHT)) {
					if (font_size.Get() == font_size.GetMax()) {
						font_size.Set(font_size.GetMin());
					} else {
						font_size.Set(font_size.Get() + 1);
					}
					return;
				}

				auto is = fs.OpenInputStream(item.second.name);
				if (is) {
					auto font = Font::CreateFtFont(std::move(is), font_size.Get(), false, false);
					if (font) {
						setting.Set(FileFinder::MakePath(fs.GetFullPath(), item.second.name));
						auto& setting_size = mincho ? cfg.font2_size : cfg.font1_size;
						setting_size.Set(font->GetCurrentStyle().size);
						Font::SetDefault(font, mincho);
						Pop();
					}
				}
			});
			set_help2();
		}
	}

	/*AddOption(font_size, [this]() mutable {
		font_size.Set(GetCurrentOption().current_value);
	});*/

	AddOption(sample_text, [this]() {
		sample_text.Set(static_cast<SampleText>(GetCurrentOption().current_value));
	});
	set_help2();

#ifdef EMSCRIPTEN
	AddOption(MenuItem("<Upload Font>", "Provide a font from your system", ""), [fs]() { Emscripten_Interface::UploadFont(); });
#elif defined(SUPPORT_FILE_BROWSER)
	AddOption(MenuItem("<Open Font directory>", "Open the font directory in a file browser", ""), [fs]() { DisplayUi->OpenURL(fs.GetFullPath()); });
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
	AddOption(MenuItem("SDL1", "Abstraction layer for graphic, audio, input and more", "LGPLv2.1+"), [](){});
#elif USE_SDL == 2
	AddOption(MenuItem("SDL2", "Abstraction layer for graphic, audio, input and more", "zlib"), [](){});
#elif USE_SDL == 3
	AddOption(MenuItem("SDL3", "Abstraction layer for graphic, audio, input and more", "zlib"), [](){});
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
#ifdef HAVE_LIBWILDMIDI
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
#ifdef HAVE_LHASA
	AddOption(MenuItem("lhasa", "For parsing LHA (.lzh) archives", "ISC"), [](){});
#endif
#ifdef HAVE_NLOHMANN_JSON
	AddOption(MenuItem("nlohmann_json", "Processing of JSON files", "MIT"), [](){});
#endif
	AddOption(MenuItem("Baekmuk", "Korean font family", "Baekmuk"), [](){});
	AddOption(MenuItem("Shinonome", "Japanese font family", "Public Domain"), [](){});
	AddOption(MenuItem("ttyp0", "ttyp0 font family", "ttyp0"), [](){});
	AddOption(MenuItem("WenQuanYi", "WenQuanYi font family (CJK)", "GPLv2+ with FE"), [](){});
#ifdef EMSCRIPTEN
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
	AddOption(cfg.speed_modifier_a, [this, &cfg](){ auto tmp = GetCurrentOption().current_value; Player::speed_modifier_a = tmp; cfg.speed_modifier_a.Set(tmp); });
	AddOption(cfg.speed_modifier_b, [this, &cfg](){ auto tmp = GetCurrentOption().current_value; Player::speed_modifier_b = tmp; cfg.speed_modifier_b.Set(tmp); });
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
				Input::TAKE_SCREENSHOT, Input::RESET, Input::FAST_FORWARD_A, Input::FAST_FORWARD_B,
				Input::PAGE_UP, Input::PAGE_DOWN };
			break;
		case 2:
			buttons = {	Input::DEBUG_MENU, Input::DEBUG_THROUGH, Input::DEBUG_SAVE, Input::DEBUG_ABORT_EVENT,
				Input::SHOW_LOG };
			break;
	}

	for (auto b: buttons) {
		auto button = static_cast<Input::InputButton>(b);

		std::string name = Input::kInputButtonNames.tag(button);

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

		std::string help = Input::kInputButtonHelp.tag(button);
		std::string value;

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
				cur_value = Input::Keys::kInputKeyNames.tag(ki->second);
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

		switch (button) {
			case Input::FAST_FORWARD_A: {
				Game_ConfigInput& cfg = Input::GetInputSource()->GetConfig();
				help = fmt::format(help, cfg.speed_modifier_a.Get());
				break;
			}
			case Input::FAST_FORWARD_B: {
				Game_ConfigInput& cfg = Input::GetInputSource()->GetConfig();
				help = fmt::format(help, cfg.speed_modifier_b.Get());
				break;
			}
			default:
				break;
		}

		auto param = MenuItem(name, help, value);
		AddOption(param,
				[this, button](){
				Push(eInputButtonOption, static_cast<int>(button));
			});
	}
}
