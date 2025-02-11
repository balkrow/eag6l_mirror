/**
   
   @copyright
   Copyright (c) 2002 - 2013, Rambus Inc. All rights reserved.
   
   ssheap_config.c
   
*/


#ifndef SSH_EAP_CONFIG_H

#define SSH_EAP_CONFIG_H 1

SshEapProtocolImpl
ssh_eap_config_get_impl_by_type(SshUInt8 type);

SshEapProtocolImpl
ssh_eap_config_get_impl_by_idx(int idx);

int
ssh_eap_config_num_of_impl(void);

#endif
