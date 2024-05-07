--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* txq.lua
--*
--* DESCRIPTION:
--*       Basic debug menu for queue buffer status ans port mapping
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
function get_bit(value,offset,size)
  if(value==nil)then
   print("nil value")
   return 0
  else
   return (math.floor(value/2^offset)%2^size)
  end
end


local function dump_port_tc_buffers(params)
    local res1,res2,val1,val2,devices,j,ports,i,tc
    local maxQ,sdmaPortsNum


    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end



    for j=1,#devices do
        if params["portNum"]==nil then
         ports=luaCLI_getDevInfo(devices[j])[devices[j]]
         ret, val = myGenWrapper("prvCpssDxChSdmaPortNumGet", {
                            { "IN", "GT_U8"  , "devNum", devices[j]},
                            { "OUT", "GT_U32", "sdmaPortNumPtr"}
                    })
         if ret ~=0 then
           print("Fail in prvCpssDxChSdmaPortNumGet")
           return
         end
         sdmaPortsNum = val.sdmaPortNumPtr
        for i=0,sdmaPortsNum-1 do
         ret, val = myGenWrapper("prvCpssDxChSdmaPhysPortGet", {
                            { "IN", "GT_U8"  , "devNum", devices[j]},
                            { "IN", "GT_U32" , "index",i},
                            { "OUT", "GT_U32", "sdmaPhysPortPtr"}
                    })
         if ret ~=0 then
          print("Fail in prvCpssDxChSdmaPhysPortGet")
          return
         end --if ret ~=0 then
         ports[#ports+1] =val.sdmaPhysPortPtr
        end--if params["portNum"]==nil then
        else
         ports={params["portNum"]}
        end


        for i=1,#ports do
            if(is_sip_6(devices[j]))then

                 res1, val1 = myGenWrapper("prvCpssSip6TxqUtilsMaxQueueOffsetGet", {
                            { "IN", "GT_U8"  , "devNum", devices[j]},
                            { "IN", "GT_U32" , "portNum",  ports[i]},
                            { "OUT", "GT_U32", "maxQ"},
                    })
                if res1 ~=0 then
                print("Fail in prvCpssSip6TxqUtilsMaxQueueOffsetGet")
                return
                end
                maxQ = val1.maxQ

                print(string.format("Device:%5d  Port:%5d ",devices[j],ports[i]))
                for tc=0,maxQ do
                    res1, val1 = myGenWrapper("cpssDxChPortTx4TcBufNumberGet", {
                            { "IN", "GT_U8"  , "devNum", devices[j]},
                            { "IN", "GT_U32" , "portNum",  ports[i]},
                            { "IN", "GT_U8" , "trafClass",  tc},
                            { "OUT", "GT_U32", "numPtr"},
                    })

                    if tc<8 then
                    res2, val2 = myGenWrapper("cpssDxChPortPfcCounterGet", {
                            { "IN", "GT_U8"  , "devNum", devices[j]},
                            { "IN", "GT_U8" , "tcQueue",  tc},
                            { "IN", "GT_U32" , "portNum",  ports[i]},
                            { "OUT","GT_U32", "pfcCounterValuePtr"},
                    })
                    else
                    --TC for ingress support only 8 tc
                    res2=18
                    end

                    if (res1==0) and (res2==0 or res2==18) then
                        if(res2==18)then
                         if tc<8 then
                         val2["pfcCounterValuePtr"]="Flow control not set on this TC"
                         else
                         val2["pfcCounterValuePtr"]="Not applicable on this TC"
                         end
                        end
                        print(string.format("  tc:%3d Egress buffers:%5d   Ingress buffers: ",tc,val1["numPtr"])..val2["pfcCounterValuePtr"])
                    else
                        print("Could not get device "..devices[j] ..  " Port "..ports[i].." data")
                    end
                end
                print("")
            else
                print("Device "..devices[j] ..  " is not supported since it is not SIP 6")
            end
        end
    end
end


local function dump_port_buffers(params)
    local res1,res2,val1,val2,devices,j,ports,i,sdmaPortsNum

    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do
        if params["portNum"]==nil then ports=luaCLI_getDevInfo(devices[j])[devices[j]]  else ports={params["portNum"]} end

        if params["portNum"]==nil then
         ports=luaCLI_getDevInfo(devices[j])[devices[j]]
         ret, val = myGenWrapper("prvCpssDxChSdmaPortNumGet", {
                            { "IN", "GT_U8"  , "devNum", devices[j]},
                            { "OUT", "GT_U32", "sdmaPortNumPtr"}
                    })
         if ret ~=0 then
           print("Fail in prvCpssDxChSdmaPortNumGet")
           return
         end
         sdmaPortsNum = val.sdmaPortNumPtr
        for i=0,sdmaPortsNum-1 do
         ret, val = myGenWrapper("prvCpssDxChSdmaPhysPortGet", {
                            { "IN", "GT_U8"  , "devNum", devices[j]},
                            { "IN", "GT_U32" , "index",i},
                            { "OUT", "GT_U32", "sdmaPhysPortPtr"}
                    })
         if ret ~=0 then
          print("Fail in prvCpssDxChSdmaPhysPortGet")
          return
         end --if ret ~=0 then
         ports[#ports+1] =val.sdmaPhysPortPtr
        end--if params["portNum"]==nil then
        else
         ports={params["portNum"]}
        end

        for i=1,#ports do
            if(is_sip_6(devices[j]))then
                res1, val1 = myGenWrapper("cpssDxChPortTxBufNumberGet", {
                        { "IN", "GT_U8"  , "devNum", devices[j]},
                        { "IN", "GT_U32" , "portNum",  ports[i]},
                        { "OUT", "GT_U32", "numPtr"},
                })

                res2, val2 = myGenWrapper("cpssDxChPortPfcCounterGet", {
                        { "IN", "GT_U8"  , "devNum", devices[j]},
                        { "IN", "GT_U8" , "tcQueue",  0xFF},
                        { "IN", "GT_U32" , "portNum",  ports[i]},
                        { "OUT","GT_U32", "pfcCounterValuePtr"},
                })


                if (res1==0) and (res2==0 or res2==18) then
                    if(res2==18)then val2["pfcCounterValuePtr"]="NA" end
                    print(string.format("Device:%5d  Port:%5d  Egress buffers:%5d   Ingress buffers: ",devices[j],ports[i],val1["numPtr"])..val2["pfcCounterValuePtr"])
                else
                    print("Could not get device "..devices[j] ..  " Port "..ports[i].." data")
                end
            else
                print("Device "..devices[j] ..  " is not supported since it is not SIP 6")
            end
        end
    end
end


-------------------------------------------------------------------------------------------------------------------------

 function dump_port_pds_params(params)
    local res,val,devices,j,ports,i,longQbit,validBit

    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do
        if params["portNum"]==nil then ports=luaCLI_getDevInfo(devices[j])[devices[j]]  else ports={params["portNum"]} end

        for i=1,#ports do
            if(is_sip_6(devices[j]))then
                res, val = myGenWrapper("prvCpssTxqSip6DebugPdsProfileGet", {
                        { "IN", "GT_U8"  , "devNum", devices[j]},
                        { "IN", "GT_U32" , "portNum",  ports[i]},
                        { "OUT", "GT_U32", "pdsProfileIndex"},
                        { "OUT", "GT_U32", "speedInG"},
                        { "OUT", "GT_BOOL", "longQueueEnable"},
                        { "OUT", "GT_U32", "headEmptyLimit"},
                        { "OUT", "GT_U32", "longQueueLimit"},
                        { "OUT", "GT_BOOL", "creditValid"}
                })

                if val["longQueueEnable"]==true then
                 longQbit=1
                else
                 longQbit=0
                end

                if val["creditValid"]==true then
                 validBit=1
                else
                 validBit=0
                end

                if (res==0)then
                    print(string.format("Dev:%3d Port:%5d Profile:%2d speed(G):%3d LongQ %d Head %3d Tail %3d Valid %d",devices[j],ports[i],val["pdsProfileIndex"],val["speedInG"],longQbit,val["headEmptyLimit"],val["longQueueLimit"],validBit))
                else
                    print("Could not get device "..devices[j] ..  " Port "..ports[i].." data")
                end
            else
                print("Device "..devices[j] ..  " is not supported since it is not SIP 6")
            end
        end
    end
end

---------------------------------------------------------------------------------


function dump_port_length_adjust_params(params)
    local res,val,devices,j,ports,i,enableBit,validBit

    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do
        if params["portNum"]==nil then ports=luaCLI_getDevInfo(devices[j])[devices[j]]  else ports={params["portNum"]} end

        for i=1,#ports do
            if(is_sip_6(devices[j]))then
                res, val = myGenWrapper("prvCpssTxqSip6DebugLengthAdjustProfileGet", {
                        { "IN", "GT_U8"  , "devNum", devices[j]},
                        { "IN", "GT_U32" , "portNum",  ports[i]},
                        { "OUT", "GT_U32", "pdsProfileIndex"},
                        { "OUT", "GT_BOOL", "lengthAdjustEnable"},
                        { "OUT", "GT_U32", "lengthAdjustSubstruct"},
                        { "OUT", "GT_U32", "lengthAdjustByteCount"},
                        { "OUT", "GT_BOOL", "creditValid"}
                })

                if val["lengthAdjustEnable"]==true then
                 enableBit=1
                else
                 enableBit=0
                end

                if val["creditValid"]==true then
                 validBit=1
                else
                 validBit=0
                end

                if (res==0)then
                    print(string.format("Dev:%3d Port:%5d Profile:%2d Enable %d Sub %d BC %5d Valid %d",devices[j],ports[i],val["pdsProfileIndex"],enableBit,val["lengthAdjustSubstruct"],val["lengthAdjustByteCount"],validBit))
                else
                    print("Could not get device "..devices[j] ..  " Port "..ports[i].." data")
                end
            else
                print("Device "..devices[j] ..  " is not supported since it is not SIP 6")
            end
        end
    end
end


---------------------------------------------------------------------------------
local function dump_port_mapping(params)
    local res,values2,values3,values4,values5,devices,j,ports,i
    local portMapShadow,isRemote,isSdma,aNodeInd
    local str,isCascade,isCpuSdma,globalQ,fullTree=false
    local hqosId0
    local goQHw


    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do
        if params["portNum"]==nil then ports=luaCLI_getDevInfo(devices[j])[devices[j]]  else ports={params["portNum"]} end

        print("Device "..devices[j])

        for i=1,#ports do
            if(is_sip_6(devices[j]))then
                isCascade=0
                isRemote=0
                isCpuSdma=0
                res, values2 = myGenWrapper("prvCpssSip7TxqUtilsTreeModeFullGet", {
                                { "IN", "GT_U8"  , "devNum", devices[j]},
                                { "OUT", "GT_BOOL", "fullTreePtr"}
                            })

                if(res~=0)then
                    print("Error "..res.." calling prvCpssSip7TxqUtilsTreeModeFullGet")
                    return
                end

                fullTree = values2.fullTreePtr

                res, portMapShadow = myGenWrapper("cpssDxChPortPhysicalPortDetailedMapGet", {
                        { "IN", "GT_U8"  , "devNum", devices[j]},
                        { "IN", "GT_U32" , "portNum",  ports[i]},
                        { "OUT", "CPSS_DXCH_DETAILED_PORT_MAP_STC", "portMapShadowPtr"},
                })

                if(res==0 and portMapShadow["portMapShadowPtr"].valid==true)then
                    res, values2 = myGenWrapper("prvCpssFalconTxqUtilsPortGlobalUniqueIndexGet", {
                        { "IN", "GT_U8"  , "devNum", devices[j]},
                        { "IN", "GT_U32" , "portNum",  ports[i]},
                        { "OUT", "GT_U32", "globalUniqueIndexPtr"},
                    })
                   if(res~=0)then
                    print("Error "..res.." calling prvCpssFalconTxqUtilsPortGlobalUniqueIndexGet")
                    return
                   end

                   res, values3 = myGenWrapper("prvCpssFalconTxqUtilsIsCascadePort", {
                            { "IN", "GT_U8"  , "devNum", devices[j]},
                            { "IN", "GT_U32" , "portNum",  ports[i]},
                            { "OUT", "GT_BOOL", "isCascadePtr"},
                        })
                    if(values3["isCascadePtr"])then
                        isCascade=1
                        aNodeInd=-1
                    end

                    if(res==0 )then
                        if(isCascade~=1)then
                        aNodeInd = portMapShadow["portMapShadowPtr"].extPortMap.txqInfo.aNodeIndex
                        end
                        globalQ=values2["globalUniqueIndexPtr"]
                        isRemote = myGenWrapper("prvCpssDxChPortRemotePortCheck", {
                            { "IN", "GT_U8"  , "devNum", devices[j]},
                            { "IN", "GT_U32" , "portNum",  ports[i]},
                        })
                        if(isRemote==0) then
                            res,isSdma = myGenWrapper("prvCpssDxChPortPhysicalPortIsCpu", {
                                { "IN", "GT_U8"  , "devNum", devices[j]},
                                { "IN", "GT_U32" , "portNum",  ports[i]},
                                { "OUT", "GT_BOOL", "isCpuPortPtr"},
                            })
                            if(isSdma["isCpuPortPtr"]==true)then isCpuSdma =1 end
                        end
                    end
                end

                if (res==0 and portMapShadow["portMapShadowPtr"].valid ==true)then

                    if(isCascade==1)then str="CSCD"
                    elseif(isRemote==1) then str="RM"
                    elseif(isCpuSdma==1) then  str="SDMA"
                    else str="LOC" end

                    res, values3 = myGenWrapper("prvCpssDxChTxqSip6GoQIndexGet", {
                            { "IN", "GT_U8"  , "devNum", devices[j]},
                            { "IN", "GT_U32" , "portNum",  ports[i]},
                            { "IN", "GT_BOOL" , "readFromHw", 0},
                            { "OUT", "GT_U32", "goQ"},
                        })

                    if(res~=0)then
                        print("Error "..res.." calling prvCpssDxChTxqSip6GoQIndexGet")
                        return
                    end

                    res, values2 = myGenWrapper("prvCpssDxChTxqSip6GoQIndexGet", {
                            { "IN", "GT_U8"  , "devNum", devices[j]},
                            { "IN", "GT_U32" , "portNum",  ports[i]},
                            { "IN", "GT_BOOL" , "readFromHw", 1},
                            { "OUT", "GT_U32", "goQ"},
                        })

                    if(res~=0)then
                        print("Error "..res.." calling prvCpssDxChTxqSip6GoQIndexGet")
                        return
                    end
                    goQHw = values2["goQ"]
                    if(isCascade~=1) then
                        res, values4 = myGenWrapper("prvCpssSip6PhysicalPortToBCIndexGet", {
                                { "IN", "GT_U8"  , "devNum", devices[j]},
                                { "IN", "GT_U32" , "portNum",  ports[i]},
                                { "OUT", "GT_U32", "bNodeIndexPtr"},
                                { "OUT", "GT_U32", "cNodeIndexPtr"},
                            })

                        if(res~=0)then
                            print("Error "..res.." calling prvCpssSip6PhysicalPortToBCIndexGet")
                            return
                        end
                        if true==fullTree then
                            res, values5 = myGenWrapper("prvCpssSip7TxqUtilsHqosIdGet", {
                                    { "IN", "GT_U8"  , "devNum", devices[j]},
                                    { "IN", "GT_U32" , "portNum",  ports[i]},
                                    { "IN", "GT_U32" , "hqosIndex",  0},
                                    { "OUT", "GT_U32", "hqosIdPtr"}
                                })
                            if(res~=0)then
                             print("Error "..res.." calling prvCpssSip7TxqUtilsHqosIdGet")
                             return
                            end
                            hqosId0 = values5.hqosIdPtr
                            res, values5 = myGenWrapper("prvCpssSip7TxqUtilsHqosIdGet", {
                                    { "IN", "GT_U8"  , "devNum", devices[j]},
                                    { "IN", "GT_U32" , "portNum",  ports[i]},
                                    { "IN", "GT_U32" , "hqosIndex",  1},
                                    { "OUT", "GT_U32", "hqosIdPtr"}
                                })
                            if(res~=0)then
                             print("Error "..res.." calling prvCpssSip7TxqUtilsHqosIdGet")
                             return
                            end
                        end

                    end

                    print("Physical port number :  "..ports[i])
                    print("  Tile               :  "..portMapShadow["portMapShadowPtr"].extPortMap.tileId)
                    print("  Local dp (aNode sw):  "..portMapShadow["portMapShadowPtr"].extPortMap.localDpInTile)
                    print("  Local port in dp   :  "..portMapShadow["portMapShadowPtr"].extPortMap.localPortInDp)


                    if is_sip_7(devices[j]) then

                     res, values2 = myGenWrapper("prvCpssSip7DebugEgfQagTargetPortGlobalDpGet", {
                            { "IN", "GT_U8"  , "devNum", devices[j]},
                            { "IN", "GT_U32" , "portNum",  ports[i]},
                            { "OUT", "GT_U32" , "globalDpIdPtr"},
                        })
                      if(res~=0)then
                        print("Error "..res.." calling prvCpssSip7DebugEgfQagTargetPortGlobalDpGet")
                        return
                      end

                     print("  Global DP (HW EGF) :  "..values2.globalDpIdPtr)

                     for i=0,portMapShadow["portMapShadowPtr"].extPortMap.txqInfo.numberOfQueues-1 do
                         res, values2 = myGenWrapper("prvCpssSip7EgfQagQueueMapTableGet", {
                                { "IN", "GT_U8"  , "devNum", devices[j]},
                                { "IN", "GT_U32" , "globalQIndex", goQHw+i},
                                { "OUT", "GT_U32" , "localDpQIndexPtr"},
                            })
                          if(res~=0)then
                            print("Error "..res.." calling prvCpssSip7DebugEgfQagTargetPortGlobalDpGet")
                            return
                          end
                          print("  Offset "..i.." Local DP [gQi = "..goQHw+i.."] (HW EGF):  "..values2.localDpQIndexPtr)
                      end --for
                    end

                    print("  PDQ first Q        :  "..portMapShadow["portMapShadowPtr"].extPortMap.txqInfo.primarySchFirstQueueIndex)
                    print("  SDQ first Q        :  "..portMapShadow["portMapShadowPtr"].extPortMap.txqInfo.secondarySchFirstQueueIndex)
                    print("  Number of Qs       :  "..portMapShadow["portMapShadowPtr"].extPortMap.txqInfo.numberOfQueues)
                    print("  Global Q index     :  "..globalQ)

                    if(isCascade~=1) then
                     print("  B node index       :  "..values4["bNodeIndexPtr"])
                     print("  C node index       :  "..values4["cNodeIndexPtr"])
                     print("  A node index       :  "..aNodeInd)
                    else
                     print("  B node index       :  N/A")
                     print("  C node index       :  N/A")
                     print("  A node index       :  N/A")
                    end
                    print("  P node index       :  "..portMapShadow["portMapShadowPtr"].extPortMap.txqInfo.pNodeIndex)
                    print("  Type(RM/CSCD/LOC)  :  "..str)
                    print("  DMA                :  "..portMapShadow["portMapShadowPtr"].portMap.txDmaNum)
                    print("  GoQ (SW db)        :  "..values3["goQ"])
                    print("  GoQ (HW)           :  "..goQHw)
                    --only for hawk
                    if(is_supported_feature(devices[j], "PREEMPTION_MAC_802_3BR"))then

                     res, values2 = myGenWrapper("prvCpssDxChTxqSip6_10PreemptionEnableGet", {
                            { "IN", "GT_U8"  , "devNum", devices[j]},
                            { "IN", "GT_U32" , "portNum",  ports[i]},
                            { "IN", "GT_U32"  , "speed", 0},
                            { "OUT", "GT_BOOL" , "enablePtr"},
                        })

                     if(res~=0)then
                        print("Error "..res.." calling prvCpssDxChTxqSip6_10PreemptionEnableGet")
                        return
                     end
                     print("  Preemption enable  :  "..tostring(values2["enablePtr"]))
                    end
                    print("  Full tree          :  "..tostring(fullTree))
                    if fullTree==true  and (isCascade~=1) then
                     print("  HQOS id 0          :  "..hqosId0)
                     print("  HQOS id 1          :  "..values5.hqosIdPtr)
                    end
                    if (isRemote==1) then
                        res, values5 = myGenWrapper("prvCpssFalconTxqUtilsGetCascadePort", {
                            { "IN", "GT_U8"  , "devNum", devices[j]},
                            { "IN", "GT_U32" , "portNum",  ports[i]},
                            { "IN", "GT_BOOL"  , "remote", true},
                            { "OUT", "GT_U32" , "casCadePortNumPtr"},
                            { "OUT", "GT_U32" , "pNodeIndPtr"},
                        })
                         if(res~=0)then
                          print("Error "..res.." calling prvCpssFalconTxqUtilsGetCascadePort")
                          return
                         end
                        print("  CSCD physical port :  "..values5.casCadePortNumPtr)
                    end
                else
                    print("Could not get device "..devices[j] ..  " Port "..ports[i].." data.")
                    if(portMapShadow["portMapShadowPtr"].valid ==false) then
                        print("Port shadow is not valid")
                    else
                        print("Port shadow is valid,res is "..res)
                    end
                end
            else
                print("Device "..devices[j] ..  " is not supported since it is not SIP 6")
            end
        end
    end
end

--------------------------------------------------------------------------------
---- command registration: dump port buffers
--------------------------------------------------------------------------------
CLI_addHelp("debug", "txq-sip6", "Dump SIP6 txq debug information")

CLI_addCommand("debug", "txq-sip6 port-buffer-count", {
   func = dump_port_buffers,
   help = "Dump ingress/egress port buffers per port",
   params={
        { type="named",
        { format="device %devID_all",optional = true, name="devID",help="The device number" } ,
        { format="port %portNum",optional = true,name="portNum",help="The port number" }
        }
    }
})



local function dump_port_tc_eligState(params)
    local res1,val1,devices,j,ports,i,tc,crdt
    local numberOfQueues,firstLocalQ,tile,dp
    local enq,dry,qcn,semiElig,cnt

    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do
        if params["portNum"]==nil then ports=luaCLI_getDevInfo(devices[j])[devices[j]]  else ports={params["portNum"]} end

        for i=1,#ports do
            if(is_sip_6(devices[j]))then
                print(string.format("Device:%5d  Port:%5d ",devices[j],ports[i]))

                res, val = myGenWrapper("cpssDxChPortPhysicalPortDetailedMapGet", {
                        { "IN", "GT_U8"  , "devNum", devices[j]},
                        { "IN", "GT_U32" , "portNum",  ports[i]},
                        { "OUT", "CPSS_DXCH_DETAILED_PORT_MAP_STC", "portMapShadowPtr"},
                })

                if(res~=0)then
                 print("Error "..res.." calling cpssDxChPortPhysicalPortDetailedMapGet")
                 return
                end

                numberOfQueues = val["portMapShadowPtr"].extPortMap.txqInfo.numberOfQueues
                firstLocalQ = val["portMapShadowPtr"].extPortMap.txqInfo.secondarySchFirstQueueIndex
                tile = val["portMapShadowPtr"].extPortMap.tileId
                dp = val["portMapShadowPtr"].extPortMap.localDpInTile

                for tc=0,numberOfQueues-1 do
                    res1, val = myGenWrapper("prvCpssFalconTxqSdqEligStateGet", {
                            { "IN", "GT_U8"  , "devNum", devices[j]},
                            { "IN", "GT_U32" , "tile",  tile},
                            { "IN", "GT_U8" , "dp",  dp},
                            { "IN", "GT_U8" , "queue",  firstLocalQ+tc},
                            { "OUT", "GT_U32", "eligStatePtr"},
                    })

                    if (res1==0)  then
                        crdt = get_bit(val["eligStatePtr"],0,2)
                        if(crdt==0)then crdt="BLW_NEG"
                         elseif (crdt==1)then crdt="BLW_LOW_ABV_NEG"
                         elseif (crdt==2)then crdt="BLW_HIGH_ABV_LOW"
                         elseif (crdt==3)then crdt="ABV_HIGH"
                        end
                        enq = get_bit(val["eligStatePtr"],6,1)
                        dry = get_bit(val["eligStatePtr"],3,1)
                        qcn = get_bit(val["eligStatePtr"],2,1)
                        semiElig = get_bit(val["eligStatePtr"],7,1)

                        cnt=",SELIG "..semiElig..",  INPRT "..get_bit(val["eligStatePtr"],4,1)
                        cnt=cnt..",  INSEL "..get_bit(val["eligStatePtr"],5,1)
                        print(" TC "..tc.." - CRDT "..crdt..", ENQ "..enq..", DRY "..dry..", QCN "..qcn..cnt)
                    else
                        print("Could not get device "..devices[j] ..  " Port "..ports[i].." data")
                    end
                end
                print("")
            else
                print("Device "..devices[j] ..  " is not supported since it is not SIP 6")
            end
        end
    end
end


local function dump_port_mac_status(params)
    local res,val,devices,j,ports,i

    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do
        if params["portNum"]==nil then ports=luaCLI_getDevInfo(devices[j])[devices[j]]  else ports={params["portNum"]} end

        for i=1,#ports do
            if(is_sip_6(devices[j]))then
                res, val = myGenWrapper("prvCpssTxqSip6DebugMacStatusGet", {
                        { "IN", "GT_U8"  , "devNum", devices[j]},
                        { "IN", "GT_U32" , "portNum",  ports[i]},
                        { "OUT", "GT_U32", "adress"},
                        { "OUT", "GT_U32", "rawRegisterValue"},
                        { "OUT", "GT_U32", "rxFifoEmpty"},
                        { "OUT", "GT_U32", "txFifoEmpty"}
                })


                if (res==0)then
                    if (1==get_bit(val["rxFifoEmpty"],2,1)) then
                     --MAC support preemption
                     print(string.format("Dev:%3d Port:%5d adress:0x%08x Val:0x%08x [PMAC_TX_EMPTY :0x%x  PMAC_RX_EMPTY:0x%x] [EMAC_TX_EMPTY :0x%x  EMAC_RX_EMPTY:0x%x] ",devices[j],ports[i],val["adress"],val["rawRegisterValue"],get_bit(val["txFifoEmpty"],0,1),get_bit(val["rxFifoEmpty"],0,1),get_bit(val["txFifoEmpty"],1,1),get_bit(val["rxFifoEmpty"],1,1)))
                    else
                    --MAC do not support preemption
                     print(string.format("Dev:%3d Port:%5d adress:0x%08x Val:0x%08x [MAC_TX_EMPTY :0x%x  MAC_RX_EMPTY:0x%x]",devices[j],ports[i],val["adress"],val["rawRegisterValue"],get_bit(val["txFifoEmpty"],0,1),get_bit(val["rxFifoEmpty"],0,1)))
                    end
                else
                    print("Could not get device "..devices[j] ..  " Port "..ports[i].." data")
                end
            else
                print("Device "..devices[j] ..  " is not supported since it is not SIP 6")
            end
        end
    end
end
--------------------------------------------------------------------------------------------------------------------------
local function dump_port_link_up(params)
    local res,res1,val,devices,j,ports,i,macLinkUp

    if (params["devID"]=="all") then devices=wrLogWrapper("wrlDevList") else devices={params["devID"]} end

    for j=1,#devices do
        if params["portNum"]==nil then ports=luaCLI_getDevInfo(devices[j])[devices[j]]  else ports={params["portNum"]} end

        for i=1,#ports do
            if(is_sip_6(devices[j]))then

                res, val = myGenWrapper("cpssDxChPortLinkStatusGet", {
                        { "IN", "GT_U8"  , "devNum", devices[j]},
                        { "IN", "GT_U32" , "portNum",  ports[i]},
                        { "OUT", "GT_BOOL", "isLinkUp"},
                })

                macLinkUp=val["isLinkUp"]

                res1, val = myGenWrapper("cpssDxChBrgEgrFltPortLinkEnableGet", {
                        { "IN", "GT_U8"  , "devNum", devices[j]},
                        { "IN", "GT_U32" , "portNum",  ports[i]},
                        { "OUT", "CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT", "portLinkStatusState"},
                })


                if (res==0 and res1==0)then
                    print(string.format("Dev:%3d Port:%5d EGF Link state:",devices[j],ports[i])..string.sub(val["portLinkStatusState"],46,-3).." Mac link up : "..tostring(macLinkUp))
                else
                    print("Could not get device "..devices[j] ..  " Port "..ports[i].." data")
                end
            else
                print("Device "..devices[j] ..  " is not supported since it is not SIP 6")
            end
        end
    end
end

local function pause_queue(params)

    local device = tonumber(params["devID"])
    local port =  tonumber(params["portNum"])
    local queue =  tonumber(params["queueNum"])
    local res,enable

    if params["pause"]==true then
     enable=false
    else
     enable=true
    end

    if(is_sip_6(device))then
      res = myGenWrapper("cpssDxChPortTxQueueTxEnableSet", {
                { "IN", "GT_U8"  , "devNum", device},
                { "IN", "GT_U32" , "portNum",  port},
                { "IN", "GT_U32" , "tcQueue",  queue},
                { "IN", "GT_BOOL" , "enable",  enable},
        })
     if(res~=0)then
      print("Error "..res.." calling cpssDxChPortTxQueueTxEnableSet")
      return
     end

    else
        print("Device "..device ..  " is not supported since it is not SIP 6")
    end
end



local function show_dequeue(params)

    local device = tonumber(params["devID"])
    local port =  tonumber(params["portNum"])

    local res,maxQ

    res, val = myGenWrapper("prvCpssSip6TxqUtilsMaxQueueOffsetGet", {
                            { "IN", "GT_U8"  , "devNum", device},
                            { "IN", "GT_U32" , "portNum",  port},
                            { "OUT", "GT_U32", "maxQ"},
                    })
    if res ~=0 then
     print("Fail in prvCpssSip6TxqUtilsMaxQueueOffsetGet")
    return
    end

    maxQ = val.maxQ


     if(is_sip_6(device))then
      for tc=0,maxQ do
          res,val = myGenWrapper("cpssDxChPortTxQueueTxEnableGet", {
                    { "IN", "GT_U8"  , "devNum", device},
                    { "IN", "GT_U32" , "portNum",  port},
                    { "IN", "GT_U32" , "tcQueue",  tc},
                    { "OUT", "GT_BOOL" , "enablePtr"},
            })
         if(res~=0)then
          print("Error "..res.." calling cpssDxChPortTxQueueTxEnableSet")
          return
         end

         print("Queue["..tc.."] deque enable "..tostring(val.enablePtr))
     end

    else
        print("Device "..device ..  " is not supported since it is not SIP 6")
    end
end



local function show_pizza(params)

    local device = tonumber(params["devID"])
    local tile = tonumber(params["tile"])
    local res,val,iterations
    local pNodes = {}
    local pNodesSize = 64

    if(is_sip_6_30(device))then
      for pNodeIterator=0,pNodesSize do
        pNodes[pNodeIterator] = 0
      end

      res,val = myGenWrapper("prvCpssSip6_30TxqPdqPortPizzaLastSliceGet", {
                    { "IN", "GT_U8"  , "devNum", device},
                    { "IN", "GT_U32"  , "tileNum", tile},
                    { "OUT", "GT_U32" , "lastSlice"}
            })
     if(res~=0)then
      print("Error "..res.." calling prvCpssSip6_30TxqPdqPortPizzaLastSliceGet")
      return
     end
     print("Last slice is "..val.lastSlice)
     iterations = (val.lastSlice+1)/4
     print("Reading "..iterations.." lines.....")

     for line=0,iterations-1 do
          res,val = myGenWrapper("prvCpssSip6_30TxqPdqPizzaSliceLineGet", {
                    { "IN", "GT_U8"  , "devNum", device},
                    { "IN", "GT_U32"  , "tileNum", tile},
                    { "IN", "GT_U32" , "lineInd",  line},
                    { "OUT", "GT_U32" , "data"},
            })
          if(res~=0)then
           print("Error "..res.." calling prvCpssSip6_30TxqPdqPizzaSliceLineGet")
          return
          end
         print("+---------------------------------------------------------------------------------------------+")
         print(string.format("[%3d] 0x%08x|Slice:%3d Data:%3d |Slice:%3d Data:%3d|Slice:%3d Data:%3d|Slice:%3d Data:%3d |",line,val.data,line*4,get_bit(val["data"],0,6),line*4+1,get_bit(val["data"],6,6),line*4+2,get_bit(val["data"],12,6),line*4+3,get_bit(val["data"],18,6)))

         pNodes[get_bit(val["data"],0,6)] = pNodes[get_bit(val["data"],0,6)]+1
         pNodes[get_bit(val["data"],6,6)] = pNodes[get_bit(val["data"],6,6)]+1
         pNodes[get_bit(val["data"],12,6)] = pNodes[get_bit(val["data"],12,6)]+1
         pNodes[get_bit(val["data"],18,6)] = pNodes[get_bit(val["data"],18,6)]+1
     end

     print("+---------------------------------------------------------------------------------------------+")

     for pNodeIterator=0,#pNodes do
        if pNodes[pNodeIterator]~= 0 then
         print("Pnode "..pNodeIterator.." has "..pNodes[pNodeIterator].." slices")
        end
     end

    else
        print("Device "..device ..  " is not supported since it is not SIP 6_30")
    end
end


local function dp_map(params)

    local device = tonumber(params["devID"])
    local tile = tonumber(params["tile"])
    local gDp = tonumber(params["globalDpIndex"])
    local res,val

    if(is_sip_7(device))then
      res,val = myGenWrapper("prvCpssDxChTxqSip7DebugDpMappingGet", {
                    { "IN", "GT_U8"  , "devNum", device},
                    { "IN", "GT_U32"  , "tileNum", tile},
                    { "IN", "GT_U32"  , "globalDp", gDp},
                    { "OUT", "GT_U32" , "tilePtr"},
                    { "OUT", "GT_U32" , "localDpPtr"}
            })
     if(res~=0)then
      print("Error "..res.." calling prvCpssDxChTxqSip7DebugDpMappingGet")
      return
     end
     print("PDX-tile     :"..val.tilePtr)
     print("PDX-local dp :"..val.localDpPtr)
     --check if we can get PDS id
     res,val = myGenWrapper("prvCpssSip7TxqPdsIdGet", {
                    { "IN", "GT_U8"  , "devNum", device},
                    { "IN", "GT_U32"  , "tileNum", val.tilePtr},
                    { "IN", "GT_U32"  , "pdsNum", val.localDpPtr},
                    { "OUT", "GT_U32" , "tilePtr"},
                    { "OUT", "GT_U32" , "idPtr"}
            })
     if(res~=0)then
      print("PDS-id         : NA [error ="..res.."]")
     else
     print("PDS-id       :"..val.idPtr)
     end

    else
        print("Device "..device ..  " is not supported since it is not SIP 7")
    end
end

local function show_td(params)

    local device = tonumber(params["devID"])
    local port =  tonumber(params["portNum"])

    local res,maxQ

    res, val = myGenWrapper("cpssDxChPortTxBindPortToDpGet", {
                             { "IN", "GT_U8"  , "devNum", device},
                             { "IN", "GT_U32" , "portNum",  port},
                             { "OUT", "CPSS_PORT_TX_DROP_PROFILE_SET_ENT" , "profileSetPtr"}
                            })
    if res ~= 0 then
         print("Fail in cpssDxChPortTxBindPortToDpGet")
        return
    end

    print("Port TD profile  "..tostring(val.profileSetPtr).."\n")

     if(is_sip_7(device))then
      res, val = myGenWrapper("prvCpssSip6TxqUtilsMaxQueueOffsetGet", {
                            { "IN", "GT_U8"  , "devNum", device},
                            { "IN", "GT_U32" , "portNum",  port},
                            { "OUT", "GT_U32", "maxQ"},
                    })
        if res ~=0 then
         print("Fail in prvCpssSip6TxqUtilsMaxQueueOffsetGet")
        return
      end

      maxQ = val.maxQ
      for tc=0,maxQ do
          res,val = myGenWrapper("prvCpssSip7TxqUtilsTailDropBindQueueToProfileGet", {
                    { "IN", "GT_U8"  , "devNum", device},
                    { "IN", "GT_U32" , "portNum",  port},
                    { "IN", "GT_U32" , "tcQueue",  tc},
                    { "OUT", "CPSS_PORT_TX_DROP_PROFILE_SET_ENT" , "profileSetPtr"},
            })
         if(res~=0)then
          print("Error "..res.." calling prvCpssSip7TxqUtilsTailDropBindQueueToProfileGet")
          return
         end

         print("Queue["..tc.."] TD profile  "..tostring(val.profileSetPtr))
     end
    end
end
--------------------------------------------------------------------------------
---- command registration: dump port mapping
--------------------------------------------------------------------------------
CLI_addCommand("debug", "txq-sip6 port-mapping", {
   func = dump_port_mapping,
    help = "Dump port mapping",
    params={
        { type="named",
        { format="device %devID_all",optional = true, name="devID",help="The device number" } ,
        { format="port %portNum",optional = true,name="portNum",help="The port number" }
        }
    }
})


CLI_addCommand("debug", "txq-sip6 port-tc-buffer-count", {
   func = dump_port_tc_buffers,
   help = "Dump ingress/egress port buffers per port/tc",
   params={
        { type="named",
        { format="device %devID_all",optional = true, name="devID",help="The device number" } ,
        { format="port %portNum",optional = true,name="portNum",help="The port number" }
        }
    }
})

CLI_addCommand("debug", "txq-sip6 port-tc-elig-state", {
   func = dump_port_tc_eligState,
   help = "Dump egress port/tc eligible state",
   params={
        { type="named",
        { format="device %devID_all",optional = true, name="devID",help="The device number" } ,
        { format="port %portNum",optional = true,name="portNum",help="The port number" }
        }
    }
})

CLI_addCommand("debug", "txq-sip6 port-pds-profile", {
   func = dump_port_pds_params,
   help = "Dump port PDS profile",
   params={
        { type="named",
        { format="device %devID_all",optional = true, name="devID",help="The device number" } ,
        { format="port %portNum",optional = true,name="portNum",help="The port number" }
        }
    }
})


CLI_addCommand("debug", "txq-sip6 port-length-adjust-profile", {
   func = dump_port_length_adjust_params,
   help = "Dump port length-adjust profile",
   params={
        { type="named",
        { format="device %devID_all",optional = true, name="devID",help="The device number" } ,
        { format="port %portNum",optional = true,name="portNum",help="The port number" }
        }
    }
})

CLI_addCommand("debug", "txq-sip6 port-mac-fifo-status", {
   func = dump_port_mac_status,
   help = "Dump port MAC status",
   params={
        { type="named",
        { format="device %devID_all",optional = true, name="devID",help="The device number" } ,
        { format="port %portNum",optional = true,name="portNum",help="The port number" }
        }
    }
})

CLI_addCommand("debug", "txq-sip6 port-link-up-state", {
   func = dump_port_link_up,
   help = "Dump port link-up",
   params={
        { type="named",
        { format="device %devID_all",optional = true, name="devID",help="The device number" } ,
        { format="port %portNum",optional = true,name="portNum",help="The port number" }
        }
    }
})
CLI_addCommand("debug", "txq-sip6 pause-queue", {
   func = pause_queue,
   help = "Stop dequeue from specific queue",
   params={
        { type="named",
        { format="device %devID", name="devID",help="The device number" } ,
        { format="port %portNum",name="portNum",help="The port number" },
        { format="queue %queueNum",name="queueNum",help="The queue number" },
        { format="pause %bool", name="pause",help="Stop dequeue"},
          mandatory={"devID","queueNum","portNum","pause"},
        }
    }
})

CLI_addCommand("debug", "txq-sip6 port-show-dequeue", {
   func = show_dequeue,
   help = "Show dequeue state of port",
   params={
        { type="named",
        { format="device %devID", name="devID",help="The device number" } ,
        { format="port %portNum",name="portNum",help="The port number" },
          mandatory={"devID","portNum"},
        }
    }
})

CLI_addCommand("debug", "txq-sip6 pizza-dump", {
   func = show_pizza,
   help = "Show txq pizza(SIP6_30 only)",
   params={
        { type="named",
        { format="device %devID", name="devID",help="The device number" } ,
        { format="tile %tile", name="tile",help="The tile number" } ,
          mandatory={"devID","tile"},
        }
    }
})
CLI_addCommand("debug", "txq-sip7 dp-map", {
   func = dp_map,
   help = "Show global to local DP mapping at PDX and PDS",
   params={
        { type="named",
        { format="device %devID", name="devID",help="The device number" } ,
        { format="tile %tile", name="tile",help="The tile number" } ,
        { format="globalDpIndex %gDp", name="globalDpIndex",help="Global DP index" } ,
          mandatory={"devID","tile"},
        }
    }
})


CLI_addCommand("debug", "txq-sip6 port-show-td", {
   func = show_td,
   help = "Show tail drop binding of the port",
   params={
        { type="named",
        { format="device %devID", name="devID",help="The device number" } ,
        { format="port %portNum",name="portNum",help="The port number" },
          mandatory={"devID","portNum"},
        }
    }
})
