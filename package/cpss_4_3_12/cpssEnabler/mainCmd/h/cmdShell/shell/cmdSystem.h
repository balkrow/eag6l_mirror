/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file cmdSystem.h
*
* @brief commander compile-time configuration header
*
*
* @version   15
********************************************************************************
*/

#ifndef __cmdSystem_h__
#define __cmdSystem_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***** Include files ***************************************************/

/***** Global Definitions **********************************************/

#define CMD_THREAD_PRIO 5      /* default thread priority      */

/* enable streaming components */
#define CMD_INCLUDE_SERIAL          /* serial port shell server */


/* enable sockets connection*/
#define CMD_INCLUDE_TCPIP           /* tcpip shell server.
                                     * This option also enables luaCLI
                                     * TCP server */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cmdSystem_H__ */



