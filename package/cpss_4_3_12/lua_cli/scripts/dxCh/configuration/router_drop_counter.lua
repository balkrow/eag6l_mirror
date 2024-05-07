--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* router_drop_counter.lua
--*
--* DESCRIPTION:  configure router drop counter 
--*       
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

-- ************************************************************************
---
--  router_drop_configure
--        @description  configure router drop counter 
--
--        @param params             - params["mode"]: mode to config
--                  - params["device"]: specific device to config
--                    
--                    
--
--        @return       true on success, otherwise false and error message
--

local function router_drop_configure(params)
    local device=params["device"]
    local mode="CPSS_DXCH_IP_DROP_CNT_COUNT_ALL_MODE_E"  --default value
    if nil~=params["mode"] then
        mode=params["mode"]
    end
    local ret
    
    --configure counter
        ret = myGenWrapper("cpssDxChIpSetDropCntMode",{  --set configure
                {"IN","GT_U8","devNum",device},
                {"IN","CPSS_DXCH_IP_DROP_CNT_MODE_ENT","dropMode",mode}
                })  
    if 0==ret then
        --reset counter
        ret = myGenWrapper("cpssDxChIpDropCntSet",{  --set counter
                {"IN","GT_U8","devNum",device},
                {"IN","GT_U32","dropCnt",0}
                })  
        if 0~=ret then
        --couldn't reset
            print("Error at reseting router drop counter: "..returnCodes[ret].."\n")
        end
    else
        --couldn't configure
        print("Error at setting router drop counter: "..returnCodes[ret].."\n")
    end

    
end



--------------------------------------------------------------------------------
-- command registration: counters configure
--------------------------------------------------------------------------------
CLI_addHelp("config", "counters", "Configure counters")
CLI_addCommand("config", "counters router-drop", {
  func   = router_drop_configure,
  help   = "Configure router drop counter",
  params={{type= "named", {format="device %devID", name="device", help="ID of the device needed to count"},
                   {format="mode %router_drop_counter_mode", name="mode", help="Configure counter mode"},
                    notMandatory = "device"
        }
  }
})