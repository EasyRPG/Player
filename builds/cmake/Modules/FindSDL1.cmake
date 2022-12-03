# - Find SDL
# Find the SDL headers and libraries
# Custom module because the bundled FindSDL.cmake fails to link on Linux
#
#  SDL::SDL - Imported target to use for building a library
#  SDL::SDLmain - Imported interface target to use if you want SDL and SDLmain.
#  SDL1_FOUND - True if SDL was found.

# Invoke pkgconfig for hints
find_package(PkgConfig QUIET)
set(SDL_INCLUDE_HINTS)
set(SDL_LIB_HINTS)
if(PKG_CONFIG_FOUND AND NOT ANDROID)
	pkg_search_module(SDLPC QUIET sdl)
	if(SDLPC_INCLUDE_DIRS)
		set(SDL_INCLUDE_HINTS ${SDLPC_INCLUDE_DIRS})
	endif()
	if(SDLPC_LIBRARY_DIRS)
		set(SDL_LIB_HINTS ${SDLPC_LIBRARY_DIRS})
	endif()
endif()

include(FindPackageHandleStandardArgs)

find_library(SDL_LIBRARY
	NAMES
	SDL
	HINTS
	${SDL_LIB_HINTS}
	PATHS
	${SDL_ROOT_DIR}
	ENV SDLDIR
	PATH_SUFFIXES lib SDL ${SDL_LIB_PATH_SUFFIX})

find_path(SDL_INCLUDE_DIR
	NAMES
	SDL_timer.h
	HINTS
	${SDL_INCLUDE_HINTS}
	PATHS
	${SDL_ROOT_DIR}
	ENV SDLDIR
	PATH_SUFFIXES include include/sdl include/SDL SDL)

# handle the QUIETLY and REQUIRED arguments and set QUATLIB_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL1
	DEFAULT_MSG
	SDL_LIBRARY
	SDL_INCLUDE_DIR
	${SDL_EXTRA_REQUIRED})

if(SDL1_FOUND)
	if(NOT TARGET SDL::SDL)
		# Create SDL::SDL
		add_library(SDL::SDL UNKNOWN IMPORTED)

		set_target_properties(SDL::SDL PROPERTIES
			IMPORTED_LOCATION "${SDL_LIBRARY}")

		set_target_properties(SDL::SDL
			PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${SDL_INCLUDE_DIR};${SDLPC_INCLUDE_DIRS}"
		)

		# Compute what to do with SDLmain
		set(SDLMAIN_LIBRARIES SDL::SDL)
		add_library(SDL::SDLmain INTERFACE IMPORTED)
		if(SDL_SDLMAIN_LIBRARY)
			add_library(SDL::SDLmain_real STATIC IMPORTED)
			set_target_properties(SDL::SDLmain_real
				PROPERTIES
				IMPORTED_LOCATION "${SDL_SDLMAIN_LIBRARY}")
			set(SDLMAIN_LIBRARIES SDL::SDLmain_real ${SDLMAIN_LIBRARIES})
		endif()

		set_target_properties(SDL::SDLmain
			PROPERTIES
			INTERFACE_LINK_LIBRARIES "${SDLMAIN_LIBRARIES}")

		# Remove -lSDL -lSDLmain from the pkg-config linker line,
		# to prevent linking against the system library
		list(REMOVE_ITEM SDLPC_STATIC_LIBRARIES SDLmain SDL)
		set_property(TARGET SDL::SDL APPEND PROPERTY
			INTERFACE_LINK_LIBRARIES "${SDLPC_STATIC_LIBRARIES}")
		set_property(TARGET SDL::SDL APPEND PROPERTY
			INTERFACE_INCLUDE_DIRECTORIES "${SDLPC_STATIC_LIBRARY_DIRS}")
	endif()
	mark_as_advanced(SDL_ROOT_DIR)
endif()

mark_as_advanced(SDL_LIBRARY
	SDL_INCLUDE_DIR
	SDL_SDLMAIN_LIBRARY)
