#include <filesystem>
#include "filesystem_native.h"
#include "easyfs.hpp"

namespace leasy::ily3::efs {
  NativeFilesystem nfs = NativeFilesystem(std::filesystem::current_path().string(), FilesystemView());
	app_fs_ctx glob = {};
}
