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
#include "output.h"
#include "filefinder.h"
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#ifdef _3DS
#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#ifdef USE_CACHE
#include "3ds_cache.h"
#else
#include "3ds_decoder.h"
#endif
extern DecodedMusic* BGM;

/*	
	+-----------------------------------------------------+
	|                                                     |
	|                      SOUNDS                         |
	|                                                     |
	+-----------------------------------------------------+
*/

int DecodeOgg(FILE* stream, DecodedSound* Sound){
	
	// Passing filestream to libogg
	int eof=0;
	OggVorbis_File* vf = (OggVorbis_File*)malloc(sizeof(OggVorbis_File));
	static int current_section;
	fseek(stream, 0, SEEK_SET);
	if(ov_open(stream, vf, NULL, 0) != 0)
	{
		fclose(stream);
		Output::Warning("Corrupt ogg file");
		return -1;
	}
	
	// Grabbing info from the header
	vorbis_info* my_info = ov_info(vf,-1);
	Sound->samplerate = my_info->rate;
	Sound->format = CSND_ENCODING_PCM16;
	u16 audiotype = my_info->channels;
	Sound->audiobuf_size = ov_pcm_total(vf,-1)<<audiotype;
	if (audiotype == 2) Sound->isStereo = true;
	else Sound->isStereo = false;
	Sound->bytepersample = audiotype<<1;
	
	// Preparing PCM16 audiobuffer
	#ifdef USE_CACHE
	allocCache(Sound);
	#else
	Sound->audiobuf = (u8*)linearAlloc(Sound->audiobuf_size);
	#endif
	
	if (isDSP) audiotype = 1; // We trick the decoder since DSP supports native stereo playback
	
	// Decoding Vorbis buffer
	int i = 0;
	if (audiotype == 1){ // Mono file
		while(!eof){
			long ret=ov_read(vf,(char*)&Sound->audiobuf[i],OGG_BUFSIZE,0,2,1,&current_section);
			if (ret == 0) eof=1;
			else i = i + ret;
		}
	}else{ // Stereo file
		char pcmout[OGG_BUFSIZE];
		int z = 0;
		u32 chn_size = Sound->audiobuf_size>>1;
		u8* left_channel = Sound->audiobuf;
		u8* right_channel = &Sound->audiobuf[chn_size];
		while(!eof){
			long ret=ov_read(vf,pcmout,OGG_BUFSIZE,0,2,1,&current_section);
			if (ret == 0) eof=1;
			else{
				for (u32 i=0;i<ret;i=i+4){
					memcpy(&left_channel[z],&pcmout[i],2);
					memcpy(&right_channel[z],&pcmout[i+2],2);
					z = z + 2;
				}
			}
		}
	}
	
	ov_clear(vf);
	#ifdef USE_CACHE
	return LAST_ENTRY;
	#else
	return 0;
	#endif
	
}

int DecodeWav(FILE* stream, DecodedSound* Sound){
	
	// Grabbing info from the header
	u16 audiotype;
	u32 chunk;
	u32 jump;
	u16 bytepersample;
	fseek(stream, 16, SEEK_SET);
	fread(&jump, 4, 1, stream);
	fread(&Sound->format, 2, 1, stream);
	fread(&audiotype, 2, 1, stream);
	fread(&Sound->samplerate, 4, 1, stream);	
	if (audiotype == 2) Sound->isStereo = true;
	else Sound->isStereo = false;
	fseek(stream, 32, SEEK_SET);
	fread(&bytepersample, 2, 1, stream);
	Sound->bytepersample = bytepersample;
	fseek(stream, 20, SEEK_SET);
	
	// Check for file audiocodec
	if (Sound->format == 0x11) Sound->format = CSND_ENCODING_ADPCM;
	else if (bytepersample == 4 || (bytepersample == 2 && audiotype == 1)) Sound->format = CSND_ENCODING_PCM16;
	else Sound->format = CSND_ENCODING_PCM8;
	
	// Skipping to audiobuffer start
	while (chunk != 0x61746164){
		fseek(stream, jump, SEEK_CUR);
		fread(&chunk, 4, 1, stream);
		fread(&jump, 4, 1, stream);
	}
	
	// Getting audiobuffer size
	int start = ftell(stream);
	fseek(stream, 0, SEEK_END);
	int end = ftell(stream);
	Sound->audiobuf_size = end - start;
	fseek(stream, start, SEEK_SET);
	#ifdef USE_CACHE
	allocCache(Sound);
	#else
	Sound->audiobuf = (u8*)linearAlloc(Sound->audiobuf_size);
	#endif
	
	if (isDSP) audiotype = 1; // We trick the decoder since DSP supports native stereo playback
	
	// Mono file
	if (audiotype == 1) fread(Sound->audiobuf, Sound->audiobuf_size, 1, stream);	
	
	// Stereo file
	else{
		u32 chn_size = Sound->audiobuf_size>>1;
		u16 byteperchannel = bytepersample>>1;
		u8* tmp_buf = (u8*)linearAlloc(Sound->audiobuf_size);
		fread(tmp_buf, Sound->audiobuf_size, 1, stream);
		int z = 0;
		for (u32 i=0;i<chn_size;i=i+bytepersample){
			memcpy(&Sound->audiobuf[z], &tmp_buf[i], byteperchannel);
			memcpy(&Sound->audiobuf[z+chn_size], &tmp_buf[i+2], byteperchannel);
			z=z+2;
		}
		linearFree(tmp_buf);
	}
	
	fclose(stream);
	#ifdef USE_CACHE
	return LAST_ENTRY;
	#else
	return 0;
	#endif
}

int DecodeSound(std::string const& filename, DecodedSound* Sound){
	
	// Opening file
	FILE* stream = FileFinder::fopenUTF8(filename, "rb");
	if (!stream) {
		Output::Warning("Couldn't open sound file %s", filename.c_str());
		return -1;
	}
	
	// Reading and parsing the magic
	u32 magic;
	fread(&magic, 4, 1, stream);
	if (magic == 0x46464952) return DecodeWav(stream, Sound);
	else if (magic == 0x5367674F) return DecodeOgg(stream, Sound);
	else{
		fclose(stream);
		Output::Warning("Unsupported sound format (%s)", filename.c_str());
		return -1;
	}
	
}

/*	
	+-----------------------------------------------------+
	|                                                     |
	|                      MUSICS                         |
	|                                                     |
	+-----------------------------------------------------+
*/


void UpdateOggStream(){	
	DecodedMusic* Sound = BGM;
	OggVorbis_File* vf = (OggVorbis_File*)Sound->handle;
	Sound->block_idx++;	
	u32 half_buf = Sound->audiobuf_size>>1;
	int bytesRead = 0;
	int current_section;
	int half_check = (Sound->block_idx)%2;
	if (!Sound->isStereo){ // Mono file
		int i = half_check * half_buf;
		while(bytesRead < half_buf){
			long ret=ov_read(vf,(char*)&Sound->audiobuf[i+bytesRead],OGG_BUFSIZE,0,2,1,&current_section);
			if (ret == 0){ // EoF
				Sound->eof_idx = Sound->block_idx + 1;
				ov_pcm_seek(vf,0);
			}else bytesRead = bytesRead + ret;
		}
	}else{ // Stereo file
		char pcmout[OGG_BUFSIZE];
		u8* left_channel = Sound->audiobuf;
		u8* right_channel = &Sound->audiobuf[half_buf];
		int z = half_check * (half_buf>>1);
		while(bytesRead < half_buf){
			long ret=ov_read(vf,pcmout,OGG_BUFSIZE,0,2,1,&current_section);
			if (ret == 0){ // EoF
				Sound->eof_idx = Sound->block_idx + 1;
				ov_pcm_seek(vf,0);
			}else{
				for (u32 i=0;i<ret;i=i+4){
					memcpy(&left_channel[z],&pcmout[i],2);
					memcpy(&right_channel[z],&pcmout[i+2],2);
					z = z + 2;
				}
				bytesRead = bytesRead + ret;
			}
		}
	}
	
}

void UpdateWavStream(){
	DecodedMusic* Sound = BGM;
	Sound->block_idx++;	
	u32 half_buf = Sound->audiobuf_size>>1;
	int bytesRead;
	int half_check = (Sound->block_idx)%2;
	
	// Mono file
	if (!Sound->isStereo){
		bytesRead = fread(Sound->audiobuf+(half_check*half_buf), 1, half_buf, Sound->handle);	
		if (bytesRead != half_buf){ // EoF
			Sound->eof_idx = Sound->block_idx + 1;
			fseek(Sound->handle, Sound->audiobuf_offs, SEEK_SET);
			fread(Sound->audiobuf+(half_check*half_buf), 1, half_buf, Sound->handle);	
		}
		
	// Stereo file
	}else{
		u8* left_channel = Sound->audiobuf;
		u8* right_channel = Sound->audiobuf + half_buf;
		u32 half_chn_size = half_buf>>1;
		u16 byteperchannel = Sound->bytepersample>>1;
		u32 z = half_chn_size * half_check;
		u8* tmp_buf = (u8*)linearAlloc(half_buf);
		bytesRead = fread(tmp_buf, 1, half_buf, Sound->handle);
		if (bytesRead != half_buf){ // EoF
			Sound->eof_idx = Sound->block_idx + 1;
			fseek(Sound->handle, Sound->audiobuf_offs, SEEK_SET);
			fread(tmp_buf, 1, half_buf, Sound->handle);
		}
		for (u32 i=0;i<half_buf;i=i+Sound->bytepersample){
			memcpy(&left_channel[z],&tmp_buf[i],2);
			memcpy(&right_channel[z],&tmp_buf[i+2],2);
			z = z + 2;	
		}
		linearFree(tmp_buf);
	}
	
}

void CloseWav(){
	if (BGM->handle != NULL) fclose(BGM->handle);
}

void CloseOgg(){
	if (BGM->handle != NULL) ov_clear((OggVorbis_File*)BGM->handle);
}

int OpenWav(FILE* stream, DecodedMusic* Sound){
	
	// Setting return code (to check if audio streaming is needed or not)
	u8 res = 0;
	
	// Grabbing info from the header
	u16 audiotype;
	u32 chunk;
	u32 jump;
	fseek(stream, 16, SEEK_SET);
	fread(&jump, 4, 1, stream);
	fread(&Sound->format, 2, 1, stream);
	fread(&audiotype, 2, 1, stream);
	fread(&Sound->samplerate, 4, 1, stream);	
	if (audiotype == 2) Sound->isStereo = true;
	else Sound->isStereo = false;
	fseek(stream, 32, SEEK_SET);
	fread(&Sound->bytepersample, 2, 1, stream);
	fseek(stream, 20, SEEK_SET);
	
	// Check for file audiocodec
	if (Sound->format == 0x11) Sound->format = CSND_ENCODING_ADPCM;
	else if (Sound->bytepersample == 4 || (Sound->bytepersample == 2 && audiotype == 1)) Sound->format = CSND_ENCODING_PCM16;
	else Sound->format = CSND_ENCODING_PCM8;
	
	// Skipping to audiobuffer start
	while (chunk != 0x61746164){
		fseek(stream, jump, SEEK_CUR);
		fread(&chunk, 4, 1, stream);
		fread(&jump, 4, 1, stream);
	}
	
	// Getting audiobuffer size
	int start = ftell(stream);
	fseek(stream, 0, SEEK_END);
	int end = ftell(stream);
	Sound->audiobuf_size = end - start;
	if (Sound->audiobuf_size <= BGM_BUFSIZE) res = 1;
	else{
		while (Sound->audiobuf_size > BGM_BUFSIZE){
			Sound->audiobuf_size = Sound->audiobuf_size>>1;
		}
	}
	Sound->audiobuf_offs = start;
	fseek(stream, start, SEEK_SET);
	Sound->audiobuf = (u8*)linearAlloc(Sound->audiobuf_size);
	
	// Mono file
	if (audiotype == 1) fread(Sound->audiobuf, 1, Sound->audiobuf_size, stream);	
	
	// Stereo file
	else{
		u32 chn_size = Sound->audiobuf_size>>1;
		u16 byteperchannel = Sound->bytepersample>>1;
		for (u32 i=0;i<chn_size;i=i+byteperchannel){
			fread(&Sound->audiobuf[i], 1, byteperchannel, stream);
			fread(&Sound->audiobuf[i+chn_size], 1, byteperchannel, stream);
		}
	}
	
	//Setting default streaming values
	Sound->block_idx = 1;
	if (res == 0) Sound->handle = stream;
	else{
		fclose(stream);
		Sound->handle = NULL;
	}
	Sound->eof_idx = 0xFFFFFFFF;
	Sound->updateCallback = UpdateWavStream;
	Sound->closeCallback = CloseWav;
	
	return res;
}


int OpenOgg(FILE* stream, DecodedMusic* Sound){
	
	// Setting return code (to check if audio streaming is needed or not)
	u8 res = 0;
	
	// Passing filestream to libogg
	int eof=0;
	OggVorbis_File* vf = (OggVorbis_File*)malloc(sizeof(OggVorbis_File));
	static int current_section;
	fseek(stream, 0, SEEK_SET);
	if(ov_open(stream, vf, NULL, 0) != 0)
	{
		fclose(stream);
		Output::Warning("Corrupt ogg file");
		return -1;
	}
	
	// Grabbing info from the header
	vorbis_info* my_info = ov_info(vf,-1);
	Sound->samplerate = my_info->rate;
	Sound->format = CSND_ENCODING_PCM16;
	u16 audiotype = my_info->channels;
	Sound->audiobuf_size = ov_pcm_total(vf,-1)<<audiotype;
	if (audiotype == 2) Sound->isStereo = true;
	else Sound->isStereo = false;
	
	// Preparing PCM16 audiobuffer
	if (Sound->audiobuf_size <= BGM_BUFSIZE) res = 1;
	else{
		while (Sound->audiobuf_size > BGM_BUFSIZE){
			Sound->audiobuf_size = Sound->audiobuf_size>>1;
		}
	}
	Sound->audiobuf = (u8*)linearAlloc(Sound->audiobuf_size);
	
	// Decoding Vorbis buffer
	int i = 0;
	if (audiotype == 1){ // Mono file
		while(!eof){
			long ret=ov_read(vf,(char*)&Sound->audiobuf[i],OGG_BUFSIZE,0,2,1,&current_section);
			if (ret == 0) eof=1;
			else i = i + ret;
		}
	}else{ // Stereo file
		char pcmout[OGG_BUFSIZE];
		int z = 0;
		u32 chn_size = Sound->audiobuf_size>>1;
		u8* left_channel = Sound->audiobuf;
		u8* right_channel = &Sound->audiobuf[chn_size];
		while(!eof){
			long ret=ov_read(vf,pcmout,OGG_BUFSIZE,0,2,1,&current_section);
			if (ret == 0) eof=1;
			else{
				for (u32 i=0;i<ret;i=i+4){
					memcpy(&left_channel[z],&pcmout[i],2);
					memcpy(&right_channel[z],&pcmout[i+2],2);
					z = z + 2;
				}
			}
		}
	}
	
	//Setting default streaming values
	Sound->block_idx = 1;
	if (res == 0) Sound->handle = (FILE*)vf; // We pass libogg filestream instead of stdio ones
	else{
		ov_clear(vf);
		Sound->handle = NULL;
	}
	Sound->eof_idx = 0xFFFFFFFF;
	Sound->updateCallback = UpdateOggStream;
	Sound->closeCallback = CloseOgg;
	
	return res;
}

int DecodeMusic(std::string const& filename, DecodedMusic* Sound){
	
	// Opening file
	FILE* stream = FileFinder::fopenUTF8(filename, "rb");
	if (!stream) {
		Output::Warning("Couldn't open music file %s", filename.c_str());
		return -1;
	}
	
	// Reading and parsing the magic
	u32 magic;
	fread(&magic, 4, 1, stream);
	if (magic == 0x46464952) return OpenWav(stream, Sound);
	else if (magic == 0x5367674F) return OpenOgg(stream, Sound);
	else{
		fclose(stream);
		Output::Warning("Unsupported music format (%s)", filename.c_str());
		return -1;
	}
	
}
#endif
 
 