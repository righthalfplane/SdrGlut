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

/* $Id: vattrf.c 4932 2007-09-07 17:17:23Z bmribler $ */
/*
   *
   * vattrf.c
   * Part of the HDF Vset attribute interface.
   *
   * C routines (short names) to be called from fortran
   *
   *
   ******************************************************/

#define VSET_INTERFACE
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
    cfindex = *findex;
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
    cfindex = *findex;
    ret = (intf) VSsetattr((int32) *vsid, (int32) cfindex, attrname,
          (int32) *dtype, (int32) *count, (VOIDP) _fcdtocp(values));
    HDfree(attrname);
    return(ret);
}

/* -------------------------------------------------------
 * vsfnats -- get total number of attributes of a vdata and
 *             its fields
 * VSnattrs -- vsfnats
 */

FRETVAL(intf)
nvsfnats(intf *vsid)
{
   intf ret;

   ret = (intf) VSnattrs((int32) *vsid); 
   return(ret);
}

/* -------------------------------------------------------
 * vsffnas -- get number of attributes of a vdata or of a field
 *
 * VSfnattrs -- vsffnas
 */

FRETVAL(intf)
nvsffnas(intf *vsid, intf *findex)
{ 
    intf ret;
    int32 cfindex;

    cfindex = *findex;
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
    cfindex = *findex;
    
    ret = (intf) VSfindattr((int32) *vsid, (int32) cfindex, attrname);
    HDfree(attrname);
    return(ret);
}

/* ---------------------------------------------------------
 * vsfainf -- get attribute info
 * VSattrinfo -- vsfcain -- vsfainf
 */

FRETVAL(intf)
nvsfcain(intf *vsid, intf *findex, intf *aindex, _fcd attrname,
         intf *dtype, intf *count, intf *size, intf *attrnamelen)
{
    CONSTR(FUNC, "vsfainf");
    intf ret = FAIL;
    /* temporary variables */
    int32 cfindex;
    int32 tdtype, tcount, tsize;
    char  *tattrname;

    cfindex = *findex;
    /* Allocate space for fortran strings */
    tattrname = (char *) HDmalloc(*attrnamelen + 1);
    if (!tattrname)
        HRETURN_ERROR(DFE_NOSPACE, FAIL);

    ret = (intf) VSattrinfo((int32) *vsid, (int32) cfindex, (int32) *aindex,
          tattrname, &tdtype, &tcount, &tsize);
    if (ret != FAIL){
	/* move values from temporary space to fortran variables */
	*dtype = tdtype;
	*count = tcount;
	*size = tsize;
	/* convert C-string results back to Fortran strings */
	HDpackFstring(tattrname, _fcdtocp(attrname), (intn) *attrnamelen);
    }
    HDfree(tattrname);
    return(ret);
}


/* ---------------------------------------------------------
 * vsfgnat -- get values of a numeric attribute 
 * VSgetattr -- vsfgnat
 */

FRETVAL(intf)
nvsfgnat(intf *vsid, intf *findex, intf *aindex, intf *values)
{
    intf ret;
    int32 cfindex;

    cfindex = *findex;
    ret = (intf) VSgetattr((int32) *vsid,(int32) cfindex,(int32) *aindex,
                           (VOIDP) values);
    return(ret);
}

/* --------------------------------------------------------
 * vsfgcat -- get values of a char type attribute 
 * VSgetattr -- vsfgcat
 */

FRETVAL(intf)
nvsfgcat(intf *vsid,intf *findex,intf *aindex,_fcd values)
{
    intf ret;
    int32 cfindex;

    cfindex = *findex;
    ret = (intf )VSgetattr((int32) *vsid, cfindex, (int32) *aindex,
                    (VOIDP) _fcdtocp(values));
    return(ret);
}

/* ---------------------------------------------------------
 * vsfisat -- test if a vdata is an attribute of other object
 * VSisattr -- vsfisat
 */

FRETVAL(intf)
nvsfisat(intf *vsid)
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
 * vfnatts -- get number of attributes of a vgroup 
 * Vnattrs -- vfnatts
 */

FRETVAL(intf)
nvfnatts(intf *vgid)
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
 * vfainf -- get attribute info
 * Vattrinfo -- vfainfo
 */

FRETVAL(intf)
nvfainfo(intf *vgid, intf *aindex, _fcd attrname,
         intf *dtype, intf *count, intf *size)
{
    intf ret;
    ret = (intf) Vattrinfo((int32) *vgid, (int32) *aindex,
          _fcdtocp(attrname), (int32 *) dtype, (int32 *) count, 
          (int32 *) size);
    return(ret);
}

/* ---------------------------------------------------------
 * vfgnatt -- get values of a numeric attribute 
 * Vgetattr -- vfgnatt
 */

FRETVAL(intf)
nvfgnatt(intf *vgid, intf *aindex, intf *values)
{
    intf ret;
    ret = (intf) Vgetattr((int32) *vgid, *aindex,
                    (VOIDP) values);
    return(ret);
}

/* --------------------------------------------------------
 * vfgcatt -- get values of a char type attribute 
 * Vgetattr -- vfgcatt
 */

FRETVAL(intf)
nvfgcatt(intf *vgid, intf *aindex, _fcd values)
{
    intf ret;
    ret = (intf) Vgetattr((int32) *vgid,(int32) *aindex,(VOIDP) _fcdtocp(values));
    return(ret);
}

/* ---------------------------------------------------------
 * vfgver -- get version number of a vgroup
 * Vgetversion -- vfgver
 */

FRETVAL(intf)
nvfgver(intf *vgid)
{
    intf ret;
    ret = (intf) Vgetversion((int32) *vgid);
    return(ret);
}

