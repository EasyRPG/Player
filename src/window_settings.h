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

#ifndef EP_WINDOW_SETTINGS_H
#define EP_WINDOW_SETTINGS_H

// Headers
#include <vector>
#include "input.h"
#include "window_numberinput.h"
#include "window_selectable.h"

/**
 * Window_Settings class.
 */
class Window_Settings : public Window_Selectable {
public:
	enum UiMode {
		eNone,
		eMain,
		eInput,
		eInputButtonCategory,
		eInputListButtonsGame,
		eInputListButtonsEngine,
		eInputListButtonsDeveloper,
		eInputButtonOption,
		eInputButtonAdd,
		eInputButtonRemove,
		eVideo,
		eAudio,
		eLicense,
		eEngine,
		eSave,
		eEnd,
		eAbout,
		eLastMode
	};

	enum OptionMode {
		eOptionNone,
		eOptionRangeInput,
		eOptionPicker
	};

	struct Option {
		std::string text;
		std::string value_text;
		std::string help;
		std::function<void(void)> action;
		OptionMode mode;
		int current_value;
		int original_value;
		int min_value;
		int max_value;
		std::vector<int> options_index;
		std::vector<std::string> options_text;
		std::vector<std::string> options_help;
	};

	struct StackFrame {
		UiMode uimode = eNone;
		int arg = -1;
		int scratch = 0;
		int scratch2 = 0;
	};

	/** Constructor  */
	Window_Settings(int ix, int iy, int iwidth, int iheight);

	/** @return true if the index points to an enabled action */
	bool IsCurrentActionEnabled() const {
		return (index >= 0
				&& index < static_cast<int>(options.size())
				&& static_cast<bool>(options[index].action));
	}

	/** Execute the action pointed to by index */
	Option& GetCurrentOption() {
		return options[index];
	}

	UiMode GetMode() const;

	void Push(UiMode ui, int arg = -1);
	void Pop();

	StackFrame& GetFrame(int n = 0);
	const StackFrame& GetFrame(int n = 0) const;

	/**
	 * Refreshes the item list.
	 */
	void Refresh();

private:

	/**
	 * Draws an item together with the quantity.
	 *
	 * @param index index of item to draw.
	 */
	void DrawOption(int index);

	template <typename Param, typename Action>
	void AddOption(const Param& p,
			Action&& action);

	template <typename T, typename Action>
	void AddOption(const RangeConfigParam<T>& p,
			Action&& action
	);

	template <typename T, typename Action, size_t S>
	void AddOption(const EnumConfigParam<T, S>& p,
			Action&& action
	);

	void RefreshInput();
	void RefreshButtonCategory();
	void RefreshButtonList();
	void RefreshVideo();
	void RefreshAudio();
	void RefreshEngine();
	void RefreshLicense();

	void UpdateHelp() override;

	std::vector<Option> options;

	struct Memory {
		int index = 0;
		int top_row = 0;
	};
	Memory memory[eLastMode] = {};

	std::array<StackFrame,8> stack;
	int stack_index = 0;
	std::vector<std::string> picker_options;

	void SavePosition();
	void RestorePosition();
};

#endif
