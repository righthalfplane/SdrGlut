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

/* $Id: cnone.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*
   FILE
   cnone.c
   HDF none encoding I/O routines

   REMARKS
   These routines are only included for completeness and are not
   actually expected to be used.

   DESIGN

   EXPORTED ROUTINES
   None of these routines are designed to be called by other users except
   for the modeling layer of the compression routines.

   AUTHOR
   Quincey Koziol

   MODIFICATION HISTORY
   4/25/94     Starting writing specs & coding prototype.
 */

/* General HDF includes */
#include "hdf.h"

#define CNONE_MASTER
#define CODER_CLIENT
/* HDF compression includes */
#include "hcompi.h"     /* Internal definitions for compression */

/* declaration of the functions provided in this module */
PRIVATE int32 HCIcnone_staccess
            (accrec_t * access_rec, int16 acc_mode);

/*--------------------------------------------------------------------------
 NAME
    HCIcnone_staccess -- Start accessing a RLE compressed data element.

 USAGE
    int32 HCIcnone_staccess(access_rec, access)
    accrec_t *access_rec;   IN: the access record of the data element
    int16 access;           IN: the type of access wanted

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Common code called by HCIcnone_stread and HCIcnone_stwrite

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE int32
HCIcnone_staccess(accrec_t * access_rec, int16 acc_mode)
{
    CONSTR(FUNC, "HCIcnone_staccess");
    compinfo_t *info;           /* special element information */

    info = (compinfo_t *) access_rec->special_info;

    if (acc_mode == DFACC_READ)
        info->aid = Hstartread(access_rec->file_id, DFTAG_COMPRESSED,
                               info->comp_ref);
    else
        info->aid = Hstartwrite(access_rec->file_id, DFTAG_COMPRESSED,
                                info->comp_ref, info->length);

    if (info->aid == FAIL)
        HRETURN_ERROR(DFE_DENIED, FAIL);
    if ((acc_mode&DFACC_WRITE) && Happendable(info->aid) == FAIL)
        HRETURN_ERROR(DFE_DENIED, FAIL);
    return (SUCCEED);
}   /* end HCIcnone_staccess() */

/*--------------------------------------------------------------------------
 NAME
    HCPcnone_stread -- start read access for compressed file

 USAGE
    int32 HCPcnone_stread(access_rec)
    accrec_t *access_rec;   IN: the access record of the data element

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Start read access on a compressed data element using no compression.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPcnone_stread(accrec_t * access_rec)
{
    CONSTR(FUNC, "HCPcnone_stread");
    int32       ret;

    if ((ret = HCIcnone_staccess(access_rec, DFACC_READ)) == FAIL)
        HRETURN_ERROR(DFE_CINIT, FAIL);
    return (ret);
}   /* HCPcnone_stread() */

/*--------------------------------------------------------------------------
 NAME
    HCPcnone_stwrite -- start write access for compressed file

 USAGE
    int32 HCPcnone_stwrite(access_rec)
    accrec_t *access_rec;   IN: the access record of the data element

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Start write access on a compressed data element using no compression.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPcnone_stwrite(accrec_t * access_rec)
{
    CONSTR(FUNC, "HCPcnone_stwrite");
    int32       ret;

    if ((ret = HCIcnone_staccess(access_rec, DFACC_WRITE)) == FAIL)
        HRETURN_ERROR(DFE_CINIT, FAIL);
    return (ret);
}   /* HCPcnone_stwrite() */

/*--------------------------------------------------------------------------
 NAME
    HCPcnone_seek -- Seek to offset within the data element

 USAGE
    int32 HCPcnone_seek(access_rec,offset,origin)
    accrec_t *access_rec;   IN: the access record of the data element
    int32 offset;       IN: the offset in bytes from the origin specified
    intn origin;        IN: the origin to seek from [UNUSED!]

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Seek to a position with a compressed data element.  The 'origin'
    calculations have been taken care of at a higher level, it is an
    un-used parameter.  The 'offset' is used as an absolute offset
    because of this.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPcnone_seek(accrec_t * access_rec, int32 offset, int origin)
{
    CONSTR(FUNC, "HCPcnone_seek");
    compinfo_t *info;           /* special element information */

    info = (compinfo_t *) access_rec->special_info;

    if (Hseek(info->aid, offset, origin) == FAIL)
        HRETURN_ERROR(DFE_CSEEK, FAIL);

    return (SUCCEED);
}   /* HCPcnone_seek() */

/*--------------------------------------------------------------------------
 NAME
    HCPcnone_read -- Read in a portion of data from a compressed data element.

 USAGE
    int32 HCPcnone_read(access_rec,length,data)
    accrec_t *access_rec;   IN: the access record of the data element
    int32 length;           IN: the number of bytes to read
    void * data;             OUT: the buffer to place the bytes read

 RETURNS
    Returns the number of bytes read or FAIL

 DESCRIPTION
    Read in a number of bytes from a RLE compressed data element.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPcnone_read(accrec_t * access_rec, int32 length, void * data)
{
    CONSTR(FUNC, "HCPcnone_read");
    compinfo_t *info;           /* special element information */

    info = (compinfo_t *) access_rec->special_info;

    if (Hread(info->aid, length, data) == FAIL)
        HRETURN_ERROR(DFE_CDECODE, FAIL);

    return (length);
}   /* HCPcnone_read() */

/*--------------------------------------------------------------------------
 NAME
    HCPcnone_write -- Write out a portion of data from a compressed data element.

 USAGE
    int32 HCPwrite(access_rec,length,data)
    accrec_t *access_rec;   IN: the access record of the data element
    int32 length;           IN: the number of bytes to write
    void * data;             IN: the buffer to retrieve the bytes written

 RETURNS
    Returns the number of bytes written or FAIL

 DESCRIPTION
    Write out a number of bytes to a data element (w/ no compression).

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPcnone_write(accrec_t * access_rec, int32 length, const void * data)
{
    CONSTR(FUNC, "HCPcnone_write");
    compinfo_t *info;           /* special element information */

    info = (compinfo_t *) access_rec->special_info;

    if (Hwrite(info->aid, length, data) == FAIL)
        HRETURN_ERROR(DFE_CENCODE, FAIL);

    return (length);
}   /* HCPcnone_write() */

/*--------------------------------------------------------------------------
 NAME
    HCPcnone_inquire -- Inquire information about the access record and data element.

 USAGE
    int32 HCPcnone_inquire(access_rec,pfile_id,ptag,pref,plength,poffset,pposn,
            paccess,pspecial)
    accrec_t *access_rec;   IN: the access record of the data element
    int32 *pfile_id;        OUT: ptr to file id
    uint16 *ptag;           OUT: ptr to tag of information
    uint16 *pref;           OUT: ptr to ref of information
    int32 *plength;         OUT: ptr to length of data element
    int32 *poffset;         OUT: ptr to offset of data element
    int32 *pposn;           OUT: ptr to position of access in element
    int16 *paccess;         OUT: ptr to access mode
    int16 *pspecial;        OUT: ptr to special code

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Inquire information about the access record and data element.
    [Currently a NOP].

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
HCPcnone_inquire(accrec_t * access_rec, int32 *pfile_id, uint16 *ptag,
                 uint16 *pref, int32 *plength, int32 *poffset,
                 int32 *pposn, int16 *paccess, int16 *pspecial)
{
    /* shut compiler up */
    access_rec = access_rec;
    pfile_id = pfile_id;
    ptag = ptag;
    pref = pref;
    plength = plength;
    poffset = poffset;
    pposn = pposn;
    paccess = paccess;
    pspecial = pspecial;

    return (SUCCEED);
}   /* HCPcnone_inquire() */

/*--------------------------------------------------------------------------
 NAME
    HCPcnone_endaccess -- Close the compressed data element

 USAGE
    int32 HCPcnone_endaccess(access_rec)
    accrec_t *access_rec;   IN: the access record of the data element

 RETURNS
    Returns SUCCEED or FAIL

 DESCRIPTION
    Close the compressed data element and free modelling info.

 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
HCPcnone_endaccess(accrec_t * access_rec)
{
    CONSTR(FUNC, "HCPcnone_endaccess");
    compinfo_t *info;           /* special element information */

    info = (compinfo_t *) access_rec->special_info;

    /* close the compressed data AID */
    if (Hendaccess(info->aid) == FAIL)
        HRETURN_ERROR(DFE_CANTCLOSE, FAIL);

    return (SUCCEED);
}   /* HCPcnone_endaccess() */
