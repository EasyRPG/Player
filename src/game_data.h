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

#ifndef EP_GAME_DATA_H
#define EP_GAME_DATA_H

#include <memory>
#include <cassert>
#include "rpg_save.h"

class Game_Player;
class Game_Screen;
class Game_Party;
class Game_EnemyParty;
class Game_Switches;
class Game_Variables;

struct Game_Data {
	public:
		/** @return reference to Game_Party object */
		static Game_Party& GetGameParty();

		/** Construct data for a new game */
		static void SetupNewGame();

		/**
		 * Construct data from given save game
		 *
		 * @param save the save to load from
		 */
		static void SetupLoadGame(RPG::Save save);

		/**
		 * Serialize state to save game format
		 *
		 * @param save the save to write to
		 */
		static void WriteSaveGame(RPG::Save& save);

		/** Destroy all data */
		static void Reset();
	private:
		struct Data {
			std::unique_ptr<Game_Party> game_party;
		};
		static Data data;
};

inline Game_Party& Game_Data::GetGameParty() {
	assert(data.game_party);
	return *data.game_party;
}

#endif

