export ARCH ?= arm64
WIDTH ?= 64
CONFIG_ASIC_SIMULATION := n
GCC ?= 12

ifeq ("$(CROSS_COMPILE)", "")
  ifeq ("$(GCC)", "10")
    export CROSS_COMPILE := /swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/gcc-arm-10.1-2020.6-x86_64-aarch64-marvell-linux/bin/aarch64-marvell-linux-gnu-
  else
    ifeq ("$(GCC)", "12")
      export CROSS_COMPILE := /swtools/devtools/gnueabi/arm_le/gcc-arm-12.04-2022.12-x86_64-aarch64-marvell-linux/bin/aarch64-marvell-linux-gnu-
    else
        ifeq ("$(GCC)", "5")
            export CROSS_COMPILE := /swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/aarch64v8-marvell-linux-gnu-5.2.1_x86_64_20151110/bin/aarch64-marvell-linux-gnu-
        endif
    endif
  endif
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

CC = $(CROSS_COMPILE)gcc
GCC_VER=$(shell ${CC}  -dumpversion | cut -d. -f1)

export CPU_FAMILY := CPU_ARM
export LSP_KERNEL_TYPE := Image# for backward compatibility
#add by balkrow
CFLAGS += -DCPU_ARM -DMVDEMO
