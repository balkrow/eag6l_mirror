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
#ifndef DISABLE_CLI
/* Includes */
#include <FreeRTOS.h>
#include <stdint.h>
#include <string.h>
#include <task.h>
#include <semphr.h>
#include <FreeRTOS_CLI.h>
#include <hw.h>
#include <cli.h>
#include "printf.h"
#include "terminal.h"

CLIStateType        CLIState;
signed char         CLIOutput[ CLI_MAX_CMD ];
xSemaphoreHandle    CLISemaphore = NULL;

#ifdef AAS_EMULATOR
#define UART_PORT 0
#else
#define UART_PORT 2
#endif

#ifdef CLI_ENHANCED
#define CLI_MAX_LINE_LEN 96    /* Max number of character per input line */
#define CLI_HISTORY_DEPTH 4    /* Number of history lines saved (cyclic) */
#define CLI_CUR_BACK "\033[1D"  /* Escape sequence for 1 movement of cursor left */
#define CLI_CUR_FW   "\033[1C"  /* Escape sequence for 1 movement of cursor right */
#define CLI_PROMPT   "> "    /* The prompt that will be displayed at start of line */

char *buffer = (char *)CLIState.command;  /* Holds current edited input line */
char history[CLI_HISTORY_DEPTH][CLI_MAX_LINE_LEN];  /* Array of line buffers to keep history */
/*MENU_TYPE_HANDLE context;*/
static void handle_char(char c);

static void console_write(char *str, int len)
{
    while (len--)
        mvUartPutc(UART_PORT, *(str++));
}

/* Example of line handler to register */
static void handle_line(char *line)
{
    portBASE_TYPE   xMoreDataToFollow;

    CLIState.cmdpos = strlen(line);
    CLIState.cmdrdy = CLI_CMD_RDY;

    /* Repeat until the command does not generate any more output. */
    xMoreDataToFollow = pdTRUE;
    while (( CLIState.command[ 0 ] != 0 ) &&
            ( xMoreDataToFollow != pdFALSE )) {
        CLIOutput[ 0 ] = 0;

        /* Pass the string to FreeRTOS+CLI. */
        xMoreDataToFollow = FreeRTOS_CLIProcessCommand(
                ( char * )CLIState.command, ( char * )CLIOutput,
                CLI_MAX_CMD );

        /* Print the output. */
        CLIOutput[ CLI_MAX_CMD - 1 ] = 0;
        printf( ( char * )CLIOutput );
    }
    memset(buffer, 0, CLI_MAX_LINE_LEN);

    CLIState.cmdrdy = CLI_CMD_END;
    printf(CLI_PROMPT);
}

/*
 * Process single key press
 *
 * Key press can be a char, escape sequence, or special char
 * (such as del / backspace / enter/ tab)
 *
 * Input:
 *          buf - buffer with characters read from input stream
 *          len - amount of characters in buf
 *
 * Return value - number of proccessed characters
 * A single char will be processed, unless esc sequence received.
 */
#define CURSOR_RIGHT(n) {\
    sprintf(cmd, "\033[%dC", n); \
    console_write(cmd, strlen(cmd));}

#define CURSOR_LEFT(n) {\
    sprintf(cmd, "\033[%dD", n); \
    console_write(cmd, strlen(cmd));}

static void handle_char(char c)
{
    static unsigned char i = 0;       /* Cursor offset in edited line */
    static unsigned char end = 0;     /* Offset of last charcter in edited line */
    static unsigned char head = 0;    /* Index of latest line in history array, changes when pressing enter */
    static unsigned char current = 0; /* Index of current edited line in history array, changes when pressing up / down */
    static char esc_buf[5];  /*  */
    static unsigned char esc_idx = 0; /*  */
    unsigned char temp = 0;
    cli_esc_seq seq;
    char cmd[10];

    if (c == 0x1b || esc_idx) {
        /*
         * Either ESC pressed or esc sequence in progress
         * Add new char to esc seq buffer, check if matches any known sequence
         */
        esc_buf[esc_idx++] = c;
        seq = process_esc_seq(esc_buf, esc_idx);

        if (seq == CLI_ESC_SEQ_MORE_E)
            return;
        else
            esc_idx = 0;

        if (seq == CLI_ESC_SEQ_ESC_E)
        {
            /* Received a normal character after ESC pressed */
            /* TODO Handle esc here, then handle new char  */
            if (c == 0x1b)
            {
                /* New char is also ESC, start new seq */
                esc_buf[esc_idx++] = c;
                return;
            }
        }
        else
        {
            switch (seq) {
            case CLI_ESC_SEQ_DOWN_E:
                if (current == head)
                    break;
                current = (current + 1) % CLI_HISTORY_DEPTH;
            case CLI_ESC_SEQ_UP_E:  /* Process up / down key press */
                if (seq == CLI_ESC_SEQ_UP_E) {
                    temp = (current == 0) ? CLI_HISTORY_DEPTH - 1 : current - 1;
                    /* If reached first item in history and no more, ignore up key press */
                    if (history[temp][0] == 0)
                        break;
                    /* If text in buffer is a new entry - copy buffer to history before switching to previous line */
                    if (current == head)
                        strcpy(history[head], buffer);
                    current = temp;
                }

                /* Move cursor to start of line and erase till end of line */
                if (i > 0)
                    sprintf(cmd, "\033[%dD\033[K", i);
                else
                    sprintf(cmd, "\033[K");
                console_write(cmd, strlen(cmd));
                i = end = strlen(history[current]);
                console_write(history[current], i);   /* Print line from history to terminal */
                memset(buffer, 0, CLI_MAX_LINE_LEN);         /* Clear buffer and copy from history to buffer */
                strcpy(buffer, history[current]);
                break;

            case CLI_ESC_SEQ_RIGHT_E:
                /* Right arrow pressed - move cursor one char right, if not at end of line */
                if (i < end) {
                    sprintf(cmd, CLI_CUR_FW);
                    console_write(cmd, 4);
                    i++;
                }
                break;

            case CLI_ESC_SEQ_LEFT_E:
                /* Left arrow pressed - move cursor one char left, if not at start of line */
                if (i > 0) {
                    sprintf(cmd, CLI_CUR_BACK);
                    console_write(cmd, 4);
                    i--;
                }
                break;

            case CLI_ESC_SEQ_HOME_E:
                if (0 == i) break;
                sprintf(cmd, "\033[%dD", end);
                console_write(cmd, strlen(cmd));
                i = 0;
                break;

            case CLI_ESC_SEQ_DEL_E:
                if (i == end)
                    break;
                /* If not at end of line, move cursor one step FW, and call backspace handler */
                sprintf(cmd, CLI_CUR_FW);
                console_write(cmd, 4);
                i++;
                goto bs;

            case CLI_ESC_SEQ_END_E:
                if (i == end) break;
                CURSOR_RIGHT(end - i);
                i = end;
                break;

            case CLI_ESC_SEQ_CTRL_RIGHT_E:
                if (i == end) break;
                /* If not at end of line, move to space after current word */
                /* Scan spaces to the right until start of next word */
                while (((i + temp) < end) && (buffer[i + temp] == ' ')) temp++;
                /* Find space after current word */
                while (((i + temp) < end) && (buffer[i + temp] != ' ')) temp++;
                i += temp;
                CURSOR_RIGHT(temp);  /* Move cursor */
                break;

            case CLI_ESC_SEQ_CTRL_LEFT_E:
                if (i == 0) break;
                /* If not at start of line, move to first char of previous word */
                /* Scan spaces to the left until end of previous word */
                while (((i - temp - 1) >= 0) && (buffer[i - temp - 1] == ' ')) temp++;
                /* Find space before current word */
                while (((i - temp - 1) >= 0) && (buffer[i - temp - 1] != ' ')) temp++;
                i -= temp;
                CURSOR_LEFT(temp);  /* Move cursor */
                break;

            case CLI_ESC_SEQ_LAST_E:
            default:
                break;
            }
            return;
        }
    }

    if (c == ENTER_KEY) {
        /* Enter pressed -
         * promote head to next cyclic entry in history
         */
        temp = (head == 0) ? CLI_HISTORY_DEPTH - 1 : head - 1;
        /* If line is not empty and not the same as previous, add to history */
        if ((buffer[0] != 0) && strcmp(buffer, history[temp])) {
            buffer[end] = 0;
            strcpy(history[head], buffer);
            head = (head + 1) % CLI_HISTORY_DEPTH;
        }
        current = head;
        printf("\n");
        CLIState.cmdrdy = CLI_CMD_RDY;
        memset(history[head], 0, CLI_MAX_LINE_LEN); /* clean next history entry */
        i = end = 0; /* update buffer indexes */
        return;
    }

    if (c == 0x0b) { /* ctrl + k */
        memset(&buffer[i], ' ', end - i);      /* erase characters after new end */
        console_write(&buffer[i], end - i);    /* Update terminal */
        CURSOR_LEFT(end - i);                  /* Move cursor back to place after new end */
        end = i;
        buffer[end] = 0;
        return;
    }

    if (c == 0x15) { /* ctrl + u */
        memmove(buffer, &buffer[i], end - i);  /* Move chars after cursor to start of line */
        memset(&buffer[end - i], ' ', i);      /* erase characters after new end */
        CURSOR_LEFT(i);                        /* Move cursor to start of line */
        console_write(buffer, end);            /* Update terminal from start to end of line */
        memset(&buffer[end - i], 0, i);        /* Set all empty chars to 0 */
        CURSOR_LEFT(end);                      /* Move cursor back to place after print */
        end -= i;
        i = 0;
        return;
    }

    if (c == 0x17) { /* ctrl + w: Erase one word to the left */
        if (i == 0)
            return; /* If at start of line - break */
        /* Scan spaces to the left until end of previous word */
        while (((i - temp - 1) >= 0) && (buffer[i - temp - 1] == ' ')) temp++;
        /* Find start previous of word */
        while (((i - temp - 1) >= 0) && (buffer[i - temp - 1] != ' ')) temp++;
        memmove(&buffer[i - temp], &buffer[i], end - i);  /* Move chars after cursor left as needed */
        memset(&buffer[end - temp], ' ', temp);    /* erase characters after new end */
        CURSOR_LEFT(temp);                         /* Move cursor back one word */
        i -= temp;                                 /* Update current char index */
        console_write(&buffer[i], end - i);        /* Update terminal from cursor to end of line */
        memset(&buffer[end - temp], 0, temp);      /* Set all empty chars to 0 */
        CURSOR_LEFT(end - i);                      /* Move cursor back to place after print */
        end -= temp;                               /* Update end of line index */
        buffer[end] = 0;
        return;
    }

    if (c == 0x7f) { /* backspace */
        if (i == 0)
            return;
bs:
        sprintf(cmd, CLI_CUR_BACK);                    /* Cursor one step left */
        console_write(cmd, 4);
        buffer[end] = ' ';                             /* Insert space at end of buffer to erase last char */
        i--;
        memmove(&buffer[i], &buffer[i + 1], end - i);  /* Move chars after cursor one char left */
        console_write(&buffer[i], end - i);     /* Print moved chars to terminal */
        CURSOR_LEFT(end - i);                          /* Move cursor back to place after print */
        buffer[end] = 0;                               /* Replace space with NULL termination */
        buffer[--end] = 0;                             /* Replace space with NULL termination */
        return;
    }

    /* Recieved standard char. Check if we have room */
    if (end == CLI_MAX_LINE_LEN - 1)
        return;
    if (i < end)
        /* Insert char in middle of command line - push line 1 char right */
        memmove(&buffer[i + 1], &buffer[i], end - i);
    end++;
    buffer[i] = c;
    /* Print to terminal from insert point to end of line */
    console_write(&buffer[i], end - i);
    i++;
    if (i < end) {
        /* if insert not in end, move cursor back to insert point (after print) */
        CURSOR_LEFT(end - i);
    }
}

static void prvCLITask( void *pvParameters )
{
    memset(history, 0, sizeof(history));
    printf(CLI_PROMPT);

    while (1)
    {
#ifdef SHM_UART
        vTaskDelay( configTICK_RATE_HZ / 200 );
        while (MV_TRUE == mvUartTstc(UART_PORT))
        {
            handle_char(mvUartGetc(UART_PORT));
            if (CLIState.cmdrdy == CLI_CMD_RDY)
                break;
        }
#else /*SHM_UART*/
        xSemaphoreTake( CLISemaphore, ( portTickType ) portMAX_DELAY);
#endif
        if (CLIState.cmdrdy == CLI_CMD_RDY)
            handle_line(buffer); /* Hook to handle line */
    }
}

void iCLIHandler( void )
{
    char c;
#ifndef SHM_UART
    portBASE_TYPE xHigherPriTaskWoken = pdFALSE;
#endif

    if (MV_TRUE == mvUartTstc(UART_PORT))
    {
        c = mvUartGetc(UART_PORT);
        handle_char(c);
    }
    if ( CLIState.cmdrdy == CLI_CMD_RDY ) {
#ifndef SHM_UART
        /* Wake up the CLI task */
        xSemaphoreGiveFromISR( CLISemaphore, &xHigherPriTaskWoken );

        portYIELD_FROM_ISR(xHigherPriTaskWoken);
#endif
    }
}
#else /* CLI_ENHANCED */

void iCLIHandler( void )
{
    unsigned char i;
#ifndef SHM_UART
    portBASE_TYPE xHigherPriTaskWoken = pdFALSE;
#endif

    if ( CLIState.cmdrdy == CLI_CMD_END ) {
        CLIState.cmdpos = 0;
        CLIState.cmdrdy = CLI_CMD_NULL;
    }

    while ( mvUartTstc( CLI_UART_PORT ) == 1 ) {
        i = mvUartGetc( CLI_UART_PORT );

        if (( i >= 32 ) && ( i <= 126 )) {
            if ( CLIState.escchr ) {
                if ( !(( i == '[' ) || ( i == 'O' ) ||
                       ( i == ';' ) ||
                       (( i >= '0' ) && ( i <= '9' ))))
                    CLIState.escchr = 0;

                i = 0;
            } else
                mvUartPutc( CLI_UART_PORT, i );
        } else {
            switch ( i ) {
            case 0x8:
            case 0x7f:
                if (( CLIState.cmdpos > 0 ) &&
                    ( CLIState.command[ CLIState.cmdpos - 1 ] > 0 )) {
                    CLIState.cmdpos--;
                    CLIState.command[ CLIState.cmdpos ] = 0;
                    mvUartPutc( CLI_UART_PORT, 0x8 );
                    mvUartPutc( CLI_UART_PORT, ' ' );
                    mvUartPutc( CLI_UART_PORT, 0x8 );
                }
                i = 0;
                break;
            case ENTER_KEY:
                mvUartPutc( CLI_UART_PORT, '\n' );
                mvUartPutc( CLI_UART_PORT, '\r' );
                if ( CLIState.cmdrdy == CLI_CMD_NULL ) {
                    CLIState.cmdrdy = CLI_CMD_PRE;
                    i = 0;
                }
                break;
            case 0x1b:
                CLIState.escchr = 1;
                i = 0;
                break;
            default:
                i = 0;
            }
        }

        if (( i > 0 ) && ( CLIState.cmdpos < CLI_MAX_CMD - 1 )) {
            CLIState.command[ CLIState.cmdpos ] = i;
            CLIState.cmdpos++;
        }
    }

    if ( CLIState.cmdrdy == CLI_CMD_PRE ) {
        CLIState.cmdrdy = CLI_CMD_RDY;
        CLIState.command[ CLIState.cmdpos ] = 0;
        CLIState.cmdpos++;

#ifndef SHM_UART
        /* Wake up the CLI task */
        xSemaphoreGiveFromISR( CLISemaphore, &xHigherPriTaskWoken );

        portYIELD_FROM_ISR(xHigherPriTaskWoken);
#endif
    }

    return;
}

static void prvCLITask( void *pvParameters )
{
    portBASE_TYPE   xMoreDataToFollow;
#ifndef SHM_UART
    portTickType dalay = ( pvParameters && *(portTickType*)pvParameters ) ? ( *(portTickType*)pvParameters ) : ( portMAX_DELAY );
#endif

    for ( ;; ) {
        printf( "> " );

#ifdef SHM_UART
        do {
            /* Sleep for 5 ms */
            vTaskDelay( configTICK_RATE_HZ / 200 );

            iCLIHandler();
        } while ( CLIState.cmdrdy != CLI_CMD_RDY );

#else
        xSemaphoreTake( CLISemaphore, ( portTickType ) dalay /*portMAX_DELAY*/ );
#endif

        /* Repeat until the command does not generate any more output. */
        xMoreDataToFollow = pdTRUE;
        while (( CLIState.command[ 0 ] != 0 ) &&
               ( xMoreDataToFollow != pdFALSE )) {
            CLIOutput[ 0 ] = 0;

            /* Pass the string to FreeRTOS+CLI. */
            xMoreDataToFollow = FreeRTOS_CLIProcessCommand(
                ( char * )CLIState.command, ( char * )CLIOutput,
                CLI_MAX_CMD );

                /* Print the output. */
                CLIOutput[ CLI_MAX_CMD - 1 ] = 0;
                printf( ( char * )CLIOutput );
        }

        CLIState.cmdrdy = CLI_CMD_END;
    }

}

#endif /* CLI_ENHANCED */

void vCLIMain( void *pvParameters )
{
    prvCLITask(pvParameters);
}


static portBASE_TYPE prvHeapCommand( char *pcWriteBuffer,
    size_t xWriteBufferLen, const char *pcCommandString )
{
    printf("Heap remaining space: %d\n\n", xPortGetFreeHeapSize());

    return pdFALSE;
}


static const CLI_Command_Definition_t xHeapCommand =
{
    ( const char * const ) "heap",
    ( const char * const ) "heap\t\t\t\t\tPrint heap status\n",
    prvHeapCommand,
    0
};

void vCLISetup( void )
{

#ifndef SHM_UART
    vSemaphoreCreateBinary( CLISemaphore );
    if ( CLISemaphore == NULL )
        return;
    xSemaphoreTake( CLISemaphore, ( portTickType ) portMAX_DELAY );
#endif
#ifndef MICRO_INIT
    /*Create the CLI task in case there is no microinit */
    xTaskCreate( prvCLITask, ( const char * const ) "CLI",
                 configMINIMAL_STACK_SIZE, NULL,
                 tskIDLE_PRIORITY + 2, NULL );
#endif
    /* Register 'heap' command */
    FreeRTOS_CLIRegisterCommand( &xHeapCommand );
}

MV_STATUS gCLIHandleFileCommand(char * cmd, size_t cmdLen)
{
    int c = 0;
    size_t lineLength = 0;

    // Restore original command line
    cmd[cmdLen] = 0x20;

    // Ensure null termination and place for end of command line
    cmd[CLI_MAX_CMD-2] = '\0';
    lineLength = strlen(cmd);

    // Put line forward character and end of command line
    cmd[lineLength++] = '\n';
    cmd[lineLength] = '\0';

    // Fill the CLI control sructure
    memcpy(&CLIState.command[0],cmd,lineLength);
    CLIState.command[lineLength] = '\0';
    CLIState.cmdpos = lineLength+1;
    CLIState.cmdrdy = CLI_CMD_RDY;

#ifndef SHM_UART
    /* Wake up the CLI task */
    xSemaphoreGive( CLISemaphore );
#endif

    // Wait 10 sec for CLI command completion
    do {
        /* Sleep for 1 sec */
        vTaskDelay( configTICK_RATE_HZ );

        if (++c > 10) {
            printf( "error: CLI can't complete the command %s.\n", cmd );
            return MV_ERROR;
        }

    } while (CLI_CMD_END != CLIState.cmdrdy);

    return MV_OK;
}
#endif /* DISABLE_CLI */

