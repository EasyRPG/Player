include(FindPackageHandleStandardArgs)

find_path(LUA_INCLUDE_DIR lua.h)
find_library(LUA_LIBRARY lua)

if(EXISTS "${LUA_LIBRARY}")
	set(LUA_LIBRARIES ${LUA_LIBRARY})
endif()

find_package_handle_standard_args(Lua
	REQUIRED_VARS LUA_INCLUDE_DIR LUA_LIBRARIES)
