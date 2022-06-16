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

/* $Id: dfsdf.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*-----------------------------------------------------------------------------
 * File:    dfsdf.c
 * Purpose: C stubs for Fortran SDS routines
 * Invokes: dfsd.c dfkit.c
 * Contents:
 *  dsgdast:       Call DFSDgetdatastrs to get attributes of data
 *  dsigdis:       Call DFSDgetdimstrs to get attributes of a dimension
 *  dsgdisc:       Call DFSDgetdimscale to get scale for a dimension
 *  dsgrang:       Call DFSDgetmaxmin to get max and min data values
 *  dssdims:       Call DFSDsetdims to set dimensions for subsequent SDGs
 *  dssdisc:       Call DFSDsetdimscale to set scale for subsequent SDGs
 *  dssrang:       Call DFSDsetmaxmin to set max/min values for subsequent SDGs
 *  dsclear:       Call DFSDclear to erase values set for subsequent SDGs
 *  dsslens:       Call DFSDsetlengths to set maximum lengths of string
 *  dsgdiln:       Call DFSDgetdimlen to get lengths of strings for a dimension
 *  dsgdaln:       Call DFSDgetdatalen to get lengths of data strings
 *  dsfirst:       Call DFSDrestart to get SDGs again from beginning of file
 *  dspslc:        Call DFSDIputslice to write slice to file
 *  dseslc:        Call DFSDendslice to end slice writes, write SDG to file
 *  dssnt:         Call DFSDsetNT to set number type
 *  dsgnt:         Call DFSDgetNT to get number type for reading
 *  dsigdim:       Call DFSDgetdims to get dimensions of next SDG
 *  dsigdat:       Call DFSDgetdata to get data values
 *  dsipdat:       Call DFSDIputdata to write SDG to new file
 *  dsiadat:       Call DFSDIputdata to append SDG to existing file
 *  dsigslc:       Call DFSDIgetslice to get slice from file
 *  dsisslc:       Call DFSDstartslice to set up to write slice
 *  dslref:        Call DFSDlastref to get ref of last SDS accessed
 *  dsinum:        Call DFSDndatasets to get number of SDG in the file
 *  dsip32s:       Call DFSDpre32sdg to test if the sdg was written by HDF prior to
 *                      version 3.2
 *  dfsdgetdatastrs_:Call DFSDgetdatastrs to get attributes of data
 *  dfsdgetdimscale_:Call DFSDgetdimscale to get scale for a dimension
 *  dfsdgetrange_:  Call DFSDgetmaxmin to get max and min data values
 *  dfsdsetdims_:   Call DFSDsetdims to set dimensions for subsequent SDGs
 *  dfsdsetdimscale_:Call DFSDsetdimscale to set scale for subsequent SDGs
 *  dfsdsetrange_:  Call DFSDsetmaxmin to set max/min values for subsequent SDGs
 *  dfsdclear_:     Call DFSDclear to erase values set for subsequent SDGs
 *  dfsdsetlengths_:Call DFSDsetlengths to set maximum lengths of string
 *  dfsdgetdimlen_: Call DFSDgetdimlen to get lengths of strings for a dimension
 *  dfsdgetdatalen_:Call DFSDgetdatalen to get lengths of data strings
 *  dfsdrestart_:   Call DFSDrestart to get SDGs again from beginning of file
 *  dfsdputslice_:  Call DFSDIputslice to write slice to file
 *  dfsdendslice_:  Call DFSDendslice to end slice writes, write SDG to file
 *  dfsdsetnt_:     Call DFSDsetNT to set number type
 *  dfsdgetnt_:     Call DFSDgetNT to get number type
 *  dfsdlastref_:   Call DFSDlastref to get ref of last SDS accessed
 *  dsiwref:        Call DFSDwriteref to set up next ref to write
 *  dssfill:        Call DFSDsetfillvalue to set fill value for SDS
 *  dsgfill:        Call DFSDgetfillvalue to get fill value from SDS
 *  dsisslab:       Call DFSDstartslab to set up write to SDS
 *  dswslab:        Call DFSDwriteslab to write slab to file
 *  dseslab:        Call DFSDendslab to end slab writes, write NDG to file
 *  dsirslab:       Call DFSDreadslab to get slab from SDS
 * Remarks: no C stubs needed for the put string routines, only Fortran stubs
 *---------------------------------------------------------------------------*/

#include "hdf.h"
#include "dfsd.h"
#include "hproto_fortran.h"


/*-----------------------------------------------------------------------------
 * Name:    dsgdisc
 * Purpose: Call DFSDgetdimscale to get scale for a dimension
 * Inputs:  dim: dimension to get attributes for
 *          maxsize: size of scale array
 *          scale: array to return scale in
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDgetdimscale
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsgdisc(intf * dim, intf * maxsize, VOIDP scale)
{
    intn        rank, cdim;
    intn        isndg;

    DFSDIisndg(&isndg);
    if (isndg)
      {
          DFSDIgetrrank(&rank);
          if (rank < *dim)
              return FAIL;
          cdim = rank - (intn) *dim + 1;
      }
    else
        cdim = (intn) *dim;

    return (DFSDgetdimscale(cdim, *maxsize, scale));
}

/*-----------------------------------------------------------------------------
 * Name:    dsgrang
 * Purpose: Call DFSDgetrange to get maximum and minimum data values
 * Inputs:  pmax: float to return maximum in
 *          pmin: float to return minimum in
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDgetrange
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsgrang(VOIDP pmax, VOIDP pmin)
{
    return (DFSDgetrange(pmax, pmin));
}

/*-----------------------------------------------------------------------------
 * Name:    dssdims
 * Purpose: Call DFSDsetdims to set dimensions for subsequent SDGs
 * Inputs:  rank: no of dimensions of SDG
 *          dimsizes: array containing dimensions of SDG
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDsetdims
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndssdims(intf * rank, intf dimsizes[])
{
    int32       i, *cdims, *p;
    intf        ret;

    p = (int32 *) HDmalloc((size_t) (*rank) * sizeof(int32));
    if (p == NULL)
        return FAIL;
    cdims = p;
    for (i = 1; i <= *rank; i++)
      {
          *p = dimsizes[*rank - i];
          p++;
      }

    ret = DFSDsetdims((intn) *rank, cdims);
    HDfree((VOIDP) cdims);
    return (ret);
}

/*-----------------------------------------------------------------------------
 * Name:    dssdisc
 * Purpose: Call DFSDsetdimscale to set scales for subsequent SDGs
 * Inputs:  dim: dimension to set scale for
 *          dimsize: size of array scale
 *          scale: array of scale values
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDsetdimscale
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndssdisc(intf * dim, intf * dimsize, VOIDP scale)
{
    int         cdim;
    intn        rank;

    DFSDIgetwrank(&rank);
    if (rank < *dim)
        return FAIL;
    cdim = rank - (intn) *dim + 1;

    return (DFSDsetdimscale(cdim, *dimsize, scale));
}

/*-----------------------------------------------------------------------------
 * Name:    dssrang
 * Purpose: Call DFSDsetrange to set max and min values for this SDG
 * Inputs:  max, min: max and min data values
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDsetrange
 * Remarks: Max and Min are set only for next SDG, reset to NULL after
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndssrang(VOIDP max, VOIDP min)
{
    return (DFSDsetrange(max, min));
}

/*-----------------------------------------------------------------------------
 * Name:    dsclear
 * Purpose: Call DFSDclear to erase values set for subsequent SDGs
 * Inputs:  none
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDclear
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsclear(void)
{
    return (DFSDclear());
}

/*-----------------------------------------------------------------------------
 * Name:    dsslens
 * Purpose: Call DFSDsetlengths to set max lengths of strings
 * Inputs:  maxlen_label, maxlen_unit, maxlen_format, maxlen_coordsys: max lens
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDsetlengths
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsslens(intf * maxlen_label, intf * maxlen_unit, intf * maxlen_format,
         intf * maxlen_coordsys)
{
    return (DFSDsetlengths((intn) *maxlen_label, (intn) *maxlen_unit,
                           (intn) *maxlen_format, (intn) *maxlen_coordsys));
}

/*-----------------------------------------------------------------------------
 * Name:    dsgdiln
 * Purpose: Call DFSDgetdimlen to get actual lengths of strings
 * Inputs:  dim: dimension to get lengths for
 *          llabel, lunit, lformat: integers to return lengths of each string in
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDgetdimlen
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsgdiln(intf * dim, intf * llabel, intf * lunit, intf * lformat)
{
    intn        rank, cdim;
    intf        ret;
    intn        isndg;
    intn        cllabel, clunit, clformat;  /* convert between intf and intn */

    ret = DFSDIisndg(&isndg);
    if (isndg)
      {
          ret = DFSDIgetrrank(&rank);
          if (rank < *dim)
              return FAIL;
          cdim = rank - (intn) *dim + 1;
      }
    else
        cdim = (intn) *dim;

    ret = (intf) DFSDgetdimlen(cdim, &cllabel, &clunit, &clformat);
    if (ret != FAIL)
      {     /* if ok, copy the values over */
          *llabel = cllabel;
          *lunit = clunit;
          *lformat = clformat;
      }     /* end if */
    return (ret);
}

/*-----------------------------------------------------------------------------
 * Name:    dsgdaln
 * Purpose: Call DFSDgetdatalen to get actual lengths of strings
 * Inputs:  llabel, lunit, lformat, lcoordsys: integers to return lengths in
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDgetdatalen
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsgdaln(intf * llabel, intf * lunit, intf * lformat, intf * lcoordsys)
{
    intf        ret;
    intn        cllabel, clunit, clformat, clcoordsys;

    ret = (intf) DFSDgetdatalen(&cllabel, &clunit, &clformat, &clcoordsys);
    if (ret != FAIL)
      {
          *llabel = cllabel;
          *lunit = clunit;
          *lformat = clformat;
          *lcoordsys = clcoordsys;
      }     /* end if */
    return (ret);
}

/*-----------------------------------------------------------------------------
 * Name:    dsfirst * Purpose: Call DFSDrestart to get SDGs again from the beginning
 * Inputs:  none
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDrestart
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsfirst(void)
{

    return (DFSDrestart());
}

/*-----------------------------------------------------------------------------
 * Name:    dspslc
 * Purpose: Call DFSDIputslice to write slice to file
 * Inputs:  windims: array of size rank, containing size of slice
 *          data: array containing slice
 *          dims: dimensions of array data
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDIputslice
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndspslc(intf windims[], VOIDP data, intf dims[])
{
    int32      *cdims, *cwindims, *p, *wp;
    intn        i, rank;
    intf        ret;

    ret = DFSDIgetwrank(&rank);
    wp = (int32 *) HDmalloc((size_t) rank * sizeof(int32));
    if (wp == NULL)
        return FAIL;
    cwindims = wp;
    p = (int32 *) HDmalloc((size_t) rank * sizeof(int32));
    if (p == NULL){
	HDfree((VOIDP) cwindims);
        return FAIL;
    }
    cdims = p;
    for (i = 1; i <= rank; i++)
      {     /* reverse dims & windims */
          *p = dims[rank - i];
          p++;
          *wp = windims[rank - i];
          wp++;
      }

    ret = DFSDIputslice(cwindims, data, cdims, 1);
    HDfree((VOIDP) cdims);
    HDfree((VOIDP) cwindims);
    return (ret);
}

/*-----------------------------------------------------------------------------
 * Name:    dseslc
 * Purpose: Call DFSDIendslice to finish slice writes and write out SDG
 * Inputs:  none
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDIendslice
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndseslc(void)
{

    return (DFSDIendslice(1));
}

/*-----------------------------------------------------------------------------
 * Name:    dssnt
 * Purpose: Call DFSDsetNT to set number type for subsequent calls to
            DFSDputdata, DFSDadddata, DFSDsetdimscales.
 * Inputs:  numbertype
 * Returns: 0 on success, FAIL on failure with error set
 * Users:   HDF Fortran programmers
 * Method:  Invokes DFSDsetNT
 * Remarks: 0 specifies default value
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndssnt(intf * numbertype)
{
    return (DFSDsetNT(*numbertype));
}

/*----------------------------------------------------------------------------
 * Name:    dsgnt
 * Purpose: Call DFSDgetNT to get number type for subsequent calls
 * Inputs:  pnumbertype
 * Returns: 0 on success, FAIL on failure with error set
 * Users:   HDF Fortran programmers
 * Method:  Invokes DFSDgetNT
 * Remarks: 0 specifies default value
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsgnt(intf * pnumbertype)
{
    return (DFSDgetNT((int32 *) pnumbertype));
}

/*-----------------------------------------------------------------------------
 * Name:    dsigdim
 * Purpose: Call DFSDgetdims to get dimensions of next SDG
 * Inputs:  filename: name of HDF file
 *          prank: integer to return rank in
 *          sizes: array to return dimensions in
 *          maxrank: dimension of array sizes
 *          lenfn: length of filename
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   Fortran stub routine
 * Invokes: DFSDgetdims
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsigdim(_fcd filename, intf * prank, intf sizes[], intf * maxrank,
         intf * lenfn)
{
    char       *fn;
    int32       i, tmp;
    intn        isndg;
    intf        ret;

    fn = HDf2cstring(filename, (intn) *lenfn);
    if (!fn)
	return(-1);
    ret = DFSDgetdims(fn, (intn *) prank, (int32 *) sizes, (intn) *maxrank);
    DFSDIisndg(&isndg);
    if (isndg)
      {
          for (i = 0; i < ((int32) *prank) / 2; i++)
            {
                tmp = sizes[i];
                sizes[i] = sizes[(int32) *prank - i - 1];
                sizes[(int32) *prank - i - 1] = tmp;
            }
      }
    HDfree(fn);
    return ret;
}

/*-----------------------------------------------------------------------------
 * Name:    dsigdat
 * Purpose: Call DFSDgetdata to get data values
 * Inputs:  filename: name of HDF file
 *          rank: no of dimensions in array data
 *          maxsizes: array containing dimensions of the array data
 *          data: array to return the data in
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDIgetdata,DFSDIrefresh,DFSDIisndg
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsigdat(_fcd filename, intf * rank, intf maxsizes[], VOIDP data, intf * fnlen)
{
    int32       i;
    intn        isndg;
    intf        ret;
    char       *fn;
    int32      *p, *cmaxsizes;

    fn = HDf2cstring(filename, (intn) *fnlen);
    if (!fn)
	return(-1);
    /* if DFSDgetdims has not be called call DFSDIsdginfo to */
    /* refresh Readsdg       */
    if (DFSDIrefresh(fn) < 0)
        return FAIL;
    ret = DFSDIisndg(&isndg);
    if (isndg)
      {
          p = (int32 *) HDmalloc((size_t) (*rank) * sizeof(int32));
          if (p == NULL)
              return FAIL;
          cmaxsizes = p;

          for (i = 1; i <= *rank; i++)
            {
                *p = maxsizes[*rank - i];
                p++;
            }
          ret = DFSDIgetdata(fn, (intn) *rank, cmaxsizes, data, 1);
          HDfree((VOIDP) cmaxsizes);
      }
    else
        ret = DFSDIgetdata(fn, (intn) *rank, (int32 *) maxsizes, data, 1);  /* 1==FORTRAN */
    HDfree(fn);
    return ret;
}

/*-----------------------------------------------------------------------------
 * Name:    dsipdat
 * Purpose: Call DFSDIputdata to write SDG to new file
 * Inputs:  filename: name of HDF file
 *          rank: no of dimensions of array data
 *          dimsizes: array containing size of each dimension of array data
 *          data: array containing data values
 *          fnlen: length of string filename
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDIputdata
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsipdat(_fcd filename, intf * rank, intf dimsizes[], VOIDP data, intf * fnlen)
{
    char       *fn;
    int32       i, *cdims, *p;
    intf        ret;

    /* reverse the dimsizes first  */
    p = (int32 *) HDmalloc((size_t) (*rank) * sizeof(int32));
    if (p == NULL)
        return FAIL;
    cdims = p;
    for (i = 1; i <= *rank; i++)
      {
          *p = dimsizes[*rank - i];
          p++;
      }
    fn = HDf2cstring(filename, (intn) *fnlen);
    if (!fn)
	return(-1);

    /* 0, 1 specify create mode, called from FORTRAN program */
    /* In HDF3.2 .hdf files, data and dimsizes are in C order  */
    ret = DFSDIputdata(fn, (intn) *rank, cdims, data, 0, 1);
    HDfree(fn);
    HDfree((VOIDP) cdims);

    return (ret);
}

/*-----------------------------------------------------------------------------
 * Name:    dsiadat
 * Purpose: Call DFSDIputdata to append SDG to existing file
 * Inputs:  filename: name of HDF file
 *          rank: no of dimensions of array data
 *          dimsizes: array containing size of each dimension of array data
 *          data: array containing data values
 *          fnlen: length of string filename
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDIputdata
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsiadat(_fcd filename, intf * rank, intf dimsizes[], VOIDP data, intf * fnlen)
{
    char       *fn;
    int32       i, *cdims, *p;
    intf        ret;

    /* reverse the dimsizes first  */
    p = (int32 *) HDmalloc((size_t) (*rank) * sizeof(int32));
    if (p == NULL)
        return FAIL;
    cdims = p;
    for (i = 1; i <= *rank; i++)
      {
          *p = dimsizes[*rank - i];
          p++;
      }
    fn = HDf2cstring(filename, (intn) *fnlen);
    if (!fn)
	return(-1);

    /* 1, 1 specify create mode, called from FORTRAN program */
    /* In HDF3.2 .hdf files, data and dimsizes are in C order  */
    ret = DFSDIputdata(fn, (intn) *rank, cdims, data, 1, 1);
    HDfree(fn);
    HDfree((VOIDP) cdims);
    return (ret);
}

/*-----------------------------------------------------------------------------
 * Name:    dsigslc
 * Purpose: Call DFSDIgetslice to read slice from file
 * Inputs:  filename: name of HDF file
 *          winst: array of size = rank of data, containing start of slice
 *          windims: array of size rank, containing end of slice
 *          data: array for returning slice
 *          ndims: no of dims of array data
 *          dims: dimensions of array data
 *          fnlen: length of filename
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDIgetslice
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsigslc(_fcd filename, intf winst[], intf windims[], VOIDP data, intf dims[],
         intf * fnlen)
{
    char       *fn;
    intf        ret;
    intn        rank, i;
    int32      *cdims, *cwindims, *cwinst, *p, *wp, *wsp;
    intn        isndg;

    fn = HDf2cstring(filename, (intn) *fnlen);
    if (!fn)
	return(-1);

    /* if DFSDgetdims has not be called call DFSDIsdginfo to */
    /* refresh Readsdg       */
    if (DFSDIrefresh(fn) < 0)
        return FAIL;

    ret = DFSDIisndg(&isndg);
    if (isndg)
      {
          ret = DFSDIgetrrank(&rank);
          p = (int32 *) HDmalloc((size_t) rank * sizeof(int32));
          if (p == NULL)
              return FAIL;
          cdims = p;
          wp = (int32 *) HDmalloc((size_t) rank * sizeof(int32));
          if (wp == NULL)
              return FAIL;
          cwindims = wp;
          wsp = (int32 *) HDmalloc((size_t) rank * sizeof(int32));
          if (wsp == NULL)
              return FAIL;
          cwinst = wsp;

          for (i = 1; i <= rank; i++)
            {
                *p = dims[rank - i];
                p++;
                *wp = windims[rank - i];
                wp++;
                *wsp = winst[rank - i];
                wsp++;
            }
          ret = DFSDIgetslice(fn, cwinst, cwindims, data, cdims, 1);
          HDfree((VOIDP) cdims);
          HDfree((VOIDP) cwindims);
          HDfree((VOIDP) cwinst);
      }
    else
        ret = DFSDIgetslice(fn, (int32 *) winst, (int32 *) windims,
                            data, (int32 *) dims, 1);
    HDfree(fn);
    return (ret);
}

/*-----------------------------------------------------------------------------
 * Name:    dsisslc
 * Purpose: Call DFSDstartslice to set up to write slice
 * Inputs:  filename: name of HDF file
 *          fnlen: length of filename
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDstartslice
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsisslc(_fcd filename, intf * fnlen)
{
    char       *fn;
    intf        ret;

    fn = HDf2cstring(filename, (intn) *fnlen);
    if (!fn)
	return(-1);
    ret = DFSDstartslice(fn);
    HDfree(fn);
    return (ret);
}

/*-----------------------------------------------------------------------------
 * Name:    dsirref
 * Purpose: Call DFSDreadref to set up next ref to read
 * Inputs:  filename: name of HDF file
 *      ref: next ref to read
 *          fnlen: length of filename
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDstartslice
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsirref(_fcd filename, intf * ref, intf * fnlen)
{
    char       *fn;
    intf        ret;

    fn = HDf2cstring(filename, (intn) *fnlen);
    if (!fn)
	return(-1);
    ret = DFSDreadref(fn, (uint16) *ref);
    HDfree(fn);
    return (ret);
}

/*-----------------------------------------------------------------------------
 * Name:    dslref
 * Purpose: Return last ref written or read
 * Inputs:  none
 * Globals: Lastref
 * Returns: ref on success, -1 on error with DFerror set
 * Users:   HDF users, utilities, other routines
 * Invokes: DFANlastref
 * Remarks: none
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndslref(void)
{
    return ((intf)DFSDlastref());
}

/*-----------------------------------------------------------------------------
 * Name:    dsinum
 * Purpose: Return number of SDGs in the file
 * Inputs:  filename: name of file
 *          len: length of Fortran string filename
 * Returns: number of SDGs on success, -1 on failure with DFerror set
 * Users:   dsnum, dfsdnumber
 * Invokes: DFSDndataset, HDf2cstring
 * Method:  convert string, call DFSDndatasets
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsinum(_fcd filename, intf * len)
{
    char       *cname;
    intf        status;

    cname = HDf2cstring(filename, (intn) *len);
    if (!cname)
	return(-1);
    status = DFSDndatasets(cname);
    HDfree(cname);

    return (status);
}

/*------------------------------------------------------------------------------
* Name:     dsip32s
* Purpose:  tests if the SDG with the specified ref was written by HDF prior to
*            version 3.2
* Input:    filename: name of HDF file
*           ref: the ref number of the SDG
*           ispre32: set to TRUE if the SDG/ref was written by old library;
*                        to FALSE otherwise.
*           len:     length of filename
* Retruns:  0 on success, -1 on failure
* Users:    HDF Fortran programmers
*------------------------------------------------------------------------------*/

FRETVAL(intf)
ndsip32s(_fcd filename, intf * ref, intf * ispre32, intf * len)
{
    char       *cname;
    intf        status;

    cname = HDf2cstring(filename, (intn) *len);
    if (!cname)
	return(-1);
    status = DFSDpre32sdg(cname, (uint16) *ref, (intn *) ispre32);

    HDfree(cname);
    return (status);
}

/*-----------------------------------------------------------------------------
 * Name:    dfsdgetdatastrs
 * Purpose: Call DFSDgetdatastrs to get the data attributes
 * Inputs:  label, unit, format, coordsys: strings to return attributes in
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDgetdatastrs
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfsdgetdatastrs(_fcd label, _fcd unit, _fcd format, _fcd coordsys)
{
    return (DFSDgetdatastrs((char *) _fcdtocp(label), (char *) _fcdtocp(unit),
                   (char *) _fcdtocp(format), (char *) _fcdtocp(coordsys)));
}

/*-----------------------------------------------------------------------------
 * Name:    dfsdgetdimstrs
 * Purpose: Call DFSDgetdimstrs to get attributes of a dimension
 * Inputs:  label, unit, format: strings to return attributes in
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDgetdimstrs
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfsdgetdimstrs(intf * dim, _fcd label, _fcd unit, _fcd format)
{
    intn        isndg;
    intn        rank, cdim;

    DFSDIisndg(&isndg);
    if (isndg)
      {
          DFSDIgetrrank(&rank);
          if (rank < *dim)
              return FAIL;
          cdim = rank - (intn) *dim + 1;
      }
    else
        cdim = (intn) *dim;

    return (DFSDgetdimstrs(cdim, (char *) _fcdtocp(label),
                       (char *) _fcdtocp(unit), (char *) _fcdtocp(format)));
}

/*-----------------------------------------------------------------------------
 * Name:    dfsdgetdimscale
 * Purpose: Call DFSDgetdimscale to get scale for a dimension
 * Inputs:  dim: dimension to get attributes for
 *          maxsize: size of scale array
 *          scale: array to return scale in
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDgetdimscale
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfsdgetdimscale(intf * dim, intf * maxsize, VOIDP scale)
{

    intn        isndg;
    intn        rank, cdim;

    DFSDIisndg(&isndg);
    if (isndg)
      {
          DFSDIgetrrank(&rank);
          if (rank < *dim)
              return FAIL;
          cdim = rank - (intn) *dim + 1;
      }
    else
        cdim = (intn) *dim;

    return (DFSDgetdimscale(cdim, *maxsize, scale));
}

/*-----------------------------------------------------------------------------
 * Name:    dfsdgetrange
 * Purpose: Call DFSDgetrange to get maximum and minimum data values
 * Inputs:  pmax: float to return maximum in
 *          pmin: float to return minimum in
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDgetrange
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfsdgetrange(VOIDP pmax, VOIDP pmin)
{
    return (DFSDgetrange(pmax, pmin));
}

/*-----------------------------------------------------------------------------
 * Name:    dfsdsetdims
 * Purpose: Call DFSDsetdims to set dimensions for subsequent SDGs
 * Inputs:  rank: no of dimensions of SDG
 *          dimsizes: array containing dimensions of SDG
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDsetdims
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfsdsetdims(intf * rank, intf dimsizes[])
{

    int32       i, *cdims, *p;
    intf        ret;

    p = (int32 *) HDmalloc((size_t) (*rank) * sizeof(int32));
    if (p == NULL)
        return FAIL;
    cdims = p;
    for (i = 1; i <= *rank; i++)
      {
          *p = dimsizes[*rank - i];
          p++;
      }

    ret = DFSDsetdims((intn) *rank, cdims);
    HDfree((VOIDP) cdims);
    return (ret);
}

/*-----------------------------------------------------------------------------
 * Name:    dfsdsetdimscale
 * Purpose: Call DFSDsetdimscale to set scales for subsequent SDGs
 * Inputs:  dim: dimension to set scale for
 *          dimsize: size of array scale
 *          scale: array of scale values
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDsetdimscale
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfsdsetdimscale(intf * dim, intf * dimsize, VOIDP scale)
{
    intn        rank, cdim;

    DFSDIgetwrank(&rank);
    if (rank < *dim)
        return FAIL;
    cdim = rank - (intn) *dim + 1;

    return (DFSDsetdimscale(cdim, *dimsize, scale));
}

/*-----------------------------------------------------------------------------
 * Name:    dfsdsetrange
 * Purpose: Call DFSDsetrange to set max and min values for this SDG
 * Inputs:  max, min: max and min data values
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDsetrange
 * Remarks: Max and Min are set only for next SDG, reset to NULL after
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfsdsetrange(VOIDP max, VOIDP min)
{
    return (DFSDsetrange(max, min));
}

/*-----------------------------------------------------------------------------
 * Name:    dfsdclear
 * Purpose: Call DFSDclear to erase values set for subsequent SDGs
 * Inputs:  none
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDclear
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfsdclear(void)
{
    return (DFSDclear());
}

/*-----------------------------------------------------------------------------
 * Name:    dfsdsetlengths
 * Purpose: Call DFSDsetlengths to set max lengths of strings
 * Inputs:  maxlen_label, maxlen_unit, maxlen_format, maxlen_coordsys: max lens
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDsetlengths
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfsdsetlengths(intf * maxlen_label, intf * maxlen_unit, intf * maxlen_format,
                intf * maxlen_coordsys)
{
    return (DFSDsetlengths((intn) *maxlen_label, (intn) *maxlen_unit,
                           (intn) *maxlen_format, (intn) *maxlen_coordsys));
}

/*-----------------------------------------------------------------------------
 * Name:    dfsdgetdimlen
 * Purpose: Call DFSDgetdimlen to get actual lengths of strings
 * Inputs:  dim: dimension to get lengths for
 *         llabel, lunit, lformat: integers to return lengths of each string in
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDgetdimlen
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfsdgetdimlen(intf * dim, intf * llabel, intf * lunit, intf * lformat)
{
    intn        isndg;
    intn        rank, cdim;
    intf        ret;
    intn        cllabel, clunit, clformat;  /* convert between intf and intn */

    ret = DFSDIisndg(&isndg);
    if (isndg)
      {
          ret = DFSDIgetrrank(&rank);
          if (rank < *dim)
              return FAIL;
          cdim = rank - (intn) *dim + 1;
      }
    else
        cdim = (intn) *dim;

    ret = (intf) DFSDgetdimlen(cdim, &cllabel, &clunit, &clformat);
    if (ret != FAIL)
      {     /* if ok, copy the values over */
          *llabel = cllabel;
          *lunit = clunit;
          *lformat = clformat;
      }     /* end if */
    return (ret);
}

/*-----------------------------------------------------------------------------
 * Name:    dfsdgetdatalen
 * Purpose: Call DFSDgetdatalen to get actual lengths of strings
 * Inputs:  llabel, lunit, lformat, lcoordsys: integers to return lengths in
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDgetdatalen
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfsdgetdatalen(intf * llabel, intf * lunit, intf * lformat, intf * lcoordsys)
{
    intf        ret;
    intn        cllabel, clunit, clformat, clcoordsys;

    ret = (intf) DFSDgetdatalen(&cllabel, &clunit, &clformat, &clcoordsys);
    if (ret != FAIL)
      {
          *llabel = cllabel;
          *lunit = clunit;
          *lformat = clformat;
          *lcoordsys = clcoordsys;
      }     /* end if */
    return (ret);
}

/*-----------------------------------------------------------------------------
 * Name:    dfsdrestart
 * Purpose: Call DFSDrestart to get SDGs again from the beginning
 * Inputs:  none
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDrestart
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfsdrestart(void)
{
    return (DFSDrestart());
}

/*-----------------------------------------------------------------------------
 * Name:    dfsdputslice
 * Purpose: Call DFSDIputslice to write slice to file
 * Inputs:  winst: array of size = rank of data, containing start of slice
 *          windims: array of size rank, containing end of slice
 *          data: array containing slice
 *          dims: dimensions of array data
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDIputslice
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfsdputslice(intf windims[], VOIDP data, intf dims[])
{
    intn        rank, i;
    intf        ret;
    int32      *cdims, *cwindims, *p, *wp;

    ret = DFSDIgetwrank(&rank);
    wp = (int32 *) HDmalloc((size_t) (rank) * sizeof(int32));
    if (wp == NULL)
        return FAIL;
    cwindims = wp;
    p = (int32 *) HDmalloc((size_t) (rank) * sizeof(int32));
    if (p == NULL)
        return FAIL;
    cdims = p;
    for (i = 1; i <= rank; i++)
      {     /* reverse dims & windims */
          *p = dims[rank - i];
          p++;
          *wp = windims[rank - i];
          wp++;
      }

    ret = DFSDIputslice(cwindims, data, cdims, 1);
    HDfree((VOIDP) cdims);
    HDfree((VOIDP) cwindims);
    return (ret);
}

/*-----------------------------------------------------------------------------
 * Name:    dfsdendslice
 * Purpose: Call DFSDendslice to finish slice writes and write out SDG
 * Inputs:  none
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDIendslice
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfsdendslice(void)
{
    return (DFSDIendslice(1));
}

/*-----------------------------------------------------------------------------
 * Name:    dfsdsetnt
 * Purpose: Call DFSDsetNT to set number type for subsequent calls to
 *          DFSDputdata, DFSDadddata, DFSDsetdimscales.
 * Inputs:  numbertype
 * Returns: 0 on success, FAIL on failure with error set
 * Users:   HDF Fortran programmers
 * Method:  Invokes DFSDsetNT
 * Remarks: 0 specifies default value
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfsdsetnt(intf * numbertype)
{
    return (DFSDsetNT(*numbertype));
}

/*-----------------------------------------------------------------------------
 * Name:    dfsdgetnt
 * Purpose: Call DFSDgetNT to get number type for subsequent calls
 * Inputs:  pnumbertype
 * Returns: 0 on success, FAIL on failure with error set
 * Users:   HDF Fortran programmers
 * Method:  Invokes DFSDgetNT
 * Remarks: 0 specifies default value
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfsdgetnt(intf * pnumbertype)
{
    return (DFSDgetNT((int32 *) pnumbertype));
}

/*-----------------------------------------------------------------------------
 * Name:    dfsdlastref
 * Purpose: Return last ref written or read
 * Inputs:  none
 * Globals: Lastref
 * Returns: ref on success, -1 on error with DFerror set
 * Users:   HDF users, utilities, other routines
 * Invokes: DFANlastref
 * Remarks: none
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfsdlastref(void)
{
    return ((intf)DFSDlastref());
}

/*-----------------------------------------------------------------------------
 * Name:    dsisdis
 * Purpose: For the given dimension, set label, unit, format
 *          This routine needs to be called once for each dimension whose
 *          values the user wants to set.
 * Inputs:  dim: the dimension that this info applies to
 *          label: label to be used to describe this dimension
 *          unit: units for dimension
 *          format: format to be used in displaying
 *          llabel, lunit, lformat: lengths of corresponding strings
 * Globals:
 * Returns: 0 on success, FAIL on failure with error set
 * Users:   HDF users, utilities, other routines
 * Invokes: DFSDIsetdimstr
 * Method:
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsisdis(intf * dim, _fcd flabel, _fcd funit, _fcd fformat,
         intf * llabel, intf * lunit, intf * lformat)
{
    char       *label = HDf2cstring(flabel, (intn) *llabel);
    char       *unit = HDf2cstring(funit, (intn) *lunit);
    char       *format = HDf2cstring(fformat, (intn) *lformat);
    intf        status;
    intn        rank, cdim;

    if (!(label && unit && format))
    {
	if (label) HDfree(label);
	if (unit) HDfree(unit);
	if (format) HDfree(format);
	return FAIL;
    }
    status = DFSDIgetwrank(&rank);

    if (rank < *dim)
        return FAIL;
    cdim = rank - (intn) *dim + 1;

    status = DFSDIsetdimstrs(cdim, label, unit, format);

    HDfree(label);
    HDfree(unit);
    HDfree(format);

    return status;

}

/*-----------------------------------------------------------------------------
 * Name:    dsigdis
 * Purpose: Call DFSDgetdimstrs to get attributes of a dimension
 * Inputs:  label, unit, format: strings to return attributes in
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDgetdimstrs
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsigdis(intf * dim, _fcd label, _fcd unit, _fcd format, intf * llabel,
         intf * lunit, intf * lformat)
{
    char       *ilabel, *iunit, *iformat;
    intn        rank, cdim;
    intn        isndg, status;

    DFSDIisndg(&isndg);
    if (isndg)
      {
          DFSDIgetrrank(&rank);
          if (rank < *dim)
              return FAIL;
          cdim = rank - (intn) *dim + 1;
      }
    else
        cdim = (intn) *dim;

    iunit = ilabel = iformat = NULL;

    if (*llabel)
        ilabel = (char *) HDmalloc((size_t) *llabel + 1);
    if (*lunit)
        iunit = (char *) HDmalloc((size_t) *lunit + 1);
    if (*lformat)
        iformat = (char *) HDmalloc((size_t) *lformat + 1);

    status = DFSDgetdimstrs(cdim, ilabel, iunit, iformat);

    HDpackFstring(ilabel, _fcdtocp(label), (intn) *llabel);
    HDpackFstring(iunit, _fcdtocp(unit), (intn) *lunit);
    HDpackFstring(iformat, _fcdtocp(format), (intn) *lformat);

    if (ilabel)
        HDfree(ilabel);
    if (iunit)
        HDfree(iunit);
    if (iformat)
        HDfree(iformat);

    return status;
}

/*-----------------------------------------------------------------------------
 * Name:    dsisdas
 * Purpose: Set label, unit and format for displaying subsequent SDGs
 * Inputs:  label: label to be used to describe data
 *          unit: unit corresponding to data values
 *          format: format to be used in displaying data values
 *          coordsys: type of coordinate system
 * Globals: Writesdg, Ref
 * Returns: 0 on success, FAIL on failure with error set
 * Users:   HDF Fortran stubs
 * Invokes: DFSDIsetdatastrs
 * Method:  Stores values in global structure Writesdg
 * Remarks:
 *---------------------------------------------------------------------------*/
FRETVAL(intf)
ndsisdas(_fcd flabel, _fcd funit, _fcd fformat, _fcd fcoordsys, intf * isfortran,
         intf * llabel, intf * lunit, intf * lformat, intf * lcoordsys)
{
    char       *label = HDf2cstring(flabel, (intn) *llabel);
    char       *unit = HDf2cstring(funit, (intn) *lunit);
    char       *format = HDf2cstring(fformat, (intn) *lformat);
    char       *coordsys = HDf2cstring(fcoordsys, (intn) *lcoordsys);
    intf        status;

    if (!(label && unit && format))
    {
	if (label) HDfree(label);
	if (unit) HDfree(unit);
	if (format) HDfree(format);
	return FAIL;
    }

    /* shut compiler up */
    isfortran = isfortran;

    status = DFSDIsetdatastrs(label, unit, format, coordsys);

    HDfree(label);
    HDfree(unit);
    HDfree(format);
    HDfree(coordsys);

    return status;
}   /* ndsisdas */

/*-----------------------------------------------------------------------------
 * Name:    dsigdas
 * Purpose: Call DFSDgetdatastrs to get the data attributes
 * Inputs:  label, unit, format, coordsys: strings to return attributes in
 * Returns: 0 on success, -1 on failure with    DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDgetdatastrs
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsigdas(_fcd label, _fcd unit, _fcd format, _fcd coordsys, intf * llabel,
         intf * lunit, intf * lformat, intf * lcoord)
{
    char       *ilabel, *iunit, *iformat, *icoord;
    intf        status;

    iunit = ilabel = iformat = icoord = NULL;

    if (*llabel)
        ilabel = (char *) HDmalloc((uint32) *llabel + 1);
    if (*lunit)
        iunit = (char *) HDmalloc((uint32) *lunit + 1);
    if (*lformat)
        iformat = (char *) HDmalloc((uint32) *lformat + 1);
    if (*lcoord)
        icoord = (char *) HDmalloc((uint32) *lcoord + 1);

    status = DFSDgetdatastrs(ilabel, iunit, iformat, icoord);

    HDpackFstring(ilabel, _fcdtocp(label), (intn) *llabel);
    HDpackFstring(iunit, _fcdtocp(unit), (intn) *lunit);
    HDpackFstring(iformat, _fcdtocp(format), (intn) *lformat);
    HDpackFstring(icoord, _fcdtocp(coordsys), (intn) *lcoord);

    if (ilabel)
        HDfree(ilabel);
    if (iunit)
        HDfree(iunit);
    if (iformat)
        HDfree(iformat);
    if (icoord)
        HDfree(icoord);

    return status;

}

/*-----------------------------------------------------------------------------
 * Name:    dsscal
 * Purpose: Call DFSDsetcal to set calibration data
 * Inputs:  cal, cal_err   : calibration and error
 *          ioff, ioff_err : offset and error
 *          cal_type       : after calibration NT
 * Returns: 0 on success, -1 on failure
 * Users:   HDF Fortran programmers
 * Invokes: DFSDgetdatastrs
 *---------------------------------------------------------------------------*/
FRETVAL(intf)
ndsscal(float64 *cal, float64 *cal_err, float64 *ioff, float64 *ioff_err,
        intf * cal_type)
{
    intf ret;
    float64 dcal=0.0;
    float64 dcal_err=0.0;
    float64 dioff=0.0;
    float64 dioff_err=0.0;
    
    HDmemcpy(&dcal, cal, sizeof(float64));
    HDmemcpy(&dcal_err, cal_err, sizeof(float64));
    HDmemcpy(&dioff, ioff, sizeof(float64));
    HDmemcpy(&dioff_err, ioff_err, sizeof(float64));

    ret=(intf)DFSDsetcal((float64)dcal, (float64)dcal_err, (float64)dioff, (float64)dioff_err, (int32)*cal_type);
    return ret;
}   /* ndsscal */

/*-----------------------------------------------------------------------------
 * Name:    dsgcal
 * Purpose: Call DFSDgetcal to get calibration data
 * Inputs:  cal, cal_err   : calibration and error
 *          ioff, ioff_err : offset and error
 *          cal_type       : after calibration NT
 * Returns: 0 on success, -1 on failure
 * Users:   HDF Fortran programmers
 * Invokes: DFSDgetcal
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsgcal(float64 *cal, float64 *cal_err, float64 *ioff, float64 *ioff_err,
        intf * cal_type)
{
    intf ret;
    float64 dcal ;
    float64 dcal_err ;
    float64 dioff ;
    float64 dioff_err ;
    
    ret =(intf) DFSDgetcal(&dcal, &dcal_err, &dioff, &dioff_err, (int32 *) cal_type);

    HDmemcpy(cal, &dcal, sizeof(float64));
    HDmemcpy(cal_err, &dcal_err, sizeof(float64));
    HDmemcpy(ioff, &dioff, sizeof(float64));
    HDmemcpy(ioff_err, &dioff_err, sizeof(float64));

    return ret;
}   /* ndsgcal */

/*-----------------------------------------------------------------------------
 * Name:    dsiwref
 * Purpose: Call DFSDwriteref to set up next ref to write
 * Inputs:  filename: name of HDF file
 *          fnlen: length of filename
 *          ref: next ref to read
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDwriteref
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsiwref(_fcd filename, intf * fnlen, intf * ref)
{
    char       *fn;
    intf        ret;

    fn = HDf2cstring(filename, (intn) *fnlen);
    if (!fn)
	return(-1);
    ret = DFSDwriteref(fn, (uint16) *ref);
    HDfree(fn);
    return (ret);
}

/*-----------------------------------------------------------------------------
 * Name:    dfsdsfill
 * Purpose: Call DFSDsetfillvalue to set fill value for SDS
 * Inputs:  fill_value: fill value for SDS
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDsetfillvalue
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndssfill(VOIDP fill_value)
{
    return DFSDsetfillvalue(fill_value);
}

/*-----------------------------------------------------------------------------
 * Name:    dsgfill
 * Purpose: Call DFSDgetfillvalue to get fill value for SDS
 * Inputs:  fill_value: fill value of SDS
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDgetfillvalue
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsgfill(VOIDP fill_value)
{
    return DFSDgetfillvalue(fill_value);
}

/*-----------------------------------------------------------------------------
 * Name:    dsisslab
 * Purpose: Set up slab writes to SDS
 * Inputs:  filename: file to which this applies
 *          fnlen: file name length
 * Returns: 0 on success, FAIL on failure
 * Users:   HDF programmers, other routines and utilities
 * Invokes: DFSDstartslab
 * Remarks:
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsisslab(_fcd filename, intf * fnlen)
{
    char       *fn;
    intf        ret;

    fn = HDf2cstring(filename, (intn) *fnlen);
    if (fn == NULL)
        return FAIL;
    ret = DFSDstartslab(fn);
    HDfree(fn);
    return ret;
}

/*-----------------------------------------------------------------------------
 * Name:    dswslab
 * Purpose: Call DFSDwriteslab to write slab to file
 * Inputs:  start: array of size = rank of data, containing start of slab
 *          stride: array for subsampling
 *          count: array of size rank, containing size of slab
 *          data: array of data to be written
 * Returns: 0 on success, -1 on failure with error set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDIgetwrank, HDmalloc, HDfree, HDf2cstring, DFSDwriteslab
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndswslab(intf start[], intf stride[],
         intf count[], VOIDP data)
{
    int32      *lstart, *lstride, *lcount, *aptr, *bptr, *cptr;
    intn        i, rank;
    intf        ret;

    /*
       ** Lets reverse the order for the arrays since we
       ** are going from fortran to C
     */
    ret = DFSDIgetwrank(&rank);
    if (ret == FAIL)
        return FAIL;

    aptr = (int32 *) HDmalloc((size_t) (3 * rank) * sizeof(int32));
    if (aptr == NULL)
        return FAIL;

    lstart = aptr;
    lstride = bptr = aptr + rank;
    lcount = cptr = bptr + rank;

    for (i = 1; i <= rank; i++)
      {     /* reverse start, stride & count */
          *aptr = start[rank - i];
          aptr++;
          *bptr = stride[rank - i];
          bptr++;
          *cptr = count[rank - i];
          cptr++;
      }

    ret = DFSDwriteslab(lstart, lstride, lcount, data);
    HDfree((VOIDP) lstart);

    return ret;
}

/*-----------------------------------------------------------------------------
 * Name:    dseslab
 * Purpose: End slab writes to SDS, Write out NDG
 * Inputs:  None
 * Returns: 0 on success, FAIL on failure
 * Users:   HDF programmers, other routines and utilities
 * Invokes: DFSDendslab
 * Remarks:
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndseslab(void)
{
    return DFSDendslab();
}

/*-----------------------------------------------------------------------------
 * Name:    dsirslab
 * Purpose: Call DFSDreadslab to read slab from SDS
 * Inputs:  filename: name of HDF file
 *          start: array of size = rank of data, containing start of slab
 *          slab_size: array of size rank, containing end of slab
 *          stride: sub sampling stride.
 *          buffer: array for returning slab
 *          buffer_size: dimensions of array data
 *          fnlen: length of filename
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDreadslab
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndsirslab(_fcd filename, intf * fnlen, intf start[], intf slab_size[],
          intf stride[], VOIDP buffer, intf buffer_size[])
{
    char       *fn;
    intf        ret;
    intn        rank, i;
    int32      *lbuffer_size, *lslab_size, *lstart, *lstride;
    int32	*p, *wp, *wsp, *sp;
    intn        isndg;

    /* Convert "filename" to fortran string */
    fn = HDf2cstring(filename, (intn) *fnlen);
    if (fn == NULL)
        return FAIL;

    /* If DFSDgetdims has not be called call DFSDIsdginfo to refresh Readsdg */
    if (DFSDIrefresh(fn) < 0)
        return FAIL;

    ret = DFSDIisndg(&isndg);
    if (isndg)
      {
          ret = DFSDIgetrrank(&rank);
          p = (int32 *) HDmalloc((size_t) rank * sizeof(int32));
          if (p == NULL)
              return FAIL;
          lbuffer_size = p;
          wp = (int32 *) HDmalloc((size_t) rank * sizeof(int32));
          if (wp == NULL)
              return FAIL;
          lslab_size = wp;
          wsp = (int32 *) HDmalloc((size_t) rank * sizeof(int32));
          if (wsp == NULL)
              return FAIL;
          lstart = wsp;
          sp = (int32 *) HDmalloc((size_t) rank * sizeof(int32));
          if (sp == NULL)
              return FAIL;
          lstride = sp;

          for (i = 1; i <= rank; i++)
            {
                *p = buffer_size[rank - i];
                p++;
                *wp = slab_size[rank - i];
                wp++;
                *wsp = start[rank - i];
                wsp++;
                *sp = stride[rank - i];
                sp++;
            }
          ret = DFSDreadslab(fn, lstart, lslab_size, lstride, buffer,
                             lbuffer_size);
          HDfree((VOIDP) lstart);
          HDfree((VOIDP) lslab_size);
          HDfree((VOIDP) lbuffer_size);
          HDfree((VOIDP) lstride);
      }
    else
        ret = DFSDreadslab(fn, (int32 *) start, (int32 *) slab_size,
                           (int32 *) stride, buffer, (int32 *) buffer_size);
    HDfree(fn);
    return (ret);
}
