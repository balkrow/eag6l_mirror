/* c_sa_builder_macsec.h
 *
 * Default configuration file for the MACsec SA Builder
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


/*----------------------------------------------------------------------------
 * Import the product specific configuration.
 */
#include <Kit/DriverFramework/incl/cs_sa_builder_macsec.h>

#ifndef SAB_MACSEC_SA_WORD_COUNT
#define SAB_MACSEC_SA_WORD_COUNT 24
#endif

/* Strict checking of function arguments if enabled */
/*#define SAB_STRICT_ARGS_CHECK */

/* log level for the SA builder.
   choose from LOG_SEVERITY_INFO, LOG_SEVERITY_WARN, LOG_SEVERITY_CRIT */
#ifndef LOG_SEVERITY_MAX
#ifndef CPSS_LOG_ENABLE
#define LOG_SEVERITY_MAX    LOG_SEVERITY_NO_OUTPUT
#else
#define LOG_SEVERITY_MAX    LOG_SEVERITY_CRIT
#endif /*CPSS_LOG_ENABLE*/
#endif



/* end of file c_sa_builder_macsec.h */
