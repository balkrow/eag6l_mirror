ifneq ($(SIM),y)
  CROSS_COMPILE ?= /swtools/devtools/gnueabi/arm_le/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu-
endif

CC = $(CROSS_COMPILE)gcc
APP_NAME := efuse

CFLAGS += -Wall -g -ansi -lm -DLINUX
LDFLAGS = -lpthread -lrt
CPSS_ROOT = $(realpath ../..)

ifeq ($(DEBUG),y)
  CFLAGS += -DDEBUG
endif

ifeq ($(SIM),y)
        CFLAGS += -DSIM
endif



SRCS := main_efuse_tool.c efuse_tool.c

OBJS = $(notdir $(SRCS:.c=.o))

print-%: ; @echo $*=$($*)

all: $(APP_NAME)
	@ echo Done

%.o : %.c efuse_tool.h
	@echo CC $@
	$(CC) $(CFLAGS) -c $< -o $@

$(APP_NAME): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(APP_NAME)

clean:
	rm -f $(OBJS) $(APP_NAME)

print-%: ; @echo $*=$($*)

help:
	@echo
	@echo "EFUSE TOOL build "
	@echo "================="
	@echo
	@echo "Usage: make [SIM=n] [DEBUG=n]"
	@echo
	@echo "Optional flags:"
	@echo  "  [SIM=y]   - Host simulation build. No cross compiler used. Tool will not access HW."
	@echo  "  [DEBUG=y] - Prints additional read/write data that may help to debug."
	@echo


