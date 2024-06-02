#ifndef ANDROID_LOGGING_H
#define ANDROID_LOGGING_H

#include <android/log.h>

// Simple logging macros
#define LOGE(tag, ...) __android_log_print(ANDROID_LOG_ERROR, tag, __VA_ARGS__)
#define LOGI(tag, ...) __android_log_print(ANDROID_LOG_INFO, tag, __VA_ARGS__)


#endif // ANDROID_LOGGING_H