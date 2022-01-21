LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE := helloworld_test
LOCAL_SRC_FILES := \
    helloworld_test.cpp \

LOCAL_SHARED_LIBRARIES := \
   liblog \
   libhidlbase \
   libutils \
   android.hardware.helloworld@1.0 \

include $(BUILD_EXECUTABLE)
