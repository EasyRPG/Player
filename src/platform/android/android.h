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

#ifndef EP_ANDROID_H
#define EP_ANDROID_H

#include <mutex>
#include <functional>

namespace EpAndroid {
	inline std::function<void()> android_fn;
	inline std::mutex android_mutex;

	inline void invoke() {
		if (!android_fn) {
			return;
		}

		android_mutex.lock();
		if (android_fn) {
			android_fn();
			android_fn = nullptr;
		}
		android_mutex.unlock();
	}

	template<typename F>
	inline void schedule(F&& fn) {
		android_mutex.lock();
		android_fn = std::move(fn);
		android_mutex.unlock();
	}
}

#endif
