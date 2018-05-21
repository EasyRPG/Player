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
# ``TARGET`` (String)
#   Target to import when the library was found.
#
# Optional Arguments
# ^^^^^^^^^^^^^^^^^^
#
# ``CONDITION`` (Bool)
#   When this variable is set to OFF the package is skipped.
#   Allows disabling of libraries via the CMake command line.
# ``DEFINITION`` (String)
#   Preprocessor definition which is set to 1 when the library was found.
# ``FOUND`` (String)
#   Specifies the variable name of the FOUND variable that is checked.
#   When missing, the variable is the ``NAME`` argument in upper case.
# ``REQUIRED`` (without arguments)
#   The library is required. Forwarded to find_package.
#
# Return variables
# ^^^^^^^^^^^^^^^^
#
# ``NAME_FOUND``
#   Set to TRUE when the library was found. NAME is the ``NAME`` argument in
#   upper case or the ``FOUND`` argument when specified.
#

function(player_find_package)
	cmake_parse_arguments(PARSE_ARGV 0 PLAYER_FIND_PACKAGE "REQUIRED" "NAME;CONDITION;DEFINITION;FOUND;TARGET" "")
	string(TOUPPER ${PLAYER_FIND_PACKAGE_NAME} FIND_PACKAGE_NAME)

	set(IS_REQUIRED "")
	if(PLAYER_FIND_PACKAGE_REQUIRED)
		set(IS_REQUIRED "REQUIRED")
	endif()

	if(PLAYER_FIND_PACKAGE_FOUND)
		set(FIND_PACKAGE_NAME ${PLAYER_FIND_PACKAGE_FOUND})
	endif()

	# Assume "true" when Condition is empty, otherwise dereference the condition variable
	if((NOT PLAYER_FIND_PACKAGE_CONDITION) OR (${PLAYER_FIND_PACKAGE_CONDITION}))
		find_package(${PLAYER_FIND_PACKAGE_NAME} ${IS_REQUIRED})
		if (${${FIND_PACKAGE_NAME}_FOUND})
			target_link_libraries(${PROJECT_NAME} ${PLAYER_FIND_PACKAGE_TARGET})
			if(PLAYER_FIND_PACKAGE_DEFINITION)
				target_compile_definitions(${PROJECT_NAME} PUBLIC ${PLAYER_FIND_PACKAGE_DEFINITION}=1)
			endif()
		endif()
	endif()

	set(${FIND_PACKAGE_NAME}_FOUND ${${FIND_PACKAGE_NAME}_FOUND} PARENT_SCOPE)
endfunction()
