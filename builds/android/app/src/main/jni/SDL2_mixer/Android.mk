LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := SDL2_mixer

ifeq ($(TARGET_ARCH_ABI),x86)
	EASYRPG_LIB_DIR = $(EASYDEV_ANDROID)/x86-toolchain/lib
else ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
	EASYRPG_LIB_DIR = $(EASYDEV_ANDROID)/armeabi-v7a-toolchain/lib
else ifeq ($(TARGET_ARCH_ABI),armeabi)
	EASYRPG_LIB_DIR = $(EASYDEV_ANDROID)/armeabi-toolchain/lib
else ifeq ($(TARGET_ARCH_ABI),mips)
	EASYRPG_LIB_DIR = $(EASYDEV_ANDROID)/mips-toolchain/lib
endif

LOCAL_SRC_FILES := $(EASYRPG_LIB_DIR)/lib$(LOCAL_MODULE).so

include $(PREBUILT_SHARED_LIBRARY)
