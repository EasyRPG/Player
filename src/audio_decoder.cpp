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
#include "audio_decoder.h"
#include "filefinder.h"
#include "output.h"

#include "system.h"

#ifdef HAVE_FMMIDI
#include "decoder_fmmidi.h"
#endif

#ifdef HAVE_MPG123
#include "decoder_mpg123.h"
#endif

void AudioDecoder::Pause() {
	paused = true;
 }

void AudioDecoder::Resume() {
	paused = false;
}

int AudioDecoder::Decode(uint8_t* buffer, int length) {
	if (paused) {
		memset(buffer, '\0', length);
		return length;
	}

	int res = FillBuffer(buffer, length);

	if (IsFinished() && looping) {
		++loops;
		Rewind();
	}

	return res;
}

int AudioDecoder::DecodeAsMono(uint8_t* left, uint8_t* right, int size) {
	int freq; Format format; int channels;
	GetFormat(freq, format, channels);

	if (channels == 1) {
		return Decode(left, size);
	}

	if (mono_buffer.size() < size * 2) {
		mono_buffer.resize(size * 2);
	}
	
	int read = Decode(mono_buffer.data(), size * 2);
	if (read < 0) {
		return -1;
	}

	int sample_size = GetSamplesizeForFormat(format);

	for (int i = 0; i < read / 2; i += sample_size) {
		memcpy(&left[i], &mono_buffer.data()[i * 2], sample_size);
		memcpy(&right[i], &mono_buffer.data()[i * 2 + sample_size], sample_size);
	}

	return read / 2;
}

static bool ends_with(std::string const& value, std::string const& ending) {
	 if (ending.size() > value.size()) return false;
	 return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::unique_ptr<AudioDecoder> AudioDecoder::Create(FILE** file, const std::string& filename) {
	char magic[4] = { 0 };
	fread(magic, 4, 1, *file);
	fseek(*file, 0, SEEK_SET);

#ifdef HAVE_FMMIDI
	if (!strncmp(magic, "MThd", 4)) {
		return std::unique_ptr<AudioDecoder>(new FmMidiDecoder());
	}
#endif

	// Prevent false positives by checking for common headers
	if (!strncmp(magic, "RIFF", 4) || // WAV
		!strncmp(magic, "FORM", 4) || // WAV AIFF
		!strncmp(magic, "OggS", 4) || // OGG
		!strncmp(magic, "fLaC", 4) // FLAC
		) {
		return std::unique_ptr<AudioDecoder>();
	}

#ifdef HAVE_MPG123
	if (strncmp(magic, "ID3", 3) == 0) {
		return std::unique_ptr<AudioDecoder>(new Mpg123Decoder());
	}

	// Parsing MP3s seems to be the only reliable way to detect them
	if (Mpg123Decoder::IsMp3(*file)) {
		// File must be reopened because mpg123 closes it when being destructed
		*file = FileFinder::fopenUTF8(filename.c_str(), "rb");
		return std::unique_ptr<AudioDecoder>(new Mpg123Decoder());
	}
#endif

	return std::unique_ptr<AudioDecoder>();
}

void AudioDecoder::SetFade(int begin, int end, int duration) {
	fade_time = 0.0;

	if (duration <= 0.0) {
		volume = end;
		return;
	}

	if (begin == end) {
		volume = end;
		return;
	}

	volume = (double)begin;
	fade_end = (double)end;
	fade_time = (double)duration;
	delta_step = (fade_end - volume) / fade_time;
}

void AudioDecoder::Update(int delta) {
	if (fade_time <= 0.0) {
		return;
	}
	
	fade_time -= delta;
	volume += delta * delta_step;

	volume = volume > 100.0 ? 100.0 :
		volume < 0.0 ? 0.0 :
		volume;
}

void AudioDecoder::SetVolume(int volume) {
	this->volume = (double)volume;
}

int AudioDecoder::GetVolume() const {
	return (int)volume;
}

bool AudioDecoder::Rewind() {
	return Seek(0, Origin::Begin);
}

bool AudioDecoder::GetLooping() const {
	return looping;
}

void AudioDecoder::SetLooping(bool enable) {
	looping = enable;
}

int AudioDecoder::GetLoopCount() const {
	return loop_count;
}

bool AudioDecoder::SetFormat(int, Format, int) {
	return false;
}

int AudioDecoder::GetPitch() const {
	return 0;
}

bool AudioDecoder::SetPitch(int) {
	return false;
}

bool AudioDecoder::Seek(size_t, Origin) {
	return false;
}

size_t AudioDecoder::Tell() {
	return -1;
}

int AudioDecoder::GetTicks() {
	return 0;
}

int AudioDecoder::GetSamplesizeForFormat(AudioDecoder::Format format) {
	switch (format) {
		case Format::S8:
		case Format::U8:
			return 1;
		case Format::S16:
		case Format::U16:
			return 2;
		case Format::S32:
		case Format::U32:
		case Format::F32:
			return 4;
	}

	assert(false && "Bad format");
	return -1;
}
