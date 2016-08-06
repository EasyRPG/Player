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

static sf_count_t sf_vio_get_filelen_impl(void* userdata) {
	FILE* f = reinterpret_cast<FILE*>(userdata);
	int fd=fileno(f); //Posix complient - should work on windows as well
	struct stat stat_buf;
	int rc = fstat(fd, &stat_buf);
	return rc == 0 ? stat_buf.st_size : 0;
}

static sf_count_t sf_vio_read_impl(void *ptr, sf_count_t count, void* userdata){
	FILE* f = reinterpret_cast<FILE*>(userdata);
	return fread(ptr, 1, count, f);
}

static sf_count_t sf_vio_write_impl(const void* /* ptr */, sf_count_t count, void* /* user_data */){
	//Writing of wav files is not necessary
	return count;
}

static sf_count_t sf_vio_seek_impl(sf_count_t offset, int seek_type, void *userdata) {
	FILE* f = reinterpret_cast<FILE*>(userdata);
	fseek(f, offset, seek_type);
	return ftell(f);
}

static sf_count_t sf_vio_tell_impl(void* userdata){
	FILE* f = reinterpret_cast<FILE*>(userdata);
	return ftell(f);
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
		fclose(file_);
	}
}

bool LibsndfileDecoder::Open(FILE* file) {
	file_=file;
	soundfile=sf_open_virtual(&vio,SFM_READ,&soundinfo,file);
	sf_command(soundfile, SFC_SET_SCALE_FLOAT_INT_READ, NULL, SF_TRUE);
	output_format=Format::F32;
	finished=false;
	return soundfile!=0;
}

bool LibsndfileDecoder::Seek(size_t offset, Origin /* origin */) {
	finished = false;
	if(soundfile == 0)
		return false;
	return sf_seek(soundfile,offset,SEEK_SET)!=-1;
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
				decoded*=sizeof(float);
			}
			break;
		case Format::S16:
			{
				decoded=sf_read_short(soundfile,(int16_t*)buffer,length/sizeof(int16_t));
				if(!decoded)
					finished=true;
				decoded*=sizeof(int16_t);
			}
			break;
		case Format::S32:
			{
				// Uses int instead of int32_t because the 3ds toolchain typedefs
				// to long int which is an incompatible pointer type
				decoded=sf_read_int(soundfile,(int*)buffer,length/sizeof(int));

				if(!decoded)
					finished=true;

				decoded *= sizeof(int);
			}
			break;
		default:
			decoded=-1;
			break;
	}
	return decoded;
}

#endif
