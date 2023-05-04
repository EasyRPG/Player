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
#include "decoder_drwav.h"

#ifdef WANT_DRWAV

#define DR_WAV_IMPLEMENTATION
// Use system dr_wav header if available
#if __has_include(<dr_wav.h>)
#  include <dr_wav.h>
#else
#  include "external/dr_wav.h"
#endif


DrWavDecoder::DrWavDecoder() {
	music_type = "wav";
}

DrWavDecoder::~DrWavDecoder() {
	drwav_uninit(&handle);
}

size_t read_func(void* userdata, void* ptr, size_t count) {
	auto* f = reinterpret_cast<Filesystem_Stream::InputStream*>(userdata);

	return f->read(reinterpret_cast<char*>(ptr), count).gcount();
}

drwav_bool32 seek_func(void* userdata, int offset, drwav_seek_origin origin) {
	auto* f = reinterpret_cast<Filesystem_Stream::InputStream*>(userdata);
	if (f->eof()) f->clear(); // emulate behaviour of fseek

	f->seekg(offset, Filesystem_Stream::CSeekdirToCppSeekdir(origin));

	return DRWAV_TRUE;
}

bool DrWavDecoder::Open(Filesystem_Stream::InputStream stream_) {
	this->stream = std::move(stream_);
	init = drwav_init_ex(&handle, read_func, seek_func, nullptr, &this->stream, nullptr, DRWAV_SEQUENTIAL, nullptr) == DRWAV_TRUE;
	return init;
}

bool DrWavDecoder::Seek(std::streamoff offset, std::ios_base::seekdir origin) {
	if (origin == std::ios_base::beg) {
		finished = false;
		drwav_seek_to_pcm_frame(&handle, offset);
		return true;
	}
	return false;
}

bool DrWavDecoder::IsFinished() const {
	return finished;
}

void DrWavDecoder::GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const {
	if (!init) return;
	frequency = handle.sampleRate;
	channels = handle.channels;
	format = AudioDecoder::Format::S16;
}

bool DrWavDecoder::SetFormat(int, AudioDecoder::Format, int) {
	return false;
}

int DrWavDecoder::GetTicks() const {
	if (!init) {
		return 0;
	}

	return decoded_samples / handle.sampleRate;
}

int DrWavDecoder::FillBuffer(uint8_t* buffer, int length) {
	if (!init) {
		return -1;
	}

	if (finished) {
		return 0;
	}

	int decoded = static_cast<int>(drwav_read_pcm_frames_s16(&handle, length / (handle.channels * 2), reinterpret_cast<drwav_int16*>(buffer)));
	decoded_samples += decoded;
	decoded *= handle.channels * 2;

	if (decoded < length)
		finished = true;

	return decoded;
}

#endif
