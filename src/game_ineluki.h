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
#include "string_view.h"
#include "async_handler.h"

/**
 * Implements Ineluki's Key Patch
 */
class Game_Ineluki {
public:
	~Game_Ineluki();

	/**
	 * Executes the specified sound effect (which is actually an INI file) as a keypatch script.
	 * The file is not requested asynchroniously.
	 *
	 * @param se Sound Effect (INI file) to execute
	 * @return Whether the file is a valid script
	 */
	bool Execute(const lcf::rpg::Sound& se);

	/**
	 * Executes the specified INI file as a keypatch script.
	 * The file is not requested asynchroniously.
	 *
	 * @param ini_file INI file to execute
	 * @return Whether the file is a valid script
	 */
	bool Execute(StringView ini_file);

	/**
	 * Executes a file containing a list of script files.
	 * Usually used for the autorun.script on startup.
	 * The scripts are requested asynchroniously as important files and executed in
	 * order but the file itself is not fetched.
	 *
	 * @param list_file File to process
	 * @return Whether the file was found
	 */
	bool ExecuteScriptList(StringView list_file);

	/**
	 * Returns the normal midi ticks or an element from the output list depending
	 * on the output state
	 *
	 * @return Midi Ticks or output list
	 */
	int GetMidiTicks();

	/**
	 * Updates the key up/down list. Must be called once per update frame.
	 */
	void Update();

private:
	/**
	 * Parses and caches the script.
	 *
	 * @param ini_file Script to parse
	 * @return Whether the file is a valid script
	 */
	bool Parse(StringView ini_file);

	struct InelukiCommand {
		std::string name;
		std::string arg;
		std::string arg2;
		std::string arg3;
	};

	using command_list = std::vector<InelukiCommand>;
	std::map<std::string, command_list> functions;

	enum class OutputMode {
		/** GetMidiTicks returns the audio ticks */
		Original,
		/** GetMidiTicks returns from the output list */
		Output
	};

	OutputMode output_mode = OutputMode::Original;
	std::vector<int> output_list;

	struct KeyItem {
		/** Key to watch */
		Input::Keys::InputKey key;
		/** Value to push when key event occurs */
		int value;
	};

	/** List of key down events to watch */
	std::vector<KeyItem> keylist_down;
	/** List of key up events to watch */
	std::vector<KeyItem> keylist_up;

	bool key_support = false;
	bool mouse_support = false;
	int mouse_id_prefix = 0;

	struct Mapping {
		Input::Keys::InputKey key;
		const char* name;
	};

	/**
	 * Mapping from input key to Ineluki key (yes, they are German)
	 * ä. ö, ü and other German keyboard keys are not supported
	 */
	static constexpr std::array<Mapping, 61> key_to_ineluki = {{
		{Input::Keys::LEFT, "(links)"},
		{Input::Keys::RIGHT, "(rechts)"},
		{Input::Keys::UP, "(hoch)"},
		{Input::Keys::DOWN, "(runter)"},
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
		{Input::Keys::RETURN, "(enter)"},
		{Input::Keys::SPACE, "(space)"},
		{Input::Keys::BACKSPACE, "(backspace)"},
		{Input::Keys::CTRL, "(strg)"},
		{Input::Keys::ALT, "(alt)"},
		{Input::Keys::CAPS_LOCK, "(capslock)"},
		{Input::Keys::NUM_LOCK, "(numlock)"},
		{Input::Keys::SCROLL_LOCK, "(scrolllock)"},
		// FIXME: Why does Ineluki have runter (down) and hoch (up)?
		{Input::Keys::LSHIFT, "(lshift runter)"},
		{Input::Keys::RSHIFT, "(rshift runter)"},
		{Input::Keys::LSHIFT, "(lshift hoch)"},
		{Input::Keys::RSHIFT, "(rshift hoch)"},
	}};

	struct CheatItem {
		/** Cheat code */
		std::vector<Input::Keys::InputKey> keys;
		/** Value to push when cheat was entered */
		int value = 0;
		/** Current index in the cheatcode */
		int index = 0;

		CheatItem(const std::string& code, int value);
	};
	std::vector<CheatItem> cheatlist;

	void OnScriptFileReady(FileRequestResult* result);
	struct AsyncArgs {
		FileRequestBinding binding;
		std::string script_name;
		bool invoked = false;

		AsyncArgs(FileRequestBinding binding, std::string script_name) :
			binding(std::move(binding)), script_name(std::move(script_name)) {}
	};
	std::vector<AsyncArgs> async_scripts;
};

#endif
