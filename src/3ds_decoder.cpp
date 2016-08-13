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

#if defined (_3DS) && defined(SUPPORT_AUDIO)
#include "output.h"
#include "filefinder.h"
#include "player.h"
#include <tremor/ivorbiscodec.h>
#include <tremor/ivorbisfile.h>

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
std::unique_ptr<AudioDecoder> audio_decoder;
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
	Sound->audiobuf_size = ov_time_total(vf,-1) * (my_info->rate<<1);
	if (audiotype == 2) Sound->isStereo = true;
	else Sound->isStereo = false;
	Sound->bytepersample = audiotype<<1;
	
	// Preparing PCM16 audiobuffer
	#ifdef USE_CACHE
	allocCache(Sound);
	#else
	Sound->audiobuf = (u8*)linearAlloc(Sound->audiobuf_size);
	#endif
	
	if (Player::use_dsp) audiotype = 1; // We trick the decoder since DSP supports native stereo playback
	
	// Decoding Vorbis buffer
	int i = 0;
	if (audiotype == 1){ // Mono file
		while(!eof){
			long ret=ov_read(vf,(char*)&Sound->audiobuf[i],OGG_BUFSIZE,&current_section);
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
			long ret=ov_read(vf,pcmout,OGG_BUFSIZE,&current_section);
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
	
	if (Player::use_dsp) audiotype = 1; // We trick the decoder since DSP supports native stereo playback
	
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

void UpdateAudioDecoderStream(){
	DecodedMusic* Sound = BGM;
	Sound->block_idx++;	
	u32 half_buf = Sound->audiobuf_size>>1;
	int half_check = (Sound->block_idx)%2;
	
	if (Sound->pitch != audio_decoder->GetPitch()) {
			audio_decoder->SetPitch(Sound->pitch);
	}
	
	// Mono file
	if ((!Sound->isStereo) || Player::use_dsp){
		audio_decoder->Decode(Sound->audiobuf+(half_check*half_buf), half_buf);		
		if (audio_decoder->GetLoopCount() > 0){ // EoF
			if (Sound->eof_idx == 0xFFFFFFFF) Sound->eof_idx = Sound->block_idx + 1;
		}
		
	// Stereo file
	}else{
		u8* left_channel = Sound->audiobuf;
		u8* right_channel = Sound->audiobuf + half_buf;
		u32 half_chn_size = half_buf>>1;
		u32 z = half_chn_size * half_check;
		audio_decoder->DecodeAsMono(&left_channel[z],&right_channel[z], half_chn_size);		
		if (audio_decoder->GetLoopCount() > 0){ // EoF
			if (Sound->eof_idx == 0xFFFFFFFF) Sound->eof_idx = Sound->block_idx + 1;
		}
	}
	
}

void CloseAudioDecoder(){
	audio_decoder.reset();
}

int OpenAudioDecoder(FILE* stream, DecodedMusic* Sound, std::string const& filename){
	
	// Initializing internal audio decoder
	int audiotype;
	fseek(stream, 0, SEEK_SET);
	audio_decoder->Open(stream);
	audio_decoder->SetLooping(true);
	AudioDecoder::Format int_format;
	int samplerate;	
	audio_decoder->GetFormat(samplerate, int_format, audiotype);
	if (strstr(filename.c_str(),".mid") != NULL){
		#ifdef MIDI_DEBUG
		samplerate = 11025;
		Output::Warning("MIDI track detected, lowering samplerate to 11025 Hz.");
		audio_decoder->SetFormat(11025, AudioDecoder::Format::S16, 2);
		#else
		Output::Warning("MIDI tracks currently unsupported (%s)", filename.c_str());
		return -1;
		#endif
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
		Sound->format = CSND_ENCODING_PCM16;
		Sound->bytepersample = audiotype<<1;
	}else{
		Sound->format = CSND_ENCODING_PCM8;
		Sound->bytepersample = audiotype;
	}
	if (audiotype == 2) Sound->isStereo = true;
	else Sound->isStereo = false;
	
	// Setting audiobuffer size
	Sound->audiobuf_size = BGM_BUFSIZE;
	Sound->audiobuf_offs = 0;
	Sound->audiobuf = (u8*)linearAlloc(Sound->audiobuf_size);
	
	if (Player::use_dsp) audiotype = 1; // We trick the decoder since DSP supports native stereo playback
	int res;
	
	// Mono file
	if (audiotype == 1) res=audio_decoder->Decode(Sound->audiobuf, Sound->audiobuf_size);	
	
	// Stereo file
	else{
		u32 chn_size = Sound->audiobuf_size>>1;
		res=audio_decoder->DecodeAsMono(Sound->audiobuf, &Sound->audiobuf[chn_size], chn_size);
	}
	
	//Setting default streaming values
	Sound->pitch = 100;
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

