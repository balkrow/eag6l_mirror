/*******************************************************************************
Copyright (C) 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions global definitions specific to Marvell
CE Serdes IP: 5FFP_COMPHY_32G_PIPE5_X4_4PLL
********************************************************************/
#ifndef MCESD_N5C32GP5X4_DEFS_H
#define MCESD_N5C32GP5X4_DEFS_H

#ifdef N5C32GP5X4

#if C_LINKAGE
#if defined __cplusplus 
extern "C" {
#endif 
#endif

/* MCU Firmware constants */
#define N5C32GP5X4_FW_MAX_SIZE         32768       /* Firmware max size in DWORDS */
#define N5C32GP5X4_FW_BASE_ADDR        0x0000      /* Firmware base address */

/* Squelch Detector Threshold Range constants */
#define N5C32GP5X4_SQ_THRESH_MAX       0x1F

/* Temperature Sensor constants */
#define N5C32GP5X4_TSENE_GAIN          950
#define N5C32GP5X4_TSENE_OFFSET        1180000

/* EOM constants */
#define N5C32GP5X4_EYE_MAX_VOLT_STEPS  64
#define N5C32GP5X4_EYE_MAX_PHASE_LEVEL 2560

/* Miscellaneous */
#define N5C32GP5X4_TOTAL_LANES         4

/* PHY Mode */
typedef enum {
    N5C32GP5X4_PHYMODE_PCIE         = 3,
    N5C32GP5X4_PHYMODE_SERDES       = 4,
} E_N5C32GP5X4_PHYMODE;

/* SERDES Speeds */
typedef enum {
    N5C32GP5X4_SERDES_1P0625G       = 0,    /* 1.0625G */
    N5C32GP5X4_SERDES_1P2288G       = 1,    /* 1.2288G */
    N5C32GP5X4_SERDES_1P25G         = 2,    /* 1.25G */
    N5C32GP5X4_SERDES_2P125G        = 3,    /* 2.125G */
    N5C32GP5X4_SERDES_2P4576G       = 4,    /* 2.4576G */
    N5C32GP5X4_SERDES_2P5G          = 5,    /* 2.5G */
    N5C32GP5X4_SERDES_3P125G        = 6,    /* 3.125G */
    N5C32GP5X4_SERDES_4P25G         = 7,    /* 4.25G */
    N5C32GP5X4_SERDES_4P9152G       = 8,    /* 4.9152G */
    N5C32GP5X4_SERDES_5G            = 9,    /* 5G */
    N5C32GP5X4_SERDES_5P15625G      = 10,   /* 5.15625G */
    N5C32GP5X4_SERDES_6P144G        = 11,   /* 6.144G */
    N5C32GP5X4_SERDES_6P25G         = 12,   /* 6.25G */
    N5C32GP5X4_SERDES_7P5G          = 13,   /* 7.5G */
    N5C32GP5X4_SERDES_8P5G          = 14,   /* 8.5G */
    N5C32GP5X4_SERDES_9P8304G       = 15,   /* 9.8304G */
    N5C32GP5X4_SERDES_10P1376G      = 16,   /* 10.1376G */
    N5C32GP5X4_SERDES_10P3125G      = 17,   /* 10.3125G */
    N5C32GP5X4_SERDES_10P51875G     = 18,   /* 10.51875G */
    N5C32GP5X4_SERDES_12P16512G     = 19,   /* 12.16512G */
    N5C32GP5X4_SERDES_12P1875G      = 20,   /* 12.1875G */
    N5C32GP5X4_SERDES_12P5G         = 21,   /* 12.5G */
    N5C32GP5X4_SERDES_12P8906G      = 22,   /* 12.8906G */
    N5C32GP5X4_SERDES_14P025G       = 23,   /* 14.025G */
    N5C32GP5X4_SERDES_20P625G       = 24,   /* 20.625G */
    N5C32GP5X4_SERDES_24P33024G     = 25,   /* 24.33024G */
    N5C32GP5X4_SERDES_25P78125G     = 26,   /* 25.78125G */
    N5C32GP5X4_SERDES_26P5625G      = 27,   /* 26.5625G */
    N5C32GP5X4_SERDES_27P5G         = 28,   /* 27.5G */
    N5C32GP5X4_SERDES_28P05G        = 29,   /* 28.05G */
    N5C32GP5X4_SERDES_28P125G       = 30,   /* 28.125G */
    N5C32GP5X4_SERDES_2P578125G     = 46,   /* 2.578125G */
    N5C32GP5X4_SERDES_15G           = 47,   /* 15G */
    N5C32GP5X4_SERDES_32G           = 48,   /* 32G */
    N5C32GP5X4_SERDES_10G           = 49,   /* 10G */
    N5C32GP5X4_SERDES_2P02752G      = 50,   /* 2.02752G */
    N5C32GP5X4_SERDES_4P08804G      = 51,   /* 4.08804G */
    N5C32GP5X4_SERDES_7P3728G       = 52,   /* 7.3728G */
    N5C32GP5X4_SERDES_8P11008G      = 53,   /* 8.11008G */
    N5C32GP5X4_SERDES_14P7456G      = 54,   /* 14.7456G */
    N5C32GP5X4_SERDES_16P22016G     = 55,   /* 16.22016G */
} E_N5C32GP5X4_SERDES_SPEED;

/* Reference Frequency Clock */
typedef enum {
    N5C32GP5X4_REFFREQ_25MHZ        = 0,    /* 25 MHz */
    N5C32GP5X4_REFFREQ_30MHZ        = 1,    /* 30 MHz */
    N5C32GP5X4_REFFREQ_40MHZ        = 2,    /* 40 MHz */
    N5C32GP5X4_REFFREQ_50MHZ        = 3,    /* 50 MHz */
    N5C32GP5X4_REFFREQ_62P5MHZ      = 4,    /* 62.5 MHz */
    N5C32GP5X4_REFFREQ_100MHZ       = 5,    /* 100 MHz */
    N5C32GP5X4_REFFREQ_125MHZ       = 6,    /* 125 MHz */
    N5C32GP5X4_REFFREQ_156P25MHZ    = 7,    /* 156.25 MHz */
    N5C32GP5X4_REFFREQ_122P88MHZ    = 8,    /* 122.88 MHz */
} E_N5C32GP5X4_REFFREQ;

/* Reference Clock Selection Group */
typedef enum {
    N5C32GP5X4_REFCLK_SEL_GROUP1    = 0,    /* 1'b0: Tx Reference clock comes from group 1 PIN_REFCLKC_IN_SIDE_A_G1 or PIN_REFCLKC_IN_SIDE_B_G1. */
    N5C32GP5X4_REFCLK_SEL_GROUP2    = 1,    /* 1'b1: Tx Reference clock comes from group 2 PIN_REFCLKC_IN_SIDE_A_G2 or PIN_REFCLKC_IN_SIDE_B_G2. */
} E_N5C32GP5X4_REFCLK_SEL;

/* TX and RX Data Bus Width */
typedef enum {
    N5C32GP5X4_DATABUS_32BIT        = 0,
    N5C32GP5X4_DATABUS_40BIT        = 1,
} E_N5C32GP5X4_DATABUS_WIDTH;

/* Tx and Rx Polarity */
typedef enum {
    N5C32GP5X4_POLARITY_NORMAL      = 0,
    N5C32GP5X4_POLARITY_INVERTED    = 1,
} E_N5C32GP5X4_POLARITY;

/* Data Path Modes */
typedef enum {
    N5C32GP5X4_PATH_NEAR_END_LB     = 0,
    N5C32GP5X4_PATH_LOCAL_LB        = 1,
    N5C32GP5X4_PATH_EXTERNAL        = 2,
    N5C32GP5X4_PATH_FAR_END_LB      = 3,
} E_N5C32GP5X4_DATAPATH;

/* Training Type */
typedef enum {
    N5C32GP5X4_TRAINING_TRX         = 0,
    N5C32GP5X4_TRAINING_RX          = 1,
} E_N5C32GP5X4_TRAINING;

/* Training Timeout */
typedef struct
{
    MCESD_BOOL enable;
    MCESD_U16 timeout; /* milliseconds */
} S_N5C32GP5X4_TRAIN_TIMEOUT;

/* PhyTest Type */
typedef enum {
    N5C32GP5X4_PHYTEST_TX           = 0,
    N5C32GP5X4_PHYTEST_RX           = 1,
} E_N5C32GP5X4_PHYTEST;

/* Hardware Pins */
typedef enum
{
    N5C32GP5X4_PIN_RESET           = 0,
    N5C32GP5X4_PIN_ISOLATION       = 1,    /* PIN_ISOLATION_ENB */
    N5C32GP5X4_PIN_BG_RDY          = 2,
    N5C32GP5X4_PIN_SIF_SEL         = 3,
    N5C32GP5X4_PIN_MCU_CLK         = 4,
    N5C32GP5X4_PIN_DIRECTACCES     = 5,    /* PIN_DIRECT_ACCESS_EN */
    N5C32GP5X4_PIN_PHY_MODE        = 6,
    N5C32GP5X4_PIN_REFCLK_TX0      = 7,    /* PIN_REFCLK_SEL_TX0 */
    N5C32GP5X4_PIN_REFCLK_TX1      = 8,    /* PIN_REFCLK_SEL_TX1 */
    N5C32GP5X4_PIN_REFCLK_TX2      = 9,    /* PIN_REFCLK_SEL_TX2 */
    N5C32GP5X4_PIN_REFCLK_TX3      = 10,   /* PIN_REFCLK_SEL_TX3 */
    N5C32GP5X4_PIN_REFCLK_RX0      = 11,   /* PIN_REFCLK_SEL_RX0 */
    N5C32GP5X4_PIN_REFCLK_RX1      = 12,   /* PIN_REFCLK_SEL_RX1 */
    N5C32GP5X4_PIN_REFCLK_RX2      = 13,   /* PIN_REFCLK_SEL_RX2 */
    N5C32GP5X4_PIN_REFCLK_RX3      = 14,   /* PIN_REFCLK_SEL_RX3 */
    N5C32GP5X4_PIN_REFFREF_TX0     = 15,   /* PIN_REF_FREF_SEL_TX0 */
    N5C32GP5X4_PIN_REFFREF_TX1     = 16,   /* PIN_REF_FREF_SEL_TX1 */
    N5C32GP5X4_PIN_REFFREF_TX2     = 17,   /* PIN_REF_FREF_SEL_TX2 */
    N5C32GP5X4_PIN_REFFREF_TX3     = 18,   /* PIN_REF_FREF_SEL_TX3 */
    N5C32GP5X4_PIN_REFFREF_RX0     = 19,   /* PIN_REF_FREF_SEL_RX0 */
    N5C32GP5X4_PIN_REFFREF_RX1     = 20,   /* PIN_REF_FREF_SEL_RX1 */
    N5C32GP5X4_PIN_REFFREF_RX2     = 21,   /* PIN_REF_FREF_SEL_RX2 */
    N5C32GP5X4_PIN_REFFREF_RX3     = 22,   /* PIN_REF_FREF_SEL_RX3 */
    N5C32GP5X4_PIN_PHY_GEN_TX0     = 23,
    N5C32GP5X4_PIN_PHY_GEN_TX1     = 24,
    N5C32GP5X4_PIN_PHY_GEN_TX2     = 25,
    N5C32GP5X4_PIN_PHY_GEN_TX3     = 26,
    N5C32GP5X4_PIN_PHY_GEN_RX0     = 27,
    N5C32GP5X4_PIN_PHY_GEN_RX1     = 28,
    N5C32GP5X4_PIN_PHY_GEN_RX2     = 29,
    N5C32GP5X4_PIN_PHY_GEN_RX3     = 30,
    N5C32GP5X4_PIN_PU_PLL0         = 31,
    N5C32GP5X4_PIN_PU_PLL1         = 32,
    N5C32GP5X4_PIN_PU_PLL2         = 33,
    N5C32GP5X4_PIN_PU_PLL3         = 34,
    N5C32GP5X4_PIN_PU_RX0          = 35,
    N5C32GP5X4_PIN_PU_RX1          = 36,
    N5C32GP5X4_PIN_PU_RX2          = 37,
    N5C32GP5X4_PIN_PU_RX3          = 38,
    N5C32GP5X4_PIN_PU_TX0          = 39,
    N5C32GP5X4_PIN_PU_TX1          = 40,
    N5C32GP5X4_PIN_PU_TX2          = 41,
    N5C32GP5X4_PIN_PU_TX3          = 42,
    N5C32GP5X4_PIN_TX_IDLE0        = 43,
    N5C32GP5X4_PIN_TX_IDLE1        = 44,
    N5C32GP5X4_PIN_TX_IDLE2        = 45,
    N5C32GP5X4_PIN_TX_IDLE3        = 46,
    N5C32GP5X4_PIN_PU_IVREF        = 47,
    N5C32GP5X4_PIN_RX_TRAINEN0     = 48,   /* PIN_RX_TRAIN_ENABLE0 */
    N5C32GP5X4_PIN_RX_TRAINEN1     = 49,   /* PIN_RX_TRAIN_ENABLE1 */
    N5C32GP5X4_PIN_RX_TRAINEN2     = 50,   /* PIN_RX_TRAIN_ENABLE2 */
    N5C32GP5X4_PIN_RX_TRAINEN3     = 51,   /* PIN_RX_TRAIN_ENABLE3 */
    N5C32GP5X4_PIN_RX_TRAINCO0     = 52,   /* PIN_RX_TRAIN_COMPLETE0 */
    N5C32GP5X4_PIN_RX_TRAINCO1     = 53,   /* PIN_RX_TRAIN_COMPLETE1 */
    N5C32GP5X4_PIN_RX_TRAINCO2     = 54,   /* PIN_RX_TRAIN_COMPLETE2 */
    N5C32GP5X4_PIN_RX_TRAINCO3     = 55,   /* PIN_RX_TRAIN_COMPLETE3 */
    N5C32GP5X4_PIN_RX_TRAINFA0     = 56,   /* PIN_RX_TRAIN_FAILED0 */
    N5C32GP5X4_PIN_RX_TRAINFA1     = 57,   /* PIN_RX_TRAIN_FAILED1 */
    N5C32GP5X4_PIN_RX_TRAINFA2     = 58,   /* PIN_RX_TRAIN_FAILED2 */
    N5C32GP5X4_PIN_RX_TRAINFA3     = 59,   /* PIN_RX_TRAIN_FAILED3 */
    N5C32GP5X4_PIN_TX_TRAINEN0     = 60,   /* PIN_TX_TRAIN_ENABLE0 */
    N5C32GP5X4_PIN_TX_TRAINEN1     = 61,   /* PIN_TX_TRAIN_ENABLE1 */
    N5C32GP5X4_PIN_TX_TRAINEN2     = 62,   /* PIN_TX_TRAIN_ENABLE2 */
    N5C32GP5X4_PIN_TX_TRAINEN3     = 63,   /* PIN_TX_TRAIN_ENABLE3 */
    N5C32GP5X4_PIN_TX_TRAINCO0     = 64,   /* PIN_TX_TRAIN_COMPLETE0 */
    N5C32GP5X4_PIN_TX_TRAINCO1     = 65,   /* PIN_TX_TRAIN_COMPLETE1 */
    N5C32GP5X4_PIN_TX_TRAINCO2     = 66,   /* PIN_TX_TRAIN_COMPLETE2 */
    N5C32GP5X4_PIN_TX_TRAINCO3     = 67,   /* PIN_TX_TRAIN_COMPLETE3 */
    N5C32GP5X4_PIN_TX_TRAINFA0     = 68,   /* PIN_TX_TRAIN_FAILED0 */
    N5C32GP5X4_PIN_TX_TRAINFA1     = 69,   /* PIN_TX_TRAIN_FAILED1 */
    N5C32GP5X4_PIN_TX_TRAINFA2     = 70,   /* PIN_TX_TRAIN_FAILED2 */
    N5C32GP5X4_PIN_TX_TRAINFA3     = 71,   /* PIN_TX_TRAIN_FAILED3 */
    N5C32GP5X4_PIN_SQ_DET_LPF0     = 72,   /* PIN_SQ_DETECTED_LPF0 */
    N5C32GP5X4_PIN_SQ_DET_LPF1     = 73,   /* PIN_SQ_DETECTED_LPF1 */
    N5C32GP5X4_PIN_SQ_DET_LPF2     = 74,   /* PIN_SQ_DETECTED_LPF2 */
    N5C32GP5X4_PIN_SQ_DET_LPF3     = 75,   /* PIN_SQ_DETECTED_LPF3 */
    N5C32GP5X4_PIN_RX_INIT0        = 76,
    N5C32GP5X4_PIN_RX_INIT1        = 77,
    N5C32GP5X4_PIN_RX_INIT2        = 78,
    N5C32GP5X4_PIN_RX_INIT3        = 79,
    N5C32GP5X4_PIN_RX_INITDON0     = 80,   /* PIN_RX_INIT_DONE0 */
    N5C32GP5X4_PIN_RX_INITDON1     = 81,   /* PIN_RX_INIT_DONE1 */
    N5C32GP5X4_PIN_RX_INITDON2     = 82,   /* PIN_RX_INIT_DONE2 */
    N5C32GP5X4_PIN_RX_INITDON3     = 83,   /* PIN_RX_INIT_DONE3 */
    N5C32GP5X4_PIN_AVDD_SEL        = 84,
    N5C32GP5X4_PIN_SPD_CFG         = 85,
    N5C32GP5X4_PIN_PIPE_SEL        = 86,
    N5C32GP5X4_PIN_TX_GRAY_CODE0   = 87,   /* PIN_TXDATA_GRAY_CODE_EN0 */
    N5C32GP5X4_PIN_TX_GRAY_CODE1   = 88,   /* PIN_TXDATA_GRAY_CODE_EN1 */
    N5C32GP5X4_PIN_TX_GRAY_CODE2   = 89,   /* PIN_TXDATA_GRAY_CODE_EN2 */
    N5C32GP5X4_PIN_TX_GRAY_CODE3   = 90,   /* PIN_TXDATA_GRAY_CODE_EN3 */
    N5C32GP5X4_PIN_RX_GRAY_CODE0   = 91,   /* PIN_RXDATA_GRAY_CODE_EN0 */
    N5C32GP5X4_PIN_RX_GRAY_CODE1   = 92,   /* PIN_RXDATA_GRAY_CODE_EN1 */
    N5C32GP5X4_PIN_RX_GRAY_CODE2   = 93,   /* PIN_RXDATA_GRAY_CODE_EN2 */
    N5C32GP5X4_PIN_RX_GRAY_CODE3   = 94,   /* PIN_RXDATA_GRAY_CODE_EN3 */
    N5C32GP5X4_PIN_TX_PRE_CODE0    = 95,   /* PIN_TXDATA_PRE_CODE_EN0 */
    N5C32GP5X4_PIN_TX_PRE_CODE1    = 96,   /* PIN_TXDATA_PRE_CODE_EN1 */
    N5C32GP5X4_PIN_TX_PRE_CODE2    = 97,   /* PIN_TXDATA_PRE_CODE_EN2 */
    N5C32GP5X4_PIN_TX_PRE_CODE3    = 98,   /* PIN_TXDATA_PRE_CODE_EN3 */
    N5C32GP5X4_PIN_RX_PRE_CODE0    = 99,   /* PIN_RXDATA_PRE_CODE_EN0 */
    N5C32GP5X4_PIN_RX_PRE_CODE1    = 100,  /* PIN_RXDATA_PRE_CODE_EN1 */
    N5C32GP5X4_PIN_RX_PRE_CODE2    = 101,  /* PIN_RXDATA_PRE_CODE_EN2 */
    N5C32GP5X4_PIN_RX_PRE_CODE3    = 102,  /* PIN_RXDATA_PRE_CODE_EN3 */
    N5C32GP5X4_PIN_PLL_READY_TX0   = 103,
    N5C32GP5X4_PIN_PLL_READY_TX1   = 104,
    N5C32GP5X4_PIN_PLL_READY_TX2   = 105,
    N5C32GP5X4_PIN_PLL_READY_TX3   = 106,
    N5C32GP5X4_PIN_PLL_READY_RX0   = 107,
    N5C32GP5X4_PIN_PLL_READY_RX1   = 108,
    N5C32GP5X4_PIN_PLL_READY_RX2   = 109,
    N5C32GP5X4_PIN_PLL_READY_RX3   = 110,
    N5C32GP5X4_PIN_RESET_TX0       = 111,  /* PIN_RESET_CORE_TX0 */
    N5C32GP5X4_PIN_RESET_TX1       = 112,  /* PIN_RESET_CORE_TX1 */
    N5C32GP5X4_PIN_RESET_TX2       = 113,  /* PIN_RESET_CORE_TX2 */
    N5C32GP5X4_PIN_RESET_TX3       = 114,  /* PIN_RESET_CORE_TX3 */
    N5C32GP5X4_PIN_RESET_RX0       = 115,  /* PIN_RESET_CORE_RX0 */
    N5C32GP5X4_PIN_RESET_RX1       = 116,  /* PIN_RESET_CORE_RX1 */
    N5C32GP5X4_PIN_RESET_RX2       = 117,  /* PIN_RESET_CORE_RX2 */
    N5C32GP5X4_PIN_RESET_RX3       = 118,  /* PIN_RESET_CORE_RX3 */
    N5C32GP5X4_PIN_RESET_ACK_TX0   = 119,  /* PIN_RESET_ACK_CORE_TX0 */
    N5C32GP5X4_PIN_RESET_ACK_TX1   = 120,  /* PIN_RESET_ACK_CORE_TX1 */
    N5C32GP5X4_PIN_RESET_ACK_TX2   = 121,  /* PIN_RESET_ACK_CORE_TX2 */
    N5C32GP5X4_PIN_RESET_ACK_TX3   = 122,  /* PIN_RESET_ACK_CORE_TX3 */
    N5C32GP5X4_PIN_RESET_ACK_RX0   = 123,  /* PIN_RESET_ACK_CORE_RX0 */
    N5C32GP5X4_PIN_RESET_ACK_RX1   = 124,  /* PIN_RESET_ACK_CORE_RX1 */
    N5C32GP5X4_PIN_RESET_ACK_RX2   = 125,  /* PIN_RESET_ACK_CORE_RX2 */
    N5C32GP5X4_PIN_RESET_ACK_RX3   = 126,  /* PIN_RESET_ACK_CORE_RX3 */
} E_N5C32GP5X4_PIN;

/* TXEQ Parameters */
typedef enum {
    N5C32GP5X4_TXEQ_EM_PRE2    = 0,
    N5C32GP5X4_TXEQ_EM_PRE     = 1,
    N5C32GP5X4_TXEQ_EM_MAIN    = 2,
    N5C32GP5X4_TXEQ_EM_POST    = 3,
    N5C32GP5X4_TXEQ_EM_NA      = 4,
} E_N5C32GP5X4_TXEQ_PARAM;

/* CTLE Parameters */
typedef enum {
    N5C32GP5X4_CTLE_CUR1_SEL        = 0,
    N5C32GP5X4_CTLE_RL1_SEL         = 1,
    N5C32GP5X4_CTLE_RL1_EXTRA       = 2,
    N5C32GP5X4_CTLE_RES1_SEL        = 3,
    N5C32GP5X4_CTLE_CAP1_SEL        = 4,
    N5C32GP5X4_CTLE_EN_MIDFREQ      = 5,
    N5C32GP5X4_CTLE_CS1_MID         = 6,
    N5C32GP5X4_CTLE_RS1_MID         = 7,
    N5C32GP5X4_CTLE_CUR2_SEL        = 8,
    N5C32GP5X4_CTLE_RL2_SEL         = 9,
    N5C32GP5X4_CTLE_RL2_TUNE_G      = 10,
    N5C32GP5X4_CTLE_RES2_SEL        = 11,
    N5C32GP5X4_CTLE_CAP2_SEL        = 12,
} E_N5C32GP5X4_CTLE_PARAM;

/* CDR Parameters */
typedef enum {
    N5C32GP5X4_CDR_SELMUFI          = 0,
    N5C32GP5X4_CDR_SELMUFF          = 1,
    N5C32GP5X4_CDR_SELMUPI          = 2,
    N5C32GP5X4_CDR_SELMUPF          = 3,
} E_N5C32GP5X4_CDR_PARAM;

/* Pattern Selection */
typedef enum {
    N5C32GP5X4_PAT_USER             = 0x0,
    N5C32GP5X4_PAT_PACKET           = 0x3,
    N5C32GP5X4_PAT_JITTER_K28P5     = 0x8,
    N5C32GP5X4_PAT_JITTER_1T        = 0x9,
    N5C32GP5X4_PAT_JITTER_2T        = 0xA,
    N5C32GP5X4_PAT_JITTER_4T        = 0xB,
    N5C32GP5X4_PAT_JITTER_5T        = 0xC,
    N5C32GP5X4_PAT_JITTER_8T        = 0xD,
    N5C32GP5X4_PAT_JITTER_10T       = 0xE,
    N5C32GP5X4_PAT_PRBS7            = 0x10,
    N5C32GP5X4_PAT_PRBS9            = 0x11,
    N5C32GP5X4_PAT_PRBS11           = 0x12,
    N5C32GP5X4_PAT_PRBS11_0         = 0x13,
    N5C32GP5X4_PAT_PRBS11_1         = 0x14,
    N5C32GP5X4_PAT_PRBS11_2         = 0x15,
    N5C32GP5X4_PAT_PRBS11_3         = 0x16,
    N5C32GP5X4_PAT_PRBS15           = 0x17,
    N5C32GP5X4_PAT_PRBS16           = 0x18,
    N5C32GP5X4_PAT_PRBS23           = 0x19,
    N5C32GP5X4_PAT_PRBS31           = 0x1A,
    N5C32GP5X4_PAT_PRBS32           = 0x1B,
} E_N5C32GP5X4_PATTERN;

/* Pattern: Gray Code */
typedef enum {
    N5C32GP5X4_GRAY_DISABLE         = 0,
    N5C32GP5X4_GRAY_ENABLE          = 1,
    N5C32GP5X4_GRAY_NOT_USED        = 2,
} E_N5C32GP5X4_GRAY_CODE;

/* Pattern Comparator Statistics */
typedef struct
{
    MCESD_BOOL lock;
    MCESD_BOOL pass;
    MCESD_U64 totalBits;
    MCESD_U64 totalErrorBits;
} S_N5C32GP5X4_PATTERN_STATS;

/* Trained Eye Height */
typedef struct
{
    MCESD_U32 f0b;
    MCESD_U32 f0n1;
    MCESD_U32 f0n2;
    MCESD_U32 f0d;
} S_N5C32GP5X4_TRAINED_EYE_H;

/* Eye Measurement Data @ X,Y and X,-Y */
typedef struct
{
    MCESD_32 phase;
    MCESD_U8 voltage;
    MCESD_U64 upperBitCount;
    MCESD_U32 upperBitErrorCount;
    MCESD_U64 lowerBitCount;
    MCESD_U32 lowerBitErrorCount;
} S_N5C32GP5X4_EOM_DATA;

/* Eye Raw Data */
typedef struct
{
    MCESD_32 eyeRawData[N5C32GP5X4_EYE_MAX_PHASE_LEVEL][(N5C32GP5X4_EYE_MAX_VOLT_STEPS * 2) - 1];
    MCESD_U32 oneUIwidth;
    MCESD_U32 upperEdge;
    MCESD_U32 lowerEdge;
    MCESD_U32 leftEdge;
    MCESD_U32 rightEdge;
    MCESD_U32 relativeCenter;
    MCESD_U32 sampleCount;
} S_N5C32GP5X4_EYE_RAW, *S_N5C32GP5X4_EYE_RAW_PTR;

#ifdef MCESD_EOM_STATS
/* Buffer Structure for Eye-Related Functions */
typedef struct
{
    double bufferData[2][(N5C32GP5X4_EYE_MAX_PHASE_LEVEL * 2) + 1];
} S_N5C32GP5X4_EYE_BUFFER, *S_N5C32GP5X4_EYE_BUFFER_PTR;
#endif

/* DFE Taps */
typedef enum {
    N5C32GP5X4_DFE_DC               = 0,
    N5C32GP5X4_DFE_DC_E             = 1,
    N5C32GP5X4_DFE_F0               = 2,
    N5C32GP5X4_DFE_F1               = 3,
    N5C32GP5X4_DFE_F1P5             = 4,
    N5C32GP5X4_DFE_F2               = 5,
    N5C32GP5X4_DFE_F3               = 6,
    N5C32GP5X4_DFE_F4               = 7,
    N5C32GP5X4_DFE_F5               = 8,
    N5C32GP5X4_DFE_F6               = 9,
    N5C32GP5X4_DFE_F7               = 10,
    N5C32GP5X4_DFE_F8               = 11,
    N5C32GP5X4_DFE_F9               = 12,
    N5C32GP5X4_DFE_F10              = 13,
    N5C32GP5X4_DFE_F11              = 14,
    N5C32GP5X4_DFE_F12              = 15,
    N5C32GP5X4_DFE_F13              = 16,
    N5C32GP5X4_DFE_F14              = 17,
    N5C32GP5X4_DFE_F15              = 18,
    N5C32GP5X4_DFE_FF0              = 19,
    N5C32GP5X4_DFE_FF1              = 20,
    N5C32GP5X4_DFE_FF2              = 21,
    N5C32GP5X4_DFE_FF3              = 22,
    N5C32GP5X4_DFE_FF4              = 23,
    N5C32GP5X4_DFE_FF5              = 24,
} E_N5C32GP5X4_DFE_TAP;

/* AVDD Select */
typedef enum {
    N5C32GP5X4_AVDD_0P8V            = 0,
    N5C32GP5X4_AVDD_0P85V           = 1,
    N5C32GP5X4_AVDD_0P95V           = 2,
    N5C32GP5X4_AVDD_1P0V            = 3,
    N5C32GP5X4_AVDD_1P05V           = 4,
    N5C32GP5X4_AVDD_1P1V            = 5,
    N5C32GP5X4_AVDD_1P15V           = 6,
    N5C32GP5X4_AVDD_1P2V            = 7,
} E_N5C32GP5X4_AVDD;

/* Speed Configuration */
typedef enum {
    N5C32GP5X4_SPD_CFG_4PLL         = 0x2,
    N5C32GP5X4_SPD_CFG_ONLY_TS      = 0x3,
} E_N5C32GP5X4_SPD_CFG;

#ifdef N5C32GP5X4_DFE_MILLIVOLTS
/* DFE mV Table */
static const MCESD_U32 N5C32GP5X4_DFE_F0_TABLE[2][2][4][64] = /* [avdd_hi_lo][dc_shift][res][index] */
{
    {   /* AVDD LO maps to Config 2*/
        {   /* DC SHIFT  0*/
            {   /* RES0 */
                   00,     190,     490,     710,     970,    1200,    1310,    1440,
                 1520,    1650,    1710,    1790,    1830,    1920,    2020,    2110,
                 2170,    2250,    2340,    2440,    2530,    2610,    2690,    2750,
                 2860,    2970,    3050,    3160,    3260,    3350,    3450,    3540,
                 3670,    3750,    3860,    3990,    4070,    4200,    4340,    4470,
                 4600,    4680,    4860,    4920,    5050,    5230,    5340,    5470,
                 5630,    5780,    5890,    6070,    6230,    6360,    6520,    6700,
                 6830,    6940,    7120,    7250,    7380,    7590,    7670,    7880,
            },
            {   /* RES1 */
                   00,     190,     490,     740,     990,    1270,    1370,    1500,
                 1680,    1810,    1910,    2020,    2150,    2260,    2390,    2500,
                 2610,    2740,    2870,    2980,    3110,    3190,    3350,    3480,
                 3640,    3810,    3920,    4080,    4260,    4390,    4550,    4750,
                 4880,    5090,    5220,    5420,    5630,    5830,    5990,    6170,
                 6400,    6580,    6760,    6940,    7200,    7350,    7530,    7720,
                 7900,    8080,    8330,    8490,    8670,    8880,    9060,    9220,
                 9520,    9730,    9790,    9990,   10220,   10500,   10730,   10750,
            },
            {   /* RES2 */
                   00,     210,     490,     760,    1010,    1310,    1500,    1720,
                 1990,    2190,    2390,    2560,    2720,    2920,    3100,    3300,
                 3550,    3730,    3940,    4140,    4420,    4640,    4920,    5170,
                 5400,    5650,    5930,    6210,    6510,    6760,    7090,    7370,
                 7620,    7880,    8110,    8410,    8670,    9040,    9270,    9620,
                 9860,   10190,   10420,   10700,   11170,   11310,   11490,   11870,
                12100,   12430,   12780,   12920,   13300,   13530,   14150,   14290,
                14660,   14920,   15130,   15510,   15720,   16070,   16420,   16510,
            },
            {   /* RES3 */
                   00,     190,     510,     780,    1070,    1370,    1680,    2030,
                 2390,    2770,    3060,    3330,    3650,    4000,    4270,    4590,
                 5030,    5380,    5750,    6190,    6540,    6940,    7360,    7780,
                 8160,    8600,    9030,    9360,    9780,   10250,   10800,   11030,
                11510,   11810,   12340,   12760,   13330,   13780,   13830,   14710,
                14780,   15400,   16000,   16280,   16640,   17070,   17540,   18000,
                18260,   18780,   19070,   19470,   19830,   20260,   20690,   20950,
                21360,   21660,   21880,   22310,   22640,   22970,   23310,   23710,
            }
        },
        {   /* DC SHIFT  1*/
            {   /* RES0 */
                  662,    1688,    2330,    2755,    3227,    3638,    3790,    4005,
                 4175,    4321,    4378,    4520,    4617,    4758,    4899,    4993,
                 5180,    5319,    5456,    5595,    5710,    5846,    5962,    6076,
                 6258,    6393,    6436,    6643,    6755,    6892,    6980,    7092,
                 7320,    7433,    7522,    7633,    7792,    7879,    8035,    8243,
                 8401,    8488,    8645,    8733,    8915,    8978,    9184,    9271,
                 9427,    9562,    9697,    9805,    9917,   10076,   10304,   10438,
                10550,   10684,   10767,   10951,   11037,   11219,   11351,   11486,
            },
            {   /* RES1 */
                  663,    1729,    2413,    2880,    3312,    3789,    4007,    4226,
                 4419,    4636,    4849,    4974,    5117,    5306,    5450,    5615,
                 5894,    6061,    6270,    6435,    6598,    6807,    6924,    7155,
                 7340,    7481,    7666,    7875,    8013,    8221,    8407,    8543,
                 8729,    8937,    9169,    9305,    9560,    9627,    9810,   10042,
                10156,   10340,   10596,   10803,   10963,   11169,   11355,   11467,
                11698,   11907,   12115,   12275,   12458,   12618,   12850,   12890,
                13072,   13354,   13560,   13695,   13854,   14038,   14246,   14501,
            },
            {   /* RES2 */
                  663,    1790,    2537,    3007,    3462,    3984,    4270,    4601,
                 4910,    5239,    5458,    5650,    5914,    6129,    6367,    6583,
                 6981,    7242,    7500,    7784,    8046,    8259,    8472,    8799,
                 9083,    9296,    9533,    9767,   10050,   10333,   10593,   10828,
                11134,   11342,   11651,   11936,   12051,   12381,   12687,   12875,
                13106,   13440,   13794,   13982,   14264,   14499,   14755,   15061,
                15273,   15531,   15886,   16024,   16380,   16591,   16946,   17056,
                17365,   17523,   17854,   18066,   18277,   18631,   18865,   19100,
            },
            {   /* RES3 */
                  682,    1892,    2643,    3156,    3697,    4137,    4578,    5020,
                 5464,    5933,    6199,    6513,    6869,    7159,    7446,    7777,
                 8344,    8631,    9012,    9276,    9724,   10039,   10348,   10683,
                11132,   11420,   11730,   12135,   12493,   12850,   13160,   13541,
                13826,   14157,   14470,   14875,   15281,   15519,   15949,   16377,
                16591,   16925,   17403,   17663,   18069,   18331,   18735,   19165,
                19450,   19833,   20044,   20426,   20735,   21022,   21524,   21663,
                22046,   22380,   22736,   23044,   23499,   23493,   23803,   24063,
            }
        },
    },
    {   /* AVDD HI maps to Config 1*/
        {   /* DC SHIFT  0*/
            {   /* RES0 */
                   60,     330,     600,     800,    1030,    1270,    1350,    1460,
                 1570,    1670,    1760,    1840,    1910,    1990,    2070,    2160,
                 2240,    2320,    2430,    2510,    2600,    2710,    2790,    2900,
                 3000,    3110,    3220,    3320,    3430,    3540,    3650,    3750,
                 3860,    3990,    4080,    4210,    4320,    4470,    4580,    4710,
                 4820,    4950,    5080,    5210,    5300,    5480,    5580,    5740,
                 5850,    5980,    6090,    6240,    6420,    6480,    6620,    6800,
                 6950,    7040,    7170,    7400,    7530,    7640,    7820,    7930,
            },
            {   /* RES1 */
                   60,     330,     600,     800,    1030,    1290,    1420,    1570,
                 1700,    1850,    1980,    2090,    2200,    2330,    2440,    2550,
                 2680,    2810,    2970,    3070,    3230,    3360,    3520,    3650,
                 3830,    3960,    4120,    4270,    4430,    4610,    4740,    4920,
                 5120,    5300,    5480,    5620,    5800,    5980,    6140,    6320,
                 6520,    6680,    6910,    7070,    7270,    7450,    7630,    7890,
                 8090,    8280,    8430,    8610,    8890,    9120,    9280,    9460,
                 9670,    9920,   10110,   10310,   10500,   10730,   10930,   11140,
            },
            {   /* RES2 */
                   60,     330,     600,     820,    1080,    1330,    1550,    1770,
                 2010,    2280,    2460,    2640,    2840,    3040,    3240,    3450,
                 3670,    3900,    4150,    4380,    4610,    4880,    5130,    5360,
                 5640,    5940,    6190,    6450,    6730,    7070,    7330,    7630,
                 7940,    8260,    8640,    8870,    9170,    9450,    9800,   10160,
                10410,   10740,   11000,   11330,   11680,   12060,   12240,   12550,
                13000,   13180,   13610,   13940,   14170,   14550,   14810,   15210,
                15390,   15750,   16170,   16480,   16860,   17090,   17520,   17760,
            },
            {   /* RES3 */
                   50,     300,     560,     790,    1060,    1340,    1670,    2040,
                 2470,    2940,    3280,    3660,    4030,    4420,    4830,    5160,
                 5670,    6130,    6550,    7040,    7510,    7960,    8430,    8900,
                 9390,    9870,   10410,   10840,   11360,   11860,   12450,   12830,
                13430,   13830,   14400,   14830,   15360,   15810,   16290,   16840,
                17290,   17740,   18320,   18800,   19230,   19710,   20210,   20740,
                21240,   21600,   22060,   22630,   22920,   23490,   23800,   24330,
                24670,   25240,   26040,   26040,   26420,   27120,   27280,   27860,
            }
        },
        {   /* DC SHIFT  1*/
            {   /* RES0 */
                  530,    1400,    2000,    2380,    2780,    3190,    3380,    3520,
                 3690,    3850,    3950,    4090,    4190,    4280,    4420,    4560,
                 4680,    4790,    4930,    5060,    5150,    5310,    5380,    5580,
                 5650,    5810,    5900,    6060,    6140,    6280,    6410,    6570,
                 6680,    6860,    6930,    7110,    7240,    7380,    7490,    7670,
                 7830,    7960,    8030,    8230,    8270,    8520,    8680,    8720,
                 8950,    9130,    9220,    9370,    9530,    9710,    9800,    9960,
                10090,   10230,   10340,   10590,   10700,   10830,   11010,   11220,
            },
            {   /* RES1 */
                  530,    1450,    2060,    2460,    2910,    3310,    3550,    3740,
                 3960,    4210,    4380,    4520,    4640,    4850,    5020,    5180,
                 5350,    5510,    5670,    5830,    6040,    6200,    6390,    6550,
                 6710,    6900,    7100,    7290,    7520,    7630,    7840,    8070,
                 8230,    8390,    8620,    8830,    8940,    9170,    9330,    9560,
                 9680,    9930,   10170,   10330,   10530,   10790,   10950,   11150,
                11390,   11550,   11660,   11890,   12100,   12380,   12490,   12750,
                12880,   13110,   13300,   13430,   13710,   13940,   14030,   14290,
            },
            {   /* RES2 */
                  530,    1510,    2170,    2650,    3100,    3570,    3880,    4230,
                 4560,    4890,    5170,    5430,    5690,    5900,    6210,    6470,
                 6750,    6970,    7270,    7530,    7820,    8050,    8380,    8620,
                 8950,    9270,    9460,    9820,   10050,   10430,   10710,   10860,
                11260,   11540,   11820,   12130,   12440,   12720,   13000,   13210,
                13640,   13880,   14130,   14420,   14720,   15080,   15460,   15620,
                16000,   16280,   16520,   16780,   17280,   17420,   17720,   18000,
                18340,   18600,   18900,   19240,   19490,   19680,   20040,   20300,
            },
            {   /* RES3 */
                  550,    1630,    2350,    2860,    3380,    3900,    4360,    4870,
                 5490,    5960,    6380,    6740,    7190,    7590,    8000,    8380,
                 8850,    9240,    9690,   10120,   10520,   10950,   11380,   11860,
                12270,   12650,   13100,   13420,   13920,   14320,   14800,   15190,
                15590,   16050,   16430,   16860,   17320,   17770,   18230,   18560,
                19020,   19400,   19790,   20190,   20700,   21030,   21540,   21800,
                22140,   22830,   23020,   23400,   23640,   24170,   24530,   24890,
                25400,   25660,   26020,   26500,   26640,   27290,   27570,   27860,
            }
        },
    },
};
#endif

typedef struct
{
    union
    {
        struct
        {
            MCESD_U32 powerLane0 : 1;
            MCESD_U32 powerLane1 : 1;
            MCESD_U32 powerLane2 : 1;
            MCESD_U32 powerLane3 : 1;
            MCESD_U32 powerReserved : 28;
        }powerLane;
        MCESD_U32 powerLaneMask;
    }u;
    MCESD_BOOL initTx;
    MCESD_BOOL initRx;
    MCESD_BOOL txOutputEn;
    MCESD_BOOL downloadFw;
    E_N5C32GP5X4_DATAPATH dataPath;
    E_N5C32GP5X4_REFCLK_SEL txRefClkSel;
    E_N5C32GP5X4_REFCLK_SEL rxRefClkSel;
    E_N5C32GP5X4_REFFREQ txRefFreq;
    E_N5C32GP5X4_REFFREQ rxRefFreq;
    E_N5C32GP5X4_DATABUS_WIDTH dataBusWidth;
    E_N5C32GP5X4_SERDES_SPEED txSpeed;
    E_N5C32GP5X4_SERDES_SPEED rxSpeed;
    E_N5C32GP5X4_AVDD avdd;
    E_N5C32GP5X4_SPD_CFG spdCfg;
    FMCESD_FW_DOWNLOAD_NEW fwDownload;
    MCESD_U32 *fwCodePtr;
    MCESD_U32 fwCodeSizeDW;
    MCESD_U32 address;
    MCESD_U32 rxInitTimeout;
} S_N5C32GP5X4_PowerOn;

#define F_N5C32GP5X4_PHY_MODE           FIELD_DEFINE(0xA424, 14, 12)
#define F_N5C32GP5X4_PHY_GEN_TX         FIELD_DEFINE(0x5534, 17, 8)
#define F_N5C32GP5X4_PHY_GEN_RX         FIELD_DEFINE(0x5624, 17, 8)
#define F_N5C32GP5X4_REF_FREF_SEL_TX    FIELD_DEFINE(0x553C, 23, 16)
#define F_N5C32GP5X4_REF_FREF_SEL_RX    FIELD_DEFINE(0x5634, 23, 16)
#define F_N5C32GP5X4_REFCLK_SEL_TX      FIELD_DEFINE(0x553C, 14, 14)
#define F_N5C32GP5X4_REFCLK_SEL_RX      FIELD_DEFINE(0x5634, 14, 14)
#define F_N5C32GP5X4_TX_SEL_BITS        FIELD_DEFINE(0x3034, 31, 31)
#define F_N5C32GP5X4_RX_SEL_BITS        FIELD_DEFINE(0x3204, 31, 31)
#define F_N5C32GP5X4_TXD_INV            FIELD_DEFINE(0x3024, 30, 30)
#define F_N5C32GP5X4_RXD_INV            FIELD_DEFINE(0x3248, 29, 29)
#define F_N5C32GP5X4_TX_FIR_C0          FIELD_DEFINE(0x30CC, 29, 24)
#define F_N5C32GP5X4_TX_FIR_C0_FORCE    FIELD_DEFINE(0x30CC, 30, 30)
#define F_N5C32GP5X4_TX_FIR_C1          FIELD_DEFINE(0x30CC, 22, 17)
#define F_N5C32GP5X4_TX_FIR_C1_FORCE    FIELD_DEFINE(0x30CC, 23, 23)
#define F_N5C32GP5X4_TX_FIR_C2          FIELD_DEFINE(0x30CC, 14, 9)
#define F_N5C32GP5X4_TX_FIR_C2_FORCE    FIELD_DEFINE(0x30CC, 15, 15)
#define F_N5C32GP5X4_TX_FIR_C3          FIELD_DEFINE(0x30CC, 6, 1)
#define F_N5C32GP5X4_TX_FIR_C3_FORCE    FIELD_DEFINE(0x30CC, 7, 7)
#define F_N5C32GP5X4_TX_FIR_C4          FIELD_DEFINE(0x30D0, 30, 25)
#define F_N5C32GP5X4_TX_FIR_C4_FORCE    FIELD_DEFINE(0x30D0, 31, 31)
#define F_N5C32GP5X4_TX_FIR_C5          FIELD_DEFINE(0x30D0, 22, 17)
#define F_N5C32GP5X4_TX_FIR_C5_FORCE    FIELD_DEFINE(0x30D0, 23, 23)
#define F_N5C32GP5X4_TX_UP              FIELD_DEFINE(0x30D0, 8, 8)
#define F_N5C32GP5X4_TX_UP_FORCE        FIELD_DEFINE(0x30D0, 7, 7)      /* TX_FIR_UPDATE_FORCE_LANE */
#define F_N5C32GP5X4_TO_ANA_TX_FIR_C0   FIELD_DEFINE(0x30D4, 29, 24)
#define F_N5C32GP5X4_TO_ANA_TX_FIR_C1   FIELD_DEFINE(0x30D4, 21, 16)
#define F_N5C32GP5X4_TO_ANA_TX_FIR_C2   FIELD_DEFINE(0x30D4, 13, 8)
#define F_N5C32GP5X4_TO_ANA_TX_FIR_C3   FIELD_DEFINE(0x30D4, 5, 0)
#define F_N5C32GP5X4_TO_ANA_TX_FIR_C4   FIELD_DEFINE(0x30D8, 29, 24)
#define F_N5C32GP5X4_TO_ANA_TX_FIR_C5   FIELD_DEFINE(0x30D8, 21, 16)
#define F_N5C32GP5X4_CTLE_CURRENT1_SEL  FIELD_DEFINE(0x1050, 3, 0)
#define F_N5C32GP5X4_CTLE_RL1_SEL       FIELD_DEFINE(0x1060, 3, 0)
#define F_N5C32GP5X4_CTLE_RL1_EXTRA     FIELD_DEFINE(0x1048, 6, 4)
#define F_N5C32GP5X4_CTLE_RES1_SEL      FIELD_DEFINE(0x1058, 3, 0)
#define F_N5C32GP5X4_CTLE_CAP1_SEL_G    FIELD_DEFINE(0x1048, 3, 0)
#define F_N5C32GP5X4_EN_CTLE_MID_FREQ   FIELD_DEFINE(0x1088, 4, 4)
#define F_N5C32GP5X4_CTLE_CS1_MID       FIELD_DEFINE(0x1040, 5, 4)
#define F_N5C32GP5X4_CTLE_RS1_MID       FIELD_DEFINE(0x1040, 7, 6)
#define F_N5C32GP5X4_CTLE_CURRENT2_SEL  FIELD_DEFINE(0x1054, 3, 0)
#define F_N5C32GP5X4_CTLE_RL2_SEL       FIELD_DEFINE(0x1064, 3, 0)
#define F_N5C32GP5X4_CTLE_RL2_TUNE_G    FIELD_DEFINE(0x1064, 6, 4)
#define F_N5C32GP5X4_CTLE_RES2_SEL      FIELD_DEFINE(0x105C, 3, 0)
#define F_N5C32GP5X4_CTLE_CAP2_SEL      FIELD_DEFINE(0x104C, 3, 0)
#define F_N5C32GP5X4_RX_SELMUFI         FIELD_DEFINE(0x3200, 3, 0)
#define F_N5C32GP5X4_RX_SELMUFF         FIELD_DEFINE(0x3200, 7, 4)
#define F_N5C32GP5X4_REG_SELMUPI        FIELD_DEFINE(0x10CC, 2, 0)
#define F_N5C32GP5X4_REG_SELMUPF        FIELD_DEFINE(0x10C4, 2, 0)
#define F_N5C32GP5X4_PT_TX_PATTERN_SEL  FIELD_DEFINE(0x3098, 29, 24)
#define F_N5C32GP5X4_PT_RX_PATTERN_SEL  FIELD_DEFINE(0x3280, 29, 24)
#define F_N5C32GP5X4_TXDATA_GRAY_EN     FIELD_DEFINE(0x553C, 12, 12)    /* TXDATA_GRAY_CODE_EN_LANE */
#define F_N5C32GP5X4_RXDATA_GRAY_EN     FIELD_DEFINE(0x5644, 30, 30)    /* RXDATA_GRAY_CODE_EN_LANE */
#define F_N5C32GP5X4_DFE_DC_D_E_SM      FIELD_DEFINE(0x40B4, 6, 0)
#define F_N5C32GP5X4_DFE_DC_E_E_SM      FIELD_DEFINE(0x40B4, 22, 16)
#define F_N5C32GP5X4_DFE_F0_D_E_SM      FIELD_DEFINE(0x40B4, 29, 24)
#define F_N5C32GP5X4_DFE_F1_D_E_SM      FIELD_DEFINE(0x40B8, 14, 8)
#define F_N5C32GP5X4_DFE_F1P5_E_SM      FIELD_DEFINE(0x40D0, 5, 0)
#define F_N5C32GP5X4_DFE_F2_D_E_SM      FIELD_DEFINE(0x40B8, 29, 24)
#define F_N5C32GP5X4_DFE_F3_E_SM        FIELD_DEFINE(0x40BC, 14, 8)
#define F_N5C32GP5X4_DFE_F4_E_SM        FIELD_DEFINE(0x40BC, 22, 16)
#define F_N5C32GP5X4_DFE_F5_E_SM        FIELD_DEFINE(0x40BC, 29, 24)
#define F_N5C32GP5X4_DFE_F6_E_SM        FIELD_DEFINE(0x40C0, 5, 0)
#define F_N5C32GP5X4_DFE_F7_E_SM        FIELD_DEFINE(0x40C0, 13, 8)
#define F_N5C32GP5X4_DFE_F8_E_SM        FIELD_DEFINE(0x40C0, 21, 16)
#define F_N5C32GP5X4_DFE_F9_E_SM        FIELD_DEFINE(0x40C0, 29, 24)
#define F_N5C32GP5X4_DFE_F10_E_SM       FIELD_DEFINE(0x40C4, 5, 0)
#define F_N5C32GP5X4_DFE_F11_E_SM       FIELD_DEFINE(0x40C4, 12, 8)
#define F_N5C32GP5X4_DFE_F12_E_SM       FIELD_DEFINE(0x40C4, 20, 16)
#define F_N5C32GP5X4_DFE_F13_E_SM       FIELD_DEFINE(0x40C4, 28, 24)
#define F_N5C32GP5X4_DFE_F14_E_SM       FIELD_DEFINE(0x40C8, 4, 0)
#define F_N5C32GP5X4_DFE_F15_E_SM       FIELD_DEFINE(0x40C8, 12, 8)
#define F_N5C32GP5X4_DFE_FF0_E_SM       FIELD_DEFINE(0x40C8, 21, 16)
#define F_N5C32GP5X4_DFE_FF1_E_SM       FIELD_DEFINE(0x40C8, 29, 24)
#define F_N5C32GP5X4_DFE_FF2_E_SM       FIELD_DEFINE(0x40CC, 5, 0)
#define F_N5C32GP5X4_DFE_FF3_E_SM       FIELD_DEFINE(0x40CC, 13, 8)
#define F_N5C32GP5X4_DFE_FF4_E_SM       FIELD_DEFINE(0x40CC, 21, 16)
#define F_N5C32GP5X4_DFE_FF5_E_SM       FIELD_DEFINE(0x40CC, 29, 24)
#define F_N5C32GP5X4_AVDD_SEL           FIELD_DEFINE(0xA420, 28, 26)
#define F_N5C32GP5X4_PIN_AVDD_SEL_RD    FIELD_DEFINE(0xA400, 31, 29)
#define F_N5C32GP5X4_SPD_CFG            FIELD_DEFINE(0xA424, 7, 4)
#define F_N5C32GP5X4_LANE_SEL           FIELD_DEFINE(0xA318, 31, 28)
#define F_N5C32GP5X4_DFE_EN             FIELD_DEFINE(0x5648, 29, 29)
#define F_N5C32GP5X4_BROADCAST          FIELD_DEFINE(0xA318, 27, 27)
#define F_N5C32GP5X4_ANA_PLL_RS_LOCK_RD FIELD_DEFINE(0x5800, 30, 30)
#define F_N5C32GP5X4_ANA_PLL_TS_LOCK_RD FIELD_DEFINE(0x5700, 6, 6)
#define F_N5C32GP5X4_PLL_READY_TX       FIELD_DEFINE(0x3000, 20, 20)
#define F_N5C32GP5X4_PLL_READY_RX       FIELD_DEFINE(0x3200, 24, 24)
#define F_N5C32GP5X4_PH_OS_DAT          FIELD_DEFINE(0x6D6C, 7, 0)
#define F_N5C32GP5X4_SQ_CAL_RESULT_RD   FIELD_DEFINE(0x1AF0, 6, 1)
#define F_N5C32GP5X4_SQ_CAL_RESULT_EXT  FIELD_DEFINE(0x1AEC, 7, 2)
#define F_N5C32GP5X4_SQ_CAL_INDV_EXT_EN FIELD_DEFINE(0x1AE4, 0, 0)
#define F_N5C32GP5X4_FW_MAJOR_VER       FIELD_DEFINE(0xE600, 31, 24)
#define F_N5C32GP5X4_FW_MINOR_VER       FIELD_DEFINE(0xE600, 23, 16)
#define F_N5C32GP5X4_FW_PATCH_VER       FIELD_DEFINE(0xE600, 15, 8)
#define F_N5C32GP5X4_FW_BUILD_VER       FIELD_DEFINE(0xE600, 7, 0)
#define F_N5C32GP5X4_ADD_ERR_EN         FIELD_DEFINE(0x3024, 29, 29)
#define F_N5C32GP5X4_ADD_ERR_NUM        FIELD_DEFINE(0x3024, 28, 26)
#define F_N5C32GP5X4_DFE_F0_D_O_2C      FIELD_DEFINE(0x4118, 31, 24)
#define F_N5C32GP5X4_DFE_F0_S_O_2C      FIELD_DEFINE(0x411C, 7, 0)
#define F_N5C32GP5X4_DFE_F0_D_E_2C      FIELD_DEFINE(0x40F8, 31, 24)
#define F_N5C32GP5X4_DFE_F0_S_E_2C      FIELD_DEFINE(0x40FC, 7, 0)
#define F_N5C32GP5X4_DFE_ADAPT_CONT     FIELD_DEFINE(0x4000, 13, 13)
#define F_N5C32GP5X4_CLI_CMD            FIELD_DEFINE(0x6068, 7, 0)
#define F_N5C32GP5X4_CLI_ARGS           FIELD_DEFINE(0x606C, 31, 0)
#define F_N5C32GP5X4_CLI_START          FIELD_DEFINE(0x6068, 8, 8)
#define F_N5C32GP5X4_DFE_F0X_SEL        FIELD_DEFINE(0x401C, 14, 10)
#define F_N5C32GP5X4_DFE_LOAD_EN        FIELD_DEFINE(0x6D60, 15, 15)
#define F_N5C32GP5X4_TRX_TIMER          FIELD_DEFINE(0x5018, 31, 16)
#define F_N5C32GP5X4_RX_TIMER           FIELD_DEFINE(0x5018, 15, 0)
#define F_N5C32GP5X4_TRX_TRAIN_TIMEOUT  FIELD_DEFINE(0x5014, 10, 10)    /* TRX_TRAIN_TIMEOUT_EN_LANE */
#define F_N5C32GP5X4_D_TX2RX_LPBK       FIELD_DEFINE(0x3248, 31, 31)    /* LOCAL_DIG_TX2RX_LPBK_EN_LANE */
#define F_N5C32GP5X4_A_TX2RX_LPBK       FIELD_DEFINE(0x1130, 3, 3)      /* LOCAL_ANA_TX2RX_LPBK_EN_LANE */
#define F_N5C32GP5X4_D_RX2TX_LPBK       FIELD_DEFINE(0x3024, 31, 31)    /* LOCAL_DIG_RX2TX_LPBK_EN_LANE */
#define F_N5C32GP5X4_DTL_CLAMPING_SEL   FIELD_DEFINE(0x3260, 26, 24)
#define F_N5C32GP5X4_R2T_NO_STOP        FIELD_DEFINE(0x3024, 25, 25)
#define F_N5C32GP5X4_R2T_RD_START       FIELD_DEFINE(0x3024, 24, 24)    /* RX2TX_FIFO_RD_START_POINT_LANE */
#define F_N5C32GP5X4_RX_FOFFSET_DISABLE FIELD_DEFINE(0x3268, 16, 16)
#define F_N5C32GP5X4_PU_LB              FIELD_DEFINE(0x10B8, 3, 3)
#define F_N5C32GP5X4_TXDATA_PRE_CODE_EN FIELD_DEFINE(0x553C, 10, 10)
#define F_N5C32GP5X4_RXDATA_PRE_CODE_EN FIELD_DEFINE(0x5644, 28, 28)
#define F_N5C32GP5X4_RX_CNT_4732        FIELD_DEFINE(0x3290, 31, 16)    /* PT_RX_CNT_LANE[47:32] */
#define F_N5C32GP5X4_RX_CNT_3100        FIELD_DEFINE(0x3294, 31, 0)     /* PT_RX_CNT_LANE[31:0] */
#define F_N5C32GP5X4_RX_ERR_4732        FIELD_DEFINE(0x3298, 31, 16)    /* PT_RX_ERR_CNT_LANE[47:32] */
#define F_N5C32GP5X4_RX_ERR_3100        FIELD_DEFINE(0x329C, 31, 0)     /* PT_RX_ERR_CNT_LANE[31:0] */
#define F_N5C32GP5X4_TX_UP_7948         FIELD_DEFINE(0x309C, 31, 0)     /* PT_TX_USER_PATTERN_LANE[79:48] */
#define F_N5C32GP5X4_TX_UP_4716         FIELD_DEFINE(0x30A0, 31, 0)     /* PT_TX_USER_PATTERN_LANE[47:16] */
#define F_N5C32GP5X4_TX_UP_1500         FIELD_DEFINE(0x30A4, 31, 16)    /* PT_TX_USER_PATTERN_LANE[15:0] */
#define F_N5C32GP5X4_RX_UP_7948         FIELD_DEFINE(0x3284, 31, 0)     /* PT_RX_USER_PATTERN_LANE[79:48] */
#define F_N5C32GP5X4_RX_UP_4716         FIELD_DEFINE(0x3288, 31, 0)     /* PT_RX_USER_PATTERN_LANE[47:16] */
#define F_N5C32GP5X4_RX_UP_1500         FIELD_DEFINE(0x328C, 31, 16)    /* PT_RX_USER_PATTERN_LANE[15:0] */
#define F_N5C32GP5X4_TX_PHYREADY        FIELD_DEFINE(0x3098, 30, 30)    /* PT_TX_PHYREADY_FORCE_LANE */
#define F_N5C32GP5X4_RX_PHYREADY        FIELD_DEFINE(0x3280, 22, 22)    /* PT_RX_PHYREADY_FORCE_LANE */
#define F_N5C32GP5X4_PT_TX_EN_MODE      FIELD_DEFINE(0x3098, 3, 2)
#define F_N5C32GP5X4_PT_RX_EN_MODE      FIELD_DEFINE(0x3280, 31, 30)
#define F_N5C32GP5X4_PT_TX_EN           FIELD_DEFINE(0x3098, 31, 31)
#define F_N5C32GP5X4_PT_RX_EN           FIELD_DEFINE(0x3280, 23, 23)
#define F_N5C32GP5X4_PT_TRX_EN          FIELD_DEFINE(0x328C, 8, 8)
#define F_N5C32GP5X4_PT_TX_RST          FIELD_DEFINE(0x3098, 5, 5)
#define F_N5C32GP5X4_PT_RX_RST          FIELD_DEFINE(0x328C, 7, 7)
#define F_N5C32GP5X4_PT_RX_LOCK         FIELD_DEFINE(0x328C, 0, 0)
#define F_N5C32GP5X4_PT_RX_PASS         FIELD_DEFINE(0x328C, 1, 1)
#define F_N5C32GP5X4_PT_RX_CNT_RST      FIELD_DEFINE(0x3280, 21, 21)
#define F_N5C32GP5X4_DFE_RATE_MODE      FIELD_DEFINE(0x4010, 9, 9)
#define F_N5C32GP5X4_DFE_UPDATE_DIS     FIELD_DEFINE(0x5648, 25, 25)
#define F_N5C32GP5X4_TRAIN_DONE         FIELD_DEFINE(0x608C, 0, 0)
#define F_N5C32GP5X4_MCU_DEBUGF         FIELD_DEFINE(0x34F4, 31, 24)
#define F_N5C32GP5X4_DFE_SAT_EN         FIELD_DEFINE(0x403C, 31, 31)
#define F_N5C32GP5X4_TSEN_ADC_DATA      FIELD_DEFINE(0xA32C, 15, 0)
#define F_N5C32GP5X4_ESM_PATH_SEL       FIELD_DEFINE(0x6058, 16, 16)
#define F_N5C32GP5X4_ESM_DFEADAPT       FIELD_DEFINE(0x6058, 13, 10)    /* ESM_DFE_ADAPT_SPLR_EN_LANE */
#define F_N5C32GP5X4_ADAPT_EVEN_ENABLE  FIELD_DEFINE(0x6D74, 8, 8)
#define F_N5C32GP5X4_ADAPT_ODD_ENABLE   FIELD_DEFINE(0x6D74, 9, 9)
#define F_N5C32GP5X4_ESM_EN             FIELD_DEFINE(0x6058, 18, 18)
#define F_N5C32GP5X4_EOM_READY          FIELD_DEFINE(0x603C, 3, 3)
#define F_N5C32GP5X4_ESM_LPNUM          FIELD_DEFINE(0x6078, 15, 0)
#define F_N5C32GP5X4_ESM_PHASE          FIELD_DEFINE(0x6078, 26, 16)
#define F_N5C32GP5X4_ESM_VOLTAGE        FIELD_DEFINE(0x603C, 15, 8)
#define F_N5C32GP5X4_EOM_DFE_CALL       FIELD_DEFINE(0x603C, 4, 4)
#define F_N5C32GP5X4_EOM_VC_P_3932      FIELD_DEFINE(0x4148, 7, 0)      /* EOM_VLD_CNT_P_LANE[39:32] */
#define F_N5C32GP5X4_EOM_VC_P_3100      FIELD_DEFINE(0x4140, 31, 0)     /* EOM_VLD_CNT_P_LANE[31:0] */
#define F_N5C32GP5X4_EOM_VC_N_3932      FIELD_DEFINE(0x414C, 7, 0)      /* EOM_VLD_CNT_N_LANE[39:32] */
#define F_N5C32GP5X4_EOM_VC_N_3100      FIELD_DEFINE(0x4144, 31, 0)     /* EOM_VLD_CNT_N_LANE[31:0] */
#define F_N5C32GP5X4_EOM_ERR_CNT_P      FIELD_DEFINE(0x4138, 31, 0)
#define F_N5C32GP5X4_EOM_ERR_CNT_N      FIELD_DEFINE(0x413C, 31, 0)
#define F_N5C32GP5X4_PHY_ISOLATE_MODE   FIELD_DEFINE(0xA318, 23, 23)
#define F_N5C32GP5X4_FW_READY           FIELD_DEFINE(0xA428, 14, 14)
#define F_N5C32GP5X4_MCU_INIT_DONE      FIELD_DEFINE(0xA200, 7, 7)
#define F_N5C32GP5X4_RX_INIT            FIELD_DEFINE(0x5630, 21, 21)
#define F_N5C32GP5X4_RX_INIT_DONE       FIELD_DEFINE(0x3200, 19, 19)
#define F_N5C32GP5X4_PU_IVREF           FIELD_DEFINE(0xA424, 2, 2)
#define F_N5C32GP5X4_PU_IVREF_FM_REG    FIELD_DEFINE(0xA424, 1, 1)
#define F_N5C32GP5X4_ANA_PU_IVREF_FORCE FIELD_DEFINE(0xA31C, 23, 23)
#define F_N5C32GP5X4_ANA_PU_TX          FIELD_DEFINE(0x3000, 30, 30)
#define F_N5C32GP5X4_ANA_PU_TX_FORCE    FIELD_DEFINE(0x3000, 31, 31)
#define F_N5C32GP5X4_ANA_PU_RX          FIELD_DEFINE(0x3200, 30, 30)
#define F_N5C32GP5X4_ANA_PU_RX_FORCE    FIELD_DEFINE(0x3200, 31, 31)
#define F_N5C32GP5X4_PU_PLL             FIELD_DEFINE(0x5534, 6, 6)
#define F_N5C32GP5X4_TX_IDLE            FIELD_DEFINE(0x3014, 18, 18)
#define F_N5C32GP5X4_MCU_FREQ           FIELD_DEFINE(0xA420, 15, 0)
#define F_N5C32GP5X4_RX_TRAIN_ENABLE    FIELD_DEFINE(0x5630, 19, 19)
#define F_N5C32GP5X4_RX_TRAIN_COMPLETE  FIELD_DEFINE(0x5020, 4, 4)
#define F_N5C32GP5X4_RX_TRAIN_FAILED    FIELD_DEFINE(0x5020, 3, 3)
#define F_N5C32GP5X4_TX_TRAIN_ENABLE    FIELD_DEFINE(0x5630, 13, 13)
#define F_N5C32GP5X4_TX_TRAIN_COMPLETE  FIELD_DEFINE(0x5020, 6, 6)
#define F_N5C32GP5X4_TX_TRAIN_FAILED    FIELD_DEFINE(0x5020, 5, 5)
#define F_N5C32GP5X4_RX_SQ_OUT          FIELD_DEFINE(0x3270, 14, 14)    /* PIN_RX_SQ_OUT_LPF_RD_LANE */
#define F_N5C32GP5X4_TX_FIR_TAP_POL_F   FIELD_DEFINE(0x30D0, 0, 0)      /* TX_FIR_TAP_POL_FORCE_LANE */
#define F_N5C32GP5X4_TX_FIR_TAP_POL     FIELD_DEFINE(0x30D0, 6, 1)
#define F_N5C32GP5X4_TO_ANA_TX_FIR_POL  FIELD_DEFINE(0x30D8, 13, 8)     /* TO_ANA_TX_FIR_TAP_POL_LANE */
#define F_N5C32GP5X4_PU_TX              FIELD_DEFINE(0x5534, 4, 4)
#define F_N5C32GP5X4_PU_RX              FIELD_DEFINE(0x5624, 6, 6)
#define F_N5C32GP5X4_BG_RDY             FIELD_DEFINE(0xA420, 24, 24)
#define F_N5C32GP5X4_RESET_CORE_TX      FIELD_DEFINE(0x553C, 28, 28)
#define F_N5C32GP5X4_RESET_CORE_RX      FIELD_DEFINE(0x5634, 27, 27)
#define F_N5C32GP5X4_RESET_CORE_ACK_TX  FIELD_DEFINE(0x3000, 21, 21)
#define F_N5C32GP5X4_RESET_CORE_ACK_RX  FIELD_DEFINE(0x321C, 2, 2)
#define F_N5C32GP5X4_RXSPEED_DIV        FIELD_DEFINE(0x10E4, 4, 2)
#define F_N5C32GP5X4_RXSPEED_DIV_2P5_EN FIELD_DEFINE(0x11B4, 0, 0)
#define F_N5C32GP5X4_DFE_RES_F0         FIELD_DEFINE(0x106C, 5, 4)
#define F_N5C32GP5X4_F0_DC_SHIFT        FIELD_DEFINE(0x1088, 6, 5)

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif

#endif /* N5C32GP5X4 */

#endif /* defined MCESD_N5C32GP5X4_DEFS_H */