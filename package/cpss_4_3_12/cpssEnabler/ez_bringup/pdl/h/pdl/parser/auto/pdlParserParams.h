/* -------------------------------------------------------------------------------------------------------
   Generated from MTS-Platform-Legacy-Yang-Module based on revision 2023-05-16:3.60 on 04/06/2023 16:40:45
   ------------------------------------------------------------------------------------------------------- */
#ifndef __PDL_AUTO_GENERATED_HPRM_PARSER_HEADER_FILE__
#define __PDL_AUTO_GENERATED_HPRM_PARSER_HEADER_FILE__
    /* find if-feature tag */
    /* HPRM STRUCT DEFINITION */
    typedef union {
        UINT_8              rlHostParamUINT8;
        UINT_16	            rlHostParamUINT16;
        UINT_32	            rlHostParamUINT32;
        BOOLEAN             rlHostParamBOOLEAN;
        MIB_OctetString_STC rlHostParamOctetString;
        RSG_ip_addr_TYP     rlHostParamIpAddress;
        RSG_ipv6_addr_TYP   rlHostParamIpv6Address;
        RSG_ipv6z_addr_TYP  rlHostParamIpv6zAddress;
        RSG_inet_addr_TYP   rlHostParamInetAddress;
        MIB_ObjectId_STC    rlHostParamObjectId;
    } PDL_FEATURE_HPRM_DATA_UNT;
    typedef struct {
        MIB_rlHostParamType_TYP type;
        UINT_32                 data_size;
        void                   *data_PTR;
    } PDL_FEATURE_HPRM_DATA_STC;
#endif /* #ifndef __PDL_AUTO_GENERATED_HPRM_PARSER_HEADER_FILE__ */
