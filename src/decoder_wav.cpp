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

#include "system.h"

#ifdef WANT_FASTWAV

// Headers
#include <cstring>
#include "decoder_wav.h"
#include "utils.h"

WavDecoder::WavDecoder() 
{
	music_type = "wav";
}

WavDecoder::~WavDecoder() {
}

bool WavDecoder::Open(Filesystem::InputStream stream) {
	decoded_samples = 0;
	this->stream = stream;
	stream->seekg(16, std::ios::ios_base::beg);
	stream->read(reinterpret_cast<char*>(&chunk_size), sizeof(chunk_size));
	Utils::SwapByteOrder(chunk_size);
	stream->seekg(2, std::ios::ios_base::cur);
	stream->read(reinterpret_cast<char*>(&nchannels), sizeof(nchannels));
	Utils::SwapByteOrder(nchannels);
	stream->read(reinterpret_cast<char*>(&samplerate), sizeof(samplerate));
	Utils::SwapByteOrder(samplerate);
	stream->seekg(6, std::ios::ios_base::cur);
	uint16_t bitspersample;
	stream->read(reinterpret_cast<char*>(&bitspersample), sizeof(bitspersample));
	Utils::SwapByteOrder(bitspersample);
	switch (bitspersample) {
		case 8:
			output_format=Format::U8;
			break;
		case 16:
			output_format=Format::S16;
			break;
		case 32:
			output_format=Format::S32;
			break;
		default:
			return false;
	}

	// Skip to next chunk using "fmt" chunk as offset
	stream->seekg(12 + 8 + chunk_size, std::ios::ios_base::beg);

	char chunk_name[4] = {0};
	stream->read(reinterpret_cast<char*>(chunk_name), sizeof(chunk_name));

	// Skipping to audiobuffer start
	while (strncmp(chunk_name, "data", 4)) {
		stream->read(reinterpret_cast<char*>(&chunk_size), sizeof(chunk_size));
		Utils::SwapByteOrder(chunk_size);
		stream->seekg(chunk_size, std::ios::ios_base::cur);
		stream->read(reinterpret_cast<char*>(chunk_name), sizeof(chunk_name));

		if (!stream->good()) {
			return false;
		}
	}

	// Get data chunk size
	stream->read(reinterpret_cast<char*>(&chunk_size), sizeof(chunk_size));
	Utils::SwapByteOrder(chunk_size);

	if (!stream->good()) {
		return false;
	}

	// Start of data chunk
	audiobuf_offset = stream->tellg();
	cur_pos = audiobuf_offset;
	finished = false;
	return stream->good();
}

bool WavDecoder::Seek(size_t offset, Origin origin) {
	finished = false;
	if (!stream)
		return false;
	if (origin == Origin::Begin) {
		offset += audiobuf_offset;
	}
	// FIXME: Proper sample count for seek
	decoded_samples = 0;

	bool success = false;
	switch (origin) {
	case Origin::Begin:
		success = stream->seekg(offset, std::ios::ios_base::beg).good();
		break;
	case Origin::Current:
		success = stream->seekg(offset, std::ios::ios_base::cur).good();
		break;
	case Origin::End:
		success = stream->seekg(offset, std::ios::ios_base::end).good();
		break;
	}
	if (!success) { stream->clear(); }
	cur_pos = stream->tellg();
	return success;
}

bool WavDecoder::IsFinished() const {
	return finished;
}

void WavDecoder::GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const {
	if (!stream) return;
	frequency = samplerate;
	channels = nchannels;
	format = output_format;
}

bool WavDecoder::SetFormat(int, AudioDecoder::Format, int) {
	return false;
}

int WavDecoder::FillBuffer(uint8_t* buffer, int length) {
	if (!stream)
		return -1;

	int real_length;

	// Handle case that another chunk is behind "data" or file ended
	if (cur_pos + length >= audiobuf_offset + chunk_size) {
		real_length = audiobuf_offset + chunk_size - cur_pos;
		cur_pos = audiobuf_offset + chunk_size;
	} else {
		real_length = length;
		cur_pos += length;
	}

	if (real_length == 0) {
		finished = true;
		return 0;
	}

	int decoded = stream->read(reinterpret_cast<char*>(buffer), real_length).gcount();

	if (output_format == AudioDecoder::Format::S16) {
		if (Utils::IsBigEndian()) {
			uint16_t *buffer_16 = reinterpret_cast<uint16_t *>(buffer);
			for (int i = 0; i < decoded / 2; ++i) {
				Utils::SwapByteOrder(buffer_16[i]);
			}
		}
		decoded_samples += (decoded / 2);
	} else if (output_format == AudioDecoder::Format::S32) {
		if (Utils::IsBigEndian()) {
			uint32_t *buffer_32 = reinterpret_cast<uint32_t *>(buffer);
			for (int i = 0; i < decoded / 4; ++i) {
				Utils::SwapByteOrder(buffer_32[i]);
			}
		}
		decoded_samples += (decoded / 4);
	}

	if (decoded < length)
		finished = true;

	return decoded;
}

int WavDecoder::GetTicks() const {
	return decoded_samples / (samplerate * nchannels);
}

#endif
