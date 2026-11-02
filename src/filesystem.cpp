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

#include "filesystem.h"
#include "filesystem_native.h"
#include "filesystem_lzh.h"
#include "filesystem_zip.h"
#include "filesystem_tar.h"
#include "filesystem_stream.h"
#include "filefinder.h"
#include "utils.h"
#include "output.h"
#include "player.h"
#include <lcf/reader_util.h>
#include <algorithm>
#include <cassert>
#include <utility>

Filesystem::Filesystem(std::string base_path, FilesystemView parent_fs) : base_path(std::move(base_path)) {
	this->parent_fs = std::make_unique<FilesystemView>(parent_fs);
	tree = DirectoryTree::Create(*this);
};

Filesystem_Stream::InputStream Filesystem::OpenInputStream(std::string_view name, std::ios_base::openmode m) const {
	if (name.empty()) {
		return Filesystem_Stream::InputStream();
	}

	std::streambuf* buf = CreateInputStreambuffer(name, m | std::ios_base::in);

	if (!buf) {
		return Filesystem_Stream::InputStream();
	}

	Filesystem_Stream::InputStream is(buf, ToString(name));
	return is;
}

Filesystem_Stream::InputStream Filesystem::OpenOrCreateInputStream(std::string_view name, std::ios_base::openmode m) const {
	auto is = OpenInputStream(name, m);

	if (!is) {
		auto os = OpenOutputStream(name, (std::ios_base::openmode)0);
		if (!os) {
			return Filesystem_Stream::InputStream();
		}
	}

	return OpenInputStream(name, m);
}

Filesystem_Stream::OutputStream Filesystem::OpenOutputStream(std::string_view name, std::ios_base::openmode m) const {
	if (name.empty()) {
		return Filesystem_Stream::OutputStream();
	}

	std::streambuf* buf = CreateOutputStreambuffer(name, m | std::ios_base::out);

	if (!buf) {
		return Filesystem_Stream::OutputStream();
	}

	std::string path;
	std::tie(path, std::ignore) = FileFinder::GetPathAndFilename(name);
	Filesystem_Stream::OutputStream os(buf, Subtree(path), ToString(name));

	return os;
}

void Filesystem::ClearCache(std::string_view path) const {
	tree->ClearCache(path);
}

FilesystemView Filesystem::Create(std::string_view path) const {
	// Determine the proper file system to use

	// When the path doesn't exist check if the path contains a file that can
	// be handled by another filesystem
	if (!IsDirectory(path, true)) {
		// Search for the deepest directory
		std::vector<std::string> components = FileFinder::SplitPathPrefixes(path);
		// Prepend empty path for the root directory if not present
		if (components.empty() || components.front() != "") {
			components.insert(components.begin(), "");
		}

		size_t archive_idx = components.size();
		for (auto it = components.rbegin(); it != components.rend(); ++it) {
			const std::string& p = *it;
			// Do not check stuff that looks like drives, such as C:, ux0: or sd:
			// Some systems do not consider them directories
			if (archive_idx > 1 || (!p.empty() && p.back() != ':')) {
				if (IsDirectory(p, true)) {
					break;
				}
			} else {
				break;
			}
			--archive_idx;
		}

		// The next component must be a file
		// search for known file extensions and "do magic"
		std::string archive_full_path = components[archive_idx];
		std::string archive_parent_path = components[archive_idx - 1];
		std::string archive_name = FileFinder::GetPathAndFilename(archive_full_path).second;
		std::string internal_path = FileFinder::GetPathInsidePath(archive_full_path, components.back());

		if (!IsFile(archive_full_path) || !FileFinder::IsSupportedArchiveExtension(archive_name)) {
			// No supported archive type found
			return {};
		}

		std::shared_ptr<Filesystem> filesystem = std::make_shared<ZipFilesystem>(archive_name, Subtree(archive_parent_path));
#if HAVE_LHASA
		if (!filesystem->IsValid()) {
			filesystem = std::make_shared<LzhFilesystem>(archive_name, Subtree(archive_parent_path));
		}
#endif
		if (!filesystem->IsValid()) {
			filesystem = std::make_shared<TarFilesystem>(archive_name, Subtree(archive_parent_path));
		}
		if (!filesystem->IsValid()) {
			return {};
		}
		if (!internal_path.empty()) {
			auto fs_view = filesystem->Create(internal_path);
			if (!fs_view) {
				return {};
			}
			return fs_view;
		}
		// This is the root of a new Filesystem
		// Check if it only contains a single folder and if yes enter that folder
		// This way archives with structure "archive/game_folder" launch the game directly
		auto fs_view = filesystem->Subtree("");
		if (!fs_view) {
			return {};
		}
		auto entries = fs_view.ListDirectory("");
		if (entries->size() == 1 && entries->begin()->second.type == DirectoryTree::FileType::Directory) {
			return fs_view.Subtree(entries->begin()->second.name);
		}
		return fs_view;
	} else {
		if (!(Exists(path) || !IsDirectory(path, true))) {
			return {};
		}

		// Handle as a normal path in the current filesystem
		return Subtree(ToString(path));
	}
}

FilesystemView Filesystem::Subtree(std::string sub_path) const {
	return FilesystemView(shared_from_this(), sub_path);
}

bool Filesystem::MakeDirectory(std::string_view path, bool follow_symlinks) const {
	if (IsDirectory(path, follow_symlinks)) {
		return true;
	}

	auto full_paths = FileFinder::SplitPathPrefixes(path);

	if (full_paths.empty()) {
		return true;
	}

	// Do not check stuff that looks like drives, such as C:, ux0: or sd:
	// Some systems do not consider them directories
	if (full_paths[0].back() == ':') {
		full_paths.erase(full_paths.begin());
	}

	// Traverse from the longest subpath and find the first one that exists
	// e.g. assuming "/a/b" exists:
	// First it checks "/a/b/c/d" (*), then "/a/b/c" (*), then ends at "/a/b"
	std::vector<std::string_view> to_create;
	for (auto it = full_paths.rbegin(); it != full_paths.rend(); ++it) {
		const std::string& p = *it;

		if (IsDirectory(p, follow_symlinks)) {
			break;
		}

		if (Exists(p)) {
			// Exists but not a directory
			return false;
		}

		to_create.push_back(p);
	}

	// (*) These paths will be created here
	for (auto it = to_create.rbegin(); it != to_create.rend(); ++it) {
		if (!vMakeDirectory(*it, follow_symlinks)) {
			Output::Debug("MakeDirectory {} failed", *it);
			return false;
		}
	}

	return true;
}

bool Filesystem::vMakeDirectory(std::string_view, bool) const {
	return false;
}

bool Filesystem::IsValid() const {
	// FIXME: better way to do this?
	return Exists("");
}

std::string Filesystem::FindFile(std::string_view filename, const Span<const std::string_view> exts) const {
	return tree->FindFile(filename, exts);
}

std::string Filesystem::FindFile(std::string_view directory, std::string_view filename, const Span<const std::string_view> exts) const {
	return tree->FindFile(directory, filename, exts);
}

std::string Filesystem::FindFile(const DirectoryTree::Args& args) const {
	return tree->FindFile(args);
}

Filesystem_Stream::InputStream Filesystem::OpenFile(std::string_view filename, const Span<const std::string_view> exts) const {
	return OpenInputStream(tree->FindFile(filename, exts));
}

Filesystem_Stream::InputStream Filesystem::OpenFile(std::string_view directory, std::string_view filename, const Span<const std::string_view> exts) const {
	return OpenInputStream(tree->FindFile(directory, filename, exts));
}

Filesystem_Stream::InputStream Filesystem::OpenFile(const DirectoryTree::Args& args) const {
	return OpenInputStream(tree->FindFile(args));
}

FilesystemView::FilesystemView(const std::shared_ptr<const Filesystem>& fs, std::string sub_path) :
	fs(fs), sub_path(std::move(sub_path)) {
	valid = (fs->ListDirectory(this->sub_path) != nullptr);
}

std::string FilesystemView::GetBasePath() const {
	assert(fs);
	return fs->GetPath();
}

std::string FilesystemView::GetSubPath() const {
	assert(fs);
	return sub_path;
}

std::string FilesystemView::GetFullPath() const {
	assert(fs);
	return FileFinder::MakePath(GetBasePath(), GetSubPath());
}

const Filesystem& FilesystemView::GetOwner() const {
	assert(fs);
	return *fs;
}

void FilesystemView::ClearCache() const {
	assert(fs);
	fs->ClearCache(GetSubPath());
}

std::string FilesystemView::FindFile(std::string_view name, const Span<const std::string_view> exts) const {
	assert(fs);
	std::string found = fs->FindFile(MakePath(name), exts);
	if (!found.empty() && !sub_path.empty()) {
		assert(StartsWith(found, sub_path));
		// substr calculation must consider if the subpath is / or drive:/
		return found.substr(sub_path.size() + (sub_path.back() == '/' ? 0 : 1));
	}
	return found;
}

std::string FilesystemView::FindFile(std::string_view dir, std::string_view name, const Span<const std::string_view> exts) const {
	assert(fs);
	std::string found = fs->FindFile(MakePath(dir), name, exts);
	if (!found.empty() && !sub_path.empty()) {
		assert(StartsWith(found, sub_path));
		// substr calculation must consider if the subpath is / or drive:/
		return found.substr(sub_path.size() + (sub_path.back() == '/' ? 0 : 1));
	}
	return found;
}

std::string FilesystemView::FindFile(const DirectoryTree::Args& args) const {
	assert(fs);
	auto args_cp = args;
	std::string path = MakePath(args.path);
	args_cp.path = path;
	std::string found = fs->FindFile(args_cp);
	if (!found.empty() && !sub_path.empty()) {
		assert(StartsWith(found, sub_path));
		// substr calculation must consider if the subpath is / or drive:/
		return found.substr(sub_path.size() + (sub_path.back() == '/' ? 0 : 1));
	}
	return found;
}

Filesystem_Stream::InputStream FilesystemView::OpenFile(std::string_view name, const Span<const std::string_view> exts) const {
	assert(fs);
	return fs->OpenFile(MakePath(name), exts);
}

Filesystem_Stream::InputStream FilesystemView::OpenFile(std::string_view dir, std::string_view name, const Span<const std::string_view> exts) const {
	assert(fs);
	return fs->OpenFile(MakePath(dir), name, exts);
}

Filesystem_Stream::InputStream FilesystemView::OpenFile(const DirectoryTree::Args &args) const {
	assert(fs);
	auto args_cp = args;
	std::string path = MakePath(args.path);
	args_cp.path = path;
	return fs->OpenFile(args_cp);
}

std::string FilesystemView::MakePath(std::string_view subdir) const {
	assert(fs);
	return FileFinder::MakePath(sub_path, subdir);
}

bool FilesystemView::IsFile(std::string_view path) const {
	assert(fs);
	return fs->IsFile(MakePath(path));
}

bool FilesystemView::IsDirectory(std::string_view path, bool follow_symlinks) const {
	assert(fs);
	return fs->IsDirectory(MakePath(path), follow_symlinks);
}

bool FilesystemView::Exists(std::string_view path) const {
	assert(fs);
	return fs->Exists(MakePath(path));
}

int64_t FilesystemView::GetFilesize(std::string_view path) const {
	assert(fs);
	return fs->GetFilesize(MakePath(path));
}

DirectoryTree::DirectoryListType* FilesystemView::ListDirectory(std::string_view path) const {
	assert(fs);
	return fs->ListDirectory(MakePath(path));
}

Filesystem_Stream::InputStream FilesystemView::OpenInputStream(std::string_view name, std::ios_base::openmode m) const {
	assert(fs);

	if (name.empty()) {
		return Filesystem_Stream::InputStream();
	}

	return fs->OpenInputStream(MakePath(name), m);
}

Filesystem_Stream::InputStream FilesystemView::OpenOrCreateInputStream(std::string_view name, std::ios_base::openmode m) const {
	assert(fs);

	if (name.empty()) {
		return Filesystem_Stream::InputStream();
	}

	return fs->OpenOrCreateInputStream(MakePath(name), m);
}

Filesystem_Stream::OutputStream FilesystemView::OpenOutputStream(std::string_view name, std::ios_base::openmode m) const {
	assert(fs);

	if (name.empty()) {
		return Filesystem_Stream::OutputStream();
	}

	return fs->OpenOutputStream(MakePath(name), m);
}

std::streambuf* FilesystemView::CreateInputStreambuffer(std::string_view path, std::ios_base::openmode mode) const {
	assert(fs);
	return fs->CreateInputStreambuffer(MakePath(path), mode);
}

std::streambuf* FilesystemView::CreateOutputStreambuffer(std::string_view path, std::ios_base::openmode mode) const {
	assert(fs);
	return fs->CreateOutputStreambuffer(MakePath(path), mode);
}

FilesystemView FilesystemView::Create(std::string_view p) const {
	assert(fs);
	return fs->Create(MakePath(p));
}

bool FilesystemView::MakeDirectory(std::string_view dir, bool follow_symlinks) const {
	assert(fs);
	return fs->MakeDirectory(MakePath(dir), follow_symlinks);
}

bool FilesystemView::IsFeatureSupported(Filesystem::Feature f) const {
	assert(fs);
	return fs->IsFeatureSupported(f);
}

FilesystemView FilesystemView::Subtree(std::string_view sub_path) const {
	assert(fs);
	return FilesystemView(fs, MakePath(sub_path));
}

std::string FilesystemView::Describe() const {
	assert(fs);
	if (GetSubPath().empty()) {
		return fs->Describe();
	} else {
		return fs->Describe() + " -> " + GetSubPath();
	}
}
