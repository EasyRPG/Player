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

#ifndef EP_WINDOW_HELP_H
#define EP_WINDOW_HELP_H

// Headers
#include "window_base.h"
#include "text.h"

/**
 * Window_Help class.
 * Shows skill and item explanations.
 */
class Window_Help : public Window_Base {

public:
	/**
	 * Constructor.
	 */
	Window_Help(int ix, int iy, int iwidth, int iheight);

	/**
	 * Sets the text that will be shown.
	 *
	 * @param text text to show.
	 * @param align text alignment.
	 */
	void SetText(std::string text, Text::Alignment align = Text::AlignLeft);

private:
	/** Text to draw. */
	std::string text;
	/** Alignment of text. */
	Text::Alignment align;
};

#endif
