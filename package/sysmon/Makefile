# Compiler flags
CFLAGS = -Wall -Wstrict-prototypes -pipe -O2 -fno-strength-reduce -g
LFLAGS =
INCLUDES = -I. -I../include  
INCLUDES += -I../cpss/cpssEnabler/mainExtDrv/h -I../cpss/mainGaltisWrapper/h
INCLUDES += -I../cpss/common/h -I../cpss/cpssEnabler/mainOs/h
INCLUDES += -I../cpss/cpssEnabler/mainOs/h -I../cpss/cpssEnabler/mainSysConfig/h
INCLUDES += -I../cpss/extension/srvCpu/common/FreeRTOS/FreeRTOS/Source/portable/GCC/ARM_CM7
INCLUDES += -I../cpss/extension/srvCpu/common/FreeRTOS/FreeRTOS/Source/include
INCLUDES += -I../cpss/extension/srvCpu/firmware/cm3_sdk/code/h/
INCLUDES += -I../cpss/mainExtUtils/h -I../cpss/mainPpDrv/h -I../cpss/cpssEnabler/cpssAppUtils/h
INCLUDES += -I../cpss/cpssEnabler/mainCmd/h -I../cpss/cpssEnabler/ezMpdCommon/h

CFLAGS += -DCPU_ARM -DPIC_BUILD -O2 -DCONFIG_V2_DMA -DMCD_RUN_WITH_CPSS -DIMPL_PP -DGT_PCI -D_linux -DCHX_FAMILY -DLINUX -DV2L_PTHREADS -DLINUX_NOKM -DNOKM_DRV_EMULATE_INTERRUPTS -DCONFIG_DDR3 -DDDR_VIEWER_TOOL -DMV_HWS_EXCLUDE_DEBUG_PRINTS -DMPD_VERSION=\""e5e85-dirty"\" -DLUA_USE_MKSTEMP -DCPSS_LOG_ENABLE -DCPSS_API_LOCK_PROTECTION -DCMD_LUA_CLI -DCMD_LUA_NET -DCMD_LUA_GLOBAL -DCMD_LUA_GENWRAPPER -DCMDFS_RAM_EMBEDDED_DATA_EXT -DCMDFS_RAM_EMBEDDED_DATA_EXT_EXAMPLES -DCMDFS_RAM_EMBEDDED_DATA_EXT_XML -DENV_MAINCMD -DCONFIG_DMA_BLK_SIZE_NUM_MB=2 -DCONFIG_DMA_LEN=4 -DPRV_CPSS_MAX_PP_DEVICES_CNS=128 -DCPSS_MAX_PORTS_NUM_CNS=1024 -DCMD_FS

COMMON_LIB = ../cpss/compilation_root/master/armv7_DX/common/libs/COMMON.a
MAINDRV_LIB = ../cpss/compilation_root/master/armv7_DX/mainPpDrv/libs/MAINPPDRV.a ../cpss/compilation_root/master/armv7_DX/cpssEnabler/libs/CPSSENABLER.a


OBJECT = sysmon.c thread.c memory.c pqueue.c sigevent.c memtypes.c cpss.c
PROG_OBJ = devmem.c pid_output.c

all: sysmon

sysmon:  ${OBJECT} ${PROG_OBJ} 
	${CC}  $(CFLAGS) $(DFLAGS) $(INCLUDES) ${COMMON_LIB} ${MAINDRV_LIB}  -lreadline -ltermcap -lrt -lcrypt -o $@ $^
	
clean:
	rm -f sysmon rm *.o
