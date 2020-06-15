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

#ifndef EP_AUDIO_MIDIOUT_H
#define EP_AUDIO_MIDIOUT_H

#include <string>
#include "filesystem_stream.h"
#include "audio_midiout_device.h"

/**
 * Manages sequencing MIDI files and emitting MIDI events to a MIDI out 
 * device.
 */
class MidiOut {
public:
	virtual ~MidiOut() = default;

	static std::unique_ptr<MidiOut> Create(Filesystem_Stream::InputStream& stream, const std::string& filename);

	/**
	 * Prepares a volume fade in/out effect.
	 * To do a fade out begin must be larger then end.
	 * Call Update to do the fade.
	 * Volume changes will not really modify the volume but are only helper
	 * functions for retrieving the volume information for the audio hardware.
	 *
	 * @param begin Begin volume (from 0-100)
	 * @param end End volume (from 0-100)
	 * @param duration Fade duration in ms
	 */
	virtual void SetFade(int begin, int end, int duration) {
		(void)begin;
		(void)end;
		(void)duration;
	}

	/**
	 * Sets the volume of the audio decoder.
	 * Volume changes will not really modify the volume but are only helper
	 * functions for retrieving the volume information for the audio hardware.
	 *
	 * @param volume (from 0-100)
	 */
	virtual void SetVolume(int volume) {
		(void)volume;
	}

	/**
	 * Gets the volume of the audio decoder.
	 * Volume changes will not really modify the volume but are only helper
	 * functions for retrieving the volume information for the audio hardware.
	 */
	virtual int GetVolume() const = 0;

	/**
	 * Pauses the MIDI sequencer.
	 */
	virtual void Pause() {};

	/**
	 * Resumes the MIDI sequencer.
	 */
	virtual void Resume() {};

	/**
	 * Rewinds the MIDI sequencer to the beginning.
	 */
	virtual void Rewind() {};

	/**
	 * Seeks in the MIDI sequence. The value of offset is implementation
	 * defined but is guaranteed to match the result of Tell.
	 * Libraries must support at least seek from the start for Rewind().
	 *
	 * @param offset Offset to seek to
	 * @param origin Position to seek from
	 * @return Whether seek was successful
	 */
	virtual bool Seek(std::streamoff offset, std::ios_base::seekdir origin) {
		(void)offset;
		(void)origin;
		return false;
	}

	/**
	 * Gets if the MIDI will loop when it finishes.
	 *
	 * @return if looping
	 */
	bool GetLooping() const {
		return looping;
	};

	/**
	 * Enables/Disables MIDI looping.
	 * When looping is enabled IsFinished will never return true and the 
	 * sequencer auto-rewinds
	 *
	 * @param enable Enable/Disable looping
	 */
	void SetLooping(bool enable) {
		looping = enable;
	};

	/**
	 * Gets the number of loops
	 *
	 * @return loop count
	 */
	virtual int GetLoopCount() const = 0;

	/**
	 * Determines whether the MIDI sequence is finished.
	 *
	 * @return true sequence ended
	 */
	virtual bool IsFinished() const = 0;

	/* Call this every ~1ms. 
	 * @param delta Time in microseconds since last called. 
	 */
	virtual void Update(long long delta) = 0;

	/**
	 * Provides an error message when Open or a Decode function fail.
	 *
	 * @return Human readable error message
	 */
	virtual std::string GetError() const = 0;

	/**
	 * Returns a value suitable for the GetMidiTicks command.
	 * For MIDI this is the amount of MIDI ticks.
	 *
	 * @return Amount of MIDI ticks
	 */
	virtual int GetTicks() const {
		return 0;
	};

	/**
	 * Gets the pitch multiplier.
	 *
	 * @return pitch multiplier
	 */
	virtual int GetPitch() const {
		return 0;
	};

	/**
	 * Sets the pitch multiplier.
	 * 100 = normal speed
	 * 200 = double speed and so on
	 *
	 * @param pitch Pitch multiplier to use
	 * @return true if pitch was set, false otherwise
	 */
	virtual bool SetPitch(int pitch) {
		return false;
	};

	/**
	 * Resets all state and instructs the MIDI device to do the same.
	 */
	virtual void Reset() = 0;

	virtual bool Open(Filesystem_Stream::InputStream stream) = 0;

protected:
	bool looping = false;
};

#endif
