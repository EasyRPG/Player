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

#include "filesystem_native.h"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include <fmt/core.h>

#include "system.h"
#include "output.h"
#include "platform.h"

NativeFilesystem::NativeFilesystem(std::string base_path, FilesystemView parent_fs) : Filesystem(std::move(base_path), parent_fs) {
}

bool NativeFilesystem::IsFile(StringView path) const {
	return Platform::File(ToString(path)).IsFile(false);
}

bool NativeFilesystem::IsDirectory(StringView dir, bool follow_symlinks) const {
	return Platform::File(ToString(dir)).IsDirectory(follow_symlinks);
}

bool NativeFilesystem::Exists(StringView filename) const {
	return Platform::File(ToString(filename)).Exists();
}

int64_t NativeFilesystem::GetFilesize(StringView path) const {
	return Platform::File(ToString(path)).GetSize();
}

std::streambuf* NativeFilesystem::CreateInputStreambuffer(StringView path, std::ios_base::openmode mode) const {
	auto* buf = new std::filebuf();
	buf->open(
#ifdef _MSC_VER
		Utils::ToWideString(path),
#else
		ToString(path),
#endif
		mode);

	if (!buf->is_open()) {
		delete buf;
		return nullptr;
	}

	return buf;
}

std::streambuf* NativeFilesystem::CreateOutputStreambuffer(StringView path, std::ios_base::openmode mode) const {
	auto* buf = new std::filebuf();
	buf->open(
#ifdef _MSC_VER
		Utils::ToWideString(path),
#else
		ToString(path),
#endif
		mode);

	if (!buf->is_open()) {
		delete buf;
		return nullptr;
	}

	return buf;
}

bool NativeFilesystem::GetDirectoryContent(StringView path, std::vector<DirectoryTree::Entry>& entries) const {
	std::string p = ToString(path);

	Platform::Directory dir(p);
	if (!dir) {
		Output::Debug("Error opening dir {}: {}", p, ::strerror(errno));
		return false;
	}

	while (dir.Read()) {
		const auto& name = dir.GetEntryName();
		Platform::FileType type = dir.GetEntryType();

		if (name == "." || name == "..") {
			continue;
		}

		bool is_directory = false;
		if (type == Platform::FileType::Directory) {
			is_directory = true;
		} else if (type == Platform::FileType::Unknown) {
			is_directory = IsDirectory(name, true);
		}

		entries.emplace_back(
			name,
			is_directory ? DirectoryTree::FileType::Directory : DirectoryTree::FileType::Regular);
	}

	return true;
}

bool NativeFilesystem::MakeDirectory(StringView path, bool follow_symlinks) const {
	return Platform::File(ToString(path)).MakeDirectory(follow_symlinks);
}

bool NativeFilesystem::IsFeatureSupported(Feature f) const {
	return f == Filesystem::Feature::Write;
}

std::string NativeFilesystem::Describe() const {
	return fmt::format("[Native] {}", GetPath());
}
