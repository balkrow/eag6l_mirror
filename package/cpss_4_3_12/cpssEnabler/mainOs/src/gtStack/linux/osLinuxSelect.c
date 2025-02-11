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
* @file osLinuxSelect.c
*
* @brief Linux User Mode Operating System wrapper.
* synchronous I/O multiplexing facility
*
* @version   3
********************************************************************************
*/

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <gtOs/gtOsMem.h>
#include <gtOs/gtGenTypes.h>
#include <gtStack/gtStackTypes.h>
#include <gtStack/gtOsSelect.h>
#include <gtUtil/gtBmPool.h>

#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>

/*global variables macros*/

#define PRV_SHARED_MAIN_OS_DIR_OS_LINUX_SELECT_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_HELPER_SHARED_GLOBAL_VAR_SET(cpssEnablerMod.mainOsDir.osLinuxSelectSrc._var,_value)

#define PRV_SHARED_MAIN_OS_DIR_OS_LINUX_SELECT_SRC_GLOBAL_VAR_GET(_var)\
    PRV_HELPER_SHARED_GLOBAL_VAR_GET(cpssEnablerMod.mainOsDir.osLinuxSelectSrc._var)

#define SOCKET_FD_SET_POOL_SIZE   50

/*******************************************************************************
* osSelectCreateSet()
*
* DESCRIPTION:
*       Create a set of file descriptors for the select function
*
* INPUTS:
*       none
*
* OUTPUTS:
*       none
*
* RETURNS:
*       Pointer to the set. If unable to create, returns null. Note that the
*       pointer is from void type.
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID * osSelectCreateSet
(
)
{
   fd_set * set ;

    if( 0 == PRV_SHARED_MAIN_OS_DIR_OS_LINUX_SELECT_SRC_GLOBAL_VAR_GET(socketFdPool))
    {

        if(gtPoolCreatePool(sizeof(fd_set),
                            GT_4_BYTE_ALIGNMENT,
                            SOCKET_FD_SET_POOL_SIZE,
                            GT_TRUE, &PRV_SHARED_MAIN_OS_DIR_OS_LINUX_SELECT_SRC_GLOBAL_VAR_GET(socketFdPool)) != GT_OK)
        {
            return NULL;
        }
    }

    set = (fd_set*) gtPoolGetBuf(PRV_SHARED_MAIN_OS_DIR_OS_LINUX_SELECT_SRC_GLOBAL_VAR_GET(socketFdPool));
    if (set == NULL)
        return NULL ;

    FD_ZERO(set) ;

    return (GT_VOID*) set ;

}


/**
* @internal osSelectEraseSet function
* @endinternal
*
* @brief   Erase (delete) a set of file descriptors
*
* @param[in] set                      - Pointer to the set.
*                                       None
*/
GT_VOID osSelectEraseSet
(
    GT_VOID *  set
)
{
    if( 0 != PRV_SHARED_MAIN_OS_DIR_OS_LINUX_SELECT_SRC_GLOBAL_VAR_GET(socketFdPool) )
    {
        gtPoolFreeBuf(PRV_SHARED_MAIN_OS_DIR_OS_LINUX_SELECT_SRC_GLOBAL_VAR_GET(socketFdPool), set);
    }
}

/**
* @internal osSelectZeroSet function
* @endinternal
*
* @brief   Zeros a set of file descriptors
*
* @param[in] set                      - Pointer to the set.
*                                       None
*/
GT_VOID osSelectZeroSet
(
    GT_VOID *  set
)
{
    FD_ZERO((fd_set *)set) ;
}


/**
* @internal osSelectAddFdToSet function
* @endinternal
*
* @brief   Add a file descriptor to a specific set
*
* @param[in] set                      - Pointer to the set
* @param[in] fd                       - A file descriptor
*                                       None
*/
GT_VOID osSelectAddFdToSet
(
    GT_VOID *  set ,
    GT_FD      fd
)
{
    FD_SET((int)fd, (fd_set*)set) ;
}


/**
* @internal osSelectClearFdFromSet function
* @endinternal
*
* @brief   Remove (clear) a file descriptor from a specific set
*
* @param[in] set                      - Pointer to the set
* @param[in] fd                       - A file descriptor
*                                       None
*/
GT_VOID osSelectClearFdFromSet
(
    GT_VOID *  set ,
    GT_FD      fd
)
{
    FD_CLR((int)fd, (fd_set*)set) ;
}


/**
* @internal osSelectIsFdSet function
* @endinternal
*
* @brief   Test if a specific file descriptor is set in a set
*
* @param[in] set                      - Pointer to the set
* @param[in] fd                       - A file descriptor
*                                       GT_TRUE (non zero) if set , returned as unsigned int
*                                       GT_FALSE (zero) if not set , returned as unsigned int
*/
GT_BOOL osSelectIsFdSet
(
    GT_VOID *  set ,
    GT_FD      fd
)
{
    if( (FD_ISSET((int)fd, (fd_set*)set)) != 0 )
    {
        return GT_TRUE;
    }
    else
    {
        return GT_FALSE;
    }
}


/**
* @internal osSelectCopySet function
* @endinternal
*
* @brief   Duplicate sets (require 2 pointers for sets)
*
* @param[in] srcSet                   - Pointer to source set
* @param[in] dstSet                   - Pointer to destination set
*                                       Mone
*/
GT_VOID osSelectCopySet
(
    GT_VOID *  srcSet ,
    GT_VOID *  dstSet
)
{
    *((fd_set*)dstSet) = *((fd_set*)srcSet) ;
}


/**
* @internal osSelect function
* @endinternal
*
* @brief   OS Dependent select function
*
* @param[in] width                    - The highest-numbered descriptor in any of the next three
*                                      sets + 1 (if zero, The default length will be taken)
* @param[in] readSet                  - Pointer to a read operation  descriptor set
* @param[in] writeSet                 - Pointer to a write operation descriptor set
* @param[in] exceptionSet             Pointer to an exception descriptor set (not supported in
*                                      all OS, such as VxWorks)
* @param[in] timeOut                  - Maximum time to wait on in milliseconds. Sending a
*                                      GT_INFINITE value will block indefinitely. Zero value cause
*                                      no block.
*                                       On success, returns the number of descriptors contained in the
*                                       descriptor sets, which may be zero if the timeout expires before
*
* @retval anything interesting happens. On  error, -1 is returned, and errno
*                                       is set appropriately; the sets and timeout become
*                                       undefined, so do not rely on their contents after an error.
*/
GT_SSIZE_T osSelect
(
    GT_FD      width ,
    GT_VOID *  readSet ,
    GT_VOID *  writeSet ,
    GT_VOID *  exceptionSet ,
    GT_U32     timeOut
)
{
    struct timeval tv ;
    int lwidth ;

    if (width == 0)
        lwidth = FD_SETSIZE ;
    else
        lwidth = (int)width ;

    if (timeOut == GT_INFINITE)
    {
        return (GT_SSIZE_T)(select(lwidth ,
               (fd_set*)readSet ,
               (fd_set*)writeSet ,
               (fd_set*)exceptionSet ,
               NULL)) ;
    }
    else
    {
        /* Convert to seconds and calculate the reminder in microseconds */
        tv.tv_sec = timeOut/1000 ;
        tv.tv_usec = (timeOut - (tv.tv_sec*1000)) * 1000;

        return (GT_SSIZE_T)(select(lwidth ,
                        (fd_set*)readSet ,
                        (fd_set*)writeSet ,
                        (fd_set*)exceptionSet ,
                        &tv)) ;
    }
    /*
    ** Note that now FD_ISSET on the file descriptor with its
    ** corresponded set will be true if the file descriptor is
    ** ready
    */
}

/**
* @internal osSocketGetSocketFdSetSize function
* @endinternal
*
* @brief   Returns the size of fd_set.
*
* @param[out] sockFdSize               - to be filled with fd_set size.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*
* @note USERS THAT IMPLEMENTS THEIR OWN OS LAYER CAN RETURN SIZE = 0.
*
*/
GT_STATUS osSocketGetSocketFdSetSize
(
    OUT GT_U32*    sockFdSize
)
{
    *sockFdSize = sizeof( fd_set);

    return GT_OK ;
}


