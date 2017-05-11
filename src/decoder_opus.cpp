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

#include <opus/opusfile.h>
#include "audio_decoder.h"
#include "decoder_opus.h"
#include "output.h"

static int custom_read(void *stream, unsigned char *ptr, int nbytes) {
	FILE* f = reinterpret_cast<FILE*>(stream);
	return fread(ptr, 1, nbytes, f);
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
	OpusFileCallbacks callbacks = {custom_read, nullptr, nullptr, nullptr};

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

	int ret = op_read_stereo(oof, reinterpret_cast<opus_int16*>(buffer), length / 2);

	if (ret < 0)
		return ret;

	return ret * 4;
}

#endif
