/**

   @copyright
   Copyright (c) 2012 - 2013, Rambus Inc. All rights reserved.

   File: softsec_platform.c

   @description
   Platform-specific functions for the software MACsec protocol for
   software cryptography based on the QuickSec library.

*/

#include "softsec_platform.h"
#include "softsec_crypto.h"
#include "sshincludes.h"
#include "sshcrypt.h"
#include "mode-gcm.h"
#include <assert.h>

void
softsec_crypto_init(
  SoftsecCryptoContext *c,
  const MacsecKey *key)
{
  SshCryptoStatus status;

  assert(sizeof c->encrctx >= ssh_gcm_aes_ctxsize());
  assert(sizeof c->decrctx >= ssh_gcm_aes_ctxsize());
  assert(key->len == 16 || key->len == 32);

  status = ssh_gcm_aes_init(c->encrctx, key->buf, key->len, TRUE);
  assert(status == SSH_CRYPTO_OK);
  status = ssh_gcm_aes_init(c->decrctx, key->buf, key->len, FALSE);
  assert(status == SSH_CRYPTO_OK);
}

void
softsec_crypto_uninit(
  SoftsecCryptoContext *c)
{
  memset(c->encrctx, 0, sizeof c->encrctx);
  memset(c->decrctx, 0, sizeof c->decrctx);
}

void
softsec_crypto_encrypt(
  SoftsecCryptoContext *c,
  SoftsecCryptoWriteBytes *ci,
  SoftsecCryptoTag *tag,
  const SoftsecCryptoIv *iv,
  SoftsecCryptoReadBytes *pl,
  SoftsecCryptoReadBytes *aad)
{
  SshCryptoStatus status;

  memcpy(c->ivctr, iv, sizeof *iv);
  c->ivctr[12] = 0x00;
  c->ivctr[13] = 0x00;
  c->ivctr[14] = 0x00;
  c->ivctr[15] = 0x01;
  ssh_gcm_reset(c->encrctx);

  ssh_gcm_update(c->encrctx, aad->buf, aad->len);

  assert(ci->len == pl->len);
  status = ssh_gcm_transform(c->encrctx, ci->buf, pl->buf, ci->len, c->ivctr);
  assert(status == SSH_CRYPTO_OK);

  status = ssh_gcm_final(c->encrctx, tag->buf);
  assert(status == SSH_CRYPTO_OK);
}

void
softsec_crypto_decrypt(
  SoftsecCryptoContext *c,
  unsigned *authentic,
  SoftsecCryptoWriteBytes *pl,
  const SoftsecCryptoIv *iv,
  SoftsecCryptoReadBytes *ci,
  SoftsecCryptoReadBytes *aad,
  const SoftsecCryptoTag *tag)
{
  SshCryptoStatus status;
  SoftsecCryptoTag tag0;

  memcpy(c->ivctr, iv, sizeof *iv);
  c->ivctr[12] = 0x00;
  c->ivctr[13] = 0x00;
  c->ivctr[14] = 0x00;
  c->ivctr[15] = 0x01;
  ssh_gcm_reset(c->decrctx);

  ssh_gcm_update(c->decrctx, aad->buf, aad->len);

  assert(pl->len == ci->len);
  status = ssh_gcm_transform(c->decrctx, pl->buf, ci->buf, ci->len, c->ivctr);
  assert(status == SSH_CRYPTO_OK);

  status = ssh_gcm_final(c->decrctx, tag0.buf);
  assert(status == SSH_CRYPTO_OK);

  *authentic = !memcmp(tag0.buf, tag->buf, sizeof tag0.buf);
}
