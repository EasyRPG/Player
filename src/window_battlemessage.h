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

#ifndef _WINDOW_BATTLEMESSAGE_H_
#define _WINDOW_BATTLEMESSAGE_H_

// Headers
#include "window_base.h"

/**
 * Displays messages during a battle
 */
class Window_BattleMessage : public Window_Base {

public:
	Window_BattleMessage(int ix, int iy, int iwidth, int iheight);

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
	void Push(const std::string& message);

	/**
	 * Pushes a message, either prepending the subject to it,
	 * or replacing all the occurences of %S with subject, depending
	 * on the engine version.
	 *
	 * @param string Message to be displayed.
	 * @param string Subject that will be displayed in the message.
	 */
	void PushWithSubject(const std::string& message, const std::string& subject);

	void Pop();

	void Clear();

	/**
	 * Remove 4 lines (determined by linesPerPage) of the battle
	 * messages, thus proceeding to the next page.
	 *
	 * @return True if there is something left to show, false is the
	 * previous page was the last one.
	 */
	bool NextPage();

	int GetLineCount();

	void Refresh();

	void Update() override;

	/**
	 * @return true is the message window is filled, false if there
	 * is space for at least one line on the first page.
	 */
	bool IsPageFilled();

	/**
	 * Number of lines that are hidden right now.
	 *
	 * Hidden lines are added when the text is word-wrapped:
	 * only the first line is shown, and others are hidden.
	 *
	 * @return number of hidden lines
	 */
	int GetHiddenLineCount();

	/**
	 * Displays the given number of hidden lines.
	 *
	 * Hidden lines are added when a word-wrapped line is pushed:
	 * then, only the first line is displayed, and others are
	 * considered hidden.
	 *
	 * @param Number of lines to display. If -1
	 * is passed, all the hidden lines are displayed.
	 */
	void ShowHiddenLines(int count);

	/**
	 * How many lines would fit into a window of battle messages.
	 */
	static const int linesPerPage = 4;

private:
	std::vector<std::string> lines;

	bool needs_refresh;

	/**
	 * How many lines are hidden right now.
	 *
	 * Hidden lines are added by PushWordWrappedLine when
	 * the pushed line doesn't fit one line. Such lines are
	 * hidden until ShowHiddenLines is called.
	 */
	int hidden_lines;
};

#endif
