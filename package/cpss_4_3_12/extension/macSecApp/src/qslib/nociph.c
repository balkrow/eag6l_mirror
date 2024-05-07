/**
   
   @copyright
   Copyright (c) 2002 - 2013, Rambus Inc. All rights reserved.
   
   nociph.c
   
   Cipher 'none'.
   
*/


#include "sshincludes.h"
#include "sshcrypt.h"
#include "nociph.h"

SshCryptoStatus ssh_none_cipher(void *context, unsigned char *dest,
				const unsigned char *src, size_t len,
				unsigned char *iv)
{
  if (src != dest)
    memcpy(dest, src, len);
  return SSH_CRYPTO_OK;
}

/* nociph.c */
