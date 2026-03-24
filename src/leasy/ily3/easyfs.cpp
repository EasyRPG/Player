#include <filesystem>
#include "filesystem_native.h"

namespace leasy::ily3::efs {
  NativeFilesystem nfs = NativeFilesystem(std::filesystem::current_path().string(), FilesystemView());
}