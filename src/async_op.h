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

#include "string_view.h"
#include <string>
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
			eQuickTeleport,
			eToTitle,
			eExitGame,
			eTerminateBattle,
			eSave,
			eLoad,
			eLoadParallel,
			eYield,
			eYieldRepeat,
			eCloneMapEvent,
			eDestroyMapEvent
		};

		AsyncOp() = default;

		/** @return a ShowScreen async operation */
		static AsyncOp MakeShowScreen(int transition_type);

		/** @return an EraseScreen async operation */
		static AsyncOp MakeEraseScreen(int transition_type);

		/** @return a CallInn async operation */
		static AsyncOp MakeCallInn();

		/** @return a QuickTeleport async operation */
		static AsyncOp MakeQuickTeleport(int map_id, int x, int y);

		/** @return a ToTitle async operation */
		static AsyncOp MakeToTitle();

		/** @return an ExitGame async operation */
		static AsyncOp MakeExitGame();

		/** @return a TerminateBattle async operation */
		static AsyncOp MakeTerminateBattle(int result);

		/** @return a Save async operation */
		static AsyncOp MakeSave(int save_slot, int save_result_var);

		/** @return a Load async operation */
		static AsyncOp MakeLoad(int save_slot);

		/** @return a LoadParallel async operation (to be used for patch compatibility) */
		static AsyncOp MakeLoadParallel(int save_slot);

		/** @return a Yield for one frame to e.g. fetch an important asset */
		static AsyncOp MakeYield();

		/** @return a Yield for one frame and repeat the command to e.g. fetch an important asset */
		static AsyncOp MakeYieldRepeat();

		/** @return a clone map event async operation */
		static AsyncOp MakeCloneMapEvent(std::string name, int src_event_id, int target_event_id, int map_id, int x, int y);

		/** @return a destroy map event async operation */
		static AsyncOp MakeDestroyMapEvent(int target_event_id);

		/** @return the type of async operation */
		Type GetType() const;

		/** @return true if this AsyncOp is active */
		bool IsActive() const;

		/**
		 * @return the type of screen transition to perform
		 * @pre If GetType() is not eShowScreen or eEraseScreen, the return value is undefined.
		 **/
		int GetTransitionType() const;

		/**
		 * @return the map id to teleport to
		 * @pre If GetType() is not eQuickTeleport, the return value is undefined.
		 */
		int GetTeleportMapId() const;

		/**
		 * @return the x coordinate to teleport to
		 * @pre If GetType() is not eQuickTeleport, the return value is undefined.
		 */
		int GetTeleportX() const;

		/**
		 * @return the y coordinate teleport to
		 * @pre If GetType() is not eQuickTeleport, the return value is undefined.
		 */
		int GetTeleportY() const;

		/**
		 * @return the desired result of the battle to terminate.
		 * @pre If GetType() is not eTerminateBattle, the return value is undefined.
		 **/
		int GetBattleResult() const;

		/**
		 * @return the desired slot to save or load
		 * @pre If GetType() is not eSave, eLoad, eLoadParallel, the return value is undefined.
		 **/
		 int GetSaveSlot() const;

		/**
		 * @return the variable to set to 1 when the save was a success.
		 * @pre If GetType() is not eSave, the return value is undefined.
		 **/
		int GetSaveResultVar() const;

		/**
		 * @return the event id of the event being cloned
		 * @pre If GetType() is not eCloneMapEvent, the return value is undefined.
		 */
		int GetSourceEventId() const;

		/**
		 * @return the event id of the event being created
		 * @pre If GetType() is not eCloneMapEvent or eDestroyMapEvent, the return value is undefined.
		 */
		int GetTargetEventId() const;

		/**
		 * @return the map id where the event is cloned from
		 * @pre If GetType() is not eCloneMapEvent, the return value is undefined.
		 */
		int GetMapId() const;

		/**
		 * @return the x coordinate where to spawn the event
		 * @pre If GetType() is not eCloneMapEvent, the return value is undefined.
		 */
		int GetX() const;

		/**
		 * @return the y coordinate where to spawn the event
		 * @pre If GetType() is not eCloneMapEvent, the return value is undefined.
		 */
		int GetY() const;

		/**
		 * @return the new name of the event
		 * @pre If GetType() is not eCloneMapEvent, the return value is undefined.
		 */
		std::string_view GetEventName() const;

	private:
		Type _type = eNone;
		int _args[5] = {};
		std::string _str_arg;

		template <typename... Args>
		explicit AsyncOp(Type type, Args&&... args);

		template <typename... Args>
		explicit AsyncOp(Type type, std::string str_arg, Args&&... args);
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

inline int AsyncOp::GetTeleportMapId() const {
	assert(GetType() == eQuickTeleport);
	return _args[0];
}

inline int AsyncOp::GetTeleportX() const {
	assert(GetType() == eQuickTeleport);
	return _args[1];
}

inline int AsyncOp::GetTeleportY() const  {
	assert(GetType() == eQuickTeleport);
	return _args[2];
}

inline int AsyncOp::GetBattleResult() const {
	assert(GetType() == eTerminateBattle);
	return _args[0];
}

inline int AsyncOp::GetSaveSlot() const {
	assert(GetType() == eSave || GetType() == eLoad || GetType() == eLoadParallel);
	return _args[0];
}

inline int AsyncOp::GetSaveResultVar() const {
	assert(GetType() == eSave);
	return _args[1];
}

inline int AsyncOp::GetSourceEventId() const {
	assert(GetType() == eCloneMapEvent);
	return _args[0];
}

inline int AsyncOp::GetTargetEventId() const {
	assert(GetType() == eCloneMapEvent || GetType() == eDestroyMapEvent);
	return _args[1];
}

inline int AsyncOp::GetMapId() const {
	assert(GetType() == eCloneMapEvent);
	return _args[2];
}

inline int AsyncOp::GetX() const {
	assert(GetType() == eCloneMapEvent);
	return _args[3];
}

inline int AsyncOp::GetY() const {
	assert(GetType() == eCloneMapEvent);
	return _args[4];
}

inline std::string_view AsyncOp::GetEventName() const {
	assert(GetType() == eCloneMapEvent);
	return _str_arg;
}

template <typename... Args>
inline AsyncOp::AsyncOp(Type type, Args&&... args)
	: _type(type), _args{std::forward<Args>(args)...}
{}

template <typename... Args>
inline AsyncOp::AsyncOp(Type type, std::string str_arg, Args&&... args)
	: _type(type), _args{std::forward<Args>(args)...}, _str_arg(std::move(str_arg))
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

inline AsyncOp AsyncOp::MakeQuickTeleport(int map_id, int x, int y) {
	return AsyncOp(eQuickTeleport, map_id, x, y);
}

inline AsyncOp AsyncOp::MakeToTitle() {
	return AsyncOp(eToTitle);
}

inline AsyncOp AsyncOp::MakeExitGame() {
	return AsyncOp(eExitGame);
}

inline AsyncOp AsyncOp::MakeTerminateBattle(int transition_type) {
	return AsyncOp(eTerminateBattle, transition_type);
}

inline AsyncOp AsyncOp::MakeSave(int save_slot, int save_result_var) {
	return AsyncOp(eSave, save_slot, save_result_var);
}

inline AsyncOp AsyncOp::MakeLoad(int save_slot) {
	return AsyncOp(eLoad, save_slot);
}

inline AsyncOp AsyncOp::MakeLoadParallel(int save_slot) {
	return AsyncOp(eLoadParallel, save_slot);
}

inline AsyncOp AsyncOp::MakeYield() {
	return AsyncOp(eYield);
}

inline AsyncOp AsyncOp::MakeYieldRepeat() {
	return AsyncOp(eYieldRepeat);
}

inline AsyncOp AsyncOp::MakeCloneMapEvent(std::string name, int src_event_id, int target_event_id, int map_id, int x, int y) {
	return AsyncOp(eCloneMapEvent, name, src_event_id, target_event_id, map_id, x, y);
}

inline AsyncOp AsyncOp::MakeDestroyMapEvent(int target_event_id) {
	return AsyncOp(eDestroyMapEvent, 0, target_event_id);
}

#endif
