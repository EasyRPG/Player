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
#include "3ds_decoder.h"
	
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
	Sound->audiobuf = (u8*)linearAlloc(Sound->audiobuf_size);

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
	return 0;
	
}
#endif
 
 