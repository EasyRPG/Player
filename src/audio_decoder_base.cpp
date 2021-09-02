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
#include "audio_decoder_base.h"
#include "output.h"
#include "system.h"
#include "utils.h"

float AudioDecoderBase::AdjustVolume(float volume) {
	// Adjust to RPG_RT (Direct Sound) volume scale
	if (volume > 0) {
		return 100.0f * std::pow(10.0f, (-100 + volume) / 60.0f);
	}
	return 0.0f;
}

int AudioDecoderBase::Decode(uint8_t* buffer, int length) {
	return Decode(buffer, length, 0);
}

std::vector<uint8_t> AudioDecoderBase::DecodeAll() {
	const int buffer_size = 8192;

	std::vector<uint8_t> buffer;
	buffer.resize(buffer_size);

	while (!IsFinished()) {
		int read = Decode(buffer.data() + buffer.size() - buffer_size, buffer_size);
		if (read < buffer_size) {
			buffer.resize(buffer.size() - (buffer_size - read));
			break;
		}

		buffer.resize(buffer.size() + buffer_size);
	}

	return buffer;
}

void AudioDecoderBase::Rewind() {
	if (!Seek(0, std::ios_base::beg)) {
		// The libs guarantee that Rewind works
		assert(false && "Rewind");
	}
}

bool AudioDecoderBase::GetLooping() const {
	return looping;
}

void AudioDecoderBase::SetLooping(bool enable) {
	looping = enable;
}

int AudioDecoderBase::GetLoopCount() const {
	return loop_count;
}

bool AudioDecoderBase::WasInited() const {
	return true;
}

std::string AudioDecoderBase::GetError() const {
	return error_message;
}

std::string AudioDecoderBase::GetType() const {
	return music_type;
}

void AudioDecoderBase::UpdateMidi(std::chrono::microseconds delta) {
	(void)delta;
}

bool AudioDecoderBase::SetFormat(int, AudioDecoderBase::Format, int) {
	return false;
}

int AudioDecoderBase::GetPitch() const {
	return 0;
}

bool AudioDecoderBase::SetPitch(int) {
	return false;
}

std::streampos AudioDecoderBase::Tell() const {
	return -1;
}

int AudioDecoderBase::Decode(uint8_t* buffer, int length, int recursion_depth) {
	int res = FillBuffer(buffer, length);

	if (res < 0) {
		memset(buffer, '\0', length);
	}
	else if (res < length) {
		memset(&buffer[res], '\0', length - res);
	}

	if (IsFinished() && looping && recursion_depth < 10) {
		++loop_count;
		Rewind();
		if (length - res > 0) {
			int res2 = Decode(&buffer[res], length - res, ++recursion_depth);
			if (res2 <= 0) {
				return res;
			}
			return res + res2;
		}
	}

	if (recursion_depth == 10 && loop_count < 50) {
		// Only report this a few times in the hope that this is only a temporary problem and to prevent log spamming
		Output::Debug("Audio Decoder: Recursion depth exceeded. Probably stream error.");
	}

	return res;
}
