/**
********************************************************************************
* @file cmdTFTPD.c
*
* @brief TFTP server implementation
*
* @version   1
********************************************************************************
*/
/*******************************************************************************
* cmdTFTPD.c
*
* DESCRIPTION:
*       TFTP server implementation
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

/***** Include files ***************************************************/

#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/FS/cmdFS.h>
#include <gtOs/gtOsTask.h>

/* for simulation target use port 8069 to avoid root privileges */
#ifndef ASIC_SIMULATION
#define TFTP_SVC_PORT   69
#else
#define TFTP_SVC_PORT   8069
#endif

#define TFTP_RX_TIMEOUT      1000
#define TFTP_MAX_RETRANSMIT  2

/*
 * Trivial File Transfer Protocol (IEN-133)
 */
#define TFTP_SEGSIZE     512         /* data segment size */

/*
 * Packet types.
 */
#define TFTP_RRQ     01              /* read request */
#define TFTP_WRQ     02              /* write request */
#define TFTP_DATA    03              /* data packet */
#define TFTP_ACK     04              /* acknowledgement */
#define TFTP_ERROR   05              /* error code */

/*
 * Error codes.
 */
#define TFTP_EUNDEF      0       /* not defined */
#define TFTP_ENOTFOUND   1       /* file not found */
#define TFTP_EACCESS     2       /* access violation */
#define TFTP_ENOSPACE    3       /* disk full or allocation exceeded */
#define TFTP_EBADOP      4       /* illegal TFTP operation */
#define TFTP_EBADID      5       /* unknown transfer ID */
#define TFTP_EEXISTS     6       /* file already exists */
#define TFTP_ENOUSER     7       /* no such user */


static GT_SOCKET_FD tftpdSocket = -1;
static char         tftpInputBuffer[TFTP_SEGSIZE+4];
static char         tftpOutputBuffer[TFTP_SEGSIZE+4];
static GT_U32       cmdTFTPD_thread_tid = 0;

/**
* @internal unpack_16bit function
* @endinternal
*
* @brief   Unpack 16bit value from network byte order to host
*
* @param[in] buf
*/
static GT_U16 unpack_16bit(
    IN const char  *buf
)
{
    return (((buf[0] << 8) & 0xff00) | (buf[1] & 0x00ff));
}

/**
* @internal pack_16bit function
* @endinternal
*
* @brief   Pack 16bit value to network byte order
*
* @param[in] value
*/
static void pack_16bit(
    IN  GT_U16  value,
    OUT char    *obuf
)
{
    obuf[0] = (value >> 8) & 0x00ff;
    obuf[1] = value & 0x00ff;
}

/*******************************************************************************
* get_asciiz
*
* DESCRIPTION:
*       get asciiz parameter
*       Check it is zero terminated
*
* INPUTS:
*       abuf
*       bufLen
*       offset
*
* OUTPUTS:
*       offset
*
*
* RETURNS:
*
*
* COMMENTS:
*       None
*
*******************************************************************************/
static char* get_asciiz(
    IN      char       *abuf,
    IN      int         bufLen,
    INOUT   int        *offset
)
{
    int o = *offset;
    while (abuf[*offset] && *offset < bufLen)
        (*offset)++;
    if (*offset >= bufLen) /* no trailing \0 */
        return NULL;
    (*offset)++;
    return abuf + o;
}




/**
* @internal create_socket function
* @endinternal
*
* @brief   Create UDP socket, bind it to port
*
* @param[in] port                     - UDP  to bind to, zero for any free
*                                       GT_SOCKET_FD or <0 if error
*/
static GT_SOCKET_FD create_socket(
    IN GT_SOCKET_PORT   port
)
{
    GT_SOCKET_FD    sock;
    GT_VOID        *bindAddr;
    GT_SIZE_T       bindAddrLen;

    sock = cmdOsSocketUdpCreate(-1);
    if (sock < 0)
    {
        return -1;
    }

    if (cmdOsSocketCreateAddr(NULL, port, &bindAddr, &bindAddrLen) != GT_OK)
    {
        cmdOsSocketUdpDestroy(sock);
        return -1;
    }

    if (cmdOsSocketBind(sock, bindAddr, bindAddrLen) != GT_OK)
    {
        cmdOsSocketDestroyAddr(bindAddr);
        cmdOsSocketUdpDestroy(sock);
        return -1;
    }
    cmdOsSocketDestroyAddr(bindAddr);
    return sock;
}

/**
* @internal select_on_socket function
* @endinternal
*
* @brief   Wait data from socket
*
* @param[in] sock
* @param[in] timeOut                  0xffffffff for infinite
*                                       GT_OK, GT_FAIL, GT_TIMEOUT
*/
static GT_STATUS select_on_socket(
    IN  GT_SOCKET_FD   sock,
    IN  GT_U32         timeOut
)
{
    GT_VOID *rdSet;
    GT_SSIZE_T k;
    GT_STATUS rc = GT_FAIL;

    rdSet = cmdOsSelectCreateSet();
    if (rdSet == NULL)
        return rc;

    cmdOsSelectZeroSet(rdSet);
    cmdOsSelectAddFdToSet(rdSet, sock);

    k = cmdOsSelect(sock+1, rdSet, NULL, NULL, timeOut);

    if (k == 0)
        rc = GT_TIMEOUT;
    if (k > 0)
    {
        if (cmdOsSelectIsFdSet(rdSet, sock) == GT_TRUE)
            rc = GT_OK;
    }

    cmdOsSelectEraseSet(rdSet);
    return rc;
}

/**
* @internal tftp_send_nak function
* @endinternal
*
* @brief   Send nagative acknowledgement to peer
*
* @param[in] sock
* @param[in] errcode
* @param[in] errmsg
*/
static GT_STATUS tftp_send_nak(
    IN  GT_SOCKET_FD    sock,
    IN  int             errcode,
    IN  const char     *errmsg
)
{
    char    tbuf[TFTP_SEGSIZE+4];
    int     r;

    pack_16bit(TFTP_ERROR, tbuf);
    pack_16bit((GT_U16)errcode, tbuf+2);
    r = 4;
    if (errmsg)
    {
        cmdOsStrCpy(tbuf + r, errmsg);
        r += cmdOsStrlen(errmsg);
        tbuf[r++] = 0;
    }
    cmdOsSocketSend(sock, tbuf, r);
    return GT_OK;
}

/**
* @internal tftpd_do_rx function
* @endinternal
*
* @brief   Receive file (TFTP_WRQ)
*
* @param[in] peer
* @param[in] bufLen
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS tftpd_do_rx(
    IN  GT_SOCKET_FD    peer,
    GT_SSIZE_T          bufLen
)
{
    char       *fname;
    char       *fmode;
    int         i = 2;
    int         rlen = 0;
    GT_U16      opcode, seq;
    GT_STATUS   rc;
    int         retransmits = 0;
    int         fd;

    fname = get_asciiz(tftpInputBuffer, (int)bufLen, &i);
    fmode = get_asciiz(tftpInputBuffer, (int)bufLen, &i);

    if (fname == NULL || fmode == NULL)
    {
        /* send NAK to peer */
        tftp_send_nak(peer, TFTP_EBADOP, "wrong request");
        return GT_FAIL;
    }

    /* TODO: check fmode */
    /* TODO: check fname */

    /* open file for write */
    fd = cmdFS.open(fname, GEN_FS_O_WRONLY | GEN_FS_O_CREAT | GEN_FS_O_TRUNC);
    if (fd < 0)
    {
        tftp_send_nak(peer, TFTP_ENOTFOUND, cmdFS.lastError());
        return GT_FAIL;
    }
    pack_16bit(TFTP_ACK, tftpOutputBuffer);
    pack_16bit(0, tftpOutputBuffer+2);
    rlen = 4;

    /* send ACK0 */
    cmdOsSocketSend(peer, tftpOutputBuffer, rlen);
    seq = 1; /* expectes seq# */

    while (1)
    {
        rc = select_on_socket(peer, TFTP_RX_TIMEOUT);
        if (rc == GT_TIMEOUT)
        {
            /* retransmit ack on timeout */
            if (retransmits > TFTP_MAX_RETRANSMIT || osTaskGracefulGetTerminationReq(cmdTFTPD_thread_tid))
            {
                break;
            }
            cmdOsSocketSend(peer, tftpOutputBuffer, rlen);
            retransmits++;
            continue;
        }
        if (rc != GT_OK)
            break;

        bufLen = cmdOsSocketRecv(peer,
                tftpInputBuffer, sizeof(tftpInputBuffer),
                GT_TRUE);

        if (bufLen < 4)
        {
            /* TODO: break??? */
            continue;
        }
        opcode = unpack_16bit(tftpInputBuffer);
        if (opcode == TFTP_DATA)
        {
            if (seq != unpack_16bit(tftpInputBuffer + 2))
            {
                /* bad seq# */
                tftp_send_nak(peer, TFTP_EBADID, "wrong seq#");
                continue;
            }

            cmdFS.write(fd, tftpInputBuffer+4, (int)(bufLen-4));

            pack_16bit(TFTP_ACK, tftpOutputBuffer);
            pack_16bit(seq, tftpOutputBuffer+2);
            rlen = 4;
            cmdOsSocketSend(peer, tftpOutputBuffer, rlen);
            retransmits = 0;
            seq++;
            /* check EOF */
            if (bufLen - 4 < TFTP_SEGSIZE)
            {
                rc = GT_OK;
                break;
            }
        }
        else
        {
            /* send NAK */
            tftp_send_nak(peer, TFTP_EBADOP, "TFTP_DATA packets expected");
        }

    }
    cmdFS.close(fd);
    return rc;
}

/**
* @internal tftpd_do_tx function
* @endinternal
*
* @brief   Send file (TFTP_RRQ)
*
* @param[in] peer
* @param[in] bufLen
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS tftpd_do_tx(
    IN  GT_SOCKET_FD    peer,
    GT_SSIZE_T          bufLen
)
{
    char       *fname;
    char       *fmode;
    int         i = 2;
    int         rlen = 0;
    GT_U16      opcode, seq;
    GT_STATUS   rc;
    int         retransmits = 0;
    int         fd;

    fname = get_asciiz(tftpInputBuffer, (int)bufLen, &i);
    fmode = get_asciiz(tftpInputBuffer, (int)bufLen, &i);

    if (fname == NULL || fmode == NULL)
    {
        /* send NAK to peer */
        tftp_send_nak(peer, TFTP_EBADOP, "wrong request");
        return GT_FAIL;
    }

    /* TODO: check fmode */
    /* TODO: check fname */

    /* open file for write */
    fd = cmdFS.open(fname, GEN_FS_O_RDONLY);
    if (fd < 0)
    {
        tftp_send_nak(peer, TFTP_ENOTFOUND, cmdFS.lastError());
        return GT_FAIL;
    }

    seq = 1;
    pack_16bit(TFTP_DATA, tftpOutputBuffer);
    pack_16bit(seq, tftpOutputBuffer+2);
    rlen = cmdFS.read(fd, tftpOutputBuffer + 4, TFTP_SEGSIZE);
    if (rlen < 0)
        rlen = 0;
    rlen += 4;
    cmdOsSocketSend(peer, tftpOutputBuffer, rlen);

    while (1)
    {
        rc = select_on_socket(peer, TFTP_RX_TIMEOUT);
        if (rc == GT_TIMEOUT)
        {
            /* retransmit ack on timeout */
            if (retransmits > TFTP_MAX_RETRANSMIT || osTaskGracefulGetTerminationReq(cmdTFTPD_thread_tid))
            {
                break;
            }
            cmdOsSocketSend(peer, tftpOutputBuffer, rlen);
            retransmits++;
            continue;
        }
        if (rc != GT_OK)
            break;

        bufLen = cmdOsSocketRecv(peer,
                tftpInputBuffer, sizeof(tftpInputBuffer),
                GT_TRUE);

        if (bufLen < 4)
        {
            /* TODO: break??? */
            continue;
        }
        opcode = unpack_16bit(tftpInputBuffer);
        if (opcode == TFTP_ACK)
        {
            if (seq != unpack_16bit(tftpInputBuffer + 2))
            {
                /* bad seq# */
                tftp_send_nak(peer, TFTP_EBADID, "wrong seq#");
                continue;
            }

            if (rlen < TFTP_SEGSIZE + 4)
            {
                /* last packet acknowleged */
                rc = GT_OK;
                break;
            }
            seq++;
            pack_16bit(TFTP_DATA, tftpOutputBuffer);
            pack_16bit(seq, tftpOutputBuffer+2);
            rlen = cmdFS.read(fd, tftpOutputBuffer+4, TFTP_SEGSIZE);
            if (rlen < 0)
                rlen = 0;
            rlen += 4;
            cmdOsSocketSend(peer, tftpOutputBuffer, rlen);
            retransmits = 0;
        }
        else
        {
            /* send NAK */
            tftp_send_nak(peer, TFTP_EBADOP, "TFTP_ACK packets expected");
        }

    }
    cmdFS.close(fd);
    return rc;
}

/*******************************************************************************
* cmdTFTPD_thread
*
* DESCRIPTION:
*
* INPUTS:
*       stream  - pointer to stream descriptor
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
static unsigned __TASKCONV cmdTFTPD_thread(void)
{
    GT_VOID        *peerAddr;
    GT_SIZE_T       peerAddrLen;
    GT_SIZE_T       len;
    GT_SSIZE_T      bufLen;
    GT_U16          opcode;
    GT_SOCKET_FD    peer;
    GT_STATUS       rc;
    GT_32 retval = 0;
    GT_VOID *rset;

    if (osTaskGetSelf(&cmdTFTPD_thread_tid) != GT_OK)
        return GT_FAIL;

    if (cmdOsSocketCreateAddr(NULL, 0, &peerAddr, &peerAddrLen) != GT_OK)
    {
        cmdOsSocketUdpDestroy(tftpdSocket);
        tftpdSocket = -1;
        return -1;
    }
    rset = cmdOsSelectCreateSet();
    if (rset == NULL)
        return -1;

    while (tftpdSocket > 0)
    {
        cmdOsSelectZeroSet(rset);
        cmdOsSelectAddFdToSet(rset, tftpdSocket);
        retval = cmdOsSelect(tftpdSocket + 1, rset, NULL, NULL, MV_DEFAULT_TIMEOUT);

        if(osTaskGracefulGetTerminationReq(cmdTFTPD_thread_tid) || retval < 0)
        {
            cmdOsSelectEraseSet(rset);
            cmdOsSocketUdpDestroy(tftpdSocket);
            tftpdSocket = -1;
            if (retval < 0)
                return -1;
            osTaskGracefulSetTerminationAck(cmdTFTPD_thread_tid);
            return 0;
        }
        if (retval == 0) /*timeOut*/
            continue;/*Termination is not needed.*/

        /* recv request on tftpdSocket */
        len = peerAddrLen;
        bufLen = cmdOsSocketRecvFrom(tftpdSocket,
                 tftpInputBuffer, sizeof(tftpInputBuffer),
                 GT_TRUE,
                 peerAddr, &len);

        if (bufLen < 4) /* short packet, ignore */
            continue;

        peer = create_socket(0); /* bind to any port */
        if (peer < 0)
            continue; /* TODO: break??? */
        rc = cmdOsSocketConnect(peer, peerAddr, peerAddrLen);
        if (rc != GT_OK)
        {
            cmdOsSocketUdpDestroy(peer);
            continue;
        }

        opcode = unpack_16bit(tftpInputBuffer);
        if (opcode != TFTP_RRQ && opcode != TFTP_WRQ)
        {
            /* send NAK to peer */
            tftp_send_nak(peer, TFTP_EBADOP, "wrong request");
            continue;
        }
        else
        {
            /* do rx/tx */
            if (opcode == TFTP_WRQ)
            {
                tftpd_do_rx(peer, bufLen);
            }
            if (opcode == TFTP_RRQ)
            {
                tftpd_do_tx(peer, bufLen);
            }
        }

        cmdOsSocketUdpDestroy(peer);

    }

#ifdef __GNUC__
    /* to avoid warnings in gcc */
    return 0;
#endif
}


/**
* @internal cmdTFTPDstart function
* @endinternal
*
* @brief   Start TFTP server
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cmdTFTPDstart(void)
{
    GT_TASK   tid;

    /* TODO check if already started */
    tftpdSocket = create_socket(TFTP_SVC_PORT);
    if (tftpdSocket < 0)
    {
        return GT_FAIL;
    }

    if (cmdOsTaskCreate(
                "TFTPD",
                6,                      /* thread priority          */
                8192,                   /* use default stack size   */
                (unsigned (__TASKCONV *)(void*))cmdTFTPD_thread,
                NULL,
                &tid) != GT_OK)
    {
        cmdOsPrintf("commander: failed to create TFTPD task\n");
        cmdOsSocketUdpDestroy(tftpdSocket);
        tftpdSocket = -1;
        return GT_FAIL;
    }
    osTaskGracefulCallerRegister(tid, GT_TRUE, NULL, NULL);

    return GT_OK;
}

