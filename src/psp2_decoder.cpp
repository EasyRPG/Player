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

#if defined (PSP2) && defined(SUPPORT_AUDIO)
#include "output.h"
#include "filefinder.h"
#include "player.h"
#include <psp2/audioout.h>
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#ifdef USE_CACHE
#include "3ds_cache.h"
#else
#include "psp2_decoder.h"
#endif
extern DecodedMusic* BGM;
std::unique_ptr<AudioDecoder> audio_decoder;


int DecodeWav(FILE* stream, DecodedSound* Sound){
	
	// Grabbing info from the header
	uint16_t audiotype;
	uint32_t chunk;
	uint32_t jump;
	uint16_t bytepersample;
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
	Sound->audiobuf = (uint8_t*)malloc(Sound->audiobuf_size);
	#endif
	
	fread(Sound->audiobuf, Sound->audiobuf_size, 1, stream);		
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
	uint32_t magic;
	fread(&magic, 4, 1, stream);
	if (magic == 0x46464952) return DecodeWav(stream, Sound);
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

void UpdateAudioDecoderStream(){
	DecodedMusic* Sound = BGM;
	Sound->block_idx++;	
	int half_check = (Sound->block_idx)%2;
	
	if (Sound->cur_audiobuf == Sound->audiobuf) Sound->cur_audiobuf = Sound->audiobuf2;
	else Sound->cur_audiobuf = Sound->audiobuf;
	audio_decoder->Decode(Sound->cur_audiobuf, Sound->audiobuf_size);	
	if (audio_decoder->GetLoopCount() > 0){ // EoF
		if (Sound->eof_idx == 0xFFFFFFFF) Sound->eof_idx = Sound->block_idx + 1;
	}
}

void CloseAudioDecoder(){
	if (BGM->handle != NULL) fclose(BGM->handle);
}

int OpenAudioDecoder(FILE* stream, DecodedMusic* Sound, std::string const& filename){
	
	// Initializing internal audio decoder
	int audiotype;
	fseek(stream, 0, SEEK_SET);
	if (!audio_decoder->Open(stream)) Output::Error("An error occured in audio decoder (%s)", audio_decoder->GetError().c_str());
	audio_decoder->SetLooping(true);
	AudioDecoder::Format int_format;
	int samplerate;	
	audio_decoder->GetFormat(samplerate, int_format, audiotype);
	if (strstr(filename.c_str(),".mid") != NULL){
		#ifdef MIDI_DEBUG
		samplerate = 11025;
		Output::Warning("MIDI track detected, lowering samplerate to 11025 Hz.", filename.c_str());
		#else
		Output::Warning("MIDI tracks currently unsupported.", filename.c_str());
		return -1;
		#endif
		audio_decoder->SetFormat(11025, AudioDecoder::Format::S16, 2);
	}else if (int_format > AudioDecoder::Format::S16){
		bool isCompatible = audio_decoder->SetFormat(samplerate, AudioDecoder::Format::S16, audiotype);
		if (!isCompatible){
			Output::Warning("Unsupported music audiocodec (%s)", filename.c_str());
			fclose(stream);
			return -1;
		}
		int_format = AudioDecoder::Format::S16;
	}
	Sound->samplerate = samplerate;
	
	// Check for file audiocodec
	if (int_format > AudioDecoder::Format::U8){
		Sound->bytepersample = audiotype<<1;
	}else{
		Sound->bytepersample = audiotype;
	}
	if (audiotype == 2) Sound->isStereo = true;
	else Sound->isStereo = false;
	
	// Setting audiobuffer size
	Sound->audiobuf_size = BGM_BUFSIZE;
	Sound->audiobuf_offs = 0;
	Sound->audiobuf = (uint8_t*)malloc(Sound->audiobuf_size);
	Sound->audiobuf2 = (uint8_t*)malloc(Sound->audiobuf_size);
	Sound->cur_audiobuf = Sound->audiobuf;	
	
	//Setting default streaming values
	Sound->block_idx = 1;
	Sound->handle = stream;
	Sound->eof_idx = 0xFFFFFFFF;
	Sound->updateCallback = UpdateAudioDecoderStream;
	Sound->closeCallback = CloseAudioDecoder;
	
	return 0;
}

int DecodeMusic(std::string const& filename, DecodedMusic* Sound){
	
	// Opening file
	FILE* stream = FileFinder::fopenUTF8(filename, "rb");
	if (!stream) {
		Output::Warning("Couldn't open music file %s", filename.c_str());
		return -1;
	}
	
	// Trying to use internal decoder
	audio_decoder = AudioDecoder::Create(stream, filename);
	if (audio_decoder != NULL) return OpenAudioDecoder(stream, Sound, filename);
	fclose(stream);
	Output::Warning("Unsupported music format (%s)", filename.c_str());
	return -1;
	
}
#endif

