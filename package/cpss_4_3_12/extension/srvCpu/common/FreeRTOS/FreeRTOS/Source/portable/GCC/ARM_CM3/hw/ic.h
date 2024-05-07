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

#ifndef IC_H
#define IC_H

#define ICMAX_INT               ( 240 )

/* Interrupt enable/disable */
#define IRQ_ENABLE  ( 1 )
#define IRQ_DISABLE ( 0 )

#define IRQ_PRIO_DEFAULT    ( 0 )

/* NVIC registers */
#define ICREG_NVIC_ISER(n)          ( 0xE000E100 + (n)*4)
#define ICREG_NVIC_ICER(n)          ( 0xE000E180 + (n)*4)
#define ICREG_NVIC_ISPR(n)          ( 0xE000E200 + (n)*4)
#define ICREG_NVIC_ICPR(n)          ( 0xE000E280 + (n)*4)
#define ICREG_NVIC_IPR(n)           ( 0xE000E400 + (n)*4)

/* Interrupt nvic vector mapping, e.g. NVIC_GLOBAL_IRQ_0 (16)
 * results with (16 * 4) 0x40 offset in the CM3 NVIC vector table */
#define NVIC_GLOBAL_IRQ(n)  ( 16 + n )

typedef void ( *nvichandler )( void );

long iICRegisterHandler(int irq_vec_offset, long interrupt, nvichandler func,
                        long enable, long config );

inline void iICSetupInt( long interrupt, long secure, long priority,
                         long edge, long enable );
long iICUnregisterHandler(int irq_vec_offset, long interrupt );
void vICInit( void );
long iICEnableInt( long interrupt);
long iICDisableInt( long interrupt );
long iICPrioritySet( long interrupt, long priority );

void tfp_printf( char *fmt, ... );
void cm3_interrupt_init(void);
void cm3_to_msys_doorbell(void);
#endif
