if(WIN32)
	# Target Unicode API
	add_compile_definitions(_UNICODE UNICODE)

	# Disable API deprecation warnings
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

	# Prevent some Windows.h global namespace pollution
	add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)

	# Make math constants available
	add_compile_definitions(_USE_MATH_DEFINES)
endif()

if(MSVC)
	if(${CMAKE_VERSION} VERSION_LESS "3.15.0") 
		message(WARNING "Your CMake version is older than 3.15")
		message(WARNING "For proper MSVC runtime library support upgrade to a newer version")
		
		option(SHARED_RUNTIME "Windows: Build using the shared runtime library (/MD), disable for static runtime (/MT)" ON)
		
		# Set compiler options.
		set(variables
			CMAKE_C_FLAGS_DEBUG
			CMAKE_C_FLAGS_MINSIZEREL
			CMAKE_C_FLAGS_RELEASE
			CMAKE_C_FLAGS_RELWITHDEBINFO
			CMAKE_CXX_FLAGS_DEBUG
			CMAKE_CXX_FLAGS_MINSIZEREL
			CMAKE_CXX_FLAGS_RELEASE
			CMAKE_CXX_FLAGS_RELWITHDEBINFO
		)
		if(SHARED_RUNTIME)
			message(STATUS "Windows: Using dynamic runtime library (/MD)")
			foreach(variable ${variables})
				if(${variable} MATCHES "/MT")
					string(REGEX REPLACE "/MT" "/MD" ${variable} "${${variable}}")
				endif()
			endforeach()
		else()
			message(STATUS "Windows: Using static runtime library (/MT)")
			foreach(variable ${variables})
				if(${variable} MATCHES "/MD")
					string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
				endif()
			endforeach()
		endif()
	else()
		# Depends on vcpkg but we don't support anything else
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<STREQUAL:${VCPKG_CRT_LINKAGE},dynamic>:DLL>" CACHE STRING "")
	endif()

	option(MSVC_MULTICORE "MSVC: Build using multiple cores (/MP)" ON)
	if (MSVC_MULTICORE)
		add_compile_options("/MP")
	endif()

	# Interpret character literals as UTF-8
	add_compile_options("/utf-8")
endif()
