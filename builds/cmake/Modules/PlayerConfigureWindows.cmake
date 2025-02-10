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
	# Depends on vcpkg but we don't support anything else
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<STREQUAL:${VCPKG_CRT_LINKAGE},dynamic>:DLL>" CACHE STRING "")

	option(MSVC_MULTICORE "MSVC: Build using multiple cores (/MP)" ON)
	if (MSVC_MULTICORE)
		add_compile_options("/MP")
	endif()

	# Interpret character literals as UTF-8
	add_compile_options("/utf-8")
endif()
