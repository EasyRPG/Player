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

#ifndef _LUA_BOT_H_
#define _LUA_BOT_H_

// Headers
#include "system.h"
#include <string>

extern "C" {
	struct lua_State;
}


struct BotInterface {
	BotInterface() : finished_(false) {}
	virtual ~BotInterface() {}

	/**
	 * Resume the bot until next yield.
	 */
	virtual void resume() = 0;

	/**
	 * Check whether bot execution is finished.
	 * Do not resume if this method returns true.
	 *
	 * @return true if execution is finished.
	 */
	bool is_finished() const { return finished_; }

protected:
	void finish() { finished_ = true; }

private:
	bool finished_;
}; // struct BotInterface

struct LuaBot : public BotInterface {
	/**
	 * Construct bot.
	 *
	 * @param script script to run in bot
	 */
	LuaBot(std::string const& script);

	/**
	 * Register EasyRPG bindings to VM.
	 *
	 * @param L VM to register
	 */
	static void register_vm(lua_State* L);

	void resume();

private:
	std::string const script_;
	EASYRPG_SHARED_PTR<lua_State> vm_;
	lua_State* executer_;
}; // class LuaBot


#endif // _LUA_BOT_H_
