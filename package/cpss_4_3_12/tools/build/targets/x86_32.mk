ARCH ?= x86
WIDTH ?= 32
CPU ?= i386
CPU_BOARD ?= i386_64
CONFIG_ASIC_SIMULATION := n
TARGET_SPECIFIC_CFLAGS := -m$(WIDTH)

GCC_VER=$(shell gcc  -dumpversion)
AR_VER=$(shell ${AR} --version|head -n 1|sed "s/[^0-9]//g")

$(info GCC ver ${GCC_VER})
$(info AR ver ${AR_VER})


ifeq ("$(AR_VER)", "234")
  AR_FLAGS = crUs
else
  AR_FLAGS = crus
endif


ifneq ("$(ARCH)", "x86")
  $(error ARCH must be x86 for this target)
endif
ifneq ("$(WIDTH)", "32")
  $(error WIDTH must be 32 for this target)
endif
ifneq ("$(CPU)", "i386")
  $(error CPU must be i386 for this target)
endif
ifneq ("$(CPU_BOARD)", "i386_64")
  $(error CPU_BOARD must be i386 for this target)
endif
SHARED_LIB_LD_ARCH := x86
OS_RUN ?= linux

CFLAGS += -DINTEL_CPU

ifeq ("$(GCC_VER)", "9")
  TARGET_SPECIFIC_CFLAGS := -Wno-sizeof-pointer-div -Wno-stringop-truncation
endif

ifeq ("$(GCC_VER)", "10")
  TARGET_SPECIFIC_CFLAGS := -Wno-enum-conversion -Wno-sizeof-pointer-div -Wno-stringop-truncation -Wno-maybe-uninitialized
endif
