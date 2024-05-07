/**

   @copyright
   Copyright (c) 2012 - 2013, Rambus Inc. All rights reserved.

   File: dot1x_crypto.c

   @description
   Crypto functions used by the 802.1X protocol implemented
   using the QuickSec crypto library.

*/


#include "dot1xpae_crypto.h"
#include "sshincludes.h"
#include "sshcrypt.h"
#include "aes_keywrap.h"
#include <assert.h>

static const unsigned char
wrap_iv[8] = {
  0xa6, 0xa6, 0xa6, 0xa6, 0xa6, 0xa6, 0xa6, 0xa6
};

void
dot1x_crypto_random_init(
  Dot1xCryptoRandom *r)
{
  SshRandom random;

  assert(ssh_random_allocate("nist-sp-800-90", &random) == SSH_CRYPTO_OK);
  r->random = random;
}

void
dot1x_crypto_random_uninit(
  Dot1xCryptoRandom *r)
{
  SshRandom random = r->random;

  ssh_random_free(random);
  r->random = NULL;
}

int
dot1x_crypto_random_generate(
  Dot1xCryptoRandom *r,
  unsigned char *buf,
  unsigned len)
{
  SshRandom random = r->random;

  if (ssh_random_get_bytes(random, buf, len) != SSH_CRYPTO_OK)
    return 0;

  return 1;
}

void
dot1x_crypto_cmac_start(
  Dot1xCryptoCmac *c,
  const MacsecKey *key)
{
  SshCryptoStatus cs;
  SshMac mac;

  assert(key->len == 16 || key->len == 32);
  assert(ssh_mac_length("cmac-aes") == sizeof((Dot1xCryptoCmacValue *)0)->buf);

  cs = ssh_mac_allocate("cmac-aes", key->buf, key->len, &mac);
  assert (cs == SSH_CRYPTO_OK);
  c->mac = mac;
}

void
dot1x_crypto_cmac_update(
  Dot1xCryptoCmac *c,
  const unsigned char *buf,
  unsigned len)
{
  SshMac mac = c->mac;

  ssh_mac_update(mac, buf, len);
}

void
dot1x_crypto_cmac_finish(
  Dot1xCryptoCmac *c,
  Dot1xCryptoCmacValue *value)
{
  SshMac mac = c->mac;

  assert(ssh_mac_final(mac, value->buf) == SSH_CRYPTO_OK);

  ssh_mac_free(mac);
  c->mac = NULL;
}

void
dot1x_crypto_wrap(
  Dot1xCryptoWrap *wrap,
  const MacsecKey *kek,
  const MacsecKey *plain)
{
  SshCryptoStatus cs;

  assert(plain->len == 16 || plain->len == 32);
  assert(kek->len == 16 || kek->len == 32);

  if (plain->len == 16)
    wrap->len = 24;
  else
    wrap->len = 40;

  cs = ssh_aes_key_wrap_kek(
    kek->buf, kek->len,
    wrap_iv, sizeof wrap_iv,
    wrap->buf, wrap->len,
    plain->buf, plain->len);
  assert(cs == SSH_CRYPTO_OK);
}

void
dot1x_crypto_unwrap(
  const Dot1xCryptoWrap *wrap,
  const MacsecKey *kek,
  MacsecKey *plain)
{
  SshCryptoStatus cs;

  assert(wrap->len == 24 || wrap->len == 40);
  assert(kek->len == 16 || kek->len == 32);

  if (wrap->len == 24)
    plain->len = 16;
  else
    plain->len = 32;

  cs = ssh_aes_key_unwrap_kek(
    kek->buf, kek->len,
    wrap_iv, sizeof wrap_iv,
    plain->buf, plain->len,
    wrap->buf, wrap->len);
  assert(cs == SSH_CRYPTO_OK);
}
