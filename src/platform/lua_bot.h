#ifndef _LUA_BOT_H_
#define _LUA_BOT_H_

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
	 * @param script script to run in bot
	 */
	LuaBot(std::string const& script);

	/**
	 * Register EasyRPG bindings to VM.
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
