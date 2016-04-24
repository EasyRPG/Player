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

static bool ends_with(std::string const& value, std::string const& ending) {
	 if (ending.size() > value.size()) return false;
	 return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::unique_ptr<AudioDecoder> AudioDecoder::Create(FILE* file, const std::string& filename) {
	char magic[4] = { 0 };
	fread(magic, 4, 1, file);

#ifdef HAVE_FMMIDI
	if (!strncmp(magic, "MThd", 4)) {
		fseek(file, 0, SEEK_SET);
		return std::unique_ptr<AudioDecoder>(new FmMidiDecoder());
	}
#endif

#ifdef HAVE_MPG123
	// Copied from SDL with some additions (.mp3 check)
	if (ends_with(filename, ".mp3") ||
		strncmp(magic, "ID3", 3) == 0 ||
		((magic[0] & 0xff) != 0xff) || // No sync bits
		((magic[1] & 0xf0) != 0xf0) || //
		((magic[2] & 0xf0) == 0x00) || // Bitrate is 0
		((magic[2] & 0xf0) == 0xf0) || // Bitrate is 15
		((magic[2] & 0x0c) == 0x0c) || // Frequency is 3
		((magic[1] & 0x06) == 0x00)) { // Layer is 4
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

bool AudioDecoder::SetFormat(int, Format, Channel) {
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
