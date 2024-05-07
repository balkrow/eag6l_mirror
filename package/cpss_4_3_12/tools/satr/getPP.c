/*******************************************************************************
*                Copyright 2016, MARVELL SEMICONDUCTOR, LTD.                   *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL.                      *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
*                                                                              *
* MARVELL COMPRISES MARVELL TECHNOLOGY GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, *
* MARVELL INTERNATIONAL LTD. (MIL), MARVELL TECHNOLOGY, INC. (MTI), MARVELL    *
* SEMICONDUCTOR, INC. (MSI), MARVELL ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K.  *
* (MJKK), MARVELL ISRAEL LTD. (MSIL).                                          *
********************************************************************************
*/
/**
********************************************************************************
* @file getPP.c
*
* @brief TODO
*
* @version   1
********************************************************************************
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "pps/sar_sw_lib.h"

#undef DBG
#ifdef DBG
#define debug(M, ...) printf(M "\n", ##__VA_ARGS__)
#else
#define debug(M, ...)
#endif
#define MG0_ADDR 0x7f900000


extern struct satr_info aldrin_satr_info[];
extern struct satr_info aldrin2_satr_info[];
extern struct satr_info bc3_satr_info[];
extern struct satr_info pipe_satr_info[];
extern struct satr_info ac5_satr_info[];

struct prestera_config {
    const char *name;
    unsigned    devId;
    unsigned    devIdMask;
    struct satr_info *pSatr_info;

} prestera_sysmap[] = {
{ "Bobcat2",   0xfc00, 0xff00, NULL },
{ "Alleycat3", 0xf400, 0xff00, NULL },
{ "Cetus",     0xbe00, 0xff00, NULL },
{ "Caelum",    0xbc00, 0xff00, NULL },
{ "Aldrin",    0xc800, 0xff00, aldrin_satr_info},
{ "Bobcat3",   0xd400, 0xff00, bc3_satr_info },
{ "Pipe"   ,   0xc400, 0xff00, pipe_satr_info },/* CPSS_98PX1012_CNS & 0xFF00 */
{ "Aldrin2",   0xcc00, 0xff00, aldrin2_satr_info },
{ "Alleycat5", 0xb400, 0xff00, ac5_satr_info },

{ NULL,0,0,NULL }
};


struct satr_info* getPP(const char** boardName)
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    unsigned tmp, ppId;
    struct prestera_config *pPrestera;
    int fd=-1, BoardId = 0;
    void *pp_space_ptr2;

    fp = fopen("/proc/bus/pci/devices", "r");
    if (fp == NULL)
        return NULL;

    while (getline(&line, &len, fp) != -1) {
        debug("%s", line);
        if ( (sscanf(line, "%x %x", &tmp, &ppId) == 2) && ((ppId >> 16) == 0x11AB) ) {
            debug("PP Id: %x", ppId);
            for (pPrestera=prestera_sysmap; pPrestera->name; pPrestera++)
                if (pPrestera->devId == (ppId & pPrestera->devIdMask) ) {
                    if (boardName)
                        *boardName = pPrestera->name;
                    return pPrestera->pSatr_info;
                }
        }
    }
    /************************* Alleycat5-Internal checkup **************************/
    if ((fd = open("/dev/mem", O_RDONLY)) < 0){
        printf("open ('/dev/mem', O_RDONLY) failed.\n");
        /*Returning NULL to indicate getPP failed*/
        return NULL;
    }
    pp_space_ptr2 = mmap(NULL,0x1000, PROT_READ | PROT_WRITE , MAP_PRIVATE , fd, (unsigned int)MG0_ADDR);
    if (pp_space_ptr2 == MAP_FAILED) {
        printf("Could not mmap %x\n", (uintptr_t)MG0_ADDR);
        /*Returning NULL to indicate getPP failed*/
        return NULL;
    }
    BoardId =  *(uint32_t *)(pp_space_ptr2 + 0x4c);
    if ((BoardId & 0x000FF000) == 0x000b4000){
        printf("AC5-Internal Detected!.\n");
        close(fd);
        *boardName = "Alleycat5";
        return ac5_satr_info;
    }
        close(fd);
        return NULL;
    }



