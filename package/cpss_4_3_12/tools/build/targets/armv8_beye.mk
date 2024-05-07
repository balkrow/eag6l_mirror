export ARCH ?= arm64
WIDTH ?= 64
CONFIG_ASIC_SIMULATION := n

ifeq ("$(CROSS_COMPILE)", "")
  export CROSS_COMPILE := /swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/aarch64v8-marvell-linux-gnu-5.2.1_x86_64_20151110/bin/aarch64-marvell-linux-gnu-
  CC := /local/store/BullseyeCoverage/bin/covc -i -a $(CROSS_COMPILE)gcc
  LD := /local/store/BullseyeCoverage/bin/covc -i -a $(CROSS_COMPILE)gcc
  $(info CROSS_COMPILE not specified, defaulting to $(CROSS_COMPILE))
endif
ifneq ("$(ARCH)", "arm64")
  $(error ARCH must be armv8 for this target)
endif
ifneq ("$(WIDTH)", "64")
  $(error WIDTH must be 64 for this target)
endif
SHARED_LIB_LD_ARCH := AARCH64v8 #TODO - this is temporary for compatibility with build_cpss.sh, remove when not needed
OS_RUN ?= linux

export CPU_FAMILY := CPU_ARM
export LSP_KERNEL_TYPE := Image

