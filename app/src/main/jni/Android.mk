LOCAL_PATH := $(call my-dir)

LOCAL_ALLOW_UNDEFINED_SYMBOLS := true

# change this folder path to yours
NCNN_INSTALL_PATH := /home/zym/Android/ncnn/build-arm64/install

include $(CLEAR_VARS)
LOCAL_MODULE := ncnn
LOCAL_SRC_FILES := $(NCNN_INSTALL_PATH)/lib/libncnn.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := ncnn_jni
LOCAL_SRC_FILES := ncnn_jni.cpp

LOCAL_C_INCLUDES := $(NCNN_INSTALL_PATH)/include

LOCAL_STATIC_LIBRARIES := ncnn

LOCAL_CFLAGS := -O2 -fvisibility=hidden -fomit-frame-pointer -fstrict-aliasing -ffunction-sections -fdata-sections -ffast-math
LOCAL_CPPFLAGS := -O2 -fvisibility=hidden -fvisibility-inlines-hidden -fomit-frame-pointer -fstrict-aliasing -ffunction-sections -fdata-sections -ffast-math
LOCAL_LDFLAGS += -Wl,--gc-sections

LOCAL_CFLAGS += -fopenmp
LOCAL_CPPFLAGS += -fopenmp
LOCAL_LDFLAGS += -fopenmp

LOCAL_LDLIBS := -lz -llog -ljnigraphics

include $(BUILD_SHARED_LIBRARY)
