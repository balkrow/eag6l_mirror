/**
   
   @copyright
   Copyright (c) 2007 - 2013, Rambus Inc. All rights reserved.
   
    File: cmac.c
   
   Implementation of AES CMAC MAC algorithm according to RFC 4493. Note that 
   RFC 4493 specifies CMAC only for the AES cipher. The original NIST 
   specification for CMAC (NIST, Special Publication 800-38B) allows CMAC
   to be used for any block cipher. The current implementation follows RFC
   4493 and assumes the cipher is AES. RFC 4493 only allows a 128 bit key
   but this implementation also supports 192 and 256 bit keys. If use of
   CMAC is required for other ciphers then AES, this implementation will
   need modification. The current implementation assumes a block cipher
   length of 16 bytes.
   
*/


#include "sshincludes.h"
#include "sshmp-xuint.h"
#include "sshcrypt.h"
#include "sshhash_i.h"
#include "sshmac_i.h"
#include "cmac.h"

#define SSH_DEBUG_MODULE "SshCryptCMac"

/* Generic CMAC interface code. */
typedef struct
{
  /* Cipher */
  const SshCipherMacBaseDefStruct *cipher_basedef;

  /* Subkeys */
  SshXUInt128 key1, key2;

  /* To hold the intermediate mac value, and any necessary buffering when the
   input data is not a multiple of the cipher block length. */
  unsigned char *workarea;

  /* This variable indicates position in the workarea buffer. */
  unsigned int counter;

  /* Error status */
  SshCryptoStatus status;

  /* Cipher context. */
  void *context;
} SshCmacCtx;


size_t ssh_cmac_ctxsize(const SshCipherMacBaseDefStruct *cipher_def)
{
  return
    sizeof(SshCmacCtx) +
    2 * cipher_def->block_length + /* two blocks for the workarea */
    (*cipher_def->ctxsize)(); /* for the cipher context */
}


SshCryptoStatus
ssh_cmac_init(void *context, const unsigned char *key, size_t keylen,
	      const SshCipherMacBaseDefStruct *cipher_def)
{
  SshCmacCtx *created = context;
  unsigned char iv[SSH_CIPHER_MAX_BLOCK_SIZE];
  unsigned char tmp[SSH_CIPHER_MAX_BLOCK_SIZE];
  SshCryptoStatus status;
  SshXUInt128 Rb;

  SSH_DEBUG(SSH_D_MY, ("Entered CMAC init"));

  /* Align the pointers in the allocated memory. */
  created->workarea = (unsigned char *)created + sizeof(SshCmacCtx);

  created->context = (unsigned char *)created->workarea +
    2 * cipher_def->block_length;

  /* Set the cipher def. */
  created->cipher_basedef = cipher_def;
  created->status = SSH_CRYPTO_OK;

  /* Clear workarea and the iv. */
  memset(created->workarea, 0, 2 * cipher_def->block_length);
  memset(iv, 0, cipher_def->block_length);
  memset(tmp, 0, cipher_def->block_length);

  /* RFC 4493 defines CMAC only for 128 bit keys. NIST SP 800-38B also
     allows 192 and 256 byte keys. */
  if (keylen != 16 && keylen != 24 && keylen != 32)
    return SSH_CRYPTO_KEY_SIZE_INVALID;
  
  if (keylen % cipher_def->block_length != 0)
    return SSH_CRYPTO_KEY_SIZE_INVALID;
  
  status = (*created->cipher_basedef->init)(created->context,
                                            key, keylen, TRUE);

  if (status != SSH_CRYPTO_OK)
    return status;

  /* Now compute the keys 'key1', 'key2' by encrypting with the base key. 
     We can use the CBC mac to encrypt, as it is equivalent to ECB encryption 
     for a single block (providing the iv is zero). */
  status = (*created->cipher_basedef->cbcmac)(created->context, tmp,
					      cipher_def->block_length, iv);

  if (status != SSH_CRYPTO_OK)
    return status;

  /* Generate K1 */
  SSH_XUINT128_BUILD(Rb, 0x87, 0, 0, 0);
  SSH_XUINT128_GET(created->key1, iv);

  if (SSH_XUINT128_EXTRACT_UINT32(created->key1, 3) & 0x80000000)
    {
      SSH_XUINT128_SLL(created->key1, created->key1, 1);      
      SSH_XUINT128_XOR(created->key1, created->key1, Rb);
    }
  else
    {
      SSH_XUINT128_SLL(created->key1, created->key1, 1);
    }

  /* Generate K2 */
  SSH_XUINT128_ASSIGN(created->key2, created->key1);
  
  if (SSH_XUINT128_EXTRACT_UINT32(created->key2, 3) & 0x80000000)
    {
      SSH_XUINT128_SLL(created->key2, created->key2, 1);      
      SSH_XUINT128_XOR(created->key2, created->key2, Rb);
    }
  else
    {
      SSH_XUINT128_SLL(created->key2, created->key2, 1);
    }

  
  /* Clean the iv and cipher context */
  memset(iv, 0, cipher_def->block_length);
  memset(created->context, 0, (*cipher_def->ctxsize)());
  
  if (created->cipher_basedef->uninit)
    (*created->cipher_basedef->uninit)(created->context);
  
  /* Reinitialize the cipher using the key 'key' */
  status = (*created->cipher_basedef->init)(created->context,
                                            key, keylen, TRUE);

  if (status != SSH_CRYPTO_OK)
    return status;

  SSH_DEBUG(SSH_D_MY, ("CMAC initialized OK"));
  return SSH_CRYPTO_OK;
}


void ssh_cmac_start(void *context)
{
  SshCmacCtx *ctx = context;

  /* Initialize counter */
  ctx->counter = 0;

  /* Clear workarea and the iv. */
  memset(ctx->workarea, 0, 2 * ctx->cipher_basedef->block_length);

  return;
}

/* This is much the same as the cbc-mac update. */
void ssh_cmac_update(void *context, const unsigned char *buf,
                        size_t len)
{
  SshCmacCtx *ctx = context;
  unsigned int i, j;
  unsigned char *iv, *block;
  SshCryptoStatus status;

  iv = ctx->workarea;
  block = iv + ctx->cipher_basedef->block_length;

  SSH_DEBUG(SSH_D_MY, ("In CMAC update"));

  /* Number of bytes processed initially with the 'block' buffer. */
  i = 0;

  if (ctx->counter < ctx->cipher_basedef->block_length)
    {
      for (j = ctx->counter;
           j < ctx->cipher_basedef->block_length && i < len;
           i++, j++)
        block[j] = buf[i];

      ctx->counter = j;

      /* Not enough input bytes to form a full block, just return and
         wait for more input. */
      if (ctx->counter != ctx->cipher_basedef->block_length)
        return;

      /* If no more input bytes, return. */
      if (len - i == 0)
        return;
    }
  else
    {
      /* If no more input bytes, return. */
      if (!len)
        return;
    }

   /* mac the single block 'block' */
  status = (*ctx->cipher_basedef->cbcmac)(ctx->context,
					  block,
					  ctx->cipher_basedef->block_length,
					  iv);

  if (status != SSH_CRYPTO_OK)
    ctx->status = status;

  /* Clean block */
  memset(block, 0, ctx->cipher_basedef->block_length);

  /* Reset the counter */
  j = (len - i) % ctx->cipher_basedef->block_length;

   if (j == 0)
     j = ctx->cipher_basedef->block_length;

   ctx->counter = j;

   if (len - i - j)
     {
       status = (*ctx->cipher_basedef->cbcmac)(ctx->context, buf + i, 
					       len - i - j, iv);
       
       if (status != SSH_CRYPTO_OK)
	 ctx->status = status;
     }

  memcpy(block, buf + (len - j), j);
}

SshCryptoStatus
ssh_cmac_final(void *context, unsigned char *digest)
{
  SshCmacCtx *ctx = context;
  SshCryptoStatus status;
  unsigned char *block, *iv;
  unsigned int i;
  SshXUInt128 A;

  SSH_DEBUG(SSH_D_MY, ("In CMAC final"));

  if (ctx->status != SSH_CRYPTO_OK)
    return ctx->status;
  
  iv = ctx->workarea;
  block = iv + ctx->cipher_basedef->block_length;

  if (ctx->counter < ctx->cipher_basedef->block_length)
    {
      /* the last block is not full or we are mac'ing the empty string,
         so need to pad with "10000..." */
      block[ctx->counter] = 0x80;
      for (i = ctx->counter + 1; i < ctx->cipher_basedef->block_length; i++)
        block[i] = 0;

      SSH_XUINT128_GET(A, block);
      SSH_XUINT128_XOR(A, A, ctx->key2);
      SSH_XUINT128_PUT(A, block);
    }
  else
    {
      SSH_XUINT128_GET(A, block);
      SSH_XUINT128_XOR(A, A, ctx->key1);
      SSH_XUINT128_PUT(A, block);
    }

  status = (*ctx->cipher_basedef->cbcmac)(ctx->context,
					  block,
					  ctx->cipher_basedef->block_length,
					  iv);

  memcpy(digest, iv, ctx->cipher_basedef->block_length);
  return status;
}

void
ssh_cmac_uninit(void *context)
{
  SshCmacCtx *ctx = context;

  if (ctx->cipher_basedef->uninit)
    (*ctx->cipher_basedef->uninit)(ctx->context);
}
