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

/* $Id: dfsd.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*-----------------------------------------------------------------------------
 File:  dfsd.c

 Purpose:
    Routines for input and output of numeric data group

 Invokes:
    df.c dfgroup.c dfkit.c dfi.h df.h dfsd.h

 Public functions:
    DFSDgetdims      - get rank and dim sizes
    DFSDgetdatastrs  - get label, unit, format and coord system of data
    DFSDgetdimstrs   - get label, unit and format for a dimension
    DFSDgetdatalen   -
    DFSDgetdimlen    -
    DFSDgetdimscale  - get scale for a dimension
    DFSDgetrange     - get max and min of data
    DFSDgetdata      - get data values
    DFSDgetNT        - get file number type for reading
    DFSDpre32sdg     - tests, without calling DFSDsdginfo,  whether or
                       not the SDG/ref written with 3.1
    DFSDsetlengths   - set lengths of label, unit, format strings on gets
    DFSDsetdims      - set rank and dim sizes
    DFSDsetdatastrs  - set data label, unit, format and coord system
    DFSDsetdimstrs   - set dim labels, units and formats
    DFSDsetdimscale  - set scale for a dimension
    DFSDsetrange     - set max and min of data
    DFSDsetorder     - set array order to C or FORTRAN order
    DFSDsetNT        - set number type to be written out
    DFSDputdata      - output data, data info, and display info
    DFSDrestart      - forget info about last file accessed - restart from
                       beginning
    DFSDndatasets    - return number of SDGs in file
    DFSDclear        - forget all info set by DFSDset* routines
    DFSDlastref      - get reference number of last SDG read or written
    DFSDgetslice     - get part of the data, specified as a slice
    DFSDstartslice   - set up to write SD
    DFSDputslice     - write specified number of data items to file
    DFSDendslice     - end of series of writes, write out SDG
    DFSDwriteref     - set reference number to be used in next SDS write slab
    DFSDsetfillvalue - set fill value to be used in next SDS written
    DFSDgetfillvalue - return fill value from SDS that is about to be read
    DFSDstartslab    - set up to write slabs
    DFSDwriteslab    - write hyperslab of values
    DFSDendslab      - end of series of hyperslab writes
    DFSDreadslab     - get part of the data, specified as a slab

Lower level functions:
    DFSDgetsdg  - read SDG into struct
    DFSDputsdg  - write SDG to file

Private functions:
    DFSDIopen      - open or reopen file
    DFSDIsdginfo   - find next sdg in file
    DFSDIisndg     - was currently read sdg written by HDF3.2
    DFSDIrefresh   - get info of next sdg if necessary
    DFSDIgetrrank  - get rank of the currently read sdg
    DFSDIgetwrank  - get rank of the sdg to be written
    DFSDIclear     - clear sdg data structure of all info
    DFSDIgetdata   - read data from file
    DFSDIputdata   - write data to file
    DFSDIgetslice  - get slice
    DFSDIputslice  - put slice
    DFSDIendslice  -
    DFSDIsetnsdg_t - set up nsdg table
    DFSDInextnsdg  - get next nsdg from nsdg table
    DFSDIgetndg    - read NDG into struct
    DFSDIputndg    - write NDG to file

Fortran stub functions:
    dsisdas - set data label, unit, format and coord system
    dsisdis - set dim labels, units and formats

 Remarks:
    These functions will be copied into dfsd.c after debugging.
    This version assumes that all the values are floating point.
 *---------------------------------------------------------------------------*/

#include "hdf.h"
#include "dfsd.h"

#ifdef _MSC_VER
#define strdup _strdup
#endif

/* MMM: make this definition correct and move to hfile.h, or wherever. */
#define DF_NOFILE 0

#define LABEL   0
#define UNIT    1
#define FORMAT  2
#define COORDSYS 3

#define NFGSDG_TYPE_SDG 0   /* a pure SDG  */
#define NFGSDG_TYPE_NFG 1   /* a pure NDG  */
#define NFGSDG_TYPE_SDGNDG 2    /* an SDG in NDG */

/* Init NSDG table header      */
PRIVATE DFnsdg_t_hdr *nsdghdr = NULL;

/* initialize aid to -1 and numbertype to DFNT_NONE.   S. Xu    */
PRIVATE DFSsdg Readsdg =        /* struct for reading */
{
    {(uint16) 0, (uint16) 0}, (intn) 0, NULL, NULL,
    {NULL, NULL, NULL},
    {NULL, NULL, NULL}, NULL,
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    (int32) DFNT_NONE, DFNTF_NONE, (int32) -1, (int32) 0, (int32) 0,
    (float64) 1.0, (float64) 0.0, (float64) 0.0, (float64) 0.0,
    (int32) -1,
    {0}, 0
};

PRIVATE DFSsdg Writesdg =       /* struct for writing */
{
    {(uint16) 0, (uint16) 0}, (intn) 0, NULL, NULL,
    {NULL, NULL, NULL},
    {NULL, NULL, NULL}, NULL,
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    (int32) DFNT_NONE, DFNTF_NONE, (int32) -1, (int32) 0, (int32) 0,
    (float64) 1.0, (float64) 0.0, (float64) 0.0, (float64) 0.0,
    (int32) -1,
    {0}, 0
};

PRIVATE uint16 Writeref = 0;    /* ref of next SDG/NDG to write to file */
PRIVATE intn Newdata = (-1);    /* Values in Readsdg fresh? */
                /* -1 : no descriptor read */
                /* 1 : descriptor read */
PRIVATE intn Nextsdg = 1;       /* Signal if DFSDgetdata should get the */
                /* next SDG/NDG */
PRIVATE int32 Sfile_id = DF_NOFILE;     /* pointer to file for slice writes */
PRIVATE int32 *Sddims;          /*dims written so far in slice write */

PRIVATE struct
  {                             /* Indicators of status (s) of info:    */
      intn        dims;         /* s = -1: there is no info in this category */
      intn        nt;           /* s = 0: info was set, but not yet written */
      intn        coordsys;     /* s > 0: info was set and written with ref no.s */
      intn        luf[3];
      intn        scales;
      intn        maxmin;
      intn        transpose;    /* It should be taken out!!!            !!!  */
      intn        cal;
      intn        fill_value;
      intn        new_ndg;
  }
Ref =
{
    -1, -1, -1,
    {
        -1, -1, -1
    }
    ,-1, -1, -1, -1, -1, -1
};

PRIVATE intn Maxstrlen[4] =
{DFS_MAXLEN, DFS_MAXLEN, DFS_MAXLEN, DFS_MAXLEN};
PRIVATE intn Ismaxmin = 0;      /* is there a max/min value on read?  */
PRIVATE intn FileTranspose = 0; /* is the data in column major order? */
PRIVATE intn Fortorder = 0;     /* should data be written col major?  */
PRIVATE intn IsCal = 0;         /* has calibration info been set?     */

/* In ver. 3.2 numbertype and file number format (subclass) are included  */
/* in DFSsdg, and  fileNTsize is local to functions .           */
/* static int fileNT=DFNTF_IEEE,         default: all IEEE       */
/*           fileNTsize=4,                       */
/*           outNT=DFNTF_IEEE,           default output: IEEE */
/*           outNTsize=4,                        */
/*           userNT=DFNTF_IEEE ;         default */

PRIVATE uint16 Readref = 0;     /* ref of next SDG/NDG to be read? */
#if 0
PRIVATE char Lastfile[DF_MAXFNLEN] = "";    /* last file opened */
#endif
PRIVATE char *Lastfile = NULL;
PRIVATE uint16 Lastref = 0;     /* Last ref to be read/written? */
PRIVATE DFdi lastnsdg;          /* last read nsdg in nsdg_t */

/* Whether we've installed the library termination function yet for this interface */
PRIVATE intn library_terminate = FALSE;

/* Private buffer */
PRIVATE uint8 *ptbuf = NULL;

/* Prototypes */
static intn DFSDIsetnsdg_t(int32 file_id, DFnsdg_t_hdr * l_nsdghdr);
static intn DFSDInextnsdg(DFnsdg_t_hdr * l_nsdghdr, DFdi * nsdg);
static intn DFSDIgetndg(int32 file_id, uint16 tag, uint16 ref, DFSsdg * sdg);
static intn DFSDIputndg(int32 file_id, uint16 ref, DFSsdg * sdg);
static intn DFSDIstart(void);

/*--------------------------------------------------------------------------
 NAME
       DFSDgetdims
 USAGE
       int DFSDgetdims(filename, prank, sizes, maxrank)
       char  *filename;   IN:  Name of file with scientific data set
       int   prank;       OUT: Number of dimensions
       int32 sizes:       OUT: array for holding dimensions of data set in file.
       int   maxrank:     IN:  size of array "dimsizes"

 RETURNS
       Returns SUCCED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Gets the number of dimensions(rank) and the sizes of the dimensions
       (dimsizes) for the next data set in the file. The input argument
       "maxrank" tells the size of the array that is allocated for storing
       the "dimsizses" array. The value of rank cannot exceed the value of
       "maxrank". The allocation of space for reading in the scientific
       data set should correspond to the values read in by "DFSDgetdims".
       The first value in the array "dimsizes" should equal the first
       dimension of the array that is allocated to hold the data set; the
       second value in "dimsizes" should equal the second dimension of the
       data set, and so forth. "DFSDgetdims" opens and closes the file.

--------------------------------------------------------------------------*/
intn
DFSDgetdims(const char *filename, intn *prank, int32 sizes[], intn maxrank)
{
  intn        i;
  int32       file_id;
  CONSTR(FUNC, "DFSDgetdims");
  intn       ret_value = SUCCEED;

  HEclear();  /* Clear error stack */

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (!prank)     /* check if ptr is valid */
    HGOTO_ERROR(DFE_BADPTR, FAIL);

  file_id = DFSDIopen(filename, DFACC_READ);  /* open/reopen file */
  if (file_id == FAIL)
    {
      ret_value = FAIL;
      goto done;
    }

  if (DFSDIsdginfo(file_id) < 0)
    {     /* reads next SDG from file */
      Hclose(file_id);
      ret_value = FAIL;  /* on error, close file and return */
      goto done;
    }

  *prank = Readsdg.rank;  /* copy rank, dimensions */
  if (maxrank < *prank)   /* if not all dimensions copied */
    HGOTO_ERROR(DFE_NOTENOUGH, FAIL);

  for (i = 0; i < *prank; i++)    /* copy dimensions */
    sizes[i] = Readsdg.dimsizes[i];
  Nextsdg = 0;

  ret_value = Hclose(file_id);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*---------------------------------------------------------------------------
 NAME
       DFSDgetdatastrs
 USAGE
       int DFSDgetdatastrs(label, unit, format, coordsys)
       char *label;    OUT: label string that describes the data
       char *unit;     OUT: unit string that describes the unit used
       char *format;   OUT: format string that describes the format for
                            displaying the data
       char *coordsys; OUT: string describing coordinate system
 RETURN
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Retrieves information about the data from its associated attribute
       strings. Particularly, it reads the label, unit and format strings
       for the data. The parameter "coordsys" gives the coordinate system
       that is to be used for interpreting the dimension information.

----------------------------------------------------------------------------*/
intn
DFSDgetdatastrs(char *label, char *unit, char *format, char *coordsys)
{
  int32       luf;
  char       *lufp;
  CONSTR(FUNC, "DFSDgetdatastrs");
  intn        ret_value = SUCCEED;

  HEclear();  /* Clear error stack */

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (Newdata < 0)
    HGOTO_ERROR(DFE_BADCALL, FAIL);
/* NOTE: Once DFSDsetdatastrs is changed to always write all three (label,
   unit and format) whenever it is called, this routine should be
   changed so that it returns all three, if any exist.  This means
   that it also should be changed to return -1 if none exist.
   (Currently it returns FAIL only if the SDS doesn't exist.)
   */

    /* copy label, unit, format */
  for (luf = LABEL; luf <= FORMAT; luf++)
    {
      lufp = (luf == LABEL) ? label : (luf == UNIT) ? unit : format;
      if (lufp)
        {
          if (Readsdg.dataluf[luf])
            HIstrncpy(lufp, Readsdg.dataluf[luf], Maxstrlen[luf]);
        }
    }     /* end for 'luf' */
  /* copy coordsys */
  if (coordsys)
    {
      if (Readsdg.coordsys)
        HIstrncpy(coordsys, Readsdg.coordsys, Maxstrlen[COORDSYS]);
      else
        coordsys[0] = '\0';
    }

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*----------------------------------------------------------------------------
 NAME
       DFSDgetdimstrs
 USAGE
       int DFSDgetdimstrs(dim, label, unit, format)
       int  dim;      OUT: Dimension this label, unit and format refer to
       char *label;   OUT: Label that describes this dimension
       char *unit;    OUT: Unit to be used with this dimension
       char *format;  OUT: Format to be used in displaying scale for this
                           dimension
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Gets the labe, unit and format strings corresponding to the specified
       dimension. The space allocated for the label, unit and format string
       must be at least 1 byte larger than the length of the string. If the
       length is unknown when the program is written, declare the array size
       as 1+maxlen_label, _unit or _format after they are set by "DFSDsetlengths".
       The maximum default string length is 255.
----------------------------------------------------------------------------*/
intn
DFSDgetdimstrs(int dim, char *label, char *unit, char *format)
{
  intn        luf;
  intn        rdim;
  char       *lufp;
  CONSTR(FUNC, "DFSDgetdimstrs");
  intn        ret_value = SUCCEED;

  HEclear();  /* Clear error stack */

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (Newdata < 0)
    HGOTO_ERROR(DFE_BADCALL, FAIL);
/* NOTE: Once DFSDsetdimstrs is changed to always write all three (label,
   unit and format) whenever it is called, this routine should be
   changed so that it returns all three, if any exist.  This means
   that it also should be changed to return -1 if none exist.
   (Currently it returns FAIL only if the SDS doesn't exist.)
   */

  rdim = dim - 1;     /* translate dim to zero origin */
  if ((rdim >= Readsdg.rank) || (rdim < 0))
    HGOTO_ERROR(DFE_BADDIM, FAIL);

    /* copy labels etc */
  for (luf = LABEL; luf <= FORMAT; luf++)
    {
      lufp = (luf == LABEL) ? label : (luf == UNIT) ? unit : format;
      if (lufp)
        {
          if (!Readsdg.dimluf)
            {     /* no labels etc */
              *lufp = '\0';
              continue;
            }
          if (Readsdg.dimluf[luf])
            HIstrncpy(lufp, Readsdg.dimluf[luf][rdim], Maxstrlen[luf]);
        }   /* end if 'lufp' */
    }     /* end for 'luf' */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*---------------------------------------------------------------------------
 NAME
       DFSDgetdatalen
 USAGE
       int DFSDgetdatalen(llabel, lunit, lformat, lcoordsys)
       int *llabel;     OUT: length of label string
       int *lunit;      OUT: length of unit string
       int *lformat;    OUT: length of format string
       int *lcoordsys;  OUT: length of coordsys string

 RETURN
       Returns SUCCED(0) if succesful and FAIL(-1) otherwise
 DESCRIPTION
       Gets actual length of label, unit, format and coordinate system
       strings. The space allocated for the label, unit, format and
       coordinate system strings must be at least 1 byte larger than the
       actual length of the string.

----------------------------------------------------------------------------*/
intn
DFSDgetdatalen(intn *llabel, intn *lunit, intn *lformat, intn *lcoordsys)
{
  CONSTR(FUNC, "DFSDgetdatalen");
  intn      ret_value = SUCCEED;

  HEclear();  /* Clear error stack */

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (Newdata < 0)
    HGOTO_ERROR(DFE_BADCALL, FAIL);

  *llabel = (intn)(Readsdg.dataluf[LABEL] ?  HDstrlen(Readsdg.dataluf[LABEL]) : 0);
  *lunit = (intn)(Readsdg.dataluf[UNIT] ?  HDstrlen(Readsdg.dataluf[UNIT]) : 0);
  *lformat = (intn)(Readsdg.dataluf[FORMAT] ?  HDstrlen(Readsdg.dataluf[FORMAT]) : 0);
  *lcoordsys = (intn)(Readsdg.coordsys ?  HDstrlen(Readsdg.coordsys) : 0);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*---------------------------------------------------------------------------
 NAME
       DFSDgetdimlen
 USAGE
       int DFSDgetdimlen(dim, llabel, lunit, lformat)
       int dim;       OUT: number of dimensions to get info about
       int *llabel;   OUT: length of label string
       int *lunit;    OUT: length of unit string
       int *lformat;  OUT: length of format string
 RETURN
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Get the length of label, unit, and format for the specified
       dimension. The space allocated to hold the label, unit, and format
       strings must be at least 1 byte larger than the actual length of the
       string.
----------------------------------------------------------------------------*/
intn
DFSDgetdimlen(intn dim, intn *llabel, intn *lunit, intn *lformat)
{
  CONSTR(FUNC, "DFSDgetdimlen");
  intn ret_value = SUCCEED;

  HEclear();  /* Clear error stack */

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (Newdata < 0)
    HGOTO_ERROR(DFE_BADCALL, FAIL);

  if (dim > Readsdg.rank)
    HGOTO_ERROR(DFE_BADDIM, FAIL);

  *llabel = (intn)(Readsdg.dimluf[LABEL][dim - 1] ?  HDstrlen(Readsdg.dimluf[LABEL][dim - 1]) : 0);
  *lunit = (intn)(Readsdg.dimluf[UNIT][dim - 1] ?  HDstrlen(Readsdg.dimluf[UNIT][dim - 1]) : 0);
  *lformat = (intn)(Readsdg.dimluf[FORMAT][dim - 1] ?  HDstrlen(Readsdg.dimluf[FORMAT][dim - 1]) : 0);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*----------------------------------------------------------------------------
 NAME
       DFSDgetdimscale
 USAGE
       int DFSDgetdimscale(dim, maxsize, scale)
       int   dim;      IN: Dimension this scale corresponds to
       int32 size;     IN:  size of scale
       VOIDP scale;    OUT: the scale
 RETURN
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Gets the scale corresponding to the specified demension. The DFSD
       interface in HDFv3.3 requires that the dimension scales are of the
       same number type as the corresponding data. To store dimension scales
       of a different number type than the corresponding data see the
       multi-file SD interface.
---------------------------------------------------------------------------*/
intn
DFSDgetdimscale(intn dim, int32 maxsize, VOIDP scale)
{
  uint32      dimsize;
  int32       numtype;
  int32       localNTsize;
  intn        rdim;
  uint8      *p1, *p2;
  CONSTR(FUNC, "DFSDgetdimscale");
  intn       ret_value = SUCCEED;

  HEclear();  /* Clear error stack */

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (Newdata < 0)
    HGOTO_ERROR(DFE_BADCALL, FAIL);

  rdim = dim - 1;     /* translate dim to zero origin */
  if ((rdim >= Readsdg.rank) || (rdim < 0))
    HGOTO_ERROR(DFE_BADDIM, FAIL);

  if (maxsize < Readsdg.dimsizes[rdim])
    HGOTO_ERROR(DFE_NOSPACE, FAIL);

  if (!scale)
    HGOTO_ERROR(DFE_BADPTR, FAIL);

  if (!Readsdg.dimscales || !Readsdg.dimscales[rdim])     /* no scale */
    HGOTO_ERROR(DFE_NOVALS, FAIL);

    /* get number type and copy data from Readsdg to scale */
  if (Readsdg.numbertype == DFNT_NONE)
    Readsdg.numbertype = DFNT_FLOAT32;

  numtype = Readsdg.numbertype;
  localNTsize = DFKNTsize((numtype | DFNT_NATIVE) & (~DFNT_LITEND));
  dimsize = (uint32)localNTsize * (uint32)Readsdg.dimsizes[rdim];     /* in bytes  */

  p1 = (uint8 *) scale;
  p2 = (uint8 *) (Readsdg.dimscales[rdim]);
  HDmemcpy(p1, p2, dimsize);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*---------------------------------------------------------------------------
 NAME
       DFSDgetrange
 USAGE
       int DFSDgetrange(max, min)
       VOIDP max;    OUT: High value stored with the scientific data set
       VOIDP min;    OUT: Low value stored with the scientific data set
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Gets the maximum and minimum values stored with the scientific data
       set. The "max" and "min" values are set by "DFSDsetrange". They are
       not automatically stored when a data set is written to a file.
       Since these values are supposed to relate to the data itself, it is
       assumed that they are of the same number type as the data itself.
       One implication of this is that in the C version of "DFSDgetrange"
       the arguments are pointers, rather than simple variables, whereas in
       the FORTRAN version they are simple variables of the same type as the
       data array. Neither "DFSDgetrange" or "DFSDgetdata" compare the "max"
       and "min" values stored with the data set to the actual values in
       the data set; they merely retrieve the data. As a result, the
       maximum and minimum values may not always reflect the actual maximum
       and minimum vlaues in the data set. In some cases the "max" and "min"
       values may actually lie outside the range of values in the data set.
---------------------------------------------------------------------------*/
int
DFSDgetrange(VOIDP pmax, VOIDP pmin)
{
  int32       numtype;
  uint32      localNTsize;
  uint8      *p1, *p2;
  CONSTR(FUNC, "DFSDgetrange");
  int         ret_value = SUCCEED;

  HEclear();  /* Clear error stack */

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (Newdata < 0)
    HGOTO_ERROR(DFE_BADCALL, FAIL);

    /* get number type and copy data  */
  if (Readsdg.numbertype == DFNT_NONE)
    Readsdg.numbertype = DFNT_FLOAT32;
  numtype = Readsdg.numbertype;
  localNTsize = (uint32)DFKNTsize((numtype | DFNT_NATIVE) & (~DFNT_LITEND));

  if (Ismaxmin)   /* Ismaxmin is a global */
    {
      p1 = (uint8 *) pmax;
      p2 = (uint8 *) &(Readsdg.max_min[0]);
      HDmemcpy(p1, p2, localNTsize);
      p1 = (uint8 *) pmin;
      p2 = &(Readsdg.max_min[localNTsize]);
      HDmemcpy(p1, p2, localNTsize);
      ret_value = SUCCEED;
      goto done;
    }
  else
    HGOTO_ERROR(DFE_NOVALS, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*---------------------------------------------------------------------------
 NAME
      DFSDgetdata
 USAGE
      intn DFSDgetdata(filename, rank, maxsizes, data)
      char  filename;    IN:  name of HDF file containing scientific data set
      intn  rank;        IN:  number of dimensions of array "data"
      int32 maxsizes;    IN:  Array that holds dimensions of buffer that will
                              hold the data
      VOIDP data;        OUT: Array for holding the data

 RETURN
       Returns SUCCEED(0) if succesful and FAIL(-1) otherwise.
 DESCRIPTION
       Reads the next data set in the file. If you do no know the values of
       "rank" or "dimsizes", you must call "DFSDgetdims" to get them and
       then use them to provide the right amount of space for the array
       "data". If you do not know the number type of the data in the
       scientific dat set, you can call "DFSDgetNT" to find out.
       Subsequent calls to "DFSDgetdata"(or to "DFSDgetdims" and "DFSDgetdata")
       will sequentially read scientific data sets from the file. For example,
       if "DFSDgetdata" is called three times in succession, the third call
       reads data from the third scientific data set in the file. Note that
       if you do no know the values of "rank" or "dimsizes" you must call
       "DFSDgetdims" to get them each time and then provide the necessary
       space to hold the data. If "DFSDgetdims" or "DFSDgetdata" is called
       and there are no more scientific data sets left in the file, an
       error code is returned and nothing is read. "DFSDrestart" can be
       used to override this convention.

---------------------------------------------------------------------------*/
intn
DFSDgetdata(const char *filename, intn rank, int32 maxsizes[], VOIDP data)
{
  intn ret_value;

  ret_value = (DFSDIgetdata(filename, rank, maxsizes, data, 0));   /* 0 == C */

  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDsetlengths
 USAGE
       int DFSDsetlengths(maxlen_label, maxlen_unit, maxlen_format, maxlen_coordsys)
       int maxlen_label;     IN: maximum length of any label
       int maxlen_unit;      IN: maximum length of any unit
       int maxlen_format;    IN: maximum length of any format
       int maxlen_coordsys;  IN: maximum length of any coordsys
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Sets the maximum lengths for the strings that will hold labels, units
       formats and the name of the coordinate system. These lengths are used
       by the routines "DFSDgetdimstrs" and "DFSDgetdatastrs" to determine
       the maximum lengths of strings that they get from the HDF file.
       Normally, "DFSDsetlengths" is not needed. If it is not called,
       default maximum lengths of 255 are used for all strings.
-----------------------------------------------------------------------------*/
intn
DFSDsetlengths(intn maxlen_label, intn maxlen_unit, intn maxlen_format,
               intn maxlen_coordsys)
{
  CONSTR(FUNC, "DFSDsetlengths");
  intn ret_value = SUCCEED;

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (maxlen_label > 0)
    Maxstrlen[LABEL] = maxlen_label;
  if (maxlen_unit > 0)
    Maxstrlen[UNIT] = maxlen_unit;
  if (maxlen_format > 0)
    Maxstrlen[FORMAT] = maxlen_format;
  if (maxlen_coordsys > 0)
    Maxstrlen[COORDSYS] = maxlen_coordsys;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDsetdims
 USAGE
       int DFSDsetdims(rank, dimsizes)
       int   rank;         IN: number of dimensions
       int32 dimsizes[];   IN: array containing dimensions of scientific data sett
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Sets the rank and dimension sizes for all subsequent scientific data
       sets written to the file. This routine must be called before calling
       either "DFSDgetdimstrs" or "DFSDsetdimscale". "DFSDsetdims" need not
       be called if other set routines are not called and the correct
       dimensions are supplied in "DFSDputdata" or "DFSDadddata". If the
       rank or dimension sizes change, all previous set calls are cleared,
       except for the number type which is set by calling "DFSDsetNT".
-----------------------------------------------------------------------------*/
intn
DFSDsetdims(intn rank, int32 dimsizes[])
{
  intn        i;
  CONSTR(FUNC, "DFSDsetdims");
  intn     ret_value = SUCCEED;

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (Sfile_id != DF_NOFILE)
    HGOTO_ERROR(DFE_BADCALL, FAIL);

  if (Writesdg.rank == rank)  /* check if dimensions same */
    {
      if (Writesdg.dimsizes)
        {
          for (i = 0; i < rank; i++)
            {
              if (Writesdg.dimsizes[i] != dimsizes[i])
                break;
            }
          if (i == rank)
            {
              ret_value= SUCCEED;     /* Dimensions same as before */
              goto done;
            }
        }
    }

  /* forget all attributes set previously */
  if (DFSDIclear((DFSsdg *) & Writesdg) < 0)
    {
      ret_value = FAIL;
      goto done;
    }

  /* allocate dimensions */
  Writesdg.dimsizes = (int32 *) HDmalloc((size_t)rank * sizeof(int32));
  if (Writesdg.dimsizes == NULL)
    {
      ret_value = FAIL;
      goto done;
    }

  /* copy dimensions */
  Writesdg.rank = rank;
  for (i = 0; i < rank; i++)
    Writesdg.dimsizes[i] = dimsizes[i];

    /* Note dimensions modified */
  Ref.dims = 0;

  /*
   *  Added side effect, allows creation of new "ref" whenever called
   *  before DFSDwriteslab().
   */
  Ref.new_ndg = 0;
  Writeref = 0;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDsetdatastrs
 USAGE
       int DFSDsetdatastrs(label, unit, format, coordsys)
       char *label;       IN: label that describes the data
       char *unit;        IN: unit to be used with the data
       char *format;      IN: format to be used in displaying the data
       char *coordsys;    IN: coordinate system
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Sets the label, unit, format and coordsys(coordinate system) strings
       to be assigned to the next data set written to the file.
-----------------------------------------------------------------------------*/
intn
DFSDsetdatastrs(const char *label, const char *unit, const char *format, const char *coordsys)
{
  intn ret_value;

  ret_value = (DFSDIsetdatastrs(label, unit, format, coordsys));

  return ret_value;
}

/*-----------------------------------------------------------------------------
 * Name:    DFSDIsetdatastrs()
 * Purpose: Set label, unit and format for displaying subsequent SDGs
 * Inputs:  label: label to be used to describe data
 *          unit: unit corresponding to data values
 *          format: format to be used in displaying data values
 *          coordsys: type of coordinate system
 * Globals: Writesdg, Ref
 * Returns: 0 on success, FAIL on failure with error set
 * Users:   HDF users, utilities, other routines
 * Invokes: none
 * Method:  Stores values in global structure Writesdg
 * Remarks: should we validate coordsys? proposed strings: "cartesian",
 *          "polar" (="spherical") and "cylindrical".  Do "spherical" and
 *          "cylindrical" make sense for 2D?
 *---------------------------------------------------------------------------*/
intn
DFSDIsetdatastrs(const char *label, const char *unit, const char *format, const char *coordsys)
{
  intn        luf;            /* takes values LABEL, UNIT, FORMAT */
                                /* in succession */
  const char *lufp;           /* points to label, unit, format */
                                /* in succession */
  CONSTR(FUNC, "DFSDIsetdatastrs");    /* for HERROR */
  intn      ret_value = SUCCEED;

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

   /* NOTE: The following code should be changed to write all three, even if
      one or more is an empty string.  Then, when DFSDgetdatastrs is called
      values will be returned for all three also, even though some might
      be empty strings.
    */
  for (luf = LABEL; luf <= FORMAT; luf++)
    {
      /* set lufp to point to label etc. as apppropriate */
      lufp = (luf == LABEL) ? label : (luf == UNIT) ? unit : format;

          /* free space if allocated */
      HDfreenclear(Writesdg.dataluf[luf]);

      /* copy string */
      if (lufp)
        {
          Writesdg.dataluf[luf] = (char *) HDstrdup(lufp);
          if (Writesdg.dataluf[luf] == NULL)
            {
              ret_value= FAIL;
              goto done;
            }
        }
    }

  HDfreenclear(Writesdg.coordsys);

  if (coordsys)
    {
      Writesdg.coordsys = (char *) HDstrdup(coordsys);
      if (Writesdg.coordsys == NULL)
        {
          ret_value = FAIL;
          goto done;
        }
    }

  /* indicate that label, unit, format and coordsys info modified */
  Ref.luf[LABEL] = Ref.luf[UNIT] = Ref.luf[FORMAT] = Ref.coordsys = 0;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDsetdimstrs()
 USAGE
       int  DFSDsetdimstrs(dim, label, unit, format)
       int  dim;        IN: dimension this label, unit and format refer to
       char *label;     IN: label that describes this dimension
       char *unit;      IN: unit to be used with this dimension
       char *format;    IN: format to be used to display scale
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Sets the label, unit, and format strings corresponding to the specified
       dimension. In both FORTRAN and C programs, dim=1 for the first
       dimension, and dim=2 for the second and so on. If the user is not
       interested in one or more strings, empty strings can be used as
       parameters for the "DFSDsetdimstrs" call. For example,
       "DFSDsetdimstrs(1, "vertical", "", "") " will set the label for the
       first dimension to "vertical" and set the unit and format strings to
       empty strings.
-----------------------------------------------------------------------------*/
intn
DFSDsetdimstrs(intn dim, const char *label, const char *unit, const char *format)
{
  intn ret_value;

  ret_value = (DFSDIsetdimstrs(dim, label, unit, format));

  return ret_value;
}   /* DFSDsetdimstrs */

/*-----------------------------------------------------------------------------
 * Name:    DFSDIsetdimstrs()
 * Purpose: For the given dimension, set label, unit, format
 *          This routine needs to be called once for each dimension whose
 *          values the user wants to set.
 * Inputs:  dim: the dimension that this info applies to
 *          label: label to be used to describe this dimension
 *          unit: units for dimension
 *          format: format to be used in displaying
 * Globals: Writesdg, Ref
 * Returns: 0 on success, FAIL on failure with error set
 * Users:   HDF users, utilities, other routines
 * Invokes: none
 * Method:  Stores values in global structure Writesdg
 *---------------------------------------------------------------------------*/
intn
DFSDIsetdimstrs(intn dim, const char *label, const char *unit, const char *format)
{
  intn        i;
  intn        rdim;
  intn        luf;            /* takes values LABEL, UNIT, FORMAT */
                                /* in succession */
  const char *lufp;           /* points to label, unit, format */
                                /* in succession */
  CONSTR(FUNC, "DFSDsetdimstrs");
  intn        ret_value = SUCCEED;

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  /* translate from 1 to 0 origin */
  rdim = dim - 1;

  if ((rdim >= Writesdg.rank) || (rdim < 0))
    HGOTO_ERROR(DFE_BADDIM, FAIL);

  for (luf = LABEL; luf <= FORMAT; luf++)
    {
      /* set lufp to point to label etc. as apppropriate */
      lufp = (luf == LABEL) ? label : (luf == UNIT) ? unit : format;

      /* allocate space if necessary */
      if (!Writesdg.dimluf[luf])
        {
          Writesdg.dimluf[luf] =
            (char **) HDmalloc((uint32) Writesdg.rank * sizeof(char *));
          if (Writesdg.dimluf[luf] == NULL)
            {
              ret_value = FAIL;
              goto done;
            }
          for (i = 0; i < Writesdg.rank; i++)     /* set allocated pointers to NULL */
            Writesdg.dimluf[luf][i] = NULL;
        }

      /* free string space if allocated */
      HDfreenclear(Writesdg.dimluf[luf][rdim]);

      /* NOTE: The following code should be changed to write all three, even if
             one or more is an empty string.  Then, when DFSDgetdimstrs is called
             values will be returned for all three also, even though some might
             be empty strings.
      */
      /* copy string */
      if (lufp)
        {
          Writesdg.dimluf[luf][rdim] = (char *) HDstrdup(lufp);
          if (Writesdg.dimluf[luf][rdim] == NULL)
            {
              ret_value = FAIL;
              goto done;
            }
        }
    }
  /* Indicate that this info has not been written to file */
  Ref.luf[LABEL] = Ref.luf[UNIT] = Ref.luf[FORMAT] = 0;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDsetdimscale
 USAGE
       int DFSDsetdimscale(dim, dimsize, scale)
       int   dim;        IN: dimension the is scale corresponds to
       int32 dimsize;    IN: size of scale in the dimension
       VOID  *scale;     IN: the scale
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Sets the scale for a dimension. A scale is a 1D array whose value
       describes reference points along one of the dimensions of the
       scientific data set. For example, a 2D scientific data set representing
       points on a mpa could have two scales, one representing points of
       latitude, and the other points of longitude.
-----------------------------------------------------------------------------*/
intn
DFSDsetdimscale(intn dim, int32 dimsize, VOIDP scale)
{
    int32       i;
    intn        rdim;
    int32       numtype;
    uint32      bytesize;
    int32       localNTsize;
    uint8      *p1, *p2;
    intn        ret_value = SUCCEED;
    CONSTR(FUNC, "DFSDsetdimscale");

    HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

    rdim = dim - 1;     /* translate from 1 to 0 origin */

    if (!Writesdg.dimsizes)
        HGOTO_ERROR(DFE_BADCALL, FAIL);

    if (Writesdg.numbertype == DFNT_NONE)
      {
          if (DFSDsetNT(DFNT_FLOAT32) < 0)
            {
              ret_value = FAIL;
              goto done;
            }
      }
    numtype = Writesdg.numbertype;
    localNTsize = DFKNTsize((numtype | DFNT_NATIVE) & (~DFNT_LITEND));

    if ((rdim >= Writesdg.rank) || (rdim < 0)   /* check dimensions */
        || (dimsize != Writesdg.dimsizes[rdim]))
      {
          HGOTO_ERROR(DFE_BADDIM, FAIL);
      }

    if (!scale)
      {     /* No scale for this dimension */
          if (Writesdg.dimscales)
              HDfreenclear(Writesdg.dimscales[rdim]);
          Ref.scales = 0;
          ret_value = SUCCEED;
          goto done;
      }

    /* get number type and size of this type in this machine  */
    if (Writesdg.numbertype == DFNT_NONE)
      {
          if (DFSDsetNT(DFNT_FLOAT32) < 0)
            {
              ret_value = FAIL;
              goto done;
            }
      }
    numtype = Writesdg.numbertype;
    localNTsize = DFKNTsize((numtype | DFNT_NATIVE) & (~DFNT_LITEND));
    bytesize = (uint32)(dimsize * localNTsize);

    /* allocate space for dimscales if necessary */
    if (!Writesdg.dimscales)
      {
          Writesdg.dimscales =
              (uint8 **) HDmalloc((uint32) Writesdg.rank * sizeof(int8 *));
          if (Writesdg.dimscales == NULL)
            {
              ret_value = FAIL;
              goto done;
            }
          for (i = 0; i < Writesdg.rank; i++)   /* set allocated pointers to NULL */
              Writesdg.dimscales[i] = NULL;
      }

    if (!Writesdg.dimscales[rdim])
      {
          /* allocate dimension scale space if necessary */
          Writesdg.dimscales[rdim] =
              (uint8 *) HDmalloc((uint32) bytesize);
          if (Writesdg.dimscales[rdim] == NULL)
            {
              ret_value = FAIL;
              goto done;
            }
      }

    /* copy scale */
    p1 = (uint8 *) scale;
    p2 = (uint8 *) Writesdg.dimscales[rdim];
    HDmemcpy(p2, p1, bytesize);

    /* Indicate scales modified */
    Ref.scales = 0;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDsetrange
 USAGE
       int DFSDsetrange(max, min)
       VOIDP max;    IN: High value in the scientific data set
       VOIDP min;    IN: Low value in the scientific data set
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Sets the maximum and minimum values to be stored with the scientific
       data set. Since these values are supposed to relate to the data itself,
       it is assumed that they are of the same number type as the data itself.
       One implication of this is that in the C version of "DFSDgetrange"
       the arguments are pointers, rather than simple variables, whereas in
       the FORTRAN version they are simple variables of the same type as the
       data array. This routine does not compute the maximum and minimum values;
       it merely stores the values it is given. As a result, the
       maximum and minimum values may not always reflect the actual maximum
       and minimum vlaues in the data set. When the maximum and minimum
       values are written to a file, the HDF elemement that hold these value
       is cleared, because it is assumed that subsequent data sets will have
       different values for "max" and "min". These values are automatically
       cleared aftera a call to either "DFSDputdata" or "DFSDaddadata".
-----------------------------------------------------------------------------*/
intn
DFSDsetrange(VOIDP maxi, VOIDP mini)
{
    int32       numtype;
    uint32      localNTsize;
    intn        i;
    uint8      *p1, *p2;
    intn        ret_value = SUCCEED;
    CONSTR(FUNC, "DFSDsetrange");    /* for HERROR */

    HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

    p1 = &(Writesdg.max_min[0]);
    for (i = 0; i < 16; i++)
        *p1++ = 0;  /* clear max_min   */

    /* get number type and copy the values to Writesdg   */
    if (Writesdg.numbertype == DFNT_NONE)
        DFSDsetNT(DFNT_FLOAT32);

    numtype = Writesdg.numbertype;
    localNTsize = (uint32)DFKNTsize((numtype | DFNT_NATIVE) & (~DFNT_LITEND));
    p1 = (uint8 *) maxi;
    p2 = (uint8 *) mini;

    HDmemcpy((uint8 *) &(Writesdg.max_min[0]), p1, localNTsize);
    HDmemcpy((uint8 *) &(Writesdg.max_min[localNTsize]), p2, localNTsize);

    Ref.maxmin = 0;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDputdata
 USAGE
       intn DFSDputdata(filename, rank, dimsizes, data)
       char  *filename;     IN: name of file to store scientific data set in
       int   rank;          IN: number of dimensions of data array to be stored
       int32 dimsizes[];    IN: array that holds sizes of dimensions
       VOID  *data;         IN: array holding data to be stored
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Writes scientific data and related information to an HDF file.
       If a new filename is used, "DFSDputdata" functions exactly like
       "DFSDadddata".
 WARNING
       "DFSDputdata" will write data to an existing file by destroying the
       contents of the original file. Use with caution.
-----------------------------------------------------------------------------*/

intn
DFSDputdata(const char *filename, intn rank, int32 dimsizes[], VOIDP data)
{
  intn ret_value;

  /* 0, 0 specify create mode, C style array (row major) */
  ret_value = (DFSDIputdata(filename, rank, dimsizes, data, 0, 0));

  return ret_value;
}

/*-----------------------------------------------------------------------------
NAME
      DFSDadddata
USAGE
      intn DFSDadddata(filename, rank, dimsizes, data)
      char  *filename;    IN: Name of HDF file to store the data set
      intn  rank;         IN: Number of dimensions in the data array to be written
      int32 dimsizes[];   IN: Array holding the size of each dimension
      VOIDP data;         IN: Array holding the data to be stored

RETURNS
      SUCCEED(0) if successful and FAIL(-1) otherwise.

DESCRIPTION
      In addition to appending any multidemensional array of data to an HDF
      file, "DFSDaddata" automatically stores any information pertinent to the
      data set. It will not overwrite existing data in the file. The array
      "data" can be of any valid type. However if no number type has been set
      by "DFSDsetNT", it is assumed that the data type is of type "float32".
      The invocation of "DFSDadddata" triggers the writing of the entire
      scientific data set. That is, when "DFSDadddat" is called, all
      information that has been set by "DFSDset* " calls is written to the
      file, along with the data array itself.
-----------------------------------------------------------------------------*/
intn
DFSDadddata(const char *filename, intn rank, int32 dimsizes[], VOIDP data)
{
  intn  ret_value;

  /* 1, 0 specifies append mode, C style array (row major) */
  ret_value = (DFSDIputdata(filename, rank, dimsizes, data, 1, 0));

  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDrestart
 USAGE
       int DFSDrestart()
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Forces the next read operation to read the first scientific data set
       in the file.
-----------------------------------------------------------------------------*/
intn
DFSDrestart(void)
{
  intn ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDndatasets");

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (Lastfile != NULL)
    *Lastfile = '\0';   /* set to 0-length string instead of NULLing ptr */
  Readref = 0;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDndatasets
 USAGE
       int32 DFSDndatasets(filename)
       char *filename;     IN: filename of HDF file
 RETURNS
       Returns the number of data sets if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Returns the number of scientific datasets in the file. In HDFv3.3
       "DFSDndatasets" replaces "DFSDnumber". In order to maintain backward
       compatibility with existing HDF applications, HDF will continue to
       support "DFSDnumber". However, it is recommended that all new applications
       use "DFSDndatasets" instead of "DFSDnumber".
-----------------------------------------------------------------------------*/
int32
DFSDndatasets(char *filename)
{
  int32       file_id;
  int32       nsdgs = 0;
  int32       ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDndatasets");

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  /* should use reopen if same file as last time - more efficient */
  file_id = DFSDIopen(filename, DFACC_READ);
  if (file_id == FAIL)
    HGOTO_ERROR(DFE_BADOPEN, FAIL);

  nsdgs = (int32)nsdghdr->size;
  if (Hclose(file_id) == FAIL)
      HGOTO_ERROR(DFE_CANTCLOSE,FAIL);

  ret_value = nsdgs;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDclear
 USAGE
       int DFSDclear()
 RETURNS
      Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
      Clears all possible set values. After a call to "DFSDclear", all
      values set by an "DFSDset*" calls will not be written unless they
      are set again.
----------------------------------------------------------------------------*/
intn
DFSDclear(void)
{
  CONSTR(FUNC, "DFSDclear");
  intn   ret_value = SUCCEED;

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  lastnsdg.tag = DFTAG_NULL;
  lastnsdg.ref = 0;
  if (DFSDIclearNT(&Writesdg) < 0)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

  ret_value = DFSDIclear(&Writesdg);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDlastref
 USAGE
       uint16 DFSDlastref()
 RETURNS
       Returns the reference number of the last accessed scientific data set
       if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Get the most recent reference number used in writing or reading a
       scientific data set.
-----------------------------------------------------------------------------*/
uint16
DFSDlastref(void)
{
  CONSTR(FUNC, "DFSDlastref");
  uint16  ret_value;

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, 0);

  ret_value = (uint16) Lastref;

done:
  if(ret_value == 0)    /* 0 is invalid ref */
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDreadref
 USAGE
       int DFSDreadref(filename, ref)
       char   *filename;    IN: name of HDF file containing scientific data set
       uint16 ref;          IN: reference number for next "DFSDgetdata" call
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Specifies teh reference number for the data set to be read during
       the next read operation. This routine is most likely to be used in
       conjunction with "DFANgetlablist" which returns a list of labels for
       a given tag together with their reference numbers. It provides a sort
       of random access to scientific data sets. There is no guarantee that
       reference numbers appear in sequence in an HDF file, so ti is not
       generally safe to assume that a reference number is a sequence number
       for a scientific data set.
-----------------------------------------------------------------------------*/

intn
DFSDreadref(char *filename, uint16 ref)
{
  int32       file_id;
  int32       aid;
  intn        ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDreadref");

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  file_id = DFSDIopen(filename, DFACC_READ);
  if (file_id == DF_NOFILE)
      HGOTO_ERROR(DFE_BADOPEN, FAIL);

  if ((aid = Hstartread(file_id, DFTAG_SDG, ref)) == FAIL
        && (aid = Hstartread(file_id, DFTAG_NDG, ref)) == FAIL)
      HCLOSE_GOTO_ERROR(file_id, DFE_NOMATCH, FAIL);

  Hendaccess(aid);
  Readref = ref;
  Newdata = -1;

  ret_value = Hclose(file_id);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDgetslice
 USAGE
       intn DFSDgetslice(filename, winst, windims, data, dims)
       char  *filename;       IN:  name of HDF file
       int32 winst[];         IN:  array containing the coordinates for the start
                                   of the slice
       int32 windims[];       IN:  array containing the dimensions of the slice
       VOID  *data;           OUT: array for returning the slice
       int32 dims[];          OUT: dimensions of array data
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Gets a part of a scientific data set from a file. "DFSDgetslice"
       accesses the data set last accessed by "DFSDgetdims". If "DFSDgetdims"
       has not been called for the named file, "DFSDgetslice" gets a slice
       from the next data set in the file. The array "winst" specifies the
       coordinates for the beginning of the slice. Array "windims" gives the
       size of the slice. The number of elements in both "winst" and "windims"
       must be equal to the rank of the data set. For example, if the file
       contains a 3D data set, "winst" may contain the values {2,4,3}, while
       "windims" contains the values {3,1,4} and the dims shuld be at least
       {3,1,4}, the same size as the slice. This will extract a 3x4,
       two-dimensional slice, containing the elements between(2,4.3) and
       (4,4,6) from the original data set. The array "data" is the array
       into which the slice is read. It must be at least as big as the desired
       slice. The array "dims" is the array containing the actual dimensions
       of the array "data". The user assigns values to "dims" before calling
       "DFSDgetslice". All parameters assume FORTRAN-style 1-based arrays.
       "DFSDgetslice" is obsoleted by "DFSDreadslab". "DFSDreadslab" is the
       recommended fucntion call to use when reading hyperslabs(previously
       known as data slices). HDFv3.3 will continue to support "DFSDgetslice"
       only to maintain backward compatibility with HDF applications built
       on earlier versions of the library.
----------------------------------------------------------------------------*/
intn
DFSDgetslice(const char *filename, int32 winst[], int32 windims[], VOIDP data,
             int32 dims[])
{
  intn ret_value;

  ret_value = (DFSDIgetslice(filename, winst, windims, data, dims, 0));

  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDstartslice
 USAGE
       int DFSDstartslice(filename)
       char *filename;   IN: name of HDF file to write to
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Prepares the DFSD interface to write a slice to a scientific data set.
       Before calling "DFSDstartslice", you must call "DFSDsetdims" to
       specify the dimensions of the data set to be written to the file.
       "DFSDstartslice" always appends a new data set to an existing file.
       Remember, you must call "DFSDstartslice" before calling "DFSDputslice"
       or "DFSDendslice". "DFSDstarslice" is obsolete in favor of "DFSDstartslab"
       "DFSDstartslab" is the recommended function to call to use when
       beginning hyperslab(i.e. data slabs) opertaions. HDFv3.3 will continue
       to support "DFSDstartslice" only to maintain backward compatibility
       with HDF applications built on earlier version of the library.
-----------------------------------------------------------------------------*/
intn
DFSDstartslice(const char *filename)
{
  intn        i;
  int32       size;
  intn        ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDstartslice");

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (!Writesdg.rank)     /* dimensions not set */
    HGOTO_ERROR(DFE_BADDIM, FAIL);

  Sfile_id = DFSDIopen(filename, DFACC_WRITE);
  if (Sfile_id == DF_NOFILE)
    HGOTO_ERROR(DFE_BADOPEN, FAIL);

  Writesdg.data.tag = DFTAG_SD;

  if (!Writeref)
    Writeref = Hnewref(Sfile_id);
  if (!Writeref)
    HGOTO_ERROR(DFE_BADREF, FAIL);
  Writesdg.data.ref = Writeref;

  if (Writesdg.numbertype == DFNT_NONE)   /* if NT not set,default to float32 */
    DFSDsetNT(DFNT_FLOAT32);

    /* set up to write data */
  size = DFKNTsize(Writesdg.numbertype);
  for (i = 0; i < Writesdg.rank; i++)
    size *= Writesdg.dimsizes[i];

  Writesdg.aid = Hstartwrite(Sfile_id, DFTAG_SD, Writeref, size);
  if (Writesdg.aid == FAIL)
    HCLOSE_GOTO_ERROR(Sfile_id,DFE_BADAID, FAIL);

  /* allocate array for keeping track of dims written */
  Sddims = (int32 *) HDmalloc((uint32) Writesdg.rank * sizeof(int32));
  if (Sddims == NULL)
    HCLOSE_GOTO_ERROR(Sfile_id,DFE_NOSPACE, FAIL);

  for (i = 0; i < Writesdg.rank; i++)
    Sddims[i] = 0;  /* nothing written so far */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDputslice
 USAGE
       int DFSDputslice(winend, data, dims)
       int32 winend[];    IN: dimensions that specify the size of slice to be
                              written
       VOID  *data;       IN: array containing slice to be written
       int32 dims[];      IN: dimensions of array data
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Writes part of a scientific data set to a file. "DFSDputslice" takes
       some contiguous part of an array in memory and stores it as part of
       the scientific data set array specified by "DFSDgetdims". Slices must
       be stored contiguously. Array "windims" specifies the size of the
       slice to be written. The number of elements in "windims" is equal to
       the number of dimensions in the scientific data set array. The array
       "data" is the array in memory containg the slice. The array "dims"
       contains the dimensions of the array "data". Note that the two arrays
       "windim" and "dims" need not be the same since the "windims" arguement
       could refer to a sub-array of "data". In this case only a portion
       of the array "data" is written to the scientific data set. All
       parameters assume FORTRAN-style 1-based arrays. "DFSDputslice" is
       obsoleted by "DFSDwriteslab". DFSDwriteslab is the recommended function
       call to use when writing hyperslabs(previously known as data slices).
       HDFv3.3 will continue to support "DFSDputslice" only to maintain
       backward compatibility with HDF applications built on earlier versions
       of the library.
-----------------------------------------------------------------------------*/
intn
DFSDputslice(int32 winend[], VOIDP data, int32 dims[])
{
  intn ret_value;

  ret_value = (DFSDIputslice(winend, data, dims, 0));

  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDendslice
 USAGE
       int DFSDendslice()
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Terminates the write operation after storing a slice of data in a
       scientific data set. "DFSDendslice" must be called after all the
       slices are written. It checks to insure that the entire data set
       has been written, and returns an error code if not. "DFSDendslice"
       is obsolete in favor of "DFSDendslab". "DFSDendslab" is the
       recommended function call to use when terminating hyperslab(previously
       known as data slices) operations. HDFv3.3 will continue to support
       "DFSDendslice" only to maintain backward compatability with HDF
       applications build on earlier versions of the library.

----------------------------------------------------------------------------*/
intn
DFSDendslice(void)
{
  intn ret_value;

  ret_value = (DFSDIendslice(0));

  return ret_value;
}

/*---------------------------------------------------------------------------
 NAME
       DFSDsetNT
 USAGE
       int DFSDsetNT(numbertype)
       int32 *numbertype;  IN: Number type of the data to be written
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Sets the number type for the data to be written in the next write
       operation. "DFSDsetNT" must be called if a number type other than
       "float32" is to be stored. "DFSDsetNT" and "DFSDsetdims" can be called
       in any order, but they should be called before any other "DFSDset*"
       functions and before "DFSDputdata" or "DFSDadddata". Valid values for
       "DFSDgetNT" are of the general form "DFNT_<numbertype>". If you include
       the headier file "hdf.h" in your program, you can use the following
       symbolic names for the number types:
            32-bit float          DFNT_FLOAT32    5
            64-bit float          DFNT_FLOAT64    6
            8-bit signed int      DFNT_INT8      20
            8-bit unsigned int    DFNT_UINT8     21
            16-bit signed int     DFNT_INT16     22
            16-bit unsigned int   DFNT_UINT16    23
            32-bit signed int     DFNT_INT32     24
            32-bit unsigned int   DFNT_UINT32    25
---------------------------------------------------------------------------*/
intn
DFSDsetNT(int32 numbertype)
{
  uint8       outNT;
  intn        ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDsetNT");

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  outNT = (uint8) (DFKisnativeNT(numbertype) ? DFKgetPNSC(numbertype, DF_MT) :
                  (DFKislitendNT(numbertype) ? DFNTF_PC : DFNTF_HDFDEFAULT));
  if ((numbertype == Writesdg.numbertype)
        && (outNT == Writesdg.filenumsubclass))
    HGOTO_DONE(SUCCEED);

  /* Forget previous numbertype  */
  if (DFSDIclearNT((DFSsdg *) & Writesdg) < 0)
    HGOTO_ERROR(DFE_INTERNAL, FAIL);

  Writesdg.numbertype = numbertype;
  Writesdg.filenumsubclass = outNT;
  Ref.nt = 0;
  Ref.dims = (Ref.dims >= 0 ? 0 : Ref.dims);
  Ref.new_ndg = 0;

  ret_value = (DFKsetNT(numbertype));

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*-------------------------------------------------------------------
* Name:    DFSDIclearNT
* Purpose: Reset all "set" values related to number types
* Inputs:  sdg: pointer to sdg struct to clear
* Globals: Ref
* Returns: 0 on success, FAIL on error with error set
* Users:   DFSDsetNT, HDF users
* Invokes: none
* Remarks:
*--------------------------------------------------------------------*/
intn
DFSDIclearNT(DFSsdg * sdg)
{
  intn        i;
  intn      ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDIclearNT");

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  sdg->numbertype = DFNT_NONE;
  sdg->filenumsubclass = DFNTF_NONE;

  /* free scale pointers. Note: scale pointer array is not freed   */
  /* sdg->dimscales will be freed only when rank is changed        */
  if (sdg->dimscales)
    {
      for (i = 0; i < sdg->rank; i++)
        HDfreenclear(sdg->dimscales[i]);
    }

  Ref.nt = -1;
  Ref.maxmin = -1;    /* maxmin and scales should be changed to */
  Ref.scales = -1;    /* new number type              */
  Ref.new_ndg = -1;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}

/*---------------------------------------------------------------------------
 NAME
       DFSDgetNT
 USAGE
       int DFSDgetNT(numbertype)
       int32 *numbertype;    OUT: Number type of the data in the scientific
                                  data set.
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Gets the number type of the current scientific data set. This
       information is then used by calls such as "DFSDgetdata" and
       "DFSDgetslice". Since "DFSDgetNT" gets the number type of the
       current data set, "DFSDgetdims" must be called before calling
       "DFSDgetNT". Valid values for "DFSDgetNT" are of the general form
       "DFNT_<numbertype>". The following are valid symbolic names and
       their number types:

            32-bit float          DFNT_FLOAT32    5
            64-bit float          DFNT_FLOAT64    6
            8-bit signed int      DFNT_INT8      20
            8-bit unsigned int    DFNT_UINT8     21
            16-bit signed int     DFNT_INT16     22
            16-bit unsigned int   DFNT_UINT16    23
            32-bit signed int     DFNT_INT32     24
            32-bit unsigned int   DFNT_UINT32    25
--------------------------------------------------------------------------- */
intn
DFSDgetNT(int32 *pnumbertype)
{
  intn    ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDgetNT");

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  *(pnumbertype) = Readsdg.numbertype;
  if (*(pnumbertype) == DFNT_NONE)
    HGOTO_ERROR(DFE_BADNUMTYPE, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*---------------------------------------------------------------------------
 NAME
       DFSDpre32sdg
 USAGE
       int DFSDpre32sdg(filename, ref, ispre32)
       char   *filename;      IN:  The name of the HDF file containing the
                                   scientific data set
       uint16 ref;            IN:  Reference number of scientific data set
       int    ispre32;        OUT: Pointer to the results of the inquiry
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Tests if the scientific data set with the specified reference number
       was created by a HDF library that precedes HDF3.2. This routine is
       for programmers who need to know whether a scientific data set was
       written by a version of the HDF library earlier than HDF3.2. If the
       scientific data set was created with a version of HDF prior to v3.2,
       "ispre32" will be set to 1, otherwise it will be set to 0. Based on
       this information, programmers can decide whether or not to transpose
       the corresponding array.
----------------------------------------------------------------------------*/
intn
DFSDpre32sdg(char *filename, uint16 ref, intn *ispre32)
{
    uint32      num;
    int32       file_id;
    intn        found = 0;
    DFnsdgle   *ptr;
    intn       ret_value = SUCCEED;
    CONSTR(FUNC, "DFSDpre32sdg");

    HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

    file_id = DFSDIopen(filename, DFACC_READ);
    if (file_id == FAIL)
        HGOTO_ERROR(DFE_BADOPEN, FAIL);
    ptr = nsdghdr->nsdg_t;
    num = nsdghdr->size;

    while ((num > 0) && (ptr != NULL) && !found)
      {
          if ((ptr->nsdg.tag == DFTAG_SDG)
              && (ptr->nsdg.ref == ref))
            {   /* pure SDG  */
                found = 1;
                *ispre32 = TRUE;
            }
          else if ((ptr->sdg.tag == DFTAG_SDG)
                   && (ptr->sdg.ref == ref))
            {   /* NDGSDG   */
                found = 1;
                *ispre32 = FALSE;
            }
          else
            {
                ptr = ptr->next;
                num--;
            }
      }     /* while  */

    if (((num == 0) && (ptr != NULL)) || ((num != 0) && (ptr == NULL)) || !found)
      HCLOSE_GOTO_ERROR(file_id, DFE_BADTABLE, FAIL);

    if (Hclose(file_id) < 0)
        ret_value = FAIL;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* end of DFSDpre32sdg   */

/******************************************************************************/
/*--------------------- Lower level routines --------------------------------*/
/******************************************************************************/

/* Functions for NDG and SDG stuff                       */

/*--------------------------------------------------------------------------
 * Name:    DFSDIsetnsdg_t
 * Purpose: Set up the NDG/SDG table. Each node has two
            fields: the 1st field is NDG or SDG, the 2nd
            field has value only when it is a special
            NDG, i.e. the data set is float32 and not
            compressed.
 * Inputs:  file_id: pointer to HDF file containing SDG
 * Returns: 0 on success, FAIL on failure with error set
 * Users:   DFSDIopen for READ
 *--------------------------------------------------------------------------*/
static intn
DFSDIsetnsdg_t(int32 file_id, DFnsdg_t_hdr * l_nsdghdr)
{
  uint32      sz_DFnsdgle = (uint32) sizeof(struct DFnsdgle);
  int32       aid;            /* access id */
  int32       ndgs;           /* number of ndg's */
  int32       sdgs;           /* number of sdg's */
  int32       GroupID;
  uint16      intag=DFTAG_NULL;
  uint16      inref=DFTAG_NULL;
  intn        moretags;
  intn        found;
  DFnsdgle   *ntb = NULL;
  DFnsdgle   *stb = NULL;
  DFnsdgle   *new;
  DFnsdgle   *nf;
  DFnsdgle   *nr;
  DFnsdgle   *sf;
  DFnsdgle   *sr;
  DFdi        di;
  DFdi        lnkdd[2];
  uint8      *bufp;
  intn       ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDsetnsdg_t");

    HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (!HDvalidfid(file_id))
    HGOTO_ERROR(DFE_BADCALL, FAIL);

    /* Check if temproray buffer has been allocated */
  if (ptbuf == NULL)
    {
      ptbuf = (uint8 *) HDmalloc(TBUF_SZ * sizeof(uint8));
      if (ptbuf == NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);
    }

  /* MMM:  Talk to Shiming and make sure the change made to the way ndgs
   and sdgs are handled is ok.
   */
  ndgs = Hnumber(file_id, DFTAG_NDG);
  sdgs = Hnumber(file_id, DFTAG_SDG);
  if ((ndgs == FAIL) || (sdgs == FAIL))
    HGOTO_ERROR(DFE_INTERNAL, FAIL);

  if ((ndgs + sdgs) == 0)
    {     /* no sdgs or ndgs in file */
      l_nsdghdr->size = 0;
      l_nsdghdr->nsdg_t = NULL;
      HGOTO_DONE(SUCCEED);
    }
  if ((ntb = (DFnsdgle *) HDmalloc(sz_DFnsdgle)) == NULL)
    HGOTO_ERROR(DFE_NOSPACE, FAIL);

    /* the first node in each table is a dummy node  */
  ntb->nsdg.tag = DFTAG_NULL;     /* set up and init an ndg table  */
  ntb->nsdg.ref = 0;
  ntb->sdg.tag = DFTAG_NULL;
  ntb->sdg.ref = 0;
  ntb->next = NULL;

  if ((stb = (DFnsdgle *) HDmalloc(sz_DFnsdgle)) == NULL)
    HGOTO_ERROR(DFE_NOSPACE, FAIL);

  stb->nsdg.tag = DFTAG_NULL;     /* set up and init an sdg table  */
  stb->nsdg.ref = 0;
  stb->sdg.tag = DFTAG_NULL;  /* this field should be named as */
  stb->sdg.ref = 0;   /* stb->ndg.tag, the ndg to which this */
  stb->next = NULL;   /* sdg belongs.                 */

  aid = Hstartread(file_id, DFTAG_WILDCARD, DFREF_WILDCARD);
  moretags = (aid != FAIL);
  while (moretags)
    {     /* read dd's and put each dd in ntb or stb */
      HQuerytagref(aid, &intag, &inref);
      /* put NDG or SDG on ntb or stb */
      if (intag == DFTAG_NDG)
        {
          nr = ntb;
          nf = ntb;
          while ((inref > nf->nsdg.ref) && (nf->next != NULL))
            {
              nr = nf;
              nf = nf->next;
            }
          /* MMM:  Tlk to Shiming and make sure the way this part was 
             rearranged is ok.
           */
          /* check for duplicate nsdg */
          if (inref == nf->nsdg.ref)
            HGOTO_ERROR(DFE_BADNDG, FAIL);

          /* add a node to the table */
          if ((new = (DFnsdgle *) HDmalloc(sz_DFnsdgle)) == NULL)
            HGOTO_ERROR(DFE_NOSPACE, FAIL);

          new->nsdg.tag = DFTAG_NDG;
          new->nsdg.ref = inref;
          new->sdg.tag = DFTAG_NULL;
          new->sdg.ref = 0;

          if (inref < nf->nsdg.ref)
            {     /* does it go before current node? */
              new->next = nf;
              nr->next = new;
            }
          else
            {     /* or at the end? */
              new->next = nf->next;
              nf->next = new;
            }

          /* Does this NDG have an SDG?       */
          if ((GroupID = DFdiread(file_id, DFTAG_NDG, inref)) < 0)
            HGOTO_ERROR(DFE_BADGROUP, FAIL);

          found = FALSE;
          di.tag = DFTAG_NULL;
          di.ref = 0;
          while ((found == 0) && (DFdiget(GroupID, &di.tag, &di.ref) == 0))
            {
              if (di.tag == DFTAG_SDLNK)
                found = TRUE;
            }

          if (found)
            {     /* read in the tag/refs in the link element */
              if (Hgetelement(file_id, di.tag, di.ref, ptbuf) == (int32) FAIL)
                {
                  DFdifree(GroupID);
                  HGOTO_ERROR(DFE_GETELEM, FAIL);
                }
              bufp = ptbuf;
              UINT16DECODE(bufp, lnkdd[0].tag);
              UINT16DECODE(bufp, lnkdd[0].ref);
              UINT16DECODE(bufp, lnkdd[1].tag);
              UINT16DECODE(bufp, lnkdd[1].ref);
              new->sdg.tag = lnkdd[1].tag;
              new->sdg.ref = lnkdd[1].ref;
              DFdifree(GroupID);
            }
        }   /* end of NDG    */

      if (intag == DFTAG_SDG)
        {
          sr = stb;
          sf = stb;
          while ((inref > sf->nsdg.ref) && (sf->next != NULL))
            {
              sr = sf;
              sf = sf->next;
            }
          if (inref == sf->nsdg.ref)
            HGOTO_ERROR(DFE_BADNDG, FAIL);

          /* insert a new node */
          if ((new = (DFnsdgle *) HDmalloc(sz_DFnsdgle)) == NULL)
            HGOTO_ERROR(DFE_NOSPACE, FAIL);

          new->nsdg.tag = DFTAG_SDG;
          new->nsdg.ref = inref;
          new->sdg.tag = DFTAG_NULL;
          new->sdg.ref = 0;

          if (inref < sf->nsdg.ref)
            {     /* does it go before current node? */
              new->next = sf;
              sr->next = new;
            }
          else
            {     /* or at the end? */
              new->next = sf->next;
              sf->next = new;
            }
          /* Does it belong to  an NDG?    */
          if ((GroupID = DFdiread(file_id, DFTAG_SDG, inref)) < 0)
              HGOTO_ERROR(DFE_BADGROUP, FAIL);
          found = FALSE;
          di.tag = DFTAG_NULL;
          di.ref = 0;
          while ((found == 0) && (DFdiget(GroupID, &di.tag, &di.ref) == 0))
            {
              if (di.tag == DFTAG_SDLNK)
                found = TRUE;
            }
          if (found)
            {     /* read in the tag/refs in the link element */
              if (Hgetelement(file_id, di.tag, di.ref, ptbuf) == (int32) FAIL)
                {
                  DFdifree(GroupID);
                  HGOTO_ERROR(DFE_GETELEM, FAIL);
                }
              bufp = ptbuf;
              UINT16DECODE(bufp, lnkdd[0].tag);
              UINT16DECODE(bufp, lnkdd[0].ref);
              UINT16DECODE(bufp, lnkdd[1].tag);
              UINT16DECODE(bufp, lnkdd[1].ref);
              new->sdg.tag = lnkdd[0].tag;
              new->sdg.ref = lnkdd[0].ref;
              DFdifree(GroupID);
            }
        }   /* end of SDG    */

      /*   get next dd   */
      moretags = (SUCCEED == Hnextread(aid, DFTAG_WILDCARD, DFREF_WILDCARD, DF_CURRENT));
    }     /* gone through the dd blocks   */
  Hendaccess(aid);

  /* merge stb and ntb        */
  /* remove SDGNDG from stb   */
  nf = ntb->next;
  while (nf != NULL)
    {
      inref = nf->sdg.ref;
      if (inref != 0)
        {   /* it has an SDG   */
          sr = stb;
          sf = stb;
          while ((sf->nsdg.ref < inref) && (sf->next != NULL))
            {
              sr = sf;
              sf = sf->next;
            }
          if (sf->nsdg.ref == inref)
            {
              if (sf->sdg.ref != nf->nsdg.ref)
                {
                  HGOTO_ERROR(DFE_BADNDG, FAIL);
                }
              else
                {
                  sr->next = sf->next;
                  HDfreenclear(sf);
                  sdgs--;
                }
            }
        }
      nf = nf->next;
    }

  /* check all SDGNDGs were removed   */
  sf = stb->next;
  while (sf != NULL)
    {
      if (sf->sdg.ref != 0)
        HGOTO_ERROR(DFE_BADSDG, FAIL);
      sf = sf->next;
    }

  /* merge the two tables into one */
  nf = ntb;   /* looking for the end of ntb   */
  while (nf->next != NULL)
    nf = nf->next;
  nf->next = stb->next;   /* the first node in stb is a dummy */
  l_nsdghdr->size = (uint32)(ndgs + sdgs);
  l_nsdghdr->nsdg_t = ntb->next;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  /* Release the first nodes in stb and ntb  */
  if (stb != NULL)
    HDfree((VOIDP) stb);
  if (ntb != NULL)
    HDfree((VOIDP) ntb);

  return ret_value;
}   /* end of DFSDsdtnsdg_t   */

/*-----------------------------------------------------------------------
* Name  DFSDInextnsdg
* Purpose: Returns next ndg or sdg in the file
* Inputs:  nsdghdr: point to the nsdg table
*      nsdg: the structure holds the di of next sdg or ndg
* Returns: 0 on succeeds, FAIL on failure
* -------------------------------------------------------------------*/
static intn
DFSDInextnsdg(DFnsdg_t_hdr * l_nsdghdr, DFdi * nsdg)
{
  uint32      num;
  intn        found = FALSE;
  DFnsdgle   *ptr;
  intn        ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDInextnsdg");

    HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  nsdg->tag = DFTAG_NULL;
  nsdg->ref = 0;
  ptr = l_nsdghdr->nsdg_t;
  num = l_nsdghdr->size;

  if ((ptr == NULL) || (num == 0))
      HGOTO_DONE(SUCCEED);

  if ((lastnsdg.tag == DFTAG_NULL) && (lastnsdg.ref == 0))
    {
      found = TRUE;
    }
  else
    {
      while ((num > 0) && (ptr != NULL) && !found)
        {
          if ((ptr->nsdg.tag == lastnsdg.tag)
              && (ptr->nsdg.ref == lastnsdg.ref))
            {
              if ((ptr = ptr->next) != NULL)
                found = TRUE;
            }
          else
            {
              ptr = ptr->next;
              num--;
            }
        }   /* while  */

      if (((num == 0) && (ptr != NULL))
          || ((num != 0) && (ptr == NULL)) || !found)
        {
          HGOTO_ERROR(DFE_BADTABLE, FAIL);
        }
    }     /* else   */

  if (found)
    {
      nsdg->tag = ptr->nsdg.tag;
      nsdg->ref = ptr->nsdg.ref;
    }

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* end of DFSDInextnsdg   */

/*-----------------------------------------------------------------------------
 * Name:    DFSDIgetndg
 * Purpose: Reads in NDG
 * Inputs:  file_id: pointer to HDF file containing NDG
 *          ref: ref of NDG to read
 *          sdg: pointer to DFSsdg struct to read NDG into
 * Returns: 0 on success, FAIL on failure with error set
 * Users:   HDF programmers, DFSDIsdginfo
 * Invokes: DFgetelement, DFdiread, DFdiget, DFaccess, DFread
        DFSDgetsdg
 * Method:  Reads in NDG using DFdiread.  Gets each tag/ref using DFdiget.
 *          Reads in dimensions using DFgetelement.
 *      Call DFSDgetsdg to read in the rest info.
 *          Mallocs space for these, freeing
 *          previously allocated space.
 * Remarks: This accepts non-float32 data
 *---------------------------------------------------------------------------*/
static intn
DFSDIgetndg(int32 file_id, uint16 tag, uint16 ref, DFSsdg * sdg)
{
  int16       int16var;
  int32       i;
  intn        luf;
  DFdi        elmt;
  DFdi        nt;
  int32       length;
  int32       numtype = 0;    /* current number type */
  int32       fileNTsize = 0; /* size of this NT as it is in the file */
  int32       localNTsize = 0;    /* size of this NT as it is in this machine */
  int32       ret;
  int32       aid;
  int32       GroupID;
  int8        fileNT = 0;     /* file number subclass */
  int8        platnumsubclass = 0;    /* platform number subclass */
  uint8       ntstring[4];
  uint8      *isscales;
  uint8      *buf;
  uint8      *p;              /* temporary pointer for moving things to buffer */
  intn       ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDIgetndg");

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (!HDvalidfid(file_id))
    HGOTO_ERROR(DFE_BADCALL, FAIL);

  if (!ref)
    HGOTO_ERROR(DFE_BADREF, FAIL);

  /* Check if temproray buffer has been allocated */
  if (ptbuf == NULL)
    {
      ptbuf = (uint8 *) HDmalloc(TBUF_SZ * sizeof(uint8));
      if (ptbuf == NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);
    }

  /* read NDG into memory */
  if ((GroupID = DFdiread(file_id, tag, ref)) < 0)
        HGOTO_ERROR(DFE_BADGROUP, FAIL);

  DFSDIclear(sdg);
  if (tag == DFTAG_NDG)
    DFSDIclearNT(sdg);
  Ismaxmin = 0;
  IsCal = FALSE;

  /*
   * Loop through all members of the group
   */
  while (!DFdiget(GroupID, &elmt.tag, &elmt.ref))
    {
      luf = -1;     /* flag value for label/unit/ */
      /* format gets process tag/ref */
      switch (elmt.tag)
        {

        case DFTAG_SD:      /* data tag/ref */
          sdg->data.tag = elmt.tag;   /* put tag/ref in struct */
          sdg->data.ref = elmt.ref;
          break;

        case DFTAG_SDD: /* dimension */
          aid = Hstartread(file_id, elmt.tag, elmt.ref);
          if (aid == FAIL)
            {
              DFdifree(GroupID);
              HGOTO_ERROR(DFE_BADAID, FAIL);
            }

          /* read rank */
          if (Hread(aid, (int32) 2, ptbuf) == FAIL)
            {
              DFdifree(GroupID);
              Hendaccess(aid);
              HGOTO_ERROR(DFE_READERROR, FAIL);
            }
          p = ptbuf;
          INT16DECODE(p, int16var);
          sdg->rank=(intn)int16var;

          /* get space for dimensions */
          sdg->dimsizes = (int32 *) HDmalloc((uint32) sdg->rank *
                                             sizeof(int32));
          if (sdg->dimsizes == NULL)
            {
              DFdifree(GroupID);
              Hendaccess(aid);
              HGOTO_ERROR(DFE_NOSPACE, FAIL);
            }

          /* read dimension record */
          if (Hread(aid, (int32) 4 * sdg->rank, ptbuf) == FAIL)
            {
              DFdifree(GroupID);
              Hendaccess(aid);
              HGOTO_ERROR(DFE_READERROR, FAIL);
            }
          p = ptbuf;
          for (i = 0; i < sdg->rank; i++)
            INT32DECODE(p, sdg->dimsizes[i]);

          /* read tag/ref of NT */
          if (Hread(aid, (int32) 4, ptbuf) == FAIL)
            {
              DFdifree(GroupID);
              Hendaccess(aid);
              HGOTO_ERROR(DFE_READERROR, FAIL);
            }
          p = ptbuf;
          UINT16DECODE(p, nt.tag);
          UINT16DECODE(p, nt.ref);

          /* read actual NT */
          if (Hgetelement(file_id, nt.tag, nt.ref, ntstring) == FAIL)
            {
              DFdifree(GroupID);
              Hendaccess(aid);
              HGOTO_ERROR(DFE_GETELEM, FAIL);
            }

          /* check for any valid NT */
          if (ntstring[1] == DFNT_NONE)
            {
              DFdifree(GroupID);
              Hendaccess(aid);
              HGOTO_ERROR(DFE_BADCALL, FAIL);
            }

          /* if looking for an SDG type must be FLOAT32 */
          if (tag == DFTAG_SDG && ntstring[1] != DFNT_FLOAT32)
            {
              DFdifree(GroupID);
              Hendaccess(aid);
              HGOTO_ERROR(DFE_BADCALL, FAIL);
            }

          /* set NT info */
          numtype = (int32)ntstring[1];
          fileNT = (int8)ntstring[3];
          platnumsubclass = DFKgetPNSC(numtype, DF_MT);
          if ((fileNT != DFNTF_HDFDEFAULT)
              && (fileNT != DFNTF_PC)
              && (fileNT != platnumsubclass))
            {
              DFdifree(GroupID);
              Hendaccess(aid);
              HGOTO_ERROR(DFE_BADCALL, FAIL);
            }
          if (fileNT != DFNTF_HDFDEFAULT)
            {     /* if native or little endian */
              if (fileNT != DFNTF_PC)   /* native */
                numtype |= DFNT_NATIVE;
              else  /* little endian */
                numtype |= DFNT_LITEND;
            }     /* end if */

          sdg->filenumsubclass = ntstring[3];
          sdg->numbertype = numtype;

          /* set size of NT    */
          fileNTsize = DFKNTsize(numtype);
          localNTsize = DFKNTsize((numtype | DFNT_NATIVE) & (~DFNT_LITEND));

          /* read and check all scale NTs */
          for (i = 0; i < sdg->rank; i++)
            {
              if (Hread(aid, (int32) 4, ptbuf) == FAIL)
                {
	              DFdifree(GroupID);
                  Hendaccess(aid);
                  HGOTO_ERROR(DFE_READERROR, FAIL);
                }
              p = ptbuf;
              UINT16DECODE(p, nt.tag);
              UINT16DECODE(p, nt.ref);

              /* read NT itself */
              if (Hgetelement(file_id, nt.tag, nt.ref, ntstring) == FAIL)
                {
	              DFdifree(GroupID);
                  Hendaccess(aid);
                  HGOTO_ERROR(DFE_GETELEM, FAIL);
                }

              /* check for any valid NT */
              if (ntstring[1] == DFNT_NONE)
                {
	              DFdifree(GroupID);
                  Hendaccess(aid);
                  HGOTO_ERROR(DFE_BADCALL, FAIL);
                }

              /* if looking for an SDG type must be FLOAT32 */
              if (tag == DFTAG_SDG && ntstring[1] != DFNT_FLOAT32)
                {
	              DFdifree(GroupID);
                  Hendaccess(aid);
                  HGOTO_ERROR(DFE_BADCALL, FAIL);
                }

            }     /* end for loop */
          Hendaccess(aid);
          break;

        case DFTAG_SDLNK:   /* SDG NDG link */
          break;  /* do nothing in 3.2  */

        case DFTAG_SDL: /* labels */
          if (luf == (-1))
            luf = LABEL;

        case DFTAG_SDU: /* units */
          if (luf == (-1))
            luf = UNIT;

        case DFTAG_SDF: /* formats */
          if (luf == (-1))
            luf = FORMAT;

          if (!sdg->dimsizes)
            {
	          DFdifree(GroupID);
              HGOTO_ERROR(DFE_CORRUPT, FAIL);
            }

          /* get needed size of buffer, allocate */
          length = Hlength(file_id, elmt.tag, elmt.ref);
          if (length == FAIL)
            {
	          DFdifree(GroupID);
              HGOTO_ERROR(DFE_BADLEN, FAIL);
            }
          buf = (uint8 *) HDmalloc((uint32) length);
          if (buf == NULL)
            {
	          DFdifree(GroupID);
              HGOTO_ERROR(DFE_NOSPACE, FAIL);
            }

          /* read in luf */
          if (Hgetelement(file_id, elmt.tag, elmt.ref, buf) == FAIL)
            {
	          DFdifree(GroupID);
              HDfree((VOIDP) buf);
              HGOTO_ERROR(DFE_GETELEM, FAIL);
            }
          p = buf;

          /* allocate data luf space */
          sdg->dataluf[luf] = (char *) HDmalloc((uint32) HDstrlen((char *) p) + 1);

          if (sdg->dataluf[luf] == NULL)
            {
	          DFdifree(GroupID);
              HDfree((VOIDP) buf);
              HGOTO_ERROR(DFE_NOSPACE, FAIL);
            }

          /* extract data luf */
          HDstrcpy(sdg->dataluf[luf], (char *) p);
          p += HDstrlen(sdg->dataluf[luf]) + 1;

          /* get space for dimluf array */
          sdg->dimluf[luf] =
            (char **) HDmalloc((uint32) sdg->rank * sizeof(char *));
          if (sdg->dimluf[luf] == NULL)
            {
	          DFdifree(GroupID);
              HDfree((VOIDP) buf);
              HGOTO_ERROR(DFE_NOSPACE, FAIL);
            }

          /* extract dimension lufs */
          for (i = 0; i < sdg->rank; i++)
            {
              sdg->dimluf[luf][i] = (char *)
                HDmalloc((uint32) HDstrlen((char *) p) + 1);
              if (sdg->dimluf[luf][i] == NULL)
                {
		          DFdifree(GroupID);
                  HDfree((VOIDP) buf);
                  HGOTO_ERROR(DFE_NOSPACE, FAIL);
                }
              HDstrcpy(sdg->dimluf[luf][i], (char *) p);
              p += HDstrlen(sdg->dimluf[luf][i]) + 1;
            }
          HDfree((VOIDP) buf);
          break;

        case DFTAG_SDS: /* scales */
          if (!sdg->dimsizes)
	        {
	          DFdifree(GroupID);
              HGOTO_ERROR(DFE_CORRUPT, FAIL);
            }

          /* set up to read scale */
          aid = Hstartread(file_id, elmt.tag, elmt.ref);
          if (aid == FAIL)
	        {
	          DFdifree(GroupID);
              HGOTO_ERROR(DFE_BADAID, FAIL);
            }

          /* read isscales */
          isscales = (uint8 *) HDmalloc((uint32) sdg->rank);
          if (isscales == NULL)
            {
	          DFdifree(GroupID);
              Hendaccess(aid);
              HGOTO_ERROR(DFE_NOSPACE, FAIL);
            }
          if (Hread(aid, (int32) sdg->rank, isscales) == FAIL)
            {
	          DFdifree(GroupID);
              Hendaccess(aid);
              HGOTO_ERROR(DFE_READERROR, FAIL);
            }

          /* allocate scale pointers */
          sdg->dimscales =
            (uint8 **) HDmalloc((uint32) sdg->rank * sizeof(int8 *));
          if (sdg->dimscales == NULL)
            {
	          DFdifree(GroupID);
              HDfree((VOIDP) isscales);
              Hendaccess(aid);
              HGOTO_ERROR(DFE_NOSPACE, FAIL);
            }

          /* read scales */
          for (i = 0; i < sdg->rank; i++)
            {
              sdg->dimscales[i] = NULL;     /* default */
              if (!isscales[i])
                continue;

              /* space for scale */
              sdg->dimscales[i] = (uint8 *)
                HDmalloc((size_t) (sdg->dimsizes[i] * localNTsize));
              if (sdg->dimscales[i] == NULL)
                {
		          DFdifree(GroupID);
                  HDfree((VOIDP) isscales);
                  Hendaccess(aid);
                  HGOTO_ERROR(DFE_NOSPACE, FAIL);
                }

              if (platnumsubclass == fileNT)
                {   /* no conversion needed */
                  ret = Hread(aid, (int32) sdg->dimsizes[i] * fileNTsize,
                              (uint8 *) sdg->dimscales[i]);
                  if (ret == FAIL)
                    {
			          DFdifree(GroupID);
                      HDfree((VOIDP) isscales);
                      Hendaccess(aid);
                      HGOTO_ERROR(DFE_READERROR, FAIL);
                    }
                }
              else
                {   /* conversion necessary */
                  /* allocate conversion buffer */
                  buf = (uint8 *) HDmalloc((size_t) (sdg->dimsizes[i] * fileNTsize));
                  if (buf == NULL)
                    {
			          DFdifree(GroupID);
                      HDfree((VOIDP) isscales);
                      Hendaccess(aid);
                      HGOTO_ERROR(DFE_NOSPACE, FAIL);
                    }

                  /* read scale from file */
                  ret = Hread(aid,(int32) (sdg->dimsizes[i] * fileNTsize), buf);
                  if (ret == FAIL)
                    {
			          DFdifree(GroupID);
                      HDfree((VOIDP) buf);
                      HDfree((VOIDP) isscales);
                      Hendaccess(aid);
                      HGOTO_ERROR(DFE_READERROR, FAIL);
                    }

                  p = buf;

                  /* convert, all at once */
                  DFKconvert((VOIDP) p, (VOIDP) sdg->dimscales[i], numtype,
                             sdg->dimsizes[i], DFACC_READ, 0, 0);

                  HDfree((VOIDP) buf);
                }
            }
          HDfree((VOIDP) isscales);
          Hendaccess(aid);
          break;

        case DFTAG_SDC: /* coordsys */
          /* find and allocate necessary space */
          length = Hlength(file_id, elmt.tag, elmt.ref);
          if (length == FAIL)
            {
	          DFdifree(GroupID);
              HGOTO_ERROR(DFE_BADLEN, FAIL);
            }

          sdg->coordsys = (char *) HDmalloc((uint32) length);
          if (sdg->coordsys == NULL)
            {
	          DFdifree(GroupID);
              HGOTO_ERROR(DFE_NOSPACE, FAIL);
            }

          /* read coordsys */
          if (Hgetelement(file_id, elmt.tag, elmt.ref,
                          (uint8 *) sdg->coordsys) == FAIL)
	        {
	          DFdifree(GroupID);
              HGOTO_ERROR(DFE_GETELEM, FAIL);
            }
          break;

        case DFTAG_SDM: /* max/min */
          if (fileNT == platnumsubclass)
            {     /* no conversion */
              if (Hgetelement(file_id, elmt.tag, elmt.ref,
                              (uint8 *) &(sdg->max_min[0])) == FAIL)
                {
		          DFdifree(GroupID);
                  HGOTO_ERROR(DFE_GETELEM, FAIL);
                }
            }
          else
            {
              /* conversion needed */
              /* allocate buffer */
              buf = (uint8 *) HDmalloc((size_t) (2 * fileNTsize));
              if (buf == NULL)
                {
 		          DFdifree(GroupID);
                 HGOTO_ERROR(DFE_NOSPACE, FAIL);
                }

              /* read and convert max/min */
              if (Hgetelement(file_id, elmt.tag, elmt.ref, buf) == FAIL)
                {
		          DFdifree(GroupID);
                  HGOTO_ERROR(DFE_GETELEM, FAIL);
                }

              DFKconvert((VOIDP) buf, (VOIDP) &(sdg->max_min[0]), numtype, 2,
                         DFACC_READ, 0, 0);

              HDfree((VOIDP) buf);
            }
          Ismaxmin = 1;
          break;

        case DFTAG_CAL:
          if (fileNT == platnumsubclass)
            {     /* no conversion */
              /* get size of element */
              intn        eltSize = (intn) Hlength(file_id, elmt.tag, elmt.ref);
              if (eltSize == FAIL)
                {
		          DFdifree(GroupID);
                  HGOTO_ERROR(DFE_BADLEN, FAIL);
                }

              if (eltSize == 36)
                {
                   /* element is new, double based type */
                  if (Hgetelement(file_id, elmt.tag, elmt.ref,
                                  (unsigned char *) &sdg->cal) < 0)
	                {
			          DFdifree(GroupID);
                      HGOTO_ERROR(DFE_GETELEM, FAIL);
                    }
                }
              else
                {
                  /* element is old float based type */
                  float32     buf2[4];

                   /* allocate input buffer */
                  if (Hgetelement(file_id, elmt.tag, elmt.ref,
                                  (unsigned char *) buf2) < 0)
	                {
			          DFdifree(GroupID);
                      HGOTO_ERROR(DFE_GETELEM, FAIL);
                    }

                  /* move 'em over */
                  sdg->ioff = (float64) buf2[0];
                  sdg->ioff_err = (float64) buf2[1];
                  sdg->cal = (float64) buf2[2];
                  sdg->cal_err = (float64) buf2[3];
                  sdg->cal_type = DFNT_INT16;

                }
            }
          else
            {
              intn        eltSize;

              /* get size of element */
              eltSize = (intn) Hlength(file_id, elmt.tag, elmt.ref);
              if (eltSize == FAIL)
                {
                  DFdifree(GroupID);
                  HGOTO_ERROR(DFE_BADLEN, FAIL);
                }

              /* allocate buffer */
              buf = (uint8 *) HDmalloc((uint32) eltSize);
              if (buf == NULL)
                {
                  DFdifree(GroupID);
                  HGOTO_ERROR(DFE_NOSPACE, FAIL);
                }

              /* read and convert calibration */
              if (Hgetelement(file_id, elmt.tag, elmt.ref, buf) == FAIL)
                {
                  DFdifree(GroupID);
                  HGOTO_ERROR(DFE_GETELEM, FAIL);
                }

              if (eltSize == 36)
                {
                  /* element is new, double based type */
                 /* read in the 64bit float factors */
                  DFKconvert((VOIDP) buf,
                             (VOIDP) &sdg->cal,
                             DFNT_FLOAT64, 4, DFACC_READ, 0, 0);

                  /* read in the 32bit integer number type */
                  DFKconvert((VOIDP) (buf + 32),
                             (VOIDP) &sdg->cal_type,
                             DFNT_INT32, 1, DFACC_READ, 0, 0);
                }
              else
                {
                  /* element is old float based type */
                  float32     buf2[4];

                  /* convert calibration factors */
                  DFKconvert((VOIDP) buf, (VOIDP) buf2, DFNT_FLOAT32, 4,
                             DFACC_READ, 0, 0);

                  /* move 'em over */
                  sdg->ioff = (float64) buf2[0];
                  sdg->ioff_err = (float64) buf2[1];
                  sdg->cal = (float64) buf2[2];
                  sdg->cal_err = (float64) buf2[3];
                  sdg->cal_type = DFNT_INT16;

                }
              HDfree((VOIDP) buf);
            }
          IsCal = TRUE;
          break;

        case DFTAG_FV:
          if (fileNT == platnumsubclass)
            {     /* no conversion */
              /* get size of element */
              intn        eltSize = (intn) Hlength(file_id, elmt.tag, elmt.ref);
              if (eltSize == FAIL)
                {
                  DFdifree(GroupID);
                  HGOTO_ERROR(DFE_BADLEN, FAIL);
                }

              /* get element */
              if (Hgetelement(file_id, elmt.tag, elmt.ref,
                              (unsigned char *) sdg->fill_value) == FAIL)
                {
                  DFdifree(GroupID);
                  HGOTO_ERROR(DFE_GETELEM, FAIL);
                }
            }
          else
            {
              intn        eltSize;

              /* get size of element  */
              eltSize = (intn) Hlength(file_id, elmt.tag, elmt.ref);
              if (eltSize == FAIL)
                {
                  DFdifree(GroupID);
                  HGOTO_ERROR(DFE_BADLEN, FAIL);
                }

              /* allocate buffer for conversion  */
              buf = (uint8 *) HDmalloc((uint32) eltSize);
              if (buf == NULL)
                {
                  DFdifree(GroupID);
                  HGOTO_ERROR(DFE_NOSPACE, FAIL);
                }

              /* read fill value into buffer */
              if (Hgetelement(file_id, elmt.tag, elmt.ref, buf) == FAIL)
                {
                  DFdifree(GroupID);
                  HGOTO_ERROR(DFE_GETELEM, FAIL);
                }

              /* convert the fill value  */
              DFKconvert((VOIDP) buf, (VOIDP) sdg->fill_value,
                         numtype, 1, DFACC_READ, 0, 0);

              HDfree((VOIDP) buf);
            }
          break;

        case DFTAG_SDT:
          FileTranspose = 1;
          break;
        default:
          if ((elmt.tag <= DFTAG_BREQ) && (elmt.tag >= DFTAG_EREQ))
	        {
	          DFdifree(GroupID);
              HGOTO_ERROR(DFE_BADNDG, FAIL);
            }
          break;
        }
    }

  /* since the dataset exists, the fill value cannot be changed */
  sdg->fill_fixed = TRUE;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}

 /*---------------------------------------------------------------------------*
 * Name:    DFSDIputndg
 * Purpose: Write NDG out to HDF file
 * Inputs:  file_id: HDF file pointer
 *          ref: ref to put NDG with
 *          sdg: struct containing NDG info to put
 * Returns: 0 on success, FAIL on failure with error set
 * Users:   HDF programmers, utilities, DFSDputdata, other routines
 * Invokes: DFIcheck, DFdistart, DFdiadd, DFdiend, DFputelement, DFaccess,
 *          DFwrite
 * Remarks: Writes out NTs
 *---------------------------------------------------------------------------*/
static intn
DFSDIputndg(int32 file_id, uint16 ref, DFSsdg * sdg)
{
  int32       i;
  intn        j;
  intn        luf;
  intn        issdg = 0;      /* issdg=1 if it is NDG SDG  */
  intn        len;
  uint16      luftag;
  uint8      *buf;
  uint8      *Isscales = NULL;
  uint8      *bufp;
  uint8       ntstring[4];
  uint8       platnumsubclass;
  uint8       outNT;          /* file number type subclass */
  int32       GroupID;
  int32       numtype;        /* current number type  */
  int32       fileNTsize;     /* size of this NT as it will be in the file */
  int32       scaleNTsize;    /* size of scale NT as it will be in the file */
  int32       aid;
  DFdi        nt;
  intn        ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDIputndg");

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (!HDvalidfid(file_id))
    HGOTO_ERROR(DFE_BADCALL, FAIL);
  if (!ref)
    HGOTO_ERROR(DFE_BADREF, FAIL);

  /* Check if temproray buffer has been allocated */
  if (ptbuf == NULL)
    {
      ptbuf = (uint8 *) HDmalloc(TBUF_SZ * sizeof(uint8));
      if (ptbuf == NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);
    }

  /* set number type and subclass     */
  if (sdg->numbertype == DFNT_NONE)
    DFSDsetNT(DFNT_FLOAT32);    /* default is float32  */
  numtype = sdg->numbertype;
  fileNTsize = DFKNTsize(numtype);
  scaleNTsize = fileNTsize;   /* for now, assume same. MAY CHANGE */
  outNT = sdg->filenumsubclass;
  platnumsubclass = (uint8)DFKgetPNSC(numtype, (int32)DF_MT);

  /* prepare to start writing ndg   */
  if ((GroupID = DFdisetup(10)) < 0)
    HGOTO_ERROR(DFE_GROUPSETUP, FAIL);

  /* put ND and ref       */
  if (DFdiput(GroupID, sdg->data.tag, sdg->data.ref) < 0)
    HGOTO_ERROR(DFE_PUTGROUP, FAIL);

  if (Ref.nt <= 0)
    {     /* will not execute if has been written in putsdg  */
      /* construct and write out NT */
      ntstring[0] = DFNT_VERSION;   /* version */
      ntstring[1] = (uint8) (numtype & 0xff);   /* type */
      ntstring[2] = (uint8) (fileNTsize * 8);   /* width of number type in bits */
      ntstring[3] = outNT;  /* class: IEEE or machine class */
      if (Hputelement(file_id, DFTAG_NT, ref, ntstring, (int32) 4) == FAIL)
        HGOTO_ERROR(DFE_PUTELEM, FAIL);
      Ref.nt = (intn)ref;
    }

  /* write out NDD (dimension record) */
  if (Ref.dims <= 0)
    { /* new NDD; write rank, dims, data NT and scale NTs */
      /* put rank & dimensions in buffer */
      bufp = ptbuf;
      UINT16ENCODE(bufp, sdg->rank);
      for (i = 0; i < sdg->rank; i++)
        INT32ENCODE(bufp, sdg->dimsizes[i]);

      /* put data NT and scale NTs  in buffer */
      nt.tag = DFTAG_NT;
      nt.ref = (uint16) Ref.nt;     /* same NT for scales too */

      /* "<=" used to put 1 data NT + rank scale NTs in buffer */
      for (i = 0; i <= sdg->rank; i++)
        {   /* scale NTs written even if no scale! */
          UINT16ENCODE(bufp, nt.tag);
          UINT16ENCODE(bufp, nt.ref);
        }
      /* write out NDD record */
      if ( Hputelement(file_id, DFTAG_SDD, ref, ptbuf, (int32) (bufp - ptbuf)) == FAIL)
        HGOTO_ERROR(DFE_PUTELEM, FAIL);
      Ref.dims = (intn)ref;
    }
  /* write dimension record tag/ref */
  if (DFdiput(GroupID, DFTAG_SDD, (uint16) Ref.dims) < 0)
    HGOTO_ERROR(DFE_PUTGROUP, FAIL);

  /* write out label/unit/format */
  for (luf = LABEL; luf <= FORMAT; luf++)
    {
      luftag = (uint16) ((luf == LABEL) ? DFTAG_SDL :
                         (luf == UNIT) ? DFTAG_SDU : DFTAG_SDF);
      bufp = ptbuf;
      /* this block of code checks if luf is NULL, else writes it */
      if (!Ref.luf[luf])
        {   /* if luf was set */
          Ref.luf[luf] = -1;  /* assume it is NULL */

          /* if dataluf non-NULL, set up to write */
          if (sdg->dataluf[luf] && sdg->dataluf[luf][0])
            {
              HDstrcpy((char *) bufp, sdg->dataluf[luf]);
              bufp += HDstrlen(bufp) + 1;
            }
          else
            {     /* dataluf NULL */
              *bufp++ = '\0';
            }

          /* for each dimluf, if non-NULL, set up to write */
          for (i = 0; i < sdg->rank; i++)
            {
              if (sdg->dimluf[luf] && sdg->dimluf[luf][i]
                  && sdg->dimluf[luf][i][0])
                {   /* dimluf not NULL */
                  HDstrcpy((char *) bufp, sdg->dimluf[luf][i]);
                  bufp += HDstrlen(bufp) + 1;
                }
              else
                {   /* dimluf NULL */
                  *bufp++ = '\0';
                }
            }     /* i loop   */
          Ref.luf[luf] = (intn)ref;     /* remember ref */
          if ( Hputelement(file_id, luftag, (uint16) Ref.luf[luf], ptbuf, (int32) (bufp - ptbuf)) == FAIL)
            HGOTO_ERROR(DFE_PUTELEM, FAIL);
        }   /* luf was set */

      /* write luf tag/ref */
      if (Ref.luf[luf] > 0)
        {
          if (DFdiput(GroupID, luftag, (uint16) Ref.luf[luf]) < 0)
            HGOTO_ERROR(DFE_PUTGROUP, FAIL);
        }
    }     /* luf loop     */

  /* check if there is a scale and write it out */
  if (!Ref.scales)
    {     /* if scale set */
      Isscales = (uint8 *) HDmalloc((uint32) sdg->rank);
      if (Isscales == NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);
      Ref.scales = (-1);    /* assume there is no scale */

      /* set up Isscales array */
      for (i = 0; i < sdg->rank; i++)
        {
          if (sdg->dimscales && sdg->dimscales[i])
            {     /* a scale exists */
              Isscales[i] = 1;
              Ref.scales = 0;   /* flag: write out scales */
            }
          else
            Isscales[i] = 0;
        }
    }

  if (!Ref.scales)
    { /* write out scales */
      /* compute space needed for scales */
      len = 0;
      for (i = 0; i < sdg->rank; i++)
        {
          if (Isscales[i] == 1)
            len += (intn) (sdg->dimsizes[i] * scaleNTsize);
        }
      len += sdg->rank;

      aid = Hstartwrite(file_id, DFTAG_SDS, ref, len);
      if (aid == FAIL)
        {
          HDfree((VOIDP) Isscales);
          HGOTO_ERROR(DFE_BADAID, FAIL);
        }

      /* write Isscales */
      if (Hwrite(aid, (int32) sdg->rank, Isscales) == FAIL)
        {
          HDfree((VOIDP) Isscales);
          HGOTO_ERROR(DFE_WRITEERROR, FAIL);
        }

      /* Write scales */
      for (j = 0; j < sdg->rank; j++)
        {
          if (!Isscales[j])
            continue;
          if (platnumsubclass == outNT)
            {     /* no conversion needed */
              if (Hwrite(aid, (int32) (fileNTsize * sdg->dimsizes[j]),
                         (uint8 *) sdg->dimscales[j]) == FAIL)
                {
                  HDfree((VOIDP) Isscales);
                  HGOTO_ERROR(DFE_WRITEERROR, FAIL);
                }
            }
          else
            {     /* convert and write */
              /* allocate buffer */
              buf = (uint8 *) HDmalloc((uint32) (fileNTsize * sdg->dimsizes[j]));
              if (buf == NULL)
                {
                  HDfree((VOIDP) Isscales);
                  HGOTO_ERROR(DFE_NOSPACE, FAIL);
                }
              /* convert, all at once */
              DFKconvert((VOIDP) sdg->dimscales[j], (VOIDP) buf, numtype,
                         sdg->dimsizes[j], DFACC_WRITE, 0, 0);
              /* write it all out */
              if (Hwrite(aid, (int32) (fileNTsize * sdg->dimsizes[j]), buf)
                  == FAIL)
                {
                  HDfree((VOIDP) Isscales);
                  HDfree((VOIDP) buf);
                  HGOTO_ERROR(DFE_WRITEERROR, FAIL);
                }
              HDfree((VOIDP) buf);
            }
        }

      Ref.scales = (intn)ref;
      Hendaccess(aid);
    }
  if (Isscales != NULL)
     HDfree((VOIDP) Isscales);
  Isscales = NULL;
  if (Ref.scales > 0)
    if (DFdiput(GroupID, DFTAG_SDS, (uint16) Ref.scales) < 0)
      HGOTO_ERROR(DFE_PUTGROUP, FAIL);

  /* write coordsys */
  if (!sdg->coordsys || !sdg->coordsys[0])
    Ref.coordsys = (-1);
  if (!Ref.coordsys)
    {
      if ( Hputelement(file_id, DFTAG_SDC, ref, (uint8 *) sdg->coordsys, (int32) (HDstrlen(sdg->coordsys) + 1)) == FAIL)
          HGOTO_ERROR(DFE_PUTELEM, FAIL);
      Ref.coordsys = (intn)ref;
    }
  if (Ref.coordsys > 0)
    {
      if (DFdiput(GroupID, DFTAG_SDC, (uint16) Ref.coordsys) < 0)
          HGOTO_ERROR(DFE_PUTGROUP, FAIL);
    }

  /* write max/min */
  if (!Ref.maxmin)
    {
      if (platnumsubclass == outNT)
        {   /* no conversion */
          if ( Hputelement(file_id, DFTAG_SDM, ref, (uint8 *) &(sdg->max_min[0]), (int32) (2 * fileNTsize)) == FAIL)
              HGOTO_ERROR(DFE_PUTELEM, FAIL);
          Ref.maxmin = (intn)ref;
        }
      else
        {
          /* allocate buffer */
          buf = (uint8 *) HDmalloc((size_t) (2 * fileNTsize));    /* max/min is 8 bytes */
          if (buf == NULL)
              HGOTO_ERROR(DFE_NOSPACE, FAIL);

          /* convert */
          DFKconvert((VOIDP) &(sdg->max_min[0]), (VOIDP) buf,
                     numtype, 2, DFACC_WRITE, 0, 0);

          /* write */
          if ( Hputelement(file_id, DFTAG_SDM, ref, buf, (int32) (2 * fileNTsize)) == FAIL)
            {
              HDfree((VOIDP) buf);
              HGOTO_ERROR(DFE_PUTELEM, FAIL);
            }

          Ref.maxmin = (intn)ref;
          HDfree((VOIDP) buf);
        }
    }
  if (Ref.maxmin > 0)
    {
      if (DFdiput(GroupID, DFTAG_SDM, (uint16) Ref.maxmin) < 0)
          HGOTO_ERROR(DFE_PUTGROUP, FAIL);
    }
  Ref.maxmin = (-1);  /* max/min should be reset for each data set */

    /* Write calibration. */
  if (!Ref.cal)
    {
      if (platnumsubclass == outNT)
        {   /* no conversion */
          if (Hputelement(file_id, DFTAG_CAL, ref,
                          (unsigned char *) &sdg->cal,
                          (int32) 36) < 0)
              HGOTO_ERROR(DFE_PUTELEM, FAIL);
          Ref.cal = (intn)ref;
        }
      else
        {
          /* allocate buffer */
          uint8       buf2[4 * sizeof(float64) + sizeof(int32)];

          /* convert doubles */
          DFKconvert((VOIDP) &sdg->cal, (VOIDP) buf2,
                     DFNT_FLOAT64, 4, DFACC_WRITE, 0, 0);

          /* convert int */
          DFKconvert((VOIDP) &sdg->cal_type, (VOIDP) (buf2 + 32),
                     DFNT_INT32, 1, DFACC_WRITE, 0, 0);

          /* write it into the file */
          if (Hputelement(file_id, DFTAG_CAL, ref,
                          (unsigned char *) buf2, (int32) 36) < 0)
              HGOTO_ERROR(DFE_PUTELEM, FAIL);
          Ref.cal = (intn)ref;

        }
    }

  if (Ref.cal > 0)
    {
      if (DFdiput(GroupID, DFTAG_CAL, (uint16) Ref.cal) < 0)
          HGOTO_ERROR(DFE_PUTGROUP, FAIL);
    }
  Ref.cal = (-1);     /* Calibration should be reset for each data set */

    /* Write fill value.  */
  if (!Ref.fill_value)
    {
      if (platnumsubclass == outNT)
        {   /* No conversion  */
          if (Hputelement(file_id, DFTAG_FV, ref,
                          (unsigned char *) sdg->fill_value,
                          (int32) fileNTsize) == FAIL)
              HGOTO_ERROR(DFE_PUTELEM, FAIL);
          Ref.fill_value = (intn)ref;
        }
      else
        {
          /* Allocate buffer  */
          uint8       buf2[DFSD_MAXFILL_LEN];

          /* Convert from native to IEEE  */
          DFKconvert((VOIDP) sdg->fill_value, (VOIDP) buf2,
                     numtype, 1, DFACC_WRITE, 0, 0);

          /* Write it into the file  */
          if (Hputelement(file_id, DFTAG_FV, ref,
                          (unsigned char *) buf2,
                          (int32) fileNTsize) == FAIL)
              HGOTO_ERROR(DFE_PUTELEM, FAIL);

          Ref.fill_value = (intn)ref;
        }
    }

  /* Check to add to DFgroup  */
  if (Ref.fill_value > 0)
    {
      if (DFdiput(GroupID, DFTAG_FV, (uint16) Ref.fill_value) == FAIL)
          HGOTO_ERROR(DFE_PUTGROUP, FAIL);
    }
  Ref.fill_value = (-1);  /* Fill value should be reset for each data set  */

  if (!Ref.transpose)
    {     /* if transposed, add transpose tag */
      if (Hdupdd(file_id, DFTAG_SDT, ref, DFTAG_SDD, ref) == FAIL)
          HGOTO_ERROR(DFE_DUPDD, FAIL);
      Ref.transpose = (intn)ref;
    }
  if (Ref.transpose > 0)
    {
      if (DFdiput(GroupID, DFTAG_SDT, (uint16) Ref.transpose) < 0)
          HGOTO_ERROR(DFE_PUTGROUP, FAIL);
    }

  if (numtype == DFNT_FLOAT32)
    {     /* if float32, add a DFTAG_SDLNK   */
      DFdi        lnkdd[2];

      issdg = 1;
      lnkdd[0].tag = DFTAG_NDG;
      lnkdd[0].ref = ref;
      lnkdd[1].tag = DFTAG_SDG;
      lnkdd[1].ref = ref;
      bufp = ptbuf;

      for (i = 0; i < 2; i++)
        {
          UINT16ENCODE(bufp, lnkdd[i].tag);
          UINT16ENCODE(bufp, lnkdd[i].ref);
        }
      if ( Hputelement(file_id, DFTAG_SDLNK, ref, ptbuf, (int32) (bufp - ptbuf)) == FAIL)
          HGOTO_ERROR(DFE_PUTELEM, FAIL);

      /* write DFTAG_SDLNK  */
      if (DFdiput(GroupID, DFTAG_SDLNK, ref) < 0)
          HGOTO_ERROR(DFE_PUTGROUP, FAIL);
    }

  /* write out NDG */
  if (DFdiwrite(file_id, GroupID, DFTAG_NDG, ref) < 0)
      HGOTO_ERROR(DFE_GROUPWRITE, FAIL);

  /* write an SDG point to the dataset if it is an NDG SDG  */
  if (issdg)
    {
      if (Hdupdd(file_id, DFTAG_SDG, ref, DFTAG_NDG, ref) == FAIL)
          HCLOSE_GOTO_ERROR(file_id,DFE_DUPDD, FAIL);
    }

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}

/*-----------------------------------------------------------------------------
 * Name:    DFSDIendslice
 * Purpose: Write of data to SDG completed, write SDG and close file
 * Inputs:  isfortran: true if called from Fortran
 * Returns: 0 on success, FAIL on failure with error set
 * Users:   DFSDIputdata
 * Invokes: DFSDputsdg, Hclose, HERROR
 * Method:  call DFSDputsdg, close Sfile_id
 * Remarks: checks that slice writes were completed.
 *---------------------------------------------------------------------------*/
intn
DFSDIendslice(intn isfortran)
{
  intn        i;
  intn        ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDIendslice");

  HEclear();

  if (Sfile_id == DF_NOFILE)
    HGOTO_ERROR(DFE_BADCALL, FAIL);

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  /* check if slice writes complete */
  for (i = 0; i < Writesdg.rank; i++)
    {
      if (!Fortorder && (i == 0) && (Sddims[i] == Writesdg.dimsizes[i]))
        continue;
      if ((isfortran || Fortorder) && (i == Writesdg.rank - 1)
          && (Sddims[i] == Writesdg.dimsizes[i]))
        continue;
      if ((isfortran || Fortorder || i > 0)
          && (!Fortorder || i < Writesdg.rank - 1) && (Sddims[i] == 0))
        continue;

      HGOTO_ERROR(DFE_BADCALL, FAIL);
    }

  if (DFSDIputndg(Sfile_id, Writeref, &Writesdg) < 0)
      HCLOSE_GOTO_ERROR(Sfile_id,DFE_INTERNAL,FAIL);

  /* old nsdg table should be reset next time  */
  if (nsdghdr != NULL)
    {
      if (nsdghdr->nsdg_t != NULL)
        {
          DFnsdgle   *rear, *front;

          rear = nsdghdr->nsdg_t;
          front = rear->next;
          while (rear != NULL)
            {
              HDfreenclear(rear);
              rear = front;
              if (rear != NULL)
                front = rear->next;
            }
          nsdghdr->size = 0;
          nsdghdr->nsdg_t = NULL;
          lastnsdg.tag = DFTAG_NULL;
          lastnsdg.ref = 0;
        }
      HDfreenclear(nsdghdr);
    }

  Lastref = Writeref;     /* remember ref written */
  Writeref = 0;   /* don't know ref to write next */

  Hendaccess(Writesdg.aid);
  ret_value = Hclose(Sfile_id);
  Sfile_id = 0;   /* partial write complete */
  HDfreenclear(Sddims);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}

/******************************************************************************/
/*----------------------- Internal routines ---------------------------------*/
/******************************************************************************/

/*-----------------------------------------------------------------------------
 * Name:    DFSDIopen
 * Purpose: open or reopen a file
 * Inputs:  filename: name of file to open
 *          acc_mode : access mode
 * Returns: file id on success, -1 (FAIL) on failure with error set
 * Users:   HDF systems programmers, many SD routines
 * Invokes: DFopen
 * Remarks: This is a hook for someday providing more efficient ways to
 *          reopen a file, to avoid re-reading all the headers
 *---------------------------------------------------------------------------*/
int32
DFSDIopen(const char *filename, intn acc_mode)
{
  int32       file_id;
  int32       ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDIopen");

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (Sfile_id != DF_NOFILE)  /* in the middle of a partial write */
    HGOTO_ERROR(DFE_ALROPEN, FAIL);

    /* if last filename buffer has not been allocated
     *    allocate buffer for "Lastfile" and open file
     * else if same file as last time
     *    use reopen - more efficient
     *  else
     *    open file for first time
     */
  if (Lastfile == NULL)
    {
      Lastfile = (char *) HDmalloc((DF_MAXFNLEN + 1) * sizeof(char));
      if (Lastfile == NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);
      /* open file */
      if (( file_id = Hopen(filename, acc_mode, (int16) 0)) == FAIL)
        HGOTO_ERROR(DFE_BADOPEN, FAIL);
    }
  else if ((HDstrcmp(Lastfile, filename)) || (acc_mode == DFACC_CREATE))
    {     /* open a new file, delete nsdg table and reset lastnsdg  */
      if (nsdghdr != NULL)
        {
          if (nsdghdr->nsdg_t != NULL)
            {
              DFnsdgle   *rear, *front;

              rear = nsdghdr->nsdg_t;
              while (rear != NULL)
                {
                  front = rear->next;
                  HDfreenclear(rear);
                  rear = front;
                }
              nsdghdr->size = 0;
              nsdghdr->nsdg_t = NULL;
              lastnsdg.tag = DFTAG_NULL;
              lastnsdg.ref = 0;
            }
          HDfreenclear(nsdghdr);
        }

      /* treat create as different file */
      if (( file_id = Hopen(filename, acc_mode, (int16) 0))== FAIL)
        HGOTO_ERROR(DFE_BADOPEN, FAIL);
      Newdata = (-1);   /* data in Readsdg is not fresh */
      Readsdg.data.ref = 0;     /* No SDG read yet */

      /* remember no info written to file */
      Ref.scales = (Ref.scales >= 0) ? 0 : Ref.scales;
      Ref.luf[LABEL] = (Ref.luf[LABEL] >= 0) ? 0 : Ref.luf[LABEL];
      Ref.luf[UNIT] = (Ref.luf[UNIT] >= 0) ? 0 : Ref.luf[UNIT];
      Ref.luf[FORMAT] = (Ref.luf[FORMAT] >= 0) ? 0 : Ref.luf[FORMAT];
      Ref.dims = (Ref.dims >= 0) ? 0 : Ref.dims;
      Ref.coordsys = (Ref.coordsys >= 0) ? 0 : Ref.coordsys;
      Ref.maxmin = (Ref.maxmin >= 0) ? 0 : Ref.maxmin;
      Ref.nt = (Ref.nt >= 0) ? 0 : Ref.nt;
      Ref.transpose = (Ref.transpose >= 0) ? 0 : Ref.transpose;
    }
  else
    {
      if (( file_id = Hopen(filename, acc_mode, (int16) 0))== FAIL)
        HGOTO_ERROR(DFE_BADOPEN, FAIL);
    }

  /* if read, set up nsdg table */
  if (nsdghdr == NULL)
    {
      nsdghdr = (DFnsdg_t_hdr *) HDmalloc((uint32) sizeof(DFnsdg_t_hdr));
      if (nsdghdr == NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);
      nsdghdr->size = 0;
      nsdghdr->nsdg_t = NULL;
    }
  if ((nsdghdr->nsdg_t == NULL) && (acc_mode == DFACC_READ))
    {
      if (DFSDIsetnsdg_t(file_id, nsdghdr) < 0)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);
      lastnsdg.tag = DFTAG_NULL;
      lastnsdg.ref = 0;
    }

  HIstrncpy(Lastfile, filename, DF_MAXFNLEN);
  /* remember filename, so reopen may be used next time if same file */

  ret_value = file_id;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}

/*-----------------------------------------------------------------------------
 * Name:    DFSDIsdginfo
 * Purpose: Locates next sdg in file
 * Inputs:  file_id: pointer to DF file
 * Returns: 0 on success, FAIL on failure with error set
 * Users:   HDF systems programmers, DFSDgetdims, DFSDgetdata
 * Invokes: DFIfind, DFSDIgetndg
 * Method:  Call DFIfind to find SDG, then DFSDIgetndg to read it in to Readsdg
 * Remarks: none
 *---------------------------------------------------------------------------*/
intn
DFSDIsdginfo(int32 file_id)
{
  DFdi        ptr;
  CONSTR(FUNC, "DFSDIsdginfo");
  int32       aid;
  intn        ret_value = SUCCEED;

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (!HDvalidfid(file_id))
    HGOTO_ERROR(DFE_BADCALL, FAIL);

  if (Readref != 0)
    {
      aid = Hstartread(file_id, DFTAG_NDG, Readref);
      if (aid != FAIL)
        {
          ptr.ref = Readref;
          ptr.tag = DFTAG_NDG;
          Hendaccess(aid);
        }
      else
        {
          aid = Hstartread(file_id, DFTAG_SDG, Readref);
          if (aid != FAIL)
            {
              ptr.ref = Readref;
              ptr.tag = DFTAG_SDG;
              Hendaccess(aid);
            }
          else
            HGOTO_ERROR(DFE_BADAID, FAIL);
        }
    }
  else
    {
      if (DFSDInextnsdg(nsdghdr, &ptr) < 0)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);
      if ((ptr.tag != DFTAG_NDG) && (ptr.tag != DFTAG_SDG))
        HGOTO_ERROR(DFE_BADTAG, FAIL);
      if (ptr.ref == DFREF_WILDCARD)
        HGOTO_ERROR(DFE_BADREF, FAIL);
      Readref = ptr.ref;
    }

  /* find next sd object */
  if (DFSDIgetndg(file_id, ptr.tag, ptr.ref, &Readsdg) < 0)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

  /* remember what type of thing we just read */
  Readsdg.isndg = (ptr.tag == DFTAG_NDG) ? 1 : 0;

  Lastref = ptr.ref;  /* remember ref read */
  lastnsdg.tag = ptr.tag;
  lastnsdg.ref = ptr.ref;

  Newdata = 1;    /* now Readsdg is fresh */
  Readref = 0;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}

/*-----------------------------------------------------------------------------
 * Name:    DFSDIrefresh
 * Purpose: get next sdg if Readsdg is not fresh
 * Inputs:  filename
 * Returns: 0 on success, FAIL on failure with error set
 * Users:   HDF systems programmers, functions in dfsdF.c
 * Invokes: DFSDIopen, DFSDIsdginfo
 * Method:  test Newdata and Nextsdg, call DFSDIsdginfo if necessary
 * Remarks: none
 *---------------------------------------------------------------------------*/
intn
DFSDIrefresh(char *filename)
{
  int32       file_id;
  intn        ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDIrefresh");

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (Newdata != 1 || Nextsdg)
    {     /* if Readsdg not fresh  */
      if (( file_id = DFSDIopen(filename, DFACC_READ))== FAIL)
            HGOTO_ERROR(DFE_BADOPEN, FAIL);
      if (DFSDIsdginfo(file_id) < 0)
        HCLOSE_GOTO_ERROR(file_id,DFE_INTERNAL, FAIL);
      if (Hclose(file_id) < 0)
        HGOTO_ERROR(DFE_CANTCLOSE, FAIL);
      Nextsdg = 0;
    }

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}

/*-----------------------------------------------------------------------------
 * Name:    DFSDIisndg
 * Purpose: is the current read sds an sdg or nsdg/ndg
 * Inputs:  isndg: 0 -- pure sdg( written by 3.1); 1 -- nsdg/ndg
 * Returns: 0 on success, FAIL on failure with error set
 * Users:   HDF systems programmers, functions in dfsdF.c
 * Invokes: none
 * Method:  Assigns Readsdg.isndg to isndg.
 * Remarks: none
 *---------------------------------------------------------------------------*/
intn
DFSDIisndg(intn *isndg)
{
  intn ret_value = SUCCEED;

  *isndg = (intn) Readsdg.isndg;

  return ret_value;
}

/*-----------------------------------------------------------------------------
 * Name:    DFSDIgetrrank
 * Purpose: get rank of the current sdg, to transpose dims for Fortran
 * Inputs:  &rank: address to return the rank
 * Returns: 0 on success, FAIL on failure with error set
 * Users:   HDF systems programmers, functions in dfsdF.c
 * Invokes: none
 * Method:  Assigns Readsdg.rank to rank.
 * Remarks: none
 *---------------------------------------------------------------------------*/
intn
DFSDIgetrrank(intn *rank)
{
  intn  ret_value = SUCCEED;

  *rank = (intn) Readsdg.rank;

  return ret_value;
}

/*-----------------------------------------------------------------------------
 * Name:    DFSDIgetwrank
 * Purpose: get rank of the current sdg, to transpose dims for Fortran
 * Inputs:  &rank: address to return the rank
 * Returns: 0 on success, FAIL on failure with error set
 * Users:   HDF systems programmers, functions in dfsdF.c
 * Invokes: none
 * Method:  Assigns Readsdg.rank to rank.
 * Remarks: none
 *---------------------------------------------------------------------------*/
intn
DFSDIgetwrank(intn *rank)
{
  intn ret_value = SUCCEED;

  *rank = (intn) Writesdg.rank;

  return ret_value;
}

/*-----------------------------------------------------------------------------
 * Name:    DFSDIclear
 * Purpose: Reset all "set" values, free allocated space
 * Inputs:  sdg: pointer to sdg struct to clear
 * Globals: Ref
 * Returns: 0 on success, FAIL on error with error set
 * Users:   HDF users, utilities, other routines
 * Invokes: none
 * Method:  Release space in sdg
 * Remarks: none
 *---------------------------------------------------------------------------*/
intn
DFSDIclear(DFSsdg * sdg)
{
  intn        i;
  intn        luf;
  intn        ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDIclear");

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (Sfile_id != DF_NOFILE)  /* cannot clear during slice writes */
    HGOTO_ERROR(DFE_BADCALL, FAIL);

  HDfreenclear(sdg->dimsizes);
  HDfreenclear(sdg->coordsys);

    /* free label/unit/format pointers */
  for (luf = LABEL; luf <= FORMAT; luf++)
    {
      if (sdg->dimluf[luf])
        {   /* free strings */
          for (i = 0; i < sdg->rank; i++)
            HDfreenclear(sdg->dimluf[luf][i]);
        }

      /* free string pointers */
      HDfreenclear(sdg->dimluf[luf]);

      /* free data string */
      HDfreenclear(sdg->dataluf[luf]);
    }

  /* free scale pointers */
  if (sdg->dimscales)
    {
      for (i = 0; i < sdg->rank; i++)
        HDfreenclear(sdg->dimscales[i]);
    }

  /* free array of scale pointers */
  HDfreenclear(sdg->dimscales);
  sdg->rank = 0;

    /* number type is independant to dimsizes   4/7/92  sxu
       sdg->numbertype = DFNT_NONE;
       sdg->filenumsubclass = DFNTF_NONE;
     */
  sdg->aid = (int32) -1;
  sdg->compression = (int32) 0;
  FileTranspose = 0;
  sdg->fill_fixed = FALSE;    /* allow fill_value to be changed */

  Ref.dims = -1;
  Ref.scales = Ref.luf[LABEL] = Ref.luf[UNIT] = Ref.luf[FORMAT] = (-1);
  Ref.coordsys = Ref.maxmin = (-1);
  Ref.new_ndg = -1;
  Ref.fill_value = -1;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}

/*-----------------------------------------------------------------------------
 * Name:    DFSDIgetdata
 * Purpose: Get data from SDG.  Will sequence to next SDG if DFSDgetdims not
 *          called.
 * Inputs:  filename: name of HDF file to use
 *          rank: no of dimensions of array "data"
 *          maxsizes: actual dimensions of array "data"
 *          data: data for returning scientific data
 *          isfortran : 0 if called from C, 1 when called from FORTRAN
 * Returns: 0 on success, FAIL on failure with error set
 * Outputs: actual scientific data in array
 * Users:   DFSDgetdata
 * Invokes: DFSDIgetslice, HDmalloc, HDfree, DFSDIopen, Hclose,
 *          HERROR, DFSDIsdginfo
 * Method:  Open file, call DFSDIsdginfo to read sdg if necessary, set up
 *          window start and end arrays, call DFSDIgetslice.
 * Remarks: maxsizes may be larger than actual size.  In that event, the actual
 *          data may not be contiguous in the array "data"
 *          User sets maxsizes before call.
 *---------------------------------------------------------------------------*/
intn
DFSDIgetdata(const char *filename, intn rank, int32 maxsizes[], VOIDP data,
             intn isfortran)
{
  intn        i;
  int32      *winst;
  int32      *windims;
  int32       file_id;
  intn        ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDIgetdata");

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (Newdata != 1 || Nextsdg)
    {     /* if Readsdg not fresh */
      if (( file_id = DFSDIopen(filename, DFACC_READ))== DF_NOFILE)
        HGOTO_ERROR(DFE_BADOPEN, FAIL);
      if (DFSDIsdginfo(file_id) < 0)
        HCLOSE_GOTO_ERROR(file_id,DFE_INTERNAL, FAIL);
      if (Hclose(file_id) == FAIL)
        HGOTO_ERROR(DFE_CANTCLOSE, FAIL);
    }

  winst = (int32 *) HDmalloc((uint32) Readsdg.rank * sizeof(int32));
  if (winst == NULL)
    HGOTO_ERROR(DFE_NOSPACE,FAIL);

  windims = (int32 *) HDmalloc((uint32) Readsdg.rank * sizeof(int32));
  if (windims == NULL)
    {
      HDfree((VOIDP) winst);
      HGOTO_ERROR(DFE_NOSPACE,FAIL);
    }

  for (i = 0; i < rank; i++)
    {
      winst[i] = 1;
      windims[i] = Readsdg.dimsizes[i];
    }

  ret_value = DFSDIgetslice(filename, winst, windims, data, maxsizes, isfortran);
  Nextsdg = 1;
  HDfree((VOIDP) winst);
  HDfree((VOIDP) windims);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}

/*-----------------------------------------------------------------------------
 * Name:    DFSDIputdata
 * Purpose: Writes entire SDG to file
 * Inputs:  filename: name of HDF file to use
 *          rank: rank of data array
 *          dimsizes: sizes of the dimensions of data array
 *          data: array that holds data
 *          accmode: 0 if write to new file, 1 if append to file
 *          isfortran: 0 if C, 1 if FORTRAN
 * Globals: Writeref
 * Returns: 0 on success, FAIL on failure with error set
 * Users:   HDF users, utilities, other routines
 * Invokes: DFSDIopen, Hclose, HDmalloc, HDfree, DFSDIputslice,
 *          DFSDstartslice, DFSDIendslice
 * Method:  Create file if necessary, allocate arrays, call slice routines
 *---------------------------------------------------------------------------*/
intn
DFSDIputdata(const char *filename, intn rank, int32 *dimsizes, VOIDP data,
             intn accmode, intn isfortran)
{
  int32       file_id;
  intn        ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDIputdata");

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (!accmode)
    {     /* new file */
      if (( file_id = DFSDIopen(filename, DFACC_CREATE))== DF_NOFILE)
        HGOTO_ERROR(DFE_BADOPEN, FAIL);
      if (Hclose(file_id) == FAIL)
        HGOTO_ERROR(DFE_CANTCLOSE, FAIL);
    }

  if (Ref.dims)
    {     /* don't call setdims if already called */
      if (DFSDsetdims(rank, dimsizes) < 0)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);
    }

  if (DFSDstartslice(filename) < 0)
    HGOTO_ERROR(DFE_INTERNAL, FAIL);

  if (DFSDIputslice(Writesdg.dimsizes, data, dimsizes, isfortran) < 0)
    HGOTO_ERROR(DFE_INTERNAL, FAIL);

  ret_value = DFSDIendslice(isfortran);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}

/*----------------------------------------------------------------------------
 * Name:    DFSDIgetslice
 * Purpose: Get slice of data from SDG.  Will sequence to next SDG if
 *          DFSDgetdims, DFSDgetdata or DFSDgetslice not called earlier.
 * Inputs:  filename: name of HDF file to use
 *          winst: array of size = rank of data, containing start of slice
 *          windims: array of size rank, containing size of slice
 *          data: array for returning slice
 *          dims: dimensions of array data
 *          isfortran : 0 if called from C, 1 when called from FORTRAN
 * Returns: 0 on success, FAIL on failure with error set
 * Outputs: slice of data in data
 * Users:   DFSDIgetdata
 * Invokes: DFSDIopen, Hclose, HERROR, DFSDIsdginfo, DFaccess, DFread
 * Method:  Open file, call DFSDIsdginfo to read sdg if necessary, read the
 *          data, convert types if necessary, place in data as appropriate
 *          data is assumed column major for FORTRAN, row major for C
 * Remarks: dims may be larger than size of slice.  In that event, the actual
 *          data may not be contiguous in the array "data".
 *          User sets dims before call.
 *--------------------------------------------------------------------------*/

/*****************************************************************************/
/* DESIGN DECISIONS                                                          */
/*****************************************************************************/
/*
   A. All stride/index/offset value will, when this is done -- refer to
   element counts rather than byte counts in the name of consistency.

   B. The conversion buffers/allcated areas... will all be char buffers --
   providing that the Cray-2 is cooperative.
 */
/*****************************************************************************/

/*****************************************************************************/
/* CHANGE LOG                                                                */
/*****************************************************************************/
/*
   A.
 */
/*****************************************************************************/
intn
DFSDIgetslice(const char *filename, int32 winst[], int32 windims[],
              VOIDP data, int32 dims[], intn isfortran)
{
  intn        rank;           /* number of dimensions in data[] */
  int32       leastsig;       /* fastest varying subscript in the array */
  int32       error;          /* flag if an error occurred, */
                              /*  used by DFconvert macro */
  int32       convert;        /* true if machine NT != NT to be read */
  int32       transposed;     /* true if we must transpose the data before writing */
  int32       done;           /* true if we are at the end of the slice */
  int32       aid;
  int32       i, j;           /* temporary loop index */
  int32       issdg;          /* 1 -- pure sdg. do what HDF3.1 does   */
  int32      *wstart;         /* tmp array containing starting slice dims */
  int32      *wdims;          /* tmp array containing the slice size */
  int32      *adims;          /* tmp array containing the dimensions of data[] */
  int32      *fdims;          /* tmp array containing the dimensions */
                              /*  of the dataset in the file */
  int32       numtype;        /* current number type  */
  int32       fileNTsize;     /* size of this NT in the file  */
  int32       localNTsize;    /* size of this NT as it occurs in this machine */
  int32       numelements;    /* number of floats to read at once */
  int32       readsize;       /* number of bytes to read at once */
  int32       datastride;     /* number of floats in one row of data[] */
  int32      *offset;         /* array for accessing the next element in data[] */
  int32      *foffset;        /* array for accessing the next element in the file */
  int32      *dimsleft;       /* array for tracking the current position in data[] */
  int32       isnative;
  int32       fileoffset;     /* offset into the current dataset in the file */
  uint8       platnumsubclass;    /* class of this NT for this platform */
  uint8       fileNT;         /* file number subclass */
  uint8      *scatterbuf;     /* buffer to hold the current row contiguously */
  uint8      *sp;             /* ptr into scatterbuf      */
  uint8      *datap;          /* ptr into data[] at starting offset */
                              /* of current block */
  uint8      *dp;             /* ptr into data[] at an element of the current row */
  uint8      *buf;            /* buffer containing the converted current row */
  int32       file_id;        /* HDF file pointer */
  intn        ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDIgetslice");

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (data == NULL)
    HGOTO_ERROR(DFE_BADPTR, FAIL);

  if (( file_id = DFSDIopen(filename, DFACC_READ))== DF_NOFILE)
    HGOTO_ERROR(DFE_BADOPEN, FAIL);

  if (Newdata != 1)
    {     /* if Readsdg not fresh */
      if (DFSDIsdginfo(file_id) < 0) /* reads next SDG from file */
        HCLOSE_GOTO_ERROR(file_id,DFE_INTERNAL, FAIL);
    }
  rank = Readsdg.rank;
  numtype = Readsdg.numbertype;
  fileNT = Readsdg.filenumsubclass;
  issdg = Readsdg.isndg ? 0 : 1;
  isnative = DFNT_NATIVE;
  localNTsize = DFKNTsize((numtype | isnative) & (~DFNT_LITEND));
  fileNTsize = DFKNTsize(numtype);
  platnumsubclass = (uint8)DFKgetPNSC(numtype & (~DFNT_LITEND), (int32)DF_MT);

  /* get dimensions of slice to extract, set nwindims. also err check */
  for (i = 0; i < (int32) rank; i++)
    {
      /* check validity for the dimension ranges */
      if ((windims[i] < 1) || (winst[i] < 1)
          || (winst[i] + windims[i] - 1 > Readsdg.dimsizes[i]))
        {
          HCLOSE_GOTO_ERROR(file_id, DFE_BADDIM, FAIL);
        }
      /* check if space allocated is sufficient */
      if (dims[i] < windims[i])
        {
          HCLOSE_GOTO_ERROR(file_id, DFE_NOTENOUGH, FAIL);
        }
    }
  /* allocate buffers */
  wstart = (int32 *) HDmalloc((size_t) (4 * rank) * sizeof(int32));
  if (wstart == NULL)
    {
      HCLOSE_GOTO_ERROR(file_id, DFE_NOSPACE, FAIL);
    }
  wdims = wstart + rank;
  adims = wdims + rank;
  fdims = adims + rank;

  /* copy arrays to private workspace (so that they are in row major order) */
  for (i = 0; i < (int32) rank; i++)
    {
      int32       ii = (issdg && isfortran) ? rank - i - 1 : i;

      adims[i] = dims[ii];
      ii = (issdg && FileTranspose) ? rank - i - 1 : i;
      wstart[i] = winst[ii] - 1;    /* translate to 0 origin */
      wdims[i] = windims[ii];
      fdims[i] = Readsdg.dimsizes[ii];
    }

  convert = (fileNT != platnumsubclass);  /* is conversion necessary */
  transposed = issdg && (isfortran ^ FileTranspose);  /* is transposition needed */

  /*
   * Note that if the data is transposed we must work on a row by row
   * basis and cannot collapse dimensions.
   */
  if (!transposed)
    {
      /* collapse dimensions if contiguous both in the file and in memory */
      for (i = (int32) rank - 1; i > 0; i--)
        {   /* stop before most sig dim */
          if (adims[i] > wdims[i]     /* not all of data[] will be filled */
              || wstart[i] != 0   /* reading only part of the dataset */
              || wdims[i] < fdims[i])
            {
              break;
            }
          wstart[i - 1] *= fdims[i];
          wdims[i - 1] *= wdims[i];
          adims[i - 1] *= adims[i];
          fdims[i - 1] *= fdims[i];
          rank--;
        }
    }
  leastsig = (int32) rank - 1;    /* which is least sig dim */

  /* position at start of data set */
  aid = Hstartread(file_id, Readsdg.data.tag, Readsdg.data.ref);
  if (aid == FAIL)
    {
      HDfree((VOIDP) wstart);
      HCLOSE_GOTO_ERROR(file_id,DFE_BADAID, FAIL);
    }

  error = 0;
  if (rank == 1 && !convert)
    {
      /* all data is contiguous with no conversions */
      readsize = adims[0] * fileNTsize;
      if ((Hseek(aid, wstart[0] * fileNTsize, 0) == FAIL)
          || (readsize != Hread(aid, readsize, (uint8 *) data)))
        {
          error = 1;
        }
    }
  else
    {
      /*
       * The data must be further manipulated.
       * It may be transposed, may need conversion, may not be contiguous, or
       * any combination of these.
       */
      numelements = wdims[leastsig];
      readsize = numelements * fileNTsize;

      /* allocate 1 row buffers */
      if (convert)
        {
          if ((buf = (uint8 *) HDmalloc((uint32) readsize)) == NULL)
            {
              HDfree((VOIDP) wstart);
              Hendaccess(aid);
              HCLOSE_GOTO_ERROR(file_id, DFE_NOSPACE, FAIL);
            }
        }
      else
        buf = NULL;

      if (transposed)
        {
          scatterbuf =
            (uint8 *) HDmalloc((size_t) (numelements * localNTsize));

          if (scatterbuf == NULL)
            {
              HDfree((VOIDP) wstart);
              HDfree((VOIDP) buf);
              Hendaccess(aid);
              HCLOSE_GOTO_ERROR(file_id, DFE_NOSPACE, FAIL);
            }
        }
      else
        scatterbuf = NULL;

      offset = (int32 *) HDmalloc((size_t) (3 * rank) * sizeof(int32));
      if (offset == NULL)
        {
          HDfree((VOIDP) wstart);
          HDfree((VOIDP) buf);
          HDfree((VOIDP) scatterbuf);
          Hendaccess(aid);
          HCLOSE_GOTO_ERROR(file_id, DFE_NOSPACE, FAIL);
        }
      foffset = offset + rank;
      dimsleft = foffset + rank;

      /* compute initial position in the data */
      for (i = leastsig; i >= 0; i--)
        dimsleft[i] = wdims[i];

      /* compute offsets in the source array */
      if (transposed)
        {
          offset[0] = 1 * localNTsize;
          for (i = 0; i < leastsig; i++)
            offset[i + 1] = offset[i] * adims[leastsig - i];
        }
      else
        {
          offset[leastsig] = 1 * localNTsize;
          for (i = leastsig; i > 0; i--)
            offset[i - 1] = offset[i] * adims[i];
        }
      datastride = offset[leastsig];

      /* compute offsets in the file */
      for (i = leastsig, foffset[i] = 1 * fileNTsize; i > 0; i--)
        foffset[i - 1] = foffset[i] * fdims[i];

       /*
        * Compute starting position in file
        * All file reads are done relative to this starting offset.
        * Cumulative offset is from most sig to next to least sig dim.
        */
      for (i = 0, fileoffset = 0; i < leastsig; i++)
        fileoffset = (fileoffset + wstart[i]) * fdims[i + 1];
      fileoffset += wstart[leastsig];   /* adjust for last dim */
      fileoffset *= fileNTsize;     /* convert to bytes */

      datap = (uint8 *) data;
      done = 0;
      /* -- now read in the data */
      do
        {
          /* move to the next data element in the file */
          if (Hseek(aid, fileoffset, 0) == FAIL)
            {
              error = 1;
              break;
            }

          /* read and convert one contiguous block of data */
          if (convert)
            {
              if (readsize != Hread(aid, readsize, buf))
                {
                  error = 1;
                  break;
                }
              DFKconvert((VOIDP) buf, transposed ? (VOIDP) scatterbuf :
                         (VOIDP) datap, numtype, numelements, DFACC_READ, 0, 0);
            }
          else
            {
              if (readsize != Hread(aid, readsize,
                                    transposed ? scatterbuf : datap))
                {
                  error = 1;
                  break;
                }
            }
          if (transposed)
            {
              /* scatter out the elements of one row */
#ifdef UNICOS
#pragma ivdep
#endif
              for (dp = datap, sp = scatterbuf, i = 0; i < numelements; i++)
                {
                  for (j = 0; j < localNTsize; j++)
                    *(dp + j) = *(sp + j);
                  sp += localNTsize;
                  dp += datastride;
                }
            }

          /*
           * Find starting place of the next row/block.
           * Note that all moves are relative:
           *   this preserves the starting offsets of each dimension
           */
          for (i = leastsig - 1; i >= 0; i--)
            {
              if (--dimsleft[i] > 0)
                {
                  /* move to next element in the current dimension */
                  datap += offset[i];
                  fileoffset += foffset[i];
                  break;
                }
              else
                {
                  dimsleft[i] = wdims[i];
                  /*
                   * Note that we are still positioned at the beginning of
                   * the last element in the current dimension
                   */
                  /* move back to the beginning of dimension i */
                  datap -= offset[i] * (wdims[i] - 1);
                  /* move back to beginning read position of dimension i */
                  fileoffset -= foffset[i] * (wdims[i] - 1);
                  if (i == 0)
                    done = 1;
                }
            }
        }
      while (!done && leastsig > 0);

      if (buf != NULL)
         HDfree((VOIDP) buf);
      if (scatterbuf != NULL)
         HDfree((VOIDP) scatterbuf);
      if (offset != NULL)
         HDfree((VOIDP) offset);
    }

  Hendaccess(aid);
  HDfree((VOIDP) wstart);
  if (error)
    {
      Hclose(file_id);
      ret_value = FAIL;
    }
  else
    ret_value = Hclose(file_id);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}

/*----------------------------------------------------------------------------
 * Name:    DFSDIputslice
 * Purpose: Put slice of data to SDG.
 * Inputs:  windims: array of size rank, containing size of slice
 *          data: array containing slice
 *          dims: dimensions of array data
 *          isfortran: 0 for C, 1 for Fortran
 * Returns: 0 on success, FAIL on failure with error set
 * Users:   DFSDputslice
 * Invokes: DFwrite, HDmalloc, HDfree,DFKnumout(if conversion
        required)
 * Method:  check dimensions for contiguity, convert types if necessary
 *          write to file
 * Remarks: dims may be larger than size of slice.  In that event, the actual
 *          data may not be contiguous in the array "data".
 *          DFSDstartslice must have been called first
 *          If DFKnumout is called, DFSDsetNT may need to have been
 *      called
 *      Note, writes must be contiguous - successive calls to putslice
 *          must write out array consecutively, according to the setting
 *          of the Fortorder variable - row major if 0, column major if 1
 *--------------------------------------------------------------------------*/
intn
DFSDIputslice(int32 windims[], VOIDP data, int32 dims[], intn isfortran)
{
  intn        rank;           /* number of dimensions in data[] */
  int32       leastsig;       /* fastest varying subscript in the array */
  int32       convert;        /* true if machine NT = NT to be written */
  int32       contiguous;     /* true if there are no gaps in the data to be written */
  int32       numtype;        /* current number type */
  int32       platnumsubclass;    /* class of this NT for this platform */
  int32       fileNTsize;     /* size of this NT as it will be in the file */
  int32       fileNT;         /* class of NT for the data to write */
  int32       isnative;
  int32       localNTsize;    /* size of this NT as it occurs in theis machine */
  int32       ret = SUCCEED;  /* return code from DFwrite */
  int32       i, j;           /* temporaries */
  int32       numelements;    /* number of elements to write out per row */
  int32       writesize;      /* number of bytes to write out per row */
  int32       datastride;     /* number of bytes in one row of data[] */
  uint8      *datap;          /* pointer into data[] at */
                                /*  the start of the current row */
  uint8      *buf;            /* buffer containing converted current row */
  intn        ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDIputslice");

  /* shut compiler up */
  isfortran = isfortran;

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (!data)
    HGOTO_ERROR(DFE_BADPTR, FAIL);

  if (Sfile_id == DF_NOFILE)
    HGOTO_ERROR(DFE_BADCALL, FAIL);

  rank = Writesdg.rank;

  for (i = 0; i < (int32) rank; i++)
    {
      /* check validity for the dimension ranges */
      if ((windims[i] <= 0) || (windims[i] > Writesdg.dimsizes[i]))
        HGOTO_ERROR(DFE_BADDIM, FAIL);

          /* check if space allocated is sufficient */
      if (dims[i] < windims[i])
        HGOTO_ERROR(DFE_NOTENOUGH, FAIL);
    }

  /* check to see if the slices fit together */
  /* Same for Fortran or C    */
  /* find the first significant dimension */
  for (i = 0; windims[i] == 1 && i < (int32) rank - 1; i++)
    /* empty */ ;
  /* check that all 'lesser' dims match */
  for (j = i + 1; j < (int32) rank; j++)
    {
      if (Writesdg.dimsizes[j] != windims[j])
        HGOTO_ERROR(DFE_BADDIM, FAIL);
    }

  /* update Sddims to reflect new write */
  Sddims[i] += windims[i];
  for (; i > 0 && Sddims[i] >= Writesdg.dimsizes[i]; i--)
    {
      Sddims[i - 1] += Sddims[i] / Writesdg.dimsizes[i];
      /* promote the unit */
      Sddims[i] %= Writesdg.dimsizes[i];
    }

  leastsig = (int32) rank - 1;    /* which is least sig dim */
  numtype = Writesdg.numbertype;

  /* get class of this num type for this platform */
  fileNT = (int32)Writesdg.filenumsubclass;
  isnative = DFNT_NATIVE;
  fileNTsize = DFKNTsize(numtype);
  localNTsize = DFKNTsize((numtype | isnative) & (~DFNT_LITEND));
  platnumsubclass = (int32)DFKgetPNSC(numtype & (~DFNT_LITEND), (int32)DF_MT);
  convert = (platnumsubclass != fileNT);

  contiguous = 1;
  for (i = 0; contiguous && i < (int32) rank; i++)
    {
      /* check if data at the end of the users array will be contiguous */
      if (dims[i] > Writesdg.dimsizes[i])
        contiguous = 0;
      /* Note: if a winstart[] array is ever added, will also need */
      /*      to check if data at start of users array will be */
      /*      contig                  */
    }

  /*
     *  2 Factors that determine how we write (in order of importance)
     *  conversion and contiguous
     */
  datap = (uint8 *) data;
  if (!convert && contiguous)
    {
      /* compute total number of elements to write */
      for (i = 0, numelements = 1; i < (int32) rank; i++)
        numelements *= windims[i];
      writesize = numelements * fileNTsize;

      if ( Hwrite(Writesdg.aid, writesize, (uint8 *) data) == FAIL)
            HCLOSE_GOTO_ERROR(Sfile_id,DFE_WRITEERROR, FAIL);
    }
  else
    {     /* must step through the data */
      /* compute number of occurrences of the least sig dim */
      if (Fortorder)
        {
          for (i = (int32) rank - 1, j = 1; i > 0; i--)
            j *= windims[i];
        }
      else
        {
          for (i = 0, j = 1; i < (int32) rank - 1; i++)
            j *= windims[i];
        }

      numelements = windims[leastsig];
      writesize = numelements * fileNTsize;
      datastride = dims[leastsig] * localNTsize;
      if (convert)
        {
          buf = (uint8 *) HDmalloc((uint32) writesize);
          if (buf == NULL)
            HCLOSE_GOTO_ERROR(Sfile_id,DFE_NOSPACE, FAIL);
          for (i = 0; i < j; i++, datap += datastride)
            {
              DFKconvert((VOIDP) datap, (VOIDP) buf, numtype,
                         numelements, DFACC_WRITE, 0, 0);
              ret = Hwrite(Writesdg.aid, writesize, buf);   /* done */
              if (ret == FAIL)
                {
                  HDfree((VOIDP) buf);
                  HCLOSE_GOTO_ERROR(Sfile_id,DFE_WRITEERROR, FAIL);
                }
            }
          HDfree((VOIDP) buf);
        }
      else
        {   /* !contiguous      */
          for (i = 0; i < j; i++, datap += datastride)
              if ( Hwrite(Writesdg.aid, writesize, datap) == FAIL)
                  HCLOSE_GOTO_ERROR(Sfile_id,DFE_WRITEERROR, FAIL);
        }
    }

  ret_value = (ret >= 0 ? 0 : -1);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDgetcal
 USAGE
       int DFSDgetcal(pcal, pcal_err, pioff, pioff_err, cal_nt)
       float64 *pcal;       OUT: calibration factor
       float64 *pcal_err;   OUT: calibration error value
       float64 *pioff;      OUT: uncalibrated offset value
       float64 *pioff_err;  OUT: uncalibrated offset error value
       int32   *cal_nt;     OUT: Nunber type of uncalibrated data

 RETURNS
       Returns SUCCED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       This routine reads the calibration record, if there is one, attached
       to the scientific data set. A calibration record contains four 64-bit
       floating point values followed by a 32-bit integer, to be interpreted
       as follows:

            cal        - calibration factor
            cal_err    - calibration error
            offset     - uncalibrated offset
            offset_err - uncalibrated offset error
            num_type   - number type of uncalibrated data

       The relationship between a value 'iy' stored in a data set and the
       actual value 'y' is defined as:

            y = cal * (iy - offset)

       The variable "offset_err" contains a potential error of offset,
       and "cal_err" contains a potential error of "cal". Currently the
       calibration record is provided for information only. The DFSD
       interface performs no operations on the data based on the calibration
       tag.

----------------------------------------------------------------------------*/
intn
DFSDgetcal(float64 *pcal, float64 *pcal_err, float64 *pioff,
           float64 *pioff_err, int32 *cal_nt)
{
  intn      ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDgetcal");

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  if (Newdata < 0)
    HGOTO_ERROR(DFE_BADCALL, FAIL);

  if (IsCal)
    {
      *pcal = Readsdg.cal;
      *pcal_err = Readsdg.cal_err;
      *pioff = Readsdg.ioff;
      *pioff_err = Readsdg.ioff_err;
      *cal_nt = Readsdg.cal_type;
      ret_value = SUCCEED;
    }
  else
    HGOTO_ERROR(DFE_NOVALS, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* DFSDgetcal */

/*-----------------------------------------------------------------------------
 NAME
       DFSDsetcal
 USAGE
       intn DFSDsetcal(cal, cal_err, ioff, ioff_err, cal_nt)
       float64 cal;         IN: calibration factor
       float64 cal_err;     IN: calibration error
       float64 ioff;        IN: uncalibrated offset
       float64 ioff_err;    IN: uncalibrated offset error
       int32   cal_nt;      IN: number type of uncalibrated data
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Sets the calibration information associated with data. This routine
       sets the calibration record attached to a data set. A calibration
       record contains four 64-bit floating point values followed by a
       32-bit integer, to be interpreted as follows:

            cal        - calibration factor
            cal_err    - calibration error
            offset     - uncalibrated offset
            offset_err - uncalibrated offset error
            num_type   - number type of uncalibrated data

       The relationship between a value 'iy' stored in a data set and the
       actual value 'y' is defined as:

            y = cal * (iy - offset)

       The variable "offset_err" contains a potential error of offset,
       and "cal_err" contains a potential error of "cal". Currently the
       calibration record is provided for information only. The DFSD
       interface performs no operations on the data based on the calibration
       tag. "DFSDsetcal" works like other "DFSDset*" routines, with one
       exception: the calibration information is automatically cleared
       after a call to "DFSDputdata" or "DFSDadddata". Hence, "DFSDsetcal"
       must be called anew for each data set that is to be written.
----------------------------------------------------------------------------*/
intn
DFSDsetcal(float64 cal, float64 cal_err, float64 ioff, float64 ioff_err,
           int32 cal_nt)
{
    CONSTR(FUNC, "DFSDsetcal");
  intn    ret_value = SUCCEED;

  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  Writesdg.cal = (float64) cal;
  Writesdg.cal_err = (float64) cal_err;
  Writesdg.ioff = (float64) ioff;
  Writesdg.ioff_err = (float64) ioff_err;
  Writesdg.cal_type = (int32) cal_nt;

  Ref.cal = 0;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDwriteref
 USAGE
       int DFSDwriteref(filename, ref)
       char   *filename;  IN: HDF file to write to
       uint16 ref;        IN: reference number for the next writing of data
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Specifies the data set to be written to next by using the reference
       number. Note that there is no gaurantee that reference numbers appear
       in sequence in and HDF file; therefore, it is not safe to assume
       that a reference number is the sequence number for a data set.
-----------------------------------------------------------------------------*/
intn
DFSDwriteref(const char *filename, uint16 ref)
{
  int32       file_id;
  int32       aid;
  intn        ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDwriteref");

  /* Clear error stack */
  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  /* Open file for read access */
  if (( file_id = DFSDIopen(filename, DFACC_READ))== DF_NOFILE)
    HGOTO_ERROR(DFE_BADOPEN, FAIL);

  /* Check for existence of SDG */
  if ((aid = Hstartread(file_id, DFTAG_SDG, ref)) == FAIL
      && (aid = Hstartread(file_id, DFTAG_NDG, ref)) == FAIL)
    {
      HCLOSE_GOTO_ERROR(file_id, DFE_NOMATCH, FAIL);
    }

  /*
   ** Probably need to call DFSDgetndg to intialize Writesdg struct
   ** This is so that we use the information of an SDG that has
   ** already been written out. Once a SDG has been written out,
   ** a user should not be able to change attributes such as
   ** numbertype, dimensions or fill value.
  */
  if ((DFSDIgetndg(file_id, DFTAG_SDG, ref, &Writesdg) < 0)
      && (DFSDIgetndg(file_id, DFTAG_NDG, ref, &Writesdg) < 0))
    {
      Hendaccess(aid);
      HCLOSE_GOTO_ERROR(file_id, DFE_INTERNAL, FAIL);
    }

  /* Close access to file, set Writeref */
  Hendaccess(aid);
  Writeref = ref;
  Lastref = ref;

  ret_value = Hclose(file_id);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDsetfillvalue
 USAGE
       int DFSDsetfillvalue(fill_value)
       VOID *fill_value;     IN: number to be stored in any unwritten locations
                                 of the scientific data set
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Set the value used to fill in any unwritten location in a scientific
       data set. Since the data is written in hyperslabs, it is possible
       that not alof the possible locations in a given data set are written
       to. The fill value is placed into all unwritten locations. The value
       gets written if the following "DFSDwriteslab" call is the first
       call to the data set i.e. the data set is created. It is assumed that
       the fille value has the same number type as the data set.
 WARNING
       Memory bug on SGI's if you try to free allocated space for fill values.
-----------------------------------------------------------------------------*/
intn
DFSDsetfillvalue(VOIDP fill_value)
{
  int32       numtype;        /* current number type  */
  uint32      localNTsize;    /* size of this NT on as it is on this machine  */
  intn        ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDsetfillvalue");

  /* Clear error stack  */
  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  /* Check to see if fill value written out already */
  if (Ref.fill_value == -1 && Writesdg.fill_fixed == TRUE)
    {
      HGOTO_ERROR(DFE_INTERNAL, FAIL);
    }
  else
    {
      /* Get local and file number type sizes  */
      numtype = Writesdg.numbertype;
      localNTsize = (uint32)DFKNTsize((numtype | DFNT_NATIVE) & (~DFNT_LITEND));

      /* Set fill value in Writesdg struct, and set fill value flag  */
      Ref.fill_value = 0;
      if (HDmemcpy(Writesdg.fill_value, fill_value, localNTsize) != NULL)
        ret_value = SUCCEED;
      else
        ret_value = FAIL;
    }

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDgetfillvalue
 USAGE
       int DFSDgetfillvalue(fill_value)
       VOID *fill_value;    OUT: Number to be stored in any unwritten
                                 locations of the scientific data set
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
      Gets the value used to fill any unwritten location in a scientific
      data set. The value is set by "DFSDsetfillvalue". Note that a call
      that initializes file information structures such as "DFSDgetdims"
      is required in order for "DFSDgetfillvalue" to succeed. This is because
      "DFSDgetfillvalue" does not take a file name as an arguement.
-----------------------------------------------------------------------------*/
intn
DFSDgetfillvalue(VOIDP fill_value)
{
  int32       numtype;        /* current number type  */
  uint32      localNTsize;    /* size of this NT on as it is on this machine  */
  intn        ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDgetfillvalue");

  /* Clear error stack  */
  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  /* Check if Readsdg is fresh  */
  if (Newdata < 0)
    HGOTO_ERROR(DFE_BADCALL, FAIL);

    /* Get local number type size  */
  numtype = Readsdg.numbertype;
  localNTsize = (uint32)DFKNTsize((numtype | DFNT_NATIVE) & (~DFNT_LITEND));

    /* Set return fill value  */
  if (HDmemcpy(fill_value, Readsdg.fill_value, localNTsize) != NULL)
    ret_value = SUCCEED;
  else
    ret_value = FAIL;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDreadslab
 USAGE
       int DFSDreadslab(filename, start, slab_size, stride, buffer, buffer_size)
       char *filename;       IN: name of HDF file to write to
       int32 start[];        IN: array of size rank containing the coordinates
                                 for the start of the slab of data
       int32 slab_size[];    IN: array of size rank containing the size of
                                 each dimension in the slab of data
       int32 stride[];       IN: sub-sampling stride(not implemented)
       VOIDP buffer;         OUT: buffer to hold the extracted slab of data
       int32 buffer_size[];  OUT: array containing the dimensions of the buffer
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Reads a sub-array of data from a scientific data set. "DFSDreadslab"
       will sequence to the next scientific data set if "DFSDgetdims" or
       "DFSDgetdata" is not called earlier. The "start[]" indices are relative
       to 1. The rank(size) of "start[]" must be the same as the number of
       dimensions of the specified variable. The elements of "slab_size[]"
       must be no larger than the dimensions of the scientific data set.
       The stride feature is not currently implemented. For now just pass
       the "start[]" array as the arguement for "stride[]" where it will be
       ignored. To extract a slab of lower dimension than that of the data
       set, enter 1 in the "slab_size[] array for each ommitted dimension.
       For example, to extract a 2D slab from a 3D data set, specify the
       beginning coordiantes in 3 dimensions in "start[]" and enter a 1 for
       the for the missing dimension in the array "slab_size[]". More
       specifically, to extract a 3x4 slab containing the elements (6,7,8)
       through (8.7,11) specify the begining coordiantes as {6,7,8} and
       the slab size as {3,1,4}.
-----------------------------------------------------------------------------*/
intn
DFSDreadslab(const char *filename, int32 start[], int32 slab_size[],
             int32 stride[], VOIDP buffer, int32 buffer_size[])
{
  intn  ret_value = SUCCEED;
  /* shut compiler up */
  stride = stride;

  ret_value = (DFSDgetslice(filename, start, slab_size, buffer, buffer_size));

  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDstartslab
 USAGE
       int DFSDstartslab(filename)
       char *filename;    IN: name of HDF file to write to
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Prepares the DFSD interface to write a slab(s) od data to a scientific
       data set. "DFSDsetdims" must be called before calling "DFSDstartslab".
       No call which needs to open a file may be made after a "DFSDstartslab"
       call until "DFSDendslab" is called. This routine will write out the
       fill values if "DFSDsetfillvalue" is called before this routine and
       this is the first write(i.e. creation) to the data set.
-----------------------------------------------------------------------------*/
intn
DFSDstartslab(const char *filename)
{
  int32       i;
  int32       sdg_size;
  int32       localNTsize;
  int32       fileNTsize;
  int32       fill_bufsize = 16384;   /* Chosen for the PC */
  int32       odd_size;
  uint8      *fill_buf;
  uint8       platnumsubclass;
  uint8       outNT;          /* file number type subclass */
  intn       ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDstartslab");

  /* Clear errors */
  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  /* Check rank set i.e. DFSDsetdims()  */
  if (!Writesdg.rank)
    HGOTO_ERROR(DFE_BADDIM, FAIL);

  /* If NT not set(i.e. DFSDsetNT() not called), default to float32  */
  if (Writesdg.numbertype == DFNT_NONE)
    DFSDsetNT(DFNT_FLOAT32);

  /* Open file */
  if (( Sfile_id = DFSDIopen(filename, DFACC_WRITE))== DF_NOFILE)
    HGOTO_ERROR(DFE_BADOPEN, FAIL);

  /*
  ** Check for Writeref set i.e. DFSDwriteref() called?
  ** If not Writeref then we create a new Writeref i.e new SDG
  ** Else use existing one.
  */
  /* Set tag, ref of SDG to write */
  Writesdg.data.tag = DFTAG_SD;
  if (!Writeref)
    Writeref = Hnewref(Sfile_id);
  if (!Writeref)
    HGOTO_ERROR(DFE_BADREF, FAIL);
  Writesdg.data.ref = Writeref;

  /* Intialize a few local variables */
  localNTsize = DFKNTsize((Writesdg.numbertype | DFNT_NATIVE) & (~DFNT_LITEND));
  fileNTsize = DFKNTsize(Writesdg.numbertype);

  /* Calculate size of of dataset */
  sdg_size = fileNTsize;
  for (i = 0; i < Writesdg.rank; i++)
    sdg_size *= Writesdg.dimsizes[i];

  /* set up to write data */
  Writesdg.aid = Hstartwrite(Sfile_id, DFTAG_SD, Writeref, sdg_size);
  if (Writesdg.aid == FAIL)
    HCLOSE_GOTO_ERROR(Sfile_id,DFE_BADAID, FAIL);

  /*
   ** Check if fill value is set
  */
  if (!Ref.fill_value)
    {
      /* make the fill buffer smaller if possible */
      if (fill_bufsize > sdg_size && localNTsize == fileNTsize)
        fill_bufsize = sdg_size;

          /* Allocate space for fill buffer */
      if ((fill_buf = (uint8 *) HDmalloc((uint32) fill_bufsize)) == NULL)
        {
          Hendaccess(Writesdg.aid);
          HCLOSE_GOTO_ERROR(Sfile_id,DFE_NOSPACE, FAIL);
        }

      /* Get number types and compare */
      outNT = Writesdg.filenumsubclass;
      platnumsubclass = (uint8)DFKgetPNSC(Writesdg.numbertype, (int32)DF_MT);

      if (platnumsubclass != outNT)
        {   /* conversion  */
          /* Allocate buffer  */
          uint8       buf2[DFSD_MAXFILL_LEN];

          /* Convert from native to IEEE  */
          DFKconvert((VOIDP)&Writesdg.fill_value, (VOIDP) buf2,
                     Writesdg.numbertype, 1, DFACC_WRITE, 0, 0);

          /* Intialize buffer to fill value */
          for (i = 0; i < fill_bufsize; i = i + localNTsize)
              HDmemcpy(&(fill_buf[i]), buf2, localNTsize);

        }
      else /* no conversion */
        {
            /* Intialize buffer to fill value */
            for (i = 0; i < fill_bufsize; i = i + localNTsize)
                HDmemcpy(&(fill_buf[i]), Writesdg.fill_value, localNTsize);
        }


      /* Write fill values */
      if (sdg_size <= fill_bufsize)
        odd_size = sdg_size;
      else
        {
          odd_size = sdg_size % fill_bufsize;
          for (i = 0; i < (sdg_size / fill_bufsize); i++)
            {     /* Write out fill buffer X times */
              if (Hwrite(Writesdg.aid, fill_bufsize, fill_buf) == FAIL)
                {
                  Hendaccess(Writesdg.aid);
                  HDfree((VOIDP) fill_buf);
                  HCLOSE_GOTO_ERROR(Sfile_id,DFE_WRITEERROR, FAIL);
                }
            }
        }
      /* Write fill values for odd size piece */
      if (Hwrite(Writesdg.aid, odd_size, fill_buf) == FAIL)
        {
          Hendaccess(Writesdg.aid);
          HDfree((VOIDP) fill_buf);
          HCLOSE_GOTO_ERROR(Sfile_id,DFE_WRITEERROR, FAIL);
        }

      Writesdg.fill_fixed = TRUE;   /* fill value cannot be changed */
      /* Free up space */
      HDfree((VOIDP) fill_buf);
    }

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*----------------------------------------------------------------------------
 NAME
       DFSDwriteslab
 USAGE
       int  DFSDwriteslab(start, stride, count, data)
       int32 start[];     IN: array containing the coordinates of the start
                              of the slab in the HDF file
       int32 stride[];    IN: array containing the dimensions of data[]
       int32 count[];     IN: array containing the size of the slab
       VOID  *data;       IN: array to hold the floating point data to write
 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Writes a slab of data to a scientific data set. The "start" indices
       are relative to 1. The rank of "start" must be the same as the
       number of dimensions of the specified variable. The elements of "start"
       must be no larger than the scientific data set's dimensions in order.
       The stride feature is not currently implemented. For now just pass the
       "start" array as the argument for "stride" where it will be ignored.
       The rank of "count" must be the same as the number of dimensions of the
       specified variable. The elements of "count" must be no larger that
       the scientific data sets's dimensions in order. The order in which
       the data will be written into the specified hyperslab is with the
       last dimension varying the fastest. The array "data" should be of
       appropriate number type for the data set. Note that neither the
       compilier nor the HDF sotfware can detect if the wrong type of data
       is written.
----------------------------------------------------------------------------*/
intn
DFSDwriteslab(int32 start[], int32 stride[],
              int32 count[], VOIDP data)
{
  intn        rank;           /* number of dimensions in data[] */
  int32       i;              /* temporary loop index */

  int32       leastsig;       /* fastest varying subscript in the array */
  int32       convert;        /* true if machine NT != NT to be read */
  int32       done;           /* true if we are at the end of the slab */
  int32       numtype;        /* current number type  */
  int32       fileNTsize;     /* size of this NT in the file  */
  int32       localNTsize;    /* size of this NT as it occurs in this machine */
  int32       numelements;    /* number of floats to read at once */
  int32       sdgsize;        /* number of bytes to be written in the SDG */
  int32       rowsize;        /* number of bytes to be written at once */
                                /*   in the hyperslab */
  int32       fileoffset;     /* offset into the current dataset in the file */
  int32      *doffset;        /* array for accessing the next element in data[] */
  int32      *foffset;        /* array for accessing  next element in the file */
  int32      *dimsleft;       /* array for tracking current position in data[] */
  int32      *startdims;      /* tmp array containing starting slab dims */
  int32      *sizedims;       /* tmp array containing the slab size */
  int32      *filedims;       /* tmp array containing the dimensions */
                                /*   of the dataset in the file */
  int32       r_error;        /* flag if an error occurred, */
                                /*   used by DFconvert macro */
  int32       aid;

  uint8       platnumsubclass;    /* class of this NT for this platform */
  uint8       fileNT;         /* file number subclass  */
  uint8      *buf;            /* buffer containing the converted current row */
  uint8      *datap;          /* ptr into data[] at starting offset */
                                /*   of current block */
  intn       ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDwriteslab");

  /* shut compiler up */
  stride = stride;

  /* Clear error stack  */
  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  /* Sanity checking of input data  */
  if (!data)
    HGOTO_ERROR(DFE_BADPTR, FAIL);

    /* Set rank */
  rank = Writesdg.rank;

  /* Do sanity checking of starting and size dimension arrays  */
  for (i = 0; i < (int32) rank; i++)
    {
      /*
      ** Check validity for the dimension ranges by
      **  checking lower bound of slab sizes
      **  checking lower bound of starting dimensions
      **  checking upper bound on writing dimensions
      */
      if ((count[i] < 1) || (start[i] < 1)
          || (start[i] + count[i] - 1 > Writesdg.dimsizes[i]))
          HCLOSE_GOTO_ERROR(Sfile_id,DFE_BADDIM, FAIL);
    }

  /* Intialize a few local variables */
  numtype = Writesdg.numbertype;
  platnumsubclass = (uint8)DFKgetPNSC(numtype & (~DFNT_LITEND), DF_MT);
  localNTsize = DFKNTsize((numtype | DFNT_NATIVE) & (~DFNT_LITEND));
  fileNTsize = DFKNTsize(numtype);
  fileNT = Writesdg.filenumsubclass;

  /* Calculate total bytes in SDS that can be written */
  sdgsize = fileNTsize;
  for (i = 0; i < Writesdg.rank; i++)
    sdgsize *= Writesdg.dimsizes[i];

  /* Set Access Id */
  aid = Writesdg.aid;

  /*
   ** Get dimensions of hyperslab to write out
   ** Allocate temporary buffers(3) to hold starting, size,
   **  and file(SDG in file) dimensions
   */
  startdims = (int32 *) HDmalloc((size_t) (3 * rank) * sizeof(int32));
  if (startdims == NULL)
      HCLOSE_GOTO_ERROR(Sfile_id,DFE_NOSPACE, FAIL);
  sizedims = startdims + rank;
  filedims = sizedims + rank;

  /* Copy arrays to private workspace (row major order) */
  for (i = 0; i < (int32) rank; i++)
    {
      startdims[i] = start[i] - 1;
      sizedims[i] = count[i];   /* dimensions of just slab */
      filedims[i] = Writesdg.dimsizes[i];   /* dimensions of whole SDG */
    }

  /* Is conversion necessary */
  convert = (fileNT != platnumsubclass);

  /* Collapse dimensions if contiguous both in the file and in memory */
  for (i = (int32) rank - 1; i > 0; i--)  /* stop before most sig dim */
    {
      /* read only part of dataset */
      if (startdims[i] != 0 || sizedims[i] < filedims[i])
        break;
      startdims[i - 1] *= filedims[i];
      sizedims[i - 1] *= sizedims[i];
      filedims[i - 1] *= filedims[i];
      rank--;
    }

  /*
   ** Which is least sig dim i.e fastest varying.
   ** In C usually the last.
  */
  leastsig = (int32) rank - 1;

  r_error = 0;
  if (rank == 1 && !convert)
    {
      /* all data is contiguous with no conversions */
      rowsize = sizedims[0] * fileNTsize;
      if ((Hseek(aid, startdims[0] * fileNTsize, 0) == FAIL)
          || (rowsize != Hwrite(aid, rowsize, (uint8 *) data)))
        {
          r_error = 1;
        }
    }
  else
    {
      /*
       * The data must be further manipulated.
       * It may need conversion, may not be contiguous, or
       * any combination of these.
       */
      numelements = sizedims[leastsig];     /* # of elmenents in a row */
      rowsize = numelements * fileNTsize;   /* # of bytes in a row */

      /* If conversion, allocate 1 row buffers */
      if (convert)
        {
          if ((buf = (uint8 *) HDmalloc((uint32) rowsize)) == NULL)
            {
              HDfree((VOIDP) startdims);
              Hendaccess(aid);
              HCLOSE_GOTO_ERROR(Sfile_id,DFE_NOSPACE, FAIL);
            }
        }
      else
        buf = NULL;

      /* Allocate space for file and data offsets and dimsleft */
      foffset = (int32 *) HDmalloc((size_t) (3 * rank) * sizeof(int32));
      if (foffset == NULL)
        {
          HDfree((VOIDP) startdims);
          HDfree((VOIDP) buf);
          Hendaccess(aid);
          HCLOSE_GOTO_ERROR(Sfile_id,DFE_NOSPACE, FAIL);
        }
      dimsleft = foffset + rank;
      doffset = dimsleft + rank;

      /* Set number of dimensions left */
      for (i = leastsig; i >= 0; i--)
        dimsleft[i] = sizedims[i];

      /* compute offsets in the source array */
      doffset[leastsig] = 1 * localNTsize;
      for (i = leastsig; i > 0; i--)
        doffset[i - 1] = doffset[i] * sizedims[i];

      /*
       ** Compute offsets in the file for dimension, according to the
       ** possible length for each dimension. Depends on numbertype.
       */
      for (i = leastsig, foffset[i] = 1 * fileNTsize; i > 0; i--)
        foffset[i - 1] = foffset[i] * filedims[i];

      /*
       ** Compute starting position in file
       ** All file writes are done relative to this starting offset.
       ** Cumulative offset is from most sig to next to least sig dim.
       */
      for (i = 0, fileoffset = 0; i < leastsig; i++)
        fileoffset = fileoffset + (startdims[i] * foffset[i]);

      /* Dont forget about last dimension */
      fileoffset = fileoffset + startdims[leastsig] * fileNTsize;

      datap = (uint8 *) data;
      done = 0;

      /* -- now write the data */
      do
        {
          /* move to the next data element in the file */
          if (Hseek(aid, fileoffset, 0) == FAIL)
            {
              r_error = 1;
              break;
            }

          /*  If convert and write one contiguous block of data */
          /*  Else write one contiguous block of data */
          if (convert)
            {
              DFKconvert((VOIDP) datap, (VOIDP) buf, numtype,
                         numelements, DFACC_WRITE, 0, 0);
              if (rowsize != Hwrite(aid, rowsize, buf))
                {
                  r_error = 1;
                  break;
                }
            }
          else
            {
              if (rowsize != Hwrite(aid, rowsize, datap))
                {
                  r_error = 1;
                  break;
                }
            }

          /*
           * Find starting place of the next row/block.
           * Note that all moves are relative:
           *   this preserves the starting offsets of each dimension
           */
          for (i = leastsig - 1; i >= 0; i--)
            {
              if (--dimsleft[i] > 0)
                {
                  /* Move to next element in the current dimension */
                  datap += doffset[i];
                  fileoffset += foffset[i];
                  break;
                }
              else
                {
                  dimsleft[i] = sizedims[i];
                  /*
                   * Note that we are still positioned at the beginning of
                   * the last element in the current dimension
                   */
                  /* move back to the beginning of dimension i */
                  datap -= doffset[i] * (sizedims[i] - 1);

                  /* move back to beginning read position of dimension i */
                  fileoffset -= foffset[i] * (sizedims[i] - 1);
                  if (i == 0)
                    done = 1;
                }
            }
        }
      while (!done && leastsig > 0);

      if (buf != NULL)
         HDfree((VOIDP) buf);
      if (foffset != NULL)
         HDfree((VOIDP) foffset);
    }

  /* Clean up time.... */
  HDfree((VOIDP) startdims);

  if (r_error)
    ret_value = FAIL;
  else
    ret_value = SUCCEED;

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*-----------------------------------------------------------------------------
 NAME
       DFSDendslab
 USAGE
       int DFSDendslab()

 RETURNS
       Returns SUCCEED(0) if successful and FAIL(-1) otherwise.
 DESCRIPTION
       Ends a sequence of slab calls started by "DFSDstartslab" by closing
       the file. Writes the NDG information to the file if this call follows
       a "DFSDstartslab" that created a new SDG.

----------------------------------------------------------------------------*/
intn
DFSDendslab(void)
{
  intn        ret_value = SUCCEED;
  CONSTR(FUNC, "DFSDendslab");

  /* Clear error stack */
  HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(DFSDIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

  /* Valid file id */
  if (Sfile_id == DF_NOFILE)
    HGOTO_ERROR(DFE_BADCALL, FAIL);

  /* Check to see if we have written out the SDG info */
  if (!Ref.new_ndg)
    {
      if (DFSDIputndg(Sfile_id, Writeref, &Writesdg) < 0)
        HCLOSE_GOTO_ERROR(Sfile_id,DFE_INTERNAL, FAIL);

      /* old nsdg table should be reset next time  */
      if (nsdghdr != NULL)
        {
          if (nsdghdr->nsdg_t != NULL)
            {
              DFnsdgle   *rear, *front;

              rear = nsdghdr->nsdg_t;
              front = rear->next;
              while (rear != NULL)
                {
                  HDfreenclear(rear);
                  rear = front;
                  if (rear != NULL)
                    front = rear->next;
                }
              nsdghdr->size = 0;
              nsdghdr->nsdg_t = NULL;
              lastnsdg.tag = DFTAG_NULL;
              lastnsdg.ref = 0;
            }

          HDfreenclear(nsdghdr);
        }

      Ref.new_ndg = -1;
    }

  /* Slab clean up */
  Hendaccess(Writesdg.aid);
  ret_value = Hclose(Sfile_id);
  Sfile_id = 0;
  Lastref = (uint16) Writeref;    /* remember ref written */
  Writeref = 0;   /* Reset Write ref */

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}

/*--------------------------------------------------------------------------
 NAME
    DFSDIstart
 PURPOSE
    DFSD-level initialization routine
 USAGE
    intn DFSDIstart()
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    Register the shut-down routine (DFSDPshutdown) for call with atexit
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE intn DFSDIstart(void)
{
    CONSTR(FUNC, "DFSDIstart");    /* for HERROR */
    intn        ret_value = SUCCEED;

    /* Don't call this routine again... */
    library_terminate = TRUE;

    /* Install atexit() library cleanup routine */
    if (HPregister_term_func(&DFSDPshutdown) != 0)
      HGOTO_ERROR(DFE_CANTINIT, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

    return(ret_value);
} /* end DFSDIstart() */

/*--------------------------------------------------------------------------
 NAME
    DFSDPshutdown
 PURPOSE
    Terminate various static buffers.
 USAGE
    intn DFSDshutdown()
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    Free various buffers allocated in the DFSD routines.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    Should only ever be called by the "atexit" function HDFend
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn DFSDPshutdown(void)
{
    DFSDIclear(&Readsdg);
    DFSDIclear(&Writesdg);

    /* old nsdg table should be reset next time  */
    if (nsdghdr != NULL)
      {
          if (nsdghdr->nsdg_t != NULL)
            {
                DFnsdgle   *rear, *front;

                rear = nsdghdr->nsdg_t;
                front = rear->next;
                while (rear != NULL)
                  {
                      HDfree(rear);
                      rear = front;
                      if (rear != NULL)
                          front = rear->next;
                  }
                lastnsdg.tag = DFTAG_NULL;
                lastnsdg.ref = 0;
            }
          HDfreenclear(nsdghdr);
      }

    if(ptbuf!=NULL)
      {
          HDfree(ptbuf);
          ptbuf=NULL;
      } /* end if */

    if(Lastfile!=NULL)
      {
          HDfree(Lastfile);
          Lastfile=NULL;
      } /* end if */
    return(SUCCEED);
} /* end DFSDPshutdown() */

