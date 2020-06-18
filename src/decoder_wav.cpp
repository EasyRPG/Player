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
	if (file_ != NULL) {
		fclose(file_);
	}
}

bool WavDecoder::Open(FILE* file) {
	decoded_samples = 0;
	file_=file;
	fseek(file_, 16, SEEK_SET);
	fread(&chunk_size, 1, 4, file_);
	Utils::SwapByteOrder(chunk_size);
	fseek(file_, 2, SEEK_CUR);
	fread(&nchannels, 1, 2, file_);
	Utils::SwapByteOrder(nchannels);
	fread(&samplerate, 1, 4, file_);
	Utils::SwapByteOrder(samplerate);
	fseek(file_, 6, SEEK_CUR);
	uint16_t bitspersample;
	fread(&bitspersample, 1, 2, file_);
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
	fseek(file_, 12 + 8 + chunk_size, SEEK_SET);

	char chunk_name[4] = {0};
	fread(chunk_name, 4, 1, file_);

	// Skipping to audiobuffer start
	while (strncmp(chunk_name, "data", 4)) {
		fread(&chunk_size, 1, 4, file_);
		Utils::SwapByteOrder(chunk_size);
		fseek(file_, chunk_size, SEEK_CUR);
		fread(chunk_name, 4, 1, file_);

		if (feof(file_) || ferror(file_)) {
			fclose(file_);
			return false;
		}
	}

	// Get data chunk size
	fread(&chunk_size, 4, 1, file_);
	Utils::SwapByteOrder(chunk_size);

	if (feof(file_) || ferror(file_)) {
		fclose(file_);
		return false;
	}

	// Start of data chunk
	audiobuf_offset = ftell(file_);
	cur_pos = audiobuf_offset;
	finished = false;
	return file_!=NULL;
}

bool WavDecoder::Seek(size_t offset, Origin origin) {
	finished = false;
	if (file_ == NULL)
		return false;
	if (origin != Origin::End) {
		offset += audiobuf_offset;
	}
	// FIXME: Proper sample count for seek
	decoded_samples = 0;

	bool success = fseek(file_,offset,(int)origin) == 0;
	cur_pos = ftell(file_);
	return success;
}

bool WavDecoder::IsFinished() const {
	return finished;
}

void WavDecoder::GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const {
	if (file_ == NULL) return;
	frequency = samplerate;
	channels = nchannels;
	format = output_format;
}

bool WavDecoder::SetFormat(int, AudioDecoder::Format, int) {
	return false;
}

int WavDecoder::FillBuffer(uint8_t* buffer, int length) {
	if (file_ == NULL)
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

	int decoded = fread(buffer, 1, real_length, file_);

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
