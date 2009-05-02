
ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libaudio

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    libmedia \
    libhardware_legacy

ifeq ($(TARGET_OS)-$(TARGET_SIMULATOR),linux-true)
LOCAL_LDLIBS += -ldl
endif

ifeq ($(strip $(BOARD_USES_QCOM_HARDWARE)), true)
  ifeq ($(strip $(BOARD_USES_QCOM_7x_CHIPSET)), true)
    LOCAL_CFLAGS += -DSURF
  else ifeq ($(strip $(BOARD_USES_QCOM_8x_CHIPSET)), true)
    LOCAL_CFLAGS += -DSURF8K
  endif
  
  LOCAL_SHARED_LIBRARIES += libdl
endif

LOCAL_SRC_FILES += AudioHardware.cpp

LOCAL_CFLAGS += \
    -fno-short-enums \
    -DMSM72XX_AUDIO \
    -DVOC_CODEC_DEFAULT=0

LOCAL_STATIC_LIBRARIES += libaudiointerface

include $(BUILD_SHARED_LIBRARY)

endif # not BUILD_TINY_ANDROID

