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
#include "window_selectable.h"

/*
FIXME:
Bold/Italic: Will only work when the font provides a bold/italic typeface
Gradient looks broken for larger font sizes
"pad to N digits" (\d[n]) is unsupported
Built-in & ExFont is not scaled to larger resolutions
*/

/**
 * Manages user generated windows.
 */
class Game_Windows {
public:
	Game_Windows() = default;

	void SetSaveData(std::vector<lcf::rpg::SaveEasyRpgWindow> save);
	std::vector<lcf::rpg::SaveEasyRpgWindow> GetSaveData() const;

	struct WindowText {
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
		std::vector<WindowText> texts;
		bool draw_frame = true;
		bool border_margin = true;
	};

	bool Create(int id, const WindowParams& params, bool& async_wait);
	void Erase(int id);

	struct Window_User {
		explicit Window_User(int id) { data.ID = id; }
		explicit Window_User(lcf::rpg::SaveEasyRpgWindow data);

		lcf::rpg::SaveEasyRpgWindow data;

		bool Create(const WindowParams& params);
		void Erase();

		void Refresh(bool& async_wait);

		/**
		 * Does an Async request of all assets.
		 * @return true when all requests are finished
		 */
		bool Request();
		void OnRequestReady(FileRequestResult* result);

		std::unique_ptr<Window_Selectable> window;
		std::vector<FileRequestBinding> request_ids;
	};

	Window_User& GetWindow(int id);
	Window_User* GetWindowPtr(int id);

private:
	std::vector<Window_User> windows;
};

#endif
