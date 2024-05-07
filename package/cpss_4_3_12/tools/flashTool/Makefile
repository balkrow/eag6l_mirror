##
# Example How to Compile this Tool for AARCH64v8:
# make CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/aarch64v8-marvell-linux-gnu-5.2.1_x86_64_20151110/bin/aarch64-marvell-linux-gnu-
#
# for ARMv7:
# make CROSS_COMPILE=/swtools/devtools/gnueabi/arm_le/arm-mv7_sft-linux-gnueabi/bin/arm-marvell-linux-gnueabi-
#

CROSS_COMPILE ?= /swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/aarch64v8-marvell-linux-gnu-5.2.1_x86_64_20151110/bin/aarch64-marvell-linux-gnu-

BASE_DIR   = .
SFLASH_DIR = $(BASE_DIR)/sflash
SPI_DIR    = $(BASE_DIR)/spi

BUILD_DIR := objects

CFLAGS += -O0 -g -DCONFIG_ALDRIN -DCONFIG_FALCON -Wall

TOOL_NAME = flashTool

CC = $(CROSS_COMPILE)gcc

INCLUDES = \
        -I. \
        -I $(SFLASH_DIR) \
        -I $(SPI_DIR) \
        -I $(BASE_DIR)

FILES = \
  flashTool.c \
  $(SPI_DIR)/mvSpi.c \
  $(SPI_DIR)/mvSpiCmnd.c \
  $(SFLASH_DIR)/mvSFlash.c \
  fileOps.c \
  imgHdr.c \
  crc32.c  \
  smi_drv.c \
  slave_smi.c

VPATH = $(sort $(dir $(FILES)))

OBJS = $(notdir $(FILES:.c=.o))
OBJS_BUILD = $(addprefix $(BUILD_DIR)/, $(OBJS))

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	echo CC $<
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $(BUILD_DIR)/$*.o

all: $(OBJS_BUILD)
	echo LD $(TOOL_NAME)
	$(CC) $(OBJS_BUILD) -o $(TOOL_NAME)

clean:
	@rm -rf objects/ $(TOOL_NAME)

# debug target. usage: make print-var will print value of variable "var"
print-%  : ; @echo $* = $($*)

help:
	@echo
	@echo "flash TOOL build "
	@echo "================="
	@echo
	@echo "Usage: make [CROSS_COMPILE=...] [-s]"

