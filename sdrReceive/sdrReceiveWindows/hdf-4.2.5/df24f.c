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

/* $Id: df24f.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*-----------------------------------------------------------------------------
 * File:     df24F.c
 * Purpose:  read and write 24-bit raster images
 * Invokes:  dfgr.c df24.c
 * Contents:
 *  d2reqil: use this interlace when returning image
 *  df24reqil: use this interlace when returning image
 *  d2sdims: set dimensions of image
 *  df24setdims: set dimensions of image
 *  d2setil: set interlace for image
 *  df24setil: set interlace for image
 *  d2first: restart 24 bit raster
 *  df24restart: restart 24 bit raster
 *  d2igdim: get dimensions of image
 *  d2igimg: read in image
 *  d2iaimg: write out image
 *  d2lref: last ref number
 *  d2scomp: set compression to use (short name)
 *  df24setcompress: set compression to use (long name)
 *  d2sjpeg:  set JPEG parameters (short name)
 *  df24setJPEG: set JPEG parameters (long name)
 *
 * Remarks:A RIG specifies attributes associated with an image - lookup table,
 *          dimension, compression, color compensation etc.
 *---------------------------------------------------------------------------*/

#include "hdf.h"
#include "dfgr.h"
#include "hproto_fortran.h"

#define LUT     0
#define IMAGE   1

static int  dimsset = 0;

/*-----------------------------------------------------------------------------
 * Name:    d2reqil
 * Purpose: get next image with specified interlace
 * Inputs:  il: interlace to get next image with
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DFGRIreqil
 * Remarks: none
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nd2reqil(intf * il)
{
    return (DFGRIreqil((intn) *il, (intn) IMAGE));
}

/*-----------------------------------------------------------------------------
 * Name:    d2sdims
 * Purpose: set dimensions of image to write next
 * Inputs:  xdim, ydim: dimensions of image
 *          il: interlace of image
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DFGRIsetdims
 * Remarks: none
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nd2sdims(intf * xdim, intf * ydim)
{
    dimsset = 1;
    return (DFGRIsetdims(*xdim, *ydim, 3, IMAGE));
}

/*-----------------------------------------------------------------------------
 * Name:    d2igdim
 * Purpose: get dimensions of next image RIG
 * Inputs:  filename: name of HDF file
 *          pxdim, pydim: pointer to locations for returning x,y dimensions
 *          pil: location for returning interlace of image in file
 *          fnlen: length of filename
 * Returns: 0 on success, -1 on failure with DFerror set
 *          *pxdim, *pydim, *pil set on success
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DF24getdims
 * Remarks: none
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nd2igdim(_fcd filename, intf * pxdim, intf * pydim, intf * pil, intf * fnlen)
{
    char       *fn;
    intf        ret;

    fn = HDf2cstring(filename, (intn) *fnlen);
    if (!fn)
	return(-1);
    ret = DF24getdims(fn, (int32 *) pxdim, (int32 *) pydim, (intn *) pil);
    HDfree((VOIDP) fn);
    return (ret);
}

/*-----------------------------------------------------------------------------
 * Name:    d2igimg
 * Purpose: get image from next RIG
 * Inputs:  filename: name of HDF file
 *          image: pointer to space to return image
 *          xdim, ydim: dimensions of space to return image
 *          fnlen: length of filename
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DFGRIgetimlut
 * Remarks: space is assumed to be xdim * ydim * 3 bytes
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nd2igimg(_fcd filename, _fcd image, intf * xdim, intf * ydim, intf * fnlen)
{
    char       *fn;
    intf        ret;

    fn = HDf2cstring(filename, (intn) *fnlen);
    if (!fn)
	return(-1);
    ret = DF24getimage(fn, (VOIDP) _fcdtocp(image), *xdim, *ydim);
    HDfree((VOIDP) fn);
    return (ret);
}

/*-----------------------------------------------------------------------------
 * Name:    d2iaimg
 * Purpose: Write out image
 * Inputs:  filename: name of HDF file
 *          image: image to write
 *          xdim, ydim: dimensions of array image
 *          fnlen: length of filename
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DFGRIaddimlut
 * Remarks: array image is assumed to be xdim * ydim * ncomps bytes
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nd2iaimg(_fcd filename, _fcd image, intf * xdim, intf * ydim, intf * fnlen,
         intf * newfile)
{
    char       *fn;
    intf        ret;

    if (!dimsset)
        if (DFGRIsetdims(*xdim, *ydim, 3, IMAGE) < 0)
            return (-1);

    fn = HDf2cstring(filename, (intn) *fnlen);
    if (!fn)
	return(-1);
    ret = DFGRIaddimlut(fn, (VOIDP) _fcdtocp(image), *xdim, *ydim,
                        IMAGE, 1, (intn) *newfile);
    HDfree((VOIDP) fn);
    return (ret);
}

/*-----------------------------------------------------------------------------
 * Name:    d2setil
 * Purpose: set interlace store with following images
 * Inputs:  il: interlace to set
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DFGRIsetil
 * Remarks: none
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nd2setil(intf * il)
{
    return (DFGRIsetil((intn) *il, IMAGE));
}

/*-----------------------------------------------------------------------------
 * Name:    d2first
 * Purpose: restart 24 bit raster file
 * Inputs:
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DFGRIrestart
 * Remarks: none
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nd2first(void)
{
    return (DFGRIrestart());
}

/*-----------------------------------------------------------------------------
 * Name:    d2lref
 * Purpose: return last reference number
 * Inputs:
 * Returns: last ref number
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DFGRIrestart
 * Remarks: none
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nd2lref(void)
{
    return ((intf)DFGRIlastref());
}

/*-----------------------------------------------------------------------------
 * Name:    d2scomp
 * Purpose: set the compression to use when writing the next image
 * Inputs:
 *      scheme - the type of compression to use
 * Returns: 0 on success, -1 for error
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DF24setcompress
 * Remarks: if the compression scheme is JPEG, this routine sets up default
 *          JPEG parameters to use, if a user wants to change them, d2sjpeg
 *          must be called.
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nd2scomp(intf * scheme)
{
    comp_info   cinfo;          /* Structure containing compression parameters */

    if (*scheme == COMP_JPEG)
      {     /* check for JPEG compression and set defaults */
          cinfo.jpeg.quality = 75;
          cinfo.jpeg.force_baseline = 1;
      }     /* end if */
    return (DF24setcompress((int32) *scheme, &cinfo));
}   /* end d2scomp() */

/*-----------------------------------------------------------------------------
 * Name:    d2sjpeg
 * Purpose: change the JPEG compression parameters
 * Inputs:
 *      quality - what the JPEG quality rating should be
 *      force_baseline - whether to force a JPEG baseline file to be written
 * Returns: 0 on success, -1 for error
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DF24setcompress
 * Remarks: none
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nd2sjpeg(intf * quality, intf * force_baseline)
{
    comp_info   cinfo;          /* Structure containing compression parameters */

    cinfo.jpeg.quality = (intn) *quality;
    cinfo.jpeg.force_baseline = (intn) *force_baseline;
    return (DF24setcompress((int32) COMP_JPEG, &cinfo));
}   /* end d2sjpeg() */

/*-----------------------------------------------------------------------------
 * Name:    df24reqil
 * Purpose: get next image with specified interlace
 * Inputs:  il: interlace to get next image with
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DFGRIreqil
 * Remarks: none
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndf24reqil(intf * il)
{
    return (DFGRIreqil((intn) *il, IMAGE));
}

/*-----------------------------------------------------------------------------
 * Name:    df24setdims
 * Purpose: set dimensions of image to write next
 * Inputs:  xdim, ydim: dimensions of image
 *          il: interlace of image
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DFGRIsetdims
 * Remarks: none
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndf24setdims(intf * xdim, intf * ydim)
{
    dimsset = 1;
    return (DFGRIsetdims(*xdim, *ydim, 3, IMAGE));
}

/*-----------------------------------------------------------------------------
 * Name:    df24setil
 * Purpose: set interlace store with following images
 * Inputs:  il: interlace to set
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DFGRIsetil
 * Remarks: none
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndf24setil(intf * il)
{
    return (DFGRIsetil((intn) *il, IMAGE));
}

/*-----------------------------------------------------------------------------
 * Name:    df24restart
 * Purpose: restart 24 bit raster file
 * Inputs:
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DFGRIrestart
 * Remarks: none
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndf24restart(void)
{
    return (DFGRIrestart());
}

/*-----------------------------------------------------------------------------
 * Name:    df24scompress
 * Purpose: set the compression to use when writing the next image
 * Inputs:
 *      scheme - the type of compression to use
 * Returns: 0 on success, -1 for error
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DF24setcompress
 * Remarks: if the compression scheme is JPEG, this routine sets up default
 *          JPEG parameters to use, if a user wants to change them, df24setjpeg
 *          must be called.
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndf24scompress(intf * scheme)
{
    comp_info   cinfo;          /* Structure containing compression parameters */

    if (*scheme == COMP_JPEG)
      {     /* check for JPEG compression and set defaults */
          cinfo.jpeg.quality = 75;
          cinfo.jpeg.force_baseline = 1;
      }     /* end if */
    return (DF24setcompress((int32) *scheme, &cinfo));
}   /* end df24setcompress() */

/*-----------------------------------------------------------------------------
 * Name:    df24sjpeg
 * Purpose: change the JPEG compression parameters
 * Inputs:
 *      quality - what the JPEG quality rating should be
 *      force_baseline - whether to force a JPEG baseline file to be written
 * Returns: 0 on success, -1 for error
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DF24setcompress
 * Remarks: none
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndf24sjpeg(intf * quality, intf * force_baseline)
{
    comp_info   cinfo;          /* Structure containing compression parameters */

    cinfo.jpeg.quality = (intn) *quality;
    cinfo.jpeg.force_baseline = (intn) *force_baseline;
    return (DF24setcompress((int32) COMP_JPEG, &cinfo));
}   /* end df24setjpeg() */

/*-----------------------------------------------------------------------------
 * Name:    d2irref
 * Purpose: Internal stub for setting ref of rig to read next
 * Inputs:  filename: name of HDF file
 *          ref: reference
 *          fnlen: length of filename
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DFGRreadref
 * Remarks:
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nd2irref(_fcd filename, intf * ref, intf * fnlen)
{
    char       *fn;
    intf        ret;

    fn = HDf2cstring(filename, (intn) *fnlen);
    if (!fn)
	return(-1);
    ret = DFGRreadref(fn, (uint16) *ref);
    HDfree((VOIDP) fn);
    return (ret);
}

/*-----------------------------------------------------------------------------
 * Name:    d2inimg
 * Purpose: Internal stub for getting the number of 24-bit raster images
 * Inputs:  filename: name of HDF file
 *          fnlen: length of filename
 * Returns: # of images on success, -1 on failure with error stack set
 * Users:   HDF HLL (high-level library) users, utilities, other routines
 * Invokes: DF24nimages
 * Remarks:
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nd2inimg(_fcd filename, intf * fnlen)
{
    char       *fn;
    intf        ret;

    fn = HDf2cstring(filename, (intn) *fnlen);
    if (!fn)
	return(-1);
    ret = DF24nimages(fn);
    HDfree((VOIDP) fn);
    return (ret);
} /* end nd2inimg */

