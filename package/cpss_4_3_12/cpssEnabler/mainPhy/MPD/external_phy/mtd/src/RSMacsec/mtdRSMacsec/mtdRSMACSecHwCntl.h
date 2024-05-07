/*******************************************************************************
Copyright (C) 2014 - 2022, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains prototype functions for initializing the driver and setting
up the user-provide MDIO access functions for the Marvell CUX3610/CUE2610
MACSec enable Ethernet PHYs.
********************************************************************/
#ifndef RSMACSEC_HWCTL_H
#define RSMACSEC_HWCTL_H

#if C_LINKAGE
#if defined __cplusplus 
    extern "C" { 
#endif 
#endif

/******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACSecRegRead64
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16  regAddr,
    OUT MTD_U64 volatile *regVal
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    regAddr - register address

 Outputs:
    regVal - 64 bits MACSec register read data from the MACSec MDIO port

  Returns:
    MTD_OK if successful
    MTD_FAIL if not

  Description:
    Reads the MACSec registers using indirect register access and returns 64 bits value. It uses 
    the rsMACSecPort MACSec MDIO port assigned in the MTD_RSMASEC_CTRL_TYPE structure. This call 
    must be completed before calling it again. This is a non-reentrant function. 

  Side effects:
    None.

  Notes/Warnings:
    This is a non-reentrant function.
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACSecRegRead64
(
    IN MTD_DEV_PTR pDev,    
    IN MTD_U16  regAddr,
    OUT MTD_U64 volatile *regVal
);

/******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACSecRegWrite64
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16  regAddr,
    IN MTD_U64  data
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    regAddr - register address
    regVal - 64 bits MACSec register data written to the MACSec MDIO port

 Outputs:
    None

  Returns:
    MTD_OK if successful
    MTD_FAIL if not

  Description:
    Writes the 64-bit MACSec registers. It uses the rsMACSecPort MACSec MDIO port assigned 
    in the MTD_RSMASEC_CTRL_TYPE structure. This call must be completed before calling it 
    again. This is a non-reentrant function.

  Side effects:
    None.

  Notes/Warnings:
    This is a non-reentrant function.
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACSecRegWrite64
(
    IN MTD_DEV_PTR pDev,    
    IN MTD_U16  regAddr,
    IN MTD_U64  data
);

/******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACSecRegRead32
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16  regAddr,
    OUT MTD_U32 *regVal
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    regAddr - register address

 Outputs:
    regVal - 32 bits MACSec register read data from the MACSec MDIO port

  Returns:
    MTD_OK if successful
    MTD_FAIL if not

  Description:
    Reads the MACSec registers using indirect register access and returns 32 bits value. It uses
    the rsMACSecPort MACSec MDIO port assigned in the MTD_RSMASEC_CTRL_TYPE structure. This call
    must be completed before calling it again. This is a non-reentrant function.

  Side effects:
    None.

  Notes/Warnings:
    This is a non-reentrant function.
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACSecRegRead32
(
    IN MTD_DEV_PTR pDev,    
    IN MTD_U16  regAddr,
    OUT MTD_U32 *regVal
);

/******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACSecRegWrite32
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16  regAddr,
    IN MTD_U32  data
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    regAddr - register address

 Outputs:
    regVal - 32 bits MACSec register read data from the MACSec MDIO port

  Returns:
    MTD_OK if successful
    MTD_FAIL if not

  Description:
    Reads the MACSec registers using indirect register access and returns 32 bits value. It uses
    the rsMACSecPort MACSec MDIO port assigned in the MTD_RSMASEC_CTRL_TYPE structure. This call
    must be completed before calling it again. This is a non-reentrant function.

  Side effects:
    None.

  Notes/Warnings:
    This is a non-reentrant function.
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACSecRegWrite32
(
    IN MTD_DEV_PTR pDev,    
    IN MTD_U16  regAddr,
    IN MTD_U32  data
);

/******************************************************************************
MTD_FUNC MTD_STATUS mtdIndirectRegRead32
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16  regAddr,
    OUT MTD_U32 volatile *regVal
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    mdioPort - MDIO port address, 0-31
    regAddr - register address

 Outputs:
    regVal - 32 bits MACSec register read data from the MACSec MDIO port

  Returns:
    MTD_OK if successful
    MTD_FAIL if not

  Description:
    Reads the provide input registers using indirect register access and returns 32 bits value. It 
    uses the provide input MDIO port. This call must be completed before calling it again. This is 
    a non-reentrant function.

  Side effects:
    None.

  Notes/Warnings:
    This is a non-reentrant function.
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdIndirectRegRead32
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16  regAddr,
    OUT MTD_U32 volatile *regVal
);

/******************************************************************************
MTD_FUNC MTD_STATUS mtdIndirectRegRead64
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16  regAddr,
    OUT MTD_U64 volatile *regVal
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    mdioPort - MDIO port address, 0-31
    regAddr - register address

 Outputs:
    regVal - 64 bits MACSec register read data from the MACSec MDIO port

  Returns:
    MTD_OK if successful
    MTD_FAIL if not

  Description:
    Similar to mtdIndirectRegRead32() but this call perform a 64-bit indirect read. Reads the provide 
    input registers using indirect register access and returns 54 bits value. It uses the provide 
    input MDIO port. This call must be completed before calling it again. This is
    a non-reentrant function.

  Side effects:
    None.

  Notes/Warnings:
    This is a non-reentrant function.
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdIndirectRegRead64
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16  regAddr,
    OUT MTD_U64 volatile *regVal
);

/******************************************************************************
MTD_FUNC MTD_STATUS mtdRegRead32
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 dev,
    IN MTD_U16 regAddr,
    OUT MTD_U32 *regVal
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    mdioPort - MDIO port address, 0-31
    dev - MMD device address, 0-31
    regAddr - register address

 Outputs:
    regVal - 32 bits register read data

  Returns:
    MTD_OK if successful
    MTD_FAIL if not

  Description:
    Reads the provide input registers using direct register access and returns 32 bits value. It
    uses the provide input MDIO port. This call must be completed before calling it again. This is
    a non-reentrant function.

  Side effects:
    None.

  Notes/Warnings:
    This is a non-reentrant function.
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdRegRead32
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 dev,
    IN MTD_U16 regAddr,
    OUT MTD_U32 *regVal
);

/******************************************************************************
MTD_FUNC MTD_STATUS mtdRegWrite32
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 dev,
    IN MTD_U16 regAddr,
    IN MTD_U32 regVal
);

 Inputs:
    devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
    mdioPort - MDIO port address, 0-31
    dev - MMD device address, 0-31
    regAddr - register address
    regVal - 32 bits register to write

 Outputs:
    None

  Returns:
    MTD_OK if successful
    MTD_FAIL if not

  Description:
    Write to the provide input registers using direct register access. It uses the provide 
    input MDIO port. This call must be completed before calling it again. This is a 
    non-reentrant function.

  Side effects:
    None.

  Notes/Warnings:
    This is a non-reentrant function.
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdRegWrite32
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 dev,
    IN MTD_U16 regAddr,
    IN MTD_U32 regVal
);

/******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACSecInit
(
    IN MTD_DEV_PTR pDev
);

 Inputs:
    pDev - pointer to MTD_DEV initialized by mtdLoadDriver() call

 Outputs:
    None

  Returns:
    MTD_OK if successful
    MTD_FAIL if not

  Description:
    Used internally in mtdLoadDriver to setup the read and write function pointers. Can also
    be used for external call to setup host software to call MACSec API.

  Side effects:
    None

  Notes/Warnings:
    None
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACSecInit
(
    IN MTD_DEV_PTR pDev
);

/******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACSecUnload
(
    IN MTD_DEV_PTR pDev
);

 Inputs:
    pDev - pointer to MTD_DEV initialized by mtdLoadDriver() call

 Outputs:
    None

  Returns:
    MTD_OK if successful
    MTD_FAIL if not

  Description:
    Call to unload the MACSec API read and write function pointers that is setup
    in the mtdRSMACSecInit.

  Side effects:
    None

  Notes/Warnings:
    None
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACSecUnload
(
    IN MTD_DEV_PTR pDev
);

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif


#endif /* RSMACSEC_HWCTL_H */

