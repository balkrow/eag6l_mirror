--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* pcl_global_cfg.lua
--*
--* DESCRIPTION:
--*       PCL global configuration
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants


-- ************************************************************************
---
--  pcl_global_cfg
--        @description  set PCL Global configuration
--
--        @param params         - params["device"]: device ID
--
--        @param params         - params["force_tt_per_vlan"]: {true/false}
--                                boolean: force CFG Table Access per VLAN for
--                                Tunnel Terminated packets
--        expected addind new parameters to the command
--
--        @return       true on success, otherwise false and error message
--
local function pcl_global_cfg(params)
    -- Common variables declaration.
    local result, values
    local devNum, devices
    local command_data = Command_Data()
    local force_tt_per_vlan = params["force_tt_per_vlan"];

    if (params["device"]=="all") then
        devices = wrLogWrapper("wrlDevList");
    else
        devices={params["device"]};
    end

    if params.metadata then 
        --set the global mode before calling to ipclUdb30Config(devNum)
        ipclUdb30GlobalModeSet(params.metadata)
    end

    for dummy, devNum in pairs(devices) do
        if force_tt_per_vlan ~= nil then
            result = myGenWrapper("cpssDxCh3PclTunnelTermForceVlanModeEnableSet", {
                        { "IN", "GT_U8",   "devNum", devNum  },
                        { "IN", "GT_BOOL", "enable", force_tt_per_vlan }});
            command_data:handleCpssErrorDevice(result,
                        "cpssDxCh3PclTunnelTermForceVlanModeEnableSet failed", devNum)
        end
        
        if params.metadata then 
            ipclUdb30Config(devNum)
        end
        
    end
    command_data:updateStatus()
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: mac address
--------------------------------------------------------------------------------
CLI_addCommand("config", "pcl-config", {
  func   = pcl_global_cfg,
  help   = "Setting of a Global PCL configurations",
  params = {
      {
          type = "values",
          {format = "%devID_all", name = "device"},
          mandatory = {"device"},
      },
      {
          type = "named",
          {format = "force-tt-per-vlan %bool", name = "force_tt_per_vlan",
              help = "force CFG Table Access per VLAN for Tunnel Terminated packets"},
          { format="ipcl-metadata %ipcl-metadata" , name="metadata", help="State the 'client' that IPCL need to use in metedata on udbIndex = 30 (default is : tpid-index)" },
      }
  }
})

-- ************************************************************************
---
--  no_pcl_global_cfg
--        @description  restore PCL Global configuration
--
--        @param params         - params["device"]: device ID
--
--        @param params         - if force_tt_per_vlan exists : restore force_tt_per_vlan to 'true'
--                                if udb30-metadata exists    : restore udb30-metadata to default
--
--        @return       true on success, otherwise false and error message
--
local function no_pcl_global_cfg(params)
    -- Common variables declaration.
    local result
    local devices
    local command_data = Command_Data()
    local force_tt_per_vlan = params["force_tt_per_vlan"];

    if (params["device"]=="all") then
        devices = wrLogWrapper("wrlDevList");
    else
        devices={params["device"]};
    end

    if params.metadata then 
        ipclUdb30GlobalModeSet(nil)-- called before ipclUdb30Config()
    end
    
    for dummy, devNum in pairs(devices) do
        if force_tt_per_vlan then
            result = myGenWrapper("cpssDxCh3PclTunnelTermForceVlanModeEnableSet", {
                        { "IN", "GT_U8",   "devNum", devNum  },
                        { "IN", "GT_BOOL", "enable", true }});
            command_data:handleCpssErrorDevice(result,
                        "cpssDxCh3PclTunnelTermForceVlanModeEnableSet failed", devNum)
        end
        
        if params.metadata then 
            ipclUdb30Config(devNum)
        end
        
    end
    command_data:updateStatus()
    command_data:analyzeCommandExecution()
    command_data:printCommandExecutionResults()

    return command_data:getCommandExecutionResults()
end

CLI_addCommand("config", "no pcl-config", {
  func   = no_pcl_global_cfg,
  help   = "Restore defaults of a Global PCL configurations (restore only mentioned parameters)",
  params = {
      {
          type = "values",
          {format = "%devID_all", name = "device"},
          mandatory = {"device"},
      },
      {
          type = "named",
          {format = "force-tt-per-vlan", name = "force_tt_per_vlan",
              help = "restore default for : CFG Table Access per VLAN for Tunnel Terminated packets (default is true)"},
          { format="ipcl-metadata" , name="metadata", help="restore default for :  the 'client' that IPCL need to use in metedata on udbIndex = 30 (default is : tpid-index)" },
      }
  }
})
