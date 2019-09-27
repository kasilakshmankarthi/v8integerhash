LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CFLAGS := -O3 -march=armv8.3-a+crypto -mfpu=neon -Wall -Werror -Wno-shift-count-overflow
LOCAL_SRC_FILES := v8integerhash.cpp

LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
LOCAL_MODULE_TAGS := release
LOCAL_MODULE := v8integerhash
include $(BUILD_EXECUTABLE)
