#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mvShmUartSmi.h"
#include "slave_smi.h"

/* static variables */
static MV_U32 uartRxBase = 0, uartTxBase = 0;
static MV_U32 uartRxOffset = 0, uartTxOffset = 0, uartRingSize = 0;
static MV_U32 reconnectFlag = 0, lastRxIndex = 0;
static MV_U8  uartTxWhichByte = 0;
static MV_U32 uartTxWordValue = 0;
static MV_U32 smiAdrrPort, smiPhyAddr;

static void SMI_CHK_RC(MV_STATUS rc)
{
    if (rc != MV_OK)
    { 
        slaveSmiDestroy(); 
        exit(1);
    }
}

/**
* @internal mvShmUartSmiInit function
* @endinternal
*
*/
MV_VOID mvShmUartSmiInit(MV_U32 base, MV_U32 size, MV_U32 _smiPhyAddr, MV_U32 _smiAdrrPort)
{
    MV_U32 ret_val, value = TRUE;
    smiAdrrPort = _smiAdrrPort;
    smiPhyAddr = _smiPhyAddr;
    uartRingSize = size;
    uartRxBase = base;
    uartTxBase = uartRxBase + uartRingSize;
    /* Flag and index for support of reopen of VUART is located after the Watchdog word */
    reconnectFlag =  base + (size * 2  + 1*sizeof(MV_U32));
    lastRxIndex = reconnectFlag + 1*sizeof(MV_U32);


    uartTxOffset = 0;
    SMI_CHK_RC(slaveSmiWrite(0 /* BusId */, smiPhyAddr, smiAdrrPort, reconnectFlag, &value)); /* reconnectFlag = 1*/
    /* If vuart is Reconnected, then gets the last RX Index from firmware */
    SMI_CHK_RC(slaveSmiRead(0 /* BusId */, smiPhyAddr, smiAdrrPort, lastRxIndex, &ret_val));
    uartRxOffset = ret_val;
    uartTxWordValue = 0;
    uartTxWhichByte = 0;

    return;
}

/**
* @internal mvShmUartSmiPutc function
* @endinternal
*
*/
MV_VOID mvShmUartSmiPutc(MV_U8 c )
{
    MV_U8 ii = 0;
    MV_U32 ret_val;

    if  (c != 0){
        while(ii < 100)
        {
            SMI_CHK_RC(slaveSmiRead(0 /* BusId */, smiPhyAddr, smiAdrrPort, uartTxBase + uartTxOffset, &ret_val));
            if((!uartTxWhichByte) && (ret_val != 0))
            {
                /* first byte in word, if the aligned word is not empty yet than sleep for 1 ms  */
                usleep(1000);
                ii++;
            }
            else
            {
                uartTxWordValue |= (MV_U32)(c) << (uartTxWhichByte * 8); /* shift left according to which byte*/
                SMI_CHK_RC(slaveSmiWrite(0 /* BusId */, smiPhyAddr, smiAdrrPort, uartTxBase + uartTxOffset, &uartTxWordValue));
                uartTxWhichByte++;
                if (uartTxWhichByte > 3)
                {
                    uartTxWhichByte = 0;
                    uartTxWordValue = 0;
                    uartTxOffset+=sizeof(MV_U32);
                    if (uartTxOffset >= uartRingSize)
                        uartTxOffset = 0;
                }
                break;
            }
        }
    }
    return;
}

/**
* @internal mvShmUartSmiGetc function
* @endinternal
*
*/
MV_U8 mvShmUartSmiGetc()
{
    char c = 0;
    MV_U32 uartRxWordValue;
    static MV_U8 uartRxWhichByte;
    MV_U32 ret_val, value = 0;

    SMI_CHK_RC(slaveSmiRead(0 /* BusId */, smiPhyAddr, smiAdrrPort, uartRxBase + uartRxOffset, &ret_val));
    uartRxWordValue = ret_val;
    c = (MV_U8)(uartRxWordValue >> (uartRxWhichByte * 8)) & 0xff;
    if (c != 0)
    {
        uartRxWhichByte++;
        if(uartRxWhichByte > 3) /* if read all 4 bytes - reset counter and erase word */
        {
            uartRxWhichByte = 0;
            SMI_CHK_RC(slaveSmiWrite(0 /* BusId */, smiPhyAddr, smiAdrrPort, uartRxBase + uartRxOffset, &value)); /* reset counter */
            uartRxOffset+=sizeof(MV_U32);
            if (uartRxOffset >= uartRingSize)
                uartRxOffset = 0;
        }
        SMI_CHK_RC(slaveSmiWrite(0 /* BusId */, smiPhyAddr, smiAdrrPort, lastRxIndex, &uartRxOffset));
    }
    return c;
}
