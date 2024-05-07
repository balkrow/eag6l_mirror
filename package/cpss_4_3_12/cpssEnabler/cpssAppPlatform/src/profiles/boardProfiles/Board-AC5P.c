/*******************************************************************************
*              (c), Copyright 2023, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file Board-AC5P.c
*
* @brief AC5P Board profiles
*
* @version   1
********************************************************************************/

#include <profiles/cpssAppPlatformProfile.h>

#ifdef ASIC_SIMULATION

#endif


static CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC WM_AC5P =
{
    _SM_(boardName             ) "WM-AC5P",
    _SM_(osCallType            ) CPSS_APP_PLATFORM_OS_CALL_TYPE_STATIC_E,
    _SM_(extDrvCallType        ) CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_STATIC_E
};

static CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC RD_AC5P_32SFP =
{
    _SM_(boardName             ) "RD-AC5P-32SFP",
    _SM_(osCallType            ) CPSS_APP_PLATFORM_OS_CALL_TYPE_STATIC_E,
    _SM_(extDrvCallType        ) CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_STATIC_E
};


static CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC DB_AC5P_8SFP =
{
    _SM_(boardName             ) "DB-AC5P-8SFP",
    _SM_(osCallType            ) CPSS_APP_PLATFORM_OS_CALL_TYPE_STATIC_E,
    _SM_(extDrvCallType        ) CPSS_APP_PLATFORM_EXT_DRV_CALL_TYPE_STATIC_E
};

CPSS_APP_PLATFORM_BOARD_PROFILE_STC WM_AC5P_BOARD_INFO =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_E,
    {
        _SM_(boardPtr          ) &WM_AC5P
    }
};

CPSS_APP_PLATFORM_BOARD_PROFILE_STC DB_AC5P_8SFP_BOARD_INFO =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_E,
    {
        _SM_(boardPtr          ) &DB_AC5P_8SFP
    }
};

CPSS_APP_PLATFORM_BOARD_PROFILE_STC RD_AC5P_32SFP_BOARD_INFO =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_E,
    {
        _SM_(boardPtr          ) &RD_AC5P_32SFP
    }
};


static CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC PP_AC5P_PP_MAP_TABLE =
{
    _SM_(mapType               ) CPSS_APP_PLATFORM_PP_MAP_TYPE_FIXED_E,
    _SM_(mngInterfaceType      ) CPSS_CHANNEL_PEX_EAGLE_E,
    _SM_(devNum                ) 0,
#ifdef ASIC_SIMULATION
    _SM_(portMap               ) NULL /*GREG ac5p_32_ports*/,
    _SM_(portMapSize           ) 0 /*GREG (sizeof(ac5p_32_ports)/sizeof(ac5p_32_ports[0]))*/,
#else
    _SM_(portMap               ) NULL,
    _SM_(portMapSize           ) 0,
#endif
    {{
#if !defined(__x86_64__)
        _SM_(busId     ) 0x01,
#else
        _SM_(busId     ) 0x06,
#endif /*__x86_64__*/
        _SM_(deviceId  ) 0x00,
        _SM_(functionId) 0x00,
        _SM_(domain    ) 0x00
    }},
    _SM_(mppSelectPtr          ) NULL,
    _SM_(mppSelectArrSize      ) 0
};

CPSS_APP_PLATFORM_BOARD_PROFILE_STC WM_AC5P_PP_MAP_INFO =
{
    _SM_(boardInfoType             ) CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_E,
    {
#ifdef ANSI_PROFILES
        _SM_(ppMapPtr   ) (CPSS_APP_PLATFORM_BOARD_PARAM_GENERIC_STC*)&PP_AC5P_PP_MAP_TABLE
#else
        _SM_(ppMapPtr   ) &PP_AC5P_PP_MAP_TABLE
#endif
    }
};

