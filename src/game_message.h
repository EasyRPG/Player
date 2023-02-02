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

#ifndef EP_GAME_MESSAGE_H
#define EP_GAME_MESSAGE_H

#include <cstdint>
#include <vector>
#include <bitset>
#include <string>
#include <functional>
#include "string_view.h"
#include "pending_message.h"
#include "memory_management.h"

class Window_Message;
class AsyncOp;

namespace Game_Message {

	static const int MAX_LINE = 4;

	/** Set the window used to display the text */
	void SetWindow(Window_Message* window);

	Window_Message* GetWindow();

	AsyncOp Update();

	/** Contains the different lines of text. */
	extern std::vector<std::string> texts;

	/**
	 * Determines the position of the message box respecting the player's map
	 * position and if obstructing is allowed.
	 *
	 * @return Best message box position
	 */
	int GetRealPosition();

	/**
	 * Sets pending message text for the messaging system to display.
	 *
	 * @param pm - the pending message text
	 */
	void SetPendingMessage(PendingMessage&& pm);

	/** Callback type for WordWrap function */
	using WordWrapCallback = const std::function<void(StringView line)>;

	/**
	 * Breaks the line into lines, each of which is equal
	 * or less than a specified limit in pixels in the
	 * given font (except in cases when breaking by spaces
	 * can't produce a short line), and calls the callback
	 * for each resulting line.
	 * 
	 * Font::Default() will be used to determine the word breaking.
	 * The caller is responsible for ensuring that Font::Default()
	 * either does not change between calling this function and
	 * displaying the results, or at least that the changed font
	 * has same metrics as the font used to calculate the line sizes.
	 *
	 * @param[in] line The line that will be broken into lines
	 * and added into the lines vector.
	 * @param[in] limit maximum size of each line after word-breaking.
	 * @param callback a function to be called for each word-wrapped line
	 */
	int WordWrap(StringView line, int limit, const WordWrapCallback& callback);
	int WordWrap(StringView line, int limit, const WordWrapCallback& callback, const Font& font);

	/**
	 * Return if it's legal to show a new message box.
	 *
	 * @param foreground true if this is in the foreground context, otherwise parallel context.
	 * @return true if we can show a message box.
	 */
	bool CanShowMessage(bool foreground);

	/** @return true if there is message text pending */
	bool IsMessagePending();
	/** @return true if message window is running */
	bool IsMessageActive();

	// EasyRPG extension allowing more recursive variables \v[\v[...]]
	static constexpr int easyrpg_default_max_recursion = 8;
	// RPG_RT only allows 1 level of recursion.
	static constexpr int rpg_rt_default_max_recursion = 1;
	// Which one we'll use by default.
	static constexpr int default_max_recursion = easyrpg_default_max_recursion;

	/** Struct returned by parameter parsing methods */
	struct ParseParamResult {
		/** iterator to the next character after parsed content */
		const char* next = nullptr;
		/** value that was parsed */
		int value = 0;
	};

	/** Parse a \v[] variable string
	 *
	 * @param iter start of utf8 string
	 * @param end end of utf8 string
	 * @param escape_char the escape character to use
	 * @param skip_prefix if true, assume prefix was already parsed and iter starts at the first left bracket.
	 * @param max_recursion How many times to allow recursive variable lookups.
	 *
	 * @return \refer ParseParamResult
	 */
	ParseParamResult ParseVariable(const char* iter, const char* end, uint32_t escape_char, bool skip_prefix = false, int max_recursion = default_max_recursion);

	/** Parse a \c[] color string
	 *
	 * @param iter start of utf8 string
	 * @param end end of utf8 string
	 * @param escape_char the escape character to use
	 * @param skip_prefix if true, assume prefix was already parsed and iter starts at the first left bracket.
	 * @param max_recursion How many times to allow recursive variable lookups.
	 *
	 * @return \refer ParseParamResult
	 */
	ParseParamResult ParseColor(const char* iter, const char* end, uint32_t escape_char, bool skip_prefix = false, int max_recursion = default_max_recursion);

	/** Parse a \s[] speed string
	 *
	 * @param iter start of utf8 string
	 * @param end end of utf8 string
	 * @param escape_char the escape character to use
	 * @param skip_prefix if true, assume prefix was already parsed and iter starts at the first left bracket.
	 * @param max_recursion How many times to allow recursive variable lookups.
	 *
	 * @return \refer ParseParamResult
	 */
	ParseParamResult ParseSpeed(const char* iter, const char* end, uint32_t escape_char, bool skip_prefix = false, int max_recursion = default_max_recursion);

	/** Parse a \n[] actor name string
	 *
	 * @param iter start of utf8 string
	 * @param end end of utf8 string
	 * @param escape_char the escape character to use
	 * @param skip_prefix if true, assume prefix was already parsed and iter starts at the first left bracket.
	 * @param max_recursion How many times to allow recursive variable lookups.
	 *
	 * @return \refer ParseParamResult
	 */
	ParseParamResult ParseActor(const char* iter, const char* end, uint32_t escape_char, bool skip_prefix = false, int max_recursion = default_max_recursion);
}


#endif
