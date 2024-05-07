ARCH ?= x86
WIDTH ?= 64
CPU ?= i386
CPU_BOARD ?= i386_64
CONFIG_ASIC_SIMULATION := n
GCC_VER=$(shell gcc  -dumpversion)
AR_VER=$(shell ${AR} --version|head -n 1|sed "s/[^0-9]//g")

ifeq ("$(AR_VER)", "234")
  AR_FLAGS = crUs
else
  AR_FLAGS = crus
endif

$(info GCC ver ${GCC_VER})
$(info AR ver ${AR_VER})

ifneq ("$(ARCH)", "x86")
  $(error ARCH must be x86 for this target)
endif
ifneq ("$(WIDTH)", "64")
  $(error WIDTH must be 32 for this target)
endif
ifneq ("$(CPU)", "i386")
  $(error CPU must be i386 for this target)
endif
ifneq ("$(CPU_BOARD)", "i386_64")
  $(error CPU_BOARD must be i386 for this target)
endif
SHARED_LIB_LD_ARCH := INTEL64 #TODO - this is temporary for compatibility with build_cpss.sh, remove when not needed
OS_RUN ?= linux

CFLAGS += -DINTEL64_CPU
ifeq ("$(GCC_VER)", "9")
  TARGET_SPECIFIC_CFLAGS := -Wno-sizeof-pointer-div -Wno-stringop-truncation
endif

ifeq ("$(GCC_VER)", "10")
  TARGET_SPECIFIC_CFLAGS := -Wno-enum-conversion -Wno-sizeof-pointer-div -Wno-stringop-truncation -Wno-maybe-uninitialized
endif

