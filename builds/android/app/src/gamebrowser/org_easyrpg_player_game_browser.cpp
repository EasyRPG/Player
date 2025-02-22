/*
 * This file is part of EasyRPG Player
 *
 * Copyright (c) EasyRPG Project. All rights reserved.
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

#include "org_easyrpg_player_game_browser.h"

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
#include "bitmap.h"
#include "font.h"
#include "cache.h"
#include "rtp.h"
#include "output.h"

#include <lcf/ldb/reader.h>
#include <lcf/reader_util.h>
#include <lcf/encoder.h>
#include <lcf/inireader.h>

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

std::string jstring_to_string(JNIEnv* env, jstring j_str) {
	if (!j_str) {
		return {};
	}
	const char* chars = env->GetStringUTFChars(j_str, NULL);
	std::string str(chars);
	env->ReleaseStringUTFChars(j_str, chars);
	return str;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_org_easyrpg_player_game_1browser_GameScanner_findGames(JNIEnv *env, jclass, jstring jpath, jstring jmain_dir_name) {
	Output::SetLogLevel(LogLevel::Error);

	auto sc = lcf::makeScopeGuard([&]() {
		// Prevent closing of the stream, is used afterwards
		Output::SetLogLevel(LogLevel::Debug);
	});

	EpAndroid::env = env;

	// jpath is the SAF path to the game, is converted to FilesystemView "root"
	std::string spath = jstring_to_string(env, jpath);
	auto root = FileFinder::Root().Create(spath);
	if (!root) {
		return nullptr;
	}

	root.ClearCache();

	auto ge_list = FileFinder::FindGames(root);

	jclass jgame_class = env->FindClass("org/easyrpg/player/game_browser/Game");
	jobjectArray jgame_array = env->NewObjectArray(ge_list.size(), jgame_class, nullptr);

	if (ge_list.empty()) {
		// No games found
		return nullptr;
	}

	jmethodID jgame_constructor_unsupported = env->GetMethodID(jgame_class, "<init>", "(I)V");
	jmethodID jgame_constructor_supported = env->GetMethodID(jgame_class, "<init>", "(Ljava/lang/String;Ljava/lang/String;[BI)V");

	std::string root_path = FileFinder::GetFullFilesystemPath(root);
	bool game_in_main_dir = false;
	if (ge_list.size() == 1) {
		if (root_path == FileFinder::GetFullFilesystemPath(ge_list[0].fs)) {
			game_in_main_dir = true;
		}
	}

	for (size_t i = 0; i < ge_list.size(); ++i) {
		auto& ge = ge_list[i];
		auto& fs = ge.fs;

		std::string full_path = FileFinder::GetFullFilesystemPath(fs);
		std::string game_dir_name;
		if (game_in_main_dir) {
			// The main dir is URI encoded, the human readable name is in jmain_dir_name
			game_dir_name = jstring_to_string(env, jmain_dir_name);
		} else {
			// In all other cases the folder name is "clean" and can be used
			game_dir_name = std::get<1>(FileFinder::GetPathAndFilename(fs.GetFullPath()));
		}

		// If game is unsupported, create a Game object with only directory name as title and project type id and continue early
		if (ge.type > FileFinder::ProjectType::Supported) {
			jobject jgame_object = env->NewObject(jgame_class, jgame_constructor_unsupported, (int)ge.type);

			// Use the directory name as the title
			jstring jfolder = env->NewStringUTF(game_dir_name.c_str());
			jmethodID jset_folder_name_method = env->GetMethodID(jgame_class, "setGameFolderName", "(Ljava/lang/String;)V");
			env->CallVoidMethod(jgame_object, jset_folder_name_method, jfolder);

			env->SetObjectArrayElement(jgame_array, i, jgame_object);
			continue;
		}

		std::string save_path;
		if (!fs.IsFeatureSupported(Filesystem::Feature::Write)) {
			// Is an archive and needs a redirected save path
			// Get archive name
			save_path = jstring_to_string(env, jmain_dir_name);

			// Compatibility with original GameScanner Java code
			// Everything after the extension is removed
			size_t ext = save_path.find_last_of('.');
			if (ext != std::string::npos) {
				save_path = save_path.substr(0, ext);
			}

			// Append subdirectory when the archive contains more than one game
			if (ge_list.size() > 1) {
				save_path += FileFinder::GetFullFilesystemPath(fs).substr(root_path.size());
			}

			// Recursion is annoying in SAF so flatten the path
			// SAF already does this replacement but better do not rely on this implementation detail
			save_path = Utils::ReplaceAll(save_path, "/", "_");
		}

		/* Obtaining of the game_dir_name image */

		// 1. When the game_dir_name directory contains only one image: Load it
		// 2. Attempt to fetch it from the database
		// 3. If this fails grab the first from the game_dir_name folder
		jbyteArray title_image = nullptr;

		auto load_image = [&](Filesystem_Stream::InputStream& stream) {
			if (!stream) {
				return;
			}

			if (stream.GetName().ends_with(".xyz")) {
				title_image = readXyz(env, stream);
			} else if (stream.GetName().ends_with(".png") || stream.GetName().ends_with(".bmp")) {
				auto vec = Utils::ReadStream(stream);
				title_image = env->NewByteArray(vec.size());
				env->SetByteArrayRegion(title_image, 0, vec.size(), reinterpret_cast<jbyte *>(vec.data()));
			}
		};

		// 1. When the game_dir_name directory contains only one image: Load it
		auto title_fs = fs.Subtree("Title");
		if (title_fs) {
			auto& content = *title_fs.ListDirectory();
			if (content.size() == 1 && content[0].second.type == DirectoryTree::FileType::Regular) {
				auto is = title_fs.OpenInputStream(content[0].second.name);
				if (!is) {
					// When opening of the image fails it is in an unsupported archive format
					// Skip this game
					continue;
				}
				load_image(is);
			}
		}

		// 2. Attempt to fetch it from the database
		if (!title_image) {
			std::string db_file = fs.FindFile("RPG_RT.ldb");
			if (!db_file.empty()) {
				// This can fail when the database file is renamed, is not an error condition
				auto is = fs.OpenInputStream(db_file);
				if (!is) {
					// When opening of the db fails it is in an unsupported archive format
					// Skip this game
					continue;
				} else {
					auto db = lcf::LDB_Reader::Load(is);
					if (!db) {
						// Database corrupted? Skip
						continue;
					}

					if (!db->system.title_name.empty()) {
						auto encodings = lcf::ReaderUtil::DetectEncodings(*db);
						for (auto &enc: encodings) {
							if (lcf::Encoder encoder(enc); encoder.IsOk()) {
								std::string title_name = lcf::ToString(db->system.title_name);
								encoder.Encode(title_name);
								auto title_is = fs.OpenFile("Title", title_name, FileFinder::IMG_TYPES);
								// Title image was found -> Load it
								load_image(title_is);
							}
						}
					}
				}
			}
		}

		// 3. Simply grab the first from the game_dir_name folder
		if (!title_image) {
			// No image loaded yet: Grab the first from the game_dir_name folder
			if (title_fs) {
				for (auto &[name, entry]: *title_fs.ListDirectory()) {
					if (entry.type == DirectoryTree::FileType::Regular) {
						auto is = title_fs.OpenInputStream(entry.name);
						load_image(is);
						if (title_image) {
							break;
						}
					}
				}
			}
		}

		/* Setting the game title */
		// By default it is just the name of the directory
		std::string title = game_dir_name;
		bool title_from_ini = false;

		// Try to grab a title from the INI file
		if (auto ini_is = fs.OpenFile("RPG_RT.ini"); ini_is) {
			if (lcf::INIReader ini(ini_is); !ini.ParseError()) {
				if (std::string ini_title = ini.GetString("RPG_RT", "GameTitle", ""); !ini_title.empty()) {
					title = ini_title;
					title_from_ini = true;
				}
			}
		}

		/* Create an instance of "Game" */
		jstring jgame_path = env->NewStringUTF(("content://" + full_path).c_str());
		jstring jsave_path = env->NewStringUTF(save_path.c_str());
		jobject jgame_object = env->NewObject(jgame_class, jgame_constructor_supported, jgame_path, jsave_path, title_image, (int)ge.type);

		if (title_from_ini) {
			// Store the raw string in the Game instance so it can be reencoded later via user setting
			jbyteArray jtitle_raw = env->NewByteArray(title.size());
			env->SetByteArrayRegion(jtitle_raw, 0, title.size(), reinterpret_cast<jbyte*>(title.data()));
			jfieldID jtitle_raw_field = env->GetFieldID(jgame_class, "titleRaw", "[B");
			env->SetObjectField(jgame_object, jtitle_raw_field, jtitle_raw);
			Java_org_easyrpg_player_game_1browser_Game_reencodeTitle(env, jgame_object);
		} else {
			// Use the folder name as the title
			jstring jtitle = env->NewStringUTF(title.c_str());
			jmethodID jset_title_method = env->GetMethodID(jgame_class, "setTitle", "(Ljava/lang/String;)V");
			env->CallVoidMethod(jgame_object, jset_title_method, jtitle);
		}

		// Set folder name
		jstring jfolder = env->NewStringUTF(game_dir_name.c_str());
		jmethodID jset_folder_name_method = env->GetMethodID(jgame_class, "setGameFolderName", "(Ljava/lang/String;)V");
		env->CallVoidMethod(jgame_object, jset_folder_name_method, jfolder);

		env->SetObjectArrayElement(jgame_array, i, jgame_object);
	}

	// Some fields of the Array can be NULL when a game was skipped due to an error
	// This is sanitized on the Java site
	return jgame_array;
}

extern "C"
JNIEXPORT void JNICALL
Java_org_easyrpg_player_game_1browser_Game_reencodeTitle(JNIEnv *env, jobject thiz) {
	jclass jgame_class = env->GetObjectClass(thiz);

	// Fetch the raw title string (result will be at the end in "title" variable)
	jfieldID jtitle_raw_field = env->GetFieldID(jgame_class, "titleRaw", "[B");
	jbyteArray jtitle_raw = reinterpret_cast<jbyteArray>(env->GetObjectField(thiz, jtitle_raw_field));

	if (!jtitle_raw) {
		return;
	}

	jbyte* title_data = env->GetByteArrayElements(jtitle_raw, NULL);
	jsize title_len = env->GetArrayLength(jtitle_raw);
	std::string title(reinterpret_cast<char*>(title_data), title_len);
	env->ReleaseByteArrayElements(jtitle_raw, title_data, 0);

	// Obtain the encoding
	jmethodID jget_encoding_method = env->GetMethodID(jgame_class, "getEncodingCode", "()Ljava/lang/String;");
	jstring jencoding = (jstring)env->CallObjectMethod(thiz, jget_encoding_method);
	std::string encoding = jstring_to_string(env, jencoding);
	if (encoding == "auto") {
		auto det_encodings = lcf::ReaderUtil::DetectEncodings(title);
		for (auto &det_enc: det_encodings) {
			if (lcf::Encoder encoder(det_enc); encoder.IsOk()) {
				encoder.Encode(title);
				break;
			}
		}
	} else {
		lcf::Encoder enc(encoding);
		enc.Encode(title);
	}

	if (title.empty()) {
		// Something failed, do not set a new title
		return;
	}

	// Set the new title after reencoding
	jstring jtitle = env->NewStringUTF(title.c_str());
	jmethodID jset_title_method = env->GetMethodID(jgame_class, "setTitle", "(Ljava/lang/String;)V");
	env->CallVoidMethod(thiz, jset_title_method, jtitle);
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_org_easyrpg_player_settings_SettingsFontActivity_DrawText(JNIEnv *env, jclass, jstring jfont, jint jsize, jboolean jfirst_font) {
	EpAndroid::env = env;

	std::string font = jstring_to_string(env, jfont);

	FontRef font_file;
	FontRef def_bitmap = Font::DefaultBitmapFont(!jfirst_font);

	if (font.empty()) {
		// Option "Built-in Font" selected
		font_file = def_bitmap;
	} else {
		auto is = FileFinder::Root().OpenInputStream(font);
		if (!is) {
			return nullptr;
		}

		font_file = Font::CreateFtFont(std::move(is), jsize, false, false);
		if (!font_file) {
			return nullptr;
		}

		font_file->SetFallbackFont(def_bitmap);
	}

	int width = MESSAGE_BOX_WIDTH - 16;
	int height = 16 * 6;

	jbyteArray buffer_array = env->NewByteArray(width * height * 4);
	jbyte* buffer_raw = env->GetByteArrayElements(buffer_array, 0);

	Bitmap::SetFormat(Bitmap::ChooseFormat(format_R8G8B8A8_a().format()));
	auto sys = Cache::System(CACHE_DEFAULT_BITMAP);
	BitmapRef draw_area = Bitmap::Create(reinterpret_cast<void*>(buffer_raw), width, height, 0, format_R8G8B8A8_a().format());
	draw_area->Fill(Color(0, 0, 0, 255));

	Text::Draw(*draw_area, 0, 16 * 0 + 2, *font_file, *sys, Font::ColorDefault, "TheQuickBrownFoxJumpsOverTheLazyDog.!?1234567890=&#%");
	Text::Draw(*draw_area, 0, 16 * 1 + 2, *font_file, *sys, Font::ColorDefault, "色は匂えど散りぬるを我が世誰ぞ常ならん有為の奥山今#日越えて浅き夢見じ酔いもせず");
	Text::Draw(*draw_area, 0, 16 * 2 + 2, *font_file, *sys, Font::ColorDefault, "天地玄黃宇宙洪荒日月盈昃辰宿列張寒來暑往秋收冬藏閏#餘成歲律呂調陽");
	Text::Draw(*draw_area, 0, 16 * 3 + 2, *font_file, *sys, Font::ColorDefault, "키스의고유조건은입술끼리만나야하고특별한기술은필요#치않다");
	Text::Draw(*draw_area, 0, 16 * 4 + 2, *font_file, *sys, Font::ColorDefault, "(+)[-]{*}</>ÀÁÂÃÄÅÆÇАБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬ#ЭЮЯ");
	Text::Draw(*draw_area, 0, 16 * 5 + 2, *font_file, *sys, Font::ColorDefault, "ÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõöøùúû#üýþÿ");

	return buffer_array;
}

extern "C"
JNIEXPORT void JNICALL
Java_org_easyrpg_player_settings_SettingsGamesFolderActivity_DetectRtp(JNIEnv *env, jobject, jstring jpath, jobject hit_info, int version) {
	EpAndroid::env = env;


	std::string path = jstring_to_string(env, jpath);

	auto fs = FileFinder::Root().Create(path);
	if (!fs) {
		return;
	}

	fs.ClearCache();

	auto hits = RTP::Detect(fs, version);

	if (hits.empty()) {
		return;
	}

	// Find RTP with highest hit rate
	double best_rate = 0;
	int best_index = 0;

	for (size_t i = 0; i < hits.size(); ++i) {
		double rate = static_cast<double>(hits[i].hits) / hits[i].max;
		if (rate > best_rate) {
			best_index = i;
			best_rate = rate;
		}
	}

	RTP::RtpHitInfo& best_hit = hits[best_index];

	jclass hit_info_cls = env->GetObjectClass(hit_info);
	jfieldID name_field = env->GetFieldID(hit_info_cls, "name", "Ljava/lang/String;");
	jfieldID version_field = env->GetFieldID(hit_info_cls, "version", "I");
	jfieldID hits_field = env->GetFieldID(hit_info_cls, "hits", "I");
	jfieldID max_field = env->GetFieldID(hit_info_cls, "max", "I");

	jstring jname = env->NewStringUTF(best_hit.name.c_str());
	env->SetObjectField(hit_info, name_field, jname);

	env->SetIntField(hit_info, version_field, best_hit.version);
	env->SetIntField(hit_info, hits_field, best_hit.hits);
	env->SetIntField(hit_info, max_field, best_hit.max);
}
