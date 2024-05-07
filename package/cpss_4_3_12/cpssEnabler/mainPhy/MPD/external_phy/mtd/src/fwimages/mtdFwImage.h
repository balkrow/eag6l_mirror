/*******************************************************************************
Copyright (C) 2014 - 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/
#ifndef MTDFWIMAGE_H
#define MTDFWIMAGE_H
#if C_LINKAGE
#if defined __cplusplus
    extern "C" {
#endif
#endif

#if MTD_INCLUDE_SLAVE_IMAGE
/* Created from x3240flashdlslave_0_6_6_5_11945.hdr */
#define MTD_SIZEOF_SLAVE_IMAGE (11080)
extern MTD_U8 MTD_slave_image[MTD_SIZEOF_SLAVE_IMAGE];

#endif

#if C_LINKAGE
#if defined __cplusplus
}
#endif 
#endif

#endif /* defined MTDFWIMAGE_H */
