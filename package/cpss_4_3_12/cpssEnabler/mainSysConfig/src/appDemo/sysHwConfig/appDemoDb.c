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
* @file appDemoDb.c
*
* @brief App demo database.
*
* @version   3
********************************************************************************
*/

#include <appDemo/sysHwConfig/appDemoDb.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#ifdef IMPL_GALTIS
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#endif /*IMPL_GALTIS*/
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpssAppUtilsCommon.h>

/**
* @internal appDemoDbDump function
* @endinternal
*
* @brief   Dumps entries set in AppDemo database to console.
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoDbDump
(
    GT_VOID
)
{
    GT_U32 i;
    GT_U32 appDemoDbSize;

    /* print header */
    cpssOsPrintf("App Demo Database Dump:\n");
    cpssOsPrintf("");

    /* check if database is empty */
    appDemoDbSize = appDemoDbSizeGet();
    if (appDemoDbSize == 0)
    {
        cpssOsPrintf("App Demo Database is empty\n");
        return GT_OK;
    }

    /* print database values */
    for (i = 0 ; i < appDemoDbSize ; i++)
    {
        CPSS_ENABLER_DB_ENTRY_STC appDemoDbEntry = appDemoDbIndexedEntryGet(i);
        cpssOsPrintf("[%3d] %s = %d\n", i+1, appDemoDbEntry.name, appDemoDbEntry.value);
    }

    return GT_OK;
}



