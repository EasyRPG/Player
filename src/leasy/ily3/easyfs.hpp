#pragma once

#include "filesystem_native.h"
#include <filesystem>
#include <string>

namespace leasy::ily3::efs {
	namespace fs = std::filesystem;
	
	struct app_fs_ctx {
		fs::path bootdir; /* This is like a protection yk. If ppl try to change the fs::current_directory() lol */
		
	};
	
  extern NativeFilesystem nfs;
	extern app_fs_ctx glob;
}
