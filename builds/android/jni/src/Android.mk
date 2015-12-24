LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

PLAYER_PATH := ../../../..

LOCAL_CPP_EXTENSION := .cxx .cpp
ifeq ($(TARGET_ARCH_ABI),x86)
        EASYRPG_TOOLCHAIN_DIR = $(EASYDEV_ANDROID)/x86-toolchain
else ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
        EASYRPG_TOOLCHAIN_DIR = $(EASYDEV_ANDROID)/arm-toolchain
endif

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/$(PLAYER_PATH)/src \
	$(LOCAL_PATH)/$(PLAYER_PATH)/lib/shinonome \
	$(LOCAL_PATH)/$(PLAYER_PATH)/lib/liblcf/src \
	$(LOCAL_PATH)/$(PLAYER_PATH)/lib/liblcf/src/generated \
	$(EASYRPG_TOOLCHAIN_DIR)/include \
	$(EASYRPG_TOOLCHAIN_DIR)/include/liblcf \
	$(EASYRPG_TOOLCHAIN_DIR)/include/pixman-1 \
	$(EASYRPG_TOOLCHAIN_DIR)/include/freetype2 \
	$(EASYRPG_TOOLCHAIN_DIR)/include/libpng16 \
	$(EASYRPG_TOOLCHAIN_DIR)/include/SDL2

# Add your application source files here...
LOCAL_SRC_FILES := SDL_android_main.c \
	org_easyrpg_player_player_EasyRpgPlayerActivity.cpp \
	$(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(LOCAL_PATH)/$(PLAYER_PATH)/src/*.cpp)) \
	$(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(LOCAL_PATH)/$(PLAYER_PATH)/src/*.c)) \
	$(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(LOCAL_PATH)/$(PLAYER_PATH)/lib/shinonome/*.cxx))

LOCAL_SHARED_LIBRARIES := SDL2 SDL2_mixer

LOCAL_STATIC_LIBRARIES := cpufeatures

LOCAL_LDLIBS := -L$(LOCAL_PATH)/../../obj/local/$(TARGET_ARCH_ABI) \
		-L$(EASYRPG_TOOLCHAIN_DIR)/lib -lGLESv1_CM -llog -lz \
		-llcf -lmad -logg -lvorbisfile -lvorbis -lfreetype \
		-lpixman-1 -lpng -licui18n -licuuc -licudata -lcpufeatures

LOCAL_CFLAGS := -O2 -Wall -Wextra -fno-rtti -DUSE_SDL \
		-DHAVE_SDL_MIXER -DLCF_SUPPORT_ICU

		LOCAL_CPPFLAGS	=	$(LOCAL_C_FLAGS) -std=c++11

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/cpufeatures)
