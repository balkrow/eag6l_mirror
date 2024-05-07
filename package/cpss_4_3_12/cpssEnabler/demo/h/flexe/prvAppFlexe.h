/*******************************************************************************
*              (c), Copyright 2021, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file prvAppFlexe.h
*
* @brief App demo Flexe header file.
*
* @version   1
********************************************************************************/

#ifndef __prvAppFlexeh
#define __prvAppFlexeh

#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>

#if __STDC_VERSION__ < 199901L
#undef __func__
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ "<unknown>"
# endif
#endif

#define APP_FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, errMsg, ...) \
        if(rc)                                         \
        {                                              \
            cpssOsPrintf("\n[ERROR (%d), %s. %d]. "  errMsg "\n", rc, __func__ , __LINE__, ##__VA_ARGS__); \
            return rc;                                 \
        }

#endif /* __prvAppFlexeh */
