# requirements:
# CROSS_COMPILE
# SHM_UART - if y, shm-uart is enabled, default is no
# SHM_UART_BASE - base address in CM3 SRAM for shm-uart
# SHM_UART_SIZE - size of a single shm-uart ring (Rx/Tx). Both rings occupy x2
# QSPI - if y, QSPI is enabled, default is no
# SDK_DEBUG - if y, SDK_DEBUG options is enabled, default is no
# WATCHDOG - if y,  watchdog mechanism is enabled, default is no
# XOR_DMA - if y, XOR_DMA is enabled, default is no
# SDMA_SUPPORT - if y,  SDMA engine support is enabled, default is no
# PHA_SUPPORT - y for programable header alteration FW load support. Default is no
# CLI_ENHANCED - enable rich CLI with history and esc seq support
# IPC_CLIENT -   enable IPC between two SCPUs

ifeq ($(ASIC_SIMULATION),y)
CROSS_COMPILE=
CPU=generic
THUMB_LD_FLAGS=
THUMB_FLAGS=
ifeq ($(ADDR_SANITIZE),y)
CFLAGS=-fsanitize=address -fno-omit-frame-pointer -static-libgcc -static-libasan -lrt -lc
LINKER_FLAGS?=-fsanitize=address -lrt -lpthread -lc -lgcc -lasan -lrt -lc -lpthread -lgcc
else
LINKER_FLAGS?=-lrt -lpthread -lc
endif
PORT_DIR_EXT=portable/ThirdParty/GCC
PORT_SPECIFIC_EXT=Posix
IPC_CLIENT?=y
WATCHDOG?=y
CLI_ENHANCED?=y
SHM_UART?=y
SHM_UART_BASE?=0x1f000
SHM_UART_SIZE?=256
SDK_DEBUG?=y
endif

CROSS_COMPILE?=/swtools/devtools/gnueabi/arm_le/arm-none-linux-gnueabi-versions/gcc-arm-none-eabi-4_9-2015q1/bin/arm-none-eabi-
APP_NAME := scpu_sdk
#
# Host configuration
#
CPU ?= cortex-m3
PORT_DIR_EXT ?= portable/GCC
PORT_SPECIFIC_EXT ?= ARM_CM3
ifeq ($(AAS),y)
PORT_SPECIFIC_EXT := ARM_CM7
endif
DEP :=
CPSS_DIR := $(realpath $(shell pwd)/../../../../..)
SIM_DIR = $(CPSS_DIR)/simulation
SIM_DIST_DIR = $(SIM_DIR)/simGeneral/h/asicSimulation/SDistributed
SRV_CPU_DIR = $(CPSS_DIR)/extension/srvCpu
COMMON_DRIVERS_DIR =  $(SRV_CPU_DIR)/common/drivers
RTOS_DIR =  $(SRV_CPU_DIR)/common/FreeRTOS/FreeRTOS
RTOS_SOURCE_DIR = $(RTOS_DIR)/Source
PORT_DIR=$(RTOS_SOURCE_DIR)/$(PORT_DIR_EXT)
PORT_SPECIFIC_DIR=$(PORT_DIR)/$(PORT_SPECIFIC_EXT)
PORT_ROOT=$(RTOS_SOURCE_DIR)/portable
PORT_DIR_CM3 = $(RTOS_SOURCE_DIR)/portable/GCC
PORT_CM3_DIR = $(PORT_DIR_CM3)/$(PORT_SPECIFIC_EXT)
I2C_DIR = $(SRV_CPU_DIR)/common/drivers/peripherals/i2c
RTOS_PLUS_DIR = $(SRV_CPU_DIR)/common/FreeRTOS/FreeRTOS-Plus
BUILD_DIR := objects

HEADER_BUILD = ../ARM_CM3_GCC/header_build

CFLAGS += -DCONFIG_BC3 -DCM3 -DCM3_SDK

# Commands for calling the compiler, archiver, linker...
CC = $(CROSS_COMPILE)gcc
ifeq ($(ASIC_SIMULATION),y)
LD = $(CROSS_COMPILE)gcc
else
LD = $(CROSS_COMPILE)ld
endif
AR = $(CROSS_COMPILE)ar
OBJCOPY = $(CROSS_COMPILE)objcopy
ELFSIZE = $(CROSS_COMPILE)size
MAKE = make

#
# CFLAGS common to both the THUMB and ARM mode builds
#
WARNINGS = -Wall -Wshadow -Wpointer-arith -Wbad-function-cast \
		 -Wcast-align -Wsign-compare -Waggregate-return -Wstrict-prototypes \
		 -Wmissing-prototypes -Wmissing-declarations -Wunused

# debug target. usage: make print-var will print value of variable "var"
print-%  : ; @echo $* = $($*)

CORE_CFLAGS = \
         -ffunction-sections -g -s \
         -I . -I $(PORT_SPECIFIC_DIR) -I $(PORT_SPECIFIC_DIR)/hw -I $(I2C_DIR) \
         -I $(RTOS_SOURCE_DIR)/include \
         -I $(RTOS_PLUS_DIR)/ \
         -I $(COMMON_DRIVERS_DIR)/operationDrivers \
         -I $(COMMON_DRIVERS_DIR)/peripherals/spi \
         -I $(COMMON_DRIVERS_DIR)/peripherals/timer \
         -I $(COMMON_DRIVERS_DIR)/infrastructure/doorbell \
         -I $(COMMON_DRIVERS_DIR)/infrastructure/ipc \
         -I $(COMMON_DRIVERS_DIR)/infrastructure/cli \
         -I $(COMMON_DRIVERS_DIR)/peripherals/watchdog \
         -I $(COMMON_DRIVERS_DIR)/peripherals/xorDma \
         -I $(COMMON_DRIVERS_DIR)/peripherals/gpio \
         -I $(COMMON_DRIVERS_DIR)/peripherals/sdma \
         -I $(COMMON_DRIVERS_DIR)/peripherals/mdio
ifneq ($(ASIC_SIMULATION),y)
  CORE_CFLAGS += -I $(PORT_CM3_DIR)/hw
endif

CFLAGS += $(WARNINGS) -mcpu=$(CPU) -MD -D inline= \
         -ffunction-sections -g -s \
         -I . -I $(SRV_CPU_DIR)/firmware/cm3_sdk/code/h

CFLAGS += -DTHUMB_INTERWORK

THUMB_FLAGS ?= -mthumb -Wa,-mthumb -mno-thumb-interwork -mfpu=vfp -msoft-float  -mfix-cortex-m3-ldrd
THUMB_LD_FLAGS ?= -mthumb

ifeq ($(DISABLE_CLI),y)
  CFLAGS += -DDISABLE_CLI
endif

ENTRY = Reset
LINKER_FLAGS ?= -T RTOSDemo-cm3.ld --entry $(ENTRY) -Map scpu_sdk.map --gc-sections
ifeq ($(AAS),y)
LINKER_FLAGS = -T RTOSDemo-cm7.ld --entry $(ENTRY) -Map scpu_sdk.map --gc-sections
endif

#enforce C89-style variable declarations in gcc
CFLAGS += -Werror=declaration-after-statement
#forbids variable len arrays, C89-style, prevent compilation fail with VC10
CFLAGS += -Werror=vla

CFLAGS += -Wno-unused-function
CFLAGS += -Werror

CFLAGS += -DMV_HWS_REDUCED_BUILD -DMV_HWS_REDUCED_BUILD_EXT_CM3
ifeq ($(ASIC_SIMULATION),y)
  CFLAGS += -DASIC_SIMULATION
  CFLAGS += -I $(PORT_SPECIFIC_DIR)
  CFLAGS += -I $(SIM_DIST_DIR)
else
  CFLAGS += -Os
endif

ifeq ($(AAS),y)
  CFLAGS += -DCONFIG_AAS

ifeq ($(EMULATOR),y)
  CFLAGS += -DAAS_EMULATOR
endif
endif

#
# Source files that can be built to THUMB mod.
#
CORE_SRC = \
  $(COMMON_DRIVERS_DIR)/operationDrivers/srvCpuServices.c \
  $(COMMON_DRIVERS_DIR)/infrastructure/ipc/srvCpuIpc.c \
  $(COMMON_DRIVERS_DIR)/infrastructure/ipc/mvShmIpc.c \
  $(COMMON_DRIVERS_DIR)/infrastructure/doorbell/srvCpuDoorbell.c \
  $(COMMON_DRIVERS_DIR)/peripherals/gpio/srvCpuGpio.c \
  $(COMMON_DRIVERS_DIR)/peripherals/mdio/srvCpuMdio.c \
  $(RTOS_SOURCE_DIR)/list.c \
  $(RTOS_SOURCE_DIR)/queue.c \
  $(RTOS_SOURCE_DIR)/tasks.c \
  $(RTOS_SOURCE_DIR)/timers.c

ifeq ($(ASIC_SIMULATION),y)
  CORE_SRC += \
  $(PORT_SPECIFIC_DIR)/port.c \
  $(PORT_SPECIFIC_DIR)/ic.c \
  $(PORT_SPECIFIC_DIR)/srvCpuTimer_Posix.c \
  $(PORT_SPECIFIC_DIR)/dist_wm_client.c \
  $(PORT_SPECIFIC_DIR)/utils/wait_for_event.c \
  $(PORT_ROOT)/MemMang/heap_3.c
else
  CORE_SRC += \
  $(PORT_CM3_DIR)/exceptions.c \
  $(PORT_CM3_DIR)/port.c \
  $(PORT_CM3_DIR)/heap.c \
  $(PORT_CM3_DIR)/hw/ic.c \
  $(COMMON_DRIVERS_DIR)/peripherals/timer/srvCpuTimer.c
endif

ifeq ($(AAS),y)
  CORE_SRC += $(PORT_SPECIFIC_DIR)/cm7_mem_init.S
endif

ifeq ($(SHM_UART),y)
  CORE_SRC += $(PORT_SPECIFIC_DIR)/hw/mvShmUart.c
  CORE_CFLAGS += -DSHM_UART
  ifdef SHM_UART_BASE
    CORE_CFLAGS += -DSHM_UART_BASE=$(SHM_UART_BASE)
  else
    $(error missing param SHM_UART_BASE)
  endif
  ifdef SHM_UART_SIZE
    CORE_CFLAGS += -DSHM_UART_SIZE=$(SHM_UART_SIZE)
  else
    $(error missing param SHM_UART_SIZE)
  endif
else
  CORE_SRC += $(PORT_CM3_DIR)/hw/uart.c
endif

# Use DEBUG options
ifeq ($(SDK_DEBUG),y)
  CORE_CFLAGS += -DSDK_DEBUG
endif

ifeq ($(XOR_DMA),y)
    CORE_SRC += $(COMMON_DRIVERS_DIR)/peripherals/xorDma/srvCpuXorDma.c
    CORE_CFLAGS += -DXOR_DMA
endif

ifeq ($(QSPI),y)
    ifeq ($(SPI),y)
       $(error cannot use SPI & QSPI simultanously)
    endif
    CORE_SRC += $(COMMON_DRIVERS_DIR)/peripherals/spi/srvCpuQspi.c
    CORE_CFLAGS += -DQSPI
    IS_SPI_QSPI = y
endif

ifeq ($(SPI),y)
    CORE_SRC += $(COMMON_DRIVERS_DIR)/peripherals/spi/prvSpiCmnd.c
    CORE_SRC += $(COMMON_DRIVERS_DIR)/peripherals/spi/srvCpuSpi.c
    CORE_CFLAGS += -DSPI
    IS_SPI_QSPI = y
endif

ifeq ($(WATCHDOG),y)
  CORE_SRC += $(COMMON_DRIVERS_DIR)/peripherals/watchdog/srvCpuWatchdog.c
  CORE_CFLAGS += -DWATCHDOG
endif

TWSI_SUPPORT?=y
ifeq ($(TWSI_SUPPORT),y)
  CORE_SRC += $(I2C_DIR)/srvCpuI2c.c
  CORE_CFLAGS += -DTWSI_SUPPORT
endif

ifneq ($(DISABLE_CLI)),y)
# first, add default features shell
  CORE_SRC += $(RTOS_PLUS_DIR)/FreeRTOS_CLI.c
  CORE_SRC += $(COMMON_DRIVERS_DIR)/peripherals/gpio/srvCpuGpio_shell.c
  CORE_SRC += $(COMMON_DRIVERS_DIR)/infrastructure/doorbell/srvCpuDoorbell_shell.c
  CORE_SRC += $(COMMON_DRIVERS_DIR)/peripherals/mdio/srvCpuMdio_shell.c
# then, adding features according to compilation flag
  ifeq ($(CLI_ENHANCED),y)
    CORE_SRC += $(CURDIR)/src/terminal.c
    CORE_CFLAGS += -DCLI_ENHANCED
    SRC := $(realpath $(SRC))
  endif
  ifeq ($(IS_SPI_QSPI),y)
    CORE_SRC += $(COMMON_DRIVERS_DIR)/peripherals/spi/srvCpuQspi_shell.c
  endif
  ifeq ($(TWSI_SUPPORT),y)
    CORE_SRC += $(I2C_DIR)/srvCpuI2c_shell.c
  endif
  ifeq ($(IPC_CLIENT),y)
    CORE_CFLAGS += -DIPC_CLIENT
    CORE_SRC += $(COMMON_DRIVERS_DIR)/infrastructure/ipc/srvCpuIpc_shell.c
  endif
  ifeq ($(XOR_DMA),y)
    CORE_SRC += $(COMMON_DRIVERS_DIR)/peripherals/xorDma/srvCpuXorDma_shell.c
  endif
  ifeq ($(WATCHDOG),y)
    CORE_SRC += $(COMMON_DRIVERS_DIR)/peripherals/watchdog/srvCpuWatchdog_shell.c
  endif
  ifeq ($(SDMA_SUPPORT),y)
    CORE_SRC += $(COMMON_DRIVERS_DIR)/peripherals/sdma/srvCpuSdma.c
    CORE_CFLAGS += -DSDMA_SUPPORT
  endif
endif

CFLAGS += $(CORE_CFLAGS)

SRC += $(CORE_SRC) \
      $(CURDIR)/src/main.c \
      $(CURDIR)/src/printf.c \
      $(CURDIR)/src/strtoul.c \
      $(COMMON_DRIVERS_DIR)/infrastructure/cli/cli.c

#
# Define all object files.
#
OBJ := $(patsubst $(realpath $(CPSS_DIR))/%.c,$(BUILD_DIR)/%.o,$(SRC))
OBJ := $(patsubst $(realpath $(CPSS_DIR))/%.S,$(BUILD_DIR)/%.o,$(OBJ))
DEP1 = $(OBJ:.o=.d)
DEP = $(filter-out 'cm7_mem_init',$(DEP1))
#
# Get the location of libgcc.a and libc.a
#
LIBGCC=$(shell $(CC) $(THUMB_LD_FLAGS) -mcpu=$(CPU) -print-libgcc-file-name)
LIBC=$(shell $(CC) $(THUMB_LD_FLAGS) -mcpu=$(CPU) -print-file-name=libc.a)
ifeq ($(ASIC_SIMULATION),y)
LINUXLIBS=-lrt -lpthread
else
LINUXLIBS=
endif

HDR_IPC_FLAGS=-S 0x2f000 -s 0x1000

#-----------------------------------------------------------------
# Function compile(cflags, output_folder)
#
# Compile a .c file into .o file using the defined toolchain, including dependecy .d file.
# Object and dependency files will be located together in output_folder.
# dep_cflags     - CFLAGS to use for creating dependency file
# compile_cflags - CFLAGS to use for creating object file
# output_folder  - object file ouptut folder path prefix. Only files
#                  which should be created in this folder, will use
#                  this recipe.
#
# Example: $(Ocall compile $(DEP_CFLAGS),$(CFLAGS),/local/objects)
# Targets: /local/objects/foo/bar/baz.o
#          /local/objects/foo/bar/baz.d
# Source file: foo/bar/baz.c
#-----------------------------------------------------------------

define compile
@# create object folder
	@mkdir -p $(dir $@)
@# create dependency file
	$(CC) -M -c $1 $< -o$2/$*.d
@# Duplicate it
	cp $2/$*.d $2/$*.tmp
@# Create empty rule per file in dependency rule to avoid "No rule to make target ..." Errors
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/d' -e 's/$$/ :/' < $2/$*.d >> $2/$*.tmp
@# Change object filename to include full path
	sed -e 's/$(subst .,\.,$(notdir $@))/$(subst /,\/,$@)/'  < $2/$*.tmp > $2/$*.d
@# Remove temporary file
	rm $2/$*.tmp
@# print compilation msg
	@ echo "  CC $<"
@# compile
	$(CC) -c $1 $< -o$@
endef


#
# The default rule
#
all: $(APP_NAME).bin

$(APP_NAME).bin : $(APP_NAME).elf
	@$(OBJCOPY) -O binary $(APP_NAME).elf $@

$(APP_NAME).elf : $(OBJ)
	@echo "  LD    $@";
	@echo $(LD) $(LINKER_FLAGS) -o $@ $(OBJ) '$(LIBC)' '$(LIBGCC)' $(LINUXLIBS)
	@$(LD) $(LINKER_FLAGS) -o $@ $(OBJ) '$(LIBC)' '$(LIBGCC)' $(LINUXLIBS)
	@SIZE=`$(ELFSIZE) $@ | tail -1|  cut -f 4`; \
	printf "    *** \033[34;1m.text+.data+.bss size:  0x%x \033[0m\n" $$SIZE

$(BUILD_DIR)/%.o: $(realpath $(CPSS_DIR))/%.c
	$(call compile,$(THUMB_FLAGS) $(CFLAGS),$(BUILD_DIR))

$(BUILD_DIR)/%.o: $(realpath $(CPSS_DIR))/%.S
	$(call compile,$(THUMB_FLAGS) $(CFLAGS),$(BUILD_DIR))

clean :
	@rm -rf ./$(BUILD_DIR) $(APP_NAME).elf $(APP_NAME).bin

# debug target. usage: make print-var will print value of variable "var"
print-%  : ; @echo $* = $($*)


-include $(DEP)



