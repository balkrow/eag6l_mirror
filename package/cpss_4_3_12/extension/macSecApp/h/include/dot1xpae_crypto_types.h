/**

   @copyright
   Copyright (c) 2012 - 2013, Rambus Inc. All rights reserved.

   File: dot1x_crypto_types.h

   @description
   Type definitions for 802.1X crypto functions.

*/


#ifndef _DOT1X_CRYPTO_TYPES_H
#define _DOT1X_CRYPTO_TYPES_H

typedef struct {
  void *random;
} Dot1xCryptoRandom;

typedef struct {
  void *mac;
} Dot1xCryptoCmac;

#endif /* _DOT1X_CRYPTO_TYPES_H */
