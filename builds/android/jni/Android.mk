LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := EasyRPG_Player
LOCAL_CPP_EXTENSION := .cxx .cpp .cc
LOCAL_CPP_FEATURES := exceptions

LOCAL_SRC_FILES := \
	$(foreach i, \
		$(wildcard ../../src/*.cpp) \
		$(wildcard ../../lib/readers/src/*.cpp) \
		$(wildcard ../../lib/readers/src/generated/*.cpp) \
		$(wildcard ../../lib/shinonome/*.cxx) \
	, ../$(i)) \

LOCAL_LDLIBS += -llog -landroid \
	-L android/lib \
	$(shell ./android/bin/freetype-config --libs) \
	-lpng -liconv \
	-lpixman-1 -lexpat \
	-lsndfile -lvorbis -lvorbisenc -lvorbisfile -logg -lFLAC \
	-lopenal -lOpenSLES \
	-lboost_chrono-mt-sd -lboost_thread-mt-sd -lboost_system-mt-sd \
	obj/local/$(APP_ABI)/libcpufeatures.a \

LOCAL_STATIC_LIBRARIES := supc++

LOCAL_CPPFLAGS := -std=gnu++11 \
	$(shell ./android/bin/freetype-config --cflags) \
	-Iandroid/include \
	-Iandroid/include/freetype2 \
	-Iandroid/include/pixman-1 \
	-I../../src \
	-I../../lib/readers/include \
	-I../../lib/shinonome \
	-DEASYRPG_IS_ANDROID \
	-DHAVE_OPENAL=1 -DHAVE_BOOST_LIBRARIES=1 \

LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
$(call import-module,android/cpufeatures)
