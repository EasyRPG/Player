#.rst:
# PlayerFindPackage
# -----------------
#
# Helper function that conditionally invokes find_package and automatically
# adds targets and compile time definitions.
#
# Required Arguments
# ^^^^^^^^^^^^^^^^^^
#
# ``NAME`` (String)
#   Name of the library to search. The name must match FindNAME.cmake.
# ``TARGET`` (list of string)
#   Targets to import when the library was found.
#   Searching ends when any of the targets is found.
#
# Optional Arguments
# ^^^^^^^^^^^^^^^^^^
#
# ``CONDITION`` (Bool)
#   When this variable is set to OFF the package is skipped.
#   Allows disabling of libraries via the CMake command line.
# ``DEFINITION`` (String)
#   Preprocessor definition which is set to 1 when the library was found.
# ``REQUIRED`` (without arguments)
#   The library is required. Forwarded to find_package.
# ``VERSION`` (String)
#   Specifies required version of the library.
# ``CONFIG_BROKEN`` (without arguments)
#   Indicates that the config file of the library is known to be broken on
#   certain systems.
#   Uses bundled FindXXX.cmake instead.
# ``ONLY_CONFIG`` (without arguments)
#   Use this for libraries that only have a config file and no find-module.
#   This silences a huge warning emitted by CMake in that case and replaces it
#   with a one-liner.
#
# Return variables
# ^^^^^^^^^^^^^^^^
#
# When found the passed ``TARGET`` becomes available.
#

function(player_find_package)
	cmake_parse_arguments(PARSE_ARGV 0 PLAYER_FIND_PACKAGE "REQUIRED;CONFIG_BROKEN;ONLY_CONFIG" "NAME;CONDITION;DEFINITION;TARGET;VERSION" "")

	set(IS_REQUIRED "")
	if(PLAYER_FIND_PACKAGE_REQUIRED)
		set(IS_REQUIRED "REQUIRED")
	endif()

	set(MODULE "")
	if(PLAYER_FIND_PACKAGE_CONFIG_BROKEN)
		set(CMAKE_FIND_PACKAGE_PREFER_CONFIG_OLD ${CMAKE_FIND_PACKAGE_PREFER_CONFIG})
		set(CMAKE_FIND_PACKAGE_PREFER_CONFIG OFF)
		set(MODULE "MODULE")
	endif()

	set(ONLY_CONFIG_QUIET "")
	if(PLAYER_FIND_PACKAGE_ONLY_CONFIG)
		set(ONLY_CONFIG_QUIET "QUIET")
	endif()

	# Assume "true" when Condition is empty, otherwise dereference the condition variable
	if((NOT PLAYER_FIND_PACKAGE_CONDITION) OR (${PLAYER_FIND_PACKAGE_CONDITION}))
		find_package(${PLAYER_FIND_PACKAGE_NAME} ${PLAYER_FIND_PACKAGE_VERSION} ${IS_REQUIRED} ${MODULE} ${ONLY_CONFIG_QUIET})
		set(DEP_FOUND FALSE)

		foreach(TARGET_ITEM ${PLAYER_FIND_PACKAGE_TARGET})
			if (TARGET ${TARGET_ITEM})
				set(DEP_FOUND TRUE)

				if(${PLAYER_FIND_PACKAGE_NAME}_DIR)
					message(STATUS "Found ${PLAYER_FIND_PACKAGE_NAME}: ${${PLAYER_FIND_PACKAGE_NAME}_DIR} (${TARGET_ITEM}, v${${PLAYER_FIND_PACKAGE_NAME}_VERSION})")
				endif()

				target_link_libraries(${PROJECT_NAME} ${TARGET_ITEM})
				if(PLAYER_FIND_PACKAGE_DEFINITION)
					target_compile_definitions(${PROJECT_NAME} PUBLIC ${PLAYER_FIND_PACKAGE_DEFINITION}=1)
				endif()

				break()
			endif()
		endforeach()

		if (PLAYER_FIND_PACKAGE_ONLY_CONFIG AND NOT DEP_FOUND)
			message(STATUS "Could NOT find ${PLAYER_FIND_PACKAGE_NAME} (missing: ${PLAYER_FIND_PACKAGE_NAME}Config.cmake)")
		endif()
	endif()

	if(PLAYER_FIND_PACKAGE_CONFIG_BROKEN)
		set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ${CMAKE_FIND_PACKAGE_PREFER_CONFIG_OLD})
	endif()
endfunction()
