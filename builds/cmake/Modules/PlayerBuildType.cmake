# Allow user override and multi-configuration generators
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	# Set build type if none was specified, git checkouts default to debug
	set(default_build_type "Release")
	if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
		set(default_build_type "Debug")
	endif()
	message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
	set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
else()
	message(STATUS "Build type is set to ${CMAKE_BUILD_TYPE}.")
endif()

# Ensure needed debugging flags are passed
string(TOLOWER "${CMAKE_BUILD_TYPE}" current_build_type)
if(current_build_type STREQUAL "debug")
	add_compile_definitions(DEBUG)
elseif(current_build_type MATCHES "^(release|minsizerel|relwithdebinfo)$")
	add_compile_definitions(NDEBUG)
endif()

# Since dkp is disabling default flags, add optimizing here
if(NINTENDO_WII OR NINTENDO_3DS OR NINTENDO_SWITCH)
	foreach(lang C CXX ASM)
		string(APPEND CMAKE_${lang}_FLAGS_DEBUG " -g -O0")
		string(APPEND CMAKE_${lang}_FLAGS_MINSIZEREL " -Os")
		string(APPEND CMAKE_${lang}_FLAGS_RELEASE " -O3")
		string(APPEND CMAKE_${lang}_FLAGS_RELWITHDEBINFO " -g -O2")
	endforeach()
endif()
