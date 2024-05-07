--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* preemption.lua
--*
--* DESCRIPTION:
--*       Test set ports that can work in preemptive mode to preemptive mode.
--*       Then sanity test that transmit paket from port in preemptive mode is executed.
--*       purpose of example:
--*       1. Check that HW can be configured to preemptive mode
--*       2. Check that control path navigate packets in preemptive channel to correct port.
--*
--*       The test executed in two phases :
--*       Phase a : PMAC and EMAC combined counting
--*       Phase b:  PMAC and EMAC sepparate counting
--* FILE REVISION NUMBER:
--*       $Revision: 22$
--*
--********************************************************************************


local devNum  = devEnv.dev

SUPPORTED_FEATURE_DECLARE(devNum, "PREEMPTION_MAC_802_3BR")

local flush_fdb = "dxCh/examples/configurations/flush_fdb.txt"
local simulation_log = "dxCh/examples/configurations/simulation_log.txt"
local simulation_log_stop = "dxCh/examples/configurations/simulation_log_stop.txt"

local preemptivePorts = {}

local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]
--ports that are configured for preemption
local poweredUpPorts  = "${port[1]},${port[2]},${port[3]},${port[4]},${port[5]},${port[6]}"
local portModeForPreemption = "KR"
local portSpeedForPreemption = "25000"
local portModeForRestore = "KR"
local portSpeedForRestore = "50000"
--ports that are powered down and restored at the end of the test
local poweredDownPorts
local portIterator
local testedPorts = 4

poweredDownPorts=poweredUpPorts


local function callWithErrorHandling(cpssApi, params)
   local rc, value = myGenWrapper(cpssApi, params,1)
   if rc ~= 0 then
       setFailState()
       print("ERROR of calling "..cpssApi..": "..returnCodes[rc])
   end
   return rc, value
end

if is_sip_6_30(devNum) then
  res,value = myGenWrapper("cpssInitSystemGet",{
            {"OUT","GT_U32","boardIdx"},
            {"OUT","GT_U32","boardRevId"},
            {"OUT","GT_U32","multiProcessApp"},
        })
  revision = value["boardRevId"]
  boardType = value["boardIdx"]
  if revision == 5 then
    printLog("Detected Ironman-L RD board due to revision "..value["boardRevId"])
  end

  rc,value = callWithErrorHandling("prvCpssTxqSip6_30_IsDeviceIsIronmanMGet",{
                {"IN","GT_U32","devNum",devNum},
                {"OUT","GT_BOOL","isMType"},
             })
  if value["isMType"]  then
   printLog("Ironman M device detected.")
  end
  if value["isMType"] or  revision == 5 then
  port1   = devEnv.port[1]
  port2   = devEnv.port[6]
  port3   = devEnv.port[3]

  testedPorts=3

  poweredUpPorts  = "${port[1]},${port[3]},${port[6]}"
  poweredDownPorts=poweredUpPorts
  portModeForPreemption = "USX_OUSGMII"
  portSpeedForPreemption = "1000"
  portModeForRestore ="USX_OUSGMII"
  portSpeedForRestore = "1000"
  else
   --need to power down more ports
   poweredDownPorts=poweredUpPorts..",50,51,52,53,54"
   portModeForPreemption = "USX_10G_QXGMII"
   portSpeedForPreemption = "2500"
   portModeForRestore ="KR"
   portSpeedForRestore = "10000"
  end


end

local powerUpPorts =
[[
end
config
interface range ethernet ${dev}/]]..poweredUpPorts..[[

speed ]]..portSpeedForPreemption..[[ mode ]]..portModeForPreemption..[[

no shutdown
exit
exit
]]

local powerDownPorts =
[[
end
config
interface range ethernet ${dev}/]]..poweredDownPorts..[[

no speed
shutdown
exit
exit
]]


local restorePorts =
[[
end
config
interface range ethernet ${dev}/]]..poweredDownPorts..[[

no speed
speed ]]..portSpeedForRestore..[[ mode ]]..portModeForRestore..[[

no shutdown
exit
exit
]]


--executeLocalConfig(simulation_log)
--executeLocalConfig(simulation_log_stop)

-- function to run LUA test from another file
local function runTest(testName)
    print("Run test "..testName)
    local res = pcall(dofile,"dxCh/examples/tests/"..testName)
    if not res then
        setFailState()
    end
    print("Done running test "..testName)
end



local function printCounters(setName,value,macCounterTx,macCounterRx)
    printLog("\n")
    printLog(setName.." TX etherStatsPkts = "..macCounterTx)
    printLog(setName.." TX octetsTransmittedOk = "..value["passPcktsPtr"]["txCounterStc"]["octetsTransmittedOk"]["l"][0])
    printLog(setName.." TX ifOutUcastPkts = "..value["passPcktsPtr"]["txCounterStc"]["ifOutUcastPkts"]["l"][0])
    printLog(setName.." TX ifOutMulticastPkts = "..value["passPcktsPtr"]["txCounterStc"]["ifOutMulticastPkts"]["l"][0])
    printLog(setName.." TX ifOutBroadcastPkts = "..value["passPcktsPtr"]["txCounterStc"]["ifOutBroadcastPkts"]["l"][0])
    printLog(setName.." TX ifOutOctets = "..value["passPcktsPtr"]["txCounterStc"]["ifOutOctets"]["l"][0])
    printLog(setName.." TX aFramesTransmittedOK = "..value["passPcktsPtr"]["txCounterStc"]["aFramesTransmittedOK"]["l"][0])
    printLog(setName.." TX etherStatsPkts64Octets = "..value["passPcktsPtr"]["txCounterStc"]["etherStatsPkts64Octets"]["l"][0])
    printLog(setName.." TX etherStatsPkts65to127Octets = "..value["passPcktsPtr"]["txCounterStc"]["etherStatsPkts65to127Octets"]["l"][0])

    printLog("\n")
    printLog(setName.." RX ifInOctets = "..value["passPcktsPtr"]["rxCounterStc"]["ifInOctets"]["l"][0])
    printLog(setName.." RX etherStatsPkts = "..macCounterRx.."\n")
    printLog("\n")
end

-- run the test
function preemptionTest(sepparateCount)

    local macMode
    local cncPassCounter,macCounterTx,macCounterRx

    if is_sip_7(devNum) then
     printLog("AAS does not have full preemption support so skip the test")
     setTestStateSkipped()
     return
    end
    if(sepparateCount == true)then
     macMode = "CPSS_PM_MAC_PREEMPTION_ENABLED_SEPARATED_E"
    else
     macMode = "CPSS_PM_MAC_PREEMPTION_ENABLED_AGGREGATED_E"
    end

    printLog("\n\n\nStart iteration with macMode = "..macMode)

    printLog("Power down all ports")

    printLog("Using ports 1 -"..port1.." 2 -"..port2.." 3 -"..port3.." 4 -"..port4)

    executeStringCliCommands(powerDownPorts)
    printLog("Configure TC 0 to be preemptive.")

    local tcDpRemapping = {}
    local rc,value,j
    local preemptionParams = {}

    j=1

    for i=1,2 do

    tcDpRemapping = {
        tc = 0,
        dp = "CPSS_DP_GREEN_E",
        isStack = "CPSS_DXCH_PORT_PROFILE_NETWORK_E",
        dsaTagCmd = "CPSS_DXCH_NET_DSA_CMD_FORWARD_E",
        targetPortTcProfile = "CPSS_DXCH_PORT_PROFILE_NETWORK_PREEMPTIVE_E",
        packetIsMultiDestination = false
    }
    --set for UC and MC
    if(i==2)then
     tcDpRemapping.packetIsMultiDestination=true
    end

    rc,value = callWithErrorHandling("cpssDxChCscdQosTcDpRemapTableGet",{
                {"IN","GT_U32","devNum",devNum},
                {"IN","CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC","tcDpRemapping",tcDpRemapping},
                {"OUT","GT_U32","newTcPtr"},
                {"OUT","CPSS_DP_LEVEL_ENT","newDpPtr"},
                {"OUT","GT_U32","newPfcTPtrc"},
                {"OUT","CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT","mcastPriorityPtr"},
                {"OUT","CPSS_DP_FOR_RX_ENT","dpForRxPtr"},
                {"OUT","GT_BOOL","preemptiveTcPtr"},
             })

    callWithErrorHandling("cpssDxChCscdQosTcDpRemapTableSet",{
            {"IN","GT_U32","devNum",devNum},
            {"IN","CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC","tcDpRemapping",tcDpRemapping},
            {"IN","GT_U32","newTc",0},
            {"IN","CPSS_DP_LEVEL_ENT","newDp","CPSS_DP_GREEN_E"},
            {"IN","GT_U32","newPfcTc",0},
            {"IN","CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT","mcastPriority","CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E"},
            {"IN","CPSS_DP_FOR_RX_ENT","dpForRx",value.dpForRxPtr},
            {"IN","GT_BOOL","preemptiveTc",true},
        })
    end

     printLog("Add LUA ports to preemptive profile and enable preemption.")
     for i=1,testedPorts do
     if     i == 1 then portIterator = port1
     elseif i == 2  then portIterator = port2
     elseif i == 3  then portIterator = port3
     elseif i == 4  then portIterator = port4
     end

      --check that this port support preemption
      rc,value=callWithErrorHandling("prvCpssFalconTxqUtilsPortPreemptionAllowedGet",{
            {"IN","GT_U32","devNum",devNum},
            {"IN","GT_PHYSICAL_PORT_NUM","portNum",portIterator},
            {"OUT","GT_BOOL","preemptionAllowedPtr"},
        })

      if(value["preemptionAllowedPtr"]==true)then
       --do not check send port since sent on queue 7 from CPU
         if(port2~=portIterator )then
          printLog(" Add  port  "..devEnv.port[i])
          callWithErrorHandling("cpssDxChCscdPortTcProfiletSet",{
                {"IN","GT_U32","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",portIterator},
                {"IN","CPSS_PORT_DIRECTION_ENT","portDirection","CPSS_PORT_DIRECTION_TX_E"},
                {"IN","CPSS_DXCH_PORT_PROFILE_ENT","portProfile","CPSS_DXCH_PORT_PROFILE_NETWORK_PREEMPTIVE_E"},
            })
          printLog(" Enable preemption on port  "..portIterator)
          preemptionParams = {
                type = macMode,
                minFragSize = "CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_64_BYTE_E",
                preemptionMethod = "CPSS_PM_MAC_PREEMPTION_METHOD_NORMAL_E",
                disableVerification = false
          }

          callWithErrorHandling("prvCpssDxChPortPreemptionParamsSetWrapper",{
                {"IN","GT_U32","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","physicalPortNum",portIterator},
                {"IN","CPSS_PM_MAC_PREEMPTION_PARAMS_STC","preemptionParamsPtr",preemptionParams}
            })


           preemptivePorts[j] = portIterator
           j=j+1
          else
           printLog(" Preemption supported on port "..portIterator.." but the port will not be checked ")
          end
      else
      printLog(" Preemption not supported on port  "..portIterator)
      end   --   if(value["preemptionAllowedPtr"]==true)then
     end

     printLog(" Reconfigure(power up)  ports in order for preemption to kick in")

     executeStringCliCommands(powerUpPorts)


     --Initialize CNC counters to count pass/drop for each port
     callWithErrorHandling("prvCpssFalconQueueStatisticInit",{
            {"IN","GT_U32","devNum",devNum}
        })


    printLog("\nFlush FDB.\n ")
    -- run the test:
--  'sanity_send_and_capture.lua'
    executeLocalConfig(flush_fdb)
    runTest("sanity_learn_mac.lua")

    --check CNC counters
    printLog("Check CNC counters for preemptive ports")
    for j=1,#preemptivePorts do
     printLog("Port "..tostring(preemptivePorts[j]))

     rc,value =callWithErrorHandling("prvCpssSip6TxqDebugCncCounterGet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",preemptivePorts[j]},
        {"IN","GT_U32","queue",0},
        {"IN","GT_BOOL","preemptiveMode",true},
        {"OUT","GT_U32","passPcktsPtr"},
        {"OUT","GT_U32","droppedPcktsPtr"},
    })

     printLog(" CNC Pass packets "..value.passPcktsPtr)
     cncPassCounter = tonumber(value.passPcktsPtr)
     printLog(" CNC Drop packets "..value.droppedPcktsPtr)
     if(value.passPcktsPtr==0 or value.droppedPcktsPtr~=0)then
       printLog(" Counter value unexpected.Failing the test.")
      setFailState()
     else
      printLog(" CNC counters value as expected.")
     end
     --check MAC counters manually
     if(sepparateCount == true)then
      printLog("Check PMAC counters for preemptive ports")

     rc,value =callWithErrorHandling("cpssDxChPortMacBrCountersOnMtiPortGet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",preemptivePorts[j]},
        {"IN","CPSS_DXCH_PORT_MAC_BR_COUNTER_TYPE_ENT","macCountMode","CPSS_DXCH_PORT_MAC_BR_COUNTER_TYPE_PMAC_E"},
        {"OUT","CPSS_PORT_MAC_MTI_COUNTER_SET_STC","passPcktsPtr"}
     })

     macCounterTx = value["passPcktsPtr"]["txCounterStc"]["etherStatsPkts"]["l"][0]
     macCounterRx = value["passPcktsPtr"]["rxCounterStc"]["etherStatsPkts"]["l"][0]

     printCounters("PMAC",value,macCounterTx,macCounterRx)

     if(macCounterTx ~= cncPassCounter)then
       printLog(" PMAC tx counter value unexpected "..macCounterTx..".Should be same as CNC "..cncPassCounter.."Failing the test.")
      setFailState()
     else
      printLog(" PMAC tx counter value as expected value == "..macCounterTx)
     end

     if(macCounterRx ~= 0)then
      printLog(" EMAC rx counter value unexpected "..macCounterRx..".Should be 0 .Failing the test.")
      setFailState()
     else
      printLog(" EMAC tx counter value as expected. Value == "..macCounterRx)
     end

     printLog("Check EMAC counters for preemptive ports")

    rc,value =callWithErrorHandling("cpssDxChPortMacBrCountersOnMtiPortGet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",preemptivePorts[j]},
        {"IN","CPSS_DXCH_PORT_MAC_BR_COUNTER_TYPE_ENT","macCountMode","CPSS_DXCH_PORT_MAC_BR_COUNTER_TYPE_EMAC_E"},
        {"OUT","CPSS_PORT_MAC_MTI_COUNTER_SET_STC","passPcktsPtr"}
     })
     macCounterTx = value["passPcktsPtr"]["txCounterStc"]["etherStatsPkts"]["l"][0]
     macCounterRx = value["passPcktsPtr"]["rxCounterStc"]["etherStatsPkts"]["l"][0]

     printCounters("EMAC",value,macCounterTx,macCounterRx)

     if(macCounterTx ~= 0)then
       printLog(" EMAC TX counter value unexpected "..macCounterTx..".Should be 0 .Failing the test.")
      setFailState()
     else
      printLog(" EMAC TX counter value as expected. Value == "..macCounterTx)
     end

     if(macCounterRx ~= 0)then
       printLog(" EMAC RX counter value unexpected "..macCounterRx..".Should be 0 .Failing the test.")
      setFailState()
     else
      printLog(" EMAC RX counter value as expected. Value == "..macCounterRx)
     end

     end
    end

    --[[RESTORE]]--


for i=1,2 do

    tcDpRemapping = {
        tc = 0,
        dp = "CPSS_DP_GREEN_E",
        isStack = "CPSS_DXCH_PORT_PROFILE_NETWORK_E",
        dsaTagCmd = "CPSS_DXCH_NET_DSA_CMD_FORWARD_E",
        targetPortTcProfile = "CPSS_DXCH_PORT_PROFILE_NETWORK_PREEMPTIVE_E",
        packetIsMultiDestination = false
    }
    --set for UC and MC
    if(i==2)then
     tcDpRemapping.packetIsMultiDestination=true
    end

    rc,value = callWithErrorHandling("cpssDxChCscdQosTcDpRemapTableGet",{
                {"IN","GT_U32","devNum",devNum},
                {"IN","CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC","tcDpRemapping",tcDpRemapping},
                {"OUT","GT_U32","newTcPtr"},
                {"OUT","CPSS_DP_LEVEL_ENT","newDpPtr"},
                {"OUT","GT_U32","newPfcTPtrc"},
                {"OUT","CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT","mcastPriorityPtr"},
                {"OUT","CPSS_DP_FOR_RX_ENT","dpForRxPtr"},
                {"OUT","GT_BOOL","preemptiveTcPtr"},
             })

    callWithErrorHandling("cpssDxChCscdQosTcDpRemapTableSet",{
            {"IN","GT_U32","devNum",devNum},
            {"IN","CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC","tcDpRemapping",tcDpRemapping},
            {"IN","GT_U32","newTc",0},
            {"IN","CPSS_DP_LEVEL_ENT","newDp","CPSS_DP_GREEN_E"},
            {"IN","GT_U32","newPfcTc",0},
            {"IN","CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT","mcastPriority","CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E"},
            {"IN","CPSS_DP_FOR_RX_ENT","dpForRx",value.dpForRxPtr},
            {"IN","GT_BOOL","preemptiveTc",false},
        })
 end

 executeStringCliCommands(powerDownPorts)

for j=1,#preemptivePorts do
     printLog(" Read PMAC counters in order to clear")
--[[

     Read PMAC counters at the end of the test in order to clear .
     If aggregated PMAC/EMAC mode is engaged then PMAC counter is also incremented
     and next ineration will result in inaccurate count.

]]--
     callWithErrorHandling("cpssDxChPortMacBrCountersOnMtiPortGet",{
        {"IN","GT_U8","devNum",devNum},
        {"IN","GT_PHYSICAL_PORT_NUM","portNum",preemptivePorts[j]},
        {"IN","CPSS_DXCH_PORT_MAC_BR_COUNTER_TYPE_ENT","macCountMode","CPSS_DXCH_PORT_MAC_BR_COUNTER_TYPE_PMAC_E"},
        {"OUT","CPSS_PORT_MAC_MTI_COUNTER_SET_STC","passPcktsPtr"}
     })

    preemptionParams = {
                type = "CPSS_PM_MAC_PREEMPTION_DISABLED_E",
                minFragSize = "CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_64_BYTE_E",
                preemptionMethod = "CPSS_PM_MAC_PREEMPTION_METHOD_NORMAL_E",
                disableVerification = false
          }
     printLog("Disable preemption on port "..tostring(preemptivePorts[j]))
     callWithErrorHandling("prvCpssDxChPortPreemptionParamsSetWrapper",{
                {"IN","GT_U32","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","physicalPortNum",preemptivePorts[j]},
                {"IN","CPSS_PM_MAC_PREEMPTION_PARAMS_STC","preemptionParamsPtr",preemptionParams}
            })

     callWithErrorHandling("cpssDxChCscdPortTcProfiletSet",{
                {"IN","GT_U32","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",preemptivePorts[j]},
                {"IN","CPSS_PORT_DIRECTION_ENT","portDirection","CPSS_PORT_DIRECTION_TX_E"},
                {"IN","CPSS_DXCH_PORT_PROFILE_ENT","portProfile","CPSS_DXCH_PORT_PROFILE_NETWORK_E"},
            })
end
         --Take care of CNC counters deinit
     callWithErrorHandling("prvCpssFalconQueueStatisticDeInit",{
            {"IN","GT_U32","devNum",devNum}
        })

     printLog(" Reconfigure ports in order for preemption disable to kick in.")
     executeStringCliCommands(restorePorts)

end





