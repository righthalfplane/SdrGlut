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

/* $Id: vhi.c 4932 2007-09-07 17:17:23Z bmribler $ */
/*
   * File
   *       vhi.c
   *       HDF Vset high-level access routines VHxxxx
   *       Feb 92 - update to use H-routines
   * Routines
   *       VHstoredata  -- stores data in a field of a vdata in an HDF file
   *       VHstoredatam -- stores data in a aggregated-typed field of a vdata
   *       VHmakegroup  -- makes a vgroup from tag/ref pairs
 */

#define VSET_INTERFACE
#include "hdf.h"

/* ------------------------ VHstoredata -------------------------------
   NAME
   VHstoredata -- stores data in a field of a new vdata
   USAGE
   int32 VHstoredata (f, field, buf, n, datatype, vsname, vsclass)
   HFILEID f;           IN: File id, returned from Hopen.
   char *  field;       IN: Name of the field.
   const uint8  *buf;       IN: Buffer of data to be stored in the field.
   int32   n;           IN: Number of elements in buf to be written.
   inter   datatype;    IN: Number type of the data to be written.
   char *  vsname;      IN: Name of the new vdata.
   char *  vsclass;     IN: Class of the new vdata.

   RETURNS
   On success returns reference number of the new vdata, a positive integer;
   on failure returns -1.
   DESCRIPTION
   Stores 'n' elements of data from 'buf' as a field 'field' in a new vdata
   called 'vsname' into the already opened HDF file (with file id 'f').
   The datatype variable must be specified as a valid HDF type; n should not
   be zero or negative.
   Returns -1 if error; ref of that new vdata (a +ve integer) if successful.
   ------------------------------------------------------------------------- */

int32
VHstoredata(HFILEID f, const char *field, const uint8 *buf, int32 n, int32 datatype,
            const char *vsname, const char *vsclass)

{
#ifdef LATER
    CONSTR(FUNC, "VHstoredata");
#endif
    int32       order = 1;
    int32       ret_value;

    ret_value = ((int32)VHstoredatam(f, field, buf, n, datatype, vsname, vsclass, order));

    return ret_value;
} /* end VHstoredata */

/* ----------------------- VHstoredatam ----------------------------
   NAME
   VHstoredatam -- Same as VHstoredata but allows aggregate-typed field.
   USAGE
   int32 VHstoredata (f, field, buf, n, datatype, vsname, vsclass, order)
   HFILEID f;           IN: File id, returned from Hopen.
   char *  field;       IN: Name of the field.
   uint8   buf[];       IN: Buffer of data to be stored in the field.
   int32   n;           IN: Number of elements in buf to be written.
   inter   datatype;    IN: Numter type of the data to be written.
   char *  vsname;      IN: Name of the new vdata.
   char *  vsclass;     IN: Class of the new vdata.
   int32   order;       IN: Order of the field.

   RETURNS
   On success returns reference number of the new vdata, a positive integer;
   on failure returns -1.
   DESCRIPTION
   Stores 'n' elements of data from 'buf' as a field 'field' in a new vdata
   called 'vsname' into the already opened HDF file (with file id 'f').
   The datatype variable must be specified as a valid HDF type;
   n should not be zero or negative.
   Returns -1 if error; ref of that new vdata (a +ve integer) if successful.
   --------------------------------------------------------------------------- */

int32
VHstoredatam(HFILEID f, const char *field, const uint8 *buf, int32 n, int32 datatype, const char *vsname, const char *vsclass, int32 order)
{
    CONSTR(FUNC, "VHstoredatam");
    int32       ref;
    int32       vs;
    int32       ret_value = SUCCEED;


    if ((vs = VSattach(f, -1, "w")) == FAIL)
        HGOTO_ERROR(DFE_CANTATTACH,FAIL);

    if ( VSfdefine(vs, field, datatype, order) == FAIL)
        HGOTO_ERROR(DFE_BADFIELDS,FAIL);

    if ( VSsetfields(vs, field) == FAIL)
        HGOTO_ERROR(DFE_BADFIELDS,FAIL);

    if (n != VSwrite(vs, buf, n, FULL_INTERLACE))
        HGOTO_ERROR(DFE_BADATTACH,FAIL);

    if(VSsetname(vs, vsname)==FAIL)
        HGOTO_ERROR(DFE_BADVSNAME,FAIL);

    if(VSsetclass(vs, vsclass)==FAIL)
        HGOTO_ERROR(DFE_BADVSCLASS,FAIL);

    ref = VSQueryref(vs);
    if(VSdetach(vs)==FAIL)
        HGOTO_ERROR(DFE_CANTDETACH,FAIL);

    ret_value = ((int32) ref);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
  return ret_value;
}   /* VHstoredatam */

/* ------------------------ VHmakegroup ------------------------------- */
/*
   NAME
   VHmakegroup -- Creates a vgroup to store pairs of tag/ref.
   USAGE
   int32 VHmakegroup (f, tagarray, refarray , n, vgname, vgclass)
   HFILEID f;           IN: File id, returned from Hopen.
   int32   tagarray[];  IN: Array of tags to be stored in the vgroup.
   int32   refarray[];  IN: Array of refs to be stored in the vgroup.
   int32   n;           IN: Number of tags/refs in the tagarray/refarray.
   char    * vgname;    IN: Name of the new vgroup.
   char    * vgclass;   IN: Class of the new vgroup.

   RETURNS
   On success returns reference number of the new vgroup, a positive integer;
   on failure returns -1.
   DESCRIPTION
   Takes an array of tags and and array of refs and create a vgroup to
   store them. You tell it how many tag/ref pairs there are. You must
   also give the vgroup a name.i Creating EMPTY vgroups is allowed.
   VHmakegroup does bot check if a tag/ref is valid or exist,
   but ALL tag/ref pairs MUST be unique.

   Returns  -1 if error; ref of the new vgroup (a +ve integre) if ok.

   --------------------------------------------------------------------------- */

int32
VHmakegroup(HFILEID f, int32 tagarray[], int32 refarray[], int32 n, const char *vgname, const char *vgclass)
{
    int32       ref, i;
    int32       vg;
    int32       ret_value = SUCCEED;
    CONSTR(FUNC, "VHmakegroup");


    if (( vg = Vattach(f, -1, "w"))== FAIL)
        HGOTO_ERROR(DFE_CANTATTACH,FAIL);

    if(vgname!=NULL)
        if(Vsetname(vg, vgname)==FAIL)
            HGOTO_ERROR(DFE_BADVGNAME,FAIL);

    if(vgclass!=NULL)
        if(Vsetclass(vg, vgclass)==FAIL)
            HGOTO_ERROR(DFE_BADVGCLASS,FAIL);

    for (i = 0; i < n; i++)
      {
          if ( Vaddtagref(vg, tagarray[i], refarray[i]) == FAIL)
              HGOTO_ERROR(DFE_CANTADDELEM,FAIL);
      }

    ref = VQueryref(vg);
    if(Vdetach(vg)==FAIL)
        HGOTO_ERROR(DFE_CANTDETACH,FAIL);

    ret_value = (ref);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */

  return ret_value;
}   /* VHmakegroup */

/* ------------------------------------------------------------------ */
