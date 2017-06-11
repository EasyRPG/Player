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

#ifdef HAVE_OPUS

#include <cstring>
#include <opus/opusfile.h>
#include "audio_decoder.h"
#include "decoder_opus.h"

static int custom_read(void* stream, unsigned char* ptr, int nbytes) {
	FILE* f = reinterpret_cast<FILE*>(stream);
	return fread(ptr, 1, nbytes, f);
}

static int custom_seek(void* stream, opus_int64 offset, int whence) {
	FILE* f = reinterpret_cast<FILE*>(stream);
	return fseek(f, offset, whence);
}

static opus_int64 custom_tell(void* stream) {
	FILE* f = reinterpret_cast<FILE*>(stream);
	return ftell(f);
}

static int custom_close(void* stream) {
	FILE* f = reinterpret_cast<FILE*>(stream);
	return fclose(f);
}

OpusDecoder::OpusDecoder() {
	music_type = "opus";
}

OpusDecoder::~OpusDecoder() {
	if (oof) {
		op_free(oof);
	}
}

bool OpusDecoder::Open(FILE* file) {
	finished = false;

	int res;
	OpusFileCallbacks callbacks = {custom_read, custom_seek, custom_tell, custom_close};

	oof = op_open_callbacks(file, &callbacks, nullptr, 0, &res);
	if (res != 0) {
		error_message = "Opus: Error reading file";
		op_free(oof);
		fclose(file);
		return false;
	}

	return true;
}

bool OpusDecoder::Seek(size_t offset, Origin origin) {
	if (offset == 0 && origin == Origin::Begin) {
		if (oof) {
			op_raw_seek(oof, 0);
		}
		finished = false;
		return true;
	}

	return false;
}

bool OpusDecoder::IsFinished() const {
	if (!oof)
		return false;

	return finished;
}

void OpusDecoder::GetFormat(int& freq, AudioDecoder::Format& format, int& chans) const {
	freq = frequency;
	format = Format::S16;
	chans = channels;
}

bool OpusDecoder::SetFormat(int freq, AudioDecoder::Format format, int chans) {
	if (freq != frequency || chans != channels || format != Format::S16)
		return false;

	return true;
}

int OpusDecoder::FillBuffer(uint8_t* buffer, int length) {
	if (!oof)
		return -1;

	// op_read_stereo doesn't overwrite the buffer completely, must be cleared to prevent noise
	memset(buffer, '\0', length);

	// Use a 16bit buffer because op_read_stereo works on one
	int length_16 = length / 2;
	opus_int16* buffer_16 = reinterpret_cast<opus_int16*>(buffer);

	int read = 0;
	int to_read = length_16;

	do {
		read = op_read_stereo(oof, buffer_16 + (length_16 - to_read), to_read);

		// stop decoding when error or end of file
		if (read <= 0)
			break;

		// "read" contains number of samples per channel and the function filled 2 channels
		to_read -= read * 2;
	} while (to_read > 0);

	if (read == 0)
		finished = true;

	if (read < 0) {
		return -1;
	}

	// Return amount of read bytes in the 8 bit what the audio decoder expects
	return (length_16 - to_read) * 2;
}

#endif
