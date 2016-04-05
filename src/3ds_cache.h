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
#include "system.h"
#ifdef _3DS
#include <3ds.h>
#include "3ds_decoder.h"
#endif

#define MAX_SOUNDS 32 // Max number of storable sounds
#define CACHE_DIM 6291456 // Dimension of the cache

extern uint32_t FREE_CACHE;
extern uint8_t ENTRIES;
extern int LAST_ENTRY;
extern char soundtable[MAX_SOUNDS][32];
extern DecodedSound decodedtable[MAX_SOUNDS];
extern uint8_t* soundCache;
extern bool FULLED;

void initCache();
void freeCache();
int lookCache(const char* file);
void allocCache(DecodedSound* Sound);
#endif