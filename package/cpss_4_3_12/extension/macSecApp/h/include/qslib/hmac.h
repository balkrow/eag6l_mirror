/**
   
   @copyright
   Copyright (c) 2002 - 2013, Rambus Inc. All rights reserved.
   
   hmac.h
   
   Message authentication code routines using the HMAC structure.
   
*/


#ifndef HMAC_H
#define HMAC_H

/* Generic Hmac interface. */

size_t
ssh_hmac_ctxsize(const SshHashDefStruct *hash_def);

SshCryptoStatus
ssh_hmac_init(void *context, const unsigned char *key, size_t keylen,
              const SshHashDefStruct *hash_def);

void ssh_hmac_uninit(void *context);

void ssh_hmac_start(void *context);

void ssh_hmac_update(void *context, const unsigned char *buf,
                     size_t len);

SshCryptoStatus ssh_hmac_final(void *context, unsigned char *digest);

SshCryptoStatus ssh_hmac_256_final(void *context, unsigned char *digest);

SshCryptoStatus ssh_hmac_192_final(void *context, unsigned char *digest);

SshCryptoStatus ssh_hmac_128_final(void *context, unsigned char *digest);

SshCryptoStatus ssh_hmac_96_final(void *context, unsigned char *digest);

SshCryptoStatus ssh_hmac_of_buffer(void *context, const unsigned char *buf,
                        size_t len, unsigned char *digest);

SshCryptoStatus ssh_hmac_256_of_buffer(void *context, const unsigned char *buf,
				       size_t len, unsigned char *digest);

SshCryptoStatus ssh_hmac_192_of_buffer(void *context, const unsigned char *buf,
				       size_t len, unsigned char *digest);

SshCryptoStatus ssh_hmac_128_of_buffer(void *context, const unsigned char *buf,
				       size_t len, unsigned char *digest);

SshCryptoStatus ssh_hmac_96_of_buffer(void *context, const unsigned char *buf,
				      size_t len, unsigned char *digest);

void ssh_hmac_zeroize(void *context);

#endif /* HMAC_H */
