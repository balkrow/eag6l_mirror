export ARCH ?= arm64
WIDTH ?= 64
CONFIG_ASIC_SIMULATION := n
TARGET_SPECIFIC_CFLAGS := -mbig-endian -DCPU_BE

ifeq ("$(CROSS_COMPILE)", "")
  export CROSS_COMPILE := /swtools/devtools/gnueabi/arm_be/aarch64ebv8-marvell-linux-gnu/bin/aarch64_be-linux-gnu-
  $(info CROSS_COMPILE not specified, defaulting to $(CROSS_COMPILE))
endif
ifneq ("$(ARCH)", "arm64")
  $(error ARCH must be arm for this target)
endif
ifneq ("$(WIDTH)", "64")
  $(error WIDTH must be 32 for this target)
endif
SHARED_LIB_LD_ARCH := AARCH64v8 #TODO - this is temporary for compatibility with build_cpss.sh, remove when not needed
OS_RUN ?= linux

CFLAGS += -DCPU_ARM

export CPU_FAMILY := CPU_ARM
export LSP_KERNEL_TYPE := Image# for backward compatibility

