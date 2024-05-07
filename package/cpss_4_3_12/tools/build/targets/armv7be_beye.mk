export ARCH ?= arm
WIDTH ?= 32
CONFIG_ASIC_SIMULATION := n
TARGET_SPECIFIC_CFLAGS := -mbig-endian -DCPU_BE

ifeq ("$(CROSS_COMPILE)", "")
  export CROSS_COMPILE := /swtools/devtools/gnueabi/arm_be/gcc-linaro-7.5.0-2019.12-x86_64_armeb-linux-gnueabi/bin/armeb-linux-gnueabi-
  CC := /local/store/BullseyeCoverage/bin/covc -i -a $(CROSS_COMPILE)gcc
  LD := /local/store/BullseyeCoverage/bin/covc -i -a $(CROSS_COMPILE)gcc
  $(info CROSS_COMPILE not specified, defaulting to $(CROSS_COMPILE))
endif
ifneq ("$(ARCH)", "arm")
  $(error ARCH must be arm for this target)
endif
ifneq ("$(WIDTH)", "32")
  $(error WIDTH must be 32 for this target)
endif
SHARED_LIB_LD_ARCH := ARMARCH7 #TODO - this is temporary for compatibility with build_cpss.sh, remove when not needed
OS_RUN ?= linux

export CPU_FAMILY := CPU_ARM
export LSP_KERNEL_TYPE := zImage# for backward compatibility

CFLAGS += -DCPU_ARM
