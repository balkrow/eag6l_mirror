
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


static const char* const PATCHES[] =
{
    /* add patches here, for example:
       "11338765 – Serdes parameters fix for 40KR",
    */
    NULL /* Last entry, do not delete */
};


GT_VOID cpssGenShowPatches()
{
    int i;

    if (PATCHES[0])
    {
        for (i = 0; PATCHES[i]; i++)
            cpssOsPrintf("%s\n", PATCHES[i]);
    }
    else
        cpssOsPrintf("No patches\n");
}

