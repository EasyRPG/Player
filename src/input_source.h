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

#ifndef EP_INPUT_SOURCE_H
#define EP_INPUT_SOURCE_H

#include <bitset>
#include <fstream>
#include <memory>
#include <fstream>
#include "input_buttons.h"

namespace Input {
	/**
	 * A source for button presses.
	 */
	class Source {
	public:

		static std::unique_ptr<Source> Create(
				ButtonMappingArray buttons,
				DirectionMappingArray directions,
				const std::string& replay_from_path);

		Source(ButtonMappingArray buttons, DirectionMappingArray directions)
			: button_mappings(std::move(buttons)), direction_mappings(std::move(directions)) {}

		virtual ~Source() = default;

		/** Called once each logical frame to update pressed_buttons. */
		virtual void Update() = 0;

		/** Called once each physical frame when no logical frames occured to update pressed_buttons for system buttons. */
		virtual void UpdateSystem() = 0;

		const std::bitset<BUTTON_COUNT>& GetPressedButtons() const {
			return pressed_buttons;
		}
		std::bitset<BUTTON_COUNT> GetPressedNonSystemButtons() const {
			auto pressed = pressed_buttons;
			for(unsigned i = 0; i < BUTTON_COUNT; ++i) {
				if (IsSystemButton(static_cast<InputButton>(i))) {
					pressed[i] = false;
				}
			}
			return pressed;
		}

		ButtonMappingArray& GetButtonMappings() { return button_mappings; }
		const ButtonMappingArray& GetButtonMappings() const { return button_mappings; }

		DirectionMappingArray& GetDirectionMappings() { return direction_mappings; }
		const DirectionMappingArray& GetDirectionMappings() const { return direction_mappings; }

		bool InitRecording(const std::string& record_to_path);

	protected:
		void Record();

		std::bitset<BUTTON_COUNT> pressed_buttons;
		ButtonMappingArray button_mappings;
		DirectionMappingArray direction_mappings;
		std::ofstream record_log;
	};

	/**
	 * Source that maps key presses from the UI to button
	 * presses.
	 */
	class UiSource : public Source {
	public:
		using Source::Source;

		void Update() override;
		void UpdateSystem() override;

	private:
		void DoUpdate(bool system_only);
	};

	/**
	 * Source that replays button presses from a log file.
	 */
	class LogSource : public Source {
	public:
		LogSource(const char* log_path, ButtonMappingArray buttons, DirectionMappingArray directions);

		void Update() override;
		void UpdateSystem() override;

		operator bool() const { return bool(log_file); }
	private:
		std::ifstream log_file;
	};

	extern std::unique_ptr<Source> source;
}

#endif
