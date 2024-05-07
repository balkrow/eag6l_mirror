--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* qbv.lua
--*
--* DESCRIPTION:
--*     1.Bind to all tc to gate 3
--*     2.Configure tableset with one entry with gate 3 closed
--*     3.Bind port to tableset (Configuration time=0 so the configuration will be taken instantly.)
--*     4.Transmit packet to that port.
--*     5.Check that packet is not transmitted.
--*     6.Open the gate.
--*     7.Transmit packet to that port.
--*     8.Check that packet is not transmitted.
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1$
--*
--********************************************************************************

SUPPORTED_FEATURE_DECLARE(devNum, "QBV")


-- function to run LUA test from another file
local function runTest(testName)
    print("Run test "..testName)
    local res = pcall(dofile,"dxCh/examples/tests/"..testName)
    if not res then
        setFailState()
    end
    print("Done running test "..testName)
end



local function callWithErrorHandling(cpssApi, params)
   local rc, value = myGenWrapper(cpssApi, params,1)
   if rc ~= 0 then
       setFailState()
       print("ERROR of calling "..cpssApi..": "..returnCodes[rc])
   end
   return rc, value
end

local devNum  = devEnv.dev
local port   = devEnv.port[2]
local gate=3
local tableset=28
local timeSlotInfoArr = {}
local cycleTime=0xFFFFFFF --28 bit
local goodOctetsSent
local rc,val
local speed,mode

speed="10000"
mode="KR"

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
    speed="1000"
    mode="USX_OUSGMII"
  end
end

local generate_Traffic =
[[
end
traffic
traffic-generator emulate port ${dev}/]] .. port .. [[ data 000111222333000222333444 count 1
]]


local show_map =
[[
dbg txq-sip6-qbv  port-config device ${dev}]]..[[ port ]]..port..[[
]]

local init_cnc =
[[
dbg txq-sip6-enqueue-stat  device ${dev}
]]

local show_cnc =
[[
dbg txq-sip6-show pass-drop device ${dev}]]..[[ port ]]..port..[[ queue 7
]]


local deinit_cnc =
[[
dbg no txq-sip6-enqueue-stat  device ${dev}
]]

local show_linkup =
[[
 dbg txq-sip6 port-link-up-state device ${dev}]]..[[ port ]]..port..[[
 ]]

local reconfig =
[[
do configure

interface ethernet ${dev}/]] .. port..[[
]]..
[[

no speed
shutdown
speed ]]..speed..[[ mode ]]..mode..[[

no shutdown
exit
]]
local show_dbg =
[[
dbg register read device 0 offset 0x8d50c050
dbg register read device 0 offset 0x8D50B100
dbg register read device 0 offset 0x8D50B104
dbg register read device 0 offset 0x8D50B108
]]
--[[
    THE TEST START
]]--

local sleepTime=5000

print("Port under test is "..port)

--Bind all queues to gate 3
print("1.Bind all queues to gate "..gate)
 for i=0,7 do
    rc= callWithErrorHandling("cpssDxChStreamEgressPortQueueGateSet",{
                {"IN","GT_U32","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
                {"IN","GT_U32","queueOffset",i},
                {"IN","GT_U32","gate",gate},
             })
 end
print(" Step 1 is done.")

-- Configure tableset with one entry with gate 3 closed
print("2.Configure tableset with one entry with closed gate "..gate.." .tableset is "..tableset)

timeSlotInfoArr = {
    timeToAdvance = cycleTime,
    gateStateBmp = 0x8, --gate 3 closed
    hold = false
}

rc= callWithErrorHandling("cpssDxChStreamEgressTableSetConfigSet",{
            {"IN","GT_U32","devNum",devNum},
            {"IN","GT_U32","tableSet",tableset},
            {"IN","GT_U32","slotsNum",1},
            {"IN","CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC","timeSlotInfoArr",timeSlotInfoArr},
         })

print(" Step 2 is done.")


--Bind port to tableset (Configuration time=0 so the configuration will be taken instantly.)
print("3.Bind port "..port.." to tableset "..tableset)

rc= callWithErrorHandling("prvCpssSip6_30TxqQbvInstantBindPortToTableSet",{
            {"IN","GT_U32","devNum",devNum},
            {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
            {"IN","GT_U32","tableSet",tableset},
            {"IN","GT_U32","numOfEntries",1},
         })

--Check bind is done


rc= callWithErrorHandling("cpssDxChStreamEgressPortBindComplete",{
            {"IN","GT_U32","devNum",devNum}
         })

print(" Step 3 is done.Bind is complete.")


print(" Sleep "..sleepTime.." ms")
delay(sleepTime)
--Transmit packet to that port.

rc= callWithErrorHandling("cpssDxChPortMacCountersClearOnReadSet",{
                {"IN","GT_U32","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
                {"IN","GT_BOOL","enable",true}
             })

--reset
rc,value =callWithErrorHandling("cpssDxChPortMacCountersOnPortGet",{
    {"IN","GT_U8","devNum",devNum},
    {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
    {"OUT","CPSS_PORT_MAC_COUNTER_SET_STC","portMacCounterSetArrayPtr"}
 })

 rc= callWithErrorHandling("cpssDxChPortMacCountersClearOnReadSet",{
                {"IN","GT_U32","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
                {"IN","GT_BOOL","enable",false}
             })

rc,val= callWithErrorHandling("cpssDxChStreamEgressTableSetConfigGet",{
            {"IN","GT_U32","devNum",devNum},
            {"IN","GT_U32","tableSet",tableset},
            {"IN","GT_U32","slotsNum",1},
            {"OUT","CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC","timeSlotInfoArr"},
         })
print(string.format("Gate BMP 0x%x",val.timeSlotInfoArr.gateStateBmp))
executeStringCliCommands(init_cnc)
executeStringCliCommands(show_map)
executeStringCliCommands(generate_Traffic)
executeStringCliCommands(show_cnc)

printLog("Check MAC counters for port  "..port)

rc,value =callWithErrorHandling("cpssDxChPortMacCountersOnPortGet",{
    {"IN","GT_U8","devNum",devNum},
    {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
    {"OUT","CPSS_PORT_MAC_COUNTER_SET_STC","portMacCounterSetArrayPtr"}
 })
goodOctetsSent = value["portMacCounterSetArrayPtr"]["goodOctetsSent"]["l"][0]

print("goodOctetsSent "..goodOctetsSent)

if(goodOctetsSent ~= 0)then
   printLog(" MAC tx counter value unexpected "..goodOctetsSent..".Should be 0 .Failing the test.")
  setFailState()
 else
  printLog(" MAC tx counter value as expected value == "..goodOctetsSent)
 end
executeStringCliCommands(show_linkup)

--check egress port
rc,value =callWithErrorHandling("cpssDxChPortTxBufNumberGet",{
    {"IN","GT_U8","devNum",devNum},
    {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
    {"OUT","GT_U32","numPtr"}
 })
 print("Number of enqueued buffers in port  "..port.." is "..value.numPtr)

 --executeStringCliCommands(show_dbg)
 --Bind to opened gate
 print("4.Bind all queues to gate "..gate+1)
 for i=0,7 do
    rc= callWithErrorHandling("cpssDxChStreamEgressPortQueueGateSet",{
                {"IN","GT_U32","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
                {"IN","GT_U32","queueOffset",i},
                {"IN","GT_U32","gate",gate+1},
             })
 end
print(" Step 4 is done.")


--Transmit packet to that port.
executeStringCliCommands(show_linkup)

rc= callWithErrorHandling("cpssDxChPortMacCountersClearOnReadSet",{
                {"IN","GT_U32","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
                {"IN","GT_BOOL","enable",true}
             })

--reset
rc,value =callWithErrorHandling("cpssDxChPortMacCountersOnPortGet",{
    {"IN","GT_U8","devNum",devNum},
    {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
    {"OUT","CPSS_PORT_MAC_COUNTER_SET_STC","portMacCounterSetArrayPtr"}
 })

 rc= callWithErrorHandling("cpssDxChPortMacCountersClearOnReadSet",{
                {"IN","GT_U32","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
                {"IN","GT_BOOL","enable",false}
             })

rc,val= callWithErrorHandling("cpssDxChStreamEgressTableSetConfigGet",{
            {"IN","GT_U32","devNum",devNum},
            {"IN","GT_U32","tableSet",tableset},
            {"IN","GT_U32","slotsNum",1},
            {"OUT","CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC","timeSlotInfoArr"},
         })
print(string.format("Gate BMP 0x%x",val.timeSlotInfoArr.gateStateBmp))
executeStringCliCommands(show_map)
executeStringCliCommands(reconfig)
executeStringCliCommands(show_linkup)
executeStringCliCommands(generate_Traffic)
executeStringCliCommands(show_cnc)

printLog("Check MAC counters for port  "..port)

rc,value =callWithErrorHandling("cpssDxChPortMacCountersOnPortGet",{
    {"IN","GT_U8","devNum",devNum},
    {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
    {"OUT","CPSS_PORT_MAC_COUNTER_SET_STC","portMacCounterSetArrayPtr"}
 })
goodOctetsSent = value["portMacCounterSetArrayPtr"]["goodOctetsSent"]["l"][0]

print("goodOctetsSent "..goodOctetsSent)

if(goodOctetsSent == 0)then
   printLog(" MAC tx counter value unexpected "..goodOctetsSent..".Should be greater then 0 .Failing the test.")
  setFailState()
 else
  printLog("goodOctetsSent value as expected value == "..goodOctetsSent)
 end

-- restore configurations

rc= callWithErrorHandling("cpssDxChPortMacCountersClearOnReadSet",{
                {"IN","GT_U32","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
                {"IN","GT_BOOL","enable",true}
             })

rc,value =callWithErrorHandling("cpssDxChStreamEgressPortUnbindSet",{
    {"IN","GT_U8","devNum",devNum},
    {"IN","GT_PHYSICAL_PORT_NUM","portNum",port}
 })

 for i=0,7 do
    rc= callWithErrorHandling("cpssDxChStreamEgressPortQueueGateSet",{
                {"IN","GT_U32","devNum",devNum},
                {"IN","GT_PHYSICAL_PORT_NUM","portNum",port},
                {"IN","GT_U32","queueOffset",i},
                {"IN","GT_U32","gate",0},
             })
 end
executeStringCliCommands(show_map)
executeStringCliCommands(deinit_cnc)
