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

#ifndef _INPUT_SOURCE_H_
#define _INPUT_SOURCE_H_

#include <bitset>
#include <fstream>
#include <memory>
#include "input_buttons.h"

namespace Input {
	/**
	 * A source for button presses.
	 */
	class Source {
	public:
		Source() = default;

		virtual ~Source() = default;

		/**
		 * Called once each frame to update pressed_buttons.
		 */
		virtual void Update() = 0;

		const std::bitset<BUTTON_COUNT>& GetPressedButtons() const {
			return pressed_buttons;
		}

	protected:
		std::bitset<BUTTON_COUNT> pressed_buttons;
	};

	/**
	 * Source that maps key presses from the UI to button
	 * presses.
	 */
	class UiSource : public Source {
	public:
		UiSource() = default;
		~UiSource() override = default;

		void Update() override;

		// NOTE: buttons/dir_buttons/InitButtons could be moved here
	};

	/**
	 * Source that replays button presses from a log file.
	 */
	class LogSource : public Source {
	public:
		LogSource(const char* log_path);
		~LogSource() override = default;

		void Update() override;

		operator bool() const { return bool(log_file); }
	private:
		std::ifstream log_file;
	};

	extern std::unique_ptr<Source> source;
}

#endif
