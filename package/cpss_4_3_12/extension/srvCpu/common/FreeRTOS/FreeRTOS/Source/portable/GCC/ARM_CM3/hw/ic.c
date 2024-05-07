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

#include "FreeRTOS.h"
#include "hw.h"
#include "ic.h"
#include <portmacro.h>

extern void InterruptHandler(void);
#define REG_WRITE(reg, val) \
            ( *(( volatile unsigned long * )( reg ))) = (val)

#define REG_READ(reg) \
            ( *(( volatile unsigned long * )( reg )))

static inline void vICRegWrite( long reg, long irq_mask )
{
    REG_WRITE(reg, irq_mask);
}
static inline long vICRegRead( long reg )
{
    return REG_READ(reg);
}


static inline void vICSetHandler( long interrupt, nvichandler func )
{
    /* set excption table - start at address 0 */
    ( *(( volatile unsigned long * ) ( interrupt * 4 ))) = ( unsigned long )func;
}


long iICRegisterHandler(int irq_vec_offset, long interrupt, nvichandler func,
                        long enable, long priority )
{
    long ret;

    if (( interrupt < 0 ) || ( interrupt >= ICMAX_INT ))
        return -1;
    if ( func == NULL )
        return -2;

    ret = iICDisableInt( interrupt );

    vICSetHandler(irq_vec_offset, func);
    iICPrioritySet(interrupt, priority);

    if ( enable )
        ret = iICEnableInt( interrupt );

    return ret;
}

inline void iICSetupInt( long interrupt, long secure, long priority,
                         long edge, long enable )
{
    if (( interrupt < 0 ) || ( interrupt >= ICMAX_INT ))
        return;

    if ( enable )
        iICEnableInt( interrupt );
    else
        iICDisableInt( interrupt );
}

long iICUnregisterHandler(int irq_vec_offset, long interrupt )
{
    long ret;

    ret = iICDisableInt( interrupt );
    vICSetHandler(irq_vec_offset, InterruptHandler);

    return ret;
}

long iICEnableInt( long interrupt )
{

    if (( interrupt < 0 ) || ( interrupt >= ICMAX_INT ))
        return -1;

    vICRegWrite(ICREG_NVIC_ISER(interrupt / 32), ( 1 << (interrupt % 32)));

    return 0;
}

long iICDisableInt( long interrupt )
{
    if (( interrupt < 0 ) || ( interrupt >= ICMAX_INT ))
        return -1;

    vICRegWrite(ICREG_NVIC_ICER(interrupt / 32), ( 1 << (interrupt % 32)));

    return 0;
}

long iICPrioritySet( long interrupt, long priority )
{
    long value, m;

    if (( interrupt < 0 ) || ( interrupt >= ICMAX_INT ) ||
                (priority < 0) || (priority > 0xff))
        return -1;

    /* The byte offset of the required Priority field in the register is: interrupt MOD 4 */
    m = interrupt % 4;
    value = vICRegRead(ICREG_NVIC_IPR(interrupt / 4));
    value &= ~(0xff << (m * 8));
    value |= (priority << (m * 8));
    vICRegWrite(ICREG_NVIC_IPR(interrupt / 4), value);

    return 0;
}

void vICInit( void )
{
}

