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

/* $Id: dfconv.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*------------------------------------------------------------------
 File:  dfconv.c

 Purpose:
    Routines to support conversion to and from HDF format

 Invokes:

 PRIVATE conversion functions: All of these are now in seperate files!
    dfknat.c
    DFKnb1b -  Native mode for 8 bit integers
    DFKnb2b -  Native mode for 16 bit integers
    DFKnb4b -  Native mode for 32 bit integers and floats
    DFKnb8b -  Native mode for 64 bit floats
    dfkswap.c
    DFKsb2b -  Byte swapping for 16 bit integers
    DFKsb4b -  Byte swapping for 32 bit integers
    DFKsb8b -  Byte swapping for 64 bit floats
    dfkcray.c
    DFKui2i -  Unicos routine for importing 16 bit unsigned integers
    DFKui2s -  Unicos routine for importing 16 bit signed integers
    DFKuo2i -  Unicos routine for exporting 16 bit integers (both)
    DFKui4i -  Unicos routine for importing unsigned 32bit integers
    DFKui4s -  Unicos routine for importing signed 32bit integers
    DFKuo4i -  Unicos routine for exporting 4 byte integers (both)
    DFKui4f -  Unicos routine for importing 32 bit floats
    DFKuo4f -  Unicos routine for exporting 32 bit floats
    DFKui8f -  Unicos routine for importing 64 bit floats
    DFKuo8f -  Unicos routine for exporting 64 bit floats
    DFKlui2i-  Unicos routine for importing little-endian 16 bit unsigned ints
    DFKlui2s-  Unicos routine for importing little-endian 16 bit signed ints
    DFKluo2i-  Unicos routine for exporting little-endian 16 bit ints (both)
    DFKlui4i-  Unicos routine for importing little-endian unsigned 32bit ints
    DFKlui4s-  Unicos routine for importing little-endian signed 32bit ints
    DFKluo4i-  Unicos routine for exporting little-endian 4 byte ints (both)
    DFKlui4f-  Unicos routine for importing little-endian 32 bit floats
    DFKluo4f-  Unicos routine for exporting little-endian 32 bit floats
    DFKlui8f-  Unicos routine for importing little-endian 64 bit floats
    DFKluo8f-  Unicos routine for exporting little-endian 64 bit floats
    dfkvms.c
    DFKvi4f -  VMS routine for importing 32 bit floats
    DFKvo4f -  VMS routine for exporting 32 bit floats
    DFKvi8f -  VMS routine for importing 64 bit floats
    DFKvo8f -  VMS routine for exporting 64 bit floats
    DFKlvi4f-  VMS routine for importing little-endian 32 bit floats
    DFKlvo4f-  VMS routine for exporting little-endian 32 bit floats
    DFKlvi8f-  VMS routine for importing little-endian 64 bit floats
    DFKlvo8f-  VMS routine for exporting little-endian 64 bit floats
    dfkconv.c
    DFKci4f -  Convex routine for importing 32 bit floats
    DFKco4f -  Convex routine for exporting 32 bit floats
    DFKci8f -  Convex routine for importing 64 bit floats
    DFKco8f -  Convex routine for exporting 64 bit floats
    DFKlci4f-  Convex routine for importing little-endian 32 bit floats
    DFKlco4f-  Convex routine for exporting little-endian 32 bit floats
    DFKlci8f-  Convex routine for importing little-endian 64 bit floats
    DFKlco8f-  Convex routine for exporting little-endian 64 bit floats
    dfkfuji.c
    DFKpi4f -  VP routine for importing 32 bit floats
    DFKpo4f -  VP routine for exporting 32 bit floats
    DFKpi8f -  VP routine for importing 64 bit floats
    DFKpo8f -  VP routine for exporting 64 bit floats

 Other PUBLIC functions:
    DFKmachineNTsize - Determine size in machine, given number type
    DFKhdfNTsize     - Determine size in HDF format, given number type
    DFKsetNT         - Set number type for future conversion calls
    DFKsetcustom    - Template for user to setup custom conversion
                      routines
    DFKisnative     - Checks whether number type is native mode
    DFKislitend     - Checks whether number type is little-endian mode
    DFconvert       - provide compatibility with 3.0 routines

 Private functions:
    DFKInoset    - Indicate that DFKsetNT hasn't been called

 Remarks:

 *------------------------------------------------------------------*/

/*****************************************************************************/
/*                                                                           */
/*    All the routines in this file marked as PRIVATE have been marked so    */
/*  for a reason.  *ANY* of these routines may or may nor be supported in    */
/*  the next version of HDF (4.00).  Furthurmore, the names, paramters, or   */
/*  functionality is *NOT* guaranteed to remain the same.                    */
/*    The *ONLY* guarantee possible is that DFKnumin(), and DFKnumout()      */
/*  will not change.  They are *NOT* guaranteed to be implemented in the     */
/*  next version of HDF as function pointers.  They are guaranteed to take   */
/*  the same arguments and produce the same results.                         */
/*    If your programs call any routines in this file except for             */
/*  DFKnumin(), DFKnumout, and/or DFKsetntype(), your code may not work      */
/*  with future versions of HDF and your code will *NOT* be portable.        */
/*                                                                           */
/*****************************************************************************/

#include <ctype.h>
#include "hdf.h"
#include "hconv.h"

/*
   **  Static function prototypes
 */
PRIVATE int DFKInoset
            (VOIDP source, VOIDP dest, uint32 num_elm, uint32 source_stride,
             uint32 dest_stride);

/* Prototypes */
extern int32 DFKqueryNT(void);
extern int DFKsetcustom(
  int (*DFKcustin) (VOIDP source, VOIDP dest, uint32 num_elm,
                           uint32 source_stride, uint32 dest_stride),
  int  (*DFKcustout) (VOIDP source, VOIDP dest, uint32 num_elm,
                             uint32 source_stride, uint32 dest_stride));
extern int DFconvert(uint8 *source, uint8 *dest, int ntype, int sourcetype, 
                     int desttype, int32 size);

/*
   **  Conversion Routine Pointer Definitions
 */
static int	(*DFKnumin)
            (VOIDP source, VOIDP dest, uint32 num_elm,
                    uint32 source_stride, uint32 dest_stride) = DFKInoset;
static int	(*DFKnumout)
            (VOIDP source, VOIDP dest, uint32 num_elm,
                    uint32 source_stride, uint32 dest_stride) = DFKInoset;

/************************************************************
 * If the programmer forgot to call DFKsetntype, then let
 * them know about it.
 ************************************************************/
PRIVATE int
DFKInoset(VOIDP source, VOIDP dest, uint32 num_elm,
          uint32 source_stride, uint32 dest_stride)
{
    CONSTR(FUNC, "DFKInoset");

    HEclear();

    /* shut the compiler up about not using the arguments */
    source = source;
    dest = dest;
    num_elm = num_elm;
    source_stride = source_stride;
    dest_stride = dest_stride;

    /* If this is causing a problem for you, call DFKsetntype */
    HERROR(DFE_BADCONV);
    return FAIL;
}

/*****************************************************************************
 * Routines that depend on the above information
 *****************************************************************************/

PRIVATE int32 g_ntype = DFNT_NONE;  /* Holds current number type. */
                    /* Initially not set.         */

/************************************************************
* DFKqueryNT()
*   Determine the current conversion settings
 ************************************************************/
int32
DFKqueryNT(void)
{
    return g_ntype;
}

/************************************************************
 * DFKNTsize()
 *   Determine the size, given the number type
 ************************************************************/
int
DFKNTsize(int32 number_type)
{
#ifdef LATER
    CONSTR(FUNC, "DFKNTsize");
#endif

    /* mask off the litend bit since little endian and big endian have */
    /* the same size.  Only need to distinguish size difference between */
    /* HDF and native types. */
    switch (number_type & (~DFNT_LITEND))
      {
	  /* native types */
          case DFNT_NUCHAR:
              return (SIZE_NUCHAR);
          case DFNT_NCHAR:
              return (SIZE_NCHAR);
          case DFNT_NINT8:
              return (SIZE_NINT8);
          case DFNT_NUINT8:
              return (SIZE_NUINT8);

          case DFNT_NINT16:
              return (SIZE_NINT16);
          case DFNT_NUINT16:
              return (SIZE_NUINT16);

          case DFNT_NINT32:
              return (SIZE_NINT32);
          case DFNT_NUINT32:
              return (SIZE_NUINT32);

          case DFNT_NFLOAT32:
              return (SIZE_NFLOAT32);

          case DFNT_NFLOAT64:
              return (SIZE_NFLOAT64);

	  /* HDF types */
          case DFNT_UCHAR:
              return (SIZE_UCHAR);
          case DFNT_CHAR:
              return (SIZE_CHAR);
          case DFNT_INT8:
              return (SIZE_INT8);
          case DFNT_UINT8:
              return (SIZE_UINT8);

          case DFNT_INT16:
              return (SIZE_INT16);
          case DFNT_UINT16:
              return (SIZE_UINT16);

          case DFNT_INT32:
              return (SIZE_INT32);
          case DFNT_UINT32:
              return (SIZE_UINT32);

          case DFNT_FLOAT32:
              return (SIZE_FLOAT32);

          case DFNT_FLOAT64:
              return (SIZE_FLOAT64);

	  /* Unknown types */
          default:
              break;
      }     /* switch       */
    /* hdf default format   */
    return FAIL;
}

/************************************************************
 * DFKsetNT()
 *   Set the number type for future conversion calls
 ************************************************************/
intn
DFKsetNT(int32 ntype)
{
    CONSTR(FUNC, "DFKsetNT");

    HEclear();

    g_ntype = ntype;

    switch (ntype)
      {
          case DFNT_CHAR8:
          case DFNT_UCHAR8:
          case DFNT_INT8:
          case DFNT_UINT8:
              DFKnumin = UI8_IN;
              DFKnumout = UI8_OUT;
              break;
          case DFNT_INT16:
              DFKnumin = SI16_IN;
              DFKnumout = SI16_OUT;
              break;
          case DFNT_UINT16:
              DFKnumin = UI16_IN;
              DFKnumout = UI16_OUT;
              break;
          case DFNT_INT32:
              DFKnumin = SI32_IN;
              DFKnumout = SI32_OUT;
              break;
          case DFNT_UINT32:
              DFKnumin = UI32_IN;
              DFKnumout = UI32_OUT;
              break;
          case DFNT_FLOAT32:
              DFKnumin = F32_IN;
              DFKnumout = F32_OUT;
              break;
          case DFNT_FLOAT64:
              DFKnumin = F64_IN;
              DFKnumout = F64_OUT;
              break;

              /*
               * NATIVE MODE 'CONVERSIONS'
               */
          case DFNT_NCHAR:
          case DFNT_NINT8:
          case DFNT_NUCHAR:
          case DFNT_NUINT8:
              DFKnumin = NUI8_IN;
              DFKnumout = NUI8_OUT;
              break;
          case DFNT_NINT16:
              DFKnumin = NSI16_IN;
              DFKnumout = NSI16_OUT;
              break;
          case DFNT_NUINT16:
              DFKnumin = NUI16_IN;
              DFKnumout = NUI16_OUT;
              break;
          case DFNT_NINT32:
              DFKnumin = NSI32_IN;
              DFKnumout = NSI32_OUT;
              break;
          case DFNT_NUINT32:
              DFKnumin = NUI32_IN;
              DFKnumout = NUI32_OUT;
              break;
          case DFNT_NFLOAT32:
              DFKnumin = NF32_IN;
              DFKnumout = NF32_OUT;
              break;
          case DFNT_NFLOAT64:
              DFKnumin = NF64_IN;
              DFKnumout = NF64_OUT;
              break;

              /*
               * Little Endian Conversions
               */
          case DFNT_LCHAR:
          case DFNT_LINT8:
          case DFNT_LUCHAR:
          case DFNT_LUINT8:
              DFKnumin = LUI8_IN;
              DFKnumout = LUI8_OUT;
              break;
          case DFNT_LINT16:
              DFKnumin = LSI16_IN;
              DFKnumout = LSI16_OUT;
              break;
          case DFNT_LUINT16:
              DFKnumin = LUI16_IN;
              DFKnumout = LUI16_OUT;
              break;
          case DFNT_LINT32:
              DFKnumin = LSI32_IN;
              DFKnumout = LSI32_OUT;
              break;
          case DFNT_LUINT32:
              DFKnumin = LUI32_IN;
              DFKnumout = LUI32_OUT;
              break;
          case DFNT_LFLOAT32:
              DFKnumin = LF32_IN;
              DFKnumout = LF32_OUT;
              break;
          case DFNT_LFLOAT64:
              DFKnumin = LF64_IN;
              DFKnumout = LF64_OUT;
              break;

/* No conversion routines are specified for DFNT_custom.  User must provide. */
/* Users should call DFCV_SetCustomIn() and DFCV_SetCustomOut() if they      */
/* choose to use DFNT_CUSTOM.  Users should provide their own method to      */
/* distinguish between multiple 'custom' conversion routines.  HDF only      */
/* knows such routines as type 'DFNT_CUSTOM'.                                */

          case DFNT_CUSTOM:
              g_ntype = DFNT_CUSTOM;
              break;
          default:
              HRETURN_ERROR(DFE_BADCONV,FAIL)
      }
  return 0;
}

/*****************************************************************************
 * The following routine provides an easy method for the user to setup custom
 * conversion routines....
 *****************************************************************************/
int
DFKsetcustom(
  int         (*DFKcustin) (VOIDP /* source */, VOIDP /* dest */, uint32 /* num_elm */,
                                 uint32 /* source_stride */, uint32 /* dest_stride */),
 int         (*DFKcustout) (VOIDP /* source */, VOIDP /* dest */, uint32 /* num_elm */,
                                   uint32 /* source_stride */, uint32 /* dest_stride */)
)
{
    DFKnumin = DFKcustin;
    DFKnumout = DFKcustout;
    DFKsetNT(DFNT_CUSTOM);  /* Keep HDF from getting confused */
    return 0;
}

/*------------------------------------------------------------------
 * Name:    DFKisnativeNT
 * Purpose: Determine whether number type is native mode
 * Inputs:  numbertype: number type
 * Returns: 1 if true, 0 if false
 * Users:   DFSDgetslice
 * Method:  Checks to see if the "native mode" bit is set
 * Remarks:
 *------------------------------------------------------------------*/

int32
DFKisnativeNT(int32 numbertype)
{
    return ((DFNT_NATIVE & numbertype) > 0 ? 1 : 0);
}

/*------------------------------------------------------------------
 * Name:    DFKislitendNT
 * Purpose: Determine whether number type is little-endian mode
 * Inputs:  numbertype: number type
 * Returns: 1 if true, 0 if false
 * Users:   DFSDgetslice
 * Method:  Checks to see if the "native mode" bit is set
 * Remarks:
 *------------------------------------------------------------------*/

int32
DFKislitendNT(int32 numbertype)
{
    return ((DFNT_LITEND & numbertype) > 0 ? 1 : 0);
}

/************************************************************
 * DFconvert()
 *
 * This routine is called by HDF version 3.0 compatibility
 * routines.  It serves as a jump point to the new version 4.0
 * comversion functions.  DFconvert() CANNOT be used by Vdata
 * applications because it assumes a stride of 1 (for
 * compatibility). Vdata routines should call DFnum_in() and
 * DFKnumout() (depending on which translation is needed)
 *
 * uint8 * source    location where the data is stored
 * uint8 * dest      location to put the converted data
 * int  * ntype       the overall number type of the data, ie DFNT_FLOAT...
 * int  * sourcetype  the specific type of the source data, ie DFNTF_IEEE...
 * int  * desttype    the specifid type of the converted data, ie DFNTF_VAX...
 * int  * size        the number (total) of BYTES to convert
 ************************************************************/
int
DFconvert(uint8 *source, uint8 *dest, int ntype, int sourcetype, int desttype,
          int32 size)
{
    uint32 num_elm;
    CONSTR(FUNC, "DFconvert");

    HEclear();

    if (DFKsetNT(ntype) == FAIL)
      {
          HERROR(DFE_BADCONV);
          return FAIL;
      }

    if (sourcetype == desttype)
      {
          HDmemcpy(dest, source, size);
          return 0;
      }

    num_elm = (uint32)size / 4;

/* Check to see if they want to covert numbers in from the disk */
    if (sourcetype == DFNTF_IEEE && (desttype == DFNTF_VAX ||
                                     desttype == DFNTF_CRAY ||
                                     desttype == DFNTF_PC))
        return (DFKnumin) ((VOIDP) source, (VOIDP) dest, num_elm, 0, 0);

/* Check to see if they want to convert numbers out to disk */
    if (desttype == DFNTF_IEEE && (sourcetype == DFNTF_VAX ||
                                   sourcetype == DFNTF_CRAY ||
                                   sourcetype == DFNTF_PC))
        return DFKnumout((VOIDP) source, (VOIDP) dest, num_elm, 0, 0);

/* Return an error because they did not specify valid translation codes */
    HERROR(DFE_BADCONV);
    return FAIL;
}

/*------------------------------------------------------------------
 * Name:    DFKgetPNSC
 * Purpose: Get PlatformNumberSubclass for a given number type
 * Inputs:  numtype: number type to get subclass for
 *          machinetype: machine-type code
 * Returns: PlatformNumberSubclass on success, FAIL on failure with
 *          error set
 * Users:   DFSDgetslice
 * Method:  Checks NT_TYPES to determine whether it is a char, int, or
 *          float, then looks in corresponding field of machine type
 *          (DFMT) to get the class.
 * Remarks:
 *------------------------------------------------------------------*/

int8
DFKgetPNSC(int32 numbertype, int32 machinetype)
{
    CONSTR(FUNC, "DFKgetPNSC");

    /* clear error stack and validate args */
    HEclear();

    /* Since the information is provided only for the 4 */
    /* classes of char, int, float, double and is indenpend */
    /* of whether it is stored native or little-endian in file, */
    /* we will use only the standard HDF format information */
    switch (numbertype & DFNT_MASK)
      {
          case DFNT_CHAR8:
          case DFNT_UCHAR8:
              return (int8) (machinetype & 0x0f);

          case DFNT_INT8:
          case DFNT_UINT8:
          case DFNT_INT16:
          case DFNT_UINT16:
          case DFNT_INT32:
          case DFNT_UINT32:
              return (int8) ((machinetype >> 4) & 0x0f);

          case DFNT_FLOAT32:
              return (int8) ((machinetype >> 8) & 0x0f);

          case DFNT_FLOAT64:
              return (int8) ((machinetype >> 12) & 0x0f);

          default:
	      HRETURN_ERROR(DFE_BADNUMTYPE, FAIL);
      }
}

/*----------------------------------------------------------------------------
* Name: DFKconvert
* Purpose: set number type and do the convert
* Inputs:  source -- location where the data is stored
*      dest -- location to put the converted data
*      ntype -- the current number type
*      num_elm -- number of elements to be converted
*      acc_mode -- DFACC_READ for numin, DFACC_WRITE for numout
*      source_stride, dest_stride -- strides in source and destination
* Returns: 0 -- succeed; FAIL -- failure
* Users:   DFSDgetsdg, DFSDputsdg, DFSDIgetslice, DFSDIgetslice
* Method:  Calls DFKsetNT, then call DFnumin or DFnumout
*---------------------------------------------------------------------------*/
int32
DFKconvert(VOIDP source, VOIDP dest, int32 ntype, int32 num_elm,
           int16 acc_mode, int32 source_stride, int32 dest_stride)
{
    int         ret;

    /* Check args (minimally) */
    if (source==NULL || dest==NULL)
        return(-1);

    DFKsetNT(ntype);
    if (acc_mode == DFACC_READ)
        ret = DFKnumin(source, dest, (uint32)num_elm, (uint32)source_stride, (uint32)dest_stride);
    else
        ret = DFKnumout(source, dest, (uint32)num_elm, (uint32)source_stride, (uint32)dest_stride);
    return (ret);
}

/*****************************************************************************
 * Miscellaneous Other Conversion Routines
 *****************************************************************************/
