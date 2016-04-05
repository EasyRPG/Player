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
 #ifdef USE_CACHE
#ifdef _3DS
#include <3ds.h>
#endif
#include <string.h>
#include <stdio.h>
#include "3ds_cache.h"
#include "output.h"

uint32_t FREE_CACHE = CACHE_DIM; // 4 bytes
uint8_t ENTRIES = 0; // 1 byte
int LAST_ENTRY = -1; // 4 bytes
char soundtable[MAX_SOUNDS][32];
DecodedSound decodedtable[MAX_SOUNDS];
uint8_t* soundCache;
bool FULLED = false; // 1 byte

void initCache(){
	#ifndef NO_DEBUG
	u32 cache_size = CACHE_DIM + 10 + (MAX_SOUNDS<<5) + sizeof(DecodedSound) * MAX_SOUNDS; 
	Output::Debug("Initializing sound cache (Dim: %i bytes, Max Sounds: %i)",cache_size,MAX_SOUNDS);
	#endif
	#ifdef _3DS
	soundCache = (uint8_t*)linearAlloc(CACHE_DIM);
	#endif
}
void freeCache(){
	#ifdef _3DS
	linearFree(soundCache);
	#endif
}

int lookCache(const char* file){
	for (int i=0;i<ENTRIES;i++){
		if (strcmp(soundtable[i],file) == 0) return i;
	}
	return -1;
}

void allocCache(DecodedSound* Sound){

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
				i++;
				if (i == ENTRIES) break;
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
			i++;
			if (i == ENTRIES) break;
		}
		
	}
	memcpy(&decodedtable[ENTRIES-1],Sound,sizeof(DecodedSound));
}
#endif