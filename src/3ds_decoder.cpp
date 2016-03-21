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

#ifdef _3DS
#include <3ds.h>
#include <stdio.h>
#include <string.h>
#ifdef USE_CACHE
#include "3ds_cache.h"
#else
#include "3ds_decoder.h"
#endif

int DecodeWav(std::string const& filename, DecodedSound* Sound){
	
	// Opening file
	FILE* stream = FileFinder::fopenUTF8(filename, "rb");
	if (!stream) {
		Output::Warning("Couldn't open sound file %s", filename.c_str());
		return -1;
	}
	
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
	
	// Calculate cache offset where to store the sound
	int offset = FREE_CACHE - Sound->audiobuf_size;
	if (offset >= 0){
		
		// Check if cache had been fulled at least once
		if (FULLED){
		
			// Store the sound in "normal" storage mode
			LAST_ENTRY++;
			if (LAST_ENTRY == ENTRIES) ENTRIES++;
			FREE_CACHE = offset;
			Sound->audiobuf = soundCache + offset;
			
			// Stub all the invalid entries due to offsets differences
			int i = LAST_ENTRY + 1;
			while (decodedtable[i].audiobuf < (Sound->audiobuf + Sound->audiobuf_size)){
				sprintf(soundtable[i],"%s","::::"); // A file with : in filename can't exist so we use that fake filename
			}
			
		}else{
			
			// Store the sound in "fast" storage mode
			Sound->audiobuf = soundCache + offset;
			LAST_ENTRY++;
			ENTRIES++;
			FREE_CACHE = offset;
			
		}
	}else{
		
		// Cache is full, so we reset to "normal" storage mode
		FREE_CACHE = CACHE_DIM;
		FULLED = true;
		LAST_ENTRY = 0;
		offset = FREE_CACHE - Sound->audiobuf_size;
		FREE_CACHE = offset;
		Sound->audiobuf = soundCache + offset;
		
		// Stub all the invalid entries due to offsets differences
		int i = 1;
		while (decodedtable[i].audiobuf < (Sound->audiobuf + Sound->audiobuf_size)){
			sprintf(soundtable[i],"%s","::::"); // A file with : in filename can't exist so we use that fake filename
		}
		
	}
	memcpy(&decodedtable[ENTRIES-1],Sound,sizeof(DecodedSound));
	#else
	Sound->audiobuf = (u8*)linearAlloc(Sound->audiobuf_size);
	#endif
	
	// Mono file
	if (audiotype == 1) fread(Sound->audiobuf, Sound->audiobuf_size, 1, stream);	
	
	// Stereo file
	else{
		u32 chn_size = Sound->audiobuf_size>>1;
		u16 byteperchannel = bytepersample>>1;
		for (u32 i=0;i<chn_size;i=i+byteperchannel){
			fread(&Sound->audiobuf[i], byteperchannel, 1, stream);
			fread(&Sound->audiobuf[i+chn_size], byteperchannel, 1, stream);
		}
	}
	
	fclose(stream);
	#ifdef USE_CACHE
	return LAST_ENTRY;
	#else
	return 0;
	#endif
}
#endif
 
 