find_package(GTest REQUIRED)

set(TEST_GAME_REPOSITORY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../lib/TestGame")
if(NOT EXISTS ${TEST_GAME_REPOSITORY_PATH})
	execute_process(COMMAND ${GIT_EXECUTABLE} clone "--depth=1"
		"https://github.com/EasyRPG/TestGame.git"
		"${TEST_GAME_REPOSITORY_PATH}")
endif()
set(TEST_GAME_PATH "${TEST_GAME_REPOSITORY_PATH}/TestGame-2000")

set(TEST_ENVS "RPG_TEST_GAME_PATH=${TEST_GAME_PATH}")
if(EXISTS $ENV{RPG_RTP_PATH})
	get_filename_component(RPG_RTP_PATH "$ENV{RPG_RTP_PATH}" ABSOLUTE)
	message(STATUS "using rtp path ${RPG_RTP_PATH}")
	list(APPEND TEST_ENVS "RPG_RTP_PATH=${RPG_RTP_PATH}")
endif()

file(GLOB TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
foreach(i ${TEST_FILES})
	get_filename_component(name "${i}" NAME_WE)
	add_executable(test_${name} ${i})
	target_link_libraries(test_${name} ${PROJECT_NAME})
	target_link_libraries(test_${name} gtest gtest_main)

	add_test(
		NAME test_${name} WORKING_DIRECTORY ${TEST_GAME_PATH}
		COMMAND ${PROJECT_BINARY_DIR}/test_${name})
	set_tests_properties(test_${name}
		PROPERTIES ENVIRONMENT "${TEST_ENVS}")
endforeach()

