
#ifndef MV_SHM_UART_SMI_H
#define MV_SHM_UART_SMI_H

#include "common.h"

MV_VOID mvShmUartSmiInit(MV_U32 base, MV_U32 size, MV_U32 _smiPhyAddr, MV_U32 _smiAdrrPort);
MV_VOID mvShmUartSmiPutc(MV_U8 c );
MV_U8 mvShmUartSmiGetc();


#endif

