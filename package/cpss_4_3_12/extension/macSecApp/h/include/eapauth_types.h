/**

   EAP related types.

   File: eapauth_types.h

   @description

   @copyright
   Copyright (c) 2012 - 2013, Rambus Inc. All rights reserved.

*/

#ifndef _EAPAUTH_TYPES_H
#define _EAPAUTH_TYPES_H

#include "macsec_types.h"
#include "dot1x_types.h"

typedef struct {
  struct EapKeystoreS *keystore;
  struct SshRadiusClientServerInfoRec *radius_servers;
  void (*wakeup_func)(void *);
  void *wakeup_arg;
} EapConfig;

typedef struct {
  struct EapContextS *context;
  Dot1xNid nid;
  struct SshEapConnectionRec *connection;
  struct SshEapRec *eap;
  struct SshEapRadiusConfigurationRec *radius_config;
  struct SshTimeoutRec *timeout;
  unsigned waitsignal : 1;
  int state;
  int protocol_state;
  unsigned char *catab[4];
  unsigned canum;
  unsigned char pktbuf[1500];
  unsigned pktlen;
} EapConversation;

typedef struct EapContextS {
  struct EapKeystoreS *keystore;
  struct SshRadiusClientServerInfoRec *radius_servers;
  struct SshEapConfigurationRec *client_config;
  struct SshEapConfigurationRec *server_config;
  void (*wakeup_func)(void *);
  void *wakeup_arg;
} EapContext;

struct EapKeystoreS *
eap_create_keystore(void);

void
eap_destroy_keystore(
  struct EapKeystoreS *keystore);

char *
eap_add_provider(
  struct EapKeystoreS *keystore,
  const char *ek_type,
  const char *ek_initialization_info);

void
eap_remove_provider(
  struct EapKeystoreS *keystore,
  char *provider);

struct SshCMCertificateRec *
eap_add_ca(
  struct EapKeystoreS *keystore,
  const unsigned char *derbuf,
  unsigned derlen);

void
eap_remove_ca(
  struct EapKeystoreS *keystore,
  struct SshCMCertificateRec *certificate);

void
eap_set_crl_check_disable(
  struct EapKeystoreS *keystore,
  int value);

struct SshCMCrlRec *
eap_add_crl(
  struct EapKeystoreS *keystore,
  const unsigned char *derbuf,
  unsigned derlen);

void
eap_remove_crl(
  struct EapKeystoreS *keystore,
  struct SshCMCrlRec *certificate);

int
eap_set_ldap_servers(
  struct EapKeystoreS *keystore,
  const char *servers);

struct EapNetworkS *
eap_add_network(
  struct EapKeystoreS *keystore,
  const Dot1xNid *nid,
  const char *identity);

void
eap_remove_network(
  struct EapNetworkS *n);

#endif /* _EAPAUTH_TYPES_H */
