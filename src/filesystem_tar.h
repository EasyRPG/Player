#ifndef FILESYSTEM_TAR
#define FILESYSTEM_TAR

#include <map>
#include <string>
#include <cstdint>
#include <variant>
#include <string_view>

#include "filesystem.h"
#include "filesystem_stream.h"

union tar_entry_raw {
    struct {
        char filename[100];
        char perms[8];
        char uid[8];
        char gid[8];
        char size[12];
        char modify_time[12];
        char checksum[8];
        char type;
        char symlink_to[100];
        struct {
            char magic[6];
            char version[2];
            char username[32];
            char groupname[32];
            char device_major[8];
            char device_minor[8];
            char path_prefix[155];
        } ustar;
    } data;
    char pack[512];
};

class TarFilesystem : public Filesystem {
    struct Entry {
        struct FileEntryInfo {
            long offs, size;
        };
        std::string name = "";
        using Children = std::map<std::string, Entry>;
        std::variant<std::monostate, Children, FileEntryInfo> data = std::monostate();

        Entry();
        Entry(std::string dirname);
        Entry(long offs, tar_entry_raw from, long *skip);

        bool valid() const;
        void invalidate();
        bool add(Entry &&entry);
        // Entry &locate(std::string path);
        Entry *locate(std::string path, bool auto_create_dirs = false);
        const Entry *locate(std::string path) const;
    };

    mutable Filesystem_Stream::InputStream stream;

    Entry rootdir = { "" };

protected:
    bool IsFile(std::string_view path) const override;
    bool IsDirectory(std::string_view path, bool follow_symlinks) const override;
    bool Exists(std::string_view path) const override;
    int64_t GetFilesize(std::string_view path) const override;
    std::streambuf* CreateInputStreambuffer(std::string_view path, std::ios_base::openmode mode) const override;
    bool GetDirectoryContent(std::string_view path, std::vector<DirectoryTree::Entry>& entries) const override;
    std::string Describe() const override;

public:
    TarFilesystem(std::string base_path, FilesystemView parent_fs);
};

#endif  // FILESYSTEM_TAR
