/*!**************************************************RND Template version 4.1
 *!     F U N C T I O N   D E S I G N   &   I M P L E M E N T A T I O N
 *!==========================================================================
 *$ TITLE: set the config file
 *!--------------------------------------------------------------------------
 *$ FILENAME: u:\work95\host_d\h_conf\src\set_file.c
 *!--------------------------------------------------------------------------
 *$ SYSTEM, SUBSYSTEM: nste
 *!--------------------------------------------------------------------------
 *$ AUTHORS: EYRAN     
 *!--------------------------------------------------------------------------
 *$ LATEST UPDATE: 08-Jan-97  10:12:09         CREATION DATE: 08-Jan-97
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

extern void CNFG_set_cnf_file (

    /*!     INPUTS:             */

    UINT_8 *file_name           /*!*/
)
{
/*!*************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!*************************************************************************/

/*!*************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!*************************************************************************/

    if(file_name)
        strcpy(CNFG_cofig_file_name,file_name) ;
}
/*$ END OF CNFG_set_cnf_file */
