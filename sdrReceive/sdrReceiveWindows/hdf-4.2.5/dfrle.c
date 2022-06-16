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

/* $Id: dfrle.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*-----------------------------------------------------------------------------
 * File:    dfrle.c
 * Purpose: RLE image compression algorithm
 * Invokes:
 * Contents:
 *  DFCIrle: compress string using run length encoding
 *  DFCIunrle: decompress string using run length encoding
 * Remarks: DFCIrle() and DFCIunrle() compress and decompress RLE encoded info
 *---------------------------------------------------------------------------*/

#include "hdf.h"

/*-----------------------------------------------------------------------------
 * Name:    DFCIrle
 * Purpose: compress a string of bytes
 * Inputs:  buf: buffer containing data to be compressed
 *          bufto: space for compressed data - assumed big enough
 *          len: number of bytes to compress
 * Returns: number of compressed bytes on success, -1 on failure
 * Users:   HDF programmers, DFputcomp, other routines
 * Invokes: none
 * Remarks: Written for efficiency
 *---------------------------------------------------------------------------*/

int32
DFCIrle(const void * buf, void * bufto, int32 len)
{
    const uint8 *p;
    const uint8 *q;
    uint8 *cfoll;
    uint8 *clead;
    const uint8 *begp;
    int32       i;

    p = buf;
    cfoll = (uint8 *) bufto;    /* place to copy to */
    clead = cfoll + 1;

    begp = p;
    while (len > 0)
      {     /* encode stuff until gone */

          q = p + 1;
          i = len - 1;
          while (i && i + 120 > len && *p == *q)
            {
                q++;
                i--;
            }

          if (q - p > 2)
            {   /* three in a row */
                if (p > begp)
                  {
                      *cfoll = (uint8) (p - begp);
                      cfoll = clead;
                  }
                *cfoll++ = (uint8) (128 | (uint8) (q - p));     /* len of seq */
                *cfoll++ = *p;  /* char of seq */
                len -= (int32)(q - p);   /* subtract len of seq */
                p = q;
                clead = cfoll + 1;
                begp = p;
            }
          else
            {
                *clead++ = *p++;    /* copy one char */
                len--;
                if (p - begp > 120)
                  {
                      *cfoll = (uint8) (p - begp);
                      cfoll = clead++;
                      begp = p;
                  }
            }

      }
/*
 *  fill in last bytecount
 */
    if (p > begp)
        *cfoll = (uint8) (p - begp);
    else
        clead--;    /* don't need count position */

    return ((int32) ((uint8 *) clead - (uint8 *) bufto));   /* how many encoded */
}

/*-----------------------------------------------------------------------------
 * Name:    DFCIunrle
 * Purpose: decompress run length encoding
 * Inputs:  buf: buffer containing compressed data
 *          bufto: space for returning decompressed data
 *          outlen: number of *decompressed* bytes desired.
 *          resetsave: don't use any stored state info - used for fresh image
 * Returns: number of compressed bytes used up on success, -1 on failure
 * Users:   HDF programmers, DFgetcomp, other routines
 * Invokes: none
 * Remarks: has been modified so it will decompress even non-rowwise compression
 *          Hence the static storage stuff
 *---------------------------------------------------------------------------*/

int32
DFCIunrle(uint8 *buf, uint8 *bufto, int32 outlen, int resetsave)
{
    int cnt;
    uint8 *p;
    uint8 *q;
    uint8      *endp;
    static uint8 save[255], *savestart = NULL, *saveend = NULL;
    /* save has a list of decompressed bytes not returned in
       previous call.  savestart and saveend specify the position
       at which this list starts and ends in the array save */

    p = (uint8 *) buf;
    endp = (uint8 *) bufto + outlen;
    q = (uint8 *) bufto;
    if (resetsave)
        savestart = saveend = save;     /* forget saved state */
    while ((saveend > savestart) && (q < endp))     /* copy saved stuff */
        *q++ = *savestart++;
    if (savestart >= saveend)
        savestart = saveend = save;     /* all copied */
    while (q < endp)
      {
          cnt = (int)*p++;   /* count field */
          if (!(cnt & 128))
            {   /* is set of uniques */
                while (cnt--)
                  {
                      if (q < endp)
                          *q++ = *p++;  /* copy unmodified */
                      else
                          *saveend++ = *p++;
                  }
            }
          else
            {
                cnt &= 127;     /* strip high bit */
                while (cnt--)
                  {
                      if (q < endp)
                          *q++ = *p;    /* copy unmodified */
                      else
                          *saveend++ = *p;
                  }
                p++;    /* skip that character */
            }
      }
    return ((int32) (p - buf));
}
