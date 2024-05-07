/**

   Type definitions related to IEEE 802.1AE-2006 (MACsec) and IEEE
   802.1X-2010.

   File: macsec_types.h

   @description

   @copyright
   Copyright (c) 2012 - 2020 Rambus Inc. All rights reserved.

*/


#ifndef _MACSEC_TYPES_H
#define _MACSEC_TYPES_H

#include "macsec_clib.h"

/** MAC address. */
typedef struct {
  /** Buffer containing the 6-byte address. */
  unsigned char buf[6];
} MacsecAddress;

/** Secure channel identifier (SCI). */
typedef struct {
  /** MAC address of transmitting station. */
  MacsecAddress address;
  /** Port identifier in transmitting station. */
  unsigned portid;
} MacsecSci;

/** An 128-bit or 256-bit AES key. */
typedef struct {
  /** Buffer containing the key. */
  unsigned char buf[32];
  /** Length of key in bytes (16 or 32). */
  unsigned len;
} MacsecKey;

/** Key identifier (KI). */
typedef struct {
  /** Buffer containing the 16-byte key identifier. */
  unsigned char buf[16];
} MacsecKi;

static const MacsecKi MACSEC_KI_INVALID = { .buf = {0,} };

/** Salt for cryptographic operations. */
typedef struct {
  /** Buffer containing the 12-byte salt. */
  unsigned char buf[12];
} MacsecSalt;

/** Short SCI (SSCI). */
typedef struct {
  /** Buffer containing the 4-byte SSCI. */
  unsigned char buf[4];
} MacsecSsci;

/** Opaque authorization data. */
typedef struct {
  /** Buffer containing the data. */
  unsigned char buf[256];
  /** Length of data in bytes. */
  unsigned len;
} MacsecAuthData;

/** EAP session id. */
typedef struct {
  /** Buffer containing the 65-byte session id. */
  unsigned char buf[65];
  /** Length of session id in bytes (65) or zero if there is no
      session id */
  unsigned len;
} MacsecSessionId;

/** The first 64 bytes of an EAP master session key. */
typedef struct {
  /** Buffer containing 64 bytes of the MSK. */
  unsigned char buf[64];
  /** Length of partial MSK in bytes (64) or zero if there is no
      session id */
  unsigned len;
} MacsecMsk;

/** Values of the validateFrames control and other similar
    controls. */
typedef enum {
  /** Do not perform integrity check. */
  MACSEC_VALIDATE_FRAMES_DISABLED,
  /** Perform integrity check do not drop failed frames. */
  MACSEC_VALIDATE_FRAMES_CHECK,
  /** Perform integrity check and drop failed frames. */
  MACSEC_VALIDATE_FRAMES_STRICT
} MacsecValidateFrames;

/** Values of the currentCipherSuite control and other similar
    controls. */
typedef enum {
  /** GCM-AES-128 cipher suite. */
  MACSEC_CIPHER_SUITE_GCM_AES_128,
  /** GCM-AES-256 cipher suite. */
  MACSEC_CIPHER_SUITE_GCM_AES_256,
  /** GCM-AES-XPN-128 cipher suite. */
  MACSEC_CIPHER_SUITE_GCM_AES_XPN_128,
  /** GCM-AES-XPN-256 cipher suite. */
  MACSEC_CIPHER_SUITE_GCM_AES_XPN_256,
  /** Number of cipher suite values. */
  MACSEC_CIPHER_SUITE_COUNT
} MacsecCipherSuite;

/** Values of the confidentialityOffset control and other similar
    controls. */
typedef enum {
  /** Do not encrypt user data following SecTAG. */
  MACSEC_CONFIDENTIALITY_NOT_USED,
  /** Encrypt all user data following SecTAG. */
  MACSEC_CONFIDENTIALITY_OFFSET_0,
  /** Leave 30 bytes of data following SecTAG unencrypted but
      authenticated. */
  MACSEC_CONFIDENTIALITY_OFFSET_30,
  /** Leave 50 bytes of data following SecTAG unencrypted but
      authenticated. */
  MACSEC_CONFIDENTIALITY_OFFSET_50
} MacsecConfidentialityOffset;

/** Values of the adminPointToPointMAC control. */
typedef enum {
  MACSEC_ADMIN_POINT_TO_POINT_MAC_FORCE_TRUE,
  MACSEC_ADMIN_POINT_TO_POINT_MAC_FORCE_FALSE,
  MACSEC_ADMIN_POINT_TO_POINT_MAC_AUTO
} MacsecAdminPointToPointMac;

/** Values of a two-bit cipher capability field with integer encoding
    for PDUs. */
typedef enum {
  /** MACsec is not implemented. */
  MACSEC_CAPABILITY_NOT_IMPLEMENTED = 0,
  /** Integrity without confidentiality. */
  MACSEC_CAPABILITY_INTEGRITY = 1,
  /** Integrity and confidentiality. */
  MACSEC_CAPABILITY_CONFIDENTIALITY = 2,
  /** Integrity and confidentiality with confidentiality offset of 0,
      30 or 50 bytes. */
  MACSEC_CAPABILITY_CONFIDENTIALITY_WITH_OFFSET = 3
} MacsecCapability;

/** Capabilities associated with a cipher suite. */
typedef struct {
  /** Cipher suite implemented. */
  unsigned implemented : 1;
  /** Cipher capabilities. */
  MacsecCapability capability;
} MacsecCipherCapability;

/** General MACsec capabilities. */
typedef struct {
  /** Maximum number of peer SCs. */
  unsigned max_peer_scs;
  /** Maximum number of keys in simultaneous use for reception. */
  unsigned max_receive_keys;
  /** Maximum number of keys in simultaneous use for transmission. */
  unsigned max_transmit_keys;
  /** Capabilities of each cipher suite. */
  MacsecCipherCapability cipher_suites[MACSEC_CIPHER_SUITE_COUNT];
} MacsecCapabilities;

/**  SecY control information (802.1AE section 10.7). */
typedef struct {
  /** The validateFrames control (802.1AE section 10.7.8). */
  MacsecValidateFrames validate_frames;
  /** The replayProtect control (802.1AE section 10.7.8). */
  unsigned replay_protect : 1;
  /** The replayWindow control (802.1AE section 10.7.8). */
  uint32_t replay_window;
  /** The protectFrames control (802.1AE section 10.7.17). */
  unsigned protect_frames : 1;
  /** The alwaysIncludeSCI control (802.1AE section 10.7.17). */
  unsigned always_include_sci : 1;
  /** The useES control (802.1AE section 10.7.17). */
  unsigned use_es : 1;
  /** The useSCB control (802.1AE section 10.7.17). */
  unsigned use_scb : 1;
  /** The currentCipherSuite control (802.1AE section 10.7.25). */
  MacsecCipherSuite current_cipher_suite;
  /** The confidentialityOffset control (802.1AE section 10.7.25). */
  MacsecConfidentialityOffset confidentiality_offset;
  /** The adminPointToPointMAC (802.1AE section 6.5). */
  MacsecAdminPointToPointMac admin_point_to_point_mac;
} MacsecSecyCtrl;

/** SecY status information (802.1AE section 10.7). */
typedef struct {
  /** inPktsUntagged (802.1AE). */
  uint64_t in_pkts_untagged;
  /** inPktsNoTag (802.1AE). */
  uint64_t in_pkts_no_tag;
  /** inPktsBadTag (802.1AE). */
  uint64_t in_pkts_bad_tag;
  /** inPktsUnknownSCI (802.1AE). */
  uint64_t in_pkts_unknown_sci;
  /** inPktsNoSCI (802.1AE). */
  uint64_t in_pkts_no_sci;
  /** inPktsOverrun (802.1AE). */
  uint64_t in_pkts_overrun;
  /** inOctetsValidated (802.1AE). */
  uint64_t in_octets_validated;
  /** inOctetsDecrypted (802.1AE). */
  uint64_t in_octets_decrypted;
  /** outPktsUntagged (802.1AE). */
  uint64_t out_pkts_untagged;
  /** outPktsTooLong (802.1AE). */
  uint64_t out_pkts_too_long;
  /** outOctetsProtected (802.1AE). */
  uint64_t out_octets_protected;
  /** outOctetsEncrypted (802.1AE). */
  uint64_t out_octets_encrypted;
} MacsecSecyStat;

/** Controlled port, uncontrolled port or common port status
    information (802.1AE section 10.7). */
typedef struct {
  /** ifInOctets. */
  uint64_t if_in_octets;
  /** ifInUcastPkts. */
  uint64_t if_in_ucast_pkts;
  /** ifInMulticastPkts. */
  uint64_t if_in_multicast_pkts;
  /** ifInBroadcastPkts. */
  uint64_t if_in_broadcast_pkts;
  /** ifInDiscards. */
  uint64_t if_in_discards;
  /** ifInErrors. */
  uint64_t if_in_errors;
  /** ifOutOctets. */
  uint64_t if_out_octets;
  /** ifOutUcastPkts. */
  uint64_t if_out_ucast_pkts;
  /** ifOutMulticastPkts. */
  uint64_t if_out_multicast_pkts;
  /** ifOutBroadcastPkts. */
  uint64_t if_out_broadcast_pkts;
  /** ifOutErrors. */
  uint64_t if_out_errors;
  /** MAC_Enabled (802.1AE). */
  unsigned mac_enabled : 1;
  /** MAC_Operational (802.1AE). */
  unsigned mac_operational : 1;
  /** operPointToPointMAC (802.1AE). */
  unsigned oper_point_to_point_mac : 1;
} MacsecPortStat;

/** Receive SC status information (802.1AE section 10.7). */
typedef struct {
  /** SCI (802.1AE). */
  MacsecSci sci;
  /** receiving (802.1AE). */
  unsigned receiving : 1;
  /** createdTime (802.1AE). */
  uint32_t created_time;
  /** startedTime (802.1AE). */
  uint32_t started_time;
  /** stoppedTime (802.1AE). */
  uint32_t stopped_time;
  /** inPktsUnchecked (802.1AE). */
  uint64_t in_pkts_unchecked;
  /** inPktsDelayed (802.1AE). */
  uint64_t in_pkts_delayed;
  /** inPktsLate (802.1AE). */
  uint64_t in_pkts_late;
  /** inPktsOK (802.1AE). */
  uint64_t in_pkts_ok;
  /** inPktsInvalid (802.1AE). */
  uint64_t in_pkts_invalid;
  /** inPktsNotValid (802.1AE). */
  uint64_t in_pkts_not_valid;
  /** inPktsNotUsingSA (802.1AE). */
  uint64_t in_pkts_not_using_sa;
  /** inPktsUnusedSA (802.1AE). */
  uint64_t in_pkts_unused_sa;
} MacsecReceiveScStat;

/** Receive SA status information (802.1AE section 10.7). */
typedef struct {
  /** inUse (802.1AE). */
  unsigned in_use : 1;
  /** nextPN (802.1AE). */
  uint64_t next_pn;
  /** lowestPN (802.1AE). */
  uint64_t lowest_pn;
  /** createdTime (802.1AE). */
  uint32_t created_time;
  /** startedTime (802.1AE). */
  uint32_t started_time;
  /** stoppedTime (802.1AE). */
  uint32_t stopped_time;
  /** inPktsOK (802.1AE). */
  uint64_t in_pkts_ok;
  /** inPktsInvalid (802.1AE). */
  uint64_t in_pkts_invalid;
  /** inPktsNotValid (802.1AE). */
  uint64_t in_pkts_not_valid;
  /** inPktsNotUsingSA (802.1AE). */
  uint64_t in_pkts_not_using_sa;
  /** inPktsUnusedSA (802.1AE). */
  uint64_t in_pkts_unused_sa;
} MacsecReceiveSaStat;

/** Transmit SC status information (802.1AE section 10.7). */
typedef struct {
  /** SCI (802.1AE). */
  MacsecSci sci;
  /** transmitting (802.1AE). */
  unsigned transmitting : 1;
  /** encodingSA (802.1AE). */
  unsigned encoding_sa;
  /** encipheringSA (802.1AE). */
  unsigned enciphering_sa;
  /** createdTime (802.1AE). */
  uint32_t created_time;
  /** startedTime (802.1AE). */
  uint32_t started_time;
  /** stoppedTime (802.1AE). */
  uint32_t stopped_time;
  /** outPktsProtected (802.1AE). */
  uint64_t out_pkts_protected;
  /** outPktsEncrypted (802.1AE). */
  uint64_t out_pkts_encrypted;
} MacsecTransmitScStat;

/** Transmit SA status information (802.1AE section 10.7). */
typedef struct {
  /** inUse (802.1AE). */
  unsigned in_use : 1;
  /** nextPN (802.1AE). */
  uint64_t next_pn;
  /** createdTime (802.1AE). */
  uint32_t created_time;
  /** startedTime (802.1AE). */
  uint32_t started_time;
  /** stoppedTime (802.1AE). */
  uint32_t stopped_time;
  /** outPktsProtected (802.1AE). */
  uint64_t out_pkts_protected;
  /** outPktsEncrypted (802.1AE). */
  uint64_t out_pkts_encrypted;
} MacsecTransmitSaStat;

#endif /* _MACSEC_TYPES_H */
