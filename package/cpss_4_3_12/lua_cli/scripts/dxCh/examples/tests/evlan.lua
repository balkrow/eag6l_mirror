--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* evlan.lua
--*
--* DESCRIPTION:
--*       The test for testing vlan with VID > 4095 support
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local devNum  = devEnv.dev
local evlan_id = is_sip_7(devNum) and 4098 or 4097
-- this test is relevant for SIP_5 devices (BOBCAT 2 and above)
SUPPORTED_SIP_DECLARE(devNum,"SIP_5")
SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM(devNum,""--[[no specific feature]],"EVLAN",evlan_id)
SUPPORTED_SINGLE_FEATURE_DECLARE_WITH_PARAM(devNum,"eVlan>=4K","ports",{devEnv.port[1],devEnv.port[2],devEnv.port[3],devEnv.port[4]})

local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]

local payloads
local status

--generate test packets
do
    status, payloads = pcall(dofile, "dxCh/examples/packets/evlan.lua")
    if not status then
        printLog ('Error in packet generator')
        setFailState()
        return
    end
end

-- clean FDB before test
executeLocalConfig("dxCh/examples/configurations/flush_fdb.txt")

printLog("============================== Set configuration ===============================")
if is_sip_7(devNum) then
    executeLocalConfig("dxCh/examples/configurations/sip7_evlan.txt")
else
    executeLocalConfig("dxCh/examples/configurations/evlan.txt")
end

--reset ports counters
resetPortCounters(devEnv.dev, devEnv.port[1])
resetPortCounters(devEnv.dev, devEnv.port[2])
resetPortCounters(devEnv.dev, devEnv.port[3])
resetPortCounters(devEnv.dev, devEnv.port[4])

--teach the device about mac DA of the next packet , so it will do single destination and not flood
--Get hwDevNum
local ret, val = myGenWrapper("cpssDxChCfgHwDevNumGet",{
    {"IN","GT_U8","devNum", devNum},
    {"OUT","GT_HW_DEV_NUM","hwDevNumPtr"}
})
if (ret ~= 0) then
    printLog ("Error in cpssDxChCfgHwDevNumGet")
    setFailState()
end
local hw_device_id_number = val["hwDevNumPtr"]; 
local mac_entry = {
        key = {
            entryType = "CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E",
            key = {
                macVlan = {
                    macAddr = "00:00:00:00:11:22",
                    vlanId = evlan_id
                }
            }
        },
        dstInterface = {
            type = "CPSS_INTERFACE_PORT_E",
            devPort = {
                hwDevNum = hw_device_id_number,
                devNum = hw_device_id_number,--the lua wrapper use this feild and NOT the 'hwDevNum'
                portNum = devEnv.port[2]
            }
        },
        daCommand = "CPSS_MAC_TABLE_FRWRD_E"
    }

printLog(" set the mac DA entry :"..to_string(mac_entry))
local ret, val = mac_entry_set(devNum, mac_entry);
if ret ~= 0 then
    printLog ("Error in mac_entry_set")
    setFailState()
end 

local transmitInfo;
transmitInfo = {devNum = devNum, portNum = port1, pktInfo = {fullPacket = payloads["my_untagged_packet"]} }
local egressInfoTable = {
    {portNum = port2, packetCount = 1 },
    {portNum = port3, packetCount = 0 },-- make sure no flood happened (due to wrong FDB learning)
    {portNum = port4, packetCount = 0 } -- make sure no flood happened (due to wrong FDB learning)
}
rc = luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)

if not rc == 0 then
    printLog ("Test failed \n")
    setFailState()
end

local command_showFdbEntry = 
[[
    end
    show mac address-table all device ${dev}
    end
]]

local isOk, msg = pcall(executeStringCliCommands, command_showFdbEntry)
if not isOk then
    setFailState()
end

local validCnt = 0
local index = 0
local enumTable = 1 -- CPSS_DXCH_CFG_TABLE_FDB_E = 1
local fdbEnd = false
local entries = {}

local function getNumOfFdbEntriesAndFirstEntry()

    repeat
        nextIdxStatus, output = cpssGenWrapper("cpssDxChTableValidIndexGetNext",{
                                                {"IN","GT_U8", "devNum", devNum},
                                                { "IN", "CPSS_DXCH_CFG_TABLES_ENT", "enumTable", enumTable },
                                                {"INOUT","GT_U32","entryIndexPtr", index}})

        if nextIdxStatus == 12 then -- GT_NO_MORE (12) - No more items found
            break
        elseif nextIdxStatus ~= 0 then
            print("error - cpssDxChTableValidIndexGetNext failed")
            break
        end

        index = output.entryIndexPtr
        local result, values = myGenWrapper(
            "cpssDxChBrgFdbMacEntryRead", {
                { "IN", "GT_U8", "dev", devNum},            -- devNum
                { "IN", "GT_U32", "index", index },
                { "OUT","GT_BOOL", "valid" },               -- *validPtr,
                { "OUT","GT_BOOL", "skip" },                -- *skipPtr,
                { "OUT","GT_BOOL", "aged" },                -- *agedPtr,
                { "OUT","GT_U8", "devNum" },                -- *associatedDevNumPtr,
                { "OUT","CPSS_MAC_ENTRY_EXT_STC", "entry" } --  *entryPtr
            })

        if values.valid and not values.skip then
            table.insert(entries, values)
            validCnt = validCnt + 1
        end

        index = index + 1
    until nextIdxStatus == 12 -- GT_NO_MORE (12) - No more items found

end
-- limit the iterations (in GM it uses the 'FDB shadow' due to poor performance of the GM)
local function gmUsed_getNumOfFdbEntriesAndFirstEntry()
    local command_data = Command_Data()
    local params = {all_device = {devNum}}
-- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllDeviceMacEntryIterator(
        params, { ["dstInterface"] =
                    {["type"]    = { "CPSS_INTERFACE_PORT_E",
                                     "CPSS_INTERFACE_TRUNK_E",
                                     "CPSS_INTERFACE_VID_E"    }}})
                                     
    --printLog("params:" ,to_string(params))
                                     
    for iterator, _devNum, entry_index in
                                command_data:getValidMacEntryIterator() do
        --printLog("iterator" , to_string(iterator))
        --printLog("_devNum" , to_string(_devNum))
        --printLog("entry_index" , to_string(entry_index))
                                
        local result, values = myGenWrapper(
        "cpssDxChBrgFdbMacEntryRead", {
            { "IN", "GT_U8", "dev", devNum},            -- devNum
            { "IN", "GT_U32", "index", entry_index },
            { "OUT","GT_BOOL", "valid" },               -- *validPtr,
            { "OUT","GT_BOOL", "skip" },                -- *skipPtr,
            { "OUT","GT_BOOL", "aged" },                -- *agedPtr,
            { "OUT","GT_U8", "devNum" },                -- *associatedDevNumPtr,
            { "OUT","CPSS_MAC_ENTRY_EXT_STC", "entry" } --  *entryPtr
        })

        if result == 0 then
            if values.valid and not values.skip then
                --printLog("values.entry" , to_string(values.entry))
                validCnt = validCnt + 1
                table.insert(entries, values)
            end
        end
    end
                                     
end

if useFdbShadow() then
    gmUsed_getNumOfFdbEntriesAndFirstEntry()
else
    getNumOfFdbEntriesAndFirstEntry()
end

 -- printLog("DBG" .. to_string(entries))

if validCnt == 2 
   and entries[1].entry.key.key.macVlan.vlanId == evlan_id 
   and entries[2].entry.key.key.macVlan.vlanId == evlan_id  then
    printLog ("Found the 2 FDB entries (SA,DA) on evlan[".. evlan_id .."]\n")
    printLog ("Test passed \n")
else
    printLog ("Error : Not Found FDB entry on evlan[".. evlan_id .."]\n")
    printLog ("Test failed \n")
    setFailState()
end


printLog(" delete the mac DA entry :"..to_string(mac_entry))
local ret, val = mac_entry_delete(devNum, mac_entry.key);
if ret ~= 0 then
    printLog ("Error in mac_entry_delete")
    setFailState()
end 

printLog("============================ Restore configuration =============================")
if is_sip_7(devNum) then
    executeLocalConfig("dxCh/examples/configurations/sip7_evlan_deconfig.txt")
else
    executeLocalConfig("dxCh/examples/configurations/evlan_deconfig.txt")
end
