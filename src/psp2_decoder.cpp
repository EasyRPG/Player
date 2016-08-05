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
std::unique_ptr<AudioDecoder> sfx_decoder[8];
uint8_t cur_decoder = 0;

void UpdateSoundDecoderStream(DecodedSound* Sound){
	
	if (Sound->cur_audiobuf == Sound->audiobuf) Sound->cur_audiobuf = Sound->audiobuf2;
	else Sound->cur_audiobuf = Sound->audiobuf;
	sfx_decoder[Sound->id]->Decode(Sound->cur_audiobuf, Sound->audiobuf_size);	
	if (sfx_decoder[Sound->id]->IsFinished()){ // EoF
		Sound->endedOnce = true;
	}
}

void CloseSoundDecoder(DecodedSound* Sound){
	sfx_decoder[Sound->id].reset();
}

void PitchSoundDecoder(DecodedSound* Sound){
	sfx_decoder[Sound->id]->SetPitch(Sound->pitch);
}

int OpenSoundDecoder(uint8_t id, FILE* stream, DecodedSound* Sound, std::string const& filename){
	
	// Initializing internal audio decoder
	int audiotype;
	fseek(stream, 0, SEEK_SET);
	if (!sfx_decoder[id]->Open(stream)) Output::Error("An error occured in audio decoder (%s)", audio_decoder->GetError().c_str());
	sfx_decoder[id]->SetLooping(false);
	AudioDecoder::Format int_format;
	int samplerate;	
	sfx_decoder[id]->SetFormat(48000, AudioDecoder::Format::S16, 2);
	sfx_decoder[id]->GetFormat(samplerate, int_format, audiotype);
	Sound->samplerate = samplerate;
	Sound->id = id;
	
	// Check for file audiocodec
	if (audiotype == 2) Sound->isStereo = true;
	else Sound->isStereo = false;
	
	// Setting audiobuffer size
	Sound->audiobuf_size = BGM_BUFSIZE;
	Sound->audiobuf = (uint8_t*)malloc(Sound->audiobuf_size);
	Sound->audiobuf2 = (uint8_t*)malloc(Sound->audiobuf_size);
	Sound->cur_audiobuf = Sound->audiobuf;	
	
	//Setting default streaming values
	Sound->handle = stream;
	Sound->endedOnce = false;
	Sound->updateCallback = UpdateSoundDecoderStream;
	Sound->closeCallback = CloseSoundDecoder;
	Sound->pitchCallback = PitchSoundDecoder;
	
	return 0;
}

int DecodeSound(std::string const& filename, DecodedSound* Sound){
	
	// Opening file
	FILE* stream = FileFinder::fopenUTF8(filename, "rb");
	if (!stream) {
		Output::Warning("Couldn't open sound file %s", filename.c_str());
		return -1;
	}
	
	// Trying to use internal decoder
	uint8_t id = cur_decoder++;
	if (cur_decoder > 7) cur_decoder = 0;
	sfx_decoder[id] = AudioDecoder::Create(stream, filename);
	if (sfx_decoder[id] != NULL) return OpenSoundDecoder(id, stream, Sound, filename);
	fclose(stream);
	Output::Warning("Unsupported sound format (%s)", filename.c_str());
	return -1;
	
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
	
	if (Sound->cur_audiobuf == Sound->audiobuf) Sound->cur_audiobuf = Sound->audiobuf2;
	else Sound->cur_audiobuf = Sound->audiobuf;
	audio_decoder->Decode(Sound->cur_audiobuf, Sound->audiobuf_size);	
	if (audio_decoder->GetLoopCount() > 0){ // EoF
		Sound->endedOnce = true;
	}
}

void CloseAudioDecoder(){
	audio_decoder.reset();
}

int OpenAudioDecoder(FILE* stream, DecodedMusic* Sound, std::string const& filename){
	
	// Initializing internal audio decoder
	int audiotype;
	fseek(stream, 0, SEEK_SET);
	if (!audio_decoder->Open(stream)) Output::Error("An error occured in audio decoder (%s)", audio_decoder->GetError().c_str());
	audio_decoder->SetLooping(true);
	AudioDecoder::Format int_format;
	int samplerate;	
	audio_decoder->SetFormat(48000, AudioDecoder::Format::S16, 2);
	audio_decoder->GetFormat(samplerate, int_format, audiotype);
	Sound->samplerate = samplerate;
	
	// Check for file audiocodec
	if (audiotype == 2) Sound->isStereo = true;
	else Sound->isStereo = false;
	
	// Setting audiobuffer size
	Sound->audiobuf_size = BGM_BUFSIZE;
	Sound->audiobuf = (uint8_t*)malloc(Sound->audiobuf_size);
	Sound->audiobuf2 = (uint8_t*)malloc(Sound->audiobuf_size);
	Sound->cur_audiobuf = Sound->audiobuf;	
	
	//Setting default streaming values
	Sound->handle = stream;
	Sound->endedOnce = false;
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

