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

#include "filesystem_hook.h"
#include "filesystem_stream.h"
#include "options.h"

#include <cassert>
#include <cstdio>
#include <streambuf>

class CaesarStreamBuf : public std::streambuf {
public:
	CaesarStreamBuf(std::streambuf* parent_sb, int n) : parent_sb(parent_sb), n(n) {
		setg(&buf, &buf + 1, &buf + 1);
	}
	CaesarStreamBuf(CaesarStreamBuf const& other) = delete;
	CaesarStreamBuf const& operator=(CaesarStreamBuf const& other) = delete;
	~CaesarStreamBuf() {
		delete parent_sb;
	}

protected:
	int_type underflow() override {
		auto byte = parent_sb->sbumpc();

		if (byte == traits_type::eof()) {
			return byte;
		}

		buf = traits_type::to_char_type(byte);
		buf -= n;

		setg(&buf, &buf, &buf + 1);

		return byte;
	}

	std::streambuf::pos_type seekoff(std::streambuf::off_type offset, std::ios_base::seekdir dir, std::ios_base::openmode) override {
		return parent_sb->pubseekoff(offset, dir);
	}

	std::streambuf::pos_type seekpos(std::streambuf::pos_type pos, std::ios_base::openmode) override {
		return parent_sb->pubseekpos(pos);
	}

private:
	std::streambuf* parent_sb;

	char buf;
	int n;
};


HookFilesystem::HookFilesystem(FilesystemView parent_fs, Hook hook) : Filesystem("", parent_fs), active_hook(hook) {
	// no-op
}

FilesystemView HookFilesystem::Detect(FilesystemView fs) {
	auto lmt = fs.OpenInputStream(TREEMAP_NAME);
	if (lmt) {
		std::array<char, 11> buf;

		lmt.ReadIntoObj(buf);

		if (!memcmp(buf.data(), "\xbMdgNbqUsff", 11)) {
			auto hook_fs = std::make_shared<HookFilesystem>(fs, Hook::SacredTears);
			return hook_fs->Subtree("");
		}
	}

	return fs;
}

bool HookFilesystem::IsFile(StringView path) const {
	return GetParent().IsFile(path);
}

bool HookFilesystem::IsDirectory(StringView path, bool follow_symlinks) const {
	return GetParent().IsDirectory(path, follow_symlinks);
}

bool HookFilesystem::Exists(StringView path) const {
	return GetParent().Exists(path);
}

int64_t HookFilesystem::GetFilesize(StringView path) const {
	return GetParent().GetFilesize(path);
}

bool HookFilesystem::MakeDirectory(StringView dir, bool follow_symlinks) const {
	return GetParent().MakeDirectory(dir, follow_symlinks);
}

bool HookFilesystem::IsFeatureSupported(Feature f) const {
	return GetParent().IsFeatureSupported(f);
}

std::streambuf* HookFilesystem::CreateInputStreambuffer(StringView path, std::ios_base::openmode mode) const {
	auto parent_sb = GetParent().CreateInputStreambuffer(path, mode);

	if (active_hook == Hook::SacredTears) {
		if (path == TREEMAP_NAME) {
			return new CaesarStreamBuf(parent_sb, 1);
		}
	}

	return parent_sb;
}

std::streambuf* HookFilesystem::CreateOutputStreambuffer(StringView path, std::ios_base::openmode mode) const {
	return GetParent().CreateOutputStreambuffer(path, mode);
}

bool HookFilesystem::GetDirectoryContent(StringView path, std::vector<DirectoryTree::Entry>& tree) const {
	auto dir_tree = GetParent().ListDirectory(path);

	if (!dir_tree) {
		return false;
	}

	for (auto& item: *dir_tree) {
		tree.push_back(item.second);
	}

	return true;
}

std::string HookFilesystem::Describe() const {
	assert(active_hook == Hook::SacredTears);

	return fmt::format("[Hook] ({})", "Sacred Tears");
}
