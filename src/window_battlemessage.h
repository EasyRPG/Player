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
#include <deque>
#include "window_message.h"

/**
 * Displays messages during a battle
 */
class Window_BattleMessage : public Window_Message {

public:
	enum MessageMode {
		/** Behaves like a normal message box */
		Mode_Normal,
		/** Special logic for the enemy listing at beginning of battle */
		Mode_EnemyEncounter,
		/** Special logic for displaying actions during battle step */
		Mode_Action
	};

	Window_BattleMessage(int ix, int iy, int iwidth, int iheight);

	/**
	 * Starts message processing by reading all
	 * non-displayed from Game_Message.
	 */
	void StartMessageProcessing();

	/**
	 * Ends the message processing.
	 */
	void FinishMessageProcessing();

	void SetMessageMode(MessageMode new_mode);
	void UpdateMessage();

private:
	void UpdateMessageEnemyEncounter();
	void UpdateMessageAction();

	MessageMode mode;
};

#endif
