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

#if defined(HAVE_TREMOR) || defined(HAVE_OGGVORBIS)

// Headers
#include "audio_decoder.h"
#include "decoder_oggvorbis.h"
#include "filesystem_stream.h"

static size_t vio_read_func(void *ptr, size_t size,size_t nmemb,void* userdata) {
	auto* f = reinterpret_cast<Filesystem_Stream::InputStream*>(userdata);
	if (size == 0) return 0;
	return f->read(reinterpret_cast<char*>(ptr), size*nmemb).gcount()/size;
}

static int vio_seek_func(void* userdata, ogg_int64_t offset, int seek_type) {
	auto* f = reinterpret_cast<Filesystem_Stream::InputStream*>(userdata);
	if (f->eof()) f->clear(); //emulate behaviour of fseek

	f->seekg(offset, Filesystem_Stream::CSeekdirToCppSeekdir(seek_type));

	return f->tellg();
}

static long vio_tell_func(void* userdata) {
	auto* f = reinterpret_cast<Filesystem_Stream::InputStream*>(userdata);
	return f->tellg();
}

static ov_callbacks vio = {
	vio_read_func,
	vio_seek_func,
	nullptr, // close not supported by istream interface
	vio_tell_func
};

OggVorbisDecoder::OggVorbisDecoder() {
	music_type = "ogg";
}

OggVorbisDecoder::~OggVorbisDecoder() {
	if (ovf) {
		ov_clear(ovf);
		delete ovf;
	}
}

bool OggVorbisDecoder::Open(Filesystem_Stream::InputStream stream) {
	finished = false;
	this->stream = std::move(stream);
	if (ovf) {
		ov_clear(ovf);
		delete ovf;
	}
	ovf = new OggVorbis_File;

	int res = ov_open_callbacks(&this->stream, ovf, nullptr, 0,vio);
	if (res < 0) {
		error_message = "OggVorbis: Error reading file";
		delete ovf;
		return false;
	}

	vorbis_info *vi = ov_info(ovf, -1);
	if (!vi) {
		error_message = "OggVorbis: Error getting file information";
		ov_clear(ovf);
		delete ovf;
		return false;
	}

	frequency = vi->rate;
	channels = vi->channels;

	vorbis_comment* vc = ov_comment(ovf, -1);
	if (vc) {
		// RPG VX loop support
		// Workaround conversion of string constant to char warning because
		// of tremor using a different signature.
#if defined(HAVE_TREMOR)
		using char_type = char*;
#else
		using char_type = const char*;
#endif

		const char* str = vorbis_comment_query(vc, (char_type)"LOOPSTART", 0);
		if (str) {
			auto total = ov_pcm_total(ovf, -1) ;
			loop.start = std::min<int64_t>(atoi(str), total);
			if (loop.start >= 0) {
				loop.looping = true;
				loop.end = total;
				str = vorbis_comment_query(vc, (char_type)"LOOPLENGTH", 0);
				if (str) {
					int len = atoi(str);
					if (len >= 0) {
						loop.end = std::min<int64_t>(loop.start + len, total);
					}
				} else {
					str = vorbis_comment_query(vc, (char_type)"LOOPEND", 0);
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

bool OggVorbisDecoder::Seek(std::streamoff offset, std::ios_base::seekdir origin) {
	if (offset == 0 && origin == std::ios_base::beg) {
		finished = false;

		if (ovf) {
			// Seeks to 0 when not looping
			ov_pcm_seek(ovf, loop.start);
		}

		if (loop.looping && loop.start == loop.end) {
			loop.to_end = true;
		}

		return true;
	}

	return false;
}

bool OggVorbisDecoder::IsFinished() const {
	if (!ovf) {
		return false;
	}

	if (loop.to_end) {
		return false;
	}

	return finished;
}

void OggVorbisDecoder::GetFormat(int& freq, AudioDecoder::Format& format, int& chans) const {
	freq = frequency;
	format = Format::S16;
	chans = channels;
}

bool OggVorbisDecoder::SetFormat(int freq, AudioDecoder::Format format, int chans) {
	if (freq != frequency || chans != channels || format != Format::S16)
		return false;

	return true;
}

int OggVorbisDecoder::GetTicks() const {
	if (!ovf) {
		return 0;
	}

	return (int)ov_time_tell(ovf);
}

int OggVorbisDecoder::FillBuffer(uint8_t* buffer, int length) {
	if (!ovf)
		return -1;

	if (loop.to_end) {
		memset(buffer, '\0', length);
		return length;
	}

	static int section;
	int read = 0;
	int to_read = length;

	do {
#ifdef HAVE_TREMOR
		read = ov_read(ovf, reinterpret_cast<char*>(buffer + length - to_read), to_read, &section);
#else
#  if defined(__WIIU__)
		// FIXME: This is the endianess of the audio and not of the host but the byteswapping in ov_read does
		// not sound like it works
		int byte_order = 1; // BE
#  else
		int byte_order = 0; // LE
#endif
		read = ov_read(ovf, reinterpret_cast<char*>(buffer + length - to_read), to_read, byte_order, 2/*16bit*/, 1/*signed*/, &section);
#endif
		// stop decoding when error or end of file
		if (read <= 0)
			break;

		// stop when loop end is reached
		if (loop.looping) {
			auto pos = ov_pcm_tell(ovf);
			if (pos >= loop.end) {
				finished = true;
				break;
			}
		}

		to_read -= read;
	} while(to_read > 0);

	// end of file
	if (read == 0)
		finished = true;

	// error
	if (read < 0)
		return -1;

	return length - to_read;
}

#endif
