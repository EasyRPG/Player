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

	struct KeyList {
		Input::Keys::InputKey key;
		int value;
	};

	std::vector<KeyList> keylist_down;
	std::vector<KeyList> keylist_up;

	bool key_support = false;
	bool mouse_support = false;
	int mouse_id_prefix = 0;

	struct Mapping {
		Input::Keys::InputKey key;
		const char* name;
	};

	static constexpr std::array<Mapping, 50> key_to_ineluki = {{
		{Input::Keys::LEFT, "(links)"},
		{Input::Keys::RIGHT, "(rechts)"},
		{Input::Keys::UP, "(oben)"},
		{Input::Keys::DOWN, "(unten)"},
		{Input::Keys::A, "a"},
		{Input::Keys::B, "b"},
		{Input::Keys::C, "c"},
		{Input::Keys::D, "d"},
		{Input::Keys::E, "e"},
		{Input::Keys::F, "f"},
		{Input::Keys::G, "g"},
		{Input::Keys::H, "h"},
		{Input::Keys::I, "i"},
		{Input::Keys::J, "j"},
		{Input::Keys::K, "k"},
		{Input::Keys::L, "l"},
		{Input::Keys::M, "m"},
		{Input::Keys::N, "n"},
		{Input::Keys::O, "o"},
		{Input::Keys::P, "p"},
		{Input::Keys::Q, "q"},
		{Input::Keys::R, "r"},
		{Input::Keys::S, "s"},
		{Input::Keys::T, "t"},
		{Input::Keys::U, "u"},
		{Input::Keys::V, "v"},
		{Input::Keys::W, "w"},
		{Input::Keys::X, "x"},
		{Input::Keys::Y, "y"},
		{Input::Keys::Z, "z"},
		{Input::Keys::N0, "0"},
		{Input::Keys::N1, "1"},
		{Input::Keys::N2, "2"},
		{Input::Keys::N3, "3"},
		{Input::Keys::N4, "4"},
		{Input::Keys::N5, "5"},
		{Input::Keys::N6, "6"},
		{Input::Keys::N7, "7"},
		{Input::Keys::N8, "8"},
		{Input::Keys::N9, "9"},
		{Input::Keys::PERIOD, "."},
		{Input::Keys::TAB, "(tab)"},
		{Input::Keys::DEL, "(entf)"},
		{Input::Keys::ENDS, "(ende)"},
		{Input::Keys::PGDN, "(bildrunter)"},
		{Input::Keys::PGUP, "(bildhoch)"},
		{Input::Keys::HOME, "(pos1)"},
		{Input::Keys::INSERT, "(einfg)"},
		{Input::Keys::ESCAPE, "(esc)"},
		{Input::Keys::RETURN, "(enter)"}
	}};
};

#endif
