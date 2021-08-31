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

#ifndef EP_AUDIO_DECODER_H
#define EP_AUDIO_DECODER_H

// Headers
#include "audio_decoder_base.h"

/**
 * The AudioDecoder class provides an abstraction over the decoding of
 * common audio formats.
 * Create will instantiate a proper audio decoder and calling Decode will
 * fill a buffer with audio data which must be passed to the audio hardware.
 */
class AudioDecoder : public AudioDecoderBase {
public:
	/**
	 * Parses the specified stream and open a proper audio decoder to handle
	 * the audio file.
	 * Upon success the stream is owned by the audio decoder and further
	 * operations on it will be undefined! Upon failure the stream points at
	 * the beginning.
	 * The filename is used for debug purposes but should match the FILE handle.
	 * Upon failure the FILE handle is valid and points at the beginning.
	 *
	 * @param stream handle to parse
	 * @param resample Whether the decoder shall be wrapped into a resampler (if supported)
	 * @return An audio decoder instance when the format was detected, otherwise null
	 */
	static std::unique_ptr<AudioDecoderBase> Create(Filesystem_Stream::InputStream& stream, bool resample = true);

	/**
	 * Returns the amount of bytes per sample.
	 *
	 * @param format Sample format
	 * @return Bytes per sample
	 */
	static int GetSamplesizeForFormat(AudioDecoderBase::Format format);

	/**
	 * Pauses the audio decoding.
	 * Calling any Decode function will return a 0-buffer.
	 */
	void Pause() final override;

	/**
	 * Resumes the audio decoding.
	 * The decode function will continue behaving as expected.
	 */
	void Resume() final override;

	/**
	 * Gets the volume of the audio decoder.
	 * Volume changes will not really modify the volume but are only helper
	 * functions for retrieving the volume information for the audio hardware.
	 *
	 * @return current volume (from 0 - 100)
	 */
	int GetVolume() const final override;

	/**
	 * Sets the volume of the audio decoder.
	 * Volume changes will not really modify the volume but are only helper
	 * functions for retrieving the volume information for the audio hardware.
	 *
	 * @param volume (from 0-100)
	 */
	void SetVolume(int volume) final override;

	/**
	 * Prepares a volume fade in/out effect.
	 * To do a fade out begin must be larger then end.
	 * Call Update to do the fade.
	 *
	 * @param end End volume (from 0-100)
	 * @param duration Fade duration in ms
	 */
	void SetFade(int end, std::chrono::milliseconds duration) final override;

	/**
	 * Updates the volume for the fade in/out effect.
	 * Volume changes will not really modify the volume but are only helper
	 * functions for retrieving the volume information for the audio hardware.
	 *
	 * @param delta Time in us since the last call of this function.
	 */
	void Update(std::chrono::microseconds delta) final override;
private:
	bool paused = false;
	float volume = 0.0f;
	float log_volume = 0.0f; // as used by RPG_RT

	int fade_volume_end = 0;
	std::chrono::microseconds fade_time = std::chrono::microseconds(0);
	float delta_volume_step = 0.0f;
};

#endif
