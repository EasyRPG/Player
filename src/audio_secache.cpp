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

// Headers
#include <cassert>
#include <cstring>
#include <map>
#include <set>
#include "audio_resampler.h"
#include "audio_secache.h"
#include "baseui.h"
#include "filefinder.h"
#include "output.h"

namespace {
	typedef std::map<std::string, AudioSeRef> cache_type;

	cache_type cache;

	const int cache_limit = 1 * 1024 * 1024;
	int cache_size = 0;

	void FreeCacheMemory() {
		int32_t cur_ticks = DisplayUi->GetTicks();

		for (auto it = cache.begin(); it != cache.end(); ) {
			if (it->second.use_count() > 1) {
				// Somebody uses this SE right now
				++it;
				continue;
			}

			if (cur_ticks - it->second->last_access < 5000) {
				// Last access < 5s
				++it;
				continue;
			}

			//Output::Debug("SE: Freeing memory of %s", it->first.c_str());

			cache_size -= it->second->buffer.size();

			it = cache.erase(it);
		}
	}
}

std::unique_ptr<AudioSeCache> AudioSeCache::Create(const std::string& filename) {
	std::unique_ptr<AudioSeCache> se;

	cache_type::iterator const it = cache.find(filename);

	se.reset(new AudioSeCache());
	se->filename = filename;

	if (it == cache.end()) {
		// Not in cache

		FILE *f = FileFinder::fopenUTF8(filename, "rb");

		if (!f) {
			se.reset();
			return se;
		}

		se->audio_decoder = AudioDecoder::Create(f, filename);

		if (se->audio_decoder) {
			if (!se->audio_decoder->Open(f)) {
				se->audio_decoder.reset();
			}
		}

		if (!se->audio_decoder) {
			se.reset();
			return se;
		}
	}

	return se;
}

void AudioSeCache::GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const {
	if (!audio_decoder) {
		if (!GetCachedFormat(frequency, format, channels)) {
			frequency = 0;
			channels = 0;
			format = AudioDecoder::Format::S16;
		}

		return;
	}

	audio_decoder->GetFormat(frequency, format, channels);

	if (mono_to_stereo_resample) {
		channels = 2;
	}
}

bool AudioSeCache::SetFormat(int frequency, AudioDecoder::Format format, int channels) {
	int cfrequency;
	AudioDecoder::Format cformat;
	int cchannels;

	if (!audio_decoder) {
		// This file is already cached, don't allow uncached format changes
		if (GetCachedFormat(cfrequency, cformat, cchannels)) {
			return frequency == cfrequency &&
					format == cformat &&
					channels == cchannels;
		}
	}

	bool success = audio_decoder->SetFormat(frequency, format, channels);

	if (!success) {
		audio_decoder->GetFormat(cfrequency, cformat, cchannels);
		// Handle Mono->Stereo conversion, is quite common for SE
		if (cfrequency == frequency && cformat == format && cchannels == 1 && channels == 2) {
			mono_to_stereo_resample = true;
			return true;
		}
	}

	return success;
}

int AudioSeCache::GetPitch() const {
	return pitch;
}

bool AudioSeCache::SetPitch(int pitch) {
#ifdef USE_AUDIO_RESAMPLER
	this->pitch = pitch;

	return pitch > 0;
#else
	return pitch == 100;
#endif
}

bool AudioSeCache::IsCached() const {
	return cache.find(filename) != cache.end();
}

bool AudioSeCache::GetCachedFormat(int& frequency, AudioDecoder::Format& format, int& channels) const {
	cache_type::const_iterator it = cache.find(filename);

	if (it != cache.end()) {
		frequency = (*it).second->frequency;
		format = (*it).second->format;
		channels = (*it).second->channels;

		return true;
	}

	return false;
}

class MemoryPitchResampler : public AudioDecoder {
public:
	MemoryPitchResampler(const AudioSeRef se) :
		se(se) {
		// no-op
	}

	bool Open(FILE*) {
		// No file operations needed
		return true;
	}

	bool IsFinished() const {
		return offset >= se->buffer.size();
	}

	void GetFormat(int& frequency, Format& format, int& channels) const {
		frequency = se->frequency;
		format = se->format;
		channels = se->channels;
	}

private:
	int FillBuffer(uint8_t* buffer, int size) {
		int real_size = size;

		if (offset + size > se->buffer.size()) {
			real_size = se->buffer.size() - offset;
		}

		memcpy(buffer, se->buffer.data() + offset, real_size);
		offset += real_size;

		return real_size;
	}

	const AudioSeRef se;
	size_t offset = 0;
};

AudioSeRef AudioSeCache::Decode() {
	// Writes a SE with pitch = 100 to the cache if it is not cached yet,
	// otherwise returns the cached result
	// For pitch != 100 the cached result is pitch-adjusted and returned.

	AudioSeRef se;

	if (IsCached()) {
		se = cache.find(filename)->second;
		se->last_access = DisplayUi->GetTicks();

		if (GetPitch() == 100) {
			// Nothing extra to do
			return se;
		}

#ifdef USE_AUDIO_RESAMPLER
		// Code path is only taken with a resampler, otherwise pitch is always 100 here
		// Falls through to the decoding logic but does not overwrite the cache

		// Don't overwrite our real cached entry
		se.reset(new AudioSeData());
		GetCachedFormat(se->frequency, se->format, se->channels);

		audio_decoder.reset(new AudioResampler(new MemoryPitchResampler(cache.find(filename)->second)));
		audio_decoder->Open(nullptr);
#else
		assert(false && "SeCache: Unexpected code path taken");
#endif
	}

	if (!se) {
		se.reset(new AudioSeData());
	}

	GetFormat(se->frequency, se->format, se->channels);

	if (IsCached()) {
		audio_decoder->SetPitch(GetPitch());
	} else {
		audio_decoder->SetPitch(100);
	}

	const int buffer_size = 8192;
	se->buffer.resize(buffer_size);

	while (!audio_decoder->IsFinished()) {
		int read = audio_decoder->Decode(se->buffer.data() + se->buffer.size() - buffer_size, buffer_size);
		if (read < 8192) {
			se->buffer.resize(se->buffer.size() - (buffer_size - read));
			break;
		}

		se->buffer.resize(se->buffer.size() + buffer_size);
	}

	if (mono_to_stereo_resample) {
		se->buffer.resize(se->buffer.size() * 2);

		int sample_size = AudioDecoder::GetSamplesizeForFormat(se->format);

		// Duplicate data from the back, allows writing to the buffer directly
		for (size_t i = se->buffer.size() / 2 - sample_size; i > 0; i -= sample_size) {
			// left channel
			memcpy(&se->buffer[i * 2 - sample_size * 2], &se->buffer[i], sample_size);
			// right channel
			memcpy(&se->buffer[i * 2 - sample_size], &se->buffer[i], sample_size);
		}
	}

	if (!IsCached()) {
		// Write normal pitched sample to cache
		// This codepath is only taken the first time upon cache miss
		cache.insert(std::make_pair(filename, se));

		se->last_access = DisplayUi->GetTicks();

		cache_size += se->buffer.size();

		if (cache_size > cache_limit) {
			FreeCacheMemory();
		}

		//Output::Debug("cache %f", cache_size / 1024.0f / 1024.0f);

		if (GetPitch() != 100) {
			// Also handle a requested resampling
			// Takes the "IsCached" codepath now
			mono_to_stereo_resample = false;
			return Decode();
		}
	}

	return se;
}

void AudioSeCache::Clear() {
	cache_size = 0;
	cache.clear();
}
