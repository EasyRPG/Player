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
#include <memory>
#include <set>
#include "audio_resampler.h"
#include "audio_secache.h"
#include "game_clock.h"
#include "filefinder.h"
#include "output.h"

using namespace std::chrono_literals;

namespace {
	typedef std::map<std::string, AudioSeRef> cache_type;

	cache_type cache;

	constexpr int cache_limit = 3 * 1024 * 1024;
	int cache_size = 0;

	void FreeCacheMemory() {
		auto cur_time = Game_Clock::GetFrameTime();

		for (auto it = cache.begin(); it != cache.end(); ) {
			if (it->second.use_count() > 1) {
				// SE is currently playing
				++it;
				continue;
			}

			if (cache_size <= cache_limit && cur_time - it->second->last_access < 3s) {
				// Below memory limit and last access < 3s
				++it;
				continue;
			}

#ifdef CACHE_DEBUG
			Output::Debug("SE: Freeing memory of {}", it->first);
#endif

			cache_size -= it->second->buffer.size();

			it = cache.erase(it);
		}

#ifdef CACHE_DEBUG
		Output::Debug("SE cache size: {}", cache_size / 1024.0 / 1024);
#endif
	}
}

std::unique_ptr<AudioSeCache> AudioSeCache::Create(Filesystem_Stream::InputStream stream, StringView name) {
	auto se = std::make_unique<AudioSeCache>();
	se->name = ToString(name);

	auto const it = cache.find(ToString(name));
	if (it == cache.end()) {
		// Not in cache
		if (!stream) {
			return {};
		}

		se->audio_decoder = AudioDecoder::Create(stream, false);

		if (se->audio_decoder) {
			if (!se->audio_decoder->Open(std::move(stream))) {
				se->audio_decoder.reset();
			}
		}

		if (!se->audio_decoder) {
			return {};
		}
	}

	return se;
}

void AudioSeCache::GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const {
	if (!audio_decoder) {
		if (!GetCachedFormat(frequency, format, channels)) {
			assert(false);
		}

		return;
	}

	audio_decoder->GetFormat(frequency, format, channels);
}

std::unique_ptr<AudioSeCache> AudioSeCache::GetCachedSe(StringView name) {
	auto se = std::make_unique<AudioSeCache>();
	se->name = ToString(name);

	auto const it = cache.find(se->name);
	if (it == cache.end()) {
		return {};
	}

	return se;
}

bool AudioSeCache::GetCachedFormat(int& frequency, AudioDecoder::Format& format, int& channels) const {
	cache_type::const_iterator it = cache.find(name);

	if (it != cache.end()) {
		frequency = (*it).second->frequency;
		format = (*it).second->format;
		channels = (*it).second->channels;

		return true;
	}

	return false;
}

std::unique_ptr<AudioDecoderBase> AudioSeCache::CreateSeDecoder() {
	AudioSeRef se;

	auto it = cache.find(name);
	if (it != cache.end()) {
		se = it->second;
		se->last_access = Game_Clock::GetFrameTime();

		std::unique_ptr<AudioDecoderBase> dec = std::make_unique<AudioSeDecoder>(se);
#ifdef USE_AUDIO_RESAMPLER
		dec = std::make_unique<AudioResampler>(std::move(dec));
#endif
		Filesystem_Stream::InputStream is;
		dec->Open(std::move(is));
		return dec;
	}

	// Not cached yet: Decode the sample without any resampling

	if (!se) {
		se.reset(new AudioSeData());
	}

	assert(audio_decoder);

	audio_decoder->GetFormat(se->frequency, se->format, se->channels);
	se->buffer = audio_decoder->DecodeAll();

	cache.insert(std::make_pair(name, se));

	cache_size += se->buffer.size();

#ifdef CACHE_DEBUG
	Output::Debug("SE cache size (Add): {}", cache_size / 1024.0 / 1024.0);
#endif

	FreeCacheMemory();

	std::unique_ptr<AudioDecoderBase> dec = std::unique_ptr<AudioDecoderBase>(new AudioSeDecoder(se));
#ifdef USE_AUDIO_RESAMPLER
	dec = std::unique_ptr<AudioDecoderBase>(new AudioResampler(std::move(dec)));
#endif
	Filesystem_Stream::InputStream is;
	dec->Open(std::move(is));
	return dec;
}

AudioSeRef AudioSeCache::GetSeData() const {
	auto it = cache.find(name);
	assert(it != cache.end());

	return it->second;
};

void AudioSeCache::Clear() {
	cache_size = 0;
	cache.clear();
}

StringView AudioSeCache::GetName() const {
	return name;
}

AudioSeDecoder::AudioSeDecoder(const AudioSeRef& se) :
	se(se) {
	se->last_access = Game_Clock::GetFrameTime();
}

bool AudioSeDecoder::IsFinished() const {
	return offset >= se->buffer.size();
}

void AudioSeDecoder::GetFormat(int &frequency, AudioDecoder::Format &format, int &channels) const {
	frequency = se->frequency;
	format = se->format;
	channels = se->channels;
}

int AudioSeDecoder::FillBuffer(uint8_t *buffer, int size) {
	int real_size = size;

	if (offset + size > se->buffer.size()) {
		real_size = se->buffer.size() - offset;
	}

	memcpy(buffer, se->buffer.data() + offset, real_size);
	offset += real_size;

	return real_size;
}

int AudioSeDecoder::GetPitch() const {
	return 100;
}
