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

#ifdef HAVE_LIBSNDFILE

// Headers
#include <cassert>
#include <sys/stat.h>
#include "decoder_libsndfile.h"
#include "output.h"

static sf_count_t sf_vio_get_filelen_impl(void*) {
	// Unknown. SF_COUNT_MAX is the size used by libsndfile for pipes.
	return SF_COUNT_MAX;
}

static sf_count_t sf_vio_read_impl(void *ptr, sf_count_t count, void* userdata){
	auto* f = reinterpret_cast<Filesystem_Stream::InputStream*>(userdata);
	return f->read(reinterpret_cast<char*>(ptr), count).gcount();
}

static sf_count_t sf_vio_write_impl(const void* /* ptr */, sf_count_t count, void* /* user_data */){
	//Writing of wav files is not necessary
	return count;
}

static sf_count_t sf_vio_seek_impl(sf_count_t offset, int seek_type, void *userdata) {
	auto* f = reinterpret_cast<Filesystem_Stream::InputStream*>(userdata);
	if (f->eof()) f->clear(); //emulate behaviour of fseek

	f->seekg(offset, Filesystem_Stream::CSeekdirToCppSeekdir(seek_type));

	return f->tellg();
}

static sf_count_t sf_vio_tell_impl(void* userdata){
	auto* f = reinterpret_cast<Filesystem_Stream::InputStream*>(userdata);
	return f->tellg();
}

static SF_VIRTUAL_IO vio = {
	sf_vio_get_filelen_impl,
	sf_vio_seek_impl,
	sf_vio_read_impl,
	sf_vio_write_impl,
	sf_vio_tell_impl
}; 

LibsndfileDecoder::LibsndfileDecoder() 
{
	music_type = "wav";
	soundfile=0;
}

LibsndfileDecoder::~LibsndfileDecoder() {
	if(soundfile != 0){
		sf_close(soundfile);
	}
}

bool LibsndfileDecoder::Open(Filesystem_Stream::InputStream stream) {
	this->stream = std::move(stream);
	soundfile=sf_open_virtual(&vio,SFM_READ,&soundinfo,&this->stream);
	sf_command(soundfile, SFC_SET_SCALE_FLOAT_INT_READ, NULL, SF_TRUE);
	output_format=Format::S16;
	finished=false;
	decoded_samples = 0;
	return soundfile!=0;
}

bool LibsndfileDecoder::Seek(std::streamoff offset, std::ios_base::seekdir origin) {
	finished = false;
	if(soundfile == 0)
		return false;

	// FIXME: Proper sample count for seek
	decoded_samples = 0;
	return sf_seek(soundfile, offset, Filesystem_Stream::CppSeekdirToCSeekdir(origin))!=-1;
}

bool LibsndfileDecoder::IsFinished() const {
	return finished;
}

void LibsndfileDecoder::GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const {
	if(soundfile==0) return;
	frequency = soundinfo.samplerate;
	channels = soundinfo.channels;
	format = output_format;
}

bool LibsndfileDecoder::SetFormat(int freq, AudioDecoder::Format fmt, int channels) {
	if(soundfile == 0)
		return false;

	switch(fmt){
		case Format::F32:
		case Format::S16:
		case Format::S32:
			output_format=fmt;
			break;
		default:
			return false;
	}
	return soundinfo.samplerate==freq && soundinfo.channels==channels && output_format==fmt;
}

int LibsndfileDecoder::FillBuffer(uint8_t* buffer, int length) {
	if(soundfile == 0)
		return -1;

	int decoded;
	switch(output_format){
		case Format::F32:
			{
				decoded=sf_read_float(soundfile,(float*)buffer,length/sizeof(float));
				if(!decoded)
					finished=true;
				decoded_samples += decoded;
				decoded *= sizeof(float);
			}
			break;
		case Format::S16:
			{
				decoded=sf_read_short(soundfile,(int16_t*)buffer,length/sizeof(int16_t));
				if(!decoded)
					finished=true;
				decoded_samples += decoded;
				decoded *= sizeof(int16_t);
			}
			break;
		case Format::S32:
			{
				// Uses int instead of int32_t because the 3ds toolchain typedefs
				// to long int which is an incompatible pointer type
				decoded=sf_read_int(soundfile,(int*)buffer,length/sizeof(int));

				if(!decoded)
					finished=true;

				decoded_samples += decoded;
				decoded *= sizeof(int);
			}
			break;
		default:
			decoded=-1;
			break;
	}

	return decoded;
}

int LibsndfileDecoder::GetTicks() const {
	if (soundfile == 0) {
		return 0;
	}

	return decoded_samples / (soundinfo.samplerate * soundinfo.channels);
}

#endif
