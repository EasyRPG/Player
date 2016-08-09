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

#ifdef HAVE_SLOW_CPU

// Headers
#include <cassert>
#include <sys/stat.h>
#include "decoder_wav.h"
#include "output.h"

WavDecoder::WavDecoder() 
{
	music_type = "wav";
}

WavDecoder::~WavDecoder() {
	if(file_ != NULL){
		fclose(file_);
	}
}

bool WavDecoder::Open(FILE* file) {
	file_=file;
	uint32_t chunk = 0xDEADBEEF;
	uint32_t jump;
	fseek(file_, 22, SEEK_SET);
	fread(&nchannels, 1, 2, file_);
	fread(&samplerate, 1, 4, file_);
	uint16_t bitspersample;
	fread(&bitspersample, 1, 2, file_);
	if (bitspersample == 8) output_format=Format::S8;
	else output_format=Format::S16;
	fseek(file_, 16, SEEK_SET);
	fread(&jump, 4, 1, file_);
	
	// Skipping to audiobuffer start
	while (chunk != 0x61746164){
		fseek(file_, jump, SEEK_CUR);
		fread(&chunk, 4, 1, file_);
		fread(&jump, 4, 1, file_);
	}
	
	audiobuf_offset = ftell(file_);
	finished=false;
	return file_!=NULL;
}

bool WavDecoder::Seek(size_t offset, Origin origin) {
	finished = false;
	if(file_ != NULL)
		return false;
	if (offset == 0) offset = audiobuf_offset;
	return fseek(file_,offset,SEEK_SET)==0;
}

bool WavDecoder::IsFinished() const {
	return finished;
}


void WavDecoder::GetFormat(int& frequency, AudioDecoder::Format& format, int& channels) const {
	if(file_ == NULL) return;
	frequency = samplerate;
	channels = nchannels;
	format = output_format;
}

bool WavDecoder::SetFormat(int freq, AudioDecoder::Format fmt, int channels) {
	return false;
}

int WavDecoder::FillBuffer(uint8_t* buffer, int length) {
	if(file_ == NULL)
		return -1;

	int decoded=fread(buffer,1,length,file_);
	if(decoded < length)
		finished=true;
	
	return decoded;
}

#endif
