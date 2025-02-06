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
#include "game_interpreter_debug.h"
#include "window_command.h"
#include "game_interpreter_shared.h"
#include "lcf/rpg/saveeventexecstate.h"
#include "lcf/rpg/saveeventexecframe.h"

class Window_Interpreter : public Window_Selectable {
public:
	enum UiAction {
		None = 0,
		ShowRuntimeFlags = 1,
		ShowMovementInfo,
		ShowStackItem
	};

	class UiSubActionLine {
	public:
		UiSubActionLine() {

		}

		UiSubActionLine(std::initializer_list<UiAction> actions, std::initializer_list<std::string> texts,
			std::initializer_list<Font::SystemColor> colors, std::initializer_list<std::function<bool()>> visibility_delegates) {

			assert(actions.size() == texts.size());
			assert(actions.size() == colors.size());
			assert(actions.size() == visibility_delegates.size());

			this->actions = actions;
			this->texts = texts;
			this->colors = colors;
			this->visibility_delegates = visibility_delegates;
		}

		bool IsVisible() const;
		void Update(Window_Selectable& parent);
		void Draw(BitmapRef contents, Rect rect) const;
		void ClearIndex();
		UiAction GetSelectedAction() const;

	private:
		int index = 0;

		std::vector<UiAction> actions;
		std::vector<std::string> texts;
		std::vector<Font::SystemColor> colors;
		std::vector<std::function<bool()>> visibility_delegates;
	};

	Window_Interpreter(int ix, int iy, int iwidth, int iheight);
	~Window_Interpreter() override;

	void Update() override;

	void SetStackState(std::string interpreter_desc, lcf::rpg::SaveEventExecState state);
	void Refresh();
	bool IsValid();

	UiAction GetSelectedAction() const;
	int GetSelectedStackFrameLine() const;

protected:
	bool IsHoveringSubActionLine() const;
	void DrawDescriptionLines();
	void DrawStackLine(int index);

#ifdef ENABLE_DYNAMIC_INTERPRETER_CONFIG
	void DrawRuntimeFlagsWindow() const;
#endif
private:
	struct InterpDisplayItem {
		std::string desc;
	};


	const int lines_without_stack_fixed = 3;

	lcf::rpg::SaveEventExecState state;
	int lines_without_stack = 0;

	int digits_stackitemno = 0, digits_evt_id = 0, digits_page_id = 0, digits_evt_combined_id = 0, digits_cmdcount = 0;

	InterpDisplayItem display_item;

	UiSubActionLine sub_actions;
	std::unique_ptr<Window_Selectable> sub_window_flags;
	std::vector<Debug::CallStackItem> stack_display_items;
};

#endif
