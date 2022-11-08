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

#include "game_windows.h"
#include "game_pictures.h"
#include "main_data.h"
#include "compiler.h"

Game_Windows::Window::Window(lcf::rpg::SaveEasyRpgWindow save)
	: data(std::move(save))
{

}

void Game_Windows::SetSaveData(std::vector<lcf::rpg::SaveEasyRpgWindow> save)
{
	windows.clear();

	int num_windows = static_cast<int>(save.size());
	windows.reserve(num_windows);
	for (int i = 0; i < num_windows; ++i) {
		windows.emplace_back(std::move(save[i]));
	}
}

std::vector<lcf::rpg::SaveEasyRpgWindow> Game_Windows::GetSaveData() const {
	std::vector<lcf::rpg::SaveEasyRpgWindow> save;

	auto data_size = static_cast<int>(windows.size());
	save.reserve(data_size);

	for (auto& win: windows) {
		save.push_back(win.data);
	}

	// RPG_RT Save game data always has a constant number of pictures
	// depending on the engine version. We replicate this, unless we have even
	// more pictures than that.
	while (data_size > static_cast<int>(save.size())) {
		lcf::rpg::SaveEasyRpgWindow data;
		data.ID = static_cast<int>(save.size()) + 1;
		save.push_back(std::move(data));
	}

	return save;
}

Game_Windows::Window& Game_Windows::GetWindow(int id) {
	if (EP_UNLIKELY(id > static_cast<int>(windows.size()))) {
		windows.reserve(id);
		while (static_cast<int>(windows.size()) < id) {
			windows.emplace_back(windows.size() + 1);
		}
	}
	return windows[id - 1];
}

Game_Windows::Window* Game_Windows::GetWindowPtr(int id) {
	return id <= static_cast<int>(windows.size())
		? &windows[id - 1] : nullptr;
}

bool Game_Windows::Window::Create(const WindowParams& params) {
	data.width = params.width;
	data.height = params.height;
	data.system_name = lcf::DBString(params.system_name);
	data.message_stretch = params.message_stretch;
	data.flags.draw_frame = params.draw_frame;
	data.flags.border_margin = params.border_margin;

	for (const auto& text: params.texts) {
		lcf::rpg::SaveEasyRpgText data_text;
		data_text.text = lcf::DBString(text.text);
		data_text.position_x = text.position_x;
		data_text.position_y = text.position_y;
		data_text.font_name = lcf::DBString(text.font_name);
		data_text.font_size = text.font_size;
		data_text.letter_spacing = text.letter_spacing;
		data_text.line_spacing = text.line_spacing;
		data_text.flags.draw_gradient = text.draw_gradient;
		data_text.flags.draw_shadow = text.draw_shadow;
		data_text.flags.bold = text.bold;
		data_text.flags.italic = text.italic;

		data.texts.push_back(data_text);
	}

	return true;
}

bool Game_Windows::Create(int id, const WindowParams& params) {
	auto& window = GetWindow(id);

	if (window.Create(params)) {
		auto& pic = Main_Data::game_pictures->GetPicture(id);
		if (pic.Show(params)) {
			pic.AttachWindow();
			return true;
		} else {
			window.Erase();
			return false;
		}
	}

	return false;
}

void Game_Windows::Window::Erase() {
	data = {};
}

void Game_Windows::Erase(int id) {
	auto* window = GetWindowPtr(id);
	if (EP_LIKELY(window)) {
		window->Erase();
	}
}
