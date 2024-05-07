/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvCpssLog.h
*
* @brief Includes definitions for CPSS log functions.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssLogh
#define __prvCpssLogh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <stdarg.h>

extern int osPrintf(

    IN  const char* format,
    IN  ...
);

/* the status of the CPSS LOG - enable/disable */
extern GT_BOOL prvCpssLogEnabled;
#ifdef  WIN32
#define __FILENAME__ (cpssOsStrRChr(__FILE__, '\\') ? cpssOsStrRChr(__FILE__, '\\') + 1 : __FILE__)
#define __FUNCNAME__ __FUNCTION__
#else
#define __FILENAME__ (cpssOsStrRChr(__FILE__, '/') ? cpssOsStrRChr(__FILE__, '/') + 1 : __FILE__)
#define __FUNCNAME__ __func__
#endif
/* empty log error string */
#define LOG_ERROR_NO_MSG    ""

/* macro to log function error by formatted string */
#define CSREF_LOG_RETURN_NOT_OK_MAC(_rc) \
    if(GT_OK != _rc)         \
    {                        \
      osPrintf("Function %s,Line %d, fail:rc=%d!\n",__FUNCNAME__,__LINE__,_rc);  \
      return _rc;             \
    }  

#define CSREF_LOG_RETURN_MAC(_rc) \
{\
      osPrintf("Function %s,Line %d, fail:rc=%d!\n",__FUNCNAME__,__LINE__,_rc);\
return _rc; }

#define CSREF_PRINT_IPV4_ADDR_MAC(ipv4AddressArr) \
      osPrintf("IPv4 Address: %d.%d.%d.%d\n",\
      ipv4AddressArr[0],ipv4AddressArr[1],ipv4AddressArr[2],ipv4AddressArr[3]);

#define CSREF_PRINT_IPV6_ADDR_MAC(ipv6AddressArr) \
      osPrintf("IPv6 Address: %X%X:%X%X:%X%X:%X%X:%X%X:%X%X:%X%X:%X%X\n",\
      ipv6AddressArr[0],ipv6AddressArr[1],ipv6AddressArr[2],ipv6AddressArr[3],\
      ipv6AddressArr[4],ipv6AddressArr[5],ipv6AddressArr[6],ipv6AddressArr[7],\
      ipv6AddressArr[8],ipv6AddressArr[9],ipv6AddressArr[10],ipv6AddressArr[11],\
      ipv6AddressArr[12],ipv6AddressArr[13],ipv6AddressArr[14],ipv6AddressArr[15]);

#define CSREF_PRINT_MAC_ADDR_MAC(macAddressArr) \
      osPrintf("MAC Address: %X:%X:%X:%X:%X:%X\n",\
      macAddressArr[0],macAddressArr[1],macAddressArr[2],\
      macAddressArr[3],macAddressArr[4],macAddressArr[5]);




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssLogh */


