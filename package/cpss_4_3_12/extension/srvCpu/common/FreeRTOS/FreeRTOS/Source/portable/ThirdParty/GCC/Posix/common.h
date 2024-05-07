/*******************************************************************************
   Copyright (C) Marvell International Ltd. and its affiliates
********************************************************************************
Marvell GNU General Public License FreeRTOS Exception

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the Lesser
General Public License Version 2.1 plus the following FreeRTOS exception.
An independent module is a module which is not derived from or based on
FreeRTOS.
Clause 1:
Linking FreeRTOS statically or dynamically with other modules is making a
combined work based on FreeRTOS. Thus, the terms and conditions of the GNU
General Public License cover the whole combination.
As a special exception, the copyright holder of FreeRTOS gives you permission
to link FreeRTOS with independent modules that communicate with FreeRTOS solely
through the FreeRTOS API interface, regardless of the license terms of these
independent modules, and to copy and distribute the resulting combined work
under terms of your choice, provided that:
1. Every copy of the combined work is accompanied by a written statement that
details to the recipient the version of FreeRTOS used and an offer by yourself
to provide the FreeRTOS source code (including any modifications you may have
made) should the recipient request it.
2. The combined work is not itself an RTOS, scheduler, kernel or related
product.
3. The independent modules add significant and primary functionality to
FreeRTOS and do not merely extend the existing functionality already present in
FreeRTOS.
Clause 2:
FreeRTOS may not be used for any competitive or comparative purpose, including
the publication of any form of run time or compile time metric, without the
express permission of Real Time Engineers Ltd. (this is the norm within the
industry and is intended to ensure information accuracy).
*******************************************************************************/

#ifndef POSIX_COMMON_H

#define MV_CPU_LE

#define DEFAULT_DEV0_NUM    ( 0 )
#define DEFAULT_DEV1_NUM    ( 1 )

#define SZ_1M               ( 0x100000 )
#define SZ_1K               ( 0x400 )

#ifndef __ASSEMBLY__
#define POSIX_COMMON_H

#include <stdlib.h>
#include <FreeRTOS.h>
#include "errno.h"

/* "Mbus clock" in terms of SoC department */
#if defined(CONFIG_BOBK)
#define CONFIG_SYS_TCLK     ( 200000000 )
#elif (defined (CONFIG_ALDRIN) || defined (CONFIG_BC3) || defined (CONFIG_PIPE)|| defined (CONFIG_ALDRIN2) || defined (CONFIG_FALCON))
#define CONFIG_SYS_TCLK     ( 250000000 )
#elif defined (CONFIG_RAVEN)
#define CONFIG_SYS_TCLK     ( 167000000 )
#elif defined (CONFIG_AC5)
#define CONFIG_SYS_TCLK     ( 328000000 )
#elif defined (CONFIG_AC5P)
#define CONFIG_SYS_TCLK     ( 328000000 ) /*TBD for AC5P*/
#elif defined (CONFIG_HARRIER)
#define CONFIG_SYS_TCLK     ( 328000000 ) /*TBD for Harrier*/
#elif defined (CONFIG_IRONMAN)
#define CONFIG_SYS_TCLK     ( 332000000 ) /*TBD for IRONMAN*/
#endif

/* TBD - must be fixed. Can be different in bobK, BC3, Aldrin, pipe */
#define MV_BOARD_REFCLK_25MHZ       ( 25000000 )

/* Function wrappers */
#define CPUID                   4
#define whoAmI()                CPUID
#define master_cpu_id           CPUID   /* XXX only single core supported */
#define mvOsMemset( p, v, s )   memset(( void * )( p ), ( long )v, ( size_t )s )
#define udelay( us )            UDELAY( us )
#define mvOsPrintf              printf
#define dmb()               __asm__ __volatile__ ("dmb" : : : "memory")

/* Register offsets */

/* Common definitions */
/* The following is a list of Marvell status    */
#define MV_ERROR		    (-1)
#define MV_OK			    (0x00)  /* Operation succeeded                   */
#define MV_FAIL			    (0x01)	/* Operation failed                      */
#define MV_BAD_VALUE        (0x02)  /* Illegal value (general)               */
#define MV_OUT_OF_RANGE     (0x03)  /* The value is out of range             */
#define MV_BAD_PARAM        (0x04)  /* Illegal parameter in function called  */
#define MV_BAD_PTR          (0x05)  /* Illegal pointer value                 */
#define MV_BAD_SIZE         (0x06)  /* Illegal size                          */
#define MV_BAD_STATE        (0x07)  /* Illegal state of state machine        */
#define MV_SET_ERROR        (0x08)  /* Set operation failed                  */
#define MV_GET_ERROR        (0x09)  /* Get operation failed                  */
#define MV_CREATE_ERROR     (0x0A)  /* Fail while creating an item           */
#define MV_NOT_FOUND        (0x0B)  /* Item not found                        */
#define MV_NO_MORE          (0x0C)  /* No more items found                   */
#define MV_NO_SUCH          (0x0D)  /* No such item                          */
#define MV_TIMEOUT          (0x0E)  /* Time Out                              */
#define MV_NO_CHANGE        (0x0F)  /* Parameter(s) is already in this value */
#define MV_NOT_SUPPORTED    (0x10)  /* This request is not support           */
#define MV_NOT_IMPLEMENTED  (0x11)  /* Request supported but not implemented */
#define MV_NOT_INITIALIZED  (0x12)  /* The item is not initialized           */
#define MV_NO_RESOURCE      (0x13)  /* Resource not available (memory ...)   */
#define MV_FULL             (0x14)  /* Item is full (Queue or table etc...)  */
#define MV_EMPTY            (0x15)  /* Item is empty (Queue or table etc...) */
#define MV_INIT_ERROR       (0x16)  /* Error occured while INIT process      */
#define MV_HW_ERROR         (0x17)  /* Hardware error                        */
#define MV_TX_ERROR         (0x18)  /* Transmit operation not succeeded      */
#define MV_RX_ERROR         (0x19)  /* Recieve operation not succeeded       */
#define MV_NOT_READY	    (0x1A)	/* The other side is not ready yet       */
#define MV_ALREADY_EXIST    (0x1B)  /* Tried to create existing item         */
#define MV_OUT_OF_CPU_MEM   (0x1C)  /* Cpu memory allocation failed.         */
#define MV_NOT_STARTED      (0x1D)  /* Not started yet         */
#define MV_BUSY             (0x1E)  /* Item is busy.                         */
#define MV_TERMINATE        (0x1F)  /* Item terminates it's work.            */
#define MV_NOT_ALIGNED      (0x20)  /* Wrong alignment                       */
#define MV_NOT_ALLOWED      (0x21)  /* Operation NOT allowed                 */
#define MV_WRITE_PROTECT    (0x22)  /* Write protected                       */

typedef signed char             int8_t;

typedef unsigned char           MV_U8;
typedef unsigned short          MV_U16;
typedef unsigned int            MV_U32;
typedef int                     MV_32;
typedef void                    MV_VOID;
/*typedef long                    MV_BOOL;*/
typedef unsigned char           MV_BOOL;

typedef long                    MV_STATUS;
typedef unsigned long           MV_UINTPTR_T;
/* The STM32 libraries already defines the bool type */
#if !defined(CONFIG_STM32) && !defined(CONFIG_MV_IPC_FREERTOS_DRIVER)
typedef long                    bool;
#endif

#define MV_FALSE	0
#define MV_TRUE     (!(MV_FALSE))

typedef enum { BC2_E, AC3_E, BOBK_E, ALDRIN_E} dev_type_ent;
#include "mvCommon.h"

#define MV_BOARD_TCLK           250000000

#define IN
#define OUT

#define WRITE_FLASH_BUFF_SIZE   0x100           //256 bytes


/* MG base address */
extern unsigned long sim_mg_base;
extern unsigned char *g_scpu_sram_ptr;
extern unsigned char *g_cnm_sram_ptr;

#define MG_BASE         (sim_mg_base)

#define SCPU_SRAM_BASE  (g_scpu_sram_ptr)
#define CNM_SRAM_BASE   (g_cnm_sram_ptr)

/* Register access macros */
#define MSS_REGS_BASE 0xa0000000

#ifdef CONFIG_A8K
#define MSS_REGS_BASE 0x40500000
#endif

#define INTER_REGS_BASE MSS_REGS_BASE

#ifdef CONFIG_RAVEN
#define SD1_REGS_BASE           (INTER_REGS_BASE  + 0x00100000)
#define GOP0_REGS_BASE          (SD1_REGS_BASE    + 0x00100000)
#define GOP1_REGS_BASE          (GOP0_REGS_BASE   + 0x00100000)
#define SERDES_REGS_BASE        (GOP1_REGS_BASE   + 0x00100000)
#define DFX_REGS_BASE           (SERDES_REGS_BASE + 0x00100000)
#endif


#define MSSREG( reg )                       \
            ( *(( volatile unsigned long * )( MSS_REGS_BASE | ( reg ))))

#define MSS_REG_WRITE( offset, val )        \
                        ( MSSREG( offset ) = ( val ))
#define MSS_REG_READ( offset )          \
                        ( MSSREG( offset ))

#define MSS_REG_BIT_SET( reg, mask )        \
                        MSSREG( reg ) = MSSREG( reg ) | ( mask )
#define MSS_REG_BIT_RESET( reg, mask )  \
                        MSSREG( reg ) = MSSREG( reg ) & ~( mask )


#define MSS_REG_BYTE( reg )             \
    ( *(( volatile unsigned char * )( MSS_REGS_BASE | ( reg ))))

#define MV_REG_BYTE_WRITE( offset, val )        \
                        ( MSS_REG_BYTE( offset ) = ( val ))
#define MV_REG_BYTE_READ( offset )              \
                        ( MSS_REG_BYTE( offset ))

/* Basic delay macros */
#define MDELAY( ms )    do {                                        \
                            long j = ( ms );                        \
                            for ( ; j > 0; j-- )                    \
                                UDELAY( 1000 );                     \
                        } while ( 0 )

#define UDELAY( us )    do {                                            \
                            unsigned long i;                            \
                            i = configCPU_CLOCK_HZ / 4000000 * us;      \
                            asm volatile(   "1:     \n\t"               \
                                            "NOP                \n\t"   \
                                            "SUB    %0, %0, #1  \n\t"   \
                                            "CMP    %0, #0      \n\t"   \
                                            "BNE    1b          \n\t"   \
                                            : "+r" ( i ) : );           \
                        } while ( 0 )


#endif
#endif
