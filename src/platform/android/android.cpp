#include "android.h"

JNIEnv* EpAndroid::env = nullptr;
std::function<void()> EpAndroid::android_fn;
std::mutex EpAndroid::android_mutex;
