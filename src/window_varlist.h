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

#ifndef EP_WINDOW_VARLIST_H
#define EP_WINDOW_VARLIST_H

// Headers
#include "window_selectable.h"

class Window_VarList : public Window_Selectable
{
public:
	enum Mode {
		eNone,
		eSwitch,
		eVariable,
		eItem,
		eTroop,
		eMap,
		eHeal,
		eLevel,
		eCommonEvent,
		eMapEvent,
		eString
	};

	/**
	 * Constructor.
	 *
	 * @param commands commands to display.
	 */
	Window_VarList();
	~Window_VarList() override;

	void Update() override;

	/**
	 * UpdateList.
	 * 
	 * @param first_value starting value.
	 */
	void UpdateList(int first_value);

	void UpdateCursorRect() override;

	/**
	 * Refreshes the window contents.
	 */
	void  Refresh();

	int GetItemIndex() const;

	void SetItemIndex(int index);

	/**
	 * Indicate what to display.
	 *
	 * @param mode the mode to set.
	 */
	void SetMode(Mode mode);

	/**
	 * Returns the current mode.
	 */
	Mode GetMode() const;

	void SetShowDetail(bool show_detail);

	bool GetShowDetail() const;

	void SetItemText(unsigned index, std::string_view text);

	static constexpr std::string_view GetPrefix(Mode mode);
	static constexpr int GetItemCount(Mode mode, bool show_detail);
	static constexpr int GetDigitCount(Mode mode);

	static int GetNumElements(Mode mode);

	int GetItemCount() const;
	int GetDigitCount() const;
private:

	void DrawItem(int index, Font::SystemColor color);

	/**
	 * Draws the value of a variable standing on a row.
	 *
	 * @param index row with the var
	 */
	void DrawItemValue(int index);

	void DrawStringVarItem(int index, int y);

	std::vector<std::string> items;
	Mode mode = eNone;
	int first_var = 0;
	bool show_detail = false;
	bool suspend_cursor_refresh = false;

	bool DataIsValid(int range_index);
};

constexpr std::string_view Window_VarList::GetPrefix(Mode mode) {
	switch (mode) {
		case eSwitch:
			return "Sw";
		case eVariable:
			return "Vr";
		case eItem:
			return "It";
		case eTroop:
			return "Bt";
		case eMap:
			return "Mp";
		case eCommonEvent:
			return "Ce";
		case eMapEvent:
			return "Me";
		case eString:
			return "St";
		default:
			assert(false);
			return {};
	}
}

constexpr int Window_VarList::GetItemCount(Mode mode, bool show_detail) {
	switch (mode) {
		case eString:
			if (show_detail) {
				return 5;
			}
			break;
		default:
			break;
	}
	return 10;
}

constexpr int Window_VarList::GetDigitCount(Mode /* mode */) {
	return 4;
}

inline int Window_VarList::GetItemCount() const {
	return GetItemCount(mode, show_detail);
}

inline int Window_VarList::GetDigitCount() const {
	return GetDigitCount(mode);
}

inline Window_VarList::Mode Window_VarList::GetMode() const {
	return mode;
}

inline bool Window_VarList::GetShowDetail() const {
	return show_detail;
}

inline void Window_VarList::SetShowDetail(bool show_detail) {
	this->show_detail = show_detail;
}

#endif
