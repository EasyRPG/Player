#include "output.h"
#include "filesystem.h"
#include "filesystem_stream.h"

#include <fmt/core.h>
#include <string>
#include <variant>
#include <string_view>

#include "filesystem_tar.h"

static std::string cstr_to_string_safe(const char *s, size_t n)
{
    std::string target;
    target.resize(n);
    strncat(target.data(), s, n);
    if (int nul = target.find('\0'); nul != std::string::npos) {
        target.resize(nul);
        target.shrink_to_fit();
    }
    return target;
}

TarFilesystem::Entry::Entry() {}

TarFilesystem::Entry::Entry(std::string dirname) : name(dirname), data(Children()) {}

TarFilesystem::Entry::Entry(long offs, entry_raw from, long *skip) {
    name = cstr_to_string_safe(from.data.ustar.path_prefix, sizeof(from.data.ustar.path_prefix)) + cstr_to_string_safe(from.data.filename, sizeof(from.data.filename));

    std::string sizestr = cstr_to_string_safe(from.data.size, sizeof(from.data.size));
    char *end;
    long filesize = strtol(sizestr.c_str(), &end, 8);

    if (memcmp(from.data.ustar.magic, "ustar ", 6)) {
        Output::Debug("TarFilesystem: invalid magic '{}' (offset {})", cstr_to_string_safe(from.data.ustar.magic, 6), offs);
        invalidate();
        return;
    }

    switch (from.data.type) {
    case '0':
    case '\0':
        if (*end != 0) {
            Output::Debug("TarFilesystem: couldn't parse file size (offset {})", offs);
            invalidate();
            return;
        }
        data = FileEntryInfo{offs, filesize};
        if ((*skip = filesize) % 512 != 0)
            *skip += 512 - *skip % 512;
        break;
    case '5':
        if (filesize != 0 || *end != 0) {
            Output::Debug("TarFilesystem: invalid size for directory (offset {})", offs);
            invalidate();
            return;
        }
        data = Children();
        *skip = 0;
        break;
    default:
        Output::Debug("TarFilesystem: unknown format '{}'", from.data.type);
        invalidate();
    }
}

bool TarFilesystem::Entry::valid() const {
    return !std::get_if<std::monostate>(&data);
}

void TarFilesystem::Entry::invalidate() {
    data = std::monostate();
}

bool TarFilesystem::Entry::add(Entry &&entry) {
    assert(!std::get_if<std::monostate>(&entry.data));

    int slashloc = entry.name.rfind('/');
    auto dirname = entry.name.substr(0, slashloc);
    entry.name = entry.name.substr(slashloc + 1);

    auto parent_entry = locate(dirname, true);
    if (!parent_entry) {
        Output::Debug("TarFilesystem: invalid path to archive entry");
        return false;
    }
    std::get<Children>(parent_entry->data)[entry.name] = std::move(entry);
    return true;
}

TarFilesystem::Entry *TarFilesystem::Entry::locate(std::string path, bool auto_create_dirs) {
    if (path == "")
        return this;

    if (name != "") {
        assert(path.substr(0, path.find('/')) == name);
        path = path.substr(1 + path.find('/'));
    }

    auto *dirdata = std::get_if<Children>(&data);
    if (!dirdata)
        return nullptr;

    auto subdir = path.substr(0, path.find('/'));
    if (dirdata->find(subdir) == dirdata->end() && auto_create_dirs)
        (*dirdata)[subdir] = Entry(subdir);

    auto iter = dirdata->find(subdir);
    if (iter == dirdata->end())
        return nullptr;
    return path.find('/') == std::string::npos ? &iter->second : iter->second.locate(std::move(path), auto_create_dirs);
}

const TarFilesystem::Entry *TarFilesystem::Entry::locate(std::string path) const {
    return const_cast<Entry *>(this)->locate(path, false);
}

TarFilesystem::TarFilesystem(std::string base_path, FilesystemView parent_fs) : Filesystem(base_path, parent_fs), stream(parent_fs.OpenInputStream(GetPath())) {
    if (!stream) {
        Output::Warning("Failed to open {}", base_path);
        return;
    }

    entry_raw data;
    int blank = 0;
    static const char key[32] = { 0 };
    while (stream.ReadIntoObj(data)) {
        if (memcmp(&data, key, sizeof(key)) == 0 && memcmp(&data, sizeof(key) + (const char *)&data, sizeof(data) - sizeof(key)) == 0) {
            if (++blank == 2)
                return;
            continue;
        }
        blank = 0;

        long skip;
        Entry entry { stream.tellg(), data, &skip };
        if (!entry.valid()) {
            rootdir.invalidate();
            return;
        }
        if (skip)
            stream.seekg(skip, std::ios_base::cur);

        if (entry.name == "")
            continue;
        if (!rootdir.add(std::move(entry))) {
            rootdir.invalidate();
            return;
        }
    }
    Output::Warning("TarFIlesystem: tar file has no terminator");
}

bool TarFilesystem::IsFile(std::string_view path) const {
    auto entry = rootdir.locate(std::string(path));
    if (!entry)
        Output::Error("TarFilesystem: no file exists at path {}", path);
    return !!std::get_if<Entry::FileEntryInfo>(&entry->data);
}

bool TarFilesystem::IsDirectory(std::string_view path, bool follow_symlinks) const {
    (void)follow_symlinks;
    return !IsFile(path);
}

bool TarFilesystem::Exists(std::string_view path) const {
    if (!rootdir.valid() && path == "")
        return false;    // Filesystem::IsValid requires this behavior
    return !!rootdir.locate(std::string(path));
}

int64_t TarFilesystem::GetFilesize(std::string_view path) const {
    auto entry = rootdir.locate(std::string(path));
    if (!entry)
        Output::Error("TarFilesystem: no file exists at path {}", path);
    auto info = std::get_if<Entry::FileEntryInfo>(&entry->data);
    if (!info)
        Output::Error("TarFilesystem: {} is a directory", path);
    return info->size;
}

std::streambuf* TarFilesystem::CreateInputStreambuffer(std::string_view path, std::ios_base::openmode) const {
    std::vector<uint8_t> data;

    auto entry = rootdir.locate(std::string(path));
    if (!entry) {
        Output::Warning("TarFilesystem: no file exists at path {}", path);
        return nullptr;
    }
    auto info = std::get_if<Entry::FileEntryInfo>(&entry->data);
    if (!info) {
        Output::Warning("TarFilesystem: {} is a directory", path);
        return nullptr;
    }
    data.resize(info->size);

    stream.seekg(info->offs, std::ios_base::beg);
    if (stream.read(reinterpret_cast<char*>(data.data()), info->size).gcount() != info->size)
        Output::Error("TarFilesystem: error reading archive");

    return new Filesystem_Stream::InputMemoryStreamBuf(std::move(data));
}

bool TarFilesystem::GetDirectoryContent(std::string_view path, std::vector<DirectoryTree::Entry>& entries) const {
    auto entry = rootdir.locate(std::string(path));
    if (!entry) {
        Output::Warning("TarFilesystem: no file exists at path {}", path);
        return false;
    }
    auto contents = std::get_if<Entry::Children>(&entry->data);
    if (!contents) {
        Output::Warning("TarFilesystem: {} is a file", path);
        return false;
    }
    for (const auto &pair : *contents)
        entries.emplace_back(std::string(pair.first), !std::get_if<Entry::FileEntryInfo>(&pair.second.data) ? DirectoryTree::FileType::Directory : DirectoryTree::FileType::Regular );
    return true;
}

std::string TarFilesystem::Describe() const {
    return fmt::format("[Tar] {}", GetPath());
}
