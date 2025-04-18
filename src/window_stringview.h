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

#ifndef EP_WINDOW_STRINGVIEW_H
#define EP_WINDOW_STRINGVIEW_H

// Headers
#include "window_command.h"

#ifdef HAVE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

class Window_StringView : public Window_Selectable {
public:
	Window_StringView(int ix, int iy, int iwidth, int iheight);
	~Window_StringView() override;

	void Update() override;

	void SetDisplayData(std::string_view data);
	std::string GetDisplayData(bool eval_cmds);

#ifdef HAVE_NLOHMANN_JSON
	void SetDisplayData(std::string_view data, const nlohmann::ordered_json& json_data);
#endif

	void Refresh();
protected:
	void DrawCmdLines();
	void DrawLine(int index);

	int GetReservedLineCount() const;
private:
	const int max_str_length = 42;
	bool auto_linebreak = false, cmd_eval = false, pretty_print = false;

	std::string display_data_raw;
#ifdef HAVE_NLOHMANN_JSON
	const nlohmann::ordered_json* json_data;
#endif

	std::vector<std::string> lines;
	std::vector<int> line_numbers;

	int line_count = 0, line_no_max_digits = 0;
};

inline int Window_StringView::GetReservedLineCount() const {
#ifdef HAVE_NLOHMANN_JSON
	if (this->json_data) {
		return 3;
	}
#endif
	return 2;
}

#endif
