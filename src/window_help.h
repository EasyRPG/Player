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
#include "font.h"

/**
 * Window_Help class.
 * Shows skill and item explanations.
 */
class Window_Help : public Window_Base {

public:
	/**
	 * Constructor.
	 */
	Window_Help(Scene* parent, int ix, int iy, int iwidth, int iheight, Drawable::Flags flags = Drawable::Flags::Default);

	/**
	 * Sets the text that will be shown.
	 *
	 * @param text text to show.
	 * @param align text alignment.
	 * @param halfwidthspace if half width spaces should be used.
	 */
	void SetText(std::string text, int color = Font::ColorDefault, Text::Alignment align = Text::AlignLeft, bool halfwidthspace = true);

	/**
	 * Clears the window
	 */
	void Clear();

	/**
	 * Adds text to the help window. This does not overwrite the old content.
	 *
	 * @param text text to add.
	 * @param color text color.
	 * @param align text alignment.
	 * @param halfwidthspace if half width spaces should be used.
	 */
	void AddText(std::string text, int color = Font::ColorDefault, Text::Alignment align = Text::AlignLeft, bool halfwidthspace = true);

private:
	/** Text to draw. */
	std::string text;
	/** Color of Text to draw. */
	int color = Font::ColorDefault;
	/** Alignment of text. */
	Text::Alignment align;
	/** Current text position */
	int text_x_offset = 0;
};

#endif
