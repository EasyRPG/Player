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

#ifndef EP_GAME_INELUKI_H
#define EP_GAME_INELUKI_H

// Headers
#include <string>
#include <vector>
#include <map>

#include <lcf/rpg/sound.h>

#include "keys.h"

/**
 * Implements Ineluki's Key Patch
 */
class Game_Ineluki {
public:
	Game_Ineluki();

	bool Execute(const lcf::rpg::Sound& se);

	int GetMidiTicks();

	void Update();

private:
	bool Parse(const lcf::rpg::Sound& se);

	struct InelukiCommand {
		std::string name;
		std::string arg;
		std::string arg2;
		std::string arg3;
	};

	using command_list = std::vector<InelukiCommand>;

	std::map<std::string, command_list> functions;

	enum class OutputMode {
		Original,
		Output
	};

	OutputMode output_mode = OutputMode::Original;
	std::vector<int> output_list;
	std::vector<Input::Keys::InputKey> keylist;

	bool key_support = false;
	bool mouse_support = false;
	int mouse_id_prefix = 0;
};

#endif
