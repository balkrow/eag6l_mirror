/* c_eip163.h
 *
 * EIP-163 Driver Library Default Configuration.
 */

/* -------------------------------------------------------------------------- */
/*                                                                            */
/*   Module        : ddk164                                                   */
/*   Version       : 3.1.1                                                    */
/*   Configuration : DDK-164                                                  */
/*                                                                            */
/*   Date          : 2021-Aug-12                                              */
/*                                                                            */
/* Copyright (c) 2008-2021 by Rambus, Inc. and/or its subsidiaries.           */
/*                                                                            */
/* All rights reserved. Unauthorized use (including, without limitation,      */
/* distribution and copying) is strictly prohibited. All use requires,        */
/* and is subject to, explicit written authorization and nondisclosure        */
/* agreements with Rambus, Inc. and/or its subsidiaries.                      */
/*                                                                            */
/* For more information or support, please go to our online support system at */
/* https://customersupport.insidesecure.com.                                  */
/* In case you do not have an account for this system, please send an e-mail  */
/* to ESSEmbeddedHW-Support@insidesecure.com.                                 */
/* -------------------------------------------------------------------------- */

#ifndef C_EIP163_H_
#define C_EIP163_H_


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

/* Top-level configuration */
#include <Kit/DriverFramework/incl/cs_eip163.h>

/* One device instance (core) mode of operation */
/*#define EIP163_DEVICE_MODE_EGRESS*/
/*#define EIP163_DEVICE_MODE_INGRESS */

#ifdef DRIVER_MODE_EGRESS
#define EIP163_DEVICE_MODE_EGRESS
#endif

#if !defined(EIP163_DEVICE_MODE_EGRESS) && !defined(EIP163_DEVICE_MODE_INGRESS)
#error "Device mode (egress or ingress) not defined"
#endif

/* Base offset used for all the EIP-163 registers */
#ifndef EIP163_CONF_BASE
#define EIP163_CONF_BASE                    0x00
#endif

/* Strict argument checking for the input parameters */
/* If required then define this parameter in the top-level configuration */
/*#define EIP163_STRICT_ARGS */

/* Number of read operations required for the 0 unsafe counter for proper CDS */
#ifndef EIP163_MAX_SYNC_COUNT
#define EIP163_MAX_SYNC_COUNT               3
#endif

/* Enable PACK timeout counter of statistics modules, */
/* granting the host access after a certain number of cycles. */
#ifndef EIP163_PACK_TIMEOUT_ENABLE
#define EIP163_PACK_TIMEOUT_ENABLE          1
#endif

/* Number of cycles for PACK timeout. */
#ifndef EIP163_PACK_TIMEOUT_VALUE
#define EIP163_PACK_TIMEOUT_VALUE           1
#endif

/* Disable EIP-163 device initialization register writes, */
/* this can be used to speed up initialization by offloading these register */
/* writes to another processor with lower EIP-163 register write overhead */
/*#define EIP163_DEV_INIT_DISABLE */

/* Number of non-control words in each TCAM entry. The actual width of */
/* TCAM words may be larger than what is required by the rules. */
/*#ifndef EIP163_TCAM_NON_CTRL_WORD_COUNT
#define EIP163_TCAM_NON_CTRL_WORD_COUNT EIP163_RULE_NON_CTRL_WORD_COUNT
#endif*/


#endif /* C_EIP163_H_ */


/* end of file c_eip163.h */
