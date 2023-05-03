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

#if defined(__ANDROID__) && !defined(USE_LIBRETRO)
#  include "platform/android/filesystem_apk.h"
#  include "platform/android/filesystem_saf.h"
#endif

constexpr const StringView root_ns = "root://";

RootFilesystem::RootFilesystem() : Filesystem("", FilesystemView()) {
	// Add platform specific namespaces here
#if defined(__ANDROID__) && !defined(USE_LIBRETRO)
	fs_list.push_back(std::make_pair("apk", std::make_unique<ApkFilesystem>()));
	fs_list.push_back(std::make_pair("content", std::make_unique<SafFilesystem>("", FilesystemView())));
#endif

	// IMPORTANT: This must be the last filesystem in the list, do not push anything to fs_list afterwards!
	fs_list.push_back(std::make_pair("file", std::make_unique<NativeFilesystem>("", FilesystemView())));

	assert(fs_list.back().first == "file" && "File namespace must be last!");
}

FilesystemView RootFilesystem::Create(StringView path) const {
	if (path.starts_with(root_ns)) {
		// Debug feature: root:// is a pseudo namespace
		// Shows a list of all namespaces in the Game Browser
		if (path.size() > root_ns.size()) {
			Output::Error("root:// does not support any path suffix");
		}
		return Subtree("");
	}

	const auto& fs = FilesystemForPath(path);
	// Strip namespace from path
	auto ns_pos = path.find("://");
	if (ns_pos != std::string::npos) {
		path = path.substr(ns_pos + 3);
	}
	return fs.Create(path);
}

bool RootFilesystem::IsFile(StringView path) const {
	return FilesystemForPath(path).IsFile(path);
}

bool RootFilesystem::IsDirectory(StringView path, bool follow_symlinks) const {
	return FilesystemForPath(path).IsDirectory(path, follow_symlinks);
}

bool RootFilesystem::Exists(StringView path) const {
	return FilesystemForPath(path).Exists(path);
}

int64_t RootFilesystem::GetFilesize(StringView  path) const {
	return FilesystemForPath(path).GetFilesize(path);
}

std::streambuf* RootFilesystem::CreateInputStreambuffer(StringView path, std::ios_base::openmode mode) const {
	return FilesystemForPath(path).CreateInputStreambuffer(path, mode);
}

std::streambuf* RootFilesystem::CreateOutputStreambuffer(StringView path, std::ios_base::openmode mode) const {
	return FilesystemForPath(path).CreateOutputStreambuffer(path, mode);

}

bool RootFilesystem::GetDirectoryContent(StringView path, std::vector<DirectoryTree::Entry>& tree) const {
	if (path.empty()) {
		// Debug feature: Return all available namespaces as a directory list
		for (const auto& p : fs_list) {
			tree.emplace_back(p.first + "://", DirectoryTree::FileType::Directory);
		}
		return true;
	}

	return FilesystemForPath(path).GetDirectoryContent(path, tree);
}

bool RootFilesystem::MakeDirectory(StringView path, bool follow_symlinks) const {
	return FilesystemForPath(path).MakeDirectory(path, follow_symlinks);
}

std::string RootFilesystem::Describe() const {
	return "[Root]";
}

const Filesystem& RootFilesystem::FilesystemForPath(StringView path) const {
	assert(!fs_list.empty());

	StringView ns;
	// Check if the path contains a namespace
	auto ns_pos = path.find("://");
	if (ns_pos != std::string::npos) {
		ns = path.substr(0, ns_pos);
		path = path.substr(ns_pos + 3);
	}

	if (ns.empty()) {
		// No namespace returns the last fs which is the NativeFilesystem
		return *fs_list.back().second;
	}

	auto it = std::find_if(fs_list.begin(), fs_list.end(), [&ns] (const auto& p) {
		return p.first == ns;
	});

	if (it == fs_list.end()) {
		// Only possible to trigger via commandline or bogus code, always user/dev error -> abort
		Output::Error("Unsupported namespace {}://{}", ns, path);
	}

	return *it->second;
}
