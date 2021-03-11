cmake_minimum_required(VERSION 3.7)

project(easyrpg_android VERSION 1.0 LANGUAGES CXX)

add_library(gamebrowser
	org_easyrpg_player_game_browser_GameScanner.cpp
	org_easyrpg_player_game_browser_GameScanner.h
)

find_package(PNG REQUIRED)
target_link_libraries(gamebrowser PNG::PNG)

if(BUILD_SHARED_LIBS)
	set_property(TARGET gamebrowser PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()
