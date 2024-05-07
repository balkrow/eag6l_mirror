.thumb
.syntax unified

cache_MPU_init: .global cache_MPU_init
                      @ Invalidate entire data cache:
.equ CCSIDR,0xE000ED80 @ Cache size ID register address
.equ CSSELR,0xE000ED84 @ Cache size selection register address
.equ DCISW,0xE000EF60 @ Cache maintenance op address: data cache clean and invalidate by set/way
                      @ CSSELR selects the cache visible in CCSIDR
MOV r0, #0            @ 0 = select level 1 data cache
LDR r11, =CSSELR      @
STR r0, [r11]         @
DSB                   @ Ensure write to CSSELR before proceeding
LDR r11, =CCSIDR      @ From CCSIDR
LDR r2, [r11]         @ Read data cache size information
AND r1, r2, #7        @ r1 = cache line size
ADD r7, r1, #4        @ r7 = number of words in a cache line
UBFX r4, r2, #3, #10  @ r4 = number of ways-1 of data cache
UBFX r2, r2, #13, #15 @ r2 = number of set-1 of data cache
CLZ r6, r4            @ calculate bit offset for way in DCISW
LDR r11, =DCISW       @ invalidate cache by set/way

inv_loop1:            @ For each set
MOV r1, r4            @ r1 = number of ways-1
LSLS r8, r2, r7       @ shift set value to bit 5 of r8

inv_loop2:            @ For each way
LSLS r3, r1, r6       @ shift way value to bit 30 in r6
ORRS r3, r3, r8       @ merge way and set value for DCISW
STR r3, [r11]         @ invalidate D-cache line
SUBS r1, r1, #1       @ decrement way
BGE inv_loop2         @ End for each way
SUBS r2, r2, #1       @ Decrement set
BGE inv_loop1         @ End for each set
DSB                   @ Data sync barrier after invalidate cache
ISB                   @ Instruction sync barrier after invalidate cache

                      @ Invalidate instruction cache
.equ ICIALLU,0xE000EF50
MOV r0, #0
LDR r11, =ICIALLU
STR r0, [r11]
DSB
ISB

                      @ Enabling data and instruction caches
.equ CCR,0xE000ED14
LDR r11, =CCR
LDR r0, [r11]
ORR r0, r0, #(1<<16) @ set CCR.DC field
ORR r0, r0, #(1<<17) @ set CCR.IC field
STR r0, [r11]
DSB
ISB

enable_TCM: .global enable_TCM
                      @ Enabling TCM
.equ CM7_ITCMCR,0xE000EF90
.equ CM7_DTCMCR,0xE000EF94

@LDR r11, =CM7_ITCMCR only enable DTCM
@LDR r0, [r11]
@ORR r0, r0, #1
@STR r0, [r11]
LDR r11, =CM7_DTCMCR
LDR r0, [r11]
ORR r0, r0, #1
STR r0, [r11]
DSB
ISB

.equ DTCM_ADDR,0x20000000
.equ DTCM_SZ,0x00008000
.equ SRAM_ADDR,0x01000000
.equ EHSM_CMD_ADDR,0x40000
.equ PERIPHERAL_BASE_ADDR,0x40000000 @ This is where the switch core and DFX registers will be remapped to
                                     @ Using the CM7 Address Peripheral decoder
@ /Cider/EBU/Seahawk/Seahawk {Current}/Tile/<CnM>CNM_IP/<CnM> <CM7 Cluster>CM7_RFU/<CM7 Cluster> Address Decoder/<CM7 Periph AD> Address Decoder
.equ CNM_ADDR,0x5C000000
                      @ Set Stack pointer to point to TCM
		      @ Set to DTCM0: (TODO: determine own CPU index and add 0x10000 * cpu_idx to DTCM_ADDR which is the base)
LDR sp, =(DTCM_ADDR + DTCM_SZ - 0x10)
                      @ Basic, initial MPU settings (only minimal address ranges required for tile #0):
.equ MPU_CTRL,0xE000ED94
.equ MPU_RNR,0xE000ED98
.equ MPU_RBAR,0xE000ED9C
.equ MPU_RASR,0xE000EDA0

LDR r11, =MPU_RNR
MOV r0, #0
STR r0, [r11]         @ set MPU region #0 to be programmed

LDR r11, =MPU_RBAR
LDR r0,  =EHSM_CMD_ADDR
STR r0, [r11]         @ eHSM CMD base address

LDR r11, =MPU_RASR
LDR r0,  =0x1301001F  @ XN=1, AP=3 (full access R/W), 2^16 size, enabled, shareable, device
STR r0, [r11]         @ write attributes

LDR r11, =MPU_RNR
MOV r0, #1
STR r0, [r11]         @ set MPU region #1 to be programmed

LDR r11, =MPU_RBAR
LDR r0,  =DTCM_ADDR
STR r0, [r11]         @ DTCM base address

LDR r11, =MPU_RASR
LDR r0,  =0x030C0025  @ AP=3 (full access R/W), 2^19 size, enabled, shareable, normal non-cachable
STR r0, [r11]         @ write attributes

LDR r11, =MPU_RNR
MOV r0, #2
STR r0, [r11]         @ set MPU region #2 to be programmed

LDR r11, =MPU_RBAR
LDR r0,  =SRAM_ADDR
STR r0, [r11]         @ SRAM base address

LDR r11, =MPU_RASR
LDR r0,  =0x0306002B  @ AP=3 (full access R/W), 2^22 size, enabled, shareable, normal inner/outer write through, no write allocate
STR r0, [r11]         @ write attributes

LDR r11, =MPU_RNR
MOV r0, #3
STR r0, [r11]         @ set MPU region #3 to be programmed

LDR r11, =MPU_RBAR
LDR r0,  =PERIPHERAL_BASE_ADDR
STR r0, [r11]         @ Switch base address

LDR r11, =MPU_RASR
LDR r0,  =0x13010039  @ XN=1, AP=3 (full access R/W), 2^29 size, enabled, shareable, device
STR r0, [r11]         @ write attributes

LDR r11, =MPU_RNR
MOV r0, #4
STR r0, [r11]         @ set MPU region #4 to be programmed

LDR r11, =MPU_RBAR
LDR r0,  =CNM_ADDR
STR r0, [r11]         @ CnM base address

LDR r11, =MPU_RASR
LDR r0,  =0x13010031  @ XN=1, AP=3 (full access R/W), 2^25 size, enabled, shareable, device
STR r0, [r11]         @ write attributes
DSB
ISB

                      @ PPB access should bypass the MPU according to ARM documentation, so no entry needed for it
		      @ Vendor at 0xe0100000 is not currently used; if needed, add an entry for it.
LDR r11, =MPU_CTRL
MOV r0, #1
STR r0, [r11]         @ Enables MPU, no fallback to default map, bypass MPU in hard fault handlers
DSB
ISB

BX  lr                @ Interworking return from subroutine
