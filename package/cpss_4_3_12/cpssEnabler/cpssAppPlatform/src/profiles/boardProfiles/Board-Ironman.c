/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file Board-WM-Ironman.c
*
* @brief Ironman Board profiles
*
* @version   1
********************************************************************************/

#include <profiles/cpssAppPlatformProfile.h>

CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC WM_IRONMAN =
{
    _SM_(boardName             ) "IRONMAN",
    _SM_(osCallType            ) CPSS_APP_PLATFORM_OS_CALL_TYPE_STATIC_E,
    _SM_(extDrvCallType        ) CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_STATIC_E
};

CPSS_APP_PLATFORM_BOARD_PROFILE_STC WM_IRONMAN_BOARD_INFO =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_E,
    {
        _SM_(boardPtr          ) &WM_IRONMAN
    }
};

CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC PP_IRONMAN_PP_MAP_TABLE =
{
    _SM_(mapType               ) CPSS_APP_PLATFORM_PP_MAP_TYPE_FIXED_E,
    _SM_(mngInterfaceType      ) CPSS_CHANNEL_PEX_EAGLE_E,
    _SM_(devNum                ) 0,
    _SM_(portMap               ) NULL,
    _SM_(portMapSize           ) 0,
    {{
        _SM_(busId      ) 0x01,
        _SM_(deviceId   ) 0x00,
        _SM_(functionId ) 0x00,
        _SM_(domain     ) 0x00,
    }},
    _SM_(mppSelectPtr          ) NULL,
    _SM_(mppSelectArrSize      ) 0
};

CPSS_APP_PLATFORM_BOARD_PROFILE_STC WM_IRONMAN_PP_MAP_INFO =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_E,
    {
#ifdef ANSI_PROFILES
        _SM_(ppMapPtr   ) (CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC*)&PP_IRONMAN_PP_MAP_TABLE
#else
        _SM_(ppMapPtr   ) &PP_IRONMAN_PP_MAP_TABLE
#endif
    }
};
