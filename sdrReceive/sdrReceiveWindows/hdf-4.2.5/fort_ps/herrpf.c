/****************************************************************************
 * NCSA HDF                                                                 *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 *                                                                          *
 * For conditions of distribution and use, see the accompanying             *
 * hdf/COPYING file.                                                        *
 *                                                                          *
 ****************************************************************************/

#ifdef RCSID
static char RcsId[] = "@(#)$Revision: 4888 $";
#endif

/* $Id: herrpf.c 4888 2007-07-17 21:16:15Z swegner $ */

/*-----------------------------------------------------------------------------
 * File:    herrf.c
 * Purpose: C stubs for error-handling Fortran routines
 * Invokes: herr.c.
 * Contents:
 *     heprnt_:    Call HEprint to print error message
 * Remarks: none
 *---------------------------------------------------------------------------*/

#include "hdf.h"
#include "hproto_fortran.h"
/*
#ifndef HERR_FNAMES
#   define HERR_FNAMES
#ifdef DF_CAPFNAMES
#   define nheprnt   FNAME(HEPRNT)
#else
#   define nheprnt   FNAME(heprnt) */
/*#endif*/ /* DF_CAPFNAMES */
/*#endif*/ /* HERR_FNAMES */

/*-----------------------------------------------------------------------------
 * Name:    heprntc
 * Purpose: call HEprint to print error messages, starting from top of stack
 * Inputs:  filename - name of the output file; if lenght is 0 then
 *                     messages will be printied to stderr. 
 *          print_levels: number of levels to print
 *          namelen - length of the filname string
 * Users:   Fortran stub routine
 * Invokes: HEprint
 * Returns: 0 on success, FAIL on failure
 *---------------------------------------------------------------------------*/

FRETVAL(intf)

#ifdef PROTOTYPE
nheprntc(_fcd filename, intf * print_levels, intf *namelen)
#else
nheprntc(filename, print_levels, namelen)
           _fcd  filename;
           intf *print_levels; 
           intf  *namelen;
#endif /* PROTOTYPE */

{
    FILE *err_file;
    char * c_name;
    intn c_len;
    int ret = 0;

    c_len = *namelen;
    if(c_len == 0) {
                HEprint(stderr, *print_levels);
                return(ret);
    }
    c_name = HDf2cstring(filename, c_len);
    	if (!c_name) return(FAIL);
    err_file = fopen(c_name, "a");
    	if (!err_file) return(FAIL);
    HEprint(err_file, *print_levels);
    fclose(err_file);
    return(ret);
    
}
/*-----------------------------------------------------------------------------
 * Name: hestringc
 * Purpose:  Calls HEstring
 * Inputs:   error_code - HDF error code
 * Outputs: error_message - error message assocoated with the error code
 * Retruns: SUCCEED (0) if successful and FAIL(-1) otherwise
 *----------------------------------------------------------------------------*/
 
 
 FRETVAL(intf)
#ifdef PROTOTYPE
nhestringc(intf *error_code,
            _fcd error_message, intf *len)
#else
nhestringc(error_code, error_message, len)
           intf *error_code;
           _fcd  error_message;
           intf  *len;
#endif /* PROTOTYPE */
{
   char *cstring = NULL;
   intn   status;
 
   status = -1;
   cstring = (char *)HEstring((hdf_err_code_t) *error_code);
   if (cstring) {
                status = 0;
                HDpackFstring(cstring,  _fcdtocp(error_message),  *len);
   }  
   return status;
 
 
}
