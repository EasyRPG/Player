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
#include "filesystem_zip.h"
#include "filefinder.h"
#include "utils.h"
#include "output.h"
#include "player.h"
#include <lcf/reader_util.h>
#include <algorithm>
#include <cassert>
#include <utility>

Filesystem::Filesystem(std::string base_path) : base_path(std::move(base_path)) {
	tree = DirectoryTree::Create(*this);
};

Filesystem_Stream::InputStream Filesystem::OpenInputStream(StringView name, std::ios_base::openmode m) const {
	std::streambuf* buf = CreateInputStreambuffer(name, m | std::ios_base::in);
	Filesystem_Stream::InputStream is(buf);
	return is;
}

Filesystem_Stream::OutputStream Filesystem::OpenOutputStream(StringView name, std::ios_base::openmode m) const {
	std::streambuf* buf = CreateOutputStreambuffer(name, m | std::ios_base::out);
	Filesystem_Stream::OutputStream os(buf);
	return os;
}

void Filesystem::ClearCache() {
	// TODO
}

FilesystemView Filesystem::Create(StringView path) const {
	if (!(Exists(path) || !IsDirectory(path, true))) {
		return FilesystemView();
	}

	// Handle as a normal path in the current filesystem
	return Subtree(ToString(path));
}

FilesystemView Filesystem::Subtree(std::string sub_path) const {
	return FilesystemView(this, tree->MakePath(sub_path));
}

bool Filesystem::IsValid() const {
	// FIXME: better way to do this?
	return Exists("");
}

std::string Filesystem::FindFile(StringView filename, Span<StringView> exts) const {
	return tree->FindFile(filename, exts);
}

std::string Filesystem::FindFile(StringView directory, StringView filename, Span<StringView> exts) const {
	return tree->FindFile(directory, filename, exts);
}

std::string Filesystem::FindFile(const DirectoryTree::Args& args) const {
	return tree->FindFile(args);
}

FilesystemView::FilesystemView(const Filesystem* fs, std::string sub_path) :
	fs(fs), sub_path(std::move(sub_path)) {
	valid = (fs->ListDirectory(this->sub_path) != nullptr);
}

std::string FilesystemView::FindFile(StringView name, Span<StringView> exts) const {
	assert(fs);
	return fs->FindFile(MakePath(name), exts);
}

std::string FilesystemView::FindFile(StringView dir, StringView name, Span<StringView> exts) const {
	assert(fs);
	return fs->tree->FindFile(MakePath(dir), name, exts);
}

std::string FilesystemView::FindFile(const DirectoryTree::Args& args) const {
	auto args_cp = args;
	std::string path = MakePath(args.path);
	args_cp.path = path;
	return fs->FindFile(args_cp);
}

std::string FilesystemView::MakePath(StringView subdir) const {
	assert(fs);
	return FileFinder::MakePath(sub_path, subdir);
}

DirectoryTree::DirectoryListType* FilesystemView::ListDirectory(StringView path) const {
	return fs->ListDirectory(fs->tree->MakePath(path));
}

Filesystem_Stream::InputStream FilesystemView::OpenInputStream(StringView name, std::ios_base::openmode m) const {
	assert(fs);
	return fs->OpenInputStream(name, m);
}

Filesystem_Stream::OutputStream FilesystemView::OpenOutputStream(StringView name, std::ios_base::openmode m) const {
	assert(fs);
	return fs->OpenOutputStream(name, m);
}

FilesystemView FilesystemView::Create(StringView p) const {
	return fs->Create(MakePath(p));
}

FilesystemView FilesystemView::Subtree(StringView sub_path) const {
	return FilesystemView(fs, MakePath(sub_path));
}
