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

#ifndef EP_WINDOW_BATTLESTATUS_H
#define EP_WINDOW_BATTLESTATUS_H

// Headers
#include "window_selectable.h"
#include "bitmap.h"

/**
 * Window BattleStatus Class.
 * Displays the party battle status.
 */
class Window_BattleStatus : public Window_Selectable {
public:
	enum ChoiceMode {
		/** Allow selection of any actor */
		ChoiceMode_All,
		/** Allow selection of alive actors */
		ChoiceMode_Alive,
		/** Allow selection of dead actors */
		ChoiceMode_Dead,
		/** Allow selection of ready (gauge full) actors (RPG2k3 only) */
		ChoiceMode_Ready,
		/** Don't allow changing the current selection (if any) */
		ChoiceMode_None
	};

	/**
	 * Constructor.
	 */
	Window_BattleStatus(int ix, int iy, int iwidth, int iheight, bool enemy = false);

	/**
	 * Renders the current status on the window.
	 */
	void Refresh();

	/**
	 * Updates the window state.
	 */
	void Update() override;

	/**
	 * Selects an active character if one is ready.
	 */
	int ChooseActiveCharacter();

	/**
	 * Defines which characters can be selected in the dialog.
	 *
	 * @param new_mode new selection mode
	 */
	void SetChoiceMode(ChoiceMode new_mode);

	void RefreshActiveFromValid();

protected:
	/**
	 * Updates the cursor rectangle.
	 */
	void UpdateCursorRect() override;

	/**
	 * Redraws the characters time gauge.
	 */
	void RefreshGauge();

	void DrawGaugeSystem2(int x, int y, int cur_value, int max_value, int which);
	void DrawNumberSystem2(int x, int y, int value);

	/**
	 * Tests whether actor is selectable in current ChoiceMode.
	 *
	 * @return true: selection possible 
	 */
	bool IsChoiceValid(const Game_Battler& battler) const;

	ChoiceMode mode;

	// Debug helper
	bool enemy;

	FileRequestBinding request_id;

	int actor_face_height = 24;
};

#endif
