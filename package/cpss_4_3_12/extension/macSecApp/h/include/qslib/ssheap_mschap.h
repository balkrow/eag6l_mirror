/**
   
   @copyright
   Copyright (c) 2002 - 2013, Rambus Inc. All rights reserved.
   
   ssheap_md5.h
   
*/


#ifndef SSH_EAP_MSCHAPV2_H

#define SSH_EAP_MSCHAPV2_H 1




































































void*
ssh_eap_mschap_v2_create(SshEapProtocol protocol,
                         SshEap eap, 
                         SshUInt8 type);
void 
ssh_eap_mschap_v2_destroy(SshEapProtocol protocol, 
                          SshUInt8 type, 
                          void *ctx);
SshEapOpStatus
ssh_eap_mschap_v2_signal(SshEapProtocolSignalEnum sig,
                         SshEap eap,
                         SshEapProtocol protocol,
                         SshBuffer buf);
SshEapOpStatus
ssh_eap_mschap_v2_key(SshEapProtocol protocol,
                      SshEap eap,
                      SshUInt8 type);

#endif /* SSH_EAP_MSCHAPV2_H */
