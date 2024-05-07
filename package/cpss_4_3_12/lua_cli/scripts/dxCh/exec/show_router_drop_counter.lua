--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_router_drop_counter.lua
--*
--* DESCRIPTION:  show router drop counter
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants
---
--  router_drop_show
--        @description  show router drop counter
--
--        @param params             -params["device"]: specific device number
--
--
--        @return       true on success, otherwise false and error message
--
local function router_drop_show(params)
    --print(to_string(params))
    local device=params["device"]
    local ret, val

    ret,val = myGenWrapper("cpssDxChIpDropCntGet",{    --get counter
    {"IN","GT_U8","devNum",device},
    {"OUT","GT_U32","dropCntPtr"}
    })

    if 0==ret then
        print() --seperation line
        print("router Ingress Drop Counter: "..val["dropCntPtr"])
        --reset counter
        ret = myGenWrapper("cpssDxChIpDropCntSet",{    --set counter
                {"IN","GT_U8","devNum",device},
                {"IN","GT_U32","dropCnt",0}
                })
        if 0~=ret then
        --couldn't reset
            print("Error at reseting router drop counter: "..returnCodes[ret].."\n")
        end
    else
        print("Error at getting router drop counter: "..returnCodes[ret].."\n")
    end
    print() --seperation line
end


--********************************************************************************

--includes

--constants
---
--  router_drop_cause
--        @description  show the cause of the router-drop
--
--        @param params             -params["device"]: specific device number
--
--
--        @return       true on success, otherwise false and error message
--


local function router_drop_cause(params)
    local device=params["device"]
    local ret, val

    ret = myGenWrapper("cpssDxChIpSetDropCntMode",{  --set mode
        {"IN","GT_U8","devNum",device},
        {"IN","CPSS_DXCH_IP_DROP_CNT_MODE_ENT","dropMode",
        "CPSS_DXCH_IP_DROP_CNT_COUNT_ALL_MODE_E"}
        })


    ret = myGenWrapper("cpssDxChIpDropCntSet",{    --set counter
        {"IN","GT_U8","devNum",device},
        {"IN","GT_U32","dropCnt",0}
        })

    delay(25)   --delay system to wait for packets

    ret,val = myGenWrapper("cpssDxChIpDropCntGet",{    --get counter
        {"IN","GT_U8","devNum",device},
        {"OUT","GT_U32","dropCntPtr"}
        })

    if 0==val["dropCntPtr"] then
        print("No router Ingress Drop")
    else
        print("router Ingress Drop Found")
        print("Detected causes:")
		for modeStr, mode in pairs(CLI_type_dict.router_drop_counter_mode.enum) do
			if(modeStr ~= "COUNT_ALL") then
				ret = myGenWrapper("cpssDxChIpSetDropCntMode",{  --set mode
                    {"IN","GT_U8","devNum",device},
                    {"IN","CPSS_DXCH_IP_DROP_CNT_MODE_ENT","dropMode",mode["value"]}
                    })

                ret = myGenWrapper("cpssDxChIpDropCntSet",{    --set counter
                    {"IN","GT_U8","devNum",device},
                    {"IN","GT_U32","dropCnt",0}
                    })

                delay(25)   --delay system to wait for packets

                ret,val = myGenWrapper("cpssDxChIpDropCntGet",{    --get counter
                    {"IN","GT_U8","devNum",device},
                    {"OUT","GT_U32","dropCntPtr"}
                    })

                if 0~= val["dropCntPtr"] then   --if packets dropped
                    print() --seperation line
                    print(mode["help"] .. " has dropped " .. val["dropCntPtr"]
                    .. " packets")
                end
            end
         end

    --reset counter
    ret = myGenWrapper("cpssDxChIpDropCntSet",{    --set counter
        {"IN","GT_U8","devNum",device},
        {"IN","GT_U32","dropCnt",0}
        })
    print() --seperation line
    end
end




--------------------------------------------------------------------------------
-- command registration: counters show
--------------------------------------------------------------------------------
CLI_addHelp("exec", "show counters", "Display counters")
CLI_addCommand("exec", "show counters router-drop", {
  func   = router_drop_show,
  help   = 'Show the counter configured by "counters router-drop"',
  params={{type= "named", {format="device %devID", name="device", help="ID of the device needed to count"},
                    mandatory = {"device"}}
  }
})

CLI_addCommand("exec", "show counters router-drop-cause", {
  func   = router_drop_cause,
  help   = 'Show the cause of the router-drop',
  params={{type= "named", {format="device %devID", name="device", help="ID of the device needed to count"},
                    mandatory = {"device"}}
  }
})


