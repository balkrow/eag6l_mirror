/*******************************************************************************
*              (c), Copyright 2020, Marvell International Ltd.                 *
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
 * @file Board-Harrier.c
 *
 * DESCRIPTION:
 *            AC5X Board profiles.
 *
 *
 * FILE REVISION NUMBER:
 *       $Revision: 1 $
 *
 *******************************************************************************/
#include <profiles/cpssAppPlatformProfile.h>

static CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC WM_HARRIER =
{
    _SM_(boardName             ) "HARRIER",
    _SM_(osCallType            ) CPSS_APP_PLATFORM_OS_CALL_TYPE_STATIC_E,
    _SM_(extDrvCallType        ) CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_STATIC_E
};

CPSS_APP_PLATFORM_BOARD_PROFILE_STC WM_HARRIER_BOARD_INFO =
{
    _SM_(boardInfoType         ) CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_E,
    {
        _SM_(boardPtr          ) &WM_HARRIER
    }
};

static CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC PP_HARRIER_PP_MAP_TABLE =
{
    _SM_(mapType               ) CPSS_APP_PLATFORM_PP_MAP_TYPE_FIXED_E,
    _SM_(mngInterfaceType      ) CPSS_CHANNEL_PEX_EAGLE_E,
    _SM_(devNum                ) 0,
    _SM_(portMap               ) NULL,
    _SM_(portMapSize           ) 0,
    {{
        _SM_(busId     ) 0x01,
        _SM_(deviceId  ) 0x00,
        _SM_(functionId) 0x00,
        _SM_(domain    ) 0x00
    }},
    _SM_(mppSelectPtr          ) NULL,
    _SM_(mppSelectArrSize      ) 0
};

/* Cannot be static. To be shared with pp.c */
CPSS_APP_PLATFORM_BOARD_PROFILE_STC WM_HARRIER_PP_MAP_INFO =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_E,
    {
#ifdef ANSI_PROFILES
        _SM_(ppMapPtr   ) (CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC*)&PP_HARRIER_PP_MAP_TABLE
#else
        _SM_(ppMapPtr   ) &PP_HARRIER_PP_MAP_TABLE
#endif
    }
};


