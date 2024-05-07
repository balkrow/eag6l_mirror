--********************************************************************************
--*              (c), Copyright 2010, Marvell International Ltd.                 *
--* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
--* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
--* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
--* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
--* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
--* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
--********************************************************************************
--* shared_library_sanity.lua
--*
--* DESCRIPTION:
--*       test shared library clients
--*
--*       purpose of example:
--*       1. Initialize all clients in background
--*       2. Verify number of active clients as expected.
--*       3. Kill all the clients.
--*       4. Verify number of active clients is 1.
--*
--*
--* FILE REVISION NUMBER:
--*       $Revision: 1 $
--*
--********************************************************************************

local res,val
local expectedClientsNum = 1
local runTest=false
local checkRx=false
local checkFdb=false
local flags
local execStr
local rxtxDump="rxTxOut.txt"
local fdbDump="fdbOut.txt"
local flush_fdb = "dxCh/examples/configurations/flush_fdb.txt"

local devNum  = devEnv.dev
local port1   = devEnv.port[1]
local port2   = devEnv.port[2]
local port3   = devEnv.port[3]
local port4   = devEnv.port[4]


-- this test is relevant to ALL tested devices
--##################################
--##################################

local delimiter = "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"

--generate test packet
local packetPayload = "6666"..-- dummy ethertype
        "000102030405060708090a0b0c0d0e0f"..
        "101112131415161718191a1b1c1d1e1f"..
        "202122232425262728292a2b2c2d2e2f"..
        "303132333435363738393a3b3c3d3e3f"

local macDa   = "000000000058"
local macSa   = "000000001111"

printLog("\n=>Expected clients num "..expectedClientsNum)
local function buildPacketUc(partAfterMac)
    if not partAfterMac then
        partAfterMac = ""
    end

    return macDa .. macSa ..  partAfterMac .. packetPayload
end

--##################################
--##################################
local ingressPacket = buildPacketUc(nil)
local transmitInfo = {portNum = port2 , pktInfo = {fullPacket = ingressPacket} }
local egressInfoTable = {
    -- expected to egress unmodified
    {portNum = port1  , pktInfo = {fullPacket = ingressPacket}},
    {portNum = port3  , pktInfo = {fullPacket = ingressPacket}},
    {portNum = port4  , pktInfo = {fullPacket = ingressPacket}}
}


local applications = {"luaCLI","fdbLearning","RxTxProcess"}

function file_exists(name)
   local f=io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
end

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

if val.globalDbToSharedMemPtr==false then
setTestStateSkipped()
return
end

local ldPath=os.getenv("LD_LIBRARY_PATH")
printLog("\n =>Path is  "..ldPath)

if  val.clientNumPtr~=1 then
printLog("\n=>Actual clients num is not 1 .Fail the test.")
setFailState()
end

 --start the programs
for i, application in ipairs(applications) do
 if file_exists(ldPath.."/"..application)==false then
    printLog("\n =>Application "..application.." not found in directory "..ldPath)
 else
    printLog("\n =>Application "..application.." found.")
     --start the program in background
     expectedClientsNum= expectedClientsNum+1
     if application=="RxTxProcess" then
        runTest=true
        checkRx=true
        flags="showRx dumpToFile -verbose"
        if file_exists("./"..rxtxDump)==true then
         printLog("\n =>remove ./"..rxtxDump)
         os.execute("rm ./"..rxtxDump)
        end
     elseif application=="fdbLearning" then
        runTest=true
        checkFdb=true
        flags="showLog dumpToFile -verbose"
        if file_exists("./"..fdbDump)==true then
         printLog("\n =>remove ./"..fdbDump)
         os.execute("rm ./"..fdbDump)
        end
     else
        flags=""
     end
     execStr=ldPath.."/"..application.." "..flags.." &"
     os.execute(execStr)
     printLog(execStr)
 end --if file_exists
end--for

--lets sleep
printLog("sleep 5 sec")
os.execute("sleep 5")
printLog("\n=>Expected clients num "..expectedClientsNum)

res, val  = myGenWrapper("prvOsHelperGlobalDbInfoGet",{
        {"OUT","GT_U32","myPidPtr"},
        {"OUT","GT_U32","clientNumPtr"},
        {"OUT","GT_BOOL","globalDbToSharedMemPtr"},
        {"OUT","GT_BOOL","ppMappingStageDonePtr"},
        {"OUT","GT_U32","initializerPidPtr"},
  })
if(res~=0)then
  printLog("Error "..res.." calling prvOsHelperGlobalDbInfoGet")
  return
end

printLog("\n=>Actual clients num "..val.clientNumPtr)

if runTest== true then
    executeLocalConfig(flush_fdb)
    printLog("\n=>Run  luaTgfTransmitPacketsWithExpectedEgressInfo")
    luaTgfTransmitPacketsWithExpectedEgressInfo(transmitInfo,egressInfoTable)
end


if  expectedClientsNum~=val.clientNumPtr then
setFailState()
else
-- luaTgfTransmitPacketsWithExpectedEgressInfo will fail the test since paket is captured by other process
setPassState()
end

 --kill the programs using SIGINT
for i, application in ipairs(applications) do
 if file_exists(ldPath.."/"..application)==true then
 printLog("\n=>Kill application "..application)
 os.execute("ps -ef | grep '"..application.."' | grep -v grep |grep -o '[^ ]*' |head -1")
 os.execute("ps -ef | grep '"..application.."' | grep -v grep |grep -o '[^ ]*' |head -1|xargs -r kill -2")
 end
end


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

printLog("=>Actual clients num after kill "..val.clientNumPtr)

if  val.clientNumPtr~=1 then
printLog("Actual clients num is not 1 .Fail the test.")
setFailState()
end

if checkRx==true then
 if file_exists("./"..rxtxDump)==false then
  printLog("Traffic to CPU was not captured by rxTxProcess .Fail the test.")
  setFailState()
 else
  printLog("Dump traffic to CPU  captured by rxTxProcess :")
  os.execute("cat ./"..rxtxDump)
  os.execute("rm ./"..rxtxDump)
  end
end
if checkFdb==true then
 if file_exists("./"..fdbDump)==false then
  printLog("New adress was not learned  by fdbLearning .Fail the test.")
  setFailState()
 else
  printLog("Dump new adress learned by fdbLearning")
  os.execute("cat ./"..fdbDump)
  os.execute("rm ./"..fdbDump)
 end
end


local function callWithErrorHandling(cpssApi, params)
   local rc, value = myGenWrapper(cpssApi, params,1)
   if rc ~= 0 then
       setFailState()
       printLog("ERROR of calling "..cpssApi..": "..returnCodes[rc])
   end
   return rc, value
end

--reinit since we registered events for events at fdbLearning and RxTxProcess
--appDemo need those events


printLog("Remove device and reset system\n ")
callWithErrorHandling("cpssResetSystem",{
            {"IN","GT_BOOL","doHwReset",1}
        })
printLog("Call cpssReInitSystem\n ")
callWithErrorHandling("cpssReInitSystem",{})

