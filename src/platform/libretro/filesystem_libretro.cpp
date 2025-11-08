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

#include "filesystem_libretro.h"
#include "filesystem_stream.h"
#include "output.h"

struct retro_vfs_interface_info LibretroFilesystem::vfs;

LibretroFilesystem::LibretroFilesystem(std::string base_path, FilesystemView parent_fs) : Filesystem(std::move(base_path), parent_fs) {
}

bool LibretroFilesystem::IsFile(std::string_view path) const {
	int flags = vfs.iface->stat(ToString(path).c_str(), nullptr);
	return flags & RETRO_VFS_STAT_IS_VALID && !(flags & RETRO_VFS_STAT_IS_DIRECTORY);
}

bool LibretroFilesystem::IsDirectory(std::string_view dir, bool) const {
	int flags = vfs.iface->stat(ToString(dir).c_str(), nullptr);
	return flags & RETRO_VFS_STAT_IS_VALID && flags & RETRO_VFS_STAT_IS_DIRECTORY;
}

bool LibretroFilesystem::Exists(std::string_view filename) const {
	int flags = vfs.iface->stat(ToString(filename).c_str(), nullptr);
	return flags & RETRO_VFS_STAT_IS_VALID;
}

int64_t LibretroFilesystem::GetFilesize(std::string_view path) const {
	int32_t size;
	int flags = vfs.iface->stat(ToString(path).c_str(), &flags);
	return flags & RETRO_VFS_STAT_IS_VALID ? size : -1;
}

class LibretroStreamBufIn : public std::streambuf {
public:
	LibretroStreamBufIn(struct retro_vfs_file_handle* handle) : std::streambuf(), handle(handle) {
		setg(buffer_start, buffer_start, buffer_start);
	}

	~LibretroStreamBufIn() override {
		LibretroFilesystem::vfs.iface->close(handle);
	}

	int underflow() override {
		ssize_t res = LibretroFilesystem::vfs.iface->read(handle, buffer.data(), buffer.size());
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
		int cdir = Filesystem_Stream::CppSeekdirToCSeekdir(dir);
		auto res = LibretroFilesystem::vfs.iface->seek(handle, offset, cdir == SEEK_CUR ? RETRO_VFS_SEEK_POSITION_CURRENT : cdir == SEEK_END ? RETRO_VFS_SEEK_POSITION_END : RETRO_VFS_SEEK_POSITION_START);
		setg(buffer_start, buffer_end, buffer_end);
		return res;
	}

	std::streambuf::pos_type seekpos(std::streambuf::pos_type pos, std::ios_base::openmode mode) override {
		return seekoff(pos, std::ios_base::beg, mode);
	}

private:
	struct retro_vfs_file_handle* handle;
	std::array<char, 4096> buffer;
	char* buffer_start = &buffer.front();
	char* buffer_end = &buffer.back();
};

std::streambuf* LibretroFilesystem::CreateInputStreambuffer(std::string_view path, std::ios_base::openmode) const {
	struct retro_vfs_file_handle* handle = vfs.iface->open(ToString(path).c_str(), RETRO_VFS_FILE_ACCESS_READ, RETRO_VFS_FILE_ACCESS_HINT_NONE);
	return handle == nullptr ? nullptr : new LibretroStreamBufIn(handle);
}

class LibretroStreamBufOut : public std::streambuf {
public:
	LibretroStreamBufOut(struct retro_vfs_file_handle* handle) : std::streambuf(), handle(handle) {
		setp(buffer_start, buffer_end);
	}

	~LibretroStreamBufOut() override {
		sync();
		LibretroFilesystem::vfs.iface->close(handle);
	}

	int overflow(int c = EOF) override {
		if (sync() < 0) {
			return traits_type::eof();
		}
		if (c != EOF) {
			char a = static_cast<char>(c);
			ssize_t res = LibretroFilesystem::vfs.iface->write(handle, &a, 1);
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
		ssize_t res = LibretroFilesystem::vfs.iface->write(handle, pbase(), len);
		setp(buffer_start, buffer_end);
		if (res < len) {
			return -1;
		}
		return 0;
	}

private:
	struct retro_vfs_file_handle* handle;
	std::array<char, 4096> buffer;
	char* buffer_start = &buffer.front();
	char* buffer_end = &buffer.back();
};

std::streambuf* LibretroFilesystem::CreateOutputStreambuffer(std::string_view path, std::ios_base::openmode mode) const {
	if ((mode & std::ios_base::app) == std::ios_base::app && Exists(path)) {
		struct retro_vfs_file_handle* handle = vfs.iface->open(ToString(path).c_str(), RETRO_VFS_FILE_ACCESS_WRITE | RETRO_VFS_FILE_ACCESS_UPDATE_EXISTING, RETRO_VFS_FILE_ACCESS_HINT_NONE);
		if (handle == nullptr) {
			return nullptr;
		}
		if (vfs.iface->seek(handle, 0, RETRO_VFS_SEEK_POSITION_END) == -1) {
			vfs.iface->close(handle);
			return nullptr;
		}
		return new LibretroStreamBufIn(handle);
	} else {
		struct retro_vfs_file_handle* handle = vfs.iface->open(ToString(path).c_str(), RETRO_VFS_FILE_ACCESS_WRITE, RETRO_VFS_FILE_ACCESS_HINT_NONE);
		return handle == nullptr ? nullptr : new LibretroStreamBufIn(handle);
	}
}

// To prevent leaking of the directory handle if an exception is thrown within GetDirectoryContent
class LibretroDirGuard {
public:
	LibretroDirGuard(struct retro_vfs_dir_handle* handle) : handle(handle) {
	}

	~LibretroDirGuard() {
		LibretroFilesystem::vfs.iface->closedir(handle);
	}

private:
	struct retro_vfs_dir_handle* handle;
};

bool LibretroFilesystem::GetDirectoryContent(std::string_view path, std::vector<DirectoryTree::Entry>& entries) const {
	std::string p = ToString(path);

	struct retro_vfs_dir_handle* dir = vfs.iface->opendir(p.c_str(), true);
	if (dir == nullptr) {
		Output::Debug("Error opening dir {}", p);
		return false;
	}
	LibretroDirGuard guard(dir);

	while (vfs.iface->readdir(dir)) {
		const char* name = vfs.iface->dirent_get_name(dir);
		if (name == nullptr) {
			continue;
		}
		bool is_directory = vfs.iface->dirent_is_dir(dir);
		entries.emplace_back(
			name,
			is_directory ? DirectoryTree::FileType::Directory : DirectoryTree::FileType::Regular);
	}

	return true;
}

bool LibretroFilesystem::MakeDirectory(std::string_view path, bool) const {
	return vfs.iface->mkdir(ToString(path).c_str()) >= 0;
}

bool LibretroFilesystem::IsFeatureSupported(Feature f) const {
	return f == Filesystem::Feature::Write;
}

std::string LibretroFilesystem::Describe() const {
	return fmt::format("[libretro] {}", GetPath());
}
