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

#ifndef EP_WINDOW_SHOPPARTY_H
#define EP_WINDOW_SHOPPARTY_H

// Headers
#include "window_base.h"
#include "async_handler.h"

/**
 * Window ShopParty Class.
 * Displays the party in the shop scene.
 */
class Window_ShopParty : public Window_Base {
public:
	/**
	 * Constructor.
	 */
	Window_ShopParty(Scene* parent, int ix, int iy, int iwidth, int iheight);

	/**
	 * Renders the current party on the window.
	 */
	void Refresh();

	/**
	 * Updates the window state.
	 */
	void Update() override;

	/**
	 * Sets the reference item.
	 */
	void SetItemId(int item_id);

protected:
	/** Reference item. */
	int item_id;
	/** Animation cycle. */
	int cycle;

	/**
	 * Character bitmaps.
	 * bitmaps[actor#][anim phase][equippable]
	 */
	BitmapRef bitmaps[4][3][2];

	void OnCharsetSpriteReady(FileRequestResult* result, int party_index);

	/** Animation rate. */
	static const int anim_rate = 12;

	std::vector<FileRequestBinding> request_ids;
};

#endif
