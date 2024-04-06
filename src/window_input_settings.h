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

#ifndef EP_WINDOW_INPUT_SETTINGS_H
#define EP_WINDOW_INPUT_SETTINGS_H

// Headers
#include <vector>
#include "input.h"
#include "input_buttons.h"
#include "window_numberinput.h"
#include "window_selectable.h"

/**
 * Window_InputSettings class.
 */
class Window_InputSettings : public Window_Selectable {
public:
	static constexpr int mapping_limit = 16;

	/** Constructor  */
	Window_InputSettings(Scene* parent, int ix, int iy, int iwidth, int iheight);

	Input::InputButton GetInputButton() const;
	void SetInputButton(Input::InputButton button);

	bool RemoveMapping();
	void ResetMapping();

	/**
	 * Refreshes the item list.
	 */
	void Refresh();

private:
	Input::InputButton button = Input::InputButton::BUTTON_COUNT;
};

#endif
