LOCAL_PATH := $(call my-dir)

##
# Sets up a module for the skia shared object to be copied into the apk.
##

include $(CLEAR_VARS)

# Name for referencing this module in other modules
LOCAL_MODULE := skia_android

# Local filename of the skia shared object
LOCAL_SRC_FILES := skia/libskia_android.so

# Makes this module into shared object that is simply copied into the apk
include $(PREBUILT_SHARED_LIBRARY)

##
# Sets up the JNI module that our app calls into to draw things with skia.
##

# Clear out the variables of the previous module
include $(CLEAR_VARS) 

# Allows the compiler to find the Skia header files
LOCAL_C_INCLUDES := $(LOCAL_PATH)/skia/include/config \
									  $(LOCAL_PATH)/skia/include/core \
									  $(LOCAL_PATH)/skia/include/gpu \
                    $(LOCAL_PATH)/skia/include/utils

# Makes the skia shared object get pulled in as a reference
LOCAL_SHARED_LIBRARIES := skia_android

LOCAL_MODULE    := native_activity
LOCAL_SRC_FILES := main.cpp
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM
LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
