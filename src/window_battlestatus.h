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
	 * Renders the current status on the window.  Updates incrementally, based on current party state when called.
	 * This allows Refresh() to be called every frame without significant CPU load.
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
	 * Get the appropriate actor for the index from the player or enemy party.
	 */
	const Game_Battler* GetActorForItem(int i_actor);

	/**
	 * Clear the graphics for one character's row or face portrait area.
	 */
	void ClearItemGraphics(int i_item);
	
	/**
	 * Redraws the characters time gauge, as well as HP and SP if applicable.
	 */
	void RefreshGauge(const Game_Battler* actor, int i_item, bool hp_changed, bool hp_max_changed, bool sp_changed, bool sp_max_changed);
	
	/**
	 * Draw the time gauge for BattleType_traditional (A) and BattleType_alternative (B).
	 */
	void DrawGauge(int cx, int cy, int cur_value, int max_value, bool is_full, bool draw_graphic, int alpha = 255);
	
	/**
	 * Used to determine if the ATB gauge needs redrawing.
	 * 
	 * @return the pixel width of the ATB gauge 
	 */
	int GaugeWidth(int cur_value, int max_value);
	
	/**
	 * Draw the gauge bars for Battletype_gauge (C)
	 */
	void DrawGaugeSystem2(int x, int y, int cur_value, int max_value, int which);
	/**
	 * Used to determine if the a gauge needs redrawing.
	 * 
	 * @return the pixel width of the HP, SP, or ATB gauge 
	 */
	int GaugeWidthSystem2(int cur_value, int max_value);
	/**
	 * Draw the time gauge for Battletype_gauge (C)
	 */
	void DrawNumberSystem2(int x, int y, int value);

	/**
	 * Tests whether actor is selectable in current ChoiceMode.
	 *
	 * @return true: selection possible 
	 */
	bool IsChoiceValid(const Game_Battler& battler) const;

	ChoiceMode mode;

	bool isEmpty = true;

	// Debug helper
	bool enemy;

	FileRequestBinding request_id;

	int actor_face_height = 24;
	struct BattleItemState {
		int i_item = 0;

		bool is_drawn = false;
		//bool has_drawn_face = false;
		int actor_id = -1; // Full redraw if this changes
		std::string actor_name = "";

		int hp = -1;
		int max_hp = -1;
		int sp = -1;
		int max_sp = -1;
		std::string state_name = "";
		int32_t state_color;

		std::string face_name = "";
		int face_index = -1;

		int atb_bar_width = INT_MAX;
		bool atb_bar_full = false;
		bool has_opaque_gauge = true;

		void reset() {
		is_drawn = false;
		//has_drawn_face = false;
		actor_id = -1;
		actor_name = "";
		hp = -1;
		max_hp = -1;
		sp = -1;
		max_sp = -1;
		state_name = "";
		state_color = 0;
		atb_bar_width = INT_MAX;
		atb_bar_full = false;
		has_opaque_gauge = true;
		face_name = "";
		face_index = -1;
		}
	};

	// Contains the current displayed data in this view. This is compared with 
	// new data, in order to make incremental updates to the UI.
	BattleItemState itemStates[4];
};

#endif
