#ifdef _WIN32
#  include "platform/registry_win.cpp"
#elif defined(HAVE_WINE)
#  include "platform/registry_wine.cpp"
#else

/*
 * define empty registry
 */

#include "registry.h"

std::string Registry::ReadStrValue(HKEY, std::string const&, std::string const&) {
	return std::string(); // return empty string
}

int Registry::ReadBinValue(HKEY, std::string const&, std::string const&, unsigned char*) {
	return 0; // empty binary
}

#endif
