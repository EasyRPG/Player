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

#ifndef EP_WINDOW_INTERPRETER_H
#define EP_WINDOW_INTERPRETER_H

// Headers
#include "window_command.h"
#include "lcf/rpg/saveeventexecstate.h"
#include "lcf/rpg/saveeventexecframe.h"

class Window_Interpreter : public Window_Selectable {
public:
	Window_Interpreter(int ix, int iy, int iwidth, int iheight);
	~Window_Interpreter() override;

	void Update() override;

	void SetStackState(bool is_ce, int owner_evt_id, std::string interpreter_desc, lcf::rpg::SaveEventExecState state);
	void Refresh();
	bool IsValid();

	int GetSelectedStackFrameLine();
protected:
	void DrawDescriptionLines();
	void DrawStackLine(int index);
private:
	struct InterpDisplayItem {
		bool is_ce;
		int owner_evt_id;
		std::string desc;
	};

	struct StackItem {
		bool is_ce;
		int evt_id, page_id;
		std::string name;
		int cmd_current, cmd_count;
	};

	const int lines_without_stack_fixed = 3;
	
	lcf::rpg::SaveEventExecState state;
	int lines_without_stack = 0;

	int digits_stackitemno = 0, digits_evt_id = 0, digits_page_id = 0, digits_evt_combined_id = 0, digits_cmdcount = 0;

	InterpDisplayItem display_item;
	std::vector<StackItem> stack_display_items;
};

#endif
