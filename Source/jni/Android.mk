LOCAL_PATH := $(call my-dir)
MAIN_LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# Here is the name of your lib.
# When you change the lib name, change also on System.loadLibrary("") under OnCreate method on StaticActivity.java
# Both must have same name
LOCAL_MODULE    := native-lib

# Code optimization
LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := -Wno-error=format-security -fpermissive
LOCAL_CFLAGS += -fno-rtti -fno-exceptions -std=c++14
LOCAL_CPPFLAGS += -ffunction-sections -fdata-sections
LOCAL_LDFLAGS += -Wl,--strip-all

# Here you add the cpp file
LOCAL_C_INCLUDES += $(MAIN_LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src/UE4
LOCAL_SRC_FILES := src/Main.cpp \
                        src/Tools.cpp \
                        src/Logger.cpp \
                        src/UE4/FunctionFlags.cpp \
                        src/UE4/PropertyFlags.cpp \
                        src/ObjectsStore.cpp \
                        src/NamesStore.cpp \
                        src/Generator.cpp \
                        src/NameValidator.cpp \
                        src/UE4/GenericTypes.cpp \
                        src/PrintHelper.cpp \
                        src/Package.cpp \
                       
                        
LOCAL_LDLIBS := -llog -landroid
include $(BUILD_SHARED_LIBRARY)
