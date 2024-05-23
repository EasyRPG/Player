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
#include <vector>
#include "window_interpreter.h"
#include "bitmap.h"
#include "game_map.h"
#include "game_message.h"
#include "game_system.h"
#include "game_variables.h"
#include "input.h"
#include "player.h"
#include "lcf/reader_util.h"

namespace {
	std::vector<std::string> CreateEmptyLines(int c) {		
		std::vector<std::string> vars;
		for (int i = 0; i < c; i++)
			vars.push_back("");
		return vars;
	}
}

Window_Interpreter::Window_Interpreter(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight) {
	column_max = 1;
}

Window_Interpreter::~Window_Interpreter() {

}

void Window_Interpreter::SetStackState(bool is_ce, int owner_evt_id, std::string interpreter_desc, lcf::rpg::SaveEventExecState state) {
	this->display_item = { is_ce, owner_evt_id, interpreter_desc };
	this->state = state;
}

void Window_Interpreter::Refresh() {
	stack_display_items.clear();

	int max_cmd_count = 0, max_evt_id = 10, max_page_id = 0;

	for (int i = state.stack.size() - 1; i >= 0; i--) {
		int evt_id = state.stack[i].event_id;
		int page_id = 0;
		if (state.stack[i].maniac_event_id > 0) {
			evt_id = state.stack[i].maniac_event_id;
			page_id = state.stack[i].maniac_event_page_id;
		}
		if (evt_id == 0 && i == 0)
			evt_id = display_item.owner_evt_id;

		bool is_calling_ev_ce = false;

		//FIXME: There are some currently unimplemented SaveEventExecFrame fields introduced via the ManiacPatch which should be used to properly get event state information
		if (evt_id == 0 && i > 0) {
			auto& prev_frame = state.stack[i - 1];
			auto& com = prev_frame.commands[prev_frame.current_command - 1];
			if (com.code == 12330) { // CallEvent
				if (com.parameters[0] == 0) {
					is_calling_ev_ce = true;
					evt_id = com.parameters[1];
				} else if (com.parameters[0] == 3 && Player::IsPatchManiac()) {
					is_calling_ev_ce = true;
					evt_id = Main_Data::game_variables->Get(com.parameters[1]);
				} else if (com.parameters[0] == 4 && Player::IsPatchManiac()) {
					is_calling_ev_ce = true;
					evt_id = Main_Data::game_variables->GetIndirect(com.parameters[1]);
				}
			}
		}
		if (evt_id > max_evt_id)
			max_evt_id = evt_id;

		if (page_id > max_page_id)
			max_page_id = page_id;

		StackItem item = StackItem();
		item.is_ce = is_calling_ev_ce || (i == 0 && this->display_item.is_ce);
		item.evt_id = evt_id;
		item.page_id = page_id;
		item.name = "";
		item.cmd_current = state.stack[i].current_command;
		item.cmd_count = state.stack[i].commands.size();

		if (item.is_ce) {
			auto* ce = lcf::ReaderUtil::GetElement(lcf::Data::commonevents, item.evt_id);
			if (ce) {
				item.name = ToString(ce->name);
			}
		} else {
			auto* ev = Game_Map::GetEvent(evt_id);
			if (ev) {
				//FIXME: map could have changed, but map_id isn't available
				item.name = ToString(ev->GetName());
			}
		}

		if (state.stack[i].commands.size() > max_cmd_count)
			max_cmd_count = state.stack[i].commands.size();

		stack_display_items.push_back(item);
	}

	item_max = stack_display_items.size() + lines_without_stack;
	lines_without_stack = lines_without_stack_fixed;

	if (state.wait_movement) {
		item_max++;
		lines_without_stack++;
	}

	digits_stackitemno = std::log10(stack_display_items.size()) + 1;
	digits_evt_id = max_evt_id == 0 ? 2 : std::log10(max_evt_id) + 1;
	digits_page_id = max_page_id == 0 ? 0 : std::log10(max_page_id) + 1;
	digits_cmdcount = std::log10(max_cmd_count) + 1;

	digits_evt_combined_id = digits_evt_id + 3;
	if (digits_page_id > 0)
		digits_evt_combined_id += digits_page_id + 2;

	CreateContents();
	contents->Clear();

	if (!IsValid())
		return;

	DrawDescriptionLines();

	for (int i = 0; i < stack_display_items.size(); ++i) {
		DrawStackLine(i);
	}
}

bool Window_Interpreter::IsValid() {
	return !display_item.desc.empty();
}

void Window_Interpreter::Update() {
	Window_Selectable::Update();
}

void Window_Interpreter::DrawDescriptionLines() {
	int i = 0;
	Rect rect = GetItemRect(i++);
	contents->ClearRect(rect);

	contents->TextDraw(rect.x, rect.y, Font::ColorDefault, display_item.desc);

	if (state.wait_movement) {
		rect = GetItemRect(i++);
		contents->ClearRect(rect);
		contents->TextDraw(rect.x, rect.y, Font::ColorCritical, "[WAITING for EV movement!]");
	}
	
	rect = GetItemRect(i++);
	contents->ClearRect(rect);

	contents->TextDraw(rect.x, rect.y, Font::ColorDefault, "Stack Size: ");
	contents->TextDraw(GetWidth() - 16, rect.y, Font::ColorCritical, std::to_string(state.stack.size()), Text::AlignRight);
}

void Window_Interpreter::DrawStackLine(int index) {

	Rect rect = GetItemRect(index + lines_without_stack);
	contents->ClearRect(rect);

	StackItem& item = stack_display_items[index];

	contents->TextDraw(rect.x, rect.y, Font::ColorDisabled, fmt::format("[{:0" + std::to_string(digits_stackitemno) + "d}]", state.stack.size() - index));
	if (item.is_ce) {
		contents->TextDraw(rect.x + (digits_stackitemno * 6) + 16, rect.y, Font::ColorDefault, fmt::format("CE{:0" + std::to_string(digits_evt_id) + "d}", item.evt_id));
	} else if (item.page_id > 0) {
		contents->TextDraw(rect.x + (digits_stackitemno * 6) + 16, rect.y, Font::ColorDefault, fmt::format("EV{:0" + std::to_string(digits_evt_id) + "d}[{:0" + std::to_string(digits_page_id) + "d}]", item.evt_id, item.page_id));
	} else {
		contents->TextDraw(rect.x + (digits_stackitemno * 6) + 16, rect.y, Font::ColorDefault, fmt::format("EV{:0" + std::to_string(digits_evt_id) + "d}", item.evt_id));
	}

	std::string name = item.name;
	int max_length = 28;
	max_length -= digits_stackitemno;
	max_length -= digits_evt_combined_id;
	max_length -= digits_cmdcount * 2;
	if (name.length() > max_length) {
		name = name.substr(0, max_length - 3) + "...";
	}
	contents->TextDraw(rect.x + ((digits_stackitemno + digits_evt_combined_id) * 6) + 16, rect.y, Font::ColorDefault, name, Text::AlignLeft);
	contents->TextDraw(GetWidth() - 16, rect.y, Font::ColorDefault, fmt::format("{:0" + std::to_string(digits_cmdcount) + "d}/{:0" + std::to_string(digits_cmdcount) + "d}", item.cmd_current, item.cmd_count), Text::AlignRight);
}

int Window_Interpreter::GetSelectedStackFrameLine() {
	if (GetIndex() < lines_without_stack) {
		return -1;
	}
	return state.stack.size() - (GetIndex() - lines_without_stack) - 1;
}
