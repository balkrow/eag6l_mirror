#ifndef __UART_POSIX_H__
#define __UART_POSIX_H__

#include <common.h>

#define MV_UART_REGS_OFFSET( port ) ( 0xa0012000 | 0x100 * ( port ))

#define mvUartBase( port )      \
        (( MV_UART_PORT * )(unsigned long)( MV_UART_REGS_OFFSET( port )))

/* This structure describes the registers offsets for one UART port(channel) */
typedef struct mvUartPort {
    MV_U8 rbr;  /* 0 = 0-3 */
    MV_U8 pad1[ 3 ];

    MV_U8 ier;  /* 1 = 4-7 */
    MV_U8 pad2[ 3 ];

    MV_U8 fcr;  /* 2 = 8-b */
    MV_U8 pad3[ 3 ];

    MV_U8 lcr;  /* 3 = c-f */
    MV_U8 pad4[ 3 ];

    MV_U8 mcr;  /* 4 = 10-13 */
    MV_U8 pad5[ 3 ];

    MV_U8 lsr;  /* 5 = 14-17 */
    MV_U8 pad6[ 3 ];

    MV_U8 msr;  /* 6 =18-1b */
    MV_U8 pad7[ 3 ];

    MV_U8 scr;  /* 7 =1c-1f */
    MV_U8 pad8[ 3 ];
} MV_UART_PORT;

extern MV_U32 CLIUartPort;
extern MV_U32 CLIUartBaud;

MV_VOID mvUartInit( MV_U32 port, MV_U32 baudDivisor, MV_UART_PORT *base );

MV_VOID mvShmUartPutc(MV_U32 port, MV_U8 c);
MV_U8 mvShmUartGetc(MV_U32 port );
MV_BOOL mvShmUartTstc(MV_U32 port );
MV_VOID mvShmUartInit(MV_U32 *base, MV_U32 size);

#define mvUartTstc( port )      mvShmUartTstc(port)
#define mvUartPutc( port, c )   mvShmUartPutc(port, c)
#define mvUartGetc( port )      mvShmUartGetc(port)

#endif
