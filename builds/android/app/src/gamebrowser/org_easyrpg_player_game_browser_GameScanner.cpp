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
Java_org_easyrpg_player_game_1browser_GameScanner_findGames(JNIEnv *env, jclass clazz, jstring jpath, jstring jmain_dir_name) {
	EpAndroid::env = env;

	const char* path = env->GetStringUTFChars(jpath, nullptr);
	std::string spath(path);
	env->ReleaseStringUTFChars(jpath, path);

	auto root = FileFinder::Root().Create(spath);
	std::vector<FilesystemView> fs_list = FileFinder::FindGames(root);

	jclass jgame_class = env->FindClass("org/easyrpg/player/game_browser/Game");
	jobjectArray jgame_array = env->NewObjectArray(fs_list.size(), jgame_class, nullptr);

	if (fs_list.empty()) {
		return jgame_array;
	}

	jmethodID jgame_constructor = env->GetMethodID(jgame_class, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[B)V");

	bool game_in_main_dir = false;
	if (fs_list.size() == 1) {
		if (FileFinder::GetFullFilesystemPath(root) == FileFinder::GetFullFilesystemPath(fs_list[0])) {
			game_in_main_dir = true;
		}
	}

	for (size_t i = 0; i < fs_list.size(); ++i) {
		auto& fs = fs_list[i];

		std::string full_path = FileFinder::GetFullFilesystemPath(fs);
		std::string title;
		if (game_in_main_dir) {
			// The main dir is URI encoded, the human readable name is in jmain_dir_name
			const char* main_dir_name = env->GetStringUTFChars(jmain_dir_name, nullptr);
			title = main_dir_name;
			env->ReleaseStringUTFChars(jmain_dir_name, main_dir_name);
		} else {
			// In all other cases the folder name is "clean" and can be used
			title = std::get<1>(FileFinder::GetPathAndFilename(fs.GetFullPath()));
		}

		std::string save_path;
		if (!fs.IsFeatureSupported(Filesystem::Feature::Write)) {
			// Is an archive and needs a redirected save path
			save_path = title;

			// compatibility with original GameScanner Java code (everything after the extension dot is removed)
			size_t ext = save_path.find('.');
			if (ext != std::string::npos) {
				save_path = save_path.substr(0, ext);
			}
		}

		// Very simple title graphic search: The first image in "Title" is used
		auto title_fs = fs.Subtree("Title");
		jbyteArray title_image = nullptr;
		if (title_fs) {
			for (auto &[name, entry]: *title_fs.ListDirectory()) {
				if (entry.type == DirectoryTree::FileType::Regular) {
					if (StringView(name).ends_with(".xyz")) {
						auto is = title_fs.OpenInputStream(entry.name);
						title_image = readXyz(env, is);
					} else if (StringView(name).ends_with(".png") ||
							   StringView(name).ends_with(".bmp")) {
						auto is = title_fs.OpenInputStream(entry.name);
						if (!is) {
							// When opening of the image fails it is an unsupported archive format
							// Skip this game
							continue;
						}

						auto vec = Utils::ReadStream(is);
						title_image = env->NewByteArray(vec.size());
						env->SetByteArrayRegion(title_image, 0, vec.size(),
												reinterpret_cast<jbyte *>(vec.data()));
					}
				}
			}
		}

		// Create an instance of "Game"
		jstring jgame_path = env->NewStringUTF(("content://" + full_path).c_str());
		jstring jsave_path = env->NewStringUTF(save_path.c_str());
		jstring jtitle = env->NewStringUTF(title.c_str());
		jobject jgame_object = env->NewObject(jgame_class, jgame_constructor, jgame_path, jsave_path, jtitle, title_image);

		env->SetObjectArrayElement(jgame_array, i, jgame_object);
	}

	// Some fields of the Array can be NULL when a game was skipped due to an error
	// This is sanitized on the Java site
	return jgame_array;
}
