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

/* $Id: dfr8.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*-----------------------------------------------------------------------------
 * File:     dfr8.c
 * Purpose:  read and write 8-bit Raster Image Groups
 * Invokes:  df.c, dfcomp.c, dfgroup.c, dfrig.h
 * Contents:
 *  DFR8setpalette  : specify palette to be used with subsequent 8-bit images
 *  DFR8setcompress : Set the compression for next image written
 *  DFR8putimage    : write 8-bit image into an HDF file
 *  DFR8addimage    : append another 8-bit image to an HDF file
 *  DFR8getdims     : retrieve information about 8-bit image dimensions
 *  DFR8getimage    : retrieve 8-bit image and associated palette
 *  DFR8putrig      : write out a raster image group for 8-bit images
 *  DFR8getrig      : read in a raster image group for 8-bit images
 *  DFR8nimages     : number of images in HDF file
 *  DFR8readref     : get image with this reference number next
 *  DFR8writeref    : put image with this reference number next
 *  DFR8lastref     : return reference number of last element read or written
 *  DFR8restart     : forget info about last file accessed, restart from
 *                      beginning
 * Private:
 *  DFR8Iopen: open/reopen file
 *  DFR8Iriginfo: obtain info about next RIG/RI8 to get
 *  DFR8Iputimage   : internal routine that write 8-bit images to files
 * Remarks: A RIG specifies attributes associated with an image - palette,
 *          dimension, compression, color compensation etc.
 *          The palette for an 8-bit image is assumed to always be 768 bytes
 *          The palette is arranged as RGBRGB...
 *---------------------------------------------------------------------------*/

#include "hdf.h"
#include "dfrig.h"

/* Private Variables */
PRIVATE uint8 *paletteBuf = NULL;
PRIVATE uint16 Refset = 0;      /* Ref of image to get next */
PRIVATE uint16 Lastref = 0;     /* Last ref read/written */
PRIVATE uint16 Writeref = 0;    /* ref of next image to put in this file */
PRIVATE intn foundRig = -1;     /* -1: don't know if HDF file has RIGs
                                   0: No RIGs, try for RI8s etc.
                                   1: RIGs used, ignore RI8s etc. */
PRIVATE intn Newdata = 0;       /* does Readrig contain fresh data? */
PRIVATE intn Newpalette = -1;   /* -1 = no palette is associated
                                   0 = palette already written out
                                   1 = new palette, not yet written out */

PRIVATE intn CompressSet = FALSE;   /* Whether the compression parameters have
                                       been set for the next image */
PRIVATE int32 CompType = COMP_NONE;     /* What compression to use for the next
                                           image */
PRIVATE comp_info CompInfo;     /* Params for compression to perform */
PRIVATE char Lastfile[DF_MAXFNLEN];     /* last file opened */
PRIVATE DFRrig Readrig =
{                               /* information about RIG being read */
    NULL, 0, 0, (float32) 0.0, (float32) 0.0,
    {(float32) 0.0, (float32) 0.0, (float32) 0.0},
    {(float32) 0.0, (float32) 0.0, (float32) 0.0},
    {(float32) 0.0, (float32) 0.0, (float32) 0.0},
    {(float32) 0.0, (float32) 0.0, (float32) 0.0},
    {0, 0},
    {0, 0, 0, 0,
     {0, 0},
     {0, 0}},
    {0, 0},
    {0, 0, 0, 0,
     {0, 0},
     {0, 0}},
    {0, 0},
    {0, 0, 0, 0,
     {0, 0},
     {0, 0}},
};
PRIVATE DFRrig Writerig =
{                               /* information about RIG being written */
    NULL, 0, 0, (float32) 0.0, (float32) 0.0,
    {(float32) 0.0, (float32) 0.0, (float32) 0.0},
    {(float32) 0.0, (float32) 0.0, (float32) 0.0},
    {(float32) 0.0, (float32) 0.0, (float32) 0.0},
    {(float32) 0.0, (float32) 0.0, (float32) 0.0},
    {0, 0},
    {0, 0, 0, 0,
     {0, 0},
     {0, 0}},
    {0, 0},
    {0, 0, 0, 0,
     {0, 0},
     {0, 0}},
    {0, 0},
    {0, 0, 0, 0,
     {0, 0},
     {0, 0}},
};
PRIVATE DFRrig Zrig =
{                               /* empty RIG for initialization */
    NULL,
    0, 0, (float32) 0.0, (float32) 0.0,
    {(float32) 0.0, (float32) 0.0, (float32) 0.0},
    {(float32) 0.0, (float32) 0.0, (float32) 0.0},
    {(float32) 0.0, (float32) 0.0, (float32) 0.0},
    {(float32) 0.0, (float32) 0.0, (float32) 0.0},
    {0, 0},
    {0, 0, 0, 0,
     {0, 0},
     {0, 0}},
    {0, 0},
    {0, 0, 0, 0,
     {0, 0},
     {0, 0}},
    {0, 0},
    {0, 0, 0, 0,
     {0, 0},
     {0, 0}},
};

/* Whether we've installed the library termination function yet for this interface */
PRIVATE intn library_terminate = FALSE;

/* private functions */
PRIVATE intn DFR8Iputimage
            (const char *filename, const void * image, int32 xdim, int32 ydim, uint16 compress,
             intn append);

PRIVATE int32 DFR8Iopen
            (const char *filename, intn acc_mode);

PRIVATE intn DFR8Iriginfo
            (int32 file_id);

PRIVATE intn DFR8getrig
            (int32 file_id, uint16 ref, DFRrig * rig);

PRIVATE intn DFR8putrig
            (int32 file_id, uint16 ref, DFRrig * rig, intn wdim);

PRIVATE intn DFR8Istart(void);

/*--------------------------------------------------------------------------
 NAME
    DFR8setcompress -- set compression scheme for 8-bit image
 USAGE
    intn DFR8setcompress(type,cinfo)
        int32 type;             IN: the type of compression to perform on the
                                    next image
        comp_info *cinfo;       IN: ptr to compression information structure
 RETURNS
    SUCCEED on success, FAIL on failure.
 DESCRIPTION
    Sets the scheme used to compress the next 8-bit raster image written out
    with the DFR8 interface.

    Valid compression types available for this interface are listed in
    hcomp.h as COMP_nnnn.
 GLOBAL VARIABLES
    Uses the CompressSet, CompType and CompInfo global variables to store
    the information about the compression scheme.
 COMMENTS, BUGS, ASSUMPTIONS
    Only the JPEG compression type currently uses the cinfo structure.
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
DFR8setcompress(int32 type, comp_info * cinfo)
{
  CONSTR(FUNC, "DFR8setcompress");
  intn   ret_value = SUCCEED;

  /* Perform global, one-time initialization */
  if (library_terminate == FALSE)
      if(DFR8Istart()==FAIL)
          HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (type == COMP_NONE)
    {     /* quick check for no compression */
      CompType = 0;
      HGOTO_DONE(SUCCEED);
    }     /* end if */

  if (type < 0 || type > COMP_MAX_COMP || compress_map[type] == 0)
    HGOTO_ERROR(DFE_BADSCHEME, FAIL);

  CompressSet = TRUE;

  /* map JPEG compression into correct type of JPEG compression */
  if (type == COMP_JPEG)
  CompType = DFTAG_GREYJPEG5;
  else    /* otherwise, just use mapped tag */
    CompType = (int32)compress_map[type];
  CompInfo = (*cinfo);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* end DFR8setcompress() */

/*--------------------------------------------------------------------------
 NAME
    DFR8getdims -- get dimensions of next image from RIG, also if there is a
                    palette
 USAGE
    intn DFR8getdims(filename,pxdim,pydim,pispal)
        char *filename;         IN: name of HDF file
        int32 *pxdim, *pydim;   OUT: ptr to locations for returning X & Y dims
        intn *pispal;           OUT: ptr to location for returning if there is
                                    a palette
 RETURNS
    SUCCEED on success, FAIL on failure.
 DESCRIPTION
    Moves to the next 8-bit raster image in a file and returns the
    dimensions and whether there is a palette associated with it.

    Will also handle file with just raster-8 tags: RI8, CI8, ID8, IP8
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
DFR8getdims(const char *filename, int32 *pxdim, int32 *pydim, intn *pispal)
{
  CONSTR(FUNC, "DFR8getdims");
  int32       file_id=(-1);
  intn        ret_value = SUCCEED;

  HEclear();

  if (!filename || !*filename || !pxdim || !pydim)
    HGOTO_ERROR(DFE_ARGS, FAIL);

  /* Perform global, one-time initialization */
  if (library_terminate == FALSE)
      if(DFR8Istart()==FAIL)
          HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if ((file_id = DFR8Iopen(filename, DFACC_READ)) == FAIL)
    HGOTO_ERROR(DFE_BADOPEN, FAIL);

  if (DFR8Iriginfo(file_id) == FAIL)  /* reads next RIG or RI8 from file */
      HGOTO_ERROR(DFE_INTERNAL,FAIL);

  Newdata = 1;
  *pxdim = Readrig.descimage.xdim;
  *pydim = Readrig.descimage.ydim;
  if (pispal)
    *pispal = Readrig.lut.tag ? 1 : 0;  /* is there a palette */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  if(file_id!=(-1))
      Hclose(file_id);

  return ret_value;
}   /* end DFR8getdims() */

/*--------------------------------------------------------------------------
 NAME
    DFR8getimage -- get next image from a RIG, get palette also if desired
 USAGE
    intn DFR8getimage(filename,image,xdim,ydim,pal)
        char *filename;         IN: name of HDF file
        uint8 *image;           OUT: ptr to buffer to store image in
        int32 xdim,ydim;        IN: dims of space allocated by user for image
        uint8 *pal;             OUT: 768-byte space for palette, NULL if palette
                                    not wanted
 RETURNS
    SUCCEED on success, FAIL on failure.
 DESCRIPTION
    Reads the next 8-bit raster image from the file specified into the image
    buffer and it's associated palette into the palette buffer if the pal
    ptr is not NULL.

    Will also get RI8s and CI8s if no RIGs in file.

    Normally,DFR8getdims is called first and it finds next image to get.
    But if that is not called, DFR8getimage will itself find next image.

    Automatically decompresses images.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
DFR8getimage(const char *filename, uint8 *image, int32 xdim, int32 ydim, uint8 *pal)
{
  CONSTR(FUNC, "DFR8getimage");
  int32       file_id=(-1);
  intn        ret_value = SUCCEED;

  HEclear();

  if (!filename || !*filename || !image || (xdim <= 0) || (ydim <= 0))
    HGOTO_ERROR(DFE_ARGS, FAIL);

  /* Perform global, one-time initialization */
  if (library_terminate == FALSE)
      if(DFR8Istart()==FAIL)
          HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if ((file_id = DFR8Iopen(filename, DFACC_READ)) == FAIL)
    HGOTO_ERROR(DFE_BADOPEN, FAIL);

  if (!Newdata)
    {     /* if Readrig not fresh */
      if (DFR8Iriginfo(file_id) == FAIL)    /*reads next RIG or RI8 from file */
          HGOTO_ERROR(DFE_INTERNAL,FAIL);
    }     /* end if */
  Newdata = 0;    /* read new RIG next time */

  if ((Readrig.descimage.xdim > xdim) || (Readrig.descimage.ydim > ydim))
    HGOTO_ERROR(DFE_ARGS, FAIL);

    /* read image */
  if (Readrig.descimage.compr.tag)
    {     /* compressed image */
      if (DFgetcomp(file_id, Readrig.image.tag, Readrig.image.ref, image,
                    Readrig.descimage.xdim, Readrig.descimage.ydim,
                    Readrig.descimage.compr.tag) == FAIL)
          HGOTO_ERROR(DFE_INTERNAL,FAIL);
    }     /* end if */
  else
    {     /* non-compressed raster image */
      if (Hgetelement(file_id, Readrig.image.tag, Readrig.image.ref, image) == FAIL)
          HGOTO_ERROR(DFE_GETELEM,FAIL);
    }     /* end else */

  if (xdim > Readrig.descimage.xdim)
    {
      int32       off1, off2;
      int32       x, y;

      off1 = (Readrig.descimage.ydim - 1) * xdim;
      off2 = (Readrig.descimage.ydim - 1) * Readrig.descimage.xdim;
      for (y = Readrig.descimage.ydim - 1; y > 0; y--)
        {
          for (x = Readrig.descimage.xdim - 1; x >= 0; x--)
            image[off1 + x] = image[off2 + x];
          off1 -= xdim;
          off2 -= Readrig.descimage.xdim;
        }   /* end for */
    }     /* end for */

  if (pal && Readrig.lut.tag)
    {     /* read palette */
      if (Hgetelement(file_id, Readrig.lut.tag, Readrig.lut.ref, pal) == FAIL)
          HGOTO_ERROR(DFE_GETELEM,FAIL);
    }     /* end if */

  if((ret_value = Hclose(file_id))==FAIL)
    HGOTO_ERROR(DFE_CANTCLOSE, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */
        if(file_id!=(-1))
            Hclose(file_id);

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end DFR8getimage() */

/*--------------------------------------------------------------------------
 NAME
    DFR8setpalette -- set palette for subsequent images
 USAGE
    intn DFR8setpalette(pal)
        uint8 *pal;             IN: 768-byte buffer for palette to use for next
                                    image
 RETURNS
    SUCCEED on success, FAIL on failure.
 DESCRIPTION
    Stores a palette for use with further 8-bit raster images written
    through this interface.

    If pal is NULL, no palette is associated with subsequent images.
 GLOBAL VARIABLES
    paletteBuf, Writerig, Newpalette
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
DFR8setpalette(uint8 *pal)
{
  CONSTR(FUNC, "DFR8setpalette");
  intn     ret_value = SUCCEED;

  /* Perform global, one-time initialization */
  if (library_terminate == FALSE)
      if(DFR8Istart()==FAIL)
          HGOTO_ERROR(DFE_CANTINIT, FAIL);

  /* Check if paletteBuf buffer has been allocated */
  if (paletteBuf == NULL)
    {
      paletteBuf = (uint8 *) HDmalloc(768 * sizeof(uint8));
      if (paletteBuf == NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);
    }     /* end if */

  if (!pal)
    {
      Newpalette = -1;  /* no palette */
      Writerig.lut.tag = 0;
      Writerig.lut.ref = 0;     /* forget tag/ref of previous palette */
      Writerig.desclut.xdim = 0;
      Writerig.desclut.ncomponents = 0;
    }     /* end if */
  else
    {     /* store palette */
      HDmemcpy(paletteBuf, pal, 768);
      Newpalette = 1;
    }     /* end else */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* end DFR8setpalette() */

/*--------------------------------------------------------------------------
 NAME
    DFR8Iputimage -- Internal routine to write RIG to file
 USAGE
    intn DFR8Iputimage(filename, image, xdim, ydim, compress, append)
        char *filename;         IN: name of HDF file
        const void * image;            IN: ptr to buffer image is stored in
        int32 xdim,ydim;        IN: dims of space allocated by user for image
        uint16 compress;        IN: type of compression to store image with
        intn append;            IN: whether to (0) overwrite existing file, or
                                    (1) append image to file.
 RETURNS
    SUCCEED on success, FAIL on failure.
 DESCRIPTION
    Stores an image in an HDF file.  If a palette has been specified to use
    with 8-bit rasters, then it will be written to the file too and associated
    with the image.
 GLOBAL VARIABLES
    paletteBuf, Newpalette, Writeref, CompressSet, CompType, CompInfo, Lastref,
    Writerig
 COMMENTS, BUGS, ASSUMPTIONS
    Palette will be associated with image is isPalette is 1
    Palette will be written to file if not written before (Palref=0)
    Creates both RIG and RI8/CI8 tags, to accomodate older programs
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE intn
DFR8Iputimage(const char *filename, const void * image, int32 xdim, int32 ydim,
              uint16 compress, intn append)
{
  CONSTR(FUNC, "DFR8Iputimage");
  intn        acc_mode;       /* create if op 0, write if op 1 */
  int32       file_id=(-1);
  uint16      r8tag;          /* RIG and raster tags of image being written */
  uint8      *pal;            /* pointer to palette to be written */
  uint8       newpal[768];    /* Imcomp creates new palette to be associated */
  intn        wdim;           /* have dimensions already been written out? */
  intn        ret_value = SUCCEED;

  HEclear();

  if (!filename || !*filename || !image || (xdim <= 0) || (ydim <= 0))
    HGOTO_ERROR(DFE_ARGS, FAIL);

  /* Perform global, one-time initialization */
  if (library_terminate == FALSE)
      if(DFR8Istart()==FAIL)
          HGOTO_ERROR(DFE_CANTINIT, FAIL);

    /* Check if Palette buffer has been allocated */
  if (paletteBuf == NULL)
    {
      paletteBuf = (uint8 *) HDmalloc(768 * sizeof(uint8));
      if (paletteBuf == NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);
    }     /* end if */

  pal = (Newpalette >= 0) ? paletteBuf : NULL;
  acc_mode = append ? DFACC_WRITE : DFACC_CREATE;

  if ((file_id = DFR8Iopen(filename, acc_mode)) == FAIL)
    HGOTO_ERROR(DFE_BADOPEN, FAIL);

    /* write out image */
  if (compress || CompressSet)
    {
      /* if a compression type has been set, check if it's the same */
      if (CompressSet == FALSE || (compress > (uint16)1 && (int32)compress != CompType &&
                                   !(compress == (uint16)COMP_JPEG && CompType == (int32)DFTAG_GREYJPEG5)))
        {
          if ((int32)compress > COMP_MAX_COMP || compress_map[compress] == 0)
            HGOTO_ERROR(DFE_BADSCHEME, FAIL);
          /* map JPEG compression into correct type of JPEG compression */
          if (compress == COMP_JPEG)
            {
              CompType = DFTAG_GREYJPEG5;
              /* set up some sane JPEG params */
              CompInfo.jpeg.quality = 75;
              CompInfo.jpeg.force_baseline = TRUE;
            }     /* end if */
          else    /* otherwise, just use mapped tag */
            CompType = (int32)compress_map[compress];
        }   /* end if */
      if (!Writeref)
        if ((Writeref = Hnewref(file_id)) == 0)
          HGOTO_ERROR(DFE_NOREF, FAIL);

      if (DFputcomp(file_id, DFTAG_CI, Writeref, image, xdim, ydim,
                    pal, newpal, (int16) CompType, &CompInfo) == FAIL)
        HGOTO_ERROR(DFE_WRITEERROR, FAIL);
      Writerig.image.tag = DFTAG_CI;
      if (CompType == DFTAG_IMC)
        {
          pal = newpal;   /* Imcomp creates new pal */
          Newpalette = 1;     /* write out palette */
        }   /* end if */
    }     /* end if */
  else
    {     /* image need not be compressed */
      if (!Writeref)
        if ((Writeref = Hnewref(file_id)) == 0)
          HGOTO_ERROR(DFE_NOREF, FAIL);

      if (Hputelement(file_id, DFTAG_RI, Writeref, image, xdim * ydim) == FAIL)
        HGOTO_ERROR(DFE_PUTELEM, FAIL);
      Writerig.image.tag = DFTAG_RI;
    }     /* end else */
  Writerig.image.ref = Writeref;
  Writerig.descimage.ncomponents = 1;
  Writerig.aspectratio = (float32) 1.0;

    /* Write out Raster-8 tags for those who want it */
    if (CompType != DFTAG_GREYJPEG5)
      {
        r8tag = (uint16) (CompType ?
                          ((CompType == DFTAG_RLE) ? DFTAG_CI8 : DFTAG_II8) : DFTAG_RI8);
        if (Hdupdd(file_id, r8tag, Writeref, Writerig.image.tag, Writeref) == FAIL)
          HGOTO_ERROR(DFE_NOFREEDD, FAIL);
      }     /* end if */

  /* Write out palette */
  if (pal)
    {     /* if there is a palette */
      if (Newpalette == 1)
        {   /* write palette */
          if (Hputelement(file_id, DFTAG_LUT, Writeref, pal, (int32) 768) == FAIL)
            HGOTO_ERROR(DFE_PUTELEM, FAIL);
          Writerig.lut.tag = DFTAG_LUT;
          Writerig.lut.ref = Writeref;
          Writerig.desclut.xdim = 768;
          Writerig.desclut.ncomponents = 1;
        }   /* end if */
      if (CompType != DFTAG_IMC)
        Newpalette = 0;
      /* if IMCOMP, original palette not written out */

      /* put in Raster-8 stuff also, for those who want it */
      Hdeldd(file_id, DFTAG_IP8, Writeref);
      if (Hdupdd(file_id, DFTAG_IP8, Writeref, Writerig.lut.tag,
                 Writerig.lut.ref) == FAIL)
        HGOTO_ERROR(DFE_NOFREEDD, FAIL);
    }     /* end if */

  /* Write out RIG */
  if ((Writerig.descimage.xdim == xdim) && (Writerig.descimage.ydim == ydim) &&
      (Writerig.descimage.compr.tag == (uint16) CompType))
    wdim = 0;
  else
    {
      wdim = 1;
      Writerig.descimage.xdim = xdim;
      Writerig.descimage.ydim = ydim;
      Writerig.descimage.compr.tag = (uint16) CompType;
      Writerig.descimage.compr.ref = Writeref;
    }     /* end else */

  /* write ID, NT */
  if (DFR8putrig(file_id, Writeref, &Writerig, wdim) == FAIL)
    HGOTO_ERROR(DFE_WRITEERROR, FAIL);

  Lastref = Writeref;     /* remember ref written */

  Writeref = 0;   /* don't know ref to write next */
  CompressSet = FALSE;    /* Reset Compression flag and type */
  CompType = COMP_NONE;

  ret_value = Hclose(file_id);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */
      if(file_id!=(-1))
          Hclose(file_id);

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end DFR8Iputimage() */

/*--------------------------------------------------------------------------
 NAME
    DFR8putimage -- Write 8-bit raster image to HDF file
 USAGE
    intn DFR8putimage(filename, image, xdim, ydim, compress)
        char *filename;         IN: name of HDF file
        const void * image;            IN: ptr to buffer to store image in
        int32 xdim,ydim;        IN: dims of space allocated by user for image
        uint16 compress;        IN: type of compression to store image with
 RETURNS
    SUCCEED on success, FAIL on failure.
 DESCRIPTION
    Stores an image in an HDF file.  If a palette has been specified to use
    with 8-bit rasters, then it will be written to the file too and associated
    with the image.

    This function overwrites existing HDF files.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
DFR8putimage(const char *filename, const void * image, int32 xdim, int32 ydim,
             uint16 compress)
{
    CONSTR(FUNC, "DFR8putimage");    /* for HERROR */
    intn ret_value;

  /* Perform global, one-time initialization */
  if (library_terminate == FALSE)
      if(DFR8Istart()==FAIL)
          HGOTO_ERROR(DFE_CANTINIT, FAIL);

  ret_value = (DFR8Iputimage(filename, image, xdim, ydim, compress, 0));

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end DFR8putimage() */

/*--------------------------------------------------------------------------
 NAME
    DFR8addimage -- Append 8-bit raster image to HDF file
 USAGE
    intn DFR8putimage(filename, image, xdim, ydim, compress)
        char *filename;         IN: name of HDF file
        const void * image;            IN: ptr to buffer to store image in
        int32 xdim,ydim;        IN: dims of space allocated by user for image
        uint16 compress;        IN: type of compression to store image with
 RETURNS
    SUCCEED on success, FAIL on failure.
 DESCRIPTION
    Stores an image in an HDF file.  If a palette has been specified to use
    with 8-bit rasters, then it will be written to the file too and associated
    with the image.

    This function does not overwrite existing HDF files, just appends the
    to the file.  It will create the file if necessary.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
DFR8addimage(const char *filename, const void * image, int32 xdim, int32 ydim,
             uint16 compress)
{
    CONSTR(FUNC, "DFR8addimage");    /* for HERROR */
    intn ret_value;

  /* Perform global, one-time initialization */
  if (library_terminate == FALSE)
      if(DFR8Istart()==FAIL)
          HGOTO_ERROR(DFE_CANTINIT, FAIL);

  ret_value = (DFR8Iputimage(filename, image, xdim, ydim, compress, 1));

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end DFR8addimage() */

/*****************************************************************************/
/* This is the next lower layer - procedures to get and put a RIG. */
/* These are specific to 8-bit */
/*****************************************************************************/

/*--------------------------------------------------------------------------
 NAME
    DFR8getrig -- Read a RIG into memory
 USAGE
    intn DFR8getrig(file_id,ref,rig)
        int32 file_id;          IN: HDF file ID of file to retrieve RIG from
        uint16 ref;             IN: ref # of RIG to get
        DFRrig *rig;            OUT: ptr to RIG structure to place info in
 RETURNS
    SUCCEED on success, FAIL on failure.
 DESCRIPTION
    Retrieves a specific RIG from an HDF file.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    This function seems to be a low level routine, but could be exported.
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE intn
DFR8getrig(int32 file_id, uint16 ref, DFRrig * rig)
{
  CONSTR(FUNC, "DFR8getrig");
  uint16      elt_tag;
  uint16      elt_ref;
  uint8       ntstring[4];
  int32       GroupID;
  uint8       R8tbuf[64];
  intn        ret_value = SUCCEED;

  HEclear();

  if (!HDvalidfid(file_id) || !ref || !rig)
    HGOTO_ERROR(DFE_ARGS, FAIL);

  /* Perform global, one-time initialization */
  if (library_terminate == FALSE)
      if(DFR8Istart()==FAIL)
          HGOTO_ERROR(DFE_CANTINIT, FAIL);

    /* read RIG into memory */
  if ((GroupID = DFdiread(file_id, DFTAG_RIG, ref)) == FAIL)
    HGOTO_ERROR(DFE_BADGROUP, FAIL);

  *rig = Zrig;    /* fill rig with zeroes */
  while (DFdiget(GroupID, &elt_tag, &elt_ref) != FAIL)
    {
      /*get next tag/ref from RIG */
      switch (elt_tag)
        {   /* process tag/ref */
        case DFTAG_CI:
        case DFTAG_RI:
          rig->image.tag = elt_tag;   /* put tag/ref in struct */
          rig->image.ref = elt_ref;
          break;

        case DFTAG_LUT:
          rig->lut.tag = elt_tag;
          rig->lut.ref = elt_ref;
          break;

        case DFTAG_ID:      /* read description info */
          if (Hgetelement(file_id, elt_tag, elt_ref, R8tbuf) != FAIL)
            {
              uint8      *p;

              p = R8tbuf;
              INT32DECODE(p, rig->descimage.xdim);
              INT32DECODE(p, rig->descimage.ydim);
              UINT16DECODE(p, rig->descimage.nt.tag);
              UINT16DECODE(p, rig->descimage.nt.ref);
              INT16DECODE(p, rig->descimage.ncomponents);
              INT16DECODE(p, rig->descimage.interlace);
              UINT16DECODE(p, rig->descimage.compr.tag);
              UINT16DECODE(p, rig->descimage.compr.ref);
            }     /* end if */
          else
            {
              DFdifree(GroupID);
              ret_value = FAIL;
              goto done;
            }
          if (rig->descimage.ncomponents != 1)
            {
              DFdifree(GroupID);
              HGOTO_ERROR(DFE_BADCALL, FAIL);
            }
          if (rig->descimage.nt.tag == 0)
            break;  /* old RIGs */

          /* read NT */
          if (Hgetelement(file_id, rig->descimage.nt.tag,
                          rig->descimage.nt.ref, ntstring) == FAIL)
            {
              DFdifree(GroupID);
              HGOTO_ERROR(DFE_GETELEM, FAIL);
            }
          if ((ntstring[2] != 8) || (ntstring[1] != DFNT_UCHAR))
            {
              DFdifree(GroupID);
              HGOTO_ERROR(DFE_BADCALL, FAIL);
            }
          break;

        default:    /* ignore unknown tags */
          break;
        }   /* end switch */
    }     /* end while */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end DFR8getrig() */

/*--------------------------------------------------------------------------
 NAME
    DFR8putrig -- Write RIG struct out to HDF file
 USAGE
    intn DFR8putrig(file_id,ref,rig,wdim)
        int32 file_id;          IN: HDF file ID of file to put RIG into
        uint16 ref;             IN: ref # of RIG to put
        DFRrig *rig;            IN: ptr to RIG structure to write to file
        intn wdim;              IN: if (1) write new descr. records, (0)
                                    if records already written
 RETURNS
    SUCCEED on success, FAIL on failure.
 DESCRIPTION
    Writes a specific RIG to an HDF file.  If wdim is 1, then the ID & ID8
    records will be written also
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    This function seems to be a low level routine, but could be exported.
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE intn
DFR8putrig(int32 file_id, uint16 ref, DFRrig * rig, intn wdim)
{
  CONSTR(FUNC, "DFR8putrig");
  static uint16 prevdimref = 0;   /*ref of previous dimension record, to reuse */
  R8dim       im8dim;
  uint8       ntstring[4];
  int32       GroupID;
  uint8       R8tbuf[64];
  intn        ret_value = SUCCEED;

  HEclear();

  if (!HDvalidfid(file_id) || !ref)
    HGOTO_ERROR(DFE_ARGS, FAIL);

  /* Perform global, one-time initialization */
  if (library_terminate == FALSE)
      if(DFR8Istart()==FAIL)
          HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (!rig->descimage.nt.tag)
    {     /* construct and write out NT */
      ntstring[0] = DFNT_VERSION;   /* version */
      ntstring[1] = DFNT_UCHAR;     /* type */
      ntstring[2] = 8;  /* width: RIG data is 8-bit chars */
      ntstring[3] = DFNTC_BYTE;     /* class: data are numeric values */
      if (Hputelement(file_id, DFTAG_NT, ref, ntstring, (int32) 4) == FAIL)
        HGOTO_ERROR(DFE_PUTELEM, FAIL);
      rig->descimage.nt.tag = DFTAG_NT;
      rig->descimage.nt.ref = ref;
    }     /* end if */

  im8dim.xd = (uint16) rig->descimage.xdim;
  im8dim.yd = (uint16) rig->descimage.ydim;
  if (wdim)
    {
      uint8      *p;

      p = R8tbuf;
      INT32ENCODE(p, rig->descimage.xdim);
      INT32ENCODE(p, rig->descimage.ydim);
      UINT16ENCODE(p, rig->descimage.nt.tag);
      UINT16ENCODE(p, rig->descimage.nt.ref);
      INT16ENCODE(p, rig->descimage.ncomponents);
      INT16ENCODE(p, rig->descimage.interlace);
      UINT16ENCODE(p, rig->descimage.compr.tag);
      UINT16ENCODE(p, rig->descimage.compr.ref);
      if (Hputelement(file_id, DFTAG_ID, ref, R8tbuf, (int32) (p - R8tbuf)) == FAIL)
        HGOTO_ERROR(DFE_PUTELEM, FAIL);

      /* write out ID8 */
      p = R8tbuf;
      UINT16ENCODE(p, im8dim.xd);
      UINT16ENCODE(p, im8dim.yd);
      if (Hputelement(file_id, DFTAG_ID8, ref, R8tbuf, (int32) 4) == FAIL)
        HGOTO_ERROR(DFE_PUTELEM, FAIL);
      prevdimref = ref;
    }     /* end if */
  if (!prevdimref)
    HGOTO_ERROR(DFE_ARGS, FAIL);

    /* prepare to start writing rig */
    /* ### NOTE: the second parameter to this call may go away */
  if ((GroupID = DFdisetup(10)) == FAIL)
    HGOTO_ERROR(DFE_GROUPSETUP, FAIL);    /* max 10 tag/refs in set */

    /* add tag/ref to RIG - image description, image and palette */
  if (DFdiput(GroupID, DFTAG_ID, prevdimref) == FAIL)
    HGOTO_ERROR(DFE_PUTGROUP, FAIL);

  if (DFdiput(GroupID, rig->image.tag, rig->image.ref) == FAIL)
    HGOTO_ERROR(DFE_PUTGROUP, FAIL);

  if (rig->lut.ref && DFdiput(GroupID, rig->lut.tag, rig->lut.ref) == FAIL)
    HGOTO_ERROR(DFE_PUTGROUP, FAIL);

    /* write out RIG */
  if((ret_value = DFdiwrite(file_id, GroupID, DFTAG_RIG, ref))==FAIL)
    HGOTO_ERROR(DFE_GROUPWRITE, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end DFR8putrig() */

/*--------------------------------------------------------------------------
 NAME
    DFR8nimages -- Determines the number of 8-bit raster images in a file
 USAGE
    intn DFR8nimages(filename)
        char *filename;         IN: filename to check # of images
 RETURNS
    number of images on success, -1 on failure.
 DESCRIPTION
    Determines the number of unique 8-bit images in the file.  Only counts
    RIGs and RI8s which point to the same image once.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    Does not count 8-bit SDS datasets.  (Should not either!)
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
DFR8nimages(const char *filename)
{
  CONSTR(FUNC, "DFR8nimages");
  int32       file_id;
  int32       group_id;       /* group ID for looking at RIG's */
  uint16      elt_tag, elt_ref;   /* tag/ref of items in a RIG */
  intn        curr_image;     /* current image gathering information about */
  intn        nimages;        /* total number of potential images */
  int32       nrig, nri8, nci8;   /* number of RIGs, RI8s, and CI8s */
  int32      *img_off;        /* storage for an array of image offsets */
  uint16      rig_tag, rig_ref;   /* storage for tag/ref pairs of RIGs */
  intn        found_8bit;     /* indicates whether a RIG is an 8-bit RIG */
  uint16      find_tag, find_ref;     /* storage for tag/ref pairs found */
  int32       find_off, find_len;     /* storage for offset/lengths of tag/refs found */
  uint8       GRtbuf[64];     /* local buffer to read the ID element into */
  intn        i, j;           /* local counting variable */
  intn        ret_value = SUCCEED;

  HEclear();

  /* Perform global, one-time initialization */
  if (library_terminate == FALSE)
      if(DFR8Istart()==FAIL)
          HGOTO_ERROR(DFE_CANTINIT, FAIL);

  /* should use reopen if same file as last time - more efficient */
  file_id = DFR8Iopen(filename, DFACC_READ);
  if (file_id == FAIL)
    HGOTO_ERROR(DFE_BADOPEN, FAIL);

    /* In a completely psychotic file, there could be RIGs with no corresponding
       RI8s and also RI8s with no corresponding RIGs, so assume the worst
       case and then run through them all to eliminate matched pairs */
  nrig = Hnumber(file_id, DFTAG_RIG);     /* count the number of RIGS */
  if (nrig == FAIL)
    HGOTO_ERROR(DFE_INTERNAL, FAIL);
  nri8 = Hnumber(file_id, DFTAG_RI8);     /* add the number of RI8 and CI8s */
  if (nri8 == FAIL)
    HGOTO_ERROR(DFE_INTERNAL, FAIL);
  nci8 = Hnumber(file_id, DFTAG_CI8);
  if (nci8 == FAIL)
    HGOTO_ERROR(DFE_INTERNAL, FAIL);
  nimages = (intn) (nrig + nri8 + nci8);

  /* if there are no images just close the file and get out */
  if (nimages == 0)
    {
      if (Hclose(file_id) == FAIL)
        ret_value = FAIL;
      else
        ret_value = nimages;

      goto done; /* we are done */
    }

  /* Get space to store the image offsets */
  if ((img_off = (int32 *) HDmalloc(nimages * sizeof(int32))) == NULL)
    HGOTO_ERROR(DFE_NOSPACE, FAIL);

    /* go through the RIGs looking for 8-bit images */
  curr_image = 0;
  find_tag = find_ref = 0;
  while (Hfind(file_id, DFTAG_RIG, DFREF_WILDCARD, &find_tag, &find_ref, &find_off, &find_len, DF_FORWARD) == SUCCEED)
    {
      /* read RIG into memory */
      if ((group_id = DFdiread(file_id, DFTAG_RIG, find_ref)) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);
      found_8bit = FALSE;   /* initialize to no 8-bit image found */
      rig_tag = rig_ref = 0;    /* initialize bogus tag/ref */
      while (!DFdiget(group_id, &elt_tag, &elt_ref))
        {   /* get next tag/ref */
          if (elt_tag == DFTAG_ID)
            {     /* just look for ID tags to get the number of components */
              if (Hgetelement(file_id, elt_tag, elt_ref, GRtbuf) != FAIL)
                {
                  uint16      temp16;   /* temporary holding variable */
                  int32       temp;   /* temporary holding variable */
                  int16       ncomponents;    /* number of image components */
                  uint8      *p;

                  p = GRtbuf;
                  INT32DECODE(p, temp);
                  INT32DECODE(p, temp);
                  UINT16DECODE(p, temp16);
                  UINT16DECODE(p, temp16);
                  INT16DECODE(p, ncomponents);
                  if (ncomponents == 1)   /* whew, all that work and we finally found an 8-bit image */
                    found_8bit = TRUE;
                }   /* end if */
              else
                {
                  DFdifree(group_id);
                  HGOTO_ERROR(DFE_GETELEM, FAIL);
                }
            }     /* end if */
          else
            /* check for the image tag/ref */ if (elt_tag == DFTAG_CI || elt_tag == DFTAG_RI)
              {     /* keep for later */
                rig_tag = elt_tag;
                rig_ref = elt_ref;
              }     /* end if */
        }   /* end while */
      if (found_8bit)
        {   /* check for finding an 8-bit RIG */
          if ((uintn)rig_tag > (uintn)0 && (uintn)rig_ref > (uintn)0)
            {     /* make certain we found an image */
              img_off[curr_image] = Hoffset(file_id, rig_tag, rig_ref);     /* store offset */
              curr_image++;
            }     /* end if */
        }   /* end if */
    }     /* end while */

  /* go through the RI8s */
  find_tag = find_ref = 0;
  while (Hfind(file_id, DFTAG_RI8, DFREF_WILDCARD, &find_tag, &find_ref, &find_off, &find_len, DF_FORWARD) == SUCCEED)
    {
      img_off[curr_image] = find_off;   /* store offset */
      curr_image++;
    }     /* end while */

  /* go through the CI8s */
  find_tag = find_ref = 0;
  while (Hfind(file_id, DFTAG_CI8, DFREF_WILDCARD, &find_tag, &find_ref, &find_off, &find_len, DF_FORWARD) == SUCCEED)
    {
      img_off[curr_image] = find_off;   /* store offset */
      curr_image++;
    }     /* end while */

  nimages = curr_image;   /* reset the number of images we really have */
  for (i = 1; i < curr_image; i++)
    {     /* go through the images looking for duplicates */
      for (j = 0; j < i; j++)
        {
          if (img_off[i] == img_off[j])
            {
                nimages--;  /* if duplicate found, decrement the number of images */
                img_off[j]=(-1); /* mark as used, so we don't count it too... */
            } /* end if */
        }   /* end for */
    }     /* end for */

  HDfree(img_off);   /* free offsets */
  if (Hclose(file_id) == FAIL)
    HGOTO_ERROR(DFE_CANTCLOSE, FAIL);

  ret_value = nimages;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* end DFR8nimages() */

/*--------------------------------------------------------------------------
 NAME
    DFR8readref -- Set ref of image to get next
 USAGE
    intn DFR8readref(char *filename, uint16 ref)
        char *filename;         IN: filename to set read ref #
        uint16 ref;             IN: ref# of next image to read
 RETURNS
    SUCCEED on success, FAIL on failure.
 DESCRIPTION
    Sets the reference # of the RIG to read from next.
 GLOBAL VARIABLES
    Refset, Newdata
 COMMENTS, BUGS, ASSUMPTIONS
    Checks if image with this ref exists.
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
DFR8readref(const char *filename, uint16 ref)
{
  CONSTR(FUNC, "DFR8readref");
  int32       file_id=(-1);
  int32       aid;
  intn        ret_value = SUCCEED;

  HEclear();

  /* Perform global, one-time initialization */
  if (library_terminate == FALSE)
      if(DFR8Istart()==FAIL)
          HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if ((file_id = DFR8Iopen(filename, DFACC_READ)) == FAIL)
    HGOTO_ERROR(DFE_BADOPEN, FAIL);

  if ((aid = Hstartread(file_id, DFTAG_RIG, ref)) == FAIL
      && (aid = Hstartread(file_id, DFTAG_RI8, ref)) == FAIL
      && (aid = Hstartread(file_id, DFTAG_CI8, ref)) == FAIL)
    HGOTO_ERROR(DFE_NOMATCH, FAIL);

  Refset = ref;
  Newdata = 0;
  Hendaccess(aid);
  ret_value = Hclose(file_id);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */
      if(file_id!=(-1))
          Hclose(file_id);

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* end DFR8readref() */

/*--------------------------------------------------------------------------
 NAME
    DFR8writeref -- Set ref of image to put next
 USAGE
    intn DFR8writeref(char *filename, uint16 ref)
        char *filename;         IN: filename to set write ref #
        uint16 ref;             IN: ref# of next image to write
 RETURNS
    SUCCEED on success, FAIL on failure.
 DESCRIPTION
    Sets the reference # of the RIG to write to next.
 GLOBAL VARIABLES
    Writeref
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
DFR8writeref(const char *filename, uint16 ref)
{
    CONSTR(FUNC, "DFR8writeref");    /* for HERROR */
  intn  ret_value = SUCCEED;

  HEclear();

  /* Perform global, one-time initialization */
  if (library_terminate == FALSE)
      if(DFR8Istart()==FAIL)
          HGOTO_ERROR(DFE_CANTINIT, FAIL);

  /* shut compiler up */
  filename = filename;
  Writeref = ref;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end DFR8writeref() */

/*--------------------------------------------------------------------------
 NAME
    DFR8restart -- Restart reading/writing from beginning of file
 USAGE
    intn DFR8restart(void)
 RETURNS
    SUCCEED on success, FAIL on failure.
 DESCRIPTION
    Restarts reading and writing of RIGs from file from the beginning.
 GLOBAL VARIABLES
    Lastfile
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
DFR8restart(void)
{
    CONSTR(FUNC, "DFR8restart");    /* for HERROR */
    intn ret_value = SUCCEED;

  /* Perform global, one-time initialization */
  if (library_terminate == FALSE)
      if(DFR8Istart()==FAIL)
          HGOTO_ERROR(DFE_CANTINIT, FAIL);

  Lastfile[0] = '\0';

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end DFR8restart() */

/*--------------------------------------------------------------------------
 NAME
    DFR8lastref -- Return last ref # written or read
 USAGE
    uint16 DFR8lastref(void)
 RETURNS
    Ref # on success, 0 on failure.
 DESCRIPTION
    Returns the last ref # written to or read from.
 GLOBAL VARIABLES
    Lastref
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
uint16
DFR8lastref(void)
{
    CONSTR(FUNC, "DFR8lastref");    /* for HERROR */
    uint16 ret_value;

  /* Perform global, one-time initialization */
  if (library_terminate == FALSE)
      if(DFR8Istart()==FAIL)
          HGOTO_ERROR(DFE_CANTINIT, 0);

  ret_value = Lastref;

done:
  if(ret_value == 0)   /* 0 is invalid ref */
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end DFR8lastref() */

/*--------------------------------------------------------------------------
 * NAME
 *   DFR8getpalref - get the reference number of the palette
 * DESCRIPTION
 *   Convience function to get reference number of the palette of 
 *   last image. Must come after DFR8getdims() since it relies on
 *   this call to fill the Readrig structure
 * RETURNS
 *   SUCCEED.
--------------------------------------------------------------------------*/
intn
DFR8getpalref(uint16 *pal_ref)
{
  CONSTR(FUNC, "DFR8getpalref");
  intn        ret_value = SUCCEED;

  HEclear();

  /* Perform global, one-time initialization */
  if (library_terminate == FALSE)
      if(DFR8Istart()==FAIL)
          HGOTO_ERROR(DFE_CANTINIT, FAIL);

  *pal_ref = Readrig.lut.ref; /* ref of palette */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end DFR8getpalref() */

/*************************************************************************/
/*----------------------- Internal routines -----------------------------*/
/*************************************************************************/

/*--------------------------------------------------------------------------
 NAME
    DFR8Iopen -- open or reopen a file
 USAGE
    int32 DFR8Iopen(filename, acc_mode)
        char *filename;             IN: name of file to open
        intn acc_mode;                IN: access mode to open file with
 RETURNS
    HDF file ID on success, FAIL on failure
 DESCRIPTION
    Used to open/reopen a file for the DFR8 interface.
 GLOBAL VARIABLES
    Lastfile, foundRig, Refset, Newdata, Readrig, Writerig, Newpalette
 COMMENTS, BUGS, ASSUMPTIONS
    This is a hook for someday providing more efficient ways to
    reopen a file, to avoid re-reading all the headers.
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
DFR8Iopen(const char *filename, intn acc_mode)
{
  CONSTR(FUNC, "DFR8Iopen");
  int32       file_id;
  int32       ret_value = SUCCEED;

  /* use reopen if same file as last time - more efficient */
  if (HDstrncmp(Lastfile, filename, DF_MAXFNLEN) || (acc_mode == DFACC_CREATE))
    {
      /* treat create as different file */
      if ((file_id = Hopen(filename, acc_mode, 0)) == FAIL)
        HGOTO_ERROR(DFE_BADOPEN, FAIL);
      foundRig = -1;    /* don't know if any RIGs in file */
      Refset = 0;   /* no ref to get set for this file */
      Newdata = 0;
      Readrig = Zrig;   /* blank out read/write RIGs */
      Writerig = Zrig;
      if (Newpalette != (-1))
        Newpalette = 1;   /* need to write out palette */
    }     /* end if */
  else
    {
      if ((file_id = Hopen(filename, acc_mode, 0)) == FAIL)
        HGOTO_ERROR(DFE_BADOPEN, FAIL);
    }     /* end else */

  /* remember filename, so reopen may be used next time if same file */
  HDstrncpy(Lastfile, filename, DF_MAXFNLEN);

  ret_value = file_id;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end DFR8Iopen() */

/*--------------------------------------------------------------------------
 NAME
    DFR8Iriginfo -- Get information about next RIG or RI8 in file
 USAGE
    intn DFR8Iriginfo(file_id)
        int32 file_id;              IN: HDF file ID to read from
 RETURNS
    SUCCEED on success, FAIL on failure
 DESCRIPTION
    Reads in a RIGs structure into internal data structures, or if no RIGs
    are found, patches things together from RI8 information.

 GLOBAL VARIABLES

 COMMENTS, BUGS, ASSUMPTIONS
    if Refset is set, gets image with that ref, if any.
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE intn
DFR8Iriginfo(int32 file_id)
{
  CONSTR(FUNC, "DFR8Iriginfo");
  uint16      riref = 0, ciref = 0;
  int32       aid = FAIL;
  uint16      ref;
  uint8       R8tbuf[64];
  intn        ret_value = SUCCEED;

  HEclear();
  /* find next rig */
  if (foundRig)
    {     /* either RIGs present or don't know */
      if (!Refset && Readrig.image.ref)
        aid = Hstartread(file_id, DFTAG_RIG, Readrig.image.ref);
      do
        {
          if (Refset)
            aid = Hstartread(file_id, DFTAG_RIG, Refset);
          else
            {
              if (!Readrig.image.ref)
                aid = Hstartread(file_id, DFTAG_RIG, DFREF_WILDCARD);
              else
                {
                  if (aid != FAIL && Hnextread(aid, DFTAG_RIG, DFREF_WILDCARD,
                                               DF_CURRENT) == FAIL)
                    {
                      if(Hendaccess(aid)==FAIL)
                          HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);
                      aid = FAIL;
                    }     /* end if */
                }   /* end else */
            }     /* end else */
          if (aid == FAIL)
            {
              if (foundRig == 1)    /*RIGs present, but no more to return */
                HGOTO_ERROR(DFE_NOMATCH, FAIL);
              foundRig = 0;     /* No RIGs present in file */
            }     /* end if */

          /* RIG found */
          if (aid != FAIL)
            {
              Hinquire(aid, (int32 *) NULL, (uint16 *) NULL, &ref,
                       (int32 *) NULL, (int32 *) NULL, (int32 *) NULL,
                       (int16 *) NULL, (int16 *) NULL);
              if (DFR8getrig(file_id, ref, &Readrig) == FAIL)
                {
                  if (Refset || (HEvalue(1) != DFE_BADCALL))
                    {
                      Refset = 0;
                      if(Hendaccess(aid)==FAIL)
                          HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);
                      HGOTO_ERROR(DFE_BADRIG, FAIL);
                    }     /* end if */
                  Readrig.image.ref = ref;
                }   /* end if */
              else
                {
                  foundRig = 1;
                  Refset = 0;
                }   /* end else */
            }     /* end if */
        } while ((aid != FAIL) && (HEvalue(1) == DFE_BADCALL));
      if (aid != FAIL)
        if(Hendaccess(aid)==FAIL)
          HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);
    }     /* end if */
  if (Refset || !foundRig)
    {     /* No RIGs present, look for RI8 and CI8 */
      /* look for Refset if DFR8ref called, else look for next ref */
      if (Refset)
        aid = Hstartread(file_id, DFTAG_RI8, Refset);
      else
        {
          if (Readrig.image.ref)
            {
              aid = Hstartread(file_id, DFTAG_RI8, Readrig.image.ref);
              if (aid != FAIL && Hnextread(aid, DFTAG_RI8, DFREF_WILDCARD,
                                           DF_CURRENT) == FAIL)
                {
                  Hendaccess(aid);
                  aid = FAIL;
                }   /* end if */
            }     /* end if */
          else
            aid = Hstartread(file_id, DFTAG_RI8, DFREF_WILDCARD);
        }   /* end else */
      if (aid != FAIL)
        {
          Hinquire(aid, (int32 *) NULL, (uint16 *) NULL, &riref,
                   (int32 *) NULL, (int32 *) NULL, (int32 *) NULL,
                   (int16 *) NULL, (int16 *) NULL);
          if(Hendaccess(aid)==FAIL)
              HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);
        }   /* end if */

      if (Refset)
        aid = Hstartread(file_id, DFTAG_CI8, Refset);
      else
        {
          if (Readrig.image.ref)
            {
              aid = Hstartread(file_id, DFTAG_CI8, Readrig.image.ref);
              if (aid != FAIL && Hnextread(aid, DFTAG_CI8, DFREF_WILDCARD,
                                           DF_CURRENT) == FAIL)
                {
                  if(Hendaccess(aid)==FAIL)
                      HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);
                  aid = FAIL;
                }   /* end if */
            }     /* end if */
          else
            aid = Hstartread(file_id, DFTAG_CI8, DFREF_WILDCARD);
        }   /* end else */
      if (aid != FAIL)
        {
          Hinquire(aid, (int32 *) NULL, (uint16 *) NULL, &ciref,
                   (int32 *) NULL, (int32 *) NULL, (int32 *) NULL,
                   (int16 *) NULL, (int16 *) NULL);
          if(Hendaccess(aid)==FAIL)
              HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);
        }   /* end if */

      Refset = 0;
      if (!riref && !ciref)
        HGOTO_ERROR(DFE_NOMATCH, FAIL);
      if ((!ciref) || (riref && (riref < ciref)))
        {   /* next image is RI8 */
          Readrig.image.ref = riref;
          Readrig.image.tag = DFTAG_RI8;
        }   /* end if */
      else
        {   /* next image is CI8 */
          Readrig.image.ref = ciref;
          Readrig.image.tag = DFTAG_CI8;
          Readrig.descimage.compr.tag = DFTAG_RLE;
        }   /* end else */

      if (Hgetelement(file_id, DFTAG_ID8, Readrig.image.ref, R8tbuf) != FAIL)
        {
          uint8      *p;
          uint16      uint16var;

          p = R8tbuf;
          UINT16DECODE(p, uint16var);
          Readrig.descimage.xdim=(int32)uint16var;
          UINT16DECODE(p, uint16var);
          Readrig.descimage.ydim=(int32)uint16var;
        }   /* end if */
      else
        HGOTO_ERROR(DFE_GETELEM, FAIL);

      if (Hexist(file_id, DFTAG_IP8, Readrig.image.ref) != FAIL)
        {
          Readrig.lut.tag = DFTAG_IP8;
          Readrig.lut.ref = Readrig.image.ref;
        }   /* end if */
    }     /* end if */
  Lastref = Readrig.image.ref;    /* remember ref read */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end DFR8Iriginfo() */

/*--------------------------------------------------------------------------
 NAME
    DFR8Istart
 PURPOSE
    DFR8-level initialization routine
 USAGE
    intn DFR8Istart()
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    Register the shut-down routine (DFR8Pshutdown) for call with atexit
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE intn DFR8Istart(void)
{
    CONSTR(FUNC, "DFR8Istart");    /* for HERROR */
    intn        ret_value = SUCCEED;

    /* Don't call this routine again... */
    library_terminate = TRUE;

    /* Install atexit() library cleanup routine */
    if (HPregister_term_func(&DFR8Pshutdown) != 0)
      HGOTO_ERROR(DFE_CANTINIT, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
    return(ret_value);
} /* end DFR8Istart() */

/*--------------------------------------------------------------------------
 NAME
    DFR8Pshutdown
 PURPOSE
    Terminate various static buffers.
 USAGE
    intn DFR8shutdown()
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    Free various buffers allocated in the DFR8 routines.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    Should only ever be called by the "atexit" function HDFend
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn DFR8Pshutdown(void)
{
    if(paletteBuf!=NULL)
      {
          HDfree(paletteBuf);
          paletteBuf=NULL;
      } /* end if */
    return(SUCCEED);
} /* end DFR8Pshutdown() */


