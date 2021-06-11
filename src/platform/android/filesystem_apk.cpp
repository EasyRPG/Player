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

#include "filesystem_apk.h"
#include "filefinder.h"
#include "output.h"

#include <jni.h>
#include <SDL_system.h>

ApkFilesystem::ApkFilesystem() : Filesystem("", FilesystemView()) {
	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject sdl_activity = (jobject)SDL_AndroidGetActivity();
	jclass cls = env->GetObjectClass(sdl_activity);
	jmethodID jni_getAssetManager = env->GetMethodID(cls, "getAssetManager", "()Landroid/content/res/AssetManager;");
	jobject asset_manager = (jobject)env->CallObjectMethod(sdl_activity, jni_getAssetManager);
	mgr = AAssetManager_fromJava(env, asset_manager);
}

bool ApkFilesystem::IsFile(StringView path) const {
	AAsset* asset = AAssetManager_open(mgr, ToString(path).c_str(), AASSET_MODE_STREAMING);
	if (!asset) {
		return false;
	}
	AAsset_close(asset);
	return true;
}

bool ApkFilesystem::IsDirectory(StringView dir, bool) const {
	// The openDir has no way to indicate failure
	// This will also report false for empty directories...
	AAssetDir* asset = AAssetManager_openDir(mgr, ToString(dir).c_str());
	if (!asset) {
		return false;
	}
	const char* name = AAssetDir_getNextFileName(asset);
	AAssetDir_close(asset);
	return name != nullptr;
}

bool ApkFilesystem::Exists(StringView filename) const {
	return IsFile(filename) || IsDirectory(filename, false);
}

int64_t ApkFilesystem::GetFilesize(StringView path) const {
	AAsset* asset = AAssetManager_open(mgr, ToString(path).c_str(), AASSET_MODE_STREAMING);
	if (!asset) {
		return -1;
	}
	auto len = AAsset_getLength(asset);
	AAsset_close(asset);
	return len;
}

class AassetStreamBuf : public Filesystem_Stream::InputMemoryStreamBufView {
public:
	AassetStreamBuf(AAsset* asset, Span<uint8_t> buffer) :
		Filesystem_Stream::InputMemoryStreamBufView(buffer), asset(asset) {
	}

	~AassetStreamBuf() override {
		if (asset) {
			AAsset_close(asset);
		}
	}

private:
	AAsset* asset;
};

std::streambuf* ApkFilesystem::CreateInputStreambuffer(StringView path, std::ios_base::openmode mode) const {
	AAsset* asset = AAssetManager_open(mgr, ToString(path).c_str(), AASSET_MODE_STREAMING);
	if (!asset) {
		return nullptr;
	}

	const auto* cbuffer = reinterpret_cast<const uint8_t*>(AAsset_getBuffer(asset));
	if (!cbuffer) {
		AAsset_close(asset);
		return nullptr;
	}
	auto* buffer = const_cast<uint8_t*>(cbuffer);
	auto len = AAsset_getLength(asset);

	return new AassetStreamBuf(asset, Span<uint8_t>(buffer, len));
}

std::streambuf* ApkFilesystem::CreateOutputStreambuffer(StringView, std::ios_base::openmode) const {
	return nullptr;
}

bool ApkFilesystem::GetDirectoryContent(StringView path, std::vector<DirectoryTree::Entry>& entries) const {
	// WARNING: API Limitation
	// According to Android source code some implementations will skip directories here!
	// The function is good enough to boot a zip in standalone mode.

	std::string p = ToString(path);

	AAssetDir* dir = AAssetManager_openDir(mgr, p.c_str());
	if (!dir) {
		Output::Debug("Error opening dir {}", p);
		return false;
	}

	for (const char* n = AAssetDir_getNextFileName(dir); n != nullptr; n = AAssetDir_getNextFileName(dir)) {
		std::string name = n;

		if (name == "." || name == "..") {
			continue;
		}

		DirectoryTree::FileType type = DirectoryTree::FileType::Directory;
		AAsset* asset = AAssetManager_open(mgr, FileFinder::MakePath(path, name).c_str(), AASSET_MODE_STREAMING);
		if (asset) {
			type = DirectoryTree::FileType::Regular;
			AAsset_close(asset);
		}

		entries.emplace_back(name, type);
	}

	return true;
}

std::string ApkFilesystem::Describe() const {
	return "[APK]";
}
