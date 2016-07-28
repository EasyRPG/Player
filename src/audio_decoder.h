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

#ifndef EASYRPG_AUDIO_DECODER_H
#define EASYRPG_AUDIO_DECODER_H

// Headers
#include <cstdio>
#include <string>
#include <vector>
#include <memory>

/**
 * The AudioDecoder class provides an abstraction over the decoding of
 * common audio formats.
 * Create will instantitate a proper audio decoder and calling Decode will
 * fill a buffer with audio data which must be passed to the audio hardware.
 */
class AudioDecoder {
public:
	virtual ~AudioDecoder() {}

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

	/** Seek origin for Seek command */
	enum class Origin {
		Begin = 0,
		Current = 1,
		End = 2
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
	 * Splits stereo into mono and Writes 'size' bytes in each of the buffers.
	 * The data matches the format reported by GetFormat, except that both
	 * buffers will contain Mono audio. When the source format was already mono
	 * the 'right' buffer is ignored (and not cleared)
	 * When size is is smaller then the amount of written bytes or an error occurs
	 * the remaining buffer space is cleared.
	 *
	 * @param left Output buffer of the left channel
	 * @param right Output buffer of the right channel (or nothing if source is mono)
	 * @param size Size of each of the buffers
	 * @return Number of bytes written in one of the buffers or -1 on error
	 */
	int DecodeAsMono(uint8_t* left, uint8_t* right, int size);

	/**
	 * Parses the specified file handle and open a proper audio decoder to handle
	 * the audio file.
	 * Upon success the file handle is owned by the audio decoder and further
	 * operations on it will be undefined! Upon failure the file handle points at
	 * the beginning.
	 * The filename is used for debug purposes but should match the FILE handle.
	 * Upon failure the FILE handle is valid and points at the beginning.
	 *
	 * @param file File handle to parse
	 * @param filename Path to the file handle
	 * @return An audio decoder instance when the format was detected, otherwise null
	 */
	static std::unique_ptr<AudioDecoder> Create(FILE* file, const std::string& filename);

	/**
	 * Updates the volume for the fade in/out effect.
	 * Volume changes will not really modify the volume but are only helper
	 * functions for retrieving the volume information for the audio hardware.
	 *
	 * @param delta Time in ms since the last call of this function.
	 */
	void Update(int delta);

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
	void SetFade(int begin, int end, int duration);

	/**
	 * Sets the volume of the audio decoder.
	 * Volume changes will not really modify the volume but are only helper
	 * functions for retrieving the volume information for the audio hardware.
	 *
	 * @param volume (from 0-100)
	 */
	void SetVolume(int volume);

	/**
	 * Gets the volume of the audio decoder.
	 * Volume changes will not really modify the volume but are only helper
	 * functions for retrieving the volume information for the audio hardware.
	 */
	int GetVolume() const;

	/**
	 * Pauses the audio decoding.
	 * Calling any Decode function will return a 0-buffer.
	 */
	void Pause();

	/**
	 * Resumes the audio decoding.
	 * The decode function will continue behaving as expected.
	 */
	void Resume();

	/**
	 * Rewinds the audio stream to the beginning.
	 */
	void Rewind();

	/**
	 * Gets if the audio stream will loop when the stream finishes.
	 *
	 * @return if looping
	 */
	bool GetLooping() const;

	/**
	 * Enables/Disables audio stream looping.
	 * When looping is enabled IsFinished will never return true and the stream
	 * auto-rewinds (assuming Rewind is supported)
	 *
	 * @param enable Enable/Disable looping
	 */
	void SetLooping(bool enable);

	/**
	 * Gets the number of loops
	 *
	 * @return loop count
	 */
	int GetLoopCount() const;

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

	// Functions to be implemented by the audio decoder
	/**
	 * Assigns a file handle to the audio decoder.
	 * Open should be only called once per audio decoder instance.
	 * Use GetError to get the error reason on failure.
	 *
	 * @return true if inititalizing was succesful, false otherwise
	 */
	virtual bool Open(FILE* file) = 0;

	/**
	 * Determines whether the stream is finished.
	 *
	 * @return true stream ended
	 */
	virtual bool IsFinished() const = 0;

	/**
	 * Retrieves the format of the audio decoder.
	 * It is guaranteed that these settings will stay constant the whole time.
	 *
	 * @param frequency Filled with the audio frequency
	 * @param format Filled with the audio format
	 * @param channel Filled with the amount of channels
	 */
	virtual void GetFormat(int& frequency, Format& format, int& channels) const = 0;

	/**
	 * Requests a prefered format from the audio decoder. Not all decoders
	 * support everything and it's recommended to use the audio hardware
	 * for audio processing.
	 * When false is returned use GetFormat to get the real format of the
	 * output data.
	 *
	 * @param frequency Audio frequency
	 * @param format Audio format
	 * @param channel Number of channels
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
	 * Not all audio decoders support this. Using the audio hardware is
	 * recommended.
	 * 
	 * @param pitch Pitch multiplier to use
	 * @return true if pitch was set, false otherwise
	 */
	virtual bool SetPitch(int pitch);

	/**
	 * Seeks in the audio stream. The value of offset is implementation
	 * defined but is guaranteed to match the result of Tell.
	 * Only Rewinding is guaranteed to work.
	 *
	 * @param offset Offset to seek to
	 * @param origin Position to seek from
	 * @return Whether seek was successful
	 */
	virtual bool Seek(size_t offset, Origin origin);

	/**
	 * Tells the current stream position. The value is implementation
	 * defined.
	 *
	 * @return Position in the stream
	 */
	virtual size_t Tell() const;

	/**
	 * Returns amount of executed ticks. Only useful for MIDI format.
	 *
	 * @return Amount of MIDI ticks.
	 */
	virtual int GetTicks() const;

	/**
	 * Returns the amount of bytes per sample.
	 *
	 * @param Sample format
	 * @return Bytes per sample
	 */
	static int GetSamplesizeForFormat(AudioDecoder::Format format);
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
private:
	bool paused = false;
	double volume = 0;
	double fade_end = 0;
	double fade_time = -1;
	double delta_step = 0;

	bool looping = false;
	int loop_count = 0;

	std::vector<uint8_t> mono_buffer;
};

#endif
