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

Window_Settings::Window_Settings(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight) {
	column_max = 1;
}

void Window_Settings::UpdateMode() {
	auto frame = GetFrame();
	auto mode = frame.uimode;

	if (mode == eInputRemap) {
		const Input::KeyStatus* keys;

		// Determine if a CANCEL key is for aborting or for mapping
		// depending on how long the key was held
		if (Input::IsPressed(Input::CANCEL)) {
			++timer;
			if (timer == 30) {
				Input::ResetKeys();
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
				Input::ResetKeys();
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
	stack[stack_index] = { ui, arg  };

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
	timer = 0;

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

static const std::string& ParamToText(const std::string& s) {
	return s;
}

static std::string ParamToText(bool b) {
	return b ? "ON" : "OFF";
}

static std::string ParamToText(int i) {
	return std::to_string(i);
}

template <typename Param, typename Action>
void Window_Settings::AddOption(const std::string& prefix,
		const Param& param,
		const std::string& suffix,
		Action&& action,
		const std::string& help)
{
	if (!param.Enabled()) {
		return;
	}
	Option opt;
	opt.text = prefix + ": " + ParamToText(param.Get()) + suffix;
	opt.help = help;
	if (!param.Locked()) {
		opt.action = std::move(action);
	}
	options.push_back(std::move(opt));
}

void Window_Settings::RefreshVideo() {
	auto cfg = DisplayUi->GetConfig();

	AddOption("Renderer", cfg.renderer, "",
			[](){},
			"The rendering engine");
	AddOption("FullScreen", cfg.fullscreen, "",
			[](){ DisplayUi->ToggleFullscreen(); },
			"Toggle between fullscreen and window mode");
	AddOption("Window Zoom", cfg.window_zoom, "",
			[](){ DisplayUi->ToggleZoom(); },
			"Toggle the window zoom level");
	AddOption("Vsync", cfg.vsync, "",
			[](){},
			"Toogle Vsync mode (Recommended: ON)");
	AddOption("Frame Limiter", cfg.fps_limit, " FPS",
			[](){},
			"Toggle the frames per second limit (Recommended: 60)");
	AddOption("Show FPS", cfg.show_fps, "",
			[](){ DisplayUi->ToggleShowFps(); },
			"Toggle display of the FPS counter");
}

void Window_Settings::RefreshAudio() {
	AddOption("BGM Volume", LockedConfigParam<int>(100), "%",
			[](){},
			"Music Volumne");
	AddOption("SFX Volume", LockedConfigParam<int>(100), "%",
			[](){},
			"Sound Effect Volume");
	AddOption("Midi Backend", LockedConfigParam<std::string>("Unknown"), "",
			[](){},
			"Which MIDI backend to use");
	AddOption("Midi Soundfont", LockedConfigParam<std::string>("Default"), "",
			[](){},
			"Which MIDI soundfont to use");
}

void Window_Settings::RefreshLicense() {
	AddOption("EasyRPG Player", ConfigParam<std::string>("MIT"), "", [](){}, "The engine you are using :)");
#ifdef HAVE_LIBSNDFILE
	AddOption("libsndfile", ConfigParam<std::string>("LGPLv2+"), "", [](){}, "A library for reading and writing files containing WAV audio");
#endif
	AddOption("libpng", ConfigParam<std::string>("zlib"), "", [](){}, "Used to handle PNG graphic files");
	AddOption("zlib", ConfigParam<std::string>("zlib"), "", [](){}, "Implements the deflate compression method (ZIP)");
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

		auto param = ConfigParam<std::string>(ss.str());
		std::string prefix = std::string("Key ") + name;
		AddOption(prefix, param, "",
				[this, button, prefix](){
				Push(eInputButton, static_cast<int>(button));
			}, help);
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
			AddOption(kname, LockedConfigParam<std::string>(""), "", [](){
			}, key_label);
		} else {
			AddOption(kname, ConfigParam<std::string>(""), "", [=]() mutable {
				Input::GetInputSource()->GetButtonMappings().Remove({button, key});
				Refresh();
			}, key_label);
		}
	}

	AddOption("<Add new>", ConfigParam<std::string>(""), "", [=](){
		Push(eInputRemap, static_cast<int>(button));
		Input::ResetKeys();
	}, key_label);
	AddOption("<Reset>", ConfigParam<std::string>(""), "", [=](){
		Input::ResetDefaultMapping(button);
		Refresh();
	}, key_label);
}

void Window_Settings::RefreshInputRemap() {
	auto button = static_cast<Input::InputButton>(GetFrame().arg);
	auto name = Input::kButtonNames.tag(button);

	AddOption("Press the new key", ConfigParam<std::string>(""), "", [](){}, fmt::format("Remapping {}", name));
	AddOption("Hold CANCEL to abort", ConfigParam<std::string>(""), "", [](){}, "");
}
