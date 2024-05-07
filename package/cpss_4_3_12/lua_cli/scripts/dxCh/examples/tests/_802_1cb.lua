--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* _802_1cb.lua
--*
--* DESCRIPTION:
--*       The test for 802.1cb
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

SUPPORTED_SIP_DECLARE(devEnv.dev,"SIP_6_30")

if is_IronMan_M_in_system() then
    printLog("IronMan M does not have enough ports for the test")
    setTestStateSkipped()
    return
end

local skip_part_1 = false
local skip_part_2 = false
local skip_part_3 = false
local debug_single_packetNum = nil--"3-10"
local debugRange = {
    start = nil--"1-1"
    ,last = nil--"3-1"
}


local testName = "802.1cb"
--test that need 'simulation log'
local ssl_test = nil--"3-11"
local sslw = false

cmdLuaCLI_registerCfunction("wrlDxChDsaToString")

-- save the orig ports
local orig_ports = {
    devEnv.port[1],
    devEnv.port[2],
    devEnv.port[3],
    devEnv.port[4],
    devEnv.port[5],
    devEnv.port[6],
}

local function restoreOrigPorts()
    --restore orig-ports :
    devEnv.port[1]  = orig_ports[1]
    devEnv.port[2]  = orig_ports[2]
    devEnv.port[3]  = orig_ports[3]
    devEnv.port[4]  = orig_ports[4]
    devEnv.port[5]  = orig_ports[5]
    devEnv.port[6]  = orig_ports[6]
    devEnv.port[7]  = nil
    devEnv.port[8]  = nil
    devEnv.port[9]  = nil
    devEnv.port[10] = nil
    devEnv.port[11] = nil
    devEnv.port[12] = nil
    devEnv.port[13] = nil
    devEnv.port[14] = nil
    devEnv.port[15] = nil
    devEnv.port[16] = nil
end

-- index                  1    2    3    4    5    6    7    8    9   10   11   12   13   14  15  16
-- not valid                        x    x                        x
local available_ports = { 0 ,  4 ,  1 ,  2 , 16 , 20 , 51 , 49 ,  3 , 48 , 52 , 53 , 54 , 8 , 12 , 50}

devEnv.port[1]      = available_ports[1]
devEnv.port[2]      = available_ports[2]
devEnv.port[3]      = available_ports[3]
devEnv.port[4]      = available_ports[4]
devEnv.port[5]      = available_ports[5]
devEnv.port[6]      = available_ports[6]
devEnv.port[7]      = available_ports[7]
devEnv.port[8]      = available_ports[8]
devEnv.port[9]      = available_ports[9]
devEnv.port[10]     = available_ports[10]
devEnv.port[11]     = available_ports[11]
devEnv.port[12]     = available_ports[12]
devEnv.port[13]     = available_ports[13]
devEnv.port[14]     = available_ports[14]
devEnv.port[15]     = available_ports[15]
devEnv.port[16]     = available_ports[16]
printLog("Array of devEnv.port = ",to_string(devEnv.port))

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]
local port5   = devEnv.port[5]
local port6   = devEnv.port[6]
local port7   = devEnv.port[7]
local port8   = devEnv.port[8]
local port9   = devEnv.port[9]
local port10  = devEnv.port[10]
local port11  = devEnv.port[11]
local port12  = devEnv.port[12]
local port13  = devEnv.port[13]
local port14  = devEnv.port[14]
local port15  = devEnv.port[15]
local port16  = devEnv.port[16]
local portCPU = devEnv.portCPU -- cpu port

local command_data = Command_Data()


local function word0_of_bmp(port0,port1)
    local value = 0

    if port0 < 32 then value = value + (2^port0) end
    if port1 and port1 < 32 then value = value + (2^port1) end

    return value
end
local function word1_of_bmp(port0,port1)
    local value = 0

    if port0 < 64 and port0 >= 32 then value = value + (2^(port0-32)) end
    if port1 and port1 < 64 and port1 >= 32 then value = value + (2^(port1-32)) end

    return value
end

local function makeMacAddrTestFormat (macAddr)
    local str = ""
    local newPart,offset,ii

    for ii = 1 , 6 do
        offset = (ii-1)*2
        newPart = string.sub(macAddr, 1 + offset, 2 + offset)
        if(str == "")then
            str = newPart
        else
            str = str .. ":" .. newPart
        end
    end

    return str
end

local info_802_1cb = {}

info_802_1cb.vlanId1 = 1
info_802_1cb.vlanId2 = 2
info_802_1cb.vlanId3 = 3
info_802_1cb.vlanId4 = 4

local macA  = "001122334455"
local macB1 = "00AA22334457"
local macB2 = "00AA22334458"
local macC  = "00987654321F"
local macD  = "001122334456"
local macE  = "001122334457"
local macF  = "00987654321E"
local macG  = "00FFEEDDBBA3"
local macH  = "00987654321A"
local macM  = "010022334458"
local macM1 = "010022334468"
local macM2 = "010022334469"

info_802_1cb.macA = makeMacAddrTestFormat(macA)
info_802_1cb.macB1= makeMacAddrTestFormat(macB1)
info_802_1cb.macC = makeMacAddrTestFormat(macC)
info_802_1cb.macD = makeMacAddrTestFormat(macD)
info_802_1cb.macE = makeMacAddrTestFormat(macE)
info_802_1cb.macF = makeMacAddrTestFormat(macF)

info_802_1cb.macA_fdb  =  makeMacAddrTestFormat(macA)
info_802_1cb.macB1_fdb =  makeMacAddrTestFormat(macB1)
info_802_1cb.macB2_fdb =  makeMacAddrTestFormat(macB2)
info_802_1cb.macM_fdb  =  makeMacAddrTestFormat(macM)
info_802_1cb.macE_fdb  =  makeMacAddrTestFormat(macE)
info_802_1cb.macF_fdb  =  makeMacAddrTestFormat(macF)

local ip1 = "192.16.1.1"
local ip2 = "192.16.1.2"
local ip3 = "192.16.1.3"
local ip4 = "192.16.1.4"

local ip91 = "192.16.9.1"
local ip92 = "192.16.9.2"
local ip93 = "192.16.9.3"
local dummy_sip = "5.6.7.8"

info_802_1cb.ip1 = ip1
info_802_1cb.ip2 = ip2
info_802_1cb.ip3 = ip3
info_802_1cb.ip4 = ip4

info_802_1cb.ip91 = ip91
info_802_1cb.ip92 = ip92
info_802_1cb.ip93 = ip93

info_802_1cb.vidx1001 = 1001
info_802_1cb.vidx1002 = 1002
info_802_1cb.vidx1003 = 1003
info_802_1cb.vidx1151 = 1151
info_802_1cb.vidx1152 = 1152
--vidx above 1999 will be concerned as 'eVidx' (L2MLL)
info_802_1cb.maxVidxIndex1999 = 1999
info_802_1cb.vidx2001 = 2001
info_802_1cb.vidx2002 = 2002
info_802_1cb.vidx2003 = 2003
info_802_1cb.vidx2151 = 2151
info_802_1cb.vidx2152 = 2152

info_802_1cb.eport7 = port7
info_802_1cb.eport111 = 111
info_802_1cb.eport112 = 112
info_802_1cb.eport121 = 121
info_802_1cb.eport122 = 122
info_802_1cb.eport131 = 131
info_802_1cb.eport132 = 132
info_802_1cb.eport211 = 211
info_802_1cb.eport212 = 212
info_802_1cb.eport214 = 214
info_802_1cb.eport215 = 215
info_802_1cb.eport216 = 216


info_802_1cb.streamId1   =   1
info_802_1cb.streamId2   =   2
info_802_1cb.streamId3   =   3
info_802_1cb.streamId4   =   4
info_802_1cb.streamId5   =   5
info_802_1cb.streamId6   =   6
info_802_1cb.streamId111 = 111
info_802_1cb.streamId112 = 112
info_802_1cb.streamId121 = 121
info_802_1cb.streamId122 = 122
info_802_1cb.streamId131 = 131
info_802_1cb.streamId132 = 132
info_802_1cb.streamId141 = 141
info_802_1cb.streamId142 = 142
info_802_1cb.streamId151 = 151
info_802_1cb.streamId152 = 152
info_802_1cb.streamId161 = 161
info_802_1cb.streamId162 = 162

info_802_1cb.l2mll = {}
info_802_1cb.l2mll.vidx2001Ltt = {index = 2001 - (info_802_1cb.maxVidxIndex1999+1),pointer = 0}
info_802_1cb.l2mll.vidx2002Ltt = {index = 2002 - (info_802_1cb.maxVidxIndex1999+1),pointer = 1}
info_802_1cb.l2mll.vidx2003Ltt = {index = 2003 - (info_802_1cb.maxVidxIndex1999+1),pointer = 2}
info_802_1cb.l2mll.vidx2151Ltt = {index = 2151 - (info_802_1cb.maxVidxIndex1999+1),pointer = 3}
info_802_1cb.l2mll.vidx2152Ltt = {index = 2152 - (info_802_1cb.maxVidxIndex1999+1),pointer = 4, pointer_plus_1 = 5}

local result, hwDevNum = device_to_hardware_format_convert(devNum)
info_802_1cb.hwDevNum = hwDevNum

info_802_1cb.srfIndex1 = 1
info_802_1cb.srfIndex2 = 2
info_802_1cb.srfIndex3 = 3
info_802_1cb.srfIndex4 = 4
info_802_1cb.srfIndex5 = 5
info_802_1cb.srfIndex6 = 6

info_802_1cb.srfEgressPortsBmp_word0_port5 = word0_of_bmp(port5)
info_802_1cb.srfEgressPortsBmp_word0_port6 = word0_of_bmp(port6)
info_802_1cb.srfEgressPortsBmp_word0_port7 = word0_of_bmp(port7)
info_802_1cb.srfEgressPortsBmp_word0_port8 = word0_of_bmp(port8)
info_802_1cb.srfEgressPortsBmp_word0_port5_and_port6 = word0_of_bmp(port5 , port6)
info_802_1cb.srfEgressPortsBmp_word0_port63 = word0_of_bmp(portCPU)

info_802_1cb.srfEgressPortsBmp_word1_port5 = word1_of_bmp(port5)
info_802_1cb.srfEgressPortsBmp_word1_port6 = word1_of_bmp(port6)
info_802_1cb.srfEgressPortsBmp_word1_port7 = word1_of_bmp(port7)
info_802_1cb.srfEgressPortsBmp_word1_port8 = word1_of_bmp(port8)
info_802_1cb.srfEgressPortsBmp_word1_port5_and_port6 = word1_of_bmp(port5 , port6)
info_802_1cb.srfEgressPortsBmp_word1_port63 = word1_of_bmp(portCPU)

info_802_1cb.arp = {}
info_802_1cb.arp.macC_ptr   = 1
info_802_1cb.arp.macD_ptr   = 2
info_802_1cb.arp.macE_ptr   = 3
info_802_1cb.arp.macF_ptr   = 4
info_802_1cb.arp.macG_ptr   = 5
info_802_1cb.arp.macH_ptr   = 6
info_802_1cb.arp.macA_ptr   = 7
info_802_1cb.arp.macB1_ptr  = 8
info_802_1cb.arp.macM1_ptr  = 10
info_802_1cb.arp.macM2_ptr  = 11

info_802_1cb.arp.macC  = makeMacAddrTestFormat(macC)
info_802_1cb.arp.macD  = makeMacAddrTestFormat(macD)
info_802_1cb.arp.macE  = makeMacAddrTestFormat(macE)
info_802_1cb.arp.macF  = makeMacAddrTestFormat(macF)
info_802_1cb.arp.macG  = makeMacAddrTestFormat(macG)
info_802_1cb.arp.macH  = makeMacAddrTestFormat(macH)
info_802_1cb.arp.macA  = makeMacAddrTestFormat(macA)
info_802_1cb.arp.macB1 = makeMacAddrTestFormat(macB1)
info_802_1cb.arp.macM1 = makeMacAddrTestFormat(macM1)
info_802_1cb.arp.macM2 = makeMacAddrTestFormat(macM2)

info_802_1cb.list_all_ports = ""..port1..","..port2..","..port3..","..port4..","..port5..","..port6..","..port7..","..port8..","..port9..","..port10..","..port11..","..port12..","..port13..","..port14..","..port15..","..port16

info_802_1cb.list_all_eports = ""..info_802_1cb.eport7..","..info_802_1cb.eport111..","..info_802_1cb.eport112..","..info_802_1cb.eport121..","..info_802_1cb.eport122..","..info_802_1cb.eport131..","..info_802_1cb.eport132..","..info_802_1cb.eport211..","..info_802_1cb.eport212..","..info_802_1cb.eport214..","..info_802_1cb.eport216


print ("info_802_1cb",to_string(info_802_1cb))

devEnv.info_802_1cb = info_802_1cb
global_test_data_env.info_802_1cb = info_802_1cb

global_test_data_env.CPU = portCPU

local function forceExplicitDsaInfo(explicitDsa)
    local isExplicitDsa = explicitDsa and "1" or "0"
    --force/unset to packet that send by the test from the CPU to use explicit DSA tag as the test will define inside the packet
    executeStringCliCommands("do shell-execute prvTgfTrafficForceExplicitDsaInfo " .. isExplicitDsa )
--[[GT_STATUS prvTgfTrafficForceExplicitDsaInfo(
    GT_U32         force
)]]
end

local flowsTable = {
    [1]= {srcPort = port1 , dstMac = macA , vlanId = 1  , sip = "192.16.1.1" , dip = "192.16.1.2" , remark = "Proxy Talker - TCAM"},
    [2]= {srcPort = port1 , dstMac = macA , vlanId = 1  , sip = "192.16.1.1" , dip = "192.16.1.3" , remark = "Proxy Talker - TCAM"},
    [3]= {srcPort = port1 , dstMac = macA , vlanId = 1  , sip = "192.16.1.1" , dip = "192.16.1.4" , remark = "Proxy FDB - TCAM"},
    [4]= {srcPort = port2 , dstMac = macB1 , vlanId = 2  , 			remark = "Split FDB"},
    [5]= {srcPort = port2 , dstMac = macB2 , vlanId = 2  , 			remark = "Split FDB"},
    [6]= {srcPort = portCPU , dstMac = macE , vlanId = 1 , dsaTag = "FORWARD" , remark = "Forward DSA tag"},

    [0x11a]= {srcPort = port15 , dstMac = macC , vlanId = 3 , sip = "192.16.1.2" , dip = "192.16.1.1" , remark = "Proxy Listener - TCAM"},
    [0x11b]= {srcPort = port16 , dstMac = macD , vlanId = 4 , sip = "192.16.1.2" , dip = "192.16.1.1" , remark = "Proxy Listener - TCAM"},

    [0x12a]= {srcPort = port15 , dstMac = macE , vlanId = 3 , sip = "192.16.1.3" , dip = "192.16.1.1" , remark = "Proxy Listener - TCAM"},
    [0x12b]= {srcPort = port16 , dstMac = macF , vlanId = 4 , sip = "192.16.1.3" , dip = "192.16.1.1" , remark = "Proxy Listener - TCAM"},

    [0x13a]= {srcPort = port15 , dstMac = macB1 , vlanId = 3 ,  		remark = "Merge-FDB"},
    [0x13b]= {srcPort = port16 , dstMac = macB1 , vlanId = 4 ,  		remark = "Merge-FDB"},

    [0x14a]= {srcPort = port15 , dstMac = macB2 , vlanId = 3 ,  		remark = "Merge-FDB"},
    [0x14b]= {srcPort = port16 , dstMac = macB2 , vlanId = 4 ,  		remark = "Merge-FDB"},

    [0x15a]= {srcPort = port15 , dstMac = macM , vlanId = 3 ,  		remark = "Multicast-FDB"},
    [0x15b]= {srcPort = port16 , dstMac = macM , vlanId = 4 ,  		remark = "Multicast-FDB"},

    [0x16a]= {srcPort = port15 , dstMac = macF , vlanId = 3 ,  		remark = "Packets to CPU"},
    [0x16b]= {srcPort = port16 , dstMac = macF , vlanId = 4 ,  		remark = "Packets to CPU"},

    [0x16c]= {srcPort = port16 , dstMac = macF , vlanId = 4 ,  		remark = "Packets to CPU"},
}

-- info accordint to :
-- https://sp.marvell.com/sites/EBUSites/Switching/Architecture/Shared%20Documents%20-%20arch%20sharepoint/Projects/Ironman/Verification%20use%20cases/802.1CB%20Use%20Cases.pdf#search=802%2E1cb

local test_name_1 = "Test1 (Proxy Talker and Split)"
local incomingPacketsDb_test1 = {
   {packetNum = "1-1" , flowNum = 1 , vlanTag = true , rTag = false , SeqNr = "-" , remark = "SeqNr=0, R=1, V=1"},
   {packetNum = "1-2" , flowNum = 1 , vlanTag = true , rTag = false , SeqNr = "-" , remark = "SeqNr=1, R=1, V=1"},
   {packetNum = "1-3" , flowNum = 1 , vlanTag = true , rTag = false , SeqNr = "-" , remark = "SeqNr=2, R=1, V=1"},
   {packetNum = "1-4" , flowNum = 2 , vlanTag = false, rTag = false , SeqNr = "-" , remark = "SeqNr=0, R=1, V=1"},
   {packetNum = "1-5" , flowNum = 2 , vlanTag = false, rTag = false , SeqNr = "-" , remark = "SeqNr=1, R=1, V=1"},
   {packetNum = "1-6" , flowNum = 2 , vlanTag = false, rTag = false , SeqNr = "-" , remark = "SeqNr=2, R=1, V=1"},
   {packetNum = "1-7" , flowNum = 3 , vlanTag = true , rTag = false , SeqNr = "-" , remark = "SeqNr=0, R=1, V=0"},
   {packetNum = "1-8" , flowNum = 3 , vlanTag = false, rTag = false , SeqNr = "-" , remark = "SeqNr=1, R=1, V=0"},
   {packetNum = "1-9" , flowNum = 3 , vlanTag = true , rTag = false , SeqNr = "-" , remark = "SeqNr=2, R=1, V=0"},
   {packetNum = "1-10", flowNum = 4 , vlanTag = true , rTag = true  , SeqNr = 1000, remark = "R=1, V=1"},
   {packetNum = "1-11", flowNum = 4 , vlanTag = false, rTag = true  , SeqNr = 1001, remark = "R=1, V=1"},
   {packetNum = "1-12", flowNum = 5 , vlanTag = true , rTag = true  , SeqNr = 1000, remark = "R=1, V=0"},
   {packetNum = "1-13", flowNum = 5 , vlanTag = false, rTag = true  , SeqNr = 1001, remark = "R=1, V=0"},
}

local outgoingPacketsDb_test1 =
{
    ["1-1"] = {
		{trgPort = port11 , macDa = macC , vlanTag = true , vlanId = 3 , up = 1 , rTag = true , SeqNr = 0   },
		{trgPort = port12 , macDa = macD , vlanTag = true , vlanId = 4 , up = 2 , rTag = true , SeqNr = 1   }
    },
    ["1-2"] = {
		{trgPort = port11 , macDa = macC , vlanTag = true , vlanId = 3 , up = 1 , rTag = true , SeqNr = 2   },
		{trgPort = port12 , macDa = macD , vlanTag = true , vlanId = 4 , up = 2 , rTag = true , SeqNr = 3   }
    },
    ["1-3"] = {
		{trgPort = port11 , macDa = macC , vlanTag = true , vlanId = 3 , up = 1 , rTag = true , SeqNr = 4   },
		{trgPort = port12 , macDa = macD , vlanTag = true , vlanId = 4 , up = 2 , rTag = true , SeqNr = 5   }
    },
    ["1-4"] = {
		{trgPort = port11 , macDa = macE , vlanTag = true , vlanId = 3 , up = 1 , rTag = true , SeqNr = 0   },
		{trgPort = port12 , macDa = macF , vlanTag = true , vlanId = 4 , up = 2 , rTag = true , SeqNr = 1   }
    },
    ["1-5"] = {
		{trgPort = port11 , macDa = macE , vlanTag = true , vlanId = 3 , up = 1 , rTag = true , SeqNr = 2   },
		{trgPort = port12 , macDa = macF , vlanTag = true , vlanId = 4 , up = 2 , rTag = true , SeqNr = 3   }
    },
    ["1-6"] = {
		{trgPort = port11 , macDa = macE , vlanTag = true , vlanId = 3 , up = 1 , rTag = true , SeqNr = 4   },
		{trgPort = port12 , macDa = macF , vlanTag = true , vlanId = 4 , up = 2 , rTag = true , SeqNr = 5   }
    },
    ["1-7"] = {
		{trgPort = port13 , macDa = macA , vlanTag = false , rTag = true , SeqNr = 0   },
		{trgPort = port14 , macDa = macA , vlanTag = false , rTag = true , SeqNr = 1   }
    },
    ["1-8"] = {
		{trgPort = port13 , macDa = macA , vlanTag = false , rTag = true , SeqNr = 2   },
		{trgPort = port14 , macDa = macA , vlanTag = false , rTag = true , SeqNr = 3   }
    },
    ["1-9"] = {
		{trgPort = port13 , macDa = macA , vlanTag = false , rTag = true , SeqNr = 4   },
		{trgPort = port14 , macDa = macA , vlanTag = false , rTag = true , SeqNr = 5   }
    },
    ["1-10"] = {
		{trgPort = port11 , macDa = macG , vlanTag = true , vlanId = 3 , up = 1 , rTag = true , SeqNr = 1000  },
		{trgPort = port12 , macDa = macH , vlanTag = true , vlanId = 4 , up = 2 , rTag = true , SeqNr = 1000  }
    },
    ["1-11"] = {
		{trgPort = port11 , macDa = macG , vlanTag = true , vlanId = 3 , up = 1 , rTag = true , SeqNr = 1001  },
		{trgPort = port12 , macDa = macH , vlanTag = true , vlanId = 4 , up = 2 , rTag = true , SeqNr = 1001  }
    },
    ["1-12"] = {
		{trgPort = port13 , macDa = macB2 , vlanTag = false , rTag = true , SeqNr = 1000  },
		{trgPort = port14 , macDa = macB2 , vlanTag = false , rTag = true , SeqNr = 1000  }
    },
    ["1-13"] = {
		{trgPort = port13 , macDa = macB2 , vlanTag = false , rTag = true , SeqNr = 1001  },
		{trgPort = port14 , macDa = macB2 , vlanTag = false , rTag = true , SeqNr = 1001  }
    },
}

local function cpssDxChStreamEntryGet(index)
    local apiName = "cpssDxChStreamEntryGet"

    printLog("Get stream IRF info for index = ",to_string(index))

    local isError , result, values =
        genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",     "devNum",   devNum },
        { "IN",     "GT_U32",    "index",    index },
        { "OUT",    "CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC"  ,     "sngIrfInfo"},
        { "OUT",    "CPSS_DXCH_STREAM_IRF_COUNTERS_ENTRY_STC",  "irfCounters"  },
        { "OUT",    "CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC"  , "srfMappingInfo"},
    })

    printLog(to_string(values))

    if not values then
        return nil,nil,nil
    end

    return values.sngIrfInfo , values.irfCounters , values.srfMappingInfo
end

local prev_sngIrfInfo , prev_irfCounters , prev_srfMappingInfo
local function pre_irf_DiscardedPacketsCounter(index)
    --clear the counters --> just read it --> do nothing with the results
    prev_sngIrfInfo , prev_irfCounters , prev_srfMappingInfo = cpssDxChStreamEntryGet(index)
end
local function post_irf_DiscardedPacketsCounter(index)
    -- get the values from the CPSS
    local sngIrfInfo , irfCounters , srfMappingInfo = cpssDxChStreamEntryGet(index)

    if irfCounters == nil or irfCounters.discardedPacketsOrSameIdCounter == nil then
		printLog ("ERROR : irfCounters.discardedPacketsOrSameIdCounter not valid in index " .. to_string(index) , irfCounters)
		testAddErrorString("irfCounters.discardedPacketsOrSameIdCounter not valid")
        return
    end

    local diff,diffFromLastRead

    diffFromLastRead = irfCounters.discardedPacketsOrSameIdCounter.l[0] - prev_irfCounters.discardedPacketsOrSameIdCounter.l[0]
    if isGmUsed() then
        -- the GM not support ROC on this counter (only 'RO')
        diff = diffFromLastRead
    else
        diff = irfCounters.discardedPacketsOrSameIdCounter.l[0]
    end

    --check that the <discardedPacketsOrSameIdCounter> was incremented
    if diff ~= 1 then
        if(diffFromLastRead == 1) then
            printLog ("ERROR : irfCounters.discardedPacketsOrSameIdCounter incremented by 1 but not doing 'clear after read' , in index " .. to_string(index) , to_string(irfCounters.discardedPacketsOrSameIdCounter) , to_string(prev_irfCounters.discardedPacketsOrSameIdCounter))
            testAddErrorString("irfCounters.discardedPacketsOrSameIdCounter incremented by 1 but not doing 'clear after read' ")
        else
            printLog ("ERROR : irfCounters.discardedPacketsOrSameIdCounter not incremented by 1 in index " .. to_string(index) , to_string(irfCounters.discardedPacketsOrSameIdCounter) , to_string(prev_irfCounters.discardedPacketsOrSameIdCounter))
            testAddErrorString("irfCounters.discardedPacketsOrSameIdCounter not incremented by 1")
        end
    end
end
local function cpssDxChStreamSrfEntryGet(index)
    local apiName = "cpssDxChStreamSrfEntryGet"

    printLog("Get stream SRF info for index = ",to_string(index))

    local isError , result, values =
        genericCpssApiWithErrorHandler(command_data,
        apiName, {
        { "IN",     "GT_U8",     "devNum",   devNum },
        { "IN",     "GT_U32",    "index",    index },
        { "OUT",    "CPSS_DXCH_STREAM_SRF_CONFIG_ENTRY_STC"  ,    "srfConfigInfo"},
        { "OUT",    "CPSS_DXCH_STREAM_SRF_COUNTERS_ENTRY_STC",    "srfCounters"  },
        { "OUT",    "CPSS_DXCH_STREAM_SRF_STATUS_ENTRY_STC"  ,    "srfStatusInfo"},
    })

    printLog(to_string(values))

    if not values then
        return nil,nil,nil
    end

    return values.srfConfigInfo , values.srfCounters , values.srfStatusInfo
end
local prev_srfConfigInfo , prev_srfCounters , prev_srfStatusInfo
local function pre_srf_RoguePacketsCounter(index)
    --read the info before sending the packet , because those are not 'read only clear' those are 'read only'
    prev_srfConfigInfo , prev_srfCounters , prev_srfStatusInfo = cpssDxChStreamSrfEntryGet(index)
end
local function post_srf_RoguePacketsCounter(index)
    -- get the values from the CPSS
    local srfConfigInfo , srfCounters , srfStatusInfo = cpssDxChStreamSrfEntryGet(index)

    if srfCounters == nil or srfCounters.roguePacketCounter == nil then
		printLog ("ERROR : srfCounters.roguePacketCounter not valid in index " .. to_string(index) , srfCounters)
		testAddErrorString("srfCounters.roguePacketCounter not valid")
        return
    end

    local diff
    -- the counters are RO (not ROC)
    diff = srfCounters.roguePacketCounter.l[0] - prev_srfCounters.roguePacketCounter.l[0]

    --check that the <Rogue Packets Counter> was incremented
    if diff ~= 1 then
		printLog ("ERROR : srfCounters.roguePacketCounter not incremented by 1 in index " .. to_string(index) , to_string(srfCounters.roguePacketCounter) , to_string(prev_srfCounters.roguePacketCounter))
		testAddErrorString("srfCounters.roguePacketCounter not incremented by 1")
    end
end

local test_name_2 = "Test2 (Proxy Listener and Merge)"
local incomingPacketsDb_test2 = {
   {packetNum = "2-1" , flowNum = 0x11a , vlanTag = true , rTag = true , SeqNr = 1001 , remark = "First packet"},
   {packetNum = "2-2" , flowNum = 0x11a , vlanTag = true , rTag = true , SeqNr = 1002 , remark = "Second packet"},
   {packetNum = "2-3" , flowNum = 0x11b , vlanTag = true , rTag = true , SeqNr = 1001 , remark = "Same First packet"},
   {packetNum = "2-4" , flowNum = 0x11a , vlanTag = false, rTag = true , SeqNr = 1003 , remark = "Third packet"},
   {packetNum = "2-5" , flowNum = 0x11b , vlanTag = true , rTag = true , SeqNr = 1002 , remark = "Same Second packet"},
   {packetNum = "2-6" , flowNum = 0x11b , vlanTag = false, rTag = true , SeqNr = 1003 , remark = "Same Third packet"},
   {packetNum = "2-7" , flowNum = 0x12a , vlanTag = true , rTag = true , SeqNr = 2001 , remark = "First packet"},
   {packetNum = "2-8" , flowNum = 0x12a , vlanTag = true , rTag = true , SeqNr = 2002 , remark = "Second packet"},
   {packetNum = "2-9" , flowNum = 0x12a , vlanTag = false, rTag = true , SeqNr = 2003 , remark = "Third packet"},
   {packetNum = "2-10", flowNum = 0x12b , vlanTag = true , rTag = true , SeqNr = 2001 , remark = "Same First packet"},
   {packetNum = "2-11", flowNum = 0x12b , vlanTag = true , rTag = true , SeqNr = 2002 , remark = "Same Second packet"},
   {packetNum = "2-12", flowNum = 0x12b , vlanTag = false, rTag = true , SeqNr = 2003 , remark = "Same Third packet"},
   {packetNum = "2-13", flowNum = 0x13a , vlanTag = true , rTag = true , SeqNr = 3001 , remark = "First packet fast stream"},
   {packetNum = "2-14", flowNum = 0x13b , vlanTag = true , rTag = true , SeqNr = 3001 , remark = "First packet slow stream"},
   {packetNum = "2-15", flowNum = 0x13b , vlanTag = true , rTag = true , SeqNr = 3002 , remark = "Second packet slow stream"},
   {packetNum = "2-16", flowNum = 0x13b , vlanTag = false, rTag = true , SeqNr = 3003 , remark = "Third packet slow stream"},
   {packetNum = "2-17", flowNum = 0x13a , vlanTag = true , rTag = true , SeqNr = 3006 , remark = "Sixth packet fast stream"},
   {packetNum = "2-18", flowNum = 0x13a , vlanTag = false, rTag = true , SeqNr = 3007 , remark = "Seventh packet fast stream"},
   {packetNum = "2-19", flowNum = 0x13b , vlanTag = true , rTag = true , SeqNr = 3004 , remark = "Fourth packet slow stream"},
   {packetNum = "2-20", flowNum = 0x13b , vlanTag = true , rTag = true , SeqNr = 3005 , remark = "Fifth packet slow stream"},
   {packetNum = "2-21", flowNum = 0x13b , vlanTag = true , rTag = true , SeqNr = 3006 , remark = "Sixth packet slow stream"},
   {packetNum = "2-22", flowNum = 0x13b , vlanTag = false, rTag = true , SeqNr = 3007 , remark = "Seventh packet slow stream"},
   {packetNum = "2-23", flowNum = 0x14a , vlanTag = true , rTag = true , SeqNr = 4001 , remark = "First packet"},
   {packetNum = "2-24", flowNum = 0x14a , vlanTag = true , rTag = true , SeqNr = 4001 , remark = "First packet from the same path",
    preTestFunc  = {[1] = {testFunc = pre_irf_DiscardedPacketsCounter  , testParams = info_802_1cb.streamId141}},
    postTestFunc = {[1] = {testFunc = post_irf_DiscardedPacketsCounter , testParams = info_802_1cb.streamId141}},
   },
   {packetNum = "2-25", flowNum = 0x14b , vlanTag = true , rTag = true , SeqNr = 4001 , remark = "First packet from the other path"},
   {packetNum = "2-26", flowNum = 0x14b , vlanTag = false, rTag = true , SeqNr = 4002 , remark = "Second packet from the other path"},
   {packetNum = "2-27", flowNum = 0x14a , vlanTag = true , rTag = true , SeqNr = 4900 , remark = "Rouge packet",
    preTestFunc  = {[1] = {testFunc = pre_srf_RoguePacketsCounter  , testParams = info_802_1cb.srfIndex4--[[for info_802_1cb.streamId141]]}},
    postTestFunc = {[1] = {testFunc = post_srf_RoguePacketsCounter , testParams = info_802_1cb.srfIndex4--[[for info_802_1cb.streamId141]]}},
   },
   {packetNum = "2-28", flowNum = 0x15a , vlanTag = true , rTag = true , SeqNr = 5001 , remark = "First packet"},
   {packetNum = "2-29", flowNum = 0x15b , vlanTag = true , rTag = true , SeqNr = 5001 , remark = "Same First packet"},
   {packetNum = "2-30", flowNum = 0x15b , vlanTag = false, rTag = true , SeqNr = 5002 , remark = "Second packet"},
   {packetNum = "2-31", flowNum = 0x15a , vlanTag = false, rTag = true , SeqNr = 5002 , remark = "Same Second packet"},

}

local outgoingPacketsDb_test2 =
{
    ["2-1"] = {
		{trgPort = port5 , macDa = macA , vlanTag = false , rTag = false , SeqNr = "-"   }
    },
    ["2-2"] = {
		{trgPort = port5 , macDa = macA , vlanTag = false , rTag = false , SeqNr = "-"   }
    },
    ["2-3"] = {
		{trgPort = port5 , notEgress = true   }
    },
    ["2-4"] = {
		{trgPort = port5 , macDa = macA , vlanTag = false , rTag = false , SeqNr = "-"   }
    },
    ["2-5"] = {
		{trgPort = port5 , notEgress = true   }
    },
    ["2-6"] = {
		{trgPort = port5 , notEgress = true   }
    },
    ["2-7"] = {
		{trgPort = port6 , macDa = macA , vlanTag = true , vlanId = 2 , up = 3 ,rTag = false , SeqNr = "-"   }
    },
    ["2-8"] = {
		{trgPort = port6 , macDa = macA , vlanTag = true , vlanId = 2 , up = 3 ,rTag = false , SeqNr = "-"   }
    },
    ["2-9"] = {
		{trgPort = port6 , macDa = macA , vlanTag = true , vlanId = 2 , up = 3 ,rTag = false , SeqNr = "-"   }
    },
    ["2-10"] = {
		{trgPort = port6 , notEgress = true   }
    },
    ["2-11"] = {
		{trgPort = port6 , notEgress = true   }
    },
    ["2-12"] = {
		{trgPort = port6 , notEgress = true   }
    },
    ["2-13"] = {
		{trgPort = port7 , macDa = macB1 , vlanTag = false , rTag = true , SeqNr = 3001   }
    },
    ["2-14"] = {
		{trgPort = port7 , notEgress = true   }
    },
    ["2-15"] = {
		{trgPort = port7 , macDa = macB1 , vlanTag = false , rTag = true , SeqNr = 3002   }
    },
    ["2-16"] = {
		{trgPort = port7 , macDa = macB1 , vlanTag = false , rTag = true , SeqNr = 3003   }
    },
    ["2-17"] = {
		{trgPort = port7 , macDa = macB1 , vlanTag = false , rTag = true , SeqNr = 3006   }
    },
    ["2-18"] = {
		{trgPort = port7 , macDa = macB1 , vlanTag = false , rTag = true , SeqNr = 3007   }
    },
    ["2-19"] = {
		{trgPort = port7 , macDa = macB1 , vlanTag = false , rTag = true , SeqNr = 3004   }
    },
    ["2-20"] = {
		{trgPort = port7 , macDa = macB1 , vlanTag = false , rTag = true , SeqNr = 3005   }
    },
    ["2-21"] = {
		{trgPort = port7 , notEgress = true   }
    },
    ["2-22"] = {
		{trgPort = port7 , notEgress = true   }
    },
    ["2-23"] = {
		{trgPort = port8 , macDa = macB1 , vlanTag = true , vlanId = 1 , up = 3 , rTag = true , SeqNr = 4001   }
    },
    ["2-24"] = {
		{trgPort = port8 , notEgress = true   }
    },
    ["2-25"] = {
		{trgPort = port8 , notEgress = true   }
    },
    ["2-26"] = {
		{trgPort = port8 , macDa = macB1 , vlanTag = true , vlanId = 1 , up = 3 , rTag = true , SeqNr = 4002   }
    },
    ["2-27"] = {
		{trgPort = port8 , notEgress = true   }
    },
    ["2-28"] = {
	 	 {trgPort = port5 , macDa = macM  , vlanTag = false , rTag = false , SeqNr = "-"   }
		,{trgPort = port6 , macDa = macM1 , vlanTag = true , vlanId = 1 , up = 1 , rTag = false , SeqNr = "-"   }
		,{trgPort = port7 , macDa = macM  , vlanTag = false , rTag = true , SeqNr = 5001   }
    },
    ["2-29"] = {
		{trgPort = port8 , macDa = macM2 , vlanTag = true , vlanId = 2 , up = 2 , rTag = true , SeqNr = 5001   }
    },
    ["2-30"] = {
	 	 {trgPort = port5 , macDa = macM  , vlanTag = false , rTag = false , SeqNr = "-"   }
		,{trgPort = port6 , macDa = macM1 , vlanTag = true , vlanId = 1 , up = 1 , rTag = false , SeqNr = "-"   }
		,{trgPort = port8 , macDa = macM2  , vlanTag = true , vlanId = 2 , up = 2 , rTag = true , SeqNr = 5002   }
    },
    ["2-31"] = {
		{trgPort = port7 , macDa = macM , vlanTag = false , rTag = true , SeqNr = 5002   }
    },
}

local test_name_3 = "Test3 (Packets to/from CPU)"
local incomingPacketsDb_test3 = {
   {packetNum = "3-1" , flowNum = 0x13a , dip = "192.16.9.3" , vlanTag = false , rTag = true , SeqNr = 3008 , remark = "First packet"},
   {packetNum = "3-2" , flowNum = 0x13b , dip = "192.16.9.3" , vlanTag = false , rTag = true , SeqNr = 3008 , remark = "Same First packet"},
   {packetNum = "3-3" , flowNum = 0x13a , dip = "192.16.9.2" , vlanTag = false , rTag = true , SeqNr = 3009 , remark = "Second packet (mirrored)"},
   {packetNum = "3-4" , flowNum = 0x13b , dip = "192.16.9.2" , vlanTag = false , rTag = true , SeqNr = 3009 , remark = "Same Second packet (mirrored)"},
   {packetNum = "3-5" , flowNum = 0x13a , dip = "192.16.9.1" , vlanTag = false , rTag = true , SeqNr = 3010 , remark = "Third packet (trapped)"},
   {packetNum = "3-6" , flowNum = 0x13b , dip = "192.16.9.1" , vlanTag = false , rTag = true , SeqNr = 3010 , remark = "Same Third packet (trapped)"},
   {packetNum = "3-7" , flowNum = 0x6   ,                      vlanTag = false , rTag = false , SeqNr = "-" , remark = "FWD DSA SeqNr=0",
        trgPort = "vidx" , dsaTag = {vidx = info_802_1cb.vidx1003},
        preTestFunc  = forceExplicitDsaInfo , preTestParams = true   ,
        postTestFunc = {[1] = {testFunc = forceExplicitDsaInfo , testParams = false}},
   },
   {packetNum = "3-8" , flowNum = 0x6   ,                      vlanTag = false , rTag = false , SeqNr = "-" , remark = "FWD DSA SeqNr=1",
        trgPort = "vidx" , dsaTag = {vidx = info_802_1cb.vidx1003},
        preTestFunc  = forceExplicitDsaInfo , preTestParams = true   ,
        postTestFunc = {[1] = {testFunc = forceExplicitDsaInfo , testParams = false}},
   },
   {packetNum = "3-9" , flowNum = 0x6   ,                      vlanTag = false , rTag = false , SeqNr = "-" , remark = "FWD DSA SeqNr=2",
        trgPort = "vidx" , dsaTag = {vidx = info_802_1cb.vidx1003},
        preTestFunc  = forceExplicitDsaInfo , preTestParams = true   ,
        postTestFunc = {[1] = {testFunc = forceExplicitDsaInfo , testParams = false}},
   },
   {packetNum = "3-10", flowNum = 0x16a ,                      vlanTag = true  , rTag = true , SeqNr = 7001 , remark = "First packet to CPU(DSA forward)"},
   {packetNum = "3-11", flowNum = 0x16a ,                      vlanTag = true  , rTag = true , SeqNr = 7001 , remark = "Same First packet to CPU(DSA forward)"},
   {packetNum = "3-12", flowNum = 0x16b ,                      vlanTag = true  , rTag = true , SeqNr = 7002 , remark = "Second packet to CPU(DSA forward)"},
   {packetNum = "3-13", flowNum = 0x16b ,                      vlanTag = true  , rTag = true , SeqNr = 7002 , remark = "Same Second packet to CPU(DSA forward)"},
   {packetNum = "3-14", flowNum = 0x16c ,                      vlanTag = false , rTag = true , SeqNr = 7003 , remark = "Third packet to CPU(DSA forward)"},
   {packetNum = "3-15", flowNum = 0x16c ,                      vlanTag = false , rTag = true , SeqNr = 7003 , remark = "Same Third packet to CPU(DSA forward)"},
   {packetNum = "3-16" , flowNum = 0x13a , dip = "192.16.9.3" , vlanTag = false , rTag = true , SeqNr = 6001 , remark = "First packet (Drop on Rouge)",
    preTestFunc  = {[1] = {testFunc = pre_srf_RoguePacketsCounter  , testParams = info_802_1cb.srfIndex3--[[for info_802_1cb.streamId131]]}},
    postTestFunc = {[1] = {testFunc = post_srf_RoguePacketsCounter , testParams = info_802_1cb.srfIndex3--[[for info_802_1cb.streamId131]]}},
   },
   {packetNum = "3-17" , flowNum = 0x13b , dip = "192.16.9.3" , vlanTag = false , rTag = true , SeqNr = 6001 , remark = "Same First packet (Drop on Rouge)",
    preTestFunc  = {[1] = {testFunc = pre_srf_RoguePacketsCounter  , testParams = info_802_1cb.srfIndex3--[[for info_802_1cb.streamId131]]}},
    postTestFunc = {[1] = {testFunc = post_srf_RoguePacketsCounter , testParams = info_802_1cb.srfIndex3--[[for info_802_1cb.streamId131]]}},
   },
   {packetNum = "3-18" , flowNum = 0x13a , dip = "192.16.9.2" , vlanTag = false , rTag = true , SeqNr = 6002 , remark = "Second packet (mirrored ,but Orig Drop on Rouge)",
    preTestFunc  = {[1] = {testFunc = pre_srf_RoguePacketsCounter  , testParams = info_802_1cb.srfIndex3--[[for info_802_1cb.streamId131]]}},
    postTestFunc = {[1] = {testFunc = post_srf_RoguePacketsCounter , testParams = info_802_1cb.srfIndex3--[[for info_802_1cb.streamId131]]}},
   },
   {packetNum = "3-19" , flowNum = 0x13b , dip = "192.16.9.2" , vlanTag = false , rTag = true , SeqNr = 6002 , remark = "Same Second packet  (mirrored ,but Orig Drop on Rouge)",
    preTestFunc  = {[1] = {testFunc = pre_srf_RoguePacketsCounter  , testParams = info_802_1cb.srfIndex3--[[for info_802_1cb.streamId131]]}},
    postTestFunc = {[1] = {testFunc = post_srf_RoguePacketsCounter , testParams = info_802_1cb.srfIndex3--[[for info_802_1cb.streamId131]]}},
   },
   {packetNum = "3-20" , flowNum = 0x13a , dip = "192.16.9.1" , vlanTag = false , rTag = true , SeqNr = 6003 , remark = "Third packet (trapped, NOT Drop on Rouge)",
   },
   {packetNum = "3-21" , flowNum = 0x13b , dip = "192.16.9.1" , vlanTag = false , rTag = true , SeqNr = 6003 , remark = "Same Third packet (trapped, NOT Drop on Rouge)",
   },
}
local outgoingPacketsDb_test3 =
{
    ["3-1"] = {
		{trgPort = port7 , macDa = macB1 , vlanTag = false , rTag = true , SeqNr = 3008   }
    },
    ["3-2"] = {
		{trgPort = port7 , notEgress = true }
    },
    ["3-3"] = {
		{trgPort = port7   , macDa = macB1 , vlanTag = false , rTag = true , SeqNr = 3009   },
		{trgPort = portCPU , macDa = macB1 , vlanTag = false , rTag = true , SeqNr = 3009   }
    },
    ["3-4"] = {
		{trgPort = port7   , notEgress = true },
		{trgPort = portCPU , macDa = macB1 , vlanTag = false , rTag = true , SeqNr = 3009   }
    },
    ["3-5"] = {
		{trgPort = port7   , notEgress = true },
		{trgPort = portCPU , macDa = macB1 , vlanTag = false , rTag = true , SeqNr = 3010   }
    },
    ["3-6"] = {
		{trgPort = port7   , notEgress = true },
		{trgPort = portCPU , macDa = macB1 , vlanTag = false , rTag = true , SeqNr = 3010   }
    },
    ["3-7"] = {
		{trgPort = port13  , macDa = macE , vlanTag = false , rTag = true , SeqNr = 0   },
		{trgPort = port14  , macDa = macE , vlanTag = false , rTag = true , SeqNr = 1   }
    },
    ["3-8"] = {
		{trgPort = port13  , macDa = macE , vlanTag = false , rTag = true , SeqNr = 2   },
		{trgPort = port14  , macDa = macE , vlanTag = false , rTag = true , SeqNr = 3   }
    },
    ["3-9"] = {
		{trgPort = port13  , macDa = macE , vlanTag = false , rTag = true , SeqNr = 4   },
		{trgPort = port14  , macDa = macE , vlanTag = false , rTag = true , SeqNr = 5   }
    },
    ["3-10"] = {
        --'trap to cpu' keep the ingress packet 'as is' when coming to the CPU (with rTag , no mac DA modify , no vid modify)
		{trgPort = portCPU , macDa = macF , vlanTag = true , vlanId = 3 , rTag = true , SeqNr = 7001   }
    },
    ["3-11"] = {
		{trgPort = portCPU ,  notEgress = true }--'notEgress' for CPU means 'not ingress' !!!
    },
    ["3-12"] = {
        --'trap to cpu' keep the ingress packet 'as is' when coming to the CPU (with rTag , no mac DA modify , no vid modify)
		{trgPort = portCPU , macDa = macF , vlanTag = true , vlanId = 4 , rTag = true , SeqNr = 7002   }
    },
    ["3-13"] = {
		{trgPort = portCPU ,  notEgress = true }--'notEgress' for CPU means 'not ingress' !!!
    },
    ["3-14"] = {
        --'trap to cpu' keep the ingress packet 'as is' when coming to the CPU (with rTag , no mac DA modify , no vid modify)
		{trgPort = portCPU , macDa = macF , vlanTag = false , vlanId = 3 , rTag = true , SeqNr = 7003   }
    },
    ["3-15"] = {
		{trgPort = portCPU ,  notEgress = true }--'notEgress' for CPU means 'not ingress' !!!
    },
    ["3-16"] = {{trgPort = port7 , notEgress = true },{trgPort = portCPU ,  notEgress = true }},--'notEgress' for CPU means 'not ingress' !!!
    ["3-17"] = {{trgPort = port7 , notEgress = true },{trgPort = portCPU ,  notEgress = true }},--'notEgress' for CPU means 'not ingress' !!!
    ["3-18"] = {
		{trgPort = port7   , notEgress = true },
		{trgPort = portCPU , macDa = macB1 , vlanTag = false , rTag = true , SeqNr = 6002   }
    },
    ["3-19"] = {
		{trgPort = port7   , notEgress = true },
		{trgPort = portCPU , macDa = macB1 , vlanTag = false , rTag = true , SeqNr = 6002   }
    },
    ["3-20"] = {
		{trgPort = port7   , notEgress = true },
		{trgPort = portCPU , macDa = macB1 , vlanTag = false , rTag = true , SeqNr = 6003   }
    },
    ["3-21"] = {
		{trgPort = port7   , notEgress = true },
		{trgPort = portCPU , macDa = macB1 , vlanTag = false , rTag = true , SeqNr = 6003   }
    },
}


local rTag_etherType = "F1C1" --0xF1C1
local rTag_etherType_plus_reserved = rTag_etherType .. "0000"
local function buildRTag(SeqNr)
    local SeqNrStr          = string.format("%4.4x",SeqNr)      --16 bits
    return rTag_etherType_plus_reserved .. SeqNrStr
end

local function buildVlanTag(vlanId,cfi,vpt)
    if vpt == nil then vpt = 0 end
    if cfi == nil then cfi = 0 end
    local vidStr            = string.format("%3.3x",vlanId)     --12 bits
    local cfiVptStr         = string.format("%1.1x",vpt*2+cfi)  -- 4 bits
    return "8100" .. cfiVptStr .. vidStr
end

local function buildDsaFrwToVidx(vidx,vlanTag)
    local hwDevNum = 0x321 --[[801 decimal]] --[[ other device in the 'stack' ]]
    local edsaStc = {
        dsaType = "CPSS_DXCH_NET_DSA_CMD_FORWARD_E",
        commonParams = { dsaTagType="CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT",
                 vpt    = (vlanTag and vlanTag.exists) and vlanTag.vpt    or 0,
                 cfiBit = (vlanTag and vlanTag.exists) and vlanTag.cfi    or 0,
                 vid    = (vlanTag and vlanTag.exists) and vlanTag.vlanId or 1
                 },
        dsaInfo = {
            forward = {
              tag0TpidIndex        = 0,
              srcHwDev             = hwDevNum,
              srcIsTagged          = (vlanTag and vlanTag.exists) and "CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E" or "CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E",
              srcIsTrunk           = false,
              isTrgPhyPortValid    = false,
              dstInterface         ={ type="CPSS_INTERFACE_VIDX_E", vidx=vidx },
              phySrcMcFilterEnable = true,
              skipFdbSaLookup      = true,

              source = {portNum = portCPU},-- src eport
              origSrcPhy = {portNum = portCPU}   -- 12 bits in eDSA because muxed with origTrunkId
            }
        }
    }

    local rc, edsaBytes = wrlDxChDsaToString(devNum, edsaStc)
    if (rc ~=0) then
        setFailState()
        printLog(edsaBytes .. ". Error code is " .. rc)
        return ""
    end

    return edsaBytes
end

local function macAddrFromPortNum(portNum)
    local portNumStr        = string.format("%2.2x",portNum % 255)      --8 bits
	return "0055667788" --[[5 bytes]] .. portNumStr --1 byte
end
--[[
build packet info:
	 l2 = {srcMac = , dstMac = }
	,rTag = { exists = true/false , SeqNr = , }
	,vlanTag = {exists = true/false , vlanId = , cfi = , vpt = }
	,payload =
	,dsaTag = {vidx = }
]]
--[[
    partsArray={
        {type="TGF_PACKET_PART_IPV4_E", partPtr=prvTgfPacketIpv4Part,
]]
local TGF_PACKET_AUTO_CALC_CHECKSUM_CNS = 0xEEEE
local prvTgfPacketIpv4Part = {
    version = 4,
    headerLen = 5,
    typeOfService = 0,
    totalLen = 0x3f,    --filled in runtime
    id = 0,
    flags = 0,
    offset = 0,
    timeToLive = 0x40,
    protocol = 0x04,
    csum = TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, --calulated filled in runtime by the C code : prvLuaTgfBuildPacketPart(...)
    srcAddr = "1.2.3.4", --filled in runtime
    dstAddr = "4.5.6.7", --filled in runtime
}

local ipv4_etherType = "0800"

local function buildPacket(info)
    local tempRTag = ""
    local tempVlanTag = ""
    local tempDsaTag = ""
    local ipv4Header = ""

    if  info.rTag and info.rTag.exists then
        local rTag = info.rTag
        tempRTag = buildRTag(rTag.SeqNr)
    end

    if info.ipv4 and info.ipv4.dip then
        if nil == info.ipv4.sip then
            --dummy sip5
            info.ipv4.sip = dummy_sip
        end
        local ipv4Info = {type="TGF_PACKET_PART_IPV4_E", partPtr = prvTgfPacketIpv4Part}
        prvTgfPacketIpv4Part.srcAddr = info.ipv4.sip -- the 'lua to c' (prv_lua_to_c_TGF_IPV4_ADDR) convertor supports "a.b.c.d" string format !!!
        prvTgfPacketIpv4Part.dstAddr = info.ipv4.dip -- the 'lua to c' (prv_lua_to_c_TGF_IPV4_ADDR) convertor supports "a.b.c.d" string format !!!
        prvTgfPacketIpv4Part.totalLen = 20 + string.len(info.payload) / 2
        --printLog("ipv4Info",to_string(ipv4Info))

        ipv4Header = ipv4_etherType .. luaTgfPartStringFromSTC(ipv4Info)
    end

    if info.dsaTag then
        if info.dsaTag.vidx then
            -- the CPU send DSA tag , that need to build
            tempDsaTag = buildDsaFrwToVidx(info.dsaTag.vidx,info.vlanTag)
        elseif info.dsaTag.note == "CPU get FRW" then
            -- the CPU get DSA FRW tag , that was removed by the CPSS
            -- we not build DSA and not vlan tag that was embedded in the FRW DSA
        end
    else
        -- if we build DSA tag as FORWARD , we indicate in the DSA that it came with vlan tag (and embedded in the DSA)
        if  info.vlanTag and info.vlanTag.exists then
            local vlanTag = info.vlanTag
            tempVlanTag = buildVlanTag(vlanTag.vlanId,vlanTag.cfi,vlanTag.vpt)
        end
    end

    return info.l2.dstMac .. info.l2.srcMac .. tempDsaTag .. tempVlanTag .. tempRTag .. ipv4Header .. info.payload
end

local command_data = Command_Data()

local function doFunction (func , params)

    if(not func)then return end

    if( type(func) == "table") then
        for _dummy, entry in pairs(func) do
            entry.testFunc(entry.testParams)
        end
    else
        func(params)
    end

end

local transmitInfo =
{
  devNum = devNum,
  pktInfo = {},
  inBetweenSendFunc = nil
}
local egressInfoTable

local payload_short =
"aaaabbbbccccddddeeeeffff00001111"..
"22223333444455556666777788889999"..
"a1a2a3a4b1b2b3b4c1c2c3c4d1d2d3d4"..
"e1e2e3e4f1f2f3f49192939481828384"

local payload_long =
payload_short ..
payload_short ..
payload_short ..
payload_short ..
payload_short ..
payload_short ..
payload_short ..
payload_short ..
payload_short ..
payload_short

-- we need to capture the egress packet that loopback from the port to the CPU
-- use iPCL rule to trap it
local function forceCaptureOnTransmitPortSet(force)

    if force then
        executeStringCliCommands(
        [[
            do configure

            access-list device ${dev} pcl-ID 10
            rule-id 0 action trap-to-cpu
            exit

            access-list device ${dev} pcl-ID 11
            rule-id 6 action trap-to-cpu
            exit

            access-list device ${dev} pcl-ID 12
            rule-id 12 action trap-to-cpu
            exit

            exit
        ]]
        )
    else
        executeStringCliCommands(
        [[
            do configure

            access-list device ${dev} pcl-ID 10
            delete rule-id 0
            exit

            access-list device ${dev} pcl-ID 11
            delete rule-id 6
            exit

            access-list device ${dev} pcl-ID 12
            delete rule-id 12
            exit

            exit
        ]]
        )
    end

end


local function doSingleCaseLogic(ingressInfo,OUT_Packets)
	local flowNum
	local flowEntry

	local caseName = "Send a packet #".. ingressInfo.packetNum .. " , note : ".. ingressInfo.remark

	printLog("========================================")
	printLog(caseName)

	flowNum = ingressInfo.flowNum
	flowEntry = flowsTable[flowNum]

	if(1 == (flowNum % 2)) then
		ingressPayload = payload_short
	else
		ingressPayload = payload_long
	end

	transmitInfo.portNum            = flowEntry.srcPort

	local forceCaptureOnTransmitPort = false

	if (flowEntry.srcPort == portCPU) then
		-- we send from the CPU to 'target port'
		if ingressInfo.trgPort ~= "vidx" then
			transmitInfo.portNum            = ingressInfo.trgPort
		end
		forceCaptureOnTransmitPort      = true
	end

--[[
build packet info:
	 l2 = {srcMac = , dstMac = }
	,rTag = { exists = true/false , SeqNr = , }
	,vlanTag = {exists = true/false , vlanId = , cfi = , vpt = }
	,payload =
	,dsaTag = {vidx = }
]]
    local packet_dip = ingressInfo.dip and ingressInfo.dip or flowEntry.dip
    local packet_sip = flowEntry.sip

	local packetInfo = {
		l2 = {srcMac = macAddrFromPortNum(flowEntry.srcPort), dstMac = flowEntry.dstMac}
	   ,rTag = { exists = ingressInfo.rTag , SeqNr = ingressInfo.SeqNr}
	   ,vlanTag = {exists = ingressInfo.vlanTag , vlanId = flowEntry.vlanId , cfi = 0, vpt = 0}
	   ,payload = ingressPayload
	   ,dsaTag = ingressInfo.dsaTag
       ,ipv4 = {sip = packet_sip , dip = packet_dip}
	}

	--printLog("packetInfo=" ,to_string(packetInfo));

	transmitInfo.pktInfo.fullPacket = buildPacket(packetInfo)
	transmitInfo.inBetweenSendFunc  = ingressInfo.inBetweenSendFunc

	if(ingressInfo.preTestFunc)then
		doFunction(ingressInfo.preTestFunc,ingressInfo.preTestParams)
	end

	--printLog("fullPacket=" ,to_string(transmitInfo.pktInfo.fullPacket));

	egressInfoTable = {}
	local index = 1
	for _dummy2, egressInfo in pairs(OUT_Packets[ingressInfo.packetNum]) do

		egressInfoTable[index] = {}

		egressInfoTable[index].portNum = egressInfo.trgPort

		if egressInfo.trgPort == portCPU then
			egressInfoTable[index].portNum = "CPU"
		end

		if (flowEntry.srcPort == portCPU and
			 egressInfo.trgPort == ingressInfo.trgPort) then
			egressInfoTable[index].portNum = "CPU"
		end

		--[[
		build packet info:
			 l2 = {srcMac = , dstMac = }
			,rTag = { exists = true/false , SeqNr = , }
			,vlanTag = {exists = true/false , vlanId = , cfi = , vpt = }
			,payload =
			,dsaTag = {vidx = }
		]]

		local egress_packetInfo = {
            l2 = {srcMac = packetInfo.l2.srcMac , dstMac = egressInfo.macDa}
		   ,rTag = { exists = egressInfo.rTag , SeqNr = egressInfo.SeqNr }
		   ,vlanTag = {exists = egressInfo.vlanTag , vlanId = egressInfo.vlanId, cfi = 0, vpt = egressInfo.up}
		   ,payload = packetInfo.payload
		   ,dsaTag = egressInfo.dsaTag
           ,ipv4 = {sip = packet_sip , dip = packet_dip} -- same as ingress
		}

		if egressInfo.notEgress == true then -- indication of drop
			egressInfoTable[index].pktInfo = nil
			egressInfoTable[index].packetCount = 0 -- check that not getting traffic
		else
			egressInfoTable[index].pktInfo = {fullPacket = buildPacket(egress_packetInfo)}
			egressInfoTable[index].packetCount = nil
		end

		index = index + 1

	end -- end of loop on egress ports

	if ingressInfo.packetNum == ssl_test then
		if sslw == true then
			shell_execute("sslw")
		else
			shell_execute("ssl")
		end
	end

	if forceCaptureOnTransmitPort then
		-- we need to capture the egress packet that loopback from the port to the CPU
		-- use iPCL rule to trap it
		forceCaptureOnTransmitPortSet(true)

	end

    --printLog("transmitInfo", to_string(transmitInfo))
    --printLog("egressInfoTable", to_string(egressInfoTable))

	local rc = luaTgfTransmitPacketsWithExpectedEgressInfo(
	   transmitInfo, egressInfoTable)
	local result_string = caseName

	if forceCaptureOnTransmitPort then
		-- restore the default
		forceCaptureOnTransmitPortSet(false)
	end

	if ingressInfo.packetNum == ssl_test then
		shell_execute("nssl")
	end

	if rc ~= 0 then
		result_string = result_string .. " FAILED"
		printLog ("ENDED : " .. result_string .. "\n")
		testAddErrorString(result_string)
	else
		printLog ("ENDED : " .. result_string .. "\n")
		testAddPassString(result_string)
	end

	if(ingressInfo.postTestFunc)then
		doFunction(ingressInfo.postTestFunc,ingressInfo.postTestParams)
	end
end

local debugRangeStarted = false

local function doMainLogic(IN_Packets,OUT_Packets)

    for _dummy1, ingressInfo in pairs(IN_Packets) do

        if ingressInfo.packetNum == debugRange.start then
            debugRangeStarted = true
        end

        if debugRange.start and debugRangeStarted == false then
            -- this test located before the start of the range , so skip it
        else

            if(debug_single_packetNum and debug_single_packetNum ~= ingressInfo.packetNum) then
                -- skip this case
            else
                doSingleCaseLogic(ingressInfo,OUT_Packets)
            end
        end

        if ingressInfo.packetNum == debugRange.last then
            --range ended
            break
        end

        if(debug_single_packetNum and debug_single_packetNum == ingressInfo.packetNum) then
            --the single test ended
            break
        end

    end -- loop on IN_Packets[]
end -- end of function doMainLogic

-------------- do the test --------------
executeLocalConfig("dxCh/examples/configurations/_802_1cb.txt")

if not skip_part_1 then
	doMainLogic(incomingPacketsDb_test1,outgoingPacketsDb_test1)
end
if not skip_part_2 then
	doMainLogic(incomingPacketsDb_test2,outgoingPacketsDb_test2)
end
if not skip_part_3 then
    executeLocalConfig("dxCh/examples/configurations/_802_1cb_cpu.txt")
	doMainLogic(incomingPacketsDb_test3,outgoingPacketsDb_test3)
    executeLocalConfig("dxCh/examples/configurations/_802_1cb_cpu_deconfig.txt")
end

executeLocalConfig("dxCh/examples/configurations/_802_1cb_deconfig.txt")
testPrintResultSummary(testName)

--restore the orig ports
restoreOrigPorts()

