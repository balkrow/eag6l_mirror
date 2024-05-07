/**

   @copyright
   Copyright (c) 2012 - 2013, Rambus Inc. All rights reserved.

   File: softsec_platform.h

   @description
   Platform-specific defintitions for the software MACsec protocol for
   software cryptography based on the QuickSec library.

*/


#ifndef _SOFTSEC_PLATFORM_H
#define _SOFTSEC_PLATFORM_H

typedef struct {
  unsigned char encrctx[420];
  unsigned char decrctx[420];
  unsigned char ivctr[16];
  enum {
    SOFTSEC_CRYPTO_CONTEXT_ENCR,
    SOFTSEC_CRYPTO_CONTEXT_DECR
  } direction;
} SoftsecCryptoContext;

#define SOFTSEC_SOFTWARE_CRYPTO

#endif /* _SOFTSEC_PLATFORM_H */
