if(WIN32)
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

	# Target Unicode API
	add_definitions(-D_UNICODE)
	add_definitions(-DUNICODE)

	# Disable API deprecation warnings
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(MSVC)
	option(MSVC_MULTICORE "MSVC: Build using multiple cores (/MP)" ON)
	if (MSVC_MULTICORE)
		add_compile_options(/MP)
	endif()
endif()
