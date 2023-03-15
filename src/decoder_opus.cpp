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

static int vio_read_func(void* stream, unsigned char* ptr, int nbytes) {
	auto* f = reinterpret_cast<Filesystem_Stream::InputStream*>(stream);
	if (nbytes == 0) return 0;
	return (int)(f->read(reinterpret_cast<char*>(ptr), nbytes).gcount());
}

static int vio_seek_func(void* stream, opus_int64 offset, int whence) {
	auto* f = reinterpret_cast<Filesystem_Stream::InputStream*>(stream);
	if (f->eof()) f->clear(); // emulate behaviour of fseek

	f->seekg(offset, Filesystem_Stream::CSeekdirToCppSeekdir(whence));

	return 0;
}

static opus_int64 vio_tell_func(void* stream) {
	auto* f = reinterpret_cast<Filesystem_Stream::InputStream*>(stream);
	return static_cast<opus_int64>(f->tellg());
}

static OpusFileCallbacks vio = {
		vio_read_func,
		vio_seek_func,
		vio_tell_func,
		nullptr // close not supported by istream interface
};

OpusAudioDecoder::OpusAudioDecoder() {
	music_type = "opus";
}

OpusAudioDecoder::~OpusAudioDecoder() {
	if (oof) {
		op_free(oof);
	}
}

bool OpusAudioDecoder::Open(Filesystem_Stream::InputStream stream) {
	this->stream = std::move(stream);
	finished = false;

	int res;

	oof = op_open_callbacks(&this->stream, &vio, nullptr, 0, &res);
	if (res != 0) {
		error_message = "Opus: Error reading file";
		op_free(oof);
		return false;
	}

	const OpusTags* ot = op_tags(oof, -1);
	if (ot) {
		// RPG VX loop support
		const char* str = opus_tags_query(ot, "LOOPSTART", 0);
		if (str) {
			auto total = op_pcm_total(oof, -1) ;
			loop.start = std::min<int64_t>(atoi(str), total);
			if (loop.start >= 0) {
				loop.looping = true;
				loop.end = total;
				str = opus_tags_query(ot, "LOOPLENGTH", 0);
				if (str) {
					int len = atoi(str);
					if (len >= 0) {
						loop.end = std::min<int64_t>(loop.start + len, total);
					}
				} else {
					str = opus_tags_query(ot, "LOOPEND", 0);
					if (str) {
						int end = atoi(str);
						if (end >= 0) {
							loop.end = Utils::Clamp<int64_t>(end, loop.start, total);
						}
					}
				}

				if (loop.start == total) {
					loop.end = total;
				}
			}
		}
	}

	if (!loop.looping) {
		loop.start = 0;
		loop.end = -1;
	}

	return true;
}

bool OpusAudioDecoder::Seek(std::streamoff offset, std::ios_base::seekdir origin) {
	if (offset == 0 && origin == std::ios::beg) {
		finished = false;

		if (oof) {
			// Seeks to 0 when not looping
			op_pcm_seek(oof, loop.start);
		}

		if (loop.looping && loop.start == loop.end) {
			loop.to_end = true;
		}

		return true;
	}

	return false;
}

bool OpusAudioDecoder::IsFinished() const {
	if (!oof) {
		return false;
	}

	if (loop.to_end) {
		return false;
	}

	return finished;
}

void OpusAudioDecoder::GetFormat(int& freq, AudioDecoder::Format& format, int& chans) const {
	freq = frequency;
	format = Format::S16;
	chans = channels;
}

bool OpusAudioDecoder::SetFormat(int freq, AudioDecoder::Format format, int chans) {
	if (freq != frequency || chans != channels || format != Format::S16)
		return false;

	return true;
}

int OpusAudioDecoder::GetTicks() const {
	if (!oof) {
		return 0;
	}

	// According to the docs it is number of samples at 48 kHz
	return op_pcm_tell(oof) / 48000;
}

int OpusAudioDecoder::FillBuffer(uint8_t* buffer, int length) {
	if (!oof)
		return -1;

	if (loop.to_end) {
		memset(buffer, '\0', length);
		return length;
	}

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

		// stop when loop end is reached
		if (loop.looping) {
			auto pos = op_pcm_tell(oof);
			if (pos >= loop.end) {
				finished = true;
				break;
			}
		}

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
