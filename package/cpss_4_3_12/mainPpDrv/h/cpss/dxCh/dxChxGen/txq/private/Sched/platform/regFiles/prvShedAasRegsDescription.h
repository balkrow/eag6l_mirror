#ifndef _PDQ_AAS_REGISTERS_DESCRIPTION_H_
#define _PDQ_AAS_REGISTERS_DESCRIPTION_H_

#define FOO(...) __VA_ARGS__
#define foo(x) x


#define     PDQ_SIP_7_Port_Pizza_Last_Slice(operation, ...) \
    TM_REG_ROW_##operation( pzlast_slice, 0, 9 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_Identity_Addr(operation, ...) \
    TM_REG_ROW_##operation( SUID, 8, 8 , 0x10, __VA_ARGS__)\
    TM_REG_ROW_##operation( UID, 0, 8 , 0x33, __VA_ARGS__)\

#define     PDQ_SIP_7_ScrubSlots_Addr(operation, ...) \
    TM_REG_ROW_##operation( PortSlots, 32, 6 , 0x4, __VA_ARGS__)\
    TM_REG_ROW_##operation( ClvlSlots, 24, 6 , 0x4, __VA_ARGS__)\
    TM_REG_ROW_##operation( BlvlSlots, 16, 6 , 0x8, __VA_ARGS__)\
    TM_REG_ROW_##operation( AlvlSlots, 8, 6 , 0x10, __VA_ARGS__)\
    TM_REG_ROW_##operation( QueueSlots, 0, 6 , 0x20, __VA_ARGS__)\

#define     PDQ_SIP_7_TreeDeqEn_Addr(operation, ...) \
    TM_REG_ROW_##operation( En, 0, 1 , 0x1, __VA_ARGS__)\

#define     PDQ_SIP_7_PDWRREnReg(operation, ...) \
    TM_REG_ROW_##operation( PrioEn, 0, 1 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_PPerCtlConf_Addr(operation, ...) \
    TM_REG_ROW_##operation( DecEn, 48, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerInterval, 16, 8 , 0xa0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerEn, 0, 1 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_PPerRateShap_Addr(operation, ...) \
    TM_REG_ROW_##operation( L, 32, 14 , 0x16e, __VA_ARGS__)\
    TM_REG_ROW_##operation( K, 16, 14 , 0x66f, __VA_ARGS__)\
    TM_REG_ROW_##operation( N, 0, 14 , 0x2, __VA_ARGS__)\

#define     PDQ_SIP_7_CPerCtlConf_Addr(operation, ...) \
    TM_REG_ROW_##operation( DecEn, 48, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerInterval, 16, 12 , 0xe39, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerEn, 0, 1 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_CPerRateShap_Addr(operation, ...) \
    TM_REG_ROW_##operation( L, 32, 14 , 0x16e, __VA_ARGS__)\
    TM_REG_ROW_##operation( K, 16, 14 , 0x66f, __VA_ARGS__)\
    TM_REG_ROW_##operation( N, 0, 14 , 0x2, __VA_ARGS__)\

#define     PDQ_SIP_7_BPerCtlConf_Addr(operation, ...) \
    TM_REG_ROW_##operation( DecEn, 48, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerInterval, 16, 12 , 0xe39, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerEn, 0, 1 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_BPerRateShap_Addr(operation, ...) \
    TM_REG_ROW_##operation( L, 32, 14 , 0x16e, __VA_ARGS__)\
    TM_REG_ROW_##operation( K, 16, 14 , 0x66f, __VA_ARGS__)\
    TM_REG_ROW_##operation( N, 0, 14 , 0x2, __VA_ARGS__)\

#define     PDQ_SIP_7_APerCtlConf_Addr(operation, ...) \
    TM_REG_ROW_##operation( DecEn, 48, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerInterval, 16, 12 , 0xe39, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerEn, 0, 1 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_APerRateShap_Addr(operation, ...) \
    TM_REG_ROW_##operation( L, 32, 14 , 0x16e, __VA_ARGS__)\
    TM_REG_ROW_##operation( K, 16, 14 , 0x66f, __VA_ARGS__)\
    TM_REG_ROW_##operation( N, 0, 14 , 0x2, __VA_ARGS__)\

#define     PDQ_SIP_7_QPerCtlConf_Addr(operation, ...) \
    TM_REG_ROW_##operation( DecEn, 48, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerInterval, 16, 12 , 0xe39, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerEn, 0, 1 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_QPerRateShap_Addr(operation, ...) \
    TM_REG_ROW_##operation( L, 32, 14 , 0x16e, __VA_ARGS__)\
    TM_REG_ROW_##operation( K, 16, 14 , 0x66f, __VA_ARGS__)\
    TM_REG_ROW_##operation( N, 0, 14 , 0x2, __VA_ARGS__)\

#define     PDQ_SIP_7_Global_Cfg(operation, ...) \
    TM_REG_ROW_##operation( scrube_dis, 0, 1 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_PPerRateShapInt_Addr(operation, ...) \
    TM_REG_ROW_##operation( I, 32, 14 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( B, 0, 14 , 0x1, __VA_ARGS__)\

#define     PDQ_SIP_7_CPerRateShapInt_Addr(operation, ...) \
    TM_REG_ROW_##operation( I, 32, 14 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( B, 0, 14 , 0x1, __VA_ARGS__)\

#define     PDQ_SIP_7_BPerRateShapInt_Addr(operation, ...) \
    TM_REG_ROW_##operation( I, 32, 14 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( B, 0, 14 , 0x1, __VA_ARGS__)\

#define     PDQ_SIP_7_APerRateShapInt_Addr(operation, ...) \
    TM_REG_ROW_##operation( I, 32, 14 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( B, 0, 14 , 0x1, __VA_ARGS__)\

#define     PDQ_SIP_7_QPerRateShapInt_Addr(operation, ...) \
    TM_REG_ROW_##operation( I, 32, 14 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( B, 0, 14 , 0x1, __VA_ARGS__)\

#define     PDQ_SIP_7_ErrorStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( FIFOOvrflowErr, 5, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( TBNegSat, 4, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( BPBSat, 3, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncECCErr, 2, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( CorrECCErr, 1, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( ForcedErr, 0, 1 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_FirstExcp_Addr(operation, ...) \
    TM_REG_ROW_##operation( FIFOOvrflowErr, 5, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( TBNegSat, 4, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( BPBSat, 3, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncECCErr, 2, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( CorrECCErr, 1, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( ForcedErr, 0, 1 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_ErrCnt_Addr(operation, ...) \
    TM_REG_ROW_##operation( Cnt, 0, 16 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_ExcpCnt_Addr(operation, ...) \
    TM_REG_ROW_##operation( Cnt, 0, 16 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_ExcpMask_Addr(operation, ...) \
    TM_REG_ROW_##operation( ECOReg, 32, 32 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( FIFOOvrflowErr, 5, 1 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( TBNegSat, 4, 1 , 0x3, __VA_ARGS__)\
    TM_REG_ROW_##operation( BPBSat, 3, 1 , 0x2, __VA_ARGS__)\
    TM_REG_ROW_##operation( UncECCErr, 2, 1 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( CorrECCErr, 1, 1 , 0x3, __VA_ARGS__)\
    TM_REG_ROW_##operation( ForcedErr, 0, 1 , 0x3, __VA_ARGS__)\

#define     PDQ_SIP_7_Debug_Read_index(operation, ...) \
    TM_REG_ROW_##operation( read_cebug_index, 0, 16 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_Plast_Addr(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 7 , 0x7f, __VA_ARGS__)\

#define     PDQ_SIP_7_PPerStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( PerPointer, 32, 7 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerRoundCnt, 0, 8 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_CPerStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( PerPointer, 32, 11 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerRoundCnt, 0, 8 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_BPerStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( PerPointer, 32, 11 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerRoundCnt, 0, 8 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_APerStatus_Addr(operation, ...) \
    TM_REG_ROW_##operation( PerPointer, 32, 11 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerRoundCnt, 0, 8 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_PortEligPrioFunc_Entry(operation, ...) \
    TM_REG_ROW_##operation( FuncOut3, 48, 6 , 0x20, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut2, 32, 6 , 0x20, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut1, 16, 6 , 0x20, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut0, 0, 6 , 0x20, __VA_ARGS__)\

#define     PDQ_SIP_7_ClvlEligPrioFunc_Entry(operation, ...) \
    TM_REG_ROW_##operation( FuncOut3, 48, 9 , 0x100, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut2, 32, 9 , 0x100, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut1, 16, 9 , 0x100, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut0, 0, 9 , 0x100, __VA_ARGS__)\

#define     PDQ_SIP_7_BlvlEligPrioFunc_Entry(operation, ...) \
    TM_REG_ROW_##operation( FuncOut3, 48, 9 , 0x100, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut2, 32, 9 , 0x100, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut1, 16, 9 , 0x100, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut0, 0, 9 , 0x100, __VA_ARGS__)\

#define     PDQ_SIP_7_AlvlEligPrioFunc_Entry(operation, ...) \
    TM_REG_ROW_##operation( FuncOut3, 48, 9 , 0x100, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut2, 32, 9 , 0x100, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut1, 16, 9 , 0x100, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut0, 0, 9 , 0x100, __VA_ARGS__)\

#define     PDQ_SIP_7_QueueEligPrioFunc(operation, ...) \
    TM_REG_ROW_##operation( FuncOut3, 48, 9 , 0x100, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut2, 32, 9 , 0x100, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut1, 16, 9 , 0x100, __VA_ARGS__)\
    TM_REG_ROW_##operation( FuncOut0, 0, 9 , 0x100, __VA_ARGS__)\

#define     PDQ_SIP_7_PortQuantumsPriosLo(operation, ...) \
    TM_REG_ROW_##operation( Deficit, 12, 12 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( Quantum, 0, 12 , 0x1010, __VA_ARGS__)\
/*manual*/
#define		PDQ_SIP_7_Port_Pizza_Slice(operation, ...) \
    TM_REG_ROW_##operation( slice3_port_id, 21, 7 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( slice2_port_id, 14, 7 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( slice1_port_id, 7, 7 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( slice0_port_id, 0, 7 , 0x10, __VA_ARGS__)\

#define     PDQ_SIP_7_PortRangeMap(operation, ...) \
    TM_REG_ROW_##operation( pdwrr_en, 30, 8 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( pfunc_ptr, 24, 6 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( chigh, 12, 12 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( clow, 0, 12 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_PlvlTokenBucketMapping(operation, ...) \
    TM_REG_ROW_##operation( MaxBurstSz, 50, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinBurstSz, 38, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxDivExp, 35, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinDivExp, 32, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxTokenRes, 29, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTokenRes, 26, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxToken, 13, 13 , 0x1fff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinToken, 0, 13 , 0x1fff, __VA_ARGS__)\

#define     PDQ_SIP_7_Queuemapping(operation, ...) \
    TM_REG_ROW_##operation( alvl, 6, 15 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( Ptr, 0, 6 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_QueueTokenBucketTokenEnDiv(operation, ...) \
    TM_REG_ROW_##operation( AbvEn, 62, 1 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxBurstSz, 50, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinBurstSz, 38, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxDivExp, 35, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinDivExp, 32, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxTokenRes, 29, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTokenRes, 26, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxToken, 13, 13 , 0x1fff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinToken, 0, 13 , 0x1fff, __VA_ARGS__)\

#define     PDQ_SIP_7_QueueQuantum(operation, ...) \
    TM_REG_ROW_##operation( Deficit, 12, 12 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( Quantum, 0, 12 , 0x40, __VA_ARGS__)\

#define     PDQ_SIP_7_AlvlTokenBucketMapping(operation, ...) \
    TM_REG_ROW_##operation( MaxBurstSz, 50, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinBurstSz, 38, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxDivExp, 35, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinDivExp, 32, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxTokenRes, 29, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTokenRes, 26, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxToken, 13, 13 , 0x1fff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinToken, 0, 13 , 0x1fff, __VA_ARGS__)\

#define     PDQ_SIP_7_AlvlQuantum(operation, ...) \
    TM_REG_ROW_##operation( Deficit, 12, 12 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( Quantum, 0, 12 , 0x40, __VA_ARGS__)\

#define     PDQ_SIP_7_ALvltoBlvlAndQueueRangeMap(operation, ...) \
    TM_REG_ROW_##operation( Blvl, 46, 14 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( adwrr_en, 38, 8 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( afunc_ptr, 32, 6 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( qhigh, 16, 16 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( qlow, 0, 16 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_BlvlTokenBucketMapping(operation, ...) \
    TM_REG_ROW_##operation( MaxBurstSz, 50, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinBurstSz, 38, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxDivExp, 35, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinDivExp, 32, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxTokenRes, 29, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTokenRes, 26, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxToken, 13, 13 , 0x1fff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinToken, 0, 13 , 0x1fff, __VA_ARGS__)\

#define     PDQ_SIP_7_BlvlQuantum(operation, ...) \
    TM_REG_ROW_##operation( Deficit, 12, 12 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( Quantum, 0, 12 , 0x40, __VA_ARGS__)\

#define     PDQ_SIP_7_BLvltoClvlAndAlvlRangeMap(operation, ...) \
    TM_REG_ROW_##operation( clvl, 44, 13 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( bdwrr_en, 36, 8 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( bfunc_ptr, 30, 6 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( ahigh, 15, 15 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( alow, 0, 15 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_ClvlTokenBucketMapping(operation, ...) \
    TM_REG_ROW_##operation( MaxBurstSz, 50, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinBurstSz, 38, 12 , 0xfff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxDivExp, 35, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinDivExp, 32, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxTokenRes, 29, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTokenRes, 26, 3 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( MaxToken, 13, 13 , 0x1fff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinToken, 0, 13 , 0x1fff, __VA_ARGS__)\

#define     PDQ_SIP_7_ClvlQuantum(operation, ...) \
    TM_REG_ROW_##operation( Deficit, 12, 12 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( Quantum, 0, 12 , 0x40, __VA_ARGS__)\

#define     PDQ_SIP_7_ClvltoPortAndBlvlRangeMap(operation, ...) \
    TM_REG_ROW_##operation( port, 42, 7 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( cdwrr_en, 34, 8 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( cfunc_ptr, 28, 6 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( bhigh, 14, 14 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( blow, 0, 14 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_PortShpBucketLvls(operation, ...) \
    TM_REG_ROW_##operation( MaxLvl, 32, 23 , 0x3fffff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinLvl, 0, 23 , 0x3fffff, __VA_ARGS__)\

#define     PDQ_SIP_7_PortShaperBucketNeg(operation, ...) \
    TM_REG_ROW_##operation( MaxNeg, 32, 32 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTBNeg, 0, 32 , 0x1, __VA_ARGS__)\

#define     PDQ_SIP_7_ClvlShpBucketLvls(operation, ...) \
    TM_REG_ROW_##operation( MaxLvl, 32, 23 , 0x3fffff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinLvl, 0, 23 , 0x3fffff, __VA_ARGS__)\

#define     PDQ_SIP_7_CLevelShaperBucketNeg(operation, ...) \
    TM_REG_ROW_##operation( MaxTBNeg, 32, 32 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTBNeg, 0, 32 , 0x1, __VA_ARGS__)\

#define     PDQ_SIP_7_BlvlShpBucketLvls(operation, ...) \
    TM_REG_ROW_##operation( MaxLvl, 32, 23 , 0x3fffff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinLvl, 0, 23 , 0x3fffff, __VA_ARGS__)\

#define     PDQ_SIP_7_BLevelShaperBucketNeg(operation, ...) \
    TM_REG_ROW_##operation( MaxTBNeg, 32, 32 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTBNeg, 0, 32 , 0x1, __VA_ARGS__)\

#define     PDQ_SIP_7_AlvlShpBucketLvls(operation, ...) \
    TM_REG_ROW_##operation( MaxLvl, 32, 23 , 0x3fffff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinLvl, 0, 23 , 0x3fffff, __VA_ARGS__)\

#define     PDQ_SIP_7_ALevelShaperBucketNeg(operation, ...) \
    TM_REG_ROW_##operation( MaxTBNeg, 32, 32 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTBNeg, 0, 32 , 0x1, __VA_ARGS__)\

#define     PDQ_SIP_7_QueueShpBucketLvls(operation, ...) \
    TM_REG_ROW_##operation( MaxLvl, 32, 23 , 0x3fffff, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinLvl, 0, 23 , 0x3fffff, __VA_ARGS__)\

#define     PDQ_SIP_7_QueueShaperBucketNeg(operation, ...) \
    TM_REG_ROW_##operation( MaxTBNeg, 32, 32 , 0x1, __VA_ARGS__)\
    TM_REG_ROW_##operation( MinTBNeg, 0, 32 , 0x1, __VA_ARGS__)\

#define     PDQ_SIP_7_PortNodeState(operation, ...) \
    TM_REG_ROW_##operation( State, 0, 11 , 0x2, __VA_ARGS__)\

#define     PDQ_SIP_7_PortMyQ(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 40 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_PortRRDWRRStatus01(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 16 , 0xffff, __VA_ARGS__)\

#define     PDQ_SIP_7_PortRRDWRRStatus23(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 16 , 0xffff, __VA_ARGS__)\

#define     PDQ_SIP_7_PortRRDWRRStatus45(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 16 , 0xffff, __VA_ARGS__)\

#define     PDQ_SIP_7_PortRRDWRRStatus67(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 16 , 0xffff, __VA_ARGS__)\

#define     PDQ_SIP_7_PortWFS(operation, ...) \
    TM_REG_ROW_##operation( WFS, 0, 32 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_PortBPFromQMgr(operation, ...) \
    TM_REG_ROW_##operation( BP, 0, 64 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_QueueNodeState(operation, ...) \
    TM_REG_ROW_##operation( State, 0, 11 , 0x2, __VA_ARGS__)\

#define     PDQ_SIP_7_QueueWFS(operation, ...) \
    TM_REG_ROW_##operation( WFS, 0, 32 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_QueueL0ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_QueueL0ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( status, 0, 64 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_QueueL1ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_QueueL1ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_QueueL2ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_QueueL2ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_QueuePerStatus(operation, ...) \
    TM_REG_ROW_##operation( PerPointer, 32, 11 , 0x0, __VA_ARGS__)\
    TM_REG_ROW_##operation( PerRoundCnt, 0, 8 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_ClvlNodeState(operation, ...) \
    TM_REG_ROW_##operation( State, 0, 11 , 0x2, __VA_ARGS__)\

#define     PDQ_SIP_7_ClvlMyQ(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 34 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_ClvlRRDWRRStatus01(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 28 , 0xfffffff, __VA_ARGS__)\

#define     PDQ_SIP_7_ClvlRRDWRRStatus23(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 28 , 0xfffffff, __VA_ARGS__)\

#define     PDQ_SIP_7_ClvlRRDWRRStatus45(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 28 , 0xfffffff, __VA_ARGS__)\

#define     PDQ_SIP_7_ClvlRRDWRRStatus67(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 28 , 0xfffffff, __VA_ARGS__)\

#define     PDQ_SIP_7_ClvlWFS(operation, ...) \
    TM_REG_ROW_##operation( WFS, 0, 32 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_ClvlL0ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_ClvlL0ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( status, 0, 64 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_BlvlNodeState(operation, ...) \
    TM_REG_ROW_##operation( State, 0, 11 , 0x2, __VA_ARGS__)\

#define     PDQ_SIP_7_BlvlMyQ(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 28 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_BlvlRRDWRRStatus01(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 30 , 0x3fffffff, __VA_ARGS__)\

#define     PDQ_SIP_7_BlvlRRDWRRStatus23(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 30 , 0x3fffffff, __VA_ARGS__)\

#define     PDQ_SIP_7_BlvlRRDWRRStatus45(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 30 , 0x3fffffff, __VA_ARGS__)\

#define     PDQ_SIP_7_BlvlRRDWRRStatus67(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 30 , 0x3fffffff, __VA_ARGS__)\

#define     PDQ_SIP_7_BlvlWFS(operation, ...) \
    TM_REG_ROW_##operation( WFS, 0, 32 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_BlvlL0ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_BlvlL0ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( status, 0, 64 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_BlvlL1ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_BlvlL1ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_AlvlNodeState(operation, ...) \
    TM_REG_ROW_##operation( State, 0, 11 , 0x2, __VA_ARGS__)\

#define     PDQ_SIP_7_AlvlMyQ(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 22 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_AlvlRRDWRRStatus01(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 32 , 0x1, __VA_ARGS__)\

#define     PDQ_SIP_7_AlvlRRDWRRStatus23(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 32 , 0x1, __VA_ARGS__)\

#define     PDQ_SIP_7_AlvlRRDWRRStatus45(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 32 , 0x1, __VA_ARGS__)\

#define     PDQ_SIP_7_AlvlRRDWRRStatus67(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 32 , 0x1, __VA_ARGS__)\

#define     PDQ_SIP_7_AlvlWFS(operation, ...) \
    TM_REG_ROW_##operation( WFS, 0, 32 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_AlvlL0ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_AlvlL0ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( status, 0, 64 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_AlvlL1ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_AlvlL1ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_AlvlL2ClusterStateLo(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#define     PDQ_SIP_7_AlvlL2ClusterStateHi(operation, ...) \
    TM_REG_ROW_##operation( Status, 0, 64 , 0x0, __VA_ARGS__)\

#endif
