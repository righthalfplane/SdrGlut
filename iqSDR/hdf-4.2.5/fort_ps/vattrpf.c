/****************************************************************
 * NCSA HDF                                                     *
 * Software Development Group                                   *
 * National Center for Supercomputing Applications              *
 * University of Illinois at Urbana-Champaign                   *
 * 605 E. Springfield, Champaign IL 61820                       *
 *                                                              *
 * For conditions of distribution and use, see the accompanying *
 * hdf/COPYING file.                                            *
 *                                                              *
 ****************************************************************/

#ifdef RCSID
static char RcsId[] = "@(#)$Revision: 4888 $";
#endif

/* $Id: vattrpf.c 4888 2007-07-17 21:16:15Z swegner $ */
/*
   *
   * vattrpf.c -- based on vattrf.c,v 1.9
   * Part of the HDF Vset attribute interface.
   *
   * C routines (short names) to be called from fortran
   *
   *
   ******************************************************/

#include "hdf.h"
#include "hproto_fortran.h"

/* ----------------- vsfcfdx ---------------------- 
 *  get field index
 *  VSfindex -- vsfcfdx -- vsffidx
 */

FRETVAL(intf)
nvsfcfdx(intf *vsid, _fcd fldnm, intf *findex,
         intf *fldnmlen)
{
    intf  ret;
    char *fld;

    fld = HDf2cstring(fldnm, (intn) *fldnmlen);
    if (!fld) return(FAIL);
    ret = (intf) VSfindex((int32) *vsid, fld, (int32 *) findex);
    HDfree(fld);
    return(ret);
}

/* -------------------------------------------------
 * vsfcsat -- set a numeric attr for a vdata or a field
 *            of a vdata
 * VSsetattr -- vsfcsat -- vsfsnat
 */

FRETVAL(intf)
nvsfcsat(intf *vsid, intf *findex, _fcd attrnm, intf *dtype,
         intf *count, intf *values, intf *attrnmlen)
{
    intf  ret;
    char *attrname;
    int32 cfindex;

    attrname = HDf2cstring(attrnm, (intn) *attrnmlen);
    if (!attrname) return(FAIL);
    cfindex = (*findex == -1) ? (int32)_HDF_VDATA : *findex;
    ret = (intf )VSsetattr((int32) *vsid, (int32) cfindex, attrname,
              (int32) *dtype, (int32) *count, (VOIDP) values);
    HDfree(attrname);
    return(ret);
} 

/* ----------------------------------------------------
 * vsfcsca -- set char attr for vdata or its field
 * 
 * VSsetattr -- vsfcsca -- vsfscat
 */

FRETVAL(intf)
nvsfcsca(intf *vsid, intf *findex, _fcd attrnm, intf *dtype,
         intf *count, _fcd values, intf *attrnmlen)
{
    intf ret;
    char *attrname;
    int32 cfindex;
 
    attrname = HDf2cstring(attrnm, (intn) *attrnmlen);
    if (!attrname) return(FAIL);
    cfindex = (*findex == -1)? (int32)_HDF_VDATA : *findex;
    ret = (intf) VSsetattr((int32) *vsid, (int32) cfindex, attrname,
          (int32) *dtype, (int32) *count, (VOIDP) _fcdtocp(values));
    HDfree(attrname);
    return(ret);
}

/* -------------------------------------------------------
 * vsfcnats -- get total number of attributes of a vdata and
 *             its fields
 * VSnattrs -- vsfcnats -- vsfnats
 */

FRETVAL(intf)
nvsfcnats(intf *vsid)
{
   intf ret;

   ret = (intf) VSnattrs((int32) *vsid); 
   return(ret);
}

/* -------------------------------------------------------
 * vsfcfnas -- get number of attributes of a vdata or of a field
 *
 * VSfnattrs -- vsfcfnas -- vsffnas
 */

FRETVAL(intf)
nvsfcfnas(intf *vsid, intf *findex)
{ 
    intf ret;
    int32 cfindex;

    cfindex = (*findex == -1)? (int32)_HDF_VDATA : *findex;
    ret = (intf) VSfnattrs((int32) *vsid, (int32) cfindex);
    return(ret);
}

/* ---------------------------------------------------------
 * vsfcfda -- find attribute of a vdata of a field
 *               of the vdata
 *    VSfindattr -- vsfcfda -- vsffdat
 */

FRETVAL(intf)
nvsfcfda(intf *vsid, intf *findex, _fcd attrnm, intf *attrnmlen)
{
    intf ret;
    char *attrname;
    int32 cfindex;

    attrname = HDf2cstring(attrnm, (intn) *attrnmlen);
    if (!attrname) return(FAIL);
    cfindex = (*findex == -1)? (int32)_HDF_VDATA : *findex;
    
    ret = (intf) VSfindattr((int32) *vsid, (int32) cfindex, attrname);
    HDfree(attrname);
    return(ret);
}

/* ---------------------------------------------------------
 * vsfcainf -- get attribute info
 * VSattrinfo -- vsfcainf -- vsfainf
 */

FRETVAL(intf)
nvsfcainf(intf *vsid, intf *findex, intf *aindex, _fcd attrname,
         intf *dtype, intf *count, intf *size)
{
    intf ret;
    int32 cfindex;

    cfindex = (*findex == -1)? (int32)_HDF_VDATA : *findex;
    ret = (intf) VSattrinfo((int32) *vsid, (int32) cfindex, (int32) *aindex,
          _fcdtocp(attrname), (int32 *) dtype, (int32 *) count, 
          (int32 *) size);
    return(ret);
}

/* ---------------------------------------------------------
 * vsfcgna -- get values of a numeric attribute 
 * VSgetattr -- vsfcgna -- vsfgnat
 */

FRETVAL(intf)
nvsfcgna(intf *vsid, intf *findex, intf *aindex, intf *values)
{
    intf ret;
    int32 cfindex;

    cfindex = (*findex == -1)? (int32)_HDF_VDATA : *findex;
    ret = (intf) VSgetattr((int32) *vsid,(int32) cfindex,(int32) *aindex,
                           (VOIDP) values);
    return(ret);
}

/* --------------------------------------------------------
 * vsfcgca -- get values of a char type attribute 
 * VSgetattr -- vsfcgca -- vsfgcat
 */

FRETVAL(intf)
nvsfcgca(intf *vsid,intf *findex,intf *aindex,_fcd values)
{
    intf ret;
    int32 cfindex;

    cfindex = (*findex == -1)? (int32)_HDF_VDATA : *findex;
    ret = (intf )VSgetattr((int32) *vsid, cfindex, (int32) *aindex,
                    (VOIDP) _fcdtocp(values));
    return(ret);
}

/* ---------------------------------------------------------
 * vsfisat -- test if a vdata is an attribute of other object
 * VSisattr -- vsfcisa -- vsfisat
 */

FRETVAL(intf)
nvsfcisa(intf *vsid)
{
    intf ret;
    ret = (intf) VSisattr((int32) *vsid);
    return(ret);
}

/* ---------------------------------------------------------
 * vfcsatt -- set a numeric attr for a vgroup
 * Vsetattr -- vfcsatt -- vfsnatt
 */

FRETVAL(intf)
nvfcsatt(intf *vgid, _fcd attrnm, intf *dtype,
         intf *count, intf *values, intf *attrnmlen)
{
    intf  ret;
    char *attrname;

    attrname = HDf2cstring(attrnm, (intn) *attrnmlen);
    if (!attrname) return(FAIL);
    ret = (intf) Vsetattr((int32) *vgid, attrname, (int32) *dtype, 
                   (int32) *count, (VOIDP) values);
    HDfree(attrname);
    return(ret);
} 

/* ----------------------------------------------------
 * vfcscat -- set char attr for vgroup
 * 
 * Vsetattr -- vfcscat -- vfscatt
 */

FRETVAL(intf)
nvfcscat(intf *vgid, _fcd attrnm, intf *dtype, intf *count, 
         _fcd values, intf *attrnmlen)
{
    intf ret;
    char *attrname;
 
    attrname = HDf2cstring(attrnm, (intn) *attrnmlen);
    if (!attrname) return(FAIL);
    ret = (intf) Vsetattr((int32) *vgid, attrname, (int32) *dtype, 
           (int32) *count, (VOIDP) _fcdtocp(values));
    HDfree(attrname);
    return(ret);
}

/* -------------------------------------------------------
 * vfcnats -- get number of attributes of a vgroup 
 * Vnattrs -- vfcnats -- vfnatts
 */

FRETVAL(intf)
nvfcnats(intf *vgid)
{
   intf ret;

   ret = (intf) Vnattrs((int32) *vgid); 
   return(ret);
}

/* ---------------------------------------------------------
 * vfcfdat -- find attribute of a vgroup
 * Vfindattr -- vfcfdat -- vffdatt
 */

FRETVAL(intf)
nvfcfdat(intf *vgid, _fcd attrnm, intf *attrnmlen)
{
    intf ret;
    char *attrname;

    attrname = HDf2cstring(attrnm, (intn) *attrnmlen);
    if (!attrname) return(FAIL);
    ret = (intf) Vfindattr((int32) *vgid, attrname);
    HDfree(attrname);
    return(ret);
}

/* ---------------------------------------------------------
 * vfcainf -- get attribute info
 * Vattrinfo -- vfcainf -- vfainfo
 */

FRETVAL(intf)
nvfcainf(intf *vgid, intf *aindex, _fcd attrname,
         intf *dtype, intf *count, intf *size)
{
    intf ret;
    ret = (intf) Vattrinfo((int32) *vgid, (int32) *aindex,
          _fcdtocp(attrname), (int32 *) dtype, (int32 *) count, 
          (int32 *) size);
    return(ret);
}

/* ---------------------------------------------------------
 * vfcgnat -- get values of a numeric attribute 
 * Vgetattr -- vfcgnat -- vfgnatt
 */

FRETVAL(intf)
nvfcgnat(intf *vgid, intf *aindex, intf *values)
{
    intf ret;
    ret = (intf) Vgetattr((int32) *vgid, (int32) *aindex,
                    (VOIDP) values);
    return(ret);
}

/* --------------------------------------------------------
 * vfcgcat -- get values of a char type attribute 
 * Vgetattr -- vfcgcat -- vfgcatt
 */

FRETVAL(intf)
nvfcgcat(intf *vgid, intf *aindex, _fcd values)
{
    intf ret;
    ret = (intf) Vgetattr((int32) *vgid,(int32) *aindex,(VOIDP) _fcdtocp(values));
    return(ret);
}

/* ---------------------------------------------------------
 * vfcgver -- get version number of a vgroup
 * Vgetversion -- vfcgver -- vfgver
 */

FRETVAL(intf)
nvfcgver(intf *vgid)
{
    intf ret;
    ret = (intf) Vgetversion((int32) *vgid);
    return(ret);
}

