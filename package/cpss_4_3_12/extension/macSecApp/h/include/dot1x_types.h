/**

   Type definitions related to IEEE 802.1X-2010.

   File: dot1x_types.h

   @description

   @copyright
   Copyright (c) 2012 - 2021 Rambus Inc, all rights reserved.

*/

#ifndef _DOT1X_TYPES_H
#define _DOT1X_TYPES_H

#include "macsec_types.h"
#include "macsec_clib.h"

/** Key management domain (KMD). */
typedef struct {
  /** Buffer for an UTF-8 string without null termination, consisting
      of at most 253 bytes . */
  unsigned char buf[253];
  /** Length of the UTF-8 string in bytes. */
  unsigned len;
} Dot1xKmd;

/** Network identity (NID). */
typedef struct {
  /** Buffer for an UTF-8 string without null termination, consisting
      of at most 100 bytes . */
  unsigned char buf[253];
  /** Length of the UTF-8 string in bytes. */
  unsigned len;
} Dot1xNid;

/**  EAPOL multicast group choice. */
typedef enum {
  /** Bridge group address (01-80-C2-00-00-00). */
  DOT1X_EAPOL_GROUP_BRIDGE,
  /** PAE group address (01-80-C2-00-00-03). */
  DOT1X_EAPOL_GROUP_PAE,
  /** LLDP group address (01-80-C2-00-00-0E). */
  DOT1X_EAPOL_GROUP_LLDP,
} Dot1xEapolGroup;

/** Values of the `useEap' control. */
typedef enum {
  /** Never use EAP. */
  DOT1X_USE_EAP_NEVER,
  /** Use EAP concurrently with MKA with cached CAK. */
  DOT1X_USE_EAP_IMMEDIATE,
  /** Use EAP if MKA with cached CAK is not possible or fails. */
  DOT1X_USE_EAP_MKA_FAIL
} Dot1xUseEap;

/** Values of the `unauthAllowed' control. */
typedef enum {
  /** Never allow unauthenticated connectivity. */
  DOT1X_UNAUTH_ALLOWED_NEVER,
  /** Allow unauthenticated connectivity concurrently with EAP or
      MKA negotiation. */
  DOT1X_UNAUTH_ALLOWED_IMMEDIATE,
  /** Allow unauthenticated connectivity if EAP or MKA is not possible
      or fails. */
  DOT1X_UNAUTH_ALLOWED_AUTH_FAIL
} Dot1xUnauthAllowed;

/** Values of the `unsecuredAllowed' control. */
typedef enum {
  /** Never allow unsecured (authenticated but not MACsec protected)
      connectivity. */
  DOT1X_UNSECURED_ALLOWED_NEVER,
  /** Allow unsecured connectivity concurrently with MKA
      negotiation. */
  DOT1X_UNSECURED_ALLOWED_IMMEDIATE,
  /** Allow unsecured connectivity if MKA negotiation is not possible
      or fails. */
  DOT1X_UNSECURED_ALLOWED_MKA_FAIL,
  /** Allow unsecured connectivity only if directed by the MKA key
      server. */
  DOT1X_UNSECURED_ALLOWED_MKA_SERVER
} Dot1xUnsecuredAllowed;

/** Values of the `accessStatus' variable with integer encodings for
    PDUs. */
typedef enum {
  /** No access other than to authentication services. */
  DOT1X_ACCESS_STATUS_NO_ACCESS = 0,
  /** Access severely limited, possibly to remedial services. */
  DOT1X_ACCESS_STATUS_REMEDIAL_ACCESS = 1,
  /** Controlled port operational but some restrictions have been
      applied. */
  DOT1X_ACCESS_STATUS_RESTRICTED_ACCESS = 2,
  /** Access provided as expected for successful authentication. */
  DOT1X_ACCESS_STATUS_EXPECTED_ACCESS = 3
} Dot1xAccessStatus;

/** Values of the `unauthenticatedAccess' control with integer
    encodings for PDUs. */
typedef enum {
  /** No access other than to authentication services. */
  DOT1X_UNAUTHENTICATED_ACCESS_NO_ACCESS = 0,
  /** Limited access can be provided after authentication failure. */
  DOT1X_UNAUTHENTICATED_ACCESS_FALLBACK_ACCESS = 1,
  /** Immediate limited access is available without authentication. */
  DOT1X_UNAUTHENTICATED_ACCESS_LIMITED_ACCESS = 2,
  /** Immediate access is available without authentication. */
  DOT1X_UNAUTHENTICATED_ACCESS_OPEN_ACCESS = 3
} Dot1xUnauthenticatedAccess;

/** Type of the `accessCapabilities' variable. */
typedef struct {
  /** EAP. */
  unsigned eap : 1;
  /** EAP + MKA. */
  unsigned eap_mka : 1;
  /** EAP + MKA + MACsec. */
  unsigned eap_mka_macsec : 1;
  /** MKA. */
  unsigned mka : 1;
  /** MKA + MACsec. */
  unsigned mka_macsec : 1;
  /** Higher Layer (WebWauth). */
  unsigned higher_layer : 1;
  /** Higher Layer Fallback (WebWauth). */
  unsigned higher_layer_fallback : 1;
  /** Vendor specific authentication mechanisms. */
  unsigned vendor_specific : 1;
} Dot1xAccessCapabilities;

/** Access information associated with a network. */
typedef struct {
  /** Access Status (802.1X section 10.1). */
  Dot1xAccessStatus access_status;
  /* Unauthenticated Access (802.1X section 10.1). */
  Dot1xUnauthenticatedAccess unauthenticated_access;
  /* Access Capabilities (802.1X section 10.1). */
  Dot1xAccessCapabilities access_capabilities;
  /* Access Requested (802.1X section 10.1). */
  unsigned access_requested : 1;
  /* Virtual Port Access (802.1X section 10.1). */
  unsigned virtual_port_access : 1;
  /* Group Access (802.1X section 10.1). */
  unsigned group_access : 1;
} Dot1xAccessInformation;

/** Network control information. */
typedef struct {
  /** The `useEap' control (802.1X section 12.5). */
  Dot1xUseEap use_eap;
  /** The `unauthAllowed' control (802.1X section 12.5). */
  Dot1xUnauthAllowed unauth_allowed;
  /** The `unsecuredAllowed' control (802.1X section 12.5). */
  Dot1xUnsecuredAllowed unsecured_allowed;
  /** Key management domain for transmitted announcements. */
  Dot1xKmd kmd;
} Dot1xNetworkCtrl;

/** Network status information. */
typedef struct {
  /** Nonzero if the access_information member is valid. */
  unsigned access_information_valid : 1;
  /** Nonzero if the cipher_suites member is valid. */
  unsigned cipher_suites_valid : 1;
  /** Nonzero if the kmd member is valid. */
  unsigned kmd_valid : 1;
  /** Access information. */
  Dot1xAccessInformation access_information;
  /* Supported Ciphersuites. */
  MacsecCipherCapability cipher_suites[MACSEC_CIPHER_SUITE_COUNT];
  /** Key management domain from received announcements. */
  Dot1xKmd kmd;
} Dot1xNetworkStat;

/** CAK status information. */
typedef struct {
  /** Key management domain. */
  Dot1xKmd kmd;
  /** Remaining lifetime in seconds. */
  unsigned lifetime;
  /** Nonzero if the CAK is enabled. */
  unsigned enabled : 1;
  /** Nonzero if this is the CAK of the principal MKA actor. */
  unsigned principal : 1;
  /** Live peer list. */
  MacsecSci live_peer_tab[32];
  unsigned live_peer_num;
  /** Number of potential peers. */
  unsigned potential_peer_count;
  /** Pre-shared CAK activated state */
  unsigned activated;
} Dot1xCakStat;

/** Real or virtual port status information. */
typedef struct {
  /** Nonzero if at least MKA actor is transmitting MKPDUs. */
  unsigned mka_active : 1;
  /** Nonzero if MKA has failed. */
  unsigned mka_failed : 1;
  /** Nonzero if MKA is operational without MACsec protection. */
  unsigned mka_authenticated : 1;
  /** Nonzero if MKA is operational with MACsec protection. */
  unsigned mka_secured : 1;
  /** SCI of the current MKA key server, or an all-zero SCI of there
      is no key server. */
  MacsecSci key_server_sci;
  /** Priority of the current MKA key server, if any. */
  unsigned key_server_priority;
  /** Key number (KN) of the currently distributed SAK. */
  uint32_t kn;
  /** Association number (AN) of the current SAs. */
  unsigned an;
} Dot1xPortStat;

/** PAE control information. */
typedef struct {
  /** Multicast group for EAPOL frames. */
  Dot1xEapolGroup eapol_group;
  /** Enable creation of virtual ports. */
  unsigned virtual_ports_enable : 1;
  /** Enable transmission of network announcements. */
  unsigned announcer_enable : 1;
  /** Enable reception of network announcements. */
  unsigned listener_enable : 1;
  /** Enable MKA. */
  unsigned mka_enable : 1;
  /** Enable 0.5 second periodic MKA PDU transmission interval and
      delay protection. If not set then periodic MKA PDUs are
      transmitted every 2 seconds and delay protection is not used. */
  unsigned bounded_delay : 1;
  /** The MKA key server priority of the local station. */
  unsigned actor_priority;
  /** The cipher suite to be selected by this station if it is the MKA
      key server. */
  MacsecCipherSuite cipher_suite;
  /** The confidentiality offset to be selected by this station if it
      is the MKA key server. */
  MacsecConfidentialityOffset confidentiality_offset;
  /** The logon control. */
  unsigned logon : 1;
  /*** The selectedNID control (802.1X 12.5). */
  Dot1xNid selected_nid;
  /** Enable caching of CAKs. */
  unsigned cak_caching_enable : 1;
  /** Enable group CA formation. */
  unsigned group_ca_enable : 1;
  /** Enable derivation of 256-bit CAKs from EAP, instead of 128-bit. */
  unsigned long_eap_caks : 1;
  /** Enable EAP supplicant. */
  unsigned supplicant_enable : 1;
  /** Enable EAP authenticator. */
  unsigned authenticator_enable : 1;
  /** The heldPeriod control. */
  unsigned held_period;
  /** The quietPeriod control. */
  unsigned quiet_period;
  /** The reAuthEnabled control. */
  unsigned reauth_enabled : 1;
  /** The reAuthPeriod control. */
  unsigned reauth_period;
  /** The retryMax control. */
  unsigned retry_max;
  /** Enable sending or receiving of EAPOL-Logoff. */
  unsigned logoff_enable;
  /** The macsecProtect control. */
  unsigned macsec_protect : 1;
  /** The macsecValidate control. */
  MacsecValidateFrames macsec_validate;
  /** The macsecReplayProtect control. */
  unsigned macsec_replay_protect : 1;
  /** The transmitDelay control. */
  unsigned transmit_delay;
  /** The retireDelay control. */
  unsigned retire_delay;
  /** The suspendFor  control. */
  unsigned suspend_for;
  /** The suspendOnRequest control. */
  unsigned suspend_on_request : 1;
  /** The suspendedWhile control. */
  unsigned suspended_while;
} Dot1xPaeCtrl;

/** PAE status information. */
typedef struct {
  /*** The connectedNID variable (802.1X 12.5). */
  Dot1xNid connected_nid;
  /*** The requestedNID variable (802.1X 12.5). */
  Dot1xNid requested_nid;
  /** suppEntersAuthenticating (802.1X). */
  uint32_t supp_enters_authenticating;
  /** suppAuthTimeoutsWhileAuthenticating (802.1X). */
  uint32_t supp_auth_timeouts_while_authenticating;
  /** suppEapLogoffWhileAuthenticating (802.1X). */
  uint32_t supp_eap_logoff_while_authenticating;
  /** suppAuthFailWhileAuthenticating (802.1X). */
  uint32_t supp_auth_fail_while_authenticating;
  /** suppAuthSuccessesWhileAuthenticating (802.1X). */
  uint32_t supp_auth_successes_while_authenticating;
  /** suppAuthFailWhileAuthenticated (802.1X). */
  uint32_t supp_auth_fail_while_authenticated;
  /** suppAuthEapLogoffWhileAuthenticated (802.1X). */
  uint32_t supp_auth_eap_logoff_while_authenticated;
  /** authEntersAuthenticating (802.1X). */
  uint32_t auth_enters_authenticating;
  /** authAuthTimeoutsWhileAuthenticating (802.1X). */
  uint32_t auth_auth_timeouts_while_authenticating;
  /** authAuthEapStartsWhileAuthenticating (802.1X). */
  uint32_t auth_auth_eap_starts_while_authenticating;
  /** authAuthEapLogoffWhileAuthenticating (802.1X). */
  uint32_t auth_auth_eap_logoff_while_authenticating;
  /** authAuthSuccessesWhileAuthenticating (802.1X). */
  uint32_t auth_auth_successes_while_authenticating;
  /** authAuthFailWhileAuthenticating (802.1X). */
  uint32_t auth_auth_fail_while_authenticating;
  /** authAuthReauthsWhileAuthenticating (802.1X). */
  uint32_t auth_auth_reauths_while_authenticated;
  /** authAuthEapStartsWhileAuthenticated (802.1X). */
  uint32_t auth_auth_eap_starts_while_authenticated;
  /** authAuthEapLogoffWhileAuthenticated (802.1X). */
  uint32_t auth_auth_eap_logoff_while_authenticated;
  /** eapolStartFramesRx (802.1X). */
  uint32_t eapol_start_frames_rx;
  /** eapolStartFramesTx (802.1X). */
  uint32_t eapol_start_frames_tx;
  /** eapolLogoffFramesRx (802.1X). */
  uint32_t eapol_logoff_frames_rx;
  /** eapolLogoffFramesTx (802.1X). */
  uint32_t eapol_logoff_frames_tx;
  /** eapolSuppEapFramesRx (802.1X). */
  uint32_t eapol_supp_eap_frames_rx;
  /** eapolAuthEapFramesRx (802.1X). */
  uint32_t eapol_auth_eap_frames_rx;
  /** eapolSuppEapFramesTx (802.1X). */
  uint32_t eapol_supp_eap_frames_tx;
  /** eapolAuthEapFramesTx (802.1X). */
  uint32_t eapol_auth_eap_frames_tx;
  /** eapolMKNoCKN (802.1X). */
  uint32_t eapol_mk_no_ckn_rx;
  /** eapolMKInvalidRx (802.1X). */
  uint32_t eapol_mk_invalid_rx;
  /** eapolMKAFramesTx (802.1X). */
  uint32_t eapol_mka_frames_tx;
  /** eapolAnnouncementsRx (802.1X). */
  uint32_t eapol_announcements_rx;
  /** eapolAnnouncementsTx (802.1X). */
  uint32_t eapol_announcements_tx;
  /** eapolAnnoucementReqsRx (802.1X). */
  uint32_t eapol_announcement_reqs_rx;
  /** eapolAnnouncementReqsTx (802.1X). */
  uint32_t eapol_announcement_reqs_tx;
  /** invalidEapolFrames (802.1X). */
  uint32_t invalid_eapol_frames;
  /** eapLengthErrorFrames (802.1X). */
  uint32_t eap_length_error_frames;
  /** eapolPortUnavailable (802.1X). */
  uint32_t eapol_port_unavailable;
  /** lastEapolFrameVersion (802.1X). */
  uint32_t last_eapol_frame_version;
  /** lastEapolFrameSource (802.1X). */
  MacsecAddress last_eapol_frame_source;
  /** suspendedWhile (802.1X). */
  unsigned suspended_while;
} Dot1xPaeStat;

#endif /* _DOT1X_TYPES_H */
