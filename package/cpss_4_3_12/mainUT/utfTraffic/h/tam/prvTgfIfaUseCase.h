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
* @file prvTgfIfaUseCase.h
*
* @brief IFA testing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIfaUseCaseh
#define __prvTgfIfaUseCaseh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/extServices/os/gtOs/gtGenTypes.h>

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }

/**
* @internal prvTgfIngressNodeIfaConfigurationSet function
* @endinternal
*
* @brief  IFA ingress node configuration settings
*/
void prvTgfIngressNodeIfaConfigurationSet();

/**
* @internal prvTgfIngressNodeIfaTrafficSend function
* @endinternal
*
* @brief  Traffic test for IFA ingress node
*/
void prvTgfIngressNodeIfaTrafficSend();

/**
* @internal prvTgfIngressNodeIfaConfigurationRestore function
* @endinternal
*
* @brief  Restore IFA ingress node configurations
*/
void prvTgfIngressNodeIfaConfigurationRestore();

/**
* @internal prvTgfTransitNodeIfaConfigurationSet function
* @endinternal
*
* @brief  IFA transit node configuration settings
*/
void prvTgfTransitNodeIfaConfigurationSet();

/**
* @internal prvTgfTransitNodeIfaTrafficSend function
* @endinternal
*
* @brief  Traffic test for IFA transit node
*/
void prvTgfTransitNodeIfaTrafficSend();

/**
* @internal prvTgfTransitNodeIfaConfigurationRestore function
* @endinternal
*
* @brief  Restore IFA transit node configurations
*/
void prvTgfTransitNodeIfaConfigurationRestore();

/**
* @internal prvTgfEgressNodeIfaConfigurationSet function
* @endinternal
*
* @brief  IFA egress node configuration settings
*/
void prvTgfEgressNodeIfaConfigurationSet();

/**
* @internal prvTgfEgressNodeIfaTrafficSend function
* @endinternal
*
* @brief  Traffic test for IFA egress node
*/
void prvTgfEgressNodeIfaTrafficSend();

/**
* @internal prvTgfEgressNodeIfaConfigurationRestore function
* @endinternal
*
* @brief  Restore IFA egress node configurations
*/
void prvTgfEgressNodeIfaConfigurationRestore();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIfaUseCaseh */


