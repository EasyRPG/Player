if (NOT UNIX)
message( FATAL_ERROR "VTune not supported on this platform")
endif()

if(NOT VTUNE_ROOT)
	set( VTUNE_ROOT /opt/intel/vtune_amplifier )
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set( VTUNE_ARCH "64" )
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
	set( VTUNE_ARCH "32" )
else()
	message( FATAL_ERROR "Unable to determine 32 or 64 bit architecture for intel" )
endif()

find_path( VTUNE_INCLUDE ittnotify.h PATHS ${VTUNE_ROOT}/include )
find_library( VTUNE_LIBRARY libittnotify.a PATHS ${VTUNE_ROOT}/lib${VTUNE_ARCH}/ )

if( NOT VTUNE_INCLUDE MATCHES NOTFOUND )
	if( NOT VTUNE_LIBRARY MATCHES NOTFOUND )
		set( VTUNE_FOUND TRUE )
		message( STATUS "Found ITT: ${VTUNE_LIBRARY}" )

		get_filename_component( VTUNE_LIBRARY_PATH ${VTUNE_LIBRARY} PATH )

		add_library(ittnotify STATIC IMPORTED)
		set_target_properties(ittnotify
			PROPERTIES
			IMPORTED_LOCATION "${VTUNE_LIBRARY}"
			INTERFACE_INCLUDE_DIRECTORIES "${VTUNE_INCLUDE}"
			INTERFACE_LINK_LIBRARIES "dl")
	endif()
endif()

