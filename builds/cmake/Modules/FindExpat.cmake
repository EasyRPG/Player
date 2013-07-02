include(FindPackageHandleStandardArgs)

find_path(EXPAT_INCLUDE_DIR expat.h)
find_library(EXPAT_LIBRARY expat)

find_package_handle_standard_args(Expat
	REQUIRED_VARS EXPAT_INCLUDE_DIR EXPAT_LIBRARY)
