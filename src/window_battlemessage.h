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

	void Push(const std::string& message);

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
	 * How much lines would fit into a window of battle messages.
	 */
	static const int linesPerPage = 4;

private:
	std::vector<std::string> lines;

	bool needs_refresh;

	/**
	 * Adds a line to the lines vector. If the line is too long to
	 * be displayed in the message and contains space characters, then
	 * it will be broken into several lines.
	 *
	 * @param line String without newline characters.
	 * @return Number of lines added after word-wrapping.
	 **/
	int PushWordWrappedLine(const std::string& line);
};

#endif
