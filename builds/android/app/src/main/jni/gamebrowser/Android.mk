LOCAL_PATH := $(call my-dir)

EASYRPG_TOOLCHAIN_DIR = $(EASYDEV_ANDROID)/$(TARGET_ARCH_ABI)-toolchain

include $(CLEAR_VARS)

LOCAL_MODULE := gamebrowser

LOCAL_C_INCLUDES :=

LOCAL_SRC_FILES := \
	org_easyrpg_player_game_browser_GameScanner.cpp

LOCAL_LDLIBS := -llog -lz -latomic

LOCAL_STATIC_LIBRARIES := png

LOCAL_CFLAGS := -O2 -Wall -Wextra

LOCAL_CXXFLAGS := $(LOCAL_C_FLAGS) -std=c++11 -fno-rtti

include $(BUILD_SHARED_LIBRARY)
