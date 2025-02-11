--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* speed.lua
--*
--* DESCRIPTION:
--*       setting of port speed and interface mode
--*
--* FILE REVISION NUMBER:
--*       $Revision: 11 $
--*
--********************************************************************************

--includes
cmdLuaCLI_registerCfunction("wrlDxChPortModeSpeedSet")
cmdLuaCLI_registerCfunction("wrlDxChPortsModeSpeedSet")
cmdLuaCLI_registerCfunction("wrlCpssDxChSerdesDefaultPowerUp")
cmdLuaCLI_registerCfunction("wrlPrvCpssDxChPortIsRemote")

--constants
local OR, XOR, AND = 1, 3, 4
local function bitoper(a, b, oper)
   local r, m, s = 0, 2^52
   repeat
      s,a,b = a+b+m, a%m, b%m
      r,m = r + m*oper%(s-a-b), m/2
   until m < 1
   return r
end

CLI_type_dict["pm_events"] = {
    checker = CLI_check_param_enum,
    complete = CLI_complete_param_enum,
    help = "port manager event types",
    enum = {
        ["all"] = { value="CPSS_PORT_MANAGER_UNMASK_MAC_LOW_LEVEL_EVENTS_ENABLE_MODE_E", help="enable all events" },
        ["mac"] = { value="CPSS_PORT_MANAGER_UNMASK_MAC_LEVEL_EVENTS_DISABLE_MODE_E", help="disable unmask of mac events" },
        ["pcs"] = { value="CPSS_PORT_MANAGER_UNMASK_LOW_LEVEL_EVENTS_DISABLE_MODE_E", help="disable unmask of pcs events" }
    }
}

-- ************************************************************************
---
--  speed_func
--        @description  sets port speed and interface mode
--
--        @param params         - params["port_speed"]: port speed;
--                                params["interface_mode"]: interface mode
--
--        @usage __global       - __global["ifRange"]: interface range
--
--        @return       true on success, otherwise false and error message
--
local function speed_func(params)
    -- Common variables declaration.
    local result, values
    local devFamily, devNum, ports, portNum
    local command_data = Command_Data()
    -- Command  specific variables declaration.
    local port_speed_changing, port_speed
    local port_interface_mode_changing, port_interface_mode_determinancy
    local port_interface_mode, port_is_remote
    local fc_intervals, interval, state

    -- Common variables initialization.
    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()

    -- power down interface
    if params.flagNo then
        local iterator
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()
            result, values =  wrLogWrapper("wrlDxChPortModeSpeedSet", "(devNum, portNum, false, 0, 0)", devNum, portNum, false, 0, 0)
            if  0x10 == result then
                command_data:setFailPortStatus()
                command_data:addWarning("It's not allowed to power down for device %d port %d", devNum, portNum)
            elseif 0 ~= result then
                command_data:setFailPortStatus()
                command_data:addWarning("Error of power downing for device %d port %d: %s", devNum, portNum, values)
            end
        end
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
        return command_data:getCommandExecutionResults()
    end

    -- Command specific variables initialization.
    port_speed_changing                 =
        command_data:getTrueIfFlagNoIsNil(params)
        port_speed = params["port_speed"]
    port_interface_mode_changing        =
        getTrueIfNotNil(params["port_interface_mode"])
    port_interface_mode_determinancy    =
        getTrueIfNotNil(params["port_interface_mode"])
    port_interface_mode = params["port_interface_mode"]

  --power up interface
  for iterator, devNum, portNum in command_data:getPortIterator() do
      command_data:clearPortStatus()
      command_data:clearLocalStatus()
      port_is_remote = wrlPrvCpssDxChPortIsRemote(devNum, portNum)
      result, values = wrLogWrapper("wrlDxChPortModeSpeedSet",
                                    "(devNum, portNum, true, port_interface_mode, port_speed)",
                                    devNum, portNum, true, port_interface_mode, port_speed)
      if  0x10 == result then
        command_data:setFailPortStatus()
        command_data:addWarning("It is not allowed to set " ..
                    "speed %s and interface " ..
                    "mode %s for device %d " ..
                    "port(s) %s",
                    speedStrGet(port_speed),
                    interfaceStrGet(
                      port_interface_mode),
                    devNum, tbltostr(ports))
      elseif 0 ~= result then
        command_data:setFailPortStatus()
        command_data:addWarning("Error at port speed %s or " ..
                    "mode %s setting of device " ..
                    "%d port(s) %s: %s",
                    speedStrGet(port_speed),
                    interfaceStrGet(
                      port_interface_mode),
                    devNum, tbltostr(ports), values)
      elseif ((0 == result) and (not port_is_remote))and (not is_sip_6(devNum)) then
        result, values = myGenWrapper("cpssDxChPortPeriodicFcEnableGet",
                      {{ "IN", "GT_U8"  , "devNum", devNum},                 -- devNum
                       { "IN", "GT_PHYSICAL_PORT_NUM" , "portNum", portNum},            -- portNum
                       { "OUT","CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT" , "statePtr"}})             -- data
        if 0 ~= result then
          command_data:setFailPortStatus()
          command_data:addWarning("Can't getting Flow Control Enable Status " ..
                      "speed %s and interface " ..
                      "mode %s for device %d " ..
                      "port(s) %s",
                      speedStrGet(port_speed),
                      interfaceStrGet(
                        port_interface_mode),
                      devNum, tbltostr(ports), values)
        else
          state = values.statePtr;
          if "CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E" ~= state then
            fc_intervals = fc_intervals_get()
            interval = fc_intervals[port_speed]
            if nil ~= interval then
              result, values = myGenWrapper("cpssDxChPortPeriodicFlowControlCounterSet",
                            {{ "IN", "GT_U8", "devNum", devNum},                                 -- devNum
                             { "IN", "GT_PHYSICAL_PORT_NUM" , "portNum", portNum},            -- portNum
                             { "IN", "GT_U32", "value", interval}})                              -- interval
              if 0 ~= result then
                command_data:setFailPortStatus()
                command_data:addWarning("Can't setting Periodic Flow Control Counter " ..
                            "speed %s and interface " ..
                            "mode %s for device %d " ..
                            "port(s) %s",
                            speedStrGet(port_speed),
                            interfaceStrGet(
                              port_interface_mode),
                            devNum, tbltostr(ports), values)
              end
            end
          end
        end
      end
  end
  command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

local function speed_func_port_mgr(params)
    local result, values
    local portStagePtr={}
    local command_data = Command_Data()
    local iterator
    local devNum, portNum
    local portParamStc
    local event = {}
    local GT_OK = 0
    local GT_NOT_INITIALIZED = 0x12
    local validAttrsBitMask = 0
    local fecMode, serdes
    -- Common variables initialization.
    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()


    if params.flagNo then
        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()

            event.portEvent =  "CPSS_PORT_MANAGER_EVENT_DELETE_E"
            result, values = myGenWrapper("cpssDxChPortManagerEventSet",
                              {{ "IN", "GT_U8", "devNum", devNum},
                               { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                               { "IN", "CPSS_PORT_MANAGER_STC", "portEventStcPtr", event}})

            if result~=GT_OK and result~=GT_NOT_INITIALIZED then
                print("Error at command: cpssDxChPortManagerEventSet : ".. returnCodes[result])
            end
        end
    else
        for iterator, devNum, portNum in command_data:getPortIterator() do
            -- prvCpssCommonPortIfModeToHwsTranslate needed only when need speed ,and not 'no speed'
            result,values = cpssGenWrapper("prvCpssCommonPortIfModeToHwsTranslate",{
                  { "IN", "GT_U8", "devNum", devNum },
                  { "IN", "CPSS_PORT_INTERFACE_MODE_ENT","cpssIfMode", params.port_interface_mode },
                  { "IN", "CPSS_PORT_SPEED_ENT","cpssSpeed", params.port_speed },
                  { "OUT", "MV_HWS_PORT_STANDARD", "ifMode" }
                })

            if result~=0 then
                printLog ("Error in prvCpssCommonPortIfModeToHwsTranslate")
            end

            if values.ifMode == "_50GBase_KR"  or values.ifMode == "_50GBase_CR"  or values.ifMode == "_50GBase_SR_LR" or
               values.ifMode == "_100GBase_KR2"  or values.ifMode == "_100GBase_CR2"  or values.ifMode == "_100GBase_SR_LR2" or
               values.ifMode == "_200GBase_KR4"  or values.ifMode == "_200GBase_CR4"  or values.ifMode == "_200GBase_SR_LR4" or
               values.ifMode == "_200GBase_KR8"  or values.ifMode == "_200GBase_CR8"  or values.ifMode == "_200GBase_SR_LR8" or
               values.ifMode == "_400GBase_KR8"  or values.ifMode == "_400GBase_CR8"  or values.ifMode == "_400GBase_SR_LR8" or
               values.ifMode == "_102GBase_KR2" then
                fecMode = fecToVal("CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E")
            elseif
               values.ifMode == "_100GBase_KR4"  or values.ifMode == "_100GBase_CR4"  or values.ifMode == "_100GBase_SR4" or
               values.ifMode == "_106GBase_KR4" or values.ifMode == "_107GBase_KR4" then
                fecMode = fecToVal("CPSS_PORT_RS_FEC_MODE_ENABLED_E")
            else
                fecMode = fecToVal("CPSS_PORT_FEC_MODE_DISABLED_E")
            end
            --[[
               There is no point to do it for all devices ,
               but leave it for future use when device number will be actually in use in
               prvCpssCommonPortIfModeToHwsTranslate
            ]]--
            break
        end


        for iterator, devNum, portNum in command_data:getPortIterator() do
            command_data:clearPortStatus()
            command_data:clearLocalStatus()
            event.portEvent =  "CPSS_PORT_MANAGER_EVENT_DELETE_E"
            result, values = myGenWrapper("cpssDxChPortManagerEventSet",
                             {{ "IN", "GT_U8", "devNum", devNum},
                              { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                              { "IN", "CPSS_PORT_MANAGER_STC", "portEventStcPtr", event}})

           if result~=GT_OK and result~=GT_NOT_INITIALIZED then
                print("Error at command: cpssDxChPortManagerEventSet : ".. returnCodes[result])
                command_data:analyzeCommandExecution()
                command_data:printCommandExecutionResults()
                return command_data:getCommandExecutionResults()
           end

           result, serdes = cpssGenWrapper("cpssPortManagerLuaSerdesTypeGet",{
                                 { "IN",  "GT_SW_DEV_NUM",             "devNum",    devNum },
                                 { "OUT", "CPSS_PORT_SERDES_TYPE_ENT", "serdesType"        }
                                 })
           if result~=0 then
               print("Error in cpssPortManagerLuaSerdesTypeGet")
           end

           portParamStc = {}
           portParamStc.magic = 0x1a2bc3d4
           portParamStc.portParamsType = {}
           portParamStc.portType = "CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E"
           portParamStc.portParamsType.regPort = {}
           portParamStc.portParamsType.regPort.speed = params.port_speed
           portParamStc.portParamsType.regPort.ifMode = params.port_interface_mode
           portParamStc.portParamsType.regPort.portAttributes = {}
           portParamStc.portParamsType.regPort.portAttributes.fecMode = fecMode
           validAttrsBitMask = 0
           validAttrsBitMask = bitoper(validAttrsBitMask, 0x20, OR)
           portParamStc.portParamsType.regPort.portAttributes.validAttrsBitMask = validAttrsBitMask
           portParamStc.portParamsType.regPort.laneParams = {}
           for i = 0,7 do
               portParamStc.portParamsType.regPort.laneParams[i] = {}
               portParamStc.portParamsType.regPort.laneParams[i].txParams = {}
               portParamStc.portParamsType.regPort.laneParams[i].rxParams = {}
               portParamStc.portParamsType.regPort.laneParams[i].txParams.type = serdes.serdesType
               portParamStc.portParamsType.regPort.laneParams[i].rxParams.type = serdes.serdesType
               portParamStc.portParamsType.regPort.laneParams[i].validLaneParamsBitMask = 0x0
           end
           portParamStc.portParamsType.regPort.portAttributes.preemptionParams = {}
           if params.preemption ~= null and params.preemption == true then
             portParamStc.portParamsType.regPort.portAttributes.preemptionParams.type = "CPSS_PM_MAC_PREEMPTION_ENABLED_AGGREGATED_E"
             portParamStc.portParamsType.regPort.portAttributes.validAttrsBitMask =
                bitoper(portParamStc.portParamsType.regPort.portAttributes.validAttrsBitMask, 0x1000, OR)--CPSS_PM_PORT_ATTR_PREEMPTION_E
            else
             portParamStc.portParamsType.regPort.portAttributes.preemptionParams.type = "CPSS_PM_MAC_PREEMPTION_DISABLED_E"
            end
           result, values = myGenWrapper("cpssDxChPortManagerPortParamsSet",
                            {{ "IN", "GT_U8"  , "devNum", devNum},
                             { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                             { "IN", "CPSS_PM_PORT_PARAMS_STC", "portParamsStcPtr", portParamStc}})

           if(result~=GT_OK) then
               print("Error at command: cpssDxChPortManagerPortParamsSet : ".. returnCodes[result])
               command_data:analyzeCommandExecution()
               command_data:printCommandExecutionResults()
               return command_data:getCommandExecutionResults()
           end
        end
    end

    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

local function speed_func_main(params)
    local result, values
    local devNum, portNum
    local command_data = Command_Data()

    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()
    devNum, portNum = command_data:getFirstPort()

    result, values = myGenWrapper("cpssDxChPortManagerEnableGet",
                          {{ "IN", "GT_U8"  , "devNum", devNum},                 -- devNum
                           { "OUT","GT_BOOL" , "enablePtr"}})                    -- is port-manager enabled/disabled
    if (values.enablePtr) then
        speed_func_port_mgr(params)
    else
        speed_func(params)
    end
end

local function fec_func_port_mgr(params)
    local result, values
    local command_data = Command_Data()
    local devNum, portNum
    local iterator
    local GT_OK = 0
    local GT_NOT_INITIALIZED = 0x12
    local GT_BAD_STATE = 0x7
    local fecMode
    local ifMode, speed
    local fecSupported
    local fecRequested
    -- Common variables initialization.
    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()

    if params.flagNo then
        fecMode = fecToVal("CPSS_PORT_FEC_MODE_DISABLED_E")
    else
        fecMode = fecToVal(params.port_fec_mode)
    end

    for iterator, devNum, portNum in command_data:getPortIterator() do
        result, values = myGenWrapper("cpssDxChSamplePortManagerFecModeSet",
                          {{ "IN", "GT_U8"  , "devNum", devNum},
                           { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                           { "IN", "CPSS_PORT_FEC_MODE_ENT", "fecMode", fecMode}})

        if result == GT_NOT_INITIALIZED then
            print("Error : Fec mode cannot be modified , Port not initialized \n")
        elseif result == GT_BAD_STATE then
            print("Error : Fec mode cannot be modified , Port is not in reset state \n")
        elseif result~=GT_OK then
            print("Error at command: cpssDxChSamplePortManagerFecModeSet :%s", result)
        end
    end
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()
    return command_data:getCommandExecutionResults()
end

local function fec_func_main(params)
    local result, values
    local devNum, portNum
    local command_data = Command_Data()

    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()

    devNum, portNum = command_data:getFirstPort()

    result, values = myGenWrapper("cpssDxChPortManagerEnableGet",
                          {{ "IN", "GT_U8"  , "devNum", devNum},                 -- devNum
                           { "OUT","GT_BOOL" , "enablePtr"}})                    -- is port-manager enabled/disabled
    if (values.enablePtr) then
        fec_func_port_mgr(params)
    end
end

local function events_func(params)
    local result, values
    local command_data = Command_Data()
    local iterator
    local devNum, portNum
    local GT_OK = 0
    local GT_BAD_STATE = 0x7
    local GT_NOT_INITIALIZED = 0x12
    local unMaskMode

    command_data:initInterfaceRangeIterator()
    command_data:initInterfaceDeviceRange()

    devNum, portNum = command_data:getFirstPort()

    result, values = myGenWrapper("cpssDxChPortManagerEnableGet",
                          {{ "IN", "GT_U8"  , "devNum", devNum},                 -- devNum
                           { "OUT","GT_BOOL" , "enablePtr"}})                    -- is port-manager enabled/disabled

    if values.enablePtr == true then
        -- Common variables initialization.
        command_data:initInterfaceRangeIterator()
        command_data:initInterfaceDeviceRange()
        command_data:initAllAvailableDevicesRange()

        if params.flagNo then
            unMaskMode = "CPSS_PORT_MANAGER_UNMASK_MAC_LOW_LEVEL_EVENTS_ENABLE_MODE_E"
        else
            unMaskMode = params.pm_events
        end

        for iterator, devNum, portNum in command_data:getPortIterator() do
            result, values = myGenWrapper("cpssDxChSamplePortManagerUnMaskModeSet",
                          {{ "IN", "GT_U8"  , "devNum", devNum},
                           { "IN", "GT_PHYSICAL_PORT_NUM", "portNum", portNum},
                           { "IN", "CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_ENT", "unMaskMode", unMaskMode}})
            if result == GT_NOT_INITIALIZED then
                print("Error : Port not initialized \n")
            elseif result == GT_BAD_STATE then
                print("Error : Port is not in reset state \n")
            elseif result~=GT_OK then
                print("Error at command: cpssDxChSamplePortManagerUnMaskModeSet :%s", result)
            end
        end
        command_data:analyzeCommandExecution()
        command_data:printCommandExecutionResults()
        return command_data:getCommandExecutionResults()
    end
end

--------------------------------------------
-- command registration: speed
--------------------------------------------
CLI_addCommand("interface", "speed", {
  func   = speed_func_main,
  help   = "Set the transmit and receive speeds",
  params = {
    { type = "values", "%port_speed"},
    { type = "named", "#interface_mode" ,
    { format="preemption %bool", name="preemption",help="mark as preemptive true/false"},
     mandatory = {"port_interface_mode"}}
  }
})

CLI_addCommand("interface", "no speed", {
  func = function(params)
    params.flagNo = true
    return speed_func_main(params)
    end,
  help = "Powering down interface"
})

--------------------------------------------
-- command registration: fec
--------------------------------------------
CLI_addCommand("interface", "fec", {
  func   = fec_func_main,
  help   = "Port forward error correction modes",
  params = {
    { type = "values", "%port_fec_mode"}
  }
})

CLI_addCommand("interface", "no fec", {
  func = function(params)
    params.flagNo = true
    return fec_func_main(params)
    end,
  help   = "Port forward error correction modes",
})

--------------------------------------------
-- command registration: events
--------------------------------------------
CLI_addCommand("interface", "events", {
  func   = events_func,
  help   = "event unmask per port",
  params = {
    { type = "values", "%pm_events"}
  }
})

CLI_addCommand("interface", "no events", {
  func = function(params)
    params.flagNo = true
    return events_func(params)
    end,
  help   = "event unmask per port",
})
