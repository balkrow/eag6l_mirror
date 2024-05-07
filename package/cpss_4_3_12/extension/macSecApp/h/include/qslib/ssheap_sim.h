/**
   
   @copyright
   Copyright (c) 2002 - 2013, Rambus Inc. All rights reserved.
   
   ssheap_sim.h
   
*/


#ifndef SSH_EAP_SIM_H
#define SSH_EAP_SIM_H 1

/* Common client and server functionality */
void *ssh_eap_sim_create(SshEapProtocol, SshEap eap, SshUInt8);
void ssh_eap_sim_destroy(SshEapProtocol, SshUInt8, void*);
SshEapOpStatus ssh_eap_sim_signal(SshEapProtocolSignalEnum, 
				  SshEap, SshEapProtocol, SshBuffer);
SshEapOpStatus
ssh_eap_sim_key(SshEapProtocol protocol, 
                SshEap eap, SshUInt8 type);
























































































#endif /** SSH_EAP_SIM_H */
