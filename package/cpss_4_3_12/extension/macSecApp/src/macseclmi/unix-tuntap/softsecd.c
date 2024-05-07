/**
   @copyright
   Copyright (c) 2020, Rambus Inc. All rights reserved.
*/

#include "macsec_dataplane.h"
#include <unistd.h>

int main(void)
{
  int ok;

  ok = macsec_dataplane_init();

  if (ok)
    {
      pause();
    }

  macsec_dataplane_uninit();
}
