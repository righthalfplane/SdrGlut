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

/* $Id: dfcomp.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*-----------------------------------------------------------------------------
 * File:    dfcomp.c
 * Purpose: File compression
 * Invokes: df.c dfimcomp.c df.h
 * Contents:
 *  DFputcomp: compress image and write it to HDF file
 *  DFgetcomp: read compressed image from HDF file and decompress it
 *  DFCrle: compress string using run length encoding
 *  DFCunrle: decompress string using run length encoding
 * Remarks: DFgetcomp and DFputcomp constitute a general compression interface
 *---------------------------------------------------------------------------*/

/* This module (dfcomp.c) used to be in */
/* charge of the general compression information */
/* but hcomp.c now supercedes it. */
#include "hdf.h"

#define R8_MAX_BLOCKS 32
#define R8_MAX_LENGTH 512

/*-----------------------------------------------------------------------------
 * Name:    DFputcomp
 * Purpose: Compress and write images to HDF file
 * Inputs:  file_id: pointer to HDF file
 *          tag, ref: tag, ref of compressed image for writing out
 *          image: image to be compressed
 *          xdim, ydim: dimensions of image
 *          palette: palette associated with image
 *          newpal: modified palette, produced if compression scheme is IMCOMP
 *          scheme: compression scheme to be used
 *          cinfo: additional information needed for compression
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF programmers, DF8putrig, other routines
 * Invokes: DFCrle, DFCimcomp, DFaccess, DFwrite, DFIcheck
 * Remarks: IMCOMP modifies the palette associated with the image
 *          Hence the palette and newpal arguments
 *          This is a general compression interface - to be used anytime image
 *          compression is needed in HDF
 *          Note that rseq does its own compression, because that is part of
 *          the interactive color raster protocol
 *          The space needed for compression and decompression can be allocated
 *          statically or dynamically, depending on the DF_DYNAMIC flag, and
 *          for entire image or part of it (reused) depending on availability
 *          Accordingly, writing out is whole image, or row by row
 *          Note that compression is always row by row for RLE.
 *---------------------------------------------------------------------------*/

intn
DFputcomp(int32 file_id, uint16 tag, uint16 ref, const uint8 *image, int32 xdim,
          int32 ydim, uint8 *palette, uint8 *newpal, int16 scheme,
          comp_info * cinfo)
{
    CONSTR(FUNC, "DFputcomp");
    uint8      *buffer;         /* buffer to hold compressed image */
    const uint8      *in;       /* pointer to input for compression */
    uint8      *out;            /* pointer to space for compressed output */
    int32       cisize;         /* maximum size of compressed image */
    int32       crowsize;       /* maximum size of compressed row */
    intn        buftype;        /* buftype = 1: buffer enough for whole image */
    /* buftype = 2: buffer holds 1 row */
    int32       n;              /* number of compressed bytes produced */
    int32       total;          /* total compressed bytes produced so far */
    int32       i;
    int32       ret = 0;
    int32       aid = 0;

    if (!HDvalidfid(file_id) || !tag || !ref || xdim <= 0 || ydim <= 0 || !image)
        HRETURN_ERROR(DFE_ARGS, FAIL);

    switch (scheme)
      {
          case DFTAG_RLE:   /* RLE compression (8-bit or 24-bit(?) images */
              cisize = ydim * (xdim * 121 / 120 + 1);   /* 120 chars can compress to 121! */
              crowsize = xdim * 121 / 120 + 128;

              /* allocate buffer for compression */
              buffer = (uint8 *) HDmalloc((uint32) cisize);
              if (!buffer)
                {
                    buffer = (uint8 *) HDmalloc((uint32) crowsize);
                    if (!buffer)
                        HRETURN_ERROR(DFE_NOSPACE, FAIL);
                    buftype = 2;    /* compress and write out row by row */
                }
              else  /* can hold whole image, then write */
                  buftype = 1;

              in = image;
              out = buffer;
              n = total = 0;    /* no bytes compressed so far */

              if (buftype == 2)
                {
                    int32       num_blocks;
                    int32       block_length;

                    num_blocks = (ydim > (int32) R8_MAX_BLOCKS) ?
                        (int32) R8_MAX_BLOCKS : ydim;
                    block_length = (xdim > (int32) R8_MAX_LENGTH) ?
                        (int32) R8_MAX_LENGTH : xdim;
                    aid = HLcreate(file_id, tag, ref, block_length, num_blocks);
                    if (aid == FAIL)
                        return FAIL;
                }

              /* compress row by row */
              for (i = 0; i < ydim; i++)
                {
                    n = DFCIrle(in, out, xdim);     /* compress row */
                    in += xdim;     /* move input pointer */
                    total += n;     /* keep running total */
                    if (buftype == 1)   /* can hold whole image */
                        out = &buffer[total];   /* move out buffer pointer */
                    else
                      {     /* buffer too small, */
                          /* write out what was produced */
                          if (Hwrite(aid, n, buffer) == FAIL)
                            {
                                ret = FAIL;     /* flag value */
                                break;
                            }
                          out = buffer;     /* reset output pointer */
                      }
                }

              if (buftype == 1)
                {   /* write out entire image */
                    ret = Hputelement(file_id, tag, ref, buffer, total);
                    HDfree((VOIDP) buffer);
                }
              break;

          case DFTAG_IMC:   /* IMCOMP compression (8-bit images) */
              if (!palette || !newpal)  /* need palette and newpal */
                  HRETURN_ERROR(DFE_ARGS, FAIL);
              cisize = xdim * ydim / 4;     /* IMCOMP always cuts to 1/4 */

              buffer = (uint8 *) HDmalloc((uint32) cisize);
              if (!buffer)
                  HRETURN_ERROR(DFE_NOSPACE, FAIL);

              DFCIimcomp(xdim, ydim, image, buffer, palette, newpal, 0);
              ret = Hputelement(file_id, tag, ref, buffer, cisize);

              HDfree((VOIDP) buffer);
              break;

          case DFTAG_JPEG5:      /* JPEG compression (for 24-bit images) */
          case DFTAG_GREYJPEG5:      /* JPEG compression (for 8-bit images) */
              ret = DFCIjpeg(file_id, tag, ref, xdim, ydim, image, scheme, cinfo);
              break;

          default:      /* unknown compression scheme */
              HRETURN_ERROR(DFE_BADSCHEME, FAIL)
      }
    return ((intn) ret);
}   /* end DFputcomp() */

/*-----------------------------------------------------------------------------
 * Name:    DFgetcomp
 * Purpose: Read compressed image and decompress it
 * Inputs:  file_id: HDF file pointer
 *          tag, ref: id of image to be decompressed
 *          image: space to return decompressed image in
 *          xdim, ydim: dimensions of decompressed image
 *          scheme: compression scheme used
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF programmers, DF8getrig, other routines
 * Invokes: DFIcheck, DFIfind, DFaccess, DFread, DFCunrle, DFCunimcomp
 * Remarks: Will use dynamic/static memory allocation for buffer
 *          will read in image in parts if memory insufficient
 *          Decompression of rle is not necessarily row by row
 *          Other encodings can also be decoded with this
 *---------------------------------------------------------------------------*/

int
DFgetcomp(int32 file_id, uint16 tag, uint16 ref, uint8 *image, int32 xdim,
          int32 ydim, uint16 scheme)
{
    CONSTR(FUNC, "DFgetcomp");
    uint8      *buffer;
    uint8      *in;
    uint8      *out;
    int32       cisize, crowsize, buflen, bufleft;  /* bufleft: bytes left in buffer */

    int32       i;
    int32       totalread;
    int32       n;
    int32       aid;

    if (!HDvalidfid(file_id) || !tag || !ref || xdim <= 0 || ydim <= 0 || !image)
        HRETURN_ERROR(DFE_ARGS, FAIL);

    /* put this call up here instead of in switch statement, to make the */
    /* code easier to follow */
    if (scheme == DFTAG_JPEG5 || scheme == DFTAG_GREYJPEG5
            || scheme==DFTAG_JPEG || scheme==DFTAG_GREYJPEG)
        return (DFCIunjpeg(file_id, tag, ref, (VOIDP) image, xdim, ydim, (int16)scheme));

    /* Only do this stuff for non-JPEG compressed images */
    aid = Hstartread(file_id, tag, ref);
    if (aid == FAIL)
        HRETURN_ERROR(DFE_NOMATCH, FAIL);
    if (Hinquire(aid, (int32 *) NULL, (uint16 *) NULL, (uint16 *) NULL, &cisize,
    (int32 *) NULL, (int32 *) NULL, (int16 *) NULL, (int16 *) NULL) == FAIL)
        return FAIL;

    switch (scheme)
      {
          case DFTAG_RLE:
              crowsize = xdim * 121 / 120 + 128;    /* max size of a row */

              buffer = (uint8 *) HDmalloc((uint32) cisize);
              if (!buffer)
                {
                    buffer = (uint8 *) HDmalloc((uint32) crowsize);
                    if (!buffer)
                      {
                          Hendaccess(aid);
                          HRETURN_ERROR(DFE_NOSPACE, FAIL)
                      }     /* end if */
                    buflen = crowsize;
                }   /* end if */
              else
                  buflen = cisize;

              in = buffer;
              out = image;
              if ((n = Hread(aid, buflen, in)) < 0)
                {
                    HDfree((VOIDP) buffer);
                    Hendaccess(aid);
                    HRETURN_ERROR(DFE_READERROR, FAIL)
                }   /* end if */
              totalread = n;
              bufleft = n;
              for (i = 0; i < ydim; i++)
                {
                    n = DFCIunrle(in, out, xdim, !i);   /* no of bytes used up */
                    /* last arg=TRUE if i=0 - resets decompress */
                    in += n;
                    out += xdim;
                    bufleft -= n;
                    /* check if more bytes may be needed for next read */
                    if ((bufleft < crowsize) && (totalread < cisize))
                      {
                          HDmemcpy(buffer, in, (size_t) bufleft);
                          in = buffer;
                          if ((n = Hread(aid, buflen - bufleft, (uint8 *) &in[bufleft])) < 0)
                            {
                                HDfree((VOIDP) buffer);
                                Hendaccess(aid);
                                HRETURN_ERROR(DFE_READERROR, FAIL)
                            }   /* end if */
                          totalread += n;
                          bufleft += n;
                      }     /* end if */
                }   /* end for */

              Hendaccess(aid);
              HDfree((VOIDP) buffer);
              break;

          case DFTAG_IMC:
              crowsize = xdim;  /* size of compressed row */

              buffer = (uint8 *) HDmalloc((uint32) cisize);
              if (!buffer)
                {
                    buffer = (uint8 *) HDmalloc((uint32) crowsize);
                    if (!buffer)
                      {
                          Hendaccess(aid);
                          HRETURN_ERROR(DFE_NOSPACE, FAIL)
                      }     /* end if */
                    buflen = crowsize;
                }   /* end if */
              else
                  buflen = cisize;
              if (buflen >= cisize)
                {
                    if (Hread(aid, cisize, buffer) < cisize)
                      {
                          HDfree((VOIDP) buffer);
                          Hendaccess(aid);
                          HRETURN_ERROR(DFE_READERROR, FAIL)
                      }     /* end if */
                    /* HDfree(buffer); */
                    Hendaccess(aid);
                    DFCIunimcomp(xdim, ydim, buffer, image);
                    HDfree((VOIDP) buffer);
                    break;  /* go to end of switch */
                }   /* end if */

              in = buffer;  /* if can only read piecemeal */
              out = image;
              if ((n = Hread(aid, buflen, in)) < 0)
                {
                    HDfree((VOIDP) buffer);
                    Hendaccess(aid);
                    HRETURN_ERROR(DFE_READERROR, FAIL)
                }   /* end if */
              totalread = n;
              bufleft = n;
              for (i = 0; i < ydim; i += 4)
                {
                    DFCIunimcomp(xdim, (int32) 4, in, out);
                    in += xdim;
                    out += 4 * xdim;
                    bufleft -= xdim;
                    if ((bufleft < crowsize) && (totalread < cisize))
                      {
                          HDmemcpy(buffer, in, (size_t) bufleft);
                          in = buffer;
                          if ((n = Hread(aid, buflen - bufleft, (uint8 *) &in[bufleft])) < 0)
                            {
                                HDfree((VOIDP) buffer);
                                Hendaccess(aid);
                                HRETURN_ERROR(DFE_READERROR, FAIL)
                            }   /* end if */
                          totalread += n;
                          bufleft += n;
                      }     /* end if */
                }   /* end for */

              HDfree((VOIDP) buffer);
              Hendaccess(aid);
              break;

          default:      /* unknown scheme */
              HRETURN_ERROR(DFE_ARGS, FAIL)
      }     /* end switch */

    return SUCCEED;
}   /* end DFgetcomp() */
