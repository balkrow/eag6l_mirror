#
# requirements:
# CROSS_COMPILE
# CROSS_COMPILE := /swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/aarch64v8-marvell-linux-gnu-5.2.1_x86_64_20151110/bin/aarch64-marvell-linux-gnu-
# CROSS_COMPILE := /swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi-
##
# Example How to Compile this Tool for AARCH64v8:
# make CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/aarch64v8-marvell-linux-gnu-5.2.1_x86_64_20151110/bin/aarch64-marvell-linux-gnu-
#
# for ARMv7: 
# make CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi-
#

CPSS_ROOT  = ../../../../../..
BASE_DIR   = ../../../..
COMMON_DIR = $(CPSS_ROOT)/common
RTOS_BASE  = $(BASE_DIR)/common/FreeRTOS/FreeRTOS
RTOS_INC   = $(RTOS_BASE)/Source/include
SCPU_BASE   = $(BASE_DIR)/firmware/cm3_sdk/code

BUILD_DIR := objects

CFLAGS += -O0 -g -DCONFIG_ALDRIN

TOOL_NAME=scpu_tool

CC = $(CROSS_COMPILE)gcc

INCLUDES = -I. -I$(COMMON_DIR)/h -I$(BASE_DIR)/common/FreeRTOS/FreeRTOS/Source/portable/GCC/ARM_CM3/ -I$(RTOS_INC) -I$(SCPU_BASE)/h

FILES = \
  scpu_tool.c \
  i2c_lib.c  \
  mvTwsi.c \
  ipc.c \
  crc32.c \
  slave_smi.c \
  smi_drv.c \
  $(COMMON_DIR)/src/cpss/generic/ipc/mvShmIpc.c

VPATH = $(sort $(dir $(FILES)))

OBJS = $(notdir $(FILES:.c=.o))
OBJS_BUILD = $(addprefix $(BUILD_DIR)/, $(OBJS))

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $(BUILD_DIR)/$*.o

all: $(OBJS_BUILD)
	$(CC) $(OBJS_BUILD) -o $(TOOL_NAME)

clean:
	@rm -rf objects/ $(TOOL_NAME)

# debug target. usage: make print-var will print value of variable "var"
print-%  : ; @echo $* = $($*)


