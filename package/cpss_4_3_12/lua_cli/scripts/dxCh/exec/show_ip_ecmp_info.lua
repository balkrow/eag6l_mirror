--********************************************************************************
--*              (c), Copyright 2011, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* show_ip_ecmp_info.lua
--*
--* DESCRIPTION:
--*       showing of the ECMP configuration for configured IP v4 Routes
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

--includes

--constants

function inTable(tbl, item)
    for key, value in pairs(tbl) do
        if value == item then return true end
    end
    return false
end

-- ************************************************************************
---
--  show_ip_ecmp_info_func
--        @description  show's the ECMP configuration for configured IP Routes
--
--        @param params             - params["all_device"]: all or given
--                                    devices iterating property, could be
--                                    irrelevant
--
--        @return       true on success, otherwise false and error message
--
function show_ip_ecmp_info_func(params)
    -- Common variables declaration
    local result, values
    local devNum
    local command_data = Command_Data()
    -- Command specific variables declaration
    local enable_routing, enable_routing_string
    local routing_enabling_first_string_prefix
    local routing_enabling_second_string_prefix
    local routing_enabling_third_string_prefix, routing_enabling_string_suffix
    local text_block_header_string, text_block_footer_string
    local ip_protocol, prefix_ip_address, prefix_length, vrId
    local prefix_string, prefix_output_string, prefix_showing, vrId_string
    local next_hop_id, next_hop_id_string, next_hop_id_output_string
    local is_ecmp, base_ecmp_index, num_ecmp_paths, num_next_hops
    local ecmp_index_output_string
    local next_hop_entry, next_hop_entry_showing, next_hop_entries
    local vlanId, vlanId_string
    local entry_interface, entry_interface_string
    local router_arp_index, mac_address, mac_address_string
    local table_header_string, table_footer_string
    local configured_ip_routs_table_string
	local distinct
	local router_arp_index_array={}
	local is_arpInTheArray
	local ecmp_id_to_filer

	local i

	local ecmpEntryTable
	local indirectEntriesMappingTable
	local routeEntriesTable
	local arpEntriesTable

    local ecmp_entry
	local ecmpNumOfPaths
	local indirectMappingIndex=0;

	local ecmpEntryIndex=0

	-- Common variables initialization
    command_data:clearResultArray()
    command_data:initAllDeviceRange(params)
    command_data:initIpPrefixEntries(params)

    -- Command specific variables initialization
    text_block_header_string                = ""
    routing_enabling_first_string_prefix    = "\n ip routing"
    routing_enabling_third_string_prefix    = ":\t"
    text_block_footer_string                = ""
    ip_protocol                             = params["ip_protocol"]
	distinct								= params["distinct"]
	ecmp_id_to_filer						= params["ecmp_id"]
    is_ipv4_protocol                        =
        isEquivalent(ip_protocol, "CPSS_IP_PROTOCOL_IPV4_E")
    routing_enabling_string_suffix          = ""
    table_header_string                     = ""
    table_footer_string                     = ""

	if(is_ipv4_protocol)then
		print(" location  vrId     prefix/mask    next-hop   vid     interface           mac        \n" ..
			  "                                      ID                                             \n" ..
			  "---------  -----  ----------------  --------  -----  -------------  ------------------\n")
	 else
		print( " location  vrId                   prefix/mask                  next-hop   vid     interface              mac          \n" ..
			   "                                                                  ID                                                  \n" ..
			   "---------- ------  -------------------------------------------  --------  -----  --------------  ----------------------\n")
	end

    -- System specific data initialization.
    command_data:enablePausedPrinting()
    command_data:setEmergencyPrintingHeaderAndFooter(table_header_string,
                                                     table_footer_string)

    -- Main device handling cycle
    if true == command_data["status"] then
        local iterator
        for iterator, devNum in  command_data:getDevicesIterator() do
            command_data:clearDeviceStatus()

            command_data:clearLocalStatus()

            -- Routing enabling getting and its string forming.
            if true  == command_data["local_status"] then
                result, values =
                    cpssPerDeviceParamGet("cpssDxChIpRoutingEnableGet", devNum,
                                          "enableRouting", "GT_BOOL")

                if        0 == result then
                    enable_routing  = values["enableRouting"]
                elseif 0x10 == result then
                    command_data:setFailDeviceStatus()
                    command_data:addWarning("Routing enabling getting is " ..
                                            "not allowed on device %d.", devNum)
                elseif    0 ~= result then
                    command_data:setFailDeviceAndLocalStatus()
                    command_data:addError("Error at routing enabling getting " ..
                                          "on device %d: %s.",
                                          devNum, returnCodes[result])
                end

                if        0 == result then
                    enable_routing_string   = boolEnabledLowerStrGet(enable_routing)
                else
                    enable_routing_string   = "n/a"
                end
            end

            -- Routing enabling second prefix forming.
            if true  == command_data["local_status"] then
                    routing_enabling_second_string_prefix =
                        stringValueIfConditionTrue(multiply_device_presence,
                                                   " on device " ..
                                                   tostring(devNum))
            end

            -- Routing enabling string formatting.
            --command_data:setResultStr(routing_enabling_first_string_prefix,
              --                        routing_enabling_second_string_prefix,
                --                      routing_enabling_third_string_prefix)
            --command_data:setResultStr(command_data["result"],
              --                        enable_routing_string,
                --                      routing_enabling_string_suffix)
            command_data:addResultToResultArray()

            command_data:updateStatus()

            command_data:updateDevices()
        end
    end

    -- Resulting routing enabling string forming.
    enable_routing_string       = command_data:getResultArrayStr()
    enable_routing_string       =
        command_data:getStrOnDevicesCount(text_block_header_string,
                                          enable_routing_string,
                                          text_block_footer_string,
                                          "There is no routing enabling information to show.\n")

    -- Common variables initialization
    command_data:clearResultArray()
    command_data:addToEmergencyPrintingHeader(enable_routing_string)

	ecmpEntryTable = {}
	indirectEntriesMappingTable = {}
	routeEntriesTable = {}
	arpEntriesTable = {}

	-- Main ip prefix handling cycle in LPM
	if true == command_data["status"] then
	local iterator
	for iterator, prefix_ip_address, prefix_length, vrId in
									command_data:getIpPrefixEcmpIterator() do
		command_data:clearEntryStatus()

		command_data:clearLocalStatus()

		-- Prefix and next-hop id string forming.
		if true == command_data["local_status"]     then
			prefix_string       = ipPrefixStrGet(ip_protocol,
												 prefix_ip_address,
												 prefix_length)
			next_hop_id         = command_data["ip-unicast-entry-index"]
			is_ecmp             = command_data["is-ecmp"]
			base_ecmp_index     = command_data["base-ecmp-index"]
			num_ecmp_paths      = command_data["num-ecmp-paths"]
			next_hop_id_string  = tostring(next_hop_id)
			ecmp_index_output_string = tostring(base_ecmp_index)
			vrId_string = tostring(vrId)
		end

		if 1 == is_ecmp then
			num_next_hops = num_ecmp_paths
		else
			num_next_hops = 1
		end

		-- Device handling cycle
		next_hop_entries    = {}
		if true == command_data["local_status"] then
			local device_iterator
			for device_iterator, devNum in
										command_data:getDevicesIterator() do
			  command_data:clearDeviceStatus()

			-- fetch needed data from DB
			if (1 == is_ecmp) then
				ecmp_entry = ecmpEntryTable[base_ecmp_index];
				if(ecmp_entry==nil)then
					-- read the entry from the HW and save it in the DB
					if true  == command_data["local_status"] then
						 result, values, ecmp_entry =wrLogWrapper("wrlCpssDxChEcmpEntryGet", "(devNum, base_ecmp_index)",
																			 devNum, base_ecmp_index)
						if 0 == result then
							ecmpEntryTable[base_ecmp_index] = ecmp_entry

						elseif 0x10 == result then
							command_data:setFailDeviceStatus()
							command_data:addWarning("ECMP entry getting " ..
													"is not  allowed on " ..
													"device %d.", devNum)
						elseif    0 ~= result then
							command_data:setFailDeviceAndLocalStatus()
							command_data:addError("Error at ECMP entry " ..
												  "getting on device %d: %s.",
												  devNum, returnCodes[result])
						end
					end
				end

				ecmpNumOfPaths = ecmp_entry["numOfPaths"]
				ecmpEntryIndex = ecmp_entry["routeEntryBaseIndex"]

				local ecmp_iter
				for ecmp_iter = ecmpEntryIndex,ecmpEntryIndex+ecmpNumOfPaths-1,1 do
					-- get indirect mapping
					indirectMappingIndex=indirectEntriesMappingTable[ecmp_iter];
					if(indirectMappingIndex==nil)then
						-- read the entry from the HW and save it in the DB
						if true  == command_data["local_status"] then
							result, values, indirectMappingIndex = wrLogWrapper("wrlCpssDxChEcmpIndirectIndexGet", "(devNum, ecmp_iter)",
																				 devNum, ecmp_iter)
							if 0 == result then
								indirectEntriesMappingTable[ecmp_iter] = indirectMappingIndex

							elseif 0x10 == result then
								command_data:setFailDeviceStatus()
								command_data:addWarning("ECMP indirect entry getting " ..
														"is not  allowed on " ..
														"device %d.", devNum)
							elseif    0 ~= result then
								command_data:setFailDeviceAndLocalStatus()
								command_data:addError("Error at ECMP indirect entry " ..
													  "getting on device %d: %s.",
													  devNum, returnCodes[result])
							end
						end
					end

					-- get route entry data
					next_hop_entry = routeEntriesTable[indirectMappingIndex];
					if(next_hop_entry==nil)then
						-- read the entry from the HW and save it in the DB
						if true  == command_data["local_status"] then
							result, next_hop_entry =wrLogWrapper("wrlCpssDxChIpUcRouteEntryRead", "(devNum, indirectMappingIndex)",
																		 devNum, indirectMappingIndex)

							if(next_hop_entry~=nil)then
								if  0 == result then
									if next_hop_entry["type"] == "CPSS_DXCH_IP_UC_ROUTE_ENTRY_E" then
										routeEntriesTable[indirectMappingIndex]=next_hop_entry
									else
										command_data:addError("next-hop entry %s is %s",
															tostring(next_hop_entry["type"]),
															to_string(indirectMappingIndex))
									end
								elseif 0x10 == result then
									command_data:setFailDeviceStatus()
									command_data:addWarning("Next-hop entry getting " ..
															"is not  allowed on " ..
															"device %d.", devNum)
								elseif    0 ~= result then
									command_data:setFailDeviceAndLocalStatus()
									command_data:addError("Error at next-hop entry " ..
														  "getting on device %d: %s.",
														  devNum, returnCodes[result])
								end
							else
								command_data:setFailDeviceAndLocalStatus()
								command_data:addError("FDB Error : NH entry is not defined " ..
													  "on device %d: %s.",
													  devNum, returnCodes[result])
							end
						end
					end


					if next_hop_entry["type"] == "CPSS_DXCH_IP_UC_ROUTE_ENTRY_E" then
						vlanId=next_hop_entry["entry"]["regularEntry"]["nextHopVlanId"]
						router_arp_index=next_hop_entry["entry"]["regularEntry"]["nextHopARPPointer"]
						entry_interface=next_hop_entry["entry"]["regularEntry"]["nextHopInterface"]

						-- Next-hop entry vlan id and its interface string forming.
						if((vlanId~=nil)and(indirectMappingIndex~=nil))then
							if true  == command_data["local_status"] then
								vlanId_string           = tostring(vlanId)
								next_hop_id_string      = tostring(indirectMappingIndex)
								entry_interface_string  = interfaceInfoStrGet(entry_interface)
							end
						else
							command_data:setFailDeviceAndLocalStatus()
							command_data:addError("Error: vlanId and indirectMappingIndex are not defined " ..
													  "on device %d: %s.",
													  devNum, returnCodes[result])
						end
					else
						command_data:addError("next-hop entry %s is %s",
											tostring(next_hop_entry["type"]),
											indirectMappingIndex)
					end


					--get arp data
					mac_address = arpEntriesTable[router_arp_index]
					if(mac_address==nil)then
						if true  == command_data["local_status"] then
							 result, values = myGenWrapper("cpssDxChIpRouterArpAddrRead",
														 {{ "IN",  "GT_U8",        "devNum", devNum },
														  { "IN",  "GT_U32",       "routerArpIndex", router_arp_index },
														  { "OUT", "GT_ETHERADDR", "arpMacAddr"     }})
							if  0 == result then
								mac_address         = values["arpMacAddr"]
								arpEntriesTable[router_arp_index]=mac_address
							elseif 0x10 == result then
								command_data:setFailDeviceStatus()
								command_data:addWarning("Arp mac-address getting " ..
														"is not allowed on " ..
														"device %d.", devNum)
							elseif    0 ~= result then
								command_data:setFailDeviceAndLocalStatus()
								command_data:addError("Error at arp mac-address " ..
													  "getting on device %d: %s.",
													  devNum, returnCodes[result])
							end
						end
					end
						mac_address_string = replaceStr(tostring(mac_address),":", "-")
						-- Detecting of output properties and storing output data.
						if true  == command_data["local_status"] then
							next_hop_entry = {vlanId, entry_interface_string, mac_address_string}
							if (distinct=="distinct")then
							if (ecmp_id_to_filer)then
								-- need to filter also according to the ECMP index
								-- the next_hop_id hold the ecmp_index
								if(ecmp_id_to_filer==base_ecmp_index)then
									next_hop_entry_showing =
										isItemNotInTable(next_hop_entry, next_hop_entries)
								else
									next_hop_entry_showing=false
								end
							else
								next_hop_entry_showing =
									isItemNotInTable(next_hop_entry, next_hop_entries)
							end

						else
							if (ecmp_id_to_filer)then
								-- need to filter also according to the ECMP index
								-- the next_hop_id hold the ecmp_index
								if(ecmp_id_to_filer==base_ecmp_index)then
									next_hop_entry_showing=true
								else
									next_hop_entry_showing=false
								end
							else
								next_hop_entry_showing=true
							end
						end
						prefix_showing = isEmptyTable(next_hop_entries)
						next_hop_entries[devNum] = next_hop_entry
					end

					-- Prefix output data updating.
					if true  == command_data["local_status"] then
						prefix_output_string        =
							stringValueIfConditionTrue(prefix_showing, prefix_string)
						next_hop_id_output_string   =
							stringValueIfConditionTrue(prefix_showing, next_hop_id_string)
					end

					command_data:updateStatus()

					-- Resulting string formatting and adding.
					if ecmp_iter == ecmpEntryIndex then
						if(next_hop_entry_showing)then
							if(is_ipv4_protocol)then
								print(string.format("  %-10s%-4s%-18s%-10s%-7s%-15s%-24s",
													alignLeftToCenterStr("LPM", 3),
													alignLeftToCenterStr(vrId_string, 2),
													alignLeftToCenterStr(prefix_output_string, 18),
													alignLeftToCenterStr(ecmp_index_output_string, 8),
													alignLeftToCenterStr("", 5),
													alignLeftToCenterStr("ECMP", 13),
													alignLeftToCenterStr("", 22)))
							 else
								print(string.format( "%-10s%-5s%-45s%-11s%-7s%-16s%-24s",
													 alignLeftToCenterStr("LPM", 3),
													 alignLeftToCenterStr(vrId_string, 2),
													 alignLeftToCenterStr(prefix_output_string, 43),
													 alignLeftToCenterStr(ecmp_index_output_string, 8),
													 alignLeftToCenterStr("", 5),
													 alignLeftToCenterStr("ECMP", 14),
													 alignLeftToCenterStr("", 22)))
							end
						end
					end

				   vrId_string = ""
				   prefix_output_string = ""
				   next_hop_id_output_string = next_hop_id_string

					if(next_hop_entry_showing)then
						if(is_ipv4_protocol)then
							print(string.format("  %-4s%-28s%-10s%-7s%-15s%-24s",
												 alignLeftToCenterStr(vrId_string, 2),
												 alignLeftToCenterStr(prefix_output_string, 18),
												 alignLeftToCenterStr(next_hop_id_output_string, 8),
												 alignLeftToCenterStr(vlanId_string, 5),
												 alignLeftToCenterStr(entry_interface_string, 13),
												 alignLeftToCenterStr(mac_address_string, 22)))
						 else
							print(string.format( "%-5s%-55s%-11s%-7s%-16s%-24s",
												 alignLeftToCenterStr(vrId_string, 2),
												 alignLeftToCenterStr(prefix_output_string, 43),
												 alignLeftToCenterStr(next_hop_id_output_string, 8),
												 alignLeftToCenterStr(vlanId_string, 5),
												 alignLeftToCenterStr(entry_interface_string, 14),
												 alignLeftToCenterStr(mac_address_string, 22)))
						end
					end
				end
			end

			command_data:updateDevices()
			end
		end

		command_data:updateStatus()

		command_data:updateEntries()
		end
    end

	-- Resulting configured ip routs string forming.
    configured_ip_routs_table_string    = command_data["result_array"]
    configured_ip_routs_table_string    =
        command_data:getStrOnEntriesCount(table_header_string,
                                          configured_ip_routs_table_string,
                                          table_footer_string,
                                          "There is no configured LPM ip routs to show.\n")

    -- Resulting table string formatting
    command_data:setResultStr(enable_routing_string,
                              configured_ip_routs_table_string)

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

    -- Main ip prefix handling cycle in FDB

	if(is_ipv4_protocol)then
			print(" location  vrId     prefix/mask    next-hop   vid     interface           mac        \n" ..
				  "                                      ID                                             \n" ..
				  "---------- -----  ----------------  --------  -----  -------------  ------------------\n")
		 else
			print( " location  vrId                   prefix/mask                  next-hop   vid     interface              mac          \n" ..
				   "                                                                  ID                                                  \n" ..
				   "--------- ------  -------------------------------------------  --------  -----  --------------  ----------------------\n")
	end

	local table_info = {
        { head="Index",len=5,align="c",path="mac_entry_index",type="number" },
        { head="Address",len=22,align="c",path="addr_s"},
        { head="Vlan",len=5,align="c",path="mac_entry.key.key.macVlan.vlanId" },
        { head="VID1",len=5,align="c",path="mac_entry.key.vid1" },
        { head="Skip",len=4,align="c",path="skip_valid_aged.skip",type="boolYN" },
        { head="Interface",len=14,path="mac_entry.dstInterface",type=interfaceInfoStrGet },
        { head="Static",len=7,path="mac_entry.isStatic",type="bool" },
        { head="DA Route",len=8,path="mac_entry.daRoute",type="boolYN" },
        { head="saCommand",len=9,path="mac_entry.saCommand",type=mac_table_cmd_view },
        { head="daCommand",len=9,path="mac_entry.daCommand",type=mac_table_cmd_view },
        { head="sp",len=7,path="mac_entry.spUnknown",type="boolYN" }
    }
	command_data:clearResultArray()
	command_data:initAllDeviceMacEntryIterator(
        params, { ["dstInterface"] =
                    {["type"]    = { "CPSS_INTERFACE_PORT_E",
                                     "CPSS_INTERFACE_TRUNK_E",
                                     "CPSS_INTERFACE_VIDX_E",
                                     "CPSS_INTERFACE_VID_E"    }}})

	command_data:initTable(table_info)
    if true == command_data["status"] then
        local iterator
        for iterator, devNum, entry_index, prefix_ip_address, prefix_length, vrId in
										  command_data:getValidFdbIpEcmpEntryIterator() do
            command_data:clearEntryStatus()

            command_data:clearLocalStatus()

            -- Prefix and next-hop id string forming.
			if((prefix_ip_address~=nil)and (prefix_length~=nil)and (vrId~=nil))then
				if true == command_data["local_status"]     then
					prefix_string       = ipPrefixStrGet(ip_protocol,
														 prefix_ip_address,
														 prefix_length)
					next_hop_id         = command_data["ip-unicast-entry-index"]
					is_ecmp             = command_data["is-ecmp"]
					base_ecmp_index     = command_data["base-ecmp-index"]
					num_ecmp_paths      = command_data["num-ecmp-paths"]
					next_hop_id_string  = tostring(next_hop_id)
					ecmp_index_output_string = tostring(base_ecmp_index)
					vrId_string = tostring(vrId)
				end

				if 1 == is_ecmp then
					num_next_hops = num_ecmp_paths
				else
					num_next_hops = 1
				end

				-- Device handling cycle
				next_hop_entries    = {}
				if true == command_data["local_status"] then
					local device_iterator
					for device_iterator, devNum in
												command_data:getDevicesIterator() do
					  command_data:clearDeviceStatus()
					-- fetch needed data from DB
					if (1 == is_ecmp) then
						ecmp_entry = ecmpEntryTable[base_ecmp_index];
						if(ecmp_entry==nil)then
							-- read the entry from the HW and save it in the DB
							if true  == command_data["local_status"] then
								 result, values, ecmp_entry =wrLogWrapper("wrlCpssDxChEcmpEntryGet", "(devNum, base_ecmp_index)",
																					 devNum, base_ecmp_index)
								if 0 == result then
									ecmpEntryTable[base_ecmp_index] = ecmp_entry

								elseif 0x10 == result then
									command_data:setFailDeviceStatus()
									command_data:addWarning("ECMP entry getting " ..
															"is not  allowed on " ..
															"device %d.", devNum)
								elseif    0 ~= result then
									command_data:setFailDeviceAndLocalStatus()
									command_data:addError("Error at ECMP entry " ..
														  "getting on device %d: %s.",
														  devNum, returnCodes[result])
								end
							end
						end

						ecmpNumOfPaths = ecmp_entry["numOfPaths"]
						ecmpEntryIndex = ecmp_entry["routeEntryBaseIndex"]

						local ecmp_iter
						for ecmp_iter = ecmpEntryIndex,ecmpEntryIndex+ecmpNumOfPaths-1,1 do
							-- get indirect mapping
							indirectMappingIndex=indirectEntriesMappingTable[ecmp_iter];
							if(indirectMappingIndex==nil)then
								-- read the entry from the HW and save it in the DB
								if true  == command_data["local_status"] then
									result, values, indirectMappingIndex = wrLogWrapper("wrlCpssDxChEcmpIndirectIndexGet", "(devNum, ecmp_iter)",
																						 devNum, ecmp_iter)
									if 0 == result then
										indirectEntriesMappingTable[ecmp_iter] = indirectMappingIndex

									elseif 0x10 == result then
										command_data:setFailDeviceStatus()
										command_data:addWarning("ECMP indirect entry getting " ..
																"is not  allowed on " ..
																"device %d.", devNum)
									elseif    0 ~= result then
										command_data:setFailDeviceAndLocalStatus()
										command_data:addError("Error at ECMP indirect entry " ..
															  "getting on device %d: %s.",
															  devNum, returnCodes[result])
									end
								end
							end

							-- get route entry data
							next_hop_entry = routeEntriesTable[indirectMappingIndex];
							if(next_hop_entry==nil)then
								-- read the entry from the HW and save it in the DB
								if true  == command_data["local_status"] then
									result, next_hop_entry =wrLogWrapper("wrlCpssDxChIpUcRouteEntryRead", "(devNum, indirectMappingIndex)",
																				 devNum, indirectMappingIndex)

									if(next_hop_entry~=nil)then
										if  0 == result then
											if next_hop_entry["type"] == "CPSS_DXCH_IP_UC_ROUTE_ENTRY_E" then
												routeEntriesTable[indirectMappingIndex]=next_hop_entry
											else
												command_data:addError("next-hop entry %s is %s",
																	tostring(next_hop_entry["type"]),
																	to_string(indirectMappingIndex))
											end
										elseif 0x10 == result then
											command_data:setFailDeviceStatus()
											command_data:addWarning("Next-hop entry getting " ..
																	"is not  allowed on " ..
																	"device %d.", devNum)
										elseif    0 ~= result then
											command_data:setFailDeviceAndLocalStatus()
											command_data:addError("Error at next-hop entry " ..
																  "getting on device %d: %s.",
																  devNum, returnCodes[result])
										end
									else
										command_data:setFailDeviceAndLocalStatus()
										command_data:addError("FDB Error : NH entry is not defined " ..
													  "on device %d: %s.",
													  devNum, returnCodes[result])
									end
								end
							end


							if next_hop_entry["type"] == "CPSS_DXCH_IP_UC_ROUTE_ENTRY_E" then
								vlanId=next_hop_entry["entry"]["regularEntry"]["nextHopVlanId"]
								router_arp_index=next_hop_entry["entry"]["regularEntry"]["nextHopARPPointer"]
								entry_interface=next_hop_entry["entry"]["regularEntry"]["nextHopInterface"]

								-- Next-hop entry vlan id and its interface string forming.
								if((vlanId~=nil)and(indirectMappingIndex~=nil))then
									if true  == command_data["local_status"] then
										vlanId_string           = tostring(vlanId)
										next_hop_id_string      = tostring(indirectMappingIndex)
										entry_interface_string  = interfaceInfoStrGet(entry_interface)
									end
								else
									command_data:setFailDeviceAndLocalStatus()
									command_data:addError("Error: vlanId and indirectMappingIndex are not defined " ..
															  "on device %d: %s.",
															  devNum, returnCodes[result])
								end
							else
								command_data:addError("next-hop entry %s is %s",
													tostring(next_hop_entry["type"]),
													indirectMappingIndex)
							end


							--get arp data
							mac_address = arpEntriesTable[router_arp_index]
							if(mac_address==nil)then
								if true  == command_data["local_status"] then
									 result, values = myGenWrapper("cpssDxChIpRouterArpAddrRead",
																 {{ "IN",  "GT_U8",        "devNum", devNum },
																  { "IN",  "GT_U32",       "routerArpIndex", router_arp_index },
																  { "OUT", "GT_ETHERADDR", "arpMacAddr"     }})
									if  0 == result then
										mac_address         = values["arpMacAddr"]
										arpEntriesTable[router_arp_index]=mac_address
									elseif 0x10 == result then
										command_data:setFailDeviceStatus()
										command_data:addWarning("Arp mac-address getting " ..
																"is not allowed on " ..
																"device %d.", devNum)
									elseif    0 ~= result then
										command_data:setFailDeviceAndLocalStatus()
										command_data:addError("Error at arp mac-address " ..
															  "getting on device %d: %s.",
															  devNum, returnCodes[result])
									end
								end
							end
								mac_address_string = replaceStr(tostring(mac_address),":", "-")

								-- Detecting of output properties and storing output data.
								if true  == command_data["local_status"] then
									next_hop_entry = {vlanId, entry_interface_string, mac_address_string}
									if (distinct=="distinct")then
									if (ecmp_id_to_filer)then
										-- need to filter also according to the ECMP index
										-- the next_hop_id hold the ecmp_index
										if(ecmp_id_to_filer==base_ecmp_index)then
											next_hop_entry_showing =
												isItemNotInTable(next_hop_entry, next_hop_entries)
										else
											next_hop_entry_showing=false
										end
									else
										next_hop_entry_showing =
											isItemNotInTable(next_hop_entry, next_hop_entries)
									end

								else
									if (ecmp_id_to_filer)then
										-- need to filter also according to the ECMP index
										-- the next_hop_id hold the ecmp_index
										if(ecmp_id_to_filer==base_ecmp_index)then
											next_hop_entry_showing=true
										else
											next_hop_entry_showing=false
										end
									else
										next_hop_entry_showing=true
									end
								end
								prefix_showing = isEmptyTable(next_hop_entries)
								next_hop_entries[devNum] = next_hop_entry
							end

							-- Prefix output data updating.
							if true  == command_data["local_status"] then
								prefix_output_string        =
									stringValueIfConditionTrue(prefix_showing, prefix_string)
								next_hop_id_output_string   =
									stringValueIfConditionTrue(prefix_showing, next_hop_id_string)
							end

							command_data:updateStatus()

							-- Resulting string formatting and adding.
							if ecmp_iter == ecmpEntryIndex then
								if(next_hop_entry_showing)then
									if(is_ipv4_protocol)then
										print(string.format("  %-10s%-4s%-18s%-10s%-7s%-15s%-24s",
											alignLeftToCenterStr("FDB", 3),
											alignLeftToCenterStr(vrId_string, 2),
											alignLeftToCenterStr(prefix_output_string, 18),
											alignLeftToCenterStr(ecmp_index_output_string, 8),
											alignLeftToCenterStr("", 5),
											alignLeftToCenterStr("ECMP", 13),
											alignLeftToCenterStr("", 22)))
									 else
										print(string.format("%-10s%-5s%-45s%-11s%-7s%-16s%-24s",
											alignLeftToCenterStr("FDB", 3),
											alignLeftToCenterStr(vrId_string, 2),
											alignLeftToCenterStr(prefix_output_string, 43),
											alignLeftToCenterStr(ecmp_index_output_string, 8),
											alignLeftToCenterStr("", 5),
											alignLeftToCenterStr("ECMP", 14),
											alignLeftToCenterStr("", 22)))
									end
								end
							end

							vrId_string = ""
							prefix_output_string = ""
							next_hop_id_output_string = next_hop_id_string

							if(next_hop_entry_showing)then
								if(is_ipv4_protocol)then
									print(string.format("  %-4s%-28s%-10s%-7s%-15s%-24s",
										alignLeftToCenterStr(vrId_string, 2),
										alignLeftToCenterStr(prefix_output_string, 18),
										alignLeftToCenterStr(next_hop_id_output_string, 8),
										alignLeftToCenterStr(vlanId_string, 5),
										alignLeftToCenterStr(entry_interface_string, 13),
										alignLeftToCenterStr(mac_address_string, 22)))
								 else
									print(string.format("%-5s%-55s%-11s%-7s%-16s%-24s",
										alignLeftToCenterStr(vrId_string, 2),
										alignLeftToCenterStr(prefix_output_string, 43),
										alignLeftToCenterStr(next_hop_id_output_string, 8),
										alignLeftToCenterStr(vlanId_string, 5),
										alignLeftToCenterStr(entry_interface_string, 14),
										alignLeftToCenterStr(mac_address_string, 22)))
								end
							end
						end
					end
					command_data:updateDevices()
				    end
				end
			end

            command_data:updateStatus()

            command_data:updateEntries()
        end
    end

	-- Resulting configured ip routs string forming.
    configured_ip_routs_table_string    = command_data["result_array"]
    configured_ip_routs_table_string    =
        command_data:getStrOnEntriesCount(table_header_string,
                                          configured_ip_routs_table_string,
                                          table_footer_string,
                                          "There is no configured FDB ip hosts to show.\n")

    -- Resulting table string formatting
    command_data:setResultStr(enable_routing_string,
                              configured_ip_routs_table_string)

    command_data:analyzeCommandExecution()

    command_data:printCommandExecutionResults()

	return command_data:getCommandExecutionResults()
end


--------------------------------------------------------------------------------
-- command registration: show ip ecmp info
--------------------------------------------------------------------------------
CLI_addHelp("exec", "show ip ecmp_info", "ipv4 ECMP Information")
CLI_addCommand("exec", "show ip ecmp_info", {
  func   = function(params)
               params.ip_protocol = "CPSS_IP_PROTOCOL_IPV4_E"
               return show_ip_ecmp_info_func(params)
           end,
  help   = "Current information of the ipv4 ECMP entries in LPM and FDB prefixes ",
  params = {
        {
            type = "named",
             "#all_device",
			mandatory = { "all_device" },
			{ format = "ecmp_id  %GT_U32", name = "ecmp_id", help = "Show all the LPM entries and FDB entries with ecmp_id" },
            { format = "distinct", name = "distinct", help = "Filter ECMP NH duplication" },
        }
    }
})
