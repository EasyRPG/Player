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

#ifndef _EASYRPG_AUDIO_DECODER_H_
#define _EASYRPG_AUDIO_DECODER_H_

// Headers
#include <string>
#include <vector>
#include <memory>

/**
 * Audio Decoder Namespace
 */
class AudioDecoder {
public:
	enum class Format {
		S8,
		U8,
		S16,
		U16
	};

	enum class Channel {
		Mono = 1,
		Stereo = 2
	};

	virtual ~AudioDecoder();

	virtual bool Open(FILE* file) = 0;

	virtual const std::vector<char>& Decode(int length) = 0;

	virtual bool IsFinished() const = 0;

	virtual std::string GetError() const = 0;

	virtual void GetFormat(int& frequency, Format& format, Channel& channels) const = 0;

	virtual bool SetFormat(int frequency, Format format, Channel channels) = 0;

	static std::unique_ptr<AudioDecoder> Create(FILE* file);
};

#endif
