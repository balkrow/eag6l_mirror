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

#include <FreeRTOS.h>
#include "terminal.h"

char esc_seq[CLI_ESC_SEQ_LAST_E][6] = {
    {0x1b, 0   , 0   , 0   , 0   , 0   },  /* esc */
    {0x1b, 0x5b, 0x41, 0   , 0   , 0   },  /* up     "[A" */
    {0x1b, 0x5b, 0x42, 0   , 0   , 0   },  /* down   "[B" */
    {0x1b, 0x5b, 0x43, 0   , 0   , 0   },  /* right  "[C" */
    {0x1b, 0x5b, 0x44, 0   , 0   , 0   },  /* left   "[D" */
    {0x1b, 0x5b, 0x31, 0x3b, 0x35, 0x43},  /* ctrl-right "[1;5C" */
    {0x1b, 0x5b, 0x31, 0x3b, 0x35, 0x44},  /* ctrl-left "[1;5D" */
    {0x1b, 0x5b, 0x31, 0x7e, 0   , 0   },  /* home   "[1~" */
    {0x1b, 0x5b, 0x33, 0x7e, 0   , 0   },  /* del    "[3~" */
    {0x1b, 0x5b, 0x34, 0x7e, 0   , 0   }   /* end    "[4~" */
};


/*
 * scan input string Vs cli_esc_seq table for special character
 * Inputs:
 *   buf - incoming string
 *   len - size of string
 * of the characters and compare with table lines
 */
cli_esc_seq process_esc_seq(char *buf, unsigned char len)
{
    int i = 1, j = 1;

    if (buf[0] != 0x1b)
        return CLI_ESC_SEQ_LAST_E;

    if (len == 1)
        return CLI_ESC_SEQ_MORE_E;

    /* esc sequence */
    while (j < len) {
        if (esc_seq[i][j] != buf[j]) {
            if (++i >= (int)CLI_ESC_SEQ_LAST_E)
                break;
        } else
            if ((++j > 5) || (esc_seq[i][j] == 0))
                return (cli_esc_seq)i;
    } /* while */

    if (j == len)
        return CLI_ESC_SEQ_MORE_E; /* Found partial sequence */

    if (j == 1)
        return CLI_ESC_SEQ_ESC_E;  /* Found esc and then some other char */

    return CLI_ESC_SEQ_LAST_E;
}


