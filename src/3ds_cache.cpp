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
#include "3ds_cache.h"
#include "output.h"

uint32_t FREE_CACHE = CACHE_DIM;
uint8_t ENTRIES = 0;
int LAST_ENTRY = -1;
char soundtable[MAX_SOUNDS][32];
DecodedSound decodedtable[MAX_SOUNDS];
uint8_t* soundCache;
bool FULLED = false;

void initCache(){
	#ifndef NO_DEBUG
	Output::Debug("Initializing sound cache (Dim: %i bytes)",CACHE_DIM);
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
#endif