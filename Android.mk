LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CPP_EXTENSION := .cpp .cc
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
	LOCAL_MODULE := CrashInfo
else
	LOCAL_MODULE := CrashInfo64
endif
LOCAL_SRC_FILES := main.cpp yyjson/src/yyjson.c
LOCAL_CFLAGS += -O2 -mfloat-abi=softfp -DNDEBUG -std=c17
LOCAL_CXXFLAGS += -O2 -mfloat-abi=softfp -DNDEBUG -std=c++17
LOCAL_C_INCLUDES += ./include
LOCAL_LDLIBS += -llog
include $(BUILD_SHARED_LIBRARY)