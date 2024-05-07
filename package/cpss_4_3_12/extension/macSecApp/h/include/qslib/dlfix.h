/**
   
   @copyright
   Copyright (c) 2002 - 2013, Rambus Inc. All rights reserved.
   
   dlfix.h
   
   Discrete logarithm predefined groups.
   
*/


#ifndef DLFIX_H
#define DLFIX_H

/* Search a parameter set of name "name". Returns TRUE if found. */
Boolean ssh_dlp_set_param(const char *name, const char **outname,
                          SshMPInteger p, SshMPInteger q, SshMPInteger g);


Boolean ssh_dlp_is_predefined_group(SshMPInteger p, SshMPInteger q,
                                    SshMPInteger g);

#endif /* DLFIX_H */
