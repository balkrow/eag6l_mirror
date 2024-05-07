#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>

#include <common.h>
#include <ic.h>
#include <srvCpuServices.h>

#define MAX 80
/* tcp_port_synchronic - port of socket where transactions initialized by application side
 * tcp_port_asynchronous - port of socket where transactions initialized by asic side
*/

#define SA struct sockaddr

static void dumpHex(unsigned char *ptr, unsigned len)
{ /* prints buffer bytes in hex */
    unsigned i;
    char msg[384], str[16];

    if (len > 100)
        len = 100;

    strcpy(msg, "");

    for (i = 0; i < len; i++)
    {
        sprintf(str, "%x ", ptr[i]);
	strcat(msg, str);
    }
    strcat(msg, "\n");
    printf("%u: %s", len, msg);
}

/*
 * Posix port of FreeRTOS uses ALRM signals
 * to simulate the OS tick interrupt.
 * This means this signal is highly likely
 * to hit us while pending on read.
 * Use this wrapper function, mread, which
 * will retry the read until succeeded or
 * failure reason is not EINTR (signal)
 * */
static ssize_t mread(int fd, void *buf, size_t count)
{
    ssize_t ret;

    do
    {
        ret = read(fd, buf, count);
    }
    while ( (ret < 0) && (errno == EINTR) );

    return ret;
}

/*
 Connect to the specified remote server port via TCP
 and return the socket number for this connection. 
*/
MV_32 connectToServer(unsigned serverPort)
{
    MV_32 sockfd;
    struct sockaddr_in servaddr, myLocalAddr;
    socklen_t addrLength;
    int ret;

    printf("%s: serverPort is %d\n", __func__, serverPort);
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        return -1;
    }

    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(serverPort);

    // connect the client socket to server socket
    ret = connect(sockfd, (SA*)&servaddr, sizeof(servaddr));

    if (ret != 0) {
        printf("%s: Socket connect failed, ret = %d errno = %d port = %u\n", __func__, ret, errno, htons(servaddr.sin_port));
        return -1;
    }

    ret = getsockname(sockfd, (struct sockaddr *)&myLocalAddr, &addrLength);
    printf("%s: Socket connect succeeded, ret = %d errno = %d port = %u fd %d\n", __func__, ret, errno, htons(servaddr.sin_port), sockfd);
    if (!ret)
        printf("Local Socket port is: %u\n", htons(myLocalAddr.sin_port));
    return sockfd;
}

/*
 Read socket data until socket has no
 more buffered data in kernel
*/
void flushSocketBuffer(MV_32 sockfd)
{
    uint8_t response[1024];
    MV_32 ret, numBytes;

    ret = ioctl(sockfd, FIONREAD, &numBytes); /* get number of readable bytes */

    if (ret < 0)
	return;

    if (numBytes <= 0)
	return;

    if (numBytes > 1024)
	numBytes = 1024;

    /*printf("ret %d numBytes %d\n", ret, numBytes);*/
    ret = mread(sockfd, &response, numBytes);
    if(ret < 0) {
        printf("%s: Socket buffer flush failed\n", __func__);
    } else {
        /*printf("%s: Socket buffer flush successful, ret = %d\n", __func__, ret);*/
    }
}

/*
  Parse response we were not expecting (e.g. interrupt)
*/
void parse_other_response(int sockfd, MSG_HDR *hdrHost)
{
    int ret;
    INTERRUPT_SET_MSG   message;
    uint8_t buf[1024];
    VARIABLE_TYPE treeId;

    if ((hdrHost->type == MSG_TYPE_INTERRUPT_SET) && (hdrHost->msgLen == (sizeof(VARIABLE_TYPE)*2))) 
    {
        ret = mread(sockfd, ((char *)&message) + sizeof(MSG_HDR), 
                   sizeof(message) - sizeof(MSG_HDR));

	treeId = htonl(message.treeId);
	printf("IRQ msg from sockfd %d treeId %d\n", sockfd, treeId);
	if ((size_t)ret >= (sizeof(message) - sizeof(MSG_HDR)))
            iICSocketIRQ(treeId);
    }
    else 
    {
        ret = mread(sockfd, buf, hdrHost->msgLen);
	printf("Got response type %d len %d\n",
	       hdrHost->type, hdrHost->msgLen);
    }
}

/*
  Parse White Model simulation distributed model response
*/
void parse_wm_msg(int sockfd)
{
    MSG_HDR             hdrNet;
    MSG_HDR             hdrHost;
    int ret;

    ret = mread(sockfd, &hdrNet, sizeof(MSG_HDR));
    if(ret < 0) {
        /*printf("Socket read failed\n");*/
        return;
    } 
    else 
    {

        hdrHost.type = ntohl(hdrNet.type);
        hdrHost.msgLen = ntohl(hdrNet.msgLen);

        if((hdrHost.type == MSG_TYPE_READ_RESPONSE) && (hdrHost.msgLen == 8))
	{
            printf("Unexpected read response\n");
	}
	else parse_other_response(sockfd, &hdrHost);
    }
}

/*
  Read register using WM distributed model sockets
*/
MV_32 readRegister(MV_32 sockfd, MV_U32 regAddr, MV_U32 *regValue)
{
    MV_32             ret;
    
    MV_U32            readLen;
    MV_U32            readVal;

    READ_MSG            sendMsg;
    MSG_HDR             hdrNet;
    MSG_HDR             hdrHost;

    flushSocketBuffer(sockfd);
    if ((regAddr & 0xfff00000) == 0xA0000000)
	    abort(); /* not supported in FW simulation */

    if ((regAddr & 0xfff00000) == SRV_CPU_DFX_WIN_BASE)
	    return -10; /* Remap windows are not simulated by WM.
			   DFX is used for low level stuff such as
			   JTAG enable. For now skip simulating DFX
			   via remap windows */
    if ((regAddr & 0xfff00000) == 0)
    {
	    return -10; /* Remap windows are not simulated by WM. */
    }

    bzero((void *)&sendMsg, sizeof(READ_MSG));

    bzero((void *)&hdrNet, sizeof(MSG_HDR));
    bzero((void *)&hdrHost, sizeof(MSG_HDR));

    sendMsg.hdr.type = MSG_TYPE_READ;
    sendMsg.hdr.msgLen = htonl(sizeof(sendMsg.deviceId) + sizeof(sendMsg.accessType) +
        sizeof(sendMsg.readLen) + sizeof(sendMsg.address));

    /* Prepare request to read register memory and send it to simulation broker: */
    sendMsg.deviceId = 0;
    sendMsg.accessType = htonl(REGISTER_MEMORY_ACCESS);
    sendMsg.readLen = htonl(1);
    sendMsg.address = htonl(regAddr);

    /*dumpHex((unsigned char *)&sendMsg, sizeof(READ_MSG));*/
    iICLock(sockfd);
    ret = write(sockfd, &sendMsg, sizeof(READ_MSG));
    if(ret < 0) 
    {
	iICUnlock(sockfd);
        printf("Socket write failed. sock %d ret %d errno %d\n",
	       sockfd, ret, errno);
        return -1;
    }

    /* Read to reply to register read request: */
    ret = mread(sockfd, &hdrNet, sizeof(MSG_HDR));
    if(ret < 0) {
	iICUnlock(sockfd);
        printf("Socket read failed, errno %d\n", errno);
        return -2;
    } 
    else 
    {

        hdrHost.type = ntohl(hdrNet.type);
        hdrHost.msgLen = ntohl(hdrNet.msgLen);
        /*printf("Got response to read with length %d\n", ret);*/

        if((hdrHost.type == MSG_TYPE_READ_RESPONSE) && (hdrHost.msgLen == 8)) 
	{ /* Verify that this is a read response with the expected size */

            ret = mread(sockfd, &readLen, sizeof(readLen)); /* read length of response */
            if(ret < 0) 
	    {
		iICUnlock(sockfd);
                printf("Socket read 2 failed, errno %d\n", errno);
                return -2;
            } 
	    else 
	    {
                readLen = ntohl(readLen) * sizeof(VARIABLE_TYPE); /* calculate length to read in native CPU byte order */

                if(readLen == 4) 
		{ /* expected length of read reply for a single register */
                    ret = mread(sockfd, &readVal, readLen); /* read response payload with memory content read */
                    if(ret < 0) 
		    {
			iICUnlock(sockfd);
                        printf("Socket read 3 failed, errno %d\n", errno);
                        return -2;
                    } 
		    else 
		    { /* copy read register content to caller */
                        readVal = ntohl(readVal);
                        *regValue = readVal;
			iICUnlock(sockfd);
			/*printf("Read Register %x val %x\n", regAddr, readVal);*/
                        return 0;
                    }
                } else printf("readLen is %d!\n", readLen);
            }
        }
	else parse_other_response(sockfd, &hdrHost);
    }
    iICUnlock(sockfd);
    return -3;
}

/*
  Send request to get peer WM standalone simulation process ID
*/
MV_32 readPeerPid(MV_32 sockfd)
{
    MV_32             ret;

    VARIABLE_TYPE     readPid;

    GET_WM_SIM_PID_MSG  sendMsg;
    MSG_HDR             hdrNet;
    MSG_HDR             hdrHost;

    flushSocketBuffer(sockfd);

    bzero((void *)&sendMsg, sizeof(GET_WM_SIM_PID_MSG));

    bzero((void *)&hdrNet, sizeof(MSG_HDR));
    bzero((void *)&hdrHost, sizeof(MSG_HDR));

    sendMsg.hdr.type = htonl(MSG_TYPE_GET_WM_SIM_PID);
    sendMsg.hdr.msgLen = htonl(sizeof(sendMsg.deviceId));

    /* Prepare request to read register memory and send it to simulation broker: */
    sendMsg.deviceId = 0;

    /*dumpHex((unsigned char *)&sendMsg, sizeof(READ_MSG));*/
    iICLock(sockfd);
    ret = write(sockfd, &sendMsg, sizeof(GET_WM_SIM_PID_MSG));
    if(ret < 0)
    {
        iICUnlock(sockfd);
        printf("Socket write failed. sock %d ret %d errno %d\n",
               sockfd, ret, errno);
        return -1;
    }

    /* Read to reply to register read request: */
    ret = mread(sockfd, &hdrNet, sizeof(MSG_HDR));
    if(ret < 0) {
        iICUnlock(sockfd);
        printf("Socket read failed, errno %d\n", errno);
        return -2;
    }
    else
    {

        hdrHost.type = ntohl(hdrNet.type);
        hdrHost.msgLen = ntohl(hdrNet.msgLen);
        /*printf("Got response to read with length %d\n", ret);*/

        if((hdrHost.type == MSG_TYPE_GET_WM_SIM_PID_RESPONSE) && (hdrHost.msgLen == 4)) 
        {
            ret = mread(sockfd, &readPid, sizeof(readPid)); /* read pid of peer from response */
            if(ret < 0)
            {
                iICUnlock(sockfd);
                printf("Socket read 2 failed, errno %d\n", errno);
                return -2;
            }
            else
            {
                iICUnlock(sockfd);
                printf("Read pid %u\n", ntohl(readPid));
                return ntohl(readPid);
            }
        }
        else parse_other_response(sockfd, &hdrHost);
    }
    iICUnlock(sockfd);
    return -3;
}

/*
  Wrapper to return register without passing pointer
  On error, flush socket and read again (once more).
*/
MV_U32 readRegisterInline(MV_32 sockfd, MV_U32 regAddr)
{
    MV_U32 val;

    if (readRegister(sockfd, regAddr, &val) < 0)
    {
        flushSocketBuffer(sockfd);
        if (readRegister(sockfd, regAddr, &val) < 0)
            return 0xbadbad;
    }

    return val;
}

/*
  Send request to WM simulation to write a register value
*/
MV_32 writeRegister(MV_32 sockfd, MV_U32 regAddr, MV_U32 regValue)
{
    MV_32             ret;
    
    MV_U32            *writeDataPtr;

    WRITE_MSG            sendMsg;

    if ((regAddr & 0xfff00000) == 0xA0000000)
	    abort(); /* not simulated in FW simulation */

    if ((regAddr & 0xfff00000) == SRV_CPU_DFX_WIN_BASE)
	    return -10; /* Remap windows are not simulated by WM.
			   DFX is used for low level stuff such as
			   JTAG enable. For now skip simulating DFX
			   via remap windows */

    bzero((void *)&sendMsg, sizeof(WRITE_MSG));

    sendMsg.hdr.type = htonl(MSG_TYPE_WRITE);
    sendMsg.hdr.msgLen = htonl(sizeof(sendMsg.deviceId) + sizeof(sendMsg.accessType) +
        sizeof(sendMsg.writeLen) + sizeof(sendMsg.address) + sizeof(sendMsg.dataPtr));

    /* Prepare request to write register memory and send it to simulation broker: */
    sendMsg.deviceId = 0;
    sendMsg.accessType = htonl(REGISTER_MEMORY_ACCESS);
    sendMsg.writeLen = htonl(1);
    sendMsg.address = htonl(regAddr);
    writeDataPtr = (MV_U32 *)&sendMsg.dataPtr;
    *writeDataPtr = htonl(regValue);

    ret = write(sockfd, &sendMsg, sizeof(WRITE_MSG));
    if(ret < 0) 
    {
        printf("Socket write failed. sock %d ret %d errno %d\n",
	       sockfd, ret, errno);
        return -1;
    }

    return ret;
}

/*
  Register interrupt line number for tree ID via
  WM simulation distrubted sockets
*/
MV_32 registerInterrupt(MV_32 sockfd, MV_U32 interruptLineNumber, MV_U32 signalID, MV_U32 treeId)
{
    MV_32             ret;
    
    REGISTER_INTERRUPT_MSG            sendMsg;

    return 0; /* requires broker */
    printf("%s: entry\n", __func__);
    bzero((void *)&sendMsg, sizeof(REGISTER_INTERRUPT_MSG));

    sendMsg.hdr.type = htonl(MSG_TYPE_REGISTER_INTERRUPT);
    sendMsg.hdr.msgLen = htonl(sizeof(sendMsg.interruptLineId) + sizeof(sendMsg.signalId) +
        sizeof(sendMsg.maskMode) + sizeof(sendMsg.treeId));

    /* Prepare request to register interrupt and send it to simulation broker: */
    sendMsg.interruptLineId = htonl(interruptLineNumber);
    sendMsg.signalId = htonl(signalID);
    sendMsg.maskMode = htonl(INTERRUPT_MODE_BROKER_USE_MASK_INTERRUPT_LINE_MSG);
    sendMsg.treeId = htonl(treeId);

    ret = write(sockfd, &sendMsg, sizeof(REGISTER_INTERRUPT_MSG));
    if(ret < 0) 
    {
        printf("Socket write failed, reg irq\n");
        return -1;
    }
    else printf("Registered int %d tree %d ret %d\n", interruptLineNumber, treeId, ret);
    return ret;
}

/*
  Unmask interrupt line number via
  WM simulation distrubted sockets
*/
MV_32 unMaskInterrupt(MV_32 sockfd, MV_U32 interruptLineNumber)
{
    MV_32             ret;
    
    UNMASK_INTERRUPT_MSG            sendMsg;

    return 0; /* requires broker */
    bzero((void *)&sendMsg, sizeof(UNMASK_INTERRUPT_MSG));

    sendMsg.hdr.type = htonl(MSG_TYPE_UNMASK_INTERRUPT);
    sendMsg.hdr.msgLen = htonl(sizeof(sendMsg.interruptLineId));

    /* Prepare request to unmask interrupt and send it to simulation broker: */
    sendMsg.interruptLineId = htonl(interruptLineNumber);

    ret = write(sockfd, &sendMsg, sizeof(UNMASK_INTERRUPT_MSG));
    if(ret < 0) 
    {
        printf("Socket write failed, unmask\n");
        return -1;
    }

    return ret;
}

/*
  Mask interrupt line number via
  WM simulation distrubted sockets
*/
MV_32 maskInterrupt(MV_32 sockfd, MV_U32 interruptLineNumber)
{
    MV_32             ret;
    
    MASK_INTERRUPT_MSG            sendMsg;

    return 0; /* requires broker */
    bzero((void *)&sendMsg, sizeof(MASK_INTERRUPT_MSG));

    sendMsg.hdr.type = htonl(MSG_TYPE_MASK_INTERRUPT);
    sendMsg.hdr.msgLen = htonl(sizeof(sendMsg.interruptLineId));

    /* Prepare request to unmask interrupt and send it to simulation broker: */
    sendMsg.interruptLineId = htonl(interruptLineNumber);

    ret = write(sockfd, &sendMsg, sizeof(MASK_INTERRUPT_MSG));
    if(ret < 0) 
    {
        printf("Socket write failed, mask\n");
        return -1;
    }

    return ret;
}

/*
  request to change WM simulation debug level via
  WM simulation distrubted sockets
*/
MV_32 sendMsgDbgLvl(MV_32 sockfd, MV_U32 dbgLvl)
{
    MV_32             ret;
    
    DEBUG_LEVEL_MSG            sendMsg;

    return 0; /* requires broker */
    bzero((void *)&sendMsg, sizeof(DEBUG_LEVEL_MSG));

    sendMsg.hdr.type = htonl(MSG_TYPE_DEBUG_LEVEL_SET);
    sendMsg.hdr.msgLen = htonl(sizeof(sendMsg.mainTarget)
		              +sizeof(sendMsg.secondaryTarget)
			      +sizeof(sendMsg.debugLevel));

    /* Prepare request to unmask interrupt and send it to simulation broker: */
    sendMsg.mainTarget = htonl(TARGET_OF_MESSAGE_ALL_E);
    sendMsg.secondaryTarget = htonl(ALL_SECONDARY_TARGETS);
    sendMsg.debugLevel = htonl(dbgLvl);

    ret = write(sockfd, &sendMsg, sizeof(DEBUG_LEVEL_MSG));
    if(ret < 0) 
    {
        printf("Socket write failed, mask\n");
        return -1;
    }

    return ret;
}


#ifdef TEST_DIST_WM_RW_REG_CLIENT_MAIN
MV_32 main()
{
    MV_32     sockfd;
    MV_32     ret;
    MV_32     regs;
    MV_U32    regValue;
    MV_U32    regAddr[] = {
#if 0
        0x0D80011C,	/* 0x2a4 * port 0 */
	0x0D8003C0,	/* 0x2a4 * port 1 */
	0x0D800664,
	0x0D800908,
	0x0D800BAC,
	0x0D800E50,
	0x0D8010F4,
	0x0D801398,	/* 0x2a4 * port 7 */
        0x0D800130,	/* 0x2a4 * port 0 */
	0x0D8003D4, 
	0x0D800678,
	0x0D80091C,
	0x0D800BC0,
	0x0D800E64,
	0x0D801108,
	0x0D8013AC,	/* 0x2a4 * port 7 */
        0x0D602008,
#endif
	0x3C200034
    };

    sockfd = connectToServer();
    if(sockfd < 0) {
        printf("%s: Failed to Connect to Distributed Sumulator\n", __func__);
        exit(0);
    } else {
        printf("%s: Successfully connected to Distributed Sumulator\n", __func__);
    }

    flushSocketBuffer(sockfd);

    for(regs=0; regs<((sizeof(regAddr)/sizeof(MV_U32))); regs++) {

        ret = readRegister(sockfd, regAddr[regs], &regValue);
        if(ret == 0)
            printf("0x%.8X = 0x%.8X\n", regAddr[regs], regValue);
        else
            printf("%s: readRegister failed, ret = %d\n", __func__, ret);

    }

    // close the socket
    close(sockfd);
}
#endif
