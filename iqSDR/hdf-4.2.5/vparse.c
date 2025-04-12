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
/* $Id: vparse.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*****************************************************************************

 file - vparse.c

 Part of the HDF VSet interface.

 NOTE: Another pass needs to made through this file to update some of
       the comments about certain sections of the code. -GV 9/8/97

************************************************************************/

#define VSET_INTERFACE
#include "hdf.h"

#define ISCOMMA(c) ( (c==',') ? 1:0 )

#if defined(macintosh) || defined(MAC) || defined(SYMANTEC_C) || defined(DMEM)   /* Dynamic memory */
PRIVATE char **symptr = NULL;   /* array of ptrs to tokens  ? */
PRIVATE char **sym = NULL;      /* array of tokens ? */
#else  /* !macintosh */
PRIVATE char *symptr[VSFIELDMAX];       /* array of ptrs to tokens  ? */
PRIVATE char sym[VSFIELDMAX][FIELDNAMELENMAX + 1];  /* array of tokens ? */
#endif /* !macintosh */
PRIVATE intn nsym;              /* token index ? */

/* Temporary buffer for I/O */
PRIVATE uint32 Vpbufsize = 0;
PRIVATE uint8 *Vpbuf = NULL;

/*******************************************************************************
 NAME
   scanattrs

 DESCRIPTION
   Given a string (attrs) , the routine parses it into token strings,
   and returns a ptr (attrv) to an array of ptrs where the tokens
   are stored.  The number of tokens are returned in attrc.
   
   Currently used only by routines that manipulate field names.
   As such each field string is truncated to a max length of
   FIELDNAMELENMAX (as defined in hdf.h). For most cases, this
   truncation doesn't happen because FIELDNAMELENMAX is a big number.
   
   Current implementation: all strings inputs converted to uppercase.
   tokens must be separated by COMMAs.
   
   Tokens are stored in static area sym , and pointers are returned
   to calling routine. Hence, tokens must be used before next call
   to scanattrs.

 RETURNS
    Returns SUCCEED/FAIL

*******************************************************************************/
int32 
scanattrs(const char *attrs, 
          int32 *attrc, 
          char ***attrv)
{
    CONSTR(FUNC, "scanattrs");
    char *s, *s0, *ss;
    intn len;
    size_t slen = HDstrlen(attrs)+1;

#if defined(macintosh) || defined(MAC) || defined(SYMANTEC_C) || defined(DMEM)   /* Dynamic memory */
    intn i;

    /* Lets allocate space for ptrs to tokens and tokens */
    if (symptr == NULL)
      {
          symptr = (char **) HDmalloc(VSFIELDMAX * sizeof(char *));
          if (symptr == NULL)
              HRETURN_ERROR(DFE_NOSPACE, FAIL);
      }

    if (sym == NULL)
      {
          sym = (char **) HDmalloc(VSFIELDMAX * sizeof(char *));
          if (sym == NULL)
              HRETURN_ERROR(DFE_NOSPACE, FAIL);

          for (i = 0; i < VSFIELDMAX; i++)
            {
                sym[i] = (char *) HDmalloc(sizeof(char) * (FIELDNAMELENMAX + 1));
                if (sym[i] == NULL)
                    HRETURN_ERROR(DFE_NOSPACE, FAIL);
            }
      }

#endif /* macintosh */

    if(slen>Vpbufsize)
      {
        Vpbufsize = (uint32)slen;
        if (Vpbuf)
            HDfree((VOIDP) Vpbuf);
        if ((Vpbuf = (uint8 *) HDmalloc(Vpbufsize)) == NULL)
            HRETURN_ERROR(DFE_NOSPACE, FAIL);
      } /* end if */

    HDstrcpy((char *)Vpbuf,attrs);
    s = (char *)Vpbuf;
    nsym = 0;

    s0 = s;
    while (*s)
      {

#ifdef VDATA_FIELDS_ALL_UPPER
          if (*s >= 'a' && *s <= 'z')
              *s = (char) toupper(*s);
#endif /* VDATA_FIELDS_ALL_UPPER */

          if (ISCOMMA(*s))
            {

                /* make sure we've got a legitimate length */
                len = (intn) (s - s0);
                if (len <= 0)
                    return (FAIL);

                /* save that token */
                ss = symptr[nsym] = sym[nsym];
                nsym++;

                /* shove the string into our static buffer.  YUCK! */
                if (len > FIELDNAMELENMAX)
                    len = FIELDNAMELENMAX;
                HIstrncpy(ss, s0, len + 1);

                /* skip over the comma */
                s++;

                /* skip over white space before the next field name */
                while (*s && *s == ' ')
                    s++;

                /* keep track of the first character of the next token */
                s0 = s;

            }
          else
            {

                /* move along --- nothing to see here */
                s++;
            }
      }

    /* save the last token */
    len = (intn) (s - s0);
    if (len <= 0)
        return (FAIL);
    ss = symptr[nsym] = sym[nsym];
    nsym++;

    if (len > FIELDNAMELENMAX)
        len = FIELDNAMELENMAX;
    HIstrncpy(ss, s0, len + 1);

    symptr[nsym] = NULL;
    *attrc = nsym;
    *attrv = (char **) symptr;

    return (SUCCEED);   /* ok */
}   /* scanattrs */

/*******************************************************************************
 NAME
    VPparse_shutdown  --  Free the Vpbuf buffer.

 DESCRIPTION
    For completeness, when the VSet interface is shut-down, free the Vpbuf.

    Should only ever be called by the "atexit" function HDFend

 RETURNS
    Returns SUCCEED/FAIL

*******************************************************************************/
intn
VPparse_shutdown(void)
{
    intn ret_value = SUCCEED;

    if(Vpbuf != NULL)
      {
        HDfree(Vpbuf);
        Vpbuf = NULL;
        Vpbufsize = 0;
      } /* end if */

    return ret_value;
} /* end VSPhshutdown() */

