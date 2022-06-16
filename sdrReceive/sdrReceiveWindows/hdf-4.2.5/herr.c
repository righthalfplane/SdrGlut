/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF.  The full HDF copyright notice, including       *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at      *
 * http://hdfgroup.org/products/hdf4/doc/Copyright.html.  If you do not have *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifdef RCSID
static char RcsId[] = "@(#)$Revision: 4932 $";
#endif

/* $Id: herr.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*LINTLIBRARY */
/* -------------------------------- herr.c -------------------------------- */
/*
   HDF error handling / reporting routines

LOCAL ROUTINES
  None
EXPORTED ROUTINES
  HEstring -- return error description
  HEclear  -- clear the error stack
  HEpush   -- push an error onto the stack
  HEreport -- give a more detailed error description
  HEprint  -- print values from the error stack
  HEvalue  -- return a error off of the error stack
 */

#define _H_ERR_MASTER_

#include "hdf.h"

/*
   ** Include files for variable argument processing for HEreport
 */
#include <stdarg.h>

/* We use a stack to hold the errors plus we keep track of the function,
   file and line where the error occurs. */

/* the structure of the error stack element */
typedef struct error_t
  {
      hdf_err_code_t error_code;    /* Error number */
      char function_name[FUNC_NAME_LEN];    /* function where error occur */
      const char *file_name;    /* file where error occur */
      intn        line;         /* line in file where error occurs */
      intn        system;       /* for system or HDF error */
      char       *desc;         /* optional supplied description */
  }
error_t;


/* pointer to the structure to hold error messages */
PRIVATE error_t *error_stack = NULL;

#ifndef DEFAULT_MESG
#   define DEFAULT_MESG "Unknown error"
#endif

/* size of error message table */
#define ERRMESG_SZ (sizeof(error_messages) / sizeof(error_messages[0]))

/*------------------------------------------------------------------------
NAME
   HEstring -- return error description
USAGE
   char * HEstring(error_code)
   int16  error_code;      IN: the numerical value of this error
RETURNS
   An error description string
DESCRIPTION
   Return a textual description of the given error.  These strings
   are statically declared and should not be free()ed by the user.
   If no string can be found to describe this error a generic
   default message is returned.

---------------------------------------------------------------------------*/
const char *
HEstring(hdf_err_code_t error_code)
{
    int         i;              /* temp int index */

    /* look for the error_code in error message table */
    for (i = 0; i < (int)ERRMESG_SZ; i++)
        if (error_messages[i].error_code == error_code)
          {
            return error_messages[i].str;
          }

    /* otherwise, return default message */
    return DEFAULT_MESG;
} /* HEstring */

/*--------------------------------------------------------------------------
NAME
   HEclear -- clear the error stack
USAGE
   VOID HEclear(VOID)
RETURNS
   NONE
DESCRIPTION
   Remove all currently reported errors from the error stack

---------------------------------------------------------------------------*/
VOID
HEPclear(void)
{
    if (!error_top)
        goto done;

    /* error_top == 0 means no error in stack */
    /* clean out old descriptions if they exist */
    for (; error_top > 0; error_top--)
      {
          if (error_stack[error_top - 1].desc)
            {
                HDfree(error_stack[error_top - 1].desc);
                error_stack[error_top - 1].desc = NULL;
            }
      }

done:
  return;
} /* HEPclear */

/*-------------------------------------------------------------------------
NAME
   HEpush -- push an error onto the stack
USAGE
   VOID HEpush(error_code, func_name, file_name, line)
   int16  error_code;      IN: the numerical value of this error
   char * func_name;       IN: function where the error happened
   char * file_name;       IN: file name of offending function
   int    line;            IN: line number of the reporting statment
RETURNS
   NONE
DESCRIPTION
   push a new error onto stack.  If stack is full, error 
   is ignored.  assumes that the character strings 
   (function_name and file_name) referred are in some 
   semi-permanent storage, so it just saves the pointer 
   to the strings.  blank out the description field so 
   that a description is reported  only if REreport is called

---------------------------------------------------------------------------*/
VOID
HEpush(hdf_err_code_t error_code, const char *function_name, const char *file_name, intn line)
{
    intn        i;

    /* if the stack is not allocated, then do it */
    if (!error_stack)
      {
          error_stack = (error_t *) HDmalloc((uint32) sizeof(error_t) * ERR_STACK_SZ);
          if (!error_stack)
            {
                puts("HEpush cannot allocate space.  Unable to continue!!");
                exit(8);
            }
          for (i = 0; i < ERR_STACK_SZ; i++)
              error_stack[i].desc = NULL;
      }

    /* if stack is full, discard error */
    /* otherwise, push error details onto stack */

    if (error_top < ERR_STACK_SZ)
      {
          HDstrcpy(error_stack[error_top].function_name,function_name);
          error_stack[error_top].file_name = file_name;
          error_stack[error_top].line = line;
          error_stack[error_top].error_code = error_code;
          if (error_stack[error_top].desc)
            {
                HDfree(error_stack[error_top].desc);
                error_stack[error_top].desc = NULL;
            }
          error_top++;
      }
}   /* HEpush */

/*-------------------------------------------------------------------------
NAME
   HEreport -- give a more detailed error description
USAGE
   VOID HEreport(format, ....)
   char * format;           IN: printf style print statement
RETURNS
   NONE
DESCRIPTION
   Using printf and the variable number of args facility allow the
   library to specify a more detailed description of a given
   error condition

---------------------------------------------------------------------------*/
VOID
HEreport(const char *format,...)
{
    va_list     arg_ptr;
    char       *tmp;
    CONSTR(FUNC, "HEreport");   /* name of function if HIalloc fails */

    va_start(arg_ptr, format);

    if ((error_top < ERR_STACK_SZ + 1) && (error_top > 0))
      {
          tmp = (char *) HDmalloc(ERR_STRING_SIZE);
          if (!tmp)
            {
                HERROR(DFE_NOSPACE);
                goto done;
            }
          vsprintf(tmp, format, arg_ptr);
          if (error_stack[error_top - 1].desc)
              HDfree(error_stack[error_top - 1].desc);
          error_stack[error_top - 1].desc = tmp;
      }

    va_end(arg_ptr);

done:
    return;
} /* HEreport */

/*-------------------------------------------------------------------------
NAME
   HEprint -- print values from the error stack
USAGE
   VOID HEprint(stream, levels)
   FILE * stream;      IN: file to print error message to
   int32  level;       IN: level at which to start printing
RETURNS
   NONE
DESCRIPTION
   Print part of the error stack to a given file.  If level == 0
   the entire stack is printed.  If an extra description has been
   added (via HEreport) it is printed too.

---------------------------------------------------------------------------*/
VOID
HEprint(FILE * stream, int32 print_levels)
{
    if (print_levels == 0 || print_levels > error_top)  /* print all errors */
        print_levels = error_top;

    /* print the errors starting from most recent */
    for (print_levels--; print_levels >= 0; print_levels--)
      {
          fprintf(stream, "HDF error: (%d) <%s>\n\tDetected in %s() [%s line %d]\n",
                  error_stack[print_levels].error_code,
                  HEstring(error_stack[print_levels].error_code),
                  error_stack[print_levels].function_name,
                  error_stack[print_levels].file_name,
                  error_stack[print_levels].line);
          if (error_stack[print_levels].desc)
              fprintf(stream, "\t%s\n", error_stack[print_levels].desc);
      }
} /* HEprint */

/* ------------------------------- HEvalue -------------------------------- */
/*

   NAME
   HEvalue -- return a error off of the error stack
   USAGE
   int16 HEvalue(level)
   int32 level;           IN: level of the error stack to return
   RETURNS
   Error code or DFE_NONE if no error
   DESCRIPTION
   Return the error code of a single error out of the error stack

   --------------------------------------------------------------------------- */
int16
HEvalue(int32 level)
{
  int16 ret_value = DFE_NONE;

  if (level > 0 && level <= error_top)
    ret_value = (int16) error_stack[error_top - level].error_code;
  else
    ret_value = DFE_NONE;

  return ret_value;
} /* HEvalue */

/*--------------------------------------------------------------------------
 NAME
    HEshutdown
 PURPOSE
    Terminate various static buffers.
 USAGE
    intn HEshutdown()
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    Free various buffers allocated in the HE routines.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    Should only ever be called by the "atexit" function HDFend
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn HEshutdown(void)
{
    if(error_stack!=NULL)
      {
          HDfree(error_stack);
          error_stack=NULL;
      } /* end if */
    return(SUCCEED);
} /* end HEshutdown() */

