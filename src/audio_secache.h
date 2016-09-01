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

#ifndef EASYRPG_AUDIO_SECACHE_H
#define EASYRPG_AUDIO_SECACHE_H

// Headers
#include <cstdio>
#include <string>
#include <vector>
#include <memory>
#include <map>

#include "audio_decoder.h"

/**
 * AudioSeData is the decoded sample of AudioSeCache.
 * Format changes and pitch are already applied to the buffer.
 */
class AudioSeData {
public:
	std::vector<uint8_t> buffer;
	int frequency;
	AudioDecoder::Format format;
	int channels;
	int last_access;
};

typedef std::shared_ptr<AudioSeData> AudioSeRef;

/**
 * AudioSeCache provides an interface for accessing sound effects.
 * It also provides an automatic cache management, any SE is only decoded
 * once, otherwise returned from the cache.
 * The cache is flushed from recently (>10 seconds by default) unused
 * samples when it reaches the memory limit (5 MB by default).
 * Uses an internal AudioDecoder for handling the decoding.
 */
class AudioSeCache {
public:
	/**
	 * Opens the passed filename with the internal audio decoder.
	 *
	 * @param filename Path to the file
	 * @return An AudioSeCache instance when the format was detected, otherwise null
	 */
	static std::unique_ptr<AudioSeCache> Create(const std::string& filename);

	/**
	 * Retrieves the format of the internal audio decoder.
	 * It is guaranteed that these settings will stay constant the whole time.
	 *
	 * @param frequency Filled with the audio frequency
	 * @param format Filled with the audio format
	 * @param channel Filled with the amount of channels
	 */
	void GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const;

	/**
	 * Requests a prefered format from the internal audio decoder. Not all decoders
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
	bool SetFormat(int frequency, AudioDecoder::Format format, int channels);

	/**
	 * Gets the pitch multiplier of the internal audio decoder.
	 *
	 * @return pitch multiplier
	 */
	int GetPitch() const;

	/**
	 * Sets the pitch multiplier of the internal audio decoder.
	 * 100 = normal speed
	 * 200 = double speed and so on
	 * 
	 * @param pitch Pitch multiplier to use
	 * @return true if pitch was set, false otherwise
	 */
	bool SetPitch(int pitch);

	/**
	 * Tells if Decode will have a cache hit when executed.
	 * SetFormat will fail when this returns true.
	 *
	 * @return true if SE is already in cache, false otherwise.
	 */
	bool IsCached() const;

	/**
	 * Retrieves the format of the cached file.
	 * It is guaranteed that these settings will stay constant the whole time.
	 * When the format is not cached (false returned) the arguments are not
	 * populated.
	 *
	 * @param frequency Filled with the audio frequency
	 * @param format Filled with the audio format
	 * @param channel Filled with the amount of channels
	 * @return true when format was cached, the values were populated
	 */
	bool GetCachedFormat(int& frequency, AudioDecoder::Format& format, int& channels) const;

	/**
	 * Decodes the whole sample with the settings specified by SetFormat and
	 * SetPitch (uses default settings of the audio file if not used).
	 * In case of a cache hit the decoding is skipped.
	 * Calling Decode multiple times with different settings is supported.
	 *
	 * @return Decoded sound effect
	 */
	AudioSeRef Decode();

	static void Clear();
private:
	int pitch = 100;

	std::unique_ptr<AudioDecoder> audio_decoder;

	std::string filename;

	bool mono_to_stereo_resample = false;
};

#endif
