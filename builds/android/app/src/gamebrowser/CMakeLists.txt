cmake_minimum_required(VERSION 3.7)

project(easyrpg_android VERSION 1.0 LANGUAGES CXX)

add_library(gamebrowser
	org_easyrpg_player_game_browser.cpp
	org_easyrpg_player_game_browser.h
)

set_target_properties(gamebrowser PROPERTIES DEBUG_POSTFIX "")

target_link_libraries(gamebrowser easyrpg_android)

if(BUILD_SHARED_LIBS)
	set_property(TARGET gamebrowser PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()
