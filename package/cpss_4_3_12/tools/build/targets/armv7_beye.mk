.NOTPARALLEL:
export ARCH ?= arm
WIDTH ?= 32
CONFIG_ASIC_SIMULATION := n

ifeq ("$(CROSS_COMPILE)", "")
  export CROSS_COMPILE := /swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi-
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

# flags used by kernel modules makefile - need to change according to kernel version
export CPU_FAMILY := CPU_ARM
export LSP_KERNEL_TYPE := zImage

