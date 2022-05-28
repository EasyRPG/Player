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

#ifndef EP_GAME_CLOCK_H
#define EP_GAME_CLOCK_H

#include "options.h"
#include <chrono>
#include "platform/clock.h"
#include <type_traits>
#include <algorithm>

/**
 * Used for time keeping in Player
 */
class Game_Clock {
public:
	using clock = Platform_Clock;

	using rep = clock::rep;
	using period = clock::period;
	using duration = clock::duration;
	using time_point = clock::time_point;

	static constexpr bool is_steady = clock::is_steady;

	/** Get current time */
	static time_point now();

	/** Sleep for the specified duration */
	template <typename R, typename P>
	static void SleepFor(std::chrono::duration<R,P> dt);

	/** Get the target frames per second for the game simulation */
	static constexpr int GetTargetGameFps();

	/** Get the amount of time each logical frame should take */
	static constexpr duration GetTargetGameTimeStep();

	/** Get the timestep for a given frames per second value */
	static constexpr duration TimeStepFromFps(int fps);

	/** Get the name of the underlying clock type */
	static constexpr const char* Name();

	/** Log information about the Game_Clock */
	static void logClockInfo();

	/**
	 * Set the maximum amount of simulation time we'll allow to pass per frame.
	 * Normally, if the main loop is slow we'll run multiple simulation steps per
	 * frame to keep the game running at constant time. However if the main loop falls
	 * below this value, we'll cap how many simulation steps we run and start running
	 * the game loop slower. This can happen on slow machines, frame spikes, large
	 * file IO, or debugger breakpoints.
	 */
	static void SetMaxGameTimePerFrame(duration dt);

	/** Set the speed up or slowdown factor we'll use to run the game. */
	static void SetGameSpeedFactor(float speed);

	/** @return the speed up or slowdown factor we'll use to run the game. */
	static float GetGameSpeedFactor();

	/** Get the time of the current frame */
	static time_point GetFrameTime();

	/** @return the number of the current frame */
	static int GetFrame();

	/** @return the estimated real frames per second */
	static float GetFPS();

	/**
	 * Call on each frame. Updates the current frame time to now
	 *
	 * @param now the current time
	 *
	 * @return The amount of time elapsed since the previous frame.
	 */
	static Game_Clock::duration OnNextFrame(time_point now);

	/**
	 * Call before running the next time step. Tells us whether we should simulate one more
	 * step and decrements the accumulator if so.
	 *
	 * @return Whether we should run an update
	 */
	static bool NextGameTimeStep();

	/**
	 * Reset the frame accumulator and time
	 *
	 * @param now the current time
	 * @param reset_frame_counter if true, reset the frame count also.
	 */
	static void ResetFrame(time_point now, bool reset_frame_counter = false);
private:
	struct Data {
		time_point frame_time;
		duration frame_accumulator;
		duration max_frame_accumulator = std::chrono::duration_cast<duration>(std::chrono::milliseconds(200));
		float speed = 1.0;
		float fps = 0.0;
		int frame = 0;
	};
	static Data data;
};

inline Game_Clock::time_point Game_Clock::now() {
	return clock::now();
}

constexpr int Game_Clock::GetTargetGameFps() {
	return DEFAULT_FPS;
}

constexpr Game_Clock::duration Game_Clock::GetTargetGameTimeStep() {
	return TimeStepFromFps(GetTargetGameFps());
}

constexpr Game_Clock::duration Game_Clock::TimeStepFromFps(int fps) {
	auto ns = std::chrono::nanoseconds(std::chrono::seconds(1)) / fps;
	return std::chrono::duration_cast<Game_Clock::duration>(ns);
}

template <typename R, typename P>
inline void Game_Clock::SleepFor(std::chrono::duration<R,P> dt) {
	clock::SleepFor(dt);
}

constexpr const char* Game_Clock::Name() {
	return clock::Name();
}

inline Game_Clock::time_point Game_Clock::GetFrameTime() {
	return data.frame_time;
}

inline int Game_Clock::GetFrame() {
	return data.frame;
}

inline float Game_Clock::GetFPS() {
	return data.fps;
}

inline bool Game_Clock::NextGameTimeStep() {
	constexpr auto dt = GetTargetGameTimeStep();
	if (data.frame_accumulator < dt) {
		return false;
	}
	data.frame_accumulator -= dt;
	return true;
}

inline void Game_Clock::SetMaxGameTimePerFrame(duration dt) {
	data.max_frame_accumulator = std::max(dt, GetTargetGameTimeStep());
}

inline void Game_Clock::SetGameSpeedFactor(float s) {
	data.speed = s;
}

inline float Game_Clock::GetGameSpeedFactor() {
	return data.speed;
}

#endif
