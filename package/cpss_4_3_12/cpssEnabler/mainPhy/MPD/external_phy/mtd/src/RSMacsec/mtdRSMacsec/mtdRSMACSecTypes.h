/*******************************************************************************
Copyright (C) 2014 - 2022, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions global register definitions for the
registers in the Marvell CUX3610/CUE2610 MACSec enable Ethernet PHYs.
********************************************************************/
#ifndef RSMACSEC_H
#define RSMACSEC_H
#if C_LINKAGE
#if defined __cplusplus 
    extern "C" { 
#endif 
#endif

/* Device Number Definitions */
#define MTD_MACSEC_UNIT      31
#define MTD_MACSEC_DEV_3     3

#define MTD_MACSEC_READ_LOW  0x97FE
#define MTD_MACSEC_READ_HIGH 0x97FF

/* Common RS_MACSec registers */
#define MTD_MPM_CONFIG_REG         0xD830
#define MTD_MPM_SOFT_RESET_REG     0xD831
#define MTD_MPM_MISC_REG           0xD832
#define MTD_MMAC_CONTROL_REGISTER  0xD833
#define MTD_PTP_CONTROL0_REGISTER  0xD820
#define MTD_PTP_CONTROL1_REGISTER  0xD821
#define MTD_BLOCK_RESET_REGISTER   0xD801

/* Common MMAC device 0x1F registers */
#define MTD_SLC_CFG_GEN          0x8000
#define MTD_PORT_MAC_SA0         0x8008
#define MTD_PORT_MAC_SA1         0x800A
#define MTD_PAUSE_CTL            0x800E
#define MTD_EDB_THRESH           0x8010
#define MTD_IDB_THRESH           0x8012
#define MTD_CFG_VLAN_ET          0x8048
#define MTD_PTP_CFG1             0x804A
#define MTD_PTP_CFG2             0x804C
#define MTD_PTP_CFG3             0x804E
#define MTD_QOS_FC               0x80A4
#define MTD_QOS_ETHERTYPE        0x80A6
#define MTD_MAC_LINK_RESET_CFG   0x80B2
#define MTD_SMC_MAC_PREEMPT_CTRL 0x80DE
#define MTD_SMC_MAC_PREEMPT_STAT 0x80E0
#define MTD_WMC_MAC_PREEMPT_CTRL 0x80EA

#define MTD_MCS_RX_DELAY_CFG_0   0x00EE
#define MTD_MCS_TX_DELAY_CFG_0   0x010A

#define MTD_SMAC_RX_STATS_REG 0x9000
#define MTD_SMAC_TX_STATS_REG 0x9080
#define MTD_WMAC_RX_STATS_REG 0x9100
#define MTD_WMAC_TX_STATS_REG 0x9300


/* structure of SMC RX stats */ 
typedef struct _MTD_RX_SMC_STATS {
    MTD_U64 goodOctets;        /* Good frame octets                  */
    MTD_U64 errorOctets;       /* Bad frame octets                   */
    MTD_U64 jabber;            /* Frame of size > MTU and bad CRC    */
    MTD_U64 fragment;          /* Frame < 48B with bad CRC           */
    MTD_U64 undersize;         /* Frame < 48B with good CRC          */
    MTD_U64 oversize;          /* Frame of size > MTU and good CRC   */
    MTD_U64 rxError;           /* sequence, code, symbol errors      */
    MTD_U64 crcErrTxUnderrun;  /* CRC error                          */
    MTD_U64 rxOverrunBadFC;    /* Overrun or bad flow control packet */
    MTD_U64 goodPkts;          /* number of good packets             */
    MTD_U64 fcPkts;            /* flow control packet                */
    MTD_U64 broadcast;         /* number of broadcast packets        */
    MTD_U64 multicast;         /* number of multicast packets        */
    MTD_U64 inPAssemblyErr;
    MTD_U64 inPFrames;
    MTD_U64 inPFrags;
    MTD_U64 inPBadFrags;
}MTD_RX_SMC_STATS;

/* structure of SMC TX stats */ 
typedef struct _MTD_SMC_TX_STATS {
    MTD_U64 goodOctets;        /* Good frame octets                  */
    MTD_U64 errorOctets;       /* Bad frame octets                   */
    MTD_U64 jabber;            /* Frame of size > MTU and bad CRC    */
    MTD_U64 fragment;          /* Frame < 48B with bad CRC           */
    MTD_U64 undersize;         /* Frame < 48B with good CRC          */
    MTD_U64 oversize;          /* Frame of size > MTU and good CRC   */
    MTD_U64 rxError;           /* sequence, code, symbol errors      */
    MTD_U64 crcErrTxUnderrun;  /* CRC error                          */
    MTD_U64 rxOverrunBadFC;    /* Overrun or bad flow control packet */
    MTD_U64 goodPkts;          /* number of good packets             */
    MTD_U64 fcPkts;            /* flow control packet                */
    MTD_U64 broadcast;         /* number of broadcast packets        */
    MTD_U64 multicast;         /* number of multicast packets        */
    MTD_U64 outPFrags;
    MTD_U64 outPFrames;
}MTD_TX_SMC_STATS;

/* structure of WMC RX stats */ 
typedef struct _MTD_RX_WMC_STATS {
    MTD_U64 pkts;               /* all packets                   */ 
    MTD_U64 droppedPkts;        /* dropped packets               */
    MTD_U64 octets;             /* all packets octets            */
    MTD_U64 jabber;             /* packet > MTU and bad CRC      */
    MTD_U64 fragment;           /* packet < 64B and bad CRC      */
    MTD_U64 undersize;          /* packet < 64B and good CRC     */
    MTD_U64 rxError;            /* sequence, code, symbol errors */
    MTD_U64 crcError;           /* CRC error packet              */
    MTD_U64 fcPkts;             /* flow control packet           */
    MTD_U64 broadcast;          /* number of broadcast packets   */
    MTD_U64 multicast;          /* number of multicast packets   */
    MTD_U64 inPAssemblyErr;
    MTD_U64 inPFrames;
    MTD_U64 inPFrags;
    MTD_U64 inPBadFrags;
}MTD_RX_WMC_STATS;

/* structure of WMC TX stats */ 
typedef struct _MTD_TX_WMC_STATS {
    MTD_U64 pkts;                /* all packets                   */
    MTD_U64 droppedPkts;         /* dropped packets               */
    MTD_U64 octets;              /* all packets octets            */
    MTD_U64 jabber;              /* packet > MTU and bad CRC      */
    MTD_U64 fragment;            /* packet < 64B and bad CRC      */
    MTD_U64 undersize;           /* packet < 64B and good CRC     */
    MTD_U64 rxError;             /* sequence, code, symbol errors */
    MTD_U64 crcError;            /* CRC error packet              */
    MTD_U64 fcPkts;              /* flow control packet           */
    MTD_U64 broadcast;           /* number of broadcast packets   */
    MTD_U64 multicast;           /* number of multicast packets   */
    MTD_U64 outPFrags;
    MTD_U64 outPFrames;
}MTD_TX_WMC_STATS;

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif


#endif

