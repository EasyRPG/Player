include(FindPackageHandleStandardArgs)

find_path(ICONV_INCLUDE_DIR iconv.h)
find_library(ICONV_LIBRARY iconv)

if(NOT (EXISTS "${ICONV_LIBRARY}"))
	unset(ICONV_LIBRARY)
endif()

find_package_handle_standard_args(Iconv
	REQUIRED_VARS ICONV_INCLUDE_DIR)
