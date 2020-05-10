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

#include "game_clock.h"
#include "output.h"

#include <thread>
#include <cinttypes>
#include <algorithm>

constexpr bool Game_Clock::is_steady;
Game_Clock::Data Game_Clock::data;

// Damping factor fps computation.
static constexpr auto _fps_smooth = 2.0f / 121.0f;

Game_Clock::duration Game_Clock::OnNextFrame(time_point now) {
	const auto mfa = std::chrono::duration_cast<duration>(data.max_frame_accumulator * data.speed);

	const auto dt = now - data.frame_time;
	data.frame_time = now;
	data.frame_accumulator += std::chrono::duration_cast<duration>(dt * data.speed);
	data.frame_accumulator = std::min(data.frame_accumulator, mfa);

	const auto fps = (1.0f / std::chrono::duration<float>(dt).count());
	data.fps = (data.fps * _fps_smooth) + (fps * (1.0f - _fps_smooth));

	++data.frame;

	return dt;
}

void Game_Clock::ResetFrame(time_point now, bool reset_frame_counter) {
	data.frame_time = now;
	data.frame_accumulator = {};
	data.fps = 0.0;
	if (reset_frame_counter) {
		data.frame = 0;
	}
}

void Game_Clock::logClockInfo() {
	const char* period_name = "custom";
	if (std::is_same<period,std::nano>::value) {
		period_name = "ns";
	} else if (std::is_same<period,std::micro>::value) {
		period_name = "us";
	} else if (std::is_same<period,std::milli>::value) {
		period_name = "ms";
	}
	Output::Debug("Clock: {} steady={} period={} ({} / {})",
			Name(),
			is_steady,
			period_name,
			period::num,
			period::den);
}
