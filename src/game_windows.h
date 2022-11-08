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

#ifndef EP_GAME_WINDOWS_H
#define EP_GAME_WINDOWS_H

// Headers
#include <string>
#include <deque>
#include <lcf/rpg/saveeasyrpgwindow.h>
#include <lcf/rpg/system.h>
#include "game_pictures.h"
#include "window.h"

/**
 * Manages user generated windows.
 */
class Game_Windows {
public:
	Game_Windows() = default;

	void SetSaveData(std::vector<lcf::rpg::SaveEasyRpgWindow> save);
	std::vector<lcf::rpg::SaveEasyRpgWindow> GetSaveData() const;

	struct Text {
		std::string text;
		int position_x = 0;
		int position_y = 0;
		std::string font_name;
		int font_size = 12;
		int letter_spacing = 0;
		int line_spacing = 4;
		bool draw_gradient = true;
		bool draw_shadow = true;
		bool bold = false;
		bool italic = false;
	};

	struct WindowParams : Game_Pictures::ShowParams {
		std::string system_name;
		int width = 0;
		int height = 0;
		int message_stretch = lcf::rpg::System::Stretch_easyrpg_none;
		std::vector<Text> texts;
		bool draw_frame = true;
		bool border_margin = true;
	};

	bool Create(int id, const WindowParams& params);
	void Erase(int id);

	struct Window {
		Window(int id) { data.ID = id; }
		Window(lcf::rpg::SaveEasyRpgWindow data);

		lcf::rpg::SaveEasyRpgWindow data;

		bool Create(const WindowParams& params);
		void Erase();
	};

	Window& GetWindow(int id);
	Window* GetWindowPtr(int id);

private:
	std::vector<Window> windows;
};

#endif
