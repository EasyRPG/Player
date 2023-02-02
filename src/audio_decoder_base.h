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

#ifndef EP_AUDIO_DECODER_BASE_H
#define EP_AUDIO_DECODER_BASE_H

// Headers
#include <cstdio>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include "filesystem_stream.h"

/**
 * The AudioDecoder class provides an abstraction over the decoding of
 * common audio formats.
 * This is the base class containing shared logic.
 * See AudioDecoder for sample based decoding and AudioDecoderMidi for
 * Midi event based decoding.
 */
class AudioDecoderBase {
public:
	/**
	 * Takes a linear volume and converts it to a logarithmic used by
	 * RPG_RT (Direct Sound).
	 * Do not use this for Midi, is already logarithmic by design.
	 *
	 * @param volume linear volume
	 * @return logarithmic volume
	 */
	static float AdjustVolume(float volume);

	virtual ~AudioDecoderBase() = default;

	/** Sample format */
	enum class Format {
		S8,
		U8,
		S16,
		U16,
		S32,
		U32,
		F32
	};

	/**
	 * Writes 'size' bytes in the specified buffer. The data matches the format
	 * reported by GetFormat.
	 * When size is is smaller then the amount of written bytes or an error occurs
	 * the remaining buffer space is cleared.
	 *
	 * @param buffer Output buffer
	 * @param size Size of the buffer in bytes
	 * @return Number of bytes written to the buffer or -1 on error
	 */
	int Decode(uint8_t* buffer, int size);

	/**
	 * Decodes the whole audio sample. The data matches the format reported by
	 * GetFormat.
	 *
	 * @return output buffer
	 */
	std::vector<uint8_t> DecodeAll();

	/**
	 * Rewinds the audio stream to the beginning.
	 */
	void Rewind();

	/**
	 * Gets if the audio stream will loop when the stream finishes.
	 *
	 * @return if looping
	 */
	virtual bool GetLooping() const;

	/**
	 * Enables/Disables audio stream looping.
	 * When looping is enabled IsFinished will never return true and the stream
	 * auto-rewinds (assuming Rewind is supported)
	 *
	 * @param enable Enable/Disable looping
	 */
	virtual void SetLooping(bool enable);

	/**
	 * Gets the number of loops
	 *
	 * @return loop count
	 */
	virtual int GetLoopCount() const;

	// Functions to be implemented by the audio decoder
	/**
	 * Assigns a stream to the audio decoder.
	 * Open should be only called once per audio decoder instance.
	 * Use GetError to get the error reason on failure.
	 *
	 * @return true if initializing was successful, false otherwise
	 */
	virtual bool Open(Filesystem_Stream::InputStream stream) = 0;

	/**
	 * Pauses the audio decoding.
	 */
	virtual void Pause() = 0;

	/**
	 * Resumes the audio decoding.
	 */
	virtual void Resume() = 0;

	/**
	 * Gets the current volume of the audio decoder.
	 * Fades are considered.
	 *
	 * @return current volume (from 0 - 100)
	 */
	virtual int GetVolume() const = 0;

	/**
	 * Sets the current volume of the audio decoder.
	 *
	 * @param volume (from 0-100)
	 */
	virtual void SetVolume(int volume) = 0;

	/**
	 * Prepares a volume fade in/out effect.
	 * To do a fade out begin must be larger then end.
	 * Call Update to do the fade.
	 *
	 * @param end End volume (from 0-100)
	 * @param duration Fade duration in ms
	 */
	virtual void SetFade(int end, std::chrono::milliseconds duration) = 0;

	/**
	 * Seeks in the audio stream. The value of offset is implementation
	 * defined but is guaranteed to match the result of Tell.
	 * Libraries must support at least seek from the start for Rewind().
	 *
	 * @param offset Offset to seek to
	 * @param origin Position to seek from
	 * @return Whether seek was successful
	 */
	virtual bool Seek(std::streamoff offset, std::ios_base::seekdir origin) = 0;

	/**
	 * Determines whether the stream is finished.
	 *
	 * @return true stream ended
	 */
	virtual bool IsFinished() const = 0;

	/**
	 * Updates the volume for the fade in/out effect.
	 *
	 * @param delta Time in us since the last call of this function.
	 */
	virtual void Update(std::chrono::microseconds delta) = 0;

	/**
	 * Updates Midi output. Only used by Midi out devices.
	 * For Midi out devices this must be called at least once per ms.
	 *
	 * @param delta Time in us since the last call of this function.
	 */
	virtual void UpdateMidi(std::chrono::microseconds delta);

	/**
	 * Retrieves the format of the audio decoder.
	 * It is guaranteed that these settings will stay constant the whole time.
	 *
	 * @param frequency Filled with the audio frequency
	 * @param format Filled with the audio format
	 * @param channels Filled with the amount of channels
	 */
	virtual void GetFormat(int& frequency, Format& format, int& channels) const = 0;

	/**
	 * Requests a preferred format from the audio decoder. Not all decoders
	 * support everything and it's recommended to use the audio hardware
	 * for audio processing.
	 * When false is returned use GetFormat to get the real format of the
	 * output data.
	 *
	 * @param frequency Audio frequency
	 * @param format Audio format
	 * @param channels Number of channels
	 * @return true when all settings were set, otherwise false (use GetFormat)
	 */
	virtual bool SetFormat(int frequency, Format format, int channels);

	/**
	 * Gets the pitch multiplier.
	 *
	 * @return pitch multiplier
	 */
	virtual int GetPitch() const;

	/**
	 * Sets the pitch multiplier.
	 * 100 = normal speed
	 * 200 = double speed and so on
	 * When not supported a resampler will handle the pitch shift.
	 *
	 * @param pitch Pitch multiplier to use
	 * @return true if pitch was set, false otherwise
	 */
	virtual bool SetPitch(int pitch);

	/**
	 * Tells the current stream position. The value is implementation
	 * defined.
	 *
	 * @return Position in the stream
	 */
	virtual std::streampos Tell() const;

	/**
	 * Returns a value suitable for the GetMidiTicks command.
	 * For MIDI this is the amount of MIDI ticks, for other
	 * formats usually the playback position in seconds.
	 *
	 * @return Amount of MIDI ticks or position in seconds
	 */
	virtual int GetTicks() const = 0;

	/**
	 * Gets the status of the newly created audio decoder.
	 * Used to make sure the underlying library is properly initialized.
	 *
	 * @return true if initializing was succesful, false otherwise
	 */
	virtual bool WasInited() const;

	/**
	 * Provides an error message when Open or a Decode function fail.
	 *
	 * @return Human readable error message
	 */
	virtual std::string GetError() const;

	/**
	 * Returns the name of the format the current audio decoder decodes in
	 * lower case.
	 *
	 * @return Format name
	 */
	virtual std::string GetType() const;

protected:
	/**
	 * Called by the Decode functions to fill the buffer.
	 *
	 * @param buffer Buffer to fill
	 * @param size Buffer size
	 * @return number of bytes read or -1 on error
	 */
	virtual int FillBuffer(uint8_t* buffer, int size) = 0;

	std::string error_message;
	std::string music_type;

	bool looping = false;
	int loop_count = 0;

	int Decode(uint8_t* buffer, int length, int recursion_depth);
};

#endif
