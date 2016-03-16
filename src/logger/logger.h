#pragma once

#include "tinyformat.h"

#ifdef PLATFORM_ANDROID
#define TAG "Alfons"

#include <android/log.h>

template <typename... Args>
void log(const char* fmt, const Args&... args) {
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "%s", tfm::format(fmt, args...).c_str());
}
#define LOGD(...)                                      \
    (void)(__android_log_print(ANDROID_LOG_DEBUG, TAG, \
                               "%s", tfm::format(__VA_ARGS__).c_str()))
#define LOGI(...)                                     \
    (void)(__android_log_print(ANDROID_LOG_INFO, TAG, \
                               "%s", tfm::format(__VA_ARGS__).c_str()))
#define LOGE(...)                                      \
    (void)(__android_log_print(ANDROID_LOG_ERROR, TAG, \
                               "%s", tfm::format(__VA_ARGS__).c_str()))

#else

template <typename... Args>
void log(const char* fmt, const Args&... args) {
    tfm::printfln(fmt, args...);
}

#define LOGD(...) (void)(tfm::printfln(__VA_ARGS__))
#define LOGI(...) (void)(tfm::printfln(__VA_ARGS__))
#define LOGE(...) (void)(tfm::printfln(__VA_ARGS__))
#endif
