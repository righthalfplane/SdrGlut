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

/* $Id: hkit.c 4932 2007-09-07 17:17:23Z bmribler $ */

#include <ctype.h>
#include "hkit.h"

#ifdef _MSC_VER
#define strdup _strdup
#endif

/*
LOCAL ROUTINES
  None
EXPORTED ROUTINES
  HDc2fstr      -- convert a C string into a Fortran string IN PLACE
  HDf2cstring   -- convert a Fortran string to a C string
  HDpackFstring -- convert a C string into a Fortran string
  HDflush       -- flush the HDF file
  HDgettagdesc  -- return a text description of a tag
  HDgettagsname -- return a text name of a tag
  HDgettagnum   -- return the tag number for a text description of a tag
  HDgetNTdesc   -- return a text description of a number-type
  HDfidtoname   -- return the filename the file ID corresponds to
*/

/* ------------------------------- HDc2fstr ------------------------------- */
/*
NAME
   HDc2fstr -- convert a C string into a Fortran string IN PLACE
USAGE
   intn HDc2fstr(str, len)
   char * str;       IN: string to convert
   intn   len;       IN: length of Fortran string
RETURNS
   SUCCEED
DESCRIPTION
   Change a C string (NULL terminated) into a Fortran string.
   Basically, all that is done is that the NULL is ripped out
   and the string is padded with spaces

---------------------------------------------------------------------------*/
intn 
HDc2fstr(char *str, intn len)
{
    int         i;

    i=(int)HDstrlen(str);
    for (; i < len; i++)
        str[i] = ' ';
    return SUCCEED;
}   /* HDc2fstr */

/* ----------------------------- HDf2cstring ------------------------------ */
/*
NAME
   HDf2cstring -- convert a Fortran string to a C string
USAGE
   char * HDf2cstring(fdesc, len)
   _fcd  fdesc;     IN: Fortran string descriptor
   intn  len;       IN: length of Fortran string
RETURNS
   Pointer to the C string if success, else NULL
DESCRIPTION
   Chop off trailing blanks off of a Fortran string and
   move it into a newly allocated C string.  It is up
   to the user to free this string.

---------------------------------------------------------------------------*/
char *
HDf2cstring(_fcd fdesc, intn len)
{
    CONSTR(FUNC, "HDf2cstring");  /* for HERROR */
    char       *cstr, *str;
    int         i;

    str = _fcdtocp(fdesc);
    /* This should be equivalent to the above test -QAK */
    for(i=len-1; i>=0 && !isgraph((int)str[i]); i--)
        /*EMPTY*/;
    cstr = (char *) HDmalloc((uint32) (i + 2));
    if (!cstr)
	HRETURN_ERROR(DFE_NOSPACE, NULL);
    cstr[i + 1] = '\0';
    HDmemcpy(cstr,str,i+1);
    return cstr;
}   /* HDf2cstring */
/* ---------------------------- HDpackFstring ----------------------------- */
/*
NAME
   HDpackFstring -- convert a C string into a Fortran string
USAGE
   intn HDpackFstring(src, dest, len)
   char * src;          IN:  source string
   char * dest;         OUT: destination
   intn   len;          IN:  length of string
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   given a NULL terminated C string 'src' convert it to
   a space padded Fortran string 'dest' of length 'len'

   This is very similar to HDc2fstr except that function does
   it in place and this one copies.  We should probably only
   support one of these.

---------------------------------------------------------------------------*/
intn 
HDpackFstring(char *src, char *dest, intn len)
{
    intn        sofar;

    for (sofar = 0; (sofar < len) && (*src != '\0'); sofar++)
        *dest++ = *src++;

    while (sofar++ < len)
        *dest++ = ' ';

    return SUCCEED;
}	/* HDpackFstring */

/* ------------------------------- HDflush -------------------------------- */
/*
NAME
   HDflush -- flush the HDF file
USAGE
   intn HDflush(fid)
   int32 fid;            IN: file ID
RETURNS
   SUCCEED / FAIL
DESCRIPTION
   Force the system to flush the HDF file stream

   This should be primarily used for debugging

   The MAC does not really support fflush() so this r
   outine just returns SUCCEED always on a MAC w/o
   really doing anything.

---------------------------------------------------------------------------*/
intn 
HDflush(int32 file_id)
{
    CONSTR(FUNC, "HDflush");    /* for HERROR */

#if !(defined (MAC) || defined (macintosh) || defined (SYMANTEC_C))
    filerec_t  *file_rec;

    file_rec = HAatom_object(file_id);
    if (BADFREC(file_rec))
        HRETURN_ERROR(DFE_ARGS, FAIL);

    HI_FLUSH(file_rec->file);
#endif /* MAC */

    return SUCCEED;
}	/* HDflush */

/* ----------------------------- HDgettagdesc ----------------------------- */
/*
NAME
   HDgettagdesc -- return a text description of a tag
USAGE
   char * HDgettagdesc(tag)
   uint16   tag;          IN: tag of element to find
RETURNS
   Descriptive text or NULL
DESCRIPTION
   Map a tag to a statically allocated text description of it.

---------------------------------------------------------------------------*/
const char *
HDgettagdesc(uint16 tag)
{
    intn        i;

    for (i = 0; i < (intn)(sizeof(tag_descriptions) / sizeof(tag_descript_t)); i++)
        if (tag_descriptions[i].tag == tag)
            return (tag_descriptions[i].desc);
    return (NULL);
} /* HDgettagdesc */

/* ----------------------------- HDgettagsname ----------------------------- */
/*
NAME
   HDgettagsname -- return a text name of a tag
USAGE
   char * HDgettagsname(tag)
   uint16   tag;          IN: tag of element to find
RETURNS
   Descriptive text or NULL
DESCRIPTION
   Map a tag to a dynamically allocated text name of it.
   Checks for special elements now.

--------------------------------------------------------------------------- */
char *
HDgettagsname(uint16 tag)
{
    CONSTR(FUNC, "HDgettagsname");  /* for HERROR */
    char       *ret = NULL;
    intn        i;

    if (SPECIALTAG(tag))
        ret = (char *) HDstrdup("Special ");
    tag = BASETAG(tag);
    for (i = 0; i < (intn)(sizeof(tag_descriptions) / sizeof(tag_descript_t)); i++)
        if (tag_descriptions[i].tag == tag)
          {
              if (ret == NULL)
                  ret = (char *) HDstrdup(tag_descriptions[i].name);
              else
                {
                    char       *t;

                    t = (char *) HDmalloc(HDstrlen(ret) +
                                    HDstrlen(tag_descriptions[i].name) + 2);
                    if (t == NULL)
                      {
                          HDfree(ret);
                          HRETURN_ERROR(DFE_NOSPACE, NULL)
                      }     /* end if */
                    HDstrcpy(t, ret);
                    HDstrcat(t, tag_descriptions[i].name);
                    HDfree(ret);
                    ret = t;
                }   /* end else */
          }     /* end if */
    return (ret);
}  /* HDgettagsname */

/* ----------------------------- HDgettagnum ------------------------------ */
/*
NAME
   HDgettagnum -- return the tag number for a text description of a tag
USAGE
   intn HDgettagnum(tag_name)
   char *   tag_name;         IN: name of tag to find
RETURNS
   Tag number (>=0) on success or FAIL on failure
DESCRIPTION
   Map a tag name to a statically allocated tag number for it.

---------------------------------------------------------------------------*/
intn 
HDgettagnum(const char *tag_name)
{
    intn        i;

    for (i = 0; i < (intn)(sizeof(tag_descriptions) / sizeof(tag_descript_t)); i++)
        if (0 == HDstrcmp(tag_descriptions[i].name, tag_name))
            return ((intn)tag_descriptions[i].tag);
    return (FAIL);
} /* HDgettagnum */

/* ----------------------------- HDgetNTdesc ----------------------------- */
/*
NAME
   HDgetNTdesc -- return a text description of a number-type
USAGE
   char * HDgetNTdesc(nt)
   int32   nt;          IN: tag of element to find
RETURNS
   Descriptive text or NULL
DESCRIPTION
   Map a number-type to a dynamically allocated text description of it.

---------------------------------------------------------------------------*/
char *
HDgetNTdesc(int32 nt)
{
    CONSTR(FUNC, "HDgetNTdesc");    /* for HERROR */
    intn        i;
    char       *ret_desc = NULL;

    /* evil hard-coded values */
    if (nt & DFNT_NATIVE)
        ret_desc = (char *) HDstrdup(nt_descriptions[0].desc);
    else if (nt & DFNT_CUSTOM)
        ret_desc = (char *) HDstrdup(nt_descriptions[1].desc);
    else if (nt & DFNT_LITEND)
        ret_desc = (char *) HDstrdup(nt_descriptions[2].desc);

    nt &= DFNT_MASK;    /* mask off unusual format types */
    for (i = 3; i < (intn)(sizeof(nt_descriptions) / sizeof(nt_descript_t)); i++)
        if (nt_descriptions[i].nt == nt)
          {
              if (ret_desc == NULL)
                  ret_desc = (char *) HDstrdup(nt_descriptions[i].desc);
              else
                {
                    char       *t;

                    t = (char *) HDmalloc(HDstrlen(ret_desc) +
                                     HDstrlen(nt_descriptions[i].desc) + 2);
                    if (t == NULL)
                      {
                          HDfree(ret_desc);
                          HRETURN_ERROR(DFE_NOSPACE, NULL)
                      }     /* end if */
                    HDstrcpy(t, ret_desc);
                    HDstrcat(t, " ");
                    HDstrcat(t, nt_descriptions[i].desc);
                    HDfree(ret_desc);
                    ret_desc = t;
                }   /* end else */
              return (ret_desc);
          }     /* end if */
    return (NULL);
}   /* end HDgetNTdesc() */

/* ------------------------------- HDfidtoname ------------------------------ */
/*
NAME
   HDfidtoname -- return the filename the file ID corresponds to
USAGE
   const char * HDfidtoname(fid)
   int32 fid;            IN: file ID
RETURNS
   SUCCEED - pointer to filename / FAIL - NULL
DESCRIPTION
   Map a file ID to the filename used to get it.  This is useful for 
   mixing old style single-file interfaces (which take filenames) and
   newer interfaces which use file IDs.

---------------------------------------------------------------------------*/
const char *
HDfidtoname(int32 file_id)
{
    CONSTR(FUNC, "HDfidtoname");    /* for HERROR */
    filerec_t  *file_rec;

    if ((file_rec = HAatom_object(file_id)) == NULL)
        HRETURN_ERROR(DFE_ARGS, NULL);

    return (file_rec->path);
}   /* HDfidtoname */

