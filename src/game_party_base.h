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

#ifndef _GAME_PARTY_BASE_H_
#define _GAME_PARTY_BASE_H_

// Headers
#include <vector>
#include <map>
#include "game_actor.h"
#include "main_data.h"

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

/**
 * Game_Party_Base class. Base class of the two Parties (Allied and Enemy)
 */
class Game_Party_Base {
public:

private:
	boost::ptr_vector<Game_Battler> members;
};

#endif
