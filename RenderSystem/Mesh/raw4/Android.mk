LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_MODULE := libraw4

LOCAL_CFLAGS := -DANDROID_NDK -Werror

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../stdtypes/ \
	$(LOCAL_PATH)/../fastmath/ \
	$(LOCAL_PATH)/../geometry/ \
	$(LOCAL_PATH)/../stdutils/ \
	$(LOCAL_PATH)/../platform/ \
	
LOCAL_SRC_FILES := \
	Raw4Loader.cpp \
	Raw4Context.cpp \

include $(BUILD_STATIC_LIBRARY)
