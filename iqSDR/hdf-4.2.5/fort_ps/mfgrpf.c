/****************************************************************************
 * NCSA HDF                                                                 *
 * Software Development Group                                               *
 * National Center for Supercomputing Applications                          *
 * University of Illinois at Urbana-Champaign                               *
 * 605 E. Springfield, Champaign IL 61820                                   *
 *                                                                          *
 * For conditions of distribution and use, see the accompanying             *
 * hdf/COPYING file.                                                        *
 *                                                                          *
 ****************************************************************************/

#ifdef RCSID
static char RcsId[] = "@(#)$Revision: 4888 $";
#endif

/* $Id: mfgrpf.c 4888 2007-07-17 21:16:15Z swegner $ */

/*-----------------------------------------------------------------------------
 * File:    mfgrpf.c, based on mfgrf.c,v 1.7
 * Purpose: C stubs for Fortran PowerStation GR routines
 * Invokes: mfgr.c
 * Contents:
 *  mgistrt:    Call GRstart to initialize the GR interface for a file
 *  mgifinf:    Call GRfileinfo for information about the file
 *  mgiend:     Call GRend to close down the GR interface to a file
 *  mgicreat:    Call GRcreate to create a raster image
 *  mgislct:    Call GRselect to select an existing raster image for I/O
 *  mgin2ndx:    Call GRnametoindex to map an image's name into a index in a file
 *  mgigiinf:    Call GRgetiminfo to get information about an image 
 *  mgiwimg:    Call GRwriteimage to write image data to the file
 *  mgirimg:    Call GRreadimage to read image data from the file
 *  mgiendac:    Call GRendaccess to end access to a raster image
 *  mgigdid:     Call GRgetdimid to get a dimension ID for an image 
 *                [Later]
 *  mgisdnam:    Call GRsetdimname to set a dimension's name [Later]
 *  mgigdinf:    Call GRgetdiminfo to get information about a dimension [Later]
 *  mgiid2r:    Call GRidtoref to map an RI ID into a ref. # for annotating
 *  mgir2dx:    Call GRreftoindex to map a ref. # into an index for the image
 *  mgiltil:    Call GRreqlutil to request the interlace of the next LUT read
 *  mgiimil:    Call GRreqimageil to request the interlace of the next image read
 *  mgiglid:    Call GRgetlutid to get a palette ID for an image
 *  mgilinf:    Call GRgetlutinfo to get information about a palette
 *  mgiwrlt:    Call GRwritelut to write a palette to the file
 *  mgiwclt:    Call GRwritelut to write a character palette to the file
 *  mgirdlt:    Call GRreadlut to read a palette from the file
 *  mgirclt:    Call GRreadlut to read a character palette from the file
 *  mgisxfil:    Call GRsetexternalfile to move an image into an external file
 *  mgiactp:    Call GRsetaccesstype to set the access type for an image
 *  mgiscomp:    Call GRsetcompress to compress an image in the file [Later]
 *  mgisattr:    Call GRsetattr to write an attribute for an object
 *  mgiscatt:   Call GRsetattr to write a char attribute for an obj.
 *  mgiainf:    Call GRattrinfo get information about an attribute
 *  mgigcat:    Call GRgetattr to read a char attribute from the file
 *  mgignat:    Call GRgetattr to read a numeric attribute from the file
 *  mgigatt:    Call GRgetattr to read an attribute from the file
 *  mgifndat:    Call GRfindattr to get the index of an attribute for a name
 * Remarks: 
 *---------------------------------------------------------------------------*/

#include "hdf.h"
#include "hproto_fortran.h"
#include "mfgr.h"

/* Local pre-processor macros */
#define XDIM    0
#define YDIM    1

/*-----------------------------------------------------------------------------
 * Name:    mgistrt
 * Purpose: Call GRstart to initialize the GR interface for a file
 * Inputs:  fid: HDF file ID of file to initialize
 * Returns: GR ID on success, FAIL on failure
 * Users:   mgstart
 * Invokes: GRstart
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgistrt(intf * fid)
{
    return((intf)GRstart((int32)*fid));
}   /* end mgistrt() */

/*-----------------------------------------------------------------------------
 * Name:    mgifinf
 * Purpose: Call GRfileinfo to get information about the images in a file
 * Inputs:  grid: GR ID of file for information
 *          n_datasets: # of raster images in the file
 *          n_attrs: # of "global" GR attributes
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   mgfinfo
 * Invokes: GRfileinfo
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgifinf(intf * grid,intf *n_datasets,intf *n_attrs)
{
    int32 n_data, n_attr;
    intf ret;

    ret=(intf)GRfileinfo((int32)*grid,&n_data,&n_attr);
    *n_datasets=(intf)n_data;
    *n_attrs=(intf)n_attr;
    return(ret);
}   /* end mgifinf() */

/*-----------------------------------------------------------------------------
 * Name:    mgiend
 * Purpose: Call GRend to close the GR interface for a file
 * Inputs:  grid: GR ID of interface to close
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   mgend
 * Invokes: GRend
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgiend(intf * grid)
{
    return((intf)GRend((int32)*grid));
}   /* end mgiend() */

/*-----------------------------------------------------------------------------
 * Name:    mgicreat
 * Purpose: Call GRcreate to create a raster image.
 * Inputs:
 *      grid: GR ID of interface to create image in
 *      name: name of raster image
 *      ncomp: number of components per pixel
 *      nt: number-type of each component
 *      il: interlace scheme to use
 *      dimsizes[2]: dimensions of the image to create
 *      nlen: length of the name string
 * Returns: RI ID on success, FAIL on failure
 * Users:   mgcreat
 * Invokes: GRcreate
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgicreat(intf * grid, _fcd name, intf *ncomp, intf *nt, intf *il, intf dimsizes[2], intf *nlen)
{
    char       *fn;
    int32       cdims[2];
    intf        ret;

    /* Convert the FORTRAN string into a C string */
    fn = HDf2cstring(name, (intn)*nlen);
    if (fn == NULL)
        return FAIL;

    /* Copy the array of dimensions into an array of the proper type */
    cdims[XDIM]=(int32)dimsizes[XDIM];
    cdims[YDIM]=(int32)dimsizes[YDIM];
    ret = (intf)GRcreate((int32)*grid, fn, (int32)*ncomp,(int32)*nt,(int32)*il,cdims);
    HDfree(fn);

    return(ret);
}   /* end mgicreat() */

/*-----------------------------------------------------------------------------
 * Name:    mgislct
 * Purpose: Call GRselect to choose an existing raster image.
 * Inputs:
 *      grid: GR ID of interface
 *      index: the index of the image to select
 * Returns: RI ID on success, FAIL on failure
 * Users:   mgselct
 * Invokes: GRselect
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgislct(intf * grid, intf *index)
{
    return((intf)GRselect((int32)*grid,(int32)*index));
}   /* end mgislct() */

/*-----------------------------------------------------------------------------
 * Name:    mgin2ndx
 * Purpose: Call GRnametoindex to map a raster image name to an index
 * Inputs:
 *      grid: GR ID of interface
 *      name: the name of the raster image to find
 *      nlen: the length of the name
 * Returns: image index on success, FAIL on failure
 * Users:   mgn2ndx
 * Invokes: GRnametoindex
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgin2ndx(intf * grid, _fcd name, intf *nlen)
{
    char       *fn;
    intf        ret;

    /* Convert the FORTRAN string into a C string */
    fn = HDf2cstring(name, (intn)*nlen);
    if (fn == NULL)
        return FAIL;

    /* Copy the array of dimensions into an array of the proper type */
    ret = (intf)GRnametoindex((int32)*grid, fn);
    HDfree(fn);

    return(ret);
}   /* end mgin2ndx() */

/*-----------------------------------------------------------------------------
 * Name:    mgigiinf
 * Purpose: Call GRgetiminfo to get information about an image
 * Inputs:
 *      riid: RI ID of the image
 *      name: the name of the raster image to find
 *      ncomp: the number of components each pixel has
 *      nt: the number type of each component
 *      il: the interlace of the image components
 *      dimsizes: the dimensions of the image
 *      nattr: the number of attributes the image has
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   mggiimf
 * Invokes: GRgetiminfo
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgigiinf(intf * riid, _fcd name, intf *ncomp, intf *nt, intf *il, intf *dimsizes, intf *nattr)
{
    int32   t_ncomp,
        t_nt,
        t_il,
        t_dimsizes[2],
        t_nattr;
    intf    ret;

    /* Copy the array of dimensions into an array of the proper type */
    ret = (intf)GRgetiminfo((int32)*riid, (char *)_fcdtocp(name), &t_ncomp, &t_nt, &t_il, t_dimsizes, &t_nattr);
    *ncomp=(intf)t_ncomp;
    *nt=(intf)t_nt;
    *il=(intf)t_il;
    dimsizes[XDIM]=t_dimsizes[XDIM];
    dimsizes[YDIM]=t_dimsizes[YDIM];
    *nattr=(intf)t_nattr;

    return(ret);
}   /* end mgigiinf() */

/*-----------------------------------------------------------------------------
 * Name:    mgiwcim
 * Purpose: Call mgwrimg to write char type image data to the file
 * Inputs:
 *      riid: RI ID of the image
 *      start: the starting location of the image data to write
 *      stride: the stride of image data to write
 *      count: the number of pixels in each dimension to write
 *      data: the image data (pixels) to write out
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   mgwcimg
 * Invokes: GRwriteimage
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgiwcim(intf * riid, intf *start, intf *stride, intf *count, _fcd data)
{
    return(nmgiwimg(riid, start, stride, count, (VOIDP)_fcdtocp(data)));
}   /* end mgiwcim() */

/*-----------------------------------------------------------------------------
 * Name:    mgiwimg
 * Purpose: Call GRwriteimage to write image data to the file
 * Inputs:
 *      riid: RI ID of the image
 *      start: the starting location of the image data to write
 *      stride: the stride of image data to write
 *      count: the number of pixels in each dimension to write
 *      data: the image data (pixels) to write out
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   mgiwcim, mgwrimg
 * Invokes: GRwriteimage
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgiwimg(intf * riid, intf *start, intf *stride, intf *count, VOIDP data)
{
    int32 t_start[2],
        t_stride[2],
        t_count[2];

    /* Copy the array of dimensions into an array of the proper type */
    t_start[XDIM]=(int32)start[XDIM]; t_start[YDIM]=(int32)start[YDIM];
    t_stride[XDIM]=(int32)stride[XDIM]; t_stride[YDIM]=(int32)stride[YDIM];
    t_count[XDIM]=(int32)count[XDIM]; t_count[YDIM]=(int32)count[YDIM];

    return((intf)GRwriteimage((int32)*riid, t_start, t_stride, t_count, data));
}   /* end mgiwimg() */

/*-----------------------------------------------------------------------------
 * Name:    mgircim
 * Purpose: Call mgrdimg to read char type image data from a file
 * Inputs:
 *      riid: RI ID of the image
 *      start: the starting location of the image data to read
 *      stride: the stride of image data to read
 *      count: the number of pixels in each dimension to read
 *      data: the image data (pixels) to read out
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   mgrcimg
 * Invokes: GRreadimage
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgircim(intf * riid, intf *start, intf *stride, intf *count, _fcd data)
{
    return(nmgirimg(riid,start,stride,count,(VOIDP)_fcdtocp(data)));
}   /* end mgircim() */

/*-----------------------------------------------------------------------------
 * Name:    mgirimg
 * Purpose: Call GRreadimage to read image data from a file
 * Inputs:
 *      riid: RI ID of the image
 *      start: the starting location of the image data to read
 *      stride: the stride of image data to read
 *      count: the number of pixels in each dimension to read
 *      data: the image data (pixels) to read out
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   mgircim, mgrdimg
 * Invokes: GRreadimage
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgirimg(intf * riid, intf *start, intf *stride, intf *count, VOIDP data)
{
    int32 t_start[2],
        t_stride[2],
        t_count[2];

    /* Copy the array of dimensions into an array of the proper type */
    t_start[XDIM]=(int32)start[XDIM]; t_start[YDIM]=(int32)start[YDIM];
    t_stride[XDIM]=(int32)stride[XDIM]; t_stride[YDIM]=(int32)stride[YDIM];
    t_count[XDIM]=(int32)count[XDIM]; t_count[YDIM]=(int32)count[YDIM];

    return((intf)GRreadimage((int32)*riid, t_start, t_stride, t_count, data));
}   /* end mgirimg() */

/*-----------------------------------------------------------------------------
 * Name:    mgiendac
 * Purpose: Call GRendaccess to terminate access to a raster image
 * Inputs:
 *      riid: RI ID of the image
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   mgendac
 * Invokes: GRendaccess
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgiendac(intf * riid)
{
    return((intf)GRendaccess((int32)*riid));
}   /* end mgiendac() */

/*-----------------------------------------------------------------------------
 * Name:    mgiid2r
 * Purpose: Call GRidtoref to map a RI index into a reference #
 * Inputs:
 *      riid: RI ID of the image
 * Returns: ref. # on success, FAIL on failure
 * Users:   HDF Fortran programmers
 * Invokes: GRidtoref
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgiid2r(intf * riid)
{
    return((intf)GRidtoref((int32)*riid));
}   /* end mgiid2r() */

/*-----------------------------------------------------------------------------
 * Name:    mgir2dx
 * Purpose: Call GRreftoindex to map a reference # into a ri index
 * Inputs:
 *      grid: GR ID of the file
 *      ref: ref. # of the image
 * Returns: a valid index # on success, FAIL on failure
 * Users:   HDF Fortran programmers
 * Invokes: GRreftoindex
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgir2dx(intf * grid, intf *ref)
{
    return((intf)GRreftoindex((int32)*grid,(uint16)*ref));
}   /* end mgir2dx() */

/*-----------------------------------------------------------------------------
 * Name:    mgiltil
 * Purpose: Call GRreqlutil to request the interlacing scheme for the next LUT
 * Inputs:
 *      riid: RI ID of the image
 *      il: interlace scheme for next read
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   mgrltil
 * Invokes: GRreqlutil
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgiltil(intf * riid, intf *il)
{
    return((intf)GRreqlutil((int32)*riid,(intn)*il));
}   /* end mgiltil() */

/*-----------------------------------------------------------------------------
 * Name:    mgiimil
 * Purpose: Call GRreqimageil to request the interlacing scheme for the next image
 * Inputs:
 *      riid: RI ID of the image
 *      il: interlace scheme for next read
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   mgrimil
 * Invokes: GRreqimageil
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgiimil(intf * riid, intf *il)
{
    return((intf)GRreqimageil((int32)*riid,(intn)*il));
}   /* end mgiimil() */

/*-----------------------------------------------------------------------------
 * Name:    mgiglid
 * Purpose: Call GRgetlutid to get the palette ID for an image
 * Inputs:
 *      riid: RI ID of the image
 *      lut_index: index of the palette to select
 * Returns: valud LUT ID on success, FAIL on failure
 * Users:   mggltid
 * Invokes: GRgetlutid
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgiglid(intf * riid, intf *lut_index)
{
    return((intf)GRgetlutid((int32)*riid,(intn)*lut_index));
}   /* end mgiglid() */

/*-----------------------------------------------------------------------------
 * Name:    mgiglinf
 * Purpose: Call GRgetlutinfo to get information about a palette
 * Inputs:
 *      lutid: LUT ID of the image
 *      ncomp: number of components per palette entry
 *      nt: number-type of components in palette entry
 *      il: interlace of components in palette entry
 *      nentries: number of palette entries
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   mgglinf
 * Invokes: GRgetlutinfo
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgiglinf(intf * lutid, intf *ncomp, intf *nt, intf *il, intf *nentries)
{
    int32 t_ncomp, t_nt, t_il, t_nentries;
    intf status;

    status=(intf)GRgetlutinfo((int32)*lutid,&t_ncomp,&t_nt,&t_il,&t_nentries);
    *ncomp=(intf)t_ncomp;
    *nt=(intf)t_nt;
    *il=(intf)t_il;
    *nentries=(intf)t_nentries;
    return(status);
}   /* end mgiglinf() */

/*-----------------------------------------------------------------------------
 * Name:    mgiwclt
 * Purpose: Call GRwritelut to write data into a character palette
 * Inputs:
 *      lutid: LUT ID of the image
 *      ncomp: number of components per palette entry
 *      nt: number-type of components in palette entry
 *      il: interlace of components in palette entry
 *      nentries: number of palette entries
 *      data: data to write
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   mgwclut
 * Invokes: GRwritelut
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgiwclt(intf * lutid, intf *ncomp, intf *nt, intf *il, intf *nentries, _fcd data)
{
    return((intf)GRwritelut((int32)*lutid,(int32)*ncomp,(int32)*nt,(int32)*il,(int32)*nentries,(VOIDP)_fcdtocp(data)));
}   /* end mgiwclt() */

/*-----------------------------------------------------------------------------
 * Name:    mgiwrlt
 * Purpose: Call GRwritelut to write data into a palette
 * Inputs:
 *      lutid: LUT ID of the image
 *      ncomp: number of components per palette entry
 *      nt: number-type of components in palette entry
 *      il: interlace of components in palette entry
 *      nentries: number of palette entries
 *      data: data to write
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   mgwrlut
 * Invokes: GRwritelut
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgiwrlt(intf * lutid, intf *ncomp, intf *nt, intf *il, intf *nentries, VOIDP data)
{
    return((intf)GRwritelut((int32)*lutid,(int32)*ncomp,(int32)*nt,(int32)*il,(int32)*nentries,data));
}   /* end mgiwrlt() */

/*-----------------------------------------------------------------------------
 * Name:    mgirclt
 * Purpose: Call GRreadlut to read data from a character palette
 * Inputs:
 *      lutid: LUT ID of the image
 *      data: data to read
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   mgrclut
 * Invokes: GRreadlut
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgirclt(intf * lutid, _fcd data)
{
    return((intf)GRreadlut((int32)*lutid,(VOIDP)_fcdtocp(data)));
}   /* end mgirclt() */

/*-----------------------------------------------------------------------------
 * Name:    mgirdlt
 * Purpose: Call GRreadlut to read data from a palette
 * Inputs:
 *      lutid: LUT ID of the image
 *      data: data to read
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   mgrdlut
 * Invokes: GRreadlut
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgirdlt(intf * lutid, VOIDP data)
{
    return((intf)GRreadlut((int32)*lutid,data));
}   /* end mgirdlt() */

/*-----------------------------------------------------------------------------
 * Name:    mgisxfil
 * Purpose: Call GRsetexternalfile to convert an image into an external image
 * Inputs:
 *      riid: RI ID of the image
 *      filename: filename of the external file
 *      offset: offset in the external file to place image at
 *      nlen: the length of the name
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   mgsxfil
 * Invokes: GRsetexternalfile
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgisxfil(intf * riid, _fcd filename, intf *offset, intf *nlen)
{
    char       *fn;
    intf        ret;

    /* Convert the FORTRAN string into a C string */
    fn = HDf2cstring(filename, (intn)*nlen);
    if (fn == NULL)
        return FAIL;

    ret=(intf)GRsetexternalfile((int32)*riid,fn,(int32)*offset);
    HDfree(fn);

    return(ret);
}   /* end mgisxfil() */

/*-----------------------------------------------------------------------------
 * Name:    mgiactp
 * Purpose: Call GRsetaccesstype to determine access mode for an image
 * Inputs:
 *      riid: RI ID of the image
 *      accesstype: the type of access to image data
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   mgsactp
 * Invokes: GRsetaccesstype
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgiactp(intf * riid, intf *accesstype)
{
    return((intf)GRsetaccesstype((int32)*riid,(uintn)*accesstype));
}   /* end mgiactp() */

/*-----------------------------------------------------------------------------
 * Name:    mgiscatt
 * Purpose: Call mgisatt to store a char attribute about an image
 * Inputs:
 *      riid: RI ID of the image
 *      name: the name of the attribute
 *      nt: the number-type of the attribute
 *      count: the number of values in the attribute
 *      data: the data for the attribute
 *      nlen: the length of the name
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   HDF Fortran programmers
 * Invokes: GRsetattr
 *---------------------------------------------------------------------------*/
FRETVAL(intf)
nmgiscatt(intf * riid, _fcd name, intf *nt, intf *count, _fcd data, intf *nlen)
{
    return(nmgisattr(riid, name, nt, count, (VOIDP) _fcdtocp(data),
                    nlen));
}   /* end mgiscatt() */
/*-----------------------------------------------------------------------------
 * Name:    mgisattr
 * Purpose: Call GRsetattr to store an attribute about an image
 * Inputs:
 *      riid: RI ID of the image
 *      name: the name of the attribute
 *      nt: the number-type of the attribute
 *      count: the number of values in the attribute
 *      data: the data for the attribute
 *      nlen: the length of the name
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   HDF Fortran programmers
 * Invokes: GRsetattr
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgisattr(intf * riid, _fcd name, intf *nt, intf *count, VOIDP data, intf *nlen)
{
    char       *fn;
    intf        ret;

    /* Convert the FORTRAN string into a C string */
    fn = HDf2cstring(name, (intn)*nlen);
    if (fn == NULL)
        return FAIL;

    ret=(intf)GRsetattr((int32)*riid,fn,(int32)*nt,(int32)*count,data);
    HDfree(fn);

    return(ret);
}   /* end mgisattr() */

/*-----------------------------------------------------------------------------
 * Name:    mgiainf
 * Purpose: Call GRattrinfo to get information about an attribute
 * Inputs:
 *      riid: RI ID of the image
 *      index: the index of the attribute
 *      name: the name of the attribute
 *      nt: the number-type of the attribute
 *      count: the number of values in the attribute
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   mgainf
 * Invokes: GRattrinfo
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgiainf(intf * riid, intf *index, _fcd name, intf *nt, intf *count)
{
    int32 t_nt, t_count;
    intf        ret;

    ret=(intf)GRattrinfo((int32)*riid,(int32)*index,(char *)_fcdtocp(name),&t_nt,&t_count);
    *nt=(intf)t_nt;
    *count=(intf)t_count;

    return(ret);
}   /* end mgiainf() */

/*-----------------------------------------------------------------------------
 * Name:    mgigcat
 * Purpose: Call mgignat to get a char attribute
 * Inputs:
 *      riid: RI ID of the image
 *      index: the index of the attribute
 *      data: the data for the attribute
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   HDF Fortran programmers
 * Invokes: GRgetattr
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgigcat(intf * riid, intf *index, _fcd data)
{
    return(nmgignat(riid, index, (VOIDP) _fcdtocp(data)));
}   /* end mgigcat() */

/*-----------------------------------------------------------------------------
 * Name:    mgignat
 * Purpose: Call GRgetattr to get a numeric attribute
 * Inputs:
 *      riid: RI ID of the image
 *      index: the index of the attribute
 *      data: the data for the attribute
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   mgigcat, mggnatt
 * Invokes: GRgetattr
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgignat(intf * riid, intf *index, VOIDP data)
{
    return((intf)GRgetattr((int32)*riid,(int32)*index,data));
}   /* end mgignat() */

/*-----------------------------------------------------------------------------
 * Name:    mgigattr
 * Purpose: Call GRgetattr to get an attribute
 * Inputs:
 *      riid: RI ID of the image
 *      index: the index of the attribute
 *      data: the data for the attribute
 * Returns: SUCCEED on success, FAIL on failure
 * Users:   mggattr
 * Invokes: GRgetattr
 * Remarks: This routine is replaced by mggcatt and mggmatt
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgigatt(intf * riid, intf *index, VOIDP data)
{
    return((intf)GRgetattr((int32)*riid,(int32)*index,data));
}   /* end mgigatt() */

/*-----------------------------------------------------------------------------
 * Name:    mgifndat
 * Purpose: Call GRfindattr to locate an attribute
 * Inputs:
 *      riid: RI ID of the image
 *      name: the name for the attribute
 *      nlen: the length of the name for the attribute
 * Returns: attribute index on success, FAIL on failure
 * Users:   mgfndat
 * Invokes: GRfindattr
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
nmgifndat(intf * riid, _fcd name, intf *nlen)
{
    char       *fn;
    intf        ret;

    /* Convert the FORTRAN string into a C string */
    fn = HDf2cstring(name, (intn)*nlen);
    if (fn == NULL)
        return FAIL;

    ret=(intf)GRfindattr((int32)*riid,fn);
    HDfree(fn);

    return(ret);
}   /* end mgifndat() */

/*-------------------------------------------------------------------------
 * Name:    mgcgichnk
 * Puporse: Call GRgetchunkinfo
 * Inputs:  id: access id to GR
 * Outputs: dim_length: chunk dimensions
 *          flags:            -1 - GR is nonchunked
 *                             0 - GR is chunked, no compression
 *                             1 - GR is chunked and compressed 
 * Returns: 0 on success, -1 on failure with error set
 * Users:   HDF Fortran programmers          
 *-------------------------------------------------------------------------*/

    FRETVAL (intf)
#ifdef PROTOTYPE
      nmgcgichnk(intf *id, intf *dim_length, intf *flags)
#else
       nmgcgichnk( id, dim_length, flags)
       intf *id;       
       intf *dim_length;
       intf *flags;
#endif /* PROTOTYPE */
{

HDF_CHUNK_DEF chunk_def;  /* Chunk definition set */
int32 riid;               /* GR id               */
comp_info cinfo;          /* compression info     */
int   i;
int32 rank, status, cflags, comp_type;
intf ret;

riid = *id;
rank = 2;

/* Get GR info */
  
       status = GRgetchunkinfo(riid, &chunk_def, &cflags);
       if(status == FAIL) return FAIL;

switch (cflags)  

  {

  case HDF_NONE:       /* Non-chunked GR */

    *flags = -1;
     ret = 0;
     return(ret);

  case HDF_CHUNK:    /* Chunked, noncompressed GR */

    *flags = 0;
     for (i=0; i < rank; i++)
          dim_length[rank-i-1] = chunk_def.chunk_lengths[i];
     ret = 0;
     return(ret);

  case (HDF_CHUNK | HDF_COMP):     /* Chunked and compressed GR */
 
     *flags = 1;
     for (i=0; i < rank; i++)
          dim_length[rank-i-1] =  chunk_def.comp.chunk_lengths[i];
     ret = 0;
     return(ret);

  default:

    return FAIL;
            
  }


}   
//#if 0 /* Commented out for now  -EIP 12/29/97 */ 
/*-----------------------------------------------------------------------------
 * Name:     mgcrcchnk
 * Purpose:  read the specified chunk of CHARACTER data to the buffer
 * Inputs:   id        - access ID to GR
 *           start     - origin of chunk to read 
 * Outputs:  char_data  - buffer the data will be read into
 * Calls:    scrchnk 
 * Reamrks:  dimensions will be flipped in scrchnk function
 * Returns:  0 on success, -1 on failure with error set
 *----------------------------------------------------------------------------*/   
    FRETVAL (intf)
#ifdef PROTOTYPE 
       nmgcrcchnk(intf *id, intf *start, _fcd char_data)
#else
       nmgcrcchnk(id, start, char_data)
                intf *id;
                intf *start;
               _fcd  char_data;
#endif /* PROTOTYPE */

{
       intf  ret;

       ret = nmgcrchnk(id, start, (VOIDP) _fcdtocp(char_data));

       return(ret);

} 

/*-----------------------------------------------------------------------------
 * Name:     mgcrchnk
 * Purpose:  read the specified chunk of NUMERIC data to the buffer
 * Inputs:   id        - access ID to GR
 *           start     - origin of chunk to read 
 * Outputs:  num_data  - buffer the data will be read into
 * Calls:    GRreadchunk
 * Remarks:  need to flip the dimensions to account for array ordering
 *           differencies (start --> cstart)
 *           If performance becomes an issue, use static cstart 
 * Returns:  0 on success, -1 on failure with error set
 *----------------------------------------------------------------------------*/   
    FRETVAL (intf)
#ifdef PROTOTYPE
       nmgcrchnk(intf *id, intf *start, VOIDP num_data)
#else
       nmgcrchnk(id, start, num_data)
                intf *id;
                intf *start;
                VOIDP num_data;
#endif /* PROTOTYPE */

{
       intf    ret;
       int32   rank, status, i;
       int32   *cstart;

       rank = 2;
/* Allocate memory for cstart array; use static array, if performance
       becomes an issue */

       cstart = (int32 *) HDmalloc(sizeof(int32) * rank);
       if(!cstart) return FAIL;

/* Flip an array to account for array odering in Fortran and C */

       for ( i=0; i < rank; i++)
             cstart[i] = start[rank-i-1] - 1;

/* Call GRreadchunk function to read the data */

       ret = GRreadchunk(*id, cstart, num_data);

/* Free memory */

       HDfree((VOIDP)cstart);
       return(ret);

} 

//#endif /*Commented out for now -EIP 12/29/97 */
/*-----------------------------------------------------------------------------
 * Name:     mgcscchnk
 * Purpose:  set the maximum number of chunks to cache 
 * Inputs:   id        - access ID to GR
 *           maxcache  - max number of chunks to cache
 *           flags     - flags= 0, HDF_CACHEALL
 *                       Currently only 0 can be passed
 * Calls:    GRsetchunkcache
 * Returns:  0 on success, -1 on failure with error set
 *----------------------------------------------------------------------------*/   
    FRETVAL (intf)
#ifdef PROTOTYPE
       nmgcscchnk(intf *id, intf *maxcache, intf *flags)
#else
       nmgcscchnk(id, maxcache, flags)
                intf *id;
                intf *maxcache;
                intf *flags;
#endif /* PROTOTYPE */

{
       intf  ret;

       ret = GRsetchunkcache(*id, *maxcache, *flags);

       return(ret);

} 

/*-------------------------------------------------------------------------
 * Name:    mgcschnk
 * Puporse: Call GRsetchunk
 * Inputs:  id: access id to GR
 *          dim_length: chunk dimensions
 *          comp_type:  type of compression
 *                              COMP_CODE_NONE    (0)
 *                              COMP_CODE_RLE     (1)
 *                              COMP_CODE_SKPHUFF (3)
 *                              COMP_CODE_DEFLATE (4)
 *                              COMP_CODE_INVALID (5)
 *          comp_prm:   compression parameters array
 *          comp_prm[0] = skphuff_skp_size: size of individual elements for 
 *                            Adaptive Huffman compression algorithm
 *          comp_prm[0] = deflate_level:    GZIP  compression parameter
 * Returns: 0 on success, -1 on failure with error set
 * Users:   HDF Fortran programmers          
 *-------------------------------------------------------------------------*/

    FRETVAL (intf)
#ifdef PROTOTYPE
      nmgcschnk(intf *id, intf *dim_length, intf *comp_type,
                intf *comp_prm)
#else
       nmgcschnk( id, dim_length, comp_type,
                 comp_prm)
       intf *id;
       intf *dim_length;
       intf *comp_type;
       intf *comp_prm;
#endif /* PROTOTYPE */
{

HDF_CHUNK_DEF chunk_def;  /* Chunk definition set */
int32 riid;               /* GR id               */
int32 cflags;             /* chunk flags          */
comp_info cinfo;          /* compression info     */
int   i, CASE;
int32 rank, status;
intf ret;

rank   = 2;
CASE   = *comp_type;
riid  = *id;
cflags = HDF_CHUNK | HDF_COMP;

switch (CASE)  {

       case 0:       /* No compression */
         cflags = HDF_CHUNK;
         for (i=0; i < rank; i++)
               chunk_def.chunk_lengths[i] = dim_length[rank-i-1]; 
         break;
    
       case 1:       /* RLE compression */
         for (i=0; i < rank; i++)
                chunk_def.comp.chunk_lengths[i] = dim_length[rank-i-1];

         chunk_def.comp.comp_type = COMP_CODE_RLE;

         break;

#ifdef GRsetchunk_does_not_support_NBIT

       case 2:      /* N-bit encoding */
         for (i=0; i < rank; i++)
                chunk_def.comp.chunk_lengths[i] = dim_length[rank-i-1]; 

         chunk_def.comp.comp_type = COMP_CODE_NBIT;
         chunk_def.comp.cinfo.nbit.sign_ext = comp_prm[0];
         chunk_def.comp.cinfo.nbit.fill_one = comp_prm[1];
         chunk_def.comp.cinfo.nbit.start_bit = comp_prm[2];
         chunk_def.comp.cinfo.nbit.bit_len = comp_prm[3];  

         break; 
 
#endif
       case 3:      /* Skipping Huffman encoding */
          for (i=0; i < rank; i++)
                chunk_def.comp.chunk_lengths[i] = dim_length[rank-i-1];

          chunk_def.comp.comp_type = COMP_CODE_SKPHUFF;
          chunk_def.comp.cinfo.skphuff.skp_size = comp_prm[0];

          break;

       case 4:      /* GZIP compression */  
          for (i=0; i < rank; i++)
                 chunk_def.comp.chunk_lengths[i] = dim_length[rank-i-1];
           
          chunk_def.comp.comp_type = COMP_CODE_DEFLATE;
          chunk_def.comp.cinfo.deflate.level = comp_prm[0];

          break;

       default:

          return FAIL;
                    
                     }

ret = GRsetchunk(riid, chunk_def, cflags);
 
return(ret);

}   
/*-----------------------------------------------------------------------------
 * Name:     mgcwcchnk
 * Purpose:  write the specified chunk of CHARACTER data to the GR 
 * Inputs:   id        - access ID to GR
 *           start     - origin of chunk to read 
 * Outputs:  char_data  - buffer the data will be read into
 * Calls:    mgcwchnk 
 * Reamrks:  dimensions will be flipped in scrchnk function
 * Returns:  0 on success, -1 on failure with error set
 *----------------------------------------------------------------------------*/   
    FRETVAL (intf)
#ifdef PROTOTYPE
       nmgcwcchnk(intf *id, intf *start, _fcd char_data)
#else
       nmgcwcchnk(id, start, char_data)
                intf *id;
                intf *start;
               _fcd  char_data;
#endif /* PROTOTYPE */

{
       intf  ret;

       ret = nmgcwchnk(id, start, (VOIDP) _fcdtocp(char_data));

       return(ret);

} 

/*-----------------------------------------------------------------------------
 * Name:     mgcwchnk
 * Purpose:  write the specified chunk of NUMERIC data to the GR
 * Inputs:   id        - access ID to GR
 *           start     - origin of chunk to write
 *           num_data  - buffer for data
 * Calls:    GRwritechunk
 * Remarks:  need to flip the dimensions to account for array ordering
 *           differencies (start --> cstart)
 *           If performance becomes an issue, use static cstart 
 * Returns:  0 on success, -1 on failure with error set
 *----------------------------------------------------------------------------*/   
    FRETVAL (intf)
#ifdef PROTOTYPE
       nmgcwchnk(intf *id, intf *start, VOIDP num_data)
#else
       nmgcwchnk(id, start, num_data)
                intf *id;
                intf *start;
                VOIDP num_data;
#endif /* PROTOTYPE */

{
       intf    ret;
       int32   rank, status, i;
       int32   *cstart;

       rank = 2;
/* Allocate memory for cstart array; use static array, if performance
       becomes an issue */

       cstart = (int32 *) HDmalloc(sizeof(int32) * rank);
       if(!cstart) return FAIL;

/* Flip an array */

       for ( i=0; i < rank; i++)
             cstart[i] = start[rank-i-1] - 1;

/* Call GRwritechunk function to write the data */

       ret = GRwritechunk(*id, cstart, num_data);

/* Free memory */ 

       HDfree((VOIDP)cstart);

       return(ret);

} 
/*-------------------------------------------------------------------------
 * Name:    mgcscompress
 * Puporse: Call GRsetcompress
 * Inputs:  id: access id to GR
 *          comp_type:  type of compression
 *                      COMP_CODE_NONE = 0
 *                      COMP_CODE_RLE  = 1
 *                      COMP_CODE_SKPHUFF = 3
 *                      COMP_CODE_DEFLATE = 4
*                      COMP_CODE_JPEG    = 6 
 *          comp_prm:   compression parameters array
 *          comp_prm[0]=skphuff_skp_size: size of individual elements for 
 *                            Adaptive Huffman compression algorithm
 *          comp_prm[0]=deflate_level:    GZIP  compression parameter
 *          comp_prm[0]=quality           JPEG parameters 
 *          comp_prm[1]=baseline  
 * Returns: 0 on success, -1 on failure with error set
 * Users:   HDF Fortran programmers          
 *-------------------------------------------------------------------------*/

    FRETVAL (intf)
#ifdef PROTOTYPE
       nmgcscompress(intf *id, intf *comp_type, intf *comp_prm)
#else
       nmgcscompress( id, comp_type, comp_prm)
       intf *id;
       intf *comp_type;
       intf *comp_prm;
#endif /* PROTOTYPE */
{

int32 riid;               /*  GR id               */
comp_info c_info;         /* compression info     */
comp_coder_t c_type;              /* compression type definition */

int   i, CASE;
intf ret;



CASE = *comp_type;
riid = *id;
switch (CASE)  {

       case COMP_CODE_NONE:       /* No compression */
         c_type = COMP_CODE_NONE;
         break;
    
       case COMP_CODE_RLE:             /* RLE compression */
         c_type = COMP_CODE_RLE;
         break;
 
       case COMP_CODE_SKPHUFF:      /* Skipping Huffman encoding */
          c_type = COMP_CODE_SKPHUFF;
          c_info.skphuff.skp_size = comp_prm[0];
          break;

       case COMP_CODE_DEFLATE:      /* GZIP compression */  
          c_type = COMP_CODE_DEFLATE;
          c_info.deflate.level = comp_prm[0];
          break;

       case COMP_CODE_JPEG:   /*JPEG compression */
          c_type = COMP_CODE_JPEG;
          c_info.jpeg.quality = comp_prm[0];
          c_info.jpeg.force_baseline = comp_prm[1];
          break;


       default:

          return FAIL;
                    
                     }

ret = GRsetcompress(riid, c_type, &c_info);
return(ret);

}   
/*-------------------------------------------------------------------------
 * Name:    mgcgnluts
 * Puporse: Call GRgetnluts
 * Inputs:  id: image identifier
 * Returns: number of palettes on success and -1 if fails.
 * Users:   HDF Fortran programmers          
 *-------------------------------------------------------------------------*/

    FRETVAL (intf)
#ifdef PROTOTYPE
       nmgcgnluts(intf *id)
#else
       nmgcgnluts( id)
       intf *id;
#endif /* PROTOTYPE */
{
   intf ret = -1;
   intn c_ret;

   c_ret = GRgetnluts ( *id );
   if(c_ret >= 0) ret = c_ret;
   return(ret);
}
/*-------------------------------------------------------------------------
 * Name:    mgcgcompress
 * Puporse: Call GRgetcompress
 * Inputs:  id: access id to GR
 * Outputs: comp_type:  type of compression
 *                      COMP_CODE_NONE = 0
 *                      COMP_CODE_RLE  = 1
 *                      COMP_CODE_SKPHUFF = 3
 *                      COMP_CODE_DEFLATE = 4
 *                      COMP_CODE_JPEG    = 7 
 *          comp_prm:   compression parameters array
 *          comp_prm[0]=skphuff_skp_size: size of individual elements for 
 *                            Adaptive Huffman compression algorithm
 *          comp_prm[0]=deflate_level:    GZIP  compression parameter
 * Returns: 0 on success, -1 on failure with error set
 * Users:   HDF Fortran programmers          
 *-------------------------------------------------------------------------*/

    FRETVAL (intf)
#ifdef PROTOTYPE
       nmgcgcompress(intf *id, intf *comp_type, intf *comp_prm)
#else
       nmgcgcompress( id, comp_type, comp_prm)
       intf *id;
       intf *comp_type;
       intf *comp_prm;
#endif /* PROTOTYPE */
{
    comp_info c_info;         /* compression info     */
    comp_coder_t c_type;              /* compression type definition */

    int CASE;
    intf ret = -1;
    intn c_ret;


    c_ret = GRgetcompress(*id, &c_type, &c_info);

    if (c_ret == 0) {
    CASE = (int)c_type;
    switch (CASE)  {

       case COMP_CODE_NONE:       /* No compression */
         *comp_type = 0;
         ret = 0;
         break;
    
       case COMP_CODE_RLE:             /* RLE compression */
         *comp_type = 1;
         ret = 0;
         break;
 
       case COMP_CODE_SKPHUFF:      /* Skipping Huffman encoding */
          *comp_type = 3;
          comp_prm[0] = (intf)c_info.skphuff.skp_size;
          ret = 0;
          break;

       case COMP_CODE_DEFLATE:      /* GZIP compression */  
          *comp_type = 4;
          comp_prm[0] = (intf)c_info.deflate.level;
          ret = 0;
          break;

       case COMP_CODE_JPEG:      /* JPEG compression */  
          *comp_type = 7;
          ret = 0;
          break;

       default:

          return FAIL;
                    
     } /*end CASE */
    } /*end if */
    return(ret);

}   
