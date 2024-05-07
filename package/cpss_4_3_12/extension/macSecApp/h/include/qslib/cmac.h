/**
   
   @copyright
   Copyright (c) 2007 - 2013, Rambus Inc. All rights reserved.
   
   File: cmac.h
   
*/


#ifndef SSH_CMAC_H
#define SSH_CMAC_H


size_t ssh_cmac_ctxsize(const SshCipherMacBaseDefStruct *cipher_def);

SshCryptoStatus
ssh_cmac_init(void *context, const unsigned char *key, size_t keylen,
                 const SshCipherMacBaseDefStruct *cipher_def);

void ssh_cmac_start(void *context);

void ssh_cmac_update(void *context, const unsigned char *buf,
                        size_t len);

SshCryptoStatus ssh_cmac_final(void *context, unsigned char *digest);

SshCryptoStatus ssh_cmac_96_final(void *context, unsigned char *digest);

void ssh_cmac_uninit(void *context);

#endif /* SSH_CMAC_H */

