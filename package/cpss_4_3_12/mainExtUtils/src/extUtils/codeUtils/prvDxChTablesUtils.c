/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file prvDxChTablesUtils.c
*
* @brief This module holds following utils.
*        Module is compiled when CODE_UTILS=y is used in make
* e.g.
* make TARGET=sim64 FAMILY=DX CODE_UTILS=y appDemo
*
* 1. printTablesDb
*    Dump content of Tables DB for specific device in to file.
*    Output file name should be defined in TABLES_INFO_OUT_FILE.
*    Type Info file name should be defined in TYPE_INFO_FILE.
*    use case for device #0:
*     cpssInitSystem
*     do shell-execute printTablesDb 0
*
* 2. convertTableDb
*    Create static DBs for Tables info of specific device
*    Output is STDIO
*    Type Info file name should be defined in TYPE_INFO_FILE.
*    use case for device #0:
*     cpssInitSystem
*     do shell-execute convertTableDb 0*
*
********************************************************************************/

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define DEBUG_TABLES
#ifdef DEBUG_TABLES
#include <stdio.h>
#include <stdlib.h>

/* path to TYPE_INFO file */
#define TYPE_INFO_FILE "/users/fileril103/vladimira/vdtemp/git/cpss_4.1/cpss/mainLuaWrapper/data/type_info_file_dx"

/* path to ouput file of Tables Info Static DB */
#define TABLES_INFO_OUT_FILE "/users/fileril103/vladimira/vdtemp/table_info.txt"


/* element to store name and value of enum's item */
typedef struct {
    GT_U32 value;
    char   name[256];
}ENUM_ENTRY_STC;

/* static DB for parsed enum */
#define ENUM_DB_SIZE_CNS 1024

static ENUM_ENTRY_STC  enumDb[ENUM_DB_SIZE_CNS];
static GT_U32          enumDbSize;

static void printEnumDb(void)
{
    GT_U32 ii;

    for (ii = 0; ii < enumDbSize; ii++)
    {
        cpssOsPrintf(" %d  %s\n", enumDb[ii].value, enumDb[ii].name);
    }
}

typedef enum {
    STR_STATE_EMPTY_E,
    STR_STATE_MID_E,
    STR_STATE_HOLD_E,
    STR_STATE_DONE_E
} STR_STATE_ENT;

static GT_STATUS handleEnum(IN FILE * fp, IN fpos_t * pos )
{
    char            buffer[1024];
    int             symbol;
    STR_STATE_ENT   strState;
    int             symbolIdx = 0;
    GT_U32          value = 0;
    GT_U32          continueParsing = 1;

    cpssOsMemSet(enumDb, 0, sizeof(enumDb));
    enumDbSize = 0;

    fsetpos (fp, pos);

    while (fgetc(fp) != '{');

    strState = STR_STATE_EMPTY_E;
    buffer[symbolIdx] = 0;

    while (continueParsing)
    {
        symbol = fgetc(fp);

        if (symbol == '}')
        {
            /* last member */
            continueParsing = 0;
            strState = STR_STATE_DONE_E;
        }
        else if (symbol == '=')
        {
            /* explicit value */
            if (EOF == fscanf(fp,"%d", &value))
            {
                cpssOsPrintf("Bad enum value\n");
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }
        else if (symbol >= 48 )
        {
            /* alpha numeric symbol - part of enum name */
            switch (strState)
            {
                case STR_STATE_EMPTY_E: strState = STR_STATE_MID_E; break;
                case STR_STATE_DONE_E: cpssOsPrintf("Bad state\n"); CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                default: break;
            }
            if (strState == STR_STATE_MID_E)
            {
                buffer[symbolIdx++] = symbol;
            }
        }
        else if (symbol == ',')
        {
            /* end of member */
            switch (strState)
            {
                case STR_STATE_MID_E:
                case STR_STATE_HOLD_E:
                    strState = STR_STATE_DONE_E;
                    break;
                case STR_STATE_DONE_E: cpssOsPrintf("Bad state\n"); CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                default: break;
            }
        }
        else
        {
            /* white spaces, new lines, e.t.c */
            switch (strState)
            {
                case STR_STATE_MID_E: strState = STR_STATE_HOLD_E; break;
                case STR_STATE_DONE_E: cpssOsPrintf("Bad state\n"); CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                default: break;
            }
        }

        if (strState == STR_STATE_DONE_E)
        {
            buffer[symbolIdx] = 0;

            cpssOsStrCpy(enumDb[enumDbSize].name, buffer);
            enumDb[enumDbSize++].value = value++;

            /*cpssOsPrintf(" %d  %s\n", value++, buffer);*/

            symbolIdx = 0;
            strState = STR_STATE_EMPTY_E;
            buffer[symbolIdx] = 0;
        }
    }

    return GT_OK;
}

static GT_STATUS findString(IN FILE * fp, IN GT_CHAR * objType, IN GT_CHAR * objName, OUT fpos_t * pos )
{
    char        buffer[1024];
    GT_U32      len = cpssOsStrlen(objName);
    fpos_t      pos1;
    GT_U32      checkType = 0;
    GT_CHAR * strToken = "typedef";

    rewind(fp);

    while (fscanf(fp,"%s", buffer) != EOF)
    {
        if (cpssOsStrCmp(buffer, strToken) == 0)
        {
            checkType = 1;
            continue;
        }

        if ((checkType == 1) && (cpssOsStrCmp(buffer, objType) == 0))
        {
            checkType = 2;
            fgetpos (fp, pos);
            continue;
        }

        if ((checkType == 2) && (cpssOsStrNCmp(buffer, objName, len) == 0))
        {
            fgetpos (fp, &pos1);
            cpssOsPrintf("Found %s on position %d name position %d\n", objName, pos->__pos, pos1.__pos);
            return GT_OK;
        }
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}

/* function to print enum members */
GT_STATUS debugPrintEnum(GT_CHAR * enumName)
{
    GT_CHAR *filePath = TYPE_INFO_FILE;
    FILE * fp;
    fpos_t pos;

    fp = fopen (filePath, "r");
    if (fp == NULL)
    {
        cpssOsPrintf("cannot open TYPE Info file\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    findString(fp,"enum", enumName, &pos);

    handleEnum(fp, &pos);

    printEnumDb();

    fclose(fp);

    cpssOsPrintf("TYPE Info file Done\n");

    return GT_OK;
}

/* element for format tables conversion DB */
typedef struct {
    char                            *tblDbEnumName;
    GT_U32                           enumRef;
    const PRV_CPSS_ENTRY_FORMAT_TABLE_STC *tblDbPtr;
    GT_U32                           tblDbSize;
    char                            *tblDbName;
} TBL_DB_STC;

/* function returns string for enum value */
static char * getEnumName
(
    IN GT_U32 value,
    IN GT_BOOL useFirst /* some enums use LAST that same as prev members.
                         This parameter define what need to be returned
                         either first name (GT_TRUE) or LAST (GT_FALSE)*/
)
{
    GT_U32 ii;
    static GT_U32  lastGoodIdx = 0xFFFFFFFF;
    static GT_U32  lastGoodValue = 0xFFFFFFFF;
    static char    buffer[2048];

    for (ii = 0; ii < enumDbSize; ii++)
    {
        if (enumDb[ii].value == value)
        {
            if (!useFirst)
            {
                /* check next also for "after _LAST" = "_LAST_" */
                if ((ii < (enumDbSize - 1)) && (enumDb[ii + 1].value == value))
                {
                    lastGoodIdx = ii + 1;
                    lastGoodValue = value;

                    return enumDb[ii + 1].name;
                }
            }
            lastGoodIdx = ii;
            lastGoodValue = value;
            return enumDb[ii].name;
        }
    }

    if (lastGoodIdx != 0xFFFFFFFF)
    {
        buffer[0] = 0;
        sprintf(buffer,"%s + %d", enumDb[lastGoodIdx].name, (value - lastGoodValue));
        return buffer;
    }

    return NULL;
}

/* print Format table */
static void printTblDb(IN TBL_DB_STC *tblDbPtr)
{
    GT_U32 ii;
    char * name;

    name = getEnumName(tblDbPtr->tblDbSize, GT_FALSE);
/*
static PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5TtiDefaultEportTableFieldsFormat[SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS___LAST_VALUE___E] =
{
*/
    cpssOsPrintf("static PRV_CPSS_ENTRY_FORMAT_TABLE_STC %s[%s] =\n{\n", tblDbPtr->tblDbName, name);

    for (ii = 0; ii < tblDbPtr->tblDbSize; ii++)
    {
        name = getEnumName(ii, GT_FALSE);
        cpssOsPrintf("    PRV_CPSS_FIELD_MAC(%4d, %3d, %s)", tblDbPtr->tblDbPtr[ii].startBit, tblDbPtr->tblDbPtr[ii].numOfBits, name);

        if (ii == (tblDbPtr->tblDbSize - 1))
        {
            cpssOsPrintf("\n");
        }
        else
        {
            cpssOsPrintf(",\n");
        }
    }

    cpssOsPrintf("};\n\n");
}

#define TBL_DB_ENTRY_MAC(_enum_name, _dbName) \
   {#_enum_name, (_enum_name)0, _dbName, sizeof(_dbName)/sizeof(_dbName[0]),  #_dbName}

/* convert Fields tables to static const final variant */
GT_STATUS convertFieldsDb(void)
{
    GT_CHAR *filePath = TYPE_INFO_FILE;
    FILE * fp;
    fpos_t pos;
    TBL_DB_STC  tblDb[] = {
#if 0
       TBL_DB_ENTRY_MAC(SIP5_EPCL_ACTION_TABLE_FIELDS_ENT, prvCpssDxChSip6_10EpclActionTableFieldsFormat),
       TBL_DB_ENTRY_MAC(SIP5_L2_MLL_TABLE_FIELDS_ENT, prvCpssDxChSip6_10L2MllTableFieldsFormat),
       TBL_DB_ENTRY_MAC(SIP5_TTI_ACTION_TABLE_FIELDS_ENT, prvCpssDxChSip6_10TtiActionTableFieldsFormat),
       TBL_DB_ENTRY_MAC(SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_ENT, prvCpssDxChSip6_30SmuIrfSngTableFieldsFormat),
#endif
    };
    GT_U32      tblDbSize = sizeof(tblDb)/sizeof(tblDb[0]);
    GT_U32      ii;

    fp = fopen (filePath, "r");
    if (fp == NULL)
    {
        cpssOsPrintf("cannot open TYPE Info file\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    for (ii = 0; ii < tblDbSize; ii++)
    {
        findString(fp, "enum", tblDb[ii].tblDbEnumName, &pos);
        handleEnum(fp, &pos);
        printTblDb(&tblDb[ii]);
    }

    fclose(fp);

    cpssOsPrintf("TYPE Info file Done\n");

    return GT_OK;
}

/* print static DBs to define Tables info for device */
static void printDevTblDb(IN PRV_CPSS_DXCH_TABLES_INFO_STC*  tablesInfoArr,
                          IN GT_U32 size,
                          IN char * dbName,
                          IN char * prefix)
{
    GT_U32 ii;
    char * name;
    GT_U32 dirIdx = 0;
    GT_U32 indirIdx = 0;
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC * directInfoPtr;

    name = getEnumName(size, GT_FALSE);

    /* Direct Info */
    cpssOsPrintf("\n/* Direct tables info */\n");
    cpssOsPrintf("static const PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC %s_directInfoArr[] =\n{\n", dbName);
    for (ii = 0; ii < size; ii++)
    {
        name = getEnumName(ii, GT_TRUE);
        if (tablesInfoArr[ii].maxNumOfEntries == 0)
        {
            continue;
        }
        else if (tablesInfoArr[ii].readAccessType == PRV_CPSS_DXCH_INDIRECT_ACCESS_E)
        {
            continue;
        }
        else
        {
            directInfoPtr = (PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC *)tablesInfoArr[ii].readTablePtr;
            cpssOsPrintf(" PRV_DIRECT_INFO_MAC(0x%08X, %3d, %d, %s),", directInfoPtr->baseAddress, directInfoPtr->step, directInfoPtr->nextWordOffset,name);
        }

        cpssOsPrintf("\n");
    }

    cpssOsPrintf("};\n\n");

    cpssOsPrintf("/* Number of entries in Direct tables info array */\n");
    cpssOsPrintf("static const GT_U32 %s_directInfoArrSize = sizeof(%s_directInfoArr)/sizeof(%s_directInfoArr[0]);\n\n", dbName, dbName, dbName);

    /* Table Info - Direct entries first */
    cpssOsPrintf("/* Tables info array */\n");
    cpssOsPrintf("static const PRV_CPSS_DXCH_TABLES_INFO_EXT_STC %s_tablesInfoArr[] =\n{\n", dbName);

    for (ii = 0; ii < size; ii++)
    {
        name = getEnumName(ii, GT_TRUE);
        if (tablesInfoArr[ii].maxNumOfEntries == 0)
        {
            continue;
        }
        else if (tablesInfoArr[ii].readAccessType == PRV_CPSS_DXCH_INDIRECT_ACCESS_E)
        {
            continue;
        }
        else
        {
            cpssOsPrintf(" %s_DIRECT_MAC(%7d, %3d, %3d, %s)", prefix, tablesInfoArr[ii].maxNumOfEntries, tablesInfoArr[ii].entrySize, dirIdx++, name);
        }

        cpssOsPrintf(",\n");
    }

    /* Table Info - Indirect entries last */
    for (ii = 0; ii < size; ii++)
    {
        name = getEnumName(ii, GT_TRUE);
        if (tablesInfoArr[ii].maxNumOfEntries == 0)
        {
            continue;
        }
        else if (tablesInfoArr[ii].readAccessType == PRV_CPSS_DXCH_INDIRECT_ACCESS_E)
        {
            cpssOsPrintf(" %s_INDIRECT_MAC(%7d, %3d, %d, %s)", prefix, tablesInfoArr[ii].maxNumOfEntries, tablesInfoArr[ii].entrySize, indirIdx++, name);
        }
        else
        {
            continue;
        }

        if (ii == (size - 1))
        {
            cpssOsPrintf("\n");
        }
        else
        {
            cpssOsPrintf(",\n");
        }
    }

    cpssOsPrintf("};\n\n");

    cpssOsPrintf("/* Number of entries in Tables info array */\n");
    cpssOsPrintf("static const GT_U32 %s_tablesInfoArrSize = sizeof(%s_tablesInfoArr)/sizeof(%s_tablesInfoArr[0]);\n\n", dbName, dbName, dbName);
}

/* create static DBs for Tables info of device */
GT_STATUS convertTableDb(GT_U8 devNum)
{
    GT_CHAR *filePath = TYPE_INFO_FILE;
    FILE * fp;
    fpos_t pos;
    PRV_CPSS_DXCH_TABLES_INFO_STC*               tablesInfoArr;
    GT_U32      tblDbSize;
    char *      prefixNames[] =  {"prvCpssDxChDefault", "prvCpssDxChHarrier","prvCpssDxChAc5x","prvCpssDxChAc5p","prvCpssDxChFalcon"};
    char *      familyNames[] =  {"DEFAULT", "HARRIER", "AC5X","AC5P","FALCON"};
    GT_U32      namesIdx;

    tablesInfoArr = PRV_CPSS_ACCESS_TABLE_INFO_PTR(devNum);
    tblDbSize = PRV_CPSS_DXCH_PP_MAC(devNum)->accessTableInfoSize;

    fp = fopen (filePath, "r");
    if (fp == NULL)
    {
        cpssOsPrintf("cannot open TYPE Info file\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    switch (PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            namesIdx = 1;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            namesIdx = 2;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            namesIdx = 3;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            namesIdx = 4;
            break;
        default:
            namesIdx = 0;
            break;
    }

    findString(fp, "enum", "CPSS_DXCH_TABLE_ENT", &pos);
    handleEnum(fp, &pos);
    printDevTblDb(tablesInfoArr, tblDbSize, prefixNames[namesIdx], familyNames[namesIdx]);

    fclose(fp);

    cpssOsPrintf("TYPE Info file Done\n");

    return GT_OK;
}

/* dump Tables info and statistic to file */
GT_STATUS printTablesDb(GT_U8 devNum)
{
    GT_CHAR *outFilePath = TABLES_INFO_OUT_FILE;
    FILE * outFp;
    GT_CHAR *filePath = TYPE_INFO_FILE;
    FILE * fp;
    fpos_t pos;
    PRV_CPSS_DXCH_TABLES_INFO_STC*               tablesInfoArr;
    GT_U32      tblDbSize;
    char * name;
    GT_U32 ii;
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC * directInfoPtr;
    PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC * inDirectInfoPtr;
    GT_U32  directEntriesNum = 0;
    GT_U32  indirectEntriesNum = 0;
    GT_U32  mainTblSize, dirTblSize, indTblSize;

    tablesInfoArr = PRV_CPSS_ACCESS_TABLE_INFO_PTR(devNum);
    tblDbSize = PRV_CPSS_DXCH_PP_MAC(devNum)->accessTableInfoSize;

    fp = fopen (filePath, "r");
    if (fp == NULL)
    {
        cpssOsPrintf("cannot open TYPE Info file\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    outFp = fopen (outFilePath, "w+");
    if (outFp == NULL)
    {
        fclose(fp);
        cpssOsPrintf("cannot open output file\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    findString(fp, "enum", "CPSS_PP_FAMILY_TYPE_ENT", &pos);
    handleEnum(fp, &pos);
    name = getEnumName(PRV_CPSS_PP_MAC(devNum)->devFamily, GT_TRUE);

    fprintf(outFp, "Tables Info for %s, Number of tables %d\n", name, tblDbSize);

    findString(fp, "enum", "CPSS_DXCH_TABLE_ENT", &pos);
    handleEnum(fp, &pos);

    fprintf(outFp, "\nDirect Tables:\n");
    fprintf(outFp, "\n");
    fprintf(outFp, " Table  | Entry | Base       | Step | Next | Idx  | Name\n");
    fprintf(outFp, " Size   | Size  | Address    |      | Word |      |\n");
    fprintf(outFp, "---------------------------------------------------------------------------------------\n");

    for (ii=0; ii < tblDbSize; ii++)
    {
        if (tablesInfoArr[ii].maxNumOfEntries == 0)
        {
            continue;
        }
        name = getEnumName(ii, GT_TRUE);

        if (tablesInfoArr[ii].readTablePtr != tablesInfoArr[ii].writeTablePtr)
        {
            fprintf(outFp, "Error 1: %4d %s\n", ii, name);
            continue;
        }

        if (tablesInfoArr[ii].readAccessType != tablesInfoArr[ii].writeAccessType)
        {
            fprintf(outFp, "Error 2: %4d %s\n", ii, name);
            continue;
        }

        if (tablesInfoArr[ii].readAccessType != PRV_CPSS_DXCH_DIRECT_ACCESS_E)
        {
            continue;
        }

        directEntriesNum++;
        directInfoPtr = (PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC *)tablesInfoArr[ii].readTablePtr;

        fprintf(outFp, " %6d | %5d | 0x%08X | %4d | %4d | %4d | %s\n", tablesInfoArr[ii].maxNumOfEntries, tablesInfoArr[ii].entrySize,
                                            directInfoPtr->baseAddress, directInfoPtr->step, directInfoPtr->nextWordOffset,
                                            ii, name);
    }

    fprintf(outFp, "\nIndirect Tables:\n");
    fprintf(outFp, "\n");
    fprintf(outFp, " Table  | Entry | Control    | Data       | Trg | Idx | Spc | Spc | Act | Idx  | Name\n");
    fprintf(outFp, " Size   | Size  | Reg        | Reg        | Bit | Bit | Val | Bit | Bit |      |\n");
    fprintf(outFp, "-----------------------------------------------------------------------------------------------------------\n");

    for (ii=0; ii < tblDbSize; ii++)
    {
        if (tablesInfoArr[ii].maxNumOfEntries == 0)
        {
            continue;
        }
        name = getEnumName(ii, GT_TRUE);

        if (tablesInfoArr[ii].readTablePtr != tablesInfoArr[ii].writeTablePtr)
        {
            continue;
        }

        if (tablesInfoArr[ii].readAccessType != tablesInfoArr[ii].writeAccessType)
        {
            continue;
        }

        if (tablesInfoArr[ii].readAccessType != PRV_CPSS_DXCH_INDIRECT_ACCESS_E)
        {
            continue;
        }

        inDirectInfoPtr = (PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC *)tablesInfoArr[ii].readTablePtr;
        indirectEntriesNum++;

        fprintf(outFp, " %6d | %5d | 0x%08X | 0x%08X | %3d | %3d | %3d | %3d | %3d | %4d | %s\n",
                                            tablesInfoArr[ii].maxNumOfEntries, tablesInfoArr[ii].entrySize,
                                            inDirectInfoPtr->controlReg, inDirectInfoPtr->dataReg,
                                            inDirectInfoPtr->trigBit,inDirectInfoPtr->indexBit,
                                            inDirectInfoPtr->specificTableValue, inDirectInfoPtr->specificTableBit,
                                            inDirectInfoPtr->actionBit,
                                            ii, name);
    }

    fprintf(outFp, "\nInvalid Tables:\n");
    fprintf(outFp, "\n");
    fprintf(outFp, " Idx  | Name\n");
    fprintf(outFp, "      |\n");
    fprintf(outFp, "-------------------------------------------------\n");

    for (ii=0; ii < tblDbSize; ii++)
    {
        /* must be before skip to convert implicit enums */
        name = getEnumName(ii, GT_TRUE);

        if (tablesInfoArr[ii].maxNumOfEntries != 0)
        {
            continue;
        }

        fprintf(outFp, " %4d | %s\n", ii, name);
    }

    /* statistic */
    mainTblSize = tblDbSize * sizeof(PRV_CPSS_DXCH_TABLES_INFO_STC);
    dirTblSize  = directEntriesNum * sizeof(PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC);
    indTblSize  = indirectEntriesNum * sizeof(PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC);

    fprintf(outFp,"\nTables Info Entries amount: %5d, size %7d bytes\n", tblDbSize, mainTblSize);
    fprintf(outFp,"Direct Entries amount     : %5d, size %7d bytes\n", directEntriesNum, dirTblSize);
    fprintf(outFp,"Indirect Entries amount   : %5d, size %7d bytes\n", indirectEntriesNum, indTblSize);
    fprintf(outFp,"Overall memory consumpton %d bytes\n", mainTblSize+dirTblSize+indTblSize);

    fclose(fp);
    fclose(outFp);

    cpssOsPrintf("Tables Info file Done\n");

    return GT_OK;
}

#endif



