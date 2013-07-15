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

#ifndef _WINDOW_BATTLESTATUS_H_
#define _WINDOW_BATTLESTATUS_H_

// Headers
#include "window_selectable.h"
#include "bitmap.h"

/**
 * Window BattleStatus Class.
 * Displays the party battle status.
 */
class Window_BattleStatus : public Window_Selectable {
public:
	/**
	 * Constructor.
	 */
	Window_BattleStatus(int ix, int iy, int iwidth, int iheight);

	/**
	 * Renders the current status on the window.
	 */
	void Refresh();

	/**
	 * Updates the window state.
	 */
	void Update();

	/**
	 * Sets the active character.
	 *
	 * @param index character index (0..3).
	 *              Returns -1 if no character is ready. FIXME
	 */
	void SetActiveCharacter(int index);

	/**
	 * Gets the active character.
	 *
	 * @return character index (0..3).
	 */
	int GetActiveCharacter();

	/**
	 * Selects an active character if one is ready.
	 */
	void ChooseActiveCharacter();

protected:
	/**
	 * Updates the cursor rectangle.
	 */
	void UpdateCursorRect();

	/**
	 * Redraws a character's time gauge.
	 *
	 * @param i character index (0..3).
	 */
	void RefreshGauge(int i);

	/**
	 * Draws a character's time gauge.
	 *
	 * @param actor actor.
	 * @param index character index (0..3).
	 * @param cx x coordinate.
	 * @param cy y coordinate.
	 */
	void DrawGauge(Game_Actor* actor, int index, int cx, int cy);

	friend class Scene_Battle;
};

#endif
