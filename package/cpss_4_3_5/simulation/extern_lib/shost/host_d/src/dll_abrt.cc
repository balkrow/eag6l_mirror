/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: dll aborting from process
 *!--------------------------------------------------------------------------
 *$ FILENAME: h:\av01\host\h_center\src\dll_abrt.c
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: ????
 *!--------------------------------------------------------------------------
 *$ AUTHORS: Eyran   
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 14-Jan-97  15:16:35         CREATION DATE: 14-Jan-97
 *!**************************************************************************
 *!
 *!**************************************************************************
 *!
 *$ FUNCTION: <FuncName>
 *!
 *$ GENERAL DESCRIPTION:
 *!
 *$ RETURNS:
 *!
 *$ ALGORITHM:   (local)
 *!
 *$ ASSUMPTIONS:
 *!
 *$ REMARKS:     (local)
 *!
 *!**************************************************************************
 *!*/

extern void SHOSTC_dll_abort(
    /* INPUT  */
    UINT_8* msg
)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/

//    printf(msg) ;

    Perror(msg);
    SHOSTG_abort() ;
}
/*$ END OF SHOSTP_dll_abort */
