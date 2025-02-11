/**
   
   @copyright
   Copyright (c) 2002 - 2013, Rambus Inc. All rights reserved.
   
   nociph.h
   
   Cipher 'none'.
   
*/


#ifndef NOCIPH_H
#define NOCIPH_H

SshCryptoStatus
ssh_none_cipher(void *context, unsigned char *dest,
		const unsigned char *src, size_t len,
		unsigned char *iv);

#endif /* NOCIPH_H */
