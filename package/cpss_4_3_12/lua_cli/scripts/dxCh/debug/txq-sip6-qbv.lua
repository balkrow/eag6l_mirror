--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* txq-sip6-qbv.lua
--*
--* DESCRIPTION:
--*       Basic debug menu for QBV
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
--dofile("dxCh/debug/txq-sip6.lua")



local function dump_tableset_calendar(params)

    local res,val,ind
    local device = tonumber(params["devID"])
    local tableset =   tonumber(params["tableset"])
    local lastIndex
    local slotsNum = 256
    local maxTableSet = 28
    local timeToAdvance,gateStateBmp,hold

    if tableset>28 then
        print("Error - tableset is out of bound - "..tableset.." .Max is "..maxTableSet)
        return
    end

    if(is_supported_feature(device, "QBV"))then
        lastIndex = slotsNum-1

        for ind=0,lastIndex do
                res, val = myGenWrapper("prvCpssSip6_30TxqSdqQbvCalendarGet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U32" , "tileNum",  0},
                { "IN", "GT_U32" , "sdqNum",  0},
                { "IN", "GT_U32" , "tableset",  tableset},
                { "IN", "GT_U32" , "index",  ind},
                { "OUT", "CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC", "timeSlotPtr"},
        })
            if(res==0)then
                timeToAdvance = val["timeSlotPtr"]["timeToAdvance"]
                gateStateBmp= val["timeSlotPtr"]["gateStateBmp"]
                hold = val["timeSlotPtr"]["hold"]
                print(string.format("[%d] timeToAdvance = 0x%08x gateStateBmp = 0x%04x hold %s",ind,timeToAdvance,gateStateBmp,tostring(hold)))
            else
                print(" prvCpssSip6_30TxqSdqQbvCalendarGet failed for device"..device ..  " rc= "..res.."index "..ind)
                return
            end --if
        end --for
    else
        print("Device "..devices[j].." does not support 802.1 QBV")
    end

end


local function port_config_show(params)

local port =   tonumber(params["portNum"])
local maxQ

 if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do
      if(is_supported_feature(devices[j], "QBV"))then
        res, val = myGenWrapper("prvCpssSip6_30TxqPortQvbEnableGet", {
                    { "IN", "GT_U8"  , "devNum", devices[j]},
                    { "IN", "GT_PHYSICAL_PORT_NUM" , "portNum",  port},
                    { "OUT", "GT_BOOL", "enablePtr"},
               })
        if(res~=0)then
          print("Error "..res.." calling prvCpssSip6_30TxqPortQvbEnableGet")
          return
        end
        print("QBV enable "..tostring(val.enablePtr))

        res, val = myGenWrapper("prvCpssSip6TxqUtilsMaxQueueOffsetGet", {
                { "IN", "GT_U8"  , "devNum", devices[j]},
                { "IN", "GT_U32" , "portNum", port},
                { "OUT", "GT_U32", "maxQ"},
        })

        if(res~=0)then
          print("Error "..res.." calling prvCpssSip6TxqUtilsMaxQueueOffsetGet")
          return
        end

        maxQ = val.maxQ
        for tc=0,maxQ do
                    res, val = myGenWrapper("cpssDxChStreamEgressPortQueueGateGet", {
                            { "IN", "GT_U8"  , "devNum", devices[j]},
                            { "IN", "GT_U32" , "portNum",  port},
                            { "IN", "GT_U8" , "queueOffset",  tc},
                            { "OUT", "GT_U32", "gate"},
                    })

        if(res~=0)then
          print("Error "..res.." calling prvCpssSip6TxqUtilsMaxQueueOffsetGet")
          return
        end
        print(" Offset "..tc.."-> gate "..val.gate)
        end

        res, val = myGenWrapper("cpssDxChStreamEgressPortBindGet", {
                { "IN", "GT_U8"  , "devNum", devices[j]},
                { "IN", "GT_U32" , "portNum", port},
                { "OUT", "GT_U32", "tableSetPtr"},
                { "OUT", "CPSS_DXCH_STREAM_EGREESS_PORT_BIND_ENTRY_STC", "bindEntryPtr"},
        })

        if(res~=0)then
          print("Error "..res.." calling cpssDxChStreamEgressPortBindGet")
          return
        end
        print("tableSet "..val.tableSetPtr)
        print("lastEntry "..val.bindEntryPtr.gateReconfigRequestParam.lastEntry)
        print("lengthAware "..tostring(val.bindEntryPtr.gateReconfigRequestParam.lengthAware))
        print("remainingBitsFactor "..val.bindEntryPtr.gateReconfigRequestParam.remainingBitsFactor)
        print("bitsFactorResolution "..val.bindEntryPtr.gateReconfigRequestParam.bitsFactorResolution)
        print("cycleTime "..val.bindEntryPtr.gateReconfigRequestParam.cycleTime)
        print("egressTodOffset "..val.bindEntryPtr.gateReconfigRequestParam.egressTodOffset)
        print("maxAlwdBcOffset "..val.bindEntryPtr.gateReconfigRequestParam.maxAlwdBcOffset)

      else
        print("Device "..devices[j].." does not support 802.1 QBV")
        return
      end
    end

end


local function qbv_api_show(params)
    print(" Configuring tableset calendar")
    print(" ==============================")
    print(" cpssDxChStreamEgressTableSetConfigSet")
    print(" cpssDxChStreamEgressTableSetConfigGet\n")
    print(" Mapping queue to gate")
    print(" ==============================")
    print(" cpssDxChStreamEgressPortQueueGateSet")
    print(" cpssDxChStreamEgressPortQueueGateGet\n")
    print(" Port binding/unbinding operations")
    print(" ==============================")
    print(" cpssDxChStreamEgressPortBindSet")
    print(" cpssDxChStreamEgressPortBindGet")
    print(" cpssDxChStreamEgressPortBindComplete")
    print(" cpssDxChStreamEgressPortUnbindSet")

end


local function qbv_display_capability(params)

local res,val,i,j

if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do
        if(is_supported_feature(devices[j], "QBV"))then
         ports=luaCLI_getDevInfo(devices[j])[devices[j]]
         for i=1,#ports do
           res, val = myGenWrapper("prvCpssSip6_30TxqPortQbvCapabilityGet", {
                { "IN", "GT_U8"  , "devNum", devices[j]},
                { "IN", "GT_PHYSICAL_PORT_NUM" , "portNum",  ports[i]},
                { "OUT", "GT_BOOL", "qbvCapablePtr"},
                { "OUT", "GT_U32", "errCodePtr"},
           })

           if(res~=0)then
            print("Error "..res.." calling prvCpssSip6_30TxqPortQbvCapabilityGet")
            return
           end
           if (val.qbvCapablePtr ==false) then
            print(string.format("Device:%5d  Port:%5d QBV capable:FALSE errCode:%3d ",devices[j],ports[i],val.errCodePtr))
           else
            print(string.format("Device:%5d  Port:%5d QBV capable:TRUE ",devices[j],ports[i]))
           end
         end
        else
         print("Device "..devices[j] ..  " does not support QBV")
        end
    end


end



--CLI_addHelp("debug", "txq-sip6-enqueue-stat", "Debug enqueue statistic")
--CLI_addHelp("debug", "txq-sip6-show", "Show pass/drop counters")


CLI_addCommand("debug", "txq-sip6-qbv  show-tableset", {
   func = dump_tableset_calendar,
   help = "Dump tableset (255 entires)",
   params={
        { type="named",
        { format="device %devID",name="devID",help="The device number" } ,
        { format="tableset %tablesetID",name="tableset",help="The tableset number" },
         mandatory={"devID","tableset"},
        }
    }
})

CLI_addCommand("debug", "txq-sip6-qbv  port-config", {
   func = port_config_show,
   help = "Show port QBV configuration",
   params={
        { type="named",
        { format="device %devID_all", name="devID",help="The device number" } ,
        { format="port %portNum",name="portNum",help="The port number" },
          mandatory={"devID","portNum"},
        }
    }
})

CLI_addCommand("debug", "txq-sip6-qbv display-api", {
   func = qbv_api_show,
   help = "Show QBV APIs",
   params={
        { type="named",
        { format="device %devID_all", name="devID",help="The device number" } ,
          mandatory={"devID"},
        }
    }
})


--------------------------------------------------------------------------------
---- command registration: txq-sip6-qbv  display-capabilities
--------------------------------------------------------------------------------

CLI_addCommand("debug", "txq-sip6-qbv display-capabilities", {
   func = qbv_display_capability,
   help = "Display device qbv capabilities",
   params={
        { type="named",
        { format="device %devID_all", name="devID",help="The device number" } ,
          mandatory={"devID"},
        }
    }
})

