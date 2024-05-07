#ifndef __INFlashToolH
#define __INFlashToolH

#include <stdint.h>
#include <stdbool.h>
/* Defines */

/* The following is a list of Marvell status    */
#define MV_ERROR            (-1)
#define MV_OK               (0) /* Operation succeeded                   */
#define MV_FAIL             (1) /* Operation failed                      */
#define MV_BAD_VALUE        (2) /* Illegal value (general)               */
#define MV_OUT_OF_RANGE     (3) /* The value is out of range             */
#define MV_BAD_PARAM        (4) /* Illegal parameter in function called  */
#define MV_BAD_PTR          (5) /* Illegal pointer value                 */
#define MV_BAD_SIZE         (6) /* Illegal size                          */
#define MV_BAD_STATE        (7) /* Illegal state of state machine        */
#define MV_SET_ERROR        (8) /* Set operation failed                  */
#define MV_GET_ERROR        (9) /* Get operation failed                  */
#define MV_CREATE_ERROR     (10)    /* Fail while creating an item           */
#define MV_NOT_FOUND        (11)    /* Item not found                        */
#define MV_NO_MORE          (12)    /* No more items found                   */
#define MV_NO_SUCH          (13)    /* No such item                          */
#define MV_TIMEOUT          (14)    /* Time Out                              */

#define MV_FALSE    0
#define MV_TRUE     (!(MV_FALSE))

#define IN
#define OUT

extern void *pp_space_ptr;
#define INTER_REGS_BASE     0x3d000000
#define CFG_DFL_MV_REGS     0xD0000000        /* boot time MV_REGS */

/*TODO: isolate all none generic defs*/
#define MV_BOARD_TCLK           250000000
#define MV_CPU_LE
#define MV_16BIT_LE(X)  (X)
#define MV_32BIT_LE(X)  (X)
/************************************/


typedef uint32_t MV_U32;
typedef uint16_t MV_U16;
typedef uint64_t MV_U64;
typedef uint8_t MV_U8;
typedef bool MV_BOOL;
typedef int MV_STATUS;
typedef void MV_VOID;
typedef unsigned long MV_UINTPTR_T;
enum flash_driver_errors_enum {
    ERROR_UNINITILIZED_E
};

extern void write_register(uint32_t address, uint32_t value);
extern uint32_t read_register(uint32_t address);

/*MACROS*/
#define MV_REG_READ(address)             \
        read_register(INTER_REGS_BASE + address)

#define MV_REG_WRITE(address, value)    \
        write_register(INTER_REGS_BASE + address, value)

#define MV_MEMIO32_WRITE(addr, data)   \
        write_register(addr, data)

#define MV_MEMIO32_READ(addr)          \
        read_register(addr)

#define MV_REG_BIT_SET(offset, bitMask)                 \
        (MV_MEMIO32_WRITE((INTER_REGS_BASE + (offset)), \
         (MV_MEMIO32_READ((INTER_REGS_BASE + (offset))) | \
          bitMask)))

#define MV_REG_BIT_RESET(offset,bitMask)                \
        (MV_MEMIO32_WRITE((INTER_REGS_BASE + (offset)), \
         (MV_MEMIO32_READ((INTER_REGS_BASE + (offset))) & \
          MV_32BIT_LE(~(bitMask)))))

#define mvOsOutputString printf
#define mvOsPrintUint(manf) printf("%u\n", manf)

#define NOR_FLASH_NOT_EXIST
#define STR_TO_BYTES_ERR "ERROR CONVERTING STR TO BYTES"

#endif /*__INFlashToolH */
