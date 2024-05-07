/**

   EAP functions used by the 802.1X protocol and associated
   configuration functions.

   File: eapauth.h

   @description

   @copyright
   Copyright (c) 2012 - 2013 Rambus Inc, all rights reserved.

*/

#ifndef _EAPAUTH_H
#define _EAPAUTH_H

/*
 * Get implentation-dependent definitions of EapConfig, EapContext,
 * EapConversation.
 */
#include "eapauth_types.h"

#include "macsec_types.h"
#include "dot1x_types.h"

/** The state of an EAP conversation. */
typedef enum {
  /** EAP conversation has not yet terminated. */
  EAP_STATE_INCOMPLETE,
  /** EAP conversation has succeeded. Authentication results can be
      retrieved using eap_get_results(). */
  EAP_STATE_SUCCEEDED,
  /** EAP conversation has terminated due to authentication
      failure. */
  EAP_STATE_FAILED,
  /** EAP conversation has terminated due to timeout. */
  EAP_STATE_TIMEDOUT
} EapState;

/** EAP partner information. An EAP partner is a remote station with
    which an EAP conversation is conducted. When creating an EAP
    conversation this information can be used to select an appropriate
    authentication method or credentials.
*/
typedef struct {
  /** MAC address of the remote station, or a zero address if the
      remote station is not known. */
  MacsecAddress mac_address;
  /** Network identity in UTF-8 format, describing the network where
      an EAP conversation takes place, obtained e.g. from network
      announcements. An empty string if network identity is not
      known. */
  Dot1xNid nid;
} EapPartner;

/** Create an EAP conversation object and start a conversation where
    the local station is the supplicant. The conversation must be
    destroyed using eap_destroy() when no longer needed.

    @param context
    Pointer to an EAP context.

    @param partner
    Pointer to information about the remote station with which the
    conversation is being started.

    @return
    Pointer to an EAP conversation object.
*/
EapConversation *
eap_create_supplicant(
  EapContext *context,
  const EapPartner *partner);

/** Create an EAP conversation object and start a conversation where
    the local station is the authenticator. The conversation must be
    destroyed using eap_destroy() when no longer needed.

    @param context
    Pointer to an EAP context.

    @param partner
    Pointer to information about the remote station with which the
    conversation is being started.

    @return
    Pointer to an EAP conversation object.
*/
EapConversation *
eap_create_authenticator(
  EapContext *context,
  const EapPartner *partner);

/** Destroy an EAP conversation object previously created using
    eap_create_supplicant() or eap_create_authenticator(). A
    conversation can be destroyed at any time, regardless of its
    state.

    @param conversation
    Pointer to an EAP conversation returned by a previous call to
    eap_create_supplicant() or eap_create_authenticator().
*/
void
eap_destroy(
  EapConversation *conversation);

/** Return the current state of an EAP conversation.

    @param conversation
    Pointer to an EAP conversation returned by a previous call to
    eap_create_supplicant() or eap_create_authenticator().

    @return
    The current state of the conversation.
*/
EapState
eap_state(
  const EapConversation *c);

/** Get the authentication results of an EAP conversation. Valid
    results are returned only if the state of the conversation is
    EAP_STATE_SUCCEEDED.

    @param conversation
    Pointer to an EAP conversation returned by a previous call to
    eap_create_supplicant() or eap_create_authenticator().

    @param session_id
    Pointer to a session id object into which the session id, if any,
    produced by the conversation is copied. If there is no space for
    the entire session id then as many bytes as possible are copied.

    @param msk
    Pointer to an MSK object into which the master session key, if
    any, produced by the conversation is copied. If there is no space
    for the entire key then as many bytes as possible are copied.

    @param auth_data
    Pointer to an authorization data object into which authorization
    data, if any, procudced by the conversation is copied. If there is
    no space for all data then as many bytes as possible are copied.

    @description
    This function returns the results of a succeeded EAP
    conversation. Depending on the EAP method, some or all of the
    results may not have been generated which is indicated by setting
    the length field of a result object to zero. If the EAP
    conversation is not in the succeeded state then all result objects
    are set to zero length.
*/
void
eap_get_results(
  EapConversation *c,
  MacsecSessionId *session_id,
  MacsecMsk *msk,
  MacsecAuthData *auth_data);

/** Try retrieving an EAP packet to be transmitted to the network from
    an EAP conversation.

    @param conversation
    Pointer to an EAP conversation returned by a previous call to
    eap_create_supplicant() or eap_create_authenticator().

    @param buf
    Pointer to a buffer into which the possible packet is copied.

    @param len
    The maximum size of the packet in bytes.

    @return
    The number of bytes copied into the buffer, or zero if the
    conversation currently has no packets to send.
*/
unsigned
eap_produce_packet(
  EapConversation *c,
  unsigned char *buf,
  unsigned len);

/** Submit an EAP packet received from the network to an EAP
    conversation.

    @param conversation
    Pointer to an EAP conversation returned by a previous call to
    eap_create_supplicant() or eap_create_authenticator().

    @param buf
    Pointer to a buffer containing the packet. The contents of the
    buffer are copied.

    @param len
    The size of the packet in bytes.
*/
void
eap_consume_packet(
  EapConversation *c,
  const unsigned char *buf,
  unsigned len);

/** Implementation-dependent initialization of an EAP context.

    @param context
    Pointer to an EAP context object to be initialized.

    @param config
    Pointer to implementation-dependent EAP configuration data.

    @return
    1 if successful, 0 otherwise.

    @description
    This function initializes an EAP context using
    implementation-dependent configuration data. It is intended to be
    called by application code and is not used by the generic 802.1X
    protocol.
*/
int
eap_init_context(
  EapContext *c,
  const EapConfig *config);

/** Implementation-dependent uninitialization of an EAP context.

    @param context
    Pointer to an EAP context previously returned by
    eap_init_context().

    @description
    This function is a companion function to eap_init_context(). It is
    intended to be called by application code and is not used by the
    generic 802.1X protocol.
*/
void
eap_uninit_context(
  EapContext *c);

#endif /* _EAPAUTH_H */
