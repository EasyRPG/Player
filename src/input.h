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

#ifndef EP_INPUT_H
#define EP_INPUT_H

// Headers
#include <vector>
#include <bitset>
#include "point.h"
#include "system.h"
#include "input_buttons.h"
#include "input_source.h"
#include "keys.h"
#include "game_config.h"

/**
 * Input namespace.
 * Input works with Button states. Buttons are
 * representations of one or more keys or actions (like
 * keyboard keys, mouse buttons, joystick axis). This way
 * buttons are platform and device independent, while the
 * assigned keys can vary by the system.
 */
namespace Input {
	/**
	 * Initializes Input.
	 *
	 * @param cfg input configuration
	 * @param replay_from_path path to a log file to
	 *  replay from, or the empty string if not replaying
	 * @param record_to_path path to a file to record
	 *  input to, or the empty string if not recording
	 */
	void Init(
		Game_ConfigInput cfg,
		const std::string& replay_from_path,
		const std::string& record_to_path
	);

	/**
	 * Updates Input state.
	 */
	void Update();

	/**
	 * Updates Input state for only system keys not used by game logic
	 */
	void UpdateSystem();

	/**
	 * Resets all button states.
	 */
	void ResetKeys();

	/**
	 * Resets only triggered states.
	 */
	void ResetTriggerKeys();

	/**
	 * Loads the default mapping for a button
	 *
	 * @param button Button to restore mapping of
	 */
	void ResetDefaultMapping(InputButton button);

	/**
	 * Loads the default mapping for all buttons
	 */
	void ResetAllMappings();

	/**
	 * Gets if a button is being pressed.
	 *
	 * @param button button ID.
	 * @return whether the button is being pressed.
	 */
	bool IsPressed(InputButton button);

	/**
	 * Gets if a button is starting to being pressed.
	 *
	 * @param button button ID.
	 * @return whether the button is being triggered.
	 */
	bool IsTriggered(InputButton button);

	/**
	 * Gets if a button is being repeated. A button is being
	 * repeated while it is maintained pressed and a
	 * certain amount of frames has passed after last
	 * repetition.
	 *
	 * @param button button ID.
	 * @return whether the button is being repeated.
	 */
	bool IsRepeated(InputButton button);

	/**
	 * Gets if a button is being released.
	 *
	 * @param button button ID.
	 * @return whether the button is being released.
	 */
	bool IsReleased(InputButton button);

	/**
	 * Gets if any button is being pressed.
	 *
	 * @return whether any button is being pressed.
	 */
	bool IsAnyPressed();

	/**
	 * Gets if any button is being triggered.
	 *
	 * @return whether any button is being triggered.
	 */
	bool IsAnyTriggered();

	/**
	 * Gets if any button is being repeated.
	 *
	 * @return whether any button is being repeated.
	 */
	bool IsAnyRepeated();

	/**
	 * Gets if any button is being released.
	 *
	 * @return whether any button is being released.
	 */
	bool IsAnyReleased();

	/**
	 * Gets if a system button is being pressed.
	 *
	 * @param button button ID.
	 * @return whether the button is being pressed.
	 */
	bool IsSystemPressed(InputButton button);

	/**
	 * Gets if a system button is starting to being pressed.
	 *
	 * @param button button ID.
	 * @return whether the button is being triggered.
	 */
	bool IsSystemTriggered(InputButton button);

	/**
	 * Gets if a system button is being repeated. A button is being
	 * repeated while it is maintained pressed and a
	 * certain amount of frames has passed after last
	 * repetition.
	 *
	 * @param button button ID.
	 * @return whether the button is being repeated.
	 */
	bool IsSystemRepeated(InputButton button);

	/**
	 * Gets if a system button is being released.
	 *
	 * @param button button ID.
	 * @return whether the button is being released.
	 */
	bool IsSystemReleased(InputButton button);

	/**
	 * Gets all buttons being pressed.
	 *
	 * @return a vector with the buttons IDs.
	 */
	std::vector<InputButton> GetAllPressed();

	/**
	 * Gets all buttons being triggered.
	 *
	 * @return a vector with the buttons IDs.
	 */
	std::vector<InputButton> GetAllTriggered();

	/**
	 * Gets all buttons being repeated.
	 *
	 * @return a vector with the buttons IDs.
	 */
	std::vector<InputButton> GetAllRepeated();

	/**
	 * Gets all buttons being released.
	 *
	 * @return a vector with the buttons IDs.
	 */
	std::vector<InputButton> GetAllReleased();

	/**
	 * Returns the key mask for manipulation. When a bit is set the key is
	 * ignored even if it is mapped. Only raw reads will return the key state.
	 *
	 * @return key mask
	 */
	Input::KeyStatus GetMask();

	/**
	 * @param new_mask The new key mask to set
	 */
	void SetMask(Input::KeyStatus new_mask);

	/**
	 * Resets the key mask. All keys are reported again.
	 */
	void ResetMask();

	/*
	 * Gets if a key is pressed.
	 * Low level function accessing keys directly bypassing the button mapping.
	 *
	 * @param key key ID.
	 * @return whether the key is being pressed.
	 */
	bool IsRawKeyPressed(Input::Keys::InputKey key);

	/*
	 * Gets if a key is triggered.
	 * Low level function accessing keys directly bypassing the button mapping.
	 *
	 * @param key key ID.
	 * @return whether the key is being released.
	 */
	bool IsRawKeyTriggered(Input::Keys::InputKey key);

	/*
	 * Gets if a key is released.
	 * Low level function accessing keys directly bypassing the button mapping.
	 *
	 * @param key key ID.
	 * @return whether the key is being released.
	 */
	bool IsRawKeyReleased(Input::Keys::InputKey key);

	/**
	 * Gets all raw keys being pressed.
	 *
	 * @return a vector with the key IDs.
	 */
	const Input::KeyStatus& GetAllRawPressed();

	/**
	 * Gets all raw keys being triggered.
	 *
	 * @return a vector with the key IDs.
	 */
	const Input::KeyStatus& GetAllRawTriggered();

	/**
	 * Gets all raw keys being released.
	 *
	 * @return a vector with the key IDs.
	 */
	const Input::KeyStatus& GetAllRawReleased();

	/**
	 * @return Position of the mouse cursor relative to the screen
	 */
	Point GetMousePosition();

	/**
	 * Used to submit additional metadata for input recording
	 * @param type type of data sent
	 * @param data Sent data
	 */
	void AddRecordingData(RecordingData type, StringView data);

	/** @return If the input is recorded */
	bool IsRecording();

	/**
	 * Used to access the underlying input source.
	 * Only use this for low level access!
	 * @return the input source
	 */
	 Source* GetInputSource();

	/** Buttons press time (in frames). */
	extern std::array<int, BUTTON_COUNT> press_time;

	/** Buttons trigger state. */
	extern std::bitset<BUTTON_COUNT> triggered;

	/** Buttons trigger state. */
	extern std::bitset<BUTTON_COUNT> repeated;

	/** Buttons trigger state. */
	extern std::bitset<BUTTON_COUNT> released;

	/** Raw keys triggered state. */
	extern std::bitset<Input::Keys::KEYS_COUNT> raw_triggered;

	/** Raw keys pressed state. */
	extern std::bitset<Input::Keys::KEYS_COUNT> raw_pressed;

	/** Raw keys released state. */
	extern std::bitset<Input::Keys::KEYS_COUNT> raw_released;

	/** Horizontal and vertical directions state. */
	extern int dir4;

	/** All cardinal directions state. */
	extern int dir8;

	bool IsWaitingInput();
	void WaitInput(bool val);
}

#endif
