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

#ifndef EP_ASYNC_OP_H
#define EP_ASYNC_OP_H

#include <utility>
#include <cassert>

/**
 * Represents an asynchronous game operation. These are usually created
 * by event interpreters. When an async operation starts, the entire game
 * loop is supposed to suspend, perform the async operation, and
 * then resume from the suspension point.
 **/
class AsyncOp {
	public:
		/** The different types of async operations */
		enum Type {
			eNone,
			eShowScreen,
			eEraseScreen,
			eCallInn,
			eToTitle,
			eExitGame
		};

		AsyncOp() = default;

		/** @return a ShowScreen async operation */
		static AsyncOp MakeShowScreen(int transition_type);

		/** @return an EraseScreen async operation */
		static AsyncOp MakeEraseScreen(int transition_type);

		/** @return an CallInn async operation */
		static AsyncOp MakeCallInn();

		/** @return a ToTitle async operation */
		static AsyncOp MakeToTitle();

		/** @return an ExitGame async operation */
		static AsyncOp MakeExitGame();

		/** @return the type of async operation */
		Type GetType() const;

		/** @return true if this AsyncOp is active */
		bool IsActive() const;

		/**
		 * @return the type of screen transition to perform
		 * @pre If GetType() is not eShowScreen or eEraseScreen, the return value is undefined.
		 **/
		int GetTransitionType() const;

	private:
		Type _type = eNone;
		int _args[1] = {};

		template <typename... Args>
		explicit AsyncOp(Type type, Args&&... args);

};

inline AsyncOp::Type AsyncOp::GetType() const {
	return _type;
}

inline bool AsyncOp::IsActive() const {
	return GetType() != eNone;
}

inline int AsyncOp::GetTransitionType() const {
	assert(GetType() == eShowScreen || GetType() == eEraseScreen);
	return _args[0];
}

template <typename... Args>
inline AsyncOp::AsyncOp(Type type, Args&&... args)
	: _type(type), _args{std::forward<Args>(args)...}
{}

inline AsyncOp AsyncOp::MakeShowScreen(int transition_type) {
	return AsyncOp(eShowScreen, transition_type);
}

inline AsyncOp AsyncOp::MakeEraseScreen(int transition_type) {
	return AsyncOp(eEraseScreen, transition_type);
}

inline AsyncOp AsyncOp::MakeCallInn() {
	return AsyncOp(eCallInn);
}

inline AsyncOp AsyncOp::MakeToTitle() {
	return AsyncOp(eToTitle);
}

inline AsyncOp AsyncOp::MakeExitGame() {
	return AsyncOp(eExitGame);
}

#endif

