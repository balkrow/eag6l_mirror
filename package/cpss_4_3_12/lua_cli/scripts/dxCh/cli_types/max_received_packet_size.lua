--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* max_received_packet_size.lua
--*
--* DESCRIPTION:
--*       type of Maximum Received Packet size
--*
--* FILE REVISION NUMBER:
--*       $Revision: 2 $
--*
--********************************************************************************

--includes


--constants
minimum_of_maximum_received_packet_size = 0 
maximum_of_maximum_received_packet_size = 16382


-- ************************************************************************
---
--  check_max_received_packet_size
--        @description  checking of maximum received packed size
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter rang
--
--        @return       true and maximum received packed size on success, 
--                      otherwise false and error message
--
local function check_max_received_packet_size(param, name, desc)
    param = tonumber(param)
    
    if param == nil then
        return false, name .. " not a number"
    end

    if tostring(name) == "port" then
        maximum_of_maximum_received_packet_size = 10304
    end

    if (param < minimum_of_maximum_received_packet_size) or 
       (param > maximum_of_maximum_received_packet_size) then
        return false, "The Maximum Received Packet size is out of range"
    end

    return true, param
end


-- ************************************************************************
---
--  complete_max_received_packet_size
--        @description  autocompleting of maximum received packed size
--
--        @param param          - checked parameter value
--        @param name           - checked parameter name
--        @param desc           - checked parameter rang
--
--        @return       autocompleting and help array 
--
local function complete_max_received_packet_size(param, name, desc)
    local values = 
        {{DEFAULT_NORMAL_MAXIMUM_RECEIVED_PACKET_SIZE,      "Default normal maximum received packet size"}, 
         {DEFAULT_JUMBO_FRAME_MAXIMUM_RECEIVED_PACKET_SIZE, "Default jumbo-frame maximum received packet size"}, 
         {16382,                                            "Maximum received packet size"}}
    local compl = {}
    local help  = {}
    local k
    
    if tostring(name) == "port" then
        values =
        {{DEFAULT_NORMAL_MAXIMUM_RECEIVED_PACKET_SIZE,      "Default normal maximum received packet size"},
         {DEFAULT_JUMBO_FRAME_MAXIMUM_RECEIVED_PACKET_SIZE, "Default jumbo-frame maximum received packet size"},
         {10240,                                            "MRU of 10240 bytes"},
         {10304,                                            "Maximum supported MRU size "}}
    end

    for k = 1, #values do
        if prefix_match(param, tostring(values[k][1])) then
            table.insert(compl, tostring(values[k][1]))
            help[#compl] = values[k][2]
        end
    end
    
    return compl, help
end


------------------------------------------------------
-- type registration: max_received_packet_size on vlan
-----------------------------------------------------
CLI_type_dict["max_received_packet_size"] = {
    checker = check_max_received_packet_size,
    name = "vlan",
    complete = complete_max_received_packet_size,
    help = "Maximum Received Packet size"
}

-----------------------------------------------------------
-- type registration: max_received_packet_size on port
-----------------------------------------------------------
CLI_type_dict["max_received_packet_size_port"] = {
    checker = check_max_received_packet_size,
    name = "port",
    complete = complete_max_received_packet_size,
    help = "Maximum Received Packet size"
}
