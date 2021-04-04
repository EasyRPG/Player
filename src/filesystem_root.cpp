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

#include "filesystem_root.h"
#include "output.h"

//constexpr const StringView apk_ns = "apk://";
constexpr const StringView file_ns = "file://";
constexpr const StringView root_ns = "root://";

RootFilesystem::RootFilesystem() : Filesystem("", FilesystemView()) {
	native_fs.reset(new NativeFilesystem("", this->Subtree("")));
}

FilesystemView RootFilesystem::Create(StringView path) const {
	if (path.starts_with(file_ns)) {
		StringView path_sub = path.substr(file_ns.size());
		if (path_sub.empty()) {
			path_sub = "/";
		}
		return native_fs->Subtree(ToString(path_sub));
	} else if (path.starts_with("root://")) {
		if (path.size() > root_ns.size()) {
			Output::Error("root:// does not support any path suffix");
		}
		return Subtree("");
	} else {
		return native_fs->Create(path);
	}
}

void show_assert() {
	assert(false && "No operations supported on RootFs. Use Create to obtain a spezialized VFS.");
}

bool RootFilesystem::IsFile(StringView) const {
	show_assert();
	return false;
}

bool RootFilesystem::IsDirectory(StringView, bool) const {
	show_assert();
	return false;
}

bool RootFilesystem::Exists(StringView path) const {
	return path.empty();
}

int64_t RootFilesystem::GetFilesize(StringView) const {
	show_assert();
	return -1;
}

std::streambuf* RootFilesystem::CreateInputStreambuffer(StringView, std::ios_base::openmode) const {
	show_assert();
	return nullptr;
}

std::streambuf* RootFilesystem::CreateOutputStreambuffer(StringView, std::ios_base::openmode) const {
	show_assert();
	return nullptr;
}

bool RootFilesystem::GetDirectoryContent(StringView path, std::vector<DirectoryTree::Entry>& tree) const {
	if (!path.empty()) {
		return false;
	}

	tree.emplace_back("file://", DirectoryTree::FileType::Directory);
	//tree.emplace_back("apk://", DirectoryTree::FileType::Directory);
	return true;
}

std::string RootFilesystem::Describe() const {
	return "[Root]";
}
