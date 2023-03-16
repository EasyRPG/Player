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
#include "filesystem_stream.h"
#include "game_config.h"
#include "game_clock.h"
#include "input_buttons.h"
#include "keys.h"
#include "point.h"

namespace Input {
	using KeyStatus = std::bitset<Input::Keys::KEYS_COUNT>;

	enum class RecordingData : char {
		CommandLine = 'C',
		EventCommand = 'E', // unused - reserved
		Hash = 'L',
		MoveRoute = 'M', // unused - reserved
		GameTitle = 'N'
	};

	/**
	 * Primary and Secondary range from -1.0 to +1.0
	 *       ^ -1
	 *       |
	 * -1 <-----> 1
	 *       |
	 *       v 1
	 * Trigger range from 0.0 (neutral) to 1.0 (pressed)
	 */
	struct AnalogInput {
		PointF primary;
		PointF secondary;
		float trigger_left = 0;
		float trigger_right = 0;

		static constexpr float kMaxValue = 1.0f;
		static constexpr float kMinValue = -1.0f;
	};

	struct TouchInput {
		bool pressed = false;
		Point position;

		// Fields for use by InputSource. Do not modify in Ui!
		bool prev_frame_pressed = false;
		Game_Clock::time_point touch_begin;
		Game_Clock::time_point touch_end;
	};

	/**
	 * A source for button presses.
	 */
	class Source {
	public:
		static std::unique_ptr<Source> Create(
				const Game_ConfigInput& cfg,
				DirectionMappingArray directions,
				const std::string& replay_from_path);

		Source(const Game_ConfigInput& cfg, DirectionMappingArray directions)
			: cfg(cfg), direction_mappings(std::move(directions)) {}

		virtual ~Source() = default;

		/** Called once each logical frame to update pressed_buttons. */
		virtual void Update() = 0;

		/** Called once each physical frame when no logical frames occured to update pressed_buttons for system buttons. */
		virtual void UpdateSystem() = 0;

		/**
		 * Used to submit additional metadata for input recording
		 * @param type type of data sent
		 * @param data Sent data
		 */
		void AddRecordingData(RecordingData type, StringView data);

		/** @return If the input is recorded */
		bool IsRecording() const {
			return bool(record_log);
		}

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
		/** @return Returns raw keystates for reading the keyboard directly.*/
		const std::bitset<Input::Keys::KEYS_COUNT>& GetPressedKeys() const {
			return keystates;
		}

		ButtonMappingArray& GetButtonMappings() { return cfg.buttons; }
		const ButtonMappingArray& GetButtonMappings() const { return cfg.buttons; }

		DirectionMappingArray& GetDirectionMappings() { return direction_mappings; }
		const DirectionMappingArray& GetDirectionMappings() const { return direction_mappings; }

		Game_ConfigInput& GetConfig() { return cfg; }
		const Game_ConfigInput& GetConfig() const { return cfg; }

		bool InitRecording(const std::string& record_to_path);

		const Point& GetMousePosition() const { return mouse_pos; }

		const AnalogInput& GetAnalogInput() const { return analog_input; };

		const KeyStatus& GetMask() const { return keymask; }
		KeyStatus& GetMask() { return keymask; }

	protected:
		void Record();
		void UpdateGamepad();
		void UpdateTouch();

		Game_ConfigInput cfg;

		std::bitset<BUTTON_COUNT> pressed_buttons;
		DirectionMappingArray direction_mappings;
		std::unique_ptr<Filesystem_Stream::OutputStream> record_log;

		KeyStatus keystates;
		KeyStatus keymask;
		Point mouse_pos;
		AnalogInput analog_input;

		int last_written_frame = -1;
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
		LogSource(const char* log_path, const Game_ConfigInput& cfg, DirectionMappingArray directions);

		void Update() override;
		void UpdateSystem() override;

		operator bool() const { return bool(log_file); }
	private:
		Filesystem_Stream::InputStream log_file;
		int version = 1;
		int last_read_frame = -1;
		// NOTE: First field is the frame number
		std::vector<std::string> keys;
	};

	extern std::unique_ptr<Source> source;
}

#endif
