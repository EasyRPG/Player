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

#include "filesystem_saf.h"
#include "filefinder.h"
#include "output.h"

#include <unistd.h>
#include <SDL_system.h>

static jobject get_jni_handle(const SafFilesystem* fs, StringView path) {
	std::string combined_path = FileFinder::MakePath(fs->GetPath(), path);

	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jobject sdl_activity = (jobject)SDL_AndroidGetActivity();
	jclass cls = env->GetObjectClass(sdl_activity);
	jmethodID jni_getFilesystemForPath = env->GetMethodID(cls, "getHandleForPath", "(Ljava/lang/String;)Lorg/easyrpg/player/player/SafFile;");
	jstring jpath = env->NewStringUTF(combined_path.c_str());
	jobject obj_res = env->CallObjectMethod(sdl_activity, jni_getFilesystemForPath, jpath);

	env->DeleteLocalRef(jpath);
	env->DeleteLocalRef(cls);
	env->DeleteLocalRef(sdl_activity);

	return obj_res;
}

SafFilesystem::SafFilesystem(std::string base_path, FilesystemView parent_fs) : Filesystem(base_path, parent_fs) {
	// no-op
}

bool SafFilesystem::IsFile(StringView path) const {
	auto obj = get_jni_handle(this, path);
	if (!obj) {
		return false;
	}

	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jclass cls = env->GetObjectClass(obj);
	jmethodID jni_method = env->GetMethodID(cls, "isFile", "()Z");
	jboolean res = env->CallBooleanMethod(obj, jni_method);

	env->DeleteLocalRef(obj);
	env->DeleteLocalRef(cls);

	return res > 0;
}

bool SafFilesystem::IsDirectory(StringView dir, bool) const {
	auto obj = get_jni_handle(this, dir);
	if (!obj) {
		return false;
	}

	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jclass cls = env->GetObjectClass(obj);
	jmethodID jni_method = env->GetMethodID(cls, "isDirectory", "()Z");
	jboolean res = env->CallBooleanMethod(obj, jni_method);

	env->DeleteLocalRef(obj);
	env->DeleteLocalRef(cls);

	return res > 0;
}

bool SafFilesystem::Exists(StringView filename) const {
	auto obj = get_jni_handle(this, filename);
	if (!obj) {
		return false;
	}

	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jclass cls = env->GetObjectClass(obj);
	jmethodID jni_method = env->GetMethodID(cls, "exists", "()Z");
	jboolean res = env->CallBooleanMethod(obj, jni_method);

	env->DeleteLocalRef(obj);
	env->DeleteLocalRef(cls);

	return res > 0;
}

int64_t SafFilesystem::GetFilesize(StringView path) const {
	auto obj = get_jni_handle(this, path);
	if (!obj) {
		return -1;
	}

	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jclass cls = env->GetObjectClass(obj);
	jmethodID jni_method = env->GetMethodID(cls, "getFilesize", "()J");
	jlong res = env->CallLongMethod(obj, jni_method);

	env->DeleteLocalRef(obj);
	env->DeleteLocalRef(cls);

	return static_cast<int64_t>(res);
}

class FdStreamBufIn : public std::streambuf {
public:
	FdStreamBufIn(int fd, std::array<char, 4096> buffer, ssize_t bytes_read) : std::streambuf(), fd(fd), buffer(buffer) {
		setg(buffer_start, buffer_start, buffer_start + bytes_read);
	}

	~FdStreamBufIn() override {
		close(fd);
	}

	int underflow() override {
		ssize_t res = read(fd, buffer.data(), buffer.size());
		if (res == 0) {
			return traits_type::eof();
		} else if (res < 0) {
			Output::Debug("underflow failed: {}", strerror(errno));
			return traits_type::eof();
		}
		setg(buffer_start, buffer_start, buffer_start + res);
		return traits_type::to_int_type(*gptr());
	}

	std::streambuf::pos_type seekoff(std::streambuf::off_type offset, std::ios_base::seekdir dir, std::ios_base::openmode mode) override {
		if (dir == std::ios_base::cur) {
			offset += static_cast<std::streambuf::off_type>(gptr() - egptr());
		}
		auto res = lseek(fd, offset, Filesystem_Stream::CppSeekdirToCSeekdir(dir));
		setg(buffer_start, buffer_end, buffer_end);
		return res;
	}

	std::streambuf::pos_type seekpos(std::streambuf::pos_type pos, std::ios_base::openmode mode) override {
		return seekoff(pos, std::ios_base::beg, mode);
	}

private:
	int fd = 0;
	std::array<char, 4096> buffer;
	char* buffer_start = &buffer.front();
	char* buffer_end = &buffer.back();
};

std::streambuf* SafFilesystem::CreateInputStreambuffer(StringView path, std::ios_base::openmode) const {
	auto obj = get_jni_handle(this, path);
	if (!obj) {
		return nullptr;
	}

	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jclass cls = env->GetObjectClass(obj);
	jmethodID jni_method = env->GetMethodID(cls, "createInputFileDescriptor", "()I");
	jint fd = env->CallIntMethod(obj, jni_method);

	env->DeleteLocalRef(obj);
	env->DeleteLocalRef(cls);

	if (fd < 0) {
		return nullptr;
	}

	// When the URI points to an inexistant file SAF will give us a fd to a directory
	// Try reading from the descriptor and if it fails consider it invalid
	// This is faster than querying SAF if the file exists beforehand
	std::array<char, 4096> buffer;
	ssize_t bytes_read = read(fd, buffer.data(), buffer.size());
	if (bytes_read < 0) {
		Output::Debug("read failed: {}", strerror(errno));
		close(fd);
		return nullptr;
	}

	// When successful the buffer is forwarded to avoid a seek-to-beginning
	return new FdStreamBufIn(fd, buffer, bytes_read);
}

class FdStreamBufOut : public std::streambuf {
public:
	FdStreamBufOut(int fd) : std::streambuf(), fd(fd) {
		setp(buffer_start, buffer_end);
	}

	~FdStreamBufOut() override {
		sync();
		close(fd);
	}

	int overflow(int c = EOF) override {
		if (sync() < 0) {
			return traits_type::eof();
		}
		if (c != EOF) {
			char a = static_cast<char>(c);
			ssize_t res = write(fd, &a, 1);
			if (res < 1) {
				return traits_type::eof();
			}
		}

		return c;
	}

	int sync() override {
		auto len = pptr() - pbase();
		if (len == 0) {
			return 0;
		}
		ssize_t res = write(fd, pbase(), len);
		setp(buffer_start, buffer_end);
		if (res < len) {
			return -1;
		}
		return 0;
	}

private:
	int fd = 0;
	std::array<char, 4096> buffer;
	char* buffer_start = &buffer.front();
	char* buffer_end = &buffer.back();
};

std::streambuf* SafFilesystem::CreateOutputStreambuffer(StringView path, std::ios_base::openmode mode) const {
	auto obj = get_jni_handle(this, path);
	if (!obj) {
		return nullptr;
	}

	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jclass cls = env->GetObjectClass(obj);
	jmethodID jni_method = env->GetMethodID(cls, "createOutputFileDescriptor", "(Z)I");
	jboolean append = static_cast<uint8_t>(((mode & std::ios_base::app) == std::ios_base::app) ? 1u : 0u);
	jint fd = env->CallIntMethod(obj, jni_method, append);

	env->DeleteLocalRef(obj);
	env->DeleteLocalRef(cls);

	if (fd < 0) {
		return nullptr;
	}

	return new FdStreamBufOut(fd);
}

bool SafFilesystem::GetDirectoryContent(StringView path, std::vector<DirectoryTree::Entry>& entries) const {
	auto obj = get_jni_handle(this, path);
	if (!obj) {
		return false;
	}

	JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
	jclass cls = env->GetObjectClass(obj);
	jmethodID jni_method = env->GetMethodID(cls, "getDirectoryContent", "()Lorg/easyrpg/player/player/DirectoryTree;");
	jobject directory_tree = env->CallObjectMethod(obj, jni_method);

	env->DeleteLocalRef(obj);
	env->DeleteLocalRef(cls);

	if (!directory_tree) {
		return false;
	}

	jclass cls_directory_tree = env->GetObjectClass(directory_tree);
	jfieldID names_field = env->GetFieldID(cls_directory_tree, "names", "[Ljava/lang/String;");
	jobjectArray names_arr = reinterpret_cast<jobjectArray>(env->GetObjectField(directory_tree, names_field));
	int length = env->GetArrayLength(names_arr);

	jfieldID types_field = env->GetFieldID(cls_directory_tree, "types", "[Z");
	std::vector<jboolean> types(static_cast<size_t>(length));
	jbooleanArray types_arr = reinterpret_cast<jbooleanArray>(env->GetObjectField(directory_tree, types_field));
	env->GetBooleanArrayRegion(types_arr, 0, length, types.data());

	for (size_t i = 0; i < static_cast<size_t>(length); ++i) {
		jstring elem = reinterpret_cast<jstring>(env->GetObjectArrayElement(names_arr, static_cast<jsize>(i)));
		const char* str = env->GetStringUTFChars(elem, nullptr);
		entries.emplace_back(str, types[i] == 0 ? DirectoryTree::FileType::Regular : DirectoryTree::FileType::Directory);
		env->ReleaseStringUTFChars(elem, str);
		env->DeleteLocalRef(elem);
	}

	env->DeleteLocalRef(cls_directory_tree);
	env->DeleteLocalRef(names_arr);
	env->DeleteLocalRef(types_arr);

	return true;
}

bool SafFilesystem::IsFeatureSupported(Filesystem::Feature f) const {
	return f == Filesystem::Feature::Write;
}

std::string SafFilesystem::Describe() const {
	return fmt::format("[SAF] {}", GetPath());
}
