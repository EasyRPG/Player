/*
 * This file is part of EasyRPG Player
 *
 * Copyright (c) 2021 EasyRPG Project. All rights reserved.
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
#include <array>


#include "filefinder.h"
#include "utils.h"
#include "string_view.h"
#include "platform/android/android.h"

// via https://stackoverflow.com/q/1821806
static void custom_png_write_func(png_structp  png_ptr, png_bytep data, png_size_t length) {
	std::vector<uint8_t> *p = reinterpret_cast<std::vector<uint8_t>*>(png_get_io_ptr(png_ptr));
	p->insert(p->end(), data, data + length);
}

jbyteArray readXyz(JNIEnv *env, std::istream& stream) {
	char header[4];

	stream.read(header, 4);
	if(memcmp(header, "XYZ1", 4) != 0) {
		return nullptr;
	}

	unsigned short width;
	unsigned short height;
	stream.read((char*) &width, 2);
	stream.read((char*) &height, 2);

	constexpr int buffer_incr = 8192;
	std::vector<char> compressed_xyz_data;
	do {
		compressed_xyz_data.resize(compressed_xyz_data.size() + buffer_incr);
		stream.read(compressed_xyz_data.data() + compressed_xyz_data.size() - buffer_incr, buffer_incr);
	} while (stream.gcount() == buffer_incr);
	compressed_xyz_data.resize(compressed_xyz_data.size() - buffer_incr + stream.gcount());

	uLongf xyz_size = 768 + (width * height);
	std::vector<Bytef> xyz_data(
		xyz_size);

	int status = uncompress(&xyz_data.front(),
		&xyz_size, reinterpret_cast<const Bytef *>(compressed_xyz_data.data()),
		compressed_xyz_data.size());

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

	std::vector<uint8_t> png_outbuf;

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

	jbyteArray result = env->NewByteArray(png_outbuf.size());

	env->SetByteArrayRegion(result, 0, png_outbuf.size(), reinterpret_cast<jbyte*>(png_outbuf.data()));

	png_destroy_write_struct(&png_ptr, &info_ptr);

	return result;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_org_easyrpg_player_game_1browser_GameScanner_findGame(JNIEnv *env, jclass clazz,
															 jstring path) {
	EpAndroid::env = env;

	const char* cpath = env->GetStringUTFChars(path, nullptr);
	std::string spath(cpath);
	env->ReleaseStringUTFChars(path, cpath);

	auto fs = FileFinder::FindGameRecursive(FileFinder::Root().Create(spath));
	if (!fs) {
		return nullptr;
	}

	std::string save_path;
	if (!fs.IsFeatureSupported(Filesystem::Feature::Write)) {
		save_path = std::get<1>(FileFinder::GetPathAndFilename(fs.GetFullPath()));

		// compatibility with Java code
		size_t ext = save_path.find('.');
		if (ext != std::string::npos) {
			save_path = save_path.substr(0, ext);
		}
	}

	jbyteArray title_image = nullptr;

	auto title = fs.Subtree("Title");
	if (title) {
		for (auto& [name, entry]: *title.ListDirectory()) {
			if (entry.type == DirectoryTree::FileType::Regular) {
				if (StringView(name).ends_with(".xyz")) {
					auto is = title.OpenInputStream(entry.name);
					title_image = readXyz(env, is);
				} else if (StringView(name).ends_with(".png") || StringView(name).ends_with(".bmp")) {
					auto is = title.OpenInputStream(entry.name);
					auto vec = Utils::ReadStream(is);

					title_image = env->NewByteArray(vec.size());
					env->SetByteArrayRegion(title_image, 0, vec.size(), reinterpret_cast<jbyte*>(vec.data()));
				}
			}
		}
	}

	jclass game_class = env->FindClass("org/easyrpg/player/game_browser/Game");

	jmethodID constructor = env->GetMethodID(game_class, "<init>", "(Ljava/lang/String;Ljava/lang/String;[B)V");

	jstring game_path = env->NewStringUTF(("content://" + FileFinder::GetFullFilesystemPath(fs)).c_str());
	jstring save_pat = env->NewStringUTF(save_path.c_str());
	jobject game_object = env->NewObject(game_class, constructor, game_path, save_pat, title_image);

	return game_object;
}
