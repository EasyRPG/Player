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
#include "window_settings.h"
#include "game_config.h"
#include "input_buttons.h"
#include "keys.h"
#include "output.h"
#include "baseui.h"
#include "bitmap.h"
#include "player.h"

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

	contents->TextDraw(rect.x, rect.y, color, option.text);
}

void Window_Settings::SavePosition() {
	if (mode != eNone) {
		auto& mem = memory[mode - 1];
		mem.index = index;
		mem.top_row = GetTopRow();
	}
}

void Window_Settings::RestorePosition() {
	if (mode != eNone) {
		auto& mem = memory[mode - 1];
		index = mem.index;
		SetTopRow(mem.top_row);
	}
}

void Window_Settings::SetMode(Mode nmode) {
	SavePosition();
	mode = nmode;
	RestorePosition();
	Refresh();
}

void Window_Settings::Refresh() {
	options.clear();

	switch (mode) {
		case eNone:
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
	}

	item_max = options.size();

	CreateContents();

	if (mode == eNone || options.empty()) {
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

void Window_Settings::RefreshInput() {
#if 0
	for (int i = 0; i < Input::BUTTON_COUNT; ++i) {
		auto button = static_cast<Input::InputButton>(i);

		if (!Player::debug_flag && (
					button == Input::DEBUG_MENU
					|| button == Input::DEBUG_SAVE
					|| button == Input::DEBUG_THROUGH)) {
			continue;
		}

		auto name = Input::kButtonNames.tag(button);
		auto help = Input::kButtonHelp.tag(button);

		auto& keys = Input::buttons[i];

		std::stringstream ss;
		for (auto ki: keys) {
			auto key = static_cast<Input::Keys::InputKey>(ki);
			auto kname = Input::Keys::kNames.tag(key);
			ss << kname << " ";
		}

		// FIXME: Example for now.
		auto param = LockedConfigParam<std::string>(ss.str());
		AddOption(std::string("Key ") + name, param, "",
				[](){},
				help);
	}
#endif
}

