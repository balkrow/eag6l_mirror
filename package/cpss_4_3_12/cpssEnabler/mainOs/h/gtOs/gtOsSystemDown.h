#ifndef __gtOsSysDown
#define __gtOsSysDown

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>
#include <gtOs/gtOsGen.h>


/************* Functions ******************************************************/

/*******************************************************************************
* osRegisterCpuResetFunc
*
* DESCRIPTION:
*       Registers the given function to be called at various events in which
*       the App terminates, including when CPU gets reset and App receives
*   	sigterm.
*   	The functions are called in the reverse order of their registration
*
* INPUTS:
*
*        None
*
* OUTPUTS:
*       Process ID
*
* RETURNS:
*      NONE
*
* COMMENTS:
* 		The same function may be registered multiple times: it is called
*   	once for each registration
*
*******************************************************************************/
GT_STATUS osRegisterCpuResetFunc(OS_AT_EXIT_FUNC rstFunc);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
