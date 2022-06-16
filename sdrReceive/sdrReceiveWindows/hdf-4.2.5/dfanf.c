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
static char RcsId[] = "@(#)$Revision: 5322 $";
#endif

/* $Id: dfanf.c 5322 2010-01-19 06:26:11Z brtnfld $ */

/*-----------------------------------------------------------------------------
 * File:    dfanF.c
 * Purpose: Fortran stubs for annotation routines
 * Invokes: dfan.c dfkit.c
 * Contents:
 *
 *  daiganl_     : get length of annotation of tag/ref
 *  daigann_     : get annotation of tag/ref
 *  daipann_     : put annotation of tag/ref
 *  dailist_     : get list of refs and labels for a given tag
 *  dalref_      : return last ref written or read
 *  daclear_     : reset annotation internal structures
 *  dfanlastref_ : return last ref written or read
 *
 *  dfanaddfds_    : add file description
 *  dfangetfidlen_ : get length of file id
 *  dfangetfdslen_ : get length of file description
 *  dfangetfid_    : get file id
 *  dfangetfds_    : get file description
 *  daafds_        : get file description
 *  dagfidl_       : get file id length
 *  dagfdsl_       : get file description length
 *  dagfid_        : get file id
 *  dagfds_        : get file description
 *
 *  daiafid_       : add file id (intermediate routine)
 *---------------------------------------------------------------------------*/
#include "dfan.h"
#include "df.h"
#include "hproto_fortran.h"

/* conventions used in forming names of routines:
   **
   **    dfan: hdf annotation routine (<dfan>addfds)
   **    add:  add item to file       dfan<add>fds
   **    get:  get item from file     dfan<get>fds
   **    f:    file                   dfanadd<f>ds
   **    id:   id                     dfanaddf<id>
   **    ds:   description            dfanaddf<ds>
   **    len:  length                 dfanaddfid<len>
   **    l:    length (short forms)   dagfid<l>
   **    da:   dfan (short forms)     <da>gfid
   **    a:    add (short forms)      da<a>fds
   **    g:    get (short forms)      da<g>fds
   **    i:    intermediate routine (not in user interface) da<i>afid
   * */

/*---------------------------------------------------------------------------
** Routines for handling tag/ref (not file) annotations
 *-------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
 * Name:    daclear
 * Purpose: Call DFANIclear to clear Lastref and DFANdir[i]
 * Inputs:  none
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF Fortran programmers
 * Invokes: DFSDIclear
 *-------------------------------------------------------------------------*/

FRETVAL(intf)
ndaclear(void)
{
    return(DFANIclear());
}

/*-----------------------------------------------------------------------------
 * Name:    daiganl
 * Purpose: get length of annotation of tag/ref
 * Inputs:  filename: name of HDF file
 *          tag, ref: tag/ref of item of which we want label
 *          type: DFAN_LABEL if label, DFAN_DESC if description
 *          fnlen: length of filename
 * Returns: length of annotation on success, -1 on failure with DFerror set
 * Users:   HDF HLL users, utilities, other routines
 * Invokes: DFANIgetannlen, HDf2cstring, DFIfreespace
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndaiganl(_fcd filename, intf * tag, intf * ref, intf * type, intf * fnlen)
{
    char       *fn;
    intf        ret;

    fn = HDf2cstring(filename, (intn) *fnlen);
    if (!fn)
	return(-1);
    ret = DFANIgetannlen(fn, (uint16) *tag, (uint16) *ref, (intn) *type);
    HDfree((VOIDP) fn);

    return (ret);
}

/*-----------------------------------------------------------------------------
 * Name:    daigann
 * Purpose: get annotation of tag/ref
 * Inputs:  filename: name of HDF file
 *          tag, ref: tag/ref of item of which we want label
 *          annotation: space to return label in
 *          maxlen: size of space to return label in
 *          type: DFAN_LABEL if label, DFAN_DESC if description
 *          fnlen: length of filename
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF HLL users, utilities, other routines
 * Invokes: DFANIgetann
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndaigann(_fcd filename, intf * tag, intf * ref, _fcd annotation, intf * maxlen,
         intf * type, intf * fnlen)
{
    char       *fn;
    intf        ret;

    fn = HDf2cstring(filename, (intn) *fnlen);
    if (!fn)
	return(-1);
    ret = DFANIgetann(fn, (uint16) *tag, (uint16) *ref,
		      (uint8 *) _fcdtocp(annotation), (int32) *maxlen, (intn) *type, 1);
    HDfree((VOIDP) fn);

    return (ret);
}

/*-----------------------------------------------------------------------------
 * Name:    daipann
 * Purpose: put annotation of tag/ref
 * Inputs:  filename: name of HDF file
 *          tag, ref: tag/ref of item of which we want label
 *          annotation: space to return label in
 *          annlen: length of annotation
 *          type: DFAN_LABEL if label, DFAN_DESC if description
 *          fnlen: length of filename
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF HLL users, utilities, other routines
 * Invokes: DFANIgetann
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndaipann(_fcd filename, intf * tag, intf * ref, _fcd annotation,
         intf * annlen, intf * type, intf * fnlen)
{
    char       *fn;
    intf        ret;

    fn = HDf2cstring(filename, (intn) *fnlen);
    if (!fn)
	return(-1);
    ret = DFANIputann(fn, (uint16) *tag, (uint16) *ref,
             (uint8 *) _fcdtocp(annotation), (int32) *annlen, (intn) *type);
    HDfree((VOIDP) fn);
    return (ret);
}

/*-----------------------------------------------------------------------------
 * Name:    dailist
 * Purpose: Return list of refs and labels for a given tag
 * Inputs:  filename: name of HDF file
 *          tag: tag to get list of refs and labels for
 *          reflist: array to place refs in
 *          labellist: array of strings to place labels in
 *          listsize: size of ref and label lists
 *          maxlen: maximum length allowed for label
 *          startpos: beginning from the startpos'th entry, upto listsize
 *              entries will be returned.
 *          fnlen: length of filename
 * Returns: number of entries on success, -1 on error with DFerror set
 * Users:   HDF users, utilities, other routines
 * Invokes: DFANIlablist
 * Method:  call DFANIlablist
 * Remarks: none
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndailist(_fcd filename, intf * tag, intf reflist[], _fcd labellist,
         intf * listsize, intf * maxlen, intf * startpos, intf * fnlen)
{
    char       *fn;
    int         i;
    intf        nrefs;
    uint16     *tempreflist;

    fn = HDf2cstring(filename, (intn) *fnlen);
    if (!fn)
	return(-1);

    /* create reflist with true uint16s to maintain compatibility
       ** with machines that allocate more than 16 bits per uint16.
     */
    tempreflist = (uint16 *) HDmalloc((size_t) (*listsize) * sizeof(uint16));
    /* 1 for isfortran */
    nrefs = DFANIlablist(fn, (uint16) *tag, tempreflist,
                         (uint8 *) _fcdtocp(labellist),
                         (int) *listsize, (int) *maxlen, (int) *startpos, 1);
    if (nrefs < 0)
        return FAIL;

    /* move ref numbers into caller's reflist */
    for (i = 0; i < *listsize; i++)
        reflist[i] = (intf)tempreflist[i];

    HDfree((VOIDP) fn);
    HDfree((VOIDP) tempreflist);

    return (nrefs);
}

/*-----------------------------------------------------------------------------
 * Name:    dalref
 * Purpose: Return last ref written or read
 * Inputs:  none
 * Globals: Lastref
 * Returns: ref on success, -1 on error with DFerror set
 * Users:   HDF users, utilities, other routines
 * Invokes: DFANlastref
 * Remarks: none
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndalref(void)
{
    return ((intf)DFANlastref());
}

/*-----------------------------------------------------------------------------
 * Name:    dfanlastref
 * Purpose: Return last ref written or read
 * Inputs:  none
 * Globals: Lastref
 * Returns: ref on success, -1 on error with DFerror set
 * Users:   HDF users, utilities, other routines
 * Invokes: DFANlastref
 * Remarks: none
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfanlastref(void)
{
    return ((intf)DFANlastref());
}

/*---------------------------------------------------------------------------
** Routines for handling file annotations
 *-------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Name:    dfanaddfds
 * Purpose: add file description (Fortran callable C version)
 * Inputs:  dfile: pointer to HDF file
 *          desc: description to write to file
 *          desclen: length of description
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF HLL users, utilities, other routines
 * Invokes: DFANaddfileann
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfanaddfds(intf * dfile, _fcd desc, intf * desclen)
{
    return (DFANIaddfann(*dfile, _fcdtocp(desc), *desclen, DFAN_DESC));
}

/*-----------------------------------------------------------------------------
 * Name:    dfangetfidlen
 * Purpose: get length of next file ID (Fortran callable C version)
 * Inputs:  dfile: pointer to HDF file
 *          isfirst: 1: start with first one; 0: get length of next one
 * Returns: On success: length of next file ID; On failure: -1, with DFerror set
 * Users:   HDF HLL users, utilities, other routines
 * Invokes: DFANIgetfannlen
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfangetfidlen(intf * dfile, intf * isfirst)
{
    return (DFANIgetfannlen(*dfile, DFAN_LABEL, (intn) *isfirst));
}

/*-----------------------------------------------------------------------------
 * Name:    dfangetfdslen
 * Purpose: get length of next file description (Fortran callable C version)
 * Inputs:  dfile: pointer to HDF file
 *          isfirst: 1: start with first one; 0: get length of next one
 * Returns: On success: length of next file ID; On failure: -1, with DFerror set
 * Users:   HDF HLL users, utilities, other routines
 * Invokes: DFANIgetfannlen
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfangetfdslen(intf * dfile, intf * isfirst)
{
    return (DFANIgetfannlen(*dfile, DFAN_DESC, (intn) *isfirst));
}

/*-----------------------------------------------------------------------------
 * Name:    dfangetfid
 * Purpose: get file ID (Fortran callable C version)
 * Inputs:  dfile: pointer to HDF file
 *          desc: description to write to file
 *          desclen: length of description
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF HLL users, utilities, other routines
 * Invokes: DFANgetfann
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfangetfid(intf * dfile, _fcd id, intf * maxlen, intf * isfirst)
{
    return (DFANIgetfann(*dfile, _fcdtocp(id), *maxlen,
                         DFAN_LABEL, (intn) *isfirst));
}

/*-----------------------------------------------------------------------------
 * Name:    dfangetfds
 * Purpose: get file description (Fortran callable C version)
 * Inputs:  dfile: pointer to HDF file
 *          desc: description to write to file
 *          desclen: length of description
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF HLL users, utilities, other routines
 * Invokes: DFANgetfann
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndfangetfds(intf * dfile, _fcd id, intf * maxlen, intf * isfirst)
{
    return (DFANIgetfann(*dfile, _fcdtocp(id), *maxlen,
                         DFAN_DESC, (intn) *isfirst));
}

/*-----------------------------------------------------------------------------
** Versions with short names
**---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Name:    daafds
 * Purpose: add file description (short form of DFANaddfds; Fortran callable)
 * Inputs:  dfile: pointer to HDF file
 *          desc: description to write to file
 *          desclen: length of description
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF HLL users, utilities, other routines
 * Invokes: DFANaddfileann
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndaafds(intf * dfile, _fcd desc, intf * desclen)
{
    return (DFANIaddfann(*dfile, _fcdtocp(desc), *desclen, DFAN_DESC));
}

/*-----------------------------------------------------------------------------
 * Name:    dagfidl
 * Purpose: get length of next file ID
 * Inputs:  dfile: pointer to HDF file
 *          isfirst: 1: start with first one; 0: get length of next one
 * Returns: On success: length of next file ID; On failure: -1, with DFerror set
 * Users:   HDF HLL users, utilities, other routines
 * Invokes: DFANIgetfannlen
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndagfidl(intf * dfile, intf * isfirst)
{
    return (DFANIgetfannlen(*dfile, DFAN_LABEL, (intn) *isfirst));
}

/*-----------------------------------------------------------------------------
 * Name:    dagfdsl
 * Purpose: get length of next file description (Fortran callable C version)
 * Inputs:  dfile: pointer to HDF file
 *          isfirst: 1: start with first one; 0: get length of next one
 * Returns: On success: length of next file ID; On failure: -1, with DFerror set
 * Users:   HDF HLL users, utilities, other routines
 * Invokes: DFANIgetfannlen
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndagfdsl(intf * dfile, intf * isfirst)
{
    return (DFANIgetfannlen(*dfile, DFAN_DESC, (intn) *isfirst));
}

/*-----------------------------------------------------------------------------
 * Name:    dagfid
 * Purpose: get file ID (short form of DFANgetfid; Fortran callable version)
 * Inputs:  dfile: pointer to HDF file
 *          desc: description to write to file
 *          desclen: length of description
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF HLL users, utilities, other routines
 * Invokes: DFANIgetfann
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndagfid(intf * dfile, _fcd id, intf * maxlen, intf * isfirst)
{
    return (DFANIgetfann(*dfile, _fcdtocp(id), *maxlen,
                         DFAN_LABEL, (intn) *isfirst));
}

/*-----------------------------------------------------------------------------
 * Name:    dagfds
 * Purpose: get file description
 *          (short form of DFANgetfds; Fortran callable C version)
 * Inputs:  dfile: pointer to HDF file
 *          desc: description to write to file
 *          desclen: length of description
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   HDF HLL users, utilities, other routines
 * Invokes: DFANgetfann
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndagfds(intf * dfile, _fcd id, intf * maxlen, intf * isfirst)
{
    return (DFANIgetfann(*dfile, _fcdtocp(id), *maxlen,
                         DFAN_DESC, (intn) *isfirst));
}

/*-----------------------------------------------------------------------------
** Intermediate routines called from user's fortran routines
**---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Name:    daiafid
 * Purpose: intermediate routine to add file ID (Fortran callable C version)
 * Inputs:  dfile: pointer to HDF file
 *          id: ID to write to file
 *          idlen: length of ID string
 * Returns: 0 on success, -1 on failure with DFerror set
 * Users:   Fortran user routines DFANaddfid and daafid
 * Invokes: DFANaddfann
 *---------------------------------------------------------------------------*/

FRETVAL(intf)
ndaiafid(intf * dfile, _fcd id, intf * idlen)
{
    return (DFANIaddfann(*dfile, _fcdtocp(id), *idlen, DFAN_LABEL));
}
