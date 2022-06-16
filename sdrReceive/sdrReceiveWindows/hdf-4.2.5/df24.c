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
static char RcsId[] = "@(#)$Revision: 4924 $";
#endif

/* $Id: df24.c 4924 2007-09-05 21:55:40Z fbaker $ */

/*-----------------------------------------------------------------------------
 * File:     df24.c
 * Purpose:  read and write 24-bit raster images
 * Invokes:  dfgr.c
 * Contents:
 *  DF24getdims:     - get dimensions of image
 *  DF24reqil:       - use this interlace when returning image
 *  DF24getimage:    - read in image
 *  DF24setdims:     - set dimensions of image
 *  DF24setil:       - set interlace of image to write next
 *  DF24setcompress: - set the compression to use when writing out next image
 *  DF24restart:     - restart looking for 24-bit images in a file
 *  DF24addimage:    - append image to file
 *  DF24putimage:    - write image to a file
 *  DF24readref:     - set ref of 24-bit RIG to get next
 *  DF24lastref:     - return reference number of last RIG read or written
 *  DF24nimages:     - get number of images in file
 * Missing:
 *  DF24writeref: set ref of 24-bit RIG to write next
 *
 * Remarks: A RIG specifies attributes associated with an image- lookup table,
 *          dimension, compression, color compensation etc.
 *---------------------------------------------------------------------------*/

#include "hdf.h"
#include "dfgr.h"

static intn Newdata = 0;        /* does Readrig contain fresh data? */
static intn dimsset = 0;        /* have dimensions been set? */
static int32 last_xdim = 0;
static int32 last_ydim = 0;     /* .....gheesh......... */

#define LUT     0
#define IMAGE   1

/*--------------------------------------------------------------------------
 NAME
    DF24getdims -- get dimensions of next image RIG
 USAGE
    intn DF24getdims(filename,pxdim,pydim,pil)
        char *filename;         IN: the file to get retrieve dims. from
        int32 *pxdim,*pydim;    OUT: ptrs to the X&Y dims retrieved
        intn *pil;              OUT: ptr to the interlace for the image
 RETURNS
    SUCCEED on success, FAIL on failure.
 DESCRIPTION
    Gets the next image's dimensions and interlace from the file specified.
 GLOBAL VARIABLES
    last_xdim, last_ydim, Newdata
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
DF24getdims(const char *filename, int32 *pxdim, int32 *pydim, intn *pil)
{
  CONSTR(FUNC, "DF24getdims");
  intn        ncomps;
  intn       ret_value = SUCCEED;

  do
    {
      if (DFGRIgetdims(filename, pxdim, pydim, &ncomps, pil, IMAGE) < 0)
        HGOTO_ERROR(DFE_NODIM, FAIL);
    }
  while (ncomps != 3);

  last_xdim = *pxdim;
  last_ydim = *pydim;
  Newdata = 1;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */
  /* Normal function cleanup */


  return ret_value;
}   /* end DF24getdims() */

/*--------------------------------------------------------------------------
 NAME
    DF24reqil -- get next image with specified interlace
 USAGE
    intn DF24reqil(il)
        intn il;            IN: the interlace requested for the next image
 RETURNS
    SUCCEED on success, FAIL on failure.
 DESCRIPTION
    Requests that the next image be returned in a particular interlace scheme.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
DF24reqil(intn il)
{
  intn ret_value;

  ret_value = (DFGRIreqil(il, IMAGE));

  return ret_value;
}   /* end DF24reqil() */

/*--------------------------------------------------------------------------
 NAME
    DF24getimage -- get image from next RIG
 USAGE
    intn DF24getimage(filename,image,xdim,ydim)
        char *filename;     IN: file name to retrieve image from
        void * image;        OUT: buffer to store image in
        int32 xdim,ydim;    IN: dimensions of image buffer
 RETURNS
    SUCCEED on success, FAIL on failure.
 DESCRIPTION
    Retrieves the next 24-bit image from a file.  The image is stored
    according to the current interlace scheme and is wedged into the upper
    left corner of the buffer.
 GLOBAL VARIABLES
    Newdata, last_xdim, last_ydim
 COMMENTS, BUGS, ASSUMPTIONS
    image buffer is assumed to be 3*xdim*ydim bytes in size.
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
DF24getimage(const char *filename, void * image, int32 xdim, int32 ydim)
{
  CONSTR(FUNC, "DF24getimage");
  intn        il;
  int32       tx, ty;
  int compressed, has_pal;
  uint16 compr_type;
  intn         ret_value = SUCCEED;

  HEclear();

  if (!filename || !*filename || !image || (xdim <= 0) || (ydim <= 0))
    HGOTO_ERROR(DFE_ARGS, FAIL);

  if (!Newdata && DF24getdims(filename, &tx, &ty, &il) == FAIL)
    HGOTO_ERROR(DFE_NODIM, FAIL);

  if (Newdata)
    {
      tx = last_xdim;
      ty = last_ydim;
    }     /* end if */

  if ((tx > xdim) || (ty > ydim))
    HGOTO_ERROR(DFE_BADDIM, FAIL);

  ret_value = DFGRIgetimlut(filename, image, xdim, ydim, IMAGE, 0,
                            &compressed, &compr_type, &has_pal);

  Newdata = 0;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */
  /* Normal function cleanup */

  return ret_value;
}   /* end DF24getimage() */

/*--------------------------------------------------------------------------
 NAME
    DF24setdims -- set dimensions of image to write next
 USAGE
    intn DF24setdims(xdim,ydim)
        int32 xdim,ydim;    IN: the dimensions of the image to write next
 RETURNS
    SUCCEED on success, FAIL on failure.
 DESCRIPTION
    Sets the dimensions of the next image to write to a file.
 GLOBAL VARIABLES
    dimsset
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
DF24setdims(int32 xdim, int32 ydim)
{
  intn ret_value;

  dimsset = 1;
  ret_value = (DFGRIsetdims(xdim, ydim, 3, IMAGE));

  return ret_value;
}   /* end DF24setdims() */

/*--------------------------------------------------------------------------
 NAME
    DF24setil -- set interlace of image to write next
 USAGE
    intn DF24setil(il)
        intn il;            IN: the interlace of the image to write next
 RETURNS
    SUCCEED on success, FAIL on failure.
 DESCRIPTION
    Sets the interlace of the next image to write to a file.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
DF24setil(intn il)
{
  intn ret_value;

  ret_value = (DFGRIsetil(il, IMAGE));

  return ret_value;
}   /* end DF24setil() */

/*--------------------------------------------------------------------------
 NAME
    DF24setcompress -- set compression scheme for next 24-bit image
 USAGE
    intn DF24setcompress(type,cinfo)
        int32 type;         IN: compression scheme for next image
        comp_info *cinfo;   IN: additional compression information for
                                certain compression schemes (currently only
                                JPEG)
 RETURNS
    SUCCEED on success, FAIL on failure.
 DESCRIPTION
    Sets the compression scheme of the next image to write to a file.
    A list of the different schemes may be found in the hcomp.h header
    file.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
DF24setcompress(int32 type, comp_info * cinfo)
{
  intn ret_value;

  ret_value = (DFGRsetcompress(type, cinfo));

  return ret_value;
}   /* end DF24setcompress() */

/*--------------------------------------------------------------------------
 NAME
    DF24restart -- restart access to a file
 USAGE
    intn DF24restart(void)
 RETURNS
    SUCCEED on success, FAIL on failure.
 DESCRIPTION
    Restarts access to a file through the DF24 interface.  Next read/write
    will start with the first 24-bit image in the file.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
DF24restart(void)
{
  intn ret_value;

  ret_value = (DFGRIrestart());

  return ret_value;
}   /* end DF24restart() */

/*--------------------------------------------------------------------------
 NAME
    DF24addimage -- append 24-bit image to file
 USAGE
    intn DF24addimage(filename,image,xdim,ydim)
        char *filename;     IN: name of HDF file to write to
        void * image;        IN: Pointer to image data
        int32 xdim,ydim;    IN: Dimensions of image to write
 RETURNS
    SUCCEED on success, FAIL on failure.
 DESCRIPTION
    Appends a 24-bit raster image to an HDF file.  Will create the file
    if it doesn't exist.
 GLOBAL VARIABLES
    dimsset
 COMMENTS, BUGS, ASSUMPTIONS
    Array image is assumed to be xdim * ydim * 3 bytes
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
DF24addimage(const char *filename, const void * image, int32 xdim, int32 ydim)
{
  CONSTR(FUNC, "DF24addimage");
  intn ret_value = SUCCEED;

  /* 0 == C */
  if (!dimsset && DFGRIsetdims(xdim, ydim, 3, IMAGE) == FAIL)
    HGOTO_ERROR(DFE_BADDIM, FAIL);
  dimsset = 0;    /* reset to new rig */

  ret_value = (DFGRIaddimlut(filename, image, xdim, ydim, IMAGE, 0, 0));

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */
  /* Normal function cleanup */

  return ret_value;
}   /* end DF24addimage() */

/*--------------------------------------------------------------------------
 NAME
    DF24putimage -- write 24-bit image to file
 USAGE
    intn DF24addimage(filename,image,xdim,ydim)
        char *filename;     IN: name of HDF file to write to
        void * image;        IN: Pointer to image data
        int32 xdim,ydim;    IN: Dimensions of image to write
 RETURNS
    SUCCEED on success, FAIL on failure.
 DESCRIPTION
    Writes a 24-bit raster image to a new HDF file.  Will overwrite existing
    files if they exist.
 GLOBAL VARIABLES
    dimsset
 COMMENTS, BUGS, ASSUMPTIONS
    Array image is assumed to be xdim * ydim * 3 bytes
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
DF24putimage(const char *filename, const void * image, int32 xdim, int32 ydim)
{
  CONSTR(FUNC, "DF24putimage");
  intn ret_value = SUCCEED;

  /* 0 == C */
  if (!dimsset && DFGRIsetdims(xdim, ydim, 3, IMAGE) == FAIL)
    HGOTO_ERROR(DFE_BADDIM, FAIL);
  dimsset = 0;    /* reset to new rig */

  ret_value = (DFGRIaddimlut(filename, image, xdim, ydim, IMAGE, 0, 1));

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */
  /* Normal function cleanup */

  return ret_value;
}   /* end DF24putimage() */

/*--------------------------------------------------------------------------
 NAME
    DF24nimages -- determines number of 24-bit raster images in a file
 USAGE
    intn DF24nimages(filename)
        char *filename;     IN: name of HDF file to check
 RETURNS
    Number of images on success, FAIL on failure.
 DESCRIPTION
    Determines the number of unique 24-bit raster images in a file.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
DF24nimages(const char *filename)
{
  CONSTR(FUNC, "DF24nimages");
  int32       file_id;
  int32       group_id;       /* group ID for looking at RIG's */
  uint16      elt_tag, elt_ref;   /* tag/ref of items in a RIG */
  intn        nimages;        /* total number of potential images */
  uint16      find_tag, find_ref;     /* storage for tag/ref pairs found */
  int32       find_off, find_len;     /* storage for offset/lengths of tag/refs found */
  uint8       GRtbuf[64];     /* local buffer to read the ID element into */
  intn        ret_value = SUCCEED;

  HEclear();

  /* should use reopen if same file as last time - more efficient */
  if ((file_id = DFGRIopen(filename, DFACC_READ)) == FAIL)
    HGOTO_ERROR(DFE_BADOPEN, FAIL);

  /* go through the RIGs looking for 24-bit images */
  nimages = 0;
  find_tag = find_ref = 0;
  while (Hfind(file_id, DFTAG_RIG, DFREF_WILDCARD, &find_tag, &find_ref, &find_off, &find_len, DF_FORWARD) == SUCCEED)
    {
      /* read RIG into memory */
      if ((group_id = DFdiread(file_id, DFTAG_RIG, find_ref)) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);
      while (!DFdiget(group_id, &elt_tag, &elt_ref))
        {   /* get next tag/ref */
          if (elt_tag == DFTAG_ID)
            {     /* just look for ID tags to get the number of components */
              if (Hgetelement(file_id, elt_tag, elt_ref, GRtbuf) != FAIL)
                {
                  uint16      uint16var;
                  int32       temp;   /* temporary holding variable */
                  int16       ncomponents;    /* number of image components */
                  uint8      *p;

                  p = GRtbuf;
                  INT32DECODE(p, temp);
                  INT32DECODE(p, temp);
                  UINT16DECODE(p, uint16var);
                  UINT16DECODE(p, uint16var);
                  INT16DECODE(p, ncomponents);
                  if (ncomponents == 3)   /* whew, all that work and we finally found a 24-bit image */
                    nimages++;
                }   /* end if */
              else	{
              	DFdifree(group_id);
                HGOTO_ERROR(DFE_GETELEM, FAIL);
              }
            }     /* end if */
        }   /* end while */
    }     /* end while */

  if (Hclose(file_id) == FAIL)
    HGOTO_ERROR(DFE_CANTCLOSE, FAIL);

  ret_value = nimages;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

    /* Normal function cleanup */

  return ret_value;
}   /* end DF24nimages() */

/*--------------------------------------------------------------------------
 NAME
    DF24readref -- set ref # of 24-bit RIG to read next
 USAGE
    intn DF24readref(filename,ref)
        char *filename;     IN: name of HDF file
        uint16 ref;         IN: ref # of next 24-bit RIG to read
 RETURNS
    SUCCEED on success, FAIL on failure.
 DESCRIPTION
    Sets the ref # of the next 24-bit RIG to read from a file.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
DF24readref(const char *filename, uint16 ref)
{
  intn ret_value;

  ret_value = (DFGRreadref(filename, ref));

  return ret_value;
}   /* end DF24readref() */

/*--------------------------------------------------------------------------
 NAME
    DF24lastref -- return ref # of last read/written 24-bit RIG
 USAGE
    uint16 DF24lastref(void)
 RETURNS
    Last ref # on success, 0 on failure.
 DESCRIPTION
    Returns the last ref # of a 24-bit RIG read to or written from a file.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
uint16
DF24lastref(void)
{
  uint16 ret_value;

  ret_value = (DFGRIlastref());

  return ret_value;
}   /* end DF24lastref() */
