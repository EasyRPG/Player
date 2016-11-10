/*
 * This file is part of EasyRPG Player
 *
 * Copyright (c) 2016 EasyRPG Project. All rights reserved.
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 */

#include "org_easyrpg_player_game_browser_GameScanner.h"

#include <zlib.h>
#include <png.h>
#include <cstring>
#include <fstream>
#include <vector>
#include <stdint.h>

// via https://stackoverflow.com/q/1821806
static void custom_png_write_func(png_structp  png_ptr, png_bytep data, png_size_t length) {
	std::vector<uint8_t> *p = (std::vector<uint8_t>*)png_get_io_ptr(png_ptr);
	p->insert(p->end(), data, data + length);
}

extern "C"
JNIEXPORT jbyteArray JNICALL Java_org_easyrpg_player_game_1browser_GameScanner_decodeXYZ
  (JNIEnv * env, jclass, jstring j_filename)
{
	const char* filename = env->GetStringUTFChars( j_filename, 0);

	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	env->ReleaseStringUTFChars(j_filename, filename);
	
	if (!file) {
		return nullptr;
	}

	long size = file.tellg();
	char* header = new char[4];

	file.seekg(0, std::ios::beg);
	file.read((char*) header, 4);
	if(memcmp(header, "XYZ1", 4) != 0) {
		delete[] header;
		return nullptr;
	}
	delete[] header;

	unsigned short width;
	unsigned short height;
	file.read((char*) &width, 2);
	file.read((char*) &height, 2);

	int compressed_xyz_size = size - 8;
	Bytef* compressed_xyz_data = new Bytef[compressed_xyz_size];

	file.read((char*) compressed_xyz_data, compressed_xyz_size);

	uLongf xyz_size = 768 + (width * height);
	std::vector<Bytef> xyz_data(
		xyz_size);

	int status = uncompress(&xyz_data.front(),
		&xyz_size, compressed_xyz_data,
		compressed_xyz_size);

	if(status != Z_OK) {
		return nullptr;
	}

	png_structp png_ptr;
	png_infop info_ptr;

	// Create PNG write structure
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,
		NULL, NULL);
	if(png_ptr == NULL)
	{
		return nullptr;
	}

	// Create PNG info structure
	info_ptr = png_create_info_struct(png_ptr);
	if(info_ptr == NULL)
	{
		png_destroy_write_struct(&png_ptr, NULL);
		return nullptr;
	}

	// Init I/O functions
	if(setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return nullptr;
	}
	
	std::vector<uint8_t*> png_outbuf;
	
	png_set_write_fn(png_ptr, &png_outbuf, custom_png_write_func, nullptr);

	// Set compression parameters
	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
	png_set_compression_mem_level(png_ptr, MAX_MEM_LEVEL);
	png_set_compression_buffer_size(png_ptr, 1024 * 1024);

	// Write header
	if(setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return nullptr;
	}
	png_set_IHDR(png_ptr, info_ptr, width, height, 8,
		PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	// Write palette
	if(setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return nullptr;
	}
	png_colorp palette = (png_colorp) png_malloc(png_ptr,
		PNG_MAX_PALETTE_LENGTH * (sizeof (png_color)));

	for(int i = 0; i < PNG_MAX_PALETTE_LENGTH; i++)
	{
		palette[i].red = xyz_data[i * 3];
		palette[i].green = xyz_data[i * 3 + 1];
		palette[i].blue = xyz_data[i * 3 + 2];
	}
	png_set_PLTE(png_ptr, info_ptr, palette,
		PNG_MAX_PALETTE_LENGTH);

	png_write_info(png_ptr, info_ptr);

	png_bytep* row_pointers = new png_bytep[height];
	for(int i = 0; i < height; i++) {
		row_pointers[i] =
			&xyz_data[768 + width * i];
	}
	png_write_image(png_ptr, row_pointers);
	delete[] row_pointers;

	png_write_end(png_ptr, info_ptr);

	png_free(png_ptr, palette);
	palette = NULL;

	// No idea why but using the png_outbuf vector we pass in as custom arg
	// directly returns the wrong value for size() o.O
	
	std::vector<uint8_t> *p = (std::vector<uint8_t>*)png_get_io_ptr(png_ptr);

	jbyteArray result=env->NewByteArray(p->size());

	env->SetByteArrayRegion(result, 0, p->size(), reinterpret_cast<jbyte*>(p->data()));

	png_destroy_write_struct(&png_ptr, &info_ptr);
	
	return result;
}
