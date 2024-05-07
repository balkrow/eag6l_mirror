/**

   @copyright
   Copyright (c) 2012 - 2013, Rambus Inc. All rights reserved.

   File: eapauth.c

   @description
   EAP functions implemented using QuickSec libraries.

*/

#include "eapauth.h"
#include "sshincludes.h"
#include "ssheap.h"
#include "sshtls.h"
#include "sshtimeouts.h"
#include "sshexternalkey.h"
#include "macsec_list.h"
#include <stddef.h>

#define SSH_DEBUG_MODULE "Eapauth"

/*
 * Types.
 */

/*
 * Certificate entry.
 */
typedef struct EapCertificateS {
  unsigned char *derbuf;
  unsigned derlen;
  struct EapKeypairS *keypair;
  MacsecListLink link;
  int inserted;
} EapCertificate;

/*
 * Certificate list and private key entry.
 */
typedef struct EapKeypairS {
  Dot1xNid nid;
  char *keypath;
  MacsecListLink certificates;
  SshPrivateKey private_key;
  struct EapKeystoreS *keystore;
  MacsecListLink link;
  int inserted;
} EapKeypair;

/*
 * Network entry.
 */
typedef struct EapNetworkS {
  Dot1xNid nid;
  char *identity;
  MacsecListLink link;
  int inserted;
} EapNetwork;

/*
 * Keystore data.
 */
typedef struct EapKeystoreS {
  SshCMContext cm;
  SshExternalKey ek;
  SshOperationHandle ek_operation;
  int ek_cert_index;
  MacsecListLink keypairs;
  MacsecListLink networks;
  int crl_check_disable;
} EapKeystore;

/*
 * Prototypes.
 */

/*
 * Externalkey notification callback.
 */
static void
ek_notify(
  SshEkEvent event, const char *keypath, const char *label,
  SshEkUsageFlags flags, void *context);

/*
 * Externalkey certificate retrieval callback.
 */
static void
ek_get_certificate_cb(
  SshEkStatus status, const unsigned char *cert_return,
  size_t cert_return_length, void *context);

/*
 * Externalkey private key retrieval callback.
 */
static void
ek_get_private_key_cb(
  SshEkStatus status, SshPrivateKey private_key_return, void *context);

/*
 * Try adding certificate to certificate manager.
 */
static void
manage_certificate(EapCertificate *c);

/*
 * Remove keypair from its keystore and free everything.
 */
static void
destroy_keypair(EapKeypair *kp);

/*
 * Remove certificate from its keypair and free everything.
 */
static void
destroy_certificate(EapCertificate *c);

/*
 * Function called by the EAP library to send an EAP packet to the
 * network.
 */
static void
output(SshEapConnection con, void *ctx, const SshBuffer buf);

/*
 * Function called by the EAP library to indicate status change or
 * request information, supplicant side.
 */
static void
client_signal(
  SshEap eap, SshUInt8 type, SshEapSignal signal, SshBuffer buf, void *ctx);

/*
 * Function called by the EAP library to indicate status change or
 * request information, authenticator side.
 */
static void
server_signal(
  SshEap eap, SshUInt8 type, SshEapSignal signal, SshBuffer buf, void *ctx);

/*
 * Terminate supplicant after timed out waiting for the first packet.
 */
static void
timeout_eap(void *context);

/*
 * Wrapper for ssh_eap_authenticate(c->eap, SSH_EAP_AUTH_CONTINUE),
 * used to cut recursion using a zero second timeout.
 */
static void
continue_eap(void *context);

/*
 * Change EAP protocol state and signal application if appropriate.
 */
static void
new_protocol_state(EapConversation *c, EapState protocol_state);

/*
 * Function called by the certificate manager during certificate
 * authority enumeration.
 */
static void
next_ca(SshCMCertificate cert, void *context);

/*
 * Network lookup by NID.
 */
static EapNetwork *
lookup_network(EapKeystore *ks, const Dot1xNid *nid);

/*
 * Keypair lookup by keypath.
 */
static EapKeypair *
lookup_keypair(EapKeystore *ks, const char *keypath);

/*
 * Static variables.
 */

/*
 *  Public functions.
 */

EapConversation *
eap_create_supplicant(
  EapContext *context,
  const EapPartner *partner)
{
  EapConversation *c = NULL;
  SshEapTlsParamsStruct tls_params;

  if (!(c = ssh_calloc(1, sizeof *c)))
    {
      SSH_TRACE(SSH_D_ERROR, ("out of memory"));
      goto fail;
    }
  c->context = context;
  c->nid = partner->nid;

  if (!(c->connection = ssh_eap_connection_create_cb(output, c)))
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot create EAP connection"));
      goto fail;
    }

  if (!(c->eap = ssh_eap_create_client(
          c, context->client_config, c->connection)))
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot create EAP client"));
      goto fail;
    }

  if (ssh_eap_accept_auth(c->eap, SSH_EAP_TYPE_TLS, 0) !=
      SSH_EAP_OPSTATUS_SUCCESS)
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot use EAP-TLS"));
      goto fail;
    }

  tls_params.cm = context->keystore->cm;
  if (context->keystore->crl_check_disable)
    tls_params.crl_check_pol = SSH_TLS_CRL_CHECK_NEVER;
  else
    tls_params.crl_check_pol = SSH_TLS_CRL_CHECK_ALWAYS;

  if (ssh_eap_configure_protocol(
        c->eap, SSH_EAP_TYPE_TLS, (void *)&tls_params, sizeof tls_params) !=
      SSH_EAP_OPSTATUS_SUCCESS)
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot configure EAP-TLS"));
      goto fail;
    }

  /* ensure timeout if no packets are received (the EAP library does
     not) */
  if (!(c->timeout = ssh_register_timeout(NULL, 2, 0, timeout_eap, c)))
    {
      SSH_TRACE(SSH_D_ERROR, ("out of timers"));
      goto fail;
    }
  c->waitsignal = 1;

  return c;

 fail:
  eap_destroy(c);
  return NULL;
}

EapConversation *
eap_create_authenticator(
  EapContext *context,
  const EapPartner *partner)
{
  EapConversation *c = NULL;
  SshEapTlsParamsStruct tls_params;

  (void)partner;

  if (!(c = ssh_calloc(1, sizeof *c)))
    {
      SSH_TRACE(SSH_D_ERROR, ("out of memory"));
      goto fail;
    }
  c->context = context;

  if (!(c->connection = ssh_eap_connection_create_cb(output, c)))
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot create EAP connection"));
      goto fail;
    }

  if (!(c->eap = ssh_eap_create_server(
          c, context->server_config, c->connection)))
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot create EAP server"));
      goto fail;
    }

  if (ssh_eap_accept_auth(c->eap, SSH_EAP_TYPE_TLS, 0) !=
      SSH_EAP_OPSTATUS_SUCCESS)
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot use EAP-TLS"));
      goto fail;
    }

  if (context->radius_servers)
    {
      if (!(c->radius_config = ssh_calloc(1, sizeof *c->radius_config)))
        {
          SSH_TRACE(SSH_D_ERROR, ("out of memory"));
          goto fail;
        }
      if (!(c->radius_config->radius_client = ssh_radius_client_create(NULL)))
        {
          SSH_TRACE(SSH_D_ERROR, ("cannot create RADIUS client"));
          goto fail;
        }
      c->radius_config->radius_servers = context->radius_servers;
      ssh_eap_radius_attach(c->eap, c->radius_config);
    }

  tls_params.cm = context->keystore->cm;
  if (context->keystore->crl_check_disable)
    tls_params.crl_check_pol = SSH_TLS_CRL_CHECK_NEVER;
  else
    tls_params.crl_check_pol = SSH_TLS_CRL_CHECK_ALWAYS;

  if (ssh_eap_configure_protocol(
        c->eap, SSH_EAP_TYPE_TLS, (void *)&tls_params, sizeof tls_params) !=
      SSH_EAP_OPSTATUS_SUCCESS)
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot configure EAP-TLS"));
      goto fail;
    }

  ssh_eap_send_identification_request(c->eap, NULL, 0);

  return c;

 fail:
  eap_destroy(c);
  return NULL;
}

void
eap_destroy(
  EapConversation *c)
{
  if (!c)
    return;

  if (c->timeout)
    ssh_cancel_timeout(c->timeout);

  if (c->eap)
    ssh_eap_destroy(c->eap);

  if (c->connection)
    ssh_eap_connection_destroy(c->connection);

  if (c->radius_config)
    {
      if (c->radius_config->radius_client)
        ssh_radius_client_destroy(c->radius_config->radius_client);
      ssh_free(c->radius_config);
    }

  ssh_free(c);
}

EapState
eap_state(
  const EapConversation *c)
{
  return c->state;
}

void
eap_get_results(
  EapConversation *c,
  MacsecSessionId *session_id,
  MacsecMsk *msk,
  MacsecAuthData *auth_data)
{
  unsigned char *mskptr = NULL, *sidptr = NULL;
  size_t msklen, sidlen;

  if (c->state != EAP_STATE_SUCCEEDED)
    goto zero;

  ssh_eap_master_session_key(c->eap, &mskptr, &msklen, &sidptr, &sidlen);

  if (mskptr == NULL || msklen == 0)
    {
      SSH_TRACE(SSH_D_ERROR, ("no EAP MSK"));
      msk->len = 0;
    }
  else if (msklen < sizeof msk->buf)
    {
      SSH_TRACE(SSH_D_ERROR, ("short EAP MSK"));
      msk->len = 0;
    }
  else
    {
      if (msklen > sizeof msk->buf)
        msk->len = sizeof msk->buf;
      else
        msk->len = msklen;

      memcpy(msk->buf, mskptr, msk->len);
    }

  if (sidptr == 0 || sidlen == 0)
    {
      SSH_TRACE(SSH_D_ERROR, ("no EAP session id"));
      session_id->len = 0;
    }
  else
    {
      if (sidlen > sizeof session_id->buf)
        session_id->len = sizeof session_id->buf;
      else
        session_id->len = sidlen;

      memcpy(session_id->buf, sidptr, session_id->len);
    }

  auth_data->len = 0;

  goto end;

 zero:
  session_id->len = 0;
  msk->len = 0;
  auth_data->len = 0;

 end:
  if (mskptr)
    ssh_free(mskptr);
  if (sidptr)
    ssh_free(sidptr);
}

unsigned
eap_produce_packet(
  EapConversation *c,
  unsigned char *buf,
  unsigned len)
{
  unsigned l;

  if (c->pktlen == 0)
    return 0;

  l = c->pktlen;
  c->pktlen = 0;

  if (c->state == EAP_STATE_INCOMPLETE && c->protocol_state != c->state)
    {
      c->state = c->protocol_state;
      c->context->wakeup_func(c->context->wakeup_arg);
    }

  if (l > len)
    {
      SSH_TRACE(SSH_D_ERROR, ("EAP output packet too large"));
      return 0;
    }

  memcpy(buf, c->pktbuf, l);
  return l;
}

void
eap_consume_packet(
  EapConversation *c,
  const unsigned char *buf,
  unsigned len)
{
  SshBufferStruct b;

  ssh_buffer_init(&b);
  ssh_buffer_wrap(&b, (void *)buf, len);
  b.end = b.alloc;
  ssh_eap_connection_input_packet(c->connection, &b);
}

struct EapKeystoreS *
eap_create_keystore(void)
{
  EapKeystore *ks = NULL;
  SshCMConfig cmc;

  if (!(ks = ssh_calloc(1, sizeof *ks)))
    {
      SSH_TRACE(SSH_D_ERROR, ("out of memory"));
      goto fail;
    }
  macsec_list_init(&ks->keypairs);
  macsec_list_init(&ks->networks);

  /* ssh_cm_allocate() frees cmc if it fails, otherwise owns it */
  if (!(cmc = ssh_cm_config_allocate()) ||
      !(ks->cm = ssh_cm_allocate(cmc)))
    {
      SSH_TRACE(SSH_D_ERROR, ("out of memory"));
      goto fail;
    }

  if (!ssh_cm_edb_ldap_init(ks->cm, (void *)""))
    SSH_TRACE(SSH_D_ERROR, ("cannot init LDAP EDB"));

  if (!ssh_cm_edb_http_init(ks->cm))
    SSH_TRACE(SSH_D_ERROR, ("cannot init HTTP EDB"));

  if (!(ks->ek = ssh_ek_allocate()))
    {
      SSH_TRACE(SSH_D_ERROR, ("out of memory"));
      goto fail;
    }

  ssh_ek_register_notify(ks->ek, ek_notify, ks);

  return ks;

 fail:
  eap_destroy_keystore(ks);
  return NULL;
}

void
eap_destroy_keystore(
  struct EapKeystoreS *ks)
{
  if (!ks)
    return;

  if (ks->ek_operation)
    ssh_operation_abort(ks->ek_operation);

  macsec_list_foreach(
    &ks->networks, offsetof(EapNetwork, link), eap_remove_network);
  macsec_list_foreach(
    &ks->keypairs, offsetof(EapKeypair, link), destroy_keypair);

  if (ks->ek)
    ssh_ek_free(ks->ek, NULL, NULL);

  if (ks->cm)
    ssh_cm_free(ks->cm);

  ssh_free(ks);
}

char *
eap_add_provider(
  struct EapKeystoreS *ks,
  const char *ek_type,
  const char *ek_initialization_info)
{
  SshEkStatus status;
  char *provider;

  status = ssh_ek_add_provider(
    ks->ek, ek_type, ek_initialization_info, NULL, 0, &provider);
  if (status != SSH_EK_OK)
    {
      SSH_TRACE(SSH_D_ERROR,
        ("cannot add externalkey provider of type %s", ek_type));
      return NULL;
    }

  return provider;
}

void
eap_remove_provider(
  struct EapKeystoreS *ks,
  char *provider)
{
  ssh_ek_remove_provider(ks->ek, provider);
  ssh_free(provider);
}

struct SshCMCertificateRec *
eap_add_ca(
  struct EapKeystoreS *ks,
  const unsigned char *derbuf,
  unsigned derlen)
{
  SshCMContext cm = ks->cm;
  SshCMCertificate cmc = NULL;
  SshCMStatus status;

  if (!(cmc = ssh_cm_cert_allocate(cm)))
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot allocate CM certificate"));
      goto fail;
    }

  if ((status = ssh_cm_cert_set_ber(cmc, derbuf, derlen)) != SSH_CM_STATUS_OK)
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot decode CM certificate (%d)", status));
      goto fail;
    }

  if ((status = ssh_cm_cert_force_trusted(cmc) != SSH_CM_STATUS_OK))
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot set CM certificate trust (%d)", status));
      goto fail;
    }

  if ((status = ssh_cm_cert_set_locked(cmc)) != SSH_CM_STATUS_OK)
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot lock CM certificate (%d)", status));
      goto fail;
    }

  if ((status = ssh_cm_add(cmc)) != SSH_CM_STATUS_OK)
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot add CM certificate (%d)", status));
      goto fail;
    }
  return cmc;

 fail:
  if (cmc)
    ssh_cm_cert_free(cmc);
  return NULL;
}

void
eap_remove_ca(
  struct EapKeystoreS *ks,
  struct SshCMCertificateRec *cmc)
{
  SshCMStatus status;

  (void)ks;

  if ((status = ssh_cm_cert_set_unlocked(cmc)) != SSH_CM_STATUS_OK)
    SSH_TRACE(SSH_D_ERROR, ("cannot unlock CM certificate (%d)", status));

  ssh_cm_cert_remove(cmc);
}

void
eap_set_crl_check_disable(
  struct EapKeystoreS *keystore,
  int value)
{
  keystore->crl_check_disable = value;
}

struct SshCMCrlRec *
eap_add_crl(
  struct EapKeystoreS *ks,
  const unsigned char *derbuf,
  unsigned derlen)
{
  SshCMContext cm = ks->cm;
  SshCMCrl cmc = NULL;
  SshCMStatus status;

  if (!(cmc = ssh_cm_crl_allocate(cm)))
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot allocate CM CRL"));
      goto fail;
    }

  if ((status = ssh_cm_crl_set_ber(cmc, derbuf, derlen)) != SSH_CM_STATUS_OK)
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot decode CM CRL (%d)", status));
      goto fail;
    }

  if ((status = ssh_cm_add_crl(cmc)) != SSH_CM_STATUS_OK)
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot add CM CRL (%d)", status));
      goto fail;
    }
  return cmc;

 fail:
  if (cmc)
    ssh_cm_crl_free(cmc);
  return NULL;
}

void
eap_remove_crl(
  struct EapKeystoreS *ks,
  struct SshCMCrlRec *cmc)
{
  (void)ks;

  ssh_cm_crl_remove(cmc);
}

int
eap_set_ldap_servers(
  struct EapKeystoreS *ks,
  const char *servers)
{
  if (!ssh_cm_edb_ldap_init(ks->cm, (unsigned char *)servers))
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot configure LDAP servers"));
      return 0;
    }

  return 1;
}

struct EapNetworkS *
eap_add_network(
  struct EapKeystoreS *ks,
  const Dot1xNid *nid,
  const char *identity)
{
  EapNetwork *n = NULL;

  if (lookup_network(ks, nid))
    {
      SSH_TRACE(SSH_D_ERROR, ("network already exists"));
      goto fail;
    }

  if (!(n = ssh_calloc(1, sizeof *n)) ||
      !(n->identity = ssh_strdup(identity)))
    {
      SSH_TRACE(SSH_D_ERROR, ("out of memory"));
      goto fail;
    }
  memcpy(&n->nid.buf, nid->buf, nid->len);
  n->nid.len = nid->len;

  macsec_list_append(&ks->networks, &n->link);
  n->inserted = 1;
  return n;

 fail:
  eap_remove_network(n);
  return NULL;
}

void
eap_remove_network(
  struct EapNetworkS *n)
{
  if (!n)
    return;

  if (n->inserted)
    macsec_list_remove(&n->link);

  if (n->identity)
    ssh_free(n->identity);

  ssh_free(n);
}

int
eap_init_context(
  EapContext *c,
  const EapConfig *config)
{
  memset(c, 0, sizeof *c);

  if (!(c->client_config = ssh_eap_config_create()) ||
      !(c->server_config = ssh_eap_config_create()))
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot create EAP config"));
      goto fail;
    }

  c->keystore = config->keystore;
  c->radius_servers = config->radius_servers;
  c->wakeup_func = config->wakeup_func;
  c->wakeup_arg = config->wakeup_arg;

  c->client_config->auth_timeout_sec = 10;
  c->client_config->re_auth_delay_sec = 0;
  c->client_config->retransmit_delay_sec = 2;
  c->client_config->num_retransmit = 2;

  memcpy(c->server_config, c->client_config, sizeof *c->server_config);

  c->client_config->signal_cb = client_signal;
  c->server_config->signal_cb = server_signal;

  return 1;

 fail:
  eap_uninit_context(c);
  return 0;
}

void
eap_uninit_context(
  EapContext *c)
{
  if (c->client_config)
    ssh_eap_config_destroy(c->client_config);

  if (c->server_config)
    ssh_eap_config_destroy(c->server_config);

  memset(c, 0, sizeof *c);
}

/*
 * Static functions.
 */

static void
ek_notify(
  SshEkEvent event, const char *keypath, const char *label,
  SshEkUsageFlags flags, void *context)
{
  EapKeystore *ks = context;
  EapKeypair *kp;

  (void)label;
  (void)flags;

  switch (event)
    {
    case SSH_EK_EVENT_KEY_AVAILABLE:
      if (!(kp = ssh_calloc(1, sizeof *kp)) ||
          !(kp->keypath = ssh_strdup(keypath)))
        {
          SSH_TRACE(SSH_D_ERROR, ("out of memory"));
          if (kp)
            ssh_free(kp);
          return;
        }
      kp->keystore = ks;
      macsec_list_init(&kp->certificates);
      ks->ek_cert_index = 0;
      ks->ek_operation = ssh_ek_get_certificate(
        ks->ek, kp->keypath, ks->ek_cert_index, ek_get_certificate_cb, kp);
      return;

    case SSH_EK_EVENT_KEY_UNAVAILABLE:
      if (!(kp = lookup_keypair(ks, keypath)))
        {
          SSH_TRACE(SSH_D_ERROR, ("nonexistent keypath removed"));
          return;
        }
      destroy_keypair(kp);
      return;

    default:
      return;
    }
}

static void
ek_get_certificate_cb(
  SshEkStatus status, const unsigned char *cert_return,
  size_t cert_return_length, void *context)
{
  EapKeypair *kp = context;
  EapKeystore *ks = kp->keystore;
  EapCertificate *c = NULL;

  ks->ek_operation = NULL;
  ks->ek_cert_index++;

  if (status == SSH_EK_NO_MORE_CERTIFICATES)
    {
      /* get private key */
      ks->ek_operation = ssh_ek_get_private_key(
        ks->ek, kp->keypath, ek_get_private_key_cb, kp);
      return;
    }

  if (status != SSH_EK_OK)
    {
      SSH_TRACE(SSH_D_ERROR,
        ("ssh_ek_get_certificate(): %s", ssh_ek_get_printable_status(status)));
      goto fail;
    }

  if (!(c = ssh_calloc(1, sizeof *c)) ||
      !(c->derbuf = ssh_malloc(cert_return_length)))
    {
      SSH_TRACE(SSH_D_ERROR, ("out of memory"));
      goto fail;
    }
  memcpy(c->derbuf, cert_return, cert_return_length);
  c->derlen = cert_return_length;

  macsec_list_append(&kp->certificates, &c->link);
  c->keypair = kp;
  c->inserted = 1;
  c = NULL;

  /* get next certificate */
  ks->ek_operation = ssh_ek_get_certificate(
    ks->ek, kp->keypath, ks->ek_cert_index, ek_get_certificate_cb, kp);
  return;

 fail:
  destroy_keypair(kp);
  if (c)
    destroy_certificate(c);
}

static void
ek_get_private_key_cb(
  SshEkStatus status, SshPrivateKey private_key_return, void *context)
{
  EapKeypair *kp = context;
  EapKeystore *ks = kp->keystore;

  if (status != SSH_EK_OK)
    {
      SSH_TRACE(SSH_D_ERROR,
        ("ssh_ek_get_private_key(): %s", ssh_ek_get_printable_status(status)));
      goto fail;
    }

  kp->private_key = private_key_return;

  macsec_list_foreach(
    &kp->certificates, offsetof(EapCertificate, link), manage_certificate);

  macsec_list_append(&ks->keypairs, &kp->link);
  kp->inserted = 1;

  return;

 fail:
  destroy_keypair(kp);
}

static void
manage_certificate(EapCertificate *c)
{
  SshCMContext cm = c->keypair->keystore->cm;
  SshCMCertificate cmc = NULL;
  SshCMStatus status;

  if (!(cmc = ssh_cm_cert_allocate(cm)))
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot allocate CM certificate"));
      goto fail;
    }

  status = ssh_cm_cert_set_ber(cmc, c->derbuf, c->derlen);
  if (status != SSH_CM_STATUS_OK)
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot decode CM certificate (%d)", status));
      goto fail;
    }

  if ((status = ssh_cm_add(cmc)) != SSH_CM_STATUS_OK)
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot add CM certificate (%d)", status));
      goto fail;
    }
  return;

 fail:
  if (cmc)
    ssh_cm_cert_free(cmc);
}

static void
destroy_keypair(EapKeypair *kp)
{
  if (!kp)
    return;

  if (kp->inserted)
    macsec_list_remove(&kp->link);

  if (kp->private_key)
    ssh_private_key_free(kp->private_key);

  macsec_list_foreach(
    &kp->certificates, offsetof(EapCertificate, link), destroy_certificate);

  if (kp->keypath)
    ssh_free(kp->keypath);

  memset(kp, 0, sizeof *kp);
  ssh_free(kp);
}

static void
destroy_certificate(EapCertificate *c)
{
  if (!c)
    return;

  if (c->inserted)
    macsec_list_remove(&c->link);

  if (c->derbuf)
    ssh_free(c->derbuf);

  memset(c, 0, sizeof *c);
  ssh_free(c);
}

static void
output(SshEapConnection con, void *ctx, const SshBuffer b)
{
  EapConversation *c = ctx;

  (void)con;

  if (ssh_buffer_len(b) >= sizeof c->pktbuf)
    {
      SSH_TRACE(SSH_D_ERROR, ("EAP output packet too large"));
      return;
    }

  if (c->protocol_state != EAP_STATE_INCOMPLETE)
    {
      SSH_TRACE(SSH_D_ERROR,
        ("ignoring output packet from terminated EAP conversation"));
      return;
    }

  memcpy(c->pktbuf, ssh_buffer_ptr(b), ssh_buffer_len(b));
  c->pktlen = ssh_buffer_len(b);

  c->context->wakeup_func(c->context->wakeup_arg);
}

static void
client_signal(
  SshEap eap, SshUInt8 type, SshEapSignal signal, SshBuffer buf, void *ctx)
{
  EapConversation *c = ctx;
  EapKeystore *ks = c->context->keystore;
  EapNetwork *n;
  EapKeypair *kp;
  SshEapTokenType token_type;
  SshEapTokenStruct token;

  (void)eap;

  /* remove no-packet timer on first signal */
  if (c->timeout && c->waitsignal)
    {
      ssh_cancel_timeout(c->timeout);
      c->timeout = NULL;
      c->waitsignal = 0;
    }

  switch (signal)
    {
      /* signals indicating success */
    case SSH_EAP_SIGNAL_AUTH_PEER_OK:
      new_protocol_state(c, EAP_STATE_SUCCEEDED);
      break;

      /* signals indicating failure */
    case SSH_EAP_SIGNAL_AUTH_FAIL_REPLY:
    case SSH_EAP_SIGNAL_AUTH_FAIL_NEGOTIATION:
    case SSH_EAP_SIGNAL_AUTH_FAIL_AUTHENTICATOR:
    case SSH_EAP_SIGNAL_NOTIFICATION:
      new_protocol_state(c, EAP_STATE_FAILED);
      break;

      /* signals indicating timeout */
    case SSH_EAP_SIGNAL_AUTH_FAIL_TIMEOUT:
      new_protocol_state(c, EAP_STATE_TIMEDOUT);
      break;

      /* signals causing immediate termination */
    case SSH_EAP_SIGNAL_FATAL_ERROR:
      SSH_TRACE(SSH_D_ERROR,
        ("terminating EAP conversation due to signal '%s'",
         ssh_eap_signal_code_to_string(signal)));
      ssh_eap_destroy(c->eap);
      c->eap = NULL;
      new_protocol_state(c, EAP_STATE_FAILED);
      break;

      /* singal requesting credentials */
    case SSH_EAP_SIGNAL_NEED_TOKEN:
      token_type = ssh_eap_get_token_type_from_buf(buf);
      switch (token_type)
        {
        case SSH_EAP_TOKEN_USERNAME:
          if ((n = lookup_network(ks, &c->nid)))
            ssh_eap_init_token_username(
              &token, (unsigned char *)n->identity, strlen(n->identity));
          else
            ssh_eap_init_token_username(&token, NULL, 0);
          ssh_eap_token(c->eap, type, &token);
          break;

        case SSH_EAP_TOKEN_PRIVATE_KEY:
          if (!(kp = macsec_list_next(
                  &ks->keypairs, offsetof(EapKeypair, link), NULL)))
            {
              SSH_TRACE(SSH_D_ERROR, ("no private key for NID"));
              ssh_eap_init_token_private_key(&token, NULL, NULL, 0);
            }
          else
            {
              ssh_eap_init_token_private_key(&token, kp->private_key, NULL, 0);
            }
          ssh_eap_token(c->eap, type, &token);
          break;

        default:
          SSH_TRACE(SSH_D_ERROR,
            ("unhandled EAP token request %d", (int)token_type));
          ssh_eap_init_token_typed(&token, token_type, NULL, 0);
          ssh_eap_token(c->eap, type, &token);
          break;
        }
      break;

      /* logged and ignored signals */
    case SSH_EAP_SIGNAL_PACKET_DISCARDED:
    case SSH_EAP_SIGNAL_TOKEN_DISCARDED:
      SSH_TRACE(SSH_D_ERROR,
        ("EAP signal '%s'", ssh_eap_signal_code_to_string(signal)));
      break;

      /* unexpected signals */
    case SSH_EAP_SIGNAL_IDENTITY:
    case SSH_EAP_SIGNAL_AUTH_AUTHENTICATOR_OK:
    case SSH_EAP_SIGNAL_AUTH_FAIL_USERNAME:
    case SSH_EAP_SIGNAL_AUTH_OK_USERNAME:
      SSH_TRACE(SSH_D_ERROR,
        ("unexpected EAP signal %s", ssh_eap_signal_code_to_string(signal)));
      break;

      /* completely ignored signals */
    case SSH_EAP_SIGNAL_AUTH_PEER_MAYBE_OK:
    case SSH_EAP_SIGNAL_NONE:
      break;
    }
}

static void
server_signal(
  SshEap eap, SshUInt8 type, SshEapSignal signal, SshBuffer buf, void *ctx)
{
  EapConversation *c = ctx;
  EapKeystore *ks = c->context->keystore;
  SshCMContext cm = ks->cm;
  EapKeypair *kp;
  SshEapTokenType token_type;
  SshEapTokenStruct token;
  SshCMStatus cs;
  int i;

  (void)eap;

  switch (signal)
    {
      /* signals indicating success */
    case SSH_EAP_SIGNAL_AUTH_AUTHENTICATOR_OK:
      new_protocol_state(c, EAP_STATE_SUCCEEDED);
      break;

      /* signals indicating failure */
    case SSH_EAP_SIGNAL_AUTH_FAIL_REPLY:
    case SSH_EAP_SIGNAL_AUTH_FAIL_NEGOTIATION:
    case SSH_EAP_SIGNAL_NOTIFICATION:
      new_protocol_state(c, EAP_STATE_FAILED);
      break;

      /* signals indicating timeout */
    case SSH_EAP_SIGNAL_AUTH_FAIL_TIMEOUT:
      new_protocol_state(c, EAP_STATE_TIMEDOUT);
      break;

      /* signals causing immediate termination */
    case SSH_EAP_SIGNAL_FATAL_ERROR:
      SSH_TRACE(SSH_D_ERROR,
        ("terminating EAP conversation due to signal '%s'",
         ssh_eap_signal_code_to_string(signal)));
      ssh_eap_destroy(c->eap);
      c->eap = NULL;
      new_protocol_state(c, EAP_STATE_FAILED);
      break;

      /* signal reporting received identity */
    case SSH_EAP_SIGNAL_IDENTITY:
      if (c->timeout)
        ssh_cancel_timeout(c->timeout);
      if (!(c->timeout = ssh_register_timeout(NULL, 0, 0, continue_eap, c)))
        {
          SSH_TRACE(SSH_D_ERROR, ("out of timers"));
          ssh_eap_authenticate(c->eap, SSH_EAP_AUTH_FAILURE);
        }
      break;

      /* signal requesting credentials */
    case SSH_EAP_SIGNAL_NEED_TOKEN:
      token_type = ssh_eap_get_token_type_from_buf(buf);
      switch (token_type)
        {
        case SSH_EAP_TOKEN_CERTIFICATE_AUTHORITY:
          c->canum = 0;
          cs = ssh_cm_cert_enumerate_class(
            cm, SSH_CM_CCLASS_TRUSTED, next_ca, c);
          if (cs != SSH_CM_STATUS_OK)
            SSH_TRACE(SSH_D_ERROR, ("CA enumerate failed (%d)", (int)cs));
          c->catab[c->canum] = NULL;
          ssh_eap_init_token_certificate_authority(&token, c->catab);
          ssh_eap_token(c->eap, type, &token);
          for (i = 0; i < c->canum; i++)
            ssh_free(c->catab[i]);
          c->canum = 0;
          break;

        case SSH_EAP_TOKEN_PRIVATE_KEY:
          if (!(kp = macsec_list_next(
                  &ks->keypairs, offsetof(EapKeypair, link), NULL)))
            {
              SSH_TRACE(SSH_D_ERROR, ("no private key for NID"));
              ssh_eap_authenticate(c->eap, SSH_EAP_AUTH_FAILURE);
            }
          else
            {
              ssh_eap_init_token_private_key(&token, kp->private_key, NULL, 0);
              ssh_eap_token(c->eap, type, &token);
            }
          break;

        default:
          SSH_TRACE(SSH_D_ERROR, ("unhandled EAP token %d", (int)token_type));
          ssh_eap_authenticate(c->eap, SSH_EAP_AUTH_FAILURE);
          break;
        }
      break;

      /* logged and ignored signals */
    case SSH_EAP_SIGNAL_PACKET_DISCARDED:
    case SSH_EAP_SIGNAL_TOKEN_DISCARDED:
      SSH_TRACE(SSH_D_ERROR,
        ("EAP signal '%s'", ssh_eap_signal_code_to_string(signal)));
      break;

      /* unexpected signals */
    case SSH_EAP_SIGNAL_AUTH_PEER_OK:
    case SSH_EAP_SIGNAL_AUTH_PEER_MAYBE_OK:
    case SSH_EAP_SIGNAL_AUTH_FAIL_AUTHENTICATOR:
    case SSH_EAP_SIGNAL_AUTH_FAIL_USERNAME:
    case SSH_EAP_SIGNAL_AUTH_OK_USERNAME:
      SSH_TRACE(SSH_D_ERROR,
        ("unexpected EAP signal %s", ssh_eap_signal_code_to_string(signal)));
      break;

      /* completely ignored signals */
    case SSH_EAP_SIGNAL_NONE:
      break;
    }
}

static void
timeout_eap(void *context)
{
  EapConversation *c = context;

  ssh_cancel_timeout(c->timeout);
  c->timeout = NULL;
  c->waitsignal = 0;

  ssh_eap_destroy(c->eap);
  c->eap = NULL;

  new_protocol_state(c, EAP_STATE_TIMEDOUT);
}

static void
continue_eap(void *context)
{
  EapConversation *c = context;

  ssh_cancel_timeout(c->timeout);
  c->timeout = NULL;
  ssh_eap_authenticate(c->eap, SSH_EAP_AUTH_CONTINUE);
}

static void
new_protocol_state(EapConversation *c, EapState state)
{
  if (c->protocol_state == state)
    return;

  if (c->protocol_state != EAP_STATE_INCOMPLETE)
    {
      SSH_TRACE(SSH_D_ERROR, ("ignoring extra EAP state change to %d", state));
      return;
    }

  c->protocol_state = state;

  if (c->pktlen == 0)
    {
      c->state = c->protocol_state;
      c->context->wakeup_func(c->context->wakeup_arg);
    }
}

static void
next_ca(SshCMCertificate cert, void *context)
{
  EapConversation *c = context;
  SshX509Certificate x509 = NULL;
  unsigned char *der = NULL;
  size_t der_len;

  if (c->canum >= sizeof c->catab / sizeof c->catab[0] - 1)
    return;

  if (ssh_cm_cert_get_x509(cert, &x509) != SSH_CM_STATUS_OK)
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot extract CA cert"));
      goto fail;
    }

  if (!ssh_x509_cert_get_subject_name_der(x509, &der, &der_len))
    {
      SSH_TRACE(SSH_D_ERROR, ("cannot get CA cert subject name"));
      goto fail;
    }
  ssh_x509_cert_free(x509);
  x509 = NULL;

  c->catab[c->canum++] = der;
  return;

 fail:
  if (der)
    ssh_free(der);
  if (x509)
    ssh_x509_cert_free(x509);
}

static EapNetwork *
lookup_network(EapKeystore *ks, const Dot1xNid *nid)
{
  EapNetwork *n = NULL;

  while ((n = macsec_list_next(&ks->networks, offsetof(EapNetwork, link), n)))
    if (n->nid.len == nid->len && !memcmp(&n->nid.buf, nid->buf, nid->len))
      return n;
  return NULL;
}

static EapKeypair *
lookup_keypair(EapKeystore *ks, const char *keypath)
{
  EapKeypair *kp = NULL;

  while ((kp = macsec_list_next(
            &ks->keypairs, offsetof(EapKeypair, link), kp)))
    if (!strcmp(kp->keypath, keypath))
      return kp;
  return NULL;
}
