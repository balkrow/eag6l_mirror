--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* shared-lib.lua
--*
--* DESCRIPTION:
--*       Basic debug menu for shared-library
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************
--dofile("dxCh/debug/txq-sip6.lua")


local function map_pp(params)
  local res,val
  res, val = myGenWrapper("extDrvReInitDrv",{})
  if(res~=0)then
      print("Error "..res.." calling extDrvReInitDrv")
      return
  end

  print("Finished ok")
end


local function shlib_show_info(params)

  local res,val
  res, val  = myGenWrapper("prvOsHelperGlobalDbInfoGet",{
        {"OUT","GT_U32","myPidPtr"},
        {"OUT","GT_U32","clientNumPtr"},
        {"OUT","GT_BOOL","globalDbToSharedMemPtr"},
        {"OUT","GT_BOOL","ppMappingStageDonePtr"},
        {"OUT","GT_U32","initializerPidPtr"},
  })
  if(res~=0)then
      print("Error "..res.." calling prvOsHelperGlobalDbInfoGet")
      return

  else
   print("")
   print("My PID....................."..val.myPidPtr)
   print("Number of clients.........."..val.clientNumPtr)
   print("Global DB as shared memory."..tostring(val.globalDbToSharedMemPtr))
   print("PP mapping done............"..tostring(val.ppMappingStageDonePtr))
   print("Master process PID........."..val.initializerPidPtr)
  end

end

local function shlib_show_adress(params)

  local res,val,j
  local pid,filename
  local file,err,line
  local devices=wrLogWrapper("wrlDevList")

    res, val  = myGenWrapper("prvOsHelperGlobalDbInfoGet",{
        {"OUT","GT_U32","myPidPtr"},
        {"OUT","GT_U32","clientNumPtr"},
        {"OUT","GT_BOOL","globalDbToSharedMemPtr"},
        {"OUT","GT_BOOL","ppMappingStageDonePtr"},
        {"OUT","GT_U32","initializerPidPtr"},
  })
  if(res~=0)then
      print("Error "..res.." calling prvOsHelperGlobalDbInfoGet")
      return
  end

  pid = val.myPidPtr
  filename = "/proc/"..pid.."/maps"

  print("Check mapping from "..filename)
  print()
  -- Open a file for read
    file,err = io.open(filename)
    if err then print("Failed to read "..filename); return; end

 -- line by line

  while true do
    line = file:read()
    if line == nil then break end
        if string.find(line,"libcpss.so") or string.find(line,"libhelper.so")
            or string.find(line,"SHM_DATA") or string.find(line,"CPSS_SHM_MALLOC")
            or string.find(line,"mvDmaDrv") or string.find(line,"pci")  or string.find(line,"mvMbusDrv")
            or string.find(line,"mvdma") then
            print (line)
        end
  end
  file:close()

  for j=1,#devices do
   print("Device ["..devices[j].."] head of DMA mapping :")
   print("==========================")
   res, val  = myGenWrapper("osGlobalDbDmaBlocksHeadGet",{
        {"IN","GT_U8","devNum",devices[j]},
        {"OUT","GT_U32","dmaVirtualAddrLowPtr"},
        {"OUT","GT_U32","dmaVirtualAddrHighPtr"},
        {"OUT","GT_U32","dmaPhysAddrLowPtr"},
        {"OUT","GT_U32","dmaLengthPtr"},
   })

   if(res~=0)then
      print("Error "..res.." calling osGlobalDbDmaWindowGet")
      return
   else
     print("virtual adress(low).....0x"..string.format("%08x",val.dmaVirtualAddrLowPtr))
     print("virtual adress(high)....0x"..string.format("%08x",val.dmaVirtualAddrHighPtr))
     print("length.................."..val.dmaLengthPtr)
     print("DMA adress(per device)..0x"..string.format("%08x",val.dmaPhysAddrLowPtr))
     print("")
   end
  end


  res, val  = myGenWrapper("prvOsHelperGlobalDbAdressInfoGet",{
        {"OUT","GT_U32","dmaVirtualAddrLowPtr"},
        {"OUT","GT_U32","dmaVirtualAddrHighPtr"},
        {"OUT","GT_U32","dmaPhysAddrLowPtr"},
        {"OUT","GT_U32","dmaPhysAddrHighPtr"},
        {"OUT","GT_U32","globalSharedDbAddrLowPtr"},
        {"OUT","GT_U32","globalSharedDbAddrHiPtr"},
  })
  if(res~=0)then
      print("Error "..res.." calling prvOsHelperGlobalDbAdressInfoGet")
      return

  else

   print("Global shared DB(high)......0x"..string.format("%08x",val.globalSharedDbAddrHiPtr))
   print("Global shared DB(low).......0x"..string.format("%08x",val.globalSharedDbAddrLowPtr))
  end

  devices=wrLogWrapper("wrlDevList")

  for j=1,#devices do
   print("\nDevice "..devices[j])
   print("============")

   res, val  = myGenWrapper("prvCpssGlobalDbHwInfoGet",{
        {"IN","GT_U8","device",devices[j]},
        {"OUT","GT_U32","busNoPtr"},
        {"OUT","GT_U32","devSelPtr"},
        {"OUT","GT_U32","funcNoPtr"}
   })

   if(res~=0)then
      print("Error "..res.." calling prvCpssGlobalDbHwInfoGet")
      return

   else
   print("busNo...........0x"..string.format("%08x",val.busNoPtr))
   print("devSelPtr.......0x"..string.format("%08x",val.devSelPtr))
   print("funcNoPtr.......0x"..string.format("%08x",val.funcNoPtr))
   end

   res, val  = myGenWrapper("prvCpssGlobalDbCnmVirtualAdressGet",{
        {"IN","GT_U8","device",devices[j]},
        {"OUT","GT_U32","cnmVirtualAddrLowPtr"},
        {"OUT","GT_U32","cnmVirtualAddrHighPtr"}
   })
   if(res~=0)then
      print("Error "..res.." calling prvCpssGlobalDbCnmVirtualAdressGet")
      return

   else


   print("CNM virtual adress(low)........0x"..string.format("%08x",val.cnmVirtualAddrLowPtr))
   print("CNM virtual adress(high).......0x"..string.format("%08x",val.cnmVirtualAddrHighPtr))
   end

   res, val  = myGenWrapper("prvCpssGlobalDbSwitchVirtualAdressGet",{
        {"IN","GT_U8","device",devices[j]},
        {"OUT","GT_U32","switchVirtualAddrLowPtr"},
        {"OUT","GT_U32","switchVirtualAddrHighPtr"}
   })
   if(res~=0)then
      print("Error "..res.." calling prvCpssGlobalDbSwitchVirtualAdressGet")
      return

   else

   print("Switch virtual adress(low).....0x"..string.format("%08x",val.switchVirtualAddrLowPtr))
   print("Switch virtual adress(high)....0x"..string.format("%08x",val.switchVirtualAddrHighPtr))
   end

   res, val  = myGenWrapper("prvCpssGlobalDbDfxVirtualAdressGet",{
        {"IN","GT_U8","device",devices[j]},
        {"OUT","GT_U32","dfxVirtualAddrLowPtr"},
        {"OUT","GT_U32","dfxVirtualAddrHighPtr"}
   })
   if(res~=0)then
      print("Error "..res.." calling prvCpssGlobalDbDfxVirtualAdressGet")
      return

   else

   print("DFX virtual adress(low)........0x"..string.format("%08x",val.dfxVirtualAddrLowPtr))
   print("DFX virtual adress(high).......0x"..string.format("%08x",val.dfxVirtualAddrHighPtr))
   end
  end --end of for


end

--------------------------------------------------------------------------------
---- command registration: shared-lib  map-pcie
--------------------------------------------------------------------------------


CLI_addCommand("debug", "shared-lib  pp-map", {
   func = map_pp,
   help = "Map PP to process memory space "
})


CLI_addHelp("debug", "shared-lib  show-info", "Show debug information")
--------------------------------------------------------------------------------
---- command registration: shared-lib  show-info
--------------------------------------------------------------------------------


CLI_addCommand("debug", "shared-lib  show-info", {
   func = shlib_show_info,
   help = "Show debug information "
})

CLI_addHelp("debug", "shared-lib  show-adress-map", "Show adresses used by shared lib")

--------------------------------------------------------------------------------
---- command registration: shared-lib  show-adress-map
--------------------------------------------------------------------------------


CLI_addCommand("debug", "shared-lib  show-adress-map", {
   func = shlib_show_adress,
   help = "Show adresses used by shared lib"
})


