/*******************************************************************************
Copyright (C) 2014 - 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains sample functions code for the Marvell 88X32X0, 
88X33X0, 88X35X0, CUX36X0, CUE26X0, 88E20X0 and 88E21X0 ethernet PHYs.
********************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "mtdFeatures.h"
#include "mtdApiTypes.h"
#include "mtdHwCntl.h"
#include "mtdAPI.h"
#include "mtdFwDownload.h"
#include "mtdInitialization.h"
#include "mtdDiagnostics.h"
#include "mtdCunit.h"
#include "mtdHunit.h"
#include "mtdHXunit.h"
#include "mtdXunit.h"
#include "mtdApiRegs.h"
#include "mtdSample.h"
#include "mtdIntr.h"
#include "mtdHwSerdesCntl.h"

#if MTD_PKG_CE_SERDES
#include "mcesdTop.h"

#if MTD_CE_SERDES28X2
#include "mcesdC28GP4X2_Defs.h"
#include "mcesdC28GP4X2_API.h"
#endif  /* MTD_CE_SERDES28X2 */

#if MTD_CE_SERDES32X1X4
#include "mcesdN5C32GP5X4_Defs.h"
#include "mcesdN5C32GP5X4_API.h"
#endif  /* MTD_CE_SERDES32X1X4 */

#endif  /* MTD_PKG_CE_SERDES */

/* Forward declarations */
MTD_STATIC MTD_STATUS mtdSampleWait(MTD_DEV_PTR dev, MTD_UINT milliSeconds);
MTD_STATIC MTD_STATUS mtdSampleCl45MdioRead(MTD_DEV* dev, MTD_U16 port, MTD_U16 mmd, MTD_U16 reg, MTD_U16* value);
MTD_STATIC MTD_STATUS mtdSampleCl45MdioWrite(MTD_DEV* dev, MTD_U16 port, MTD_U16 mmd, MTD_U16 reg, MTD_U16 value);
MTD_STATIC MTD_STATUS mtdSampleCl22MdioRead(MTD_DEV* dev, MTD_U16 port, MTD_U16 reg, MTD_U16* value);
MTD_STATIC MTD_STATUS mtdSampleCl22MdioWrite(MTD_DEV* dev, MTD_U16 port, MTD_U16 reg, MTD_U16 value);

/******************************************************************************
MTD_STATUS mtdSampleInitDrv
(
    INOUT MTD_DEV_PTR devPtr,
    IN MTD_VOID *pHostContext  
)

 Inputs:
    devPtr - allocated memory for the device structure
    pHostContext - optional host context

 Outputs:
    devPtr - returns an initialized device structure

 Returns:
    Returns MTD_OK or MTD_FAIL if driver initialization fails
    
 Description:
    Sample function to show the calling of the mtdLoadDriver driver initialization
    and passing in the parameters. The valid read/write MDIO function and wait function
    pointers must be passed in or the mtdHwXmdioRead/mtdHwXmdioWrite and mtdWait
    require changes to provide this functionality.

 Side effects:
    None

 Notes/Warnings:
    The device pointer devPtr must be allocated with the appropriate memory size and
    must not be freed until mtdUnloadDriver() is called or the API is no longer
    going to be accessed.

    The host context pHostContext is an optional data that is provided for the host to 
    keep track of any host necessary info that will be passed from the upper
    layer of the API to the lowest layer. It could be used, for example, to pass
    a pointer to a structure containing a semaphore id, or an MDIO bus id, etc.

*******************************************************************************/
MTD_STATUS mtdSampleInitDrv
(
    INOUT MTD_DEV_PTR devPtr,
    IN MTD_VOID *pHostContext  /* optional host context */
)
{
    MTD_STATUS status = MTD_FAIL;
    MTD_U16 errCode;

    /* if readMdio and writeMdio function pointers are NULL, the mtdHwXmdioRead  
       and mtdHwXmdioWrite in mtdHwCntl.c must handle the MDIO read/write */
#if (MTD_CLAUSE_22_MDIO == 0)       
    FMTD_READ_MDIO readMdio = mtdSampleCl45MdioRead;        /* pointer to host's function to do MDIO read */
    FMTD_WRITE_MDIO writeMdio = mtdSampleCl45MdioWrite;     /* pointer to host's function to do MDIO write */
#else
    FMTD_READ_MDIO readMdio = mtdSampleCl22MdioRead;        /* pointer to host's function to do MDIO read */
    FMTD_WRITE_MDIO writeMdio = mtdSampleCl22MdioWrite;     /* pointer to host's function to do MDIO write */
#endif
    FMTD_WAIT_FUNC waitFunc = mtdSampleWait;                /* pointer to host's function for wait */

    MTD_U16 anyPort = 0; /* port address of any MDIO port for this device */

    if (!devPtr)
    {
        MTD_DBG_INFO("mtdSampleInitDrv: Error - NULL devPtr pointer\n");
        return status;
    }

    devPtr->appData = pHostContext; /* to allow host to pass some data all the way through call stack if desired */
    
    devPtr->devEnabled = MTD_FALSE; /* will be set to MTD_TRUE by mtdLoadDriver() if successful */

    status = mtdLoadDriver(readMdio, writeMdio, waitFunc, anyPort, MTD_FALSE, devPtr, &errCode);

    return status;
}


#define MTD_SAMPLE_MAX_LOGICAL_PORTS 69 /* assumes logical ports are numbers 1...68, there is no logical port 0 */

MTD_DEV mtdDevStructures[MTD_SAMPLE_MAX_LOGICAL_PORTS]; /* assumes first entry not used */

typedef struct
{
    int mdioBus; /* MDIO Bus Number/Id to Host */
    int mdioPortAddress; /* MDIO Address To Port */
} MTD_MDIO_BUS_MAP_STRUCT;

/******************************************************************************
  Diagram for the MDIO Address and Logical Port mapping for the sample function 
  mtdSampleSystemInitDrv():

      MDIO Bus #1              PHY Port            Logical Port
       to Host               MDIO Address   (Front Panel/CLI Marking)
         |
         |         |-------|<------0------>             1
         |<------->| X3240 |<------1------>             3
         |         |       |<------2------>             2
         |         |-------|<------3------>             4
         |
         |         |-------|<------4------>             5
         |<------->| X3240 |<------5------>             7
         |         |       |<------6------>             6
         |         |-------|<------7------>             8
         |
         |            ...         ...
         |
         |         |-------|<-----28------>            29
         |<------->| X3240 |<-----29------>            31
                   |       |<-----30------>            30
                   |-------|<-----31------>            32

      MDIO Bus #2              PHY Port     
       to Host               MDIO Address   
         |     
         |         |-------|<------0------>            33
         |<------->| X3240 |<------1------>            35
         |         |       |<------2------>            34
         |         |-------|<------3------>            36
         |
         |         |-------|<------4------>            37
         |<------->| X3240 |<------5------>            39
         |         |       |<------6------>            38
         |         |-------|<------7------>            40
         |
         |            ...         ...
         |
         |         |-------|<------28----->            61
         |<------->| X3240 |<------29----->            63
                   |       |<------30----->            62
                   |-------|<------31----->            64

      MDIO Bus #3              PHY Port     
       to Host               MDIO Address   
         |    
         |         |-------|<------0------>            65
         |<------->| X3220 |
         |         |       |<------1------>            66
         |         |-------|
         |
         |         |-------|<------2------>            67
         |<------->| X3220 |
                   |       |<------3------>            68
                   |-------|

*******************************************************************************/
/* Indexed by Logical Port Number */
MTD_MDIO_BUS_MAP_STRUCT mtdMDIOPortMap[MTD_SAMPLE_MAX_LOGICAL_PORTS] =
{
    {0,0}, /* logical port 0 not used */
    {1,0}, /* logical port 1 is on bus 1 mdio 0 */ /* first X3240 */
    {1,2}, /* logical port 2 is on bus 1 mdio 2 */
    {1,1}, /* logical port 3 is on bus 1 mdio 1 */
    {1,3}, /* logical port 4 is on bus 1 mdio 3 */
    {1,4}, /* logical port 5 is on bus 1 mdio 4 */ /* second X3240 */
    {1,6}, /* logical port 6 is on bus 1 mdio 6 */
    {1,5}, /* logical port 7 is on bus 1 mdio 5 */
    {1,7}, /* logical port 8 is on bus 1 mdio 7 */
        
/* and so on, for brevity middle ones excluded  */ 
    
    {1,28}, /* logical port 29 is on bus 1 mdio 28 */ /* last X3240 */
    {1,30}, /* logical port 30 is on bus 1 mdio 30 */
    {1,29}, /* logical port 31 is on bus 1 mdio 29 */
    {1,31}, /* logical port 32 is on bus 1 mdio 31 */
        
    {2,0}, /* logical port 33 is on bus 2 mdio 0 */ /* first X3240 */
    {2,2}, /* logical port 34 is on bus 2 mdio 2 */
    {2,1}, /* logical port 35 is on bus 2 mdio 1 */
    {2,3}, /* logical port 36 is on bus 2 mdio 3 */
    {2,4}, /* logical port 37 is on bus 2 mdio 4 */ /* second X3240 */
    {2,6}, /* logical port 38 is on bus 2 mdio 6 */
    {2,5}, /* logical port 39 is on bus 2 mdio 5 */
    {2,7}, /* logical port 40 is on bus 2 mdio 7 */
        
/* and so on, for brevity middles ones excluded  */ 
    
    {2,28}, /* logical port 61 is on bus 2 mdio 28 */ /* last X3240 */
    {2,30}, /* logical port 62 is on bus 2 mdio 30 */
    {2,29}, /* logical port 63 is on bus 2 mdio 29 */
    {2,31}, /* logical port 64 is on bus 2 mdio 31 */
        
    {3,0}, /* logical port 65 is on bus 3 mdio 28 */ /* first X3220 */
    {3,1}, /* logical port 66 is on bus 3 mdio 30 */
    {3,2}, /* logical port 67 is on bus 3 mdio 29 */ /* first X3220 */
    {3,3}, /* logical port 68 is on bus 3 mdio 31 */
};


/******************************************************************************
MTD_STATUS mtdSampleSystemInitDrv
(
    void
)

 Inputs:
    None

 Outputs:
    None

 Returns:
    Returns MTD_OK or MTD_FAIL if system initialization failed.
    
 Description:
    This sample shows how to initialize a more complicated system with
    multiple MDIO buses and multiple chip types.

    It assumes the system configuration is as follows:

    MDIO Bus #1 has 8 x X3240 devices
    MDIO Bus #2 has 8 x X3240 devices
    MDIO Bus #3 has 2 x X3220 devices

    Please see the logical-to-physical mapping table for how the
    buses and MDIO addresses are mapped from logical to physical.

    Please see the above diagram for the MDIO Address and 
    Logical Port mapping of this system.
    
 Side effects:
    None

 Notes/Warnings:
    To save memory, you could have one mtdLoadDriver() call and one MTD_DEV
    structure per MDIO bus instead of per port, as long as all the devices
    on that bus are of the same PHY type. 

    This example is one way to do this using the appData field in the MTD_DEV
    structure to pass the MDIO bus number to the lower level MDIO read/write
    functions. Another way might be to initialize the MTD_DEV structure with
    pointers to different MDIO read/write functions for different buses. You
    might also have the appData field be a data structure with a callback
    to do the conversion, and the lower level read/write function could do
    the conversion instead of the caller.

    NOTE FOR X3540/X3580:

    MDIO address is the physical MDIO address 0...31. It's an integral part 
    of the Clause 45 MDIO protocol. It's a 5-bit field that identifies a PHY 
    on the MDIO bus. 

    For X3540, there are 4 PHYs per "IC/Chip". The strap configures the starting 
    PHY address for the chip (0,4,8,16,20,24,28). The other 3 ports go 
    in sequence +1, +2, +3.

    For X3580, there are 8 PHYs per "IC/Chip". The strap configures the starting
    PHY address for the chip (0,8,16,24). The other 7 ports go in sequence, +1, +2, etc.

    You will need one MTD DEV structure per device type per MDIO bus (if you will mix 
    multiple of our PHYs on a bus). You're also free to have one MTD DEV structure per 
    IC/chip. If you will have multiple MDIO buses, you'll have to manage this 
    yourself using the "appData" field we provide in our structure. Our assumption was 
    that you would not modify our MTD DEV structure, but instead create your own structure 
    and tie it to the MTD DEV structure by making "appData" to point to your own structure 
    if you need that complexity for say bus identification or semaphore information, etc. 
    Whatever you need to pass to the bottom we will pass the MTD DEV structure all the way 
    through to the MDIO read/write functions you provide.

    After allocating the MTD DEV structure(s) if you pass in the phy address to the mtdLoadDriver() 
    and pass the MTD DEV and PHY address to the API, the rest of the mapping will be handled for you 
    inside the API so you don't need to worry about the details of how the serdes lanes and copper ports 
    are related to each other. This complexity is handled inside the API.
    
    A single X3540 has 4 x T Units (X3580 has 8). Each T unit has a single microprocessor. 
    A T unit maps to a copper interface. Each T unit has one NT unit which maps to the serdes interface. 
    So there are 4 x copper interfaces and 4 x serdes interfaces in one X3540. However, there is one 
    serdes processor internally for every 2 x NT units. Lanes 0 has a serdes processor and Lane 2 
    has a serdes processor. You can think of each PHY has having 2 sides: an NT (serdes) 
    which connects to your ASIC on your board, and a T unit (copper interface) which connects to 
    the MDI/MDIX connector.

    The serdes processors have to have their firmware loaded and initialized by the 2 T units. 
    So Ports 0-1 are tied together architecturally by a single serdes processor and Port 2-3 are tied 
    together by a single serdes processor. This has several ramifications for things like RAM download, 
    T unit resets, etc. This stuff is hidden inside the API. The easiest way to handle this 
    is just to try and use the API as-is according to the guidelines above. 

    See the note in mtdApiTypes.h above the definition of the MTD_DEV structure for
    information about copying this data structure.

*******************************************************************************/
MTD_STATUS mtdSampleSystemInitDrv
(
    void
)
{
    MTD_U32 logicalPort;
    MTD_STATUS status;
    MTD_U16 errCode;

    /* initialize all the fields for single Dev that will be the same for each port */
    /* if you'll use your own mdio read/write functions, initialize them here */

    for (logicalPort = 1; logicalPort < MTD_SAMPLE_MAX_LOGICAL_PORTS; logicalPort++)
    {
        mtdDevStructures[logicalPort].appData = (MTD_PVOID)logicalPort;   

#if (MTD_CLAUSE_22_MDIO == 0)        
        status = mtdLoadDriver(mtdSampleCl45MdioRead, mtdSampleCl45MdioWrite, mtdSampleWait, mtdMDIOPortMap[logicalPort].mdioPortAddress, MTD_FALSE,
                               &mtdDevStructures[logicalPort], &errCode);
#else
        status = mtdLoadDriver(mtdSampleCl22MdioRead, mtdSampleCl22MdioWrite, mtdSampleWait, mtdMDIOPortMap[logicalPort].mdioPortAddress, MTD_FALSE,
                               &mtdDevStructures[logicalPort], &errCode);
#endif

        if (status == MTD_FAIL)
        {
            return MTD_FAIL;
        }
        
    }

    /* This assumes that the low-level MDIO read/write function called in 
       mtdHwXmdioRead/mtdHwXmdioWrite will be modified to grab the appData to 
       read from/write to the correct MDIO bus */
    
    /* After this initialization you can access any PHY port */
    /* using just the MDIO port number and passing the MTD_DEV structure. */

    /* For example, to do a chip reset on the first chip (can use logicalPorts 1..4): */
    /* status = mtdChipHardwareReset(&mtdDevStructures[logicalPort], mtdMDIOPortMap[logicalPort].mdioPortAddress); */

    return MTD_TRUE;
}



/******************************************************************************
MTD_STATUS mtdSampleEnableSpeed
(
    IN MTD_DEV_PTR devPtr, 
    IN MTD_U16 port
)

 Inputs:
    devPtr - allocated memory for the device structure
    port - MDIO port address of the near-end port, 0-31

 Outputs:
    None

 Returns:
    Returns MTD_OK or MTD_FAIL if enable speed test fails or no link-up
    
 Description:
    This sample test shows how to enable the 1G speed mode and test the 
    link status. 
    
 Side effects:
    None

 Notes/Warnings:
    The far-end port must support 1G speed mode.

*******************************************************************************/
MTD_STATUS mtdSampleEnableSpeed
(
    IN MTD_DEV_PTR devPtr, 
    IN MTD_U16 port
)
{    
    MTD_U16 portSpeed;
    MTD_BOOL portLinkUp;
    MTD_U16 speed_to_try = MTD_SPEED_1GIG_FD; /* enable a single speed, | multiple together to enable multiple speeds */
    MTD_U16 speeds_enabled;
    MTD_U16 timeCount = 0;

    /* enable 1G speed on port with mtdAutonegRestart*/
    MTD_ATTEMPT(mtdEnableSpeeds(devPtr, port, speed_to_try, MTD_TRUE)); 

    MTD_DBG_INFO("mtdSampleEnableSpeed: Testing speed :0x%x\n\n", speed_to_try);

    MTD_ATTEMPT(mtdGetSpeedsEnabled(devPtr, port, &speeds_enabled)); /* read back speed enabled */
    if (speeds_enabled != speed_to_try)
    {
        MTD_DBG_ERROR("Get Enable Speed failed\n");
        return MTD_FAIL;
    }

    /* Poll mtdIsBaseTUp() for linkup every 0.5 second for up to 10 seconds */
    do
    {
        timeCount++;
        MTD_ATTEMPT(mtdWait(devPtr, 500)); 
        MTD_ATTEMPT(mtdIsBaseTUp(devPtr, port, &portSpeed, &portLinkUp));
    } while ((portLinkUp == MTD_FALSE) && (timeCount < 20));

    if (portSpeed != speed_to_try || portLinkUp != MTD_TRUE)
    {
        MTD_DBG_ERROR("Enable Speed Test failed\n");
        return MTD_FAIL;
    }

    MTD_DBG_INFO("Enable Speed Test completed successfully\n");
    MTD_DBG_INFO("Speed set to: 0x%X\n", portSpeed);
    MTD_DBG_INFO("Link up in %d seconds\n", timeCount/2);

    return MTD_OK;
}


/******************************************************************************
MTD_STATUS mtdSampleForceSpeed
(
    IN MTD_DEV_PTR devPtr, 
    IN MTD_U16 nearEndPort,
    IN MTD_U16 farEndPort
)

 Inputs:
    devPtr - allocated memory for the device structure
    nearEndPort - MDIO port address of the near-end port, 0-31
    farEndPort - MDIO port address of the far-end port, 0-31

 Outputs:
    None

 Returns:
    Returns MTD_OK or MTD_FAIL if force speed fails
    
 Description:
    This sample shows how to force the T-unit to one of the supported speeds. The 
    near-end and far-end ports used in this sample helps to show the linkup status 
    between 2 ports after the force speed is configured. 

    This sample forces both the near-end and far-end ports speed to 100BT speed,
    MTD_SPEED_100M_FD_AN_DIS. After the force speed is completed, it shows how to 
    test the link status.  Lastly it undoes the forced speed and tests the link 
    status.

    "Forced" means that auto-negotiation is disabled and only that speed attempts
    to train. It is only supported for speeds at 100BT and 10BT.
    
 Side effects:
    None

 Notes/Warnings:
    This sample test requires an MDIO connection between the nearEndPort and 
    farEndPort. 

*******************************************************************************/
MTD_STATUS mtdSampleForceSpeed
(
    IN MTD_DEV_PTR devPtr, 
    IN MTD_U16 nearEndPort,
    IN MTD_U16 farEndPort
)
{
    MTD_U16 speed_to_try = MTD_SPEED_100M_FD_AN_DIS; /* may select only one of these speeds */
    MTD_BOOL nearEndPortLinkUp, farEndPortLinkUp;
    MTD_U16 nearEndPortSpeed,farEndPortSpeed;
    MTD_BOOL nearEndPortIsForced,farEndPortIsForced;
    MTD_U16 timeCount = 0;

    /* Force both sides to this speed, and AN disabled */
    MTD_ATTEMPT(mtdForceSpeed(devPtr,nearEndPort,speed_to_try)); 
    MTD_ATTEMPT(mtdForceSpeed(devPtr,farEndPort,speed_to_try));

    /* Make sure both sides are set to that speed */
    MTD_ATTEMPT(mtdGetForcedSpeed(devPtr,nearEndPort,&nearEndPortIsForced,&nearEndPortSpeed));
    MTD_ATTEMPT(mtdGetForcedSpeed(devPtr,farEndPort,&farEndPortIsForced,&farEndPortSpeed));

    if (nearEndPortIsForced == MTD_FALSE || farEndPortIsForced == MTD_FALSE)
    {
        MTD_DBG_ERROR("Get Force speed status failed\n");
        return MTD_FAIL;
    }

    if (nearEndPortSpeed != speed_to_try || farEndPortSpeed != speed_to_try)
    {
        MTD_DBG_ERROR("Get Force speed mode failed\n");
        return MTD_FAIL;
    }

    /* Poll mtdIsBaseTUp() for linkup every 0.5 second for up to 10 seconds */
    do
    {
        timeCount++;
        MTD_ATTEMPT(mtdWait(devPtr,500)); 
        MTD_ATTEMPT(mtdIsBaseTUp(devPtr,nearEndPort,&nearEndPortSpeed,&nearEndPortLinkUp));
        MTD_ATTEMPT(mtdIsBaseTUp(devPtr,farEndPort,&farEndPortSpeed,&farEndPortLinkUp));
    } while ((nearEndPortLinkUp == MTD_FALSE || farEndPortLinkUp == MTD_FALSE) && timeCount < 20);

    if (nearEndPortSpeed != speed_to_try || farEndPortSpeed != speed_to_try ||
        nearEndPortLinkUp != MTD_TRUE || farEndPortLinkUp != MTD_TRUE)
    {
        MTD_DBG_ERROR("Force speed failed\n");
        return MTD_FAIL;
    }

    MTD_DBG_INFO("Force speed set to: 0x%X\n", nearEndPortSpeed);
    MTD_DBG_INFO("Force speed link up in %d seconds\n", timeCount/2);

    /* Undo the forced speed */
    MTD_ATTEMPT(mtdUndoForcedSpeed(devPtr,nearEndPort,MTD_FALSE)); /* Calls mtdAutonegEnable() */
    MTD_ATTEMPT(mtdUndoForcedSpeed(devPtr,farEndPort,MTD_FALSE)); /* Calls mtdAutonegEnable() */

    /* NOTE: use MTD_SPEED_ALL_33X0_35X0 to include 5G/2.5G speeds for 88X33X0/88X35X0 parts */
    /* use MTD_SPEED_ALL_2XX0 to advertise 5G and below only */
    /* this sample will enable 10G, which is not available on E20X0 and E21X0 parts */
    /* and does not enable 5G/2.5G, so if you run this sample on a E20X0/E21X0 part, 1G will come up */
    /* and speed resolution test below will fail. */
    MTD_ATTEMPT(mtdEnableSpeeds(devPtr,nearEndPort,MTD_SPEED_ALL_32X0,MTD_TRUE)); /* Uses mtdAutonegRestart() */
    MTD_ATTEMPT(mtdEnableSpeeds(devPtr,farEndPort,MTD_SPEED_ALL_32X0,MTD_TRUE)); /* Uses mtdAutonegRestart() */
    
    /* Poll mtdIsBaseTUp() for linkup every 0.5 second for up to 10 seconds */
    do
    {
        timeCount++;
        MTD_ATTEMPT(mtdWait(devPtr,500)); 
        MTD_ATTEMPT(mtdIsBaseTUp(devPtr,nearEndPort,&nearEndPortSpeed,&nearEndPortLinkUp));
        MTD_ATTEMPT(mtdIsBaseTUp(devPtr,farEndPort,&farEndPortSpeed,&farEndPortLinkUp));
    } while ((nearEndPortLinkUp == MTD_FALSE || farEndPortLinkUp == MTD_FALSE) && timeCount < 20);

    if (nearEndPortSpeed != MTD_SPEED_10GIG_FD || farEndPortSpeed != MTD_SPEED_10GIG_FD ||
        nearEndPortLinkUp != MTD_TRUE || farEndPortLinkUp != MTD_TRUE)
    {
        MTD_DBG_ERROR("Undo Force speed failed\n");
        return MTD_FAIL;
    }

    MTD_DBG_INFO("Undo Force speed set to: 0x%X\n", nearEndPortSpeed);
    MTD_DBG_INFO("Undo Force speed link up in %d seconds\n", timeCount/2);

    MTD_DBG_INFO("Force speed Test completed successfully\n");

    return MTD_OK;
}

/* Rename these files to match the image filenames */
#define MTD_SLAVE_IMAGE_FILE     "x3240flashdlslave_0_6_6_5_11945.hdr" /* this is the helper program for flash download */

/* #define MTD_APP_IMAGE_FILE       "x3240fw_0_3_12_0_8433.hdr" */
#define MTD_APP_IMAGE_FILE       "x3310fw_0_2_3_3_8502.hdr" /* this is the PHY application code */

/******************************************************************************
MTD_STATUS mtdSampleUpdateRamImage
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
)

 Inputs:
    devPtr - allocated memory for the device structure
    port - MDIO port address, 0-31

 Outputs:
    None

 Returns:
    Returns MTD_OK or MTD_FAIL if RAM update fails
    
 Description:
    This sample test shows how the application code is downloaded to RAM via MDIO
    when the PHY isn't equipped with flash or flash is being bypassed. 

    It shows the operations to open and read the binary file, gets the image data size 
    and loads the binary data to be downloaded into the RAM buffer. 

    The mtdUpdateRamImage() is called with the above inputs. After the 
    mtdUpdateRamImage() call is completed, it checks the firmware version.

    Using this method, each port must be downloaded one-at-a-time. You can download
    multiple ports at once using MDIO broadcast to all devices on the same MDIO
    bus by using the function mtdParallelUpdateRamImage().
    
 Side effects:
    None

 Notes/Warnings:
    Make sure the file is opened in "rb"(read+binary) mode.
    This sample is not valid for 88X3580/88E2580 devices which
    require the use of mtdParallelUpdateRamImage.

*******************************************************************************/
MTD_STATUS mtdSampleUpdateRamImage
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
)
{
    MTD_STATUS status = MTD_FAIL;
    MTD_U16 errorCode;
    MTD_U8 *appData = NULL;
    MTD_U32 appSize = 0;
    FILE *file;

    /* Make sure the file is opened in "rb"(read+binary) mode */ 
    file = fopen(MTD_APP_IMAGE_FILE, "rb");
    if (file != NULL)
    {
        fseek(file, 0, SEEK_END);
    }
    else
    {
        MTD_DBG_ERROR("File pointer is NULL\n");
        return status;
    }

    /* get the application code image size needed for the mtdUpdateRamImage() */
    appSize = (MTD_U32)ftell(file);
    rewind(file);

    if( appSize == 0 )
    {
        fclose(file);
        MTD_DBG_ERROR("File is empty\n");
        return status;
    }

    appData = (MTD_U8*)malloc(appSize + 1);
    if (appData == NULL)
    {
        fclose(file);
        MTD_DBG_ERROR("Failed to allocate memory\n");
        return status;
    }

    fread(appData, appSize, 1, file);
    fclose(file);
    appData[appSize] = 0;

    status = mtdUpdateRamImage(devPtr, port, appData, appSize, &errorCode);

    if (status == MTD_OK)
    {
        MTD_U8 major;
        MTD_U8 minor;
        MTD_U8 inc;
        MTD_U8 test;

        /* if the mtdUpdateRamImage() is successful, calls the mtdGetFirmwareVersion()
           to check and verify the updated version number */
        mtdWait(devPtr, 1000);
        mtdGetFirmwareVersion(devPtr, port, &major, &minor, &inc, &test);
        MTD_DBG_INFO("RAM image loaded successful\n");
        MTD_DBG_INFO("Firmware version: %d.%d.%d.%d\n", major, minor, inc, test);
    }
    else
    {
        MTD_DBG_ERROR("mtdSampleUpdateRamImage: Failed with error code: 0x%x\n", errorCode);
    }

    if (appData)
    {
        free(appData);
    }

    return status;
}

/******************************************************************************
MTD_STATUS mtdSampleUpdateFlashImage
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
)

 Inputs:
    devPtr - allocated memory for the device structure
    port - MDIO port address, 0-31

 Outputs:
    None

 Returns:
    Returns MTD_OK or MTD_FAIL if Flash update fails
    
 Description:
    This sample test shows how to download a new application image to the flash.

    Depending on the part, there may be one flash per 1 port, 2 ports, or 4 ports. 
    Only the first port on each device needs to be downloaded. All ports on the
    same device share the same image on the flash.

    The flash download program uses a small "helper" program that is downloaded
    to the PHYs RAM first. That helper program interacts with the host to
    take the image being downloaded and program it into the flash via the SPI.

    This sample shows the operations to open and read the binary application code image
    to be loaded to flash and the binary helper program, gets the both
    images data sizes and loads both into its separate buffers. 

    The mtdUpdateFlashImage() is called with the above inputs. After the 
    mtdUpdateFlashImage() call is completed, it takes the PHY out of download mode, 
    resets the PHY, then verifies new image is running.

    Each device (one port per device that is sharing the same flash) must be
    downloaded with one call to mtdUpdateFlashImage(). Multiple
    devices can be broadcast downloaded over the same MDIO bus by using the
    function mtdParallelUpdateFlashImage().

 Side effects:
    None

 Notes/Warnings:
    Make sure the files are opened in "rb"(read+binary) mode

*******************************************************************************/
MTD_STATUS mtdSampleUpdateFlashImage
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
)
{
    MTD_STATUS status = MTD_FAIL;
    MTD_U16 errorCode;
    MTD_U8 *appData = NULL;
    MTD_U8 *slaveData = NULL;
    MTD_U32 appSize = 0;
    MTD_U32 slaveSize = 0;
    FILE *appFile, *slaveFile;

    /* Make sure the application code image file is opened in "rb"(read+binary) mode */ 
    appFile = fopen(MTD_APP_IMAGE_FILE, "rb");
    if (appFile != NULL)
    {
        fseek(appFile, 0, SEEK_END);
    }
    else
    {
        MTD_DBG_ERROR("appFile pointer is NULL\n");
        return status;
    }

    /* get the application code image size needed for the mtdUpdateFlashImage() */
    appSize = (MTD_U32)ftell(appFile);
    rewind(appFile);

    if( appSize == 0 )
    {
        fclose(appFile);
        MTD_DBG_ERROR("Incorrect application code size\n");
        return status;
    }

    /* Make sure the helper program file is opened in "rb"(read+binary) mode */ 
    slaveFile = fopen(MTD_SLAVE_IMAGE_FILE, "rb");
    if (slaveFile != NULL)
    {
        fseek(slaveFile, 0, SEEK_END);
    }
    else
    {
        fclose(appFile);
        MTD_DBG_ERROR("slaveFile pointer is NULL\n");
        return status;
    }

    /* get the flash size needed for the mtdUpdateFlashImage() */
    slaveSize = (MTD_U32)ftell(slaveFile);
    rewind(slaveFile);

    if( slaveSize == 0 )
    {
        fclose(slaveFile);
        fclose(appFile);
        MTD_DBG_ERROR("Incorrect slave size\n");
        return status;
    }

    appData = (MTD_U8*)malloc(appSize + 1);
    if (appData == NULL)
    {
        fclose(slaveFile);
        fclose(appFile);
        MTD_DBG_ERROR("Failed to allocate memory for application code image\n");
        return status;
    }

    slaveData = (MTD_U8*)malloc(slaveSize + 1);
    if (slaveData == NULL)
    {
        free(appData);
        fclose(slaveFile);
        fclose(appFile);
        MTD_DBG_ERROR("Failed to allocate memory for slave helper program image\n");
        return status;
    }

    fread(appData, appSize, 1, appFile);
    fclose(appFile);
    appData[appSize] = 0;

    fread(slaveData, slaveSize, 1, slaveFile);
    fclose(slaveFile);
    slaveData[slaveSize] = 0;

    status = mtdUpdateFlashImage(devPtr, port, appData, appSize, slaveData, slaveSize, &errorCode);

    if (status == MTD_OK)
    {
        MTD_U8 major;
        MTD_U8 minor;
        MTD_U8 inc;
        MTD_U8 test;

        /* if the mtdUpdateFlashImage() is successful, calls the mtdGetFirmwareVersion()
           to check and verify the updated version number */
        mtdRemovePhyMdioDownloadMode(devPtr, port); /* takes PHY out of download mode and does a hardware */
                                                   /* reset on the T unit to reload the new image from flash */
        mtdWait(devPtr, 1000);
        mtdGetFirmwareVersion(devPtr, port, &major, &minor, &inc, &test);
        MTD_DBG_INFO("Application code image loaded successful to flash\n");
        MTD_DBG_INFO("Firmware version: %d.%d.%d.%d\n", major, minor, inc, test);
    }
    else
    {
        MTD_DBG_ERROR("mtdSampleUpdateFlashImage: Failed with error code: 0x%x\n", errorCode);
    }

    if (appData)
    {
        free(appData);
    }

    if (slaveData)
    {
        free(slaveData);
    }

    return status;
}

#define MTD_NUM_HALF_PORTS 4 /* Number of ports in the upper/lower half of device */
/******************************************************************************
    MTD_STATUS mtdSampleUpdateRamImageX3580
    (
        IN MTD_DEV_PTR devPtr,
        IN MTD_U16 port,
        IN MTD_U8 appData[],
        IN MTD_U32 appSize
    )

 Inputs:
    devPtr - allocated memory for the device structure
    port - MDIO base port of an X3580/E2580 device
    appData - the image data read in from the firmware file to download to 
              the PHY's RAM on all ports on the device
    appSize - length of appData in bytes (needs to be even)
          
 Outputs:
    None

 Returns:
    Returns MTD_OK or MTD_FAIL if Flash update fails
    
 Description:
    This is sample code for doing application code RAM download to X3580/E2580.

    This device has an errata that requires a chip reset after power-on reset/
    digital on-board reset and also that the upper ports be loaded and
    running prior to the lower ports being loaded.

    For X3580/E2580, if doing a RAM download the following sequence must be
    followed (if multi-device, the sequence must be applied to all groups
    of upper 4 ports, followed by all groups of lower 4 ports):

    1. Chip reset all upper 4 ports
    2. 10ms delay needed before any MDIO operation after this (IMPORTANT)
    3. Parallel ram download all upper 4 ports
    4. Start code on upper 4 ports
    5. Chip reset applied to lower 4 ports
    6. 10ms delay needed before any MDIO operation after this (IMPORTANT)
    7. Parallel ram download all lower 4 ports
    8. Start code on lower 4 ports

    This sample code shows only one device being handled. If multiple devices
    are to be downloaded, for each device, it's important that the upper
    ports be handled before the lower ports. It could be done in parallel.
    For example, in a loop reset all 4 upper ports, wait 10ms, download
    all 4 upper ports on all devices in parallel, start code on all 4 upper
    ports, then handle lower ports the same way.

 Side effects:
    None

 Notes/Warnings:
    See mtdSampleUpdateRamImage() for an example of how to read image file in.

*******************************************************************************/
MTD_STATUS mtdSampleUpdateRamImageX3580
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U8 appData[],
    IN MTD_U32 appSize
)
{
    MTD_STATUS status = MTD_FAIL;
    MTD_U16 errorCode, aryIndex, portIndex;
    MTD_U16 portList[MTD_NUM_HALF_PORTS], errorPortList[MTD_NUM_HALF_PORTS] = {0,0,0,0};

    if ((port % devPtr->numPortsPerDevice) != 0)
    {
        MTD_DBG_ERROR("Port number should be base port of device\n");
        return status;
    }


    /* Special sequence for X3580/E2580 starts here .... */
    /* Chip reset upper ports */                                
    if (mtdHwXmdioWrite(devPtr,port+((devPtr->numPortsPerDevice))/2,MTD_CUNIT_PORT_CTRL,(1<<14)) == MTD_FAIL) /* If multiple-devices, should do all. This resets uppper and lower. */
    {                                                                                                           /* Very important, delay needed after this or will fail */
        MTD_DBG_ERROR("File is empty\n");                            
        return status;
    }      

    status = mtdWait(devPtr,10); /* Wait 10ms before any MDIO access, should check status for failure */

    /* Download upper 4 ports first */
    for (aryIndex = 0, portIndex = port + (devPtr->numPortsPerDevice)/2; 
         portIndex < (port + devPtr->numPortsPerDevice); 
         aryIndex++,portIndex++)
    {
        portList[aryIndex] = portIndex; /* Build list of ports for upper half of device */
    }

    if (mtdParallelUpdateRamImage(devPtr, portList, appData, appSize, (devPtr->numPortsPerDevice)/2, errorPortList, &errorCode) == MTD_OK)
    {
        MTD_DBG_INFO("Download upper ports success\n");        
    }
    else
    {
        MTD_DBG_ERROR("Download upper ports failed, error code was 0x%04X\n", errorCode);
        return status;    
    }


    /* Chip reset lower ports */
    if(mtdHwXmdioWrite(devPtr,port,MTD_CUNIT_PORT_CTRL,(1<<14)) == MTD_FAIL) /* Very important, delay needed after this or will fail */
    {
        MTD_DBG_ERROR("Chip reset on lower ports failed\n");
        return status;  
    }

    status = mtdWait(devPtr,10); /* Wait 10ms before any MDIO access, should check status for failure */    

    /* Download lower 4 ports next */
    for (aryIndex = 0, portIndex = port; 
         portIndex < port + (devPtr->numPortsPerDevice)/2; 
         aryIndex++,portIndex++)
    {
        portList[aryIndex] = portIndex; /* Build list of ports for lower half of device */
    }

    if (mtdParallelUpdateRamImage(devPtr, portList, appData, appSize, (devPtr->numPortsPerDevice)/2, errorPortList, &errorCode) == MTD_OK)
    {
        MTD_DBG_INFO("Download upper ports success\n");        
    }
    else
    {
        MTD_DBG_ERROR("Download lower ports failed, error code was 0x%04X\n", errorCode);
        return status;    
    }

    /* Special sequence for X3580/E2580 ends here... */

    return status;
}

/******************************************************************************
MTD_STATUS mtdSampleCopperToFiberMode1G
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 nearEndPort,
    IN MTD_U16 farEndPort
)

 Inputs:
    devPtr - allocated memory for the device structure
    nearEndPort - MDIO port address, 0-31
    farEndPort - MDIO port address, 0-31

 Outputs:
    None

 Returns:
    Returns MTD_OK or MTD_FAIL if changing Copper to Fiber mode fails
    
 Description:
    This sample shows how to switch from the Copper mode to the Fiber mode. The 
    near-end and far-end ports used in this sample helps to show the linkup status 
    between 2 ports after switching from the Copper to the Fiber mode. 

    This test sets the H unit to SGMII, the X unit to 1000BX and the C Unit to fiber 
    only mode. After the modes are set, it checks the AN complete and link status.

 Side effects:
    None

 Notes/Warnings:
    This test requires an MDIO connection to both near end and far end ports.

*******************************************************************************/
MTD_STATUS mtdSampleCopperToFiberMode1G
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 nearEndPort,
    IN MTD_U16 farEndPort
)
{
    MTD_BOOL an_complete;  /* AutoNeg completion status */
    MTD_BOOL remote_fault;
    MTD_BOOL ls_latched;
    MTD_BOOL ls_current;
    
    /* Configure both H units for fiber only 1000BASE-X mode */
    MTD_ATTEMPT(mtdSetCunitTopConfig(devPtr, nearEndPort, MTD_F2R_OFF, MTD_MS_FBR_ONLY,
                                 MTD_FT_1000BASEX, MTD_FALSE, MTD_FALSE, MTD_TRUE, MTD_FALSE));

    MTD_ATTEMPT(mtdSetCunitTopConfig(devPtr, farEndPort, MTD_F2R_OFF, MTD_MS_FBR_ONLY, 
                                 MTD_FT_1000BASEX, MTD_FALSE, MTD_FALSE, MTD_TRUE, MTD_FALSE));
    
    MTD_ATTEMPT(mtdSetMacInterfaceControl(devPtr, nearEndPort, MTD_MAC_TYPE_XFI_SGMII_AN_EN, MTD_TRUE,
                                      MTD_MAC_SNOOP_OFF, 0, MTD_MAC_SPEED_10_GBPS, 
                                      MTD_MAX_MAC_SPEED_LEAVE_UNCHANGED, MTD_TRUE,MTD_TRUE));

    MTD_ATTEMPT(mtdSetMacInterfaceControl(devPtr, farEndPort, MTD_MAC_TYPE_XFI_SGMII_AN_EN, MTD_TRUE, 
                                      MTD_MAC_SNOOP_OFF, 0, MTD_MAC_SPEED_10_GBPS, 
                                      MTD_MAX_MAC_SPEED_LEAVE_UNCHANGED, MTD_TRUE, MTD_TRUE));

    /* Configure the BASE-X in both directions, and make one a little different to check status
       Make the fault different in one direction and the pause different in one direction, everything else the same */
    MTD_ATTEMPT(mtdSet1000BXAutoneg_Advertisement(devPtr,nearEndPort,MTD_1000BX_REMOTE_FAULT_LF,MTD_SYM_PAUSE,MTD_TRUE));
    MTD_ATTEMPT(mtdSet1000BXAutoneg_Advertisement(devPtr,farEndPort,MTD_1000BX_REMOTE_FAULT_NONE,MTD_ASYM_PAUSE,MTD_TRUE));
    
    MTD_ATTEMPT(mtdSet1000BXSGMIIControl(devPtr,nearEndPort,MTD_X_UNIT,MTD_FALSE,MTD_SGMII_SPEED_1G,MTD_TRUE,MTD_FALSE,MTD_TRUE,MTD_TRUE));
    MTD_ATTEMPT(mtdSet1000BXSGMIIControl(devPtr,farEndPort,MTD_X_UNIT,MTD_FALSE,MTD_SGMII_SPEED_1G,MTD_TRUE,MTD_FALSE,MTD_TRUE,MTD_TRUE));

    mtdWait(devPtr,1000);

    MTD_ATTEMPT(mtdGet1000BXSGMIIStatus(devPtr,nearEndPort,MTD_X_UNIT,&an_complete,&remote_fault,&ls_latched,&ls_current));

    if (an_complete == MTD_FALSE || remote_fault == MTD_TRUE || ls_latched == MTD_TRUE || ls_current == MTD_FALSE)
    {
        MTD_DBG_ERROR("CopperToFiberMode: X unit failed Port:%u an_complete:%u remote_fault:%u ls_latched:%u, ls_current:%u\n",
                     nearEndPort, an_complete, remote_fault, ls_latched, ls_current);
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdGet1000BXSGMIIStatus(devPtr,farEndPort,MTD_X_UNIT,&an_complete,&remote_fault,&ls_latched,&ls_current));

    if (an_complete == MTD_FALSE || remote_fault == MTD_FALSE || ls_latched == MTD_TRUE || ls_current == MTD_FALSE)
    {
        MTD_DBG_ERROR("CopperToFiberMode: X unit failed Port:%u an_complete:%u remote_fault:%u ls_latched:%u, ls_current:%u\n",
                     farEndPort, an_complete, remote_fault, ls_latched, ls_current);
        return MTD_FAIL;
    }

    MTD_DBG_INFO("Copper mode to Fiber 1G mode changed completed successfully.\n");

    return MTD_OK;
}


/******************************************************************************
MTD_STATUS mtdSampleCopperToFiberMode10G
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 nearEndPort,
    IN MTD_U16 farEndPort
)

 Inputs:
    devPtr - allocated memory for the device structure
    nearEndPort - MDIO port address, 0-31
    farEndPort - MDIO port address, 0-31

 Outputs:
    None

 Returns:
    Returns MTD_OK or MTD_FAIL if changing Copper to Fiber mode fails
    
 Description:
    This sample shows how to switch from the Copper mode to the Fiber mode. The 
    near-end and far-end ports used in this sample helps to show the linkup status 
    between 2 ports after switching from the Copper to the Fiber mode. 

    This test changes the mode to 10GBASE-R (XFI) on both near-end and far-end ports
    and checks the link status on both X and H unit.
    
 Side effects:
    None

 Notes/Warnings:
    This test requires an MDIO connection to both near end and far end ports.

    This test is reading the link's current, real-time status. To get the latched
    link status, call mtdGet10GBRStatus1() (to check if the link dropped since
    the last call, for example.)
 
*******************************************************************************/
MTD_STATUS mtdSampleCopperToFiberMode10G
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 nearEndPort,
    IN MTD_U16 farEndPort
)
{
    MTD_BOOL ls_current;
    MTD_BOOL high_ber;
    MTD_BOOL block_lock;

    /* Test Fiber Only Mode with 10GBR on both sides */
    MTD_ATTEMPT(mtdSetCunitTopConfig(devPtr, nearEndPort, MTD_F2R_OFF, MTD_MS_FBR_ONLY,
                                 MTD_FT_10GBASER, MTD_FALSE, MTD_FALSE, MTD_FALSE, MTD_FALSE));

    MTD_ATTEMPT(mtdSetCunitTopConfig(devPtr, farEndPort, MTD_F2R_OFF, MTD_MS_FBR_ONLY, 
                                 MTD_FT_10GBASER, MTD_FALSE, MTD_FALSE, MTD_FALSE, MTD_FALSE));
    
    MTD_ATTEMPT(mtdSetMacInterfaceControl(devPtr, nearEndPort, MTD_MAC_TYPE_XFI_SGMII_AN_EN,
                                      MTD_TRUE, MTD_MAC_SNOOP_OFF, 0, MTD_MAC_SPEED_10_GBPS,
                                      MTD_MAX_MAC_SPEED_LEAVE_UNCHANGED, MTD_TRUE, MTD_TRUE));

    MTD_ATTEMPT(mtdSetMacInterfaceControl(devPtr, farEndPort, MTD_MAC_TYPE_XFI_SGMII_AN_EN,
                                      MTD_TRUE, MTD_MAC_SNOOP_OFF, 0, MTD_MAC_SPEED_10_GBPS,
                                      MTD_MAX_MAC_SPEED_LEAVE_UNCHANGED, MTD_TRUE, MTD_TRUE));

    /* optional, does a soft reset on the 10GBASE-R block  */
    MTD_ATTEMPT(mtdSet10GBRControl(devPtr, nearEndPort, MTD_X_UNIT, MTD_FALSE, MTD_FALSE, MTD_TRUE));
    MTD_ATTEMPT(mtdSet10GBRControl(devPtr, nearEndPort, MTD_H_UNIT, MTD_FALSE, MTD_FALSE, MTD_TRUE));
   
    MTD_ATTEMPT(mtdSet10GBRControl(devPtr, farEndPort, MTD_X_UNIT, MTD_FALSE, MTD_FALSE, MTD_TRUE));
    MTD_ATTEMPT(mtdSet10GBRControl(devPtr, farEndPort, MTD_H_UNIT, MTD_FALSE, MTD_FALSE, MTD_TRUE));


    mtdWait(devPtr, 1000); /* wait for both to come up */

    MTD_ATTEMPT(mtdGet10GBRReceiveStatus(devPtr,nearEndPort,MTD_H_UNIT,&ls_current,&high_ber,&block_lock));

    if (ls_current == MTD_FALSE || high_ber == MTD_TRUE || block_lock == MTD_FALSE)
    {
        MTD_DBG_ERROR("CopperToFiberMode: H unit failed Port:%u ls_current:%u high_ber:%u block_lock:%u\n",
                      nearEndPort, ls_current, high_ber, block_lock);
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdGet10GBRReceiveStatus(devPtr,nearEndPort,MTD_X_UNIT,&ls_current,&high_ber,&block_lock));

    if (ls_current == MTD_FALSE || high_ber == MTD_TRUE || block_lock == MTD_FALSE)
    {
        MTD_DBG_ERROR("CopperToFiberMode: X unit failed Port:%u ls_current:%u high_ber:%u block_lock:%u\n",
                      nearEndPort, ls_current, high_ber, block_lock);
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdGet10GBRReceiveStatus(devPtr,farEndPort,MTD_H_UNIT,&ls_current,&high_ber,&block_lock));

    if (ls_current == MTD_FALSE || high_ber == MTD_TRUE || block_lock == MTD_FALSE)
    {
        MTD_DBG_ERROR("CopperToFiberMode: H unit failed Port:%u ls_current:%u high_ber:%u block_lock:%u\n",
                      farEndPort, ls_current, high_ber, block_lock);
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdGet10GBRReceiveStatus(devPtr,farEndPort,MTD_X_UNIT,&ls_current,&high_ber,&block_lock));

    if (ls_current == MTD_FALSE || high_ber == MTD_TRUE || block_lock == MTD_FALSE)
    {
        MTD_DBG_ERROR("CopperToFiberMode: X unit failed Port:%u ls_current:%u high_ber:%u block_lock:%u\n",
                     farEndPort, ls_current, high_ber, block_lock);
        return MTD_FAIL;
    }

    MTD_DBG_INFO("Copper mode to Fiber 10G mode changed completed successfully.\n");

    return MTD_OK;
}


/******************************************************************************
MTD_STATUS mtdSampleFiberToCopperMode
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 nearEndPort,
    IN MTD_U16 farEndPort
)

 Inputs:
    devPtr - allocated memory for the device structure
    nearEndPort - MDIO port address, 0-31
    farEndPort - MDIO port address, 0-31

 Outputs:
    None

 Returns:
    Returns MTD_OK or MTD_FAIL if changing Fiber to Copper mode fails
    
 Description:
    This sample shows how to switch from the Fiber mode to the Copper mode. The 
    near-end and far-end ports used in this sample helps to show the linkup status 
    between 2 ports after switching from the Fiber to the Copper mode. 

    This test changes the configuration to Copper mode 10GBASE-T on both near-end 
    and far-end ports. After the change, it polls every half a second up to 10 seconds
    to check the copper link status. 

 Side effects:
    None

 Notes/Warnings:
    This test requires an MDIO connection to both near end and far end ports.

    This test assumes both ends were previously set to advertise 10GBT
    or defaulted to this on reset.

*******************************************************************************/
MTD_STATUS mtdSampleFiberToCopperMode
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 nearEndPort,
    IN MTD_U16 farEndPort
)
{
    MTD_U16 baseTSpeed;
    MTD_BOOL islinkup;
    MTD_U16 timeCount = 0;

    if (!MTD_HAS_X_UNIT(devPtr))
    {
        /* no fiber interface on this PHY */
        MTD_ATTEMPT(mtdSetCunitTopConfig(devPtr, nearEndPort, MTD_F2R_OFF, MTD_MS_CU_ONLY, 
                                 MTD_FT_NONE, MTD_FALSE, MTD_FALSE, MTD_TRUE, MTD_FALSE));

        MTD_ATTEMPT(mtdSetCunitTopConfig(devPtr, farEndPort, MTD_F2R_OFF, MTD_MS_CU_ONLY,
                                 MTD_FT_NONE, MTD_FALSE, MTD_FALSE, MTD_TRUE, MTD_FALSE));
    }
    else
    {
        MTD_ATTEMPT(mtdSetCunitTopConfig(devPtr, nearEndPort, MTD_F2R_OFF, MTD_MS_CU_ONLY, 
                                 MTD_FT_10GBASER, MTD_FALSE, MTD_FALSE, MTD_TRUE, MTD_FALSE));

        MTD_ATTEMPT(mtdSetCunitTopConfig(devPtr, farEndPort, MTD_F2R_OFF, MTD_MS_CU_ONLY,
                                 MTD_FT_10GBASER, MTD_FALSE, MTD_FALSE, MTD_TRUE, MTD_FALSE));
    }

    MTD_ATTEMPT(mtdSetMacInterfaceControl(devPtr, nearEndPort, MTD_MAC_TYPE_XFI_SGMII_AN_EN, 
                                      MTD_TRUE, MTD_MAC_SNOOP_OFF, 0, MTD_MAC_SPEED_10_GBPS,
                                      MTD_MAX_MAC_SPEED_LEAVE_UNCHANGED, MTD_TRUE, MTD_TRUE));
    
    MTD_ATTEMPT(mtdSetMacInterfaceControl(devPtr, farEndPort, MTD_MAC_TYPE_XFI_SGMII_AN_EN, 
                                      MTD_TRUE, MTD_MAC_SNOOP_OFF, 0, MTD_MAC_SPEED_10_GBPS,
                                      MTD_MAX_MAC_SPEED_LEAVE_UNCHANGED, MTD_TRUE, MTD_TRUE));
    
    /* Poll mtdIsBaseTUp() for linkup every 0.5 second for up to 10 seconds */
    do
    {
        timeCount++;
        MTD_ATTEMPT(mtdWait(devPtr, 500)); 
        MTD_ATTEMPT(mtdIsBaseTUp(devPtr, nearEndPort, &baseTSpeed, &islinkup));
    } while (islinkup == MTD_FALSE && timeCount < 20);

    if (!(islinkup == MTD_TRUE && (baseTSpeed == MTD_SPEED_10GIG_FD || baseTSpeed == MTD_SPEED_5GIG_FD)))
    {
        MTD_DBG_ERROR("FiberToCopperMode: failed Port:%u is linkup:0x%X baseTSpeed:0x%X\n", 
                      nearEndPort, islinkup, baseTSpeed);
        return MTD_FAIL;
    }

    if (baseTSpeed == MTD_SPEED_10GIG_FD)
    {
        MTD_DBG_INFO("Fiber to Copper mode changed completed successfully. Link is up at 10G speed in %u seconds.\n", 
                  timeCount/2);
    }
    else
    {
        MTD_DBG_INFO("Fiber to Copper mode changed completed successfully. Link is up at 5G speed in %u seconds.\n", 
                  timeCount/2);
    }

    return MTD_OK;
}

#if MTD_ORIGSERDES

/******************************************************************************
MTD_STATUS mtdSamplePRBSLockedTest
(
    IN MTD_DEV_PTR devPtr0,
    IN MTD_U16 port0,
    IN MTD_DEV_PTR devPtr1,
    IN MTD_U16 port1,
    IN MTD_U16 HorXUnit
);

 Inputs:
    devPtr0 - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port0 - MDIO port address, 0-31
    devPtr1 - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port1 - MDIO port address, 0-31
    HorXunit - MTD_X_UNIT or MTD_H_UNIT, depending on which interface is
               being read

 Outputs:
    None.

 Returns:
    MTD_OK if successfully
    MTD_FAIL if failed

 Description:
    This sample function set PRBS pattern on two ports and enabling PRBS
    transmitter and receiver. Port0 is sending PBRS to port0 while is expected
    to receive PRBS from port1 at the same time and vice versa.

    The PRBS start generating immediately once PRBS transmitter is enabled.
    Thus, users should enable the PBRS receiver first then enable the transmitter
    to ensure all packets get received and counted.

    Related APIs:
        mtdSetPRBSPattern()
        mtdSetPRBSEnableTxRx()
        mtdPRBSCounterReset()
        mtdSetPRBSWaitForLock()
        mtdGetPRBSWaitForLock()
        mtdSetPRBSClearOnRead()
        mtdGetPRBSClearOnRead()
        mtdGetPRBSLocked()
        mtdGetPRBSCounts()

 Side effects:
    None.

 Notes/Warnings:
    None.
*******************************************************************************/
MTD_STATUS mtdSamplePRBSLockedTest
(
    IN MTD_DEV_PTR devPtr0,
    IN MTD_U16 port0,
    IN MTD_DEV_PTR devPtr1,
    IN MTD_U16 port1,
    IN MTD_U16 HorXUnit
)
{
    /* Refers to the definitions above mtdSetPRBSPattern() declaration */
    MTD_U16 patternSelect = MTD_PRBS31;
    MTD_U16 disableWaitForLock = 0x0;   /* HW/SW reset value is also 0 */
    MTD_U16 timeCount;
    MTD_BOOL prbsLocked;
    MTD_U64 txBitCount, rxBitCount, rxBitErrorCount;

    MTD_BOOL prbsLocked1;
    MTD_U64 txBitCount1, rxBitCount1, rxBitErrorCount1;

    /* Clear counter before start PRBS transmitter/receiver */
    MTD_ATTEMPT(mtdPRBSCounterReset(devPtr0, port0, HorXUnit));
    MTD_ATTEMPT(mtdPRBSCounterReset(devPtr1, port1, HorXUnit));

    /* OR */
    /* Turn on the clear-on-read feature */
    MTD_ATTEMPT(mtdSetPRBSClearOnRead(devPtr0, port0, HorXUnit, MTD_TRUE));
    MTD_ATTEMPT(mtdSetPRBSClearOnRead(devPtr1, port1, HorXUnit, MTD_TRUE));

    /* Users can disable wait-for-lock by setting disableWaitForLock to 1 to */
    /* make PRBS error counter start counting immediately */
    MTD_ATTEMPT(mtdSetPRBSWaitForLock(devPtr0, port0, HorXUnit, disableWaitForLock));
    MTD_ATTEMPT(mtdSetPRBSWaitForLock(devPtr1, port1, HorXUnit, disableWaitForLock));

    /* port0 send PRBS to port1 while port1 send PRBS to port0 */
    /* Enable the receiver first before enabling the transmitter */
    /* to ensure all packet sent get received */
    MTD_ATTEMPT(mtdSetPRBSEnableTxRx(devPtr0, port0, HorXUnit, MTD_TRUE, MTD_FALSE));
    MTD_ATTEMPT(mtdSetPRBSEnableTxRx(devPtr1, port1, HorXUnit, MTD_TRUE, MTD_FALSE));

    MTD_ATTEMPT(mtdSetPRBSEnableTxRx(devPtr0, port0, HorXUnit, MTD_TRUE, MTD_TRUE));
    MTD_ATTEMPT(mtdSetPRBSEnableTxRx(devPtr1, port1, HorXUnit, MTD_TRUE, MTD_TRUE));

    /* Set PRBS Pattern, see mtdSetPRBSPattern() for details */
    MTD_ATTEMPT(mtdSetPRBSPattern(devPtr0, port0, HorXUnit, patternSelect));
    MTD_ATTEMPT(mtdSetPRBSPattern(devPtr1, port1, HorXUnit, patternSelect));

    timeCount = 0;
    MTD_DBG_INFO("Polling PRBS Locked bit...\n");
    do
    {
        timeCount++;
        MTD_ATTEMPT(mtdWait(devPtr0, 100)); /* 100ms is a example */
        MTD_ATTEMPT(mtdGetPRBSLocked(devPtr0, port0, HorXUnit, &prbsLocked));
        MTD_ATTEMPT(mtdGetPRBSLocked(devPtr1, port1, HorXUnit, &prbsLocked1));
    } while ((prbsLocked != MTD_TRUE || prbsLocked1 != MTD_TRUE) && timeCount < 1*2);  /* e.g. 200ms timeout */

    if (prbsLocked != MTD_TRUE || prbsLocked1 != MTD_TRUE)
    {
        MTD_DBG_ERROR("mtdSapmlePRBSLockedTest: PRBS wait-for-locked time out\n");
        return MTD_FAIL;
    }

    /* Stop PRBS transmitter and receiver */
    MTD_ATTEMPT(mtdSetPRBSEnableTxRx(devPtr0, port0, HorXUnit, MTD_FALSE, MTD_FALSE));
    MTD_ATTEMPT(mtdSetPRBSEnableTxRx(devPtr1, port1, HorXUnit, MTD_FALSE, MTD_FALSE));

    /* Read-to-clear, the counters should have stop counting and after this read operation they should reset to 0*/
    MTD_ATTEMPT(mtdGetPRBSCounts(devPtr0, port0, HorXUnit, &txBitCount, &rxBitCount, &rxBitErrorCount));
    MTD_ATTEMPT(mtdGetPRBSCounts(devPtr1, port1, HorXUnit, &txBitCount1, &rxBitCount1, &rxBitErrorCount1));

    /* Verify Clear-on-read: all counter should all reset to 0*/
    MTD_ATTEMPT(mtdGetPRBSCounts(devPtr0, port0, HorXUnit, &txBitCount, &rxBitCount, &rxBitErrorCount));
    MTD_ATTEMPT(mtdGetPRBSCounts(devPtr1, port1, HorXUnit, &txBitCount1, &rxBitCount1, &rxBitErrorCount1));

    if (txBitCount != 0 || rxBitCount != 0 || rxBitErrorCount != 0 ||
        txBitCount1 != 0 || rxBitCount1 != 0 || rxBitErrorCount1 != 0)
    {
        MTD_DBG_ERROR("mtdSapmlePRBSLockedTest: Counter is not cleared after read.\n");
        return MTD_FAIL;
    }

    return MTD_OK;
}

/******************************************************************************
MTD_STATUS mtdSampleGetEye
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port, 
    IN MTD_U16 lane
)

 Inputs:
    devPtr - allocated memory for the device structure
    port - MDIO port address, 0-31
    lane - eye data of the lane 

 Outputs:
    None

 Returns:
    Returns MTD_OK or MTD_FAIL if the eye diagram reading or plotting fails
    
 Description:
    Sample code to extract the eye data and plotting eye diagram by calling 
    the mtdGetSerdesEyeStatistics and mtdSerdesEyePlotChart APIs.
    The mtdSerdesEyePlotChart will plot the eye diagram to the buffer defined 
    in the mtdDbgPrint().

 Side effects:
    None

 Notes/Warnings:
    Must wait for link up before extracting eye data.


*******************************************************************************/
MTD_STATUS mtdSampleGetEye
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port, 
    IN MTD_U16 lane
)
{
    MTD_U16 eyeArea;
    MTD_U16 eyeWidth; 
    MTD_U16 eyeHeight;

    /* provide the arrays for the eye data */
    MTD_U32 eyeDataUp[(MTD_EYEDIAGRAM_NROWS + 1) / 2][MTD_EYEDIAGRAM_NCOLS];
    MTD_U32 eyeDataDn[(MTD_EYEDIAGRAM_NROWS + 1) / 2][MTD_EYEDIAGRAM_NCOLS];

    MTD_ATTEMPT(mtdGetSerdesEyeStatistics(devPtr, port, lane, eyeDataUp, eyeDataDn, 
                                      &eyeArea, &eyeWidth, &eyeHeight));

    MTD_ATTEMPT(mtdSerdesEyePlotChart(devPtr, eyeDataUp, eyeDataDn));


    MTD_DBG_INFO("mtdSampleGetEye: eyeArea:%u eyeWidth:%u eyeHeight:%u\n", 
                 eyeArea, eyeWidth, eyeHeight);

    return MTD_OK;
}

#endif /* MTD_ORIGSERDES */

/******************************************************************************
MTD_STATUS mtdSampleGetChipIntrStatus
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16     port,
    OUT MTD_INTR_STATUS interruptStatus[]
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port   - mdio address of first port on device 

 Outputs:
    interruptStatus[] - array of interrupt structures, one per port, total size 
                        determined by devPtr->numPortsPerDevice. User required to allocate
                        memory at least devPtr->numPortsPerDevice structures.

 Returns:
    MTD_OK or MTD_FAIL

 Description:
    Get the status of all the triggered interrupts on this device. 

    This function uses devPtr->numPortsPerDevice to determine how many ports to check
    and return. Caller may use this to determine how much memory to allocate.

    Please check the interrupt definition of the corresponding unit to identify 
    the interrupt source. See the defines in mtdIntr.h for the defines for
    these interrupts.

 Side effects:
    None

 Notes/Warnings:
    No X-unit on E20X0 and E21X0 PHYs. 
    
    All individual interrupts are cleared after this API call. This function
    checks all interrupts regardless of enable/disable (mask).
******************************************************************************/
MTD_STATUS mtdSampleGetChipIntrStatus
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16     port,
    OUT MTD_INTR_STATUS interruptStatus[]
)
{
    MTD_U16  i;
    MTD_BOOL interruptTrig;

    for (i=0; i<devPtr->numPortsPerDevice; i++)
    {
        MTD_ATTEMPT(mtdGetPortIntrStatus(devPtr, (port+i), &interruptTrig, &interruptStatus[i]));
        if (interruptTrig == MTD_TRUE)
        {
             MTD_DBG_INFO("Interrupt triggered on port: %d\n", (port+i));
        }
    }

    return MTD_OK;
}

/******************************************************************************
MTD_STATUS mtdSampleE21X0EnableSerialLED
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31 

 Outputs:
    none

 Returns:
    MTD_OK if LEDs are successfully configured
    MTD_FAIL if the device is not supported or other failure

 Description:
    This sample function applies to only 88E2110 and 88E2180 PHY devices.  It shows 
    how to enable the Serial LEDs for various activities, statuses, features, etc...  
    
    The following are some of the LED registers to configure the LEDs. Refer to the 
    datasheet for the complete details on these registers and other LED registers:
        31.0xF03A - LED Control Register is a global register. A write to 31.F03A on 
        any port will update all 8 ports on the 88E2180
        31.0xF020 - LED[0] Control
        31.0xF021 - LED[1] Control
        31.0xF022 - LED[2] Control

    This sample sets the LED1 to turn ON when port is running at 5G speed and 
    LED2 to turn ON when port is running at 2.5G speed.

 Side effects:
    None

 Notes/Warnings:
******************************************************************************/
MTD_STATUS mtdSampleE21X0EnableSerialLED
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
)
{
    if (!(MTD_IS_E21X0_BASE(devPtr->deviceId)))
    {
        MTD_DBG_ERROR("mtdSampleE21X0EnableSerialLED: Device is not supported.\n");
        return MTD_FAIL;
    }

    /* if device is not a 1 or 8-port device, it's not supported */
    if (!((devPtr->numPortsPerDevice == 1) || (devPtr->numPortsPerDevice == 8)))
    {
        MTD_DBG_ERROR("mtdSampleE21X0EnableSerialLED: Device with port count:%d is not supported.\n", devPtr->numPortsPerDevice);
        return MTD_FAIL;
    }

    /* enables all 3 LEDs for E2110 and E2180 on 31.F03A; 31.F03A is a global register; 
       A write to 31.F03A to any port will update all 8 ports */
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_C_UNIT_GENERAL, CUNIT_SLED_CONTROL, 0x0112));

    if (devPtr->numPortsPerDevice == 1) /* E2110 */
    {
        /* LED1 turns on when linkup on 5G, 31.F021 */
        MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_C_UNIT_GENERAL, CUNIT_LED1_CONTROL, 0x00B0));

        /* LED2 turns on when linkup on 2.5G, 31.F022 */
        MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_C_UNIT_GENERAL, CUNIT_LED2_CONTROL, 0x00A8));

    }
    else /* E2180 */
    {
        MTD_U16 numPortsPerDevice, basePort, portIndex;

        numPortsPerDevice = devPtr->numPortsPerDevice;

        /* calculates the MDIO port number of the 1st(base) port */
        basePort = (port/numPortsPerDevice) * numPortsPerDevice;   
        for (portIndex = basePort; portIndex < devPtr->numPortsPerDevice; portIndex++) 
        {
            /* LED1 turns on when linkup on 5G, 31.F021 */
            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, portIndex, MTD_C_UNIT_GENERAL, CUNIT_LED1_CONTROL, 0x00B0));

            /* LED2 turns on when linkup on 2.5G, 31.F022 */
            MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, portIndex, MTD_C_UNIT_GENERAL, CUNIT_LED2_CONTROL, 0x00A8));
        }
    }

    return MTD_OK;
}


/******************************************************************************
MTD_STATUS mtdSampleE2180EnableParallelLED
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31 

 Outputs:
    none

 Returns:
    MTD_OK if LEDs are successfully configured
    MTD_FAIL if the device is not supported or other failure

 Description:
    This sample function applies to only 88E2180 PHY device.  It shows 
    how to enable the parallel LEDs on the 88E2180 for various activities, 
    statuses, features, etc...  
    
    The following are some of the LED registers to configure the LEDs. Refer to the 
    datasheet for the complete details on these registers and other LED registers:
        31.0xF015 - GPIO Interrupt Control
        31.0xF020 - LED[0] Control
        31.0xF021 - LED[1] Control

    This sample sets the parallel LEDs where LED0 turns on for copper link and blinks
    on tx/rx activity, and LED1 turns on for 5G speed (off for lower speeds).

 Side effects:
    None

 Notes/Warnings:
*******************************************************************************/
MTD_STATUS mtdSampleE2180EnableParallelLED
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
)
{
    MTD_U16 numPortsPerDevice, basePort, portIndex;

    if (!(MTD_IS_E21X0_BASE(devPtr->deviceId)))
    {
        MTD_DBG_ERROR("mtdSampleE2180EnableParallelLED: Device is not supported.\n");
        return MTD_FAIL;
    }

    /* if device is not a 8-port device, it's not supported */
    if (!(devPtr->numPortsPerDevice == 8))
    {
        MTD_DBG_ERROR("mtdSampleE2180EnableParallelLED: Device with port count:%d is not supported.\n", devPtr->numPortsPerDevice);
        return MTD_FAIL;
    }

    numPortsPerDevice = devPtr->numPortsPerDevice;

    /* calculates the MDIO port number of the 1st(base) port */
    basePort = (port/numPortsPerDevice) * numPortsPerDevice; 

    for (portIndex = basePort; portIndex < devPtr->numPortsPerDevice; portIndex++) 
    {
        /* enable 2 LED mode control by GPIO, 31.F015 */
        MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, portIndex, MTD_C_UNIT_GENERAL, CUNIT_GPIO_CONTROL, 0x8800));
        
        /* uses default for LED0, on for copper link, blink for tx/rx activity */

        /* LED1 turns on when linkup on 5G, 31.F021 */
        MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, portIndex, MTD_C_UNIT_GENERAL, CUNIT_LED1_CONTROL, 0x00B0));
    }

    return MTD_OK;
}

/******************************************************************************
MTD_STATUS mtdX35X0LedSetControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 ledNum,
    IN MTD_U16 polarity,
    IN MTD_U16 blinkRate,
    IN MTD_U16 solidBehavior,
    IN MTD_U16 blinkBehavior    
)

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    ledNum - which LED is being configured MTD_X35X0_LED0...3
    polarity - selects the polarity (see header file for explanation of values)    
        MTD_X35X0_LED_POLARITY_LH
        MTD_X35X0_LED_POLARITY_HL
        MTD_X35X0_LED_POLARITY_LT
        MTD_X35X0_LED_POLARITY_HT
        or
        MTD_X35X0_LED_DO_NOT_CHANGE
    blinkRate - selects which blink rate in the timer control register to use
        MTD_X35X0_LED_BLINK_RATE1
        MTD_X35X0_LED_BLINK_RATE2
        or
        MTD_X35X0_LED_DO_NOT_CHANGE
    solidBehavior - selects the solid behavior
        MTD_X35X0_LED_SLD_OFF
        MTD_X35X0_LED_SLD_TXRX
        MTD_X35X0_LED_SLD_TX
        MTD_X35X0_LED_SLD_RX
        MTD_X35X0_LED_SLD_COLLISION
        MTD_X35X0_LED_SLD_CU_LINK
        MTD_X35X0_LED_SLD_NOT_USED
        MTD_X35X0_LED_SLD_CU_LINK
        MTD_X35X0_LED_SLD_10M
        MTD_X35X0_LED_SLD_100M
        MTD_X35X0_LED_SLD_1G
        MTD_X35X0_LED_SLD_10G
        MTD_X35X0_LED_SLD_10_100M
        MTD_X35X0_LED_SLD_10_100_1G
        MTD_X35X0_LED_SLD_100_10G
        MTD_X35X0_LED_SLD_1G_10G
        MTD_X35X0_LED_SLD_1G_10G_SLAVE
        MTD_X35X0_LED_SLD_1G_10G_MASTER
        MTD_X35X0_LED_SLD_HALF_DUPLEX
        MTD_X35X0_LED_SLD_FULL_DUPLEX
        MTD_X35X0_LED_SLD_ENERGY_EFF
        MTD_X35X0_LED_SLD_2P5G
        MTD_X35X0_LED_SLD_5G
        MTD_X35X0_LED_SLD_ON
        MTD_X35X0_LED_SLD_2P5
        MTD_X35X0_LED_SLD_2P5G_5G_MASTER
        or
        MTD_X35X0_LED_DO_NOT_CHANGE
        
    blinkRate - selects the blinking behavior 
        MTD_X35X0_LED_BLNK_OFF 
        MTD_X35X0_LED_BLNK_TXRX
        MTD_X35X0_LED_BLNK_TX
        MTD_X35X0_LED_BLNK_RX
        MTD_X35X0_LED_BLNK_COLLISION
        MTD_X35X0_LED_BLNK_CU_LINK
        MTD_X35X0_LED_BLNK_NOT_USED
        MTD_X35X0_LED_BLNK_CU_LINK
        MTD_X35X0_LED_BLNK_10M
        MTD_X35X0_LED_BLNK_100M
        MTD_X35X0_LED_BLNK_1G
        MTD_X35X0_LED_BLNK_10G
        MTD_X35X0_LED_BLNK_10_100M
        MTD_X35X0_LED_BLNK_10_100_1G
        MTD_X35X0_LED_BLNK_100_10G
        MTD_X35X0_LED_BLNK_1G_10G
        MTD_X35X0_LED_BLNK_1G_10G_SLAVE
        MTD_X35X0_LED_BLNK_1G_10G_MASTER
        MTD_X35X0_LED_BLNK_HALF_DUPLEX
        MTD_X35X0_LED_BLNK_FULL_DUPLEX
        MTD_X35X0_LED_BLNK_ENERGY_EFF
        MTD_X35X0_LED_BLNK_2P5G
        MTD_X35X0_LED_BLNK_5G
        MTD_X35X0_LED_BLNK_ON
        MTD_X35X0_LED_BLNK_2P5G_5G_SLAVE
        MTD_X35X0_LED_BLNK_2P5G_5G_MASTER
        MTD_X35X0_LED_BLNK_BLNK_MIX

        Dual-Mode Options (see note below)
        MTD_X35X0_LED_BLNK_SLD_MIX
        MTD_X35X0_LED_BLNK_SPD_EEE_TYPE1
        MTD_X35X0_LED_BLNK_SPEED_TYPE1
        MTD_X35X0_LED_BLNK_SPD_EEE_TYPE2
        MTD_X35X0_LED_BLNK_SPEED_TYPE2

        or
        MTD_X35X0_LED_DO_NOT_CHANGE

 Outputs:
    none

 Returns:
    MTD_OK if LEDs are successfully configured
    MTD_FAIL if the device is not supported or other failure

 Description:
    Configures LED0...LED3. For any parameter desired to be left unchanged,
    pass MTD_X35X0_LED_DO_NOT_CHANGE.

 Side effects:
    None

 Notes/Warnings:
    When LED0 is put into dual-mode, the LED 1 controls for solid/blinking/rate
    are physically ignored. Similarly for LED2 and LED3.

    Some options may not be applicable for E2540/E2580 PHYs.
    
*******************************************************************************/
MTD_STATUS mtdX35X0LedSetControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 ledNum,
    IN MTD_U16 polarity,
    IN MTD_U16 blinkRate,
    IN MTD_U16 solidBehavior,
    IN MTD_U16 blinkBehavior    
)
{
    MTD_U16 regValue;

    if (ledNum > MTD_X35X0_LED3)
    {
        return MTD_FAIL; /* LED out of range */
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_X35X0_LED_CONTROL(ledNum), &regValue));

    if (polarity != MTD_X35X0_LED_DO_NOT_CHANGE)
    {
        if (polarity <= MTD_X35X0_LED_POLARITY_HT)
        {
            MTD_ATTEMPT(mtdHwSetRegFieldToWord(regValue, polarity, MTD_X35X0_LED_POLARITY_BOFFSET, MTD_X35X0_LED_POLARITY_BLENGTH, &regValue));
        }
        else
        {
            return MTD_FAIL; /* polarity out of range */
        }
    }

    if (blinkRate != MTD_X35X0_LED_DO_NOT_CHANGE)
    {
        if (blinkRate <= MTD_X35X0_LED_BLINK_RATE2)
        {
            MTD_ATTEMPT(mtdHwSetRegFieldToWord(regValue, blinkRate, MTD_X35X0_LED_BLINKRT_BOFFSET, MTD_X35X0_LED_BLINKRT_BLENGTH, &regValue));
        }
        else
        {
            return MTD_FAIL; /* blink rate selection out of range */
        }
    }

    if (solidBehavior != MTD_X35X0_LED_DO_NOT_CHANGE)
    {
        if (solidBehavior <= MTD_X35X0_LED_SLD_2P5G_5G_MASTER)
        {
            MTD_ATTEMPT(mtdHwSetRegFieldToWord(regValue, solidBehavior, MTD_X35X0_LED_SOLID_BOFFSET, MTD_X35X0_LED_SOLID_BLENGTH, &regValue));
        }
        else
        {
            return MTD_FAIL; /* solid behavior selection out of range */
        }
    }
    
    if (blinkBehavior != MTD_X35X0_LED_DO_NOT_CHANGE)
    {
        if (blinkBehavior <= MTD_X35X0_LED_SLD_2P5G_5G_MASTER)
        {
            MTD_ATTEMPT(mtdHwSetRegFieldToWord(regValue, blinkBehavior, MTD_X35X0_LED_BLINK_BOFFSET, MTD_X35X0_LED_BLINK_BLENGTH, &regValue));
        }
        else
        {
            return MTD_FAIL; /* blink behavior selection out of range */
        }
    }


    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_X35X0_LED_CONTROL(ledNum), regValue));

    return (MTD_OK);
    
}


/******************************************************************************
MTD_STATUS mtdX35X0LedSetMixingControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 led20MixPercentage,
    IN MTD_U16 led31MixPercentage,
    IN MTD_U16 ledActivitySelection
)

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    led20MixPercentage - Selects mixing percentage for LED2 and LED0
        MTD_X35X0_LED_MIX_0
        MTD_X35X0_LED_MIX_12P5
        MTD_X35X0_LED_MIX_50 (see notes below)
        MTD_X35X0_LED_MIX_87P5
        MTD_X35X0_LED_MIX_100
        or
        MTD_X35X0_LED_DO_NOT_CHANGE
    led31MixPercentage - Selects mixing percentage for LED1 and LED1
        MTD_X35X0_LED_MIX_0
        MTD_X35X0_LED_MIX_12P5
        MTD_X35X0_LED_MIX_50 (see notes below)
        MTD_X35X0_LED_MIX_87P5
        MTD_X35X0_LED_MIX_100
        or
        MTD_X35X0_LED_DO_NOT_CHANGE
    ledActivitySelection - Selects what counts as activity    
        MTD_X35X0_LED_COUNT_RX
        MTD_X35X0_LED_COUNT_TX
        MTD_X35X0_LED_COUNT_RXTX
        or
        MTD_X35X0_LED_DO_NOT_CHANGE


 Outputs:
    none

 Returns:
    MTD_OK if LEDs are successfully configured
    MTD_FAIL if the device is not supported or other failure

 Description:
    Configures the LED mixing control register.

 Side effects:
    None

 Notes/Warnings:
    The mix percentage should not be set greater than 50% when using two terminal
    bi-color LEDs.

    
*******************************************************************************/
MTD_STATUS mtdX35X0LedSetMixingControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 led20MixPercentage,
    IN MTD_U16 led31MixPercentage,
    IN MTD_U16 ledActivitySelection
)
{
    MTD_U16 regValue;

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_X35X0_LED_MIX_CTRL, &regValue));

    if (led20MixPercentage != MTD_X35X0_LED_DO_NOT_CHANGE)
    {
        if (led20MixPercentage <= MTD_X35X0_LED_MIX_100)
        {
            MTD_ATTEMPT(mtdHwSetRegFieldToWord(regValue, led20MixPercentage, MTD_X35X0_LED_20MIX_BOFFSET, MTD_X35X0_LED_20MIX_BLENGTH, &regValue));
        }
        else
        {
            return MTD_FAIL; /* LED2 and LED0 mix percentage out of range */
        }
    }

    if (led31MixPercentage != MTD_X35X0_LED_DO_NOT_CHANGE)
    {
        if (led31MixPercentage <= MTD_X35X0_LED_MIX_100)
        {
            MTD_ATTEMPT(mtdHwSetRegFieldToWord(regValue, led31MixPercentage, MTD_X35X0_LED_31MIX_BOFFSET, MTD_X35X0_LED_31MIX_BLENGTH, &regValue));
        }
        else
        {
            return MTD_FAIL; /* LED3 and LED1 mix percentage out of range */
        }
    }

    if (ledActivitySelection != MTD_X35X0_LED_DO_NOT_CHANGE)
    {
        if (ledActivitySelection <= MTD_X35X0_LED_COUNT_RXTX)
        {
            MTD_ATTEMPT(mtdHwSetRegFieldToWord(regValue, ledActivitySelection, MTD_X35X0_LED_ACTIVITY_BOFFSET, MTD_X35X0_LED_ACTIVITY_BLENGTH, &regValue));
        }
        else
        {
            return MTD_FAIL; /* activity selection out of range */
        }
    }


    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_X35X0_LED_MIX_CTRL, regValue));

    return (MTD_OK);
    
}


/******************************************************************************
MTD_STATUS mtdX35X0LedSetTimerControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 speedOnPulse,
    IN MTD_U16 speedOffPulse,
    IN MTD_U16 blinkRate1,
    IN MTD_U16 blinkRate2,
    IN MTD_U16 pulseStretchDuration
)


 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    speedOnPulse - Selects on pulse period
        MTD_X35X0_LED_PERIOD_81MS
        MTD_X35X0_LED_PERIOD_161MS
        MTD_X35X0_LED_PERIOD_322MS
        MTD_X35X0_LED_PERIOD_81MS
        or
        MTD_X35X0_LED_DO_NOT_CHANGE
    speedOffPulse - Selects off pulse period
        MTD_X35X0_LED_PERIOD_81MS
        MTD_X35X0_LED_PERIOD_161MS
        MTD_X35X0_LED_PERIOD_322MS
        MTD_X35X0_LED_PERIOD_81MS
        or
        MTD_X35X0_LED_DO_NOT_CHANGE
    blinkRate1 - Selects first blink rate   
        MTD_X35X0_LED_RATE_40MS
        MTD_X35X0_LED_RATE_81MS
        MTD_X35X0_LED_RATE_161MS
        MTD_X35X0_LED_RATE_322MS 
        MTD_X35X0_LED_RATE_644MS
        MTD_X35X0_LED_RATE_1P3S
        MTD_X35X0_LED_RATE_2P6S
        MTD_X35X0_LED_RATE_5P2S
        or
        MTD_X35X0_LED_DO_NOT_CHANGE
    blinkRate2 - Selects second blink rate
        MTD_X35X0_LED_RATE_40MS
        MTD_X35X0_LED_RATE_81MS
        MTD_X35X0_LED_RATE_161MS
        MTD_X35X0_LED_RATE_322MS 
        MTD_X35X0_LED_RATE_644MS
        MTD_X35X0_LED_RATE_1P3S
        MTD_X35X0_LED_RATE_2P6S
        MTD_X35X0_LED_RATE_5P2S
        or
        MTD_X35X0_LED_DO_NOT_CHANGE
    pulseStretchDuration - Selects pulse stretch
        MTD_X35X0_LED_STRETCH_NONE
        MTD_X35X0_LED_STRETCH_20_40MS
        MTD_X35X0_LED_STRETCH_40_81MS
        MTD_X35X0_LED_STRETCH_81_161MS
        MTD_X35X0_LED_STRETCH_161_322MS
        MTD_X35X0_LED_STRETCH_322_644MS
        MTD_X35X0_LED_STRETCH_644_1P3S
        MTD_X35X0_LED_STRETCH_1P3_2P6S
        or
        MTD_X35X0_LED_DO_NOT_CHANGE

 Outputs:
    none

 Returns:
    MTD_OK if LEDs are successfully configured
    MTD_FAIL if the device is not supported or other failure

 Description:
    Configures the LED timer control register.

 Side effects:
    None

 Notes/Warnings:
    

    
*******************************************************************************/
MTD_STATUS mtdX35X0LedSetTimerControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 speedOnPulse,
    IN MTD_U16 speedOffPulse,
    IN MTD_U16 blinkRate1,
    IN MTD_U16 blinkRate2,
    IN MTD_U16 pulseStretchDuration
)
{
    MTD_U16 regValue;

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_X35X0_LED_TIMER_CTRL, &regValue));

    if (speedOnPulse != MTD_X35X0_LED_DO_NOT_CHANGE)
    {
        if (speedOnPulse <= MTD_X35X0_LED_PERIOD_81MS)
        {
            MTD_ATTEMPT(mtdHwSetRegFieldToWord(regValue, speedOnPulse, MTD_X35X0_LED_SPDON_BOFFSET, MTD_X35X0_LED_SPDON_BLENGTH, &regValue));
        }
        else
        {
            return MTD_FAIL; /* speed on pulse period out of range */
        }
    }

    if (speedOffPulse != MTD_X35X0_LED_DO_NOT_CHANGE)
    {
        if (speedOffPulse <= MTD_X35X0_LED_PERIOD_81MS)
        {
            MTD_ATTEMPT(mtdHwSetRegFieldToWord(regValue, speedOffPulse, MTD_X35X0_LED_SPDOFF_BOFFSET, MTD_X35X0_LED_SPDOFF_BLENGTH, &regValue));
        }
        else
        {
            return MTD_FAIL; /* speed off pulse period out of range */
        }
    }

    if (blinkRate1 != MTD_X35X0_LED_DO_NOT_CHANGE)
    {
        if (blinkRate1 <= MTD_X35X0_LED_RATE_5P2S)
        {
            MTD_ATTEMPT(mtdHwSetRegFieldToWord(regValue, blinkRate1, MTD_X35X0_LED_BLINK1_BOFFSET, MTD_X35X0_LED_BLINK1_BLENGTH, &regValue));
        }
        else
        {
            return MTD_FAIL; /* blink rate 1 out of range */
        }
    }

    if (blinkRate2 != MTD_X35X0_LED_DO_NOT_CHANGE)
    {
        if (blinkRate2 <= MTD_X35X0_LED_RATE_5P2S)
        {
            MTD_ATTEMPT(mtdHwSetRegFieldToWord(regValue, blinkRate2, MTD_X35X0_LED_BLINK2_BOFFSET, MTD_X35X0_LED_BLINK2_BLENGTH, &regValue));
        }
        else
        {
            return MTD_FAIL; /* blink rate 2 out of range */
        }
    }

    if (pulseStretchDuration != MTD_X35X0_LED_DO_NOT_CHANGE)
    {
        if (pulseStretchDuration <= MTD_X35X0_LED_STRETCH_1P3_2P6S)
        {
            MTD_ATTEMPT(mtdHwSetRegFieldToWord(regValue, pulseStretchDuration, MTD_X35X0_LED_STRETCH_BOFFSET, MTD_X35X0_LED_STRETCH_BLENGTH, &regValue));
        }
        else
        {
            return MTD_FAIL; /* pulse stretch duration out of range */
        }
    }

    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_X35X0_LED_TIMER_CTRL, regValue));

    return (MTD_OK);
    
}


/******************************************************************************
MTD_STATUS mtdX35X0EnableSelectRCLK
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 copperOrFiber
)

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, must be an even port number, 0,2,4...30
    copperOrFiber - which data path is active, copper or fiber, one of:
        MTD_X35X0_AMD_SEL_CU_RCLK, to selects passed in port's copper port as source 
        MTD_X35X0_AMD_SEL_FI_RCLK, to selects passed in (port+1)'s fiber port as source 
        MTD_X35X0_AMD_SEL_NO_RCLK, to disable the RCLK
        
 Outputs:
    none

 Returns:
    MTD_OK if successful
    MTD_FAIL if there are any errors

 Description:
    This function is to be called in AMD (auto-media detect) mode
    to enable and select the RCLK output from either the copper side or the
    fiber side data connection.

 Side effects:
    None

 Notes/Warnings:
    The mode is not error-checked. This function is not a general purpose
    function. It's to be used in AMD mode to enable the RCLK and select the
    source as either the copper port on the even port, or the fiber port
    on the even port + 1.

    This is for the mode where the P0/P2's H unit is connected to the host, 
    the copper is connected to P0/P2 and the fiber is connected to P1/P3's 
    H unit.
    
*******************************************************************************/
MTD_STATUS mtdX35X0EnableSelectRCLK
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 copperOrFiber
)
{
    if ((port % 2) || ((copperOrFiber != MTD_X35X0_AMD_SEL_CU_RCLK) && 
                       (copperOrFiber != MTD_X35X0_AMD_SEL_FI_RCLK) && 
                       (copperOrFiber != MTD_X35X0_AMD_SEL_NO_RCLK)))
    {
        /* Odd port number, or a bad value for */
        MTD_DBG_ERROR("mtdX35X0EnableSelectRCLK: Out of range parameter!\n");
        return MTD_FAIL;    
    }

    /* Disable both ports RCLK to start with, both 
       will be left disabled if MTD_X35X0_AMD_SEL_NO_RCLK selected, sets both
       back to default of disabled and T unit */
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, 31, 0xF002, 0, 2, MTD_DISABLE));        
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port+1, 31, 0xF002, 0, 2, MTD_DISABLE));    

    if (copperOrFiber == MTD_X35X0_AMD_SEL_CU_RCLK)
    {
        /* select t unit on port as source */    
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, 31, 0xF002, 1, 1, 0));

        /* enable first port, leave second's RCLK disabled */
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, 31, 0xF002, 0, 1, MTD_ENABLE));        
    

    }
    else if (copperOrFiber == MTD_X35X0_AMD_SEL_FI_RCLK)
    {
        /* select fiber on port+1 as source */    
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, 31, 0xF002, 1, 1, 1));    

        /* enable secpnd port, leave first's RCLK disabled */        
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, 31, 0xF002, 0, 1, MTD_ENABLE));    
    }

    return MTD_OK;
}


/******************************************************************************
MTD_STATUS mtdSetX35X0OxgmiiWithRsfecDisabled
(
    IN MTD_DEV_PTR devPtrOxgmii,
    IN MTD_DEV_PTR devPtrOxgmiiPartner,
    IN MTD_U16 oxgmiiPort,
    IN MTD_U16 oxgmiiPartnerPort
)

 Inputs:
    devPtrOxgmii - allocated memory for the device structure of the OXGMII 
    devPtrOxgmiiPartner - allocated memory for the device structure of the OXGMII partner
    oxgmiiPort - first MDIO port address of the OXGMII primary quad-port device
    oxgmiiPartnerPort - first MDIO port address of the OXGMII partner quad-port device

 Outputs:
    None

 Returns:
    Returns MTD_OK or MTD_FAIL 
    
 Description:
    RS-FEC is not allowed to be disabled through API in OXGMII mode for X35X0/E25X0.
    This sample shows how to manually configure OXGMII mode for X35X0/E25X0 with 
    RS-FEC disabled. RS-FEC is enabled by default, disable it only when the link partner 
    don't support this feature.


 Side effects:
    None

 Notes/Warnings:
    None

*******************************************************************************/
MTD_STATUS mtdSetX35X0OxgmiiWithRsfecDisabled
(
    IN MTD_DEV_PTR devPtrOxgmii,
    IN MTD_DEV_PTR devPtrOxgmiiPartner,
    IN MTD_U16 oxgmiiPort,
    IN MTD_U16 oxgmiiPartnerPort
)
{
    MTD_U16 idx;

    /* set OXGMII mac type to the OXGMII primary quad-port core */
    /* select appropriate configuration for the PHY being configured */
    MTD_ATTEMPT(mtdSetMacInterfaceCopperOnlyPhy(devPtrOxgmii, oxgmiiPort, MTD_MAC_TYPE_OXGMII, MTD_TRUE, MTD_MAC_SPEED_NOT_APPLICABLE, MTD_MAX_MAC_SPEED_NOT_APPLICABLE));

    /* disable RS-FEC in all 4 ports in the primary core */
    for (idx = 0; idx < 4; idx++)
    {
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtrOxgmii, oxgmiiPort + idx, MTD_CUNIT_PORT_CTRL2, 13, 2, ((1<<1)|0)));
    }

    /* c unit need to apply a sw reset to take effect, only need to apply to the primary port of the group */
    MTD_ATTEMPT(mtdCunitSwReset(devPtrOxgmii, oxgmiiPort));

    /* set QXGMII mac type to the OXGMII partner core */  
    /* select appropriate configuration for the PHY being configured */
    MTD_ATTEMPT(mtdSetMacInterfaceCopperOnlyPhy(devPtrOxgmiiPartner, oxgmiiPartnerPort, MTD_MAC_TYPE_QXGMII, MTD_TRUE, MTD_MAC_SPEED_NOT_APPLICABLE, MTD_MAX_MAC_SPEED_NOT_APPLICABLE));

    return MTD_OK;
}


/******************************************************************************
MTD_STATUS mtdX35X0OxgmiiDisableRsfec
(
    IN MTD_DEV_PTR devPtrOxgmii,
    IN MTD_DEV_PTR devPtrOxgmiiPartner,
    IN MTD_U16 oxgmiiPort,
    IN MTD_U16 oxgmiiPartnerPort
)

 Inputs:
    devPtrOxgmii - allocated memory for the device structure of the OXGMII 
    devPtrOxgmiiPartner - allocated memory for the device structure of the OXGMII partner
    oxgmiiPort - first MDIO port address of the OXGMII primary quad-port device
    oxgmiiPartnerPort - first MDIO port address of the OXGMII partner quad-port device

 Outputs:
    None

 Returns:
    Returns MTD_OK or MTD_FAIL 
    
 Description:
    RS-FEC is enabled by default and not allowed to be disabled through API in OXGMII 
    mode for X35X0/E25X0. This sample shows how to disable RS-FEC on OXGMII mode for 
    X35X0/E25X0. it assume the mac type has been set previously on the OXGMII ports and 
    OMXGII partner ports.


 Side effects:
    None

 Notes/Warnings:
    None

*******************************************************************************/
MTD_STATUS mtdX35X0OxgmiiDisableRsfec
(
    IN MTD_DEV_PTR devPtrOxgmii,
    IN MTD_DEV_PTR devPtrOxgmiiPartner,
    IN MTD_U16 oxgmiiPort,
    IN MTD_U16 oxgmiiPartnerPort
)
{
    MTD_U16 idx;

    /* disable RS-FEC in all 4 ports in the OXGMII ports */
    for (idx = 0; idx < 4; idx++)
    {
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtrOxgmii, oxgmiiPort + idx, MTD_CUNIT_PORT_CTRL2, 13, 2, ((1<<1)|0)));
    }

    /* c unit need to apply a sw reset to take effect, only need to apply to the primary port of the group */
    MTD_ATTEMPT(mtdCunitSwReset(devPtrOxgmii, oxgmiiPort));

    /* disable PARTNER bit in the oxgmiiPartnerPorts */
    for (idx = 0; idx < 4; idx++)
    {
        MTD_ATTEMPT(mtdHwSetPhyRegField(devPtrOxgmiiPartner,oxgmiiPartnerPort + idx,4,0xF0AC,13,1,0x0));
    }

    /* c unit need to apply a sw reset to take effect, only need to apply to the primary port of the group */
    MTD_ATTEMPT(mtdCunitSwReset(devPtrOxgmiiPartner, oxgmiiPartnerPort));

    return MTD_OK;

}
/******************************************************************************
MTD_STATUS mtdSampleHXUnitPktGenChk
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXUnit
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31
    HorXunit - MTD_X_UNIT or MTD_H_UNIT, depending on which interface is
               being read

 Outputs:
    None.

 Returns:
    MTD_OK if successfully
    MTD_FAIL if failed

 Description:
    This sample function provides a work flow for using H/X Unit packet generator/checker.
    The packet generator/checker MUST be enabled for read-to-clear counter or
    using mtdPktGeneratorCounterReset() to reset counters.

    Users can start packet generator with traffic running or start traffic later
    by calling mtdStartStopPktGeneratorTraffic(). This is controlled by numPktsToSend
    filed in the mtdConfigurePktGeneratorChecker()

    One traffic must be marked as "stopped" by setting numPktsToSend=0 in the
    mtdStartStopPktGeneratorTraffic() before starting the next traffic.

    Related APIs:
        mtdConfigurePktGeneratorChecker()
        mtdGetPktGeneratorCheckerConfig()
        mtdEnablePktGeneratorChecker()
        mtdStartStopPktGeneratorTraffic()
        mtdPktGeneratorCounterReset()
        mtdPktGeneratorCheckerGetLinkDrop()

 Side effects:
    None.

 Notes/Warnings:
    None.
*******************************************************************************/
MTD_STATUS mtdSampleHXUnitPktGenChk
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 HorXUnit
)
{
    /* Variables for packet generator */
    MTD_U64 counters[2][3];      /* [pkt/byte][tx/rx/err] */
    MTD_U16 counterIndex;
    MTD_U16 linkdropcounter;
    MTD_U16 pktCntIdx = 0, byteCntIdx = 1;

    /* Please refers to the header of mtdConfigurePktGeneratorChecker() for details */
    MTD_BOOL readToClear = MTD_FALSE;    /* Disable the read-to-clear feature for all counters */
    MTD_BOOL dontuseSFDinChecker = MTD_FALSE;
    MTD_U16 pktPatternControl = MTD_PKT_RANDOM_BYTE;
    MTD_BOOL generateCRCoff = MTD_FALSE;
    MTD_U32 initialPayload = 0x0;
    MTD_U16 frameLengthControl = MTD_PKT_RAND_LEN0; /* Please refers to definitions in the mtdDiagnostics.h */
    MTD_U16 numPktsToSend = 0x0000; /* 0x1 - 0xFFFE is fixed number of packets, 0xFFFF to send continuously */
    MTD_BOOL randomIPG = MTD_FALSE;
    MTD_U16 ipgDuration = 40;

    /* Start generator and checker, no packet is generating */
    MTD_ATTEMPT(mtdConfigurePktGeneratorChecker(devPtr, port, HorXUnit, readToClear, dontuseSFDinChecker, pktPatternControl,
                                                generateCRCoff, initialPayload, frameLengthControl, numPktsToSend, randomIPG, ipgDuration));

    /* User can also start with preset numPktsToSent, and the traffic will start immediately */
    /* after calling this function */
    MTD_ATTEMPT(mtdEnablePktGeneratorChecker(devPtr, port, HorXUnit, MTD_TRUE, MTD_TRUE));

    /* This API only works when read-to-clear is disabled */
    MTD_ATTEMPT(mtdPktGeneratorCounterReset(devPtr, port, HorXUnit));

    /* OR */
    /* Enable read-to-clear feature */
    /* Either way of clearing counter only take effect after enabling the packet generator/checker */
    MTD_ATTEMPT(mtdConfigurePktGeneratorChecker(devPtr, port, HorXUnit, readToClear, dontuseSFDinChecker, pktPatternControl,
                                                generateCRCoff, initialPayload, frameLengthControl, numPktsToSend, randomIPG, ipgDuration));

    /* Read to clear counters */
    for (counterIndex = MTD_PKT_GET_TX; counterIndex < (MTD_PKT_GET_ERR+1); counterIndex++)
    {
        MTD_ATTEMPT(mtdPktGeneratorGetCounter(devPtr, port, HorXUnit, counterIndex,
                                              &counters[pktCntIdx][counterIndex], &counters[byteCntIdx][counterIndex]));
    }

    /* Start the First traffic */
    numPktsToSend = 0x0404; /* the number of packers to send */
    /* Stop traffic after generating numPktsToSend packets */
    MTD_ATTEMPT(mtdStartStopPktGeneratorTraffic(devPtr, port, HorXUnit, numPktsToSend));

    /* allow some time pass to allow new packets/bytes to be sent/received */
    MTD_ATTEMPT(mtdWait(devPtr, 1000));    /* 1 sec is just an example */

    /* Setting 0x0000 is necessary to finish last traffic and start a new traffic */
    /* Even there is no more packet generating from last traffic */
    /* Recommend always do this for good practice */
    MTD_ATTEMPT(mtdStartStopPktGeneratorTraffic(devPtr, port, HorXUnit, 0x0000));

    /* Start the Second traffic */
    numPktsToSend = 0x6666; /* the number of packers to send */
    /* Stop traffic after generating numPktsToSend packets */
    MTD_ATTEMPT(mtdStartStopPktGeneratorTraffic(devPtr, port, HorXUnit, numPktsToSend));

    /* allow some time pass to allow new packets/bytes to be received */
    MTD_ATTEMPT(mtdWait(devPtr, 1000));    /* 1 sec is just an example */

    /* Stop packet generator/checker */
    MTD_ATTEMPT(mtdEnablePktGeneratorChecker(devPtr, port, HorXUnit, MTD_FALSE, MTD_FALSE));

    /* Read Generator(TX) counter, Checker(RX) counter and error counter */
    for (counterIndex = MTD_PKT_GET_TX; counterIndex < (MTD_PKT_GET_ERR+1); counterIndex++)
    {
        MTD_ATTEMPT(mtdPktGeneratorGetCounter(devPtr, port, HorXUnit, counterIndex,
                                              &counters[pktCntIdx][counterIndex], &counters[byteCntIdx][counterIndex]));
    }

    /* Read link drop counter */
    MTD_ATTEMPT(mtdPktGeneratorCheckerGetLinkDrop(devPtr, port, HorXUnit, &linkdropcounter));

    return MTD_OK;
}

/******************************************************************************
MTD_STATUS mtdSampleTUnitPktGenChk
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    port - MDIO port address, 0-31

 Outputs:
    None.

 Returns:
    MTD_OK if successfully
    MTD_FAIL if failed

 Description:
    This sample function provides a work flow for using T Unit packet generator/checker.
    The packet generator/checker MUST be enabled for read-to-clear counter or
    using mtdTunitPktGeneratorCounterReset() to reset counters.

    Users can start packet generator with traffic running or start traffic later
    by calling mtdTunitStartStopPktGenTraffic(). This is controlled by numPktsToSend
    filed in the mtdTunitConfigurePktGeneratorChecker()

    One traffic must be marked as "stopped" by setting numPktsToSend=0 in the
    mtdTunitStartStopPktGenTraffic() before starting the next traffic.

    10M/100M/1G use different packet generators than speed 2.5G/5G/10G
    The API is designed to help use them both easily.

    Please refers to the headers of mtdTunitConfigurePktGeneratorChecker() for the main
    differences between two packet generators.

    The T-Unit counters should only be read when the packet generator/checker is enabled.

    Related APIs:
        mtdTunitConfigurePktGeneratorChecker()
        mtdTunit10GPktGenMuxSelect()
        mtdTunitEnablePktGeneratorChecker()
        mtdTunitPktGeneratorCounterReset()
        mtdTunitPktGeneratorGetCounter()
        mtdTunitStartStopPktGenTraffic()


 Side effects:
    None.

 Notes/Warnings:
    None.
*******************************************************************************/
MTD_STATUS mtdSampleTUnitPktGenChk
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
)
{

    /* Variables for packet generator */
    MTD_U64 counters[2][3];      /* [pkt/byte][tx/rx/err] */
    MTD_U16 counterIndex;
    MTD_U16 pktCntIdx = 0, byteCntIdx = 1;
    MTD_U16 speed;

    /* Packet Generator/Checker Configurations */
    MTD_BOOL readToClear;   /* Note: Read-to-clear is NOT available on the 10M/100M/1G packet generator. */
    MTD_U16  pktPatternControl = MTD_PKT_NO_MASK;
    MTD_BOOL generateCRCoff = MTD_FALSE;
    MTD_U32  initialPayload = 0x5555aaaa;
    MTD_U16  frameLengthControl = MTD_PKT_RAND_LEN0; /* Please refers to definitions in the mtdDiagnostics.h */
    MTD_U16  numPktsToSend; /* 0x1 - 0xFFFE is fixed number of packets, 0xFFFF to send continuously */
    MTD_BOOL randomIPG = MTD_TRUE;
    MTD_U16  ipgDuration = 0x1;
    MTD_BOOL clearInitCounters = MTD_TRUE;  /* clear counters before start */

    /* 2.5G/5G/10G Packet generator direction control */
    /* See mtdTunit10GPktGenMuxSelect() headers for value definitions */
    MTD_U16  generatorControl;
    MTD_U16  checkerControl;

    /* Variables for T-Unit link polling */
    MTD_U16 timeCount = 0;
    MTD_U16 baseTSpeed;
    MTD_BOOL islinkup;
    MTD_U16 speeds_available;

    /* set devices to its highest speed, an example */
    MTD_ATTEMPT(mtdGetSpeedsAvailable(devPtr, port, &speeds_available));

    /* If 10M/100M/1G speed is linked up, API will use different registers */
    /* to control a different packet generator, read-to-clear will NOT work */
    /* for 10M/100M/1G speed */
    if (speeds_available & MTD_SPEED_10GIG_FD)
    {
        speed = MTD_SPEED_10GIG_FD;
    }
    else
    {
        speed = MTD_SPEED_5GIG_FD;
    }

    /* Set cooper link and restart link */
    MTD_ATTEMPT(mtdEnableSpeeds(devPtr, port, speed, MTD_TRUE));

    /* The cooper link MUST be up at the speed configured */
    /* before enabling the packet generator/checker and MUST stay up */

    /* Wait for AN finish and Cu link come up */
    timeCount = 0;
    do
    {
        timeCount++;
        MTD_ATTEMPT(mtdWait(devPtr, 1000));
        MTD_ATTEMPT(mtdIsBaseTUp(devPtr, port, &baseTSpeed, &islinkup));
    } while ((islinkup == MTD_FALSE) && timeCount < 8);

    if (!(islinkup == MTD_TRUE && baseTSpeed == speed))
    {
        return MTD_FAIL;
    }

    /* Start with readToClear off */
    readToClear = MTD_FALSE;
    numPktsToSend = 0x1454; /* finite number of packets */
    /* The traffic will start immediately after packet generator is enabled */
    /* The traffic will stop once h'0x1454(d'5204) packets has been sent. */

    /* Configure Packet generator/checker */
    MTD_ATTEMPT(mtdTunitConfigurePktGeneratorChecker(devPtr, port, speed, readToClear, pktPatternControl,
                                                     generateCRCoff, initialPayload, frameLengthControl,
                                                     numPktsToSend, randomIPG, ipgDuration, clearInitCounters));

    /* The 2.5G/5G/10G packet generator can send data towards the line, MAC or both */
    /* The 10M/100M/1G packet generator can only send data towards the line. */
    generatorControl = MTD_PACKETGEN_TO_LINE;
    checkerControl = MTD_CHECKER_FROM_LINE;
    /* Note: Calling this function for 1G and below speed will result unpredictable result! */
    if ((speed == MTD_SPEED_10GIG_FD) || (speed == MTD_SPEED_2P5GIG_FD) || (speed == MTD_SPEED_5GIG_FD))
    {
        MTD_ATTEMPT(mtdTunit10GPktGenMuxSelect(devPtr, port, generatorControl, checkerControl));
    }

    /* Enable the packet generator and checker */
    MTD_ATTEMPT(mtdTunitEnablePktGeneratorChecker(devPtr, port, speed, MTD_TRUE, MTD_TRUE));

    /* allow some time pass to allow new packets/bytes to be sent/received */
    MTD_ATTEMPT(mtdWait(devPtr, 1000));    /* 1 sec is just an example */

    /* Read counters */
    /* T Unit counters should only be read when the packet generator/checker is enabled */
    for (counterIndex = MTD_PKT_GET_TX; counterIndex < MTD_PKT_GET_ERR+1; counterIndex++)
    {
        MTD_ATTEMPT(mtdTunitPktGeneratorGetCounter(devPtr, port, speed, counterIndex,
                                                   &counters[pktCntIdx][counterIndex], &counters[byteCntIdx][counterIndex]));
    }

    /* Notes: For the 10M/100M/1G packet generator/checker, this is the only way to clear the counters */
    /* Noticed that no traffic is running now since we have only sent finite number of packets */
    MTD_ATTEMPT(mtdTunitPktGeneratorCounterReset(devPtr, port, speed));

    /* Either way of clearing counter only take effect after enabling the packet generator/checker */
    {
        /* OR */
        /* If read-to-clear feature was enabled above in the mtdTunitConfigurePktGeneratorChecker() */
        /* DO NOT call mtdTunitConfigurePktGeneratorChecker() after enabling the T Unit packet generator/checker */
        /* to change the read-to-clear settings, which will disable the generator/checker */

        /* Read to clear counter */
        /* Note: There is NO read-to-clear functionality for 10M/100M/1G packet generator */
        for (counterIndex = MTD_PKT_GET_TX; counterIndex < MTD_PKT_GET_ERR+1; counterIndex++)
        {
            MTD_ATTEMPT(mtdTunitPktGeneratorGetCounter(devPtr, port, speed, counterIndex,
                                                       &counters[pktCntIdx][counterIndex], &counters[byteCntIdx][counterIndex]));
        }
    }

    /* Note: The only way to start a new traffic without disable-then-enable packet generator is to */
    /*       set numPktsToSend to 0 to mark last traffic as "stopped" first then set it to a new number */
    numPktsToSend = 0x0;
    MTD_ATTEMPT(mtdTunitStartStopPktGenTraffic(devPtr, port, speed, numPktsToSend))
    numPktsToSend = 0x1234;
    MTD_ATTEMPT(mtdTunitStartStopPktGenTraffic(devPtr, port, speed, numPktsToSend))

    /* Stop packet generator/checker */
    MTD_ATTEMPT(mtdTunitEnablePktGeneratorChecker(devPtr, port, speed, MTD_FALSE, MTD_FALSE));

   return MTD_OK;
}

#if MTD_CE_SERDES28X2
/******************************************************************************
MTD_STATUS mtdSampleCall28X2SerdesAPI
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    mdioPort - MDIO port address, 0-31 

 Outputs:
    None.

 Returns:
    MTD_OK or MTD_FAIL, if query was successful or not

 Description:
    This sample code calls COMPHY Serdes API to return temperature read on serdes sensor
    as an example.

    To correctly call COMPHY API, mtdSetSerdesPortGetDevPtr() is called.
    These two function MUST be called prior any of the COMPHY Serdes API calls. Because there is a
    mapping between passed-in mdioPort and the devPtr->serdesPort, which control different
    serdes for different mdioPorts.

    On the X3540, which has 28X2 COMPHY serdes:
    serdesPort = 0 if mdioPort = 0 or 1 (relative port number)
    serdesPort = 2 if mdioPort = 2 or 3 (relative port number)
    The actually physical port calculation is done in the mtdSetSerdesPortGetDevPtr(),
    which also serves other devices.

 Side effects:
    None.

 Notes/Warnings:
    This sample code is only applicable to the PHYs with COMPHY serdes.
    mtdSetSerdesPortGetDevPtr is called in this function and it
    assume that one of the COMPHY packages is selected in the mtdFeatures.h

    This is a non-reentrant function. mtdSetSerdesPortGetDevPtr(), mtdGetSerdesDevPtr()
    or mtdSetSerdesPort() shall not be called in other places before calling the following
    API_C28GP4X2_GetTemperature() function.
*******************************************************************************/
MTD_STATUS mtdSampleCall28X2SerdesAPI
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort
)
{
    MCESD_DEV_PTR pSerdesDev;
    MCESD_32 temperature;

    MTD_ATTEMPT(mtdSetSerdesPortGetDevPtr(devPtr, mdioPort, &pSerdesDev));

    if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        MTD_ATTEMPT(API_C28GP4X2_GetTemperature(pSerdesDev, &temperature));
        MTD_DBG_INFO("mtdSampleCall28X2SerdesAPI: Serdes Temperature on Port%d is %.2f C.\n", mdioPort, (double)temperature/1000);
    }
    else
    {
        MTD_DBG_ERROR("mtdSampleCall28X2SerdesAPI: Device doesn't support any of the COMPHY Serdes.\n");
        return MTD_FAIL;
    }

    return MTD_OK;
}

/******************************************************************************
MTD_STATUS mtdSerdesSample28X2PRBS
(
    IN MTD_DEV_PTR devPtr0, 
    IN MTD_U16 mdioPort0,
    IN MTD_DEV_PTR devPtr1, 
    IN MTD_U16 mdioPort1
);

Inputs:
    devPtr0 - pointer of the first port/device to MTD_DEV initialized by mtdLoadDriver() call
    mdioPort0 - MDIO port address 0, 0-31
    devPtr1 - pointer of the second port/device to MTD_DEV initialized by mtdLoadDriver() call
    mdioPort1 - MDIO port address 1, 0-31

Outputs:
    None.

Returns:
    MTD_OK or MTD_FAIL, if test was successful or not (No error bits / pass and lock are TRUE)

Description:
    This sample code calls COMPHY Serdes API to perform Serdes PRBS test.

    To correctly call COMPHY API, mtdSetSerdesPortGetDevPtr() is called to setup the Serdes access
    fields.  These function MUST be called prior any of the COMPHY Serdes API calls. Because there 
    is a mapping between passed-in mdioPort and the devPtr->serdesPort, which control different
    serdes for different mdioPorts.

    On the X3540, which has 28X2 COMPHY serdes:
    serdesPort = 0 if mdioPort = 0 or 1 (relative port number)
    serdesPort = 2 if mdioPort = 2 or 3 (relative port number)
    The actually physical port calculation is done in the mtdSetSerdesPortGetDevPtr(),
    which also serves other devices.

    For the setup, connect a SFP28 cable between the two serdes ports.

Side effects:
    None.

Notes/Warnings:
    This sample code is only applicable to the PHYs with COMPHY serdes.
    mtdSetSerdesPortGetDevPtr is called in this function and it
    assume that one of the COMPHY packages is selected in the mtdFeatures.h

    This is a non-reentrant function. mtdSetSerdesPortGetDevPtr(), mtdGetSerdesDevPtr()
    or mtdSetSerdesPort() shall not be called in other places before calling the following
    COMPHY Serdes APIs.

    This sample is written for testing between two ports on a single 8-port board (X3580) or
    two ports on two boards. mtdSetSerdesPort is needed to switch between ports when testing
    on one board.
*******************************************************************************/

MTD_STATUS mtdSerdesSample28X2PRBS
(
    IN MTD_DEV_PTR devPtr0, 
    IN MTD_U16 mdioPort0,
    IN MTD_DEV_PTR devPtr1, 
    IN MTD_U16 mdioPort1
)
{
    MCESD_DEV_PTR pSerdesDev0, pSerdesDev1 = NULL;
    S_C28GP4X2_PATTERN txPattern, rxPattern;
    S_C28GP4X2_PATTERN_STATISTICS stats0, stats1;
    MCESD_U8 serdesLane0, serdesLane1;

    /* Even ports set even lanes, odd ports set odd lanes */
    serdesLane0 = (mdioPort0%2);
    serdesLane1 = (mdioPort1%2);

    /* Set Serdes Dev Pointers */
    MTD_ATTEMPT(mtdSetSerdesPortGetDevPtr(devPtr0, mdioPort0, &pSerdesDev0));
    MTD_ATTEMPT(mtdSetSerdesPortGetDevPtr(devPtr1, mdioPort1, &pSerdesDev1));
        
    /* PRBS Init - freeze PCS */
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr0, mdioPort0, 4, 0xF08D, 0x0016));
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr1, mdioPort1, 4, 0xF08D, 0x0016));

    /* Set path, configure TX and RX pattern to PRBS31, start PHY Test: Pattern Generator and Pattern Comparator */
    txPattern.pattern = C28GP4X2_PAT_PRBS31;
    txPattern.enc8B10B = C28GP4X2_ENC_8B10B_DISABLE;
    rxPattern.pattern = C28GP4X2_PAT_PRBS31;
    rxPattern.enc8B10B = C28GP4X2_ENC_8B10B_DISABLE;

    MTD_ATTEMPT(mtdSetSerdesPort(devPtr0, mdioPort0));
    MTD_ATTEMPT((MTD_STATUS)API_C28GP4X2_SetDataPath(pSerdesDev0, serdesLane0, C28GP4X2_PATH_EXTERNAL));
    MTD_ATTEMPT((MTD_STATUS)API_C28GP4X2_SetTxRxPattern(pSerdesDev0, serdesLane0, &txPattern, &rxPattern, C28GP4X2_SATA_NOT_USED, C28GP4X2_DISPARITY_NOT_USED, "", C28GP4X2_PAT_USER));
    MTD_ATTEMPT((MTD_STATUS)API_C28GP4X2_StartPhyTest(pSerdesDev0, serdesLane0));

    MTD_ATTEMPT(mtdSetSerdesPort(devPtr1, mdioPort1));
    MTD_ATTEMPT((MTD_STATUS)API_C28GP4X2_SetDataPath(pSerdesDev1, serdesLane1, C28GP4X2_PATH_EXTERNAL));
    MTD_ATTEMPT((MTD_STATUS)API_C28GP4X2_SetTxRxPattern(pSerdesDev1, serdesLane1, &txPattern, &rxPattern, C28GP4X2_SATA_NOT_USED, C28GP4X2_DISPARITY_NOT_USED, "", C28GP4X2_PAT_USER));
    MTD_ATTEMPT((MTD_STATUS)API_C28GP4X2_StartPhyTest(pSerdesDev1, serdesLane1));

    /* Initiate Training */
    MTD_ATTEMPT(mtdSetSerdesPort(devPtr0, mdioPort0));
    MTD_ATTEMPT((MTD_STATUS)API_C28GP4X2_ExecuteTraining(pSerdesDev0, serdesLane0, C28GP4X2_TRAINING_RX));

    MTD_ATTEMPT(mtdSetSerdesPort(devPtr1, mdioPort1));         
    MTD_ATTEMPT((MTD_STATUS)API_C28GP4X2_ExecuteTraining(pSerdesDev1, serdesLane1, C28GP4X2_TRAINING_RX));
    
    MTD_ATTEMPT(mtdWait(devPtr0, 1000));

    /* Clear Comparator stats */
    MTD_ATTEMPT(mtdSetSerdesPort(devPtr0, mdioPort0));
    MTD_ATTEMPT((MTD_STATUS)API_C28GP4X2_ResetComparatorStats(pSerdesDev0, serdesLane0));

    MTD_ATTEMPT(mtdSetSerdesPort(devPtr1, mdioPort1));
    MTD_ATTEMPT((MTD_STATUS)API_C28GP4X2_ResetComparatorStats(pSerdesDev1, serdesLane1));

    /* Wait 2 seconds to accumulate statistics */
    MTD_ATTEMPT(mtdWait(devPtr0, 2000));

    /* Retrieve statistics */
    MTD_ATTEMPT(mtdSetSerdesPort(devPtr0, mdioPort0));
    MTD_ATTEMPT((MTD_STATUS)API_C28GP4X2_GetComparatorStats(pSerdesDev0, serdesLane0, &stats0));
    
    MTD_ATTEMPT(mtdSetSerdesPort(devPtr1, mdioPort1));
    MTD_ATTEMPT((MTD_STATUS)API_C28GP4X2_GetComparatorStats(pSerdesDev1, serdesLane1, &stats1));

    /* Stop PHY Test */
    MTD_ATTEMPT(mtdSetSerdesPort(devPtr0, mdioPort0));
    MTD_ATTEMPT((MTD_STATUS)API_C28GP4X2_StopPhyTest(pSerdesDev0, serdesLane0));

    MTD_ATTEMPT(mtdSetSerdesPort(devPtr1, mdioPort1));
    MTD_ATTEMPT((MTD_STATUS)API_C28GP4X2_StopPhyTest(pSerdesDev1, serdesLane1));

    /* Check statistics */
    if (((stats0.totalBits != 0) && (stats0.totalErrorBits == 0) && (stats1.totalBits != 0) && (stats1.totalErrorBits == 0)) &&
        ((stats0.pass == MCESD_TRUE) && (stats0.lock == MCESD_TRUE) && (stats1.pass == MCESD_TRUE) && (stats1.lock == MCESD_TRUE)))
    {
        return MTD_OK;
    }
    else
    {
        MTD_DBG_ERROR("Error occurred during mtdSerdesSample28X2PRBS Test.\n");
        MTD_DBG_ERROR("1st Port Comparator Statistics: Total Bits: %llu, Total Errors: %llu, Pass: %s, Locked: %s\n", (unsigned long long)stats0.totalBits,
            (unsigned long long)stats0.totalErrorBits, (stats0.pass == MCESD_TRUE) ? "TRUE" : "FALSE", (stats1.lock == MCESD_TRUE) ? "TRUE" : "FALSE");

        MTD_DBG_ERROR("2nd Port Comparator Statistics: Total Bits: %llu, Total Errors: %llu, Pass: %s, Locked: %s\n", (unsigned long long)stats1.totalBits,
            (unsigned long long)stats1.totalErrorBits, (stats1.pass == MCESD_TRUE) ? "TRUE" : "FALSE", (stats1.lock == MCESD_TRUE) ? "TRUE" : "FALSE");
        return MTD_FAIL;
    }
    
    return MTD_OK;
}


/******************************************************************************
MTD_STATUS mtdSerdesSample28GX2EyeStats
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort,
    IN MTD_U8 serdesLane
)

 Inputs:
    devPtr - allocated memory for the device structure
    port - MDIO port address, 0-31
    serdesLane - eye data of the lane 

 Outputs:
    None

 Returns:
    Returns MTD_OK or MTD_FAIL if the eye diagram reading or plotting fails
    
 Description:
    Sample code to extract the eye data and plotting eye diagram by calling 
    the mtdSerdes28GX2EyeRawData and mtdSerdes28GX2EyePlotStats APIs.
    The mtdSerdes28GX2EyePlotStats will plot the eye diagram to the buffer defined 
    in the mtdDbgPrint().

 Side effects:
    None

 Notes/Warnings:
    Must wait for link up before extracting eye data.
*******************************************************************************/
MTD_STATUS mtdSerdesSample28GX2EyeStats
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort,
    IN MTD_U8 serdesLane
)
{
    MTD_SERDES_28GX2_EYE_RAW eyeRawData;
    MTD_U16 eyeWidth;
    MTD_U16 eyeHeight;
    MTD_U16 eyeArea;

    MTD_ATTEMPT(mtdSerdes28GX2EyeRawData(devPtr, mdioPort, serdesLane, &eyeRawData));
    MTD_ATTEMPT(mtdSerdes28GX2EyePlotStats(devPtr, &eyeRawData, &eyeWidth, &eyeHeight, &eyeArea));

    return MTD_OK;
}

#endif  /* MTD_CE_SERDES28X2 */

#if MTD_CE_SERDES32X1X4
/******************************************************************************
MTD_STATUS mtdSerdesSample32X1X4Temperature
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    mdioPort - MDIO port address, 0-31 

 Outputs:
    None.

 Returns:
    MTD_OK or MTD_FAIL, if query was successful or not

 Description:
    This sample code calls COMPHY Serdes API to return temperature read on SerDes sensor
    as an example.

    To correctly call COMPHY API, mtdSetSerdesPortGetDevPtr() is called.
    These two function MUST be called prior any of the COMPHY Serdes API calls. Because there is a
    mapping between passed-in mdioPort and the devPtr->serdesPort, which control different
    serdes for different mdioPorts.

 Side effects:
    None.

 Notes/Warnings:
    This sample code is only applicable to the PHYs with COMPHY serdes.
    mtdSetSerdesPortGetDevPtr is called in this function and it
    assume that one of the COMPHY packages is selected in the mtdFeatures.h

    This is a non-reentrant function. mtdSetSerdesPortGetDevPtr(), mtdGetSerdesDevPtr()
    or mtdSetSerdesPort() shall not be called in other places before calling the following
    API_N5C32GP5X4_GetTemperature() function.
*******************************************************************************/
MTD_STATUS mtdSerdesSample32X1X4Temperature
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort
)
{
    MCESD_DEV_PTR pSerdesDev = NULL;
    MCESD_32 temperature = 0;

    MTD_ATTEMPT(mtdSetSerdesPortGetDevPtr(devPtr, mdioPort, &pSerdesDev));

    if (MTD_IS_X36X0_BASE(devPtr->deviceId))
    {

        MTD_ATTEMPT(API_N5C32GP5X4_GetTemperature(pSerdesDev, &temperature));
        MTD_DBG_INFO("mtdSerdesSample32X1X4Temperature: Serdes Temperature on Port%d is %.4f C.\n", mdioPort, (double)temperature/10000);
    }
    else
    {
        MTD_DBG_ERROR("mtdSerdesSample32X1X4Temperature: Device doesn't support any of the COMPHY Serdes.\n");
        return MTD_FAIL;
    }

    return MTD_OK;
}


/******************************************************************************
MTD_STATUS mtdSerdesSample32X1X4GetEyeWidthHeight
    Sample code to measure the eye width and height without plotting the eye diagram.

    These calls API_N5C32GP5X4_EOMGetWidthHeight() and API_N5C32GP5X4_EOMConvertWidthHeight() 
    will return the measurement quicker without plotting the eye diagram.

    The API_N5C32GP5X4_EOMConvertWidthHeight() converts the code words from 
    API_N5C32GP5X4_EOMGetWidthHeight() to width in mUI and height in mV.

    Note: 
    minSamples - minimum number of bits to sample
    berThreshold - Bit Error Rate Threshold in nano (factor of 1E-9)

    Below are some commonly used minSamples/berThreshold values for the various speeds:
    Common range of inputs for minSamples and berThreshold:
    berThreshold = 10000000
    minSamples = 10

    For X3680 multi-ports, the laneOffset maps to the MDIO port accordingly, single port
    X3610 is always laneOffset 0

*******************************************************************************/
MTD_STATUS mtdSerdesSample32X1X4GetEyeWidthHeight
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort,
    IN MTD_U16 host_or_line,
    IN MTD_U16 laneOffset,
    IN MTD_U32 minSamples,
    IN MTD_U32 berThreshold,
    OUT MTD_U16 *eyeWidth,
    OUT MTD_U16 *eyeHeightUpper,
    OUT MTD_U16 *eyeHeightLower,
    OUT MTD_U32 *sampleCount
)
{
    MCESD_DEV_PTR pSerdesDev;

    MTD_ATTEMPT(mtdSetSerdesPortGetDevPtr(devPtr, mdioPort, &pSerdesDev));

    laneOffset = (MTD_U8)((mdioPort / 4) * 4); /* For X3680 multi-ports, the laneOffset maps to the MDIO port accordingly;
                                               single port X3610 is always laneOffset 0 */

    MTD_ATTEMPT(API_N5C32GP5X4_EOMGetWidthHeight(pSerdesDev, (MTD_U8)laneOffset, minSamples, berThreshold, eyeWidth, 
                                                 eyeHeightUpper, eyeHeightLower, sampleCount));

    MTD_DBG_INFO("mtdSerdesSample32X1X4GetEyeWidthHeight width_mUI:%dmUI height_mV upper:%dmV height_mV lower:%dmV\n", 
                 *eyeWidth, *eyeHeightUpper, *eyeHeightLower);

    return MTD_OK;
}

/******************************************************************************
MTD_STATUS mtdSerdesSample32X1X4EyePlot
    Sample code calling the SerDes API to plot an eye diagram. The eye plotting may
    takes several seconds to minutes depending on the speed. The lower the speed, the 
    longer it takes to plot the eye diagram.

    Note: 
    minSamples - minimum number of bits to sample at each measured point.
    berThreshold - Bit Error Rate Threshold in nano (factor of 1E-9)

    Below are some commonly used minSamples/berThreshold values for the various speeds:
    Common range of inputs for minSamples and berThreshold:
    berThreshold = 10000000
    minSamples = 10

    For X3680 multi-ports, the laneOffset maps to the MDIO port accordingly, single port 
    X3610 is always laneOffset 0 

    The eye plotting output uses the MCESD_DBG_INFO() macro. Make sure this macro is updated
    to print the output to the appropriate logging to capture the eye diagram.

    The mtdSerdesEyePlotStats() is the older eye plotting API that is remained for
    backward compatibility. The preferred way is to use this mtdSampleSerDesEyePlot()
    sample to plot the eye diagram.
*******************************************************************************/
MTD_STATUS mtdSerdesSample32X1X4EyePlot
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort,
    IN MTD_U16 host_or_line,
    IN MTD_U16 laneOffset,
    IN MTD_U32 minSamples,
    IN MTD_U32 phaseStepSize,
    IN MTD_U32 voltageStepSize,
    IN MTD_U32 berThreshold
)
{
    MCESD_DEV_PTR pSerdesDev;
    S_N5C32GP5X4_EYE_RAW eyeRawData;

    MTD_ATTEMPT(mtdSetSerdesPortGetDevPtr(devPtr, mdioPort, &pSerdesDev));

    laneOffset = (MTD_U8)((mdioPort / 4) * 4); /* For X3680 multi-ports, the laneOffset maps to the MDIO port accordingly;
                                                  single port X3610 is always laneOffset 0 */

    /* calling API_N5C32GP5X4_EOMGetEyeData() to get eye for plotting the eye diagram */
    MTD_ATTEMPT(API_N5C32GP5X4_EOMGetEyeData(pSerdesDev, (MTD_U8)laneOffset, minSamples, 
                                             berThreshold, MTD_FALSE, &eyeRawData));

    /* The phase and voltage step size values used in API_N5C32GP5X4_EOMPlotEyeData() are the values 
       used to increment the phase/voltage for loops when plotting. Using too large of a step size 
       may skip too many points when plotting. If the provided step size is 1, then the API will 
       plot every value without skipping. */
    phaseStepSize = 1;
    voltageStepSize = 1;
    
    /* API_N5C32GP5X4_EOMPlotEyeData(eyeRawDataPtr, berThreshold, berThresholdMax)
       - berThresholdMax expands the eye plotting.
       - berThreshold shows the gradual changes in symbols in the plot; the berThreshold should be less 
         than the berThresholdMax 
       Below is a sample plotting the eye where berThresholdMax is 10 times greater than berThreshold. */
    MTD_ATTEMPT(API_N5C32GP5X4_EOMPlotEyeData(&eyeRawData, phaseStepSize, voltageStepSize, berThreshold, berThreshold*10));

    return MTD_OK;
}

#endif /* MTD_CE_SERDES32X1X4 */


MTD_STATIC MTD_STATUS mtdSampleWait
(
    MTD_DEV_PTR dev,
    MTD_UINT milliSeconds
)
{
    /* Instead of modifying the mtdWait() function, customer can implement system-dependent */
    /* wait function here */
    return MTD_OK;
}

MTD_STATIC MTD_STATUS mtdSampleCl45MdioRead
(
    MTD_DEV* dev,
    MTD_U16 port,
    MTD_U16 mmd,
    MTD_U16 reg,
    MTD_U16* value  /* data read from register */
)
{
    /* Instead of modifying the mtdHwXmdioRead() function, customer can implement Clause 45 */
    /* MDIO-dependent read function here */
    return MTD_OK;
}

MTD_STATIC MTD_STATUS mtdSampleCl45MdioWrite
(
    MTD_DEV* dev,
    MTD_U16 port,
    MTD_U16 mmd,
    MTD_U16 reg,
    MTD_U16 value   /* date to write into register */
)
{
    /* Instead of modifying the mtdHwXmdioWrite() function, customer can implement Clause 45 */
    /* MDIO-dependent write function here */
    return MTD_OK;
}

MTD_STATIC MTD_STATUS mtdSampleCl22MdioRead
(
    MTD_DEV* dev,
    MTD_U16 port,
    MTD_U16 reg,
    MTD_U16* value  /* data read from register */
)
{
    /* Instead of modifying the mtdHwXmdioRead() function, customer can implement Clause 22 */
    /* MDIO-dependent read function here */
    return MTD_OK;
}

MTD_STATIC MTD_STATUS mtdSampleCl22MdioWrite
(
    MTD_DEV* dev,
    MTD_U16 port,
    MTD_U16 reg,
    MTD_U16 value   /* date to write into register */
)
{
    /* Instead of modifying the mtdHwXmdioWrite() function, customer can implement Clause 22 */
    /* MDIO-dependent write function here */
    return MTD_OK;
}

