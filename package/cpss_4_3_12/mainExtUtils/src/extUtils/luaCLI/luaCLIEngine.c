/*******************************************************************************
*              (c), Copyright 2015, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file luaCLIEngine.c
*
* @brief Common function for luaCLIEngine
*
* @version   1
********************************************************************************
*/

/***** Include files ***************************************************/

#include <gtOs/gtOsMem.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <string.h>
#include <stdlib.h>
#include <extUtils/luaCLI/luaCLIEngine.h>
#include <cmdShell/os/cmdExecute.h>
/***** Externs  ********************************************************/

int luaEngineGenWrapper_init(lua_State *L);
int luaEngineFSregisterLoader(lua_State *L);
int luaEngineFSregisterAPI(lua_State *L);


int luaCLIEngineNetRegisterAPI(lua_State *L);

/* global storage for Lua data */
int luaGlobalStore(lua_State *L);
int luaGlobalGet(lua_State *L);

/***** Private declarations ********************************************/

/***** Public Functions ************************************************/

/*************** Global ******************************************************/
typedef GT_STATUS (*FUNC_PTR)();
/*******************************************************************************
* luaCLIEngineGetData
*
* DESCRIPTION:
*       Returns pointer to lightuserdata, print error if not defined
*
* INPUTS:
*       L        - lua state
*       name     - name of data
*       printErr - print error if any
*
* OUTPUTS:
*       None
*
* RETURNS:
*       pointer or NULL
*
* COMMENTS:
*       None
*
*******************************************************************************/
void* luaCLIEngineGetData(lua_State* L, const char *name, int printErr)
{
    void *ptr = NULL;

    lua_getglobal(L, name);
    if (lua_islightuserdata(L, -1))
        ptr = lua_touserdata(L, -1);
    lua_pop(L, 1);
    if (ptr != NULL || printErr == 0)
        return ptr;
    lua_getglobal(L, "print");
    lua_pushstring(L, "Failure: ");
    lua_pushstring(L, name);
    lua_pushstring(L, " is not defined or NULL");
    lua_concat(L, 3);
    lua_call(L, 1, 0);
    return ptr;
}

#ifdef CPSS_BLOB
static int luaCLIEnginePrintStdout(
    lua_State *L
)
{
    int n;
    int i;

    n=lua_gettop(L);
    for (i=1; i<=n; i++)
    {
        if (i>1)
            cpssOsPrintf("\t");
        if (lua_isstring(L,i))
            cpssOsPrintf("%s", lua_tostring(L,i));
        else if (lua_isnil(L,i))
            cpssOsPrintf("nil");
        else if (lua_isboolean(L,i))
            cpssOsPrintf(lua_toboolean(L,i) ? "true" : "false");
        else if (lua_islightuserdata(L,i))
        {
            cpssOsPrintf("<pointer=%p>", lua_touserdata(L, i));
        }
        else
        {
            /* printf("%s:%p",luaL_typename(L,i),lua_topointer(L,i)); */
            cpssOsPrintf("<%s>",luaL_typename(L,i));
        }
    }
    cpssOsPrintf("\n");
    return 0;
}
#endif /* CPSS_BLOB */
/**
* @internal luaCLIEnginePrint function
* @endinternal
*
* @brief   lua print function
*
* @param[in] L                        - Lua state
*                                       0
*/
static int luaCLIEnginePrint(
    lua_State *L
)
{
    IOStreamPTR IOStream;
    int n;
    int i;

    IOStream = (IOStreamPTR)luaCLIEngineGetData(L, "_IOStream", 0);
    if (IOStream == NULL)
#ifndef CPSS_BLOB
        return 0;
#else
        return luaCLIEnginePrintStdout(L);
#endif

    n=lua_gettop(L);
    for (i=1; i<=n; i++)
    {
        if (i>1)
            IOStream->writeLine(IOStream, "\t");
        if (lua_isstring(L,i))
            IOStream->writeLine(IOStream, lua_tostring(L,i));
        else if (lua_isnil(L,i))
            IOStream->writeLine(IOStream, "nil");
        else if (lua_isboolean(L,i))
            IOStream->writeLine(IOStream, lua_toboolean(L,i) ? "true" : "false");
        else if (lua_islightuserdata(L,i))
        {
            char buf[40];
            cpssOsSprintf(buf, "<pointer=%p>", lua_touserdata(L, i));
            IOStream->writeLine(IOStream, buf);
        }
        else
        {
            /* printf("%s:%p",luaL_typename(L,i),lua_topointer(L,i)); */
            IOStream->writeLine(IOStream, "<");
            IOStream->writeLine(IOStream, luaL_typename(L,i));
            IOStream->writeLine(IOStream, ">");
        }
    }
    IOStream->writeLine(IOStream, "\n");
    return 0;
}

/**
* @internal luaCLI_splitline function
* @endinternal
*
* @brief   execute command
*/
static int luaCLI_splitline(lua_State *L)
{
    enum {
        STATE_NONE,  /* skip spaces */
        STATE_PARAM, /* parameter */
        STATE_DQUOTE, /* double quote(parameter) */
        STATE_DQUOTE_BACKSLASH /* backslash in double quotes */
    } state = STATE_NONE;
    const char *line;
    unsigned int p, linelen, buflen = 0;
    char buf[64];
    if (lua_type(L, 1) != LUA_TSTRING)
    {
        return 0;
    }
    lua_settop(L, 1);
    line = lua_tostring(L, 1);
    linelen = (int)lua_objlen(L, 1);
    lua_newtable(L); /* 2 */
    for (p = 0; p < linelen; p++)
    {
        int is_space = 0;
        switch(line[p])
        {
            case 0:
            case ' ':
            case '\t':
            case '\r':
            case '\n':
            case '\v':
                is_space = 1;
                break;

        }
        switch (state)
        {
            case STATE_NONE:
                if (is_space)
                    continue;
                state = STATE_PARAM;
                buflen = 0;
                lua_pushliteral(L, "");
                GT_ATTR_FALLTHROUGH;
            case STATE_PARAM:
                if (is_space)
                {
                    if (buflen > 0)
                    {
                        lua_pushlstring(L, buf, buflen);
                        lua_concat(L, 2);
                    }
                    lua_rawseti(L, 2, (int)lua_objlen(L, 2) + 1);
                    state = STATE_NONE;
                    break;
                }
                if (line[p] == '"')
                {
                    state = STATE_DQUOTE;
                    break;
                }
                buf[buflen++] = line[p];
                break;
            case STATE_DQUOTE:
                switch (line[p])
                {
                    case '"':
                        state = STATE_PARAM;
                        break;
                    case '\\':
                        state = STATE_DQUOTE_BACKSLASH;
                        break;
                    default:
                        buf[buflen++] = line[p];
                }
                break;
            case STATE_DQUOTE_BACKSLASH:
                switch (line[p])
                {
                    case 'r':
                        buf[buflen++] = '\r';
                        break;
                    case 'n':
                        buf[buflen++] = '\n';
                        break;
                    case 't':
                        buf[buflen++] = '\t';
                        break;
                    default:
                        buf[buflen++] = line[p];
                        break;
                }
                state = STATE_DQUOTE;
                break;
        }
        if (buflen >= sizeof(buf))
        {
            lua_pushlstring(L, buf, buflen);
            lua_concat(L, 2);
            buflen = 0;
        }
    }
    if (state != STATE_NONE)
    {
        if (buflen > 0)
        {
            lua_pushlstring(L, buf, buflen);
            lua_concat(L, 2);
        }
        lua_rawseti(L, 2, (int)lua_objlen(L, 2) + 1);
    }
    return 1;
}

/**
* @internal luaCLI_registerCfunction function
* @endinternal
*
* @brief   Add commands
*/
static int luaCLI_registerCfunction(lua_State *L)
{
    lua_CFunction funcPtr;
    LUA_CLI_GETFUNCBYNAME_T  getFunctionByName;

    if (!lua_isstring(L, 1))
    {
        lua_getglobal(L, "print");
        lua_pushstring(L, "luaCLI_registerCfunction() error: "
            "first parameter must be string");
        lua_call(L, 1, 0);
        return 0;
    }
    getFunctionByName = (LUA_CLI_GETFUNCBYNAME_T)
            luaCLIEngineGetData(L, "_getFunctionByName", 1);
    if (getFunctionByName == NULL)
        return 0;
    if (lua_gettop(L) > 1)
    {
        lua_settop(L, 2);
        if (!lua_isstring(L, 2))
        {
            lua_getglobal(L, "print");
            lua_pushstring(L, "luaCLI_registerCfunction() error: "
                "second parameter must be string");
            lua_call(L, 1, 0);
            return 0;
        }
    }
    else
    {
        lua_pushstring(L, "");
    }
    lua_pushvalue(L, 1);
    lua_concat(L, 2);

    funcPtr = (lua_CFunction) getFunctionByName(lua_tostring(L, 1));
    if (!funcPtr)
    {
        lua_getglobal(L, "print");
        lua_pushstring(L, "luaCLI_registerCfunction() error: "
            "symbol not found: ");
        lua_pushvalue(L, 1);
        lua_concat(L, 2);
        lua_call(L, 1, 0);
        return 0;
    }
    lua_register(L, lua_tostring(L, 2), funcPtr);
    /* add function to registry */
    lua_getglobal(L, "cmdLuaCLI_CfunctionsRegistry");
    lua_pushcfunction(L, funcPtr);
    lua_setfield(L, -2, lua_tostring(L, 2));
    return 0;
}

/**
* @internal luaCLI_callCfunction function
* @endinternal
*
* @brief   Call Lua C function without registering
*/
static int luaCLI_callCfunction(lua_State *L)
{
    lua_CFunction funcPtr;
    LUA_CLI_GETFUNCBYNAME_T  getFunctionByName;
    if (!lua_isstring(L, 1))
    {
        lua_getglobal(L, "print");
        lua_pushstring(L, "luaCLI_callCfunction() error: "
            "first parameter must be string");
        lua_call(L, 1, 0);
        return 0;
    }
    getFunctionByName = (LUA_CLI_GETFUNCBYNAME_T)
            luaCLIEngineGetData(L, "_getFunctionByName", 1);
    if (getFunctionByName == NULL)
        return 0;

    funcPtr = (lua_CFunction) getFunctionByName(lua_tostring(L, 1));
    if (!funcPtr)
    {
        lua_getglobal(L, "print");
        lua_pushstring(L, "luaCLI_callCfunction() error: "
            "symbol not found: ");
        lua_pushvalue(L, 1);
        lua_concat(L, 2);
        lua_call(L, 1, 0);
        return 0;
    }
    lua_pushcfunction(L, funcPtr);
    lua_replace(L, 1);
    lua_call(L, lua_gettop(L) - 1, LUA_MULTRET);
    return lua_gettop(L);
}

/**
* @internal luaCLI_require function
* @endinternal
*
* @brief   Builtin command: require
*/
static int luaCLI_require(lua_State *L)
{
    if (lua_istable(L, 1))
    {
        lua_rawgeti(L, 1, 1);
        lua_replace(L, 1);
    }
    if (!lua_isstring(L, 1))
    {
        lua_getglobal(L, "print");
        lua_pushstring(L, "luaCLI_require() error: "
            "parameter must be a string");
        lua_call(L, 1, 0);
        return 0;
    }
    /* load libLuaCliDefs.lua */
    lua_getglobal(L, "require");
    lua_pushvalue(L, 1);
    if (lua_pcall(L, 1, 0, 0) != 0)
    {
        lua_getglobal(L, "print");
        lua_pushstring(L, "Failed to load ");
        lua_pushvalue(L, 1);
        lua_pushstring(L, ": ");
        lua_pushvalue(L, -5);
        lua_concat(L, 4);
        lua_call(L, 1, 0);
    }
    return 0;
}

/*******************************************************************************
* luaCLIEngine_alloc_wrapper
*
* DESCRIPTION:
*       The type of the memory-allocation function used by Lua states.
*       This allocator function provides a functionality similar to realloc,
*       but not exactly the same
*
* INPUTS:
*       ud      - user defined pointer, ignored
*       ptr     - a pointer to the block being allocated/reallocated/freed
*       osize   - the original size of the block
*       nsize   - the new size of the block
*
* OUTPUTS:
*       None
*
* RETURNS:
*       pointer to allocated/reallocated block
*
* COMMENTS:
*       (From Lua documentation)
*       ptr is NULL if and only if osize is zero.
*       When nsize is zero, the allocator must return NULL;
*       if osize is not zero, it should free the block pointed to by ptr.
*       When nsize is not zero, the allocator returns NULL if and only if
*         it cannot fill the request.
*       When nsize is not zero and osize is zero, the allocator should
*         behave like malloc.
*       When nsize and osize are not zero, the allocator behaves like realloc.
*       Lua assumes that the allocator never fails when osize >= nsize
*
*******************************************************************************/
static void* luaCLIEngine_alloc_wrapper
(
    LUA_CLI_MALLOC_STC  *memMgr,
    void                *ptr,
    size_t              osize,
    size_t              nsize
)
{
    void    *nptr;
    if (nsize == 0)
    {
        if (ptr != NULL)
        {
            memMgr->free(ptr);
#ifdef LUACLI_MEMSTAT
            memMgr->totalUsed -= (int)osize;
            memMgr->nChunks--;
#endif /* LUACLI_MEMSTAT */
        }
        return NULL;
    }

    nptr = memMgr->malloc((int)nsize);
    if (nptr == NULL && osize >= nsize)
    {
        return ptr;
    }
#ifdef LUACLI_MEMSTAT
    memMgr->totalUsed += (int)nsize;
    memMgr->nChunks++;
#endif /* LUACLI_MEMSTAT */
    if (nptr != NULL && ptr != NULL)
    {
        osMemCpy(nptr, ptr, (GT_U32)((osize < nsize) ? osize : nsize));
    }
    if (ptr != NULL)
    {
        memMgr->free(ptr);
#ifdef LUACLI_MEMSTAT
        memMgr->totalUsed -= (int)osize;
        memMgr->nChunks--;
#endif /* LUACLI_MEMSTAT */
    }
    return nptr;
}

/**
* @internal osShellParseCmdLine function
* @endinternal
*
* @brief   parse command line
*
* @param[in,out] s        - null terminated command string, will be modified
* @param[out]    cmd      - (pointer to) format string
* @param[out]    argv     - (pointer to) vector of output string arguments.
* @param[out]    argc     - (pointer to) number of arguments in output string
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS osShellParseCmdLine
(
    INOUT   char        *s,
    OUT     char*       *cmd,
    OUT     GT_UINTPTR  argv[],
    OUT     GT_32       *argc
)
{
    enum {
        ST_SPACE,
        ST_NUMERIC,
        ST_STRING,
        ST_STRING_Q,
        ST_STRING_HEX0,
        ST_STRING_HEX1
    } state = ST_SPACE;
    char *tok_begin = s;
    char *tok_end = s;
    GT_U32  number = 0;

    *argc = 0;
    /* skip leading spaces */
    while (*s == ' ' || *s == '\t')
        s++;

    /* get command name */
    *cmd = s;
    while (strchr(" \t\n,", *s) == NULL)
        s++;
    if (*s == 0)
    {
        return GT_OK;
    }
    *s++ = 0;

    /* parse parameters */
    for (; *s; s++)
    {
        switch (state)
        {
            case ST_SPACE:
                if (strchr(" \t\n,", *s) != NULL)
                    continue;
                argv[*argc] = 0;
                (*argc)++;
                if (*s == '"')
                {
                    state = ST_STRING;
                    tok_begin = s;
                    tok_end = s;
                    argv[*argc-1] = (GT_UINTPTR)tok_begin;
                }
                else
                {
                    state = ST_NUMERIC;
                    tok_begin = s;
                }
                continue;
            case ST_NUMERIC:
                if (strchr(" \t\n,", *s) != NULL)
                {
                    *s = 0;
                    argv[*argc-1] = strtoul(tok_begin, 0, 0);
                    state = ST_SPACE;
                    continue;
                }
                continue;
            case ST_STRING:
                if (*s == '"')
                {
                    *tok_end = 0;
                    state = ST_SPACE;
                    continue;
                }
                if (*s == '\\')
                {
                    state = ST_STRING_Q;
                    continue;
                }
                *tok_end++ = *s;
                continue;
            case ST_STRING_Q:
                switch (*s)
                {
                    case 'x': state = ST_STRING_HEX0; number = 0; continue;
                    case 'r': *tok_end++ = '\r'; break;
                    case 't': *tok_end++ = '\t'; break;
                    case 'n': *tok_end++ = '\n'; break;
                    default:
                        *tok_end++ = *s;
                }
                state = ST_STRING;
                continue;
            case ST_STRING_HEX0:
                if (strchr("0123456789abcdef", *s) == NULL)
                {
                    *tok_end++ = (char)number;
                    state = ST_STRING;
                    continue;
                }
                number = (*s > '9')
                    ? (*s + 10 - 'a')
                    : (*s - '0');
                state = ST_STRING_HEX1;
                continue;
            case ST_STRING_HEX1:
                if (strchr("0123456789abcdef", *s) != NULL)
                {
                    number *= 16;
                    number += (*s > '9')
                        ? (*s + 10 - 'a')
                        : (*s - '0');
                }
                *tok_end++ = (char)number;
                state = ST_STRING;
                continue;
        }
    }
    if (state == ST_SPACE)
        return GT_OK;

    if (state == ST_NUMERIC)
    {
        argv[*argc-1] = strtoul(tok_begin, 0, 0);
    }
    else
    {
        /* string */
        *tok_end = 0;
    }
    return GT_OK;
}

/**
* @internal luaCli_printf function
* @endinternal
*
* @brief  Function to print the data to telnet socket
*/

static void luaCli_printf(void *para, const char *fmt, ...)
{
    char buf[1024];
    IOStreamPTR IOStream = (IOStreamPTR)para;
    va_list ap;
    va_start(ap, fmt);
    cpssOsVsnprintf(buf, sizeof(buf), fmt, ap);
    IOStream->writeLine(IOStream, buf);
    va_end(ap);
}

/**
* @internal luaCpssForUpperLevelToDump function
* @endinternal
*
* @brief   wrapper function to print upper level's data to luaCLI.
*/
static int luaCpssForUpperLevelToDump(lua_State *L)
{
    FUNC_PTR func;
    GT_UINTPTR argv[32];
    GT_32 argc;
    IOStreamPTR IOStream;
    size_t len;
    const char *funcStr;
    GT_STATUS rc;
    char  * cmd;

    funcStr = lua_tolstring(L,1, &len);
    if (! lua_isstring(L, 1))
    {
        /*error*/
        return GT_FAIL;
    }

    if (osShellParseCmdLine((char *)funcStr, &cmd, argv, &argc) != GT_OK)
    {
        return GT_FAIL;
    }

    IOStream = (IOStreamPTR)luaCLIEngineGetData(L, "_IOStream", 0); /* get IOStream pointer */

    func = (FUNC_PTR) osShellGetFunctionByName(cmd); /* find function's pointer by name */
    if (0 == func)
    {
        return GT_FAIL;
    }

    rc = func(IOStream, luaCli_printf, argc, argv); /* call function, the log printed to luaCLI */

    cmdOsPrintf("return code is %d\n", rc);
    return rc;
}

/*******************************************************************************
* luaCLIEngine_newstate
*
* DESCRIPTION:
*       Init new Lua state, enable minimal functionality
*       Should be used to create luaCLI and for new Lua task
*
* INPUTS:
*       memMgr            - malloc/free struct pointer. Mandatory
*       IOStream          - (optional)
*       fs                - FS API pointer (optional)
*       getFunctionByName - (optional)
*
* OUTPUTS:
*       None
*
* RETURNS:
*       luaPtr
*
* COMMENTS:
*       None
*
*******************************************************************************/
lua_State* luaCLIEngine_newstate
(
    IN LUA_CLI_MALLOC_STC      *memMgr,
    IN IOStreamPTR              IOStream,
    IN genFS_PTR                fs,
    IN LUA_CLI_GETFUNCBYNAME_T  getFunctionByName
)
{
    lua_State *L;

    L = lua_newstate((lua_Alloc)luaCLIEngine_alloc_wrapper, memMgr);
    if (L == NULL)
        return NULL;

    lua_pushlightuserdata(L, memMgr);
    lua_setglobal(L, "_memMgr");
    lua_pushlightuserdata(L, IOStream);
    lua_setglobal(L, "_IOStream");
    lua_pushlightuserdata(L, fs);
    lua_setglobal(L, "_genFS");
    lua_pushlightuserdata(L, getFunctionByName);
    lua_setglobal(L, "_getFunctionByName");

    luaL_openlibs(L);
    /* register(override) print function */
    lua_register(L, "print", luaCLIEnginePrint);
    lua_register(L, "splitline", luaCLI_splitline);
    lua_register(L, "cmdLuaCLI_require", luaCLI_require);
    /* create C functions registry */
    lua_newtable(L);
    lua_setglobal(L, "cmdLuaCLI_CfunctionsRegistry");
    lua_register(L, "cmdLuaCLI_registerCfunction", luaCLI_registerCfunction);
    lua_register(L, "cmdLuaCLI_callCfunction", luaCLI_callCfunction);

    /* register global storage for Lua data */
    lua_register(L, "globalStore", luaGlobalStore);
    lua_register(L, "globalGet", luaGlobalGet);

    /* Register XPS API call to LUA */
    lua_register(L, "upperLevelDump", luaCpssForUpperLevelToDump);

    if (fs != NULL)
    {
        luaEngineFSregisterLoader(L);
        luaEngineFSregisterAPI(L);
    }

    luaCLIEngineNetRegisterAPI(L);

    /* register generic wrapper and generic types */
    lua_pushcfunction(L, luaEngineGenWrapper_init);
    lua_call(L, 0, 0);

    return L;
}

