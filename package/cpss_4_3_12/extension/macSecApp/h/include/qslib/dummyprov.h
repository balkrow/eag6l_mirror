/**
   
   @copyright
   Copyright (c) 2002 - 2013, Rambus Inc. All rights reserved.
   
   dummyprov.h 
   
   Example provider file, which can be used as a starting point for new
   providers.
   
   
*/


#ifndef DUMMYPROV_H_INCLUDED
#define DUMMYPROV_H_INCLUDED

#include "extkeyprov.h"

/* the dummy provider array structure, which needs to be included to
   the array of supported providers in sshexternalkey.c */
extern struct SshEkProviderOpsRec ssh_ek_dummy_ops;

#endif /* DUMMYPROV_H_INCLUDED */
