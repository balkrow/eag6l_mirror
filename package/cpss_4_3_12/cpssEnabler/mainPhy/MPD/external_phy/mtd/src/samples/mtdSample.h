/*******************************************************************************
Copyright (C) 2014 - 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains sample function prototypes, macros and definitions 
for the Marvell 88X32X0, 88X33X0, 88X35X0, CUX36X0, CUE26X0, 88E20X0 
and 88E21X0 ethernet PHYs.
********************************************************************/
#ifndef MTD_SAMPLE_H
#define MTD_SAMPLE_H

#if C_LINKAGE
#if defined __cplusplus
    extern "C" {
#endif
#endif

#if 0 /* obsolete, change 0-> if used these previously and want to continue using them */
/* NOTE: These macros were given out in a pre-release before the loopback functions had been provided. */
/* They match the terminology in an older version of the datasheet. Please use the C functions: */
/* mtdSetTunitDeepMacLoopback() */
/* mtdSetTunitShallowLineLoopback() */
/* mtdSetHXunitLineLoopback() */
/* mtdSetHXunitMacLoopback() */
/* instead if possible. */

/* 
   Loopback macros for various units: 
   devPtr - allocated memory for the device structure
   port - MDIO port address, 0-31
   flag - MTD_TRUE to enable loopback; MTD_FALSE set to normal/disable 
*/
#define MTD_TUNIT_MAC_LOOPBACK(devPtr,port,flag)                 mtdHwSetPhyRegField(devPtr,port,MTD_TUNIT_IEEE_PCS_CTRL1,14,1,flag)
#define MTD_TUNIT_LINE_10M_100M_1000M_LOOPBACK(devPtr,port,flag) mtdHwSetPhyRegField(devPtr,port,MTD_T_UNIT_PCS_CU,0x8002,5,1,flag)
#define MTD_TUNIT_LINE_2P5G_5G_10G_LOOPBACK(devPtr,port,flag)    mtdHwSetPhyRegField(devPtr,port,1,0xC000,11,1,flag)

#define MTD_XUNIT_MAC_LOOPBACK(devPtr,port,flag)   mtdHwSetPhyRegField(devPtr,port,MTD_X_UNIT,0x1000,14,1,flag)
#define MTD_XUNIT_LINE_LOOPBACK(devPtr,port,flag)  mtdHwSetPhyRegField(devPtr,port,MTD_X_UNIT,0xF003,12,1,flag)

#define MTD_HUNIT_MAC_LOOPBACK(devPtr,port,flag)  mtdHwSetPhyRegField(devPtr,port,MTD_H_UNIT,0x0000,14,1,flag)
#define MTD_HUNIT_LINE_LOOPBACK(devPtr,port,flag) mtdHwSetPhyRegField(devPtr,port,MTD_H_UNIT,0xF003,12,1,flag)
#endif

/* 
   Force link good macros for various units: 
   devPtr - allocated memory for the device structure
   port - MDIO port address, 0-31
   flag - MTD_TRUE to Force Link good; MTD_FALSE set to normal operation  
*/
#define MTD_TUNIT_FORCE_LINK_GOOD(devPtr,port,flag)  mtdHwSetPhyRegField(devPtr,port,MTD_T_UNIT_PCS_CU,0x8002,1,1,flag)
#define MTD_HUNIT_FORCE_LINK_GOOD(devPtr,port,flag)  mtdHwSetPhyRegField(devPtr,port,MTD_H_UNIT,MTD_SERDES_CONTROL1,10,1,flag)
#define MTD_XUNIT_FORCE_LINK_GOOD(devPtr,port,flag)  mtdHwSetPhyRegField(devPtr,port,MTD_X_UNIT,MTD_SERDES_CONTROL1,10,1,flag)


/*
    X35X0 LEDs Control
*/

#define MTD_X35X0_LED0 0 /* To select which LED has its control being set/get */
#define MTD_X35X0_LED1 1
#define MTD_X35X0_LED2 2
#define MTD_X35X0_LED3 3

#define MTD_X35X0_LED_CONTROL(mtdLedNum)    MTD_C_UNIT_GENERAL,(0xF020+mtdLedNum) /* Control Register Address for LED number 0...3 */
/* NOTE: 31.F021 is ignored if 31.F020.12:11 = 0x3, Dual Mode */
/* and   31.F023 is ignored if 31.F022.12:11 - 0x3, Dual Mode */

/* LED_CONTROL Bit Fields */
#define MTD_X35X0_LED_POLARITY_BOFFSET      0 /* Polarity Control */
#define MTD_X35X0_LED_POLARITY_BLENGTH      2
#define MTD_X35X0_LED_BLINKRT_BOFFSET       2 /* Blink Rate Control */
#define MTD_X35X0_LED_BLINKRT_BLENGTH       1
#define MTD_X35X0_LED_SOLID_BOFFSET         3 /* Solid Behavior Selection */
#define MTD_X35X0_LED_SOLID_BLENGTH         5
#define MTD_X35X0_LED_BLINK_BOFFSET         8 /* Blink Behavior Selection */
#define MTD_X35X0_LED_BLINK_BLENGTH         5

/* LED_CONTROL Bit Field Values */

#define MTD_X35X0_LED_POLARITY_LH    0x0 /* Polarity control - On-Low Off-High DEFAULT */
#define MTD_X35X0_LED_POLARITY_HL    0x1 /* On-High Off-Low */
#define MTD_X35X0_LED_POLARITY_LT    0x2 /* On-Low Off-Tristate */
#define MTD_X35X0_LED_POLARITY_HT    0x3 /* On-High Off-Tristate */

#define MTD_X35X0_LED_BLINK_RATE1   0x0 /* Selects Blink Rate 1/2 DEFAULT */ 
#define MTD_X35X0_LED_BLINK_RATE2   0x1

#define MTD_X35X0_LED_SLD_OFF               0x0 /* Selects Solid Behavior */
#define MTD_X35X0_LED_SLD_TXRX              0x1
#define MTD_X35X0_LED_SLD_TX                0x2
#define MTD_X35X0_LED_SLD_RX                0x3
#define MTD_X35X0_LED_SLD_COLLISION         0x4
#define MTD_X35X0_LED_SLD_CU_LINK2          0x5
#define MTD_X35X0_LED_SLD_NOT_USED          0x6      
#define MTD_X35X0_LED_SLD_CU_LINK           0x7
#define MTD_X35X0_LED_SLD_10M               0x8
#define MTD_X35X0_LED_SLD_100M              0x9
#define MTD_X35X0_LED_SLD_1G                0xA
#define MTD_X35X0_LED_SLD_10G               0xB
#define MTD_X35X0_LED_SLD_10_100M           0xC
#define MTD_X35X0_LED_SLD_10_100_1G         0xD
#define MTD_X35X0_LED_SLD_100_10G           0xE
#define MTD_X35X0_LED_SLD_1G_10G            0xF
#define MTD_X35X0_LED_SLD_1G_10G_SLAVE      0x10
#define MTD_X35X0_LED_SLD_1G_10G_MASTER     0x11
#define MTD_X35X0_LED_SLD_HALF_DUPLEX       0x12
#define MTD_X35X0_LED_SLD_FULL_DUPLEX       0x13
#define MTD_X35X0_LED_SLD_ENERGY_EFF        0x14
#define MTD_X35X0_LED_SLD_2P5G              0x15
#define MTD_X35X0_LED_SLD_5G                0x16
#define MTD_X35X0_LED_SLD_ON                0x17
#define MTD_X35X0_LED_SLD_2P5G_5G_SLAVE     0x18
#define MTD_X35X0_LED_SLD_2P5G_5G_MASTER    0x19

#define MTD_X35X0_LED_BLNK_OFF               0x0 /* Selects Blink Behavior */
#define MTD_X35X0_LED_BLNK_TXRX              0x1
#define MTD_X35X0_LED_BLNK_TX                0x2
#define MTD_X35X0_LED_BLNK_RX                0x3
#define MTD_X35X0_LED_BLNK_COLLISION         0x4
#define MTD_X35X0_LED_BLNK_CU_LINK2          0x5
#define MTD_X35X0_LED_BLNK_NOT_USE           0x6      
#define MTD_X35X0_LED_BLNK_CU_LINK           0x7
#define MTD_X35X0_LED_BLNK_10M               0x8
#define MTD_X35X0_LED_BLNK_100M              0x9
#define MTD_X35X0_LED_BLNK_1G                0xA
#define MTD_X35X0_LED_BLNK_10G               0xB
#define MTD_X35X0_LED_BLNK_10_100M           0xC
#define MTD_X35X0_LED_BLNK_10_100_1G         0xD
#define MTD_X35X0_LED_BLNK_100_10G           0xE
#define MTD_X35X0_LED_BLNK_1G_10G            0xF
#define MTD_X35X0_LED_BLNK_1G_10G_SLAVE      0x10
#define MTD_X35X0_LED_BLNK_1G_10G_MASTER     0x11
#define MTD_X35X0_LED_BLNK_HALF_DUPLEX       0x12
#define MTD_X35X0_LED_BLNK_FULL_DUPLEX       0x13
#define MTD_X35X0_LED_BLNK_ENERGY_EFF        0x14
#define MTD_X35X0_LED_BLNK_2P5G              0x15
#define MTD_X35X0_LED_BLNK_5G                0x16
#define MTD_X35X0_LED_BLNK_ON                0x17
#define MTD_X35X0_LED_BLNK_2P5G_5G_SLAVE     0x18
#define MTD_X35X0_LED_BLNK_2P5G_5G_MASTER    0x19
#define MTD_X35X0_LED_BLNK_BLNK_MIX          0x1A
                                                  /* Dual Mode options, 31.F020 controls both LED0/LED1 */
                                                  /* and 31.F022 controls both LED2/3 if these are selected */
#define MTD_X35X0_LED_BLNK_SLD_MIX           0x1B /* Dual Mode Solid Mix */
#define MTD_X35X0_LED_BLNK_SPD_EEE_TYPE1     0x1C /* Dual Mode Speed/EEE Type 1 */
#define MTD_X35X0_LED_BLNK_SPEED_TYPE1       0x1D /* Dual Mode Speed Type 1 */
#define MTD_X35X0_LED_BLNK_SPD_EEE_TYPE2     0x1E /* Dual Mode Speed/EEE Type2 */
#define MTD_X35X0_LED_BLNK_SPEED_TYPE2       0x1F /* Dual Mode Speed Type 2 */

/* Defaults for X3540/X3580, E2540/E2580 may have different defaults */
/* LED0 Solid is 7 (Copper Link) */
/* LED0 Blink is 1 (Tx/Rx Activity) */
/* LED0 Blink Rate is 0 (Blink Rate 1) */
/* LED0 Polarity is 0 (On drive low, Off drive high)*/

/* LED1 Solid is 0xB (10G Link) */
/* LED1 Blink is 0 (Off) */
/* LED1 Blink Rate is 0 (Blink Rate 1) */
/* LED1 Polarity is 0 (On drive low, Off drive high)*/

/* LED2 Solid is 0xA (1G Link) */
/* LED2 Blink is 0 (Off) */
/* LED2 Blink Rate is 0 (Blink Rate 1) */
/* LED2 Polarity is 0 (On drive low, Off drive high)*/

/* LED3 Solid is 0 (100M Link) */
/* LED3 Blink is 0 (Off) */
/* LED3 Blink Rate is 0 (Blink Rate 1) */
/* LED3 Polarity is 0 (On drive low, Off drive high)*/

#define MTD_X35X0_LED_MIX_CTRL              MTD_C_UNIT_GENERAL,0xF026  

/* LED_MIX Bit Fields */
#define MTD_X35X0_LED_20MIX_BOFFSET         0 /* LED2 LED0 Mix Percentage */
#define MTD_X35X0_LED_20MIX_BLENGTH         4
#define MTD_X35X0_LED_31MIX_BOFFSET         4 /* LED3 LED1 Mix Percentage */
#define MTD_X35X0_LED_31MIX_BLENGTH         4
#define MTD_X35X0_LED_ACTIVITY_BOFFSET     10 /* Activity Selection */
#define MTD_X35X0_LED_ACTIVITY_BLENGTH      2

/* LED_MIX Bit Field Values */
#define MTD_X35X0_LED_MIX_0                 0 /* Percentages, when using 2 terminal bi-coloR LEDs */
#define MTD_X35X0_LED_MIX_12P5              1 /* the mix percentage should not be set greater than 50% */
#define MTD_X35X0_LED_MIX_50                4 /* DEFAULT 50% */
#define MTD_X35X0_LED_MIX_87P5              7 /* Bit values are 1 bit = 12.5%, can select in between values */
#define MTD_X35X0_LED_MIX_100               8 

#define MTD_X35X0_LED_COUNT_RX              0 /* Activity, count RX frames only */
#define MTD_X35X0_LED_COUNT_TX              1 /* Count TX frames only */
#define MTD_X35X0_LED_COUNT_RXTX            2 /* Count both */
#define MTD_X35X0_LED_COUNT_TXRX            3 /* Count both */


#define MTD_X35X0_LED_TIMER_CTRL            MTD_C_UNIT_GENERAL,0xF027

/* LED_TIMER Bit Fields */
#define MTD_X35X0_LED_SPDON_BOFFSET         0 /* Speed On Pulse Period */
#define MTD_X35X0_LED_SPDON_BLENGTH         2
#define MTD_X35X0_LED_SPDOFF_BOFFSET        2 /* Speed Off Pulse Period */
#define MTD_X35X0_LED_SPDOFF_BLENGTH        2
#define MTD_X35X0_LED_BLINK1_BOFFSET        4 /* Blink Rate 1 */
#define MTD_X35X0_LED_BLINK1_BLENGTH        3
#define MTD_X35X0_LED_BLINK2_BOFFSET        8 /* Blink Rate 2 */
#define MTD_X35X0_LED_BLINK2_BLENGTH        3
#define MTD_X35X0_LED_STRETCH_BOFFSET      12 /* Pulse Stretch Duration */
#define MTD_X35X0_LED_STRETCH_BLENGTH       3

/* LED_TIMER Bit Field Values */
#define MTD_X35X0_LED_PERIOD_81MS           0  /* Speed On/Off Pulse Period in ms */
#define MTD_X35X0_LED_PERIOD_161MS          1  /* DEFAULT */
#define MTD_X35X0_LED_PERIOD_322MS          2
#define MTD_X35X0_LED_PERIOD_644MS          3

#define MTD_X35X0_LED_RATE_40MS             0 /* Blink Rate */            
#define MTD_X35X0_LED_RATE_81MS             1 /* DEFAULT for Blink Rate 1 */ 
#define MTD_X35X0_LED_RATE_161MS            2
#define MTD_X35X0_LED_RATE_322MS            3
#define MTD_X35X0_LED_RATE_644MS            4
#define MTD_X35X0_LED_RATE_1P3S             5 /* DEFAULT for Blink Rate 2 */
#define MTD_X35X0_LED_RATE_2P6S             6
#define MTD_X35X0_LED_RATE_5P2S             7

#define MTD_X35X0_LED_STRETCH_NONE          0 /* Pulse Stretch */            
#define MTD_X35X0_LED_STRETCH_20_40MS       1 
#define MTD_X35X0_LED_STRETCH_40_81MS       2
#define MTD_X35X0_LED_STRETCH_81_161MS      3
#define MTD_X35X0_LED_STRETCH_161_322MS     4 /* DEFAULT */
#define MTD_X35X0_LED_STRETCH_322_644MS     5 
#define MTD_X35X0_LED_STRETCH_644_1P3S      6
#define MTD_X35X0_LED_STRETCH_1P3_2P6S      7

#define MTD_X35X0_LED_DO_NOT_CHANGE       0xFF /* Value to pass for any field to leave that field as-is */


/*
    X35X0 RCLK Enable/Select (to be used in AMD mode)
*/

#define MTD_X35X0_AMD_SEL_CU_RCLK 1 /* Selects passed in port's copper port as source */
#define MTD_X35X0_AMD_SEL_FI_RCLK 2 /* Selects passed in (port+1)'s fiber port as source */
#define MTD_X35X0_AMD_SEL_NO_RCLK 3 /* Disables the RCLK */

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif


#endif /* MTD_SAMPLE_H */
