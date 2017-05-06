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
	 * Pushes an 'Enemy appeared' message into the message list.
	 *
	 * @param string Enemy name that will be displayed in the message.
	 */
	void EnemyAppeared(const std::string& enemy_name);

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

	/**
	 * How many lines are hidden right now.
	 *
	 * Hidden lines are added by PushWordWrappedLine when
	 * the pushed line doesn't fit one line. Such lines are
	 * hidden until ShowHiddenLines is called.
	 */
	int hidden_lines;

	bool needs_refresh;

	/**
	 * Adds a line to the lines vector. If the line is too long to
	 * be displayed in the message and contains space characters, then
	 * it will be broken into several lines.
	 *
	 * When the line is word-wrapped, only the first line is shown. The
	 * other lines are initially hidden. The number of hidden lines can
	 * be retrieved wih GetHiddenLineCount, and shown usign 
	 * ShowHiddenLines.
	 *
	 * @param line String without newline characters.
	 * @return Number of lines added after word-wrapping.
	 **/
	int PushWordWrappedLine(const std::string& line);
};

#endif
