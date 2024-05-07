/**
   
   @copyright
   Copyright (c) 2002 - 2013, Rambus Inc. All rights reserved.
   
   ssheapi.h
   
*/


#ifndef SSH_EAP_CONNECTION_H

#define SSH_EAP_CONNECTION_H 1

#define SSH_EAP_F_DISABLED 1

void 
ssh_eap_connection_output_packet(SshEapConnection con, SshBuffer buf);

void 
ssh_eap_connection_attach(SshEapConnection con, SshEap eap);

void 
ssh_eap_connection_detach(SshEapConnection con);


#endif
