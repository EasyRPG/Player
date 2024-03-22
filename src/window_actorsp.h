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

#ifndef EP_WINDOW_ACTOR_SP_H
#define EP_WINDOW_ACTOR_SP_H

// Headers
#include "window_base.h"
#include "text.h"

/**
 * Window_ActorSp class.
 * Shows Actor skill points for traditional 2k3 battle system.
 */
class Window_ActorSp : public Window_Base {

public:
	/**
	 * Constructor.
	 */
	Window_ActorSp(Scene* parent, int ix, int iy, int iwidth, int iheight);

	/**
	 * Sets the battler whose SP will be shown.
	 *
	 * @param battler battler to use.
	 */
	void SetBattler(const Game_Battler& battler);
};

#endif
