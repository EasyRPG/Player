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

#ifndef EP_AUDIO_SECACHE_H
#define EP_AUDIO_SECACHE_H

// Headers
#include <cstdio>
#include <string>
#include <vector>
#include <memory>
#include <map>

#include "audio_decoder.h"
#include "game_clock.h"

class AudioSeCache;

/**
 * AudioSeData contains the decoded sample of AudioSeCache.
 */
class AudioSeData {
public:
	std::vector<uint8_t> buffer;
	Game_Clock::time_point last_access;
	int frequency;
	AudioDecoder::Format format;
	int channels;
};

typedef std::shared_ptr<AudioSeData> AudioSeRef;

/**
 * AudioSeDecoder operates on supplied AudioSeData and does format
 * conversions through the resamplers.
 */
class AudioSeDecoder : public AudioDecoder {
public:
	AudioSeDecoder(AudioSeRef se);

	bool Open(FILE*) override { return true; };
	bool IsFinished() const override;
	void GetFormat(int& frequency, Format& format, int& channels) const override;
	int GetPitch() const override;

private:
	int FillBuffer(uint8_t* buffer, int size) override;

	AudioSeRef se;
	size_t offset = 0;
};

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
	 * @param channels Filled with the amount of channels
	 */
	void GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const;

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
	 * @param channels Filled with the amount of channels
	 * @return true when format was cached, the values were populated
	 */
	bool GetCachedFormat(int& frequency, AudioDecoder::Format& format, int& channels) const;

	/**
	 * Decodes the whole sample without doing any resampling and caches it.
	 * When cached the decoding step is skipped.
	 * The returned AudioDecoder contains the SE sample.
	 *
	 * @return Decoded sound effect
	 */
	std::unique_ptr<AudioDecoder> CreateSeDecoder();

	/**
	 * Returns the SE sample data handled by this SeCache.
	 *
	 * @return sample data
	 */
	AudioSeRef GetSeData() const;

	static void Clear();
private:
	std::unique_ptr<AudioDecoder> audio_decoder;

	std::string filename;
};

#endif
