/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplCpssGenEvents.c
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 13 $
*
*******************************************************************************/

/* Common galtis includes */
/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/events/cpssGenEventUnifyTypes.h>

#include <lua.h>
#include <extUtils/luaCLI/luaCLIEngine.h>
#include <appDemo/userExit/userEventHandler.h>

#ifdef CPSS_APP_PLATFORM
#include <cpssAppPlatformRunTimeConfig.h>
#endif

#include <cpssAppUtilsHa.h>

/*******************************************************************************
* wrlCpssGenEventTableGet
*
* DESCRIPTION:
*     get events from events table entry.
*
* INPUTS:
*     none
*
* OUTPUTS:
*      Table fields as describe in the table description are written into
*      outArgs parameter.
*
* RETURNS:
*     CMD_OK            - on success.
*     CMD_AGENT_ERROR   - on failure.
*     CMD_FIELD_UNDERFLOW - not enough field arguments.
*     CMD_FIELD_OVERFLOW  - too many field arguments.
*
* COMMENTS:
*     none
*
*******************************************************************************/
int wrlCpssGenEventTableGet
(
    lua_State* L
)
{
    GT_U32 uniEvIndex;
    GT_U32 endUniIndex;
    GT_U8  devNum;
    GT_U32 devType;
    GT_BOOL verbose;
    GT_U32 counter;
    GT_STATUS   ret;
    APP_UTILS_UNI_EVENT_COUNTER_STC * currentEntryPtr;

    /* check for valid arguments */
    if(!lua_isnumber(L,1) || !lua_isnumber(L,2)){
        lua_pushnil(L);
        lua_pushstring(L,"parameter must be integer");
        return -1;
    }

    devNum = (GT_U8) lua_tonumber(L, 1); /* devId */
    devType = (GT_U32) lua_tonumber(L, 2); /* devType */
    verbose = (GT_BOOL) lua_toboolean(L, 3); /* verbose */

    if(devType == 0) /* In case of PP,  the first and last index*/
    {
        uniEvIndex  = CPSS_PP_UNI_EV_MIN_E;
        endUniIndex = CPSS_PP_UNI_EV_MAX_E;
    }
    else
    {
        uniEvIndex  = 0;
        endUniIndex = 0;
    }

    /* Create event results table */
    lua_newtable(L);

    for (;uniEvIndex <= endUniIndex; uniEvIndex++)
    {
        if (verbose == GT_TRUE)
        {
            /* Set NULL pointer to retrieve first entry for event */
            currentEntryPtr = NULL;
            cpssAppUtilsGenExtendedEventCounterEntryGet(devNum, uniEvIndex, &currentEntryPtr);
            if (currentEntryPtr->counterValue == 0)
            {
                /* Move to the next event */
                continue;
            }
            /* Push event tyoe key */
            lua_pushnumber(L, uniEvIndex);  /* push key   */
            /* Create extended data results table */
            lua_newtable(L);

            while (currentEntryPtr)
            {
                if (currentEntryPtr->counterValue != 0)
                {
                    lua_pushnumber(L, currentEntryPtr->extData);        /* push extended data key */
                    lua_pushnumber(L, currentEntryPtr->counterValue);   /* push counter value */
                    lua_rawset(L, -3);
                }
                cpssAppUtilsGenExtendedEventCounterEntryGet(devNum, uniEvIndex, &currentEntryPtr);
            }
            lua_rawset(L, -3);
        }
        else
        {
            ret = cpssAppUtilsEventCounterGet(devNum, uniEvIndex, GT_FALSE, &counter);
            if ((ret != GT_OK) || (counter == 0))
            {
                continue;
            }
            lua_pushnumber(L, uniEvIndex);  /* push key */
            lua_pushnumber(L, counter);     /* push value */
            lua_rawset(L, -3);
        }
    }

    return 1;
}


/*******************************************************************************
* wrlCpssGenEventTableClear
*
* DESCRIPTION:
*     clear all entries in event counter entry block.
*
* INPUTS:
*     none
*
* OUTPUTS:
*      Table fields as describe in the table description are written into
*      outArgs parameter.
*
* RETURNS:
*     CMD_OK            - on success.
*     CMD_AGENT_ERROR   - on failure.
*     CMD_FIELD_UNDERFLOW - not enough field arguments.
*     CMD_FIELD_OVERFLOW  - too many field arguments.
*
* COMMENTS:
*     none
*
*******************************************************************************/
int wrlCpssGenEventTableClear
(
    lua_State* L
)
{
    GT_U32 uniEvIndex;
    GT_U32 endUniIndex;
    GT_U8  devNum;
    GT_U32 devType;

    /* check for valid arguments */
    if(!lua_isnumber(L,1) || !lua_isnumber(L,2)){
        lua_pushnil(L);
        lua_pushstring(L,"parameter must be integer");
        return -1;
    }

    devNum = (GT_U8) lua_tonumber(L, 1); /* devId */
    devType= (GT_U32) lua_tonumber(L, 2); /* devType */
    if(devType == 0) /* In case of PP,  the first and last index*/
    {
        uniEvIndex  = CPSS_PP_UNI_EV_MIN_E;
        endUniIndex = CPSS_PP_UNI_EV_MAX_E;
    }
    else
    {
        uniEvIndex  = 0;
        endUniIndex = 0;
    }

    /* create result table */
    lua_newtable(L);

    for (; uniEvIndex <= endUniIndex; uniEvIndex++)
    {
        cpssAppUtilsEventCounterGet(devNum, uniEvIndex, GT_TRUE, NULL);
    }
    lua_pushboolean(L, 0);
    return 1;
}

