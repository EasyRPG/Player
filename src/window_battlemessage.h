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

#ifndef EP_WINDOW_BATTLEMESSAGE_H
#define EP_WINDOW_BATTLEMESSAGE_H

// Headers
#include "window_base.h"

/**
 * Displays messages during a battle
 */
class Window_BattleMessage : public Window_Base {

public:
	Window_BattleMessage(Scene* parent, int ix, int iy, int iwidth, int iheight);

	/**
	 * Adds message to be displayed.
	 *
	 * If hidden lines exist prior to pushing the message, they
	 * are shown. However, the newly-added lines message might
	 * be initially hidden if the engine supports word-wrapping
	 * and the line is long, only the first line is shown, and
	 * other lines are hidden unless ShowHiddenLines is called.
	 *
	 * @param message The text to be displayed.
	 */
	void Push(StringView message);

	/**
	 * Pushes a message, either prepending the subject to it,
	 * or replacing all the occurences of %S with subject, depending
	 * on the engine version.
	 *
	 * @param message Message to be displayed.
	 * @param subject Subject that will be displayed in the message.
	 */
	void PushWithSubject(StringView message, StringView subject);

	void Pop();

	void PopUntil(int line_number);

	void Clear();

	/**
	 * Scrolls down in the console so that the last 4 lines are displayed.
	 */
	void ScrollToEnd();

	int GetLineCount();

	void Refresh();

	void Update() override;

	/**
	 * @return true is the message window is filled, false if there
	 * is space for at least one line on the first page.
	 */
	bool IsPageFilled();

	/**
	 * @return the line index we will start displaying
	 */
	int GetIndex() const;

	/**
	 * Set the first line index to display
	 *
	 * @param val the first line index to display
	 */
	void SetIndex(int val);

	const std::vector<std::string>& GetLines() const;

	/**
	 * How many lines would fit into a window of battle messages.
	 */
	static const int linesPerPage = 4;

protected:
	void PushLine(StringView line);
private:
	std::vector<std::string> lines;

	int index = 0;
	bool needs_refresh = false;
};


inline int Window_BattleMessage::GetIndex() const {
	return index;
}

inline void Window_BattleMessage::SetIndex(int val) {
	index = val;
}

inline const std::vector<std::string>& Window_BattleMessage::GetLines() const {
	return lines;
}

#endif
